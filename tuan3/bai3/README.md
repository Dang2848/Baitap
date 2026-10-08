# BAI 3 TUAN 3 - NUT NHAN + UART + DMA

## Chuc nang
- PA0 la nut nhan, noi xuong GND.
- PA0 dung EXTI0 de phat hien canh xuong.
- ISR chi dat co `button_event`; phan tao chuoi va gui UART duoc xu ly trong `main()`.
- Moi lan nhan nut, gia tri tang lien tuc: 1, 2, 3, ..., 9, 10, 11, ..., n.
- Khong quay ve 0 sau 9.
- Chuoi duoc gui bang DMA1 Channel 4 -> USART1 TX.
- UART 115200, 8N1.

## Ban tin
```text
ELE1415-20261-03:15:BTN:1
ELE1415-20261-03:15:BTN:2
...
ELE1415-20261-03:15:BTN:9
ELE1415-20261-03:15:BTN:10
ELE1415-20261-03:15:BTN:11
...
ELE1415-20261-03:15:BTN:n
```

## Ket noi
- PA0 -> mot chan nut nhan.
- Chan con lai cua nut -> GND.
- PA9 (USART1_TX) -> RX cua USB-UART CH340.
- PA10 (USART1_RX) -> TX cua USB-UART CH340.
- GND STM32 -> GND CH340.

## Luong xu ly
PA0 -> EXTI0 -> EXTI0_IRQHandler() -> button_event -> main() -> button_count++ -> tx_buffer[] -> DMA1 Channel 4 -> USART1->DR -> PA9 -> USB-UART -> PC

## Build
```bash
make clean
make
```

## Nap
```bash
make flash
```

## UART
```bash
minicom -D /dev/ttyUSB0 -b 115200
```
