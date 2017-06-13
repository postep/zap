#include <stm32l1xx_hal.h>
#include <stm32l1xx_hal_uart.h>
#include "serv_port.h"

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance == SERV_PORT_UART_INSTANCE) {
		// get command string from UART
		HAL_StatusTypeDef status = serv_port_gets(huart, serv_port_in_buf, SERV_PORT_BUF_SIZE, '\r'); 
    
		switch(status) {
			case HAL_OK:
				serv_port_process_cmd(serv_port_in_buf);
				break;
			case HAL_TIMEOUT:
				serv_port_print(huart, "error: command must end with CR\r\n");
				break;
			default:
				serv_port_print(huart, "error: command error\r\n");
		}
    
    // clear all UART error flags by register read sequence
    uint8_t t = huart->Instance->SR;
    t = huart->Instance->DR;
  }
}
