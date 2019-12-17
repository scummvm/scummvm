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

#ifndef ULTIMA8_AUDIO_MIDI_FMOPL_H
#define ULTIMA8_AUDIO_MIDI_FMOPL_H

#ifdef USE_FMOPL_MIDI

namespace Ultima {
namespace Ultima8 {
namespace FMOpl_Pentagram {

typedef void (*OPL_TIMERHANDLER)(int channel, double interval_Sec);
typedef void (*OPL_IRQHANDLER)(int param, int irq);
typedef void (*OPL_UPDATEHANDLER)(int param, int min_interval_us);

#define OPL_TYPE_WAVESEL   0x01  /* waveform select    */

// Modulation Registers
#define INDEX_AVEKM_M   0
#define INDEX_KSLTL_M   2
#define INDEX_AD_M      4
#define INDEX_SR_M      6
#define INDEX_WAVE_M    8

// Carrier Registers
#define INDEX_AVEKM_C   1
#define INDEX_KSLTL_C   3
#define INDEX_AD_C      5
#define INDEX_SR_C      7
#define INDEX_WAVE_C    9

#define INDEX_FB_C      10
#define INDEX_PERC      11

#define CHP_CHAN        0
#define CHP_NOTE        1
#define CHP_COUNTER     2
#define CHP_VEL         3

/* Saving is necessary for member of the 'R' mark for suspend/resume */
/* ---------- OPL one of slot  ---------- */
typedef struct fm_opl_slot {
	int TL;     /* total level     :TL << 8             */
	int TLL;    /* adjusted now TL                      */
	uint8 KSR;  /* key scale rate  :(shift down bit)    */
	int *AR;    /* attack rate     :&AR_TABLE[AR<<2]    */
	int *DR;    /* decay rate      :&DR_TABLE[DR<<2]    */
	int SL;     /* sustain level   :SL_TABLE[SL]        */
	int *RR;    /* release rate    :&DR_TABLE[RR<<2]    */
	uint8 ksl;  /* keyscale level  :(shift down bits)   */
	uint8 ksr;  /* key scale rate  :kcode>>KSR          */
	uint32 mul; /* multiple        :ML_TABLE[ML]        */
	uint32 Cnt; /* frequency count                      */
	uint32 Incr;    /* frequency step                       */

	/* envelope generator state */
	uint8 eg_typ;/* envelope type flag                  */
	uint8 evm;  /* envelope phase                       */
	int evc;    /* envelope counter                     */
	int eve;    /* envelope counter end point           */
	int evs;    /* envelope counter step                */
	int evsa;   /* envelope step for AR :AR[ksr]        */
	int evsd;   /* envelope step for DR :DR[ksr]        */
	int evsr;   /* envelope step for RR :RR[ksr]        */

	/* LFO */
	uint8 ams;      /* ams flag                            */
	uint8 vib;      /* vibrate flag                        */
	/* wave selector */
	int **wavetable;
} OPL_SLOT;

/* ---------- OPL one of channel  ---------- */
typedef struct fm_opl_channel {
	OPL_SLOT SLOT[2];
	uint8 CON;          /* connection type                  */
	uint8 FB;           /* feed back       :(shift down bit)*/
	int *connect1;      /* slot1 output pointer             */
	int *connect2;      /* slot2 output pointer             */
	int op1_out[2];     /* slot1 output for selfeedback     */

	/* phase generator state */
	uint32  block_fnum; /* block+fnum                       */
	uint8 kcode;        /* key code        : KeyScaleCode   */
	uint32  fc;         /* Freq. Increment base             */
	uint32  ksl_base;       /* KeyScaleLevel Base step          */
	uint8 keyon;        /* key on/off flag                  */
	uint8 PAN;          /* pan                              */
} OPL_CH;

/* OPL state */
typedef struct fm_opl_f {
	uint8 type;         /* chip type                         */
	int clock;          /* master clock  (Hz)                */
	int rate;           /* sampling rate (Hz)                */
	double freqbase;    /* frequency base                    */
	double TimerBase;   /* Timer base time (==sampling time) */
	uint8 address;      /* address register                  */
	uint8 status;       /* status flag                       */
	uint8 statusmask;   /* status mask                       */
	uint32 mode;            /* Reg.08 : CSM , notesel,etc.       */

	/* Timer */
	int T[2];           /* timer counter                     */
	uint8 st[2];        /* timer enable                      */

	/* FM channel slots */
	OPL_CH *P_CH;       /* pointer of CH                     */
	int max_ch;         /* maximum channel                   */

	/* Rythm sention */
	uint8 rythm;        /* Rythm mode , key flag */

	/* time tables */
	int AR_TABLE[75];   /* atttack rate tables              */
	int DR_TABLE[75];   /* decay rate tables                */
	uint32 FN_TABLE[1024];/* fnumber -> increment counter       */

	/* LFO */
	int *ams_table;
	int *vib_table;
	int amsCnt;
	int amsIncr;
	int vibCnt;
	int vibIncr;

	/* wave selector enable flag */
	uint8 wavesel;

	/* external event callback handler */
	OPL_TIMERHANDLER  TimerHandler;     /* TIMER handler   */
	int TimerParam;                     /* TIMER parameter */
	OPL_IRQHANDLER    IRQHandler;       /* IRQ handler    */
	int IRQParam;                       /* IRQ parameter  */
	OPL_UPDATEHANDLER UpdateHandler;    /* stream update handler   */
	int UpdateParam;                    /* stream update parameter */
} FM_OPL;

/* ---------- Generic interface section ---------- */
#define OPL_TYPE_YM3526 (0)
#define OPL_TYPE_YM3812 (OPL_TYPE_WAVESEL)

// Modulation Registers
#define OPL_REG_AVEKM_M     0x20
#define OPL_REG_KSLTL_M     0x40
#define OPL_REG_AD_M        0x60
#define OPL_REG_SR_M        0x80
#define OPL_REG_WAVE_M      0xE0

// Carrier Registers
#define OPL_REG_AVEKM_C     0x23
#define OPL_REG_KSLTL_C     0x43
#define OPL_REG_AD_C        0x63
#define OPL_REG_SR_C        0x83
#define OPL_REG_WAVE_C      0xE3

#define OPL_REG_FB_C        0xC0


FM_OPL *OPLCreate(int type, int clock, int rate);
void OPLDestroy(FM_OPL *OPL);
void OPLSetTimerHandler(FM_OPL *OPL, OPL_TIMERHANDLER TimerHandler, int channelOffset);
void OPLSetIRQHandler(FM_OPL *OPL, OPL_IRQHANDLER IRQHandler, int param);
void OPLSetUpdateHandler(FM_OPL *OPL, OPL_UPDATEHANDLER UpdateHandler, int param);

void OPLResetChip(FM_OPL *OPL);
int OPLWrite(FM_OPL *OPL, int a, int v);
unsigned char OPLRead(FM_OPL *OPL, int a);
int OPLTimerOver(FM_OPL *OPL, int c);
void OPLWriteReg(FM_OPL *OPL, int r, int v);
void OPLSetPan(FM_OPL *OPL, int c, int pan);

void YM3812UpdateOne_Mono(FM_OPL *OPL, int16 *buffer, int length);
void YM3812UpdateOne_Stereo(FM_OPL *OPL, int16 *buffer, int length);

} // End of namespace FMOpl_Pentagram
} // End of namespace Ultima8
} // End of namespace Ultima

#endif

#endif
