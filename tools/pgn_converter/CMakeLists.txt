project(pgnconverter)
cmake_minimum_required(VERSION 3.5.1)
set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME} src/pgn_to_coordinates_converter.cpp)
add_subdirectory(../../libchess libchess)
target_include_directories(pgnconverter PRIVATE include)
target_link_libraries(${PROJECT_NAME} libchess-static)
target_include_directories(${PROJECT_NAME} PRIVATE ../../libchess/src/libchess)

