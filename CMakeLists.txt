cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

project(qfsm CXX)
set(AUTOMOC ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
    # set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH
  ${CMAKE_PREFIX_PATH}
  ${Qt5_DIR}
)
message(STATUS "CMAKE_PREFIX_PATH is ${CMAKE_PREFIX_PATH}")
# Find Tarantool and Lua dependecies
set(TARANTOOL_FIND_REQUIRED ON)
find_package(Tarantool)
# find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5Core REQUIRED)
message(STATUS "Qt version is Qt${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}")
include_directories(
  ${TARANTOOL_INCLUDE_DIRS}
)

# Find other dependencies
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=gnu++11 -Wall -Wextra")

# Build module
add_subdirectory(qfsm)

enable_testing()

set (LUA_PATH "LUA_PATH=${PROJECT_SOURCE_DIR}/?.lua\\;${PROJECT_SOURCE_DIR}/?/init.lua\\;;")

add_test(qfsm ${CMAKE_SOURCE_DIR}/test/qfsm.test.lua)
set_tests_properties(qfsm PROPERTIES ENVIRONMENT "${LUA_PATH}")

# Add `make check`
add_custom_target(check
    WORKING_DIRECTORY ${PROJECT_BUILD_DIR}
    COMMAND ctest -V)
