# fLua

[<img src="https://img.shields.io/github/license/esrrhs/fLua">](https://github.com/esrrhs/fLua)
[<img src="https://img.shields.io/github/languages/top/esrrhs/fLua">](https://github.com/esrrhs/fLua)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fLua/cmake.yml?branch=master">](https://github.com/esrrhs/fLua/actions)

> **A diff and patch library for Lua tables, with pure Lua and C++ implementations.**

[English](README.md) | [Chinese](README_CN.md)

---

## Overview
**fLua** (`f` stands for fix / diff) computes incremental differences between two Lua table values and applies those differences back as patches. It ships both a pure Lua implementation and a C++ library that can be used standalone or bridged into Lua.

## Features
* **Pure Lua API**: Drop in `pure_lua_diff.lua` and call `lua_diff` / `lua_patch` with no native dependencies.
* **Native C++ API**: Use `CalDiff` / `PatchDiff` directly from C++ with the same algorithm semantics.
* **Lua ↔ C++ Bridge**: Load `libdifflua.so` from Lua to run the C++ implementation on Lua tables.
* **Object Array Awareness**: Optional `get_id` callback reduces diffs for large object arrays by matching elements on a stable identity.
* **Deletion Tracking**: Removed keys are recorded under `__diff_lua_del` so patches can fully restore the destination shape.

## Prerequisites
* **Lua**: 5.3+ (interpreter for pure Lua usage; development headers/libraries for the C++ build)
* **Compiler & Build Tools**: CMake (>= 3.12), C++11 compiler (GCC / Clang / MSVC)
* **Platform**: Linux, macOS, or Windows

## Build
Run the build script:
```bash
./build.sh
```
Or use modern CMake commands:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
```
Artifacts are written to `build/bin/`:
* `libdifflua.so` — shared library / Lua module
* `flua_test` — C++ unit tests

## Usage

### 1. Pure Lua
See [`pure_lua_test.lua`](pure_lua_test.lua):
```bash
lua pure_lua_test.lua
```

Source table:
```lua
local src = {
    a = 1,
    b = "2",
    c = 3.3,
    d = true,
    sub = {
        a = 11,
        b = "22",
        c = 33.3,
        d = true,
    },
    array = { 1, 2, 3, 4 },
    obj_array = {
        { id = 1, a = 1, b = 2 },
        { id = 2, a = 3, b = 4 },
        { id = 3, a = 5, b = 6 },
        { id = 4, a = 7, b = 8 },
    }
}
```

Destination table:
```lua
local dst = {
    b = "2",
    c = 3.4,
    d = false,
    sub = {
        a = 12,
        b = "11",
    },
    array = { 2, 3 },
    obj_array = {
        { id = 2, a = 3, b = 4 },
        { id = 3, a = 5, b = 6 },
        { id = 4, a = 70, b = 80 },
    }
}
```

Compute the diff:
```lua
local get_id = function(v)
    return v.id
end

local diff = lua_diff(src, dst, get_id)
```

`get_id` returns a stable identity for each element in object arrays (here, the `id` field). Returning `nil` falls back to a full-array diff, which may produce a larger result.

Example diff:
```lua
{
    ['d'] = false,
    ['c'] = 3.4,
    ['sub'] = {
        ['b'] = '11',
        ['a'] = 12,
        ['__diff_lua_del'] = {
            [1] = 'd',
            [2] = 'c'
        }
    },
    ['obj_array'] = {
        [4] = {
            ['b'] = 80,
            ['a'] = 70
        },
        ['__diff_lua_array'] = true,
        ['__diff_lua_del'] = {
            [1] = 1
        }
    },
    ['array'] = {
        [1] = 2,
        [2] = 3,
        ['__diff_lua_del'] = {
            [1] = 3,
            [2] = 4
        }
    },
    ['__diff_lua_del'] = {
        [1] = 'a'
    }
}
```

Apply the patch:
```lua
local new_dst = lua_patch(src, diff, get_id)
```
`new_dst` equals `dst` in content. Object-array element order is not guaranteed to be identical.

### 2. Pure C++
See `pure_cpp_test` in [`test/main.cpp`](./test/main.cpp). The algorithm matches the Lua version:
```cpp
#include "diff_lua.h"

auto diff = CalDiff(src, dst, get_id, new_func);
auto new_dst = PatchDiff(src, diff, get_id, new_func);
```

### 3. Mixed C++ + Lua
See `mix_cpp_test` in [`test/main.cpp`](./test/main.cpp) and [`mix_cpp_test.lua`](./mix_cpp_test.lua).

Compute a diff from Lua through the C++ library:
```lua
local libdifflua = require("libdifflua")
local diff = libdifflua.cal_diff(src, dst, _G.lua_get_id, _G.lua_new_func)
```

Apply a patch:
```lua
local new_dst = libdifflua.patch_diff(src, diff, _G.lua_get_id, _G.lua_new_func)
```
The C++ side converts Lua tables into its internal structure, computes the diff/patch, then converts the result back to Lua tables.

---

## Related Projects
* [Lua Family Bucket](https://github.com/esrrhs/lua-family-bucket)

## License
This project is licensed under the [MIT License](LICENSE).
