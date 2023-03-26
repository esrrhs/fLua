require("pure_lua_diff")

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

local function print_table(node)
    if type(node) ~= "table" then
        print(tostring(node))
        return
    end

    local cache, stack, output = {}, {}, {}
    local depth = 1
    local output_str = "{\n"

    while true do
        local size = 0
        for k, v in pairs(node) do
            size = size + 1
        end

        local cur_index = 1
        for k, v in pairs(node) do
            if (cache[node] == nil) or (cur_index >= cache[node]) then

                if (string.find(output_str, "}", output_str:len())) then
                    output_str = output_str .. ",\n"
                elseif not (string.find(output_str, "\n", output_str:len())) then
                    output_str = output_str .. "\n"
                end

                -- This is necessary for working with HUGE tables otherwise we run out of memory using concat on huge strings
                table.insert(output, output_str)
                output_str = ""

                local key
                if (type(k) == "number" or type(k) == "boolean") then
                    key = "[" .. tostring(k) .. "]"
                else
                    key = "['" .. tostring(k) .. "']"
                end

                if (type(v) == "number" or type(v) == "boolean") then
                    output_str = output_str .. string.rep('\t', depth) .. key .. " = " .. tostring(v)
                elseif (type(v) == "table") then
                    output_str = output_str .. string.rep('\t', depth) .. key .. " = {\n"
                    table.insert(stack, node)
                    table.insert(stack, v)
                    cache[node] = cur_index + 1
                    break
                else
                    output_str = output_str .. string.rep('\t', depth) .. key .. " = '" .. tostring(v) .. "'"
                end

                if (cur_index == size) then
                    output_str = output_str .. "\n" .. string.rep('\t', depth - 1) .. "}"
                else
                    output_str = output_str .. ","
                end
            else
                -- close the table
                if (cur_index == size) then
                    output_str = output_str .. "\n" .. string.rep('\t', depth - 1) .. "}"
                end
            end

            cur_index = cur_index + 1
        end

        if (size == 0) then
            output_str = output_str .. "\n" .. string.rep('\t', depth - 1) .. "}"
        end

        if (#stack > 0) then
            node = stack[#stack]
            stack[#stack] = nil
            depth = cache[node] == nil and depth + 1 or depth - 1
        else
            break
        end
    end

    -- This is necessary for working with HUGE tables otherwise we run out of memory using concat on huge strings
    table.insert(output, output_str)
    output_str = table.concat(output)

    print(output_str)
end

local get_id = function(v)
    return v.id
end

local diff = lua_diff(src, dst, get_id)
print_table(diff)

local new_dst = lua_patch(src, diff, get_id)
print_table(new_dst)
