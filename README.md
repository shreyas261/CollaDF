# CollaDF: High-Performance C++ Data Manipulation Toolkit

**CollaDF** (Columnar Data Frame) is a lightweight, high-performance data manipulation library built in C++. It is designed to provide a subset of Pandas-like functionality while maximizing execution speed through columnar memory storage, cache optimization, and multithreaded execution.

## 🚀 Key Features
- **Columnar Storage:** Data is stored in contiguous memory blocks, ensuring high cache hit rates and enabling SIMD optimizations.
- **Fast I/O:** Custom chunk-based CSV parser that outperforms standard `std::getline` and `stringstream`.
- **Zero-Copy Architecture:** Utilizes `std::string_view` and move semantics to minimize memory overhead.
- **Python Bindings:** Integrated with **PyBind11**, allowing users to write C++ speed logic but interface with it via Python.
- **Multithreaded Aggregations:** Parallelized GroupBy and Mean/Sum operations using C++17 execution policies.

---

## 📂 Project Structure

```text
CollaDF/
├── cmake/                   # CMake configuration files
├── docs/                    # Documentation and Architecture diagrams
├── external/                # Third-party libraries (e.g., pybind11 as a submodule)
├── include/                 # Header files (.hpp)
│   ├── core/
│   │   ├── Column.hpp       # Template-based Column storage
│   │   ├── DataFrame.hpp    # Main DataFrame class
│   │   └── Types.hpp        # Type definitions (variants, enums)
│   ├── io/
│   │   └── CSVReader.hpp    # Fast CSV parsing logic
│   └── ops/
│       ├── Aggregators.hpp  # Sum, Mean, Count logic
│       └── GroupBy.hpp      # Hash-based grouping logic
├── src/                     # Implementation files (.cpp)
│   ├── core/
│   │   └── DataFrame.cpp
│   ├── io/
│   │   └── CSVReader.cpp
│   └── main.cpp             # CLI entry point for testing C++ performance
├── python/                  # Python wrapper and scripts
│   ├── colladf_py.cpp       # PyBind11 binding code
│   └── example.py           # Example usage in Python
├── tests/                   # Unit tests (using Google Test)
│   ├── test_dataframe.cpp
│   └── test_io.cpp
├── benchmarks/              # Performance comparison scripts
│   ├── benchmark_pandas.py
│   └── benchmark_colladf.py
├── CMakeLists.txt           # Build system configuration
└── README.md
```