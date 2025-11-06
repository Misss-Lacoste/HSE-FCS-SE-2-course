#include "account.h" //регистрируем аккаунт

BankAccount::BankAccount(int id, const std::string& name, double balance)
    : id_(id), name_(name), balance_(balance) {}

int BankAccount::getId() const 
{ 
    return id_; 
}
std::string BankAccount::getName() const 
{ 
    return name_; 
}
double BankAccount::getBalance() const { 
    return balance_; 
}

void BankAccount::updateBalance(double amount) {
    balance_ += amount;
}

void BankAccount::setName(const std::string& name) {
    name_ = name;
}