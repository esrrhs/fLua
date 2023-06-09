#include "diff_lua.h"
#include "stdio.h"

bool check_table_same(DiffVarInterface *left, DiffVarInterface *right) {
    if (left->DiffGetTableSize() != right->DiffGetTableSize()) {
        return false;
    }
    auto left_iter = left->DiffGetTableIterator();
    auto right_iter = right->DiffGetTableIterator();
    while (left_iter->Next() && right_iter->Next()) {
        if (left_iter->Key()->GetDiffType() == DiffType::DT_TABLE &&
            right_iter->Key()->GetDiffType() == DiffType::DT_TABLE) {
            if (!check_table_same(left_iter->Key(), right_iter->Key())) {
                return false;
            }
        } else {
            if (!left_iter->Key()->DiffEqual(right_iter->Key())) {
                return false;
            }
        }

        if (left_iter->Value()->GetDiffType() == DiffType::DT_TABLE &&
            right_iter->Value()->GetDiffType() == DiffType::DT_TABLE) {
            if (!check_table_same(left_iter->Value(), right_iter->Value())) {
                return false;
            }
        } else {
            if (!left_iter->Value()->DiffEqual(right_iter->Value())) {
                return false;
            }
        }
    }
    return true;
}

int pure_cpp_test() {
    auto get_id = [](DiffVarInterface *v) {
        return v->DiffGetTableValue(DiffPool<DiffVar>::Instance().Alloc()->DiffSetString("id", 2));
    };

    auto new_func = []() {
        auto ret = DiffPool<DiffVar>::Instance().Alloc();
        ret->DiffSetNil();
        return ret;
    };

    auto src = new_func()->DiffSetTable();
    auto dst = new_func()->DiffSetTable();

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

    /*
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
     */
    dst->DiffSetTableKeyValue(new_func()->DiffSetString("b", 1), new_func()->DiffSetString("2", 1));
    dst->DiffSetTableKeyValue(new_func()->DiffSetString("c", 1), new_func()->DiffSetNumber(3.4));
    dst->DiffSetTableKeyValue(new_func()->DiffSetString("d", 1), new_func()->DiffSetBoolean(false));

    dst->DiffSetTableKeyValue(new_func()->DiffSetString("sub", 3), new_func()->DiffSetTable());
    dst->DiffGetTableValue(new_func()->DiffSetString("sub", 3))->DiffSetTableKeyValue(new_func()->DiffSetString("a", 1),
                                                                                      new_func()->DiffSetInteger(12));
    dst->DiffGetTableValue(new_func()->DiffSetString("sub", 3))->DiffSetTableKeyValue(new_func()->DiffSetString("b", 1),
                                                                                      new_func()->DiffSetString("11",
                                                                                                                2));

    dst->DiffSetTableKeyValue(new_func()->DiffSetString("array", 5), new_func()->DiffSetTable());
    dst->DiffGetTableValue(new_func()->DiffSetString("array", 5))->DiffSetTableKeyValue(new_func()->DiffSetInteger(1),
                                                                                        new_func()->DiffSetInteger(2));
    dst->DiffGetTableValue(new_func()->DiffSetString("array", 5))->DiffSetTableKeyValue(new_func()->DiffSetInteger(2),
                                                                                        new_func()->DiffSetInteger(3));

    dst->DiffSetTableKeyValue(new_func()->DiffSetString("obj_array", 9), new_func()->DiffSetTable());
    dst->DiffGetTableValue(new_func()->DiffSetString("obj_array", 9))->DiffSetTableKeyValue(
            new_func()->DiffSetInteger(1),
            new_func()->DiffSetTable());
    dst->DiffGetTableValue(new_func()->DiffSetString("obj_array", 9))->DiffGetTableValue(
            new_func()->DiffSetInteger(1))->DiffSetTableKeyValue(new_func()->DiffSetString("id", 2),
                                                                 new_func()->DiffSetInteger(2));
    dst->DiffGetTableValue(new_func()->DiffSetString("obj_array", 9))->DiffGetTableValue(
            new_func()->DiffSetInteger(1))->DiffSetTableKeyValue(new_func()->DiffSetString("a", 1),
                                                                 new_func()->DiffSetInteger(3));
    dst->DiffGetTableValue(new_func()->DiffSetString("obj_array", 9))->DiffGetTableValue(
            new_func()->DiffSetInteger(1))->DiffSetTableKeyValue(new_func()->DiffSetString("b", 1),
                                                                 new_func()->DiffSetInteger(4));

    dst->DiffGetTableValue(new_func()->DiffSetString("obj_array", 9))->DiffSetTableKeyValue(
            new_func()->DiffSetInteger(2), new_func()->DiffSetTable());
    dst->DiffGetTableValue(new_func()->DiffSetString("obj_array", 9))->DiffGetTableValue(
            new_func()->DiffSetInteger(2))->DiffSetTableKeyValue(new_func()->DiffSetString("id", 2),
                                                                 new_func()->DiffSetInteger(3));
    dst->DiffGetTableValue(new_func()->DiffSetString("obj_array", 9))->DiffGetTableValue(
            new_func()->DiffSetInteger(2))->DiffSetTableKeyValue(new_func()->DiffSetString("a", 1),
                                                                 new_func()->DiffSetInteger(5));
    dst->DiffGetTableValue(new_func()->DiffSetString("obj_array", 9))->DiffGetTableValue(
            new_func()->DiffSetInteger(2))->DiffSetTableKeyValue(new_func()->DiffSetString("b", 1),
                                                                 new_func()->DiffSetInteger(6));

    dst->DiffGetTableValue(new_func()->DiffSetString("obj_array", 9))->DiffSetTableKeyValue(
            new_func()->DiffSetInteger(3), new_func()->DiffSetTable());
    dst->DiffGetTableValue(new_func()->DiffSetString("obj_array", 9))->DiffGetTableValue(
            new_func()->DiffSetInteger(3))->DiffSetTableKeyValue(new_func()->DiffSetString("id", 2),
                                                                 new_func()->DiffSetInteger(4));
    dst->DiffGetTableValue(new_func()->DiffSetString("obj_array", 9))->DiffGetTableValue(
            new_func()->DiffSetInteger(3))->DiffSetTableKeyValue(new_func()->DiffSetString("a", 1),
                                                                 new_func()->DiffSetInteger(70));
    dst->DiffGetTableValue(new_func()->DiffSetString("obj_array", 9))->DiffGetTableValue(
            new_func()->DiffSetInteger(3))->DiffSetTableKeyValue(new_func()->DiffSetString("b", 1),
                                                                 new_func()->DiffSetInteger(80));

    printf("dst is %s\n", dst->DiffDump(0).c_str());

    auto diff = CalDiff(src, dst, get_id, new_func);
    if (!diff) {
        printf("calc_env_diff failed\n");
        return -1;
    }

    auto diff_str = diff->DiffDump(0);
    printf("diff is %s", diff_str.c_str());

    auto patch = PatchDiff(src, diff, get_id, new_func);
    if (!patch) {
        printf("patch_env_diff failed\n");
        return -1;
    }

    auto patch_str = patch->DiffDump(0);
    printf("patch is %s", patch_str.c_str());

    if (check_table_same(dst, patch)) {
        printf("pure_cpp_test patch success\n");
    } else {
        printf("pure_cpp_test patch failed\n");
        return -1;
    }

    DiffPool<DiffVar>::Instance().Reset();

    return 0;
}

int lua_get_id(lua_State *L) {
    auto v = (DiffVarInterface *) lua_touserdata(L, -1);
    auto id = v->DiffGetTableValue(DiffPool<DiffLuaVar>::Instance().Alloc()->DiffSetString("id", 2));
    lua_pushlightuserdata(L, id);
    return 1;
}

int lua_new_func(lua_State *L) {
    auto v = DiffPool<DiffLuaVar>::Instance().Alloc();
    lua_pushlightuserdata(L, v);
    return 1;
}

int mix_cpp_test() {
    auto L = luaL_newstate();
    luaL_openlibs(L);

    lua_pushcfunction(L, lua_get_id);
    lua_setglobal(L, "lua_get_id");

    lua_pushcfunction(L, lua_new_func);
    lua_setglobal(L, "lua_new_func");

    if (luaL_dofile(L, "mix_cpp_test.lua") != 0) {
        printf("mix_cpp_test luaL_dofile failed %s\n", lua_tostring(L, -1));
        return -1;
    }

    printf("mix_cpp_test success\n");
    lua_close(L);

    return 0;
}

int main(int argc, char *argv[]) {
    if (pure_cpp_test() != 0) {
        return -1;
    }
    if (mix_cpp_test() != 0) {
        return -1;
    }
    return 0;
}
