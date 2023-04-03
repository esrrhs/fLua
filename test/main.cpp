#include "diff_lua.h"
#include "stdio.h"

int main(int argc, char *argv[]) {

    auto get_id = [](DiffVarInterface *v) {
        return v->DiffGetTableValue(DiffVarPoolAlloc()->DiffSetString("id", 2));
    };

    auto new_func = []() {
        return DiffVarPoolAlloc();
    };

    auto src = new_func()->DiffSetTable();
    auto input = new_func()->DiffSetTable();

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
    src->DiffSetTableKeyValue(new_func()->DiffSetString("a", 1), new_func()->DiffSetInteger(1));
    src->DiffSetTableKeyValue(new_func()->DiffSetString("b", 1), new_func()->DiffSetString("2", 1));
    src->DiffSetTableKeyValue(new_func()->DiffSetString("c", 1), new_func()->DiffSetNumber(3.3));
    src->DiffSetTableKeyValue(new_func()->DiffSetString("d", 1), new_func()->DiffSetBoolean(true));

    src->DiffSetTableKeyValue(new_func()->DiffSetString("sub", 3), new_func()->DiffSetTable());
    src->DiffGetTableValue(new_func()->DiffSetString("sub", 3))->DiffSetTableKeyValue(new_func()->DiffSetString("a", 1),
                                                                                      new_func()->DiffSetInteger(11));
    src->DiffGetTableValue(new_func()->DiffSetString("sub", 3))->DiffSetTableKeyValue(new_func()->DiffSetString("b", 1),
                                                                                      new_func()->DiffSetString("22",
                                                                                                                2));
    src->DiffGetTableValue(new_func()->DiffSetString("sub", 3))->DiffSetTableKeyValue(new_func()->DiffSetString("c", 1),
                                                                                      new_func()->DiffSetNumber(33.3));
    src->DiffGetTableValue(new_func()->DiffSetString("sub", 3))->DiffSetTableKeyValue(new_func()->DiffSetString("d", 1),
                                                                                      new_func()->DiffSetBoolean(true));

    src->DiffSetTableKeyValue(new_func()->DiffSetString("array", 5), new_func()->DiffSetTable());
    src->DiffGetTableValue(new_func()->DiffSetString("array", 5))->DiffSetTableKeyValue(new_func()->DiffSetInteger(1),
                                                                                        new_func()->DiffSetInteger(1));
    src->DiffGetTableValue(new_func()->DiffSetString("array", 5))->DiffSetTableKeyValue(new_func()->DiffSetInteger(2),
                                                                                        new_func()->DiffSetInteger(2));
    src->DiffGetTableValue(new_func()->DiffSetString("array", 5))->DiffSetTableKeyValue(new_func()->DiffSetInteger(3),
                                                                                        new_func()->DiffSetInteger(3));
    src->DiffGetTableValue(new_func()->DiffSetString("array", 5))->DiffSetTableKeyValue(new_func()->DiffSetInteger(4),
                                                                                        new_func()->DiffSetInteger(4));

    src->DiffSetTableKeyValue(new_func()->DiffSetString("obj_array", 9), new_func()->DiffSetTable());
    src->DiffGetTableValue(new_func()->DiffSetString("obj_array", 9))->DiffSetTableKeyValue(
            new_func()->DiffSetInteger(1), new_func()->DiffSetTable());
    src->DiffGetTableValue(new_func()->DiffSetString("obj_array", 9))->DiffGetTableValue(
            new_func()->DiffSetInteger(1))->DiffSetTableKeyValue(new_func()->DiffSetString("id", 2),
                                                                 new_func()->DiffSetInteger(1));
    src->DiffGetTableValue(new_func()->DiffSetString("obj_array", 9))->DiffGetTableValue(
            new_func()->DiffSetInteger(1))->DiffSetTableKeyValue(new_func()->DiffSetString("a", 1),
                                                                 new_func()->DiffSetInteger(1));
    src->DiffGetTableValue(new_func()->DiffSetString("obj_array", 9))->DiffGetTableValue(
            new_func()->DiffSetInteger(1))->DiffSetTableKeyValue(new_func()->DiffSetString("b", 1),
                                                                 new_func()->DiffSetInteger(2));

    src->DiffGetTableValue(new_func()->DiffSetString("obj_array", 9))->DiffSetTableKeyValue(
            new_func()->DiffSetInteger(2), new_func()->DiffSetTable());
    src->DiffGetTableValue(new_func()->DiffSetString("obj_array", 9))->DiffGetTableValue(
            new_func()->DiffSetInteger(2))->DiffSetTableKeyValue(new_func()->DiffSetString("id", 2),
                                                                 new_func()->DiffSetInteger(2));
    src->DiffGetTableValue(new_func()->DiffSetString("obj_array", 9))->DiffGetTableValue(
            new_func()->DiffSetInteger(2))->DiffSetTableKeyValue(new_func()->DiffSetString("a", 1),
                                                                 new_func()->DiffSetInteger(3));
    src->DiffGetTableValue(new_func()->DiffSetString("obj_array", 9))->DiffGetTableValue(
            new_func()->DiffSetInteger(2))->DiffSetTableKeyValue(new_func()->DiffSetString("b", 1),
                                                                 new_func()->DiffSetInteger(4));

    src->DiffGetTableValue(new_func()->DiffSetString("obj_array", 9))->DiffSetTableKeyValue(
            new_func()->DiffSetInteger(3), new_func()->DiffSetTable());
    src->DiffGetTableValue(new_func()->DiffSetString("obj_array", 9))->DiffGetTableValue(
            new_func()->DiffSetInteger(3))->DiffSetTableKeyValue(new_func()->DiffSetString("id", 2),
                                                                 new_func()->DiffSetInteger(3));
    src->DiffGetTableValue(new_func()->DiffSetString("obj_array", 9))->DiffGetTableValue(
            new_func()->DiffSetInteger(3))->DiffSetTableKeyValue(new_func()->DiffSetString("a", 1),
                                                                 new_func()->DiffSetInteger(5));
    src->DiffGetTableValue(new_func()->DiffSetString("obj_array", 9))->DiffGetTableValue(
            new_func()->DiffSetInteger(3))->DiffSetTableKeyValue(new_func()->DiffSetString("b", 1),
                                                                 new_func()->DiffSetInteger(6));

    src->DiffGetTableValue(new_func()->DiffSetString("obj_array", 9))->DiffSetTableKeyValue(
            new_func()->DiffSetInteger(4), new_func()->DiffSetTable());
    src->DiffGetTableValue(new_func()->DiffSetString("obj_array", 9))->DiffGetTableValue(
            new_func()->DiffSetInteger(4))->DiffSetTableKeyValue(new_func()->DiffSetString("id", 2),
                                                                 new_func()->DiffSetInteger(4));
    src->DiffGetTableValue(new_func()->DiffSetString("obj_array", 9))->DiffGetTableValue(
            new_func()->DiffSetInteger(4))->DiffSetTableKeyValue(new_func()->DiffSetString("a", 1),
                                                                 new_func()->DiffSetInteger(7));
    src->DiffGetTableValue(new_func()->DiffSetString("obj_array", 9))->DiffGetTableValue(
            new_func()->DiffSetInteger(4))->DiffSetTableKeyValue(new_func()->DiffSetString("b", 1),
                                                                 new_func()->DiffSetInteger(8));

    printf("src is %s\n", src->DiffDump(0).c_str());

    auto diff = CalDiff(src, input, get_id, new_func);
    if (!diff) {
        printf("calc_env_diff failed\n");
        return -1;
    }

    auto diff_str = diff->DiffDump(0);
    printf("diff is %s", diff_str.c_str());

    return 0;
}
