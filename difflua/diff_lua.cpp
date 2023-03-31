#include <vector>
#include "diff_lua.h"

#define USE_DIFF_LOG

#ifdef USE_DIFF_LOG
#define LOG_ERROR(fmt, ...) do { \
    { \
        char buf[1024] = {0}; \
        snprintf(buf,1024,fmt,##__VA_ARGS__); \
        printf("[%s] %s:%d %s %s","ERROR",__FILE__,__LINE__,__FUNCTION__,buf); \
    } \
} while(0)

#define LOG_DEBUG(fmt, ...) do { \
    { \
        char buf[1024] = {0}; \
        snprintf(buf,1024,fmt,##__VA_ARGS__); \
        printf("[%s] %s:%d %s %s","DEBUG",__FILE__,__LINE__,__FUNCTION__,buf); \
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
            for (const auto it: m_table) {
                ret += "table:\n";
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
    m_table[k] = v;
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
        std::unordered_map<DiffVarInterface *, DiffVarInterface *, DiffVar::DiffVarInterfacePtrHash, DiffVar::DiffVarInterfacePtrEqual>::iterator it,
        std::unordered_map<DiffVarInterface *, DiffVarInterface *, DiffVar::DiffVarInterfacePtrHash, DiffVar::DiffVarInterfacePtrEqual>::iterator end) {
    m_it = it;
    m_end = end;
}

DiffVarInterface *DiffVar::DiffVarTableIterator::Key() {
    return m_it->first;
}

DiffVarInterface *DiffVar::DiffVarTableIterator::Value() {
    return m_it->second;
}

bool DiffVar::DiffVarTableIterator::Next() {
    if (m_it == m_end) {
        return false;
    }
    ++m_it;
    return true;
}

DiffVar::DiffTableIteratorPtr DiffVar::DiffGetTableIterator() {
    return std::make_shared<DiffVarTableIterator>(m_table.begin(), m_table.end());
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

class DiffVarPool {
public:
    DiffVarPool() {
    }

    ~DiffVarPool() {
        for (auto it: m_free) {
            delete it;
        }
        for (auto it: m_used) {
            delete it;
        }
    }

    DiffVar *Alloc() {
        if (m_free.empty()) {
            auto ret = new DiffVar();
            m_used.push_back(ret);
            return ret;
        }
        auto ret = m_free.back();
        m_free.pop_back();
        m_used.push_back(ret);
        return ret;
    }

    void Reset() {
        for (auto it: m_used) {
            m_free.push_back(it);
        }
        m_used.clear();
    }

private:
    std::vector<DiffVar *> m_free;
    std::vector<DiffVar *> m_used;
};

DiffVarPool g_diff_var_pool;

extern "C" DiffVar *DiffVarPoolAlloc() {
    auto ret = g_diff_var_pool.Alloc();
    ret->DiffSetNil();
    return ret;
}

extern "C" void DiffVarPoolReset(DiffVar *var) {
    g_diff_var_pool.Reset();
}

static bool IsArrayHasId(DiffVarInterface *arr, DiffArrayElementGetIdFunc get_id_func) {
    int i = 0;
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
            del->DiffSetTableKeyValue(new_func()->DiffSetInteger(del->DiffGetTableSize() + 1), dst_v);
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
