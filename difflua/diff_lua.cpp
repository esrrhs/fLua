#include <vector>
#include <stdexcept>
#include <string>
#include <cstdio>
#include <cstring>
#include <utility>
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

    DiffVarInterface *array_flag_key = nullptr;
    if (IsArrayHasId(src, get_id_func) && IsArrayHasId(dst, get_id_func)) {
        src = ArrayToMap(src, get_id_func, new_func);
        dst = ArrayToMap(dst, get_id_func, new_func);
        array_flag_key = new_func()->DiffSetString("__diff_lua_array", strlen("__diff_lua_array"));
        diff->DiffSetTableKeyValue(array_flag_key, new_func()->DiffSetBoolean(true));
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
    } else if (array_flag_key && diff->DiffGetTableSize() == 1) {
        // Identical object arrays should produce an empty diff (no leftover flag).
        diff->DiffSetTableKeyValue(array_flag_key, nullptr);
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

struct LuaDiffError : std::exception {
    std::string msg;

    explicit LuaDiffError(std::string m) : msg(std::move(m)) {}

    const char *what() const noexcept override {
        return msg.c_str();
    }
};

static const char *LuaTypeName(lua_State *L, int index) {
    return lua_typename(L, lua_type(L, index));
}

static DiffVarInterface *LuaToDiffVarInterface(lua_State *L, int index, const DiffVarNewFunc &new_func);

static int DiffVarInterfaceToLua(lua_State *L, DiffVarInterface *src) {
    if (!lua_checkstack(L, 1)) {
        throw LuaDiffError("Lua stack overflow");
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
                    throw LuaDiffError("failed to convert diff key to Lua");
                }
                if (DiffVarInterfaceToLua(L, it->Value()) != 0) {
                    lua_settop(L, top);
                    throw LuaDiffError("failed to convert diff value to Lua");
                }
                lua_settable(L, -3);
            }
            return 0;
        }
        default:
            throw LuaDiffError("unknown diff value type");
    }
}

static DiffVarInterface *LuaToDiffVarInterface(lua_State *L, int index, const DiffVarNewFunc &new_func) {
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
                throw LuaDiffError("Lua stack overflow");
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
            throw LuaDiffError(std::string("unsupported Lua type '") + LuaTypeName(L, index) +
                               "'; only nil, boolean, number, string, and table are supported");
    }
}

// Session for one cal_diff / patch_diff call.
// - Recommended: cal_diff(src, dst [, get_id]) with a Lua get_id and a call-local allocator.
// - Compatible: cal_diff(src, dst, c_get_id, c_new_func) where both are C functions
//   (c_get_id takes lightuserdata DiffVarInterface* and returns lightuserdata).
class LuaDiffSession {
public:
    lua_State *L;
    DiffPool<DiffLuaVar> default_pool;
    bool legacy = false;
    lua_CFunction legacy_get_id = nullptr;
    lua_CFunction legacy_new_func = nullptr;
    int get_id_abs = 0;
    int anchor_abs = 0;
    int next_anchor = 1;

    explicit LuaDiffSession(lua_State *L_) : L(L_) {}

    DiffVarInterface *NewVar() {
        if (legacy) {
            int top = lua_gettop(L);
            if (!lua_checkstack(L, 2)) {
                throw LuaDiffError("Lua stack overflow");
            }
            lua_pushcfunction(L, legacy_new_func);
            if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
                std::string msg = "new_func failed";
                if (lua_isstring(L, -1)) {
                    msg = std::string("new_func failed: ") + lua_tostring(L, -1);
                }
                lua_settop(L, top);
                throw LuaDiffError(std::move(msg));
            }
            auto ret = (DiffVarInterface *) lua_touserdata(L, -1);
            lua_settop(L, top);
            if (!ret) {
                throw LuaDiffError("new_func returned nil");
            }
            return ret;
        }
        auto v = default_pool.Alloc();
        v->DiffSetNil();
        return v;
    }

    DiffVarInterface *GetId(DiffVarInterface *v) {
        if (legacy) {
            int top = lua_gettop(L);
            if (!lua_checkstack(L, 3)) {
                throw LuaDiffError("Lua stack overflow");
            }
            lua_pushcfunction(L, legacy_get_id);
            lua_pushlightuserdata(L, v);
            if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
                std::string msg = "get_id failed";
                if (lua_isstring(L, -1)) {
                    msg = std::string("get_id failed: ") + lua_tostring(L, -1);
                }
                lua_settop(L, top);
                throw LuaDiffError(std::move(msg));
            }
            if (lua_isnoneornil(L, -1)) {
                lua_settop(L, top);
                return nullptr;
            }
            auto id = (DiffVarInterface *) lua_touserdata(L, -1);
            lua_settop(L, top);
            return id;
        }

        if (get_id_abs == 0) {
            return nullptr;
        }

        int top = lua_gettop(L);
        if (!lua_checkstack(L, 3)) {
            throw LuaDiffError("Lua stack overflow");
        }
        lua_pushvalue(L, get_id_abs);
        DiffVarInterfaceToLua(L, v);
        if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
            std::string msg = "get_id failed";
            if (lua_isstring(L, -1)) {
                msg = std::string("get_id failed: ") + lua_tostring(L, -1);
            }
            lua_settop(L, top);
            throw LuaDiffError(std::move(msg));
        }
        if (lua_isnoneornil(L, -1)) {
            lua_settop(L, top);
            return nullptr;
        }
        // Pin the returned id so string/table ids stay alive for this call.
        if (anchor_abs != 0) {
            lua_pushvalue(L, -1);
            lua_rawseti(L, anchor_abs, next_anchor++);
        }
        auto id = LuaToDiffVarInterface(L, -1, [this]() { return NewVar(); });
        lua_settop(L, top);
        return id;
    }

    DiffVarNewFunc NewFunc() {
        return [this]() { return NewVar(); };
    }

    DiffArrayElementGetIdFunc IdFunc() {
        return [this](DiffVarInterface *element) { return GetId(element); };
    }
};

// Parse args: (src, dst_or_diff [, get_id [, new_func]])
// 4-arg form with two C functions is the legacy embedder API.
static void ParseLuaDiffArgs(lua_State *L, const char *api, LuaDiffSession &session) {
    int argc = lua_gettop(L);
    if (argc < 2) {
        throw LuaDiffError(std::string(api) + " requires at least 2 arguments (src, dst_or_diff [, get_id])");
    }

    if (argc >= 4 && lua_iscfunction(L, 3) && lua_iscfunction(L, 4)) {
        session.legacy = true;
        session.legacy_get_id = lua_tocfunction(L, 3);
        session.legacy_new_func = lua_tocfunction(L, 4);
        if (!session.legacy_get_id || !session.legacy_new_func) {
            throw LuaDiffError(std::string(api) +
                               " legacy form requires C functions for get_id and new_func");
        }
        return;
    }

    if (argc >= 4 && !lua_isnoneornil(L, 4)) {
        throw LuaDiffError(std::string(api) +
                           " 4-argument form is only for legacy C callbacks; omit new_func and pass a Lua get_id");
    }

    if (argc >= 3 && !lua_isnoneornil(L, 3)) {
        if (!lua_isfunction(L, 3)) {
            throw LuaDiffError(std::string(api) + " get_id must be a function, or omitted");
        }
        session.get_id_abs = lua_absindex(L, 3);
    }
}

static int LuaDiffFinish(lua_State *L, LuaDiffSession &session, DiffVarInterface *result, const char *api) {
    if (!result) {
        throw LuaDiffError(std::string(api) + " returned nil");
    }
    DiffVarInterfaceToLua(L, result);
    if (session.anchor_abs != 0) {
        lua_remove(L, session.anchor_abs);
        session.anchor_abs = 0;
    }
    return 1;
}

static int LuaDiffExec(lua_State *L, const char *api, bool patch) {
    char errbuf[1024];
    errbuf[0] = '\0';
    int nresults = -1;
    try {
        LuaDiffSession session(L);
        ParseLuaDiffArgs(L, api, session);
        lua_newtable(L);
        session.anchor_abs = lua_gettop(L);

        auto src = LuaToDiffVarInterface(L, 1, session.NewFunc());
        auto second = LuaToDiffVarInterface(L, 2, session.NewFunc());
        DiffVarInterface *result = nullptr;
        if (patch) {
            result = PatchDiff(src, second, session.IdFunc(), session.NewFunc());
        } else {
            result = CalDiff(src, second, session.IdFunc(), session.NewFunc());
        }
        nresults = LuaDiffFinish(L, session, result, api);
    } catch (const LuaDiffError &e) {
        snprintf(errbuf, sizeof(errbuf), "%s", e.msg.c_str());
    } catch (const std::exception &e) {
        snprintf(errbuf, sizeof(errbuf), "%s failed: %s", api, e.what());
    }
    // luaL_error may longjmp; only POD remains in this frame.
    if (nresults < 0) {
        return luaL_error(L, "%s", errbuf);
    }
    return nresults;
}

extern "C" int LuaCalDiff(lua_State *L) {
    return LuaDiffExec(L, "libdifflua.cal_diff", false);
}

extern "C" int LuaPatchDiff(lua_State *L) {
    return LuaDiffExec(L, "libdifflua.patch_diff", true);
}

extern "C" int luaopen_libdifflua(lua_State *L) {
    luaL_Reg l[] = {
            {"cal_diff",   LuaCalDiff},
            {"patch_diff", LuaPatchDiff},
            {nullptr,      nullptr}
    };
    luaL_newlib(L, l);
    return 1;
}
