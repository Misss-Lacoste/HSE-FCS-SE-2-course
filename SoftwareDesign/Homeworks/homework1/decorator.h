#ifndef DECORATOR_H
#define DECORATOR_H

#include "command.h"
#include <chrono>
#include <iostream>

class TimedCommand : public Command { //время выполнения команд(причем любой)
private:
    std::unique_ptr<Command> command_;
    std::string name_;

public:
    TimedCommand(std::unique_ptr<Command> command, const std::string& name)
        : command_(std::move(command)), name_(name) 
        {
            
        }

    void execute() override {
        auto start = std::chrono::high_resolution_clock::now();
        command_->execute();
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Command '" << name_ << "' executed in " << duration.count() << " ms\n";
    }

    void undo() override {
        command_->undo();
    }
};

#endif