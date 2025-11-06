#ifndef IMPORT_EXPORT_H
#define IMPORT_EXPORT_H

#include "account.h"
#include "category.h"
#include "operation.h"
#include "account_facade.h"
#include "category_service.h"
#include "operation_service.h"
#include "factory.h"
#include "visitor.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>

class BaseExporter {
public:
    virtual ~BaseExporter() = default;
    virtual std::string exportAccounts(const std::vector<BankAccount*>& accounts) = 0;
    virtual std::string exportCategories(const std::vector<Category*>& categories) = 0;
    virtual std::string exportOperations(const std::vector<Operation*>& operations) = 0;
};

class CSVExporter : public BaseExporter {
public:
    std::string exportAccounts(const std::vector<BankAccount*>& accounts) override {
        std::string result = "id,name,balance\n";
        for (const auto& account : accounts) {
            result += std::to_string(account->getId()) + "," +
                     escapeCsv(account->getName()) + "," +
                     std::to_string(account->getBalance()) + "\n";
        }
        return result;
    }

    std::string exportCategories(const std::vector<Category*>& categories) override {
        std::string result = "id,type,name\n";
        for (const auto& category : categories) {
            std::string typeStr = (category->getType() == CategoryType::INCOME) ? "INCOME" : "EXPENSE";
            result += std::to_string(category->getId()) + "," +
                     typeStr + "," +
                     escapeCsv(category->getName()) + "\n";
        }
        return result;
    }

    std::string exportOperations(const std::vector<Operation*>& operations) override {
        std::string result = "id,type,account_id,amount,date,description,category_id\n";
        for (const auto& operation : operations) {
            std::string typeStr = (operation->getType() == OperationType::INCOME) ? "INCOME" : "EXPENSE";
            result += std::to_string(operation->getId()) + "," +
                     typeStr + "," +
                     std::to_string(operation->getBankAccountId()) + "," +
                     std::to_string(operation->getAmount()) + "," +
                     escapeCsv(operation->getDate()) + "," +
                     escapeCsv(operation->getDescription()) + "," +
                     std::to_string(operation->getCategoryId()) + "\n";
        }
        return result;
    }

private:
    std::string escapeCsv(const std::string& field) {
        if (field.find(',') != std::string::npos || field.find('"') != std::string::npos || field.find('\n') != std::string::npos) {
            return "\"" + field + "\"";
        }
        return field;
    }
};

class YAMLExporter : public BaseExporter {
public:
    std::string exportAccounts(const std::vector<BankAccount*>& accounts) override {
        std::string result = "accounts:\n";
        for (const auto& account : accounts) {
            result += "  - id: " + std::to_string(account->getId()) + "\n";
            result += "    name: \"" + account->getName() + "\"\n";
            result += "    balance: " + std::to_string(account->getBalance()) + "\n";
        }
        return result;
    }

    std::string exportCategories(const std::vector<Category*>& categories) override {
        std::string result = "categories:\n";
        for (const auto& category : categories) {
            std::string typeStr = (category->getType() == CategoryType::INCOME) ? "INCOME" : "EXPENSE";
            result += "  - id: " + std::to_string(category->getId()) + "\n";
            result += "    type: " + typeStr + "\n";
            result += "    name: \"" + category->getName() + "\"\n";
        }
        return result;
    }

    std::string exportOperations(const std::vector<Operation*>& operations) override {
        std::string result = "operations:\n";
        for (const auto& operation : operations) {
            std::string typeStr = (operation->getType() == OperationType::INCOME) ? "INCOME" : "EXPENSE";
            result += "  - id: " + std::to_string(operation->getId()) + "\n";
            result += "    type: " + typeStr + "\n";
            result += "    account_id: " + std::to_string(operation->getBankAccountId()) + "\n";
            result += "    amount: " + std::to_string(operation->getAmount()) + "\n";
            result += "    date: \"" + operation->getDate() + "\"\n";
            result += "    description: \"" + operation->getDescription() + "\"\n";
            result += "    category_id: " + std::to_string(operation->getCategoryId()) + "\n";
        }
        return result;
    }
};

class JSONExporterAdapter : public BaseExporter {
private:
    JSONExportVisitor visitor_;

public:
    std::string exportAccounts(const std::vector<BankAccount*>& accounts) override {
        std::string result = "{\"accounts\": [\n";
        for (size_t i = 0; i < accounts.size(); ++i) {
            result += visitor_.visitAccount(*accounts[i]);
            if (i < accounts.size() - 1) result += ",";
            result += "\n";
        }
        result += "]}";
        return result;
    }

    std::string exportCategories(const std::vector<Category*>& categories) override {
        std::string result = "{\"categories\": [\n";
        for (size_t i = 0; i < categories.size(); ++i) {
            result += visitor_.visitCategory(*categories[i]);
            if (i < categories.size() - 1) result += ",";
            result += "\n";
        }
        result += "]}";
        return result;
    }

    std::string exportOperations(const std::vector<Operation*>& operations) override {
        std::string result = "{\"operations\": [\n";
        for (size_t i = 0; i < operations.size(); ++i) {
            result += visitor_.visitOperation(*operations[i]);
            if (i < operations.size() - 1) result += ",";
            result += "\n";
        }
        result += "]}";
        return result;
    }
};

//шабл.метод для импорта
class DataImporter {
public:
    virtual ~DataImporter() = default;
    
    //импортирует - то есть алгос
    void importData(const std::string& filename, AccountFacade& AccountFacade, 
                   CategoryService& categoryService, OperationService& operationService) {
        std::cout << "Starting import from " << filename << "\n";
        
        std::string content = readFile(filename);
        std::cout << "File read successfully, size: " << content.size() << " bytes\n";
        
        auto data = parseData(content);
        std::cout << "Data parsed: " << data.accounts.size() << " accounts, "
                  << data.categories.size() << " categories, "
                  << data.operations.size() << " operations\n";
        
        processData(data, AccountFacade, categoryService, operationService);
        std::cout << "Import completed successfully\n";
    }

protected:
    virtual std::string readFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Can't open this file: " + filename);
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    //абстрактные методы в подклассах
    virtual std::vector<std::unique_ptr<BankAccount>> parseAccounts(const std::string& content) = 0;
    virtual std::vector<std::unique_ptr<Category>> parseCategories(const std::string& content) = 0;
    virtual std::vector<std::unique_ptr<Operation>> parseOperations(const std::string& content) = 0;

private:
    struct ImportData {
        std::vector<std::unique_ptr<BankAccount>> accounts;
        std::vector<std::unique_ptr<Category>> categories;
        std::vector<std::unique_ptr<Operation>> operations;
    };

    ImportData parseData(const std::string& content) {
        ImportData data;
        data.accounts = parseAccounts(content);
        data.categories = parseCategories(content);
        data.operations = parseOperations(content);
        return data;
    }

    void processData(const ImportData& data, AccountFacade& AccountFacade, 
                    CategoryService& categoryService, OperationService& operationService) {
        //импорт счетов
        for (auto& account : data.accounts) {
            try {
                AccountFacade.createAccount(account->getName(), account->getBalance());
            } catch (const std::exception& e) {
                std::cerr << "Error importing account: " << e.what() << std::endl;
            }
        }

        //импорт категорий
        for (auto& category : data.categories) {
            try {
                categoryService.createCategory(category->getType(), category->getName());
            } catch (const std::exception& e) {
                std::cerr << "Error importing category: " << e.what() << std::endl;
            }
        }

        //импорт операций
        for (auto& operation : data.operations) {
            try {
                operationService.createOperation(operation->getType(), 
                                               operation->getBankAccountId(),
                                               operation->getAmount(),
                                               operation->getDate(),
                                               operation->getDescription(),
                                               operation->getCategoryId());
            } catch (const std::exception& e) {
                std::cerr << "Error importing operation: " << e.what() << std::endl;
            }
        }
    }
};

class CSVDataImporter : public DataImporter {
protected:
    std::vector<std::unique_ptr<BankAccount>> parseAccounts(const std::string& content) override {
        std::vector<std::unique_ptr<BankAccount>> accounts;
        if (content.find("name,balance") != std::string::npos) {
        }
        return accounts;
    }

    std::vector<std::unique_ptr<Category>> parseCategories(const std::string& content) override {
        std::vector<std::unique_ptr<Category>> categories;
        return categories;
    }

    std::vector<std::unique_ptr<Operation>> parseOperations(const std::string& content) override {
        std::vector<std::unique_ptr<Operation>> operations;
        std::istringstream stream(content);
        std::string line;
        bool firstLine = true;

        while (std::getline(stream, line)) {
            if (firstLine) {
                firstLine = false;
                continue;
            }
            
            auto fields = parseCSVLine(line);
            if (fields.size() >= 6) {
                try {
                    OperationType type = (fields[1] == "INCOME") ? OperationType::INCOME : OperationType::EXPENSE;
                    auto operation = DomainFactory::createOperation(
                        0, type, std::stoi(fields[2]), std::stod(fields[3]), 
                        fields[4], fields[5], std::stoi(fields[6])
                    );
                    operations.push_back(std::move(operation));
                } catch (const std::exception& e) {
                    std::cerr << "Error parsing operation: " << e.what() << std::endl;
                }
            }
        }
        return operations;
    }

private:
    std::vector<std::string> parseCSVLine(const std::string& line) {
        std::vector<std::string> result;
        std::string field;
        bool inQuotes = false;
        
        for (char c : line) {
            if (c == '"') {
                inQuotes = !inQuotes;
            } else if (c == ',' && !inQuotes) {
                result.push_back(field);
                field.clear();
            } else {
                field += c;
            }
        }
        result.push_back(field);
        return result;
    }
};

class JSONDataImporter : public DataImporter {
protected:
    std::vector<std::unique_ptr<BankAccount>> parseAccounts(const std::string& content) override {
        std::vector<std::unique_ptr<BankAccount>> accounts;
        size_t pos = content.find("\"accounts\"");
        if (pos != std::string::npos) {
        }
        return accounts;
    }

    std::vector<std::unique_ptr<Category>> parseCategories(const std::string& content) override {
        std::vector<std::unique_ptr<Category>> categories;
        return categories;
    }

    std::vector<std::unique_ptr<Operation>> parseOperations(const std::string& content) override {
        std::vector<std::unique_ptr<Operation>> operations;
        return operations;
    }
};

class YAMLDataImporter : public DataImporter {
protected:
    std::vector<std::unique_ptr<BankAccount>> parseAccounts(const std::string& content) override {
        std::vector<std::unique_ptr<BankAccount>> accounts;
        return accounts;
    }

    std::vector<std::unique_ptr<Category>> parseCategories(const std::string& content) override {
        std::vector<std::unique_ptr<Category>> categories;
        return categories;
    }

    std::vector<std::unique_ptr<Operation>> parseOperations(const std::string& content) override {
        std::vector<std::unique_ptr<Operation>> operations;
        return operations;
    }
};

class DataImportExportService {
private:
    AccountFacade& account_service_;
    CategoryService& category_service_;
    OperationService& operation_service_;
    
    std::unique_ptr<CSVExporter> csv_exporter_;
    std::unique_ptr<YAMLExporter> yaml_exporter_;
    std::unique_ptr<JSONExporterAdapter> json_exporter_;

public:
    DataImportExportService(AccountFacade& as, CategoryService& cs, OperationService& os)
        : account_service_(as), category_service_(cs), operation_service_(os),
          csv_exporter_(std::make_unique<CSVExporter>()),
          yaml_exporter_(std::make_unique<YAMLExporter>()),
          json_exporter_(std::make_unique<JSONExporterAdapter>()) {}

    //во все форматы экспортируем
    void exportAllData(const std::string& baseFilename) {
        auto accounts = account_service_.getAllAccounts();
        auto categories = category_service_.getAllCategories();
        auto operations = operation_service_.getAllOperations();

        //scv
        std::ofstream csvAccounts(baseFilename + "_accounts.csv");
        csvAccounts << csv_exporter_->exportAccounts(accounts);
        csvAccounts.close();

        std::ofstream csvCategories(baseFilename + "_categories.csv");
        csvCategories << csv_exporter_->exportCategories(categories);
        csvCategories.close();

        std::ofstream csvOperations(baseFilename + "_operations.csv");
        csvOperations << csv_exporter_->exportOperations(operations);
        csvOperations.close();

        //джейсон
        std::ofstream jsonFile(baseFilename + ".json");
        jsonFile << "{\n";
        jsonFile << "\"accounts\": " << json_exporter_->exportAccounts(accounts) << ",\n";
        jsonFile << "\"categories\": " << json_exporter_->exportCategories(categories) << ",\n";
        jsonFile << "\"operations\": " << json_exporter_->exportOperations(operations) << "\n";
        jsonFile << "}";
        jsonFile.close();

        //ямли
        std::ofstream yamlFile(baseFilename + ".yaml");
        yamlFile << yaml_exporter_->exportAccounts(accounts) << "\n";
        yamlFile << yaml_exporter_->exportCategories(categories) << "\n";
        yamlFile << yaml_exporter_->exportOperations(operations);
        yamlFile.close();

        std::cout << "Data exported to " << baseFilename << ".* (CSV, JSON, YAML)\n";
    }

    //разноформатный иморт
    void importData(const std::string& filename) {
        std::unique_ptr<DataImporter> importer;
        
        if (filename.find(".csv") != std::string::npos) {
            importer = std::make_unique<CSVDataImporter>();
        } else if (filename.find(".json") != std::string::npos) {
            importer = std::make_unique<JSONDataImporter>();
        } else if (filename.find(".yaml") != std::string::npos || filename.find(".yml") != std::string::npos) {
            importer = std::make_unique<YAMLDataImporter>();
        } else {
            std::cerr << "Unsupported file format: " << filename << std::endl;
            return;
        }

        importer->importData(filename, account_service_, category_service_, operation_service_);
    }
};

#endif