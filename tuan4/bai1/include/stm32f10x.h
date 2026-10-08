#ifndef STM32F10X_H
#define STM32F10X_H

#include <stdint.h>

#define __IO volatile

typedef struct
{
    __IO uint32_t CRL;
    __IO uint32_t CRH;
    __IO uint32_t IDR;
    __IO uint32_t ODR;
    __IO uint32_t BSRR;
    __IO uint32_t BRR;
    __IO uint32_t LCKR;
} GPIO_TypeDef;

typedef struct
{
    __IO uint32_t CR;
    __IO uint32_t CFGR;
    __IO uint32_t CIR;
    __IO uint32_t APB2RSTR;
    __IO uint32_t APB1RSTR;
    __IO uint32_t AHBENR;
    __IO uint32_t APB2ENR;
    __IO uint32_t APB1ENR;
    __IO uint32_t BDCR;
    __IO uint32_t CSR;
    __IO uint32_t AHBRSTR;
    __IO uint32_t CFGR2;
} RCC_TypeDef;

#define PERIPH_BASE          (0x40000000UL)
#define APB2PERIPH_BASE     (PERIPH_BASE + 0x10000UL)
#define AHBPERIPH_BASE      (PERIPH_BASE + 0x20000UL)

#define GPIOA_BASE           (APB2PERIPH_BASE + 0x0800UL)
#define RCC_BASE             (AHBPERIPH_BASE + 0x1000UL)

#define GPIOA                ((GPIO_TypeDef *)GPIOA_BASE)
#define RCC                  ((RCC_TypeDef *)RCC_BASE)

#define RCC_APB2ENR_IOPAEN  (1UL << 2)

#endif
