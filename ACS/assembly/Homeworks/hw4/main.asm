.include "macros.inc" #как и в плюсах подключаем файлы с макросами
.include "sqrt.inc"


.data
    input_number:    .asciz "Hello! Enter the number, please(positive): "
    input_tochnost:  .asciz "Enter the tochnost' of computation: "
    result:          .asciz "Square root is: "
    error:           .asciz "Error! The number must be positive!!\n"
    newline:         .asciz "\n"
    
    number:          .float 0.0
    tochnost:        .float 0.0
    result_val:      .float 0.0
    zero_float:      .float 0.0  #Константа 0.0 для сравнения при проверке положительности
    
.text
.globl main

main:
    INPUT_NUMBER     # ввод нашего числа # Макрос запрашивает число у пользователя и проверяет, что оно > 0
                     # Если число отрицательное, выводит ошибку и запрашивает снова
                     # Результат сохраняется в переменной number
    INPUT_TOCHNOST   # ввод точности
    COMPUTE_ROOT     # вычисляем корень
    OUTPUT_RESULT    # результат вычислений
    
    li a7, 10        # завершение программы
    ecall
