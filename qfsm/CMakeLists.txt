if (APPLE)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -undefined suppress -flat_namespace")
endif(APPLE)
file(GLOB HEADERS "${PROJECT_SOURCE_DIR}/qfsm/*.h")
message(STATUS PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR})
message(STATUS HEADERS " ${HEADERS}")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

qt_wrap_cpp(MOC_SOURCES ${HEADERS})
message(STATUS MOC_SOURCES ${MOC_SOURCES})
# Add CPP library
add_library(lib SHARED
  qfsmlib.cpp
  smcontroller.cpp
  pimachine.cpp
  pistates.cpp
  stringtransition.cpp
  ${MOC_SOURCES}
)
target_link_libraries(lib Qt5::Core)
set_target_properties(lib PROPERTIES PREFIX "" OUTPUT_NAME "qfsmlib")

# Install module
message(STATUS ${TARANTOOL_INSTALL_LIBDIR}/${PROJECT_NAME}/)
install(FILES init.lua validators.lua DESTINATION ${TARANTOOL_INSTALL_LUADIR}/${PROJECT_NAME}/)
install(TARGETS lib LIBRARY DESTINATION ${TARANTOOL_INSTALL_LIBDIR}/${PROJECT_NAME}/)
