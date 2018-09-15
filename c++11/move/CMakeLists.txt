project(TestCpp11Move)

cmake_minimum_required(VERSION 2.8)

set(MODULE_NAME testmove)

aux_source_directory(${PROJECT_SOURCE_DIR}/src SOURCES_MAIN)

add_executable(${MODULE_NAME} ${SOURCES_MAIN})

set_target_properties(${MODULE_NAME} PROPERTIES
                                     RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}
                                     ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE})
