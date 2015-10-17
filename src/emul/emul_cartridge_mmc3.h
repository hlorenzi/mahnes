#ifndef EMUL_CARTRIDGE_MMC3_H
#define EMUL_CARTRIDGE_MMC3_H

#include "emul_cartridge.h"
#include <stdio.h>

namespace MahNES
{
	class EmulatorCartridgeMMC3 : public EmulatorCartridge
	{
	public:
		int8* prgROM;
		int8* chrROM;
		int8  prgRAM[0x2000];

		int prgROMSize;
		int chrROMSize;
		int maxPRGBank;
		int maxCHRBank;

		int8 bankSelectReg;
		int8 bankDataReg[8];
		bool mirroringReg;

		int chrAddr[8];
		int prgAddr[4];

		int LoadROM(EmulatorROM* rom) override
		{
			ptr = (void*)this;

			bankSelectReg = 0;
			for (int i = 0; i < 8; i++) bankDataReg[i] = 0;
			mirroringReg = false;

			prgROM = rom->prgROM;
			chrROM = rom->chrROM;
			prgROMSize = rom->prgSize;
			chrROMSize = rom->chrSize;
			maxPRGBank = prgROMSize / 0x2000;
			maxCHRBank = chrROMSize / 0x0400;

			cpuReadFunction = CPURead;
			cpuWriteFunction = CPUWrite;

			ppuReadFunction = PPURead;
			ppuWriteFunction = PPUWrite;
			ppuCIRAMEnableFunction = PPUCIRAMEnable;
			ppuCIRAMMirrorFunction = PPUCIRAMMirror;

            SetBankAddresses();

			return 0;
		}

		void SetBankAddresses()
		{
            if (bankSelectReg & 0x40)
            {
                prgAddr[0] = prgROMSize - 0x2000 - 0x2000;
                prgAddr[1] = (bankDataReg[7] % maxPRGBank) * 0x2000;
                prgAddr[2] = (bankDataReg[6] % maxPRGBank) * 0x2000;
                prgAddr[3] = prgROMSize - 0x2000;
            }
            else
            {
                prgAddr[0] = (bankDataReg[6] % maxPRGBank) * 0x2000;
                prgAddr[1] = (bankDataReg[7] % maxPRGBank) * 0x2000;
                prgAddr[2] = prgROMSize - 0x2000 - 0x2000;
                prgAddr[3] = prgROMSize - 0x2000;
            }

            if (bankSelectReg & 0x80)
            {
                chrAddr[0] = (bankDataReg[2] % maxCHRBank) * 0x400;
                chrAddr[1] = (bankDataReg[3] % maxCHRBank) * 0x400;
                chrAddr[2] = (bankDataReg[4] % maxCHRBank) * 0x400;
                chrAddr[3] = (bankDataReg[5] % maxCHRBank) * 0x400;
                chrAddr[4] = ((bankDataReg[0] & 0xfe) % maxCHRBank) * 0x400;
                chrAddr[5] = ((bankDataReg[0] | 0x01) % maxCHRBank) * 0x400;
                chrAddr[6] = ((bankDataReg[1] & 0xfe) % maxCHRBank) * 0x400;
                chrAddr[7] = ((bankDataReg[1] | 0x01) % maxCHRBank) * 0x400;
            }
            else
            {
                chrAddr[0] = ((bankDataReg[0] & 0xfe) % maxCHRBank) * 0x400;
                chrAddr[1] = ((bankDataReg[0] | 0x01) % maxCHRBank) * 0x400;
                chrAddr[2] = ((bankDataReg[1] & 0xfe) % maxCHRBank) * 0x400;
                chrAddr[3] = ((bankDataReg[1] | 0x01) % maxCHRBank) * 0x400;
                chrAddr[4] = (bankDataReg[2] % maxCHRBank) * 0x400;
                chrAddr[5] = (bankDataReg[3] % maxCHRBank) * 0x400;
                chrAddr[6] = (bankDataReg[4] % maxCHRBank) * 0x400;
                chrAddr[7] = (bankDataReg[5] % maxCHRBank) * 0x400;
            }
		}

		static int8 CPURead(void* ptr, int16 addr)
		{
		    EmulatorCartridgeMMC3* mmc3 = (EmulatorCartridgeMMC3*)ptr;

		    if (addr < 0x8000)
                return mmc3->prgRAM[addr % 0x2000];
		    else
                return mmc3->prgROM[mmc3->prgAddr[(addr >> 13) & 0x3] + addr % 0x2000];
		}

		static void CPUWrite(void* ptr, int16 addr, int8 value)
		{
		    EmulatorCartridgeMMC3* mmc3 = (EmulatorCartridgeMMC3*)ptr;

		    if (addr >= 0x6000 && addr < 0x8000)
            {
                mmc3->prgRAM[addr % 0x2000] = value;
            }
            else if (addr >= 0x8000)
            {
                if ((addr & 0xe000) == 0x8000)
                {
                    if ((addr & 0x1) == 0x0)
                        mmc3->bankSelectReg = value;
                    else
                    {
                        mmc3->bankDataReg[mmc3->bankSelectReg & 0x7] = value;
                        mmc3->SetBankAddresses();
                    }
                }
                else if ((addr & 0xe000) == 0xa000)
                {
                    if ((addr & 0x1) == 0x0)
                        mmc3->mirroringReg = (value & 0x1);
                }
            }
		}

		static int8 PPURead(void* ptr, int16 addr)
		{
		    EmulatorCartridgeMMC3* mmc3 = (EmulatorCartridgeMMC3*)ptr;

		    return mmc3->chrROM[mmc3->chrAddr[(addr >> 10) & 0x7] + addr % 0x400];
		}

		static void PPUWrite(void* ptr, int16 addr, int8 value)
		{
		    (void)ptr;
		    (void)addr;
		    (void)value;
		}

        static bool PPUCIRAMEnable(void* ptr, int16 addr)
        {
            (void)ptr;
            (void)addr;
			return true;
        }

        static bool PPUCIRAMMirror(void* ptr, int16 addr)
        {
            if (((EmulatorCartridgeMMC3*)ptr)->mirroringReg)
                return (addr & (0x1 << 11)) > 0 ? 1 : 0;
            else
                return (addr & (0x1 << 10)) > 0 ? 1 : 0;
        }
	};
}

#endif
