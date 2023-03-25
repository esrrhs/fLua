# fLua
Lua的增量修改和合并库，同时支持纯Lua和CPP

# 使用
### 纯Lua使用
参考[pure_lua_test.lua](pure_lua_test.lua)中的示例，运行方式：
```shell
lua pure_lua_test.lua
```
原始table为：
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
    array = { 1, 2, 3, 4 }
}
```
新的table为：
```lua
local dst = {
    b = "2",
    c = 3.4,
    d = false,
    sub = {
        a = 12,
        b = "11",
    },
    array = { 2, 3 }
}
```
diff结果为：
```lua
{
    ['c'] = 3.4,
    ['d'] = false,
    ['__diff_lua_del'] = {
        [1] = 'a'
    },
    ['sub'] = {
        ['a'] = 12,
        ['b'] = '11',
        ['__diff_lua_del'] = {
            [1] = 'd',
            [2] = 'c'
        }
    },
    ['array'] = {
        [1] = 2,
        [2] = 3,
        ['__diff_lua_del'] = {
            [1] = 3,
            [2] = 4
        }
    }
}
```

### CPP使用
TODO
