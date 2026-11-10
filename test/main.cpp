#include "diff_lua.h"
#include "stdio.h"
#include <cstring>

static DiffVarInterface *MakeTable(const DiffVarNewFunc &n) {
    return n()->DiffSetTable();
}

static DiffVarInterface *MakeInt(const DiffVarNewFunc &n, int64_t v) {
    return n()->DiffSetInteger(v);
}

static DiffVarInterface *MakeNum(const DiffVarNewFunc &n, double v) {
    return n()->DiffSetNumber(v);
}

static DiffVarInterface *MakeStr(const DiffVarNewFunc &n, const char *s) {
    return n()->DiffSetString(s, strlen(s));
}

static DiffVarInterface *MakeBool(const DiffVarNewFunc &n, bool v) {
    return n()->DiffSetBoolean(v);
}

static void SetKV(DiffVarInterface *t, DiffVarInterface *k, DiffVarInterface *v) {
    t->DiffSetTableKeyValue(k, v);
}

static DiffVarInterface *ObjAB(const DiffVarNewFunc &n, int64_t id, int64_t a, int64_t b) {
    auto o = MakeTable(n);
    SetKV(o, MakeStr(n, "id"), MakeInt(n, id));
    SetKV(o, MakeStr(n, "a"), MakeInt(n, a));
    SetKV(o, MakeStr(n, "b"), MakeInt(n, b));
    return o;
}

static DiffVarInterface *ObjV(const DiffVarNewFunc &n, int64_t id, int64_t v) {
    auto o = MakeTable(n);
    SetKV(o, MakeStr(n, "id"), MakeInt(n, id));
    SetKV(o, MakeStr(n, "v"), MakeInt(n, v));
    return o;
}

static void SetIndex(DiffVarInterface *arr, const DiffVarNewFunc &n, int64_t i, DiffVarInterface *v) {
    SetKV(arr, MakeInt(n, i), v);
}

static bool IsArrayHasIdForTest(DiffVarInterface *arr, const DiffArrayElementGetIdFunc &get_id) {
    if (!arr || arr->GetDiffType() != DiffType::DT_TABLE || !get_id) {
        return false;
    }
    int i = 1;
    auto it = arr->DiffGetTableIterator();
    while (it->Next()) {
        auto k = it->Key();
        auto v = it->Value();
        if (k->GetDiffType() != DiffType::DT_INTEGER || k->DiffGetInteger() != i) {
            return false;
        }
        if (v->GetDiffType() != DiffType::DT_TABLE) {
            return false;
        }
        if (!get_id(v)) {
            return false;
        }
        ++i;
    }
    return true;
}

static DiffVarInterface *ArrayToMapForTest(DiffVarInterface *arr, const DiffArrayElementGetIdFunc &get_id,
                                           const DiffVarNewFunc &n) {
    auto ret = MakeTable(n);
    auto it = arr->DiffGetTableIterator();
    while (it->Next()) {
        auto id = get_id(it->Value());
        ret->DiffSetTableKeyValue(id, it->Value());
    }
    return ret;
}

static bool check_value_same(DiffVarInterface *left, DiffVarInterface *right,
                             const DiffArrayElementGetIdFunc &get_id, const DiffVarNewFunc &n);

// Order-independent table compare; object arrays are matched by get_id when possible.
static bool check_table_same(DiffVarInterface *left, DiffVarInterface *right,
                             const DiffArrayElementGetIdFunc &get_id, const DiffVarNewFunc &n) {
    if (left == right) {
        return true;
    }
    if (!left || !right) {
        return false;
    }
    if (left->GetDiffType() != DiffType::DT_TABLE || right->GetDiffType() != DiffType::DT_TABLE) {
        return false;
    }
    if (get_id && IsArrayHasIdForTest(left, get_id) && IsArrayHasIdForTest(right, get_id)) {
        left = ArrayToMapForTest(left, get_id, n);
        right = ArrayToMapForTest(right, get_id, n);
    }
    if (left->DiffGetTableSize() != right->DiffGetTableSize()) {
        return false;
    }
    auto it = left->DiffGetTableIterator();
    while (it->Next()) {
        auto rv = right->DiffGetTableValue(it->Key());
        if (!rv) {
            return false;
        }
        if (!check_value_same(it->Value(), rv, get_id, n)) {
            return false;
        }
    }
    return true;
}

static bool check_value_same(DiffVarInterface *left, DiffVarInterface *right,
                             const DiffArrayElementGetIdFunc &get_id, const DiffVarNewFunc &n) {
    if (left == right) {
        return true;
    }
    if (!left || !right) {
        return false;
    }
    if (left->GetDiffType() == DiffType::DT_TABLE || right->GetDiffType() == DiffType::DT_TABLE) {
        return check_table_same(left, right, get_id, n);
    }
    return left->DiffEqual(right);
}

static int g_failed = 0;

typedef bool (*DiffCheckFn)(DiffVarInterface *diff, const DiffVarNewFunc &n);

static void run_case(const char *name, DiffVarInterface *src, DiffVarInterface *dst,
                     const DiffArrayElementGetIdFunc &get_id, const DiffVarNewFunc &n,
                     DiffCheckFn check_diff = nullptr) {
    auto diff = CalDiff(src, dst, get_id, n);
    if (!diff) {
        printf("FAIL  pure_cpp/%s (CalDiff returned null)\n", name);
        ++g_failed;
        return;
    }
    if (check_diff && !check_diff(diff, n)) {
        printf("FAIL  pure_cpp/%s (diff check)\n", name);
        printf("diff is %s\n", diff->DiffDump().c_str());
        ++g_failed;
        return;
    }
    auto patch = PatchDiff(src, diff, get_id, n);
    if (!patch || !check_table_same(dst, patch, get_id, n)) {
        printf("FAIL  pure_cpp/%s (patch mismatch)\n", name);
        if (dst) {
            printf("dst is %s\n", dst->DiffDump().c_str());
        }
        if (patch) {
            printf("patch is %s\n", patch->DiffDump().c_str());
        }
        ++g_failed;
        return;
    }
    printf("OK  pure_cpp/%s\n", name);
}

static bool check_has_array_flag(DiffVarInterface *diff, const DiffVarNewFunc &n) {
    auto flag = diff->DiffGetTableValue(MakeStr(n, "__diff_lua_array"));
    return flag && flag->GetDiffType() == DiffType::DT_BOOLEAN && flag->DiffGetBoolean();
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

    const DiffVarNewFunc n = new_func;
    const DiffArrayElementGetIdFunc gid = get_id;

    auto src = MakeTable(n);
    SetKV(src, MakeStr(n, "a"), MakeInt(n, 1));
    SetKV(src, MakeStr(n, "b"), MakeStr(n, "2"));
    SetKV(src, MakeStr(n, "c"), MakeNum(n, 3.3));
    SetKV(src, MakeStr(n, "d"), MakeBool(n, true));

    auto src_sub = MakeTable(n);
    SetKV(src, MakeStr(n, "sub"), src_sub);
    SetKV(src_sub, MakeStr(n, "a"), MakeInt(n, 11));
    SetKV(src_sub, MakeStr(n, "b"), MakeStr(n, "22"));
    SetKV(src_sub, MakeStr(n, "c"), MakeNum(n, 33.3));
    SetKV(src_sub, MakeStr(n, "d"), MakeBool(n, true));

    auto src_array = MakeTable(n);
    SetKV(src, MakeStr(n, "array"), src_array);
    SetIndex(src_array, n, 1, MakeInt(n, 1));
    SetIndex(src_array, n, 2, MakeInt(n, 2));
    SetIndex(src_array, n, 3, MakeInt(n, 3));
    SetIndex(src_array, n, 4, MakeInt(n, 4));

    auto src_obj = MakeTable(n);
    SetKV(src, MakeStr(n, "obj_array"), src_obj);
    SetIndex(src_obj, n, 1, ObjAB(n, 1, 1, 2));
    SetIndex(src_obj, n, 2, ObjAB(n, 2, 3, 4));
    SetIndex(src_obj, n, 3, ObjAB(n, 3, 5, 6));
    SetIndex(src_obj, n, 4, ObjAB(n, 4, 7, 8));

    auto dst = MakeTable(n);
    SetKV(dst, MakeStr(n, "b"), MakeStr(n, "2"));
    SetKV(dst, MakeStr(n, "c"), MakeNum(n, 3.4));
    SetKV(dst, MakeStr(n, "d"), MakeBool(n, false));

    auto dst_sub = MakeTable(n);
    SetKV(dst, MakeStr(n, "sub"), dst_sub);
    SetKV(dst_sub, MakeStr(n, "a"), MakeInt(n, 12));
    SetKV(dst_sub, MakeStr(n, "b"), MakeStr(n, "11"));

    auto dst_array = MakeTable(n);
    SetKV(dst, MakeStr(n, "array"), dst_array);
    SetIndex(dst_array, n, 1, MakeInt(n, 2));
    SetIndex(dst_array, n, 2, MakeInt(n, 3));

    auto dst_obj = MakeTable(n);
    SetKV(dst, MakeStr(n, "obj_array"), dst_obj);
    SetIndex(dst_obj, n, 1, ObjAB(n, 2, 3, 4));
    SetIndex(dst_obj, n, 2, ObjAB(n, 3, 5, 6));
    SetIndex(dst_obj, n, 3, ObjAB(n, 4, 70, 80));

    run_case("readme_example", src, dst, gid, n);

    auto u_src = MakeTable(n);
    SetIndex(u_src, n, 1, ObjAB(n, 10, 1, 2));
    SetIndex(u_src, n, 2, ObjAB(n, 20, 3, 4));
    auto u_dst = MakeTable(n);
    SetIndex(u_dst, n, 1, ObjAB(n, 10, 100, 2));
    SetIndex(u_dst, n, 2, ObjAB(n, 20, 3, 4));
    run_case("obj_array_update_only_id_ne_index", u_src, u_dst, gid, n,
             [](DiffVarInterface *diff, const DiffVarNewFunc &nn) {
                 if (!check_has_array_flag(diff, nn)) {
                     return false;
                 }
                 auto ten = diff->DiffGetTableValue(MakeInt(nn, 10));
                 if (!ten || ten->GetDiffType() != DiffType::DT_TABLE) {
                     return false;
                 }
                 auto a = ten->DiffGetTableValue(MakeStr(nn, "a"));
                 if (!a || a->DiffGetInteger() != 100) {
                     return false;
                 }
                 if (diff->DiffGetTableValue(MakeInt(nn, 1))) {
                     return false;
                 }
                 if (diff->DiffGetTableValue(MakeStr(nn, "__diff_lua_del"))) {
                     return false;
                 }
                 return true;
             });

    auto a_src = MakeTable(n);
    SetIndex(a_src, n, 1, ObjAB(n, 10, 1, 0));
    a_src->DiffGetTableValue(MakeInt(n, 1))->DiffSetTableKeyValue(MakeStr(n, "b"), nullptr);
    auto a_dst = MakeTable(n);
    SetIndex(a_dst, n, 1, ObjAB(n, 10, 1, 0));
    a_dst->DiffGetTableValue(MakeInt(n, 1))->DiffSetTableKeyValue(MakeStr(n, "b"), nullptr);
    auto added = MakeTable(n);
    SetKV(added, MakeStr(n, "id"), MakeInt(n, 30));
    SetKV(added, MakeStr(n, "a"), MakeInt(n, 3));
    SetIndex(a_dst, n, 2, added);
    run_case("obj_array_add_only_id_ne_index", a_src, a_dst, gid, n,
             [](DiffVarInterface *diff, const DiffVarNewFunc &nn) {
                 if (!check_has_array_flag(diff, nn)) {
                     return false;
                 }
                 auto thirty = diff->DiffGetTableValue(MakeInt(nn, 30));
                 if (!thirty || thirty->GetDiffType() != DiffType::DT_TABLE) {
                     return false;
                 }
                 if (diff->DiffGetTableValue(MakeInt(nn, 2))) {
                     return false;
                 }
                 return true;
             });

    run_case("empty_both", MakeTable(n), MakeTable(n), gid, n,
             [](DiffVarInterface *diff, const DiffVarNewFunc &) {
                 return diff->DiffGetTableSize() == 0;
             });

    auto e2m_dst = MakeTable(n);
    SetKV(e2m_dst, MakeStr(n, "a"), MakeInt(n, 1));
    SetKV(e2m_dst, MakeStr(n, "b"), MakeBool(n, false));
    run_case("empty_to_map", MakeTable(n), e2m_dst, gid, n);

    auto m2e_src = MakeTable(n);
    SetKV(m2e_src, MakeStr(n, "a"), MakeInt(n, 1));
    SetKV(m2e_src, MakeStr(n, "b"), MakeBool(n, false));
    run_case("map_to_empty", m2e_src, MakeTable(n), gid, n);

    auto fv_src = MakeTable(n);
    SetKV(fv_src, MakeStr(n, "a"), MakeBool(n, false));
    SetKV(fv_src, MakeStr(n, "b"), MakeInt(n, 1));
    auto fv_dst = MakeTable(n);
    SetKV(fv_dst, MakeStr(n, "a"), MakeBool(n, false));
    SetKV(fv_dst, MakeStr(n, "b"), MakeInt(n, 2));
    run_case("false_value_unchanged", fv_src, fv_dst, gid, n,
             [](DiffVarInterface *diff, const DiffVarNewFunc &nn) {
                 if (diff->DiffGetTableValue(MakeStr(nn, "a"))) {
                     return false;
                 }
                 auto b = diff->DiffGetTableValue(MakeStr(nn, "b"));
                 return b && b->GetDiffType() == DiffType::DT_INTEGER && b->DiffGetInteger() == 2;
             });

    auto fc_src = MakeTable(n);
    SetKV(fc_src, MakeStr(n, "a"), MakeBool(n, false));
    auto fc_dst = MakeTable(n);
    SetKV(fc_dst, MakeStr(n, "a"), MakeBool(n, true));
    run_case("false_value_changed", fc_src, fc_dst, gid, n);

    auto fk_src = MakeTable(n);
    SetKV(fk_src, MakeBool(n, false), MakeInt(n, 1));
    SetKV(fk_src, MakeBool(n, true), MakeInt(n, 2));
    auto fk_dst = MakeTable(n);
    SetKV(fk_dst, MakeBool(n, false), MakeInt(n, 3));
    SetKV(fk_dst, MakeBool(n, true), MakeInt(n, 2));
    run_case("false_key_changed", fk_src, fk_dst, gid, n,
             [](DiffVarInterface *diff, const DiffVarNewFunc &nn) {
                 auto fv = diff->DiffGetTableValue(MakeBool(nn, false));
                 if (!fv || fv->DiffGetInteger() != 3) {
                     return false;
                 }
                 return diff->DiffGetTableValue(MakeBool(nn, true)) == nullptr;
             });

    auto af_src = MakeTable(n);
    SetKV(af_src, MakeStr(n, "a"), MakeInt(n, 1));
    auto af_dst = MakeTable(n);
    SetKV(af_dst, MakeStr(n, "a"), MakeInt(n, 1));
    SetKV(af_dst, MakeStr(n, "b"), MakeBool(n, false));
    run_case("add_false_value", af_src, af_dst, gid, n);

    auto df_src = MakeTable(n);
    SetKV(df_src, MakeStr(n, "a"), MakeBool(n, false));
    SetKV(df_src, MakeStr(n, "b"), MakeInt(n, 1));
    auto df_dst = MakeTable(n);
    SetKV(df_dst, MakeStr(n, "b"), MakeInt(n, 1));
    run_case("delete_false_value", df_src, df_dst, gid, n);

    auto tr_src = MakeTable(n);
    SetKV(tr_src, MakeStr(n, "a"), MakeInt(n, 1));
    auto tr_src_b = MakeTable(n);
    SetKV(tr_src_b, MakeStr(n, "c"), MakeInt(n, 2));
    SetKV(tr_src, MakeStr(n, "b"), tr_src_b);
    auto tr_dst = MakeTable(n);
    auto tr_dst_a = MakeTable(n);
    SetKV(tr_dst_a, MakeStr(n, "c"), MakeInt(n, 2));
    SetKV(tr_dst, MakeStr(n, "a"), tr_dst_a);
    auto tr_dst_b = MakeTable(n);
    SetKV(tr_dst_b, MakeStr(n, "c"), MakeInt(n, 2));
    SetKV(tr_dst, MakeStr(n, "b"), tr_dst_b);
    run_case("type_replace_number_to_table", tr_src, tr_dst, gid, n);

    auto tt_src = MakeTable(n);
    auto tt_src_a = MakeTable(n);
    SetKV(tt_src_a, MakeStr(n, "c"), MakeInt(n, 2));
    SetKV(tt_src, MakeStr(n, "a"), tt_src_a);
    SetKV(tt_src, MakeStr(n, "b"), MakeInt(n, 3));
    auto tt_dst = MakeTable(n);
    SetKV(tt_dst, MakeStr(n, "a"), MakeInt(n, 9));
    SetKV(tt_dst, MakeStr(n, "b"), MakeInt(n, 3));
    run_case("type_replace_table_to_number", tt_src, tt_dst, gid, n);

    auto nest_src_g = MakeTable(n);
    auto g100 = MakeTable(n);
    SetKV(g100, MakeStr(n, "id"), MakeInt(n, 100));
    auto g100_items = MakeTable(n);
    SetIndex(g100_items, n, 1, ObjV(n, 10, 1));
    SetIndex(g100_items, n, 2, ObjV(n, 20, 2));
    SetKV(g100, MakeStr(n, "items"), g100_items);
    auto g200 = MakeTable(n);
    SetKV(g200, MakeStr(n, "id"), MakeInt(n, 200));
    auto g200_items = MakeTable(n);
    SetIndex(g200_items, n, 1, ObjV(n, 30, 3));
    SetKV(g200, MakeStr(n, "items"), g200_items);
    SetIndex(nest_src_g, n, 1, g100);
    SetIndex(nest_src_g, n, 2, g200);
    auto nest_src = MakeTable(n);
    SetKV(nest_src, MakeStr(n, "groups"), nest_src_g);

    auto nest_dst_g = MakeTable(n);
    auto d100 = MakeTable(n);
    SetKV(d100, MakeStr(n, "id"), MakeInt(n, 100));
    auto d100_items = MakeTable(n);
    SetIndex(d100_items, n, 1, ObjV(n, 10, 11));
    SetIndex(d100_items, n, 2, ObjV(n, 20, 2));
    SetKV(d100, MakeStr(n, "items"), d100_items);
    auto d200 = MakeTable(n);
    SetKV(d200, MakeStr(n, "id"), MakeInt(n, 200));
    auto d200_items = MakeTable(n);
    SetIndex(d200_items, n, 1, ObjV(n, 30, 3));
    SetIndex(d200_items, n, 2, ObjV(n, 40, 4));
    SetKV(d200, MakeStr(n, "items"), d200_items);
    SetIndex(nest_dst_g, n, 1, d100);
    SetIndex(nest_dst_g, n, 2, d200);
    auto nest_dst = MakeTable(n);
    SetKV(nest_dst, MakeStr(n, "groups"), nest_dst_g);
    run_case("nested_obj_array_update_and_add", nest_src, nest_dst, gid, n);

    auto keep_src_arr = MakeTable(n);
    SetIndex(keep_src_arr, n, 1, ObjAB(n, 10, 1, 0));
    keep_src_arr->DiffGetTableValue(MakeInt(n, 1))->DiffSetTableKeyValue(MakeStr(n, "b"), nullptr);
    auto keep_src = MakeTable(n);
    SetKV(keep_src, MakeStr(n, "keep"), keep_src_arr);
    SetKV(keep_src, MakeStr(n, "changed"), MakeInt(n, 1));
    auto keep_dst_arr = MakeTable(n);
    SetIndex(keep_dst_arr, n, 1, ObjAB(n, 10, 1, 0));
    keep_dst_arr->DiffGetTableValue(MakeInt(n, 1))->DiffSetTableKeyValue(MakeStr(n, "b"), nullptr);
    auto keep_dst = MakeTable(n);
    SetKV(keep_dst, MakeStr(n, "keep"), keep_dst_arr);
    SetKV(keep_dst, MakeStr(n, "changed"), MakeInt(n, 2));
    run_case("nested_unchanged_obj_array_omitted", keep_src, keep_dst, gid, n,
             [](DiffVarInterface *diff, const DiffVarNewFunc &nn) {
                 if (diff->DiffGetTableValue(MakeStr(nn, "keep"))) {
                     return false;
                 }
                 auto c = diff->DiffGetTableValue(MakeStr(nn, "changed"));
                 return c && c->DiffGetInteger() == 2;
             });

    auto pos_src = MakeTable(n);
    SetIndex(pos_src, n, 1, MakeInt(n, 1));
    SetIndex(pos_src, n, 2, MakeInt(n, 2));
    SetIndex(pos_src, n, 3, MakeInt(n, 3));
    SetIndex(pos_src, n, 4, MakeInt(n, 4));
    auto pos_dst = MakeTable(n);
    SetIndex(pos_dst, n, 1, MakeInt(n, 2));
    SetIndex(pos_dst, n, 2, MakeInt(n, 3));
    run_case("simple_array_positional", pos_src, pos_dst, gid, n);

    if (g_failed) {
        printf("pure_cpp_test failed: %d case(s)\n", g_failed);
        DiffPool<DiffVar>::Instance().Reset();
        return -1;
    }

    DiffPool<DiffVar>::Instance().Reset();
    printf("pure_cpp_test success\n");
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
        lua_close(L);
        return -1;
    }

    printf("mix_cpp_test success\n");
    lua_close(L);
    DiffPool<DiffLuaVar>::Instance().Reset();
    return 0;
}

int main(int, char *[]) {
    if (pure_cpp_test() != 0) {
        return -1;
    }
    if (mix_cpp_test() != 0) {
        return -1;
    }
    return 0;
}
