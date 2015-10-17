#ifndef EMUL_CARTRIDGE_MMC1_H
#define EMUL_CARTRIDGE_MMC1_H

#include "emul_cartridge.h"
#include <stdio.h>

namespace MahNES
{
	class EmulatorCartridgeMMC1 : public EmulatorCartridge
	{
	public:
		int8* prgROM;
		int8* chrROM;
		int8  prgRAM[0x2000];
		int8  chrRAM[0x2000];

		int prgROMSize;
		int chrROMSize;

		int8  shiftReg;
		int8  controlReg;
		int8  chr0Reg, chr1Reg;
		int8  prgReg;

		int prgLowAddr, prgHighAddr;
		int chrLowAddr, chrHighAddr;

		int LoadROM(EmulatorROM* rom) override
		{
			ptr = (void*)this;

			shiftReg = 0x10;
			controlReg = 0x0c;
			chr0Reg = chr1Reg = 0;
			prgReg = 0;

			prgROM = rom->prgROM;
			chrROM = rom->chrROM;
			prgROMSize = rom->prgSize;
			chrROMSize = rom->chrSize;

			cpuReadFunction = CPURead;
			cpuWriteFunction = CPUWrite;

			if (rom->chrSize == 0) ppuReadFunction = PPUReadCHRRAM;
			else ppuReadFunction = PPUReadCHRROM;

			ppuWriteFunction = PPUWrite;
			ppuCIRAMEnableFunction = PPUCIRAMEnable;
			ppuCIRAMMirrorFunction = PPUCIRAMMirror;

            SetBankAddresses();

			return 0;
		}

		void SetBankAddresses()
		{
		    int prgMode = (controlReg >> 2) & 0x3;
		    if (prgMode == 0x2)
            {
                prgLowAddr = 0;
                prgHighAddr = (prgReg & 0xf) * 0x4000;
            }
		    else if (prgMode == 0x3)
            {
                prgLowAddr = (prgReg & 0xf) * 0x4000;
                prgHighAddr = prgROMSize - 0x4000;
            }
            else
            {
                prgLowAddr = (prgReg & 0xe) * 0x4000;
                prgHighAddr = prgLowAddr + 0x4000;
            }

            int chrMode = (controlReg >> 4) & 0x1;
            if (chrMode == 0)
            {
                chrLowAddr = (chr0Reg & 0x1e) * 0x1000;
                chrHighAddr = chrLowAddr + 0x1000;
            }
            else
            {
                chrLowAddr = (chr0Reg & 0x1f) * 0x1000;
                chrHighAddr = (chr1Reg & 0x1f) * 0x1000;
            }
		}

		static int8 CPURead(void* ptr, int16 addr)
		{
		    EmulatorCartridgeMMC1* mmc1 = (EmulatorCartridgeMMC1*)ptr;

		    if (addr < 0x8000)
                return mmc1->prgRAM[addr % 0x2000];
		    else if (addr < 0xc000)
                return mmc1->prgROM[mmc1->prgLowAddr + addr % 0x4000];
            else
                return mmc1->prgROM[mmc1->prgHighAddr + addr % 0x4000];
		}

		static void CPUWrite(void* ptr, int16 addr, int8 value)
		{
		    EmulatorCartridgeMMC1* mmc1 = (EmulatorCartridgeMMC1*)ptr;

		    if (addr >= 0x6000 && addr < 0x8000)
            {
                mmc1->prgRAM[addr % 0x2000] = value;
            }
            else if (addr >= 0x8000)
            {
                if (value & 0x80)
                {
                    mmc1->controlReg = (mmc1->controlReg | 0x0c);
                    mmc1->shiftReg = 0x10;
                    mmc1->SetBankAddresses();
                }
                else
                {
                    bool full = (mmc1->shiftReg & 0x1);

                    mmc1->shiftReg >>= 1;
                    mmc1->shiftReg |= (value & 0x1) << 4;

                    if (full)
                    {
                        switch (addr & 0x6000)
                        {
                            case 0x0000: mmc1->controlReg = mmc1->shiftReg; break;
                            case 0x2000: mmc1->chr0Reg = mmc1->shiftReg; break;
                            case 0x4000: mmc1->chr1Reg = mmc1->shiftReg; break;
                            case 0x6000: mmc1->prgReg = mmc1->shiftReg; break;
                        }

                        mmc1->shiftReg = 0x10;
                    }

                    mmc1->SetBankAddresses();
                }
            }
		}

		static int8 PPUReadCHRROM(void* ptr, int16 addr)
		{
		    EmulatorCartridgeMMC1* mmc1 = (EmulatorCartridgeMMC1*)ptr;

		    if (addr < 0x1000)
                return mmc1->chrROM[mmc1->chrLowAddr + addr % 0x1000];
            else
                return mmc1->chrROM[mmc1->chrHighAddr + addr % 0x1000];
		}

		static int8 PPUReadCHRRAM(void* ptr, int16 addr)
		{
			return ((EmulatorCartridgeMMC1*)ptr)->chrRAM[addr % 0x2000];
		}

		static void PPUWrite(void* ptr, int16 addr, int8 value)
		{
			if (addr < 0x2000)
				((EmulatorCartridgeMMC1*)ptr)->chrRAM[addr % 0x2000] = value;
		}

        static bool PPUCIRAMEnable(void* ptr, int16 addr)
        {
            (void)ptr;
            (void)addr;
			return true;
        }

        static bool PPUCIRAMMirror(void* ptr, int16 addr)
        {
		    EmulatorCartridgeMMC1* mmc1 = (EmulatorCartridgeMMC1*)ptr;

            int mirrorMode = (mmc1->controlReg & 0x3);

            if (mirrorMode == 0x0)
                return 0;
            else if (mirrorMode == 0x2)
                return (addr & (0x1 << 10)) > 0 ? 1 : 0;
            else if (mirrorMode == 0x3)
                return (addr & (0x1 << 11)) > 0 ? 1 : 0;
            else
                return 1;
        }
	};
}

#endif
