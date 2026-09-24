#include "stm32f10x.h"
#include <stdint.h>

/*
 * BAI 1 - MPU6050
 *
 * UART1:
 * PA9  -> TX
 * PA10 -> RX
 * 115200 baud, PCLK2 = 8 MHz
 *
 * I2C1:
 * PB6 -> SCL
 * PB7 -> SDA
 *
 * MPU6050:
 * AD0 -> GND => address 0x68
 */


static void uart_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;

    /* PA9 AF push-pull 50MHz; PA10 floating input */
    GPIOA->CRH &= ~((0xFU << 4) | (0xFU << 8));
    GPIOA->CRH |=  ((0xBU << 4) | (0x4U << 8));

    USART1->CR1 = 0;
    USART1->CR2 = 0;
    USART1->CR3 = 0;
    USART1->BRR = 0x45U; /* PCLK2=8MHz, 115200 baud */
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

static void uart_putc(char c)
{
    while (!(USART1->SR & USART_SR_TXE)) {}
    USART1->DR = (uint8_t)c;
}

static void uart_puts(const char *s)
{
    while (*s) uart_putc(*s++);
}

/* DELAY */

static void delay(volatile uint32_t n)
{
    while (n > 0U)
    {
        n--;
    }
}

/*  IN SO NGUYEN */

static void uart_i32(int32_t value)
{
    char buf[12];
    uint32_t i = 0U;
    uint32_t x;

    if (value < 0)
    {
        uart_putc('-');
        x = (uint32_t)(-value);
    }
    else
    {
        x = (uint32_t)value;
    }

    if (x == 0U)
    {
        uart_putc('0');
        return;
    }

    while (x > 0U)
    {
        buf[i++] = (char)('0' + (x % 10U));
        x /= 10U;
    }

    while (i > 0U)
    {
        uart_putc(buf[--i]);
    }
}

/* I2C1 */

#define MPU6050_ADDR         0x68U

#define MPU6050_WHO_AM_I     0x75U
#define MPU6050_PWR_MGMT_1   0x6BU
#define MPU6050_SMPLRT_DIV   0x19U
#define MPU6050_CONFIG       0x1AU
#define MPU6050_GYRO_CONFIG  0x1BU
#define MPU6050_ACCEL_CONFIG 0x1CU
#define MPU6050_ACCEL_XOUT_H 0x3BU

static int i2c_wait_sr1(uint16_t mask, uint32_t timeout)
{
    while ((I2C1->SR1 & mask) == 0U)
    {
        if (timeout == 0U)
        {
            return -1;
        }
        timeout--;
    }
    return 0;
}

static int i2c_wait_not_busy(uint32_t timeout)
{
    while ((I2C1->SR2 & I2C_SR2_BUSY) != 0U)
    {
        if (timeout == 0U)
        {
            return -1;
        }
        timeout--;
    }
    return 0;
}

static void i2c_init(void)
{
    /* Bat clock GPIOB va I2C1 */
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    /*
     * PB6/PB7 = Alternate Function Open Drain, 50MHz
     * CNF=11, MODE=11 => 0xF
     */
    GPIOB->CRL &= ~((0xFU << 24) | (0xFU << 28));
    GPIOB->CRL |=  ((0xFU << 24) | (0xFU << 28));

    /* Khong dung macro SWRST de phu hop voi ca header toi gian */
    I2C1->CR1 = 0U;

    /* PCLK1 = 8MHz */
    I2C1->CR2 = 8U;

    /* Standard mode 100kHz: CCR = 8MHz/(2*100kHz) = 40 */
    I2C1->CCR = 40U;

    /* TRISE = FREQ + 1 = 9 */
    I2C1->TRISE = 9U;

    I2C1->CR1 = I2C_CR1_PE | I2C_CR1_ACK;
}

static int i2c_write_byte(uint8_t addr, uint8_t reg, uint8_t data)
{
    if (i2c_wait_not_busy(200000U) != 0)
    {
        return -1;
    }

    /* START */
    I2C1->CR1 |= I2C_CR1_START;
    if (i2c_wait_sr1(I2C_SR1_SB, 200000U) != 0)
    {
        return -1;
    }

    /* Slave address + WRITE */
    (void)I2C1->SR1;
    I2C1->DR = (uint8_t)(addr << 1);

    if (i2c_wait_sr1(I2C_SR1_ADDR, 200000U) != 0)
    {
        I2C1->CR1 |= I2C_CR1_STOP;
        return -1;
    }

    /* Clear ADDR */
    (void)I2C1->SR1;
    (void)I2C1->SR2;

    /* Gui register */
    if (i2c_wait_sr1(I2C_SR1_TXE, 200000U) != 0)
    {
        I2C1->CR1 |= I2C_CR1_STOP;
        return -1;
    }
    I2C1->DR = reg;

    /* Gui data */
    if (i2c_wait_sr1(I2C_SR1_TXE, 200000U) != 0)
    {
        I2C1->CR1 |= I2C_CR1_STOP;
        return -1;
    }
    I2C1->DR = data;

    if (i2c_wait_sr1(I2C_SR1_BTF, 200000U) != 0)
    {
        I2C1->CR1 |= I2C_CR1_STOP;
        return -1;
    }

    I2C1->CR1 |= I2C_CR1_STOP;
    return 0;
}

static int i2c_read_bytes(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t i;

    if (len == 0U)
    {
        return -1;
    }

    if (i2c_wait_not_busy(200000U) != 0)
    {
        return -1;
    }

    /* START + address WRITE */
    I2C1->CR1 |= I2C_CR1_START;
    if (i2c_wait_sr1(I2C_SR1_SB, 200000U) != 0)
    {
        return -1;
    }

    (void)I2C1->SR1;
    I2C1->DR = (uint8_t)(addr << 1);

    if (i2c_wait_sr1(I2C_SR1_ADDR, 200000U) != 0)
    {
        I2C1->CR1 |= I2C_CR1_STOP;
        return -1;
    }

    (void)I2C1->SR1;
    (void)I2C1->SR2;

    /* Gui dia chi thanh ghi */
    if (i2c_wait_sr1(I2C_SR1_TXE, 200000U) != 0)
    {
        I2C1->CR1 |= I2C_CR1_STOP;
        return -1;
    }
    I2C1->DR = reg;

    if (i2c_wait_sr1(I2C_SR1_BTF, 200000U) != 0)
    {
        I2C1->CR1 |= I2C_CR1_STOP;
        return -1;
    }

    /* RESTART + address READ */
    I2C1->CR1 |= I2C_CR1_START;
    if (i2c_wait_sr1(I2C_SR1_SB, 200000U) != 0)
    {
        return -1;
    }

    (void)I2C1->SR1;
    I2C1->DR = (uint8_t)((addr << 1) | 1U);

    if (i2c_wait_sr1(I2C_SR1_ADDR, 200000U) != 0)
    {
        I2C1->CR1 |= I2C_CR1_STOP;
        return -1;
    }

    if (len == 1U)
    {
        /* NACK byte duy nhat */
        I2C1->CR1 &= ~I2C_CR1_ACK;

        (void)I2C1->SR1;
        (void)I2C1->SR2;

        I2C1->CR1 |= I2C_CR1_STOP;

        if (i2c_wait_sr1(I2C_SR1_RXNE, 200000U) != 0)
        {
            I2C1->CR1 |= I2C_CR1_ACK;
            return -1;
        }

        buf[0] = (uint8_t)I2C1->DR;
        I2C1->CR1 |= I2C_CR1_ACK;
        return 0;
    }

    /* Doc nhieu byte */
    I2C1->CR1 |= I2C_CR1_ACK;

    (void)I2C1->SR1;
    (void)I2C1->SR2;

    for (i = 0U; i < len; i++)
    {
        if (i == (uint8_t)(len - 1U))
        {
            /* Byte cuoi: NACK + STOP */
            I2C1->CR1 &= ~I2C_CR1_ACK;
            I2C1->CR1 |= I2C_CR1_STOP;
        }

        if (i2c_wait_sr1(I2C_SR1_RXNE, 200000U) != 0)
        {
            I2C1->CR1 |= I2C_CR1_ACK;
            return -1;
        }

        buf[i] = (uint8_t)I2C1->DR;
    }

    I2C1->CR1 |= I2C_CR1_ACK;
    return 0;
}

/* CHUYEN 2 BYTE SIGNED */

static int16_t make_i16(uint8_t high, uint8_t low)
{
    return (int16_t)(((uint16_t)high << 8) | (uint16_t)low);
}

/* HAM MAIN  */

int main(void)
{
    uint8_t who = 0U;
    uint8_t data[14];

    int16_t ax, ay, az;
    int16_t temp_raw;
    int16_t gx, gy, gz;

    int32_t ax_mg, ay_mg, az_mg;
    int32_t gx_mdps, gy_mdps, gz_mdps;
    int32_t temp_x10;

    uart_init();

    /* GUI CHUOI THONG BAO UART SAN SANG */
    uart_puts("\r\nMPU6050 I2C UART Ready\r\n");

    i2c_init();
    delay(100000U);

    /* Kiem tra WHO_AM_I */
    if (i2c_read_bytes(MPU6050_ADDR, MPU6050_WHO_AM_I, &who, 1U) != 0)
    {
        uart_puts("ERROR: I2C READ WHO_AM_I\r\n");
        while (1) {}
    }

    uart_puts("WHO_AM_I=0x");
    uart_i32((int32_t)who);
    uart_puts("\r\n");

    if (who != 0x68U)
    {
        uart_puts("ERROR: MPU6050 NOT FOUND\r\n");
        while (1) {}
    }

    uart_puts("MPU6050 FOUND\r\n");

    /* Wake up */
    if (i2c_write_byte(MPU6050_ADDR, MPU6050_PWR_MGMT_1, 0x00U) != 0)
    {
        uart_puts("ERROR: MPU POWER\r\n");
        while (1) {}
    }

    /* Sample rate  Standard = 100Hz */
    i2c_write_byte(MPU6050_ADDR, MPU6050_SMPLRT_DIV, 0x09U);

    /* DLPF */
    i2c_write_byte(MPU6050_ADDR, MPU6050_CONFIG, 0x03U);

    /* Gyro +/-250 dps */
    i2c_write_byte(MPU6050_ADDR, MPU6050_GYRO_CONFIG, 0x00U);

    /* Accel +/-2g */
    i2c_write_byte(MPU6050_ADDR, MPU6050_ACCEL_CONFIG, 0x00U);

    uart_puts("MPU6050 CONFIG OK\r\n");

    while (1)
    {
        if (i2c_read_bytes(MPU6050_ADDR,
                           MPU6050_ACCEL_XOUT_H,
                           data,
                           14U) == 0)
        {
            ax = make_i16(data[0],  data[1]);
            ay = make_i16(data[2],  data[3]);
            az = make_i16(data[4],  data[5]);
            temp_raw = make_i16(data[6], data[7]);
            gx = make_i16(data[8],  data[9]);
            gy = make_i16(data[10], data[11]);
            gz = make_i16(data[12], data[13]);

            /* +/-2g: 16384 LSB/g -> mg */
            ax_mg = ((int32_t)ax * 1000) / 16384;
            ay_mg = ((int32_t)ay * 1000) / 16384;
            az_mg = ((int32_t)az * 1000) / 16384;

            /* +/-250dps: 131 LSB/(deg/s) -> mdps */
            gx_mdps = ((int32_t)gx * 1000) / 131;
            gy_mdps = ((int32_t)gy * 1000) / 131;
            gz_mdps = ((int32_t)gz * 1000) / 131;

            /* T(C) = raw/340 + 36.53; in ra 0.1C */
            temp_x10 = ((int32_t)temp_raw * 10) / 340 + 365;

            uart_puts("ACC(mg): X=");
            uart_i32(ax_mg);
            uart_puts(" Y=");
            uart_i32(ay_mg);
            uart_puts(" Z=");
            uart_i32(az_mg);

            uart_puts(" | TEMP=");
            uart_i32(temp_x10 / 10);
            uart_putc('.');
            uart_i32(temp_x10 % 10);
            uart_puts("C");

            uart_puts(" | GYRO(mdps): X=");
            uart_i32(gx_mdps);
            uart_puts(" Y=");
            uart_i32(gy_mdps);
            uart_puts(" Z=");
            uart_i32(gz_mdps);
            uart_puts("\r\n");
        }
        else
        {
            uart_puts("ERROR: I2C READ DATA\r\n");
        }

        /* Delay xap xi de quan sat */
        delay(2500000U);
    }
}
