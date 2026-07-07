#include "../include/colladf/DataFrame.hpp"


DataFrame DataFrame::head(int n) const{
    size_t limit = std::min(n, (int)this->num_rows());
    DataFrame result;

    for(const std::string& name : column_names){
        std::shared_ptr<Series> base_series = this->get_column(name);
        DataType type = base_series -> type();

        if (type == DataType::INTEGER) {
            auto* typed_col = static_cast<Column<int64_t>*>(base_series.get());
            const std::vector<int64_t>& old_data = typed_col -> get_column();
            std::vector<int64_t> new_data(old_data.begin(), old_data.begin() + limit);
            result.add_column(name, std::make_shared<Column<int64_t>>(std::move(new_data)));
        } 

        else if (type == DataType::DOUBLE) {
            auto* typed_col = static_cast<Column<double>*>(base_series.get());
            const std::vector<double>& old_data = typed_col->get_column();
            
            std::vector<double> new_data(old_data.begin(), old_data.begin() + limit);
            
            result.add_column(name, std::make_shared<Column<double>>(std::move(new_data)));
        } 

        else if (type == DataType::STRING) {
            auto* typed_col = static_cast<Column<std::string>*>(base_series.get());
            const std::vector<std::string>& old_data = typed_col->get_column();
            
            std::vector<std::string> new_data(old_data.begin(), old_data.begin() + limit);
            
            result.add_column(name, std::make_shared<Column<std::string>>(std::move(new_data)));
        }
    
    }

    return result;
}

DataFrame DataFrame::tail(int n) const{
    size_t limit = std::min(n, (int)this->num_rows());
    DataFrame result;

    for(const std::string& name : column_names){
        std::shared_ptr<Series> base_series = this->get_column(name);
        DataType type = base_series -> type();

        if (type == DataType::INTEGER) {
            auto* typed_col = static_cast<Column<int64_t>*>(base_series.get());
            const std::vector<int64_t>& old_data = typed_col -> get_column();
            std::vector<int64_t> new_data(old_data.end() - limit, old_data.end());
            result.add_column(name, std::make_shared<Column<int64_t>>(std::move(new_data)));
        } 

        else if (type == DataType::DOUBLE) {
            auto* typed_col = static_cast<Column<double>*>(base_series.get());
            const std::vector<double>& old_data = typed_col->get_column();
            
            std::vector<double> new_data(old_data.end() - limit, old_data.end());
            
            result.add_column(name, std::make_shared<Column<double>>(std::move(new_data)));
        } 

        else if (type == DataType::STRING) {
            auto* typed_col = static_cast<Column<std::string>*>(base_series.get());
            const std::vector<std::string>& old_data = typed_col->get_column();
            
            std::vector<std::string> new_data(old_data.end() - limit, old_data.end());
            
            result.add_column(name, std::make_shared<Column<std::string>>(std::move(new_data)));
        }
    
    }

    return result;
}


void DataFrame::describe() const {
    for (const std::string& col_name : column_names) {
        std::shared_ptr<Series> base_series = dataframe.at(col_name);

        if (base_series->type() == DataType::STRING) continue; 
        
        ColumnStats stats;
        
        if (base_series->type() == DataType::INTEGER) {
            auto* int_col = static_cast<Column<int64_t>*>(base_series.get());
            stats = int_col -> compute_stats();
        } 
        else if (base_series->type() == DataType::DOUBLE) {
            auto* double_col = static_cast<Column<double>*>(base_series.get());
            stats = double_col->compute_stats();
        }
        
        std::cout << "--- " << col_name << " ---\n";
        std::cout << "Count: " << stats.count << "\n";
        std::cout << "Mean:  " << stats.mean << "\n";
        std::cout << "Min:   " << stats.min << "\n";
        std::cout << "Max:   " << stats.max << "\n\n";
    }
}

DataFrame DataFrame::select(const std::vector<std::string>& columns) const {
    DataFrame result;
    for (const std::string& name : columns) {
        std::shared_ptr<Series> ptr = this -> get_column(name);
        result.add_column(name, ptr);
    }
    return result;
}

DataFrame DataFrame::drop(const std::vector<std::string>& columns) const {
    DataFrame result;
    std::unordered_set<std::string> drop_set(columns.begin(), columns.end());

    for (const std::string& name : column_names) {
        if (drop_set.find(name) == drop_set.end()) {
            result.add_column(name, this->get_column(name));
        }
    }
    return result;
}

DataFrame DataFrame::filter(const std::vector<bool>& mask) const {
    if (mask.size() != num_rows()) {
        throw std::invalid_argument("Mask size must match DataFrame row count.");
    }

    DataFrame result;
    
    for (const std::string& name : column_names) {
        std::shared_ptr<Series> base_series = this->get_column(name);
        std::shared_ptr<Series> filtered_series = base_series -> apply_mask(mask);
        result.add_column(name, filtered_series);
    }
    
    return result;
}

DataFrame DataFrame::operator[](const std::vector<bool>& mask){
    return filter(mask);
}

DataFrame DataFrame::operator[](const std::string& name){
    DataFrame result;
    result.add_column(name,this-> get_column(name));
    return result;
}

std::vector<bool> DataFrame::operator>(const ScalarValue& v) const {
    if (column_names.size() != 1)
        throw std::runtime_error("Comparison operators require a single-column DataFrame (use df[\"col\"] first)");
    return get_column(column_names[0])->compare_gt(v);
}

std::vector<bool> DataFrame::operator<(const ScalarValue& v) const {
    if (column_names.size() != 1)
        throw std::runtime_error("Comparison operators require a single-column DataFrame");
    return get_column(column_names[0])->compare_lt(v);
}

std::vector<bool> DataFrame::operator==(const ScalarValue& v) const {
    if (column_names.size() != 1)
        throw std::runtime_error("Comparison operators require a single-column DataFrame");
    return get_column(column_names[0])->compare_eq(v);
}

std::vector<bool> DataFrame::operator>=(const ScalarValue& v) const {
    if (column_names.size() != 1)
        throw std::runtime_error("Comparison operators require a single-column DataFrame");
    return get_column(column_names[0]) -> compare_ge(v);
}

std::vector<bool> DataFrame::operator<=(const ScalarValue& v) const {
    if (column_names.size() != 1)
        throw std::runtime_error("Comparison operators require a single-column DataFrame");
    return get_column(column_names[0])->compare_le(v);
}

std::vector<bool> DataFrame::operator!=(const ScalarValue& v) const {
    if (column_names.size() != 1)
        throw std::runtime_error("Comparison operators require a single-column DataFrame");
    return get_column(column_names[0])->compare_ne(v);
}

DataFrame DataFrame::operator+(const ScalarValue& v) const {
    if (column_names.size() != 1) throw std::runtime_error("Cannot Broadcast into multiple rows");
    DataFrame result;
    result.add_column(column_names[0], get_column(column_names[0])->add(v));
    return result;
}

DataFrame DataFrame::operator-(const ScalarValue& v) const {
    if (column_names.size() != 1) throw std::runtime_error("Cannot Broadcast into multiple rows");
    DataFrame result;
    result.add_column(column_names[0], get_column(column_names[0])->subtract(v));
    return result;
}

DataFrame DataFrame::operator*(const ScalarValue& v) const {
    if (column_names.size() != 1) throw std::runtime_error("Cannot Broadcast into multiple rows");
    DataFrame result;
    result.add_column(column_names[0], get_column(column_names[0])->multiply(v));
    return result;
}

DataFrame DataFrame::operator/(const ScalarValue& v) const {
    if (column_names.size() != 1) throw std::runtime_error("Cannot Broadcast into multiple rows");
    DataFrame result;
    result.add_column(column_names[0], get_column(column_names[0])->divide(v));
    return result;
}



DataFrame DataFrame::operator+(const DataFrame& other) const {
    if (column_names.size() != 1 || other.column_names.size() != 1)
        throw std::runtime_error("Arithmetic requires single-column DataFrames");

    auto this_series  = get_column(column_names[0]);
    auto other_series = other.get_column(other.column_names[0]);

    DataFrame result;
    result.add_column(column_names[0], this_series->add(*other_series));
    return result;
}

DataFrame DataFrame::operator-(const DataFrame& other) const {
    if (column_names.size() != 1 || other.column_names.size() != 1)
        throw std::runtime_error("Arithmetic requires single-column DataFrames");

    auto this_series  = get_column(column_names[0]);
    auto other_series = other.get_column(other.column_names[0]);

    DataFrame result;
    result.add_column(column_names[0], this_series->subtract(*other_series));
    return result;
}

DataFrame DataFrame::operator*(const DataFrame& other) const {
    if (column_names.size() != 1 || other.column_names.size() != 1)
        throw std::runtime_error("Arithmetic requires single-column DataFrames");

    auto this_series  = get_column(column_names[0]);
    auto other_series = other.get_column(other.column_names[0]);

    DataFrame result;
    result.add_column(column_names[0], this_series->multiply(*other_series));
    return result;
}

DataFrame DataFrame::operator/(const DataFrame& other) const {
    if (column_names.size() != 1 || other.column_names.size() != 1)
        throw std::runtime_error("Arithmetic requires single-column DataFrames");

    auto this_series  = get_column(column_names[0]);
    auto other_series = other.get_column(other.column_names[0]);

    DataFrame result;
    result.add_column(column_names[0], this_series->divide(*other_series));
    return result;
}

std::vector<bool> operator&(const std::vector<bool>& a, const std::vector<bool>& b){
    if (a.size() != b.size()) throw std::invalid_argument("Mask size mismatch");
    std::vector<bool> result(a.size());
    for (size_t i = 0; i < a.size(); ++i) result[i] = a[i] && b[i];
    return result;
}

std::vector<bool> operator|(const std::vector<bool>& a, const std::vector<bool>& b){
    if (a.size() != b.size()) throw std::invalid_argument("Mask size mismatch");
    std::vector<bool> result(a.size());
    for (size_t i = 0; i < a.size(); ++i) result[i] = a[i] || b[i];
    return result;
}

std::vector<bool> operator!(const std::vector<bool>& a){
    std::vector<bool> result(a.size());
    for (size_t i = 0; i < a.size(); ++i) result[i] = !a[i];
    return result;
}



