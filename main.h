#ifndef __MAIN_H
#define __MAIN_H

#include <string.h>
#include "stm32l1xx_hal.h"
#include "stm32l1xx_hal_flash.h"
#include "stm32l1xx_hal_cryp.h"
#include "stm32l1xx_hal_tim.h"
#include "stm32l1xx_nucleo.h"
#include "stm32l1xx_ll_bus.h"
#include "stm32l1xx_ll_rcc.h"
#include "stm32l1xx_ll_pwr.h"
#include "stm32l1xx_ll_system.h"
#include "stm32l1xx_ll_gpio.h"
#include "stm32l1xx_ll_exti.h"
#include "stm32l1xx_ll_tim.h"
#include "aes.h"
#include "my_aes.h"

/* Cipher-specific declarations */
#define PLAINTEXT_DATA                   ((uint8_t*)0x08010000)
#define PLAINTEXT_SIZE                   0x00010000
#define CIPHERTEXT_DATA                  ((uint8_t*)0x08020000)
#define CIPHERTEXT_SIZE                  0x00010000
#define DECIPHERTEXT_DATA                ((uint8_t*)0x08030000)
#define DECIPHERTEXT_SIZE                0x00010000

/* USARTx Peripheral definitions */
#define USARTx                           USART2
#define USARTx_CLK_ENABLE()              __HAL_RCC_USART2_CLK_ENABLE()
#define USARTx_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define USARTx_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()

#define USARTx_FORCE_RESET()             __HAL_RCC_USART2_FORCE_RESET()
#define USARTx_RELEASE_RESET()           __HAL_RCC_USART2_RELEASE_RESET()

/* USARTx Pin definitions */
#define USARTx_TX_PIN                    GPIO_PIN_2
#define USARTx_TX_GPIO_PORT              GPIOA
#define USARTx_TX_AF                     GPIO_AF7_USART2
#define USARTx_RX_PIN                    GPIO_PIN_3
#define USARTx_RX_GPIO_PORT              GPIOA
#define USARTx_RX_AF                     GPIO_AF7_USART2

/* USARTx NVIC definitions */
#define USARTx_IRQn                      USART2_IRQn
#define USARTx_IRQHandler                USART2_IRQHandler

/* USARTx definitions */
#define TXBUFFERSIZE                     (COUNTOF(aTxBuffer) - 1)
#define RXBUFFERSIZE                     TXBUFFERSIZE
#define UARTTIMEOUT                      0xFFFF

/* TIMx definitions */
#define TIMx                             TIM5
#define TIMx_CLK_BUS                     LL_APB1_GRP1_PERIPH_TIM5  
#define TIMx_CLK_ENABLE()              __HAL_RCC_TIM5_CLK_ENABLE()

/* Helper macros */
#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))

/* Symbol definitions */
extern unsigned char key[16];
extern unsigned char plaintext[16];
extern unsigned char state[16];
extern uint8_t init_vec[16];

extern TIM_HandleTypeDef perf_tim;
extern TIM_HandleTypeDef* perf_htim;

/* Function declarations */
__STATIC_INLINE void Configure_TIMTimeBase(void);

#endif /* __MAIN_H */
