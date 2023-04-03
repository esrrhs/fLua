#pragma once

#include <cstdint>
#include <memory>
#include <cstring>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
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

    // 设置为Nil
    virtual DiffVarInterface *DiffSetNil() = 0;

    // 设置为Table
    virtual DiffVarInterface *DiffSetTable() = 0;

    // 设置为string
    virtual DiffVarInterface *DiffSetString(const char *s, size_t len) = 0;

    // 设置为integer
    virtual DiffVarInterface *DiffSetInteger(int64_t i) = 0;

    // 设置为number
    virtual DiffVarInterface *DiffSetNumber(double n) = 0;

    // 设置为boolean
    virtual DiffVarInterface *DiffSetBoolean(bool b) = 0;

    // 设置table的k v
    virtual DiffVarInterface *DiffSetTableKeyValue(DiffVarInterface *k, DiffVarInterface *v) = 0;

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

    virtual DiffVarInterface *DiffSetNil() override;

    virtual DiffVarInterface *DiffSetTable() override;

    virtual DiffVarInterface *DiffSetString(const char *s, size_t len) override;

    virtual DiffVarInterface *DiffSetInteger(int64_t i) override;

    virtual DiffVarInterface *DiffSetNumber(double n) override;

    virtual DiffVarInterface *DiffSetBoolean(bool b) override;

    virtual DiffVarInterface *DiffSetTableKeyValue(DiffVarInterface *k, DiffVarInterface *v) override;

    virtual const char *DiffGetString(size_t &len) override;

    virtual int64_t DiffGetInteger() override;

    virtual double DiffGetNumber() override;

    virtual bool DiffGetBoolean() override;

    virtual DiffVarInterface *DiffGetTableValue(DiffVarInterface *k) override;

    virtual size_t DiffGetTableSize() override;

    class DiffVarTableIterator : public DiffTableIterator {
    public:
        DiffVarTableIterator(
                std::vector<std::pair<DiffVarInterface *, DiffVarInterface *>>::iterator it,
                std::vector<std::pair<DiffVarInterface *, DiffVarInterface *>>::iterator end);

        virtual DiffVarInterface *Key() override;

        virtual DiffVarInterface *Value() override;

        virtual bool Next() override;

    private:
        friend class DiffVar;

        std::vector<std::pair<DiffVarInterface *, DiffVarInterface *>>::iterator m_it;
        std::vector<std::pair<DiffVarInterface *, DiffVarInterface *>>::iterator m_end;
        DiffVarInterface *m_key = nullptr;
        DiffVarInterface *m_value = nullptr;
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
    std::vector<std::pair<DiffVarInterface *, DiffVarInterface *>> m_vec;
};

// 从池子中分配一个DiffVar
extern "C" DiffVar *DiffVarPoolAlloc();

// 释放池子中所有DiffVar
extern "C" void DiffVarPoolReset(DiffVar *var);

typedef std::function<DiffVarInterface *()> DiffVarNewFunc;

typedef std::function<DiffVarInterface *(DiffVarInterface *element)> DiffArrayElementGetIdFunc;

// 计算差分，确保env同时只被一个线程使用
// 返回src->dst的diff
extern "C" DiffVarInterface *
CalDiff(DiffVarInterface *src, DiffVarInterface *dst, DiffArrayElementGetIdFunc get_id_func, DiffVarNewFunc new_func);
