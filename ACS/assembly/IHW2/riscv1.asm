.data #начинаем стандартно с секции данных

prompt:         .asciz "Enter the number of iterations to calculate Pi: "
result_msg:     .asciz "Calculated result of Pi: "
error_msg:      .asciz "Warning! Enter the positive number for iterations!\n"
newline:        .asciz "\n"

#данные для тестирования
test_header:    .asciz "Here are automatic tests represented\n"
test_iter_msg:  .asciz "Iterations: "
test_pi_msg:    .asciz " Pi:  "
test_error_msg: .asciz " Error rate: "
test_percent:   .asciz "% "
test_pass:      .asciz "DONE\n"
test_fail:      .asciz "Mistakes have been occured\n"
divider:        .asciz " | "

#тестовые наборы данных для итераций
test_iterations: .word 10, 100, 1000, 10000 #создадим массив тестовых данных для кол-ва итераций
test_count:     .word 4 #сам счётчик тестов(их 4, сделаем как степени десятки)

#константы по ТЗ в double
six:            .double 6.0
one:            .double 1.0
hundred:        .double 100.0
pi_reference:   .double 3.141592653589793 #эталонное значение
threshold_0_1:  .double 0.1      #пороговое значение 0.1% (согласно ТЗ)

#начало секции кода
.text
.globl main #отметим глобальной меточкой

#внизу будут макросы для ввода/вывода
#макрос для вывода строки
.macro PRINT_STRING(%str_addr)
    la a0, %str_addr #загружаем адрес строки в a0
    li a7, 4 #в регистр а7 кладём код системного вызова
    ecall #системный вызов
.end_macro

#макрос для вывода целого числа
.macro PRINT_INT(%reg)
    mv a0, %reg #копируем значение из регистра в а0
    li a7, 1 #код вывода целого числа
    ecall
.end_macro

#макрос для вывода double
.macro PRINT_DOUBLE(%freg)
    fmv.d fa0, %freg #копируем дабл значение в fa0
    li a7, 3 #код вывода дабла
    ecall
.end_macro

#макрос для ввода целого числа
.macro INPUT_INT_TO(%reg)
    li a7, 5 #код ввода целого цисла
    ecall
    mv %reg, a0 #сохраняем результат в указанный регистр
.end_macro

#макросы-обёртки для подпрограмм

#макрос-обёртка для вычисления Pi с автоматическим сохранением регистров
.macro COMPUTE_PI(%iter_reg, %result_freg)
    addi sp, sp, -24 #резервируем 24 байта в стеке
    sw ra, 0(sp) #сохраняем адрес возврата
    fsd f8, 8(sp) # сохраняем регистр f8
    sw a0, 16(sp) # сохраняем регистр a0
    
    mv a0, %iter_reg   #передаем количество итераций - передаем параметр в a0
    call compute_pi_zeta #вызываем функцию
    fmv.d %result_freg, fa0 #сохраняем результат
    
    lw a0, 16(sp) #почти аналогично, как выше(работаем с регистрами, загружаем адреса возврата в регистры стека)
    fld f8, 8(sp)
    lw ra, 0(sp)
    addi sp, sp, 24
.end_macro

#макрос-обертка для вычисления погрешности
.macro COMPUTE_ERROR(%computed_freg, %error_freg)
    addi sp, sp, -16
    fsd f8, 0(sp)
    sw ra, 8(sp)
    
    fmv.d fa0, %computed_freg
    call calculate_error
    fmv.d %error_freg, fa0
    
    lw ra, 8(sp)
    fld f8, 0(sp)
    addi sp, sp, 16
.end_macro

#основная прога
main:
    call initialize_constants #инициализация констант
    call automated_testing #вызываем авто-тесты
    call interactive_mode #интерактивный режим
    li a7, 10 #завершаем прогу системным вызовом завершения программы
    ecall
    
#подпрограммы

#инициализация констант в регистрах
initialize_constants:
    #загрузка констант из памяти в регистры
    la t0, six #загружаем адрес константы sixв t0
    fld f20, 0(t0) #можно представить как f20=6.0
    la t0, one
    fld f21, 0(t0) #f.21=1.0 а тут загружаем double значение в f20
    la t0, hundred
    fld f22, 0(t0) #f22 = 100.0
    la t0, pi_reference
    fld f23, 0(t0) #f23=Pi (эталонное значение)
    ret

#вычисление π через дзета-функцию Римана
#ищем ряд, потом берем из него корень, в отчете будет расписана работа с формулрй
#аргументы: a0-кол-во итераций (через регистр)
#возвращает: fa0-вычисленное значение Pi
compute_pi_zeta:
    addi sp, sp, -48 #работа со стеком
    sw ra, 0(sp)  #схраняем адрес возврата
    fsd f24, 8(sp) #локальная переменная: текущее слагаемое
    fsd f25, 16(sp) #локальная переменная: сумма ряда
    fsd f26, 24(sp)   #локальная переменная: k как double
    sw s0, 32(sp) #сохраняем s0 (счетчик)
    sw s1, 40(sp)  #сохраняем s1 (количество итераций)
    mv s1, a0 #сохраняем количество итераций
    
 #инициализация суммы ряда
    fcvt.d.w f25, zero #sum=0.0
    li s0, 1 #k=1

compute_loop:
    bgt s0, s1, compute_pi_done  #если k>n, то выыходим из цикла
    
    #1 / k в квадрате
    fcvt.d.w f26, s0    #k -> double
    fmul.d f26, f26, f26 #k в квадрате
    fdiv.d f24, f21, f26 #1/k в квадрате
    
    #добавляем к сумме
    fadd.d f25, f25, f24
    
    addi s0, s0, 1 #инкремент к
    j compute_loop

compute_pi_done:
    #вычисляем Pi = корень из суммы, умноженноу на 6
    fmul.d f25, f25, f20 #6*sum
    fsqrt.d fa0, f25 # корень из (6 * sum)
    
    #восстанавливаем регистры стека
    lw s1, 40(sp)
    lw s0, 32(sp)
    fld f26, 24(sp)
    fld f25, 16(sp)
    fld f24, 8(sp)
    lw ra, 0(sp)
    addi sp, sp, 48
    ret 
#вычисление погрешности в процентах
#аргументы: fa0 - вычисленное значение π (через регистр)
#возвращает: fa0 - погрешность в процентах
calculate_error:
    addi sp, sp, -24
    fsd f24, 0(sp)  #лкальная переменная
    sw ra, 8(sp)  #адрес возврата
    sw a0, 16(sp)  #сохраняем a0
    
    #абсолютная погрешность
    fsub.d f24, fa0, f23  #разница = computed- reference
    fabs.d f24, f24       # absolute_error = |разница|
    
    #относительная погрешность в процентах
    fdiv.d f24, f24, f23  # relative_error = absolute_error / reference
    fmul.d fa0, f24, f22  # error_percent = relative_error * 100%
    
    # Восстанавливаем регистры
    lw a0, 16(sp)
    lw ra, 8(sp)
    fld f24, 0(sp)
    addi sp, sp, 24
    ret

#Функция выполнения одного теста
#a0 - кол-во итераций
run_single_test:
    addi sp, sp, -32
    sw ra, 0(sp)
    fsd f8, 8(sp)    # вычисленный π
    fsd f9, 16(sp)   # погрешность
    sw a0, 24(sp)    # сохраняем итерации
    
    # Вычисляем π
    call compute_pi_zeta
    fmv.d f8, fa0
    
    # Вычисляем погрешность
    fmv.d fa0, f8
    call calculate_error
    fmv.d f9, fa0
    
    # Выводим результаты теста
    PRINT_STRING(test_iter_msg)
    lw a0, 24(sp)
    PRINT_INT(a0)
    
    PRINT_STRING(test_pi_msg)
    PRINT_DOUBLE(f8)
    
    PRINT_STRING(test_error_msg)
    PRINT_DOUBLE(f9)
    PRINT_STRING(test_percent)
    
    # Проверяем условие точности (0.1%)
    la t0, threshold_0_1
    fld f10, 0(t0)
    flt.d t0, f9, f10
    
    bnez t0, test_passed
    PRINT_STRING(test_fail)
    j test_end
    
test_passed:
    PRINT_STRING(test_pass)
    
test_end:
    lw a0, 24(sp)
    fld f9, 16(sp)
    fld f8, 8(sp)
    lw ra, 0(sp)
    addi sp, sp, 32
    ret

#автоматическое тестирование
automated_testing:
    addi sp, sp, -16
    sw ra, 0(sp)
    sw s0, 4(sp) #указатель на тестовые данные
    sw s1, 8(sp) #счетчик тестов
    
    PRINT_STRING(newline)
    PRINT_STRING(test_header)
    PRINT_STRING(newline)
    
    #инициализация тестирования
    la s0, test_iterations
    lw s1, test_count
    
automated_test_loop:
    beqz s1, automated_test_done
    
    #загружаем количество итераций и запускаем тест
    lw a0, 0(s0)
    call run_single_test
    
    #следующий тестовый набор
    addi s0, s0, 4
    addi s1, s1, -1
    j automated_test_loop

automated_test_done:
    PRINT_STRING(newline)
    
    lw s1, 8(sp)
    lw s0, 4(sp)
    lw ra, 0(sp)
    addi sp, sp, 16
    ret

#интерактивный режим
interactive_mode:
    addi sp, sp, -32
    sw ra, 0(sp)
    fsd f24, 8(sp)      # Вычисленное знач Pi
    fsd f25, 16(sp)     #погрешность
    sw s0, 24(sp)       #кол-во итераций
    
    #ввод данных
    PRINT_STRING(prompt)
    INPUT_INT_TO(s0)
    
    #корректность ввода
    blez s0, interactive_error
    
    #вычисление Pi с использованием макроса-обёртки
    COMPUTE_PI(s0, f24)
    
    #вычисление погрешности
    COMPUTE_ERROR(f24, f25)
    
    #вывод результатов
    PRINT_STRING(result_msg)
    PRINT_DOUBLE(f24)
    PRINT_STRING(newline)
    
    #информация о погрешности
    PRINT_STRING(test_error_msg)
    PRINT_DOUBLE(f25)
    PRINT_STRING(test_percent)
    PRINT_STRING(newline)
    
    #проверка точности
    la t0, threshold_0_1
    fld f26, 0(t0)
    flt.d t0, f25, f26
    beqz t0, precision_warning
    j interactive_done

precision_warning:
    la a0, precision_warn_msg #выводим сообщение, если точность не достигнута
    li a7, 4
    ecall
    j interactive_done

interactive_error:
    PRINT_STRING(error_msg)

interactive_done:
    lw s0, 24(sp)
    fld f25, 16(sp)
    fld f24, 8(sp)
    lw ra, 0(sp)
    addi sp, sp, 32
    ret

.data
precision_warn_msg: .asciz "Warning! Precision is under 0.1%. Make bigger number of iterations.\n"