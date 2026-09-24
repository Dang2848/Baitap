#include "main.h"   // Thư viện cấu hình chính của project do STM32Cube sinh ra
#include <stdio.h>  // Thư viện chuẩn C hỗ trợ hàm định dạng chuỗi sprintf

/* ====================================================================
 *                 KHAI BÁO BIẾN TOÀN CỤC DÙNG TRONG HỆ THỐNG
 * ==================================================================== */

// Con trỏ quản lý cổng giao tiếp USART1
UART_HandleTypeDef huart1;

// Con trỏ quản lý kênh DMA truyền dữ liệu cho USART1 (USART1_TX dùng DMA1 Channel 4)
DMA_HandleTypeDef hdma_usart1_tx;

// Biến lưu số lần nhấn nút bấm, khởi tạo bằng 0
// Dùng từ khóa 'volatile' để báo cho trình biên dịch biết biến này có thể thay đổi
// bất ngờ bởi chương trình ngắt, tránh bị tối ưu hóa sai
volatile uint16_t btn_count = 0;

// Cờ báo hiệu có sự kiện nhấn nút (0: không nhấn, 1: vừa nhấn nút xong)
volatile uint8_t btn_flag = 0;

// Mảng bộ đệm chứa chuỗi ký tự bản tin gửi lên máy tính qua UART
char uart_buf[100];


/* ====================================================================
 *                 KHAI BÁO NGUYÊN MẪU CÁC HÀM CẤU HÌNH
 * ==================================================================== */
void SystemClock_Config(void);          // Cấu hình nguồn xung nhịp hệ thống (Clock)
static void MX_GPIO_Init(void);         // Cấu hình chân vào/ra và ngắt ngoài (PA0)
static void MX_DMA_Init(void);          // Cấu hình bộ điều khiển DMA
static void MX_USART1_UART_Init(void);  // Cấu hình giao tiếp UART1


/* ====================================================================
 *                         HÀM CHÍNH (MAIN)
 * ==================================================================== */
int main(void)
{
  /* 1. Khởi tạo thư viện HAL, thiết lập Flash và cấu hình bộ định thời SysTick */
  HAL_Init();

  /* 2. Cấu hình xung nhịp hoạt động cho vi điều khiển (sử dụng dao động nội HSI) */
  SystemClock_Config();

  /* 3. Khởi tạo phần cứng ngoại vi:
   * Chú ý: Cấu hình DMA bắt buộc phải gọi TRƯỚC USART1 
   * để khi USART1 liên kết với luồng DMA thì tài nguyên DMA đã sẵn sàng.
   */
  MX_GPIO_Init();         // Khởi tạo chân nút bấm PA0 và bật ngắt ngoài EXTI0
  MX_DMA_Init();          // Cấp xung và bật ngắt cho kênh DMA1 Channel 4
  MX_USART1_UART_Init();  // Khởi tạo thông số UART1 (115200 baud)

  /* 4. Vòng lặp vô tận xử lý tác vụ */
  while (1)
  {
    /* Kiểm tra xem cờ ngắt nút bấm có được bật lên 1 không */
    if (btn_flag == 1) 
    {
      /* Tăng biến đếm số lần nhấn nút lên 1 đơn vị */
      btn_count++;

      /* Đóng gói nội dung bản tin theo đúng cú pháp đề bài:
       * Cấu trúc: <ID-Lớp><ID-Nhóm>:BTN:<Giá trị nút nhấn>\r\n
* Hàm sprintf trả về số lượng byte ký tự thực tế được ghi vào chuỗi (len).
       */
      uint16_t len = sprintf(uart_buf, "D23N03-Nhom15:BTN:%d\r\n", btn_count);

      /* Bắn toàn bộ chuỗi ký tự trong bộ đệm 'uart_buf' lên PC qua UART bằng DMA:
       * - Không sử dụng CPU để đẩy từng byte (không dùng vòng lặp chờ).
       * - Phần cứng DMA sẽ tự động lấy dữ liệu từ RAM đẩy thẳng ra thanh ghi UART.
       */
      HAL_UART_Transmit_DMA(&huart1, (uint8_t*)uart_buf, len);

      /* Xóa cờ ngắt về 0 để chờ lần bấm tiếp theo */
      btn_flag = 0;

      /* Tạo khoảng trễ 250ms để lọc nhiễu rung tiếp điểm cơ khí của nút bấm (Debounce),
       * giúp 1 lần bấm vật lý không bị đếm nhầm thành nhiều lần liên tục.
       */
      HAL_Delay(250);
    }
  }
}


/* ====================================================================
 *                 HÀM PHỤC VỤ NGẮT NGOÀI (EXTI CALLBACK)
 * ==================================================================== */
/**
 * @brief  Hàm ngắt được phần cứng gọi tự động mỗi khi có sự kiện thay đổi mức logic trên chân GPIO.
 * @param  GPIO_Pin: Chân GPIO kích hoạt ngắt.
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  /* Kiểm tra xem ngắt có phải do chân PA0 (nút nhấn) tạo ra hay không */
  if(GPIO_Pin == GPIO_PIN_0) 
  {
    /* Đặt cờ báo để vòng lặp while(1) ở hàm main biết và thực thi gửi bản tin.
     * Tránh viết các hàm trễ hoặc xử lý chuỗi dài bên trong hàm ngắt này.
     */
    btn_flag = 1;
  }
}


/* ====================================================================
 *                 HÀM CẤU HÌNH XUNG NHỊP HỆ THỐNG
 * ==================================================================== */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /* Cấu hình bộ dao động: sử dụng nguồn thạch anh nội RC HSI (8 MHz) */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE; // Không dùng nhân tần PLL
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler(); // Xử lý lỗi nếu bật xung nhịp thất bại
  }

  /* Cấu hình bus truyền dẫn cho CPU (HCLK), APB1 và APB2 */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                              | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI; // Nguồn xung hệ thống lấy từ HSI
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}


/* ====================================================================
 *                 HÀM KHỞI TẠO NGOẠI VI UART1
 * ==================================================================== */
static void MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;                    // Chọn bộ phần cứng USART1
  huart1.Init.BaudRate = 115200;               // Tốc độ truyền nhận: 115200 baud
  huart1.Init.WordLength = UART_WORDLENGTH_8B; // Khung truyền dữ liệu: 8 bit
  huart1.Init.StopBits = UART_STOPBITS_1;      // 1 bit dừng (Stop bit)
  huart1.Init.Parity = UART_PARITY_NONE;       // Không dùng bit kiểm tra chẵn lẻ (No parity)
  huart1.Init.Mode = UART_MODE_TX_RX;          // Bật cả chế độ truyền (TX) và nhận (RX)
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE; // Không dùng chân điều khiển luồng phần cứng RTS/CTS
  huart1.Init.OverSampling = UART_OVERSAMPLING_16; // Tỷ lệ lấy mẫu 16 lần

  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
}


/* ====================================================================
 *                 HÀM KHỞI TẠO BỘ ĐIỀU KHIỂN DMA
 * ==================================================================== */
static void MX_DMA_Init(void)
{
  /* Cấp xung clock hoạt động cho module DMA1 */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* Cấu hình ưu tiên và kích hoạt vector ngắt cho DMA1 Channel 4 (kênh TX của USART1)
   * Nhờ đó khi DMA đẩy hết toàn bộ chuỗi byte, nó sẽ gửi tín hiệu báo hoàn thành.
   */
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
}


/* ====================================================================
 *                 HÀM CẤU HÌNH GPIO VÀ NGẮT NGOÀI
 * ==================================================================== */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Cấp xung clock cho cổng GPIOA */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* Cấu hình chân PA0 nối với nút bấm:
   * - Pin: PA0
   * - Mode: GPIO_MODE_IT_FALLING (Ngắt theo sườn xuống - khi bấm nút chân bị kéo từ 3.3V xuống GND)
   * - Pull: GPIO_PULLUP (Kéo điện trở nội lên nguồn 3.3V khi chưa bấm nút)
   */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Thiết lập mức ưu tiên ngắt và kích hoạt ngắt ngoài EXTI tuyến 0 (EXTI0 ứng với chân PA0) */
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}


/* ====================================================================
 *                 HÀM XỬ LÝ LỖI HỆ THỐNG
* ==================================================================== */
void Error_Handler(void)
{
  /* Tắt toàn bộ ngắt và đưa chip vào vòng lặp vô tận khi gặp lỗi khởi tạo */
  __disable_irq();
  while (1)
  {
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  /* Điểm bẫy lỗi kiểm tra điều kiện đầu vào của thư viện HAL */
}
#endif /* USE_FULL_ASSERT */