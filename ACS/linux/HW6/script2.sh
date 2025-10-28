#!/bin/bash 
echo "Please, enter a number: " #пвывод сообщения в скобках(аналог print в питоне или printf в СИ)
read num  #считываем введенное пользователем число

if [ $num -gt 0 ]; then #условие проверяем в кв скобках, причем не забыть сдклать после и перед такими скобками пробелы
                        # $num - аналог & в Си, то есть сравниваеи знач переменной с нулем. Флажок -gt = greater than. Then побуждает к выводу последующего сообщения
    echo "$num is positive" #если условие истинной, то выводим сообщение
elif [ $num -lt 0 s ]; then #aka else if. Тут проверяем флажком -ls = less than 0
    echo "$num is negative"
else                        #else как в сишных языках
    echo "$num is a zero"
fi                          #конец цикла