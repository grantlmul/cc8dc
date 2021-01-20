#include "opdecode.h"

#include <stdint.h>

uint16_t
decode_address(short instruction)
{
	return instruction&0x0FFF;
}

uint8_t
decode_byte(short instruction)
{
	return instruction&0x00FF;
}
uint8_t
decode_n(short instruction)
{
	return instruction&0x000F;
}
uint8_t
decode_x(short instruction)
{
	return decode_n(instruction>>8);
}
uint8_t
decode_y(short instruction)
{
	return decode_n(instruction>>4);
}
