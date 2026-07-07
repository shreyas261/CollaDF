cmake_minimum_required(VERSION 3.14)
project(CollaDF VERSION 1.0 DESCRIPTION "A C++ Based Data Manipulation Toolkit" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


add_library(colladf 
    src/DataFrame.cpp 
    src/Reader.cpp 
    src/GroupBy.cpp
)

target_include_directories(colladf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

enable_testing()

find_package(GTest REQUIRED)

add_executable(test_dataframe tests/test_dataframe.cpp)
target_link_libraries(test_dataframe PRIVATE 
    colladf 
    GTest::gtest 
    GTest::gtest_main 
    pthread
)

add_executable(test_math tests/test_math.cpp)
target_link_libraries(test_math PRIVATE 
    colladf 
    GTest::gtest 
    GTest::gtest_main 
    pthread
)

add_test(NAME DataFrameTest COMMAND test_dataframe)
add_test(NAME MathTest COMMAND test_math)