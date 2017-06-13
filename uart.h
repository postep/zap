/*
 ************************************ AFCS *************************************
 *
 *	File			: uart.h
 *	Author			: Borys Jelenski
 *	Version			: 1.0
 *	Date			: 26-08-2015
 *	Description		: UART configuration & function prototypes
 *
 *******************************************************************************
 */

#ifndef __UART_H
#define __UART_H

#include "stm32l1xx_hal.h"
#include "stm32l1xx_hal_uart.h"
#include "stm32l1xx_nucleo.h"


/* UART parameters */
#define UART_TIMEOUT 					1000
#define UART_NO_TIMEOUT 			0xFFFFFFFF
#define UART_GETS_TIMEOUT			5
#define UART_GETB_TIMEOUT			5
#define UART_BUF_SIZE					64

/* UART declarations */
extern UART_HandleTypeDef usart2_global;
extern UART_HandleTypeDef* husart2;
extern char uart_in_buf[UART_BUF_SIZE];
extern char uart_out_buf[UART_BUF_SIZE];

/* UART functions */
UART_HandleTypeDef* usart2_init(void);
HAL_StatusTypeDef uart_print(UART_HandleTypeDef* huart, const char* str);
HAL_StatusTypeDef uart_gets(UART_HandleTypeDef* huart, char* str, int max_size, char stop_ch);
HAL_StatusTypeDef uart_getb(UART_HandleTypeDef* huart, uint8_t* bt);
void uart_putb(UART_HandleTypeDef* huart, uint8_t bt);

#endif
