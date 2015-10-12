#ifndef EMUL_APU_H
#define EMUL_APU_H

#include "blip_buf.h"
#include "emul_types.h"

namespace MahNES
{
	class EmulatorAPU
	{
	private:
		  int8  reg_control, reg_mode;
		  int8  reg_sqr1_control, reg_sqr1_sweep, reg_sqr1_notelow, reg_sqr1_notehigh;
		  int8  reg_sqr2_control, reg_sqr2_sweep, reg_sqr2_notelow, reg_sqr2_notehigh;
		  int8  reg_tri_control, reg_tri_notelow, reg_tri_notehigh;
		  int8  reg_noise_control, reg_noise_period, reg_noise_notehigh;

		   int  sqr1_silence_sweep, sqr1_silence_length, sqr1_silence_envelope;
		   int  sqr2_silence_sweep, sqr2_silence_length, sqr2_silence_envelope;
		   int  noise_silence_length, noise_silence_envelope;
		   int  sqr1_volume, sqr2_volume, noise_volume;

		   int  reg_sqr1_counter, reg_sqr1_sweepdelay, reg_sqr1_length, reg_sqr1_envelope, reg_sqr1_envelopedelay, reg_sqr2_counter, reg_sqr2_sweepdelay, reg_sqr2_length, reg_sqr2_envelope, reg_sqr2_envelopedelay;
		   int  reg_tri_haltflag, reg_tri_linearcounter, reg_tri_length;
		   int  reg_noise_counter, reg_noise_envelope, reg_noise_envelopedelay, reg_noise_length;

		 float  highrestimer;
		 float  timer, cycles_total;
		   int  counter_irq, counter_length, counter_envelope;
		   int  cycles_irq, cycles_length, cycles_envelope;
		   int  cycles_sqr1, cycles_sqr2, cycles_tri, cycles_noise;
		   int  sqr1_phase, sqr1_amp;
		   int  sqr2_phase, sqr2_amp;
		   int  tri_phase,  tri_amp;
		   int  noise_phase, noise_amp;

		   int  produce_sound;

		   int  samples_per_sec;
		   int  num_buffers;
		   int  buffer_size;
		   int  buffer_count;
		   int  clock_rate;

		blip_t* blip_data;
		 short* sample;

	public:
		 typedef void(*OutputFunction)(void*, short*, int, int);

		   int  Init(int samppersec, int buffersize);
		   void Reset();

		   void SetOutputFunction(void* ptr, OutputFunction f);
		  void  WriteRegister(int16 address, int8 value);
		  int8  ReadRegister(int16 address);
		  void  Execute(float cycles);

		  OutputFunction outputFunction;
		  void* outputFunctionObj;
	};
}

#endif
