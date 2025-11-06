#ifndef CATEGORY_H
#define CATEGORY_H

#include <string>
/*Классифицируем тут финансовые операции - income & expense*/
enum class CategoryType { INCOME, EXPENSE };

class Category {
public:
    Category(int id, CategoryType type, const std::string& name);
    
    int getId() const;
    CategoryType getType() const;
    std::string getName() const;
    void setName(const std::string& name);

private:
    int id_;
    CategoryType type_;
    std::string name_;
};

#endif