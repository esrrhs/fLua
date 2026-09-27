add_library(difflua SHARED
    diff_lua.cpp
)

target_include_directories(difflua
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${LUA_INCLUDE_DIR}
)

set_target_properties(difflua PROPERTIES
    OUTPUT_NAME "difflua"
    PREFIX "lib"
    POSITION_INDEPENDENT_CODE ON
)

# Lua C modules leave Lua symbols unresolved on UNIX; they are provided by the
# host interpreter (or by the embedding executable linked with --export-dynamic).
if(WIN32)
    target_link_libraries(difflua
        PUBLIC
            ${LUA_LIBRARIES}
    )
elseif(APPLE)
    target_link_options(difflua PRIVATE -undefined dynamic_lookup)
endif()

target_compile_options(difflua
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra>
)
