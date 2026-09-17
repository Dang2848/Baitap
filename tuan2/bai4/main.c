#include "stm32f10x.h"
#include <stdint.h>

/* HAM MAIN: CAU HINH TIM3 PHAT PWM TREN 4 CHAN GPIO. */\n
int main(void)
{
    /* BAT CLOCK GPIOA/GPIOB/AFIO VA TIMER TIM3. 
    TIM3_CH1..CH4: PA6, PA7, PB0, PB1 */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    /* PA6/PA7 AF push-pull 50MHz */
    GPIOA->CRL &= ~((0xFU<<24)|(0xFU<<28));
    GPIOA->CRL |=  ((0xBU<<24)|(0xBU<<28));
    /* PB0/PB1 AF push-pull 50MHz */
    GPIOB->CRL &= ~((0xFU<<0)|(0xFU<<4));
    GPIOB->CRL |=  ((0xBU<<0)|(0xBU<<4));

    /* DAT TAN SO PWM VA CAC DUTY CYCLE. 
    Timer clock 8MHz: PSC=7 -> 1MHz; ARR=999 -> 1kHz */
    TIM3->PSC=7U;
    TIM3->ARR=999U;
    TIM3->CCR1=100U; /* 10% */
    TIM3->CCR2=300U; /* 30% */
    TIM3->CCR3=500U; /* 50% */
    TIM3->CCR4=700U; /* 70% */

    /* CHON PWM MODE 1, CHO PHEP PRELOAD VA BAT CAC KENH. 
    PWM mode 1 + preload for CH1/2 and CH3/4 */
    
    TIM3->CCMR1=(6U<<4)|(1U<<3)|(6U<<12)|(1U<<11);
    TIM3->CCMR2=(6U<<4)|(1U<<3)|(6U<<12)|(1U<<11);
    TIM3->CCER= (1U<<0)|(1U<<4)|(1U<<8)|(1U<<12);
    TIM3->CR1=TIM_CR1_ARPE;
    TIM3->EGR=TIM_EGR_UG;
    TIM3->CR1|=TIM_CR1_CEN;
    while (1) {}
}
