-- Shared regression cases for the pure Lua and C++ (libdifflua) implementations.
local M = {}

local function is_array_has_id(t, get_id)
    if type(get_id) ~= "function" or type(t) ~= "table" then
        return false
    end
    local i = 0
    for _ in pairs(t) do
        i = i + 1
        if t[i] == nil or type(t[i]) ~= "table" or get_id(t[i]) == nil then
            return false
        end
    end
    return true
end

local function array_to_map(t, get_id)
    local map = {}
    for _, v in ipairs(t) do
        map[get_id(v)] = v
    end
    return map
end

function M.deep_copy(o)
    if type(o) ~= "table" then
        return o
    end
    local r = {}
    for k, v in pairs(o) do
        r[M.deep_copy(k)] = M.deep_copy(v)
    end
    return r
end

-- Order-independent for object arrays when get_id is provided; keys compared exactly
-- (false is not treated as missing).
function M.deep_equal(a, b, get_id)
    if a == b then
        return true
    end
    if type(a) ~= "table" or type(b) ~= "table" then
        return false
    end
    if is_array_has_id(a, get_id) and is_array_has_id(b, get_id) then
        a = array_to_map(a, get_id)
        b = array_to_map(b, get_id)
    end
    for k, v in pairs(a) do
        if not M.deep_equal(v, b[k], get_id) then
            return false
        end
    end
    for k, _ in pairs(b) do
        if a[k] == nil then
            return false
        end
    end
    return true
end

local function by_id(v)
    return v.id
end

local function computed_id(v)
    return "x" .. tostring(v.id)
end

M.cases = {
    {
        name = "readme_example",
        src = {
            a = 1,
            b = "2",
            c = 3.3,
            d = true,
            sub = { a = 11, b = "22", c = 33.3, d = true },
            array = { 1, 2, 3, 4 },
            obj_array = {
                { id = 1, a = 1, b = 2 },
                { id = 2, a = 3, b = 4 },
                { id = 3, a = 5, b = 6 },
                { id = 4, a = 7, b = 8 },
            },
        },
        dst = {
            b = "2",
            c = 3.4,
            d = false,
            sub = { a = 12, b = "11" },
            array = { 2, 3 },
            obj_array = {
                { id = 2, a = 3, b = 4 },
                { id = 3, a = 5, b = 6 },
                { id = 4, a = 70, b = 80 },
            },
        },
        get_id = by_id,
    },
    {
        name = "obj_array_update_only_id_ne_index",
        src = {
            { id = 10, a = 1, b = 2 },
            { id = 20, a = 3, b = 4 },
        },
        dst = {
            { id = 10, a = 100, b = 2 },
            { id = 20, a = 3, b = 4 },
        },
        get_id = by_id,
        check_diff = function(diff)
            if diff.__diff_lua_array ~= true then
                return false, "expected __diff_lua_array"
            end
            if type(diff[10]) ~= "table" or diff[10].a ~= 100 then
                return false, "expected change keyed by id 10"
            end
            if diff[1] ~= nil then
                return false, "should not key the update by array index 1"
            end
            if diff.__diff_lua_del ~= nil then
                return false, "update-only case should have no __diff_lua_del"
            end
            return true
        end,
    },
    {
        name = "obj_array_add_only_id_ne_index",
        src = {
            { id = 10, a = 1 },
        },
        dst = {
            { id = 10, a = 1 },
            { id = 30, a = 3 },
        },
        get_id = by_id,
        check_diff = function(diff)
            if diff.__diff_lua_array ~= true then
                return false, "expected __diff_lua_array"
            end
            if type(diff[30]) ~= "table" or diff[30].id ~= 30 then
                return false, "expected new element keyed by id 30"
            end
            if diff[2] ~= nil then
                return false, "should not key the new element by array index 2"
            end
            return true
        end,
    },
    {
        name = "empty_both",
        src = {},
        dst = {},
        get_id = by_id,
        check_diff = function(diff)
            if next(diff) ~= nil then
                return false, "identical empty tables should yield an empty diff"
            end
            return true
        end,
    },
    {
        name = "empty_to_map",
        src = {},
        dst = { a = 1, b = false },
    },
    {
        name = "map_to_empty",
        src = { a = 1, b = false },
        dst = {},
    },
    {
        name = "false_value_unchanged",
        src = { a = false, b = 1 },
        dst = { a = false, b = 2 },
        check_diff = function(diff)
            if diff.a ~= nil then
                return false, "unchanged false value must not appear in the diff"
            end
            if diff.b ~= 2 then
                return false, "expected b = 2"
            end
            return true
        end,
    },
    {
        name = "false_value_changed",
        src = { a = false },
        dst = { a = true },
    },
    {
        name = "false_key_changed",
        src = { [false] = 1, [true] = 2 },
        dst = { [false] = 3, [true] = 2 },
        check_diff = function(diff)
            if diff[false] ~= 3 then
                return false, "expected false-key to be updated to 3"
            end
            if diff[true] ~= nil then
                return false, "unchanged true-key should not appear"
            end
            return true
        end,
    },
    {
        name = "add_false_value",
        src = { a = 1 },
        dst = { a = 1, b = false },
        check_diff = function(diff)
            if diff.b ~= false then
                return false, "expected b = false to be added"
            end
            return true
        end,
    },
    {
        name = "delete_false_value",
        src = { a = false, b = 1 },
        dst = { b = 1 },
    },
    {
        name = "type_replace_number_to_table",
        src = { a = 1, b = { c = 2 } },
        dst = { a = { c = 2 }, b = { c = 2 } },
    },
    {
        name = "type_replace_table_to_number",
        src = { a = { c = 2 }, b = 3 },
        dst = { a = 9, b = 3 },
    },
    {
        name = "nested_obj_array_update_and_add",
        src = {
            groups = {
                { id = 100, items = { { id = 10, v = 1 }, { id = 20, v = 2 } } },
                { id = 200, items = { { id = 30, v = 3 } } },
            },
        },
        dst = {
            groups = {
                { id = 100, items = { { id = 10, v = 11 }, { id = 20, v = 2 } } },
                { id = 200, items = { { id = 30, v = 3 }, { id = 40, v = 4 } } },
            },
        },
        get_id = by_id,
        check_diff = function(diff)
            local groups = diff.groups
            if type(groups) ~= "table" or groups.__diff_lua_array ~= true then
                return false, "expected groups to be an object-array diff"
            end
            if type(groups[100]) ~= "table" or type(groups[100].items) ~= "table" then
                return false, "expected nested change keyed by group id 100"
            end
            local items = groups[100].items
            if items.__diff_lua_array ~= true or type(items[10]) ~= "table" or items[10].v ~= 11 then
                return false, "expected nested item update keyed by id 10"
            end
            if items[1] ~= nil then
                return false, "nested item update should not be keyed by index 1"
            end
            if type(groups[200]) ~= "table" or type(groups[200].items) ~= "table"
                or type(groups[200].items[40]) ~= "table" then
                return false, "expected nested add keyed by item id 40"
            end
            return true
        end,
    },
    {
        name = "nested_unchanged_obj_array_omitted",
        src = {
            keep = { { id = 10, a = 1 } },
            changed = 1,
        },
        dst = {
            keep = { { id = 10, a = 1 } },
            changed = 2,
        },
        get_id = by_id,
        check_diff = function(diff)
            if diff.keep ~= nil then
                return false, "unchanged nested object array should be omitted from the diff"
            end
            if diff.changed ~= 2 then
                return false, "expected changed = 2"
            end
            return true
        end,
    },
    {
        name = "string_ids_update_only",
        src = { { id = "aa", v = 1 }, { id = "bb", v = 2 } },
        dst = { { id = "aa", v = 9 }, { id = "bb", v = 2 } },
        get_id = by_id,
        check_diff = function(diff)
            if diff.__diff_lua_array ~= true then
                return false, "expected __diff_lua_array"
            end
            if type(diff.aa) ~= "table" or diff.aa.v ~= 9 then
                return false, "expected change keyed by string id 'aa'"
            end
            return true
        end,
    },
    {
        name = "computed_string_ids",
        src = { { id = 10, v = 1 }, { id = 20, v = 2 } },
        dst = { { id = 10, v = 9 }, { id = 20, v = 2 } },
        get_id = computed_id,
        check_diff = function(diff)
            if diff.__diff_lua_array ~= true or type(diff.x10) ~= "table" or diff.x10.v ~= 9 then
                return false, "expected change keyed by computed id 'x10'"
            end
            if diff[10] ~= nil or diff[1] ~= nil then
                return false, "should not key the update by raw id or index"
            end
            return true
        end,
    },
    {
        name = "simple_array_positional_no_get_id",
        src = { 1, 2, 3, 4 },
        dst = { 2, 3 },
    },
    {
        name = "table_of_tables_without_get_id_is_positional",
        src = { { id = 10, a = 1 }, { id = 20, a = 2 } },
        dst = { { id = 20, a = 2 }, { id = 10, a = 1 } },
        check_diff = function(diff)
            if diff.__diff_lua_array then
                return false, "without get_id the table must not be treated as an object array"
            end
            return true
        end,
    },
}

local function run_one(case, diff_fn, patch_fn, label)
    local get_id = case.get_id
    local ok, diff_or_err = pcall(diff_fn, case.src, case.dst, get_id)
    if not ok then
        return false, label .. " diff error: " .. tostring(diff_or_err)
    end
    local diff = diff_or_err
    if case.check_diff then
        local cok, cmsg = case.check_diff(diff)
        if not cok then
            return false, label .. " check_diff failed: " .. tostring(cmsg)
        end
    end
    local src_copy = M.deep_copy(case.src)
    local pok, patch_or_err = pcall(patch_fn, src_copy, diff, get_id)
    if not pok then
        return false, label .. " patch error: " .. tostring(patch_or_err)
    end
    if not M.deep_equal(case.dst, patch_or_err, get_id) then
        return false, label .. " patched result does not match dst"
    end
    return true
end

-- diff_fn/patch_fn have the signature (src, dst_or_diff, get_id).
-- Returns fail_count, total, messages.
function M.run(diff_fn, patch_fn, label)
    label = label or "test"
    local failed = 0
    local messages = {}
    for _, case in ipairs(M.cases) do
        local ok, msg = run_one(case, diff_fn, patch_fn, label .. "/" .. case.name)
        if ok then
            print("OK  " .. label .. "/" .. case.name)
        else
            failed = failed + 1
            messages[#messages + 1] = msg
            print("FAIL  " .. msg)
        end
    end
    return failed, #M.cases, messages
end

function M.run_cross(lua_diff_fn, lua_patch_fn, cpp_diff_fn, cpp_patch_fn)
    local failed = 0
    local total = 0
    local messages = {}
    local combos = {
        { "lua_diff+cpp_patch", lua_diff_fn, cpp_patch_fn },
        { "cpp_diff+lua_patch", cpp_diff_fn, lua_patch_fn },
    }
    for _, combo in ipairs(combos) do
        for _, case in ipairs(M.cases) do
            total = total + 1
            local ok, msg = run_one(case, combo[2], combo[3], combo[1] .. "/" .. case.name)
            if ok then
                print("OK  " .. combo[1] .. "/" .. case.name)
            else
                failed = failed + 1
                messages[#messages + 1] = msg
                print("FAIL  " .. msg)
            end
        end
    end
    return failed, total, messages
end

return M
