project(ClientSSL)

cmake_minimum_required(VERSION 2.8)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion COMMAND tr -d '\n' OUTPUT_VARIABLE GCC_VERSION)
    message(STATUS "GCC version: ${GCC_VERSION}")
    if(GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
        message(STATUS "Compiler option: c++11 enabled")
    elseif(GCC_VERSION VERSION_GREATER 4.3 OR GCC_VERSION VERSION_EQUAL 4.3)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
        message(STATUS "Compiler option: c++0x enabled")
    endif(GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7)
endif(CMAKE_SYSTEM_NAME STREQUAL "Linux")

set(MODULE_NAME ssl_client)

find_package(Boost COMPONENTS system thread REQUIRED)
find_package(OpenSSL REQUIRED)

message(STATUS "Boost libs: ${Boost_LIBRARIES}")
message(STATUS "OpenSSL libs: ${OPENSSL_LIBRARIES}")

#add_definitions(-DBOOST_ASIO_SSL #-DSYNC_CONNECTION)
add_definitions(-DBOOST_ASIO_SSL -DSYNC_CONNECTION)

aux_source_directory(${PROJECT_SOURCE_DIR}/src SOURCES_MAIN)

add_executable(${MODULE_NAME} ${SOURCES_MAIN})

target_link_libraries(${MODULE_NAME} ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} pthread boost_thread)

set_target_properties(${MODULE_NAME} PROPERTIES
                                     RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}
                                     ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE})
