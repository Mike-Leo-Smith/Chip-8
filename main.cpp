#include <cstdio>
#include <opencv2/opencv.hpp>
#include "chip8.h"

using namespace cv;
using namespace chip8;

int main(int argc, char *argv[])
{
	if(argc<2)
		printf("COMMAND ERROR!\n");
	FILE *fp = fopen(argv[1], "rb");
	int rom_size;
	unsigned char *rom;
	
	if (fp == NULL) printf("ROM ERROR!\n");
	
	fseek(fp, 0, SEEK_END);
	
	rom_size = (int)ftell(fp);
	rom = new unsigned char[rom_size];
	
	rewind(fp);
	
	if (fread(rom, sizeof(char), rom_size, fp) != rom_size)
		printf("ROM ERROR!\n");
	
	chip myChip8;
	
	myChip8.initialize();
	myChip8.loadROM(rom, rom_size);
	
	Mat screen = Mat(256, 512, CV_8UC1, myChip8.disp);
	namedWindow("Chip-8", WINDOW_AUTOSIZE);
	
	for (;;)
	{
		if (myChip8.drawFlag == REDRAW)
		{
			imshow("Chip-8", screen);
			myChip8.drawFlag = NONE;
		}
		else if (myChip8.drawFlag == CLEAR)
		{
			memset(myChip8.disp, 0, 2048 * 64);
			imshow("Chip-8", screen);
			myChip8.drawFlag = NONE;
		}
		
		int k = cvWaitKey(5);
		
		if (k == 27) break;
		myChip8.emulateCycle(k);
	}
	
	
	return 0;
}