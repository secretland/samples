project(ClientSSL)

cmake_minimum_required(VERSION 3.0)

set(MODULE_NAME ssl_client)

find_package(OpenSSL REQUIRED)

message(STATUS "OpenSSL libs: ${OPENSSL_LIBRARIES}")

#add_definitions(-DBOOST_ASIO_SSL #-DSYNC_CONNECTION)
add_definitions(-DBOOST_ASIO_SSL -DSYNC_CONNECTION)

aux_source_directory(${PROJECT_SOURCE_DIR}/src SOURCES_MAIN)

add_executable(${MODULE_NAME} ${SOURCES_MAIN})

target_link_libraries(${MODULE_NAME} ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} pthread boost_thread)

set_target_properties(${MODULE_NAME} PROPERTIES
                                     RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}
                                     ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE})
