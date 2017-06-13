#include "main.h"
#include "serv_port.h"
#include "user.h"
#include "my_aes.h"
#include "acc_aes.h"

/* Definitions */

/* Performance Timer */
TIM_HandleTypeDef perf_tim;
TIM_HandleTypeDef* perf_htim;

UART_HandleTypeDef UartHandle;
FLASH_EraseInitTypeDef EraseInitStruct;

uint8_t aTxBuffer[128] = {0};
uint8_t aRxBuffer[RXBUFFERSIZE];

uint32_t g_aes_rkeys[11][4];

/* Function declarations */
void SystemClock_Config(void);
static void Error_Handler(void);

void print_words(UART_HandleTypeDef* huart, uint32_t* arr, size_t size);
void print_state(UART_HandleTypeDef* huart, AES_state state);

/* AES encryption structures */
unsigned char key[16] = {
  0x2b, 0x7e, 0x15, 0x16,
  0x28, 0xae, 0xd2, 0xa6,
  0xab, 0xf7, 0x15, 0x88,
  0x09, 0xcf, 0x4f, 0x3c
};
unsigned char plaintext[16] = {
  0x32, 0x43, 0xf6, 0xa8,
  0x88, 0x5a, 0x30, 0x8d,
  0x31, 0x31, 0x98, 0xa2,
  0xe0, 0x37, 0x07, 0x34
};
unsigned char state[16] = {0};

uint8_t init_vec[16] = {
  0x00, 0x01, 0x02, 0x03,
  0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b,
  0x0c, 0x0d, 0x0e, 0x0f
};

int main(void)
{
  HAL_Init();

  SystemClock_Config();
  
  /* Performance Timer init */
  perf_tim.Instance = TIMx;

  uint32_t defpsc = 0; 
  perf_tim.Init.Prescaler         = defpsc;
  perf_tim.Init.Period            = 0xFFFFFFFF;
  perf_tim.Init.ClockDivision     = 0;
  perf_tim.Init.CounterMode       = TIM_COUNTERMODE_UP;
  
  HAL_TIM_Base_Init(&perf_tim);
  perf_htim = &perf_tim;
  
  /* LED2 init */
  LED2_GPIO_CLK_ENABLE();
  
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

  GPIO_InitStruct.Pin = LED2_PIN;
  HAL_GPIO_Init(LED2_GPIO_PORT, &GPIO_InitStruct);
	
  /* Service Port init */
  serv_port_init();
  
  /* Memory space map init */
  Memspace_desc memspace_desc;
  
  memspace_desc.base = (uint32_t)PLAINTEXT_DATA;
  memspace_desc.size = PLAINTEXT_SIZE;
  memspace_map[DATATYPE_PLAINTEXT][AES_MEMTYPE_FLASH] = memspace_desc;
  
  memspace_desc.base = (uint32_t)CIPHERTEXT_DATA;
  memspace_desc.size = CIPHERTEXT_SIZE;
  memspace_map[DATATYPE_CIPHERTEXT][AES_MEMTYPE_FLASH] = memspace_desc;
  
  memspace_desc.base = (uint32_t)aes_ciphertext_buf;
  memspace_desc.size = AES_CIPHERTEXT_BUF_SIZE;
  memspace_map[DATATYPE_CIPHERTEXT][AES_MEMTYPE_RAM] = memspace_desc;
  
  memspace_desc.base = (uint32_t)DECIPHERTEXT_DATA;
  memspace_desc.size = DECIPHERTEXT_SIZE;
  memspace_map[DATATYPE_DECIPHERTEXT][AES_MEMTYPE_FLASH] = memspace_desc;
  
  memspace_desc.base = (uint32_t)aes_deciphertext_buf;
  memspace_desc.size = AES_DECIPHERTEXT_BUF_SIZE;
  memspace_map[DATATYPE_DECIPHERTEXT][AES_MEMTYPE_RAM] = memspace_desc;
  
  /* AES hardware encryption */
  acc_aes_init();
 
  /* Key expansion */
  AES_rkeys rkeys = g_aes_rkeys;
  aes128_expand_key(key, rkeys);
  
  /* Data encryption */
  HAL_FLASH_Unlock();
  
  // erase Flash memory
  HAL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_PIN);
  
  uint32_t PageError;
  
  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.PageAddress = (uint32_t)CIPHERTEXT_DATA;
  EraseInitStruct.NbPages     = CIPHERTEXT_SIZE / FLASH_PAGE_SIZE;
  HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
  
  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.PageAddress = (uint32_t)DECIPHERTEXT_DATA;
  EraseInitStruct.NbPages     = DECIPHERTEXT_SIZE / FLASH_PAGE_SIZE;
  HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
  
  HAL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_PIN);
  
//  HAL_Delay(1000);
//  
//  HAL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_PIN);
//  
//  LL_TIM_SetCounter(TIMx, 0);
//  LL_TIM_EnableCounter(TIMx);
//  
//  //aes128_enc_ecb(rkeys, PLAINTEXT_DATA, 0x100, state, (uint32_t*)CIPHERTEXT_DATA, AES_MEMTYPE_FLASH);
//  acc_aes128_enc_ecb(key, PLAINTEXT_DATA, 0x100, state, (uint32_t*)CIPHERTEXT_DATA, AES_MEMTYPE_FLASH);
//  //acc_aes128_enc_cbc(key, init_vec, PLAINTEXT_DATA, 0x100, state, (uint32_t*)CIPHERTEXT_DATA, AES_MEMTYPE_FLASH);
//  
//  uint32_t my_cc = LL_TIM_GetCounter(TIMx);  
//  LL_TIM_DisableCounter(TIMx);
//  
//  snprintf(serv_port_out_buf, SERV_PORT_BUF_SIZE, "CC elapsed: %i\r\n", my_cc);
//  serv_port_print(serv_port_huart,  serv_port_out_buf);
//    
//  HAL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_PIN);
//  
//  HAL_Delay(1000);
//  
//  HAL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_PIN);
//  aes128_dec_ecb(rkeys, CIPHERTEXT_DATA, 0x100, state, (uint32_t*)(DECIPHERTEXT_DATA), AES_MEMTYPE_FLASH);
//  //acc_aes128_dec_ecb(key, CIPHERTEXT_DATA, 0x100, state, (uint32_t*)(DECIPHERTEXT_DATA), AES_MEMTYPE_FLASH);
//  //aes128_dec_cbc(rkeys, init_vec, CIPHERTEXT_DATA, 0x100, state, (uint32_t*)(DECIPHERTEXT_DATA), AES_MEMTYPE_FLASH);
//  //acc_aes128_dec_cbc(key, init_vec, CIPHERTEXT_DATA, 0x100, state, (uint32_t*)(DECIPHERTEXT_DATA), AES_MEMTYPE_FLASH);
//  HAL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_PIN);

  
//  serv_port_print(serv_port_huart, "Plaintext:\r\n\r\n");
//  print_state(serv_port_huart, (AES_state)plaintext);
//  
//  serv_port_print(serv_port_huart, "AES hardware encrytion START...\r\n");
//  
//  LL_TIM_SetCounter(TIMx, 0);
//  LL_TIM_EnableCounter(TIMx);
//  HAL_CRYP_AESECB_Encrypt(&acc_aes_global, plaintext, sizeof(plaintext), state, 1000);
//  //aes128_enc_ecb(rkeys, plaintext, sizeof(plaintext), state, 0, AES_MEMTYPE_DISCARD); 
//  uint32_t my_cc = LL_TIM_GetCounter(TIMx);  
//  LL_TIM_DisableCounter(TIMx);
//  
//  serv_port_print(serv_port_huart, "Encryption END.\r\n\r\n");
//  
//  snprintf(serv_port_out_buf, SERV_PORT_BUF_SIZE, "CC elapsed: %i\r\n\r\n", my_cc);
//  serv_port_print(serv_port_huart,  serv_port_out_buf);
//  
//  serv_port_print(serv_port_huart, "Ciphertext:\r\n\r\n");
//  print_state(serv_port_huart, (AES_state) state);
  
  HAL_FLASH_Lock();
  
  while(1) {
  }
}


void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};

  // set clock source to PLL at 32 MHz
  RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL          = RCC_PLL_MUL6;
  RCC_OscInitStruct.PLL.PLLDIV          = RCC_PLL_DIV3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  // set full voltage
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  
  while (__HAL_PWR_GET_FLAG(PWR_FLAG_VOS) != RESET) {};

  // set peripheral clocks to full speed
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

static void Error_Handler(void)
{
  while(1)
  {
  }
}

void print_words(UART_HandleTypeDef* huart, uint32_t* arr, size_t size) {
  for (int i = 0; i < size; ++i) {
    uint8_t* p = (uint8_t*)(arr + i);
  
    char buf[32] = {0};
    snprintf(buf, 32, " 0x%02x 0x%02x 0x%02x 0x%02x", p[0], p[1], p[2], p[3]);
    
    if(i % 4 == 0) {
      HAL_UART_Transmit(huart, (uint8_t*)"\r\n", strlen("\r\n"), UARTTIMEOUT);
    }
    
    HAL_UART_Transmit(huart , (uint8_t*)buf, strlen(buf), UARTTIMEOUT);
  }
}

void print_state(UART_HandleTypeDef* huart, AES_state state) {
  uint8_t* p = (uint8_t*)state;
  for (int i = 0; i < AES_STATE_COLS; ++i, p += 1) {
    char buf[32] = {0};
    snprintf(buf, 32, " 0x%02x 0x%02x 0x%02x 0x%02x\r\n", p[0], p[4], p[8], p[12]);
    
    HAL_UART_Transmit(huart , (uint8_t*)buf, strlen(buf), UARTTIMEOUT);
  }
  
  HAL_UART_Transmit(huart, (uint8_t*)"\r\n", strlen("\r\n"), UARTTIMEOUT);
}

