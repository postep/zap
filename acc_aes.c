#include "main.h"
#include "acc_aes.h"

CRYP_HandleTypeDef acc_aes_global;
CRYP_HandleTypeDef acc_aes;

HAL_StatusTypeDef acc_aes_init(void) {
  /* AES hardware encryption */
  acc_aes_global.Instance = AES;
  acc_aes_global.Init.DataType = CRYP_DATATYPE_8B;
  acc_aes_global.Init.pKey = key;
  acc_aes_global.Init.pInitVect = init_vec;
  
  if(HAL_CRYP_Init(&acc_aes_global) == HAL_ERROR) {
    while(1);
  }
  
  return HAL_OK;
}

void acc_aes128_enc_ecb(uint8_t* key, uint8_t* data, size_t size, uint8_t* buf, uint32_t* out_addr, AES_memtype memtype) {
  AES_ARG_CHECK(size);
  
  // AES reset neccessary before processing the next chunk of data
  ACC_AES_RESET();
  
  AES_state state = (AES_state)buf;
  uint32_t* wstate = (uint32_t*) state;
  for (uint8_t* p = data; size > 0; p += AES_BLOCK_SIZE, size -= AES_BLOCK_SIZE, out_addr += AES_BLOCK_WSIZE) {
    HAL_CRYP_AESECB_Encrypt(&acc_aes_global, p, AES_BLOCK_SIZE, buf, ACC_AES_TIMEOUT);
    
    AES_SAVE_STATE(out_addr, wstate, memtype);
  }
}

void acc_aes128_dec_ecb(uint8_t* key, uint8_t* data, size_t size, uint8_t* buf, uint32_t* out_addr, AES_memtype memtype) {
  AES_ARG_CHECK(size);
  
  // AES reset neccessary before processing the next chunk of data
  ACC_AES_RESET();
  
  AES_state state = (AES_state)buf;
  uint32_t* wstate = (uint32_t*) state;
  for (uint8_t* p = data; size > 0; p += AES_BLOCK_SIZE, size -= AES_BLOCK_SIZE, out_addr += AES_BLOCK_WSIZE) {
    HAL_CRYP_AESECB_Decrypt(&acc_aes_global, p, AES_BLOCK_SIZE, buf, ACC_AES_TIMEOUT);
    
    AES_SAVE_STATE(out_addr, wstate, memtype);
  }
}

void acc_aes128_enc_cbc(uint8_t* key, uint8_t* iv, uint8_t* data, size_t size, uint8_t* buf, uint32_t* out_addr, AES_memtype memtype) {
  AES_ARG_CHECK(size);
  
  // AES reset neccessary before processing the next chunk of data
  ACC_AES_RESET();
  
  AES_state state = (AES_state)buf;
  uint32_t* wstate = (uint32_t*)state;
  memcpy(state, data, AES_BLOCK_SIZE);
  
  // XOR 1st block with init vector
  uint32_t* wiv = (uint32_t*)iv;

  wstate[0] ^= wiv[0];
  wstate[1] ^= wiv[1];
  wstate[2] ^= wiv[2];
  wstate[3] ^= wiv[3];
  
  //aes128_enc_block(state, rkeys);
  HAL_CRYP_AESECB_Encrypt(&acc_aes_global, (uint8_t*)state, AES_BLOCK_SIZE, buf, ACC_AES_TIMEOUT);
  
  AES_SAVE_STATE(out_addr, wstate, memtype);
  size -= AES_BLOCK_SIZE;
  
  for(uint32_t* wp = (uint32_t*)data; size > 0; wp += AES_BLOCK_WSIZE, size -= AES_BLOCK_SIZE) {
    // copy block & encrypt in place
    memcpy(state, wp + 4, AES_BLOCK_SIZE);
    
    // XOR with previously encrypted block
    wstate[0] ^= out_addr[0];
    wstate[1] ^= out_addr[1];
    wstate[2] ^= out_addr[2];
    wstate[3] ^= out_addr[3];
    
    //aes128_enc_block(state, rkeys);
    HAL_CRYP_AESECB_Encrypt(&acc_aes_global, (uint8_t*)state, AES_BLOCK_SIZE, buf, ACC_AES_TIMEOUT);
    
    out_addr += AES_BLOCK_WSIZE;
    AES_SAVE_STATE(out_addr, wstate, memtype);
  }
}

void acc_aes128_dec_cbc(uint8_t* key, uint8_t* iv, uint8_t* data, size_t size, uint8_t* buf, uint32_t* out_addr, AES_memtype memtype) {
  AES_ARG_CHECK(size);
  
  // AES reset neccessary before processing the next chunk of data
  ACC_AES_RESET();
  
  out_addr += size / 4 - AES_BLOCK_WSIZE; 
  AES_state state = (AES_state)buf;
  uint32_t* wstate = (uint32_t*)state;
  size -= AES_BLOCK_SIZE;
  
  for(uint32_t* wp = (uint32_t*)(data + size - AES_BLOCK_SIZE); size > 0; wp -= AES_BLOCK_WSIZE, size -= AES_BLOCK_SIZE, out_addr -= AES_BLOCK_WSIZE) {
    // copy block & decrypt in place
    memcpy(state, wp + 4, AES_BLOCK_SIZE);
    //aes128_dec_block(state, rkeys);
    HAL_CRYP_AESECB_Decrypt(&acc_aes_global, (uint8_t*)state, AES_BLOCK_SIZE, buf, ACC_AES_TIMEOUT);
    
    // XOR with previously encrypted block
    wstate[0] ^= wp[0];
    wstate[1] ^= wp[1];
    wstate[2] ^= wp[2];
    wstate[3] ^= wp[3];
    
    AES_SAVE_STATE(out_addr, wstate, memtype); 
  }
  
  memcpy(state, data, AES_BLOCK_SIZE);
  //aes128_dec_block(state, rkeys);
  HAL_CRYP_AESECB_Decrypt(&acc_aes_global, (uint8_t*)state, AES_BLOCK_SIZE, buf, ACC_AES_TIMEOUT);
  
  // XOR 1st block with init vector
  uint32_t* wiv = (uint32_t*)iv;
  wstate[0] ^= wiv[0];
  wstate[1] ^= wiv[1];
  wstate[2] ^= wiv[2];
  wstate[3] ^= wiv[3];
  
  AES_SAVE_STATE(out_addr, wstate, memtype); 
}
