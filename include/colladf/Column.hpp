#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <limits>
#include <algorithm>
#include <unordered_set>

enum class DataType {
    INTEGER,
    DOUBLE,
    STRING
};

struct ColumnStats {
    size_t count = 0;
    double mean = 0.0;
    double min = std::numeric_limits<double>::max();
    double max = std::numeric_limits<double>::lowest();
};

class Series{
    public:
        virtual ~Series() = default;
        virtual size_t size() const = 0; 
        virtual DataType type() const = 0; 
        virtual std::shared_ptr<Series> apply_mask(const std::vector<bool>& mask) const = 0;
};

template <typename T> 
class Column : public Series{
    private:
        std::vector<T> column;
        ColumnStats stat;

    public:
        
        Column() = default;

        Column(std::vector<T>& data) : column(std::move(data)) {}

        size_t size() const override { return column.size(); }
        std::vector<T>& get_column() { return column; }

        DataType type() const override {
            if constexpr (std::is_same_v<T, int64_t>) return DataType::INTEGER;
            else if constexpr (std::is_same_v<T, double>) return DataType::DOUBLE;
            else if constexpr (std::is_same_v<T, std::string>) return DataType::STRING;
            else throw std::runtime_error("Unsupported Data Type");
        }

        ColumnStats compute_stats(){
            stat.count = column.size();
            
            if (stat.count == 0) return stat;

            if constexpr (std::is_same_v<T, std::string>) {
                return stat;
            } 

            double sum = 0.0;

            for (const T& val : column) {
                double d_val = static_cast<double>(val);
                
                if (d_val < stat.min) stat.min = d_val;
                if (d_val > stat.max) stat.max = d_val;
                sum += d_val;
            }
            
            stat.mean = sum / stat.count;
            return stat;
        }

        std::shared_ptr<Series> apply_mask(const std::vector<bool>& mask){
            if (mask.size() != column.size()) {
                throw std::invalid_argument("Mask size must match column size");
            }

            std::vector<T> result; 

            for (size_t i = 0; i < column.size(); ++i) {
                if (mask[i]) {
                    result.push_back(column[i]);
                }
            }
            
            return std::make_shared<Column<T>>(std::move(result));
        }


        std::vector<bool> operator>(const T& other){
            std::vector<bool> result;
            for(T& data : column){
                result.push_back(other > data);
            }
            return result;
        }

        std::vector<bool> operator<(const T& other){
            std::vector<bool> result;
            for(T& data : column){
                result.push_back(other < data);
            }
            return result;
        }

        std::vector<bool> operator==(const T& other){
            std::vector<bool> result;
            for(T& data : column){
                result.push_back(other == data);
            }
            return result;
        }

        std::vector<bool> operator<=(const T& other){
            std::vector<bool> result;
            for(T& data : column){
                result.push_back(other <= data);
            }
            return result;
        }

        std::vector<bool> operator>=(const T& other){
            std::vector<bool> result;
            for(T& data : column){
                result.push_back(other >= data);
            }
            return result;
        }
        
        std::vector<bool> operator!=(const T& other){
            std::vector<bool> result;
            for(T& data : column){
                result.push_back(other != data);
            }
            return result;
        }
};