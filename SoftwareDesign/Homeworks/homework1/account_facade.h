#ifndef ACCOUNT_SERVICE_H
#define ACCOUNT_SERVICE_H

#include "account.h"
#include "factory.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <iostream>

class AccountFacade { //паттерн для управления банковскими счетами(Фасад)- High cohesion и High coupling
private:
    std::unordered_map<int, std::unique_ptr<BankAccount>> accounts_; //хранилище счетов
    int next_id_; //счетчик для генерации айдишников

public:
    AccountFacade() : next_id_(1) {} //конструктор сервиса счетов

    BankAccount* createAccount(const std::string& name, double initialBalance = 0.0) {
        auto account = DomainFactory::createAccount(next_id_++, name, initialBalance);
        auto* ptr = account.get();
        accounts_[ptr->getId()] = std::move(account); 
        return ptr;
    }

    BankAccount* getAccount(int id) { //получаем счет по айди
        auto it = accounts_.find(id);
        return (it != accounts_.end()) ? it->second.get() : nullptr;
    }

    bool updateAccount(int id, const std::string& newName) { //обновляем название счета
        auto account = getAccount(id);
        if (account) {
            account->setName(newName);
            return true;
        }
        return false;
    }

    bool deleteAccount(int id) { //удялем счет
        return accounts_.erase(id) > 0;
    }

    std::vector<BankAccount*> getAllAccounts() { //получаем все счета
        std::vector<BankAccount*> result;
        for (const auto& pair : accounts_) {
            result.push_back(pair.second.get());
        }
        return result;
    }

    void recalculateBalance(int accountId) { //для баланса счета, перерасчет
        auto account = getAccount(accountId);
        if (account) {
            std::cout << "Now we are recalculating balance for account " << accountId << "\n";
        }
    }
};

#endif