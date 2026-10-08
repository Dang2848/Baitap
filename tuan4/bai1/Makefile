CC      = arm-none-eabi-gcc
AS      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size
NM      = arm-none-eabi-nm

PROGRAMMER = $(HOME)/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_Programmer_CLI

TARGET = freertos_bai1

INCLUDES = \
    -Iinclude \
    -IFreeRTOS/Kernel/include \
    -IFreeRTOS/Kernel/portable/GCC/ARM_CM3

CFLAGS = \
    -mcpu=cortex-m3 \
    -mthumb \
    -std=c11 \
    -O0 \
    -Wall \
    -Wextra \
    -ffreestanding \
    -fno-builtin \
    -fno-unwind-tables \
    -fno-asynchronous-unwind-tables \
    -ffunction-sections \
    -fdata-sections \
    -DSTM32F10X_MD \
    $(INCLUDES)

ASFLAGS = \
    -mcpu=cortex-m3 \
    -mthumb \
    $(INCLUDES)

LDFLAGS = \
    -mcpu=cortex-m3 \
    -mthumb \
    -nostdlib \
    -T linker.ld \
    -Wl,--gc-sections


KERNEL_OBJECTS = \
    FreeRTOS/Kernel/tasks.o \
    FreeRTOS/Kernel/list.o \
    FreeRTOS/Kernel/portable/MemMang/heap_4.o \
    FreeRTOS/Kernel/portable/GCC/ARM_CM3/port.o


SYSTEM_OBJECTS = \
    include/system_stm32f10x.o


OBJECTS = \
    main.o \
    $(SYSTEM_OBJECTS) \
    $(KERNEL_OBJECTS) \
    startup.o


all: $(TARGET).elf $(TARGET).bin


main.o: main.c include/stm32f10x.h include/FreeRTOSConfig.h
	$(CC) $(CFLAGS) -c $< -o $@


include/system_stm32f10x.o: include/system_stm32f10x.c include/stm32f10x.h
	$(CC) $(CFLAGS) -c $< -o $@


FreeRTOS/Kernel/tasks.o: FreeRTOS/Kernel/tasks.c
	$(CC) $(CFLAGS) -c $< -o $@


FreeRTOS/Kernel/list.o: FreeRTOS/Kernel/list.c
	$(CC) $(CFLAGS) -c $< -o $@


FreeRTOS/Kernel/portable/MemMang/heap_4.o: FreeRTOS/Kernel/portable/MemMang/heap_4.c
	$(CC) $(CFLAGS) -c $< -o $@


FreeRTOS/Kernel/portable/GCC/ARM_CM3/port.o: FreeRTOS/Kernel/portable/GCC/ARM_CM3/port.c
	$(CC) $(CFLAGS) -c $< -o $@


startup.o: startup.s linker.ld
	$(AS) $(ASFLAGS) -c $< -o $@


$(TARGET).elf: $(OBJECTS) linker.ld
	$(CC) $(LDFLAGS) $(OBJECTS) -lgcc -o $@


$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@
	$(SIZE) $<


check-kernel:
	@test -f FreeRTOS/Kernel/tasks.c || (echo "Missing FreeRTOS Kernel. Run ./setup_freertos.sh"; exit 1)
	@test -f FreeRTOS/Kernel/list.c || (echo "Missing FreeRTOS Kernel. Run ./setup_freertos.sh"; exit 1)
	@test -f FreeRTOS/Kernel/portable/MemMang/heap_4.c || (echo "Missing heap_4.c"; exit 1)
	@test -f FreeRTOS/Kernel/portable/GCC/ARM_CM3/port.c || (echo "Missing ARM_CM3 port"; exit 1)
	@echo "FreeRTOS Kernel OK"


info: $(TARGET).elf
	$(SIZE) $(TARGET).elf
	$(NM) $(TARGET).elf | grep -E "SystemInit|vTaskStartScheduler|xTaskCreate|vPortSVCHandler|xPortPendSVHandler|xPortSysTickHandler|SVC_Handler|PendSV_Handler|SysTick_Handler" || true


flash: $(TARGET).bin
	$(PROGRAMMER) -c port=SWD -w $(TARGET).bin 0x08000000 -v -rst


clean:
	rm -f \
	    main.o \
	    startup.o \
	    $(SYSTEM_OBJECTS) \
	    $(KERNEL_OBJECTS) \
	    $(TARGET).elf \
	    $(TARGET).bin


.PHONY: all clean flash check-kernel info
