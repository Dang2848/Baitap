TARGET = bai3_uart_dma

CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

CFLAGS = -mcpu=cortex-m3 -mthumb -O0 -Wall -Wextra \
         -ffreestanding -fno-builtin -fno-unwind-tables \
         -fno-asynchronous-unwind-tables -Iinclude \
         -DSTM32F10X_MD

LDFLAGS = -mcpu=cortex-m3 -mthumb -nostdlib -T linker.ld \
          -Wl,--gc-sections

all: $(TARGET).elf $(TARGET).bin

$(TARGET).elf: main.o startup.o linker.ld
	$(CC) $(LDFLAGS) main.o startup.o -lgcc -o $@
	$(SIZE) $@

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

startup.o: startup.s
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

flash: $(TARGET).bin
	$(HOME)/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_Programmer_CLI \
	-c port=SWD -w $(TARGET).bin 0x08000000 -v -rst

clean:
	rm -f *.o *.elf *.bin
