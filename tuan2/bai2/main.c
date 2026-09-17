#include "stm32f10x.h"
#include <stdint.h>

/* DELAY UOC LUONG BANG VONG LAP */
static void delay_loop(volatile uint32_t n)
{
    while (n > 0U)
    {
        n--;
    }
}

/* HAM MAIN: CAU HINH PA0..PA2 LAM OUTPUT VA DAO TRANG THAI LED */
int main(void)
{
    /* Bat clock GPIOA tren bus APB2. */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

    /* Dat PA0, PA1, PA2 la output push-pull, toc do 2MHz. */
    GPIOA->CRL &= ~0xFFFU;
    GPIOA->CRL |=  0x222U;
    GPIOA->ODR &= ~0x7U;

    while (1)
    {
        /* PA0: dao LED, tre dai de tao chu ky cham. */
        GPIOA->ODR ^= (1U << 0);
        delay_loop(2500000U);

        /* PA1: dao LED, tre ngan hon PA0. */
        GPIOA->ODR ^= (1U << 1);
        delay_loop(250000U);

        /* PA2: dao LED, tre ngan hon PA1. */
        GPIOA->ODR ^= (1U << 2);
        delay_loop(25000U);
    }
}
