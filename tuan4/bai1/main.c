```c
#include <stdint.h>

#include "stm32f10x.h"

#include "FreeRTOS.h"
#include "task.h"


/* =========================================================
 * KHAI BAO CHAN LED
 *
 * LED1 -> PA0
 * LED2 -> PA1
 * LED3 -> PA2
 *
 * Moi bit trong GPIOA->ODR tuong ung voi mot chan GPIO.
 * ========================================================= */

#define LED1_PIN    ((uint16_t)(1U << 0))
#define LED2_PIN    ((uint16_t)(1U << 1))
#define LED3_PIN    ((uint16_t)(1U << 2))

/* Gom 3 chan LED thanh mot mask */
#define LED_MASK    ((uint32_t)(LED1_PIN | LED2_PIN | LED3_PIN))


/* =========================================================
 * CAC HAM THAY THE THU VIEN LIBC
 *
 * Project dung -nostdlib nen linker khong tu dong cung cap
 * cac ham nhu memset(), memcpy(), memmove(), memcmp().
 *
 * FreeRTOS co the su dung cac ham nay, vi vay ta tu dinh nghia
 * de tranh loi:
 *
 *     undefined reference to 'memset'
 * ========================================================= */


/* ---------------------------------------------------------
 * memset()
 *
 * Gan cung mot gia tri cho mot vung nho.
 *
 * ptr   : dia chi vung nho
 * value : gia tri can ghi
 * num   : so byte can ghi
 * --------------------------------------------------------- */

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


/* ---------------------------------------------------------
 * memcpy()
 *
 * Sao chep num byte tu src sang dest.
 * Hai vung nho khong duoc chong lan.
 * --------------------------------------------------------- */

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


/* ---------------------------------------------------------
 * memmove()
 *
 * Sao chep num byte tu src sang dest.
 *
 * Khac memcpy(), memmove() van hoat dong dung khi hai vung
 * nho bi chong lan.
 * --------------------------------------------------------- */

void *memmove(void *dest, const void *src, unsigned int num)
{
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;

    /*
     * Truong hop vung dich nam truoc vung nguon:
     * sao chep tu dau den cuoi.
     */
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
        /*
         * Truong hop vung nho co the chong lan:
         * sao chep tu cuoi ve dau de tranh ghi de du lieu.
         */
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


/* ---------------------------------------------------------
 * memcmp()
 *
 * So sanh num byte cua hai vung nho.
 *
 * Tra ve:
 *     0  : hai vung giong nhau
 *    -1  : a < b
 *     1  : a > b
 * --------------------------------------------------------- */

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


/* =========================================================
 * KHOI TAO GPIOA
 *
 * Su dung:
 *     PA0 -> LED1
 *     PA1 -> LED2
 *     PA2 -> LED3
 *
 * Che do:
 *     Output Push-Pull
 *     Toc do 50 MHz
 * ========================================================= */

static void LED_Init(void)
{
    /*
     * Bat clock cho GPIOA.
     *
     * IOPAEN = 1:
     * GPIOA duoc cap clock va co the truy cap cac thanh ghi
     * GPIOA.
     */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;


    /*
     * Cau hinh PA0, PA1, PA2.
     *
     * GPIOA->CRL dieu khien PA0 -> PA7.
     *
     * Moi chan GPIO su dung 4 bit:
     *
     * PA0 -> bit 3:0
     * PA1 -> bit 7:4
     * PA2 -> bit 11:8
     *
     * Gia tri 0x3:
     *
     * MODE = 11 -> Output 50 MHz
     * CNF  = 00 -> Output Push-Pull
     */
    GPIOA->CRL &= ~0x00000FFFUL;
    GPIOA->CRL |=  0x00000333UL;


    /*
     * Tat ca LED luc khoi dong.
     *
     * BRR = Bit Reset Register.
     * Ghi 1 vao bit nao thi chan GPIO tuong ung ve muc 0.
     */
    GPIOA->BRR = LED_MASK;
}


/* =========================================================
 * HAM DIEU KHIEN NHAP NHAY LED
 *
 * Dau vao:
 *
 *     pin
 *         Chan GPIO dieu khien LED.
 *
 *     frequency
 *         Tan so nhap nhay, don vi Hz.
 *
 * Cong thuc:
 *
 *     T = 1 / f
 *
 * Mot chu ky LED gom:
 *
 *     ON + OFF
 *
 * Do do thoi gian moi trang thai:
 *
 *     T/2 = 1 / (2*f)
 *
 * Quy doi sang ms:
 *
 *     delay_ms = 1000 / (2*f)
 *
 *
 * Vi du:
 *
 *     0.1 Hz -> delay = 5000 ms
 *     1 Hz   -> delay =  500 ms
 *     10 Hz  -> delay =   50 ms
 * ========================================================= */

static void LED_Blink(uint16_t pin, float frequency)
{
    uint32_t delay_ms;


    /*
     * Kiem tra tan so khong hop le.
     *
     * Neu frequency <= 0 thi xoa task hien tai.
     * Sau do return de tranh thuc hien phep chia cho 0.
     */
    if (frequency <= 0.0f)
    {
        vTaskDelete(NULL);
        return;
    }


    /*
     * Tinh thoi gian giu moi trang thai LED.
     *
     * 1000 ms / (2*f)
     */
    delay_ms =
        (uint32_t)(1000.0f / (2.0f * frequency));


    /*
     * Vong lap dieu khien LED.
     *
     * LED ON
     * -> cho mot nua chu ky
     * -> LED OFF
     * -> cho mot nua chu ky
     * -> lap lai
     */
    while (1)
    {
        /*
         * Bat LED.
         *
         * BSS Register:
         * ghi 1 vao bit -> GPIO = 1.
         */
        GPIOA->BSRR = (uint32_t)pin;

        /*
         * Tam dung task trong delay_ms.
         *
         * vTaskDelay() khong chiem CPU nhu delay bang vong lap.
         * Task se vao trang thai Blocked de Scheduler co the
         * chuyen sang task khac.
         */
        vTaskDelay(pdMS_TO_TICKS(delay_ms));


        /*
         * Tat LED.
         *
         * BRR:
         * ghi 1 vao bit -> GPIO = 0.
         */
        GPIOA->BRR = (uint32_t)pin;

        /*
         * Cho het nua chu ky con lai.
         */
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}


/* =========================================================
 * TASK DIEU KHIEN LED1
 *
 * PA0 -> 0.1 Hz
 *
 * Chu ky:
 *     T = 1 / 0.1 = 10 giay
 *
 * Thoi gian:
 *     ON  = 5 giay
 *     OFF = 5 giay
 * ========================================================= */

static void LED1_Task(void *argument)
{
    /*
     * Task nay khong su dung tham so argument.
     */
    (void)argument;


    /*
     * Goi ham dieu khien LED chung.
     */
    LED_Blink(
        LED1_PIN,
        0.1f
    );
}


/* =========================================================
 * TASK DIEU KHIEN LED2
 *
 * PA1 -> 1 Hz
 *
 * Chu ky:
 *     T = 1 / 1 = 1 giay
 *
 * Thoi gian:
 *     ON  = 500 ms
 *     OFF = 500 ms
 * ========================================================= */

static void LED2_Task(void *argument)
{
    (void)argument;

    LED_Blink(
        LED2_PIN,
        1.0f
    );
}


/* =========================================================
 * TASK DIEU KHIEN LED3
 *
 * PA2 -> 10 Hz
 *
 * Chu ky:
 *     T = 1 / 10 = 0.1 giay
 *
 * Thoi gian:
 *     ON  = 50 ms
 *     OFF = 50 ms
 * ========================================================= */

static void LED3_Task(void *argument)
{
    (void)argument;

    LED_Blink(
        LED3_PIN,
        10.0f
    );
}


/* =========================================================
 * HAM MAIN
 *
 * Trinh tu:
 *
 *     1. Khoi tao GPIO
 *     2. Tao Task 1
 *     3. Tao Task 2
 *     4. Tao Task 3
 *     5. Khoi dong FreeRTOS Scheduler
 * ========================================================= */

int main(void)
{
    /*
     * Khoi tao GPIOA.
     */
    LED_Init();


    /*
     * Tao Task 1:
     *
     * PA0 -> 0.1 Hz
     *
     * 256 la kich thuoc stack cua task,
     * don vi word cua MCU.
     */
    (void)xTaskCreate(
        LED1_Task,
        "LED1",
        256U,
        NULL,
        1U,
        NULL
    );


    /*
     * Tao Task 2:
     *
     * PA1 -> 1 Hz
     */
    (void)xTaskCreate(
        LED2_Task,
        "LED2",
        256U,
        NULL,
        1U,
        NULL
    );


    /*
     * Tao Task 3:
     *
     * PA2 -> 10 Hz
     */
    (void)xTaskCreate(
        LED3_Task,
        "LED3",
        256U,
        NULL,
        1U,
        NULL
    );


    /*
     * Khoi dong FreeRTOS Scheduler.
     *
     * Sau lenh nay Scheduler se quan ly CPU va phan chia
     * thoi gian cho 3 task LED.
     */
    vTaskStartScheduler();


    /*
     * Neu chuong trinh chay den day thi Scheduler khong
     * khoi dong duoc.
     *
     * Binh thuong ham nay khong duoc thuc thi.
     */
    while (1)
    {
    }
}
```

