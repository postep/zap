#ifndef __SERV_PORT_HANDLERS
#define __SERV_PORT_HANDLERS

#include "serv_port.h"

extern Serv_port_cmd_table_entry serv_port_cmd_table[];
extern uint8_t serv_port_cmd_table_size;

/* Service Port commands handlers */
HAL_StatusTypeDef serv_port_clrflash(int argc, char* argv[]);
HAL_StatusTypeDef serv_port_setclkres_handler(int argc, char* argv[]);
HAL_StatusTypeDef serv_port_enc_handler(int argc, char* argv[]);
HAL_StatusTypeDef serv_port_dec_handler(int argc, char* argv[]);
HAL_StatusTypeDef serv_port_printplain_handler(int argc, char* argv[]);
HAL_StatusTypeDef serv_port_printcipher_handler(int argc, char* argv[]);
HAL_StatusTypeDef serv_port_printdecipher_handler(int argc, char* argv[]);

#endif
