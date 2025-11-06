#ifndef OPERATION_SERVICE_H
#define OPERATION_SERVICE_H

#include "operation.h"
#include "factory.h"
#include "account_facade.h"
#include "category_service.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <iostream>

class OperationService { //фасад для управления фин операциями
private:
    std::unordered_map<int, std::unique_ptr<Operation>> operations_; //ранилище операций
    AccountFacade& account_service_; //сервис счетов
    CategoryService& category_service_; //сервис категорий
    int next_id_;

public: //создаем новую фин операцию, и вся информация о ней
    OperationService(AccountFacade& AccountFacade, CategoryService& categoryService) //внедряем зависимость AccountFacade &categoryservice
        : account_service_(AccountFacade), category_service_(categoryService), next_id_(1) {}

    Operation* createOperation(OperationType type, int accountId, double amount, 
                              const std::string& date, const std::string& description, int categoryId) {
        if (!account_service_.getAccount(accountId) || !category_service_.getCategory(categoryId)) {
            return nullptr;
        }

        auto operation = DomainFactory::createOperation(next_id_++, type, accountId, amount, date, description, categoryId);
        auto* ptr = operation.get();
        operations_[ptr->getId()] = std::move(operation);

        //обновлем баланс автоматом
        updateAccountBalance(accountId);
        return ptr;
    }

    Operation* getOperation(int id) {
        auto it = operations_.find(id);
        return (it != operations_.end()) ? it->second.get() : nullptr;
    }

    bool deleteOperation(int id) {
        auto it = operations_.find(id);
        if (it != operations_.end()) {
            int accountId = it->second->getBankAccountId();
            operations_.erase(it);
            updateAccountBalance(accountId); //пересчитываем баланс
            return true;
        }
        return false;
    }

    std::vector<Operation*> getAllOperations() {
        std::vector<Operation*> result;
        for (const auto& pair : operations_) {
            result.push_back(pair.second.get());
        }
        return result;
    }

    std::vector<Operation*> getOperationsByAccount(int accountId) {
        std::vector<Operation*> result;
        for (const auto& pair : operations_) {
            if (pair.second->getBankAccountId() == accountId) {
                result.push_back(pair.second.get());
            }
        }
        return result;
    }

    std::vector<Operation*> getOperationsByCategory(int categoryId) {
        std::vector<Operation*> result;
        for (const auto& pair : operations_) {
            if (pair.second->getCategoryId() == categoryId) {
                result.push_back(pair.second.get());
            }
        }
        return result;
    }

    void recalculateAllBalances() {
        auto accounts = account_service_.getAllAccounts();
        for (auto* account : accounts) {
            updateAccountBalance(account->getId());
        }
        std::cout << "Recalculated balances for all accounts\n";
    }

private:
    void updateAccountBalance(int accountId) {
        auto account = account_service_.getAccount(accountId);
        if (!account) return;

        double newBalance = 0.0;
        auto accountOperations = getOperationsByAccount(accountId);
        
        for (auto* operation : accountOperations) {
            if (operation->getType() == OperationType::INCOME) {
                newBalance += operation->getAmount();
            } else {
                newBalance -= operation->getAmount();
            }
        }

        auto tempAccount = DomainFactory::createAccount(account->getId(), account->getName(), 0);
        account_service_.deleteAccount(accountId);
        account_service_.createAccount(tempAccount->getName(), newBalance);
    }
};

#endif