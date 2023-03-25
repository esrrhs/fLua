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

class DiffVarInterface;

typedef std::shared_ptr<DiffVarInterface> DiffVarInterfacePtr;

// 用于表达树形结构的变量，不可成环
class DiffVarInterface {
public:
    // 获取变量类型
    virtual DiffType Type() = 0;

    // 打印变量信息，用于调试
    virtual std::string Dump(int tab = 0) = 0;

    // 从另一个变量复制一份
    // 返回0表示成功，否则失败
    virtual int CloneFrom(DiffVarInterfacePtr other) = 0;

    // 生成一个新的变量Table
    virtual DiffVarInterfacePtr NewTable() = 0;

    // 生成一个新的变量string
    virtual DiffVarInterfacePtr NewString(const char *s, size_t len) = 0;

    // 生成一个新的变量integer
    virtual DiffVarInterfacePtr NewInteger(int64_t i) = 0;

    // 生成一个新的变量number
    virtual DiffVarInterfacePtr NewNumber(double n) = 0;

    // 生成一个新的变量boolean
    virtual DiffVarInterfacePtr NewBoolean(bool b) = 0;

    // 迭代器
    class TableIterator {
    public:
        virtual DiffVarInterfacePtr Key() = 0;

        virtual DiffVarInterfacePtr Value() = 0;

        virtual bool Next() = 0;
    };

    typedef std::shared_ptr<TableIterator> TableIteratorPtr;

    // 设置table的k v
    virtual void SetTableKeyValue(DiffVarInterfacePtr k, DiffVarInterfacePtr v) = 0;

    // 通过k获取table的v
    virtual DiffVarInterfacePtr GetTableValue(DiffVarInterfacePtr k) = 0;

    // 获取kv数量
    virtual size_t GetTableSize() = 0;

    // 获取迭代器，用来遍历table
    virtual TableIteratorPtr GetTableIterator() = 0;

    // 判断是否相同
    virtual bool Equal(DiffVarInterfacePtr other) = 0;

    // 获取Hash值
    virtual size_t Hash() = 0;
};

// 通用的DiffVar实现，也可以自己实现一种结构来对接
class DiffVar : public DiffVarInterface {
public:
    DiffVar() {
    }

    virtual ~DiffVar() {
    }

    virtual DiffType Type() override {
        return m_type;
    }

    virtual std::string Dump(int tab = 0) override {
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
            case DT_TABLE:
                std::string tabstr;
                for (int i = 0; i < tab; ++i) {
                    tabstr += " ";
                }
                for (const auto it: m_table) {
                    ret += "table:\n";
                    ret += tabstr + it.first->Dump(tab + 4) + "->" + it.second->Dump(tab + 4) + "\n";
                }
                break;
        }
        return ret;
    }

    virtual int CloneFrom(DiffVarInterfacePtr other) override {
        // TODO
        m_type = other->Type();
        return 0;
    }

    virtual DiffVarInterfacePtr NewTable() override {
        auto ret = std::make_shared<DiffVar>();
        ret->m_type = DT_TABLE;
        return ret;
    }

    // 生成一个新的变量string
    virtual DiffVarInterfacePtr NewString(const char *s, size_t len) override {
        auto ret = std::make_shared<DiffVar>();
        ret->m_type = DT_STRING;
        ret->m_string.assign(s, len);
        return ret;
    }

    // 生成一个新的变量integer
    virtual DiffVarInterfacePtr NewInteger(int64_t i) override {
        auto ret = std::make_shared<DiffVar>();
        ret->m_type = DT_INTEGER;
        ret->m_integer = i;
        return ret;
    }

    // 生成一个新的变量number
    virtual DiffVarInterfacePtr NewNumber(double n) override {
        auto ret = std::make_shared<DiffVar>();
        ret->m_type = DT_NUMBER;
        ret->m_number = n;
        return ret;
    }

    // 生成一个新的变量boolean
    virtual DiffVarInterfacePtr NewBoolean(bool b) override {
        auto ret = std::make_shared<DiffVar>();
        ret->m_type = DT_BOOLEAN;
        ret->m_boolean = b;
        return ret;
    }

    // 迭代器
    class DiffVarTableIterator : public TableIterator {
    public:
        virtual DiffVarInterfacePtr Key() override {

        }

        virtual DiffVarInterfacePtr Value() override {

        }

        virtual bool Next() override {

        }
    };

    typedef std::shared_ptr<DiffVarTableIterator> DiffVarTableIteratorPtr;

    // 设置table的k v
    virtual void SetTableKeyValue(DiffVarInterfacePtr k, DiffVarInterfacePtr v) override {

    }

    // 通过k获取table的v
    virtual DiffVarInterfacePtr GetTableValue(DiffVarInterfacePtr k) override {

    }

    // 获取kv数量
    virtual size_t GetTableSize() override {

    }

    // 获取迭代器，用来遍历table
    virtual TableIteratorPtr GetTableIterator() override {

    }

    // 判断是否相同
    virtual bool Equal(DiffVarInterfacePtr other) override {

    }

    virtual size_t Hash() override {
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


    struct DiffVarInterfacePtrHash {
        size_t operator()(const DiffVarInterfacePtr &p) const {
            return p->Hash();
        }
    };

    struct DiffVarInterfacePtrEqual {
        bool operator()(const DiffVarInterfacePtr &left, const DiffVarInterfacePtr &right) const {
            return left->Equal(right);
        }
    };

private:
    DiffType m_type = DT_NIL;
    std::string m_string;
    int64_t m_integer = 0;
    double m_number = 0;
    bool m_boolean = false;
    std::unordered_map<DiffVarInterfacePtr, DiffVarInterfacePtr, DiffVarInterfacePtrHash, DiffVarInterfacePtrEqual> m_table;
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
extern "C" DiffVarInterfacePtr calc_env_diff(DiffEnv *env, DiffLoggerInterface *log, DiffVarInterfacePtr input);

// 释放上下文
extern "C" void free_diff_env(DiffEnv *env);
