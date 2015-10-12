#ifndef EMUL_CARTRIDGE_UXROM_H
#define EMUL_CARTRIDGE_UXROM_H

#include "emul_cartridge.h"
#include <stdio.h>

namespace MahNES
{
	class EmulatorCartridgeUxROM : public EmulatorCartridge
	{
	public:
		int8* prgROM;
		int8* chrROM;
		int8  chrRAM[0x2000];
		int   currentBank;
		int   bankNumber;

		int LoadROM(EmulatorROM* rom) override
		{
			ptr = (void*)this;

			prgROM = rom->prgROM;
			chrROM = rom->chrROM;
			currentBank = 0;
			bankNumber = rom->prg16KBBankNumber;

			cpuReadFunction = CPURead;
			cpuWriteFunction = CPUWrite;
			ppuReadFunction = PPUReadCHRRAM;
			ppuWriteFunction = PPUWrite;
			ppuCIRAMEnableFunction = PPUCIRAMEnable;

			if (rom->mirroring) ppuCIRAMMirrorFunction = PPUCIRAMMirrorHorizontal;
			else ppuCIRAMMirrorFunction = PPUCIRAMMirrorVertical;

			return 0;
		}

		static int8 CPURead(void* ptr, int16 addr)
		{
			if (addr >= 0xc000) return ((EmulatorCartridgeUxROM*)ptr)->prgROM[(((EmulatorCartridgeUxROM*)ptr)->bankNumber - 1) * 0x4000 + addr % 0x4000];
			else return ((EmulatorCartridgeUxROM*)ptr)->prgROM[((EmulatorCartridgeUxROM*)ptr)->currentBank * 0x4000 + addr % 0x4000];
		}

		static void CPUWrite(void* ptr, int16 addr, int8 value)
		{
			if (addr >= 0x8000) ((EmulatorCartridgeUxROM*)ptr)->currentBank = value & 0xf;
			if (value >= ((EmulatorCartridgeUxROM*)ptr)->bankNumber) value >>= 1;
		}

		static int8 PPUReadCHRRAM(void* ptr, int16 addr)
		{
			return ((EmulatorCartridgeUxROM*)ptr)->chrRAM[addr % 0x2000];
		}

		static void PPUWrite(void* ptr, int16 addr, int8 value)
		{
			if (addr < 0x2000)
				((EmulatorCartridgeUxROM*)ptr)->chrRAM[addr % 0x2000] = value;
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
