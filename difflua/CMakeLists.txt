cmake_minimum_required(VERSION 2.8)
project(difflua)

include_directories(.)

aux_source_directory(./ DIFFLUA_SRC_LIST)

add_library(difflua SHARED ${DIFFLUA_SRC_LIST})
IF (WIN32)
    target_link_libraries(difflua lua)
ELSE ()
    target_link_libraries(difflua lua dl)
ENDIF ()
