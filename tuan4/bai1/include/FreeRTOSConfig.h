#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* STM32F103C8T6: su dung HSI mac dinh 8 MHz */
#define configCPU_CLOCK_HZ                       ( 8000000UL )
#define configTICK_RATE_HZ                       ( 1000U )

#define configUSE_PREEMPTION                     1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION  0
#define configUSE_TICKLESS_IDLE                  0

#define configMAX_PRIORITIES                     ( 5U )
#define configMINIMAL_STACK_SIZE                 ( 128U )
#define configMAX_TASK_NAME_LEN                  ( 16U )
#define configUSE_16_BIT_TICKS                   0
#define configIDLE_SHOULD_YIELD                 1

#define configSUPPORT_DYNAMIC_ALLOCATION         1
#define configSUPPORT_STATIC_ALLOCATION          0
#define configTOTAL_HEAP_SIZE                   ( 10U * 1024U )

#define configUSE_IDLE_HOOK                      0
#define configUSE_TICK_HOOK                      0
#define configCHECK_FOR_STACK_OVERFLOW           0
#define configUSE_MALLOC_FAILED_HOOK             0

#define INCLUDE_vTaskPrioritySet                 1
#define INCLUDE_uxTaskPriorityGet                1
#define INCLUDE_vTaskDelete                      1
#define INCLUDE_vTaskCleanUpResources            1
#define INCLUDE_vTaskSuspend                     1
#define INCLUDE_vTaskDelayUntil                  1
#define INCLUDE_vTaskDelay                       1

#define configPRIO_BITS                          4U
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15U
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5U

#define configKERNEL_INTERRUPT_PRIORITY \
    ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8U - configPRIO_BITS) )

#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
    ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8U - configPRIO_BITS) )

#define vPortSVCHandler       SVC_Handler
#define xPortPendSVHandler    PendSV_Handler
#define xPortSysTickHandler   SysTick_Handler

#define configASSERT(x)                         \
    do {                                        \
        if ((x) == 0) {                         \
            taskDISABLE_INTERRUPTS();          \
            for (;;) {                          \
            }                                   \
        }                                       \
    } while (0)

#endif
