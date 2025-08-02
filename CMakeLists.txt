cmake_minimum_required(VERSION 3.1)

project(COMP_BINARY_TRIES VERSION 1.0)

include_directories(~/include)
include_directories(${COMP_BINARY_TRIES_SOURCE_DIR}/include)
link_directories(~/lib)

set(CMAKE_CXX_STANDARD 17)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_compile_definitions("LINUX")
else()
    add_compile_definitions("MACOS")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
# add_library(utils ../src/utils.cpp ../include/utils.hpp)
add_library(intersection ../src/intersection.cpp ../include/intersection.hpp)
add_library(fastTrieIntersection ../src/fastTrieIntersection.cpp ../include/fastTrieIntersection.hpp)
add_library(xFastTrieIntersection ../src/xFastTrieIntersection.cpp ../include/xFastTrieIntersection.hpp)
add_library(xRFastTrieIntersection ../src/xRFastTrieIntersection.cpp ../include/xRFastTrieIntersection.hpp)
add_library(xTFastTrieIntersection ../src/xTFastTrieIntersection.cpp ../include/xTFastTrieIntersection.hpp)
add_library(xTRFastTrieIntersection ../src/xTRFastTrieIntersection.cpp ../include/xTRFastTrieIntersection.hpp)
add_library(xRFastTrie3WIntersection ../src/xRFastTrie3WIntersection.cpp ../include/xRFastTrie3WIntersection.hpp)
add_library(xTRFastTrie3WIntersection ../src/xTRFastTrie3WIntersection.cpp ../include/xTRFastTrie3WIntersection.hpp)
# target_link_libraries(fastTrieIntersection PUBLIC sdsl divsufsort divsufsort64)

add_subdirectory(test)
add_subdirectory(tools)

