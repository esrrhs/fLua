#include <vector>
#include "diff_lua.h"

#define LOG_ERROR(fmt, ...) do { \
    if(log && log->Level() <= DiffLoggerInterface::ERROR) { \
        char buf[1024] = {0}; \
        snprintf(buf,1024,fmt,##__VA_ARGS__); \
        log->Log(DiffLoggerInterface::ERROR,__FILE__,__LINE__,__FUNCTION__,buf); \
    } \
} while(0)

#define LOG_INFO(fmt, ...) do { \
    if(log && log->Level() <= DiffLoggerInterface::INFO) { \
        char buf[1024] = {0}; \
        snprintf(buf,1024,fmt,##__VA_ARGS__); \
        log->Log(DiffLoggerInterface::INFO,__FILE__,__LINE__,__FUNCTION__,buf); \
    } \
} while(0)

#define LOG_DEBUG(fmt, ...) do { \
    if(log && log->Level() <= DiffLoggerInterface::DEBUG) { \
        char buf[1024] = {0}; \
        snprintf(buf,1024,fmt,##__VA_ARGS__); \
        log->Log(DiffLoggerInterface::DEBUG,__FILE__,__LINE__,__FUNCTION__,buf); \
    } \
} while(0)

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
            ret = "unknown";
    }

    return ret;
}

DiffVarInterface *DiffVar::DiffNew() {
    auto ret = DiffVarPoolAlloc();
    ret->m_type = DT_NIL;
    ret->m_integer = 0;
    ret->m_number = 0;
    ret->m_boolean = false;
    ret->m_string.clear();
    ret->m_table.clear();
    return ret;
}

void DiffVar::DiffSetTable() {
    m_type = DT_TABLE;
}

void DiffVar::DiffSetString(const char *s, size_t len) {
    m_type = DT_STRING;
    m_string = std::string(s, len);
}

void DiffVar::DiffSetInteger(int64_t i) {
    m_type = DT_INTEGER;
    m_integer = i;
}

void DiffVar::DiffSetNumber(double n) {
    m_type = DT_NUMBER;
    m_number = n;
}

void DiffVar::DiffSetBoolean(bool b) {
    m_type = DT_BOOLEAN;
    m_boolean = b;
}

void DiffVar::DiffSetTableKeyValue(DiffVarInterface *k, DiffVarInterface *v) {
    m_table[k] = v;
}

const char *DiffVar::DiffGetString(size_t &len) {
    if (m_type != DT_STRING) {
        len = 0;
        return "";
    }
    len = m_string.size();
    return m_string.c_str();
}

int64_t DiffVar::DiffGetInteger() {
    if (m_type != DT_INTEGER) {
        return 0;
    }
    return m_integer;
}

double DiffVar::DiffGetNumber() {
    if (m_type != DT_NUMBER) {
        return 0;
    }
    return m_number;
}

bool DiffVar::DiffGetBoolean() {
    if (m_type != DT_BOOLEAN) {
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
        default:
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
    return g_diff_var_pool.Alloc();
}

extern "C" void DiffVarPoolReset(DiffVar *var) {
    g_diff_var_pool.Reset();
}

static DiffVarInterface *table_diff(DiffVarInterface *src, DiffVarInterface *dst) {
    auto ret = dst->DiffNew();
    ret->DiffSetTable();

    auto del = dst->DiffNew();
    del->DiffSetTable();

    auto it = src->DiffGetTableIterator();
    while (it->Next()) {
        auto k = it->Key();
        auto v = it->Value();

        auto dst_v = dst->DiffGetTableValue(k);
        if (!dst_v) {
            auto dst_v = dst->DiffNew();
            dst_v->DiffSetInteger(0);
            del->DiffSetTableKeyValue(k, dst_v);
        } else {
            auto v_is_table = v->GetDiffType() == DT_TABLE;
            auto dst_v_is_table = dst_v->GetDiffType() == DT_TABLE;
            if (v_is_table && dst_v_is_table) {
                auto sub_diff = table_diff(v, dst_v);
                if (sub_diff->DiffGetTableSize() > 0) {
                    ret->DiffSetTableKeyValue(k, sub_diff);
                }
            } else {
                if (!v->DiffEqual(dst_v)) {
                    ret->DiffSetTableKeyValue(k, dst_v);
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
            ret->DiffSetTableKeyValue(k, v);
        }
    }

    if (del->DiffGetTableSize() > 0) {
        auto k = dst->DiffNew();
        k->DiffSetString("__diff_lua_del", strlen("__diff_lua_del"));
        ret->DiffSetTableKeyValue(k, del);
    }

    return ret;
}

extern "C" DiffVarInterface *calc_env_diff(DiffLoggerInterface *log, DiffVarInterface *src, DiffVarInterface *input) {
    if (!src || !input) {
        LOG_ERROR("invalid param");
        return 0;
    }

    LOG_DEBUG("start calc_env_diff src is %s", src->DiffDump().c_str());

    auto cur_type = src->GetDiffType();
    auto input_type = input->GetDiffType();
    auto cur_is_table = cur_type == DT_TABLE;
    auto input_is_table = input_type == DT_TABLE;

    if (cur_is_table && input_is_table) {
        LOG_DEBUG("src and input are both table, need diff");
        return table_diff(src, input);
    } else {
        LOG_DEBUG("diff is simple now %s", input->DiffDump().c_str());
        return input;
    }
}
