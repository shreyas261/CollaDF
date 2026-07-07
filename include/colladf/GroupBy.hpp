#pragma once
#include "DataFrame.hpp"
#include <limits>

struct VectorHash {
    size_t operator()(const std::vector<std::string>& vec) const {
        size_t seed = 0;
        for (const std::string& str : vec) {
            size_t current_hash = std::hash<std::string>{}(str);
            seed ^= current_hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

class GroupBy {
    private:
        const DataFrame& df_ref; 
        std::vector<std::string> names; 
        std::unordered_map<std::vector<std::string>, std::vector<size_t>, VectorHash> groups;
        void group();

    public:
        GroupBy(const DataFrame& df, const std::string& col) : df_ref(df), names({col}) { group(); }
        GroupBy(const DataFrame& df, const std::vector<std::string>& col) : df_ref(df), names(col) { group(); }

        //Aggregate Functions
        DataFrame mean(const std::string& target_col) const;
        DataFrame max(const std::string& target_col) const;
        DataFrame min(const std::string& target_col) const;
        DataFrame count(const std::string& target_col) const;
        DataFrame sum(const std::string& target_col) const;
};