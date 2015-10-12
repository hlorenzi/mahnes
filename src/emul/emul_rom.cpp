#include "emul_rom.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace MahNES
{
	int EmulatorROM::LoadROM(const char* filename)
	{
		FILE* f = fopen(filename, "rb");
		if (f == NULL) return -1;

		fseek(f, 0, SEEK_END);
		unsigned int filesize;

		filesize = ftell(f);
		rewind(f);

		int8* filedata = (int8*)malloc(sizeof(int8) * filesize);

		if (filedata == NULL)
			return -2;

		if (fread(filedata, 1, filesize, f) != filesize)
		{
			if (!feof(f))
			{
				free(filedata);
				return -3;
			}
		}

		fclose(f);

		if ( (filedata[0] == 'N' &&
			  filedata[1] == 'E' &&
			  filedata[2] == 'S' &&
			  filedata[3] == 0x1A))
		{
			//format   = EMUL_FORMAT_INES;
			//size     = s - 16;
			//strcpy(filename, file);

			prg16KBBankNumber = filedata[4];
			chr8KBBankNumber = filedata[5];
			prgSize = prg16KBBankNumber * 0x4000;
			chrSize = chr8KBBankNumber * 0x2000;

			prgROM = (int8*)malloc(sizeof(int8) * (filesize - 16));
			memcpy(prgROM, filedata + 16, filesize - 16);

			chrROM = prgROM + prgSize;

			if (!(filedata[10] == 0 && filedata[9] == 0 && filedata[8] == 0 && filedata[7] == 0))
			{
				mapperID = ((filedata[6] & 0xF0) >> 4);
			}
			else
			{
				mapperID = (filedata[7] & 0xF0) | ((filedata[6] & 0xF0) >> 4);
			}

			//emul_set_mapper(mapn);
			mirroring       = !(filedata[6] & 0x01);
			hasSRAM         = (filedata[6]  & 0x02) > 0;
			hasTrainer      = (filedata[6]  & 0x04) > 0;
			hasExtraRAM     = (filedata[10] & 0x10) > 0;
			hasBusConflicts = (filedata[10] & 0x20) > 0;

			region = filedata[10] & 0x3;//(filedata[10] & 0x1) ? EMUL_REGION_NTSC : ((filedata[10] & 0x2) ? EMUL_REGION_PAL : -1);

			free(filedata);
			return 0;
		}
		else
		{
			free(filedata);
			return -4;
		}
	}
}
