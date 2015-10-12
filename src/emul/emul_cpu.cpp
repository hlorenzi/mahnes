/*

	6502 CPU Emulator
	Copyright 2015 Henrique Lorenzi

*/

#include "emul_cpu.h"
#include "emul_disasm.h"
#include <stdio.h>
#include <stdlib.h>

namespace MahNES
{
	EmulatorCPU::EmulatorCPU()
	{

	}

	EmulatorCPU::~EmulatorCPU()
	{

	}

	void EmulatorCPU::SetReadFunction(void* ptr, ReadFunction f)
	{
		readFunction = f;
		readFunctionObj = ptr;
	}

	void EmulatorCPU::SetWriteFunction(void* ptr, WriteFunction f)
	{
		writeFunction = f;
		writeFunctionObj = ptr;
	}

	void EmulatorCPU::SetClockDivider(int divider)
	{
		clockDivider = divider;
	}


	void EmulatorCPU::Reset()
	{
	    registerPC = readFunction(readFunctionObj, 0xfffc) | (readFunction(readFunctionObj, 0xfffd) << 8);

		opcodeTiming = 0;
		registerA = registerX = registerY = 0;
		registerS = 0xfd;
		signalNMI = -1;

		frameCounter = 1;
		cycleCounter = 0;
		instructionCounter = 0;
		frameCycleCounter = 0;
	}

	void EmulatorCPU::NMI(int delayCycles)
	{
		signalNMI = delayCycles;
	}

	void EmulatorCPU::Execute(int cycles)
	{
		static const int IMP = 0;
		static const int IMM = 1;
		static const int ZER = 2;
		static const int ZRX = 3;
		static const int ZRY = 4;
		static const int ABS = 5;
		static const int ABX = 6;
		static const int ABY = 7;
		static const int PTX = 8;
		static const int PTY = 9;
		static const int REL = 10;
		static const int IND = 11;
		static const int STK = 12;

		static const int addressModes[256] =
		{
			//0   1   2   3    4   5   6   7    8   9   a   b    c   d   e   f
			STK,PTX,IMM,PTX, ZER,ZER,ZER,ZER, STK,IMM,IMP,IMM, ABS,ABS,ABS,ABS, // 0
			REL,PTY,IMP,PTY, ZRX,ZRX,ZRX,ZRX, IMP,ABY,IMP,ABY, ABX,ABX,ABX,ABX, // 1
			STK,PTX,IMM,PTX, ZER,ZER,ZER,ZER, STK,IMM,IMP,IMM, ABS,ABS,ABS,ABS, // 2
			REL,PTY,IMP,PTY, ZRX,ZRX,ZRX,ZRX, IMP,ABY,IMP,ABY, ABX,ABX,ABX,ABX, // 3

			STK,PTX,IMM,PTX, ZER,ZER,ZER,ZER, STK,IMM,IMP,IMM, ABS,ABS,ABS,ABS, // 4
			REL,PTY,IMP,PTY, ZRX,ZRX,ZRX,ZRX, IMP,ABY,IMP,ABY, ABX,ABX,ABX,ABX, // 5
			STK,PTX,IMM,PTX, ZER,ZER,ZER,ZER, STK,IMM,IMP,IMM, IND,ABS,ABS,ABS, // 6
			REL,PTY,IMP,PTY, ZRX,ZRX,ZRX,ZRX, IMP,ABY,IMP,ABY, ABX,ABX,ABX,ABX, // 7

			IMP,PTX,IMM,PTX, ZER,ZER,ZER,ZER, IMP,IMM,IMP,IMM, ABS,ABS,ABS,ABS, // 8
			REL,PTY,IMP,PTY, ZRX,ZRX,ZRY,ZRY, IMP,ABY,IMP,ABY, ABX,ABX,ABY,ABY, // 9
			IMM,PTX,IMM,PTX, ZER,ZER,ZER,ZER, IMP,IMM,IMP,IMM, ABS,ABS,ABS,ABS, // a
			REL,PTY,IMP,PTY, ZRX,ZRX,ZRY,ZRY, IMP,ABY,IMP,ABY, ABX,ABX,ABY,ABY, // b

			IMM,PTX,IMM,PTX, ZER,ZER,ZER,ZER, IMP,IMM,IMP,IMM, ABS,ABS,ABS,ABS, // c
			REL,PTY,IMP,PTY, ZRX,ZRX,ZRX,ZRX, IMP,ABY,IMP,ABY, ABX,ABX,ABX,ABX, // d
			IMM,PTX,IMM,PTX, ZER,ZER,ZER,ZER, IMP,IMM,IMP,IMM, ABS,ABS,ABS,ABS, // e
			REL,PTY,IMP,PTY, ZRX,ZRX,ZRX,ZRX, IMP,ABY,IMP,ABY, ABX,ABX,ABX,ABX  // f
			//0   1   2   3    4   5   6   7    8   9   a   b    c   d   e   f
		};

		static const int READ = 0;
		static const int MDFY = 1;
		static const int WRIT = 2;
		static const int NONE = 3;

		static const int functionModes[256] =
		{
			// 0    1    2    3     4    5    6    7     8    9    a    b     c    d    e    f
			NONE,READ,NONE,MDFY, READ,READ,MDFY,MDFY, NONE,READ,MDFY,NONE, READ,READ,MDFY,MDFY, // 0
			NONE,READ,NONE,MDFY, READ,READ,MDFY,MDFY, NONE,READ,NONE,MDFY, READ,READ,MDFY,MDFY, // 1
			NONE,READ,NONE,MDFY, READ,READ,MDFY,MDFY, NONE,READ,MDFY,NONE, READ,READ,MDFY,MDFY, // 2
			NONE,READ,NONE,MDFY, READ,READ,MDFY,MDFY, NONE,READ,NONE,MDFY, READ,READ,MDFY,MDFY, // 3

			NONE,READ,NONE,MDFY, READ,READ,MDFY,MDFY, NONE,READ,MDFY,NONE, NONE,READ,MDFY,MDFY, // 4
			NONE,READ,NONE,MDFY, READ,READ,MDFY,MDFY, NONE,READ,NONE,MDFY, READ,READ,MDFY,MDFY, // 5
			NONE,READ,NONE,MDFY, READ,READ,MDFY,MDFY, NONE,READ,MDFY,NONE, NONE,READ,MDFY,MDFY, // 6
			NONE,READ,NONE,MDFY, READ,READ,MDFY,MDFY, NONE,READ,NONE,MDFY, READ,READ,MDFY,MDFY, // 7

			READ,WRIT,NONE,WRIT, WRIT,WRIT,WRIT,WRIT, NONE,READ,NONE,NONE, WRIT,WRIT,WRIT,WRIT, // 8
			NONE,WRIT,NONE,NONE, WRIT,WRIT,WRIT,WRIT, NONE,WRIT,NONE,NONE, NONE,WRIT,NONE,NONE, // 9
			READ,READ,READ,NONE, READ,READ,READ,NONE, NONE,READ,NONE,NONE, READ,READ,READ,READ, // a
			NONE,READ,NONE,NONE, READ,READ,READ,NONE, NONE,READ,NONE,NONE, READ,READ,READ,NONE, // b

			READ,READ,NONE,MDFY, READ,READ,MDFY,MDFY, NONE,READ,NONE,NONE, READ,READ,MDFY,MDFY, // c
			NONE,READ,NONE,MDFY, READ,READ,MDFY,MDFY, NONE,READ,NONE,MDFY, READ,READ,MDFY,MDFY, // d
			READ,READ,NONE,MDFY, READ,READ,MDFY,MDFY, NONE,READ,NONE,READ, READ,READ,MDFY,MDFY, // e
			NONE,READ,NONE,MDFY, READ,READ,MDFY,MDFY, NONE,READ,NONE,MDFY, READ,READ,MDFY,MDFY  // f
			// 0    1    2    3     4    5    6    7     8    9    a    b     c    d    e    f
		};

		int8 localInternalData = internalData;
		int res = 0;
		int tempCarry = (registerP & 0x01) > 0 ? 1 : 0;
		int tempZero  = (registerP & 0x02) > 0 ? 1 : 0;
		int tempInter = (registerP & 0x04) > 0 ? 1 : 0;
		int tempDec   = (registerP & 0x08) > 0 ? 1 : 0;
		int tempBreak = (registerP & 0x10) > 0 ? 1 : 0;
		int tempUn    = (registerP & 0x20) > 0 ? 1 : 0;
		int tempOverf = (registerP & 0x40) > 0 ? 1 : 0;
		int tempSign  = (registerP & 0x80) > 0 ? 1 : 0;

#define UNPACK_P(x)		 \
		tempCarry = (x & 0x01) > 0 ? 1 : 0; \
		tempZero  = (x & 0x02) > 0 ? 1 : 0; \
		tempInter = (x & 0x04) > 0 ? 1 : 0; \
		tempDec   = (x & 0x08) > 0 ? 1 : 0; \
		tempBreak = (x & 0x10) > 0 ? 1 : 0; \
		tempUn    = (x & 0x20) > 0 ? 1 : 0; \
		tempOverf = (x & 0x40) > 0 ? 1 : 0; \
		tempSign  = (x & 0x80) > 0 ? 1 : 0;

#define PACK_P() \
			(tempCarry > 0 ? 0x01 : 0) | \
			(tempZero  > 0 ? 0x02 : 0) | \
			(tempInter > 0 ? 0x04 : 0) | \
			(tempDec   > 0 ? 0x08 : 0) | \
			(tempBreak > 0 ? 0x10 : 0) | \
			(tempUn    > 0 ? 0x20 : 0) | \
			(tempOverf > 0 ? 0x40 : 0) | \
			(tempSign  > 0 ? 0x80 : 0)

#define SET_CARRY(x)     (tempCarry = (x) != 0)
#define SET_ZERO(x)      (tempZero  = (x) == 0)
#define SET_INTERRUPT(x) (tempInter = (x) != 0)
#define SET_DECIMAL(x)   (tempDec   = (x) != 0)
#define SET_BREAK(x)     (tempBreak = (x) != 0)
#define SET_OVERFLOW(x)  (tempOverf = (x) != 0)
#define SET_SIGN(x)      (tempSign  = ((x) & 0x80) != 0)

#define GET_CARRY()      (tempCarry)

		clocksRemaining += cycles;

		while (clocksRemaining >= clockDivider)
		{
			clocksRemaining -= clockDivider;
			cycleCounter++;
			frameCycleCounter++;

			opcodeTiming++;
			if (signalNMI > 0) signalNMI--;

		branchPrefetch:

			switch (opcodeTiming)
			{
				case 1:
				{
					if (signalNMI == 0)
					{
						signalNMI = -1;
						writeFunction(writeFunctionObj, 0x100 + registerS, ((registerPC) >> 8) & 0xff);
						registerS--;
						writeFunction(writeFunctionObj, 0x100 + registerS, (registerPC) & 0xff);
						registerS--;
						writeFunction(writeFunctionObj, 0x100 + registerS, registerP);
						registerS--;

						frameCounter++;
						frameCycleCounter = 0;

						opcodeTiming = 0;
						registerPC = readFunction(readFunctionObj, 0xfffa) | (readFunction(readFunctionObj, 0xfffb) << 8);
					}
					else
						opcode = readFunction(readFunctionObj, registerPC++);
                    break;
				}

				case 2:
				{
					switch (addressModes[opcode])
					{
						case IMM:
							localInternalData = readFunction(readFunctionObj, registerPC++); opcodeTiming = 0; goto doOperationRead; break;
						case IMP:
							localInternalData = readFunction(readFunctionObj, registerPC); opcodeTiming = 0; goto doOperationImplied; break;
						case REL:
							localInternalData = readFunction(readFunctionObj, registerPC++); break;
						case ZER:
						case ZRX:
						case ZRY:
						case ABS:
						case ABX:
						case ABY:
						case PTX:
						case PTY:
						case IND:
							internalAddress = readFunction(readFunctionObj, registerPC++); break;
						case STK:
							localInternalData = readFunction(readFunctionObj, registerPC);
							if (opcode == 0x00 /* BRK */ || opcode == 0x20 /* JSR */) registerPC++;
							break;
						default:
							printf("\t[!] Unhandled: @ 0x%04X, t = %d, opcode = %02X, addr = %d\n", registerPC, opcodeTiming, opcode, addressModes[opcode]);
					} break;
				}

				case 3:
				{
					switch (addressModes[opcode])
					{
						case ZER:
							switch (functionModes[opcode])
							{
								case READ: localInternalData = readFunction(readFunctionObj, internalAddress); opcodeTiming = 0; goto doOperationRead; break;
								case MDFY: localInternalData = readFunction(readFunctionObj, internalAddress); break;
								case WRIT: opcodeTiming = 0; goto doOperationWrite; break;
							} break;
						case ZRX:
							readFunction(readFunctionObj, internalAddress); internalAddress = (internalAddress & 0xff00) | ((internalAddress + registerX) & 0xff); break;
						case ZRY:
							readFunction(readFunctionObj, internalAddress); internalAddress = (internalAddress & 0xff00) | ((internalAddress + registerY) & 0xff); break;
                        case ABS:
							if (opcode == 0x4c /* JMP abs */) { internalAddress |= (readFunction(readFunctionObj, registerPC) << 8); registerPC = internalAddress; opcodeTiming = 0; }
							else { internalAddress |= (readFunction(readFunctionObj, registerPC++) << 8); } break;
						case ABX:
						case ABY:
						case IND:
							internalAddress |= (readFunction(readFunctionObj, registerPC++) << 8); break;
						case PTX:
							readFunction(readFunctionObj, internalAddress);
							localInternalData = (internalAddress + registerX) & 0xff;
							break;
						case PTY:
							localInternalData = readFunction(readFunctionObj, internalAddress); break;
						case REL:
							switch (opcode)
							{
								/* BPL */ case 0x10: res = (tempSign  == 0); break;
								/* BMI */ case 0x30: res = (tempSign  != 0); break;
								/* BVC */ case 0x50: res = (tempOverf == 0); break;
								/* BVS */ case 0x70: res = (tempOverf != 0); break;
								/* BCC */ case 0x90: res = (tempCarry == 0); break;
								/* BCS */ case 0xb0: res = (tempCarry != 0); break;
								/* BNE */ case 0xd0: res = (tempZero  == 0); break;
								/* BEQ */ case 0xf0: res = (tempZero  != 0); break;
							}
							if (!res) { opcodeTiming = 1; goto branchPrefetch; }
							else readFunction(readFunctionObj, registerPC);
							break;
						case STK:
							switch (opcode)
							{
								/* BRK */ case 0x00: writeFunction(writeFunctionObj, 0x100 + registerS, (registerPC & 0xff00) >> 8); registerS--; break;
								/* RTI */ case 0x40:
								/* RTS */ case 0x60:
								/* PLA */ case 0x68:
								/* PLP */ case 0x28: registerS++; break;
								/* PHA */ case 0x48: writeFunction(writeFunctionObj, 0x100 + registerS, registerA); registerS--; opcodeTiming = 0; break;
								/* PHP */ case 0x08: writeFunction(writeFunctionObj, 0x100 + registerS, PACK_P() | 0x10 | 0x20); registerS--; opcodeTiming = 0; break;
								/* JSR */ case 0x20: break;
							} break;

						default:
							printf("\t[!] Unhandled: @ 0x%04X, t = %d, opcode = %02X, addr = %d\n", registerPC, opcodeTiming, opcode, addressModes[opcode]);
					}
					break;
				}

				case 4:
				{
					switch (addressModes[opcode])
					{
						case ZER:
							writeFunction(writeFunctionObj, internalAddress, localInternalData); goto doOperationModify; break;
                        case ZRX:
                        case ZRY:
							switch (functionModes[opcode])
							{
								case READ: localInternalData = readFunction(readFunctionObj, internalAddress); opcodeTiming = 0; goto doOperationRead; break;
								case MDFY: localInternalData = readFunction(readFunctionObj, internalAddress); break;
								case WRIT: opcodeTiming = 0; goto doOperationWrite; break;
							} break;
                        case ABS:
							switch (functionModes[opcode])
							{
								case READ: localInternalData = readFunction(readFunctionObj, internalAddress); opcodeTiming = 0; goto doOperationRead; break;
								case MDFY: localInternalData = readFunction(readFunctionObj, internalAddress); break;
								case WRIT: opcodeTiming = 0; goto doOperationWrite; break;
							} break;
						case ABX:
							res = (internalAddress & 0xff00) | ((internalAddress + registerX) & 0xff);
							localInternalData = readFunction(readFunctionObj, res);
							internalAddress += registerX;
							if (functionModes[opcode] == READ && res == internalAddress) { opcodeTiming = 0; goto doOperationRead; }
							break;
						case ABY:
							res = (internalAddress & 0xff00) | ((internalAddress + registerY) & 0xff);
							localInternalData = readFunction(readFunctionObj, res);
							internalAddress += registerY;
							if (functionModes[opcode] == READ && res == internalAddress) { opcodeTiming = 0; goto doOperationRead; }
							break;
						case PTX:
							internalAddress = readFunction(readFunctionObj, localInternalData); break;
						case PTY:
							internalAddress = (localInternalData) | (readFunction(readFunctionObj, (internalAddress + 1) & 0xff) << 8); break;
						case IND:
							localInternalData = readFunction(readFunctionObj, internalAddress); break;
						case REL:
                            res = (registerPC & 0xff00) | ((registerPC + (localInternalData & 0x80 ? -(256 - localInternalData) : localInternalData)) & 0xff);
                            if (res == ((registerPC + (localInternalData & 0x80 ? -(256 - localInternalData) : localInternalData)) & 0xffff)) { registerPC = res; opcodeTiming = 1; goto branchPrefetch; }
                            else readFunction(readFunctionObj, res);
                            break;
						case STK:
							switch (opcode)
							{
								/* BRK */ case 0x00: writeFunction(writeFunctionObj, 0x100 + registerS, registerPC & 0xff); registerS--; break;
								/* RTI */ case 0x40: res = readFunction(readFunctionObj, 0x100 + registerS); UNPACK_P(res); registerS++; break;
								/* RTS */ case 0x60: registerPC = readFunction(readFunctionObj, 0x100 + registerS); registerS++; break;
								/* PLA */ case 0x68: registerA = readFunction(readFunctionObj, 0x100 + registerS); SET_ZERO(registerA); SET_SIGN(registerA); opcodeTiming = 0; break;
								/* PLP */ case 0x28: res = readFunction(readFunctionObj, 0x100 + registerS); UNPACK_P(res); opcodeTiming = 0; break;
								/* JSR */ case 0x20: writeFunction(writeFunctionObj, 0x100 + registerS, (registerPC & 0xff00) >> 8); registerS--; break;
							} break;

						default:
							printf("\t[!] Unhandled: @ 0x%04X, t = %d, opcode = %02X, addr = %d\n", registerPC, opcodeTiming, opcode, addressModes[opcode]);
					}
					break;
				}

				case 5:
				{
					switch (addressModes[opcode])
					{
						case ZER:
							writeFunction(writeFunctionObj, internalAddress, localInternalData); opcodeTiming = 0; break;
						case ZRX:
						case ZRY:
						case ABS:
							writeFunction(writeFunctionObj, internalAddress, localInternalData); goto doOperationModify; break;
						case ABX:
						case ABY:
							switch (functionModes[opcode])
							{
								case READ: localInternalData = readFunction(readFunctionObj, internalAddress); opcodeTiming = 0; goto doOperationRead; break;
								case MDFY: localInternalData = readFunction(readFunctionObj, internalAddress); break;
								case WRIT: opcodeTiming = 0; goto doOperationWrite; break;
							} break;
						case PTX:
							internalAddress |= (readFunction(readFunctionObj, (localInternalData + 1) & 0xff)) << 8;
							break;
						case PTY:
							res = (internalAddress & 0xff00) | ((internalAddress + registerY) & 0xff);
							localInternalData = readFunction(readFunctionObj, res);
							internalAddress += registerY;
							if (functionModes[opcode] == READ && res == internalAddress) { opcodeTiming = 0; goto doOperationRead; }
							break;
						case REL:
                            registerPC += (localInternalData & 0x80 ? -(256 - localInternalData) : localInternalData);
                            opcodeTiming = 1;
                            goto branchPrefetch;
                            break;
						case IND:
							registerPC = (localInternalData) | (readFunction(readFunctionObj, (internalAddress & 0xff00) | ((internalAddress + 1) & 0xff)) << 8); opcodeTiming = 0; break;
						case STK:
							switch (opcode)
							{
								/* BRK */ case 0x00: writeFunction(writeFunctionObj, 0x100 + registerS, PACK_P()); registerS--; break;
								/* RTI */ case 0x40: registerPC = readFunction(readFunctionObj, 0x100 + registerS); registerS++; break;
								/* RTS */ case 0x60: registerPC |= readFunction(readFunctionObj, 0x100 + registerS) << 8; break;
								/* JSR */ case 0x20: writeFunction(writeFunctionObj, 0x100 + registerS, registerPC & 0xff); registerS--; break;
							} break;

						default:
							printf("\t[!] Unhandled: @ 0x%04X, t = %d, opcode = %02X, addr = %d\n", registerPC, opcodeTiming, opcode, addressModes[opcode]);
					}
					break;
				}

				case 6:
				{
					switch (addressModes[opcode])
					{
						case ZRX:
						case ZRY:
						case ABS:
							writeFunction(writeFunctionObj, internalAddress, localInternalData); opcodeTiming = 0; break;
						case ABX:
						case ABY:
							writeFunction(writeFunctionObj, internalAddress, localInternalData); goto doOperationModify; break;
						case PTX:
							switch (functionModes[opcode])
							{
								case READ: localInternalData = readFunction(readFunctionObj, internalAddress); opcodeTiming = 0; goto doOperationRead; break;
								case MDFY: localInternalData = readFunction(readFunctionObj, internalAddress); break;
								case WRIT: opcodeTiming = 0; goto doOperationWrite; break;
							} break;
						case PTY:
							switch (functionModes[opcode])
							{
								case READ: localInternalData = readFunction(readFunctionObj, internalAddress); opcodeTiming = 0; goto doOperationRead; break;
								case MDFY: localInternalData = readFunction(readFunctionObj, internalAddress); break;
								case WRIT: opcodeTiming = 0; goto doOperationWrite; break;
							} break;
						case STK:
							switch (opcode)
							{
								/* BRK */ case 0x00: registerPC = readFunction(readFunctionObj, 0xfffe); break;
								/* RTI */ case 0x40: registerPC |= readFunction(readFunctionObj, 0x100 + registerS) << 8; opcodeTiming = 0; break;
								/* RTS */ case 0x60: registerPC++; /*fprintf(logfile, " (back to $%04X) -------------", registerPC);*/ opcodeTiming = 0; break;
								/* JSR */ case 0x20: registerPC = (localInternalData) | (readFunction(readFunctionObj, registerPC) << 8); opcodeTiming = 0; break;
							} break;


						default:
							printf("\t[!] Unhandled: @ 0x%04X, t = %d, opcode = %02X, addr = %d\n", registerPC, opcodeTiming, opcode, addressModes[opcode]);
					}
					break;
				}

				case 7:
				{
					switch (addressModes[opcode])
					{
						case ABX:
						case ABY:
							writeFunction(writeFunctionObj, internalAddress, localInternalData); opcodeTiming = 0; break;
						case PTX:
						case PTY:
							writeFunction(writeFunctionObj, internalAddress, localInternalData); goto doOperationModify; break;
						case STK:
							registerPC |= readFunction(readFunctionObj, 0xffff) << 8; /*fprintf(logfile, " (back to $%04X) -------------", registerPC);*/ opcodeTiming = 0; break;

						default:
							printf("\t[!] Unhandled: @ 0x%04X, t = %d, opcode = %02X, addr = %d\n", registerPC, opcodeTiming, opcode, addressModes[opcode]);
					}
					break;
				}

				case 8:
				{
					switch (addressModes[opcode])
					{
						case PTX:
						case PTY:
							writeFunction(writeFunctionObj, internalAddress, localInternalData); opcodeTiming = 0; break;

						default:
							printf("\t[!] Unhandled: @ 0x%04X, t = %d, opcode = %02X, addr = %d\n", registerPC, opcodeTiming, opcode, addressModes[opcode]);
					}
					break;
				}
				default:
					printf("\t[!] Unhandled timing: @ 0x%04X, t = %d, opcode = %02X, addr = %d\n", registerPC, opcodeTiming, opcode, addressModes[opcode]);
			}

			continue;
		doOperationImplied:
			switch (opcode)
			{
				/* TXA */ case 0x8a: registerA = registerX; SET_SIGN(registerA); SET_ZERO(registerA); break;
				/* TAX */ case 0xaa: registerX = registerA; SET_SIGN(registerX); SET_ZERO(registerX); break;
				/* DEX */ case 0xca: registerX--; SET_SIGN(registerX); SET_ZERO(registerX); break;
				/* NOP */ case 0xea: case 0x1a: case 0x3a: case 0x5a: case 0x7a: case 0xda: case 0xfa: case 0x80: break;
				/* CLC */ case 0x18: SET_CARRY(0); break;
				/* SEC */ case 0x38: SET_CARRY(1); break;
				/* CLI */ case 0x58: SET_INTERRUPT(0); break;
				/* SEI */ case 0x78: SET_INTERRUPT(1); break;
				/* TYA */ case 0x98: registerA = registerY; SET_SIGN(registerA); SET_ZERO(registerA); break;
				/* CLV */ case 0xb8: SET_OVERFLOW(0); break;
				/* CLD */ case 0xd8: SET_DECIMAL(0); break;
				/* SED */ case 0xf8: SET_DECIMAL(1); break;
				/* TXS */ case 0x9a: registerS = registerX; break;
				/* TSX */ case 0xba: registerX = registerS; SET_SIGN(registerX); SET_ZERO(registerX); break;

				/* PHP */ case 0x08: break;
				/* PLP */ case 0x28: break;
				/* PHA */ case 0x48: break;
				/* PLA */ case 0x68: break;
				/* DEY */ case 0x88: registerY--; SET_SIGN(registerY); SET_ZERO(registerY); break;
				/* TAY */ case 0xa8: registerY = registerA; SET_SIGN(registerY); SET_ZERO(registerY); break;
				/* INY */ case 0xc8: registerY++; SET_SIGN(registerY); SET_ZERO(registerY); break;
				/* INX */ case 0xe8: registerX++; SET_SIGN(registerX); SET_ZERO(registerX); break;

				/* ASL */ case 0x0a: SET_CARRY(registerA & 0x80); registerA <<= 1; SET_SIGN(registerA); SET_ZERO(registerA); break;
				/* LSR */ case 0x4a: SET_CARRY(registerA & 0x01); registerA >>= 1; SET_SIGN(registerA); SET_ZERO(registerA); break;
				/* ROL */ case 0x2a: { res = (registerA << 1); res |= GET_CARRY(); SET_CARRY(res > 0xff); registerA = res; SET_SIGN(registerA); SET_ZERO(registerA); } break;
				/* ROR */ case 0x6a: { res = registerA; res |= (GET_CARRY() << 8); SET_CARRY(res & 0x01); res >>= 1; registerA = res; SET_SIGN(registerA); SET_ZERO(registerA); } break;

				default:
					printf("\t[!] Unhandled opImplied: @ 0x%04X, t = %d, opcode = %02X, addr = %d\n", registerPC, opcodeTiming, opcode, addressModes[opcode]);
			}
			continue;

		doOperationRead:
			switch (opcode)
			{
				/* LDA */ case 0xb1: case 0xa1: case 0xbd: case 0xb9: case 0xad: case 0xa9: case 0xa5: case 0xb5: registerA = localInternalData; SET_SIGN(localInternalData); SET_ZERO(localInternalData); break;
				/* LDX */ case 0xbe: case 0xae: case 0xa2: case 0xa6: case 0xb6: registerX = localInternalData; SET_SIGN(localInternalData); SET_ZERO(localInternalData); break;
				/* LDY */ case 0xbc: case 0xac: case 0xa0: case 0xa4: case 0xb4: registerY = localInternalData; SET_SIGN(localInternalData); SET_ZERO(localInternalData); break;
				/* EOR */ case 0x51: case 0x41: case 0x5d: case 0x59: case 0x4d: case 0x49: case 0x45: case 0x55: registerA = registerA ^ localInternalData; SET_SIGN(registerA); SET_ZERO(registerA); break;
				/* AND */ case 0x31: case 0x21: case 0x3d: case 0x39: case 0x2d: case 0x29: case 0x25: case 0x35: registerA = registerA & localInternalData; SET_SIGN(registerA); SET_ZERO(registerA);  break;
				/* ORA */ case 0x11: case 0x01: case 0x1d: case 0x19: case 0x0d: case 0x09: case 0x05: case 0x15: registerA = registerA | localInternalData; SET_SIGN(registerA); SET_ZERO(registerA);  break;
				/* ADC */ case 0x71: case 0x61: case 0x7d: case 0x79: case 0x6d: case 0x69: case 0x65: case 0x75: { res = registerA + localInternalData + GET_CARRY(); SET_ZERO(res & 0xff); SET_SIGN(res & 0xff);
					SET_OVERFLOW(!((registerA ^ localInternalData) & 0x80) && ((registerA ^ res) & 0x80)); SET_CARRY(res > 0xff); registerA = res; } break;
				/* SBC */ case 0xf1: case 0xe1: case 0xfd: case 0xf9: case 0xed: case 0xe9: case 0xeb: case 0xe5: case 0xf5: {

					res = (int)registerA - (int)localInternalData - (int)(GET_CARRY() ? 0 : 1);
					SET_SIGN(res);
					SET_ZERO(res & 0xff);	/* Sign and Zero are invalid in decimal mode */
					SET_OVERFLOW(((registerA ^ res) & 0x80) && ((registerA ^ localInternalData) & 0x80));
					SET_CARRY((res & 0xff00) == 0);
					registerA = (res & 0xff);

					 } break;
				/* CMP */ case 0xd1: case 0xc1: case 0xdd: case 0xd9: case 0xcd: case 0xc9: case 0xc5: case 0xd5: { res = registerA - (int)localInternalData; SET_CARRY(res >= 0); SET_SIGN(res); SET_ZERO(res & 0xff); } break;
				/* CPX */ case 0xe0: case 0xe4: case 0xec: { res = registerX - (int)localInternalData; SET_CARRY(res >= 0); SET_SIGN(res); SET_ZERO(res & 0xff); } break;
				/* CPY */ case 0xc0: case 0xc4: case 0xcc: { res = registerY - (int)localInternalData; SET_CARRY(res >= 0); SET_SIGN(res); SET_ZERO(res & 0xff); } break;
				/* BIT */ case 0x2c: case 0x24: SET_SIGN(localInternalData); SET_OVERFLOW(0x40 & localInternalData); SET_ZERO(localInternalData & registerA); break;
				/* NOP */ case 0x1a: case 0x3a: case 0x5a: case 0x7a: case 0xda: case 0xfa: case 0x80: case 0x1c: case 0x3c: case 0x5c: case 0x7c: case 0xdc:
					case 0xfc: case 0x0c: case 0x89: case 0x04: case 0x44: case 0x64: case 0x14: case 0x34: case 0x54: case 0x74: case 0xd4: case 0xf4: break;

				/* LAX */ case 0xb3: case 0xa3: case 0xbf: case 0xaf: case 0xa7: case 0xb7: break;
				/* ANC */ case 0x0b: case 0x2b: break;
				/* ASR */ case 0x4b: break;
				/* ARR */ case 0x6b: break;
				/* ANE */ case 0x8b: break;
				/* LXA */ case 0xab: break;
				/* SBX */ case 0xcb: break;
				/* SHX */ case 0x9e: break;
				/* SHY */ case 0x9c: break;
				/* SHA */ case 0x93: case 0x9f: break;
				default:
					printf("\t[!] Unhandled opRead: @ 0x%04X, t = %d, opcode = %02X, addr = %d\n", registerPC, opcodeTiming, opcode, addressModes[opcode]);
			}
			continue;

		doOperationWrite:
			switch (opcode)
			{
				/* STA */ case 0x91: case 0x81: case 0x9d: case 0x99: case 0x8d: case 0x85: case 0x95: writeFunction(writeFunctionObj, internalAddress, registerA); break;
				/* STX */ case 0x8e: case 0x86: case 0x96: writeFunction(writeFunctionObj, internalAddress, registerX); break;
				/* STY */ case 0x8c: case 0x84: case 0x94: writeFunction(writeFunctionObj, internalAddress, registerY); break;
				/* SAX */ case 0x83: case 0x8f: case 0x87: case 0x97: break;
				default:
					printf("\t[!] Unhandled opWrite: @ 0x%04X, t = %d, opcode = %02X, addr = %d\n", registerPC, opcodeTiming, opcode, addressModes[opcode]);
			}
			continue;

		doOperationModify:
			switch (opcode)
			{
				/* ASL */ case 0x1e: case 0x0e: case 0x06: case 0x16: SET_CARRY(localInternalData & 0x80); localInternalData <<= 1; SET_SIGN(localInternalData); SET_ZERO(localInternalData); break;
				/* LSR */ case 0x5e: case 0x4e: case 0x46: case 0x56: SET_CARRY(localInternalData & 0x01); localInternalData >>= 1; SET_SIGN(localInternalData); SET_ZERO(localInternalData); break;
				/* ROL */ case 0x3e: case 0x2e: case 0x26: case 0x36: { res = (localInternalData << 1); res |= GET_CARRY(); SET_CARRY(res > 0xff); localInternalData = res; SET_SIGN(localInternalData); SET_ZERO(localInternalData); } break;
				/* ROR */ case 0x7e: case 0x6e: case 0x66: case 0x76: { res = localInternalData; res |= (GET_CARRY() << 8); SET_CARRY(res & 0x01); res >>= 1; localInternalData = res; SET_SIGN(localInternalData); SET_ZERO(localInternalData); } break;
				/* INC */ case 0xfe: case 0xee: case 0xe6: case 0xf6: localInternalData++; SET_SIGN(localInternalData); SET_ZERO(localInternalData); break;
				/* DEC */ case 0xde: case 0xce: case 0xc6: case 0xd6: localInternalData--; SET_SIGN(localInternalData); SET_ZERO(localInternalData); break;
				/* SLO */ case 0x13: case 0x03: case 0x1f: case 0x1b: case 0x0f: case 0x07: case 0x17: break;
				/* SRE */ case 0x53: case 0x43: case 0x5f: case 0x5b: case 0x4f: case 0x47: case 0x57: break;
				/* RLA */ case 0x33: case 0x23: case 0x3f: case 0x3b: case 0x2f: case 0x27: case 0x37: break;
				/* RRA */ case 0x73: case 0x63: case 0x7f: case 0x7b: case 0x6f: case 0x67: case 0x77: break;
				/* ISB */ case 0xf3: case 0xe3: case 0xff: case 0xfb: case 0xef: case 0xe7: case 0xf7: break;
				/* DCP */ case 0xd3: case 0xc3: case 0xdf: case 0xdb: case 0xcf: case 0xc7: case 0xd7: break;
				/* SHS */ case 0x9b: break;
				/* LAS */ case 0xbb: break;
				default:
					printf("\t[!] Unhandled opModify: @ 0x%04X, t = %d, opcode = %02X, addr = %d\n", registerPC, opcodeTiming, opcode, addressModes[opcode]);
			}
			continue;
		}

		internalData = localInternalData;
		registerP = PACK_P();
	}
}
