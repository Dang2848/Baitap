#include "stm32f10x.h"
#include <stdint.h>

/* CAU HINH UART1 DE GUI KET QUA ADC. */
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
/* GUI MOT KY TU QUA USART1. */
static void uart_putc(char c)
{
    while (!(USART1->SR & USART_SR_TXE)) {}
    USART1->DR = (uint8_t)c;
}
/* GUI CHUOI QUA USART1. */
static void uart_puts(const char *s)
{
    while (*s) uart_putc(*s++);
}
/* DELAY UOC LUONG BANG VONG LAP THUONG */
static void delay(volatile uint32_t n)
{
    while (n > 0U)
    {
        n--;
    }
}

/* CAU HINH ADC1 DOC KENH 0 TAI CHAN PA0. */
static void adc_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_ADC1EN;
    /* PA0 analog input */
    GPIOA->CRL &= ~0xFU;
    /* ADC clock = PCLK2/6 = 1.33MHz */
    RCC->CFGR = (RCC->CFGR & ~(3U << 14)) | (2U << 14);
    ADC1->CR1 = 0;
    ADC1->CR2 = ADC_CR2_ADON;
    ADC1->SMPR2 = (7U << 0); 
    ADC1->SQR1 = 0;
    ADC1->SQR3 = 0;         
    delay(10000U);
    ADC1->CR2 |= ADC_CR2_ADON;
    delay(10000U);
    ADC1->CR2 |= ADC_CR2_CAL;
    while (ADC1->CR2 & ADC_CR2_CAL) {}
}

/* BAT DAU CHUYEN DOI VA DOC GIA TRI ADC. */
static uint16_t adc_read(void)
{
    ADC1->CR2 |= ADC_CR2_ADON;
    while (!(ADC1->SR & ADC_SR_EOC)) {}
    return (uint16_t)ADC1->DR;
}

/* DOI SO NGUYEN SANG CHUOI SO VA GUI UART. */
static void send_u32(uint32_t v)
{
    char d[10]; uint32_t n=0;
    if (!v) { uart_putc('0'); return; }
    while (v && n<10U) { d[n++]=(char)('0'+v%10U); v/=10U; }
    while (n) uart_putc(d[--n]);
}

/* HAM MAIN: DOC ADC, DOI SANG MILIVOLT VA GUI UART DINH KY */
int main(void)
{
    uart_init();
    adc_init();
    uart_puts("\\r\\nADC PA0 ready; report periodically\\r\\n");

    while (1)
    {
        /* Lay mau ADC 12-bit (0..4095). */
        uint32_t raw = adc_read();

        /* Quy doi gia tri ADC sang mV voi Vref uoc tinh 3.3V. */
        uint32_t mv = (raw * 3300U) / 4095U;

        /* Gui gia tri ADC va dien ap qua UART. */
        uart_puts("ADC=");
        send_u32(raw);
        uart_puts("  Voltage=");
        send_u32(mv / 1000U);
        uart_putc('.');
        if ((mv % 1000U) < 100U) uart_putc('0');
        if ((mv % 1000U) < 10U) uart_putc('0');
        send_u32(mv % 1000U);
        uart_puts(" V\\r\\n");

        /* Tao khoang nghi xap xi 1 giay; can hieu chinh theo clock/build. */
        delay(8000000U);
    }
}
