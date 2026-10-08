.syntax unified
.cpu cortex-m3
.thumb

.global g_pfnVectors
.global Reset_Handler
.extern main
.extern _sidata
.extern _sdata
.extern _edata
.extern _sbss
.extern _ebss

.section .isr_vector,"a",%progbits
.type g_pfnVectors, %object
g_pfnVectors:
    .word _estack
    .word Reset_Handler
    .word NMI_Handler
    .word HardFault_Handler
    .word MemManage_Handler
    .word BusFault_Handler
    .word UsageFault_Handler
    .word 0
    .word 0
    .word 0
    .word 0
    .word SVC_Handler
    .word DebugMon_Handler
    .word 0
    .word PendSV_Handler
    .word SysTick_Handler

    /* External IRQ 0..14 */
    .word WWDG_IRQHandler
    .word PVD_IRQHandler
    .word TAMPER_IRQHandler
    .word RTC_IRQHandler
    .word FLASH_IRQHandler
    .word RCC_IRQHandler
    .word EXTI0_IRQHandler
    .word EXTI1_IRQHandler
    .word EXTI2_IRQHandler
    .word EXTI3_IRQHandler
    .word EXTI4_IRQHandler
    .word DMA1_Channel1_IRQHandler
    .word DMA1_Channel2_IRQHandler
    .word DMA1_Channel3_IRQHandler
    .word DMA1_Channel4_IRQHandler

    /* IRQ 15..41 */
    .rept 27
    .word Default_Handler
    .endr

.section .text.Reset_Handler,"ax",%progbits
.thumb_func
Reset_Handler:
    ldr r0, =_sidata
    ldr r1, =_sdata
    ldr r2, =_edata
1:
    cmp r1, r2
    bcs 2f
    ldr r3, [r0], #4
    str r3, [r1], #4
    b 1b
2:
    ldr r1, =_sbss
    ldr r2, =_ebss
    movs r3, #0
3:
    cmp r1, r2
    bcs 4f
    str r3, [r1], #4
    b 3b
4:
    bl main
5:
    b 5b

.thumb_func
Default_Handler:
    b Default_Handler

.macro WEAK_DEFAULT name
    .weak \name
    .thumb_set \name, Default_Handler
.endm

WEAK_DEFAULT NMI_Handler
WEAK_DEFAULT HardFault_Handler
WEAK_DEFAULT MemManage_Handler
WEAK_DEFAULT BusFault_Handler
WEAK_DEFAULT UsageFault_Handler
WEAK_DEFAULT SVC_Handler
WEAK_DEFAULT DebugMon_Handler
WEAK_DEFAULT PendSV_Handler
WEAK_DEFAULT SysTick_Handler
WEAK_DEFAULT WWDG_IRQHandler
WEAK_DEFAULT PVD_IRQHandler
WEAK_DEFAULT TAMPER_IRQHandler
WEAK_DEFAULT RTC_IRQHandler
WEAK_DEFAULT FLASH_IRQHandler
WEAK_DEFAULT RCC_IRQHandler
WEAK_DEFAULT EXTI0_IRQHandler
WEAK_DEFAULT EXTI1_IRQHandler
WEAK_DEFAULT EXTI2_IRQHandler
WEAK_DEFAULT EXTI3_IRQHandler
WEAK_DEFAULT EXTI4_IRQHandler
WEAK_DEFAULT DMA1_Channel1_IRQHandler
WEAK_DEFAULT DMA1_Channel2_IRQHandler
WEAK_DEFAULT DMA1_Channel3_IRQHandler
WEAK_DEFAULT DMA1_Channel4_IRQHandler
