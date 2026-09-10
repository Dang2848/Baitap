.syntax unified
.cpu cortex-m3
.thumb

.global Reset_Handler
.global __stack_top

.section .isr_vector,"a",%progbits
.word __stack_top
.word Reset_Handler + 1

.section .text
.type Reset_Handler, %function

Reset_Handler:
    bl main

Loop:
    b Loop

