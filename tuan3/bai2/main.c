#include "stm32f10x.h"
#include <stdint.h>

/* 
  DINH NGHIA CHAN KET NOI
  PA4  -> MAX7219 CS/LOAD
  PA5  -> MAX7219 CLK
 PA7  -> MAX7219 DIN
 PA9  -> UART TX
 PA10 -> UART RX
*/

#define MAX_CS_HIGH()   (GPIOA->BSRR = (1U << 4))
#define MAX_CS_LOW()    (GPIOA->BRR  = (1U << 4))


/* 
  HAM DELAY
  Tao tre bang vong lap
 */

static void delay_ms(uint32_t ms)
{
    volatile uint32_t i;

    while (ms--)
    {
        for (i = 0U; i < 8000U; i++)
        {
        }
    }
}


/* 
 KHOI TAO UART1
 PA9  = TX
 PA10 = RX
 Baudrate = 115200
 PCLK2 = 8 MHz
*/

static void uart_init(void)
{
    /* Bat clock cho GPIOA va USART1 */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;

    /* PA9: TX, AF Push-Pull, 50MHz */
    GPIOA->CRH &= ~(0xFU << 4);
    GPIOA->CRH |=  (0xBU << 4);

    /* PA10: RX, Input Pull-up */
    GPIOA->CRH &= ~(0xFU << 8);
    GPIOA->CRH |=  (0x8U << 8);
    GPIOA->BSRR = (1U << 10);

    /* Cau hinh baudrate 115200 voi PCLK2 = 8MHz */
    USART1->BRR = 0x0045U;

    /* Bat USART, cho phep truyen va nhan */
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}


/* Gui mot ky tu qua UART */
static void uart_putc(char c)
{
    while ((USART1->SR & USART_SR_TXE) == 0U)
    {
    }

    USART1->DR = (uint16_t)c;
}


/* Gui chuoi ky tu qua UART */
static void uart_str(const char *str)
{
    while (*str != '\0')
    {
        uart_putc(*str++);
    }
}


/*
  KHOI TAO SPI1
  PA4 = CS
  PA5 = SCK
  PA7 = MOSI/DIN
  SPI Master, Mode 0, MSB first
*/

static void spi1_init(void)
{
    /* Bat clock cho GPIOA va SPI1 */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_SPI1EN;

    /* PA4: CS, Output Push-Pull, 50MHz */
    GPIOA->CRL &= ~(0xFU << 16);
    GPIOA->CRL |=  (0x3U << 16);

    /* PA5: SCK, Alternate Function Push-Pull, 50MHz */
    GPIOA->CRL &= ~(0xFU << 20);
    GPIOA->CRL |=  (0xBU << 20);

    /* PA7: MOSI, Alternate Function Push-Pull, 50MHz */
    GPIOA->CRL &= ~(0xFU << 28);
    GPIOA->CRL |=  (0xBU << 28);

    /* Dua CS len HIGH khi khong truyen */
    MAX_CS_HIGH();

    /* Cau hinh SPI Master, Software NSS, Mode 0, PCLK2/32 */
    SPI1->CR1 = SPI_CR1_MSTR | SPI_CR1_BR_2 | SPI_CR1_BR_0 |SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_SPE;
}


/* Gui mot byte du lieu qua SPI1 */
static void spi1_send(uint8_t data)
{
    /* Cho thanh ghi truyen trong */
    while ((SPI1->SR & SPI_SR_TXE) == 0U)
    {
    }

    SPI1->DR = data;

    /* Cho qua trinh truyen hoan tat */
    while ((SPI1->SR & SPI_SR_RXNE) == 0U)
    {
    }

    (void)SPI1->DR;
}


/* 
  GIAO TIEP VOI MAX7219
  Moi lenh gom dia chi thanh ghi va du lieu
*/

static void max7219_send(uint8_t reg, uint8_t data)
{
    MAX_CS_LOW();

    spi1_send(reg);
    spi1_send(data);

    MAX_CS_HIGH();
}


/* Xoa tat ca 8 hang cua ma tran LED */
static void max7219_clear(void)
{
    uint8_t i;

    for (i = 1U; i <= 8U; i++)
    {
        max7219_send(i, 0x00U);
    }
}


/* 
 *
 KHOI TAO MAX7219
 Do sang = 8/16, Scan Limit = 8 hang 
 */

static void max7219_init(void)
{
    /* Tam thoi tat MAX7219 trong luc cau hinh */
    max7219_send(0x0CU, 0x00U);

    /* Tat Decode Mode */
    max7219_send(0x09U, 0x00U);

    /* Dat do sang muc 8/16 */
    max7219_send(0x0AU, 0x08U);

    /* Su dung 8 hang */
    max7219_send(0x0BU, 0x07U);

    /* Chuyen sang che do hoat dong binh thuong */
    max7219_send(0x0CU, 0x01U);

    /* Xoa noi dung cu */
    max7219_clear();
}


/* 
 HIEN THI BITMAP 8x8
 Moi byte tuong ung voi mot hang cua LED Matrix
*/

static void max7219_display(const uint8_t *bitmap)
{
    uint8_t i;

    for (i = 0U; i < 8U; i++)
    {
        max7219_send((uint8_t)(i + 1U), bitmap[i]);
    }
}


/* 
 BANG MA HIEN THI SO 0 DEN 9
 Moi so gom 8 byte tuong ung 8 hang LED
 Bit 1 = LED sang
 Bit 0 = LED tat
*/

static const uint8_t font[10][8] =
{
    /* So 0 */
    {
        0x3CU, 0x66U, 0x6EU, 0x76U,
        0x66U, 0x66U, 0x3CU, 0x00U
    },

    /* So 1 */
    {
        0x18U, 0x38U, 0x18U, 0x18U,
        0x18U, 0x18U, 0x7EU, 0x00U
    },

    /* So 2 */
    {
        0x3CU, 0x66U, 0x06U, 0x0CU,
        0x18U, 0x30U, 0x7EU, 0x00U
    },

    /* So 3 */
    {
        0x3CU, 0x66U, 0x06U, 0x1CU,
        0x06U, 0x66U, 0x3CU, 0x00U
    },

    /* So 4 */
    {
        0x0CU, 0x1CU, 0x3CU, 0x6CU,
        0x7EU, 0x0CU, 0x0CU, 0x00U
    },

    /* So 5 */
    {
        0x7EU, 0x60U, 0x7CU, 0x06U,
        0x06U, 0x66U, 0x3CU, 0x00U
    },

    /* So 6 */
    {
        0x1CU, 0x30U, 0x60U, 0x7CU,
        0x66U, 0x66U, 0x3CU, 0x00U
    },

    /* So 7 */
    {
        0x7EU, 0x06U, 0x0CU, 0x18U,
        0x30U, 0x30U, 0x30U, 0x00U
    },

    /* So 8 */
    {
        0x3CU, 0x66U, 0x66U, 0x3CU,
        0x66U, 0x66U, 0x3CU, 0x00U
    },

    /* So 9 */
    {
        0x3CU, 0x66U, 0x66U, 0x3EU,
        0x06U, 0x0CU, 0x38U, 0x00U
    }
};


/* 
 CHUONG TRINH CHINH
 Hien thi tuan tu 0 -> 9
 Moi so giu trong khoang 1 giay
*/

int main(void)
{
    uint8_t number = 0U;

    /* Khoi tao cac ngoai vi */
    uart_init();
    spi1_init();
    max7219_init();

    /* Gui thong bao khoi dong qua UART */
    uart_str("\r\n");
    uart_str("STM32F103 + MAX7219 + 8x8\r\n");
    uart_str("BAI 2 - WEEK 03\r\n");

    while (1)
    {
        /* Hien thi so hien tai */
        max7219_display(font[number]);

        /* Gui so dang hien thi qua UART */
        uart_putc((char)('0' + number));
        uart_str("\r\n");

        /* Tao thoi gian giua hai lan doi so */
        delay_ms(100U);

        /* Tang so va quay lai 0 sau so 9 */
        number++;

        if (number >= 10U)
        {
            number = 0U;
        }
    }
}
