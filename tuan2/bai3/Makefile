TARGET = firmware
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
PROGRAMMER = $(HOME)/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_Programmer_CLI

CFLAGS = -mcpu=cortex-m3 -mthumb -O0 -Wall -Wextra -ffreestanding -fno-builtin \
         -fno-unwind-tables -fno-asynchronous-unwind-tables -Iinclude -DSTM32F10X_MD
LDFLAGS = -mcpu=cortex-m3 -mthumb -nostdlib -T linker.ld -Wl,--gc-sections

all: $(TARGET).bin

$(TARGET).elf: startup.s main.c linker.ld
	$(CC) $(CFLAGS) $(LDFLAGS) startup.s main.c -lgcc -o $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

flash: $(TARGET).bin
	$(PROGRAMMER) -c port=SWD -w $(TARGET).bin 0x08000000 -v -rst

clean:
	rm -f $(TARGET).elf $(TARGET).bin

.PHONY: all flash clean
