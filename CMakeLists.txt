cmake_minimum_required(VERSION 3.12)
project(flua LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

# Keep build artifacts together so the test binary can load libdifflua.so
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Prefer a shared Lua library so it can be linked into libdifflua.so
if(NOT WIN32)
    set(_flua_saved_suffixes "${CMAKE_FIND_LIBRARY_SUFFIXES}")
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".so" ".a")
endif()

find_package(Lua REQUIRED)

if(NOT WIN32)
    set(CMAKE_FIND_LIBRARY_SUFFIXES "${_flua_saved_suffixes}")
endif()

message(STATUS "Lua include dir: ${LUA_INCLUDE_DIR}")
message(STATUS "Lua libraries:   ${LUA_LIBRARIES}")

add_subdirectory(difflua)
add_subdirectory(test)

enable_testing()

find_program(LUA_EXECUTABLE NAMES lua lua5.4 lua5.3 lua5.2 lua5.1)
if(LUA_EXECUTABLE)
    # Quote LUA_PATH so the trailing ";;" is not split as a CMake list.
    add_test(
        NAME pure_lua_test
        COMMAND ${CMAKE_COMMAND} -E env
            "LUA_PATH=${CMAKE_SOURCE_DIR}/?.lua;;"
            ${LUA_EXECUTABLE} ${CMAKE_SOURCE_DIR}/pure_lua_test.lua
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
else()
    if(DEFINED ENV{CI})
        message(FATAL_ERROR "Lua interpreter not found; pure_lua_test is required in CI")
    else()
        message(WARNING "Lua interpreter not found; skipping pure_lua_test")
    endif()
endif()

add_test(
    NAME cpp_test
    COMMAND $<TARGET_FILE:flua_test>
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
