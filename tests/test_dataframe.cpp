// test_dataframe.cpp
// End-to-end smoke test for CollaDF: generates temp.csv, then exercises
// every DataFrame method currently implemented (read_csv -> head/tail ->
// describe -> select/drop -> filter). Run with a "-v" arg for verbose
// column dumps.

#include "../include/colladf/DataFrame.hpp"
#include "../include/colladf/Reader.hpp"
#include <fstream>
#include <iostream>
#include <cassert>
#include <algorithm>

// ---------------------------------------------------------------------
// Small helper: DataFrame has no generic "print everything" method yet
// (only describe(), which skips string columns and only shows stats).
// This walks every column via the same type()-dispatch pattern the
// library itself uses in head()/tail(), so you can eyeball the actual
// row data.
// ---------------------------------------------------------------------
void print_dataframe(const DataFrame& df, const std::string& label) {
    std::cout << "\n=== " << label << "  (shape: "
              << df.shape().first << " rows x " << df.shape().second << " cols) ===\n";

    const auto& cols = df.columns();
    for (const auto& name : cols) std::cout << name << "\t";
    std::cout << "\n";

    size_t rows = df.num_rows();
    for (size_t r = 0; r < rows; ++r) {
        for (const auto& name : cols) {
            auto series = df.get_column(name);
            switch (series->type()) {
                case DataType::INTEGER: {
                    auto* c = static_cast<Column<int64_t>*>(series.get());
                    std::cout << c->get_column()[r] << "\t";
                    break;
                }
                case DataType::DOUBLE: {
                    auto* c = static_cast<Column<double>*>(series.get());
                    std::cout << c->get_column()[r] << "\t";
                    break;
                }
                case DataType::STRING: {
                    auto* c = static_cast<Column<std::string>*>(series.get());
                    std::cout << c->get_column()[r] << "\t";
                    break;
                }
            }
        }
        std::cout << "\n";
    }
}

// ---------------------------------------------------------------------
// Step 0: write a small temp.csv with a mix of int / double / string
// columns so type inference and every downstream method get exercised.
// ---------------------------------------------------------------------
void write_temp_csv(const std::string& path) {
    std::ofstream out(path);
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

int main(int argc, char** argv) {
    bool verbose = (argc > 1 && std::string(argv[1]) == "-v");
    const std::string path = "temp.csv";

    // ---- Reader::read_csv ----
    write_temp_csv(path);
    DataFrame df = Reader::read_csv(path);
    std::cout << "[OK] read_csv loaded " << df.shape().first
              << " rows, " << df.shape().second << " cols\n";
    if (verbose) print_dataframe(df, "Full DataFrame");

    // ---- shape / columns ----
    auto [rows, cols] = df.shape();
    assert(rows == 8 && cols == 5);
    std::cout << "[OK] shape() = (" << rows << ", " << cols << ")\n";

    std::cout << "[OK] columns(): ";
    for (const auto& c : df.columns()) std::cout << c << " ";
    std::cout << "\n";

    // ---- head / tail ----
    DataFrame h = df.head(3);
    assert(h.num_rows() == 3);
    if (verbose) print_dataframe(h, "head(3)");
    std::cout << "[OK] head(3) -> " << h.num_rows() << " rows\n";

    DataFrame t = df.tail(2);
    assert(t.num_rows() == 2);
    if (verbose) print_dataframe(t, "tail(2)");
    std::cout << "[OK] tail(2) -> " << t.num_rows() << " rows\n";

    // ---- describe ----
    std::cout << "\n[describe()] (numeric columns only)\n";
    df.describe();

    // ---- select ----
    DataFrame sel = df.select({"name", "salary"});
    assert(sel.num_columns() == 2);
    if (verbose) print_dataframe(sel, "select({name, salary})");
    std::cout << "[OK] select({name, salary}) -> " << sel.num_columns() << " cols\n";

    // ---- drop ----
    DataFrame dropped = df.drop({"rating"});
    assert(dropped.num_columns() == 4);
    if (verbose) print_dataframe(dropped, "drop({rating})");
    std::cout << "[OK] drop({rating}) -> " << dropped.num_columns() << " cols\n";

    // ---- filter / operator[](vector<bool>) ----
    // ColumnRef / df.col(name) > value isn't wired up yet, so build the
    // mask manually the same way filter() expects: one bool per row.
    // (Column<T>::operator> exists but compares "other > data", i.e. it's
    // reversed from what the name suggests -- worth fixing before relying
    // on it. Building the mask by hand here sidesteps that for now.)
    //
    // IMPORTANT: always check series->type() before static_cast-ing to a
    // Column<T>*. "salary" here has no decimal points in the CSV, so the
    // reader infers INTEGER, not DOUBLE -- casting to the wrong Column<T>
    // silently reads garbage memory instead of failing loudly.
    auto salary_series = df.get_column("salary");
    std::vector<bool> high_earners(salary_series->size());

    if (salary_series->type() == DataType::DOUBLE) {
        auto* col = static_cast<Column<double>*>(salary_series.get());
        const auto& vals = col->get_column();
        for (size_t i = 0; i < vals.size(); ++i) high_earners[i] = vals[i] > 70000.0;
    } else if (salary_series->type() == DataType::INTEGER) {
        auto* col = static_cast<Column<int64_t>*>(salary_series.get());
        const auto& vals = col->get_column();
        for (size_t i = 0; i < vals.size(); ++i) high_earners[i] = vals[i] > 70000;
    } else {
        throw std::runtime_error("salary column is not numeric");
    }

    DataFrame filtered = df.filter(high_earners);
    if (verbose) print_dataframe(filtered, "filter(salary > 70000)");
    std::cout << "[OK] filter(salary > 70000) -> " << filtered.num_rows() << " rows\n";

    DataFrame filtered2 = df[high_earners]; // operator[](vector<bool>)
    assert(filtered2.num_rows() == filtered.num_rows());
    std::cout << "[OK] operator[](mask) matches filter() -> "
              << filtered2.num_rows() << " rows\n";

    // ---- operator[](string) : single-column DataFrame (via implicit
    // ColumnRef -> DataFrame conversion) ----
    DataFrame name_col = df["name"];
    assert(name_col.num_columns() == 1);
    if (verbose) print_dataframe(name_col, "df[\"name\"]");
    std::cout << "[OK] df[\"name\"] -> " << name_col.num_columns() << " col\n";

    // ---- pandas-style comparison: df["col"] > value, bare literals ----
    std::vector<bool> mask_pandas_style = df["salary"] > 70000;
    assert(mask_pandas_style == high_earners);
    std::cout << "[OK] df[\"salary\"] > 70000 matches manual mask ("
              << std::count(mask_pandas_style.begin(), mask_pandas_style.end(), true)
              << " rows)\n";

    DataFrame filtered_pandas_style = df[ df["salary"] > 70000 ];
    assert(filtered_pandas_style.num_rows() == filtered.num_rows());
    if (verbose) print_dataframe(filtered_pandas_style, "df[df[\"salary\"] > 70000]");
    std::cout << "[OK] df[df[\"salary\"] > 70000] -> "
              << filtered_pandas_style.num_rows() << " rows\n";

    // ---- string-column comparison, bare string literal ----
    std::vector<bool> is_engineering = df["department"] == "Engineering";
    DataFrame eng = df[is_engineering];
    if (verbose) print_dataframe(eng, "department == Engineering");
    std::cout << "[OK] department == Engineering -> " << eng.num_rows() << " rows\n";

    // ---- boolean mask combinators: & | ! ----
    std::vector<bool> combined_and = (df["salary"] > 70000) & is_engineering;
    DataFrame eng_high_earners = df[combined_and];
    if (verbose) print_dataframe(eng_high_earners, "salary>70000 & department==Engineering");
    std::cout << "[OK] (salary > 70000) & (dept == Engineering) -> "
              << eng_high_earners.num_rows() << " rows\n";

    std::vector<bool> combined_or = (df["salary"] > 100000) | is_engineering;
    DataFrame eng_or_top_earners = df[combined_or];
    if (verbose) print_dataframe(eng_or_top_earners, "salary>100000 | department==Engineering");
    std::cout << "[OK] (salary > 100000) | (dept == Engineering) -> "
              << eng_or_top_earners.num_rows() << " rows\n";

    std::vector<bool> negated = !is_engineering;
    DataFrame not_eng = df[negated];
    if (verbose) print_dataframe(not_eng, "!(department == Engineering)");
    std::cout << "[OK] !(department == Engineering) -> " << not_eng.num_rows() << " rows\n";

    // ---- double comparison, bare literal ----
    std::vector<bool> high_rating = df["rating"] > 4.5;
    DataFrame top_rated = df[high_rating];
    if (verbose) print_dataframe(top_rated, "rating > 4.5");
    std::cout << "[OK] rating > 4.5 -> " << top_rated.num_rows() << " rows\n";

    std::cout << "\nAll checks passed.\n";
    return 0;
}