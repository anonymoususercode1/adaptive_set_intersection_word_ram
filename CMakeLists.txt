cmake_minimum_required(VERSION 3.1)

project(COMP_BINARY_TRIES VERSION 1.0)

include(ExternalProject)

set(SDSL_INSTALL_DIR ${CMAKE_BINARY_DIR}/sdsl-install)

ExternalProject_Add(sdsl_project
    GIT_REPOSITORY https://github.com/simongog/sdsl-lite.git
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ${CMAKE_BINARY_DIR}/sdsl-src/install.sh ${SDSL_INSTALL_DIR}
    SOURCE_DIR ${CMAKE_BINARY_DIR}/sdsl-src
    BINARY_DIR ${CMAKE_BINARY_DIR}/sdsl-build
)

include_directories(${COMP_BINARY_TRIES_SOURCE_DIR}/include)
include_directories(${SDSL_INSTALL_DIR}/include)
link_directories(${SDSL_INSTALL_DIR}/lib)

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


add_library(intersection ../src/intersection.cpp ../include/intersection.hpp)
add_library(wBTrieIntersection ../src/wBTrieIntersection.cpp ../include/wBTrieIntersection.hpp)
add_library(x2WBTrieIntersection ../src/x2WBTrieIntersection.cpp ../include/x2WBTrieIntersection.hpp)
add_library(x2WRBTrieIntersection ../src/x2WRBTrieIntersection.cpp ../include/x2WRBTrieIntersection.hpp)
add_library(x2WTBTrieIntersection ../src/x2WTBTrieIntersection.cpp ../include/x2WTBTrieIntersection.hpp)
add_library(x2WTRBTrieIntersection ../src/x2WTRBTrieIntersection.cpp ../include/x2WTRBTrieIntersection.hpp)
add_library(x3WRBTrieIntersection ../src/x3WRBTrieIntersection.cpp ../include/x3WRBTrieIntersection.hpp)
add_library(x3WTRBTrieIntersection ../src/x3WTRBTrieIntersection.cpp ../include/x3WTRBTrieIntersection.hpp)

add_subdirectory(test)
add_subdirectory(tools)