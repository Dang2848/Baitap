.syntax unified
.cpu cortex-m3
.thumb

.global __stack_top
.global _sidata
.global _sdata
.global _edata
.global _sbss
.global _ebss
.global Reset_Handler

.extern main
.extern DMA1_Channel1_IRQHandler
.extern DMA1_Channel4_IRQHandler

.section .isr_vector, "a", %progbits
.type g_pfnVectors, %object

g_pfnVectors:
    /* Core exceptions 0..15 */
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
    .word Default_Handler
    .word Default_Handler
    .word 0
    .word Default_Handler
    .word Default_Handler

    /* IRQ0..IRQ10 */
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler

    /* IRQ11 = DMA1 Channel 1 */
    .word DMA1_Channel1_IRQHandler

    /* IRQ12..IRQ13 */
    .word Default_Handler
    .word Default_Handler

    /* IRQ14 = DMA1 Channel 4 */
    .word DMA1_Channel4_IRQHandler

    /* IRQ15..IRQ42 */
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler

.section .text.Reset_Handler
.type Reset_Handler, %function

Reset_Handler:
    ldr r0, =_sidata
    ldr r1, =_sdata
    ldr r2, =_edata

copy_data:
    cmp r1, r2
    bcs clear_bss
    ldr r3, [r0]
    str r3, [r1]
    adds r0, r0, #4
    adds r1, r1, #4
    b copy_data

clear_bss:
    ldr r1, =_sbss
    ldr r2, =_ebss
    movs r3, #0

zero_bss:
    cmp r1, r2
    bcs call_main
    str r3, [r1]
    adds r1, r1, #4
    b zero_bss

call_main:
    bl main

hang:
    b hang

.type Default_Handler, %function
Default_Handler:
    b Default_Handler
