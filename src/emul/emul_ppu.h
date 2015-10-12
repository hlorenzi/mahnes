#ifndef EMUL_PPU_H
#define EMUL_PPU_H

#include "emul_types.h"

namespace MahNES
{
	class EmulatorPPU
	{
	public:
		typedef int8(*ReadFunction)(void*, int16);
		typedef void(*WriteFunction)(void*, int16, int8);
		typedef void(*ProduceCPUCyclesFunction)(void*, int);
		typedef void(*OutputFunction)(void*, int, int, int8, int8);
		typedef void(*RiseNMIFunction)(void*, int);

		EmulatorPPU();
		~EmulatorPPU();

		void SetReadFunction(void* ptr, ReadFunction f);
		void SetWriteFunction(void* ptr, WriteFunction f);
		void SetProduceCPUCyclesFunction(void* ptr, ProduceCPUCyclesFunction f);
		void SetOutputFunction(void* ptr, OutputFunction f);
		void SetRiseNMIFunction(void* ptr, RiseNMIFunction f);

		void Reset();
		void ExecuteFrame();
		void ExecuteVisibleScanline();
		void ExecuteSprites(int8 bkg2bit, int8 bkgpalette);
		void ExecuteSpriteFetch();

		void WriteRegisterCTRL(int8 value);
		void WriteRegisterMASK(int8 value);
		int8 ReadRegisterSTATUS();
		void WriteRegisterOAMADDR(int8 value);
		void WriteRegisterOAMDATA(int8 value);
		int8 ReadRegisterOAMDATA();
		void WriteRegisterSCROLL(int8 value);
		void WriteRegisterADDR(int8 value);
		void WriteRegisterDATA(int8 value);
		int8 ReadRegisterDATA();

	public:
		int16 scrollV, scrollT;
		int8 scrollX;
		int8 oamAddress;

		int8 registerCTRL, registerMASK, registerSTATUS;
		int addressNibble;
		int8 internalLatch;

		bool statusOneCycleNMIMiss;
		bool generatingNMI;

		int scanline, dot;
		int frame;
		int cycles;

		static const int STATUS_VBLANK_FLAG = 0x80;

		static const int IGNORE_TIMER = 0;//29658 * 3;

		struct
		{
			int8 id;
			int16 tilen;
			int8 pattern0, pattern1;
			int8 x;
			int8 y;
			int8 palette;
			int8 priority;
		} spriteTable[8];

		int8 oam[256];

		ReadFunction readFunction;
		void* readFunctionObj;
		WriteFunction writeFunction;
		void* writeFunctionObj;
		ProduceCPUCyclesFunction produceCPUCyclesFunction;
		void* produceCPUCyclesFunctionObj;
		OutputFunction outputFunction;
		void* outputFunctionObj;
		RiseNMIFunction riseNMIFunction;
		void* riseNMIFunctionObj;
	};
}

#endif

