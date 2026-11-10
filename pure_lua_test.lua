require("pure_lua_diff")
local regression = require("flua_regression")

local failed, total = regression.run(lua_diff, lua_patch, "pure_lua")
print(string.format("pure_lua_test: %d/%d passed", total - failed, total))
if failed > 0 then
    error("pure_lua_test failed: " .. failed .. " case(s)")
end
