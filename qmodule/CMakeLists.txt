if (APPLE)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -undefined suppress -flat_namespace")
endif(APPLE)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# message(STATUS "source files is ${PROJECT_SOURCE_DIR}/qmodule/*.cpp")
# file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/qmodule/*.cpp")
# file(GLOB HEADERS "${PROJECT_SOURCE_DIR}/qmodule/*.hpp")
# Add CPP library
add_library(lib SHARED
  qfsmlib.cpp
  testclass.cpp
  pimachine.cpp
  stringtransition.cpp
)
target_link_libraries(lib ${MSGPUCK_LIBRARIES})
target_link_libraries(lib Qt${QT_VERSION_MAJOR}::Core)
set_target_properties(lib PROPERTIES LINKER_LANGUAGE CXX PREFIX "" OUTPUT_NAME "qfsmlib")

# Install module
install(FILES init.lua DESTINATION ${TARANTOOL_INSTALL_LUADIR}/${PROJECT_NAME}/)
install(TARGETS lib LIBRARY DESTINATION ${TARANTOOL_INSTALL_LIBDIR}/${PROJECT_NAME}/)
