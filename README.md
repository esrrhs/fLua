# fLua
Lua的增量修改和合并库，同时支持纯Lua和Cpp

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
	['d'] = false,
	['c'] = 3.4,
	['sub'] = {
		['a'] = 12,
		['b'] = '11',
		['__diff_lua_del'] = {
			['d'] = 0,
			['c'] = 0
		}
	},
	['array'] = {
		[1] = 2,
		[2] = 3,
		['__diff_lua_del'] = {
			[4] = 0,
			[3] = 0
		}
	},
	['__diff_lua_del'] = {
		['a'] = 0
	}
}
```

### Cpp使用
TODO
