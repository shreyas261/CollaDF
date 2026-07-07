#include <iostream>
#include <chrono>
#include "../include/colladf/Reader.hpp"
#include "../include/colladf/DataFrame.hpp"
#include "../include/colladf/GroupBy.hpp"


void benchmark(const std::string& loc){
    std::cout << "\n---------------------------------------------" << std::endl;
    std::cout << loc << std::endl;
    std::cout << "---------------------------------------------" << std::endl;

    auto start_total = std::chrono::high_resolution_clock::now();

    // 1. Time the CSV Reader
    auto start_read = std::chrono::high_resolution_clock::now();
    DataFrame df = Reader::read_csv("loc");
    auto end_read = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double> read_time = end_read - start_read;
    std::cout << "Read CSV Time: " << read_time.count() << " seconds\n";

    // 2. Time a Filter Operation (Salary > 80000)
    auto start_filter = std::chrono::high_resolution_clock::now();
    DataFrame rich_df = df[df["salary"] > 80000];
    auto end_filter = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double> filter_time = end_filter - start_filter;
    std::cout << "Filter Time: " << filter_time.count() << " seconds\n";

    // 3. Time a GroupBy Operation
    auto start_groupby = std::chrono::high_resolution_clock::now();
    DataFrame grouped = df.groupby("department").mean("salary");
    auto end_groupby = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> groupby_time = end_groupby - start_groupby;
    std::cout << "GroupBy Mean Time: " << groupby_time.count() << " seconds\n";

    auto end_total = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> total_time = end_total - start_total;
    
    std::cout << "------------------------------------\n";
    std::cout << "Total CollaDF "<< loc <<" Time: " << total_time.count() << " seconds\n\n\n";

}

int main() {
    std::cout << "Starting CollaDF Benchmark..." << std::endl;
    auto start_total = std::chrono::high_resolution_clock::now();
    auto end_total = std::chrono::high_resolution_clock::now();
    benchmark("10M.csv");
    std::chrono::duration<double> total_time = end_total - start_total;
    std::cout << "Total CollaDF Time: " << total_time.count() << " seconds\n\n\n";
    return 0;
}