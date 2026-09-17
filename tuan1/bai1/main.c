#define RCC_APB2ENR (*(volatile unsigned int*)0x40021018)
#define GPIOC_CRH   (*(volatile unsigned int*)0x40011004)
#define GPIOC_ODR   (*(volatile unsigned int*)0x4001100C)
void delay(int time)
{
    int i,j;
    for(i = 0; i < time; i++)
    {
	for(j = 0; j < 0x0400; j++)
        {
        }
    }
}
int main(void)
{
    RCC_APB2ENR |= 0x00000010;//cap clk cho pc13
    // cau hinh pc13 o che do output pp 50mhz
    GPIOC_CRH &= ~0x00F00000;
    GPIOC_CRH |= 0x00300000;
    while(1)
    {
        GPIOC_ODR ^= (1 << 13);// dao trang thai
        delay(1000);
    }
}
