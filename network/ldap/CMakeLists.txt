project(ldap)

cmake_minimum_required(VERSION 2.8)

set(MODULE_NAME ldap)
set(SOURCES_MAIN ${PROJECT_SOURCE_DIR}/src/ldapConnection.cpp
                 ${PROJECT_SOURCE_DIR}/src/main.cpp)

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

add_executable(${MODULE_NAME} ${SOURCES_MAIN})

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(${MODULE_NAME} ldap)
else(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(${MODULE_NAME} Wldap32)
endif(CMAKE_SYSTEM_NAME STREQUAL "Linux")

set_target_properties(${MODULE_NAME} PROPERTIES 
                                     RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}
                                     ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE})