project(nbt)

cmake_minimum_required(VERSION 2.8)

set(MODULE_NAME nbt)
aux_source_directory(${PROJECT_SOURCE_DIR}/src SOURCES_MAIN)

list(REMOVE_ITEM SOURCES_MAIN #${PROJECT_SOURCE_DIR}/src/nbt.c
                              ${PROJECT_SOURCE_DIR}/src/main_org.c)

add_definitions(-D _WINSOCK_DEPRECATED_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS)

add_executable(${MODULE_NAME} ${SOURCES_MAIN})

target_link_libraries(${MODULE_NAME} WS2_32.lib)

set_target_properties(${MODULE_NAME} PROPERTIES 
                                     RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}
                                     ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE})