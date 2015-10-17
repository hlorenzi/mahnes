#ifndef EMUL_CARTRIDGE_AXROM_H
#define EMUL_CARTRIDGE_AXROM_H

#include "emul_cartridge.h"
#include <stdio.h>

namespace MahNES
{
	class EmulatorCartridgeAxROM : public EmulatorCartridge
	{
	public:
		int8* prgROM;
		int8* chrROM;
		int8  chrRAM[0x2000];
		int   currentBank;
		int   bankNumber;
		bool  mirroring;

		int LoadROM(EmulatorROM* rom) override
		{
			ptr = (void*)this;

			prgROM = rom->prgROM;
			chrROM = rom->chrROM;
			currentBank = 0;
			bankNumber = rom->prg16KBBankNumber / 2;
			mirroring = 0;

			cpuReadFunction = CPURead;
			cpuWriteFunction = CPUWrite;
			ppuReadFunction = PPUReadCHRRAM;
			ppuWriteFunction = PPUWrite;
			ppuCIRAMEnableFunction = PPUCIRAMEnable;
			ppuCIRAMMirrorFunction = PPUCIRAMMirror;

			return 0;
		}

		static int8 CPURead(void* ptr, int16 addr)
		{
			return ((EmulatorCartridgeAxROM*)ptr)->prgROM[((EmulatorCartridgeAxROM*)ptr)->currentBank * 0x8000 + addr % 0x8000];
		}

		static void CPUWrite(void* ptr, int16 addr, int8 value)
		{
            EmulatorCartridgeAxROM* axrom = ((EmulatorCartridgeAxROM*)ptr);
			if (addr >= 0x8000)
            {
                axrom->mirroring = (value & 0x10) ? 1 : 0;

                axrom->currentBank = value & 0xf;
                while (axrom->currentBank >= axrom->bankNumber)
                    axrom->currentBank >>= 1;
            }
		}

		static int8 PPUReadCHRRAM(void* ptr, int16 addr)
		{
			return ((EmulatorCartridgeAxROM*)ptr)->chrRAM[addr % 0x2000];
		}

		static void PPUWrite(void* ptr, int16 addr, int8 value)
		{
			if (addr < 0x2000)
				((EmulatorCartridgeAxROM*)ptr)->chrRAM[addr % 0x2000] = value;
		}

        static bool PPUCIRAMEnable(void* ptr, int16 addr)
        {
            (void)ptr;
            (void)addr;
			return true;
        }

        static bool PPUCIRAMMirror(void* ptr, int16 addr)
        {
            EmulatorCartridgeAxROM* axrom = ((EmulatorCartridgeAxROM*)ptr);
			return axrom->mirroring;
        }
	};
}

#endif
