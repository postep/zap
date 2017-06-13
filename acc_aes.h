#ifndef __ACC_AES_H
#define __ACC_AES_H

#include "main.h"

/* Defines */
#define ACC_AES_TIMEOUT   1000

/* Macros */
#define ACC_AES_RESET() {             \
  HAL_CRYP_DeInit(&acc_aes_global);   \
  HAL_CRYP_Init(&acc_aes_global);     \
}                                     \

/* Symbol declarations */
extern CRYP_HandleTypeDef acc_aes_global;
extern CRYP_HandleTypeDef acc_aes;

/* Function declarations */
HAL_StatusTypeDef acc_aes_init(void);
void acc_aes128_set_key(uint8_t* key);
void acc_aes128_enc_block(AES_state state);
void acc_aes128_dec_block(AES_state state);
void acc_aes128_enc_ecb(uint8_t* key, uint8_t* data, size_t size, uint8_t* buf, uint32_t* out_addr, AES_memtype memtype);
void acc_aes128_dec_ecb(uint8_t* key, uint8_t* data, size_t size, uint8_t* buf, uint32_t* out_addr, AES_memtype memtype);
void acc_aes128_enc_cbc(uint8_t* key, uint8_t* iv, uint8_t* data, size_t size, uint8_t* buf, uint32_t* out_addr, AES_memtype memtype);
void acc_aes128_dec_cbc(uint8_t* key, uint8_t* iv, uint8_t* data, size_t size, uint8_t* buf, uint32_t* out_addr, AES_memtype memtype);

#endif
