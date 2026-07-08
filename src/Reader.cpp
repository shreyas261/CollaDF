#include "../include/colladf/Reader.hpp"


std::vector<std::string> Reader::parse_line(const std::string& line, char delimiter){
    std::vector<std::string> result;
    std::stringstream ss(line);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        result.push_back(token);
    }
    return result;
}

std::vector<std::string_view> Reader::ssplit(std::string_view str, char delim){
    std::vector<std::string_view> tokens;
    size_t start = 0, end = 0;
    
    while ((end = str.find(delim, start)) != std::string_view::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + 1;
    }
    tokens.push_back(str.substr(start));
    return tokens;
}

DataType Reader::infer_type(std::string_view token) {
    if (token.empty()) return DataType::STRING;

    bool has_dot = false;
    bool is_number = true;

    for (size_t i = 0; i < token.size(); ++i) {
        char c = token[i];
        if (c == '-') {
            if (i != 0) { is_number = false; break; }
        } 
        else if (c == '.') {
            if (has_dot) { is_number = false; break; }
            has_dot = true;
        } 
        else if (!std::isdigit(static_cast<unsigned char>(c))) {
            is_number = false;
            break;
        }
    }

    if (is_number && (token == "-" || token == "." || token == "-.")) {
        is_number = false;
    }

    if (is_number) {
        return has_dot ? DataType::DOUBLE : DataType::INTEGER;
    }
    return DataType::STRING;
}


DataType Reader::promote_type(DataType current, DataType new_type) {
    if (current == DataType::STRING || new_type == DataType::STRING) return DataType::STRING;
    if (current == DataType::DOUBLE || new_type == DataType::DOUBLE) return DataType::DOUBLE;
    return DataType::INTEGER;
}


DataFrame Reader::read_csv(const std::string& filepath) {
    MmapFile mmap_file(filepath);
    std::string_view file_content = mmap_file.get_view();

    if (file_content.empty()) throw std::runtime_error("File is empty");

    size_t current_pos = 0;
    auto get_next_line = [&]() -> std::string_view {
        if (current_pos >= file_content.size()) return {}; 
        
        size_t newline_pos = file_content.find('\n', current_pos);
        if (newline_pos == std::string_view::npos) {
            auto line = file_content.substr(current_pos);
            current_pos = file_content.size();
            return line;
        }
        
        auto line = file_content.substr(current_pos, newline_pos - current_pos);
        current_pos = newline_pos + 1;
        return line;
    };

    std::string_view header_line = get_next_line();
    auto header_tokens = ssplit(header_line, ',');
    std::vector<std::string> headers;
    for (auto v : header_tokens) headers.emplace_back(v);
    
    size_t num_cols = headers.size();

    size_t data_start_pos = current_pos; 
    std::vector<DataType> col_types(num_cols, DataType::INTEGER); 

    int lookahead_count = 0;
    std::string_view peek_line;
    while (lookahead_count < 100 && !(peek_line = get_next_line()).empty()) {
        auto tokens = ssplit(peek_line, ',');
        for (size_t i = 0; i < std::min(num_cols, tokens.size()); ++i) {
            if (!tokens[i].empty()) {
                DataType t = infer_type(tokens[i]);
                col_types[i] = promote_type(col_types[i], t);
            }
        }
        lookahead_count++;
    }

    current_pos = data_start_pos;

    using ColumnVariant = std::variant<std::vector<int64_t>, std::vector<double>, std::vector<std::string>>;
    std::vector<ColumnVariant> temp_columns(num_cols);

    for (size_t i = 0; i < num_cols; ++i) {
        if (col_types[i] == DataType::INTEGER) temp_columns[i] = std::vector<int64_t>();
        else if (col_types[i] == DataType::DOUBLE) temp_columns[i] = std::vector<double>();
        else temp_columns[i] = std::vector<std::string>();
    }

    std::string_view line;
    while (!(line = get_next_line()).empty()) {
        auto tokens = ssplit(line, ',');
        
        for (size_t i = 0; i < num_cols; ++i) {
            std::string_view token = (i < tokens.size()) ? tokens[i] : "";

            if (col_types[i] == DataType::INTEGER) {
                int64_t val = 0;
                if (!token.empty()) {
                    std::from_chars(token.data(), token.data() + token.size(), val);
                }
                std::get<std::vector<int64_t>>(temp_columns[i]).push_back(val);
            } 
            else if (col_types[i] == DataType::DOUBLE) {
                double val = 0.0;
                if (!token.empty()) {
                    val = std::stod(std::string(token)); // stod needs null-terminated string
                }
                std::get<std::vector<double>>(temp_columns[i]).push_back(val);
            } 
            else {
                std::get<std::vector<std::string>>(temp_columns[i]).emplace_back(token);
            }
        }
    }

    DataFrame df;
    for (size_t i = 0; i < num_cols; ++i) {
        std::shared_ptr<Series> series_ptr;

        if (col_types[i] == DataType::INTEGER) {
            series_ptr = std::make_shared<Column<int64_t>>(
                std::move(std::get<std::vector<int64_t>>(temp_columns[i]))
            );
        } 
        else if (col_types[i] == DataType::DOUBLE) {
            series_ptr = std::make_shared<Column<double>>(
                std::move(std::get<std::vector<double>>(temp_columns[i]))
            );
        } 
        else {
            series_ptr = std::make_shared<Column<std::string>>(
                std::move(std::get<std::vector<std::string>>(temp_columns[i]))
            );
        }
        df.add_column(headers[i], series_ptr);
    }

    return df;
}

void Reader::to_csv(const DataFrame& df, const std::string& filepath) {
    std::ofstream out(filepath, std::ios::binary);
    if (!out.is_open()) throw std::runtime_error("Could not open file for writing: " + filepath);

    size_t num_rows = df.num_rows();
    size_t num_cols = df.num_columns();
    if (num_rows == 0 || num_cols == 0) return;

    const auto& headers = df.columns();


    std::string buffer;
    buffer.reserve(1024 * 1024); 

    for (size_t i = 0; i < num_cols; ++i) {
        buffer += headers[i];
        if (i < num_cols - 1) buffer += ",";
    }
    buffer += "\n";

  

    using RawDataPtr = std::variant<const std::vector<int64_t>*, const std::vector<double>*, const std::vector<std::string>*>;
    std::vector<RawDataPtr> raw_columns(num_cols);

    for (size_t i = 0; i < num_cols; ++i) {
        const auto& base_series = df.get_column(headers[i]);
        if (base_series->type() == DataType::INTEGER) {
            raw_columns[i] = &static_cast<const Column<int64_t>*>(base_series.get())->get_column();
        } 
        else if (base_series->type() == DataType::DOUBLE) {
            raw_columns[i] = &static_cast<const Column<double>*>(base_series.get())->get_column();
        } 
        else {
            raw_columns[i] = &static_cast<const Column<std::string>*>(base_series.get())->get_column();
        }
    }

    char num_buf[64]; 

    for (size_t r = 0; r < num_rows; ++r) {
        
        for (size_t c = 0; c < num_cols; ++c) {
            
            if (std::holds_alternative<const std::vector<int64_t>*>(raw_columns[c])) {
                int64_t val = (*std::get<const std::vector<int64_t>*>(raw_columns[c]))[r];
                auto [ptr, ec] = std::to_chars(num_buf, num_buf + sizeof(num_buf), val);
                buffer.append(num_buf, ptr - num_buf);
            } 
            else if (std::holds_alternative<const std::vector<double>*>(raw_columns[c])) {
                double val = (*std::get<const std::vector<double>*>(raw_columns[c]))[r];
                auto [ptr, ec] = std::to_chars(num_buf, num_buf + sizeof(num_buf), val, std::chars_format::general);
                buffer.append(num_buf, ptr - num_buf);
            } 
            else {
                buffer += (*std::get<const std::vector<std::string>*>(raw_columns[c]))[r];
            }

            if (c < num_cols - 1) buffer += ",";
        }
        buffer += "\n";
        if (buffer.size() >= 1024 * 1024) {
            out.write(buffer.data(), buffer.size());
            buffer.clear();
        }
    }

    if (!buffer.empty()) {
        out.write(buffer.data(), buffer.size());
    }

    out.close();
}