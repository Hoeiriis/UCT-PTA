cmake_minimum_required(VERSION 3.5)
project(example_exec VERSION 0.0.1 LANGUAGES CXX)

add_executable(example_exec src/main)
#target_compile_features(example_exec PRIVATE cxx_std_17)

find_package(MCTS 0.1.0 REQUIRED)
if(MCTS_FOUND)
    target_link_libraries(example_exec MCTS)
endif()

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(example_exec OpenMP::OpenMP_CXX)
else()
    message(STATUS "lort")
endif()

