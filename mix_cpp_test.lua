-- Prefer the locally built shared library next to the test binary
package.cpath = "./?.so;./?.dylib;./?.dll;" .. package.cpath
package.path = "./?.lua;" .. package.path

local libdifflua = require("libdifflua")
require("pure_lua_diff")
local regression = require("flua_regression")

local function cpp_diff(src, dst, get_id)
    if get_id then
        return libdifflua.cal_diff(src, dst, get_id)
    end
    return libdifflua.cal_diff(src, dst)
end

local function cpp_patch(src, diff, get_id)
    if get_id then
        return libdifflua.patch_diff(src, diff, get_id)
    end
    return libdifflua.patch_diff(src, diff)
end

local failed = 0
local total = 0

local f, t = regression.run(cpp_diff, cpp_patch, "libdifflua")
failed = failed + f
total = total + t

f, t = regression.run_cross(lua_diff, lua_patch, cpp_diff, cpp_patch)
failed = failed + f
total = total + t

-- Backward-compatible 4-arg C callback form, injected by flua_test.
if type(lua_get_id) == "function" and type(lua_new_func) == "function" then
    local src = {
        { id = 10, a = 1 },
        { id = 20, a = 2 },
    }
    local dst = {
        { id = 10, a = 100 },
        { id = 20, a = 2 },
    }
    local ok, diff = pcall(libdifflua.cal_diff, src, dst, lua_get_id, lua_new_func)
    if not ok then
        failed = failed + 1
        print("FAIL  legacy_c_callbacks: cal_diff error: " .. tostring(diff))
    else
        local src_copy = regression.deep_copy(src)
        local pok, patched = pcall(libdifflua.patch_diff, src_copy, diff, lua_get_id, lua_new_func)
        if not pok then
            failed = failed + 1
            print("FAIL  legacy_c_callbacks: patch_diff error: " .. tostring(patched))
        elseif not regression.deep_equal(dst, patched, function(v) return v.id end) then
            failed = failed + 1
            print("FAIL  legacy_c_callbacks: patched result does not match dst")
        elseif diff.__diff_lua_array ~= true or type(diff[10]) ~= "table" then
            failed = failed + 1
            print("FAIL  legacy_c_callbacks: expected id-keyed object-array diff")
        else
            print("OK  legacy_c_callbacks")
        end
    end
    total = total + 1
end

local function expect_error(name, fn, needle)
    total = total + 1
    local ok, err = pcall(fn)
    if ok then
        failed = failed + 1
        print("FAIL  " .. name .. ": expected an error")
        return
    end
    err = tostring(err)
    if needle and not string.find(err, needle, 1, true) then
        failed = failed + 1
        print("FAIL  " .. name .. ": unexpected error: " .. err)
        return
    end
    print("OK  " .. name)
end

expect_error("error_unsupported_type", function()
    libdifflua.cal_diff({ f = function() end }, { f = 1 })
end, "unsupported Lua type")

expect_error("error_get_id_throws", function()
    libdifflua.cal_diff({ { id = 1 } }, { { id = 2 } }, function()
        error("boom")
    end)
end, "get_id failed")

expect_error("error_bad_get_id_arg", function()
    libdifflua.cal_diff({}, {}, "not a function")
end, "get_id must be a function")

expect_error("error_too_few_args", function()
    libdifflua.cal_diff({})
end, "requires at least 2 arguments")

print(string.format("mix_cpp_test: %d/%d passed", total - failed, total))
if failed > 0 then
    error("mix_cpp_test failed: " .. failed .. " case(s)")
end
