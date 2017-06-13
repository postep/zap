#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "main.h"
#include "serv_port.h"
#include "serv_port_handlers.h"

UART_HandleTypeDef serv_port_uart_global;     // Service Port UART handle
UART_HandleTypeDef* serv_port_huart;          // pointer to UART handle for convenience
char serv_port_in_buf[SERV_PORT_BUF_SIZE];    // buffer for input
char serv_port_out_buf[SERV_PORT_BUF_SIZE];   // buffer for output
char* serv_port_argv[SERV_PORT_ARGV_SIZE];

static char* serv_port_strtok_ptr;            // buffer for strtok_r function

HAL_StatusTypeDef serv_port_init(void) {
	/* Service Port init */
	serv_port_huart = serv_port_uart_init();
	
	return HAL_OK;
}

UART_HandleTypeDef* serv_port_uart_init(void) {	
	/* Service Port UART init */
	serv_port_uart_global.Instance = SERV_PORT_UART_INSTANCE;
	serv_port_uart_global.Init.BaudRate = SERV_PORT_UART_BAUDRATE;
	serv_port_uart_global.Init.WordLength = UART_WORDLENGTH_8B;
	serv_port_uart_global.Init.StopBits = UART_STOPBITS_1;
	serv_port_uart_global.Init.Parity = UART_PARITY_NONE;
	serv_port_uart_global.Init.Mode = UART_MODE_TX_RX;
	serv_port_uart_global.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  
	if( HAL_UART_Init(&serv_port_uart_global) != HAL_OK ) {
		while(1);
	}
	
	// enable interrupt on character reception
	__HAL_UART_ENABLE_IT(&serv_port_uart_global, UART_IT_RXNE);
	
	return &serv_port_uart_global;
}

void serv_port_irq_handler(UART_HandleTypeDef* huart) {  
}

HAL_StatusTypeDef serv_port_print(UART_HandleTypeDef* huart, const char* str) {
	uint16_t length = (uint16_t) strlen(str);
	
	return HAL_UART_Transmit(huart, (uint8_t*) str, length, SERV_PORT_UART_TIMEOUT);
}

HAL_StatusTypeDef serv_port_gets(UART_HandleTypeDef* huart, char* str, int max_size, char stop_ch) {
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
		while( !(huart->Instance->SR & USART_SR_RXNE) && ( HAL_GetTick() - ticks < SERV_PORT_GETS_TIMEOUT ) );
		
		// timeout occured
		if( HAL_GetTick() - ticks >= SERV_PORT_GETS_TIMEOUT ) {
			// restore SysTick priority
			HAL_NVIC_SetPriority(SysTick_IRQn, TICK_INT_PRIORITY, 0);
			
			huart->State = HAL_UART_STATE_READY;
			__HAL_UNLOCK(huart);
						
			return HAL_TIMEOUT;
		}
		
		*str++ = (char) huart->Instance->DR;
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

HAL_StatusTypeDef serv_port_getb(UART_HandleTypeDef* huart, uint8_t* bt) {
	// set SysTick priority to allow ticks incrementation
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

	uint32_t ticks = HAL_GetTick();
	// wait for RXNE flag until timeout
	while( !(huart->Instance->SR & USART_SR_RXNE) && ( HAL_GetTick() - ticks < SERV_PORT_GETS_TIMEOUT ) );
	
	// timeout occured
	if( HAL_GetTick() - ticks >= SERV_PORT_GETS_TIMEOUT ) {
		// restore SysTick priority
		HAL_NVIC_SetPriority(SysTick_IRQn, TICK_INT_PRIORITY, 0);
		
		return HAL_TIMEOUT;
	} else {
		*bt = huart->Instance->DR;
	}
	
	// restore SysTick priority
	HAL_NVIC_SetPriority(SysTick_IRQn, TICK_INT_PRIORITY, 0);
	
	return HAL_OK;
}

HAL_StatusTypeDef serv_port_process_cmd(char* cmd_str) {
	// search for the command string
	serv_port_argv[0] = strtok_r(cmd_str, SERV_PORT_ARG_DELIMITERS, &serv_port_strtok_ptr);
	if(serv_port_argv[0] == NULL) {
		return HAL_ERROR;
	}
	
	// tokenize arguments list
	int argc = 1;
	FlagStatus end_reached = RESET; 
	for(; argc < SERV_PORT_ARGV_SIZE && !end_reached;) {
		serv_port_argv[argc] = strtok_r(NULL, SERV_PORT_ARG_DELIMITERS, &serv_port_strtok_ptr);
		
		if(serv_port_argv[argc] == NULL) {
			end_reached = SET;
		} else {
			++argc;
		}
	}
	
	// search for command handler and call it if found
	for(int i = 0; i < serv_port_cmd_table_size; ++i) {
		if( strcmp(serv_port_cmd_table[i].cmd_name, serv_port_argv[0]) == 0 ) {
			return serv_port_cmd_table[i].cmd_handler(argc, serv_port_argv);
		}
	}
	
	serv_port_print(serv_port_huart, "error: unknown command\r\n"); 
	return HAL_ERROR;
}
