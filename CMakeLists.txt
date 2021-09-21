cmake_minimum_required(VERSION 3.18.0 FATAL_ERROR)

project(qmodule CXX)
set(AUTOMOC ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" ${CMAKE_MODULE_PATH})
set(QT_DIR "/home/qdrin/Qt/5.15.2/gcc_64")
set(LD_LIBRARY_PATH "${QT_DIR}/lib")
set(Qt5_DIR "${LD_LIBRARY_PATH}/cmake/Qt5")
message(STATUS "Qt search path is ${Qt5_DIR}")
set(CMAKE_PREFIX_PATH
  ${CMAKE_PREFIX_PATH}
  ${Qt5_DIR}
)
# Find Tarantool and Lua dependecies
set(TARANTOOL_FIND_REQUIRED ON)
find_package(Tarantool)
find_package(Qt NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Core REQUIRED)
message(STATUS "Qt version is Qt${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}")
include_directories(
  ${TARANTOOL_INCLUDE_DIRS}
)

# Find other dependencies

# Set CFLAGS
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
# set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -Wextra")
# Set CXXFLAGS
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra")

# Build module
add_subdirectory(qmodule)

enable_testing()

set (LUA_PATH "LUA_PATH=${PROJECT_SOURCE_DIR}/?.lua\\;${PROJECT_SOURCE_DIR}/?/init.lua\\;;")

add_test(qmodule ${CMAKE_SOURCE_DIR}/test/qmodule.test.lua)
set_tests_properties(qmodule PROPERTIES ENVIRONMENT "${LUA_PATH}")

# Add `make check`
add_custom_target(check
    WORKING_DIRECTORY ${PROJECT_BUILD_DIR}
    COMMAND ctest -V)
