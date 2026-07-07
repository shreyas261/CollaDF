#include <gtest/gtest.h>
#include <stdexcept>
#include "../include/colladf/DataFrame.hpp"
#include "../include/colladf/Reader.hpp"

class MathTest : public ::testing::Test {
protected:
    DataFrame df;
    const std::string filepath = "gtest_temp.csv";

    void SetUp() override {
        // 1. Create the file locally so this suite is completely independent
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

        // 2. Load the DataFrame
        df = Reader::read_csv(filepath);
    }

    void TearDown() override {
        // 3. Clean up the sandbox
        std::remove(filepath.c_str());
    }
};
// -----------------------------------------------------------------------------
// 1. Scalar Arithmetic on Integer Columns (int64_t)
// -----------------------------------------------------------------------------
TEST_F(MathTest, IntegerScalarAddition) {
    DataFrame result = df["salary"] + static_cast<int64_t>(5000);
    
    auto* typed_col = static_cast<Column<int64_t>*>(result.get_column("salary").get());
    const auto& vec = typed_col->get_column();
    
    EXPECT_EQ(vec[0], 77000); // Alice: 72000 + 5000
    EXPECT_EQ(vec[1], 63000); // Bob: 58000 + 5000
}

TEST_F(MathTest, IntegerScalarSubtraction) {
    DataFrame result = df["salary"] - static_cast<int64_t>(2000);
    
    auto* typed_col = static_cast<Column<int64_t>*>(result.get_column("salary").get());
    const auto& vec = typed_col->get_column();
    
    EXPECT_EQ(vec[0], 70000); // Alice: 72000 - 2000
}

TEST_F(MathTest, IntegerScalarMultiplication) {
    DataFrame result = df["salary"] * static_cast<int64_t>(2);
    
    auto* typed_col = static_cast<Column<int64_t>*>(result.get_column("salary").get());
    const auto& vec = typed_col->get_column();
    
    EXPECT_EQ(vec[2], 182000); // Charlie: 91000 * 2
}

TEST_F(MathTest, IntegerScalarDivision) {
    DataFrame result = df["salary"] / static_cast<int64_t>(1000);
    
    auto* typed_col = static_cast<Column<int64_t>*>(result.get_column("salary").get());
    const auto& vec = typed_col->get_column();
    
    EXPECT_EQ(vec[3], 64); // Diana: 64000 / 1000
}

// -----------------------------------------------------------------------------
// 2. Scalar Arithmetic on Double Columns (double)
// -----------------------------------------------------------------------------
TEST_F(MathTest, DoubleScalarArithmetic) {
    // Testing a chained operation: (rating * 2.0) + 1.0
    DataFrame doubled = df["rating"] * 2.0;
    DataFrame result = doubled + 1.0;
    
    auto* typed_col = static_cast<Column<double>*>(result.get_column("rating").get());
    const auto& vec = typed_col->get_column();
    
    // Alice rating is 4.5 -> (4.5 * 2.0) + 1.0 = 10.0
    EXPECT_DOUBLE_EQ(vec[0], 10.0);
    // Bob rating is 3.8 -> (3.8 * 2.0) + 1.0 = 8.6
    EXPECT_DOUBLE_EQ(vec[1], 8.6);
}

// -----------------------------------------------------------------------------
// 3. DataFrame-to-DataFrame Arithmetic (Column to Column)
// -----------------------------------------------------------------------------
TEST_F(MathTest, ColumnToColumnAddition) {
    // Let's add the salary column to itself
    DataFrame salary_col = df["salary"];
    DataFrame result = salary_col + salary_col;
    
    auto* typed_col = static_cast<Column<int64_t>*>(result.get_column("salary").get());
    const auto& vec = typed_col->get_column();
    
    EXPECT_EQ(vec[0], 144000); // Alice: 72000 + 72000
}

TEST_F(MathTest, ColumnToColumnMultiplicationDouble) {
    // Let's multiply the rating column by itself (rating squared)
    DataFrame rating_col = df["rating"];
    DataFrame result = rating_col * rating_col;
    
    auto* typed_col = static_cast<Column<double>*>(result.get_column("rating").get());
    const auto& vec = typed_col->get_column();
    
    // Alice: 4.5 * 4.5 = 20.25
    EXPECT_DOUBLE_EQ(vec[0], 20.25);
}

// -----------------------------------------------------------------------------
// 4. Exception Handling & Edge Cases
// -----------------------------------------------------------------------------
TEST_F(MathTest, InvalidStringArithmetic) {
    // Attempting arithmetic on string columns should throw an invalid_argument exception
    EXPECT_THROW(df["name"] + static_cast<int64_t>(5), std::invalid_argument);
    EXPECT_THROW(df["department"] - 2.0, std::invalid_argument);
    EXPECT_THROW(df["name"] * df["department"], std::invalid_argument);
}

TEST_F(MathTest, MultiColumnBroadcastException) {
    // Arithmetic operators in CollaDF are restricted to single-column DataFrames.
    // Selecting multiple columns and trying to add a scalar should throw a runtime_error.
    DataFrame multi_col = df.select({"salary", "rating"});
    EXPECT_THROW(multi_col + static_cast<int64_t>(10), std::runtime_error);
}