.data #начало секции данных. Объявляем переменные и строки
	delimoe: .word 0 #объявл. 4-байтовой переменной, инициализируем ее нулём
	delitel: .word 0 #тоже 4-байт. переменная, сначала инициализированная нулем. будет хранить модуль делителя
	delimoe_sign: .byte 0 # 1-байтовая переменная для хранения знака делимого
	delitel_sign: .byte 0 #1-байтовая переменная для хранения знака делителя
	quotient: .word 0 #переменная для хранения частного
	quotient_sign: .byte 0 #для знака переменной
	remaining: .word 0 #храним остаток от деления
	remaining_sign: .byte 0 #знак остатка (совп. со знаком делимого)
	delimoe_s: .asciz "Enter delimoe: "
	delitel_s: .asciz "Enter delitel: "
	quotient_s: .asciz "Here's your quotient: "
	remaining_s: .asciz "\nIt's your ostatok: "
	exception: .asciz "Throw std::invalid_argument: "

.text #Subsequent items (instructions) stored in Text segment at next available address
main:
	la a0, delimoe_s #загружаем в а0 адрес строки о вводе делимого
	li a7, 4 #syscall о выводе строки, на которую указывет а0
	ecall
	li a7, 5 #syscall о чтении целого числа из консоли. Помещаем в a0
	ecall
	bgez a0, positive_delimoe # Branch if Greater than or Equal to Zero : Branch to statement at label if t1 >= 0. если а0>=0, то переходим к метке positive_delimoe, то переходим к метке positive_delimoe
	j negative_delimoe # Jump to statement at label (типа как else, то есть переходим к случаю, если negative_delimoe
	
positive_delimoe: 
	la t0, delimoe
	sw a0, 0(t0) #сохраняем значение делимого в память
	li t5, 1 #устанавливаем знак делимого = 1
	la t0, delimoe_sign
	sb t5, 0(t0) #сохраняем знак делимого
	la t0, remaining_sign
	sb t5, 0(t0) #знак остатка
	j enter_delitel #переход к вводу делителя
		
negative_delimoe:
	neg t0, a0 #модуль делимого: t0 = -a0
	la t1, delimoe
	sw t0, 0(t1) #сохраняем модуль в память
	li t5, 0 #знак отрицательный
	la t1, delimoe_sign
	sb t5, 0(t1) #сохраняем знак делимого
	la t1, remaining_sign
	sb t5, 0(t1) #знак остатка тоже отрицательный
	j enter_delitel #переход к вводу делителя
  	
enter_delitel:
	la a0, delitel_s
	li a7, 4 #выводим строку о том, чтобы пользователь ввел делитель
	ecall
	li a7, 5 #сист вызов, читаем целое число в а0
	ecall
	beqz a0, division_by_zero #если а0 = 0 - деление на 0, переход к обработке ошибки
	bgtz a0, positive_delitel #если а0>0, переходим к обработке положительного делителя
	j negative_delitel
  		
positive_delitel:
	la t0, delitel
	sw a0, 0(t0) #сохраняем делитель
	li t5, 1 #положительный знак
	la t0, delitel_sign
	sb t5, 0(t0) #сохраняем знак делителя
	j set_quotient_sign

negative_delitel:
	neg t1, a0 #модуль делителя: t1 = -a0
	la t0, delitel
	sw t1, 0(t0) #сохраняем модуль делителя
	li t5, 0 #отрицательный знак
	la t0, delitel_sign
	sb t5, 0(t0) #сохраняем знак делителя
	j set_quotient_sign

set_quotient_sign:
	la t0, delimoe_sign
	lb t5, 0(t0) #загруз байт - знак делимого - t5
	la t0, delitel_sign
	lb t6, 0(t0) #загружаем знак делителя в t6
	xor t6, t6, t5 #если знаки разные, то результат отрицательный
	li t5, 1
	beq t5, t6, negative_quotient #если знаки разные, результат отрицательный
	la t0, quotient_sign
	sb t5, 0(t0) #сохраняем положительный знак частного
	j calculations

negative_quotient:
	li t5, 0
	la t0, quotient_sign
	sb t5, 0(t0) #сохраняем отрицательный знак частного
	j calculations

calculations:
	la t0, delimoe
	lw t0, 0(t0) #загружаем делимое
	la t1, delitel
	lw t1, 0(t1) #загружаем делитель
	li t2, 0 #счетчик частного, начинаем с нуля
	blt t0, t1, result_calc_finish #если делимое < делителя - деление закончено

calculating_process:
	sub t0, t0, t1 #вычитаем делитель: t0 = t0 - t1
	addi t2, t2, 1 #увеличиваем частное на 1
	bge t0, t1, calculating_process #если t0 >= t1 то родолжаем цикл
	j result_calc_finish #переход к завершению вычислений

division_by_zero:
	la a0, exception
	li a7, 4
	ecall #вывод сообщения о делении на 0
	j enter_delitel #повторный ввод делителя

result_calc_finish:
	la t3, quotient_sign
	lb t5, 0(t3) #загружаем знак результата
	beqz t5, change_quotient_sign #если знак отрицательный, меняем знак частного
	la t3, quotient
	sw t2, 0(t3) #сохраняем частное
	j remaining_calc_finish

change_quotient_sign:
	neg t2, t2 #меняем знак частного: t2 = -t2
	la t3, quotient
	sw t2, 0(t3) #сохраняем частное
	j remaining_calc_finish

remaining_calc_finish:
	la t3, remaining_sign
	lb t5, 0(t3) #загружаем знак остатка
	beqz t5, change_remaining_sign #если знак отрицательный, меняем знак остатка
	la t3, remaining
	sw t0, 0(t3) #сохраняем остаток
	j finish

change_remaining_sign:
	neg t0, t0 #меняем знак остатка: t0 = -t0
	la t3, remaining
	sw t0, 0(t3) #сохраняем остаток
	j finish

finish:
	la a0, quotient_s
	li a7, 4
	ecall #выводим "Here's your quotient: "
	la t0, quotient
	lw a0, 0(t0) #загружаем значение частного
	li a7, 1
	ecall #выводим значение частного
	
	la a0, remaining_s
	li a7, 4
	ecall #выводим "\nIt's your ostatok: "
	la t0, remaining
	lw a0, 0(t0) #загружаем значение остатка
	li a7, 1
	ecall #выводим значение остатка
	
	li a7, 10 #системный вызов 10 - завершение программы
	ecall