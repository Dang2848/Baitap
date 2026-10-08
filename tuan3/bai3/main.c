#include "stm32f10x.h"
#include "core_cm3.h"
#include <stdint.h>


/* BIEN SU KIEN NUT NHAN VA TRANG THAI DMA */
static volatile uint8_t button_event = 0U;
static volatile uint32_t button_count = 0U;
static volatile uint8_t dma_busy = 0U;

/* BO NHO CHUA CHUOI GUI QUA DMA */
static char tx_buffer[64];


/* DELAY DON GIAN */
static void delay_ms(uint32_t ms)
{
    volatile uint32_t i;

    while (ms > 0U)
    {
        for (i = 0U; i < 8000U; i++)
        {
            /* Tao thoi gian cho viec chong doi nut */
        }

        ms--;
    }
}


/*
 UART1 - PA9 TX, PA10 RX
 Baudrate: 115200
 PCLK2 = 8 MHz
 */
static void uart_init(void)
{
    /* Bat clock GPIOA va USART1 */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;

    /* PA9 = USART1_TX
     * Output AF push-pull, 50 MHz
     */
    GPIOA->CRH &= ~(0xFU << 4);
    GPIOA->CRH |=  (0xBU << 4);

    /* PA10 = USART1_RX
     * Input floating
     */
    GPIOA->CRH &= ~(0xFU << 8);
    GPIOA->CRH |=  (0x4U << 8);

    /* Baudrate 115200 voi PCLK2 = 8 MHz */
    USART1->BRR = 0x0045U;

    /* Cho phep USART, TX va RX */
    USART1->CR1 = USART_CR1_UE |
                  USART_CR1_TE |
                  USART_CR1_RE;
}


/*
 GUI 1 KY TU QUA UART
 Chi dung cho thong bao khoi dong.
 */
static void uart_putc(char c)
{
    while ((USART1->SR & USART_SR_TXE) == 0U)
    {
        /* Cho thanh ghi du lieu trong */
    }

    USART1->DR = (uint16_t)c;
}


/* GUI CHUOI KHOI DONG */
static void uart_str(const char *str)
{
    while (*str != '\0')
    {
        uart_putc(*str);
        str++;
    }
}


/*
 DOI SO NGUYEN KHONG DAU SANG CHUOI
 Ham nay ho tro ca 1 chu so va nhieu chu so:
 1, 2, ..., 9, 10, 11, ..., 100, ..., n
 */
static uint32_t uint32_to_str(uint32_t value, char *str)
{
    char temp[11];
    uint32_t n = 0U;
    uint32_t i;

    if (value == 0U)
    {
        str[0] = '0';
        return 1U;
    }

    while (value > 0U)
    {
        temp[n++] = (char)('0' + (value % 10U));
        value /= 10U;
    }

    for (i = 0U; i < n; i++)
    {
        str[i] = temp[n - 1U - i];
    }

    return n;
}


/*
 GPIO + EXTI0 CHO NUT NHAN
 PA0 la input pull-up.
 Nut nhan noi PA0 xuong GND.

 Tha nut : PA0 = 1
 Nhan nut: PA0 = 0

 */
static void button_init(void)
{
    /* Bat clock GPIOA va AFIO */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;

    /* PA0 = input pull-up/pull-down */
    GPIOA->CRL &= ~(0xFU << 0);
    GPIOA->CRL |=  (0x8U << 0);

    /* Chon pull-up */
    GPIOA->ODR |= (1U << 0);

    /* EXTI0 nhan tin hieu tu PA0 */
    AFIO->EXTICR[0] &= ~(0xFU << 0);

    /* Cho phep ngat EXTI0 */
    EXTI->IMR |= EXTI_IMR_MR0;

    /* Chi kich hoat khi co canh xuong: 1 -> 0 */
    EXTI->RTSR &= ~EXTI_RTSR_TR0;
    EXTI->FTSR |= EXTI_FTSR_TR0;

    /* Xoa co pending truoc khi cho phep ngat */
    EXTI->PR = EXTI_PR_PR0;

    /* Cho phep EXTI0 trong NVIC */
    NVIC_EnableIRQ(EXTI0_IRQn);
}


/*
 NGAT EXTI0
 ISR chi ghi nhan su kien.
 */
void EXTI0_IRQHandler(void)
{
    /* Kiem tra co pending cua EXTI0 */
    if ((EXTI->PR & EXTI_PR_PR0) != 0U)
    {
        /* Xoa co pending */
        EXTI->PR = EXTI_PR_PR0;

        /* Bao cho main biet co mot lan nhan nut */
        button_event = 1U;

        /* Tam khoa EXTI0 de tranh nhieu lan do doi nut */
        EXTI->IMR &= ~EXTI_IMR_MR0;
    }
}


/*
 DMA1 CHANNEL 4 -> USART1_TX
 STM32F103 su dung DMA1 Channel 4 cho USART1_TX.

 DIR  = 1 : RAM -> Peripheral
 MINC = 1 : tang dia chi RAM sau moi byte
 TCIE = 1 : ngat khi truyen xong

 Du lieu di theo luong:
 tx_buffer -> DMA1 Channel 4 -> USART1->DR -> PA9
 */
static void dma_usart1_tx_init(void)
{
    /* Bat clock DMA1 */
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;

    /* Tat channel truoc khi cau hinh */
    DMA1_Channel4->CCR = 0U;

    /* Xoa tat ca co cua Channel 4 */
    DMA1->IFCR = DMA_IFCR_CGIF4 |
                 DMA_IFCR_CTCIF4 |
                 DMA_IFCR_CHTIF4 |
                 DMA_IFCR_CTEIF4;

    /* Cau hinh DMA: RAM -> USART, tang dia chi RAM, ngat TC */
    DMA1_Channel4->CCR = DMA_CCR4_DIR |
                         DMA_CCR4_MINC |
                         DMA_CCR4_TCIE;

    /* Cho USART1 su dung DMA khi gui TX */
    USART1->CR3 |= USART_CR3_DMAT;

    /* Cho phep ngat DMA Channel 4 */
    NVIC_EnableIRQ(DMA1_Channel4_IRQn);
}


/*
 GUI MOT CHUOI BANG DMA
 Khong ghi lai tx_buffer khi DMA dang gui.
 */
static void dma_send(const char *data, uint32_t length)
{
    /* Neu khong co du lieu hoac DMA dang ban thi khong gui */
    if ((length == 0U) || (dma_busy != 0U))
    {
        return;
    }

    dma_busy = 1U;

    /* Tat DMA truoc khi nap thong so moi */
    DMA1_Channel4->CCR &= ~DMA_CCR4_EN;

    /* Xoa co DMA */
    DMA1->IFCR = DMA_IFCR_CGIF4 |
                 DMA_IFCR_CTCIF4 |
                 DMA_IFCR_CHTIF4 |
                 DMA_IFCR_CTEIF4;

    /* Dia chi USART1 Data Register */
    DMA1_Channel4->CPAR = (uint32_t)&USART1->DR;

    /* Dia chi buffer trong RAM */
    DMA1_Channel4->CMAR = (uint32_t)data;

    /* So byte can truyen */
    DMA1_Channel4->CNDTR = (uint16_t)length;

    /* Bat DMA */
    DMA1_Channel4->CCR |= DMA_CCR4_EN;
}


/*
 NGAT DMA1 CHANNEL 4
 Xay ra khi DMA da truyen xong toan bo chuoi.
 */
void DMA1_Channel4_IRQHandler(void)
{
    /* Kiem tra Transfer Complete */
    if ((DMA1->ISR & DMA_ISR_TCIF4) != 0U)
    {
        /* Xoa co Transfer Complete */
        DMA1->IFCR = DMA_IFCR_CTCIF4;

        /* Tat DMA */
        DMA1_Channel4->CCR &= ~DMA_CCR4_EN;

        /* Cho phep lan gui tiep theo */
        dma_busy = 0U;
    }
}


/*
 TAO BAN TIN

 */
static uint32_t make_message(uint32_t value, char *buffer)
{
    static const char prefix[] = "ELE1415-20261-03:15:BTN:";
    uint32_t i = 0U;
    uint32_t number_length;

    /* Chep phan dau vao buffer */
    while (prefix[i] != '\0')
    {
        buffer[i] = prefix[i];
        i++;
    }

    /* Them gia tri nut nhan */
    number_length = uint32_to_str(value, &buffer[i]);
    i += number_length;

    /* Ket thuc dong */
    buffer[i++] = '\r';
    buffer[i++] = '\n';

    return i;
}


/* HAM MAIN */
int main(void)
{
    uint32_t message_length;

    /* Khoi tao UART */
    uart_init();

    /* Khoi tao nut nhan va EXTI0 */
    button_init();

    /* Khoi tao DMA USART1 TX */
    dma_usart1_tx_init();

    /* Thong bao khoi dong */
    uart_str("Bai 3 DMA UART Ready\r\n");

    while (1)
    {
        /* Kiem tra co su kien nhan nut do EXTI0 tao ra */
        if (button_event != 0U)
        {
            /* Xoa co su kien */
            button_event = 0U;

            /* Chong doi nut */
            delay_ms(20U);

            /* Chi xu ly neu nut van dang duoc nhan */
            if ((GPIOA->IDR & (1U << 0)) == 0U)
            {
                /* Tang lien tuc: 1, 2, ..., 9, 10, 11, ..., n */
                button_count++;

                /* Tao ban tin trong RAM */
                message_length = make_message(button_count, tx_buffer);

                /* Gui toan bo ban tin bang DMA */
                dma_send(tx_buffer, message_length);

                /* Cho tha nut */
                while ((GPIOA->IDR & (1U << 0)) == 0U)
                {
                    /* Cho nguoi dung tha nut */
                }

                /* Chong doi khi tha nut */
                delay_ms(20U);
            }

            /* Xoa co pending neu trong qua trinh debounce co phat sinh */
            EXTI->PR = EXTI_PR_PR0;

            /* Cho phep EXTI0 cho lan nhan tiep theo */
            EXTI->IMR |= EXTI_IMR_MR0;
        }
    }
}
