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
    array = { 1, 2, 3, 4 },
    obj_array = {
        { id = 1, a = 1, b = 2 },
        { id = 2, a = 3, b = 4 },
        { id = 3, a = 5, b = 6 },
        { id = 4, a = 7, b = 8 },
    }
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
    array = { 2, 3 },
    obj_array = {
        { id = 2, a = 3, b = 4 },
        { id = 3, a = 5, b = 6 },
        { id = 4, a = 70, b = 80 },
    }
}
```
计算diff：
```lua
local diff = lua_diff(src, dst, get_id)
```
注意：这里的get_id，是用来针对obj_array中的每个元素，获取其唯一标识的函数。目的是为了尽量减少大Obj数组的diff差异。这里的示例中，我们假设obj_array中的每个元素都有一个id字段，用来唯一标识该元素，所以get_id函数就是：
```lua
local get_id = function(v)
    return v.id
end
```
当然，如果obj_array中的元素没有id字段，那么get_id函数就可以返回nil，这样就会对obj_array进行全量diff，这样的话，diff结果就会比较大。

最终的diff结果为：
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
然后可以将diff应用到原始table上，得到新的table：
```lua
local new_dst = lua_patch(src, diff, get_id)
```
最终新的table与dst是相等的：
```lua
{
	['b'] = '2',
	['d'] = false,
	['c'] = 3.4,
	['obj_array'] = {
		[1] = {
			['b'] = 4,
			['a'] = 3,
			['id'] = 2
		},
		[2] = {
			['b'] = 6,
			['a'] = 5,
			['id'] = 3
		},
		[3] = {
			['b'] = 80,
			['a'] = 70,
			['id'] = 4
		}
	},
	['array'] = {
		[1] = 2,
		[2] = 3
	},
	['sub'] = {
		['b'] = '11',
		['a'] = 12
	}
}
```

### CPP使用
TODO
