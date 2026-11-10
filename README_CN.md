# fLua

[<img src="https://img.shields.io/github/license/esrrhs/fLua">](https://github.com/esrrhs/fLua)
[<img src="https://img.shields.io/github/languages/top/esrrhs/fLua">](https://github.com/esrrhs/fLua)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fLua/cmake.yml?branch=master">](https://github.com/esrrhs/fLua/actions)

> **Lua table 的增量 diff / patch 库，同时提供纯 Lua 与 C++ 实现。**

[English](README.md) | [中文说明](README_CN.md)

---

## 简介
**fLua**（`f` 代表 fix / diff）用于计算两个 Lua table 之间的增量差异，并将差异作为 patch 合并回去。项目同时提供纯 Lua 实现，以及可独立使用或桥接到 Lua 的 C++ 库。

## 特性
* **纯 Lua API**：引入 `pure_lua_diff.lua` 后即可调用 `lua_diff` / `lua_patch`，无需原生依赖。
* **原生 C++ API**：可直接在 C++ 中调用 `CalDiff` / `PatchDiff`，语义与 Lua 版本一致。
* **Lua ↔ C++ 桥接**：通过 `require("libdifflua")` 在 Lua 中调用 C++ 实现。
* **对象数组感知**：可选 `get_id` 回调按稳定身份匹配大对象数组元素，显著减小 diff。
* **删除追踪**：被删除的键记录在 `__diff_lua_del` 中，确保 patch 能完整还原目标结构。

## 前置依赖
* **Lua**：5.3+（纯 Lua 使用只需解释器；编译 C++ 部分需要开发头文件与库）
* **编译器与构建工具**：CMake (>= 3.12)，支持 C++11 的编译器（GCC / Clang / MSVC）
* **平台**：Linux、macOS 或 Windows

## 编译
直接运行编译脚本：
```bash
./build.sh
```
或使用现代 CMake 命令：
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
```
产物输出到 `build/bin/`：
* `libdifflua.so` — 动态库 / Lua 模块
* `flua_test` — C++ 单元测试

## 使用方法

### 1. 纯 Lua 使用
参考 [`pure_lua_test.lua`](pure_lua_test.lua)：
```bash
lua pure_lua_test.lua
```

原始 table：
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

新的 table：
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

计算 diff：
```lua
local get_id = function(v)
    return v.id
end

local diff = lua_diff(src, dst, get_id)
```

`get_id` 用于从对象数组元素中取出稳定唯一标识（本例为 `id` 字段）。若返回 `nil`，则退化为全量数组 diff，结果通常更大。

示例 diff 结果：
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

将 diff 应用到原始 table：
```lua
local new_dst = lua_patch(src, diff, get_id)
```
最终 `new_dst` 与 `dst` 内容相等；注意 `obj_array` 中元素顺序不一定完全一致。

### 2. 纯 C++ 使用
参考 [`test/main.cpp`](./test/main.cpp) 中的 `pure_cpp_test`，算法与 Lua 版本一致：
```cpp
#include "diff_lua.h"

auto diff = CalDiff(src, dst, get_id, new_func);
auto new_dst = PatchDiff(src, diff, get_id, new_func);
```

### 3. C++ + Lua 混合使用
参考 [`mix_cpp_test.lua`](./mix_cpp_test.lua) 与 [`test/main.cpp`](./test/main.cpp) 中的 `mix_cpp_test`。在 Lua 中加载编译出的 `libdifflua` 模块即可（如有需要，将 `build/bin` 加入 `package.cpath`）：

```lua
local libdifflua = require("libdifflua")

local get_id = function(v)
    return v.id
end

local diff = libdifflua.cal_diff(src, dst, get_id)
local new_dst = libdifflua.patch_diff(src, diff, get_id)
```

`get_id` 是普通的 Lua 函数 `function(elem) return id end`。省略该参数则不做对象数组的身份对齐。遇到不支持的类型、`get_id` 报错或参数不正确时，会抛出 Lua error。

C++ 宿主仍可把两个 C 函数作为第 3、第 4 个参数传入（`get_id` 接收 `DiffVarInterface*` lightuserdata，`new_func` 负责分配）。普通 Lua 使用无需这种形式。

C++ 侧会将 Lua table 转为内置数据结构，完成计算后再转回 Lua table。

---

## 相关项目
* [lua全家桶](https://github.com/esrrhs/lua-family-bucket)

## 许可证
本项目采用 [MIT License](LICENSE)。
