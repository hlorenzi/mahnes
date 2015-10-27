#ifndef EMUL_CPU_H
#define EMUL_CPU_H

#include "emul_types.h"

namespace MahNES
{
	class EmulatorCPU
	{
	public:
		typedef int8(*ReadFunction)(void*, int16);
		typedef void(*WriteFunction)(void*, int16, int8);

		EmulatorCPU();
		~EmulatorCPU();

		void SetReadFunction(void* ptr, ReadFunction f);
		void SetWriteFunction(void* ptr, WriteFunction f);
		void SetClockDivider(int divider);

		void Reset();
		void NMI(int delayCycles);
		void IRQ(int delayCycles);
		void Execute(int cycles);

	private:
		ReadFunction readFunction;
		void* readFunctionObj;
		WriteFunction writeFunction;
		void* writeFunctionObj;

		int signalNMI;
		int signalIRQ;

		int clockDivider;
		int clocksRemaining;
		int frameCounter, cycleCounter, instructionCounter, frameCycleCounter;

		int opcode, opcodeTiming;
		int16 registerPC;
		int8 registerA, registerX, registerY, registerS, registerP;
		int16 internalAddress;
		int8 internalData;
	};
}

#endif
