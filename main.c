#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "opdecode.h"
#include "rs_primitives.h"

/* 0x000-0x080 : Characters  *
 * 0x081-0x1FF : Free memory *
 * 0x200-0xFFF : Program     */

int
main(int argc, char* argv[])
{
	/* check for file in parameters and if it exists */
	if (argc < 2)
		return 3;
	if (access(argv[1], F_OK) != 0) {
		printf("fail access %i\n", access(argv[1], F_OK));
		return 4;
	}
	/* open rom file */
	FILE* rom = fopen(argv[1], "rb");
	u16 fileData[0xDFF] = {0}; /* dff is max rom size */
	fseek(rom, 0L, SEEK_END);
	int romsize = ftell(rom)/2;
	rewind(rom);
	for (int i = 0; i < romsize; i++) {
		u8 readto[2];
		fread(readto, 1, 2, rom);
		fileData[i] = (readto[0]<<8)|readto[1];
	}
	fclose(rom);

	/* 65k should be plenty for disassembled output */
	/* character array for storing the output */
	char output[0xFFFF] = {0};
	/* current index of output, incremented when written */
	int outpos = 0;

	/* indices called, stored for giving labels */
	

	bool wasGoto = false;
	/* #Value = `1` */
	for (int i = 0; i < 0xDFF; i++) {
		//printf("working %x\n",fileData[i]);
		switch (fileData[i]&0xF000) {
		case 0x0000:
		{
			u8 y = decode_y(fileData[i]);
			u8 c = decode_byte(fileData[i]);
			if (y != 0) {
				outpos+=strlen(strcpy(output, "nop # would call system code but we will not\n"));
			} else if (c == 0xE0) {
				outpos+=strlen(strcpy(output, "cls\n"));
			} else {
				outpos+=strlen(strcpy(output, "ret\n"));
			}
			break;
		}
		case 0x1000:
		{
			u16 addr = decode_address(fileData[i]);
			outpos += sprintf(&output[outpos], "jp func_%hu\n", addr);
			wasGoto = true;
			break;
		}
		case 0x3000:
		{
			u8 x = decode_x(fileData[i]);
			u8 byte = decode_byte(fileData[i]);
			outpos += sprintf(&output[outpos],"se V%X, 0x%02X\n", x, byte);
			break;
		}
		case 0x4000:
		{
			u8 x = decode_x(fileData[i]);
			u8 byte = decode_byte(fileData[i]);
			outpos += sprintf(&output[outpos],"sne V%X, 0x%02X\n", x, byte);
			break;
		}
		case 0x5000:
		{
			u8 x = decode_x(fileData[i]);
			u8 y = decode_y(fileData[i]);
			outpos += sprintf(&output[outpos],"se V%X, V%X\n", x, y);
			break;
		}
		case 0x6000:
		{
			u8 reg = decode_x(fileData[i]);
			u8 byte = decode_byte(fileData[i]);
			outpos += sprintf(&output[outpos],"ld V%X, 0x%02X\n", reg, byte);
			break;
		}
		case 0x7000:
		{
			u8 reg = decode_x(fileData[i]);
			u8 byte = decode_byte(fileData[i]);
			outpos += sprintf(&output[outpos],"add V%X, 0x%02X\n", reg, byte);
			break;
		}
		case 0x8000:
		{
			u8 x = decode_x(fileData[i]);
			u8 y = decode_y(fileData[i]);
			switch (decode_n(fileData[i])) {
			case 1:
			{
				outpos += sprintf(&output[outpos],"or V%X, V%X\n", x, y);
				break;
			}
			case 2:
			{
				outpos += sprintf(&output[outpos],"and V%X, V%X\n", x, y);
				break;
			}
			case 3:
			{
				outpos += sprintf(&output[outpos],"xor V%X, V%X\n", x, y);
				break;
			}
			case 5:
			{
				outpos += sprintf(&output[outpos],"sub V%X, V%X\n", x, y);
				break;
			}
			case 6:
			{
				outpos += sprintf(&output[outpos],"shr V%X\n", x);
				break;
			}
			case 7:
			{
				outpos += sprintf(&output[outpos],"subn V%X, V%X\n", x, y);
				break;
			}
			case 14:
			{
				outpos += sprintf(&output[outpos],"shl V%X\n", x);
				break;
			}
			default:
			puts((char*)output);
			printf("unknown %x, %#x\n\n", fileData[i], fileData[i]&0xF00F);
			return 5;
			break;
			}
			break;
		}
		case 0xA000:
		{
			outpos += sprintf(&output[outpos],"ld I, 0x%X\n", decode_address(fileData[i]));
			break;
		}
		case 0xD000:
		{
			u8 x = decode_x(fileData[i]);
			u8 y = decode_y(fileData[i]);
			u8 n = decode_n(fileData[i]);
			outpos += sprintf(&output[outpos], "drw V%X, V%X, 0x%X\n", x,y,n);
			break;
		}
		case 0xF000:
		{
			switch (decode_byte(fileData[i])) {
			u8 x = decode_x(fileData[i]);

			case 0x1E:
			{
				outpos += sprintf(&output[outpos], "add I, V%X\n", x);
				break;
			}
			case 0x33:
			{
				outpos += sprintf(&output[outpos], "ld B, V%X\n", x);
				break;
			}
			case 0x29:
			{
				outpos += sprintf(&output[outpos], "ld F, V%X\n", x);
				break;
			}
			case 0x55:
			{
				outpos += sprintf(&output[outpos], "ld [I], V%X\n", x);
				break;
			}
			case 0x65:
			{
				outpos += sprintf(&output[outpos], "ld V%X, [I]\n", x);
				break;
			}

			default:
			puts(output);
			printf("unknown %x, %#x\n\n", fileData[i], fileData[i]&0xF0FF);
			return 5;
			break;
			}
			break;
		}

		default:
		puts(output);
		printf("unknown %x, %#x\n\n", fileData[i], fileData[i]&0xF000);
		return 5;
		break;
		}
	}
	puts(output);
	
	return 0;
}
