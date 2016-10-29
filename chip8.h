//
// Created by mike on 16-9-29.
//

#ifndef CHIP_8_CHIP8_H
#define CHIP_8_CHIP8_H

namespace chip8
{
	enum draw_flags
	{
		REDRAW,
		CLEAR,
		NONE
	};
	
	class chip
	{
	private:
		unsigned short opcode;          //  cpu opcodes
		unsigned char memory[4096];     //  memory
		unsigned char V[16];            //  registers, V[0x0f] is carry flag
		unsigned short I;               //  index register
		unsigned short pc;              //  program counter
		unsigned char gfx[64 * 32];     //  VRAM
		unsigned char delay_timer;      //  delay timer, counting down in cpu ops
		unsigned char sound_timer;      //  sound_timer, beep when counts down to 0
		unsigned short stack[16];       //  stack, restoring pc when goto/jump/a func is called
		unsigned short sp;              //  pointer to the top of the stack
	
	public:
		void initialize(void);
		void emulateCycle(int k);
		void loadROM(const unsigned char *buff, int size);
		
		unsigned char disp[256][512];   //  display
		draw_flags drawFlag = REDRAW;   //  update display
		
		unsigned char *getGFX(void)
		{
			return gfx;
		}
	};
	
	const unsigned char FONTSET[] = {
			0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
			0x20, 0x60, 0x20, 0x20, 0x70, // 1
			0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
			0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
			0x90, 0x90, 0xF0, 0x10, 0x10, // 4
			0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
			0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
			0xF0, 0x10, 0x20, 0x40, 0x40, // 7
			0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
			0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
			0xF0, 0x90, 0xF0, 0x90, 0x90, // A
			0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
			0xF0, 0x80, 0x80, 0x80, 0xF0, // C
			0xE0, 0x90, 0x90, 0x90, 0xE0, // D
			0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
			0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};
}

#endif //CHIP_8_CHIP8_H
