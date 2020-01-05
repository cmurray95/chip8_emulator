#include "crisp8.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <chrono>
#include <thread>

using namespace std::this_thread; // Allows for sleep, used to delay timer
using namespace std::chrono_literals; //Offers time notation (ms, s, h)

crisp8::crisp8()
{
	init();
}

crisp8::~crisp8()
{
	
}

void crisp8::cycle() {
	//Fetch opcode
	//Retreives consecutive opcodes and merges for decoding/execution;
	opcode = mem[pc] << 8 | mem[pc + 1];
	decode(opcode);
}

void crisp8::clearScreen() {
	for (int i = 0; i < DISPLAY_LENGTH; ++i) {
		graphics[i] = 0x0;
	}
}

void crisp8::decode(unsigned short opcode) {
	//AND code to receive 4 bit operation  
	switch (opcode & 0xF000) {
	//Operations 0x00E0 and 0x00EE must be compared using last 4 bits
	case 0x0000:
		switch (opcode & 0x000F) {
		//0x00E0 -> Clears the screen
		case 0x0000:
			clearScreen();
			//Set drawflag to enable screen to be written to next cycle
			drawFlag = true;
			//Increment Program Counter
			pc += 2;
			printf("Opcode: 0x00E0\n");
			break;
		//0x00EE -> Returns from current subroutine
		case 0x000E:
			//Decrementing stack pointer stops overwritten data
			--sp;
			//Return
			pc = stack[sp];
			pc += 2;
			printf("Opcode: 0x00EE\n");
			break;
		default:
			printf("Unknown opcode: %i\n", opcode);
			break;
		}
		break;

	//0x1NNN -> jump to address NNN
	case 0x1000:
		pc = opcode & 0x0FFF;
		printf("Opcode: 0x1000\n");
		break;
	//0x2NNN -> Call subroutine stored at NNN
	case 0x2000:
		//Load current address into stack
		stack[sp] = pc;
		++sp;
		//Set program counter to address at NNN
		pc = opcode & 0x0FFF;
		printf("Opcode: 0x2NNN\n");
		break;
	//0x3XNN -> skip instruction if: VX == NN
	case 0x3000:
		if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
			//Skip next instruction
			pc += 4;
		}
		else {
			pc += 2;
		}
		printf("Opcode: 0x3XNN\n");
		break;
	//0x4XNN -> skip instruction if: VX != NN
	case 0x4000:
		if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
			//Skip next instruction
			pc += 4;
		}
		else {
			pc += 2;
		}
		printf("Opcode: 0x4NNN\n");
		break;
	//0x5XY0 -> skip instruction if: VX == VY
	case 0x5000:
		if (V[(opcode & 0x0F00) >> 8] == V[opcode & 0x00F0 >> 4]) {
			pc += 4;
		}
		else {
			pc += 2;
		}
		printf("Opcode: 0x5XY0\n");
		break;
	//0x6XNN -> Set VX to NN
	case 0x6000:
		V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
		pc += 2;
		printf("Opcode: 0x6XNN\n");
		break;
	//0x7XNN -> Add NN to VX
	case 0x7000:
		V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
		pc += 2;
		printf("Opcode: 0x7XNN\n");
		break;

	//0x8XXX -> Math/BitOp operations
	case 0x8000:
		switch (opcode & 0x000F) {
		//0x8XY0 -> Set VX to VY
		case 0x0000:
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
			pc += 2;	
			printf("Opcode: 0x8XY0\n");
			break;
		//0x8XY1 -> Set VX to (VX OR VY)
		case 0x0001:
			V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			printf("Opcode: 0x8XY1\n");
			break;
		//0x8XY2 -> Set VX to (VY AND VY)
		case 0x0002:
			V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			printf("Opcode: 0x8XY2\n");
			break;
		//0x8XY3 -> Set VX to (VX XOR VY)
		case 0x0003:
			V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
			printf("Opcode: 0x8XY3\n");
			pc += 2;
			break;
		//0x8XY4 -> Add VY to VX. VF = 1 if carry
		case 0x0004:
			//Test for carry
			if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8]) {
				//Set carry flag 
				V[0xF] = 1;
			}
			else {
				V[0xF] = 0;
			}
			//Add VY to VX
			V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
			pc += 2;
			printf("Opcode: 0x8XY4\n");
			break;
		//0x8XY5 -> Subtract VY from VX, VF = 0 if borrow
		case 0x0005:
			//Test for borrow
			if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8]) {
				//Set carry flag 
				V[0xF] = 0;
			}
			else {
				V[0xF] = 1;
			}
			//Add VY to VX
			V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			printf("Opcode: 0x8XY5\n");
			break;
		//0x8XY6 -> Store least significant bit of VX in VF. Shift VX >> 1
		case 0x0006:
			//Store bit
			V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
			//Shift
			V[(opcode & 0x0F00) >> 8] >>= 1;
			pc += 2;
			printf("Opcode: 0x8XY6\n");
			break;
		//0x8XY7 -> Set VX to VY - VX, VF = 0 if borrow
		case 0x0007:
			//Test for borrow
			if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4]) {
				//Set carry flag 
				V[0xF] = 0;
			}
			else {
				V[0xF] = 1;
			}
			//Set VX to VY
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
			pc += 2;
			printf("Opcode: 0x8XY7\n");
			break;
		//0x8XYE -> Store most significant bit of VX in VF. VX << 1
		case 0x000E:
			//Store bit
			V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
			//Shift
			V[(opcode & 0x0F00) >> 8] <<= 1;
			pc += 2;
			printf("Opcode: 0x8XYE\n");
			break;
		default:
			printf("Unknown opcode: %i\n", opcode);
			break;
		}
		break;

	//0x9XY0 -> Skip next instruction if VX != VY
	case 0x9000:
		if (V[(opcode & 0x0F00) >> 8] != V[opcode & 0x00F0] >> 4) {
			pc += 4;
		}
		else {
			pc += 2;
		}
		printf("Opcode: 0x9XY0\n");
		break;
	//0xANNN -> Set register I address NNN
	case 0xA000:
		I = opcode & 0x0FFF;
		pc += 2;
		printf("Opcode: 0xANNN\n");
		break;
	//0xBNNN -> Jump to address NNNN + V[0]
	case 0xB000:
		pc = (opcode & 0x0FFF) + V[0];
		printf("Opcode: 0xB000\n");
		break;
	//0xCXNN -> Set VX to result of rand(255) & NN
	case 0xC000:
		V[(opcode & 0x0F00) >> 8] = (rand() % 255) & (opcode & 0x00FF);
		pc += 2;
		printf("Opcode: 0xCXNN\n");
		break;
	//0xDXYN -> Draw 8xN sprite at (VX, VY). Each row of 8 pixels is read from memory[i] where i is unchanged
	//			VF set to 1 if any pixels are flipped when sprite is drawn
	case 0xD000:
	{
		//Retrieve X, Y coords alongside render height;
		unsigned short x = V[(opcode & 0x0F00) >> 8];
		unsigned short y = V[(opcode & 0x00F0) >> 4];
		unsigned short height = opcode & 0x000F;
		unsigned short pixel;

		//Reset register
		V[0xF] = 0;
		for (int yline = 0; yline < height; yline++)
		{
			//Retrieve pixel value from memory starting at location I
			pixel = mem[I + yline];
			for (int xline = 0; xline < 8; xline++)
			{
				//Check if pixel is to be drawn (equal to 1)
				if ((pixel & (0x80 >> xline)) != 0)
				{
					if (graphics[(x + xline + ((y + yline) * 64))] == 1)
					{
						//If pixel set to 1, add to register
						V[0xF] = 1;
					}
					//Set pixel value
					graphics[x + xline + ((y + yline) * 64)] ^= 1;
				}
			}
		}

		//Draw updated screen
		drawFlag = true;
		pc += 2;
	}
	break;

	case 0xE000:
		switch (opcode & 0x00FF) {
		//0xEX9E -> Skip next instruction if key at VX is pressed
		case 0x009E:
			if (key[V[(opcode & 0x0F00) >> 8]] != 0) {
				//Key pressed, skip
				pc += 4;
			}
			else {
				pc += 2;
			}
			printf("Opcode: 0xEX9E\n");
			break;
		//0xEXA1 -> Skip next instruction if key at VX is not pressed
		case 0x00A1:
			if (key[V[(opcode & 0x0F00) >> 8]] == 0) {
				//Key not pressed, skip
				pc += 4;
			}
			else {
				pc += 2;
			}
			printf("Opcode: 0xEXA1\n");
			break;
		default:
			printf("Unknown opcode: %i\n", opcode);
			break;
		}
		break;
	case 0xF000:
		switch (opcode & 0x00FF) {
		//0xFX07 -> set VX to value of delay timer
		case 0x0007:
			V[(opcode & 0x0F00) >> 8] = delay_timer;
			pc += 2;
			break;
		//0xFX0A -> Wait for key press then store in VX
		case 0x000A:
			//Reset keypress
			keyPressed = false;
			//Test each possible key entry for input
			for (int i = 0; i < 16; ++i) {
				if (key[i] != 0) {
					V[(opcode & 0x0F00) >> 8] = i;
					keyPressed = true;
				}
			}

			//Return and restart cycle
			if (!keyPressed) {
				return;
			}
			pc += 2;
			printf("Opcode: 0xFX0A\n");
			break;
		//0xFX15 -> Set delay timer to VX
		case 0x0015:
			delay_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			printf("Opcode: 0xFX15\n");
			printf("Delay timer!!!!!!! %x\n", delay_timer);
			break;
		//0xFX18 -> Set sound timer to VX
		case 0x0018:
			sound_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			printf("Opcode: 0xFX18\n");
			break;
		//0xFX1E -> Add VX to I
		case 0x001E:
			if (I + V[(opcode & 0xF00) >> 8] > 0xFFF) {
				V[0xF] = 1;
			}
			else {
				V[0xF] = 0;
			}
			I += V[(opcode & 0x0F00) >> 8];
			pc += 2;
			printf("Opcode: 0xFX1E\n");
			break;
		//0xFX29 -> Set I to location of sprite for character stored in VX
		case 0x0029:
			I = V[(opcode & 0x0F00) >> 8] * 0x5;
			pc += 2;
			printf("Opcode: 0xFX29\n");
			break;
		//0xFX33 -> Take decimal representation of VX, store first digit in I, second in i+1, last in i+2
		//			E.G. 142 == i = 1, [i+1] = 4, [i+2] = 2
		case 0x0033:
			//Retrieve decimal variations and set appropiately
			//Sets first digit
			mem[I] = V[(opcode & 0x0F00) >> 8] / 100;
			//Sets second digit
			mem[++I] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
			//Sets third digit
			mem[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
			pc += 2;
			break;
		//0xFX55 -> Store V0 through VX into memory starting at address I
		case 0x0055:
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i) {
				//Fill memory
				mem[i] = V[i + I];
			}
			//Shift offset
			I += ((opcode & 0x0F00) >> 8) + 1;
			pc += 2;
			printf("Opcode: 0xFX55\n");
			break;
		//0xFX65 -> Fill V0 through VX with values from memory starting at address I
		case 0x0065:
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i) {
				//Fill V[]
				V[i] = mem[I + i];
			}
			//Shift offset
			I += ((opcode & 0x0F00) >> 8) + 1;
			pc += 2;
			printf("Opcode: 0xFX65\n");
			break;
		}
	default:
		printf("Unknown opcode: 0x%x\n", opcode);
	}

	std::chrono::milliseconds timeDelay = 1ms;
	//Update timers
	if (delay_timer > 0) {
		--delay_timer;
	}

	if (sound_timer > 0)
	{
		if (sound_timer == 1)
			printf("Noise placeholder!!\n");
		--sound_timer;
	}
	std::this_thread::sleep_for(timeDelay);

}

void crisp8::init() {
	//Set program counter to 0x200 (entry point)
	pc = 0x200;
	//Reset values
	opcode = 0;
	I = 0;
	sp = 0;

	//Clear display
	clearScreen();

	//Clear Stack
	for (int i = 0; i < STACK_LENGTH; ++i) {
		stack[i] = 0;
	}
	//Clear registers
	for (int i = 0; i < REGISTER_LENGTH; ++i) {
		V[i] = 0;
	}
	//Clear memory
	for (int i = 0; i < MEMORY_LENGTH; ++i) {
		mem[i] = 0;
	}

	//Clear keys
	for (int i = 0; i < KEY_LENGTH; ++i) {
		key[i] = 0;
	}

	initFontset();

	//Reset timers
	delay_timer = 0;
	sound_timer = 0;

	//Set clearscreen flag to true to draw initial screen
	drawFlag = true;

	//Seed timer
	//srand(time(nullptr));

}

//Load fontset into memory
void crisp8::initFontset() {
	for (int i = 0; i < FONTSET_LENGTH; ++i) {
		mem[i] = fontset[i];
	}
}

bool crisp8::loadProgram(const char* file) {
	// Init and reset existing values
	init();

	//Open file
	printf("Attempting to open file %s\n", file);

	FILE* pFile = fopen(file, "rb");
	if (pFile == NULL)
	{
		fputs("File could not be loaded!", stderr);
		return false;
	}

	//Check file size
	fseek(pFile, 0, SEEK_END);
	long fSize = ftell(pFile);
	//Move back to start of stream
	rewind(pFile);
	printf("File size: %d\n", (int)fSize);

	//Allocate memory for file storage
	char* buffer = (char*)malloc(sizeof(char) * fSize);
	if (buffer == nullptr) {
		fputs("Error allocating memory!", stderr);
		return false;
	}

	//Copy file to buffer
	size_t result = fread(buffer, 1, fSize, pFile);
	//Compare result to file size for validation
	if (result != fSize) {
		fputs("Error reading file!", stderr);
		return false;
	}


	//Copy buffer to memory starting at memory address 512
	if ((MEMORY_LENGTH - 512) > fSize) {
		for (int i = 0; i < fSize; ++i) {
			mem[i + 512] = buffer[i];
		}
	}
	else {
		printf("File too big to be read!");
		return false;
	}
	//Clean up
	fclose(pFile);
	free(buffer);

	return true;
	
}

