/* ScummVM - Scumm Interpreter
 * Copyright (C) 1999/2000 Tatsuyuki Satoh
 * Copyright (C) 2001/2002 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 * LGPL licensed version of MAMEs fmopl (V0.37a modified) by
 * Tatsuyuki Satoh. Included from LGPL'ed AdPlug.
 */

#ifndef __FMOPL_H_
#define __FMOPL_H_
#include "scummsys.h"

typedef int16 OPLSAMPLE;

typedef void (*OPL_TIMERHANDLER)(int channel,double interval_Sec);
typedef void (*OPL_IRQHANDLER)(int param,int irq);
typedef void (*OPL_UPDATEHANDLER)(int param,int min_interval_us);
typedef void (*OPL_PORTHANDLER_W)(int param,unsigned char data);
typedef unsigned char (*OPL_PORTHANDLER_R)(int param);

/* !!!!! here is private section , do not access there member direct !!!!! */

#define OPL_TYPE_WAVESEL   0x01  /* waveform select    */
#define OPL_TYPE_ADPCM     0x02  /* DELTA-T ADPCM unit */
#define OPL_TYPE_KEYBOARD  0x04  /* keyboard interface */
#define OPL_TYPE_IO        0x08  /* I/O port */

/* Saving is necessary for member of the 'R' mark for suspend/resume */
/* ---------- OPL one of slot  ---------- */
typedef struct fm_opl_slot {
	int32 TL;		/* total level     :TL << 8            */
	int32 TLL;		/* adjusted now TL                     */
	uint8  KSR;		/* key scale rate  :(shift down bit)   */
	int32 *AR;		/* attack rate     :&AR_TABLE[AR<<2]   */
	int32 *DR;		/* decay rate      :&DR_TALBE[DR<<2]   */
	int32 SL;		/* sustin level    :SL_TALBE[SL]       */
	int32 *RR;		/* release rate    :&DR_TABLE[RR<<2]   */
	uint8 ksl;		/* keyscale level  :(shift down bits)  */
	uint8 ksr;		/* key scale rate  :kcode>>KSR         */
	uint32 mul;		/* multiple        :ML_TABLE[ML]       */
	uint32 Cnt;		/* frequency count :                   */
	uint32 Incr;	/* frequency step  :                   */
	/* envelope generator state */
	uint8 eg_typ;	/* envelope type flag                  */
	uint8 evm;		/* envelope phase                      */
	int32 evc;		/* envelope counter                    */
	int32 eve;		/* envelope counter end point          */
	int32 evs;		/* envelope counter step               */
	int32 evsa;	/* envelope step for AR :AR[ksr]           */
	int32 evsd;	/* envelope step for DR :DR[ksr]           */
	int32 evsr;	/* envelope step for RR :RR[ksr]           */
	/* LFO */
	uint8 ams;		/* ams flag                            */
	uint8 vib;		/* vibrate flag                        */
	/* wave selector */
	int32 **wavetable;
}OPL_SLOT;

/* ---------- OPL one of channel  ---------- */
typedef struct fm_opl_channel {
	OPL_SLOT SLOT[2];
	uint8 CON;			/* connection type                     */
	uint8 FB;			/* feed back       :(shift down bit)   */
	int32 *connect1;	/* slot1 output pointer                */
	int32 *connect2;	/* slot2 output pointer                */
	int32 op1_out[2];	/* slot1 output for selfeedback        */
	/* phase generator state */
	uint32  block_fnum;	/* block+fnum      :                   */
	uint8 kcode;		/* key code        : KeyScaleCode      */
	uint32  fc;			/* Freq. Increment base                */
	uint32  ksl_base;	/* KeyScaleLevel Base step             */
	uint8 keyon;		/* key on/off flag                     */
} OPL_CH;

/* OPL state */
struct FM_OPL {
	uint8 type;			/* chip type                         */
	int clock;			/* master clock  (Hz)                */
	int rate;			/* sampling rate (Hz)                */
	double freqbase;	/* frequency base                    */
	double TimerBase;	/* Timer base time (==sampling time) */
	uint8 address;		/* address register                  */
	uint8 status;		/* status flag                       */
	uint8 statusmask;	/* status mask                       */
	uint32 mode;		/* Reg.08 : CSM , notesel,etc.       */
	/* Timer */
	int T[2];			/* timer counter                     */
	uint8 st[2];		/* timer enable                      */
	/* FM channel slots */
	OPL_CH *P_CH;		/* pointer of CH                     */
	int	max_ch;			/* maximum channel                   */
	/* Rythm sention */
	uint8 rythm;		/* Rythm mode , key flag */
	/* time tables */
	/* LFO */
	int32 *ams_table;
	int32 *vib_table;
	int32 amsCnt;
	int32 amsIncr;
	int32 vibCnt;
	int32 vibIncr;
	/* wave selector enable flag */
	uint8 wavesel;
	/* external event callback handler */
	OPL_TIMERHANDLER  TimerHandler;		/* TIMER handler   */
	int TimerParam;						/* TIMER parameter */
	OPL_IRQHANDLER    IRQHandler;		/* IRQ handler    */
	int IRQParam;						/* IRQ parameter  */
	OPL_UPDATEHANDLER UpdateHandler;	/* stream update handler   */
	int UpdateParam;					/* stream update parameter */
	int32 AR_TABLE[75];	/* atttack rate tables */
	int32 DR_TABLE[75];	/* decay rate tables   */
	uint32 FN_TABLE[1024];  /* fnumber -> increment counter */
};

/* ---------- Generic interface section ---------- */
#define OPL_TYPE_YM3526 (0)
#define OPL_TYPE_YM3812 (OPL_TYPE_WAVESEL)
#define OPL_TYPE_Y8950  (OPL_TYPE_ADPCM|OPL_TYPE_KEYBOARD|OPL_TYPE_IO)

FM_OPL *OPLCreate(int type, int clock, int rate);
void OPLDestroy(FM_OPL *OPL);
void OPLSetTimerHandler(FM_OPL *OPL,OPL_TIMERHANDLER TimerHandler,int channelOffset);
void OPLSetIRQHandler(FM_OPL *OPL,OPL_IRQHANDLER IRQHandler,int param);
void OPLSetUpdateHandler(FM_OPL *OPL,OPL_UPDATEHANDLER UpdateHandler,int param);

void OPLResetChip(FM_OPL *OPL);
int OPLWrite(FM_OPL *OPL,int a,int v);
unsigned char OPLRead(FM_OPL *OPL,int a);
int OPLTimerOver(FM_OPL *OPL,int c);

void OPLWriteReg(FM_OPL *OPL, int r, int v);

/* YM3626/YM3812 local section */
void YM3812UpdateOne(FM_OPL *OPL, int16 *buffer, int length);

#endif
