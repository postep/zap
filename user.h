#ifndef __USER_H
#define __USER_H

#include <stdint.h>

/* Defines */
#define DATATYPE_COUNT    3

/* Enum definitions */
typedef enum {
  DATATYPE_PLAINTEXT,
  DATATYPE_CIPHERTEXT,
  DATATYPE_DECIPHERTEXT
} Datatype;

typedef enum {
  PERFCLK_US,
  PERFCLK_MS,
  PERFCLK_CC,
} Perfclk_sel;

typedef enum {
  AESIMPL_ACCELERATED,
  AESIMPL_MBEDTLS,
  AESIMPL_CUSTOM,
} AES_impl;

typedef enum {
  SIZE_GRAN_BLOCKS,
  SIZE_GRAN_KILOBYTES
} Size_gran;

/* Structure definitions */
typedef struct {
  const char* symbol;
  uint32_t res;
} Perfclk_cfg;

typedef struct {
  uint32_t base;
  uint32_t size;
} Memspace_desc;

/* Symbol declarations */
extern Perfclk_cfg perfclk_cfgs[];
extern uint8_t perfclk_cfgs_size;
extern Perfclk_sel perfclk_sel; 
extern const char* aes_impl_symbols[];
extern const char* size_gran_symbols[];
extern const char* memtype_symbols[];
extern Memspace_desc memspace_map[DATATYPE_COUNT][AES_MEMTYPE_COUNT];

/* Function declarations */
void flash_erase(uint32_t addr, uint32_t pages_count);
HAL_StatusTypeDef str_to_int(char* str, long int* out_val);
HAL_StatusTypeDef str_to_double(char* str, double* out_val);
int is_in_range_int(int val, int lo_bound, int up_bound);
int is_in_range_float(float val, float lo_bound, float up_bound);
int is_in_range_double(double val, double lo_bound, double up_bound);

#endif // __USER_H
