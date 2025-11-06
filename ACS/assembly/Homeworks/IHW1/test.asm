# test.asm - Тестовая программа для проверки подпрограмм
# Запускается отдельно от основной программы

.include "io.asm"
.include "math.asm"

.data
# Сообщения для тестирования
test_msg:       .asciz "=== ТЕСТ "
test_passed:    .asciz " ПРОЙДЕН"
test_failed:    .asciz " ПРОВАЛЕН" 
test_newline:   .asciz "\n"
avg_msg:        .asciz " Среднее арифметическое значение = " 

# Тестовые данные
test1_A: .word 1, 2, 3, 4, 5      # Исходный массив
test1_N: .word 5                   # Размер массива
test1_B: .word 0, 0, 0, 0, 0      # Массив для результата
# Ожидаемый результат: [1, 2, 3, 3, 3] (среднее = 3)

test2_A: .word 10, 20, 30         # Исходный массив
test2_N: .word 3                   # Размер массива  
test2_B: .word 0, 0, 0            # Массив для результата
# Ожидаемый результат: [10, 20, 20] (среднее = 20)

# ДОБАВЛЕННЫЕ ТЕСТЫ
test3_A: .word 5                  # Один элемент
test3_N: .word 1                  # Размер массива
test3_B: .word 0                  # Массив для результата
# Ожидаемый результат: [5] (среднее = 5)

test4_A: .word 3, 3, 3            # Все одинаковые
test4_N: .word 3                  # Размер массива
test4_B: .word 0, 0, 0            # Массив для результата
# Ожидаемый результат: [3, 3, 3] (среднее = 3)

test5_A: .word -1, -2, 6          # С отрицательными
test5_N: .word 3                  # Размер массива
test5_B: .word 0, 0, 0            # Массив для результата
# Ожидаемый результат: [-1, -2, 1] (среднее = 1)

test6_A: .word 1, 2               # Дробное среднее (1.5)
test6_N: .word 2                  # Размер массива
test6_B: .word 0, 0               # Массив для результата
# Ожидаемый результат: [1, 1] (среднее = 1.5 → 1 при округлении)

.text
.globl test_main  # Переименовали, чтобы не конфликтовать с main

test_main:
    # Тест 1: обычный случай
    la a0, test_msg
    li a7, 4
    ecall
    li a0, 1
    li a7, 1
    ecall
    
    # Вычисление среднего для теста 1
    la a0, test1_A
    lw a1, test1_N
    call calculate_avg
    fmv.s fs0, fa0      # Сохраняем среднее значение в fs0
    
    # Вывод среднего значения для наглядности
    la a0, avg_msg
    li a7, 4
    ecall
    fmv.s fa0, fs0
    li a7, 2            # Код системного вызова для печати float
    ecall
    la a0, test_newline
    li a7, 4
    ecall
    
    # Формирование массива B для теста 1
    la a0, test1_A
    la a1, test1_B
    lw a2, test1_N
    fmv.s fa0, fs0      # аналогично
    call form_array_B
    
    # Вывод результатов теста 1
    la a0, test1_A
    lw a1, test1_N
    call print_array    # вызов подпроги
    la a0, test_newline
    li a7, 4
    ecall
    
    la a0, test1_B
    lw a1, test1_N
    call print_array    # вызов подпроги
    la a0, test_newline
    li a7, 4
    ecall
    la a0, test_newline  # Дополнительный перевод строки для разделения тестов
    li a7, 4
    ecall
    
    # Тест 2: все элементы больше среднего
    la a0, test_msg
    li a7, 4
    ecall
    li a0, 2            # Загружаем номер теста (2)
    li a7, 1
    ecall
    la a0, test_newline  # Загружаем адрес строки с переводом строки
    li a7, 4
    ecall
    
    # Вычисление среднего для теста 2
    la a0, test2_A
    lw a1, test2_N
    call calculate_avg
    fmv.s fs0, fa0      # Сохраняем среднее значение в регистре fs0
    
    # Вывод среднего значения для наглядности
    la a0, avg_msg
    li a7, 4
    ecall
    fmv.s fa0, fs0
    li a7, 2            # Код системного вызова для печати float
    ecall
    la a0, test_newline
    li a7, 4
    ecall
    
    # Формирование массива B для теста 2
    la a0, test2_A
    la a1, test2_B
    lw a2, test2_N
    fmv.s fa0, fs0      # Сохраняем среднее значение в регистре fs0
    call form_array_B
    
    # Вывод результатов теста 2
    la a0, test2_A #тут все аналогично
    lw a1, test2_N
    call print_array
    la a0, test_newline
    li a7, 4
    ecall
    
    la a0, test2_B
    lw a1, test2_N
    call print_array
    la a0, test_newline
    li a7, 4
    ecall
    la a0, test_newline  # Дополнительный перевод строки для разделения тестов
    li a7, 4
    ecall

    # ТЕСТ 3: Один элемент
    la a0, test_msg
    li a7, 4
    ecall
    li a0, 3            # Загружаем номер теста (3)
    li a7, 1
    ecall
    la a0, test_newline
    li a7, 4
    ecall
    
    # Вычисление среднего для теста 3
    la a0, test3_A
    lw a1, test3_N
    call calculate_avg
    fmv.s fs0, fa0      # Сохраняем среднее значение
    
    # Вывод среднего значения для наглядности
    la a0, avg_msg
    li a7, 4
    ecall
    fmv.s fa0, fs0
    li a7, 2            # Код системного вызова для печати float
    ecall
    la a0, test_newline
    li a7, 4
    ecall
    
    # Формирование массива B для теста 3
    la a0, test3_A
    la a1, test3_B
    lw a2, test3_N
    fmv.s fa0, fs0
    call form_array_B
    
    # Вывод результатов теста 3
    la a0, test3_A
    lw a1, test3_N
    call print_array
    la a0, test_newline
    li a7, 4
    ecall
    
    la a0, test3_B
    lw a1, test3_N
    call print_array
    la a0, test_newline
    li a7, 4
    ecall
    la a0, test_newline  # Дополнительный перевод строки для разделения тестов
    li a7, 4
    ecall

    # ТЕСТ 4: Все одинаковые элементы
    la a0, test_msg
    li a7, 4
    ecall
    li a0, 4            # Загружаем номер теста (4)
    li a7, 1
    ecall
    la a0, test_newline
    li a7, 4
    ecall
    
    # Вычисление среднего для теста 4
    la a0, test4_A
    lw a1, test4_N
    call calculate_avg
    fmv.s fs0, fa0      # Сохраняем среднее значение
    
    # Вывод среднего значения для наглядности
    la a0, avg_msg
    li a7, 4
    ecall
    fmv.s fa0, fs0
    li a7, 2            # Код системного вызова для печати float
    ecall
    la a0, test_newline
    li a7, 4
    ecall
    
    # Формирование массива B для теста 4
    la a0, test4_A
    la a1, test4_B
    lw a2, test4_N
    fmv.s fa0, fs0
    call form_array_B
    
    # Вывод результатов теста 4
    la a0, test4_A
    lw a1, test4_N
    call print_array
    la a0, test_newline
    li a7, 4
    ecall
    
    la a0, test4_B
    lw a1, test4_N
    call print_array
    la a0, test_newline
    li a7, 4
    ecall
    la a0, test_newline  # Дополнительный перевод строки для разделения тестов
    li a7, 4
    ecall

    # ТЕСТ 5: С отрицательными числами
    la a0, test_msg
    li a7, 4
    ecall
    li a0, 5            # Загружаем номер теста (5)
    li a7, 1
    ecall
    la a0, test_newline
    li a7, 4
    ecall
    
    # Вычисление среднего для теста 5
    la a0, test5_A
    lw a1, test5_N
    call calculate_avg
    fmv.s fs0, fa0      # Сохраняем среднее значение
    
    # Вывод среднего значения для наглядности
    la a0, avg_msg
    li a7, 4
    ecall
    fmv.s fa0, fs0
    li a7, 2            # Код системного вызова для печати float
    ecall
    la a0, test_newline
    li a7, 4
    ecall
    
    # Формирование массива B для теста 5
    la a0, test5_A
    la a1, test5_B
    lw a2, test5_N
    fmv.s fa0, fs0
    call form_array_B
    
    # Вывод результатов теста 5
    la a0, test5_A
    lw a1, test5_N
    call print_array
    la a0, test_newline
    li a7, 4
    ecall
    
    la a0, test5_B
    lw a1, test5_N
    call print_array
    la a0, test_newline
    li a7, 4
    ecall
    la a0, test_newline  # Дополнительный перевод строки для разделения тестов
    li a7, 4
    ecall

    # ТЕСТ 6: Дробное среднее значение 
    la a0, test_msg
    li a7, 4
    ecall
    li a0, 6            # Загружаем номер теста (6)
    li a7, 1
    ecall
    la a0, test_newline
    li a7, 4
    ecall
    
    # Вычисление среднего для теста 6
    la a0, test6_A
    lw a1, test6_N
    call calculate_avg
    fmv.s fs0, fa0      # Сохраняем среднее значение
    
    # Вывод среднего значения для наглядности
    la a0, avg_msg
    li a7, 4
    ecall
    fmv.s fa0, fs0
    li a7, 2            # Код системного вызова для печати float
    ecall
    la a0, test_newline
    li a7, 4
    ecall
    
    # Формирование массива B для теста 6
    la a0, test6_A
    la a1, test6_B
    lw a2, test6_N
    fmv.s fa0, fs0
    call form_array_B
    
    # Вывод результатов теста 6
    la a0, test6_A
    lw a1, test6_N
    call print_array
    la a0, test_newline
    li a7, 4
    ecall
    
    la a0, test6_B
    lw a1, test6_N
    call print_array
    la a0, test_newline
    li a7, 4
    ecall
    
    li a7, 10           # Завершение тестовой программы
    ecall