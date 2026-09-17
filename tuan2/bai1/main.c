#include "stm32f10x.h"
#include <stdint.h>

/* CAU HINH USART1, PA9 TX VA PA10 RX. */
static void uart_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;
    /* PA9 AF push-pull 50MHz; PA10 floating input */
    GPIOA->CRH &= ~((0xFU << 4) | (0xFU << 8));
    GPIOA->CRH |=  ((0xBU << 4) | (0x4U << 8));
    USART1->CR1 = 0;
    USART1->CR2 = 0;
    USART1->CR3 = 0;
    USART1->BRR = 0x45U; /* PCLK2=8MHz, 115200 baud */
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}
/* GUI MOT KY TU UART, DOI TXE SAN SANG. */
static void uart_putc(char c)
{
    while (!(USART1->SR & USART_SR_TXE)) {}
    USART1->DR = (uint8_t)c;
}
/*GUI CHUOI KY TU QUA UART. */
static void uart_puts(const char *s)
{
    while (*s) uart_putc(*s++);
}
/* TAO TRE BANG VONG LAP */
static void delay(volatile uint32_t n)
{
    while (n > 0U)
    {
        n--;
    }
}

#define RX_SIZE 128U
static char rx[RX_SIZE];
static uint32_t pos;

/* HAM MAIN: NHAN CHUOI DEN KY TU ! ROI GUI PHAN HOI. */
int main(void)
{
    uart_init();
    uart_puts("\r\nUART ready!\r\nEnter text, then press !\r\n");
    while (1)
    {
        if (USART1->SR & USART_SR_RXNE)
        {
            char c = (char)(USART1->DR & 0xFFU);
            if (c == '\r' || c == '\n') continue;
            if (c == '!')
            {
                rx[pos] = '\0';
                uart_puts("\r\nELE1415-2026-03 - Nhom 15: ");
                uart_puts(rx);
                uart_puts("\n\r");
                pos = 0;
            }
            else if (pos < RX_SIZE - 1U)
            {
                rx[pos++] = c;
            }
            else
            {
                pos = 0;
                uart_puts("\r\nError: input too long\r\n");
            }
        }
    }
}
