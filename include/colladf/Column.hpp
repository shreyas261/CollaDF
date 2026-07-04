#pragma once
#include <bits/stdc++.h>

class Series{
    public:
        virtual ~Series() = default;
        virtual size_t size() const = 0; 
};

template <typename T> 
class Column{
    private:
        std::vector<T> column;

    public:
        
        Column() {}

        Column(std::vector<T>& data){
            column = data;
        }

        size_t size() const override { return column.size() };
        std::vector<T>& get_column() {return column; }
};