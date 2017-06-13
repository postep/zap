/*
 ************************************ AFCS *************************************
 *
 *	File			: uart.c
 *	Author			: Borys Jelenski
 *	Version			: 1.0
 *	Date			: 26-08-2015
 *	Description		: UART configuration & function definitions
 *
 *******************************************************************************
 */

#include <string.h>
#include "uart.h"
#include "stm32l1xx.h"

// USART2 handle
UART_HandleTypeDef usart2_global;
// pointer to USART2 handle for convenience
UART_HandleTypeDef* husart2;
// buffer for UART input
char uart_in_buf[UART_BUF_SIZE];
// buffer for UART output
char uart_out_buf[UART_BUF_SIZE];

UART_HandleTypeDef* usart2_init(void) {	
	/* USART2 init */
	usart2_global.Instance = USART2;
	usart2_global.Init.BaudRate = 256000;
	usart2_global.Init.WordLength = UART_WORDLENGTH_8B;
	usart2_global.Init.StopBits = UART_STOPBITS_1;
	usart2_global.Init.Parity = UART_PARITY_NONE;
	usart2_global.Init.Mode = UART_MODE_TX_RX;
	usart2_global.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	usart2_global.Init.OverSampling = UART_OVERSAMPLING_16;
	
	return &usart2_global;
}

HAL_StatusTypeDef uart_print(UART_HandleTypeDef* huart, const char* str) {
	uint16_t length = (uint16_t) strlen(str);
	return HAL_UART_Transmit(huart, (uint8_t*) str, length, UART_TIMEOUT);
}

HAL_StatusTypeDef uart_gets(UART_HandleTypeDef* huart, char* str, int max_size, char stop_ch) {
	if(str == NULL || max_size == 0) {
		return HAL_ERROR;
	}
	
	if(huart->State != HAL_UART_STATE_READY) {
		return HAL_BUSY;
	}
	
	__HAL_LOCK(huart);
	huart->State = HAL_UART_STATE_BUSY_RX;
	
	// set SysTick priority to allow ticks incrementation
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

	uint32_t ticks = HAL_GetTick();
	for(int rx_count = 0; rx_count < max_size - 1 && *(str - 1) != stop_ch; ++rx_count) {
		// wait for RXNE flag until timeout
		while( !(huart->Instance->ISR & USART_ISR_RXNE) && ( HAL_GetTick() - ticks < UART_GETS_TIMEOUT ) );
		
		// timeout occured
		if( HAL_GetTick() - ticks >= UART_GETS_TIMEOUT ) {
			// restore SysTick priority
			HAL_NVIC_SetPriority(SysTick_IRQn, TICK_INT_PRIORITY, 0);
			
			huart->State = HAL_UART_STATE_READY;
			__HAL_UNLOCK(huart);
						
			return HAL_TIMEOUT;
		}
		
		*str++ = (char) huart->Instance->RDR;
		ticks = HAL_GetTick();
	}
	
	// append null char at the end of the string
	*str = 0;
	
	// restore SysTick priority
	HAL_NVIC_SetPriority(SysTick_IRQn, TICK_INT_PRIORITY, 0);
	
	huart->State = HAL_UART_STATE_READY;
	__HAL_UNLOCK(huart);
	
	return HAL_OK;
}

HAL_StatusTypeDef uart_getb(UART_HandleTypeDef* huart, uint8_t* bt) {
	// set SysTick priority to allow ticks incrementation
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

	uint32_t ticks = HAL_GetTick();
	// wait for RXNE flag until timeout
	while( !(huart->Instance->ISR & USART_ISR_RXNE) && ( HAL_GetTick() - ticks < UART_GETS_TIMEOUT ) );
	
	// timeout occured
	if( HAL_GetTick() - ticks >= UART_GETS_TIMEOUT ) {
		// restore SysTick priority
		HAL_NVIC_SetPriority(SysTick_IRQn, TICK_INT_PRIORITY, 0);
		
		return HAL_TIMEOUT;
	} else {
		*bt = huart->Instance->RDR;
	}
	
	// restore SysTick priority
	HAL_NVIC_SetPriority(SysTick_IRQn, TICK_INT_PRIORITY, 0);
	
	return HAL_OK;
}

void uart_putb(UART_HandleTypeDef* huart, uint8_t bt) {
	if(huart->State != HAL_UART_STATE_READY) {
		return;
	}
	
	// wait for TXE flag and send byte
	while( !(huart->Instance->ISR & USART_ISR_TXE) );
	huart->Instance->TDR = bt;
	// wait for TC flag
	while( !(huart->Instance->ISR & USART_ISR_TC) );
}
