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

#ifndef __FMOPL1_H_
#define __FMOPL1_H_

#include<stdlib.h>
#include "opl.h"

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
typedef unsigned char   UINT8;   /* unsigned  8bit */
typedef unsigned short  UINT16;  /* unsigned 16bit */
typedef unsigned int    UINT32;  /* unsigned 32bit */
typedef signed char     INT8;    /* signed  8bit   */
typedef signed short    INT16;   /* signed 16bit   */
typedef signed int      INT32;   /* signed 32bit   */
#endif

#if (OPL_SAMPLE_BITS==16)
typedef INT16 OPLSAMPLE;
#endif
#if (OPL_SAMPLE_BITS==8)
typedef INT8 OPLSAMPLE;
#endif


typedef void (*OPL_TIMERHANDLER)(int channel, double interval_Sec);
typedef void (*OPL_IRQHANDLER)(int param, int irq);
typedef void (*OPL_UPDATEHANDLER)(int param, int min_interval_us);
typedef void (*OPL_PORTHANDLER_W)(int param, unsigned char data);
typedef unsigned char (*OPL_PORTHANDLER_R)(int param);

/* Saving is necessary for member of the 'R' mark for suspend/resume */

typedef struct {
	UINT32  ar;         /* attack rate: AR<<2           */
	UINT32  dr;         /* decay rate:  DR<<2           */
	UINT32  rr;         /* release rate:RR<<2           */
	UINT8   KSR;        /* key scale rate               */
	UINT8   ksl;        /* keyscale level               */
	UINT8   ksr;        /* key scale rate: kcode>>KSR   */
	UINT8   mul;        /* multiple: mul_tab[ML]        */

	/* Phase Generator */
	UINT32  Cnt;        /* frequency counter            */
	UINT32  Incr;       /* frequency counter step       */
	UINT8   FB;         /* feedback shift value         */
	INT32   *connect1;  /* slot1 output pointer         */
	INT32   op1_out[2]; /* slot1 output for feedback    */
	UINT8   CON;        /* connection (algorithm) type  */

	/* Envelope Generator */
	UINT8   eg_type;    /* percussive/non-percussive mode */
	UINT8   state;      /* phase type                   */
	UINT32  TL;         /* total level: TL << 2         */
	INT32   TLL;        /* adjusted now TL              */
	INT32   volume;     /* envelope counter             */
	UINT32  sl;         /* sustain level: sl_tab[SL]    */

	UINT8   eg_sh_ar;   /* (attack state)               */
	UINT8   eg_sel_ar;  /* (attack state)               */
	UINT8   eg_sh_dr;   /* (decay state)                */
	UINT8   eg_sel_dr;  /* (decay state)                */
	UINT8   eg_sh_rr;   /* (release state)              */
	UINT8   eg_sel_rr;  /* (release state)              */

	UINT32  key;        /* 0 = KEY OFF, >0 = KEY ON     */

	/* LFO */
	UINT32  AMmask;     /* LFO Amplitude Modulation enable mask */
	UINT8   vib;        /* LFO Phase Modulation enable flag (active high)*/

	/* waveform select */
	unsigned int wavetable;
} OPL_SLOT;

typedef struct {
	OPL_SLOT SLOT[2];
	/* phase generator state */
	UINT32  block_fnum; /* block+fnum                   */
	UINT32  fc;         /* Freq. Increment base         */
	UINT32  ksl_base;   /* KeyScaleLevel Base step      */
	UINT8   kcode;      /* key code (for key scaling)   */
} OPL_CH;

/* OPL state */
typedef struct fm_opl_f {
	/* FM channel slots */
	OPL_CH  P_CH[9];                /* OPL/OPL2 chips have 9 channels*/

	UINT32  eg_cnt;                 /* global envelope generator counter    */
	UINT32  eg_timer;               /* global envelope generator counter works at frequency = chipclock/72 */
	UINT32  eg_timer_add;           /* step of eg_timer                     */
	UINT32  eg_timer_overflow;      /* envelope generator timer overlfows every 1 sample (on real chip) */

	UINT8   rhythm;                 /* Rhythm mode                  */

	UINT32  fn_tab[1024];           /* fnumber->increment counter   */

	/* LFO */
	UINT8   lfo_am_depth;
	UINT8   lfo_pm_depth_range;
	UINT32  lfo_am_cnt;
	UINT32  lfo_am_inc;
	UINT32  lfo_pm_cnt;
	UINT32  lfo_pm_inc;

	UINT32  noise_rng;              /* 23 bit noise shift register  */
	UINT32  noise_p;                /* current noise 'phase'        */
	UINT32  noise_f;                /* current noise period         */

	UINT8   wavesel;                /* waveform select enable flag  */

	int     T[2];                   /* timer counters               */
	UINT8   st[2];                  /* timer enable                 */

	/* external event callback handlers */
	OPL_TIMERHANDLER  TimerHandler; /* TIMER handler                */
	int TimerParam;                 /* TIMER parameter              */
	OPL_IRQHANDLER    IRQHandler;   /* IRQ handler                  */
	int IRQParam;                   /* IRQ parameter                */
	OPL_UPDATEHANDLER UpdateHandler;/* stream update handler        */
	int UpdateParam;                /* stream update parameter      */

	UINT8 type;                     /* chip type                    */
	UINT8 address;                  /* address register             */
	UINT8 status;                   /* status flag                  */
	UINT8 statusmask;               /* status mask                  */
	UINT8 mode;                     /* Reg.08 : CSM,notesel,etc.    */

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

	UINT32  LFO_AM;
	INT32   LFO_PM;

	bool    use16bit, stereo;
	int oplRate;

public:
	OplClass(int rate, bool bit16, bool usestereo); // rate = sample rate
	~OplClass() {
		YM3812Shutdown();
	}

	int getRate() {
		return oplRate;
	}

	void update(short *buf, int samples);   // fill buffer

	// template methods
	void write(int reg, int val);
	void init();

private:
	int  YM3812Init(int num, int clock, int rate);
	void YM3812Shutdown(void);
	void YM3812ResetChip(int which);
	int  YM3812Write(int which, int a, int v);
	unsigned char YM3812Read(int which, int a);
	int  YM3812TimerOver(int which, int c);
	void YM3812UpdateOne(int which, INT16 *buffer, int length);

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
	inline signed int op_calc(UINT32 phase, unsigned int env, signed int pm, unsigned int wave_tab);
	inline signed int op_calc1(UINT32 phase, unsigned int env, signed int pm, unsigned int wave_tab);
	inline void OPL_CALC_CH(OPL_CH *CH);
	inline void OPL_CALC_RH(OPL_CH *CH, unsigned int noise);
};


#endif
