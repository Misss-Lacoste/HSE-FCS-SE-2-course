#ifndef CONCRETE_COMMANDS_H
#define CONCRETE_COMMANDS_H

#include "command.h"
#include "account_facade.h"
#include "category_service.h"
#include "operation_service.h"
#include "analytics_service.h"
#include <iostream>

class CreateAccountCommand : public Command { //паттерн команда для конкретной команды для создания счета
private:
    AccountFacade& service_; //счеты сервис
    std::string name_; //имя нового счетв
    double balance_; //изначальный баланс
    BankAccount* created_account_;

public:
    CreateAccountCommand(AccountFacade& service, const std::string& name, double balance = 0.0)
        : service_(service), name_(name), balance_(balance), created_account_(nullptr) {}

    void execute() override {
        created_account_ = service_.createAccount(name_, balance_);
        if (created_account_) {
            std::cout << "Created account: " << created_account_->getName() 
                      << " with balance: " << created_account_->getBalance() << "\n";
        }
    }

    void undo() override {
        if (created_account_) {
            service_.deleteAccount(created_account_->getId());
            std::cout << "Undo: deleted account " << name_ << "\n";
            created_account_ = nullptr;
        }
    }
};

class CreateCategoryCommand : public Command {
private:
    CategoryService& service_;
    CategoryType type_;
    std::string name_;
    Category* created_category_;

public:
    CreateCategoryCommand(CategoryService& service, CategoryType type, const std::string& name)
        : service_(service), type_(type), name_(name), created_category_(nullptr) {}

    void execute() override {
        created_category_ = service_.createCategory(type_, name_);
        if (created_category_) {
            std::string typeStr = (type_ == CategoryType::INCOME) ? "Income" : "Expense";
            std::cout << "Created " << typeStr << " category: " << created_category_->getName() << "\n";
        }
    }

    void undo() override {
        if (created_category_) {
            service_.deleteCategory(created_category_->getId());
            std::cout << "Undo: deleted category " << name_ << "\n";
            created_category_ = nullptr;
        }
    }
};

class CreateOperationCommand : public Command {
private:
    OperationService& service_;
    OperationType type_;
    int account_id_;
    double amount_;
    std::string date_;
    std::string description_;
    int category_id_;
    Operation* created_operation_;

public:
    CreateOperationCommand(OperationService& service, OperationType type, int accountId,
                          double amount, const std::string& date, const std::string& description, int categoryId)
        : service_(service), type_(type), account_id_(accountId), amount_(amount),
          date_(date), description_(description), category_id_(categoryId), created_operation_(nullptr) 
          {

          }

    void execute() override {
        created_operation_ = service_.createOperation(type_, account_id_, amount_, date_, description_, category_id_);
        if (created_operation_) {
            std::string typeStr = (type_ == OperationType::INCOME) ? "Income" : "Expense";
            std::cout << "Created " << typeStr << " operation: " << amount_ << " for account " << account_id_ << "\n";
        } else {
            std::cout << "Failed to create operation\n";
        }
    }

    void undo() override {
        if (created_operation_) {
            service_.deleteOperation(created_operation_->getId());
            std::cout << "Undo: deleted operation\n";
            created_operation_ = nullptr;
        }
    }
};

class AnalyzeFinancesCommand : public Command {
private:
    AnalyticsService& service_;

public:
    AnalyzeFinancesCommand(AnalyticsService& service) : service_(service) {}

    void execute() override {
        service_.printAnalysis();
    }

    void undo() override {
        std::cout << "Analysis can't be undone\n";
    }
};

#endif