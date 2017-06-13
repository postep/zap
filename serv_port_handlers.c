#include "serv_port.h"
#include "serv_port_handlers.h"
#include "main.h"
#include "user.h"
#include "my_aes.h"
#include "acc_aes.h"
#include "aes.h"

/* Service Port command handlers table */
Serv_port_cmd_table_entry serv_port_cmd_table[] = {
  { "clrflash", serv_port_clrflash },
  { "setclkres", serv_port_setclkres_handler },
  { "enc", serv_port_enc_handler },
  { "dec", serv_port_dec_handler },
  { "printplain", serv_port_printplain_handler },
  { "printcipher", serv_port_printcipher_handler },
  { "printdecipher", serv_port_printdecipher_handler }
};

uint8_t serv_port_cmd_table_size = sizeof(serv_port_cmd_table) / sizeof(Serv_port_cmd_table_entry);

/* Service Port commands handlers */

HAL_StatusTypeDef serv_port_clrflash(int argc, char* argv[]) {

  HAL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_PIN);
  
  HAL_FLASH_Unlock();
  
  FLASH_EraseInitTypeDef EraseInitStruct;
  uint32_t PageError;
  
  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.PageAddress = (uint32_t)CIPHERTEXT_DATA;
  EraseInitStruct.NbPages     = CIPHERTEXT_SIZE / FLASH_PAGE_SIZE;
  HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
  
  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.PageAddress = (uint32_t)DECIPHERTEXT_DATA;
  EraseInitStruct.NbPages     = DECIPHERTEXT_SIZE / FLASH_PAGE_SIZE;
  HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
  
  HAL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_PIN);
  
  return HAL_OK;
}

HAL_StatusTypeDef serv_port_setclkres_handler(int argc, char* argv[]) {
  if(argc < 2) {
    serv_port_print(serv_port_huart, "Usage: setclkres [res_symbol]\r\n");
    serv_port_print(serv_port_huart, "Where:\r\n\r\n");
    serv_port_print(serv_port_huart, "res_symbol = cc, us, ms:\r\n\r\n");
    
    return HAL_ERROR;
  }
  
  char* symbol = argv[1];
  for (int i = 0; i < perfclk_cfgs_size; ++i) {
    if (strcmp(perfclk_cfgs[i].symbol, symbol) == 0) {
      perfclk_sel = (Perfclk_sel)i;
      uint32_t psc;
      if(perfclk_sel == PERFCLK_CC) {
        psc = 0;
      } else {
        psc = (uint32_t)(SystemCoreClock / perfclk_cfgs[perfclk_sel].res) - 1; 
      }
      
      HAL_TIM_Base_DeInit(perf_htim);
      
      perf_tim.Init.Prescaler         = psc;
      perf_tim.Init.Period            = 0xFFFFFFFF;
      perf_tim.Init.ClockDivision     = 0;
      perf_tim.Init.CounterMode       = TIM_COUNTERMODE_UP;
      HAL_TIM_Base_Init(perf_htim);
      
      snprintf(serv_port_out_buf, SERV_PORT_BUF_SIZE, "setclkres: Clock resolution: %s (%i Hz)\r\n\r\n", perfclk_cfgs[i].symbol, perfclk_cfgs[i].res);
      serv_port_print(serv_port_huart, serv_port_out_buf); 
    
      return HAL_OK;
    }
  }
  
  serv_port_print(serv_port_huart, "setcllkres: Unrecgnized clock resolution symbol\r\n");

  return HAL_OK;
}

HAL_StatusTypeDef serv_port_enc_handler(int argc, char* argv[]) {
  if(argc < 4) {
    serv_port_print(serv_port_huart, "Usage: enc [size] [size_unit] [impl] [memtype]\r\n");
    serv_port_print(serv_port_huart, "Where:\r\n\r\n");
    serv_port_print(serv_port_huart, "size - size in blocks or kilobytes\r\b");
    serv_port_print(serv_port_huart, "size_unit = blocks, kB\r\b");
    serv_port_print(serv_port_huart, "impl = acc (hardware accelerated), mbedtls (mbedTLS impl.), custom (custom impl.)\r\b");
    serv_port_print(serv_port_huart, "memtype = [optional, default: discard] flash, ram, discard\r\n\r\n");
    
    return HAL_ERROR;
  }
  
  long int size = 0;
  if (str_to_int(argv[1], &size) != HAL_OK) {
    serv_port_print(serv_port_huart, "enc: unable to parse size\r\n\r\n");
    
    return HAL_ERROR;
  }
  
  Size_gran size_gran;
  int byte_mult = 0;
  if (strcmp(argv[2], size_gran_symbols[SIZE_GRAN_BLOCKS]) == 0) {
    size_gran = SIZE_GRAN_BLOCKS;
    byte_mult = 0x10;
  } else if(strcmp(argv[2], size_gran_symbols[SIZE_GRAN_KILOBYTES]) == 0) {
    size_gran = SIZE_GRAN_KILOBYTES;
    byte_mult = 0x400;
  } else {
    serv_port_print(serv_port_huart, "enc: incorrect size_unit\r\n\r\n");
    
    return HAL_ERROR;
  }
  
  AES_impl aes_impl;
  if(strcmp(argv[3], aes_impl_symbols[AESIMPL_ACCELERATED]) == 0) {
    aes_impl = AESIMPL_ACCELERATED;
  } else if(strcmp(argv[3], aes_impl_symbols[AESIMPL_MBEDTLS]) == 0) {
    aes_impl = AESIMPL_MBEDTLS;
  } else if(strcmp(argv[3], aes_impl_symbols[AESIMPL_CUSTOM]) == 0) {
    aes_impl = AESIMPL_CUSTOM;
  } else {
    serv_port_print(serv_port_huart, "enc: incorrect impl\r\n\r\n");
  
    return HAL_ERROR;
  }
  
  AES_memtype memtype = AES_MEMTYPE_DISCARD;
  if(argc > 4) {
    if(strcmp(argv[4], memtype_symbols[AES_MEMTYPE_FLASH]) == 0) {
      memtype = AES_MEMTYPE_FLASH;
    } else if(strcmp(argv[4], memtype_symbols[AES_MEMTYPE_RAM]) == 0) {
      memtype = AES_MEMTYPE_RAM;
    } else if(strcmp(argv[4], memtype_symbols[AES_MEMTYPE_DISCARD]) == 0) {
      memtype = AES_MEMTYPE_DISCARD;
    } else {
      serv_port_print(serv_port_huart, "enc: incorrect memtype\r\n\r\n");
    
      return HAL_ERROR;
    }
  }
  
  if(memtype != AES_MEMTYPE_DISCARD && !is_in_range_int(size * byte_mult, 0, memspace_map[DATATYPE_CIPHERTEXT][memtype].size)) {
    serv_port_print(serv_port_huart, "enc: data to encrypt/decrypt is too large or negative\r\n\r\n");

    return HAL_ERROR;
  }
  
  snprintf(serv_port_out_buf, SERV_PORT_BUF_SIZE, "Encryption START (size = %i %s, impl = %s, memtype = %s)\r\n\r\n",
    (int)size, size_gran_symbols[size_gran], aes_impl_symbols[aes_impl], memtype_symbols[memtype]);
  serv_port_print(serv_port_huart, serv_port_out_buf);
  
  /* Flash erase if needed */ 
  if(memtype == AES_MEMTYPE_FLASH) {
    serv_port_print(serv_port_huart, "Erasing Flash memory... (CIPHERTEXT sector)\r\n");
    HAL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_PIN);    
    flash_erase((uint32_t)CIPHERTEXT_DATA, CIPHERTEXT_SIZE / FLASH_PAGE_SIZE);
    HAL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_PIN);    
    serv_port_print(serv_port_huart, "Flash memory ERASED.\r\n\r\n");
    
    HAL_FLASH_Unlock();
  }
 
  // start timer
  perf_htim->Instance->CNT = 0;
  HAL_TIM_Base_Start(perf_htim);

  switch(aes_impl) {
    case AESIMPL_ACCELERATED: {
        acc_aes128_enc_cbc(aes_key_global, aes_init_vec_global, PLAINTEXT_DATA, size * byte_mult, aes_state_global, (uint32_t*)CIPHERTEXT_DATA, memtype);
      }
      break;
    case AESIMPL_MBEDTLS: {
          serv_port_print(serv_port_huart, "mbedTLS implementation not supported\r\n\r\n");
        }
      break;
    case AESIMPL_CUSTOM: {
        AES_rkeys rkeys = aes_rkeys_global;
        aes128_expand_key(aes_key_global, rkeys);
        aes128_enc_cbc(rkeys, aes_init_vec_global, PLAINTEXT_DATA, size * byte_mult, aes_state_global, (uint32_t*)CIPHERTEXT_DATA, memtype);
      }
      break;
    default:
      return HAL_ERROR;
  }
  
  //stop timer
  HAL_TIM_Base_Stop(perf_htim);
  uint32_t ticks = perf_htim->Instance->CNT;
  
  if(memtype == AES_MEMTYPE_FLASH) {
    HAL_FLASH_Lock();
  }
  
  snprintf(serv_port_out_buf, SERV_PORT_BUF_SIZE, "Encryption END. Elapsed: %u %s\r\n\r\n", ticks, perfclk_cfgs[perfclk_sel].symbol);
  serv_port_print(serv_port_huart, serv_port_out_buf);
  
  return HAL_OK;
}

HAL_StatusTypeDef serv_port_dec_handler(int argc, char* argv[]) {
  if(argc < 4) {
    serv_port_print(serv_port_huart, "Usage: dec [size] [size_unit] [impl] [memtype]\r\n");
    serv_port_print(serv_port_huart, "Where:\r\n\r\n");
    serv_port_print(serv_port_huart, "size - size in blocks or kilobytes\r\b");
    serv_port_print(serv_port_huart, "size_unit = blocks, kB\r\b");
    serv_port_print(serv_port_huart, "impl = acc (hardware accelerated), mbedtls (mbedTLS impl.), custom (custom impl.)\r\b");
    serv_port_print(serv_port_huart, "memtype = [optional, default: discard] flash, ram, discard\r\n\r\n");
    
    return HAL_ERROR;
  }
  
  long int size = 0;
  if (str_to_int(argv[1], &size) != HAL_OK) {
    serv_port_print(serv_port_huart, "dec: unable to parse size\r\n\r\n");
    
    return HAL_ERROR;
  }
  
  Size_gran size_gran;
  int byte_mult = 0;
  if (strcmp(argv[2], size_gran_symbols[SIZE_GRAN_BLOCKS]) == 0) {
    size_gran = SIZE_GRAN_BLOCKS;
    byte_mult = 0x10;
  } else if(strcmp(argv[2], size_gran_symbols[SIZE_GRAN_KILOBYTES]) == 0) {
    size_gran = SIZE_GRAN_KILOBYTES;
    byte_mult = 0x400;
  } else {
    serv_port_print(serv_port_huart, "dec: incorrect size_unit\r\n\r\n");
    
    return HAL_ERROR;
  }
  
  AES_impl aes_impl;
  if(strcmp(argv[3], aes_impl_symbols[AESIMPL_ACCELERATED]) == 0) {
    aes_impl = AESIMPL_ACCELERATED;
  } else if(strcmp(argv[3], aes_impl_symbols[AESIMPL_MBEDTLS]) == 0) {
    aes_impl = AESIMPL_MBEDTLS;
  } else if(strcmp(argv[3], aes_impl_symbols[AESIMPL_CUSTOM]) == 0) {
    aes_impl = AESIMPL_CUSTOM;
  } else {
    serv_port_print(serv_port_huart, "dec: incorrect impl\r\n\r\n");
  
    return HAL_ERROR;
  }
  
  AES_memtype memtype = AES_MEMTYPE_DISCARD;
  if(argc > 4) {
    if(strcmp(argv[4], memtype_symbols[AES_MEMTYPE_FLASH]) == 0) {
      memtype = AES_MEMTYPE_FLASH;
    } else if(strcmp(argv[4], memtype_symbols[AES_MEMTYPE_RAM]) == 0) {
      memtype = AES_MEMTYPE_RAM;
    } else if(strcmp(argv[4], memtype_symbols[AES_MEMTYPE_DISCARD]) == 0) {
      memtype = AES_MEMTYPE_DISCARD;
    } else {
      serv_port_print(serv_port_huart, "dec: incorrect memtype\r\n\r\n");
    
      return HAL_ERROR;
    }
  }
  
  if(memtype != AES_MEMTYPE_DISCARD && !is_in_range_int(size * byte_mult, 0, memspace_map[DATATYPE_CIPHERTEXT][memtype].size)) {
    serv_port_print(serv_port_huart, "dec: data to encrypt/decrypt is too large or negative\r\n\r\n");

    return HAL_ERROR;
  }
  
  snprintf(serv_port_out_buf, SERV_PORT_BUF_SIZE, "Decryption START (size = %i %s, impl = %s, memtype = %s)\r\n\r\n",
    (int)size, size_gran_symbols[size_gran], aes_impl_symbols[aes_impl], memtype_symbols[memtype]);
  serv_port_print(serv_port_huart, serv_port_out_buf);
  
  /* Flash erase if needed */ 
  if(memtype == AES_MEMTYPE_FLASH) {
    serv_port_print(serv_port_huart, "Erasing Flash memory... (DECIPHERTEXT sector)\r\n");
    HAL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_PIN);    
    flash_erase((uint32_t)DECIPHERTEXT_DATA, DECIPHERTEXT_SIZE / FLASH_PAGE_SIZE);
    HAL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_PIN);    
    serv_port_print(serv_port_huart, "Flash memory ERASED.\r\n\r\n");
    
    HAL_FLASH_Unlock();
  }
 
  // start timer
  perf_htim->Instance->CNT = 0;
  HAL_TIM_Base_Start(perf_htim);

  switch(aes_impl) {
    case AESIMPL_ACCELERATED: {
        acc_aes128_dec_cbc(aes_key_global, aes_init_vec_global, CIPHERTEXT_DATA, size * byte_mult, aes_state_global, (uint32_t*)DECIPHERTEXT_DATA, memtype);
      }
      break;
    case AESIMPL_MBEDTLS: {
          serv_port_print(serv_port_huart, "mbedTLS implementation not supported\r\n\r\n");
        }
      break;
    case AESIMPL_CUSTOM: {
        AES_rkeys rkeys = aes_rkeys_global;
        aes128_expand_key(aes_key_global, rkeys);
        aes128_dec_cbc(rkeys, aes_init_vec_global, CIPHERTEXT_DATA, size * byte_mult, aes_state_global, (uint32_t*)DECIPHERTEXT_DATA, memtype);
      }
      break;
    default:
      return HAL_ERROR;
  }
  
  //stop timer
  HAL_TIM_Base_Stop(perf_htim);
  uint32_t ticks = perf_htim->Instance->CNT;
  
  if(memtype == AES_MEMTYPE_FLASH) {
    HAL_FLASH_Lock();
  }
  
  snprintf(serv_port_out_buf, SERV_PORT_BUF_SIZE, "Decryption END. Elapsed: %u %s\r\n\r\n", ticks, perfclk_cfgs[perfclk_sel].symbol);
  serv_port_print(serv_port_huart, serv_port_out_buf);
  
  return HAL_OK;
}

HAL_StatusTypeDef serv_port_printplain_handler(int argc, char* argv[]) {
  if(argc < 2) {
    serv_port_print(serv_port_huart, "Usage: printplain [size] [size_unit]\r\n");
    serv_port_print(serv_port_huart, "Where:\r\n\r\n");
    serv_port_print(serv_port_huart, "size - size in blocks or kilobytes\r\b");
    serv_port_print(serv_port_huart, "size_unit = blocks, kB\r\b");
    
    return HAL_ERROR;
  }
  
  long int size = 0;
  if (str_to_int(argv[1], &size) != HAL_OK) {
    serv_port_print(serv_port_huart, "printplain: unable to parse size\r\n\r\n");
    
    return HAL_ERROR;
  }
  
  Size_gran size_gran;
  int byte_mult = 0;
  if (strcmp(argv[2], size_gran_symbols[SIZE_GRAN_BLOCKS]) == 0) {
    size_gran = SIZE_GRAN_BLOCKS;
    byte_mult = 0x10;
  } else if(strcmp(argv[2], size_gran_symbols[SIZE_GRAN_KILOBYTES]) == 0) {
    size_gran = SIZE_GRAN_KILOBYTES;
    byte_mult = 0x400;
  } else {
    serv_port_print(serv_port_huart, "printplain: incorrect size_unit\r\n\r\n");
    
    return HAL_ERROR;
  }
  
  long int printed = size * byte_mult;
  for (uint8_t* p = PLAINTEXT_DATA; printed > 0; p += AES_BLOCK_SIZE, printed -= AES_BLOCK_SIZE) {
    snprintf(serv_port_out_buf, SERV_PORT_BUF_SIZE, "%c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c\r\n",
      p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
    serv_port_print(serv_port_huart, serv_port_out_buf);
  }
  
  serv_port_print(serv_port_huart, "\r\n");

  return HAL_OK;
}

HAL_StatusTypeDef serv_port_printcipher_handler(int argc, char* argv[]) {
  if(argc < 2) {
    serv_port_print(serv_port_huart, "Usage: printcipher [size] [size_unit]\r\n");
    serv_port_print(serv_port_huart, "Where:\r\n\r\n");
    serv_port_print(serv_port_huart, "size - size in blocks or kilobytes\r\b");
    serv_port_print(serv_port_huart, "size_unit = blocks, kB\r\b");
    
    return HAL_ERROR;
  }
  
  long int size = 0;
  if (str_to_int(argv[1], &size) != HAL_OK) {
    serv_port_print(serv_port_huart, "printcipher: unable to parse size\r\n\r\n");
    
    return HAL_ERROR;
  }
  
  Size_gran size_gran;
  int byte_mult = 0;
  if (strcmp(argv[2], size_gran_symbols[SIZE_GRAN_BLOCKS]) == 0) {
    size_gran = SIZE_GRAN_BLOCKS;
    byte_mult = 0x10;
  } else if(strcmp(argv[2], size_gran_symbols[SIZE_GRAN_KILOBYTES]) == 0) {
    size_gran = SIZE_GRAN_KILOBYTES;
    byte_mult = 0x400;
  } else {
    serv_port_print(serv_port_huart, "printcipher: incorrect size_unit\r\n\r\n");
    
    return HAL_ERROR;
  }
  
  long int printed = size * byte_mult;
  for (uint8_t* p = CIPHERTEXT_DATA; printed > 0; p += AES_BLOCK_SIZE, printed -= AES_BLOCK_SIZE) {
    snprintf(serv_port_out_buf, SERV_PORT_BUF_SIZE, "%c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c\r\n",
      p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
    serv_port_print(serv_port_huart, serv_port_out_buf);
  }
  
  serv_port_print(serv_port_huart, "\r\n");

  return HAL_OK;
}

HAL_StatusTypeDef serv_port_printdecipher_handler(int argc, char* argv[]) {
  if(argc < 2) {
    serv_port_print(serv_port_huart, "Usage: printdecipher [size] [size_unit]\r\n");
    serv_port_print(serv_port_huart, "Where:\r\n\r\n");
    serv_port_print(serv_port_huart, "size - size in blocks or kilobytes\r\b");
    serv_port_print(serv_port_huart, "size_unit = blocks, kB\r\b");
    
    return HAL_ERROR;
  }
  
  long int size = 0;
  if (str_to_int(argv[1], &size) != HAL_OK) {
    serv_port_print(serv_port_huart, "printdecipher: unable to parse size\r\n\r\n");
    
    return HAL_ERROR;
  }
  
  Size_gran size_gran;
  int byte_mult = 0;
  if (strcmp(argv[2], size_gran_symbols[SIZE_GRAN_BLOCKS]) == 0) {
    size_gran = SIZE_GRAN_BLOCKS;
    byte_mult = 0x10;
  } else if(strcmp(argv[2], size_gran_symbols[SIZE_GRAN_KILOBYTES]) == 0) {
    size_gran = SIZE_GRAN_KILOBYTES;
    byte_mult = 0x400;
  } else {
    serv_port_print(serv_port_huart, "printdecipher: incorrect size_unit\r\n\r\n");
    
    return HAL_ERROR;
  }
  
  long int printed = size * byte_mult;
  for (uint8_t* p = DECIPHERTEXT_DATA; printed > 0; p += AES_BLOCK_SIZE, printed -= AES_BLOCK_SIZE) {
    snprintf(serv_port_out_buf, SERV_PORT_BUF_SIZE, "%c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c\r\n",
      p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
    serv_port_print(serv_port_huart, serv_port_out_buf);
  }
  
  serv_port_print(serv_port_huart, "\r\n");

  return HAL_OK;
}

