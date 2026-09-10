CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

CFLAGS = -mcpu=cortex-m3 -mthumb -Iinclude -DSTM32F10X_MD
LDFLAGS = -mcpu=cortex-m3 -mthumb -nostdlib -T linker.ld

all: bai3.bin

main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o

bai3.elf: startup.s main.o
	$(CC) $(LDFLAGS) startup.s main.o -o bai3.elf

bai3.bin: bai3.elf
	$(OBJCOPY) -O binary bai3.elf bai3.bin

flash: bai3.bin
	STM32_Programmer_CLI -c port=SWD -w bai3.bin 0x08000000 -v -rst

clean:
	rm -f main.o bai3.elf bai3.bin

