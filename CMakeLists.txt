cmake_minimum_required(VERSION 3.12)
project(kmer_list_to_hist)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

add_subdirectory(abseil-cpp)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

add_executable(kmer_list_to_hist main.cpp)
target_include_directories(kmer_list_to_hist PRIVATE abseil-cpp/)
target_link_libraries(kmer_list_to_hist
        absl::base
        absl::strings
        absl::str_format
        absl::flat_hash_map
        absl::time)
target_compile_options(kmer_list_to_hist PRIVATE -Wall -Wextra)

if (${CMAKE_BUILD_TYPE} MATCHES "^Debug.*")
    set(CMAKE_POSITION_INDEPENDENT_CODE OFF)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
    target_compile_options(kmer_list_to_hist PRIVATE
            -fbuiltin
            -g3
            -Og
            -fno-omit-frame-pointer)
    target_link_options(kmer_list_to_hist PRIVATE
            -fno-omit-frame-pointer)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(kmer_list_to_hist PRIVATE
                -fprofile-instr-generate
                -fcoverage-mapping)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(kmer_list_to_hist PRIVATE
                -fprofile-arcs
                -ftest-coverage)
        target_link_options(kmer_list_to_hist PRIVATE
                -fprofile-arcs
                -ftest-coverage)
    endif ()
else ()
    target_compile_options(kmer_list_to_hist PRIVATE -O3 -mtune=native -march=native)
endif ()
