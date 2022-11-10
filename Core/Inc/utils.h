#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#define _D DecodeBCD
#define _E EncodeBCD

uint8_t DecodeBCD(uint8_t bin);
uint8_t EncodeBCD(uint8_t dec);


#endif	//UTILS_H