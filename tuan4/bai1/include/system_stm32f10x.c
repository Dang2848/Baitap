#include <stdint.h>
#include "stm32f10x.h"

// Dia chi thanh ghi FLASH ACR
#define FLASH_ACR (*(volatile uint32_t *)0x40022000UL)

// RCC_CR
#define RCC_CR_HSION    (1UL << 0)
#define RCC_CR_HSIRDY   (1UL << 1)
#define RCC_CR_HSEON    (1UL << 16)
#define RCC_CR_HSERDY   (1UL << 17)
#define RCC_CR_PLLON    (1UL << 24)
#define RCC_CR_PLLRDY   (1UL << 25)

// RCC_CFGR
#define RCC_CFGR_SW             (3UL << 0)
#define RCC_CFGR_SWS            (3UL << 2)
#define RCC_CFGR_HPRE           (15UL << 4)
#define RCC_CFGR_PPRE1          (7UL << 8)
#define RCC_CFGR_PPRE2          (7UL << 11)
#define RCC_CFGR_ADCPRE         (3UL << 14)
#define RCC_CFGR_PLLSRC         (1UL << 16)
#define RCC_CFGR_PLLXTPRE       (1UL << 17)
#define RCC_CFGR_PLLMULL        (15UL << 18)

// Gia tri cau hinh
#define RCC_CFGR_PPRE1_DIV2     (4UL << 8)
#define RCC_CFGR_PPRE2_DIV1     (0UL << 11)
#define RCC_CFGR_ADCPRE_DIV6    (2UL << 14)
#define RCC_CFGR_PLLSRC_HSE     (1UL << 16)
#define RCC_CFGR_PLLMULL9       (7UL << 18)
#define RCC_CFGR_SW_PLL         (2UL << 0)

// FLASH ACR
#define FLASH_ACR_LATENCY       (7UL << 0)
#define FLASH_ACR_PRFTBE        (1UL << 4)

void SystemInit(void)
{
    // Bat HSI
    RCC->CR |= RCC_CR_HSION;

    // Cho HSI san sang
    while ((RCC->CR & RCC_CR_HSIRDY) == 0UL)
    {
    }

    // Tat PLL va HSE truoc khi cau hinh
    RCC->CR &= ~(RCC_CR_PLLON | RCC_CR_HSEON);

    // Reset cau hinh RCC
    RCC->CFGR = 0UL;

    // Bat HSE 8 MHz
    RCC->CR |= RCC_CR_HSEON;

    // Cho HSE san sang
    while ((RCC->CR & RCC_CR_HSERDY) == 0UL)
    {
    }

    // Cau hinh Flash cho 72 MHz
    FLASH_ACR &= ~FLASH_ACR_LATENCY;
    FLASH_ACR |= FLASH_ACR_PRFTBE;
    FLASH_ACR |= 2UL;

    // Cau hinh bus
    RCC->CFGR &= ~(RCC_CFGR_SW |
                   RCC_CFGR_HPRE |
                   RCC_CFGR_PPRE1 |
                   RCC_CFGR_PPRE2 |
                   RCC_CFGR_ADCPRE |
                   RCC_CFGR_PLLSRC |
                   RCC_CFGR_PLLXTPRE |
                   RCC_CFGR_PLLMULL);

    // APB1 = 72 / 2 = 36 MHz
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;

    // APB2 = 72 MHz
    RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;

    // ADC clock = 72 / 6 = 12 MHz
    RCC->CFGR |= RCC_CFGR_ADCPRE_DIV6;

    // PLL = HSE x 9 = 72 MHz
    RCC->CFGR |= RCC_CFGR_PLLSRC_HSE;
    RCC->CFGR |= RCC_CFGR_PLLMULL9;

    // Bat PLL
    RCC->CR |= RCC_CR_PLLON;

    // Cho PLL san sang
    while ((RCC->CR & RCC_CR_PLLRDY) == 0UL)
    {
    }

    // Chuyen SYSCLK sang PLL
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL;

    // Cho den khi PLL tro thanh SYSCLK
    while ((RCC->CFGR & RCC_CFGR_SWS) != (2UL << 2))
    {
    }
}
