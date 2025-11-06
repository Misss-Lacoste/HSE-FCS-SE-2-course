#ifndef OPERATION_H
#define OPERATION_H

#include <string>
/*тип финансовой операции, так же incomes и expenses*/
enum class OperationType { INCOME, EXPENSE }; //доменный класс по типу операции

class Operation { 
public:
    Operation(int id, OperationType type, int bankAccountId, double amount, 
              const std::string& date, const std::string& description, int categoryId);
    //геттеры соответсвующие
    int getId() const;
    OperationType getType() const;
    int getBankAccountId() const;
    double getAmount() const;
    std::string getDate() const;
    std::string getDescription() const;
    int getCategoryId() const;
    
    void setAmount(double amount); //для новой суммы операции
    void setDescription(const std::string& description);//описание новой операции

private:
    int id_;
    OperationType type_;
    int bank_account_id_;
    double amount_;
    std::string date_;
    std::string description_;
    int category_id_;
};

#endif