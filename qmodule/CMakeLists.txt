if (APPLE)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -undefined suppress -flat_namespace")
endif(APPLE)
file(GLOB HEADERS "${PROJECT_SOURCE_DIR}/qmodule/*.h")
message(STATUS PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR})
message(STATUS HEADERS "${HEADERS}")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
qt_wrap_cpp(MOC_SOURCES ${HEADERS})
message(STATUS MOC_SOURCES ${MOC_SOURCES})
# Add CPP library
add_library(lib SHARED
  qfsmlib.cpp
  testclass.cpp
  pimachine.cpp
  stringtransition.cpp
  ${MOC_SOURCES}
)
target_link_libraries(lib Qt${QT_VERSION_MAJOR}::Core)
set_target_properties(lib PROPERTIES PREFIX "" OUTPUT_NAME "qfsmlib")

# Install module
install(FILES init.lua DESTINATION ${TARANTOOL_INSTALL_LUADIR}/${PROJECT_NAME}/)
install(TARGETS lib LIBRARY DESTINATION ${TARANTOOL_INSTALL_LIBDIR}/${PROJECT_NAME}/)
