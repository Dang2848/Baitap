.syntax unified
.cpu cortex-m3
.thumb

.global _estack
.global Reset_Handler
.global USART1_IRQHandler
.global main

.global _sidata
.global _sdata
.global _edata
.global _sbss
.global _ebss

.section .isr_vector

.word _estack
.word Reset_Handler

.rept 51
.word 0
.endr

.word USART1_IRQHandler

.section .text
.thumb_func

Reset_Handler:

    ldr r0, =_sidata
    ldr r1, =_sdata
    ldr r2, =_edata

copy_data:
    cmp r1, r2
    bcs zero_bss

    ldr r3, [r0]
    str r3, [r1]

    adds r0, r0, #4
    adds r1, r1, #4

    b copy_data

zero_bss:

    ldr r1, =_sbss
    ldr r2, =_ebss
    movs r3, #0

clear_bss:
    cmp r1, r2
    bcs start_main

    str r3, [r1]
    adds r1, r1, #4

    b clear_bss

start_main:

    bl main

hang:
    b hang
