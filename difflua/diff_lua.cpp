#include <vector>
#include "diff_lua.h"

//#define USE_DIFF_LOG

#ifdef USE_DIFF_LOG
#define LOG_ERROR(fmt, ...) do { \
    { \
        char buf[1024] = {0}; \
        snprintf(buf,1024,fmt,##__VA_ARGS__); \
        printf("[%s] %s:%d %s %s\n","ERROR",__FILE__,__LINE__,__FUNCTION__,buf); \
    } \
} while(0)

#define LOG_DEBUG(fmt, ...) do { \
    { \
        char buf[1024] = {0}; \
        snprintf(buf,1024,fmt,##__VA_ARGS__); \
        printf("[%s] %s:%d %s %s\n","DEBUG",__FILE__,__LINE__,__FUNCTION__,buf); \
    } \
} while(0)
#else
#define LOG_ERROR(fmt, ...)
#define LOG_DEBUG(fmt, ...)

#endif

DiffVar::DiffVar() {
}

DiffVar::~DiffVar() {
}

DiffType DiffVar::GetDiffType() {
    return m_type;
}

std::string DiffVar::DiffDump(int tab) {
    std::string ret;

    switch (m_type) {
        case DT_NIL:
            ret = "nil";
            break;
        case DT_INTEGER:
            ret = std::to_string(m_integer);
            break;
        case DT_NUMBER:
            ret = std::to_string(m_number);
            break;
        case DT_BOOLEAN:
            ret = m_boolean ? "true" : "false";
            break;
        case DT_STRING:
            ret = m_string;
            break;
        case DT_TABLE: {
            std::string tabstr;
            for (int i = 0; i < tab; ++i) {
                tabstr += " ";
            }
            ret += "table:\n";
            for (const auto it: m_vec) {
                ret += tabstr + it.first->DiffDump(tab + 4) + "->" + it.second->DiffDump(tab + 4) + "\n";
            }
            break;
        }
        default:
            LOG_ERROR("unknown type %d", m_type);
            ret = "unknown";
    }

    return ret;
}

DiffVarInterface *DiffVar::DiffSetNil() {
    m_type = DT_NIL;
    return this;
}

DiffVarInterface *DiffVar::DiffSetTable() {
    m_type = DT_TABLE;
    m_table.clear();
    m_vec.clear();
    return this;
}

DiffVarInterface *DiffVar::DiffSetString(const char *s, size_t len) {
    m_type = DT_STRING;
    m_string = std::string(s, len);
    return this;
}

DiffVarInterface *DiffVar::DiffSetInteger(int64_t i) {
    m_type = DT_INTEGER;
    m_integer = i;
    return this;
}

DiffVarInterface *DiffVar::DiffSetNumber(double n) {
    m_type = DT_NUMBER;
    m_number = n;
    return this;
}

DiffVarInterface *DiffVar::DiffSetBoolean(bool b) {
    m_type = DT_BOOLEAN;
    m_boolean = b;
    return this;
}

DiffVarInterface *DiffVar::DiffSetTableKeyValue(DiffVarInterface *k, DiffVarInterface *v) {
    if (m_type != DT_TABLE) {
        LOG_ERROR("type is not table");
        return this;
    }

    if (!v || v->GetDiffType() == DT_NIL) {
        auto it = m_table.find(k);
        if (it != m_table.end()) {
            for (auto it = m_vec.begin(); it != m_vec.end(); ++it) {
                if (it->first->DiffEqual(k)) {
                    m_vec.erase(it);
                    break;
                }
            }
            m_table.erase(it);
        }
        return this;
    }

    auto it = m_table.find(k);
    if (it != m_table.end()) {
        it->second = v;
        for (auto it = m_vec.begin(); it != m_vec.end(); ++it) {
            if (it->first->DiffEqual(k)) {
                it->second = v;
                break;
            }
        }
    } else {
        m_table[k] = v;
        m_vec.push_back(std::make_pair(k, v));
    }
    return this;
}

const char *DiffVar::DiffGetString(size_t &len) {
    if (m_type != DT_STRING) {
        LOG_ERROR("type is not string");
        len = 0;
        return "";
    }
    len = m_string.size();
    return m_string.c_str();
}

int64_t DiffVar::DiffGetInteger() {
    if (m_type != DT_INTEGER) {
        LOG_ERROR("type is not integer");
        return 0;
    }
    return m_integer;
}

double DiffVar::DiffGetNumber() {
    if (m_type != DT_NUMBER) {
        LOG_ERROR("type is not number");
        return 0;
    }
    return m_number;
}

bool DiffVar::DiffGetBoolean() {
    if (m_type != DT_BOOLEAN) {
        LOG_ERROR("type is not boolean");
        return false;
    }
    return m_boolean;
}

DiffVarInterface *DiffVar::DiffGetTableValue(DiffVarInterface *k) {
    if (m_type != DT_TABLE) {
        LOG_ERROR("type is not table");
        return nullptr;
    }
    auto it = m_table.find(k);
    if (it == m_table.end()) {
        return nullptr;
    }
    return it->second;
}

size_t DiffVar::DiffGetTableSize() {
    return m_table.size();
}

DiffVar::DiffVarTableIterator::DiffVarTableIterator(
        std::vector<std::pair<DiffVarInterface *, DiffVarInterface *>>::iterator it,
        std::vector<std::pair<DiffVarInterface *, DiffVarInterface *>>::iterator end) {
    m_it = it;
    m_end = end;
}

DiffVarInterface *DiffVar::DiffVarTableIterator::Key() {
    return m_key;
}

DiffVarInterface *DiffVar::DiffVarTableIterator::Value() {
    return m_value;
}

bool DiffVar::DiffVarTableIterator::Next() {
    if (m_it == m_end) {
        return false;
    }
    m_key = m_it->first;
    m_value = m_it->second;
    ++m_it;
    return true;
}

DiffVar::DiffTableIteratorPtr DiffVar::DiffGetTableIterator() {
    if (m_type != DT_TABLE) {
        LOG_ERROR("type is not table");
        return nullptr;
    }
    return std::make_shared<DiffVarTableIterator>(m_vec.begin(), m_vec.end());
}

bool DiffVar::DiffEqual(DiffVarInterface *other) {
    if (m_type != other->GetDiffType()) {
        if (m_type == DT_NUMBER && other->GetDiffType() == DT_INTEGER) {
            return m_number == other->DiffGetInteger();
        }
        if (m_type == DT_INTEGER && other->GetDiffType() == DT_NUMBER) {
            return m_integer == other->DiffGetNumber();
        }
        return false;
    }

    switch (m_type) {
        case DT_NIL:
            return true;
        case DT_STRING: {
            size_t size = 0;
            auto str = other->DiffGetString(size);
            return m_string == std::string(str, size);
        }
        case DT_INTEGER:
            return m_integer == other->DiffGetInteger();
        case DT_NUMBER:
            return m_number == other->DiffGetNumber();
        case DT_BOOLEAN:
            return m_boolean == other->DiffGetBoolean();
        case DT_TABLE: {
            return this == other;
        }
        default:
            LOG_ERROR("unknown type %d", m_type);
            return false;
    }
}

size_t DiffVar::DiffHash() {
    switch (m_type) {
        case DT_STRING:
            return std::hash<std::string>()(m_string);
        case DT_INTEGER:
            return std::hash<int64_t>()(m_integer);
        case DT_NUMBER:
            return std::hash<double>()(m_number);
        case DT_BOOLEAN:
            return std::hash<bool>()(m_boolean);
        case DT_TABLE:
            return std::hash<DiffVar *>()(this);
        default:
            LOG_ERROR("unknown type %d", m_type);
            return 0;
    }
}

static bool IsArrayHasId(DiffVarInterface *arr, DiffArrayElementGetIdFunc get_id_func) {
    int i = 1;
    auto it = arr->DiffGetTableIterator();
    while (it->Next()) {
        auto k = it->Key();
        auto v = it->Value();

        if (k->GetDiffType() != DT_INTEGER) {
            return false;
        }

        if (k->DiffGetInteger() != i) {
            return false;
        }

        if (v->GetDiffType() != DT_TABLE) {
            return false;
        }

        auto id = get_id_func(v);
        if (!id) {
            return false;
        }

        ++i;
    }
    return true;
}

static DiffVarInterface *
ArrayToMap(DiffVarInterface *arr, DiffArrayElementGetIdFunc get_id_func, DiffVarNewFunc new_func) {
    auto ret = new_func()->DiffSetTable();

    auto it = arr->DiffGetTableIterator();
    while (it->Next()) {
        auto v = it->Value();
        auto id = get_id_func(v);
        ret->DiffSetTableKeyValue(id, v);
    }

    return ret;
}

static DiffVarInterface *MapToArray(DiffVarInterface *map, DiffVarNewFunc new_func) {
    auto ret = new_func()->DiffSetTable();

    auto it = map->DiffGetTableIterator();
    int i = 1;
    while (it->Next()) {
        auto v = it->Value();
        ret->DiffSetTableKeyValue(new_func()->DiffSetInteger(i), v);
        ++i;
    }

    return ret;
}

extern "C" DiffVarInterface *
CalDiff(DiffVarInterface *src, DiffVarInterface *dst, DiffArrayElementGetIdFunc get_id_func, DiffVarNewFunc new_func) {
    LOG_DEBUG("start CalDiff src is %s", src->DiffDump().c_str());
    LOG_DEBUG("start CalDiff input is %s", dst->DiffDump().c_str());

    if (src->GetDiffType() != DT_TABLE || dst->GetDiffType() != DT_TABLE) {
        LOG_DEBUG("diff is simple now %s", dst->DiffDump().c_str());
        return dst;
    }

    auto diff = new_func()->DiffSetTable();

    auto del = new_func()->DiffSetTable();

    if (IsArrayHasId(src, get_id_func) && IsArrayHasId(dst, get_id_func)) {
        src = ArrayToMap(src, get_id_func, new_func);
        dst = ArrayToMap(dst, get_id_func, new_func);
        diff->DiffSetTableKeyValue(new_func()->DiffSetString("__diff_lua_array", strlen("__diff_lua_array")),
                                   new_func()->DiffSetBoolean(true));
    }

    auto it = src->DiffGetTableIterator();
    while (it->Next()) {
        auto k = it->Key();
        auto v = it->Value();

        auto dst_v = dst->DiffGetTableValue(k);
        if (!dst_v) {
            del->DiffSetTableKeyValue(new_func()->DiffSetInteger(del->DiffGetTableSize() + 1), k);
        } else {
            if (v->GetDiffType() == DT_TABLE && dst_v->GetDiffType() == DT_TABLE) {
                auto sub_diff = CalDiff(v, dst_v, get_id_func, new_func);
                if (sub_diff->DiffGetTableSize() > 0) {
                    diff->DiffSetTableKeyValue(k, sub_diff);
                }
            } else {
                if (!v->DiffEqual(dst_v)) {
                    diff->DiffSetTableKeyValue(k, dst_v);
                }
            }
        }
    }

    it = dst->DiffGetTableIterator();
    while (it->Next()) {
        auto k = it->Key();
        auto v = it->Value();

        auto src_v = src->DiffGetTableValue(k);
        if (!src_v) {
            diff->DiffSetTableKeyValue(k, v);
        }
    }

    if (del->DiffGetTableSize() > 0) {
        diff->DiffSetTableKeyValue(new_func()->DiffSetString("__diff_lua_del", strlen("__diff_lua_del")), del);
    }

    LOG_DEBUG("end CalDiff output is %s", diff->DiffDump().c_str());

    return diff;
}

extern "C" DiffVarInterface *
PatchDiff(DiffVarInterface *src, DiffVarInterface *diff, DiffArrayElementGetIdFunc get_id_func,
          DiffVarNewFunc new_func) {
    LOG_DEBUG("start PatchDiff src is %s", src->DiffDump().c_str());
    LOG_DEBUG("start PatchDiff input is %s", diff->DiffDump().c_str());

    if (src->GetDiffType() != DT_TABLE || diff->GetDiffType() != DT_TABLE) {
        LOG_DEBUG("dst is simple now %s", diff->DiffDump().c_str());
        return diff;
    }

    auto __diff_lua_array_str = new_func()->DiffSetString("__diff_lua_array", strlen("__diff_lua_array"));
    auto __diff_lua_array = diff->DiffGetTableValue(__diff_lua_array_str);
    if (__diff_lua_array && __diff_lua_array->DiffGetBoolean()) {
        src = ArrayToMap(src, get_id_func, new_func);
    }

    auto __diff_lua_del_str = new_func()->DiffSetString("__diff_lua_del", strlen("__diff_lua_del"));
    auto __diff_lua_del = diff->DiffGetTableValue(__diff_lua_del_str);
    if (__diff_lua_del) {
        auto it = __diff_lua_del->DiffGetTableIterator();
        while (it->Next()) {
            auto k = it->Key();
            auto v = it->Value();
            src->DiffSetTableKeyValue(v, nullptr);
        }
    }

    auto it = diff->DiffGetTableIterator();
    while (it->Next()) {
        auto k = it->Key();
        auto v = it->Value();

        if (k->DiffEqual(__diff_lua_array_str) || k->DiffEqual(__diff_lua_del_str)) {
            continue;
        }

        if (v->GetDiffType() == DT_TABLE) {
            auto src_v = src->DiffGetTableValue(k);
            if (!src_v) {
                src->DiffSetTableKeyValue(k, v);
            } else {
                auto sub = PatchDiff(src_v, v, get_id_func, new_func);
                src->DiffSetTableKeyValue(k, sub);
            }
        } else {
            src->DiffSetTableKeyValue(k, v);
        }
    }

    if (__diff_lua_array && __diff_lua_array->DiffGetBoolean()) {
        src = MapToArray(src, new_func);
    }

    return src;
}

DiffLuaVar::DiffLuaVar() {
}

DiffLuaVar::~DiffLuaVar() {
}

DiffType DiffLuaVar::GetDiffType() {
    return m_type;
}

std::string DiffLuaVar::DiffDump(int tab) {
    std::string ret;

    switch (m_type) {
        case DT_NIL:
            ret = "nil";
            break;
        case DT_INTEGER:
            ret = std::to_string(m_integer);
            break;
        case DT_NUMBER:
            ret = std::to_string(m_number);
            break;
        case DT_BOOLEAN:
            ret = m_boolean ? "true" : "false";
            break;
        case DT_STRING:
            ret = m_string;
            break;
        case DT_TABLE: {
            std::string tabstr;
            for (int i = 0; i < tab; ++i) {
                tabstr += " ";
            }
            ret += "table:\n";
            for (const auto it: m_vec) {
                ret += tabstr + it.first->DiffDump(tab + 4) + "->" + it.second->DiffDump(tab + 4) + "\n";
            }
            break;
        }
        default:
            LOG_ERROR("unknown type %d", m_type);
            ret = "unknown";
    }

    return ret;
}

DiffVarInterface *DiffLuaVar::DiffSetNil() {
    m_type = DT_NIL;
    return this;
}

DiffVarInterface *DiffLuaVar::DiffSetTable() {
    m_type = DT_TABLE;
    m_vec.clear();
    return this;
}

DiffVarInterface *DiffLuaVar::DiffSetString(const char *s, size_t len) {
    m_type = DT_STRING;
    m_string = s;
    m_string_len = len;
    return this;
}

DiffVarInterface *DiffLuaVar::DiffSetInteger(int64_t i) {
    m_type = DT_INTEGER;
    m_integer = i;
    return this;
}

DiffVarInterface *DiffLuaVar::DiffSetNumber(double n) {
    m_type = DT_NUMBER;
    m_number = n;
    return this;
}

DiffVarInterface *DiffLuaVar::DiffSetBoolean(bool b) {
    m_type = DT_BOOLEAN;
    m_boolean = b;
    return this;
}

DiffVarInterface *DiffLuaVar::DiffSetTableKeyValue(DiffVarInterface *k, DiffVarInterface *v) {
    if (m_type != DT_TABLE) {
        LOG_ERROR("type is not table");
        return this;
    }

    if (!v || v->GetDiffType() == DT_NIL) {
        for (auto it = m_vec.begin(); it != m_vec.end(); ++it) {
            if (it->first->DiffEqual(k)) {
                m_vec.erase(it);
                break;
            }
        }
        return this;
    }

    for (auto it = m_vec.begin(); it != m_vec.end(); ++it) {
        if (it->first->DiffEqual(k)) {
            it->second = v;
            return this;
        }
    }

    m_vec.push_back(std::make_pair(k, v));
    return this;
}

const char *DiffLuaVar::DiffGetString(size_t &len) {
    if (m_type != DT_STRING) {
        LOG_ERROR("type is not string");
        len = 0;
        return "";
    }
    len = m_string_len;
    return m_string;
}

int64_t DiffLuaVar::DiffGetInteger() {
    if (m_type != DT_INTEGER) {
        LOG_ERROR("type is not integer");
        return 0;
    }
    return m_integer;
}

double DiffLuaVar::DiffGetNumber() {
    if (m_type != DT_NUMBER) {
        LOG_ERROR("type is not number");
        return 0;
    }
    return m_number;
}

bool DiffLuaVar::DiffGetBoolean() {
    if (m_type != DT_BOOLEAN) {
        LOG_ERROR("type is not boolean");
        return false;
    }
    return m_boolean;
}

DiffVarInterface *DiffLuaVar::DiffGetTableValue(DiffVarInterface *k) {
    if (m_type != DT_TABLE) {
        LOG_ERROR("type is not table");
        return nullptr;
    }
    for (auto it = m_vec.begin(); it != m_vec.end(); ++it) {
        if (it->first->DiffEqual(k)) {
            return it->second;
        }
    }
    return nullptr;
}

size_t DiffLuaVar::DiffGetTableSize() {
    return m_vec.size();
}

DiffLuaVar::DiffLuaVarTableIterator::DiffLuaVarTableIterator(
        std::vector<std::pair<DiffVarInterface *, DiffVarInterface *>>::iterator it,
        std::vector<std::pair<DiffVarInterface *, DiffVarInterface *>>::iterator end) {
    m_it = it;
    m_end = end;
}

DiffVarInterface *DiffLuaVar::DiffLuaVarTableIterator::Key() {
    return m_key;
}

DiffVarInterface *DiffLuaVar::DiffLuaVarTableIterator::Value() {
    return m_value;
}

bool DiffLuaVar::DiffLuaVarTableIterator::Next() {
    if (m_it == m_end) {
        return false;
    }
    m_key = m_it->first;
    m_value = m_it->second;
    ++m_it;
    return true;
}

DiffVar::DiffTableIteratorPtr DiffLuaVar::DiffGetTableIterator() {
    if (m_type != DT_TABLE) {
        LOG_ERROR("type is not table");
        return nullptr;
    }
    return std::make_shared<DiffLuaVarTableIterator>(m_vec.begin(), m_vec.end());
}

bool DiffLuaVar::DiffEqual(DiffVarInterface *other) {
    if (m_type != other->GetDiffType()) {
        if (m_type == DT_NUMBER && other->GetDiffType() == DT_INTEGER) {
            return m_number == other->DiffGetInteger();
        }
        if (m_type == DT_INTEGER && other->GetDiffType() == DT_NUMBER) {
            return m_integer == other->DiffGetNumber();
        }
        return false;
    }

    switch (m_type) {
        case DT_NIL:
            return true;
        case DT_STRING: {
            size_t size = 0;
            auto str = other->DiffGetString(size);
            return m_string_len == size && memcmp(m_string, str, size) == 0;
        }
        case DT_INTEGER:
            return m_integer == other->DiffGetInteger();
        case DT_NUMBER:
            return m_number == other->DiffGetNumber();
        case DT_BOOLEAN:
            return m_boolean == other->DiffGetBoolean();
        case DT_TABLE: {
            return this == other;
        }
        default:
            LOG_ERROR("unknown type %d", m_type);
            return false;
    }
}

size_t DiffLuaVar::DiffHash() {
    switch (m_type) {
        case DT_STRING:
            return std::hash<std::string>()(std::string(m_string, m_string_len));
        case DT_INTEGER:
            return std::hash<int64_t>()(m_integer);
        case DT_NUMBER:
            return std::hash<double>()(m_number);
        case DT_BOOLEAN:
            return std::hash<bool>()(m_boolean);
        case DT_TABLE:
            return std::hash<DiffLuaVar *>()(this);
        default:
            LOG_ERROR("unknown type %d", m_type);
            return 0;
    }
}

extern "C" DiffVarInterface *LuaToDiffVarInterface(lua_State *L, int index, DiffVarNewFunc new_func) {
    switch (lua_type(L, index)) {
        case LUA_TNIL:
            return new_func();
        case LUA_TSTRING: {
            size_t len = 0;
            auto str = lua_tolstring(L, index, &len);
            return new_func()->DiffSetString(str, len);
        }
        case LUA_TNUMBER: {
            auto num = lua_tonumber(L, index);
            if (num == (int64_t) num) {
                return new_func()->DiffSetInteger((int64_t) num);
            } else {
                return new_func()->DiffSetNumber(num);
            }
        }
        case LUA_TBOOLEAN: {
            return new_func()->DiffSetBoolean(lua_toboolean(L, index));
        }
        case LUA_TTABLE: {
            int top = lua_gettop(L);

            if (index < 0 && -index <= top) {
                index = top + index + 1;
            }

            if (!lua_checkstack(L, 1)) {
                LOG_ERROR("lua stack overflow");
                return nullptr;
            }

            auto ret = new_func()->DiffSetTable();
            lua_pushnil(L);
            while (lua_next(L, index) != 0) {
                auto key = LuaToDiffVarInterface(L, -2, new_func);
                auto value = LuaToDiffVarInterface(L, -1, new_func);
                ret->DiffSetTableKeyValue(key, value);
                lua_pop(L, 1);
            }
            return ret;
        }
        default:
            LOG_ERROR("unknown type %d", lua_type(L, index));
            return nullptr;
    }
}

static int DiffVarInterfaceToLua(lua_State *L, DiffVarInterface *src) {
    if (!lua_checkstack(L, 1)) {
        LOG_ERROR("lua stack overflow");
        return -1;
    }

    switch (src->GetDiffType()) {
        case DT_NIL:
            lua_pushnil(L);
            return 0;
        case DT_STRING: {
            size_t size = 0;
            auto str = src->DiffGetString(size);
            lua_pushlstring(L, str, size);
            return 0;
        }
        case DT_INTEGER:
            lua_pushinteger(L, src->DiffGetInteger());
            return 0;
        case DT_NUMBER:
            lua_pushnumber(L, src->DiffGetNumber());
            return 0;
        case DT_BOOLEAN:
            lua_pushboolean(L, src->DiffGetBoolean());
            return 0;
        case DT_TABLE: {
            int top = lua_gettop(L);
            lua_createtable(L, 16, 16);
            auto it = src->DiffGetTableIterator();
            while (it->Next()) {
                if (DiffVarInterfaceToLua(L, it->Key()) != 0) {
                    lua_settop(L, top);
                    return -1;
                }
                if (DiffVarInterfaceToLua(L, it->Value()) != 0) {
                    lua_settop(L, top);
                    return -1;
                }
                lua_settable(L, -3);
            }
            return 0;
        }
        default:
            LOG_ERROR("unknown type %d", src->GetDiffType());
            return -1;
    }
}

// 计算差分，针对Lua的接口
static int LuaCalDiff(lua_State *L) {
    int top = lua_gettop(L);

    auto new_func = []() {
        auto ret = DiffPool<DiffLuaVar>::Instance().Alloc();
        ret->DiffSetNil();
        return ret;
    };

    auto src = LuaToDiffVarInterface(L, 1, new_func);
    if (src == nullptr) {
        LOG_ERROR("src is nil");
        lua_settop(L, top);
        return 0;
    }
    auto dst = LuaToDiffVarInterface(L, 2, new_func);
    if (dst == nullptr) {
        LOG_ERROR("dst is nil");
        lua_settop(L, top);
        return 0;
    }
    lua_CFunction get_id_lua_cfunc = lua_tocfunction(L, 3);
    if (get_id_lua_cfunc == nullptr) {
        LOG_ERROR("get_id_lua_cfunc is nil");
        lua_settop(L, top);
        return 0;
    }

    auto get_id = [=](DiffVarInterface *v) {
        auto top = lua_gettop(L);
        lua_pushlightuserdata(L, v);
        get_id_lua_cfunc(L);
        auto id = (DiffVarInterface *) lua_touserdata(L, -1);
        lua_settop(L, top);
        return id;
    };

    auto diff = CalDiff(src, dst, get_id, new_func);

    if (DiffVarInterfaceToLua(L, diff) != 0) {
        LOG_ERROR("DiffVarInterfaceToLua failed");
        lua_settop(L, top);
        return 0;
    }

    return 1;
}

extern "C" int luaopen_libdifflua(lua_State *L) {
    luaL_Reg l[] = {
            {"cal_diff", LuaCalDiff},
            {nullptr,    nullptr}
    };
    luaL_newlib(L, l);
    return 1;
}
