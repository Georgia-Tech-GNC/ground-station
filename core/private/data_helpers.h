#ifndef DATA_HELPERS_H
#define DATA_HELPERS_H

#include "stdint.h"

void quantize_float_8(uint8_t* buffer, float x, float mn, float mx);
void quantize_float_16(uint8_t* buffer, float x, float mn, float mx);
void quantize_float_32(uint8_t* buffer, double x, double mn, double mx);
void quantize_float_64(uint8_t* buffer, double x, double mn, double mx);
void unquantize_float_8(float* x, const uint8_t* const buffer, float mn, float mx);
void unquantize_float_16(float* x, const uint8_t* const buffer, float mn, float mx);
void unquantize_float_32(double* z, const uint8_t* const buffer, double mn, double mx);
void unquantize_float_64(double* z, const uint8_t* const buffer, double mn, double mx);

#endif