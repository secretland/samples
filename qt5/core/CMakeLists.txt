project(qt5core_test)

set(MODULE_NAME core_test)

find_package(Qt5Core REQUIRED)

aux_source_directory(${PROJECT_SOURCE_DIR}/src SOURCES_MAIN)

#include_directories(${Qt5Core_INCLUDES})

message(STATUS "${Qt5Core_INCLUDES} : ${Qt5Core_LIBRARIES}")

add_executable(${MODULE_NAME} ${SOURCES_MAIN})

target_link_libraries(${MODULE_NAME} ${Qt5Core_LIBRARIES})

set_target_properties(${MODULE_NAME} PROPERTIES 
                                     RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}
                                     ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE})
