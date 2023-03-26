local function is_array_has_id(t, get_id)
    local i = 0
    for _ in pairs(t) do
        i = i + 1
        if t[i] == nil then
            return false
        end
        if type(t[i]) ~= "table" then
            return false
        end
        if not get_id(t[i]) then
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

local function map_to_array(t)
    local array = {}
    for _, v in pairs(t) do
        table.insert(array, v)
    end
    return array
end

function _G.lua_diff(A, B, get_id)
    if type(A) ~= "table" or type(B) ~= "table" then
        return B
    end

    local diff = { __diff_lua_del = {} }

    if is_array_has_id(A, get_id) and is_array_has_id(B, get_id) then
        A = array_to_map(A, get_id)
        B = array_to_map(B, get_id)
        diff.__diff_lua_array = true
    end

    for k, v in pairs(A) do
        if type(A[k]) == "function" or type(A[k]) == "userdata" or type(A[k]) == "thread" then
            error("table_diff only supports diffs of tables!")
        elseif B[k] ~= nil and type(A[k]) == "table" and type(B[k]) == "table" then
            diff[k] = lua_diff(A[k], B[k], get_id)

            if next(diff[k]) == nil then
                diff[k] = nil
            end
        elseif B[k] == nil then
            diff.__diff_lua_del[#diff.__diff_lua_del + 1] = k
        elseif B[k] ~= v then
            diff[k] = B[k]
        end
    end

    for k, v in pairs(B) do
        if type(B[k]) == "function" or type(B[k]) == "userdata" or type(B[k]) == "thread" then
            error("table_diff only supports diffs of tables!")
        elseif not A[k] then
            diff[k] = v
        end
    end

    if next(diff.__diff_lua_del) == nil then
        diff.__diff_lua_del = nil
        diff.__diff_lua_array = nil
    end

    return diff
end

function _G.lua_patch(A, diff, get_id)
    if type(A) ~= "table" or type(diff) ~= "table" then
        return diff
    end

    if diff.__diff_lua_array then
        A = array_to_map(A, get_id)
    end

    if diff.__diff_lua_del ~= nil then
        for _, k in pairs(diff.__diff_lua_del) do
            A[k] = nil
        end
    end

    for k, v in pairs(diff) do
        if k ~= "__diff_lua_del" and k ~= "__diff_lua_array" then
            if type(v) == "table" then
                A[k] = lua_patch(A[k], v, get_id)
            else
                A[k] = v
            end
        end
    end

    if diff.__diff_lua_array then
        A = map_to_array(A)
    end

    return A
end
