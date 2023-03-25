#pragma once

#include "lua.hpp"

enum DiffType {
    DT_NIL,
    DT_INTEGER,
    DT_NUMBER,
    DT_BOOLEAN,
    DT_STRING,
    DT_TABLE,
};

// 用于表达树形结构的变量，不可成环
class DiffVarInterface {
public:
    // 获取变量类型
    virtual DiffType Type() = 0;

    // 打印变量信息，用于调试
    virtual const char *Dump() = 0;

    // 从另一个变量复制一份
    // 返回0表示成功，否则失败
    virtual int CloneFrom(DiffVarInterface *other) = 0;
};

class DiffEnv;

class DiffLoggerInterface {
public:
    enum LogLevel {
        DEBUG,
        INFO,
        ERROR,
        INVALID,
    };

    virtual LogLevel Level() = 0;

    virtual void Log(LogLevel level, const char *file, int line, const char *func, const char *msg) = 0;
};

// 生成上下文，用于计算差分
extern "C" DiffEnv *new_diff_env();

// 重置上下文，清空内部状态，用于重新计算差分
extern "C" void reset_env(DiffEnv *env, DiffLoggerInterface *log);

// 计算差分，确保env同时只被一个线程使用
// input为输入，diff为输出
// 返回0表示成功，否则失败
extern "C" int calc_env_diff(DiffEnv *env, DiffLoggerInterface *log, DiffVarInterface *input, DiffVarInterface *&diff);

// 释放上下文
extern "C" void free_diff_env(DiffEnv *env);
