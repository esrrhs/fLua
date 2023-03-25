function _G.lua_diff(A, B)
    local diff = { __diff_lua_del = {} }

    for k, v in pairs(A) do
        if type(A[k]) == "function" or type(A[k]) == "userdata" then
            error("table_diff only supports diffs of tables!")
        elseif B[k] ~= nil and type(A[k]) == "table" and type(B[k]) == "table" then
            diff[k] = lua_diff(A[k], B[k])

            if next(diff[k]) == nil then
                diff[k] = nil
            end
        elseif B[k] == nil then
            diff.__diff_lua_del[k] = 0
        elseif B[k] ~= v then
            diff[k] = B[k]
        end
    end

    for k, v in pairs(B) do
        if type(B[k]) == "function" or type(B[k]) == "userdata" then
            error("table_diff only supports diffs of tables!")
        elseif not A[k] then
            diff[k] = v
        end
    end

    if next(diff.__diff_lua_del) == nil then
        diff.__diff_lua_del = nil
    end

    return diff
end

function _G.lua_patch(A, diff)
    if diff.__diff_lua_del ~= nil then
        for k, _ in pairs(diff.__diff_lua_del) do
            A[k] = nil
        end
    end

    for k, v in pairs(diff) do
        if k ~= "__diff_lua_del" then
            if type(v) == "table" then
                A[k] = lua_patch(A[k], v)
            else
                A[k] = v
            end
        end
    end

    return A
end
