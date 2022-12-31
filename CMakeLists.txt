project(Scarlett-G)
cmake_minimum_required(VERSION 3.5.1)
set(CMAKE_CXX_STANDARD 20)

file(GLOB SOURCES "src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})
#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/EvolutionaryAlgorithms)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libchess)
target_link_libraries(${PROJECT_NAME} libchess-shared)

# Add all headers files under the include directory
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_include_directories(${PROJECT_NAME} PRIVATE libchess/src/libchess)
