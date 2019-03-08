cmake_minimum_required (VERSION 3.13)
project (P-Machine VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)  # enable C++17 standard
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(src/VM)
add_subdirectory(src/Lexical\ Analyzer)