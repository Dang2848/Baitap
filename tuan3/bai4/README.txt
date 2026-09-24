BAI 4 - ADC + TIMER 100Hz + DMA ADC + DMA UART

KET NOI:
PA0  -> chan giua bien tro
PA9  -> RX USB-UART
PA10 -> TX USB-UART
GND  -> GND USB-UART
Hai dau bien tro -> 3.3V va GND

YEU CAU:
- TIM3 = 100Hz
- ADC = 100 mau / 1 giay
- DMA1 Channel 1: ADC -> RAM, circular
- Half Transfer sau 50 mau
- Transfer Complete sau 100 mau
- DMA1 Channel 4: RAM -> USART1 TX
- UART = 115200 8N1

BUILD:
make clean
make

NAP:
make flash

MINICOM:
minicom -D /dev/ttyUSB0 -b 115200

OUTPUT:
0
123
2048
4095
...
