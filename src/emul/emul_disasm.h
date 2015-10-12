#ifndef EMUL_DISASM_H
#define EMUL_DISASM_H

#include "emul_types.h"
#include <string.h>
#include <stdio.h>

namespace MahNES
{
	char* Disassemble(int16 pc, int8 b1, int8 b2, int8 b3)
	{
		static char str[60];

		switch(b1) {
	/* BRK         */ case 0x00: sprintf(str,"BRK"); break;
	/* NOP         */ case 0xEA: sprintf(str,"NOP"); break;

	/* JMP add     */ case 0x4C: sprintf(str,"JMP $%02X%02X",b3,b2); break;
	/* JMP (add)   */ case 0x6C: sprintf(str,"JMP ($%02X%02X)",b3,b2); break;
	/* JSR add     */ case 0x20: sprintf(str,"JSR $%02X%02X",b3,b2); break;
	/* RTS         */ case 0x60: sprintf(str,"RTS"); break;
	/* RTI         */ case 0x40: sprintf(str,"RTI"); break;

	/* CMP #x      */ case 0xC9: sprintf(str,"CMP #%02X",b2); break;
	/* CMP 0add    */ case 0xC5: sprintf(str,"CMP <$%02X",b2); break;
	/* CMP 0add,X  */ case 0xD5: sprintf(str,"CMP <$%02X, X",b2); break;
	/* CMP add     */ case 0xCD: sprintf(str,"CMP $%02X%02X",b3,b2); break;
	/* CMP add,X   */ case 0xDD: sprintf(str,"CMP $%02X%02X, X",b3,b2); break;
	/* CMP add,Y   */ case 0xD9: sprintf(str,"CMP $%02X%02X, Y",b3,b2); break;
	/* CMP (0add,X)*/ case 0xC1: sprintf(str,"CMP ($%02X, X)",b2); break;
	/* CMP (0add),Y*/ case 0xD1: sprintf(str,"CMP ($%02X), Y",b2); break;
	/* CPX #x      */ case 0xE0: sprintf(str,"CPX #%02X",b2); break;
	/* CPX 0add    */ case 0xE4: sprintf(str,"CPX <$%02X",b2); break;
	/* CPX add     */ case 0xEC: sprintf(str,"CPX $%02X%02X",b3,b2); break;
	/* CPY #x      */ case 0xC0: sprintf(str,"CPY #%02X",b2); break;
	/* CPY 0add    */ case 0xC4: sprintf(str,"CPY <$%02X",b2); break;
	/* CPY add     */ case 0xCC: sprintf(str,"CPY $%02X%02X",b3,b2); break;

	/* BEQ rel     */ case 0xF0: sprintf(str,"BEQ $%04X",pc + 2 + (b2 & 0x80 ? -(256 - b2) : b2)); break;
	/* BNE rel     */ case 0xD0: sprintf(str,"BNE $%04X",pc + 2 + (b2 & 0x80 ? -(256 - b2) : b2)); break;
	/* BMI rel     */ case 0x30: sprintf(str,"BMI $%04X",pc + 2 + (b2 & 0x80 ? -(256 - b2) : b2)); break;
	/* BPL rel     */ case 0x10: sprintf(str,"BPL $%04X",pc + 2 + (b2 & 0x80 ? -(256 - b2) : b2)); break;
	/* BCS rel     */ case 0xB0: sprintf(str,"BCS $%04X",pc + 2 + (b2 & 0x80 ? -(256 - b2) : b2)); break;
	/* BCC rel     */ case 0x90: sprintf(str,"BCC $%04X",pc + 2 + (b2 & 0x80 ? -(256 - b2) : b2)); break;
	/* BVS rel     */ case 0x70: sprintf(str,"BVS $%04X",pc + 2 + (b2 & 0x80 ? -(256 - b2) : b2)); break;
	/* BVC rel     */ case 0x50: sprintf(str,"BVC $%04X",pc + 2 + (b2 & 0x80 ? -(256 - b2) : b2)); break;

	/* AND #x      */ case 0x29: sprintf(str,"AND #%02X",b2); break;
	/* AND 0add    */ case 0x25: sprintf(str,"AND <$%02X",b2); break;
	/* AND 0add,X  */ case 0x35: sprintf(str,"AND <$%02X, X",b2); break;
	/* AND add     */ case 0x2D: sprintf(str,"AND $%02X%02X",b3,b2); break;
	/* AND add,X   */ case 0x3D: sprintf(str,"AND $%02X%02X, X",b3,b2); break;
	/* AND add,Y   */ case 0x39: sprintf(str,"AND $%02X%02X, Y",b3,b2); break;
	/* AND (0add,X)*/ case 0x21: sprintf(str,"AND ($%02X, X)",b2); break;
	/* AND (0add),Y*/ case 0x31: sprintf(str,"AND ($%02X), Y",b2); break;
	/* EOR #x      */ case 0x49: sprintf(str,"EOR #%02X",b2); break;
	/* EOR 0add    */ case 0x45: sprintf(str,"EOR <$%02X",b2); break;
	/* EOR 0add,X  */ case 0x55: sprintf(str,"EOR <$%02X, X",b2); break;
	/* EOR add     */ case 0x4D: sprintf(str,"EOR $%02X%02X",b3,b2); break;
	/* EOR add,X   */ case 0x5D: sprintf(str,"EOR $%02X%02X, X",b3,b2); break;
	/* EOR add,Y   */ case 0x59: sprintf(str,"EOR $%02X%02X, Y",b3,b2); break;
	/* EOR (0add,X)*/ case 0x41: sprintf(str,"EOR ($%02X, X)",b2); break;
	/* EOR (0add),Y*/ case 0x51: sprintf(str,"EOR ($%02X), Y",b2); break;
	/* ORA #x      */ case 0x09: sprintf(str,"ORA #%02X",b2); break;
	/* ORA 0add    */ case 0x05: sprintf(str,"ORA <$%02X",b2); break;
	/* ORA 0add,X  */ case 0x15: sprintf(str,"ORA <$%02X, X",b2); break;
	/* ORA add     */ case 0x0D: sprintf(str,"ORA $%02X%02X",b3,b2); break;
	/* ORA add,X   */ case 0x1D: sprintf(str,"ORA $%02X%02X, X",b3,b2); break;
	/* ORA add,Y   */ case 0x19: sprintf(str,"ORA $%02X%02X, Y",b3,b2); break;
	/* ORA (0add,X)*/ case 0x01: sprintf(str,"ORA ($%02X, X)",b2); break;
	/* ORA (0add),Y*/ case 0x11: sprintf(str,"ORA ($%02X), Y",b2); break;

	/* ASL A       */ case 0x0A: sprintf(str,"ASL A"); break;
	/* ASL 0add    */ case 0x06: sprintf(str,"ASL <$%02X",b2); break;
	/* ASL 0add,X  */ case 0x16: sprintf(str,"ASL <$%02X, X",b2); break;
	/* ASL add     */ case 0x0E: sprintf(str,"ASL $%02X%02X",b3,b2); break;
	/* ASL add,X   */ case 0x1E: sprintf(str,"ASL $%02X%02X, X",b3,b2); break;
	/* LSR A       */ case 0x4A: sprintf(str,"LSR A"); break;
	/* LSR 0add    */ case 0x46: sprintf(str,"LSR <$%02X",b2); break;
	/* LSR 0add,X  */ case 0x56: sprintf(str,"LSR <$%02X, X",b2); break;
	/* LSR add     */ case 0x4E: sprintf(str,"LSR $%02X%02X",b3,b2); break;
	/* LSR add,X   */ case 0x5E: sprintf(str,"LSR $%02X%02X, X",b3,b2); break;
	/* ROL A       */ case 0x2A: sprintf(str,"ROL A"); break;
	/* ROL 0add    */ case 0x26: sprintf(str,"ROL <$%02X",b2); break;
	/* ROL 0add,X  */ case 0x36: sprintf(str,"ROL <$%02X, X",b2); break;
	/* ROL add     */ case 0x2E: sprintf(str,"ROL $%02X%02X",b3,b2); break;
	/* ROL add,X   */ case 0x3E: sprintf(str,"ROL $%02X%02X, X",b3,b2); break;
	/* ROR A       */ case 0x6A: sprintf(str,"ROR A"); break;
	/* ROR 0add    */ case 0x66: sprintf(str,"ROR <$%02X",b2); break;
	/* ROR 0add,X  */ case 0x76: sprintf(str,"ROR <$%02X, X",b2); break;
	/* ROR add     */ case 0x6E: sprintf(str,"ROR $%02X%02X",b3,b2); break;
	/* ROR add,X   */ case 0x7E: sprintf(str,"ROR $%02X%02X, X",b3,b2); break;

	/* BIT 0add    */ case 0x24: sprintf(str,"BIT <$%02X",b2); break;
	/* BIT add     */ case 0x2C: sprintf(str,"BIT $%02X%02X",b3,b2); break;

	/* ADC #x      */ case 0x69: sprintf(str,"ADC #%02X",b2); break;
	/* ADC 0add    */ case 0x65: sprintf(str,"ADC <$%02X",b2); break;
	/* ADC 0add,X  */ case 0x75: sprintf(str,"ADC <$%02X, X",b2); break;
	/* ADC add     */ case 0x6D: sprintf(str,"ADC $%02X%02X",b3,b2); break;
	/* ADC add,X   */ case 0x7D: sprintf(str,"ADC $%02X%02X, X",b3,b2); break;
	/* ADC add,Y   */ case 0x79: sprintf(str,"ADC $%02X%02X, Y",b3,b2); break;
	/* ADC (0add,X)*/ case 0x61: sprintf(str,"ADC ($%02X, X)",b2); break;
	/* ADC (0add),Y*/ case 0x71: sprintf(str,"ADC ($%02X), Y",b2); break;
	/* SBC #x      */ case 0xE9: sprintf(str,"SBC #%02X",b2); break;
	/* SBC 0add    */ case 0xE5: sprintf(str,"SBC <$%02X",b2); break;
	/* SBC 0add,X  */ case 0xF5: sprintf(str,"SBC <$%02X, X",b2); break;
	/* SBC add     */ case 0xED: sprintf(str,"SBC $%02X%02X",b3,b2); break;
	/* SBC add,X   */ case 0xFD: sprintf(str,"SBC $%02X%02X, X",b3,b2); break;
	/* SBC add,Y   */ case 0xF9: sprintf(str,"SBC $%02X%02X, Y",b3,b2); break;
	/* SBC (0add,X)*/ case 0xE1: sprintf(str,"SBC ($%02X, X)",b2); break;
	/* SBC (0add),Y*/ case 0xF1: sprintf(str,"SBC ($%02X), Y",b2); break;

	/* INC 0add    */ case 0xE6: sprintf(str,"INC <$%02X",b2); break;
	/* INC 0add,X  */ case 0xF6: sprintf(str,"INC <$%02X, X",b2); break;
	/* INC add     */ case 0xEE: sprintf(str,"INC $%02X%02X",b3,b2); break;
	/* INC add,X   */ case 0xFE: sprintf(str,"INC $%02X%02X, X",b3,b2); break;
	/* DEC 0add    */ case 0xC6: sprintf(str,"DEC <$%02X",b2); break;
	/* DEC 0add,X  */ case 0xD6: sprintf(str,"DEC <$%02X, X",b2); break;
	/* DEC add     */ case 0xCE: sprintf(str,"DEC $%02X%02X",b3,b2); break;
	/* DEC add,X   */ case 0xDE: sprintf(str,"DEC $%02X%02X, X",b3,b2); break;

	/* LDA #x      */ case 0xA9: sprintf(str,"LDA #%02X",b2); break;
	/* LDA 0add    */ case 0xA5: sprintf(str,"LDA <$%02X",b2); break;
	/* LDA 0add,X  */ case 0xB5: sprintf(str,"LDA <$%02X, X",b2); break;
	/* LDA add     */ case 0xAD: sprintf(str,"LDA $%02X%02X",b3,b2); break;
	/* LDA add,X   */ case 0xBD: sprintf(str,"LDA $%02X%02X, X",b3,b2); break;
	/* LDA add,Y   */ case 0xB9: sprintf(str,"LDA $%02X%02X, Y",b3,b2); break;
	/* LDA (0add,X)*/ case 0xA1: sprintf(str,"LDA ($%02X, X)",b2); break;
	/* LDA (0add),Y*/ case 0xB1: sprintf(str,"LDA ($%02X), Y",b2); break;
	/* STA 0add    */ case 0x85: sprintf(str,"STA <$%02X",b2); break;
	/* STA 0add,X  */ case 0x95: sprintf(str,"STA <$%02X, X",b2); break;
	/* STA add     */ case 0x8D: sprintf(str,"STA $%02X%02X",b3,b2); break;
	/* STA add,X   */ case 0x9D: sprintf(str,"STA $%02X%02X, X",b3,b2); break;
	/* STA add,Y   */ case 0x99: sprintf(str,"STA $%02X%02X, Y",b3,b2); break;
	/* STA (0add,X)*/ case 0x81: sprintf(str,"STA ($%02X, X)",b2); break;
	/* STA (0add),Y*/ case 0x91: sprintf(str,"STA ($%02X), Y",b2); break;

	/* INX         */ case 0xE8: sprintf(str,"INX"); break;
	/* DEX         */ case 0xCA: sprintf(str,"DEX"); break;
	/* LDX #x      */ case 0xA2: sprintf(str,"LDX #%02X",b2); break;
	/* LDX 0add    */ case 0xA6: sprintf(str,"LDX <$%02X",b2); break;
	/* LDX 0add,Y  */ case 0xB6: sprintf(str,"LDX <$%02X, Y",b2); break;
	/* LDX add     */ case 0xAE: sprintf(str,"LDX $%02X%02X",b3,b2); break;
	/* LDX add,Y   */ case 0xBE: sprintf(str,"LDX $%02X%02X, Y",b3,b2); break;
	/* STX 0add    */ case 0x86: sprintf(str,"STX <$%02X",b2); break;
	/* STX 0add,Y  */ case 0x96: sprintf(str,"STX <$%02X, Y",b2); break;
	/* STX add     */ case 0x8E: sprintf(str,"STX $%02X%02X",b3,b2); break;
	/* INY         */ case 0xC8: sprintf(str,"INY"); break;
	/* DEY         */ case 0x88: sprintf(str,"DEY"); break;
	/* LDY #x      */ case 0xA0: sprintf(str,"LDY #%02X",b2); break;
	/* LDY 0add    */ case 0xA4: sprintf(str,"LDY <$%02X",b2); break;
	/* LDY 0add,X  */ case 0xB4: sprintf(str,"LDY <$%02X, X",b2); break;
	/* LDY add     */ case 0xAC: sprintf(str,"LDY $%02X%02X",b3,b2); break;
	/* LDY add,X   */ case 0xBC: sprintf(str,"LDY $%02X%02X, X",b3,b2); break;
	/* STY 0add    */ case 0x84: sprintf(str,"STY <$%02X",b2); break;
	/* STY 0add,X  */ case 0x94: sprintf(str,"STY <$%02X, X",b2); break;
	/* STY add     */ case 0x8C: sprintf(str,"STY $%02X%02X",b3,b2); break;

	/* PHA         */ case 0x48: sprintf(str,"PHA"); break;
	/* PHP         */ case 0x08: sprintf(str,"PHP"); break;
	/* PLA         */ case 0x68: sprintf(str,"PLA"); break;
	/* PLP         */ case 0x28: sprintf(str,"PLP"); break;

	/* TAX         */ case 0xAA: sprintf(str,"TAX"); break;
	/* TAY         */ case 0xA8: sprintf(str,"TAY"); break;
	/* TSX         */ case 0xBA: sprintf(str,"TSX"); break;
	/* TXS         */ case 0x9A: sprintf(str,"TXS"); break;
	/* TXA         */ case 0x8A: sprintf(str,"TXA"); break;
	/* TYA         */ case 0x98: sprintf(str,"TYA"); break;

	/* SEC         */ case 0x38: sprintf(str,"SEC"); break;
	/* SED         */ case 0xF8: sprintf(str,"SED"); break;
	/* SEI         */ case 0x78: sprintf(str,"SEI"); break;
	/* CLC         */ case 0x18: sprintf(str,"CLC"); break;
	/* CLD         */ case 0xD8: sprintf(str,"CLD"); break;
	/* CLI         */ case 0x58: sprintf(str,"CLI"); break;
	/* CLV         */ case 0xB8: sprintf(str,"CLV"); break;

					  default: sprintf(str, "???"); break;
		}

		return str;
	}
}

#endif
