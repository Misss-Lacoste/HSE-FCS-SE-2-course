#ifndef COMMAND_H
#define COMMAND_H

#include <memory>

class Command { //паттерн - команда, ставим запросы в очередь, чуть дальше там стек сделаем
public:
    virtual ~Command() = default;
    virtual void execute() = 0; //выполняем команду
    virtual void undo() = 0; //отменяем команду
};

#endif