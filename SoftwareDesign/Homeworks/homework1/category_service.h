#ifndef CATEGORY_SERVICE_H
#define CATEGORY_SERVICE_H

#include "category.h"
#include "factory.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <iostream>

class CategoryService { //иная структурка для категорий и опций
private:
    std::unordered_map<int, std::unique_ptr<Category>> categories_;
    int next_id_;

public:
    CategoryService() : next_id_(1) {}

    Category* createCategory(CategoryType type, const std::string& name) {
        auto category = DomainFactory::createCategory(next_id_++, type, name);
        auto* ptr = category.get();
        categories_[ptr->getId()] = std::move(category);
        return ptr;
    }

    Category* getCategory(int id) {
        auto it = categories_.find(id);
        return (it != categories_.end()) ? it->second.get() : nullptr;
    }

    bool updateCategory(int id, const std::string& newName) {
        auto category = getCategory(id);
        if (category) {
            category->setName(newName);
            return true;
        }
        return false;
    }

    bool deleteCategory(int id) {
        return categories_.erase(id) > 0;
    }

    std::vector<Category*> getAllCategories() {
        std::vector<Category*> result;
        for (const auto& pair : categories_) {
            result.push_back(pair.second.get());
        }
        return result;
    }

    std::vector<Category*> getCategoriesByType(CategoryType type) {
        std::vector<Category*> result;
        for (const auto& pair : categories_) {
            if (pair.second->getType() == type) {
                result.push_back(pair.second.get());
            }
        }
        return result;
    }
};

#endif