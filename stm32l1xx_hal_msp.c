#include "main.h"

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{  
  /* Clocks */
  USARTx_TX_GPIO_CLK_ENABLE();
  USARTx_RX_GPIO_CLK_ENABLE();
  USARTx_CLK_ENABLE(); 
  
  /* Pins */
  GPIO_InitTypeDef  GPIO_InitStruct;
  
  GPIO_InitStruct.Pin       = USARTx_TX_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = USARTx_TX_AF;
  
  HAL_GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStruct);
    
  GPIO_InitStruct.Pin = USARTx_RX_PIN;
  GPIO_InitStruct.Alternate = USARTx_RX_AF;
    
  HAL_GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStruct);
  
    /* NVIC config */
  HAL_NVIC_SetPriority(USARTx_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(USARTx_IRQn);
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
  USARTx_FORCE_RESET();
  USARTx_RELEASE_RESET();

  HAL_GPIO_DeInit(USARTx_TX_GPIO_PORT, USARTx_TX_PIN);
  HAL_GPIO_DeInit(USARTx_RX_GPIO_PORT, USARTx_RX_PIN);
}

void HAL_CRYP_MspInit(CRYP_HandleTypeDef * hcryp) {
  /* Clocks */
  __HAL_RCC_CRYP_FORCE_RESET();
  __HAL_RCC_CRYP_RELEASE_RESET();
  
  __HAL_RCC_CRYP_CLK_ENABLE();
  
  /* NVIC config */
  HAL_NVIC_SetPriority(AES_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(AES_IRQn);
}

void HAL_CRYP_MspDeInit(CRYP_HandleTypeDef * hcryp) {
  __HAL_RCC_CRYP_FORCE_RESET();
  __HAL_RCC_CRYP_RELEASE_RESET();
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim) {
  TIMx_CLK_ENABLE();
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef *htim) {
  __HAL_RCC_TIM5_FORCE_RESET();
  __HAL_RCC_TIM5_RELEASE_RESET();
}
