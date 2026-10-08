#include "stm32f10x.h"
#include <stdint.h>

/*
  - Clock su dung: 8 MHz
  - UART1: PA9 TX, PA10 RX, 115200 8N1
  - ADC1 Channel 0: PA0
  - TIM3 tao tin hieu TRGO 100 Hz
  - ADC duoc kich boi TIM3
  - DMA1 Channel 1: ADC1 -> RAM
  - DMA1 Channel 4: RAM -> USART1
  - Moi 1 giay thu 100 mau ADC
  - DMA ADC co ngat Half Transfer va Transfer Complete
  - DMA duoc dung cho ca ADC va UART.
* /




/* So luong mau ADC thu trong 1 giay */
#define ADC_SAMPLE_COUNT    100U

/* So mau tai thoi diem Half Transfer */
#define ADC_HALF_COUNT      50U


/*  BIEN TOAN CUC */

/*
  Bo dem ADC gom 100 phan tu.
 
  DMA1 Channel 1 se tu dong ghi:
  ADC1->DR -> adc_buffer[]
 
  Khi DMA da ghi 50 mau:
  -> phat sinh ngat Half Transfer
 
  Khi DMA da ghi du 100 mau:
  -> phat sinh ngat Transfer Complete
 
  Che do Circular cho phep DMA tiep tuc ghi lai tu dau.
 */
static volatile uint16_t adc_buffer[ADC_SAMPLE_COUNT];

/*
  Co bao hieu xu ly nua dau bo dem.
 
  = 1: DMA da ghi xong 50 mau dau
  = 0: chua co yeu cau xu ly
 */
static volatile uint8_t adc_half_ready = 0U;

/*
  Co bao hieu xu ly nua sau bo dem.
 
  = 1: DMA da ghi xong 50 mau cuoi
  = 0: chua co yeu cau xu ly
 */
static volatile uint8_t adc_full_ready = 0U;


/*
  Hai bo dem UART.
 
  Trong khi DMA1 Channel 4 dang gui mot bo dem,
  CPU co the chuan bi noi dung vao bo dem con lai.
 
  Cach nay han che viec CPU thay doi noi dung
  trong khi DMA van dang doc bo dem.
 */
static char uart_buffer_0[600];
static char uart_buffer_1[600];

/*
  Trang thai DMA UART.
 
  = 1: DMA dang gui du lieu
  = 0: DMA dang ranh
 */
static volatile uint8_t uart_dma_busy = 0U;


/*
  HAM DELAY 
  Day chi la delay busy-wait.
 
  Gia tri delay phu thuoc vao tan so CPU va trinh bien dich,
  vi vay khong dung ham nay de tao tan so chinh xac.
*/
static void delay_simple(volatile uint32_t count)
{
    while (count > 0U)
    {
        count--;
    }
}


/*
  PA9  -> USART1_TX
  PA10 -> USART1_RX
 
  USART1:
  - Baudrate: 115200
  - Data: 8 bit
  - Stop: 1 bit
  - Parity: none
 
  USART1 TX duoc noi voi DMA1 Channel 4.
*/
static void uart_init(void)
{
    /*
      Bat clock GPIOA va USART1.
     */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;


    /* PA9:
      - Output
      - Alternate Function Push-Pull
      - Speed 50 MHz
      */
    GPIOA->CRH &= ~(0xFU << 4);
    GPIOA->CRH |= (0xBU << 4);


    /* PA10:Input floating */
    GPIOA->CRH &= ~(0xFU << 8);
    GPIOA->CRH |= (0x4U << 8);


    /*
      Cau hinh baudrate 115200.
     
      Voi PCLK2 = 8 MHz:
      BRR = 0x0045
     */
    USART1->BRR = 0x0045U;


    /*
      Bat:
      - USART
      - Transmitter
      - Receiver
     */
    USART1->CR1 =
        USART_CR1_UE |
        USART_CR1_TE |
        USART_CR1_RE;
}


/*
  KHOI TAO ADC1
  ADC1 Channel 0 su dung PA0.
  ADC duoc cau hinh:
  - PA0 o che do Analog Input
  - Clock ADC = PCLK2 / 6
  - Kenh chuyen doi = Channel 0
  - Sample time = 239.5 cycles
  - Trigger tu TIM3 TRGO
  - DMA mode duoc bat
*/
static void adc_init(void)
{
    /* Bat clock GPIOA va ADC1. */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;


    /* PA0:
      Dat MODE = 00 va CNF = 00
      -> Analog Input
     */
    GPIOA->CRL &= ~(0xFU << 0);


    /*
      ADC clock = PCLK2 / 6.
     
      PCLK2 = 8 MHz
      ADC clock xap xi 1.33 MHz
     */
    RCC->CFGR &= ~RCC_CFGR_ADCPRE;
    RCC->CFGR |= RCC_CFGR_ADCPRE_DIV6;


    /*
      Cau hinh so luong chuyen doi trong regular sequence.
     
      SQR1 = 0 -> 1 conversion.
     */
    ADC1->SQR1 = 0U;
    ADC1->SQR2 = 0U;
    ADC1->SQR3 = 0U;


    /*
      Chon thoi gian lay mau lon nhat:
      239.5 ADC cycles.
     
      Channel 0 nam o SMPR2 bit 0..2.
     */
    ADC1->SMPR2 &= ~(7U << 0);
    ADC1->SMPR2 |= (7U << 0);


    /*
      Chon nguon trigger ngoai cho regular conversion.
     
      EXTSEL = 100
      -> TIM3 TRGO.
     */
    ADC1->CR2 &= ~ADC_CR2_EXTSEL;
    ADC1->CR2 |= (4U << 17);


    /*
      Cho phep ADC nhan trigger ngoai.
     */
    ADC1->CR2 |= ADC_CR2_EXTTRIG;


    /*
      Cho phep ADC su dung DMA.
     */
    ADC1->CR2 |= ADC_CR2_DMA;


    /*
      Bat ADC1.
     */
    ADC1->CR2 |= ADC_CR2_ADON;


    /*
      Cho ADC on dinh truoc khi calibration.
     */
    delay_simple(10000U);


    /*
      Reset calibration.
     */
    ADC1->CR2 |= ADC_CR2_RSTCAL;

    while ((ADC1->CR2 & ADC_CR2_RSTCAL) != 0U)
    {
        /* Cho reset calibration hoan tat */
    }


    /*
      Bat dau calibration.
     */
    ADC1->CR2 |= ADC_CR2_CAL;

    while ((ADC1->CR2 & ADC_CR2_CAL) != 0U)
    {
        /* Cho calibration hoan tat */
    }
}


/*
  KHOI TAO TIM3
  TIM3 duoc dung lam nguon trigger cho ADC.
  TIM3 clock = 8 MHz:
  Timer frequency:
       8 MHz / (PSC + 1) / (ARR + 1)
 
       PSC = 7999
       ARR = 9
 
  => 8,000,000 / 8000 / 10
  => 100 Hz
 
  Moi 10 ms TIM3 tao mot TRGO.
  ADC nhan TRGO va lay 1 mau.
 
  100 mau / 100 Hz = 1 giay.
*/
static void tim3_init(void)
{
    /*
      Bat clock cho TIM3.
     */
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;


    /*
      Prescaler.
     
      8 MHz / (7999 + 1)
      = 1 kHz
     */
    TIM3->PSC = 7999U;


    /*
      Auto Reload.
     
      1 kHz / (9 + 1)
      = 100 Hz
     */
    TIM3->ARR = 9U;


    /*
      Chon TIM3 Update Event lam TRGO.
     
      MMS = 010
      -> Update event.
     */
    TIM3->CR2 &= ~TIM_CR2_MMS;
    TIM3->CR2 |= (2U << 4);


    /*
      Dat gia tri dem ban dau.
     */
    TIM3->CNT = 0U;


    /*
      Bat TIM3.
     */
    TIM3->CR1 |= TIM_CR1_CEN;
}


/*
  KHOI TAO DMA1 CHANNEL 1 CHO ADC
  DMA1 Channel 1 co nhiem vu:
 
       ADC1->DR  --->  adc_buffer[]
 
  ADC moi lan chuyen doi xong se tao yeu cau DMA.
 
  DMA:
  - Peripheral address = ADC1->DR
  - Memory address = adc_buffer
  - 100 phan tu
  - Peripheral size = 16 bit
  - Memory size = 16 bit
  - Memory increment = ON
  - Circular = ON
  - Half Transfer interrupt = ON
  - Transfer Complete interrupt = ON
*/
static void dma_adc_init(void)
{
    /*
      Bat clock DMA1.
     */
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;


    /*
      Tat DMA Channel 1 truoc khi cau hinh.
     */
    DMA1_Channel1->CCR &= ~DMA_CCR1_EN;


    /*
      Xoa cac co ngat cu cua Channel 1.
     */
    DMA1->IFCR =
        DMA_IFCR_CGIF1 |
        DMA_IFCR_CTCIF1 |
        DMA_IFCR_CHTIF1 |
        DMA_IFCR_CTEIF1;


    /*
      Dia chi ngoai vi:
      ADC1 Data Register.
     */
    DMA1_Channel1->CPAR = (uint32_t)&ADC1->DR;


    /*
      Dia chi bo nho:
      adc_buffer.
     */
    DMA1_Channel1->CMAR = (uint32_t)adc_buffer;


    /*
      So luong du lieu can chuyen.
     
      100 mau ADC.
     */
    DMA1_Channel1->CNDTR = ADC_SAMPLE_COUNT;


    /*
      Cau hinh DMA Channel 1:
     
      MINC       : tang dia chi RAM sau moi lan ghi
      PSIZE_0    : Peripheral = 16 bit
      MSIZE_0    : Memory = 16 bit
      CIRC       : chay vong tron
      HTIE       : ngat Half Transfer
      TCIE       : ngat Transfer Complete
     
      DIR khong bat:
      -> Peripheral -> Memory
     */
    DMA1_Channel1->CCR =
        DMA_CCR1_MINC |
        DMA_CCR1_PSIZE_0 |
        DMA_CCR1_MSIZE_0 |
        DMA_CCR1_CIRC |
        DMA_CCR1_HTIE |
        DMA_CCR1_TCIE;


    /*
      Cho phep ngat DMA Channel 1 trong NVIC.
     */
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);


    /*
      Bat DMA Channel 1.
     */
    DMA1_Channel1->CCR |= DMA_CCR1_EN;
}


/*
  KHOI TAO DMA1 CHANNEL 4 CHO UART
  DMA1 Channel 4 co nhiem vu:
 
       RAM  --->  USART1->DR
 
  Moi khi USART1 can gui ky tu tiep theo,
  DMA se tu dong lay 1 byte tu RAM va ghi vao DR.
 
  DMA UART duoc bat/tat moi lan gui mot bo dem.
*/
static void dma_uart_init(void)
{
    /*
      Tat DMA Channel 4 truoc khi cau hinh.
     */
    DMA1_Channel4->CCR &= ~DMA_CCR4_EN;


    /*
      Xoa cac co ngat cu cua Channel 4.
     */
    DMA1->IFCR =
        DMA_IFCR_CGIF4 |
        DMA_IFCR_CTCIF4 |
        DMA_IFCR_CHTIF4 |
        DMA_IFCR_CTEIF4;


    /*
      Cho phep USART1 phat yeu cau DMA khi gui du lieu.
     */
    USART1->CR3 |= USART_CR3_DMAT;


    /*
      Cho phep ngat DMA Channel 4.
     */
    NVIC_EnableIRQ(DMA1_Channel4_IRQn);
}


/*
  CHUYEN SO NGUYEN SANG CHUOI KY TU
  Vi du:
 
       2048 -> "2048"
 
*/
static uint32_t uint_to_string(uint32_t value, char *buffer)
{
    char temp[10];
    uint32_t i = 0U;
    uint32_t j = 0U;


    /*
      Truong hop gia tri = 0.
     */
    if (value == 0U)
    {
        buffer[0] = '0';
        return 1U;
    }


    /*
      Tach tung chu so.
     
      Chu so lay tu phai sang trai nen tam luu
      vao temp truoc.
     */
    while (value > 0U)
    {
        temp[i] = (char)('0' + (value % 10U));
        value /= 10U;
        i++;
    }


    /*
      Dao nguoc chuoi de duoc thu tu dung.
     */
    while (i > 0U)
    {
        i--;
        buffer[j] = temp[i];
        j++;
    }


    return j;
}


/*
  TAO NOI DUNG UART TU MOT PHAN BO ADC
  Moi gia tri ADC duoc gui theo dang:
 
       2048\n\r
       2050\n\r
       2047\n\r
 
  Tra ve tong so byte da ghi vao buffer.
 
  start_index:
       Vi tri bat dau trong adc_buffer.
 
  count:
       So luong mau can chuyen.
*/
static uint32_t make_adc_block(
    char *buffer,
    uint32_t start_index,
    uint32_t count)
{
    uint32_t i;
    uint32_t pos = 0U;
    uint32_t len;


    for (i = 0U; i < count; i++)
    {
        /*
          Chuyen gia tri ADC thanh chuoi.
         */
        len = uint_to_string(
            adc_buffer[start_index + i],
            &buffer[pos]
        );

        pos += len;


        /*
          Ket thuc moi mau bang:
          LF + CR
         */
        buffer[pos++] = '\n';
        buffer[pos++] = '\r';
    }


    return pos;
}


/*
  GUI DU LIEU BANG DMA UART
  buffer:
       Bo dem can gui.
 
  length:
       So byte can gui.
 
  Neu DMA UART dang ban:
       Khong gui them.
 
  Neu DMA UART ranh:
       Cau hinh lai Channel 4 va bat DMA.
*/
static void uart_dma_send(char *buffer, uint32_t length)
{
    /*
      Neu DMA dang gui thi thoat.
     */
    if (uart_dma_busy != 0U)
    {
        return;
    }


    /*
      Danh dau DMA UART dang ban.
     */
    uart_dma_busy = 1U;


    /*
      Tat DMA Channel 4 truoc khi thay doi cau hinh.
     */
    DMA1_Channel4->CCR &= ~DMA_CCR4_EN;


    /*
      Xoa cac co ngat cu.
     */
    DMA1->IFCR =
        DMA_IFCR_CGIF4 |
        DMA_IFCR_CTCIF4 |
        DMA_IFCR_CHTIF4 |
        DMA_IFCR_CTEIF4;


    /*
      Dia chi ngoai vi:
      USART1 Data Register.
     */
    DMA1_Channel4->CPAR = (uint32_t)&USART1->DR;


    /*
      Dia chi bo nho can gui.
     */
    DMA1_Channel4->CMAR = (uint32_t)buffer;


    /*
      So byte can gui.
     */
    DMA1_Channel4->CNDTR = length;


    /*
      Cau hinh DMA:
     
      DIR  = 1
      -> RAM -> Peripheral
     
      MINC = 1
      -> tang dia chi RAM sau moi byte
     
      TCIE = 1
      -> ngat khi gui xong toan bo buffer
     */
    DMA1_Channel4->CCR =
        DMA_CCR4_DIR |
        DMA_CCR4_MINC |
        DMA_CCR4_TCIE;


    /*
      Bat DMA Channel 4.
     */
    DMA1_Channel4->CCR |= DMA_CCR4_EN;
}


/*
  NGAT DMA1 CHANNEL 1
  Channel 1 phuc vu ADC.
 
  HTIF1:
       DMA da ghi xong 50 mau dau.
 
  TCIF1:
       DMA da ghi xong 100 mau.
 
  Do DMA chay Circular:
       sau TC DMA quay lai dau buffer.
*/
void DMA1_Channel1_IRQHandler(void)
{
    uint32_t status = DMA1->ISR;


    /*
      Kiem tra Half Transfer.
     */
    if ((status & DMA_ISR_HTIF1) != 0U)
    {
        /*
          Xoa cau hinh Half Transfer.
         */
        DMA1->IFCR = DMA_IFCR_CHTIF1;


        /*
          Bao cho main() biet 50 mau dau da san sang.
         */
        adc_half_ready = 1U;
    }


    /*
      Kiem tra Transfer Complete.
     */
    if ((status & DMA_ISR_TCIF1) != 0U)
    {
        /*
          Xoa cau hinh Transfer Complete.
         */
        DMA1->IFCR = DMA_IFCR_CTCIF1;


        /*
          Bao cho main() biet 50 mau cuoi da san sang.
         */
        adc_full_ready = 1U;
    }
}


/*
  NGAT DMA1 CHANNEL 4
  Channel 4 phuc vu UART TX.
 
  Khi DMA gui xong toan bo bo dem:
  - Xoa co TC
  - Tat DMA Channel 4
  - Danh dau DMA UART ranh
 
  Sau do main() co the gui bo du lieu tiep theo.
*/
void DMA1_Channel4_IRQHandler(void)
{
    /*
      Kiem tra Transfer Complete cua Channel 4.
     */
    if ((DMA1->ISR & DMA_ISR_TCIF4) != 0U)
    {
        /*
          Xoa cau  Transfer Complete.
         */
        DMA1->IFCR = DMA_IFCR_CTCIF4;


        /*
          Tat DMA Channel 4.
        */
        DMA1_Channel4->CCR &= ~DMA_CCR4_EN;


        /*
          Bao DMA UART da gui xong.
         */
        uart_dma_busy = 0U;
    }
}


/* HAM MAIN */
int main(void)
{
    uint32_t length;


    /*
      Khoi tao UART1.
     */
    uart_init();


    /*
      Khoi tao ADC1.
     */
    adc_init();


    /*
      Khoi tao DMA cho ADC.
     */
    dma_adc_init();


    /*
      Khoi tao DMA cho UART.
     */
    dma_uart_init();


    /*
      Khoi tao TIM3.
     
      TIM3 se tao trigger 100 Hz cho ADC.
     */
    tim3_init();


    /*
      Vong lap chinh.
     
      CPU khong can tu doc ADC bang cach polling.
      ADC + TIM3 + DMA tu dong hoat dong.
     */
    while (1)
    {
        /*
          XU LY 50 MAU DAU
         
          Khi DMA bao Half Transfer:
          adc_buffer[0..49] da co du lieu moi.
         */
        if (adc_half_ready != 0U)
        {
            /*
              Xoa co xu ly.
             */
            adc_half_ready = 0U;


            /*
              Tao chuoi tu 50 mau dau.
             */
            length = make_adc_block(uart_buffer_0,0U,ADC_HALF_COUNT);


            /*
              Gui 50 mau qua UART bang DMA.
             
              Neu DMA UART dang ban thi ham se bo qua.
             */
            uart_dma_send(uart_buffer_0,length);
        }


        /*
          XU LY 50 MAU CUOI
         
          Khi DMA bao Transfer Complete:
          adc_buffer[50..99] da co du lieu moi.
         */
        if (adc_full_ready != 0U)
        {
            /*
              Xoa co xu ly.
             */
            adc_full_ready = 0U;


            /*
              Tao chuoi tu 50 mau cuoi.
             */
            length = make_adc_block(uart_buffer_1,ADC_HALF_COUNT,ADC_HALF_COUNT);


            /*
              Gui 50 mau cuoi qua UART bang DMA.
             */
            uart_dma_send(uart_buffer_1,length);
        }
    }
}
