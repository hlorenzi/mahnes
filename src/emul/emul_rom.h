#ifndef EMUL_ROM_H
#define EMUL_ROM_H

#include "emul_types.h"

namespace MahNES
{
	class EmulatorROM
	{
	public:
		int LoadROM(const char* filename);

		int8* prgROM;
		int8* chrROM;
		int prgSize;
		int prg16KBBankNumber;
		int chrSize;
		int chr8KBBankNumber;
		int mapperID;
		int region;
		bool mirroring;
		bool hasSRAM;
		bool hasTrainer;
		bool hasExtraRAM;
		bool hasBusConflicts;
	};
}

#endif
