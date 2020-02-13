/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef NUVIE_SOUND_ADPLUG_OPL_CLASS_H
#define NUVIE_SOUND_ADPLUG_OPL_CLASS_H

#include "ultima/nuvie/sound/adplug/opl.h"

namespace Ultima {
namespace Nuvie {

#define HAS_YM3812  1

/* --- select emulation chips --- */
#define BUILD_YM3812 (HAS_YM3812)
#define BUILD_YM3526 (HAS_YM3526)
#define BUILD_Y8950  (HAS_Y8950)

/* select output bits size of output : 8 or 16 */
#define OPL_SAMPLE_BITS 16

/* compiler dependence */
#ifndef OSD_CPU_H
#define OSD_CPU_H
typedef unsigned char   uint8;   /* unsigned  8bit */
typedef unsigned short  UINT16;  /* unsigned 16bit */
typedef unsigned int    uint32;  /* unsigned 32bit */
typedef signed char     int8;    /* signed  8bit   */
typedef signed short    int16;   /* signed 16bit   */
typedef signed int      int32;   /* signed 32bit   */
#endif

#if (OPL_SAMPLE_BITS==16)
typedef int16 OPLSAMPLE;
#endif
#if (OPL_SAMPLE_BITS==8)
typedef int8 OPLSAMPLE;
#endif


typedef void (*OPL_TIMERHANDLER)(int channel, double interval_Sec);
typedef void (*OPL_IRQHANDLER)(int param, int irq);
typedef void (*OPL_UPDATEHANDLER)(int param, int min_interval_us);
typedef void (*OPL_PORTHANDLER_W)(int param, unsigned char data);
typedef unsigned char (*OPL_PORTHANDLER_R)(int param);

/* Saving is necessary for member of the 'R' mark for suspend/resume */

typedef struct {
	uint32  ar;         /* attack rate: AR<<2           */
	uint32  dr;         /* decay rate:  DR<<2           */
	uint32  rr;         /* release rate:RR<<2           */
	uint8   KSR;        /* key scale rate               */
	uint8   ksl;        /* keyscale level               */
	uint8   ksr;        /* key scale rate: kcode>>KSR   */
	uint8   mul;        /* multiple: mul_tab[ML]        */

	/* Phase Generator */
	uint32  Cnt;        /* frequency counter            */
	uint32  Incr;       /* frequency counter step       */
	uint8   FB;         /* feedback shift value         */
	int32   *connect1;  /* slot1 output pointer         */
	int32   op1_out[2]; /* slot1 output for feedback    */
	uint8   CON;        /* connection (algorithm) type  */

	/* Envelope Generator */
	uint8   eg_type;    /* percussive/non-percussive mode */
	uint8   state;      /* phase type                   */
	uint32  TL;         /* total level: TL << 2         */
	int32   TLL;        /* adjusted now TL              */
	int32   volume;     /* envelope counter             */
	uint32  sl;         /* sustain level: sl_tab[SL]    */

	uint8   eg_sh_ar;   /* (attack state)               */
	uint8   eg_sel_ar;  /* (attack state)               */
	uint8   eg_sh_dr;   /* (decay state)                */
	uint8   eg_sel_dr;  /* (decay state)                */
	uint8   eg_sh_rr;   /* (release state)              */
	uint8   eg_sel_rr;  /* (release state)              */

	uint32  key;        /* 0 = KEY OFF, >0 = KEY ON     */

	/* LFO */
	uint32  AMmask;     /* LFO Amplitude Modulation enable mask */
	uint8   vib;        /* LFO Phase Modulation enable flag (active high)*/

	/* waveform select */
	unsigned int wavetable;
} OPL_SLOT;

typedef struct {
	OPL_SLOT SLOT[2];
	/* phase generator state */
	uint32  block_fnum; /* block+fnum                   */
	uint32  fc;         /* Freq. Increment base         */
	uint32  ksl_base;   /* KeyScaleLevel Base step      */
	uint8   kcode;      /* key code (for key scaling)   */
} OPL_CH;

/* OPL state */
typedef struct fm_opl_f {
	/* FM channel slots */
	OPL_CH  P_CH[9];                /* OPL/OPL2 chips have 9 channels*/

	uint32  eg_cnt;                 /* global envelope generator counter    */
	uint32  eg_timer;               /* global envelope generator counter works at frequency = chipclock/72 */
	uint32  eg_timer_add;           /* step of eg_timer                     */
	uint32  eg_timer_overflow;      /* envelope generator timer overlfows every 1 sample (on real chip) */

	uint8   rhythm;                 /* Rhythm mode                  */

	uint32  fn_tab[1024];           /* fnumber->increment counter   */

	/* LFO */
	uint8   lfo_am_depth;
	uint8   lfo_pm_depth_range;
	uint32  lfo_am_cnt;
	uint32  lfo_am_inc;
	uint32  lfo_pm_cnt;
	uint32  lfo_pm_inc;

	uint32  noise_rng;              /* 23 bit noise shift register  */
	uint32  noise_p;                /* current noise 'phase'        */
	uint32  noise_f;                /* current noise period         */

	uint8   wavesel;                /* waveform select enable flag  */

	int     T[2];                   /* timer counters               */
	uint8   st[2];                  /* timer enable                 */

	/* external event callback handlers */
	OPL_TIMERHANDLER  TimerHandler; /* TIMER handler                */
	int TimerParam;                 /* TIMER parameter              */
	OPL_IRQHANDLER    IRQHandler;   /* IRQ handler                  */
	int IRQParam;                   /* IRQ parameter                */
	OPL_UPDATEHANDLER UpdateHandler;/* stream update handler        */
	int UpdateParam;                /* stream update parameter      */

	uint8 type;                     /* chip type                    */
	uint8 address;                  /* address register             */
	uint8 status;                   /* status flag                  */
	uint8 statusmask;               /* status mask                  */
	uint8 mode;                     /* Reg.08 : CSM,notesel,etc.    */

	int clock;                      /* master clock  (Hz)           */
	int rate;                       /* sampling rate (Hz)           */
	double freqbase;                /* frequency base               */
	double TimerBase;               /* Timer base time (==sampling time)*/
} FM_OPL;

#define MAX_OPL_CHIPS 2

/* sinwave entries */
#define SIN_BITS        10
#define SIN_LEN         (1<<SIN_BITS)
#define SIN_MASK        (SIN_LEN-1)

#define TL_RES_LEN      (256)   /* 8 bits addressing (real chip) */

/*  TL_TAB_LEN is calculated as:
*   12 - sinus amplitude bits     (Y axis)
*   2  - sinus sign bit           (Y axis)
*   TL_RES_LEN - sinus resolution (X axis)
*/
#define TL_TAB_LEN (12*2*TL_RES_LEN)

class OplClass: public Copl {
private:

	FM_OPL *OPL_YM3812[MAX_OPL_CHIPS];  /* array of pointers to the YM3812's */
	int YM3812NumChips;                 /* number of chips */

	signed int tl_tab[TL_TAB_LEN];

	/* sin waveform table in 'decibel' scale */
	/* four waveforms on OPL2 type chips */
	unsigned int sin_tab[SIN_LEN * 4];

	/* lock level of common table */
	int num_lock;

	/* work table */
	void *cur_chip; /* current chip point */
	OPL_SLOT *SLOT7_1, *SLOT7_2, *SLOT8_1, *SLOT8_2;

	signed int phase_modulation;        /* phase modulation input (SLOT 2) */
	signed int output[1];

	uint32  LFO_AM;
	int32   LFO_PM;

	bool    use16bit, stereo;
	int oplRate;

public:
	OplClass(int rate, bool bit16, bool usestereo); // rate = sample rate
	~OplClass() override {
		YM3812Shutdown();
	}

	int getRate() {
		return oplRate;
	}

	void update(short *buf, int samples);   // fill buffer

	// template methods
	void write(int reg, int val) override;
	void init() override;

private:
	int  YM3812Init(int num, int clock, int rate);
	void YM3812Shutdown(void);
	void YM3812ResetChip(int which);
	int  YM3812Write(int which, int a, int v);
	unsigned char YM3812Read(int which, int a);
	int  YM3812TimerOver(int which, int c);
	void YM3812UpdateOne(int which, int16 *buffer, int length);

	void YM3812SetTimerHandler(int which, OPL_TIMERHANDLER TimerHandler, int channelOffset);
	void YM3812SetIRQHandler(int which, OPL_IRQHANDLER IRQHandler, int param);
	void YM3812SetUpdateHandler(int which, OPL_UPDATEHANDLER UpdateHandler, int param);

	int init_tables(void);
	void OPLWriteReg(FM_OPL *OPL, int r, int v);
	void OPLResetChip(FM_OPL *OPL);
	int OPL_LockTable(void);
	FM_OPL *OPLCreate(int type, int clock, int rate);
	void OPL_UnLockTable(void);
	void OPLDestroy(FM_OPL *OPL);
	int OPLWrite(FM_OPL *OPL, int a, int v);

	inline void advance_lfo(FM_OPL *OPL);
	inline void advancex(FM_OPL *OPL);
	inline signed int op_calc(uint32 phase, unsigned int env, signed int pm, unsigned int wave_tab);
	inline signed int op_calc1(uint32 phase, unsigned int env, signed int pm, unsigned int wave_tab);
	inline void OPL_CALC_CH(OPL_CH *CH);
	inline void OPL_CALC_RH(OPL_CH *CH, unsigned int noise);
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
