cmake_minimum_required(VERSION 2.8)
project(test)

include_directories(.)
include_directories(../difflua)

aux_source_directory(./ TEST_SRC_LIST)

add_executable(test ${TEST_SRC_LIST})
IF (WIN32)
    target_link_libraries(test lua difflua)
ELSE ()
    target_link_libraries(test lua difflua dl)
ENDIF ()

add_custom_command(
        TARGET test
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/../mix_cpp_test.lua ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/
)
