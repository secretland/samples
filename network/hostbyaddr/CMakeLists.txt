project(HostByAddr)

cmake_minimum_required(VERSION 2.8)

set(MODULE_NAME hostByAddr)
set(SOURCES_MAIN ${PROJECT_SOURCE_DIR}/src/main.cpp)

add_executable(${MODULE_NAME} ${SOURCES_MAIN})

target_link_libraries(${MODULE_NAME} WS2_32.lib)

set_target_properties(${MODULE_NAME} PROPERTIES 
                                     RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}
                                     ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE})