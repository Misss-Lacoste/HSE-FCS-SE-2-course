#include "category.h"

Category::Category(int id, CategoryType type, const std::string& name)
    : id_(id), type_(type), name_(name) {}

int Category::getId() const { 
    return id_; 
}
CategoryType Category::getType() const 
{ 
    return type_; 
}
std::string Category::getName() const 
{ 
    return name_; 
}

void Category::setName(const std::string& name) {
    name_ = name;
}