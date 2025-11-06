#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <string>
#include <memory>
/*здесь мы создадим доменный класс банковского аккаунта, который
котрый будет отвечать за хранение инфы о счете: айди, название счета, баланс
соответствует принципу SOLID(S - Single Responsibility Principle)
*/
class BankAccount {
public:
    BankAccount(int id, const std::string& name, double balance = 0.0);
    
    int getId() const;
    std::string getName() const;
    double getBalance() const;
    
    void updateBalance(double amount);
    void setName(const std::string& name);

private:
    int id_;
    std::string name_;
    double balance_;
};

#endif