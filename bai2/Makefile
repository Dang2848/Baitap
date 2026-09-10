CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

CFLAGS = -mcpu=cortex-m3 -mthumb -Iinclude -DSTM32F10X_MD
LDFLAGS = -mcpu=cortex-m3 -mthumb -nostdlib -T linker.ld

all: bai2.bin

main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o

bai2.elf: startup.s main.o
	$(CC) $(LDFLAGS) startup.s main.o -o bai2.elf

bai2.bin: bai2.elf
	$(OBJCOPY) -O binary bai2.elf bai2.bin

flash: bai2.bin
	STM32_Programmer_CLI -c port=SWD -w bai2.bin 0x08000000 -v -rst

clean:
	rm -f main.o bai2.elf bai2.bin
