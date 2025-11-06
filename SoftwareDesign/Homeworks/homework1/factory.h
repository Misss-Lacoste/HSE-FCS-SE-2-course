#ifndef FACTORY_H
#define FACTORY_H

#include "account.h"
#include "category.h"
#include "operation.h"
#include <memory>
#include <stdexcept>

class DomainFactory { //паттерн-фабрика создания доменных объектов
    //централизируем создание всех доменных объектов тут
    //создаем новый банковский счет, новую категорию, новую фин операцию
public:
    static std::unique_ptr<BankAccount> createAccount(int id, const std::string& name, double balance = 0.0) {
        if (balance < 0) {
            throw std::invalid_argument("Balance can't be negative");
        }
        if (name.empty()) {
            throw std::invalid_argument("Account name can't be empty");
        }
        return std::make_unique<BankAccount>(id, name, balance);
    }

    static std::unique_ptr<Category> createCategory(int id, CategoryType type, const std::string& name) {
        if (name.empty()) {
            throw std::invalid_argument("Category name can't be empty");
        }
        return std::make_unique<Category>(id, type, name);
    }

    static std::unique_ptr<Operation> createOperation(int id, OperationType type, int bankAccountId, 
                                                     double amount, const std::string& date, 
                                                     const std::string& description, int categoryId) {
        if (amount <= 0) 
        {
            throw std::invalid_argument("Operation amount must be positive");
        }
        if (date.empty()) 
        {
            throw std::invalid_argument("Operation date can't be empty");
        }
        return std::make_unique<Operation>(id, type, bankAccountId, amount, date, description, categoryId);
    }
};

#endif