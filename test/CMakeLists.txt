add_executable(flua_test
    main.cpp
)

target_include_directories(flua_test
    PRIVATE
        ${LUA_INCLUDE_DIR}
)

target_link_libraries(flua_test
    PRIVATE
        difflua
        ${LUA_LIBRARIES}
)

if(NOT WIN32)
    target_link_libraries(flua_test
        PRIVATE
            ${CMAKE_DL_LIBS}
            m
    )
endif()

# Export Lua symbols so libdifflua.so can resolve them when loaded via require()
if(UNIX AND NOT APPLE)
    target_link_options(flua_test PRIVATE -Wl,--export-dynamic)
endif()

target_compile_options(flua_test
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra>
)

add_custom_command(
    TARGET flua_test
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/mix_cpp_test.lua
        $<TARGET_FILE_DIR:flua_test>/mix_cpp_test.lua
    COMMENT "Copy mix_cpp_test.lua next to flua_test"
)
