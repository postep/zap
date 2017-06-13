#ifndef __SERV_PORT_H
#define __SERV_PORT_H

#include "main.h"
#include "stm32l1xx_hal.h"
#include "stm32l1xx_nucleo.h"

/* Service Port parameters */
#define SERV_PORT_UART_INSTANCE     USARTx
#define SERV_PORT_UART_BAUDRATE     115200
#define SERV_PORT_UART_TIMEOUT      1000
#define SERV_PORT_UART_NO_TIMEOUT   0xFFFFFFFF
#define SERV_PORT_GETS_TIMEOUT      5
#define SERV_PORT_GETB_TIMEOUT      5
#define SERV_PORT_BUF_SIZE          128
#define SERV_PORT_ARGV_SIZE         8
#define SERV_PORT_ARG_DELIMITERS    (" \r\n\t")

typedef struct {
	const char* cmd_name;
	HAL_StatusTypeDef (*cmd_handler)(int argc, char* argv[]);
} Serv_port_cmd_table_entry;

/* Service Port declarations */
extern UART_HandleTypeDef serv_port_uart_global;
extern UART_HandleTypeDef* serv_port_huart;
extern char serv_port_in_buf[SERV_PORT_BUF_SIZE];
extern char serv_port_out_buf[SERV_PORT_BUF_SIZE];
extern Serv_port_cmd_table_entry serv_port_cmd_table[];
extern char* serv_port_argv[SERV_PORT_ARGV_SIZE];

/* Service Port functions */
HAL_StatusTypeDef serv_port_init(void);
UART_HandleTypeDef* serv_port_uart_init(void);
HAL_StatusTypeDef serv_port_process_cmd(char* cmd_str);
HAL_StatusTypeDef serv_port_print(UART_HandleTypeDef* huart, const char* str);
HAL_StatusTypeDef serv_port_gets(UART_HandleTypeDef* huart, char* str, int max_size, char stop_ch);
HAL_StatusTypeDef serv_port_getb(UART_HandleTypeDef* huart, uint8_t* bt);
void serv_port_putb(UART_HandleTypeDef* huart, uint8_t bt);

#endif
