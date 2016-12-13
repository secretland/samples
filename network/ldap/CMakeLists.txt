project(ldap)

cmake_minimum_required(VERSION 2.8)

set(MODULE_NAME ldap)
set(SOURCES_MAIN ${PROJECT_SOURCE_DIR}/src/ldapConnection.cpp
                 ${PROJECT_SOURCE_DIR}/src/main.cpp)

add_executable(${MODULE_NAME} ${SOURCES_MAIN})

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(${MODULE_NAME} ldap)
else(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(${MODULE_NAME} Wldap32)
endif(CMAKE_SYSTEM_NAME STREQUAL "Linux")

set_target_properties(${MODULE_NAME} PROPERTIES 
                                     RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}
                                     ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE})