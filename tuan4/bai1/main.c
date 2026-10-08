#include <stdint.h>

#include "stm32f10x.h"
#include "system_stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
void SystemInit(void);
/*
=========================================================
KHAI BAO CHAN LED

LED1 -> PA0
LED2 -> PA1
LED3 -> PA2
=========================================================
*/

#define LED1_PIN    ((uint16_t)(1U << 0))
#define LED2_PIN    ((uint16_t)(1U << 1))
#define LED3_PIN    ((uint16_t)(1U << 2))

#define LED_MASK    ((uint32_t)(LED1_PIN | LED2_PIN | LED3_PIN))


/*
=========================================================
CAC HAM THAY THE THU VIEN LIBC

Do project su dung -nostdlib nen tu dinh nghia cac ham
xu ly bo nho ma FreeRTOS co the su dung.
=========================================================
*/

void *memset(void *ptr, int value, unsigned int num)
{
    unsigned char *p = (unsigned char *)ptr;

    while (num > 0U)
    {
        *p++ = (unsigned char)value;
        num--;
    }

    return ptr;
}


void *memcpy(void *dest, const void *src, unsigned int num)
{
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;

    while (num > 0U)
    {
        *d++ = *s++;
        num--;
    }

    return dest;
}


void *memmove(void *dest, const void *src, unsigned int num)
{
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;

    if (d < s)
    {
        while (num > 0U)
        {
            *d++ = *s++;
            num--;
        }
    }
    else
    {
        d += num;
        s += num;

        while (num > 0U)
        {
            *--d = *--s;
            num--;
        }
    }

    return dest;
}


int memcmp(const void *a, const void *b, unsigned int num)
{
    const unsigned char *p = (const unsigned char *)a;
    const unsigned char *q = (const unsigned char *)b;

    while (num > 0U)
    {
        if (*p != *q)
        {
            return (*p < *q) ? -1 : 1;
        }

        p++;
        q++;
        num--;
    }

    return 0;
}


/*
=========================================================
KHOI TAO GPIOA

PA0, PA1, PA2:
Output Push-Pull
Toc do 50 MHz
=========================================================
*/

static void LED_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

    GPIOA->CRL = 0x33333333UL;

    GPIOA->BRR = 0x00FFUL;
}


/*
=========================================================
CAU TRUC CAU HINH TASK

Moi task co:

pin
    Chan LED

frequency
    Tan so nhap nhay Hz
=========================================================
*/

typedef struct
{
    uint16_t pin;
    float frequency;
} LED_Config;


/*
=========================================================
HAM CHUNG DIEU KHIEN NHAP NHAY LED

Dau vao:

pin
    Chan LED

frequency
    Tan so nhap nhay, don vi Hz

Cong thuc:

T = 1 / f

Moi chu ky gom:

ON + OFF

Do do:

T_on = T_off = 1 / (2*f)

Doi sang ms:

delay_ms = 1000 / (2*f)
=========================================================
*/

static void LED_Blink(uint16_t pin, float frequency)
{
    uint32_t delay_ms;


    /*
    Kiem tra tan so hop le
    */
    if (frequency <= 0.0f)
    {
        vTaskDelete(NULL);
        return;
    }


    /*
    Tinh thoi gian cho moi trang thai LED
    */
    delay_ms =
        (uint32_t)(1000.0f / (2.0f * frequency));


    /*
    Lap vo han:

    ON
    Delay
    OFF
    Delay
    Lap lai
    */
    while (1)
    {
        /*
        Bat LED
        */
        GPIOA->BSRR = (uint32_t)pin;


        /*
        Tam dung task.
        Scheduler co the chay task khac.
        */
        vTaskDelay(pdMS_TO_TICKS(delay_ms));


        /*
        Tat LED
        */
        GPIOA->BRR = (uint32_t)pin;


        /*
        Cho het nua chu ky
        */
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}


/*
=========================================================
TASK CHUNG CHO CA 3 LED

Moi task nhan mot cau hinh khac nhau:

LED1 -> PA0 + 0.1 Hz
LED2 -> PA1 + 1 Hz
LED3 -> PA2 + 10 Hz
=========================================================
*/

static void LED_Task(void *argument)
{
    LED_Config *config = (LED_Config *)argument;


    /*
    Goi ham nhap nhay chung
    */
    LED_Blink(
        config->pin,
        config->frequency
    );


    /*
    LED_Blink khong return trong truong hop binh thuong
    */
    while (1)
    {
    }
}


/*
=========================================================
CAU HINH CHO 3 LED

Chi can thay doi frequency tai day de thay doi
tan so nhap nhay cua tung LED.
=========================================================
*/


static LED_Config led1_config =
{
    LED1_PIN,
   0.1f
};

static LED_Config led2_config =
{
    LED2_PIN,
    1.0f
};

static LED_Config led3_config =
{
    LED3_PIN,
    10.0f
};


/*
=========================================================
HAM MAIN

Trinh tu:

1. Khoi tao clock
2. Khoi tao GPIO
3. Tao 3 task
4. Khoi dong Scheduler
=========================================================
*/

int main(void)
{
    /*
    Khoi tao clock he thong
    configCPU_CLOCK_HZ = 72 MHz
    */
    SystemInit();


    /*
    Khoi tao GPIOA
    */
    LED_Init();


    /*
    =====================================================
    TAO TASK 1

    PA0
    0.1 Hz

    &led1_config duoc truyen vao LED_Task
    =====================================================
    */

    (void)xTaskCreate(
        LED_Task,
        "LED1",
        256U,
        &led1_config,
        1U,
        NULL
    );


    /*
    =====================================================
    TAO TASK 2

    PA1
    1 Hz
    =====================================================
    */

    (void)xTaskCreate(
        LED_Task,
        "LED2",
        256U,
        &led2_config,
        1U,
        NULL
    );


    /*
    =====================================================
    TAO TASK 3

    PA2
    10 Hz
    =====================================================
    */

    (void)xTaskCreate(
        LED_Task,
        "LED3",
        256U,
        &led3_config,
        1U,
        NULL
    );


    /*
    Khoi dong FreeRTOS Scheduler

    Tu day Scheduler bat dau quan ly 3 task
    */
    vTaskStartScheduler();


    /*
    Scheduler binh thuong khong return
    */
    while (1)
    {
    }

    return 0;
}
