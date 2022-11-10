#include "utils.h"

uint8_t DecodeBCD(uint8_t bin) {
	return (((bin & 0xf0) >> 4) * 10) + (bin & 0x0f);
}

uint8_t EncodeBCD(uint8_t dec) {
	return (dec % 10 + ((dec / 10) << 4));
}
