#include "stm32f10x.h"

char command[30]; // luu lenh uart
int cmd_index = 0;
int pwm = 50; // luu do sang 
int led = 0; // trang thai led

void Clock_Init(void)
{
    RCC->CR |= (1 << 16); // bat HSE( xung clk ngoai cua stm32 thach anh) 8MHz 
    while (!(RCC->CR & (1 << 17))); //cho HSE san sang 
    FLASH->ACR |= 2; //flash wait state = 2
    RCC->CFGR |= (4 << 8); // APB1 = 36MHz 
    RCC->CFGR |= (7 << 18);// PLL = HSE x 9 = 72MHz 
    RCC->CFGR |= (1 << 16);//chon HSE lam nguon PLL 
    RCC->CR |= (1 << 24); //bat PLL 
    while (!(RCC->CR & (1 << 25)));
    RCC->CFGR |= (2 << 0); //chon PLL lam SYSCLK 
    while ((RCC->CFGR & (3 << 2)) != (2 << 2));
}

void UART_SendChar(char c)
{
    while (!(USART1->SR & (1 << 7)));// cho TX san sang 
    USART1->DR = c;
}

void UART_SendString(char *str)
{
    int i = 0;
    while (str[i] != '\0')
    {
        UART_SendChar(str[i]);
        i++;
    }
}

int Compare(char *a, char *b)
{
    int i = 0;
    while (a[i] != '\0' || b[i] != '\0')
    {
        if (a[i] != b[i])
        {
            return 0;
        }
        i++;
    }
    return 1;
}

void PWM_Init(void)
{ 
    RCC->APB2ENR |= (1 << 2);// bat clock GPIOA 
    RCC->APB1ENR |= (1 << 0);// bat clock TIM2 
    
    GPIOA->CRL &= ~(0xF << 0); //PA0 = TIM2_CH1
    GPIOA->CRL |= (0xB << 0);//PP 50MHz 

    TIM2->PSC = 71; // 72MHz / 72 = 1MHz

    TIM2->ARR = 999; // 1MHz / 1000 = 1kHz 
    
    // PWM mode 1 
    TIM2->CCMR1 &= ~(0x7 << 4);
    TIM2->CCMR1 |= (0x6 << 4);

    // cho phep CCR1 thay doi
    TIM2->CCMR1 |= (1 << 3);

    // bat kenh CH1 
    TIM2->CCER |= (1 << 0);

    //tat LED 
    TIM2->CCR1 = 0;

    // preload ARR cap phep cho bo dem thanh ghi
    TIM2->CR1 |= (1 << 7);

    // cap nhat thanh ghi 
    TIM2->EGR |= (1 << 0);

    // bat TIM2 
    TIM2->CR1 |= (1 << 0);
}

void UART_Init(void)
{
    
    RCC->APB2ENR |= (1 << 2);// bat clock GPIOA 
    RCC->APB2ENR |= (1 << 14); // bat clock USART1 

    // PA9 = TX 
    GPIOA->CRH &= ~(0xF << 4);
    GPIOA->CRH |= (0xB << 4);

    // PA10 = RX 
    GPIOA->CRH &= ~(0xF << 8);
    GPIOA->CRH |= (0x4 << 8);

    // Baudrate 115200 
    USART1->BRR = 0x0271;

    // bat USART 
    USART1->CR1 |= (1 << 13);

    // bat TX 
    USART1->CR1 |= (1 << 3);

    // bat RX 
    USART1->CR1 |= (1 << 2);

    // cho phep ngat RX 
    USART1->CR1 |= (1 << 5);

    // cho phep ngat USART1 
    NVIC->ISER[1] |= (1 << 5);
}

void Process_Command(void)
{
    int value = 0;
    int i;
    if (Compare(command, "ON"))
    {
        led = 1;
        // bat LED theo PWM hien tai
        TIM2->CCR1 = pwm * 10;
        UART_SendString("LED ON\r\n");
    }
    else if (Compare(command, "OFF"))
    {
        led = 0;
        TIM2->CCR1 = 0; // tat led
        UART_SendString("LED OFF\r\n");
    }
    else if (command[0] == 'P' && command[1] == 'W' && command[2] == 'M' && command[3] == ':')
    {
        i = 4;
        while (command[i] >= '0' && command[i] <= '9') // doc so
        {
            value = value * 10;
            value = value + (command[i] - '0');
            i++;
        }
        if (command[i] == '%' && command[i + 1] == '\0') // ktra dau %
        {
            if (value <= 100)
            {
                pwm = value;
                if (led == 1) // neu led dang sang thi doi do sang ngay
                {
                    TIM2->CCR1 = pwm * 10;
                }
                UART_SendString("PWM OK\r\n");
            }
            else
            {
                UART_SendString("PWM ERROR\r\n");
            }
        }
        else
        {
            UART_SendString("ERROR\r\n");
        }
    }
    else if (Compare(command, "Status"))
    {
        UART_SendString("Status: ");
        if (led == 1)
        {
            UART_SendString("ON\r\n");
        }
        else
        {
            UART_SendString("OFF\r\n");
        }
    }
    else
    {
        UART_SendString("ERROR\r\n"); // sai lenh
    }
}

void USART1_IRQHandler(void)
{
    char c;
    if (USART1->SR & (1 << 5)) // ktra dulieu tren RX
    {
        c = USART1->DR;// doc ki tu
        // gap ! thi ket thuc lenh
        if (c == '!')
        {
            command[cmd_index] = '\0';
            Process_Command();// xly lenh
            cmd_index = 0; // xao vtri
        }
        else
        {
            // luu ktu vao mang
            if (cmd_index < 29)
            {
                command[cmd_index] = c;
                cmd_index++;
            }
            else
            {
                cmd_index = 0; // neu dai thi xoa
            }
        }
    }
}

int main(void)
{
    Clock_Init();
    PWM_Init();
    UART_Init();
    UART_SendString("STM32 READY\r\n");
    while (1)
    {
    }
}
