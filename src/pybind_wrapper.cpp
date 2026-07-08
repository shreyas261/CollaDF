#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/operators.h>
#include <cstdint> // <--- Added for uint8_t
#include "../include/colladf/DataFrame.hpp"
#include "../include/colladf/Reader.hpp"
#include "../include/colladf/GroupBy.hpp"

namespace py = pybind11;

// Using uint8_t to avoid the std::vector<bool> bit-packing bottleneck
struct BoolMask {
    std::vector<uint8_t> mask;
};

PYBIND11_MODULE(colladf_py, m) {
    m.doc() = "CollaDF: A fast C++ DataFrame library exposed to Python";

    // Bind the Mask Object and bitwise operators (&, |, ~)
    py::class_<BoolMask>(m, "BoolMask")
        .def(py::init<std::vector<uint8_t>>())
        // Explicitly calling the global operators defined in DataFrame.hpp
        .def("__and__", [](const BoolMask& a, const BoolMask& b) { return BoolMask{operator&(a.mask, b.mask)}; })
        .def("__or__", [](const BoolMask& a, const BoolMask& b) { return BoolMask{operator|(a.mask, b.mask)}; })
        .def("__invert__", [](const BoolMask& a) { return BoolMask{operator!(a.mask)}; });

    // Bind GroupBy
    py::class_<GroupBy>(m, "GroupBy")
        .def("mean", &GroupBy::mean)
        .def("max", &GroupBy::max)
        .def("min", &GroupBy::min)
        .def("count", &GroupBy::count)
        .def("sum", &GroupBy::sum);

    // Bind DataFrame
    py::class_<DataFrame>(m, "DataFrame")
        .def(py::init<>())
        .def("shape", &DataFrame::shape)
        .def("columns", &DataFrame::columns)
        .def("head", &DataFrame::head, py::arg("n") = 5)
        .def("tail", &DataFrame::tail, py::arg("n") = 5)
        .def("describe", &DataFrame::describe)
        .def("select", &DataFrame::select)
        .def("drop", &DataFrame::drop)
        
        // Use lambdas for overloaded methods to avoid compiler confusion
        .def("groupby", [](const DataFrame& df, const std::string& col) { return df.groupby(col); })
        .def("groupby", [](const DataFrame& df, const std::vector<std::string>& cols) { return df.groupby(cols); })
        
        .def("sort_values", [](const DataFrame& df, const std::string& name, bool asc) { return df.sort_values(name, asc); }, py::arg("name"), py::arg("ascending") = true)
        .def("sort_values", [](const DataFrame& df, const std::vector<std::string>& cols, bool asc) { return df.sort_values(cols, asc); }, py::arg("columns"), py::arg("ascending") = true)

        // Indexing: df["col"] and df[mask]
        .def("__getitem__", [](DataFrame& df, const std::string& name) { return df[name]; })
        .def("__getitem__", [](DataFrame& df, const BoolMask& m) { return df.filter(m.mask); })

        // Arithmetic with Scalars
        .def("__add__", [](const DataFrame& df, const ScalarValue& v) { return df + v; })
        .def("__sub__", [](const DataFrame& df, const ScalarValue& v) { return df - v; })
        .def("__mul__", [](const DataFrame& df, const ScalarValue& v) { return df * v; })
        .def("__truediv__", [](const DataFrame& df, const ScalarValue& v) { return df / v; })

        // Arithmetic with other DataFrames
        .def("__add__", [](const DataFrame& df, const DataFrame& other) { return df + other; })
        .def("__sub__", [](const DataFrame& df, const DataFrame& other) { return df - other; })
        .def("__mul__", [](const DataFrame& df, const DataFrame& other) { return df * other; })
        .def("__truediv__", [](const DataFrame& df, const DataFrame& other) { return df / other; })

        // Comparisons (Returning our fast BoolMask object)
        .def("__gt__", [](const DataFrame& df, const ScalarValue& v) { return BoolMask{df > v}; })
        .def("__lt__", [](const DataFrame& df, const ScalarValue& v) { return BoolMask{df < v}; })
        .def("__eq__", [](const DataFrame& df, const ScalarValue& v) { return BoolMask{df == v}; })
        .def("__ge__", [](const DataFrame& df, const ScalarValue& v) { return BoolMask{df >= v}; })
        .def("__le__", [](const DataFrame& df, const ScalarValue& v) { return BoolMask{df <= v}; })
        .def("__ne__", [](const DataFrame& df, const ScalarValue& v) { return BoolMask{df != v}; });

    // Bind Reader methods to the module root
    m.def("read_csv", &Reader::read_csv, "Read CSV into DataFrame");
    m.def("to_csv", &Reader::to_csv, "Write DataFrame to CSV");
}