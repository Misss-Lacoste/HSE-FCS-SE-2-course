#ifndef IMPORT_EXPORT_COMMANDS_H
#define IMPORT_EXPORT_COMMANDS_H

#include "command.h"
#include "import_export.h"
#include "operation_service.h"
#include <iostream>

class ExportDataCommand : public Command {
private:
    DataImportExportService& service_;
    std::string filename_;

public:
    ExportDataCommand(DataImportExportService& service, const std::string& filename)
        : service_(service), filename_(filename) {}

    void execute() override {
        service_.exportAllData(filename_);
        std::cout << "Export completed successfully\n";
    }

    void undo() override {
        std::cout << "Export operation cannot be undone\n";
    }
};

class ImportDataCommand : public Command {
private:
    DataImportExportService& service_;
    std::string filename_;

public:
    ImportDataCommand(DataImportExportService& service, const std::string& filename)
        : service_(service), filename_(filename) {}

    void execute() override {
        service_.importData(filename_);
        std::cout << "Import completed successfully\n";
    }

    void undo() override {
        std::cout << "Import operation cannot be undone\n";
    }
};

class RecalculateBalancesCommand : public Command {
private:
    OperationService& service_;

public:
    RecalculateBalancesCommand(OperationService& service) : service_(service) {}

    void execute() override {
        service_.recalculateAllBalances();
        std::cout << "All balances recalculated\n";
    }

    void undo() override {
        std::cout << "Balance recalculation cannot be undone\n";
    }
};

#endif