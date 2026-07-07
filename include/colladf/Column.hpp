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
#include <variant>
#include <numeric>


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

using ScalarValue = std::variant<int64_t, double, std::string>;

class Series{
    public:
        virtual ~Series() = default;
        virtual size_t size() const = 0; 
        virtual DataType type() const = 0; 
        virtual std::shared_ptr<Series> apply_mask(const std::vector<bool>& mask) const = 0;
        virtual std::vector<size_t> argsort(bool ascending = true) const = 0;
        virtual std::shared_ptr<Series> reorder(const std::vector<size_t>& indices) const = 0;

        virtual std::vector<bool> compare_gt(const ScalarValue& v) const = 0;
        virtual std::vector<bool> compare_lt(const ScalarValue& v) const = 0;
        virtual std::vector<bool> compare_eq(const ScalarValue& v) const = 0;
        virtual std::vector<bool> compare_ge(const ScalarValue& v) const = 0;
        virtual std::vector<bool> compare_le(const ScalarValue& v) const = 0;
        virtual std::vector<bool> compare_ne(const ScalarValue& v) const = 0;

        virtual std::shared_ptr<Series> add(const ScalarValue& v) const = 0;
        virtual std::shared_ptr<Series> subtract(const ScalarValue& v) const = 0;
        virtual std::shared_ptr<Series> multiply(const ScalarValue& v) const = 0;
        virtual std::shared_ptr<Series> divide(const ScalarValue& v) const = 0;

        virtual std::shared_ptr<Series> add(const Series& other) const = 0;
        virtual std::shared_ptr<Series> subtract(const Series& other) const = 0;
        virtual std::shared_ptr<Series> multiply(const Series& other) const = 0;
        virtual std::shared_ptr<Series> divide(const Series& other) const = 0;
};

template <typename T> 
class Column : public Series{
    private:
        std::vector<T> column;
        ColumnStats stat;

        T extract(const ScalarValue& v) const {
            if (!std::holds_alternative<T>(v))
                throw std::invalid_argument("Type mismatch in comparison");
            return std::get<T>(v);
        }

    public:
        
        Column() = default;

        Column(std::vector<T>& data)  : column(data) {};
        Column(std::vector<T>&& data) : column(std::move(data)) {}

        size_t size() const override { return column.size(); }
        const std::vector<T>& get_column() const { return column; }

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

        std::shared_ptr<Series> apply_mask(const std::vector<bool>& mask) const {
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

        std::vector<bool> compare_gt(const ScalarValue& v) const override {
            T target = extract(v);
            std::vector<bool> result(column.size());
            for (size_t i = 0; i < column.size(); ++i)
                result[i] = column[i] > target;
            return result;
        }

        std::vector<bool> compare_lt(const ScalarValue& v) const override {
            T target = extract(v);
            std::vector<bool> result(column.size());
            for (size_t i = 0; i < column.size(); ++i)
                result[i] = column[i] < target;
            return result;
        }

        std::vector<bool> compare_eq(const ScalarValue& v) const override {
            T target = extract(v);
            std::vector<bool> result(column.size());
            for (size_t i = 0; i < column.size(); ++i)
                result[i] = column[i] == target;
            return result;
        }

        std::vector<bool> compare_ne(const ScalarValue& v) const override {
            T target = extract(v);
            std::vector<bool> result(column.size());
            for (size_t i = 0; i < column.size(); ++i)
                result[i] = column[i] != target;
            return result;
        }

        std::vector<bool> compare_le(const ScalarValue& v) const override {
            T target = extract(v);
            std::vector<bool> result(column.size());
            for (size_t i = 0; i < column.size(); ++i)
                result[i] = column[i] <= target;
            return result;
        }

        std::vector<bool> compare_ge(const ScalarValue& v) const override {
            T target = extract(v);
            std::vector<bool> result(column.size());
            for (size_t i = 0; i < column.size(); ++i)
                result[i] = column[i] >= target;
            return result;
        }
        

        std::shared_ptr<Series> add(const ScalarValue& v) const override {
            T target = extract(v);
            std::vector<T> result(column.size());
            
            for (size_t i = 0; i < column.size(); ++i)
                result[i] = column[i] + target;
            return std::make_shared<Column<T>>(std::move(result));
        }

        std::shared_ptr<Series> subtract(const ScalarValue& v) const override {
            T target = extract(v);
            if constexpr (std::is_same_v<T, std::string>) {
                throw std::invalid_argument("Arithmetic is not supported on string columns");
            }
            else{
                std::vector<T> result(column.size());
                for (size_t i = 0; i < column.size(); ++i)
                    result[i] = column[i] - target;
                return std::make_shared<Column<T>>(std::move(result));
        
            }
        }

        std::shared_ptr<Series> multiply(const ScalarValue& v) const override {
            T target = extract(v);

            if constexpr (std::is_same_v<T, std::string>) {
                throw std::invalid_argument("Arithmetic is not supported on string columns");
            }
            else{
                std::vector<T> result(column.size());
                for (size_t i = 0; i < column.size(); ++i)
                    result[i] = column[i] * target;
                return std::make_shared<Column<T>>(std::move(result));
            }
        }

        std::shared_ptr<Series> divide(const ScalarValue& v) const override {
            T target = extract(v);
            
            if constexpr (std::is_same_v<T, std::string>) {
                throw std::invalid_argument("Arithmetic is not supported on string columns");
            }
            else{
                std::vector<T> result(column.size());
                for (size_t i = 0; i < column.size(); ++i)
                    result[i] = column[i] / target;
                return std::make_shared<Column<T>>(std::move(result));
            }
        }


        std::shared_ptr<Series> add(const Series& other) const override {
            if (size() != other.size()) throw std::invalid_argument("Column size mismatch");

            if constexpr (std::is_same_v<T, std::string>) {
                if (other.type() != DataType::STRING) {
                    throw std::invalid_argument("Cannot concatenate a string column with a numeric column");
                }
                
                const auto& other_col = static_cast<const Column<std::string>&>(other).get_column();
                std::vector<std::string> result(column.size());
                
                for (size_t i = 0; i < column.size(); ++i) {
                    result[i] = column[i] + other_col[i]; // String concatenation
                }
                return std::make_shared<Column<std::string>>(std::move(result));
                
            } 
            else {
                if (other.type() == DataType::STRING) {
                    throw std::invalid_argument("Cannot add a numeric column to a string column");
                }

                if (type() == DataType::DOUBLE || other.type() == DataType::DOUBLE) {
                    std::vector<double> result(column.size());
                    
                    if (other.type() == DataType::DOUBLE) {
                        const auto& other_col = static_cast<const Column<double>&>(other).get_column();
                        for (size_t i = 0; i < column.size(); ++i) {
                            result[i] = static_cast<double>(column[i]) + other_col[i];
                        }
                    } else {
                        const auto& other_col = static_cast<const Column<int64_t>&>(other).get_column();
                        for (size_t i = 0; i < column.size(); ++i) {
                            result[i] = static_cast<double>(column[i]) + static_cast<double>(other_col[i]);
                        }
                    }
                    return std::make_shared<Column<double>>(std::move(result));
                }

                const auto& other_col = static_cast<const Column<int64_t>&>(other).get_column();
                std::vector<int64_t> result(column.size());
                for (size_t i = 0; i < column.size(); ++i) {
                    result[i] = column[i] + other_col[i];
                }
                return std::make_shared<Column<int64_t>>(std::move(result));
            }
        }
        
        std::shared_ptr<Series> subtract(const Series& other) const override {
            if constexpr (std::is_same_v<T, std::string>) {
                throw std::invalid_argument("Arithmetic is not supported on string columns");
            }
            else{
                if (size() != other.size()) {
                    throw std::invalid_argument(
                        "Column arithmetic requires equal-length columns (no index alignment)"
                    );
                }
            
                bool result_is_double = (type() == DataType::DOUBLE || other.type() == DataType::DOUBLE);

                if (result_is_double) {
                    std::vector<double> result(column.size());

                    if (other.type() == DataType::DOUBLE) {
                        const auto& other_col = static_cast<const Column<double>&>(other).get_column();
                        for (size_t i = 0; i < column.size(); ++i) {
                            result[i] = static_cast<double>(column[i]) - other_col[i];
                        }
                    } 
                    else {
                        const auto& other_col = static_cast<const Column<int64_t>&>(other).get_column();
                        for (size_t i = 0; i < column.size(); ++i) {
                            result[i] = static_cast<double>(column[i]) - static_cast<double>(other_col[i]);
                        }
                    }
                    return std::make_shared<Column<double>>(std::move(result));
                }

                
                const auto& other_col = static_cast<const Column<int64_t>&>(other).get_column();
                std::vector<int64_t> result(column.size());
                for (size_t i = 0; i < column.size(); ++i) {
                    result[i] = column[i] - other_col[i];
                }
                return std::make_shared<Column<int64_t>>(std::move(result));
            }
        }

        std::shared_ptr<Series> divide(const Series& other) const override {
            if constexpr (std::is_same_v<T, std::string>) {
                throw std::invalid_argument("Arithmetic is not supported on string columns");
            }    
            else{
                if (size() != other.size()) {
                    throw std::invalid_argument(
                        "Column arithmetic requires equal-length columns (no index alignment)"
                    );
                }
            
                bool result_is_double = (type() == DataType::DOUBLE || other.type() == DataType::DOUBLE);

                if (result_is_double) {
                    std::vector<double> result(column.size());

                    if (other.type() == DataType::DOUBLE) {
                        const auto& other_col = static_cast<const Column<double>&>(other).get_column();
                        for (size_t i = 0; i < column.size(); ++i) {
                            result[i] = static_cast<double>(column[i]) / other_col[i];
                        }
                    } 
                    else {
                        const auto& other_col = static_cast<const Column<int64_t>&>(other).get_column();
                        for (size_t i = 0; i < column.size(); ++i) {
                            result[i] = static_cast<double>(column[i]) / static_cast<double>(other_col[i]);
                        }
                    }
                    return std::make_shared<Column<double>>(std::move(result));
                }

                
                const auto& other_col = static_cast<const Column<int64_t>&>(other).get_column();
                std::vector<int64_t> result(column.size());
                for (size_t i = 0; i < column.size(); ++i) {
                    result[i] = column[i] / other_col[i];
                }
                return std::make_shared<Column<int64_t>>(std::move(result));
            }
        }

        std::shared_ptr<Series> multiply(const Series& other) const override {
            if constexpr (std::is_same_v<T, std::string>) {
                throw std::invalid_argument("Arithmetic is not supported on string columns");
            }            
            else{
                if (size() != other.size()) {
                    throw std::invalid_argument(
                        "Column arithmetic requires equal-length columns (no index alignment)"
                    );
                }
            
                bool result_is_double = (type() == DataType::DOUBLE || other.type() == DataType::DOUBLE);

                if (result_is_double) {
                    std::vector<double> result(column.size());

                    if (other.type() == DataType::DOUBLE) {
                        const auto& other_col = static_cast<const Column<double>&>(other).get_column();
                        for (size_t i = 0; i < column.size(); ++i) {
                            result[i] = static_cast<double>(column[i]) * other_col[i];
                        }
                    } else {
                        const auto& other_col = static_cast<const Column<int64_t>&>(other).get_column();
                        for (size_t i = 0; i < column.size(); ++i) {
                            result[i] = static_cast<double>(column[i]) * static_cast<double>(other_col[i]);
                        }
                    }
                    return std::make_shared<Column<double>>(std::move(result));
                }

                
                const auto& other_col = static_cast<const Column<int64_t>&>(other).get_column();
                std::vector<int64_t> result(column.size());
                for (size_t i = 0; i < column.size(); ++i) {
                    result[i] = column[i] * other_col[i];
                }
                return std::make_shared<Column<int64_t>>(std::move(result));
            }
        }

    std::vector<size_t> argsort(bool ascending = true) const override {
        std::vector<size_t> indices(column.size());
        
        std::iota(indices.begin(), indices.end(), 0);
        if (ascending) {
            std::sort(indices.begin(), indices.end(),[this](size_t a, size_t b) { 
                return column[a] < column[b]; 
            });
        } 
        else {
            std::sort(indices.begin(), indices.end(),[this](size_t a, size_t b) { 
                return column[a] > column[b]; 
            });
        }

        return indices;
    }
        
    std::shared_ptr<Series> reorder(const std::vector<size_t>& indices) const override {
        if (indices.size() != column.size()) {
            throw std::invalid_argument("Index array size must match column size");
        }

        std::vector<T> reordered_data;
        reordered_data.reserve(column.size());


        for (size_t idx : indices) {
            reordered_data.push_back(column[idx]);
        }

        return std::make_shared<Column<T>>(std::move(reordered_data));
    }
};