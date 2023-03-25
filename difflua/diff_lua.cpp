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

class DiffEnv {
public:
    DiffEnv() {
        m_cur = std::make_shared<DiffVar>();
    }

    virtual ~DiffEnv() {
    }

    DiffVarInterfacePtr GetCur() {
        return m_cur;
    }

    void SetCur(DiffVarInterfacePtr pVar) {
        m_cur = pVar;
    }

private:
    DiffVarInterfacePtr m_cur;
};

extern "C" DiffEnv *new_diff_env() {
    return new DiffEnv();
}

extern "C" void free_diff_env(DiffEnv *env) {
    delete env;
}

extern "C" void reset_env(DiffEnv *env, DiffLoggerInterface *log) {
    if (!env) {
        LOG_ERROR("invalid param");
        return;
    }

    LOG_DEBUG("start reset_env");

    auto cur = env->GetCur();
    LOG_DEBUG("cur is %s", cur->Dump());

    env->SetCur(std::make_shared<DiffVar>());
}

static DiffVarInterfacePtr table_diff(DiffVarInterfacePtr src, DiffVarInterfacePtr dst) {
    auto ret = dst->NewTable();

    auto del = dst->NewTable();

    auto it = src->GetTableIterator();
    while (it->Next()) {
        auto k = it->Key();
        auto v = it->Value();

        auto dst_v = dst->GetTableValue(k);
        if (!dst_v) {
            del->SetTableKeyValue(k, dst->NewInteger(0));
        } else {
            auto v_is_table = v->Type() == DT_TABLE;
            auto dst_v_is_table = dst_v->Type() == DT_TABLE;
            if (v_is_table && dst_v_is_table) {
                auto sub_diff = table_diff(v, dst_v);
                if (sub_diff->GetTableSize() > 0) {
                    ret->SetTableKeyValue(k, sub_diff);
                }
            } else {
                if (!v->Equal(dst_v)) {
                    ret->SetTableKeyValue(k, dst_v);
                }
            }
        }
    }

    it = dst->GetTableIterator();
    while (it->Next()) {
        auto k = it->Key();
        auto v = it->Value();

        auto src_v = src->GetTableValue(k);
        if (!src_v) {
            ret->SetTableKeyValue(k, v);
        }
    }

    if (del->GetTableSize() > 0) {
        ret->SetTableKeyValue(dst->NewString("__diff_lua_del", strlen("__diff_lua_del")), del);
    }

    return ret;
}

extern "C" DiffVarInterfacePtr calc_env_diff(DiffEnv *env, DiffLoggerInterface *log, DiffVarInterfacePtr input) {
    if (!env || !input) {
        LOG_ERROR("invalid param");
        return 0;
    }

    LOG_DEBUG("start calc_env_diff");

    auto cur = env->GetCur();
    LOG_DEBUG("cur is %s", cur->Dump().c_str());

    auto cur_type = cur->Type();
    auto input_type = input->Type();
    auto cur_is_table = cur_type == DT_TABLE;
    auto input_is_table = input_type == DT_TABLE;

    if (cur_is_table && input_is_table) {
        LOG_DEBUG("cur and input are both table, need diff");
        return table_diff(cur, input);
    } else {
        LOG_DEBUG("just simple clone input: %s to cur: %s", input->Dump().c_str(), cur->Dump().c_str());
        auto ret = cur->CloneFrom(input);
        if (ret != 0) {
            LOG_ERROR("clone from input failed %s", input->Dump().c_str());
            return 0;
        }
        LOG_DEBUG("cur and diff is now %s", cur->Dump().c_str());
        return input;
    }
}
