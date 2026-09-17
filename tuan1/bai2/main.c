#define RCC_APB2ENR (*(volatile unsigned int*)0x40021018)//dchi thanh ghi rcc bat clk cua chan gpio A
#define GPIOA_CRL   (*(volatile unsigned int*)0x40010800)//dc thanh ghi cau hinh gpio A
#define GPIOA_ODR   (*(volatile unsigned int*)0x4001080C)//dchi thanh ghi xoa dl gpio A
void delay_ms(unsigned int t)
{
    unsigned int i, j;
    for(i = 0; i < t; i++)
    {
        for(j = 0; j <= 0x0400; j++)
        {
        }
    }
}
int main(void)
{
    int i;
    RCC_APB2ENR |= 0x00000004;// cap clk cho  gpio A
    GPIOA_CRL &= ~0xFFFFFFFF;
    GPIOA_CRL |= 0x33333333;//cau hinh pa0->7 o che do output pp 50mhz
    while(1)
    {
        //nhay led tu pa0->7
        for(i = 0; i < 8; i++)
        {
            GPIOA_ODR = (1 << i);// bat led
            delay_ms(1000);
        }
        for(i = 6; i >= 0; i--)
        {
            GPIOA_ODR = (1 << i);
            delay_ms(1000);
        }
    }
}
