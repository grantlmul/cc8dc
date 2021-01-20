#pragma once

#include <stdint.h>

uint16_t decode_address(short instruction);
uint8_t decode_char(short instruction);
uint8_t decode_n(short instruction);
uint8_t decode_x(short instruction);
uint8_t decode_y(short instruction);
