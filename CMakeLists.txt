# Copyright (C) 2022 Roberto Rossini <roberros@uio.no>
#
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.16)
project(kmer_list_to_hist)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# cmake-format: off
FetchContent_Declare(
        abseil-cpp
        URL ${CMAKE_CURRENT_SOURCE_DIR}/external/abseil-cpp-v20211102.0.tar.gz
        URL_HASH SHA512=fed68aa434c02ec6faa9d1c81f1ad35b60ec024b44957e2e0ac31e6075e385c06a3e1b616afeb4bb7c2413191fd7827d82e1f9f0796b52ed21fb2c41dd9031cf
)
FetchContent_Declare(
        mscharconv
        URL ${CMAKE_CURRENT_SOURCE_DIR}/external/mscharconv.tar.xz
        URL_HASH SHA512=4378f5be5336c726c3c9d104941a45379484519cb34d61001c408c496b47b53547c9af3a82c9cd0eb332df0c97d244e2f6a20a34aa437cb304980e567e364c2c
)
# cmake-format: on

set(ABSL_PROPAGATE_CXX_STD ON)
FetchContent_GetProperties(abseil-cpp)
FetchContent_GetProperties(mscharconv)

if(NOT ${abseil-cpp}_POPULATED)
  FetchContent_Populate(abseil-cpp)
endif()
add_subdirectory(${abseil-cpp_SOURCE_DIR} ${abseil-cpp_BINARY_DIR} EXCLUDE_FROM_ALL)

if(NOT ${mscharconv}_POPULATED)
  FetchContent_Populate(mscharconv)
endif()
add_subdirectory(${mscharconv_SOURCE_DIR} ${mscharconv_BINARY_DIR})

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_executable(kmer_list_to_hist ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

target_link_libraries(
  kmer_list_to_hist
  absl::base
  absl::strings
  absl::str_format
  absl::flat_hash_map
  absl::time
  mscharconv)

set(CLANG_WARNINGS
    -Wall
    -Wextra # reasonable and standard
    -Wnon-virtual-dtor # warn the user if a class with virtual functions has a non-virtual destructor. This helps
    # catch hard to track down memory errors
    -Wold-style-cast # warn for c-style casts
    -Wcast-align # warn for potential performance problem casts
    -Wunused # warn on anything being unused
    -Woverloaded-virtual # warn if you overload (not override) a virtual function
    -Wconversion # warn on type conversions that may lose data
    -Wsign-conversion # warn on sign conversions
    -Wnull-dereference # warn if a null dereference is detected
    -Wdouble-promotion # warn if float is implicit promoted to double
    -Wformat=2 # warn on security issues around functions that format output (ie printf)
    -Wimplicit-fallthrough # warn on statements that fallthrough without an explicit annotation
)

set(GCC_WARNINGS
    ${CLANG_WARNINGS}
    -Wmisleading-indentation # warn if indentation implies blocks where blocks do not exist
    -Wduplicated-cond # warn if if / else chain has duplicated conditions
    -Wduplicated-branches # warn if if / else branches have duplicated code
    -Wlogical-op # warn about logical operations being used where bitwise were probably wanted
    -Wuseless-cast # warn if you perform a cast to the same type
)

target_compile_options(kmer_list_to_hist PRIVATE $<IF:$<CXX_COMPILER_ID:GNU>,${GCC_WARNINGS},${CLANG_WARNINGS}>)

set_property(
  TARGET kmer_list_to_hist
  APPEND
  PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES ${abseil-cpp_SOURCE_DIR})
