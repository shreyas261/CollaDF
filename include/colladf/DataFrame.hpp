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

        DataFrame(const std::string& name,const std::shared_ptr<Series>& col){
            dataframe[name] = col;
            column_names.push_back(name);
        }

        void add_column(const std::string& name,const std::shared_ptr<Series>& col) {
            dataframe[name] = col;
            column_names.push_back(name);
        }

        std::shared_ptr<Series> get_column(const std::string& name) const {
            auto itr = dataframe.find(name);
            if(itr != dataframe.end()) return itr -> second;
            throw std::invalid_argument("Column '" + name + "' does not exist.");
        }

        DataFrame operator[](const std::vector<bool>& mask);
        DataFrame operator[](const std::string& name);

        DataFrame operator+(const ScalarValue& v) const; 
        DataFrame operator-(const ScalarValue& v) const; 
        DataFrame operator*(const ScalarValue& v) const;
        DataFrame operator/(const ScalarValue& v) const;

        std::vector<bool> operator>(const ScalarValue& v) const;
        std::vector<bool> operator<(const ScalarValue& v) const;
        std::vector<bool> operator==(const ScalarValue& v) const;
        std::vector<bool> operator>=(const ScalarValue& v) const; 
        std::vector<bool> operator<=(const ScalarValue& v) const;
        std::vector<bool> operator!=(const ScalarValue& v) const;


        DataFrame operator+(const DataFrame& other) const; 
        DataFrame operator-(const DataFrame& other) const; 
        DataFrame operator*(const DataFrame& other) const; 
        DataFrame operator/(const DataFrame& other) const; 




        size_t num_columns() const { return dataframe.size(); }; 
        size_t num_rows() const { return dataframe.size() > 0 ? dataframe.at(column_names[0]) -> size() : 0; }
        std::pair<int,int> shape() const {return {num_rows(), num_columns()}; }
        const std::vector<std::string>& columns() const { return column_names; }


        // Basic utilities for DF
        DataFrame head(int n = 5) const;
        DataFrame tail(int n = 5) const;
        void describe() const;

        // Subset selection
        DataFrame select(const std::vector<std::string>& columns) const;
        DataFrame drop(const std::vector<std::string>& columns) const;
        DataFrame filter(const std::vector<bool>& mask) const;




};
