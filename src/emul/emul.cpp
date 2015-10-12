#include "emul.h"
#include "emul_rom.h"
#include "emul_cartridge_nrom.h"
#include "emul_cartridge_uxrom.h"
#include "emul_cartridge_axrom.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../gui/win/gui_win_main.h"

namespace MahNES
{
	int8 Emulator::CPURead(void* ptr, int16 addr)
	{
		int8 cartridgeRead = ((Emulator*)ptr)->cartridge->cpuReadFunction(((Emulator*)ptr)->cartridge->ptr, addr);

		if (addr < 0x2000) return ((Emulator*)ptr)->ram[addr % 0x800];
		else if (addr >= 0x2000 && addr < 0x3000)
		{
			int16 resolvedAddr = 0x2000 | (addr & 0x7);
			if (resolvedAddr == 0x2002) return ((Emulator*)ptr)->ppu.ReadRegisterSTATUS();
			else if (resolvedAddr == 0x2004) return ((Emulator*)ptr)->ppu.ReadRegisterOAMDATA();
			else if (resolvedAddr == 0x2007) return ((Emulator*)ptr)->ppu.ReadRegisterDATA();
			else return 0;
		}
		else if (addr == 0x4015) return ((Emulator*)ptr)->apu.ReadRegister(addr);
		else if (addr == 0x4016)
		{
			int bit = ((Emulator*)ptr)->controller[0] & 0x1;
			((Emulator*)ptr)->controller[0] >>= 1;
			return (bit);
		}
		else if (addr == 0x4017)
			return 0;
		else if (addr >= 0x8000) return cartridgeRead;
		else
			return 0;
	}

	void Emulator::CPUWrite(void* ptr, int16 addr, int8 value)
	{
		((Emulator*)ptr)->cartridge->cpuWriteFunction(((Emulator*)ptr)->cartridge->ptr, addr, value);

		if (addr < 0x2000) ((Emulator*)ptr)->ram[addr % 0x800] = value;
		else if (addr >= 0x2000 && addr < 0x3000)
		{
			int16 resolvedAddr = 0x2000 | (addr & 0x7);
			if (resolvedAddr == 0x2000) ((Emulator*)ptr)->ppu.WriteRegisterCTRL(value);
			else if (resolvedAddr == 0x2001) ((Emulator*)ptr)->ppu.WriteRegisterMASK(value);
			else if (resolvedAddr == 0x2003) ((Emulator*)ptr)->ppu.WriteRegisterOAMADDR(value);
			else if (resolvedAddr == 0x2004) ((Emulator*)ptr)->ppu.WriteRegisterOAMDATA(value);
			else if (resolvedAddr == 0x2005) ((Emulator*)ptr)->ppu.WriteRegisterSCROLL(value);
			else if (resolvedAddr == 0x2006) ((Emulator*)ptr)->ppu.WriteRegisterADDR(value);
			else if (resolvedAddr == 0x2007) ((Emulator*)ptr)->ppu.WriteRegisterDATA(value);
		}
		else if (addr == 0x4014)
		{
			if (value != 0x40)
			{
				for (int i = 0; i < 256; i++)
					((Emulator*)ptr)->ppu.oam[i] = CPURead(ptr, (value << 8) | i);
			}
		}
		else if (addr == 0x4016)
		{
			if (value & 0x1) {
				((Emulator*)ptr)->controllerStrobe = 0;
				((Emulator*)ptr)->controller[0] = (((GetKeyState(VK_SPACE)&0x80)?1:0) |
								(((GetKeyState('X')&0x80)?1:0)<<1) |
								(((GetKeyState(VK_CONTROL)&0x80)?1:0)<<2) |
								(((GetKeyState(VK_RETURN)&0x80)?1:0)<<3) |
								(((GetKeyState(VK_UP)&0x80)?1:0)<<4) |
								(((GetKeyState(VK_DOWN)&0x80)?1:0)<<5) |
								(((GetKeyState(VK_LEFT)&0x80)?1:0)<<6) |
								(((GetKeyState(VK_RIGHT)&0x80)?1:0)<<7));
			} else
				((Emulator*)ptr)->controllerStrobe = 0x1;
		}
		else if (addr >= 0x4000 && addr < 0x4017)
		{
			((Emulator*)ptr)->apu.Execute(((Emulator*)ptr)->apuCycles / 3.0);
			((Emulator*)ptr)->apuCycles = 0;
			((Emulator*)ptr)->apu.WriteRegister(addr, value);
		}
	}

	int8 Emulator::PPURead(void* ptr, int16 addr)
	{
		int8 cartridgeRead = ((Emulator*)ptr)->cartridge->ppuReadFunction(((Emulator*)ptr)->cartridge->ptr, addr);

		if (addr < 0x2000) return cartridgeRead;
		else if (addr < 0x3000) return ((Emulator*)ptr)->vram[((addr & 0x3ff) | ((((Emulator*)ptr)->cartridge->ppuCIRAMMirrorFunction(((Emulator*)ptr)->cartridge->ptr, addr)) ? 0x400 : 0)) & 0x7ff];
		else if (addr >= 0x3f00 && addr < 0x4000) return ((Emulator*)ptr)->paletteram[(addr - 0x3f00) % 0x20];
		else return 0;
	}

	void Emulator::PPUWrite(void* ptr, int16 addr, int8 value)
	{
		((Emulator*)ptr)->cartridge->ppuWriteFunction(((Emulator*)ptr)->cartridge->ptr, addr, value);

		if (addr >= 0x2000 && addr < 0x3000) ((Emulator*)ptr)->vram[((addr & 0x3ff) | ((((Emulator*)ptr)->cartridge->ppuCIRAMMirrorFunction(((Emulator*)ptr)->cartridge->ptr, addr | 0x8000)) ? 0x400 : 0)) & 0x7ff] = value;
		else if (addr >= 0x3f00 && addr < 0x4000)
		{
			if (!(addr & 0xf)) {
				((Emulator*)ptr)->paletteram[0x3F00 - 0x3f00] =
				((Emulator*)ptr)->paletteram[0x3F04 - 0x3f00] =
				((Emulator*)ptr)->paletteram[0x3F08 - 0x3f00] =
				((Emulator*)ptr)->paletteram[0x3F0C - 0x3f00] =
				((Emulator*)ptr)->paletteram[0x3F10 - 0x3f00] =
				((Emulator*)ptr)->paletteram[0x3F14 - 0x3f00] =
				((Emulator*)ptr)->paletteram[0x3F18 - 0x3f00] =
				((Emulator*)ptr)->paletteram[0x3F1C - 0x3f00] = value;
			} else if (addr & 0x3) {
				((Emulator*)ptr)->paletteram[(addr - 0x3f00) % 0x20] = value;
			}
		}
	}

	void Emulator::PPUProduceCPUCycles(void* ptr, int cycles)
	{
		((Emulator*)ptr)->cpu.Execute(cycles);
		((Emulator*)ptr)->apuCycles += cycles;
		if (((Emulator*)ptr)->apuCycles > 1000)
		{
			((Emulator*)ptr)->apu.Execute(((Emulator*)ptr)->apuCycles / 3.0);
			((Emulator*)ptr)->apuCycles = 0;
		}
	}

	void Emulator::PPUOutput(void* ptr, int scanline, int pixel, int8 palette, int8 mask)
	{
        (void)ptr;
        (void)scanline;
        (void)pixel;
        (void)palette;
        (void)mask;
	}

	void Emulator::PPURiseNMI(void* ptr, int delayCycles)
	{
		((Emulator*)ptr)->apu.Execute(((Emulator*)ptr)->apuCycles / 3.0);
		((Emulator*)ptr)->apuCycles = 0;
		((Emulator*)ptr)->cpu.NMI(delayCycles);
	}

	void Emulator::SetPPUOutputFunction(void* ptr, EmulatorPPU::OutputFunction f)
	{
		ppu.SetOutputFunction(ptr, f);
	}

	void Emulator::SetAPUOutputFunction(void* ptr, EmulatorAPU::OutputFunction f)
	{
		apu.SetOutputFunction(ptr, f);
	}

	Emulator::Emulator()
	{
		cpu.SetReadFunction((void*)this, CPURead);
		cpu.SetWriteFunction((void*)this, CPUWrite);
		cpu.SetClockDivider(3);

		ppu.SetReadFunction((void*)this, PPURead);
		ppu.SetWriteFunction((void*)this, PPUWrite);
		ppu.SetProduceCPUCyclesFunction((void*)this, PPUProduceCPUCycles);
		ppu.SetOutputFunction((void*)this, PPUOutput);
		ppu.SetRiseNMIFunction((void*)this, PPURiseNMI);

		apu.Init(44100, 44100 / 20);
		apuCycles = 0;
	}

	Emulator::~Emulator()
	{

	}

	void Emulator::Reset()
	{
		cpu.Reset();
		ppu.Reset();
		apu.Reset();
	}

	void Emulator::ExecuteFrame()
	{
		ppu.ExecuteFrame();
	}

	int Emulator::LoadROM(const char* filename)
	{
		EmulatorROM rom;
		int res = rom.LoadROM(filename);

		if (res == 0)
		{
			if (rom.mapperID == 0x2) cartridge = new EmulatorCartridgeUxROM;
			else if (rom.mapperID == 0x7) cartridge = new EmulatorCartridgeAxROM;
            else cartridge = new EmulatorCartridgeNROM;

            cartridge->LoadROM(&rom);
		}

		return res;
	}
}
