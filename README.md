# CollaDF 🚀 

**CollaDF (Columnar DataFrame)** is a lightweight, high-performance data manipulation and analysis toolkit written in Modern C++17. 

Built from scratch with a focus on CPU cache-friendly **columnar memory layout**, it is designed to perform blazing-fast vectorized operations on numerical datasets. It bypasses the overhead of generic tools by focusing on core Machine Learning and Data Engineering workloads. Includes **Python bindings** via PyBind11 to seamlessly integrate into standard ML pipelines.

## ✨ Features
*   **Columnar Storage:** Data is stored in contiguous memory blocks (`std::vector`), maximizing CPU cache hits and enabling SIMD auto-vectorization.
*   **Fast I/O:** Custom zero-copy CSV parser using memory buffering and `std::string_view`.
*   **Vectorized Arithmetic:** Highly optimized column-to-column operations without row-by-row iteration overhead.
*   **Hash-Based GroupBy:** Fast aggregations (`mean`, `sum`, `count`) utilizing `std::unordered_map`.
*   **Python Integration:** Native Python bindings to use C++ speed with Python's ease of use (bypassing the GIL).

## 📂 Directory Structure
This project follows modern C++ project architecture (Pitchfork Layout):

```text
CollaDF/
│
├── CMakeLists.txt          # Main CMake build configuration
├── README.md               # Project documentation
│
├── include/                # Public header files (.hpp)
│   └── colladf/
│       ├── DataFrame.hpp   # Core DataFrame class
│       ├── Column.hpp      # Template Column storage
│       ├── CsvReader.hpp   # Fast CSV parsing logic
│       └── GroupBy.hpp     # Aggregation logic
│
├── src/                    # C++ Source files (.cpp)
│   ├── DataFrame.cpp
│   ├── CsvReader.cpp
│   └── GroupBy.cpp
│
├── bindings/               # PyBind11 Python wrappers
│   └── python_bindings.cpp # Exposes C++ classes to Python
│
├── tests/                  # Unit tests (Google Test)
│   ├── CMakeLists.txt
│   ├── test_dataframe.cpp
│   └── test_math.cpp
│
└── benchmarks/             # Performance testing scripts
    ├── generate_data.py    # Script to create large dummy CSVs
    ├── benchmark_pandas.py # Timing Pandas
    └── benchmark_colladf.py# Timing CollaDF
```
