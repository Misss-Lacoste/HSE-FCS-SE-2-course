#!/bin/bash

add() {
    local result=$(( $1 + $2 )) #Функция сложения, объявляем локальную переменную, только внутри функции
    echo $result #выводим тот результат, котрый получился
}

subtract() { #f вычитания
    local result=$(( $1 - $2 )) #аналогично присваиваем матем.операции переменную
    echo $result #выводим значение переменной
}

multiply() { #f умножения
    local result=$(( $1 * $2 ))
    echo $result
}

divide() { #f деления
    if [ $2 -eq 0 ]; then #проверяем делить на то, чтоб он не явл. нулем
        echo "Error: Division by zero!" #иначе выбрасываем ошибку
        return 1 #и возвращаем 1
    fi
    local result=$(( $1 / $2 )) #тут сама процедура деления
    echo $result
}

#основная функция калькулятора - вызывает другие функции
calculator() {
    echo "Calculator"
    echo "Enter first number: "
    read num1
    echo "Enter second number: "
    read num2
    echo "Enter operation (+, -, *, /): "
    read operation
    
    case $operation in #аналог switch в C++
        "+")
            result=$(add $num1 $num2) #если ввели операцию +, то вызывается сумма
            ;; #какой милый синтаксис - конец ветки case
        "-")
            result=$(subtract $num1 $num2) #по аналогии, как выше написала
            ;;
        "*")
            result=$(multiply $num1 $num2)
            ;;
        "/")
            result=$(divide $num1 $num2)
            ;;
        *) 
            echo "Invalid operation" #ветка - все ост. случаи
            return 1
            ;;
    esac #конец конструкции case
    
    echo "Result: $num1 $operation $num2 = $result"
}

#Запуск калькулятора
calculator