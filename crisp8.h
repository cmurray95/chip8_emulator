#pragma once
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

class crisp8 {
	static const int FONTSET_LENGTH = 80;
	static const int STACK_LENGTH = 16;
	static const int REGISTER_LENGTH = 16;
	static const int KEY_LENGTH = 16;
	static const int MEMORY_LENGTH = 4096;
	static const int DISPLAY_LENGTH = 64 * 32;
	//Chip 8 fontset- each character is 4x5 pixels
	const unsigned char fontset[FONTSET_LENGTH] = {
		0xF0, 0x90, 0x90, 0x90, 0xF0,								// 0
		0x20, 0x60, 0x20, 0x20, 0x70,								// 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0,								// 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0,								// 3
		0x90, 0x90, 0xF0, 0x10, 0x10,								// 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0,								// 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0,								// 6
		0xF0, 0x10, 0x20, 0x40, 0x40,								// 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0,								// 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0,								// 9
		0xF0, 0x90, 0xF0, 0x90, 0x90,								// A
		0xE0, 0x90, 0xE0, 0x90, 0xE0,								// B
		0xF0, 0x80, 0x80, 0x80, 0xF0,								// C
		0xE0, 0x90, 0x90, 0x90, 0xE0,								// D
		0xF0, 0x80, 0xF0, 0x80, 0xF0,								// E
		0xF0, 0x80, 0xF0, 0x80, 0x80								// F
	};
		
public:
	//Input for chip 8 uses 16 button keypad
	unsigned char key[KEY_LENGTH];
	//Graphics displayed as 64x32 array of pixels
	unsigned char graphics[DISPLAY_LENGTH];
	//Sets if screen is to be redrawn
	bool drawFlag = false;

	crisp8();
	~crisp8();
	void cycle();
	void init();
	bool loadProgram(const char*);
private:
	//Chip 8 opcodes are 2 bytes long, hence unsigned short is suitable
	unsigned short opcode;
	//Chip 8 can store 4K of memory
	unsigned char mem[MEMORY_LENGTH];
	//Chip 8 contains 15 8-bit registers + carry flag labelled V0 through V£
	unsigned char V[REGISTER_LENGTH];
	//Index register and Program Counter - Value range: 0x000 - 0xFFF
	unsigned short I, pc;
	//Stack for storing instructions
	unsigned short stack[STACK_LENGTH];
	//Stack pointer
	unsigned short sp;
	//Cycle timers
	unsigned char delay_timer;
	unsigned char sound_timer;
	//Check for keypress
	bool keyPressed = false;

	void decode(unsigned short);
	void initFontset();
	void clearScreen();
};