import os
import sys
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
import pybind11

# Add standard C++17 flags
class get_pybind_include(object):
    def __str__(self):
        return pybind11.get_include()

ext_modules = [
    Extension(
        "colladf_py",  # This is the name you will import in Python
        sources=[
            "src/pybind_wrapper.cpp",
            "src/DataFrame.cpp",
            "src/GroupBy.cpp",
            "src/Reader.cpp",
        ],
        include_dirs=[
            get_pybind_include(),
            "include/colladf",
        ],
        language="c++",
        extra_compile_args=["-O3", "-std=c++17", "-march=native", "-Wall"],
    )
]

setup(
    name="colladf_py",
    version="0.1.0",
    author="namjoshisp",
    description="Python bindings for CollaDF",
    ext_modules=ext_modules,
    setup_requires=["pybind11>=2.5.0"],
    install_requires=["pybind11>=2.5.0"],
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
)