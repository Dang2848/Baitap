CC = arm-none-eabi-gcc

CFLAGS = -mcpu=cortex-m3 -mthumb -O0 -Wall \
-DSTM32F10X_MD \
-I. \
-I./include

LDFLAGS = -mcpu=cortex-m3 -mthumb -nostdlib \
-Wl,-T,linker.ld

all: bai5.elf bai5.bin

main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o

startup.o: startup.s
	$(CC) $(CFLAGS) -c startup.s -o startup.o

bai5.elf: main.o startup.o
	$(CC) $(LDFLAGS) startup.o main.o -o bai5.elf

bai5.bin: bai5.elf
	arm-none-eabi-objcopy -O binary bai5.elf bai5.bin

flash: bai5.bin
	STM32_Programmer_CLI -c port=SWD -w bai5.bin 0x08000000 -v -rst

clean:
	rm -f *.o *.elf *.bin
uart:
	minicom -D /dev/ttyUSB0 -b 115200
