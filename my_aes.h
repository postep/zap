#ifndef MY_AES_H
#define MY_AES_H

#include <stdint.h>
#include <stm32l1xx_hal_flash.h>

/* Defines */
#define AES_STATE_COLS              4
#define AES_STATE_ROWS              4
#define AES_STATE_ELEMS             16
#define AES_KEY_SIZE                16
#define AES_KEY_WSIZE               4
#define AES_BLOCK_SIZE              16
#define AES_BLOCK_WSIZE             4
#define AES_SBOX_SIZE               256
#define AES_RCON_SIZE               256
#define AES_ROUNDS                  10
#define AES_MEMTYPE_COUNT           3
#define AES_CIPHERTEXT_BUF_SIZE     256
#define AES_DECIPHERTEXT_BUF_SIZE   256

/* Type aliases */
typedef uint8_t (*AES_state)[AES_STATE_COLS];
typedef uint32_t* AES_rkey;
typedef uint32_t (*AES_rkeys)[AES_KEY_WSIZE];

/* Enums */
typedef enum {
  AES_MEMTYPE_FLASH,
  AES_MEMTYPE_RAM,
  AES_MEMTYPE_DISCARD
} AES_memtype;

/* Macros */
#define AES_SAVE_STATE_FLASH(p_addr32, p_state32) {                               \
    uint32_t v_out_addr = (uint32_t)(p_addr32);                                   \
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, v_out_addr, (p_state32)[0]);        \
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, v_out_addr + 4, (p_state32)[1]);    \
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, v_out_addr + 8, (p_state32)[2]);    \
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, v_out_addr + 12, (p_state32)[3]);   \
  }

#define AES_SAVE_STATE_RAM(p_addr32, p_state32) {   \
    (p_addr32)[0] = p_state32[0];                   \
    (p_addr32)[1] = p_state32[1];                   \
    (p_addr32)[2] = p_state32[2];                   \
    (p_addr32)[3] = p_state32[3];                   \
  }

#define AES_SAVE_STATE(p_addr32, p_state32, memtype) {    \
    if((memtype) == AES_MEMTYPE_FLASH) {                  \
      AES_SAVE_STATE_FLASH(out_addr, wstate);             \
    } else if((memtype) == AES_MEMTYPE_RAM) {             \
      AES_SAVE_STATE_RAM(out_addr, wstate);               \
    }                                                     \
  }
  
#define AES_ARG_CHECK(size) {                             \
    if(size == 0) {                                       \
      return;                                             \
    }                                                     \
    unsigned int excess_bytes = size % AES_BLOCK_SIZE;    \
    if(excess_bytes != 0) {                               \
      size -= excess_bytes;                               \
    }                                                     \
  }
  
/* Symbol definitions */
extern uint32_t aes_rkeys_global[AES_ROUNDS + 1][AES_KEY_WSIZE];
extern uint8_t aes_key_global[AES_KEY_SIZE];
extern uint8_t aes_init_vec_global[AES_BLOCK_SIZE];
extern uint8_t aes_state_global[AES_STATE_ELEMS];
extern uint8_t aes_ciphertext_buf[AES_CIPHERTEXT_BUF_SIZE];
extern uint8_t aes_deciphertext_buf[AES_DECIPHERTEXT_BUF_SIZE];

/* Function declarations */
void aes128_expand_key(uint8_t* key, AES_rkeys rkeys);
void aes128_enc_ecb_discard(AES_rkeys rkeys, uint8_t* data, size_t size, uint8_t* buf);
void aes128_dec_ecb_discard(AES_rkeys rkeys, uint8_t* data, size_t size, uint8_t* buf);
void aes128_enc_ecb(AES_rkeys rkeys, uint8_t* data, size_t size, uint8_t* buf, uint32_t* out_addr, AES_memtype memtype);
void aes128_dec_ecb(AES_rkeys rkeys, uint8_t* data, size_t size, uint8_t* buf, uint32_t* out_addr, AES_memtype memtype);
void aes128_enc_cbc(AES_rkeys rkeys, uint8_t* iv, uint8_t* data, size_t size, uint8_t* buf, uint32_t* out_addr, AES_memtype memtype);
void aes128_dec_cbc(AES_rkeys rkeys, uint8_t* iv, uint8_t* data, size_t size, uint8_t* buf, uint32_t* out_addr, AES_memtype memtype);

#endif // MY_AES_H
