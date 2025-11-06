#ifndef VISITOR_H
#define VISITOR_H

#include "account.h"
#include "category.h"
#include "operation.h"
#include <vector>
#include <string>

class DataExportVisitor { //паттерн для экспорта данных, новые операции к объектам можно добавить
public: //посещаем банкаккаунт, категорию, оперейшн для экспорта данных 
    virtual ~DataExportVisitor() = default;
    virtual std::string visitAccount(const BankAccount& account) = 0;
    virtual std::string visitCategory(const Category& category) = 0;
    virtual std::string visitOperation(const Operation& operation) = 0;
};

class JSONExportVisitor : public DataExportVisitor { //тут конкретный посетитель для экспорта в джейсон
public:
    std::string visitAccount(const BankAccount& account) override {
        return "{\"id\":" + std::to_string(account.getId()) + 
               ",\"name\":\"" + account.getName() + 
               "\",\"balance\":" + std::to_string(account.getBalance()) + "}";
    }

    std::string visitCategory(const Category& category) override {
        std::string typeStr = (category.getType() == CategoryType::INCOME) ? "INCOME" : "EXPENSE";
        return "{\"id\":" + std::to_string(category.getId()) + 
               ",\"type\":\"" + typeStr + 
               "\",\"name\":\"" + category.getName() + "\"}";
    }

    std::string visitOperation(const Operation& operation) override {
        std::string typeStr = (operation.getType() == OperationType::INCOME) ? "INCOME" : "EXPENSE";
        return "{\"id\":" + std::to_string(operation.getId()) + 
               ",\"type\":\"" + typeStr + 
               "\",\"account_id\":" + std::to_string(operation.getBankAccountId()) + 
               ",\"amount\":" + std::to_string(operation.getAmount()) + 
               ",\"date\":\"" + operation.getDate() + 
               "\",\"description\":\"" + operation.getDescription() + 
               "\",\"category_id\":" + std::to_string(operation.getCategoryId()) + "}";
    }
};

#endif