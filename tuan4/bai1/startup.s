.syntax unified
.cpu cortex-m3
.thumb

.global g_pfnVectors
.global Reset_Handler
.global Default_Handler

.extern main
.extern SVC_Handler
.extern PendSV_Handler
.extern SysTick_Handler

.extern __stack_top
.extern __data_load
.extern __data_start
.extern __data_end
.extern __bss_start
.extern __bss_end

.section .isr_vector,"a",%progbits
.type g_pfnVectors, %object
.align 2

g_pfnVectors:
    .word __stack_top
    .word Reset_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word 0
    .word 0
    .word 0
    .word 0
    .word SVC_Handler
    .word Default_Handler
    .word 0
    .word PendSV_Handler
    .word SysTick_Handler

    /* STM32F103 medium-density: 43 IRQ */
    .rept 43
    .word Default_Handler
    .endr

.size g_pfnVectors, . - g_pfnVectors

.section .text.Reset_Handler,"ax",%progbits
.type Reset_Handler, %function
.thumb_func
Reset_Handler:
    /* Copy .data tu Flash sang RAM */
    ldr r0, =__data_load
    ldr r1, =__data_start
    ldr r2, =__data_end

1:
    cmp r1, r2
    bcs 2f
    ldr r3, [r0]
    str r3, [r1]
    adds r0, r0, #4
    adds r1, r1, #4
    b 1b

2:
    /* Xoa .bss */
    ldr r1, =__bss_start
    ldr r2, =__bss_end
    movs r3, #0

3:
    cmp r1, r2
    bcs 4f
    str r3, [r1]
    adds r1, r1, #4
    b 3b

4:
    bl main

5:
    b 5b

.size Reset_Handler, . - Reset_Handler

.section .text.Default_Handler,"ax",%progbits
.type Default_Handler, %function
.thumb_func
Default_Handler:
6:
    b 6b
.size Default_Handler, . - Default_Handler
