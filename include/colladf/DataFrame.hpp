#pragma once
#include "Column.hpp"

class GroupBy;

std::vector<uint8_t>  operator&(const std::vector<uint8_t> & a,const std::vector<uint8_t> & b);
std::vector<uint8_t>  operator|(const std::vector<uint8_t> & a,const std::vector<uint8_t> & b);
std::vector<uint8_t>  operator!(const std::vector<uint8_t> & a);


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

        const std::shared_ptr<Series>& get_column(const std::string& name) const {
            auto itr = dataframe.find(name);
            if(itr != dataframe.end()) return itr -> second;
            throw std::invalid_argument("Column '" + name + "' does not exist.");
        }

        // void to_csv(const std::string& filepath) const { Reader::to_csv(*this,filepath); } 

        DataFrame operator[](const std::vector<uint8_t> & mask);
        DataFrame operator[](const std::string& name);

        DataFrame operator+(const ScalarValue& v) const; 
        DataFrame operator-(const ScalarValue& v) const; 
        DataFrame operator*(const ScalarValue& v) const;
        DataFrame operator/(const ScalarValue& v) const;

        std::vector<uint8_t>  operator>(const ScalarValue& v) const;
        std::vector<uint8_t>  operator<(const ScalarValue& v) const;
        std::vector<uint8_t>  operator==(const ScalarValue& v) const;
        std::vector<uint8_t>  operator>=(const ScalarValue& v) const; 
        std::vector<uint8_t>  operator<=(const ScalarValue& v) const;
        std::vector<uint8_t>  operator!=(const ScalarValue& v) const;


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
        DataFrame filter(const std::vector<uint8_t> & mask) const;

        //Apply a lamda
        template <typename T, typename Func>
        std::shared_ptr<Series> apply(const std::string& col_name, Func f) const {
            
            std::shared_ptr<Series> base_series = this->get_column(col_name);

            auto* typed_col = dynamic_cast<Column<T>*>(base_series.get());
            if (!typed_col) {
                throw std::invalid_argument("Apply Error: Column is not of the specified type T.");
            }
            using R = std::invoke_result_t<Func, T>;
            std::vector<R> new_data;
            new_data.reserve(typed_col->size());
            for (const T& val : typed_col->get_column()) {
                new_data.push_back(f(val));
            }
            return std::make_shared<Column<R>>(std::move(new_data));
        }

        //Sorting
        DataFrame sort_values(const std::vector<std::string>& columns,bool ascending = true) const;
        DataFrame sort_values(const std::string& name,bool ascending = true) const;

        //Aggregations
        GroupBy groupby(const std::string& col) const;
        GroupBy groupby(const std::vector<std::string>& cols) const;

};
