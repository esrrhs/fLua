#include "diff_lua.h"
#include "stdio.h"

int main(int argc, char *argv[]) {
    auto src = DiffVarPoolAlloc();
    auto input = DiffVarPoolAlloc();

    /*
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
     * */
    src->DiffSetTableKeyValue(src->DiffSetString("a", 1), src->DiffSetInteger(1));
    src->DiffSetTableKeyValue(src->DiffSetString("b", 1), src->DiffSetString("2", 1));
    src->DiffSetTableKeyValue(src->DiffSetString("c", 1), src->DiffSetNumber(3.3));
    src->DiffSetTableKeyValue(src->DiffSetString("d", 1), src->DiffSetBoolean(true));
    src->DiffSetTableKeyValue(src->DiffSetString("sub", 3), src->DiffSetTable());


    auto get_id = [](DiffVarInterface *v) {
        return v->DiffGetTableValue(DiffVarPoolAlloc()->DiffSetString("id", 2));
    };

    auto new_func = []() {
        return DiffVarPoolAlloc();
    };

    auto diff = CalDiff(src, input, get_id, new_func);
    if (!diff) {
        printf("calc_env_diff failed\n");
        return -1;
    }

    auto diff_str = diff->DiffDump(0);
    printf("diff is %s", diff_str.c_str());

    DiffVarPoolReset(src);
    return 0;
}
