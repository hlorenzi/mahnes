#ifndef EMUL_CARTRIDGE_H
#define EMUL_CARTRIDGE_H

#include "emul_rom.h"

namespace MahNES
{
	class EmulatorCartridge
	{
	public:
        typedef int8(*CPUReadFunction)(void*, int16);
        typedef void(*CPUWriteFunction)(void*, int16, int8);
        typedef int8(*PPUReadFunction)(void*, int16);
        typedef void(*PPUWriteFunction)(void*, int16, int8);
        typedef bool(*PPUCIRAMEnableFunction)(void*, int16);
        typedef bool(*PPUCIRAMMirrorFunction)(void*, int16);

		virtual int LoadROM(EmulatorROM* rom) = 0;

		EmulatorCPU* cpu;
		EmulatorPPU* ppu;
		void* ptr;
		CPUReadFunction cpuReadFunction;
		CPUWriteFunction cpuWriteFunction;
		PPUReadFunction ppuReadFunction;
		PPUWriteFunction ppuWriteFunction;
		PPUCIRAMEnableFunction ppuCIRAMEnableFunction;
		PPUCIRAMMirrorFunction ppuCIRAMMirrorFunction;
	};
}

#endif
