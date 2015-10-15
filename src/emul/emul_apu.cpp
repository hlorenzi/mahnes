#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "emul.h"

namespace MahNES
{
   void EmulatorAPU::SetOutputFunction(void* ptr, OutputFunction f)
   {
		outputFunction = f;
		outputFunctionObj = ptr;
   }

	int EmulatorAPU::Init(int samppersec, int buffersize)
	{
		int ret = 1;
		samples_per_sec = samppersec;
		buffer_size     = buffersize;
		buffer_count    = 0;

		clock_rate = 1789773;//2.727;

		if (true) {
			blip_data = blip_new(buffer_size * 4);
			if (blip_data == NULL) {
				ret = -1;
			}

			blip_set_rates(blip_data,clock_rate,samples_per_sec);

			sample = (short*)calloc(sizeof(short), buffer_size);
			if (sample == NULL) {
				ret = -2;
			}
		}

		return ret;
	}

	void EmulatorAPU::Reset()
	{

		reg_control = reg_mode = 0;
		reg_sqr1_control  = reg_sqr1_sweep   = reg_sqr1_notelow  = reg_sqr1_notehigh = 0;
		reg_sqr2_control  = reg_sqr2_sweep   = reg_sqr2_notelow  = reg_sqr2_notehigh = 0;
		reg_tri_control   = reg_tri_notelow  = reg_tri_notehigh  = 0;
		reg_noise_control = reg_noise_period = reg_noise_notehigh = 0;

		sqr1_silence_sweep = sqr1_silence_length = sqr1_silence_envelope = 0;
		sqr2_silence_sweep = sqr2_silence_length = sqr2_silence_envelope = 0;
		noise_silence_length = noise_silence_envelope = 0;
		sqr1_volume = sqr2_volume = noise_volume = 0;

		reg_sqr1_counter  = reg_sqr1_sweepdelay   = reg_sqr1_length = reg_sqr1_envelope = reg_sqr1_envelopedelay = 0;
		reg_sqr2_counter  = reg_sqr2_sweepdelay   = reg_sqr2_length = reg_sqr2_envelope = reg_sqr2_envelopedelay = 0;
		reg_tri_haltflag  = reg_tri_linearcounter = reg_tri_length  = 0;
		reg_noise_counter = reg_noise_envelope    = reg_noise_envelopedelay = reg_noise_length = 0;

		counter_irq = counter_length = counter_envelope = 0;
		cycles_irq  = cycles_length = cycles_envelope = 0;
		cycles_sqr1 = cycles_sqr2   = cycles_tri = cycles_noise = 0;

		sqr1_phase = sqr2_phase = tri_phase = noise_phase = 1;
		sqr1_amp   = sqr2_amp   = tri_amp   = noise_amp   = 0;

		timer = 0;
		highrestimer = 0;
	}

	int apu_length[] = {0x0A, 0xFE, 0x14, 0x02, 0x28, 0x04, 0x50, 0x06, 0xA0, 0x08, 0x3C, 0x0A, 0x0E, 0x0C, 0x1A, 0x0E, 0x0C, 0x10, 0x18, 0x12, 0x30, 0x14, 0x60, 0x16, 0xC0, 0x18, 0x48, 0x1A, 0x10, 0x1C, 0x20, 0x1E};
	int apu_length_noise[] = {4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068};

	void EmulatorAPU::WriteRegister(int16 address, int8 value) {
		//emul_run_apu(rom->update_apu,0);
		//rom->update_apu = 0;

		switch(address) {
			case 0x4000:	reg_sqr1_control    = value; break;
			case 0x4001:	reg_sqr1_sweep      = value; break;
			case 0x4002:	reg_sqr1_notelow    = value; break;
			case 0x4003:	reg_sqr1_notehigh   = value; reg_sqr1_envelope=0xF; reg_sqr1_envelopedelay=reg_sqr1_control&0xF; reg_sqr1_length=apu_length[value>>3]; break;

			case 0x4004:	reg_sqr2_control    = value; break;
			case 0x4005:	reg_sqr2_sweep      = value; break;
			case 0x4006:	reg_sqr2_notelow    = value; break;
			case 0x4007:	reg_sqr2_notehigh   = value; reg_sqr2_envelope=0xF; reg_sqr2_envelopedelay=reg_sqr2_control&0xF; reg_sqr2_length=apu_length[value>>3]; break;

			case 0x4008:	reg_tri_control     = value; break;
			case 0x4009:	break; //reg_tri_sweep       = value; break;
			case 0x400A:	reg_tri_notelow     = value; break;
			case 0x400B:	reg_tri_notehigh    = value; reg_tri_haltflag = 1; reg_tri_length = apu_length[value >> 3]; break;

			case 0x400C:	reg_noise_control   = value; break;
			case 0x400D:	break;
			case 0x400E:	reg_noise_period    = value; break;
			case 0x400F:	reg_noise_notehigh  = value; reg_noise_length = apu_length[value >> 3]; break;

			case 0x4015:	{
				reg_control = value;
				if (!(reg_control & 0x1)) reg_sqr1_length  = 0;
				if (!(reg_control & 0x2)) reg_sqr2_length  = 0;
				if (!(reg_control & 0x4)) reg_tri_length   = 0;
				if (!(reg_control & 0x8)) reg_noise_length = 0;
				break;
			}
			case 0x4017:    reg_mode = value; break;
		}
	}

	int8 EmulatorAPU::ReadRegister(int16 address) {
		switch(address) {
			case 0x4015: { // APU_STATUS
				return (reg_sqr1_length > 0) | ((reg_sqr2_length > 0) << 1) | ((reg_tri_length > 0) << 2) | ((reg_noise_length > 0) << 3);
			}
		}

		return 0;
	}

	inline void add_delta(blip_t* b, int time, int* cur_amp, int new_amp) {
		int delta = new_amp - *cur_amp;
		*cur_amp += delta;
		blip_add_delta_fast(b,time,delta);
	}

	int apu_lengthframe[2][5]   = {{0,1,0,1,0}, {0,1,0,0,1}};
	int apu_envelopeframe[2][5] = {{1,1,1,1,0}, {1,1,1,0,1}};
	int apu_irqframe[2][5]      = {{0,0,0,1,0}, {0,0,0,0,0}};

	void EmulatorAPU::Execute(float cycles) {
		timer += cycles;

		for( ; cycles_irq < timer; cycles_irq += 29781 / 4) {
			int dotimer;
			if (reg_mode & 0x80) {
				counter_irq = (counter_irq + 1) % 5;
				dotimer     = apu_irqframe[1][counter_irq];
			} else {
				counter_irq = (counter_irq + 1) % 4;
				dotimer     = apu_irqframe[0][counter_irq];
			}

			if (dotimer) {
				//if (!(reg_mode & 0x40) && !(rom->regP & I_FLAG)) rom->interrupt_irq = 1;
			}
		}

		for( ; cycles_length < timer;   cycles_length += 29781 / 4) {
			int dotimer;
			if (reg_mode & 0x80) {
				counter_length = (counter_length + 1) % 5;
				dotimer        = apu_lengthframe[1][counter_length];
			} else {
				counter_length = (counter_length + 1) % 4;
				dotimer        = apu_lengthframe[0][counter_length];
			}

			if (!dotimer) continue;

			if (!(reg_sqr1_control&0x20)) {
				if (reg_sqr1_length > 0) {
					reg_sqr1_length--;
				}
			} else {
				reg_sqr1_length = 0;
			}

			if (--reg_sqr1_sweepdelay<=0 && reg_sqr1_sweep&0x80 && reg_sqr1_sweep&0x7) {
				reg_sqr1_sweepdelay = ((reg_sqr1_sweep >> 4)&0x7)+1;
				int freq = ((reg_sqr1_notehigh&0x07)<<8)|reg_sqr1_notelow;
				int target = (freq>>(reg_sqr1_sweep&0x07));

				if (target>0x7FF || freq<8) {
					sqr1_silence_sweep = 1;
				} else {
					sqr1_silence_sweep = 0;
					if (reg_sqr1_sweep&0x08) {
						freq-=target;
					} else {
						freq+=target;
					}
					reg_sqr1_notelow = freq & 0xFF;
					reg_sqr1_notehigh &= ~0x7;
					reg_sqr1_notehigh |= (freq>>8)&0x7;
				}
			} else {
				sqr1_silence_sweep = 0;
			}

			if (!(reg_sqr2_control&0x20)) {
				if (reg_sqr2_length > 0) {
					reg_sqr2_length--;
				}
			} else {
				reg_sqr2_length = 0;
			}

			if (--reg_sqr2_sweepdelay<=0 && reg_sqr2_sweep&0x80 && reg_sqr2_sweep&0x7) {
				reg_sqr2_sweepdelay = ((reg_sqr2_sweep >> 4)&0x7)+1;
				int freq = ((reg_sqr2_notehigh&0x07)<<8)|reg_sqr2_notelow;
				int target = (freq>>(reg_sqr2_sweep&0x07));

				if (target>0x7FF || freq<8) {
					sqr2_silence_sweep = 1;
				} else {
					sqr2_silence_sweep = 0;
					if (reg_sqr2_sweep&0x08) {
						freq-=target+1;
					} else {
						freq+=target;
					}
					reg_sqr2_notelow = freq & 0xFF;
					reg_sqr2_notehigh &= ~0x7;
					reg_sqr2_notehigh |= (freq>>8)&0x7;
				}
			} else {
				sqr2_silence_sweep = 0;
			}

			if (!reg_tri_haltflag) {
				if (reg_tri_length > 0) {
					reg_tri_length--;
				}
			} else {
				//reg_tri_length = 0;
			}

			if (!(reg_noise_control & 0x20)) {
				if (reg_noise_length > 0) {
					reg_noise_length--;
				}
			} else {
				reg_noise_length = 0;
			}
		}

		for( ; cycles_envelope < timer; cycles_envelope += 29781 / 4) {
			int dotimer;
			if (reg_mode & 0x80) {
				counter_envelope = (counter_envelope + 1) % 5;
				dotimer          = apu_envelopeframe[1][counter_envelope];
			} else {
				counter_envelope = (counter_envelope + 1) % 4;
				dotimer          = apu_envelopeframe[0][counter_envelope];
			}

			if (!dotimer) continue;

			if (!(reg_sqr1_control & 0x10)) {
				if (--reg_sqr1_envelopedelay < 0) {
					reg_sqr1_envelopedelay = (reg_sqr1_control & 0xF) + 1;
					if (reg_sqr1_envelope <= 0) {
						if (reg_sqr1_control & 0x20) {
							reg_sqr1_envelope = 0xF;
						} else {
							reg_sqr1_envelope = 0;
						}
					} else {
						reg_sqr1_envelope--;
					}
				}
				sqr1_volume = reg_sqr1_envelope;

				if (sqr1_volume == 0) sqr1_silence_envelope = 1;
				else sqr1_silence_envelope = 0;
			} else {
				sqr1_volume   = (reg_sqr1_control & 0xF);
				sqr1_silence_envelope = 0;
			}

			if (!(reg_sqr2_control & 0x10)) {
				if (--reg_sqr2_envelopedelay < 0) {
					reg_sqr2_envelopedelay = (reg_sqr2_control & 0xF) + 1;
					if (reg_sqr2_envelope <= 0) {
						if (reg_sqr2_control & 0x20) {
							reg_sqr2_envelope = 0xF;
						} else {
							reg_sqr2_envelope = 0;
						}
					} else {
						reg_sqr2_envelope--;
					}
				}
				sqr2_volume = reg_sqr2_envelope;
				if (sqr2_volume == 0) sqr2_silence_envelope = 1;
				else sqr2_silence_envelope = 0;
			} else {
				sqr2_volume   = (reg_sqr2_control & 0xF);
				sqr2_silence_envelope = 0;
			}

			if (reg_tri_haltflag) {
				reg_tri_linearcounter = reg_tri_control & 0x7F;
			} else if (reg_tri_linearcounter > 0) {
				reg_tri_linearcounter--;
			}
			reg_tri_haltflag = (reg_tri_control & 0x80);

			if (!(reg_noise_control & 0x10)) {
				if (--reg_noise_envelopedelay < 0) {
					reg_noise_envelopedelay = (reg_noise_control & 0xF) + 1;
					if (reg_noise_envelope <= 0) {
						if (reg_noise_control & 0x20) {
							reg_noise_envelope = 0xF;
						} else {
							reg_noise_envelope = 0;
						}
					} else {
						reg_noise_envelope--;
					}
				}
				noise_volume = reg_noise_envelope;
				if (noise_volume==0) noise_silence_envelope = 1;
				else noise_silence_envelope = 0;
			} else {
				noise_volume = (reg_noise_control & 0xF);;
				noise_silence_envelope = 0;
			}
		}

		int sqr1_period  = (((reg_sqr1_notehigh & 0x7) << 8) | reg_sqr1_notelow) + 1;
		int sqr2_period  = (((reg_sqr2_notehigh & 0x7) << 8) | reg_sqr2_notelow) + 1;
		int tri_period   = (((reg_tri_notehigh  & 0x7) << 8) | reg_tri_notelow)  + 1;
		int noise_period = apu_length_noise[reg_noise_period & 0xF];

		int sqr1silence  = (!(reg_sqr1_control & 0x10) && sqr1_silence_envelope) || (!(reg_sqr1_control & 0x20) && reg_sqr1_length <= 0) || ((reg_sqr1_control & 0x80) && sqr1_silence_sweep);
		int sqr2silence  = (!(reg_sqr2_control & 0x10) && sqr2_silence_envelope) || (!(reg_sqr2_control & 0x20) && reg_sqr2_length <= 0) || ((reg_sqr2_control & 0x80) && sqr2_silence_sweep);
		int trisilence   = (tri_period < 10) || (reg_tri_length <= 0) || (reg_tri_linearcounter <= 0);
		int noisesilence = (!(reg_noise_control & 0x10) && noise_silence_envelope) || (!(reg_noise_control & 0x20) && reg_noise_length <= 0);

		if (!sqr1silence  && (reg_control & 0x1)) { // SQR1 ENABLE
			int sqr1_duty = 1;
			switch((reg_sqr1_control & 0xC0) >> 6) {
				case 0x0:   // 12.5% Duty Cycle
				case 0x3: { // 12.5% Duty Cycle Negated
					sqr1_duty = 1; break;
				}
				case 0x1: { // 25% Duty Cycle
					sqr1_duty = 2; break;
				}
				case 0x2: { // 50% Duty Cycle
					sqr1_duty = 4; break;
				}
			}

			for( ; cycles_sqr1 < timer; cycles_sqr1 += sqr1_period << 1) {
				sqr1_phase = (sqr1_phase + 1) & 0x7;
				add_delta(blip_data, cycles_sqr1, &sqr1_amp, (sqr1_phase < sqr1_duty) ? 0 : 1000 * sqr1_volume);
			}
		} else {
			for( ; cycles_sqr1 < timer; cycles_sqr1 = timer + 1) {
				add_delta(blip_data, cycles_sqr1, &sqr1_amp, 0);
			}
		}

		if (!sqr2silence  && (reg_control & 0x2)) { // SQR2 ENABLE
			int sqr2_duty = 1;
			switch((reg_sqr2_control & 0xC0) >> 6) {
				case 0x0:   // 12.5% Duty Cycle
				case 0x3: { // 12.5% Duty Cycle Negated
					sqr2_duty = 1; break;
				}
				case 0x1: { // 25% Duty Cycle
					sqr2_duty = 2; break;
				}
				case 0x2: { // 50% Duty Cycle
					sqr2_duty = 4; break;
				}
			}

			for( ; cycles_sqr2 < timer; cycles_sqr2 += sqr2_period << 1) {
				sqr2_phase = (sqr2_phase + 1) & 0x7;
				add_delta(blip_data, cycles_sqr2, &sqr2_amp, (sqr2_phase < sqr2_duty) ? 0 : 1000 * sqr2_volume);
			}
		} else {
			for( ; cycles_sqr2 < timer; cycles_sqr2 = timer + 1) {
				add_delta(blip_data, cycles_sqr2, &sqr2_amp, 0);
			}
		}

		if (!trisilence   && (reg_control & 0x4)) { // TRI ENABLE
			for( ; cycles_tri < timer; cycles_tri += tri_period) {
				tri_phase = (tri_phase + 1) & 0x1F;
				add_delta(blip_data, cycles_tri, &tri_amp, (tri_phase < 16) ? tri_phase << 10 : (31 - tri_phase) << 10);
			}
		} else {
			cycles_tri = timer;
		}

		if (!noisesilence && (reg_control & 0x8)) { // NOISE ENABLE
			for( ; cycles_noise < timer; cycles_noise += noise_period) {
				int feedback;
				if (reg_noise_period & 0x80) {
					feedback = (noise_phase & 0x1) ^ ((noise_phase & 0x40) >> 6);
				} else {
					feedback = (noise_phase & 0x1) ^ ((noise_phase & 0x02) >> 1);
				}
				noise_phase = ((noise_phase >> 1) & 0x3FFF) | (feedback << 14);
				add_delta(blip_data, cycles_noise, &noise_amp, (noise_phase & 0x1) ? 0 : 1000 * noise_volume);
			}
		} else {
			cycles_noise = timer;
		}

		cycles_total+=cycles;
		int clocks = blip_clocks_needed(blip_data, buffer_size);

		if (timer >= clocks) {
			blip_end_frame(blip_data,timer);

			timer           -= clocks;
			cycles_sqr1     -= clocks;
			cycles_sqr2     -= clocks;
			cycles_tri      -= clocks;
			cycles_noise    -= clocks;
			cycles_length   -= clocks;
			cycles_envelope -= clocks;
			cycles_irq      -= clocks;

			while (blip_samples_avail(blip_data) >= buffer_size) {
				int readn = blip_read_samples(blip_data,sample,buffer_size,0);

				outputFunction(outputFunctionObj, sample, readn, samples_per_sec);
			}
		}
	}
}
