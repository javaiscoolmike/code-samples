#pragma once

#include "trade.hpp"
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/rolling_mean.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/optional/optional.hpp>

using namespace boost::accumulators;

namespace exercise
{
    /*
     * Stores previous values for a given symbol.
     */
    struct symbol_data
    {
        symbol_data()
        {}
        void add_trade(const trade& trade_)
        {
            previous_price = trade_.price;
            previous_quantity = trade_.quantity;
        }
        
        int64_t predict_prices() const
        {
            if(!previous_price)
                return 0;
            else
                return previous_price.get();
        }
        
        int64_t predict_quantity() const
        {
            if(!previous_quantity)
                return 0;
            else
                return previous_quantity.get();
        }
                
    private:
        boost::optional<int32_t> previous_price;
        boost::optional<int32_t> previous_quantity;
    };
    
    /*
     * Makes predictions of values bases on previous values.
     */
    struct stats_collector
    {
        stats_collector():
        send_time_gap_acc(tag::rolling_window::window_size = 100),
        received_time_diff_acc(tag::rolling_window::window_size = 100)
        {}
        
        int64_t predict_send_time() const
        {
            if(!previous_send_time)
                return 0;
            
            if(count(send_time_gap_acc) == 0)
                return previous_send_time.get();
            
            return previous_send_time.get() + mean(send_time_gap_acc);
        }
        
        int64_t predict_received_time_diff() const
        {
            if(!previous_received_time_diff)
                return 0;
            
            if(count(received_time_diff_acc) == 0)
                return previous_received_time_diff.get();
            
            return previous_received_time_diff.get() + mean(received_time_diff_acc);
        }
        
        int64_t predict_price(const std::string& symbol) const
        {
            int64_t prediction = 0;
            
            if(symbol_stat_map.find(symbol) != boost::end(symbol_stat_map))
                prediction = symbol_stat_map.find(symbol)->second.predict_prices();
            
            return prediction;
        }
        
        int64_t predict_quantity(const std::string& symbol) const
        {
            int64_t prediction = 0;
            
            if(symbol_stat_map.find(symbol) != boost::end(symbol_stat_map))
                prediction = symbol_stat_map.find(symbol)->second.predict_quantity();
            
            return prediction;
        }
        
        void add_trade(const trade& trade_)
        {
            if(previous_send_time)
            {
                send_time_gap_acc(trade_.send_time - previous_send_time.get());
                received_time_diff_acc((trade_.receive_time - trade_.send_time) - previous_received_time_diff.get());
            }
            
            previous_send_time = trade_.send_time;
            previous_received_time_diff = trade_.receive_time - trade_.send_time;
                
            symbol_stat_map[trade_.symbol].add_trade(trade_);   
        }
        
    private:
        //Map from symbol to its previous values
        std::map<std::string, symbol_data> symbol_stat_map;
        boost::optional<int64_t> previous_send_time;
        boost::optional<int64_t> previous_received_time_diff;
        accumulator_set<double, stats<tag::mean, tag::count>> send_time_gap_acc;
        accumulator_set<double, stats<tag::mean, tag::count>> received_time_diff_acc;
    };
}