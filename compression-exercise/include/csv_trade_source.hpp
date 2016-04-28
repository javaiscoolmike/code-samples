#pragma once

#include <boost/iterator/iterator_facade.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <memory>
#include "trade.hpp"

namespace exercise
{
    namespace detail 
    {
        /*
         * iterator of trade data
         */
        struct csv_source_iterator
        : boost::iterator_facade<
                csv_source_iterator,
                trade,
                boost::forward_traversal_tag,
                trade>
        {
            explicit csv_source_iterator(std::shared_ptr<std::ifstream> stream_);
            csv_source_iterator();

            void increment();
            bool equal(csv_source_iterator const& other) const;
            trade dereference() const;

        private:
            void parse_line();
            std::shared_ptr<std::ifstream> stream;
            std::string line;
            char line_ending;
        };
    }
    
    /*
     * Convert one line from a csv input file to a trade
     */
    trade parse_trade(const std::string& line, const char new_line);
     
    /*
     * Creates an iterator_range of all of the trades in a  given csv input file.
     * Iterator of variables of type exercise::trade. Each trade is read from a
     * csv input file. This is used to make a iterator_range of all of the trades in a
     * given csv input file. Each trade is read lazily. 
     */
    boost::iterator_range<detail::csv_source_iterator> make_csv_source_range(const boost::filesystem::path& path);
}