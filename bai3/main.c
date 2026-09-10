#define RCC_APB2ENR (*(volatile unsigned int*)0x40021018)// thanh ghi cap clk cho PA
#define GPIOA_CRL   (*(volatile unsigned int*)0x40010800)// thanh ghi cau hinh chan gpio pa0->7
#define GPIOA_CRH   (*(volatile unsigned int*)0x40010804)// thanh ghi cau hinh chan gpio pa8->15
#define GPIOA_IDR   (*(volatile unsigned int*)0x40010808)// thanh ghi doc GPIOA
#define GPIOA_ODR   (*(volatile unsigned int*)0x4001080C)// thanh ghi ghi GPIOA
int main(void)
{
    RCC_APB2ENR |= 0x00000004; // cap xung clk cho PA
    GPIOA_CRL = 0x33333333; // cau hinh PA0->7 o che do outpush pp 50mhz
    GPIOA_CRH = 0x44444444; // cau hinh pa8->15 o che do input floating
    while(1)
    {
        int data;
        data = (GPIOA_IDR >> 8) & 0x000000FF;//doc tin hieu o chan 8->15
        data = (~data) & 0x000000FF;// dao bit
        GPIOA_ODR &= 0xFFFFFF00;//xoa tt cu cua pa0->7
        GPIOA_ODR |= data;//ghi kq
    }
}
