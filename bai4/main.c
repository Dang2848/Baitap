#define RCC_APB2ENR (*(volatile unsigned int*)0x40021018) // dchi thanh ghi cap clk GPIO A
#define GPIOA_CRL   (*(volatile unsigned int*)0x40010800) // dchi thanh ghi câu hinh pa0->7
#define GPIOA_IDR   (*(volatile unsigned int*)0x40010808) // dia chi thanh ghi doc tt chan gpio A
#define GPIOA_ODR   (*(volatile unsigned int*)0x4001080C) // dia chi xuat muc logic ra chan A
int main(void)
{
    //led
    RCC_APB2ENR |= 0x00000004; // cap clk cho PA
    GPIOA_CRL &= ~0xF0000000; // xoa cau hinh cu cua PA7
    GPIOA_CRL |=  0x30000000;//cau hinh cho PA7 o trang thai output pp 50mhz

    //nut nhan
    GPIOA_CRL &= ~0x0F000000; // xoa cau hinh cu
    GPIOA_CRL |=  0x08000000; // cau hinh PA6 thanh nut nhan pull up, down

    GPIOA_ODR |= 0x00000040; // cau hinh muc logic cua nut nhan len 1 tao pull up
    GPIOA_ODR &= ~0x00000080; // tat led
    while(1)
    {
          while(GPIOA_IDR & 0x00000040)
        {
        }//pa6=1 nghia la nut chua dc bam và cho toi khi nut bam thi dk vong while dung pa6=0 no se out vong while
        
        while(!(GPIOA_IDR & 0x00000040))
        {
        }//luc nay pa6=0 ta tien hanh nha nut bam thi pa6=1 thoat vong while
        //hiểu đơn giản thì mình sẽ đọc trạng thái của pa6 ban đầu là pa6=1 và cho tới khi ta giữ nút bấm thì pa6=0 nó sẽ out ra vòng while đầu và tới 
        // vòng while tiếp theo lúc này ta đang giữ nút bấm nên pa6=0 và chừng nào ta nhả nút thì pa6=1 và sẽ thoát vòng while
        GPIOA_ODR ^= 0x00000080;// dao trang thai p7
    }
}
