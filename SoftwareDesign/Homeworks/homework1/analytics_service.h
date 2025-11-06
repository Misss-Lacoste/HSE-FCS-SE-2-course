#ifndef ANALYTICS_SERVICE_H
#define ANALYTICS_SERVICE_H

#include "operation_service.h"
#include "category_service.h"
#include <vector>
#include <map>
#include <string>
#include <iostream>
//тоже создаем фасад для удобства
struct FinancialAnalysis { //общая структурка для след. опций:
    double totalIncome; //ведём учет доходов
    double totalExpenses; //...и расходов
    double netBalance;
    std::map<int, double> incomeByCategory;
    std::map<int, double> expensesByCategory;
};

class AnalyticsService {
private:
    OperationService& operation_service_;
    CategoryService& category_service_;

public:
    AnalyticsService(OperationService& operationService, CategoryService& categoryService)
        : operation_service_(operationService), category_service_(categoryService) {}

    FinancialAnalysis analyzeFinances() {
        FinancialAnalysis analysis{0, 0, 0, {}, {}};
        auto operations = operation_service_.getAllOperations();

        for (auto* operation : operations) {
            if (operation->getType() == OperationType::INCOME) {
                analysis.totalIncome += operation->getAmount();
                analysis.incomeByCategory[operation->getCategoryId()] += operation->getAmount();
            } else {
                analysis.totalExpenses += operation->getAmount();
                analysis.expensesByCategory[operation->getCategoryId()] += operation->getAmount();
            }
        }

        analysis.netBalance = analysis.totalIncome - analysis.totalExpenses;
        return analysis;
    }

    void printAnalysis() {
        auto analysis = analyzeFinances();
        
        std::cout << "\nFinancial analysis\n";
        std::cout << "Total income: " << analysis.totalIncome << "\n";
        std::cout << "Total expenses: " << analysis.totalExpenses << "\n";
        std::cout << "Net balance: " << analysis.netBalance << "\n";
        
        std::cout << "\nIncome by category:\n";
        for (const auto& [categoryId, amount] : analysis.incomeByCategory) {
            auto category = category_service_.getCategory(categoryId);
            std::string categoryName = category ? category->getName() : "Unknown";
            std::cout << "  " << categoryName << ": " << amount << "\n";
        }
        
        std::cout << "\nExpenses by category:\n";
        for (const auto& [categoryId, amount] : analysis.expensesByCategory) {
            auto category = category_service_.getCategory(categoryId);
            std::string categoryName = category ? category->getName() : "Unknown";
            std::cout << "  " << categoryName << ": " << amount << "\n";
        }
    }
};

#endif