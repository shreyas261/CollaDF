#pragma once
#include "Column.hpp"


class DataFrame{
    private:
        std::unordered_map<std::string,std::shared_ptr<Series>> dataframe;
        std::vector<std::string> column_names;


    public:
        DataFrame() = default;

        DataFrame(const std::vector<std::string>& cols){
            for(const std::string& c : cols){
    
                dataframe.insert({c,nullptr});
                column_names.push_back(c);
            }
        }

        DataFrame(const std::string& name, std::shared_ptr<Series>& col){
            dataframe[name] = col;
            column_names.push_back(name);
        }

        void add_column(const std::string& name, std::shared_ptr<Series>& col) {
            dataframe[name] = col;
            column_names.push_back(name);
        }

        std::shared_ptr<Series> get_column(const std::string& name) const {
            auto itr = dataframe.find(name);
            if(itr != dataframe.end()) return itr -> second;
            throw std::invalid_argument("Column '" + name + "' does not exist.");
        }

        size_t num_columns() const { return dataframe.size(); }; 
        const std::vector<std::string>& columns() const { return column_names; }

        

};
