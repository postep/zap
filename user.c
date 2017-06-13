#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include "main.h"
#include "user.h"
#include "my_aes.h"

Perfclk_cfg perfclk_cfgs[] = {
  { "us", 1000000 },
  { "ms", 1000 },
  { "cc", 0 }
};

uint8_t perfclk_cfgs_size = sizeof(perfclk_cfgs) / sizeof(Perfclk_cfg);
Perfclk_sel perfclk_sel = PERFCLK_CC; 

const char* aes_impl_symbols[] = {
  "acc",
  "mbedtls",
  "custom"
};

const char* size_gran_symbols[] = {
  "blocks",
  "kB"
};

const char* memtype_symbols[] = {
  "flash",
  "ram",
  "discard"
};

Memspace_desc memspace_map[DATATYPE_COUNT][AES_MEMTYPE_COUNT];

void flash_erase(uint32_t addr, uint32_t pages_count) {
  FLASH_EraseInitTypeDef EraseInitStruct;
  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.PageAddress = addr;
  EraseInitStruct.NbPages     = pages_count;
  
  uint32_t PageError;
  
  HAL_FLASH_Unlock();
  HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
  HAL_FLASH_Lock();
}

HAL_StatusTypeDef str_to_int(char* str, long int* out_val) {
	// strtol wrapper: convert str to long int
	errno = 0;
	char* temp = 0;
	*out_val = strtol(str, &temp, 0);

	if((temp == str || *temp != '\0') || ((*out_val == LONG_MIN || *out_val == LONG_MAX) && errno == ERANGE)) {
		return HAL_ERROR;
	}
	
	return HAL_OK;
}

HAL_StatusTypeDef str_to_double(char* str, double* out_val) {
	// strtod wrapper: convert str to double
	errno = 0;
	char* temp = 0;
	*out_val = strtod(str, &temp);
	
	if((temp == str || *temp != '\0') || ((*out_val == -HUGE_VAL || *out_val == HUGE_VAL) && errno == ERANGE)) {
		return HAL_ERROR;
	}		
	
	return HAL_OK;
}
 
int is_in_range_int(int val, int lo_bound, int up_bound) {
	return (val >= lo_bound && val <= up_bound);
}

int is_in_range_float(float val, float lo_bound, float up_bound) {
	return ( val >= lo_bound && val <= up_bound );
}

int is_in_range_double(double val, double lo_bound, double up_bound) {
	return ( val >= lo_bound && val <= up_bound );
}
