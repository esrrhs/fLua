#pragma once

#include <cstdint>
#include <memory>
#include <cstring>
#include <string>
#include <unordered_map>
#include "lua.hpp"

enum DiffType {
    DT_NIL,
    DT_INTEGER,
    DT_NUMBER,
    DT_BOOLEAN,
    DT_STRING,
    DT_TABLE,
};

// 用于表达树形结构的变量，不可成环。与lua的变量类型对应
class DiffVarInterface {
public:
    // 获取变量类型
    virtual DiffType GetDiffType() = 0;

    // 打印变量信息，用于调试
    virtual std::string DiffDump(int tab = 0) = 0;

    // 生成一个新的变量nil
    virtual DiffVarInterface *DiffNew() = 0;

    // 设置为Table
    virtual void DiffSetTable() = 0;

    // 设置为string
    virtual void DiffSetString(const char *s, size_t len) = 0;

    // 设置为integer
    virtual void DiffSetInteger(int64_t i) = 0;

    // 设置为number
    virtual void DiffSetNumber(double n) = 0;

    // 设置为boolean
    virtual void DiffSetBoolean(bool b) = 0;

    // 设置table的k v
    virtual void DiffSetTableKeyValue(DiffVarInterface *k, DiffVarInterface *v) = 0;

    // 拿到string
    virtual const char *DiffGetString(size_t &len) = 0;

    // 拿到integer
    virtual int64_t DiffGetInteger() = 0;

    // 拿到number
    virtual double DiffGetNumber() = 0;

    // 拿到boolean
    virtual bool DiffGetBoolean() = 0;

    // 通过k获取table的v
    virtual DiffVarInterface *DiffGetTableValue(DiffVarInterface *k) = 0;

    // 获取kv数量
    virtual size_t DiffGetTableSize() = 0;

    // 迭代器
    class DiffTableIterator {
    public:
        virtual DiffVarInterface *Key() = 0;

        virtual DiffVarInterface *Value() = 0;

        virtual bool Next() = 0;
    };

    typedef std::shared_ptr<DiffTableIterator> DiffTableIteratorPtr;

    // 获取迭代器，用来遍历table
    virtual DiffTableIteratorPtr DiffGetTableIterator() = 0;

    // 判断是否相同，采用Lua的规则
    virtual bool DiffEqual(DiffVarInterface *other) = 0;

    // 获取Hash值
    virtual size_t DiffHash() = 0;
};

// 通用的DiffVar实现，也可以自己实现一种结构来对接
class DiffVar : public DiffVarInterface {
public:
    struct DiffVarInterfacePtrHash {
        size_t operator()(DiffVarInterface *p) const {
            return p->DiffHash();
        }
    };

    struct DiffVarInterfacePtrEqual {
        bool operator()(DiffVarInterface *left, DiffVarInterface *right) const {
            return left->DiffEqual(right);
        }
    };

    DiffVar();

    virtual ~DiffVar();

    virtual DiffType GetDiffType() override;

    virtual std::string DiffDump(int tab = 0) override;

    virtual DiffVarInterface *DiffNew() override;

    virtual void DiffSetTable() override;

    virtual void DiffSetString(const char *s, size_t len) override;

    virtual void DiffSetInteger(int64_t i) override;

    virtual void DiffSetNumber(double n) override;

    virtual void DiffSetBoolean(bool b) override;

    virtual void DiffSetTableKeyValue(DiffVarInterface *k, DiffVarInterface *v) override;

    virtual const char *DiffGetString(size_t &len) override;

    virtual int64_t DiffGetInteger() override;

    virtual double DiffGetNumber() override;

    virtual bool DiffGetBoolean() override;

    virtual DiffVarInterface *DiffGetTableValue(DiffVarInterface *k) override;

    virtual size_t DiffGetTableSize() override;

    class DiffVarTableIterator : public DiffTableIterator {
    public:
        DiffVarTableIterator(
                std::unordered_map<DiffVarInterface *, DiffVarInterface *, DiffVarInterfacePtrHash, DiffVarInterfacePtrEqual>::iterator it,
                std::unordered_map<DiffVarInterface *, DiffVarInterface *, DiffVarInterfacePtrHash, DiffVarInterfacePtrEqual>::iterator end);

        virtual DiffVarInterface *Key() override;

        virtual DiffVarInterface *Value() override;

        virtual bool Next() override;

    private:
        friend class DiffVar;

        std::unordered_map<DiffVarInterface *, DiffVarInterface *, DiffVarInterfacePtrHash, DiffVarInterfacePtrEqual>::iterator m_it;
        std::unordered_map<DiffVarInterface *, DiffVarInterface *, DiffVarInterfacePtrHash, DiffVarInterfacePtrEqual>::iterator m_end;
    };

    virtual DiffTableIteratorPtr DiffGetTableIterator() override;

    virtual bool DiffEqual(DiffVarInterface *other) override;

    virtual size_t DiffHash() override;

private:
    DiffType m_type = DT_NIL;
    std::string m_string;
    int64_t m_integer = 0;
    double m_number = 0;
    bool m_boolean = false;
    std::unordered_map<DiffVarInterface *, DiffVarInterface *, DiffVarInterfacePtrHash, DiffVarInterfacePtrEqual> m_table;
};

// 从池子中分配一个DiffVar
extern "C" DiffVar *DiffVarPoolAlloc();

// 释放池子中所有DiffVar
extern "C" void DiffVarPoolReset(DiffVar *var);

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

// 计算差分，确保env同时只被一个线程使用
// input为输入，diff为输出
extern "C" DiffVarInterface *calc_env_diff(DiffLoggerInterface *log, DiffVarInterface *cur, DiffVarInterface *input);
