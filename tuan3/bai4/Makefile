TARGET = bai4
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size
CPU = cortex-m3

CFLAGS = -mcpu=$(CPU) -mthumb -O0 -Wall -Wextra \
         -ffreestanding -fno-builtin \
         -fno-unwind-tables -fno-asynchronous-unwind-tables \
         -DSTM32F10X_MD -Iinclude

LDFLAGS = -mcpu=$(CPU) -mthumb -nostdlib \
          -T linker.ld -Wl,--gc-sections

OBJS = main.o startup.o

PROGRAMMER = $(HOME)/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_Programmer_CLI

all: $(TARGET).elf $(TARGET).bin
	$(SIZE) $(TARGET).elf

$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ -lgcc

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

startup.o: startup.s
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

flash: $(TARGET).bin
	$(PROGRAMMER) -c port=SWD -w $(TARGET).bin 0x08000000 -v -rst

clean:
	rm -f $(OBJS) $(TARGET).elf $(TARGET).bin

size: $(TARGET).elf
	$(SIZE) $(TARGET).elf

.PHONY: all flash clean size
