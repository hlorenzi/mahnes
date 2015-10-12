#include "emul_ppu.h"
#include <stdio.h>

namespace MahNES
{
	EmulatorPPU::EmulatorPPU()
	{

	}

	EmulatorPPU::~EmulatorPPU()
	{

	}

	void EmulatorPPU::SetReadFunction(void* ptr, ReadFunction f)
	{
		readFunction = f;
		readFunctionObj = ptr;
	}

	void EmulatorPPU::SetWriteFunction(void* ptr, WriteFunction f)
	{
		writeFunction = f;
		writeFunctionObj = ptr;
	}

	void EmulatorPPU::SetProduceCPUCyclesFunction(void* ptr, ProduceCPUCyclesFunction f)
	{
		produceCPUCyclesFunction = f;
		produceCPUCyclesFunctionObj = ptr;
	}

	void EmulatorPPU::SetOutputFunction(void* ptr, OutputFunction f)
	{
		outputFunction = f;
		outputFunctionObj = ptr;
	}

	void EmulatorPPU::SetRiseNMIFunction(void* ptr, RiseNMIFunction f)
	{
		riseNMIFunction = f;
		riseNMIFunctionObj = ptr;
	}

	void EmulatorPPU::Reset()
	{
		scrollV = scrollT = 0;
		scrollX = 0;
		registerCTRL = 0;
		registerMASK = 0;
		registerSTATUS = 0;
		addressNibble = 0;
		statusOneCycleNMIMiss = false;
		generatingNMI = false;
		oamAddress = 0;
		frame = 0;
		scanline = 0;
		dot = 0;
		cycles = 0;
		for (int i = 0; i < 256; i++) oam[i] = 0;
	}

	void EmulatorPPU::WriteRegisterCTRL(int8 value)
	{
		if (cycles < IGNORE_TIMER)
			return;

		//int8 old     = registerCTRL;
		registerCTRL = value;
		scrollT &= ~(0x3 << 10);
		scrollT |= (value & 0x3) << 10;

		//if ((value & 0x80) && !(old & 0x80) && (registerSTATUS & STATUS_VBLANK_FLAG))
		//	riseNMIFunction(riseNMIFunctionObj, 2);

		//if (nmi_cansuppress && (PPU_REG_MASK & 0x80)) nmi_suppress = 1;
	}

	void EmulatorPPU::WriteRegisterMASK(int8 value)
	{
		if (cycles < IGNORE_TIMER)
			return;

		registerMASK = value;
	}

	int8 EmulatorPPU::ReadRegisterSTATUS()
	{
		addressNibble    = 0;
		int8 ret         = registerSTATUS;
		registerSTATUS  &= 0x7f;
		if (statusOneCycleNMIMiss)
			ret &= ~STATUS_VBLANK_FLAG;
		return ret;
	}

	void EmulatorPPU::WriteRegisterOAMADDR(int8 value)
	{
		oamAddress = value;
	}

	void EmulatorPPU::WriteRegisterOAMDATA(int8 value)
	{
		oam[oamAddress++] = value;
	}

	int8 EmulatorPPU::ReadRegisterOAMDATA()
	{
		return oam[oamAddress] & (((oamAddress & 0x3) == 0x2) ? 0xe3 : 0xff);
	}

	void EmulatorPPU::WriteRegisterSCROLL(int8 value)
	{
		if (cycles < IGNORE_TIMER)
			return;

		if (!addressNibble)
		{
			scrollX = value & 0x7;

			scrollT &= ~0x1f;
			scrollT |= (value >> 3) & 0x1f;
		}
		else
		{
			scrollT &= ~(0x7 << 12);
			scrollT |= (value & 0x7) << 12;

			scrollT &= ~(0xf8 >> 3 << 5);
			scrollT |= (value & 0xf8) >> 3 << 5;
		}
		addressNibble = !addressNibble;
	}

	void EmulatorPPU::WriteRegisterADDR(int8 value)
	{
		if (cycles < IGNORE_TIMER)
			return;

		if (!addressNibble)
		{
			scrollT &= ~(0x1 << 14);
			scrollT &= ~(0x3f << 8);
			scrollT |= (value & 0x3f) << 8;
		}
		else
		{
            scrollT &= ~(0xff);
            scrollT |= value & 0xff;
            scrollV = scrollT;
		}
		addressNibble = !addressNibble;
	}

	void EmulatorPPU::WriteRegisterDATA(int8 value)
	{
		writeFunction(writeFunctionObj, scrollV, value);
		scrollV += (registerCTRL & 0x04) ? 32 : 1;
	}

	int8 EmulatorPPU::ReadRegisterDATA()
	{
		int8 ret = internalLatch;
		internalLatch = readFunction(readFunctionObj, scrollV);

		if (scrollV >= 0x3f00 && scrollV < 0x4000)
		{
			internalLatch = readFunction(readFunctionObj, scrollV - 0x1000);
			ret = readFunction(readFunctionObj, scrollV);
			if (registerMASK & 0x1) ret &= 0x30;
		}

		scrollV += (registerCTRL & 0x04) ? 32 : 1;
		return ret;
	}

#define ADVANCE_CYCLES(n)	\
		for (int _i = 0; _i < n; _i++) \
		{ \
			cycles += 1; \
			produceCPUCyclesFunction(produceCPUCyclesFunctionObj, 1); \
			bool oldNMI = generatingNMI; \
			generatingNMI = ((registerCTRL & 0x80) && (registerSTATUS & 0x80)); \
			if (!oldNMI && generatingNMI) riseNMIFunction(riseNMIFunctionObj, 0); \
		}


	void EmulatorPPU::ExecuteFrame()
	{
		printf("cycles: %d\n", cycles);
		cycles = 0;

		/* 0 - 239 : Visible Frame ======================================= */
		for (scanline = 0; scanline <= 239; scanline += 1)
		{
			ExecuteVisibleScanline();
		}

		/* 240 : Post-render line */
		scanline = 240;
		ADVANCE_CYCLES(341);

		/* 241 : Set V-Blank flag ======================================== */
		scanline = 241;
		statusOneCycleNMIMiss = true;
		registerSTATUS |= STATUS_VBLANK_FLAG;
		//bool nmiShouldOccur = registerCTRL & 0x80;
		ADVANCE_CYCLES(1);
		statusOneCycleNMIMiss = false;

		ADVANCE_CYCLES(5)
		//if (nmiShouldOccur)
		//	riseNMIFunction(riseNMIFunctionObj, 0);
		ADVANCE_CYCLES(335);

		/* 242 - 260 : V-Blank =========================================== */
		for (scanline = 242; scanline <= 260; scanline += 1)
		{
			ADVANCE_CYCLES(341);
		}

		/* 261 : Pre-render line ========================================= */
		scanline = 261;
		ADVANCE_CYCLES(1);
		registerSTATUS &= ~STATUS_VBLANK_FLAG;
		registerSTATUS &= 0x1F;

		ADVANCE_CYCLES(279);
		for (int i = 280; i <= 304; i++)
		{
			if (registerMASK & 0x18)
			{
				scrollV &= ~(0x7be0);
				scrollV |= scrollT & 0x7be0;
			}
			ADVANCE_CYCLES(1);
		}

		ADVANCE_CYCLES(35);

		if (!(registerMASK & 0x18) || (frame & 0x1))
			ADVANCE_CYCLES(1);

		frame++;



		/*for(int j = 0; j < 8; j++)
		{
			spriteTable[j].id = 255;
		}

		scanline = 241;
		dot = -1;

		int nmi_shouldoccur = 0;

		ADVANCE_CYCLES(341);

		if (cycles >= 27384 * 3)
			registerSTATUS |= 0x80; // VBlank Started Flag

		ADVANCE_CYCLES(2);
		nmi_shouldoccur = registerCTRL & 0x80;
		ADVANCE_CYCLES(18);

		if (nmi_shouldoccur)// && !ppu->nmi_suppress) // VBlank Generate NMI Flag
			riseNMIFunction(riseNMIFunctionObj);

		ADVANCE_CYCLES(20 * 341 - 20);
		registerSTATUS &= 0x1F;

		ADVANCE_CYCLES(327);
		if (!(registerMASK & 0x18) || (frame & 0x1))
			ADVANCE_CYCLES(1);

		frame++;
		ADVANCE_CYCLES(340 - 327);

		if (registerMASK & 0x18)
		{
			scrollV &= ~(0x7be0);
			scrollV |= scrollT & 0x7be0;
		}

		for (int i = 0; i < 240; i++)
		{
			scanline = i;
			ExecuteVisibleScanline();
		}*/
	}

	void EmulatorPPU::ExecuteVisibleScanline()
	{
		int8 pal = 0;
		int8 tiledata1 = 0, tiledata2 = 0;

		dot = 0;

		int internalScrollX = scrollX;

		for(int i = 0; i < 256; i++)
		{
			ADVANCE_CYCLES(1);

			if (!(registerMASK & 0x18))
			{
				int8 bkgpalette = 0;
				if (scrollV >= 0x3f00 && scrollV < 0x4000)
					bkgpalette = 0x3f & readFunction(readFunctionObj, scrollV);
				else
					bkgpalette = 0x3f & readFunction(readFunctionObj, 0x3f00);

				outputFunction(outputFunctionObj, scanline, dot, bkgpalette, registerMASK);
			}
			else
			{
				if (dot == 0 || internalScrollX == 0)
				{
					pal = readFunction(readFunctionObj, 0x23c0 | (scrollV & 0x0c00) | ((scrollV >> 4) & 0x38) | ((scrollV >> 2) & 0x07));
					pal = (pal >> ((scrollV & 0x2) | ((scrollV >> 4) & 0x4))) & 0x3;

					int tilen = readFunction(readFunctionObj, 0x2000 | (scrollV & 0x0fff));

					int ptptr = ((registerCTRL & 0x10) << 8) + (tilen << 4) + (scrollV >> 12);
					tiledata1 = readFunction(readFunctionObj, ptptr);
					tiledata2 = readFunction(readFunctionObj, ptptr + 8);
				}

				int8 bkg2bit = 0;
				int8 bkgpalette = 0;

				if (!(registerMASK & 0x08) || (!(registerMASK & 0x02) && dot < 8))
				{
					bkg2bit = 0;
					bkgpalette = 0x3f & readFunction(readFunctionObj, 0x3f00);
				}
				else
				{
					bkg2bit = ((tiledata1 & (0x80 >> internalScrollX)) > 0) | (((tiledata2 & (0x80 >> internalScrollX)) > 0) << 1);
					bkgpalette = 0x3f & readFunction(readFunctionObj, 0x3f00 | ((pal << 2) | bkg2bit));
				}

				if (registerMASK & 0x1) bkgpalette &= 0x30;
				ExecuteSprites(bkg2bit, bkgpalette);

				internalScrollX = (internalScrollX + 1) % 8;
				if (internalScrollX == 0)
				{
					if ((scrollV & 0x001f) == 31)
					{
						scrollV &= ~0x001f;
						scrollV ^= 0x0400;
					}
					else
						scrollV += 1;
				}
			}

			dot++;
		}

		if (registerMASK & 0x10) ExecuteSpriteFetch();

		ADVANCE_CYCLES(1);

		if (registerMASK & 0x18)
		{
			if ((scrollV & 0x7000) != 0x7000)
				scrollV += 0x1000;
			else
			{
				scrollV &= ~0x7000;
				int y = (scrollV & 0x03e0) >> 5;
				if (y == 29)
				{
					y = 0;
					scrollV ^= 0x0800;
				}
				else if (y == 31)
					y = 0;
				else
					y += 1;

				scrollV = (scrollV & ~0x03e0) | (y << 5);
			}
		}

		ADVANCE_CYCLES(1);

		if (registerMASK & 0x18)
		{
			scrollV &= ~(0x41f);
			scrollV |= scrollT & 0x41f;
		}

		ADVANCE_CYCLES(341 - 256 - 2);
	}

	void EmulatorPPU::ExecuteSprites(int8 bkg2bit, int8 bkgpalette)
	{
		if (!(registerMASK & 0x10) || (!(registerMASK & 0x04) && dot < 8))
			goto outputBkg;

		for(int i = 0; i < 8; i++)
		{
			if (spriteTable[i].id == 255) break;

			int8 sprx     =  spriteTable[i].x;
			bool inverth  = (spriteTable[i].priority & 0x02) >> 1;
			bool priority = (spriteTable[i].priority & 0x01);

			int xx = dot - sprx;
			if (xx >= 0 && xx < 8)
			{
				int8 spr2bit;

				if (inverth)
					spr2bit = (((spriteTable[i].pattern0 >> xx) & 0x01) ? 0x1 : 0) | (((spriteTable[i].pattern1 >> xx) & 0x01) ? 0x2 : 0);
				else
					spr2bit = (((spriteTable[i].pattern0 << xx) & 0x80) ? 0x1 : 0) | (((spriteTable[i].pattern1 << xx) & 0x80) ? 0x2 : 0);

				if (spr2bit == 0) continue;

				if (spriteTable[i].id == 0 && bkg2bit != 0 && dot < 255)
					registerSTATUS |= 0x40;

				if (priority && bkg2bit != 0) goto outputBkg;

				int8 sprpalette = 0x3F & readFunction(readFunctionObj, 0x3F10 | ((spriteTable[i].palette << 2) | spr2bit));
				if (registerMASK & 0x1) sprpalette &= 0x30;

				outputFunction(outputFunctionObj, scanline, dot, sprpalette, registerMASK);
				return;
			}
		}

	outputBkg:
		outputFunction(outputFunctionObj, scanline, dot, bkgpalette, registerMASK);
	}

	void EmulatorPPU::ExecuteSpriteFetch()
	{
		for(int i = 0; i < 8; i++)
		{
			spriteTable[i].id = 255;
		}

		int8 size          = registerCTRL & 0x20 ? 16 : 8;
		int16 patterntable = registerCTRL & 0x08 ? 0x1000 : 0x0000;

		int oamslot = 0;
		for(int i = 0; i < 64 && oamslot < 8; i++)
		{
			int8 spry = oam[(i << 2)];

			if (!(scanline - spry >= 0 && scanline - spry < size)) continue;

			if ((oamslot > 7 && ((registerMASK & 0x18) != 0) && spry < 240))
			{
				registerSTATUS |= 0x20;
				break;
			}

			int8 sprtile  = oam[(i << 2) + 1];
			int8 sprattr  = oam[(i << 2) + 2];
			int8 sprx     = oam[(i << 2) + 3];
			bool inverth  = (sprattr & 0x40 ? 1 : 0);
			bool invertv  = (sprattr & 0x80 ? 1 : 0);
			bool priority = (sprattr & 0x20 ? 1 : 0);
			int8 pput_tile= sprtile;
			int8 sprty    = (invertv ? 7 : 0);

			if (size == 16)
			{
				patterntable = (sprtile & 0x1) ? 0x1000 : 0;
				pput_tile   &= 0xFE;
			}

			int yy = scanline - spry;
			if (size == 16)
			{
				if (invertv)
				{
					if (yy >= 8)
						sprty      = 15 - yy;
					else
					{
						pput_tile += 1;
						sprty      = 7  - yy;
					}
				}
				else
				{
					if (yy >= 8)
					{
						pput_tile += 1;
						sprty      = yy - 8;
					}
					else
						sprty      = yy;
				}
			}
			else
			{
				if (invertv) sprty = 7 - yy;
				else sprty = yy;
			}

			spriteTable[oamslot].id       = i;
			spriteTable[oamslot].tilen    = patterntable + (pput_tile << 4) + sprty;
			spriteTable[oamslot].x        = sprx;
			spriteTable[oamslot].y        = spry;
			spriteTable[oamslot].palette  = sprattr & 0x03;
			spriteTable[oamslot].priority = priority | (inverth << 1);
			spriteTable[oamslot].pattern0 = readFunction(readFunctionObj, spriteTable[oamslot].tilen);
			spriteTable[oamslot].pattern1 = readFunction(readFunctionObj, spriteTable[oamslot].tilen + 8);
			oamslot++;
		}
	}
}
