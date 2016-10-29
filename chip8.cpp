//
// Created by mike on 16-9-29.
//

#include "stdio.h"
#include "memory.h"
#include "stdlib.h"
#include "unistd.h"
#include "opencv2/opencv.hpp"
#include "chip8.h"

namespace chip8
{
	void chip::initialize()
	{
		opcode = 0;
		I = 0;
		pc = 0x200;
		
		memset(memory, 0, 4096);
		memset(V, 0, 16);
		memset(gfx, 0, 2048);
		memset(disp, 0, 512 * 256);
		
		memset(stack, 0, 16);
		sp = 0;
		
		for (int i = 0; i < 80; i++)
		{
			memory[i] = FONTSET[i];
		}
		
		delay_timer = sound_timer = 0;
	}
	
	void chip::emulateCycle(int k)
	{
		//  handling keys
		if (k != -1)
		{
			switch (k)
			{
			case '1':
				k = 0x01;
				break;
			case '2':
				k = 0x02;
				break;
			case '3':
				k = 0x03;
				break;
			case '4':
				k = 0x0c;
				break;
			case 'q':
				k = 0x04;
				break;
			case 'w':
				k = 0x05;
				break;
			case 'e':
				k = 0x06;
				break;
			case 'r':
				k = 0x0d;
				break;
			case 'a':
				k = 0x07;
				break;
			case 's':
				k = 0x08;
				break;
			case 'd':
				k = 0x09;
				break;
			case 'f':
				k = 0x0e;
				break;
			case 'z':
				k = 0x0a;
				break;
			case 'x':
				k = 0x00;
				break;
			case 'c':
				k = 0x0b;
				break;
			case 'v':
				k = 0x0f;
				break;
			default:
				k = -1;
				break;
			}
		}
		//printf("%c",k);
		//  handing opcodes
		opcode = (memory[pc] << 8) | memory[pc + 1];    //  retrieving opcodes
		//printf("0x%04x\n", opcode);
		//  decoding opcodes
		if (opcode == 0x00e0)                   //  00E0: clear the screen
		{
			drawFlag = CLEAR;
			memset(gfx, 0, 2048);
			pc += 2;
		}
		else if (opcode == 0x00ee)              //  00EE: return from a subroutine
		{
			sp--;
			pc = stack[sp];
			pc += 2;
		}
		else if ((opcode & 0xf000) == 0x1000)   //  1NNN: jump to address NNN
		{
			pc = (unsigned short)(opcode & 0x0fff);
		}
		else if ((opcode & 0xf000) == 0x2000)   //  2NNN: call subroutine at NNN
		{
			stack[sp] = pc;
			sp++;
			pc = (unsigned short)(opcode & 0x0fff);
		}
		else if ((opcode & 0xf000) == 0x3000)   //  3XNN: skip next instruction if VX equals NN
		{
			if (V[(opcode & 0x0f00) >> 8] == (opcode & 0x00ff))
				pc += 4;
			else
				pc += 2;
		}
		else if ((opcode & 0xf000) == 0x4000)   //  4XNN: skip next instruction if VX doesn't equal NN
		{
			if (V[(opcode & 0x0f00) >> 8] != (opcode & 0x00ff))
				pc += 4;
			else
				pc += 2;
		}
		else if ((opcode & 0xf00f) == 0x5000)   //  5XY0: skip nest instruction if VX equals VY
		{
			if (V[(opcode & 0x0f00) >> 8] == V[(opcode & 0x00f0) >> 4])
				pc += 4;
			else
				pc += 2;
		}
		else if ((opcode & 0xf000) == 0x6000)   //  6XNN: set VX to NN
		{
			V[(opcode & 0x0f00) >> 8] = (unsigned char)(opcode & 0x00ff);
			pc += 2;
		}
		else if ((opcode & 0xf000) == 0x7000)   //  7XNN: add NN to VX
		{
			V[(opcode & 0x0f00) >> 8] += opcode & 0x00ff;
			pc += 2;
		}
		else if ((opcode & 0xf00f) == 0x8000)   //  8XY0: set VX to the value of VY
		{
			V[(opcode & 0x0f00) >> 8] = V[(opcode & 0x00f0) >> 4];
			pc += 2;
		}
		else if ((opcode & 0xf00f) == 0x8001)   //  8XY1: set VX to (VX | VY)
		{
			V[(opcode & 0x0f00) >> 8] |= V[(opcode & 0x00f0) >> 4];
			pc += 2;
		}
		else if ((opcode & 0xf00f) == 0x8002)   //  8XY2: set VX to (VX & VY)
		{
			V[(opcode & 0x0f00) >> 8] &= V[(opcode & 0x00f0) >> 4];
			pc += 2;
		}
		else if ((opcode & 0xf00f) == 0x8003)   //  8XY3: set VX to (VX ^ VY)
		{
			V[(opcode & 0x0f00) >> 8] ^= V[(opcode & 0x00f0) >> 4];
			pc += 2;
		}
		else if ((opcode & 0xf00f) == 0x8004)   //  8XY4: add VY to VX, VF set to 1 when carry and 0 when not
		{
			if (int(V[(opcode & 0x0f00) >> 8]) + int(V[(opcode & 0x00f0) >> 4]) > 0xff)
				V[0x0f] = 1;
			else
				V[0x0f] = 0;
			V[(opcode & 0x0f00) >> 8] += V[(opcode & 0x00f0) >> 4];
			pc += 2;
		}
		else if ((opcode & 0xf00f) == 0x8005)   //  8XY5: subtract VY from VX, VF set to 0 when borrow and 1 when not
		{
			if (V[(opcode & 0x0f00) >> 8] < V[(opcode & 0x00f0) >> 4])
				V[0x0f] = 0;
			else
				V[0x0f] = 1;
			V[(opcode & 0x0f00) >> 8] -= V[(opcode & 0x00f0) >> 4];
			pc += 2;
		}
		else if ((opcode & 0xf00f) == 0x8006)   //  8XY6: VX >>= 1, VF set to the least significant bit of VX before shift
		{
			V[0x0f] = (unsigned char)(V[(opcode & 0x0f00) >> 8] & 0x01);
			V[(opcode & 0x0f00) >> 8] >>= 1;
			pc += 2;
		}
		else if ((opcode & 0xf00f) == 0x8007)   //  8XY7: set VX to (VY - VX), VF set to 0 when borrow and 1 when not
		{
			if (V[(opcode & 0x0f00) >> 8] > V[(opcode & 0x00f0) >> 4])
				V[0x0f] = 0;
			else
				V[0x0f] = 1;
			V[(opcode & 0x0f00) >> 8] = V[(opcode & 0x00f0) >> 4] - V[(opcode & 0x0f00) >> 8];
			pc += 2;
		}
		else if ((opcode & 0xf00f) == 0x800E)   //  8XYE: VX <<= 1, VF set to the most significant bit of VX before shift
		{
			V[0x0f] = V[(opcode & 0x0f00) >> 8] >> 7;
			V[(opcode & 0x0f00) >> 8] <<= 1;
			pc += 2;
		}
		else if ((opcode & 0xf00f) == 0x9000)   //  9XY0: skip nest instruction if VX doesn't equal VY
		{
			if (V[(opcode & 0x0f00) >> 8] != V[(opcode & 0x00f0) >> 4])
				pc += 4;
			else
				pc += 2;
		}
		else if ((opcode & 0xf000) == 0xa000)   //  ANNN: set I to address NNN
		{
			I = (unsigned short)(opcode & 0x0fff);
			pc += 2;
		}
		else if ((opcode & 0xf000) == 0xb000)   //  BNNN: jump to address (NNN + V0)
		{
			pc = (unsigned short)((opcode & 0x0fff) + V[0]);
		}
		else if ((opcode & 0xf000) == 0xc000)   //  CXNN: set VX to the result of (random & NN)
		{
			V[(opcode & 0x0f00) >> 8] = (unsigned char)((opcode & 0xff) & (rand() % 0xff));
			pc += 2;
		}
		else if ((opcode & 0xf000) == 0xd000)   //  DXYN: draw a 8 * N sprite at (VX, VY), pixels read from memory[I], VF set to 1 if any pixel cleared
		{
			unsigned short x = V[(opcode & 0x0f00) >> 8];
			unsigned short y = V[(opcode & 0x00f0) >> 4];
			unsigned short height = (unsigned short)(opcode & 0x000f);
			unsigned short pixel;
			unsigned int yline, xline, pos, xpos, ypos;
			
			V[0x0f] = 0;
			for (yline = 0; yline < height; yline++)
			{
				pixel = memory[I + yline];
				for (xline = 0; xline < 8; xline++)
				{
					if ((pixel & (0x80 >> xline)) != 0)
					{
						if (gfx[(x + xline + ((y + yline) << 6))])
						{
							V[0x0f] = 1;
						}
						pos = x + xline + ((y + yline) << 6);
						gfx[pos] ^= 0xff;
						ypos = pos >> 6;            //  y position on gfx
						xpos = pos - (ypos << 6);   //  x position on gfx
						xpos <<= 3;     //  x position on disp
						ypos <<= 3;     //  y position on disp
						for (int i = 0; i < 8; i++)
							for (int j = 0; j < 8; j++)
								disp[i + ypos][j + xpos] ^= 0xff;
					}
				}
			}
			
			drawFlag = REDRAW;
			pc += 2;
		}
		else if ((opcode & 0xf0ff) == 0xe09e)   //  EX9E: skip next instruction if the key stored in VX is pressed
		{
			if (k == V[(opcode & 0x0f00) >> 8])
				pc += 4;
			else
				pc += 2;
		}
		else if ((opcode & 0xf0ff) == 0xe0a1)   //  EXA1: skip next instruction if the key stored in VX isn't pressed
		{
			if (k != V[(opcode & 0x0f00) >> 8])
				pc += 4;
			else
				pc += 2;
		}
		else if ((opcode & 0xf0ff) == 0xf007)   //  FX07: set VX to the value of delay timer
		{
			V[(opcode & 0x0f00) >> 8] = delay_timer;
			pc += 2;
		}
		else if ((opcode & 0xf0ff) == 0xf00a)   //  FX0A: set VX to the key press awaited
		{
			if (k != -1)
			{
				V[(opcode & 0x0f00) >> 8] = (unsigned char)k;
				pc += 2;
			}
		}
		else if ((opcode & 0xf0ff) == 0xf015)   //  FX15: set delay timer to VX
		{
			delay_timer = V[(opcode & 0x0f00) >> 8];
			pc += 2;
		}
		else if ((opcode & 0xf0ff) == 0xf018)   //  FX18: set the sound timer to VX
		{
			sound_timer = V[(opcode & 0x0f00) >> 8];
			pc += 2;
		}
		else if ((opcode & 0xf0ff) == 0xf01e)   //  FX1E: add VX to I
		{
			if (I + V[(opcode & 0x0f00) >> 8] > 0x0fff)
				V[0x0f] = 1;
			else
				V[0x0f] = 0;
			I += V[(opcode & 0x0f00) >> 8];
			pc += 2;
		}
		else if ((opcode & 0xf0ff) == 0xf029)   //  FX29: set I to the location of the sprite for the character in VX
		{
			I = (unsigned short)(V[(opcode & 0x0f00) >> 8] * 0x05);
			pc += 2;
		}
		else if ((opcode & 0xf0ff) == 0xf033)   //  FX33: stores the binary-coded decimal representation of VX, with the most significant of three digits at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2.
		{
			memory[I] = (unsigned char)(V[(opcode & 0x0f00) >> 8] / 100);
			memory[I + 1] = (unsigned char)(V[(opcode & 0x0f00) >> 8] / 10 % 10);
			memory[I + 2] = (unsigned char)(V[(opcode & 0x0f00) >> 8] % 10);
			pc += 2;
		}
		else if ((opcode & 0xf0ff) == 0xf055)   //  FX55: store V0 to VX in memory starting at address I
		{
			for (int i = 0; i <= ((opcode & 0x0f00) >> 8); i++)
				memory[I + i] = V[i];
			I += ((opcode & 0x0f00) >> 8) + 1;
			pc += 2;
		}
		else if ((opcode & 0xf0ff) == 0xf065)   //  FX65: fill V0 to VX with memory starting at address I
		{
			for (int i = 0; i <= ((opcode & 0x0f00) >> 8); i++)
				V[i] = memory[I + i];
			I += ((opcode & 0x0F00) >> 8) + 1;
			pc += 2;
		}
		else
		{
			printf("UNRECOGNIZED OPCODE 0x%04x\n", opcode);
		}
		
		if (delay_timer > 0)
			delay_timer--;
		
		if (sound_timer > 0)
		{
			sound_timer--;
			if (sound_timer == 0)
				printf("BEEP!\n");
		}
		
		// usleep(15000);
	}
	
	void chip::loadROM(const unsigned char *buff, int size)
	{
		for (int i = 0; i < size; i++)
			memory[i + 512] = buff[i];
	}
	
}