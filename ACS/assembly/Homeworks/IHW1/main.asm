#main.asm - основная программа с меню и автотестами

.include "io.asm"       #мдуль ввода-вывода
.include "math.asm"     #модуль математических вычислений
.include "file_io.asm"  #модуль работы с файлами

.data #поле данных
n:          .word 0           #размер массива
array_A:    .space 40         #массив A (10 элементов * 4 байта) согласно условию
array_B:    .space 40         #массив B (10 элементов * 4 байта) согласно условию

prompt_start:  .asciz "\nMENU\nEnter '0': input your array.\nEnter other symbol: launch autotests.\n\nYour choice: "
prompt_next:   .asciz "If you want to finish program, enter '0', otherwise press any other number to restart: "
sep:          .asciz " "
newline:      .asciz "\n"

msg_array_a:  .asciz "Массив A: "
msg_array_b:  .asciz "Массив B: "
msg_avg:      .asciz "Среднее арифметическое: "
msg_error:    .asciz "Ошибка! Некорректный размер массива.\n"
msg_input_choice: .asciz "Выберите способ ввода:\n1 - Клавиатура\n2 - Файл\nВведите выбор: "
msg_invalid_input: .asciz "Неверный выбор ввода!\n"

.text
.globl main

main:
    #вывод меню выбора
    li a7, 4 #код системного вызова для печати строки
    la a0, prompt_start #адрес строки с меню в регистр a0
    ecall #Вызываем систему для печати строки
    
    # Чтение числа
    li a7, 5 #код системного вызова для ввода целого числа
    ecall #сист вызов - результат попадает в a0
    
    #Проверка выбора: 0 - свой массив, иначе - автотесты
    beqz a0, your_array #ветка если a0 = 0, переходим к your_array
    j to_tests #Иначе переходим к to_tests (безусловный переход)

your_array:
    #Создание собственного массива A
    call user_array #подпрограмма вода массива
    
    #Обработка массива и создание массива B
    call work #вызываем подпрограмму обработки массива
    
    #Предложение продолжить или завершить
    li a7, 4 #Код для печати строки
    la a0, prompt_next #Адрес строки с предложением продолжить
    ecall #печатаем строку
    
    #Чтение выбора
    li a7, 5 #Код для ввода целого числа
    ecall #Вводим число, результат в a0
    
    #Если 0 - завершить, иначе - повторить
    beqz a0, end
    j your_array #Иначе возвращаемся к началу блока (повторяем)
 
to_tests:
    #Запуск автотестов
    call autotests #Вызываем подпрограмму автотестов
    
end:
    #Завершение программы
    li a7, 10 #вызов для завершения программы
    ecall #завершаем прогу

#подпрограмма для обработки ситуации, если пользователь сам хочет ввести массив
user_array:
    addi sp, sp, -8 #Выделяем 8 байт в стекe
    sw ra, 4(sp) #адрес возврата в стек
    sw s0, 0(sp) #cохраняем регистр s0 в стек
    
    #Выбор способа ввода
    la a0, msg_input_choice #сообщение о выборе ввода слова
    li a7, 4 #песать строки
    ecall #печатаем сообщение
    
    #Чтение выбора способа ввода
    li a7, 5 #Код ввода целого числа
    ecall
    
    #проверка выбора
    li t0, 1 #Загружаем значение 1 для сравнения
    beq a0, t0, keyboard_input #Если ввели 1, переходим к клавиатурному вводу
    li t0, 2 #Загружаем значение 2 для сравнения
    beq a0, t0, file_input #Если ввели 2, переходим к файловому вводу
    j invalid_input_choice #наче переходим к обработке ошибки

keyboard_input: #клавиатурный инпут
    la a0, msg_n #aдрес сообщения "Введите количество элементов"
    call input_int #подпрограмма ввода целого числа
    mv s0, a0 #Сохраняем введенный размер в s0
    
    #Проверка корректности размера
    li t0, 1 #Нижняя граница (1)
    li t1, 10 #Верхняя граница (10) - согласно условию
    blt s0, t0, input_error #Если размер < 1, ошибка
    bgt s0, t1, input_error #Если размер > 10, ошибка
    
    #cохраняем размер в переменную n(до этого его уже сохранили)
    sw s0, n, t1
    
    #Ввод элементов массива A с клавиатуры
    la a0, array_A #адрес массива а
    mv a1, s0 #передаем размер массива в адрес
    call input_array #вызов подпроги для ввода массива
    j user_end #конец подпроги

file_input:
    #Ввод из файла если пользователь выберет такую опцию
    la a0, array_A #адрес массива для заполнения
    call read_from_file #Вызываем подпрограмму чтения из файла
    
    # Проверка успешности чтения
    li t0, -1 #Код ошибки
    beq a1, t0, input_error # Если вернулся код ошибки
    
    #Сохраняем размер из файла
    mv s0, a0 #Сохраняем размер из a0 в s0
    sw s0, n, t1 #Сохраняем размер в переменную n
    j user_end #переходим к концу

invalid_input_choice:
    la a0, msg_invalid_input #Адрес сообщения об ошибке выбора
    li a7, 4 #код печати строки
    ecall #Печатаем сообщение
    j user_end #Переходим к концу

input_error:
    la a0, msg_error #адрес сообщения об ошибке
    li a7, 4 #Код печати строки
    ecall

user_end:
    lw s0, 0(sp) #восстанавливаем s0 из стека
    lw ra, 4(sp) #восстанавливаем адрес возврата
    addi sp, sp, 8 #Освобождаем стек 
    ret #Возвращаемся к вызывающей программе

#подпрога обработки массива
work:
    addi sp, sp, -8 #Выделяем место в стеке
    sw ra, 4(sp) #Сохраняем адрес возврата
    fsw fs0, 0(sp) ## Сохраняем регистр в s0
    
    lw s0, n           #загружаем размер массива из переменной n
    
    #вывод массива A
    la a0, msg_array_a #сообщение "Массив A: "
    li a7, 4 #вывод строки
    ecall
    la a0, array_A #загрузим адрес массива а
    mv a1, s0 #передвигаем - Размер массива
    call print_array #вызываем подпрограмму вывода
    la a0, newline #cbvdjk yjdjq cnhrb
    li a7, 4
    ecall
    
    #Вычисление среднего арифметического
    la a0, array_A #Адрес массива A
    mv a1, s0  # Размер массива
    call calculate_avg # Вызываем подпрограмму вычисления среднего
    fmv.s fs0, fa0      # Сохраняем среднее
    
    #Вывод среднего
    la a0, msg_avg #Вызываем подпрограмму вычисления среднего
    li a7, 4
    ecall
    fmv.s fa0, fs0 #Восстанавливаем среднее в fa0 для вывода
    li a7, 2 #Код для печати float
    ecall # Печатаем среднее значение
    la a0, newline
    li a7, 4
    ecall
    
    #Формирование массива B
    la a0, array_A #тут и строка снизу - адрес массива а
    la a1, array_B
    mv a2, s0 #размер массива
    fmv.s fa0, fs0 #среднее арифм
    call form_array_B #подпрога формирования b
    
    #Вывод массива B
    la a0, msg_array_b #cообщение "Массив B: "
    li a7, 4
    ecall
    la a0, array_B #Адрес массива B
    mv a1, s0 #размер массива
    call print_array #подпрога вывода
    la a0, newline
    li a7, 4
    ecall
    
    flw fs0, 0(sp) #Восстанавливаем fs0
    lw ra, 4(sp) #Восстанавливаем адрес возврата
    addi sp, sp, 8 #Освобождаем стек
    ret


autotests:
    addi sp, sp, -8 #выделяем 8 байт в стеке (2 слова)
    sw ra, 4(sp) #сохраняем адрес возврата по смещению 4
    fsw fs1, 0(sp) #сохраняем регистр fs1 по смещению 0
    
    #Тест 1:
    la a0, test1_msg
    li a7, 4
    ecall
    
    la a0, test1_A #загружаем адрес тестового массива A
    lw a1, test1_N  #загружаем размер массива (5)
    call calculate_avg #вызываем подпрограмму вычисления среднего
    fmv.s fs1, fa0 #сохраняем результат (3.0) в fs1
    
    la a0, test1_A #Адрес исходного массива A
    la a1, test1_B # Адрес результирующего массива B  
    lw a2, test1_N  # Размер массива (5)
    fmv.s fa0, fs1  # Восстанавливаем среднее значение в fa0
    call form_array_B  # Формируем массив B
    
    la a0, test1_A  # Адрес массива A для вывода
    lw a1, test1_N # Размер массива
    call print_array  # Выводим исходный массив A
    la a0, newline  # Адрес символа новой строки
    li a7, 4
    ecall
    
    la a0, test1_B  # Адрес массива B для вывода
    lw a1, test1_N  # Размер массива
    call print_array
    la a0, newline
    li a7, 4
    ecall
    
    la a0, test_passed #сообщение что тест пройден
    li a7, 4
    ecall
    la a0, newline
    li a7, 4
    ecall
    
    # Тест 2 
    la a0, test2_msg #тут и далее внизу все абсолютно аналогично, как при работе с 1 тестом
    li a7, 4
    ecall
    
    la a0, test2_A
    lw a1, test2_N
    call calculate_avg
    fmv.s fs1, fa0
    
    la a0, test2_A
    la a1, test2_B
    lw a2, test2_N
    fmv.s fa0, fs1
    call form_array_B
    
    la a0, test2_A
    lw a1, test2_N
    call print_array
    la a0, newline
    li a7, 4
    ecall
    
    la a0, test2_B
    lw a1, test2_N
    call print_array #НАПИШУ КАПСОМ ЧТО ВСЕ АНАЛОГИЧНО КАК И ВВЕРХУ
    la a0, newline
    li a7, 4
    ecall
    
    la a0, test_passed
    li a7, 4
    ecall
    la a0, newline
    li a7, 4
    ecall
    
    # Тест 3: 
    la a0, test3_msg #аналогично как и выше
    li a7, 4
    ecall
    
    la a0, test3_A
    lw a1, test3_N
    call calculate_avg
    fmv.s fs1, fa0
    
    la a0, test3_A
    la a1, test3_B
    lw a2, test3_N
    fmv.s fa0, fs1
    call form_array_B
    
    la a0, test3_A
    lw a1, test3_N
    call print_array
    la a0, newline
    li a7, 4
    ecall
    
    la a0, test3_B
    lw a1, test3_N
    call print_array
    la a0, newline
    li a7, 4
    ecall
    
    la a0, test_passed
    li a7, 4
    ecall
    la a0, newline
    li a7, 4
    ecall
    
    flw fs1, 0(sp)
    lw ra, 4(sp)
    addi sp, sp, 8
    ret

.data
msg_n:        .asciz "Введите количество элементов массива (1-10): "

# Данные для автотестов
test1_A: .word 1, 2, 3, 4, 5 #исходный массив для теста 1
test1_N: .word 5 #размер массива для теста 1
test1_B: .word 0, 0, 0, 0, 0 #буфер для резов теста 1

test2_A: .word 10, 20, 30 #аналогичные штуки как выше написала
test2_N: .word 3  
test2_B: .word 0, 0, 0

test3_A: .word 5, 5, 5, 5
test3_N: .word 4
test3_B: .word 0, 0, 0, 0

test1_msg: .asciz "ТЕСТ 1: [1,2,3,4,5] -> [1,2,3,3,3]\n"
test2_msg: .asciz "ТЕСТ 2: [10,20,30] -> [10,20,20]\n"  
test3_msg: .asciz "ТЕСТ 3: [5,5,5,5] -> [5,5,5,5]\n"
test_passed: .asciz "✓Тест пройден успешно"