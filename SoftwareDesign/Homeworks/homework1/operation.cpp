#include "operation.h"

Operation::Operation(int id, OperationType type, int bankAccountId, double amount, const std::string& date, const std::string& description, int categoryId)
    : id_(id), type_(type), bank_account_id_(bankAccountId), amount_(amount),
      date_(date), description_(description), category_id_(categoryId) {}

int Operation::getId() const 
{ 
    return id_; 
}
OperationType Operation::getType() const { 
    return type_; 
}
int Operation::getBankAccountId() const { 
    return bank_account_id_; 
}
double Operation::getAmount() const { 
    return amount_; 
}
std::string Operation::getDate() const { 
    return date_; 
}
std::string Operation::getDescription() const { 
    return description_; 
}
int Operation::getCategoryId() const { 
    return category_id_; 
}

void Operation::setAmount(double amount) {
    amount_ = amount;
}

void Operation::setDescription(const std::string& description) {
    description_ = description;
}