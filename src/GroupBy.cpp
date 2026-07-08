#include "../include/colladf/GroupBy.hpp"
#include <stdexcept>

void GroupBy::group() {
    if (names.empty() || df_ref.num_rows() == 0) return;
    
    size_t row_count = df_ref.num_rows();
    size_t num_group_cols = names.size();

    std::vector<const std::vector<std::string>*> raw_cols;
    raw_cols.reserve(num_group_cols);
    
    for (const std::string& col_name : names) {
        auto base_series = df_ref.get_column(col_name);
        auto* typed_col = static_cast<Column<std::string>*>(base_series.get());
        raw_cols.emplace_back(&(typed_col->get_column()));
    }
    
    std::vector<std::string_view> current_key(num_group_cols);
    for (size_t r = 0; r < row_count; ++r) {
        for (size_t c = 0; c < num_group_cols; ++c) {
            current_key[c] = (*raw_cols[c])[r]; 
        }
        groups[current_key].emplace_back(r);
    }
}
DataFrame GroupBy::mean(const std::string& target_col_name) const {
    auto base_series = df_ref.get_column(target_col_name);
    DataType target_type = base_series->type();

    if (target_type == DataType::STRING) {
        throw std::invalid_argument("Cannot calculate mean on a STRING column.");
    }

    std::vector<std::vector<std::string>> output_key_cols(names.size());
    std::vector<double> output_means;

    const std::vector<int64_t>* int_data = nullptr;
    const std::vector<double>* double_data = nullptr;
    
    if (target_type == DataType::INTEGER) {
        int_data = &(static_cast<Column<int64_t>*>(base_series.get())->get_column());
    } else {
        double_data = &(static_cast<Column<double>*>(base_series.get())->get_column());
    }

    for (const auto& [key_vec, row_indices] : groups) {
        double sum = 0.0;
        
        if (target_type == DataType::INTEGER) {
            for (size_t idx : row_indices) sum += static_cast<double>((*int_data)[idx]);
        } else {
            for (size_t idx : row_indices) sum += (*double_data)[idx];
        }

        for (size_t c = 0; c < names.size(); ++c) {
            output_key_cols[c].emplace_back(key_vec[c]);
        }
        output_means.emplace_back(sum / row_indices.size());
    }

    DataFrame result;
    for (size_t c = 0; c < names.size(); ++c) {
        result.add_column(names[c], std::make_shared<Column<std::string>>(std::move(output_key_cols[c])));
    }
    result.add_column(target_col_name + "_mean", std::make_shared<Column<double>>(std::move(output_means)));

    return result;
}

DataFrame GroupBy::sum(const std::string& target_col_name) const {
    auto base_series = df_ref.get_column(target_col_name);
    DataType target_type = base_series->type();

    if (target_type == DataType::STRING) {
        throw std::invalid_argument("Cannot calculate sum on a STRING column.");
    }

    std::vector<std::vector<std::string>> output_key_cols(names.size());
    std::vector<double> output_sums; 

    const std::vector<int64_t>* int_data = nullptr;
    const std::vector<double>* double_data = nullptr;
    
    if (target_type == DataType::INTEGER) {
        int_data = &(static_cast<Column<int64_t>*>(base_series.get())->get_column());
    } else {
        double_data = &(static_cast<Column<double>*>(base_series.get())->get_column());
    }

    for (const auto& [key_vec, row_indices] : groups) {
        double sum = 0.0;
        
        if (target_type == DataType::INTEGER) {
            for (size_t idx : row_indices) sum += static_cast<double>((*int_data)[idx]);
        } else {
            for (size_t idx : row_indices) sum += (*double_data)[idx];
        }

        for (size_t c = 0; c < names.size(); ++c) {
            output_key_cols[c].emplace_back(key_vec[c]);
        }
        output_sums.emplace_back(sum);
    }

    DataFrame result;
    for (size_t c = 0; c < names.size(); ++c) {
        result.add_column(names[c], std::make_shared<Column<std::string>>(std::move(output_key_cols[c])));
    }
    result.add_column(target_col_name + "_sum", std::make_shared<Column<double>>(std::move(output_sums)));

    return result;
}

DataFrame GroupBy::min(const std::string& target_col_name) const {
    auto base_series = df_ref.get_column(target_col_name);
    DataType target_type = base_series->type();

    if (target_type == DataType::STRING) {
        throw std::invalid_argument("Cannot calculate min on a STRING column.");
    }

    std::vector<std::vector<std::string>> output_key_cols(names.size());
    std::vector<double> output_mins;

    const std::vector<int64_t>* int_data = nullptr;
    const std::vector<double>* double_data = nullptr;
    
    if (target_type == DataType::INTEGER) {
        int_data = &(static_cast<Column<int64_t>*>(base_series.get())->get_column());
    } else {
        double_data = &(static_cast<Column<double>*>(base_series.get())->get_column());
    }

    for (const auto& [key_vec, row_indices] : groups) {
        double min_val = std::numeric_limits<double>::max();
        
        if (target_type == DataType::INTEGER) {
            for (size_t idx : row_indices) min_val = std::min(min_val, static_cast<double>((*int_data)[idx]));
        } else {
            for (size_t idx : row_indices) min_val = std::min(min_val, (*double_data)[idx]);
        }

        for (size_t c = 0; c < names.size(); ++c) {
            output_key_cols[c].emplace_back(key_vec[c]);
        }
        output_mins.emplace_back(min_val);
    }

    DataFrame result;
    for (size_t c = 0; c < names.size(); ++c) {
        result.add_column(names[c], std::make_shared<Column<std::string>>(std::move(output_key_cols[c])));
    }
    result.add_column(target_col_name + "_min", std::make_shared<Column<double>>(std::move(output_mins)));

    return result;
}

DataFrame GroupBy::max(const std::string& target_col_name) const {
    auto base_series = df_ref.get_column(target_col_name);
    DataType target_type = base_series->type();

    if (target_type == DataType::STRING) {
        throw std::invalid_argument("Cannot calculate max on a STRING column.");
    }

    std::vector<std::vector<std::string>> output_key_cols(names.size());
    std::vector<double> output_maxes;

    const std::vector<int64_t>* int_data = nullptr;
    const std::vector<double>* double_data = nullptr;
    
    if (target_type == DataType::INTEGER) {
        int_data = &(static_cast<Column<int64_t>*>(base_series.get())->get_column());
    } else {
        double_data = &(static_cast<Column<double>*>(base_series.get())->get_column());
    }

    for (const auto& [key_vec, row_indices] : groups) {
        double max_val = std::numeric_limits<double>::lowest();
        
        if (target_type == DataType::INTEGER) {
            for (size_t idx : row_indices) max_val = std::max(max_val, static_cast<double>((*int_data)[idx]));
        } else {
            for (size_t idx : row_indices) max_val = std::max(max_val, (*double_data)[idx]);
        }

        for (size_t c = 0; c < names.size(); ++c) {
            output_key_cols[c].emplace_back(key_vec[c]);
        }
        output_maxes.emplace_back(max_val);
    }

    DataFrame result;
    for (size_t c = 0; c < names.size(); ++c) {
        result.add_column(names[c], std::make_shared<Column<std::string>>(std::move(output_key_cols[c])));
    }
    result.add_column(target_col_name + "_max", std::make_shared<Column<double>>(std::move(output_maxes)));

    return result;
}

DataFrame GroupBy::count(const std::string& target_col_name) const {
    // We just verify the column exists, we don't actually need its data
    df_ref.get_column(target_col_name); 

    std::vector<std::vector<std::string>> output_key_cols(names.size());
    std::vector<int64_t> output_counts; // Counts are strictly integers

    for (const auto& [key_vec, row_indices] : groups) {
        for (size_t c = 0; c < names.size(); ++c) {
            output_key_cols[c].emplace_back(key_vec[c]);
        }
        output_counts.emplace_back(static_cast<int64_t>(row_indices.size()));
    }

    DataFrame result;
    for (size_t c = 0; c < names.size(); ++c) {
        result.add_column(names[c], std::make_shared<Column<std::string>>(std::move(output_key_cols[c])));
    }
    result.add_column(target_col_name + "_count", std::make_shared<Column<int64_t>>(std::move(output_counts)));

    return result;
}