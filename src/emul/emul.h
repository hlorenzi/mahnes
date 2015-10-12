#ifndef EMUL_H
#define EMUL_H

#include "emul_cpu.h"
#include "emul_ppu.h"
#include "emul_apu.h"

namespace MahNES
{
	class EmulatorCartridge;

	class Emulator
	{
	public:
		Emulator();
		~Emulator();

		int LoadROM(const char* filename);
		void Reset();
		void ExecuteFrame();

		void SetPPUOutputFunction(void* ptr, EmulatorPPU::OutputFunction f);
		void SetAPUOutputFunction(void* ptr, EmulatorAPU::OutputFunction f);

	public:
		EmulatorCartridge* cartridge;
		EmulatorCPU cpu;
		EmulatorPPU ppu;
		EmulatorAPU apu;

		int prgSize, chrSize;

		int apuCycles;

		int8 ram[0x800];
		int8 vram[0x800];
		int8 paletteram[0x20];

		int controller[2];
		int controllerStrobe;

		static int8 CPURead(void* ptr, int16 addr);
		static void CPUWrite(void* ptr, int16 addr, int8 value);

		static int8 PPURead(void* ptr, int16 addr);
		static void PPUWrite(void* ptr, int16 addr, int8 value);
		static void PPUProduceCPUCycles(void* ptr, int cycles);
		static void PPUOutput(void* ptr, int scanline, int pixel, int8 palette, int8 mask);
		static void PPURiseNMI(void* ptr, int delayCycles);
	};
}

#endif
