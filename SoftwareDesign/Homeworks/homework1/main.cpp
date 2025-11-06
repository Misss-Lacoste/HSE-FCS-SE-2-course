#include <iostream>
#include <memory>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <stack>
#include <map>
#include "account.h"
#include "category.h"
#include "operation.h"
#include "account_facade.h"
#include "category_service.h"
#include "operation_service.h"
#include "analytics_service.h"
#include "command.h"

class FinancialManager {
private:
    AccountFacade AccountFacade;
    CategoryService categoryService;
    OperationService operationService;
    AnalyticsService analyticsService;
    
    std::stack<std::unique_ptr<Command>> undoStack; //cтек для undo/redo
    std::stack<std::unique_ptr<Command>> redoStack;
    
    std::map<std::string, std::function<void()>> commands; //cистема команд

public:
    FinancialManager() 
        : operationService(AccountFacade, categoryService)
        , analyticsService(operationService, categoryService) {
        initializeCommands();
        initializeTestData();
    }

    void run() {
        std::cout << "Good afternoon, ladies and gentleman!\n";
        std::cout << "Welcome to our HSE Bank!";
        std::cout << "Type 'help' to see how we can help you!\n\n";
        
        while (true) {
            std::cout << "> ";
            std::string input;
            std::getline(std::cin, input);
            
            if (input == "exit" || input == "0") {
                std::cout << "Goodbye! We were glad to see you! Have a nice day!\n";
                break;
            }
            
            processCommand(input);
        }
    }

private:
    void initializeCommands() {
        commands = {
            {"create-account", [this]() { createAccount(); }},
            {"delete-account", [this]() { deleteAccount(); }},
            {"show-accounts", [this]() { showAccounts(); }},
            {"create-category", [this]() { createCategory(); }},
            {"delete-category", [this]() { deleteCategory(); }},
            {"show-categories", [this]() { showCategories(); }},
            {"create-operation", [this]() { createOperation(); }},
            {"delete-operation", [this]() { deleteOperation(); }},
            {"show-operations", [this]() { showOperations(); }},
            {"show-account-operations", [this]() { showAccountOperations(); }},
            {"export-data", [this]() { exportData(); }},
            {"import-data", [this]() { importData(); }},
            {"undo", [this]() { undo(); }},
            {"redo", [this]() { redo(); }},
            {"help", [this]() { showHelp(); }},
            {"analytics", [this]() { showAnalytics(); }}
        };
    }

    void initializeTestData() {
        categoryService.createCategory(CategoryType::INCOME, "Salary"); //тестовые категории
        categoryService.createCategory(CategoryType::INCOME, "Bonus");
        categoryService.createCategory(CategoryType::EXPENSE, "Food");
        categoryService.createCategory(CategoryType::EXPENSE, "Transport");
        categoryService.createCategory(CategoryType::EXPENSE, "Entertainment");
        
        AccountFacade.createAccount("Main account", 1000.0); //тестовые счета
        AccountFacade.createAccount("Savings account", 5000.0);
    }

    void processCommand(const std::string& input) {
        std::istringstream iss(input);
        std::string command;
        iss >> command;
        
        auto it = commands.find(command);
        if (it != commands.end()) {
            it->second();
        } else {
            std::cout << "Oh, miss/mister, sorry, we don't have options for such a command. Don't you misspelled?\n";
        }
    }

    void showHelp() {
        std::cout << "\nLet's see what we have prepared for you, darlings!\n";
        std::cout << "create-account          - Create new HSE Bank account!\n";
        std::cout << "delete-account          - Delete Hse Bank account :(\n";
        std::cout << "show-accounts           - Show all you HSE bank accounts\n";
        std::cout << "create-category         - Create new category in HSE Bank account!\n";
        std::cout << "delete-category         - Delete category in HSE Bank account..\n";
        std::cout << "show-categories         - Show all categories in HSE Bank account!\n";
        std::cout << "create-operation        - Create financial operation in our favourite HSE Bank account!!\n";
        std::cout << "delete-operation        - Delete operation in HSE Bank account!\n";
        std::cout << "show-operations         - Show all operations in HSE Bank account!\n";
        std::cout << "show-account-operations - Show operations for specific account!\n";
        std::cout << "export-data             - Export data to file (CSV/JSON/YAML)!\n";
        std::cout << "import-data             - Import data from file (CSV/JSON/YAML)!\n";
        std::cout << "undo                    - Undo last command!\n";
        std::cout << "redo                    - Redo last undone command!\n";
        std::cout << "analytics               - Show financial analytics!\n";
        std::cout << "help                    - Show this help!\n";
        std::cout << "exit                    - Exit application!\n\n";
    }

    void createAccount() {
        std::cout << "My darling, what's your name? Please, enter it: ";
        std::string name;
        std::getline(std::cin, name);
        
        std::cout << "How much money do you have? Enter you initial balance, please: ";
        double balance;
        std::cin >> balance;
        std::cin.ignore();
        
        auto account = AccountFacade.createAccount(name, balance);
        if (account) {
            std::cout << "Oh, darling, account have been created successfully! Your ID: " << account->getId() << "\n";
        } else {
            std::cout << "Oh, something went wrong...\n";
        }
    }

    void deleteAccount() {
        showAccounts();
        std::cout << "Darling, enter account ID to delete it: ";
        int id;
        std::cin >> id;
        std::cin.ignore();
        
        if (AccountFacade.deleteAccount(id)) {
            std::cout << "Well, account have been deleted successfully.\n";
        } else {
            std::cout << "Hmm, we can't find such an account...\n";
        }
    }

    void showAccounts() {
        auto accounts = AccountFacade.getAllAccounts();
        std::cout << "\nInformation:\n";
        for (const auto& account : accounts) {
            std::cout << "ID: " << account->getId() 
                      << " | Name: " << account->getName()
                      << " | Balance: " << account->getBalance() << "\n";
        }
        std::cout << "Total: " << accounts.size() << " accounts\n\n";
    }

    void createCategory() {
        std::cout << "Dear, select category type (1=Income, 2=Expense): ";
        int typeChoice;
        std::cin >> typeChoice;
        std::cin.ignore();
        
        CategoryType type = (typeChoice == 1) ? CategoryType::INCOME : CategoryType::EXPENSE;
        
    std::cout << "Please, my dear, enter category name: ";
        std::string name;
        std::getline(std::cin, name);
        
        auto category = categoryService.createCategory(type, name);
        if (category) {
            std::cout << "Hurray! Category have been created successfully! Your ID: " << category->getId() << "\n";
        } else {
            std::cout << "Something went wrong...\n";
        }
    }

    void deleteCategory() {
        showCategories();
        std::cout << "and now enter category ID to delete: ";
        int id;
        std::cin >> id;
        std::cin.ignore();
        
        if (categoryService.deleteCategory(id)) {
            std::cout << "Ok, your category have been deleted successfully.\n";
        } else {
            std::cout << "Oops, 404: category not found.\n";
        }
    }

    void showCategories() {
        auto categories = categoryService.getAllCategories();
        std::cout << "\nCATEGORIES\n";
        for (const auto& category : categories) {
            std::string typeStr = (category->getType() == CategoryType::INCOME) ? "INCOME" : "EXPENSE";
            std::cout << "ID: " << category->getId() 
                      << " | Type: " << typeStr
                      << " | Name: " << category->getName() << "\n";
        }
        std::cout << "Total: " << categories.size() << " categories\n\n";
    }

    void createOperation() {
        showAccounts();
        std::cout << "Dear, enter account ID: ";
        int accountId;
        std::cin >> accountId;
        std::cin.ignore();
        
        showCategories();
        std::cout << "Dear, enter category ID: ";
        int categoryId;
        std::cin >> categoryId;
        std::cin.ignore();
        
        std::cout << "Darling, enter operation type (1=Income, 2=Expense): ";
        int typeChoice;
        std::cin >> typeChoice;
        std::cin.ignore();
        OperationType type = (typeChoice == 1) ? OperationType::INCOME : OperationType::EXPENSE;
        
        std::cout << "My dear, enter amount: ";
        double amount;
        std::cin >> amount;
        std::cin.ignore();
        
        std::cout << "Please, my dear, enter date (YYYY-MM-DD): ";
        std::string date;
        std::getline(std::cin, date);
        
        std::cout << "Can yoy enter description here? So, please: ";
        std::string description;
        std::getline(std::cin, description);
        
        auto operation = operationService.createOperation(type, accountId, amount, date, description, categoryId);
        if (operation) {
            std::cout << "Thanks God! Operation have been created successfully! ID: " << operation->getId() << "\n";
        } else {
            std::cout << "Oh, no, bad gateway, 404...Failed to create operation. Check account and category IDs.\n";
        }
    }

    void deleteOperation() {
        showOperations();
        std::cout << "My belowed client, enter operation ID to delete: ";
        int id;
        std::cin >> id;
        std::cin.ignore();
        
        if (operationService.deleteOperation(id)) {
            std::cout << "Ok, lets's see... Aha! Operation have been deleted successfully.\n";
        } else {
            std::cout << "Hmm, 404. Operation have not found.\n";
        }
    }

    void showOperations() {
        auto operations = operationService.getAllOperations();
        std::cout << "\nOPERATIONS\n";
        for (const auto& operation : operations) {
            std::string typeStr = (operation->getType() == OperationType::INCOME) ? "INCOME" : "EXPENSE";
            std::cout << "ID: " << operation->getId()
                      << " | Type: " << typeStr
                      << " | Account: " << operation->getBankAccountId()
                      << " | Amount: " << operation->getAmount()
                      << " | Date: " << operation->getDate()
                      << " | Desc: " << operation->getDescription() << "\n";
        }
        std::cout << "Total: " << operations.size() << " operations\n\n";
    }

    void showAccountOperations() {
        showAccounts();
        std::cout << "My favourite, enter account ID: ";
        int accountId;
        std::cin >> accountId;
        std::cin.ignore();
        
        auto operations = operationService.getOperationsByAccount(accountId);
        std::cout << "\nACCOUNT OPERATIONS\n";
        for (const auto& operation : operations) {
            std::string typeStr = (operation->getType() == OperationType::INCOME) ? "INCOME" : "EXPENSE";
            std::cout << "ID: " << operation->getId()
                      << " | Type: " << typeStr
                      << " | Amount: " << operation->getAmount()
                      << " | Date: " << operation->getDate()
                      << " | Desc: " << operation->getDescription() << "\n";
        }
        std::cout << "Total: " << operations.size() << " operations\n\n";
    }

    void exportData() {
        std::cout << "Please, select format (1=CSV, 2=JSON, 3=YAML): ";
        int format;
        std::cin >> format;
        std::cin.ignore();
        
        std::cout << "Darling, enter filename (without extension): ";
        std::string filename;
        std::getline(std::cin, filename);
        
        switch (format) {
            case 1:
                exportToCSV(filename + ".csv");
                break;
            case 2:
                exportToJSON(filename + ".json");
                break;
            case 3:
                exportToYAML(filename + ".yaml");
                break;
            default:
                std::cout << "Invalid format selected.\n";
        }
    }

    void importData() {
        std::cout << "Enter filename with extension: ";
        std::string filename;
        std::getline(std::cin, filename);
        
        if (filename.find(".csv") != std::string::npos) {
            importFromCSV(filename);
        } else if (filename.find(".json") != std::string::npos) {
            importFromJSON(filename);
        } else if (filename.find(".yaml") != std::string::npos || filename.find(".yml") != std::string::npos) {
            importFromYAML(filename);
        } else {
            std::cout << "Dear, you know, it's unsupported file format...\n";
        }
    }

    void undo() {
        if (undoStack.empty()) {
            std::cout << "Well, it's actually nothing to undo!\n";
            return;
        }
        
        auto command = std::move(undoStack.top());
        undoStack.pop();
        command->undo();
        redoStack.push(std::move(command));
        std::cout << "Dear, the undo operation completed!\n";
    }

    void redo() {
        if (redoStack.empty()) {
            std::cout << "Nothing to redo, Dear!\n";
            return;
        }
        
        auto command = std::move(redoStack.top());
        redoStack.pop();
        command->execute();
        undoStack.push(std::move(command));
        std::cout << "Redo completed, my Darling!\n";
    }

    void showAnalytics() {
        std::cout << "\nFINANCIAL ANALYTICS\n";
        
        auto analysis = analyticsService.analyzeFinances();
        std::cout << "Total Income: " << analysis.totalIncome << "\n";
        std::cout << "Total Expenses: " << analysis.totalExpenses << "\n";
        std::cout << "Net Balance: " << analysis.netBalance << "\n";
        
        std::cout << "\nIncome by Category:\n";
        for (const auto& [categoryId, amount] : analysis.incomeByCategory) 
        {
            auto category = categoryService.getCategory(categoryId);
            std::string categoryName = category ? category->getName() : "Unknown";
            std::cout << "  " << categoryName << ": " << amount << "\n";
        }
        
        std::cout << "\nExpenses by Category:\n";
        for (const auto& [categoryId, amount] : analysis.expensesByCategory) {
            auto category = categoryService.getCategory(categoryId);
            std::string categoryName = category ? category->getName() : "Unknown";
            std::cout << "  " << categoryName << ": " << amount << "\n";
        }
        std::cout << "\n";
    }

    void exportToCSV(const std::string& filename) {
        std::ofstream file(filename);
        file << "ACCOUNTS\n";
        file << "ID,Name,Balance\n";
        auto accounts = AccountFacade.getAllAccounts();
        for (const auto& account : accounts) {
            file << account->getId() << "," << account->getName() << "," << account->getBalance() << "\n";
        }
        
        file << "\nCATEGORIES\n";
        file << "ID,Type,Name\n";
        auto categories = categoryService.getAllCategories();
        for (const auto& category : categories) {
            std::string typeStr = (category->getType() == CategoryType::INCOME) ? "INCOME" : "EXPENSE";
            file << category->getId() << "," << typeStr << "," << category->getName() << "\n";
        }
        
        file << "\nOPERATIONS\n";
        file << "ID,Type,AccountID,Amount,Date,Description,CategoryID\n";
        auto operations = operationService.getAllOperations();
        for (const auto& operation : operations) {
            std::string typeStr = (operation->getType() == OperationType::INCOME) ? "INCOME" : "EXPENSE";
            file << operation->getId() << "," << typeStr << "," << operation->getBankAccountId()
                 << "," << operation->getAmount() << "," << operation->getDate()
                 << "," << operation->getDescription() << "," << operation->getCategoryId() << "\n";
        }
        
        file.close();
        std::cout << "My Darling, we have exported data to " << filename << "\n";
    }

    void exportToJSON(const std::string& filename) {
        std::ofstream file(filename);
        file << "{\n";
        
        file << "  \"accounts\": [\n";
        auto accounts = AccountFacade.getAllAccounts();
        for (size_t i = 0; i < accounts.size(); ++i) {
            file << "    {\n";
            file << "      \"id\": " << accounts[i]->getId() << ",\n";
            file << "      \"name\": \"" << accounts[i]->getName() << "\",\n";
            file << "      \"balance\": " << accounts[i]->getBalance() << "\n";
            file << "    }";
            if (i < accounts.size() - 1) file << ",";
            file << "\n";
        }
        file << "  ],\n";
        
        // Export categories
        file << "  \"categories\": [\n";
        auto categories = categoryService.getAllCategories();
        for (size_t i = 0; i < categories.size(); ++i) {
            std::string typeStr = (categories[i]->getType() == CategoryType::INCOME) ? "INCOME" : "EXPENSE";
            file << "    {\n";
            file << "      \"id\": " << categories[i]->getId() << ",\n";
            file << "      \"type\": \"" << typeStr << "\",\n";
            file << "      \"name\": \"" << categories[i]->getName() << "\"\n";
            file << "    }";
            if (i < categories.size() - 1) file << ",";
            file << "\n";
        }
        file << "  ],\n";
        
        file << "  \"operations\": [\n";
        auto operations = operationService.getAllOperations();
        for (size_t i = 0; i < operations.size(); ++i) {
            std::string typeStr = (operations[i]->getType() == OperationType::INCOME) ? "INCOME" : "EXPENSE";
            file << "    {\n";
            file << "      \"id\": " << operations[i]->getId() << ",\n";
            file << "      \"type\": \"" << typeStr << "\",\n";
            file << "      \"account_id\": " << operations[i]->getBankAccountId() << ",\n";
            file << "      \"amount\": " << operations[i]->getAmount() << ",\n";
            file << "      \"date\": \"" << operations[i]->getDate() << "\",\n";
            file << "      \"description\": \"" << operations[i]->getDescription() << "\",\n";
            file << "      \"category_id\": " << operations[i]->getCategoryId() << "\n";
            file << "    }";
            if (i < operations.size() - 1) file << ",";
            file << "\n";
        }
        file << "  ]\n";
        file << "}\n";
        
        file.close();
        std::cout << "Dear, we have exported data to " << filename << "\n";
    }

    void exportToYAML(const std::string& filename) {
        std::ofstream file(filename);
        file << "accounts:\n";
        auto accounts = AccountFacade.getAllAccounts();
        for (const auto& account : accounts) {
            file << "  - id: " << account->getId() << "\n";
            file << "    name: \"" << account->getName() << "\"\n";
            file << "    balance: " << account->getBalance() << "\n";
        }
        
        file << "\ncategories:\n";
        auto categories = categoryService.getAllCategories();
        for (const auto& category : categories) {
            std::string typeStr = (category->getType() == CategoryType::INCOME) ? "INCOME" : "EXPENSE";
            file << "  - id: " << category->getId() << "\n";
            file << "    type: " << typeStr << "\n";
            file << "    name: \"" << category->getName() << "\"\n";
        }
        
        file << "\noperations:\n";
        auto operations = operationService.getAllOperations();
        for (const auto& operation : operations) {
            std::string typeStr = (operation->getType() == OperationType::INCOME) ? "INCOME" : "EXPENSE";
            file << "  - id: " << operation->getId() << "\n";
            file << "    type: " << typeStr << "\n";
            file << "    account_id: " << operation->getBankAccountId() << "\n";
            file << "    amount: " << operation->getAmount() << "\n";
            file << "    date: \"" << operation->getDate() << "\"\n";
            file << "    description: \"" << operation->getDescription() << "\"\n";
            file << "    category_id: " << operation->getCategoryId() << "\n";
        }
        
        file.close();
        std::cout << "My dear,we have exported data to " << filename << "\n";
    }

    void importFromCSV(const std::string& filename) {
        std::cout << "CSV import from " << filename << " - is in process...\n";
    }

    void importFromJSON(const std::string& filename) {
        std::cout << "JSON import from " << filename << " - is in process...\n";
    }

    void importFromYAML(const std::string& filename) {
        std::cout << "YAML import from " << filename << " - is in process...\n";
    }
};

int main() {
    try {
        FinancialManager manager;
        manager.run();
    } catch (const std::exception& e) {
        std::cerr << "Oops, badgateway, 404, error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}