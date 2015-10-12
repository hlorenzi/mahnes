#ifndef EMUL_CARTRIDGE_NROM_H
#define EMUL_CARTRIDGE_NROM_H

#include "emul_cartridge.h"
#include <stdio.h>

namespace MahNES
{
	class EmulatorCartridgeNROM : public EmulatorCartridge
	{
	public:
		int8* prgROM;
		int8* chrROM;
		int8  chrRAM[0x2000];

		int LoadROM(EmulatorROM* rom) override
		{
			ptr = (void*)this;

			prgROM = rom->prgROM;
			chrROM = rom->chrROM;

			if (rom->prgSize < 32000) cpuReadFunction = CPURead16KB;
			else cpuReadFunction = CPURead32KB;

			cpuWriteFunction = CPUWrite;

			if (rom->chrSize == 0) ppuReadFunction = PPUReadCHRRAM;
			else ppuReadFunction = PPUReadCHRROM;

			ppuWriteFunction = PPUWrite;

			ppuCIRAMEnableFunction = PPUCIRAMEnable;

			if (rom->mirroring) ppuCIRAMMirrorFunction = PPUCIRAMMirrorHorizontal;
			else ppuCIRAMMirrorFunction = PPUCIRAMMirrorVertical;

			return 0;
		}

		static int8 CPURead16KB(void* ptr, int16 addr)
		{
			return ((EmulatorCartridgeNROM*)ptr)->prgROM[addr % 0x4000];
		}

		static int8 CPURead32KB(void* ptr, int16 addr)
		{
			return ((EmulatorCartridgeNROM*)ptr)->prgROM[addr % 0x8000];
		}

		static void CPUWrite(void* ptr, int16 addr, int8 value)
		{
            (void)ptr;
            (void)addr;
            (void)value;
		}

		static int8 PPUReadCHRROM(void* ptr, int16 addr)
		{
			return ((EmulatorCartridgeNROM*)ptr)->chrROM[addr % 0x2000];
		}

		static int8 PPUReadCHRRAM(void* ptr, int16 addr)
		{
			return ((EmulatorCartridgeNROM*)ptr)->chrRAM[addr % 0x2000];
		}

		static void PPUWrite(void* ptr, int16 addr, int8 value)
		{
			if (addr < 0x2000)
				((EmulatorCartridgeNROM*)ptr)->chrRAM[addr % 0x2000] = value;
		}

        static bool PPUCIRAMEnable(void* ptr, int16 addr)
        {
            (void)ptr;
            (void)addr;
			return true;
        }

        static bool PPUCIRAMMirrorVertical(void* ptr, int16 addr)
        {
            (void)ptr;
            return (addr & (0x1 << 10)) > 0 ? 1 : 0;
        }

        static bool PPUCIRAMMirrorHorizontal(void* ptr, int16 addr)
        {
            (void)ptr;
            return (addr & (0x1 << 11)) > 0 ? 1 : 0;
        }
	};
}

#endif
