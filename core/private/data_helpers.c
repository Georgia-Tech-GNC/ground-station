#include "data_helpers.h"

#include <stdint.h>
#include <string.h>

void quantize_float_8(uint8_t* buffer, float x, float mn, float mx) {
    uint8_t q = (x >= mx) ? 255 : ((x <= mn) ? 0 : ((x - mn) * (255.0/(mx - mn))));
    memcpy(buffer, &q, 1);
}
void quantize_float_16(uint8_t* buffer, float x, float mn, float mx) {
    uint16_t q = (x >= mx) ? 65535 : ((x <= mn) ? 0 : ((x - mn) * (65535.0/(mx - mn))));
    memcpy(buffer, &q, 2);
}
void quantize_float_32(uint8_t* buffer, double x, double mn, double mx) {
    uint32_t q = (x >= mx) ? 4294967295 : ((x <= mn) ? 0 : ((x - mn) * (4294967295.0/(mx - mn))));
    memcpy(buffer, &q, 4);
}
void quantize_float_64(uint8_t* buffer, double x, double mn, double mx) {
    uint64_t q = (x >= mx) ? 18446744073709551615ULL : ((x <= mn) ? 0 : ((x - mn) * (18446744073709551615.0/(mx - mn))));
    memcpy(buffer, &q, 8);
}

void unquantize_float_8(float* x, const uint8_t* const buffer, float mn, float mx) {
    uint8_t q; memcpy(&q, buffer, 1);
    *x = mn + ((float)q) * ((mx - mn) / 255.0);
}
void unquantize_float_16(float* x, const uint8_t* const buffer, float mn, float mx) {
    uint16_t q; memcpy(&q, buffer, 2);
    *x = mn + ((float)q) * ((mx - mn) / 65535.0);
}
void unquantize_float_32(double* x, const uint8_t* const buffer, double mn, double mx) {
    uint32_t q; memcpy(&q, buffer, 4);
    *x = mn + ((float)q) * ((mx - mn) / 4294967295.0);
}
void unquantize_float_64(double* x, const uint8_t* const buffer, double mn, double mx) {
    uint64_t q; memcpy(&q, buffer, 8);
    *x = mn + ((float)q) * ((mx - mn) / 18446744073709551615.0);
}