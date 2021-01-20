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

	


	/* #Value = `1` */
	for (int i = 0; i < 0xDFF; i++) {
		printf("working %x\n",fileData[i]);
		switch (fileData[i]&0xF000) {
		case 0x0000:
		{
			u8 y = decode_y(fileData[i]);
			u8 c = decode_char(fileData[i]);
			if (y != 0) {
				output[outpos] = "nop # would call system code but we will not\n";
				outpos+=strlen("nop # would call system code but we will not\n");
			} else if (c == 0xE0) {
				output[outpos] = "cls\n";
				outpos+=strlen("cls\n");
			} else {
				output[outpos] = "ret\n";
				outpos+=strlen("ret\n");
			}
			break;
		}
		case 0x1000:
		{
			u16 addr = decode_address(fileData[i]);
			/* pootis pencer here */
			char* poot;
			sprintf(poot, "goto func_%hu\n", addr);
			output[outpos] = *poot;
			outpos+=strlen(poot);
			break;
		}
		case 0x6000:
		{
			u8 reg = decode_x(fileData[i]);
			u8 byte = decode_char(fileData[i]);
			outpos += sprintf(&output[outpos],"ld V%hhu, %#x\n", reg, byte);
			break;
		}

		default:
		printf("unknown %x, %#x\n", fileData[i], fileData[i]&0xF000);
		puts((char*)output);
		return 5;
		break;
		}
	}
	puts((char*)output);
	
	return 0;
}
