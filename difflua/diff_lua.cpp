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

class DiffVar : public DiffVarInterface {
public:
    DiffVar() {
    }

    virtual ~DiffVar() {
    }

    virtual DiffType Type() {
        return m_type;
    }

    virtual const char *Dump() {
        // TODO
        return "TODO";
    }

    virtual int CloneFrom(DiffVarInterface *other) {
        // TODO
        m_type = other->Type();
        return 0;
    }

private:
    DiffType m_type = DT_NIL;
};

class DiffEnv {
public:
    DiffEnv() {
        m_cur = new DiffVar();
    }

    virtual ~DiffEnv() {
        delete m_cur;
    }

    DiffVar *GetCur() {
        return m_cur;
    }

    void SetCur(DiffVar *pVar) {
        m_cur = pVar;
    }

private:
    DiffVar *m_cur = 0;
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

    delete cur;
    env->SetCur(new DiffVar());
}

extern "C" int calc_env_diff(DiffEnv *env, DiffLoggerInterface *log, DiffVarInterface *input, DiffVarInterface *&diff) {
    diff = 0;

    if (!env || !input) {
        LOG_ERROR("invalid param");
        return -1;
    }

    LOG_DEBUG("start calc_env_diff");

    auto cur = env->GetCur();
    LOG_DEBUG("cur is %s", cur->Dump());

    auto cur_type = cur->Type();
    auto input_type = input->Type();
    auto cur_is_table = cur_type == DT_TABLE;
    auto input_is_table = input_type == DT_TABLE;

    if (cur_is_table && input_is_table) {
        LOG_DEBUG("cur and input are both table, need diff");

        // TODO
    } else {
        LOG_DEBUG("just simple clone input: %s to cur: %s", input->Dump(), cur->Dump());
        auto ret = cur->CloneFrom(input);
        if (ret != 0) {
            LOG_ERROR("clone from input failed %s", input->Dump());
            return -1;
        }
        LOG_DEBUG("cur is now %s", cur->Dump());
        diff = input;
        LOG_DEBUG("diff is %s", diff->Dump());
    }

    return 0;
}
