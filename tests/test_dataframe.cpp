#include <gtest/gtest.h>
#include <fstream>
#include <algorithm>
#include "../include/colladf/DataFrame.hpp"
#include "../include/colladf/Reader.hpp"
#include "../include/colladf/GroupBy.hpp"

// -----------------------------------------------------------------------------
// Test Fixture: Handles CSV Setup and Teardown
// -----------------------------------------------------------------------------
class CollaDFTest : public ::testing::Test {
protected:
    const std::string filepath = "gtest_temp.csv";

    void SetUp() override {
        std::ofstream out(filepath);
        out << "id,name,department,salary,rating\n";
        out << "1,Alice,Engineering,72000,4.5\n";
        out << "2,Bob,Sales,58000,3.8\n";
        out << "3,Charlie,Engineering,91000,4.9\n";
        out << "4,Diana,Marketing,64000,4.1\n";
        out << "5,Eve,Sales,49000,3.2\n";
        out << "6,Frank,Engineering,105000,4.7\n";
        out << "7,Grace,Marketing,71000,4.0\n";
        out << "8,Heidi,Engineering,88000,4.6\n";
        out.close();
    }

    void TearDown() override {
        std::remove(filepath.c_str());
    }
};

// -----------------------------------------------------------------------------
// Core IO and Shape Tests
// -----------------------------------------------------------------------------
TEST_F(CollaDFTest, ReadCSVAndShape) {
    DataFrame df = Reader::read_csv(filepath);
    
    auto [rows, cols] = df.shape();
    EXPECT_EQ(rows, 8);
    EXPECT_EQ(cols, 5);
    
    const auto& col_names = df.columns();
    EXPECT_EQ(col_names[0], "id");
    EXPECT_EQ(col_names[2], "department");
}

// -----------------------------------------------------------------------------
// Subsetting Tests
// -----------------------------------------------------------------------------
TEST_F(CollaDFTest, HeadAndTail) {
    DataFrame df = Reader::read_csv(filepath);
    
    DataFrame h = df.head(3);
    EXPECT_EQ(h.num_rows(), 3);
    
    DataFrame t = df.tail(2);
    EXPECT_EQ(t.num_rows(), 2);
    
    // Boundary check: requesting more rows than exist
    DataFrame h_large = df.head(100);
    EXPECT_EQ(h_large.num_rows(), 8);
}

TEST_F(CollaDFTest, SelectAndDrop) {
    DataFrame df = Reader::read_csv(filepath);
    
    DataFrame selected = df.select({"name", "rating"});
    EXPECT_EQ(selected.num_columns(), 2);
    
    DataFrame dropped = df.drop({"salary", "department"});
    EXPECT_EQ(dropped.num_columns(), 3);
}

// -----------------------------------------------------------------------------
// Filtering and Masking Tests
// -----------------------------------------------------------------------------
TEST_F(CollaDFTest, BooleanFiltering) {
    DataFrame df = Reader::read_csv(filepath);
    
    // Engineering has 4 rows
    std::vector<bool> eng_mask = df["department"] == "Engineering";
    DataFrame eng_df = df[eng_mask];
    EXPECT_EQ(eng_df.num_rows(), 4);

    // High Earners (>80k) has 3 rows (Charlie, Frank, Heidi)
    std::vector<bool> salary_mask = df["salary"] > 80000;
    DataFrame rich_df = df[salary_mask];
    EXPECT_EQ(rich_df.num_rows(), 3);

    // Combinator Test: Engineering AND > 80k
    std::vector<bool> combined_mask = eng_mask & salary_mask;
    DataFrame eng_rich_df = df[combined_mask];
    EXPECT_EQ(eng_rich_df.num_rows(), 3);
}

// -----------------------------------------------------------------------------
// Arithmetic Tests
// -----------------------------------------------------------------------------
TEST_F(CollaDFTest, ScalarArithmetic) {
    DataFrame df = Reader::read_csv(filepath);
    
    DataFrame boosted = df["salary"] + 5000;
    
    auto base_series = boosted.get_column("salary");
    auto* typed_col = static_cast<Column<int64_t>*>(base_series.get());
    const auto& vec = typed_col->get_column();
    
    // Alice's new salary: 72000 + 5000
    EXPECT_EQ(vec[0], 77000); 
}

// -----------------------------------------------------------------------------
// GroupBy & Aggregation Tests
// -----------------------------------------------------------------------------
TEST_F(CollaDFTest, GroupBySingleColumnMean) {
    DataFrame df = Reader::read_csv(filepath);
    DataFrame result = df.groupby("department").mean("salary");
    
    EXPECT_EQ(result.num_columns(), 2); // department, salary_mean
    EXPECT_EQ(result.num_rows(), 3);    // Engineering, Sales, Marketing

    auto dept_col = static_cast<Column<std::string>*>(result.get_column("department").get())->get_column();
    auto mean_col = static_cast<Column<double>*>(result.get_column("salary_mean").get())->get_column();

    // Verify Engineering Mean: (72k + 91k + 105k + 88k) / 4 = 89000
    auto eng_it = std::find(dept_col.begin(), dept_col.end(), "Engineering");
    ASSERT_NE(eng_it, dept_col.end());
    size_t eng_idx = std::distance(dept_col.begin(), eng_it);
    EXPECT_DOUBLE_EQ(mean_col[eng_idx], 89000.0);

    // Verify Sales Mean: (58k + 49k) / 2 = 53500
    auto sales_it = std::find(dept_col.begin(), dept_col.end(), "Sales");
    ASSERT_NE(sales_it, dept_col.end());
    size_t sales_idx = std::distance(dept_col.begin(), sales_it);
    EXPECT_DOUBLE_EQ(mean_col[sales_idx], 53500.0);
}

TEST_F(CollaDFTest, GroupByMultiColumnMax) {
    DataFrame df = Reader::read_csv(filepath);
    
    // Grouping by department and name. Since names are unique here, 
    // we should get 8 groups back, each matching the original rows.
    DataFrame result = df.groupby(std::vector<std::string>{"department", "name"}).max("rating");
    
    EXPECT_EQ(result.num_columns(), 3); // department, name, rating_max
    EXPECT_EQ(result.num_rows(), 8);
}

TEST_F(CollaDFTest, GroupByCount) {
    DataFrame df = Reader::read_csv(filepath);
    DataFrame result = df.groupby("department").count("id");
    
    auto dept_col = static_cast<Column<std::string>*>(result.get_column("department").get())->get_column();
    auto count_col = static_cast<Column<int64_t>*>(result.get_column("id_count").get())->get_column();

    auto eng_it = std::find(dept_col.begin(), dept_col.end(), "Engineering");
    ASSERT_NE(eng_it, dept_col.end());
    size_t eng_idx = std::distance(dept_col.begin(), eng_it);
    
    EXPECT_EQ(count_col[eng_idx], 4); // 4 Engineers
}

// -----------------------------------------------------------------------------
// Exception Handling Tests
// -----------------------------------------------------------------------------
TEST_F(CollaDFTest, ErrorHandling) {
    DataFrame df = Reader::read_csv(filepath);
    
    // Attempting arithmetic on string columns should throw
    EXPECT_THROW(df["name"] + 10, std::invalid_argument);
    
    // Attempting mean on a string column should throw
    EXPECT_THROW(df.groupby("department").mean("name"), std::invalid_argument);
    
    // Fetching a non-existent column should throw
    EXPECT_THROW(df.get_column("does_not_exist"), std::invalid_argument);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}