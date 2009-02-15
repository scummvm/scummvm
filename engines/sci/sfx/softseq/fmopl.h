/***************************************************************************
 fmopl.h Copyright (C) 1999-2000 Tatsuyuki Satoh
 		       2001-2004 The ScummVM project
 		       2002 Solomon Peachy
 		       2004 Walter van Niftrik

 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.

 LGPL licensed version of MAMEs fmopl (V0.37a modified) by
 Tatsuyuki Satoh. Included from LGPL'ed AdPlug.

***************************************************************************/

#ifndef FMOPL_H_
#define FMOPL_H_

#include "sci/include/scitypes.h"

#define TWELVE_VOICE 1
#ifdef TWELVE_VOICE
#define ADLIB_VOICES 12
#else
#define ADLIB_VOICES 9
#endif

enum {
	FMOPL_ENV_BITS_HQ = 16,
	FMOPL_ENV_BITS_LQ = 8,
	FMOPL_EG_ENT_HQ = 4096,
	FMOPL_EG_ENT_LQ = 128
};


typedef void (*OPL_TIMERHANDLER)(int channel, double interval_Sec);
typedef void (*OPL_IRQHANDLER)(int param, int irq);
typedef void (*OPL_UPDATEHANDLER)(int param, int min_interval_us);

#define OPL_TYPE_WAVESEL   0x01  /* waveform select    */

/* Saving is necessary for member of the 'R' mark for suspend/resume */
/* ---------- OPL one of slot  ---------- */
typedef struct fm_opl_slot {
	int TL;		/* total level     :TL << 8				*/
	int TLL;	/* adjusted now TL						*/
	guint8 KSR;	/* key scale rate  :(shift down bit)	*/
	int *AR;	/* attack rate     :&AR_TABLE[AR<<2]	*/
	int *DR;	/* decay rate      :&DR_TABLE[DR<<2]	*/
	int SL;		/* sustain level   :SL_TABLE[SL]		*/
	int *RR;	/* release rate    :&DR_TABLE[RR<<2]	*/
	guint8 ksl;	/* keyscale level  :(shift down bits)	*/
	guint8 ksr;	/* key scale rate  :kcode>>KSR			*/
	guint32 mul;	/* multiple        :ML_TABLE[ML]		*/
	guint32 Cnt;	/* frequency count						*/
	guint32 Incr;	/* frequency step						*/

	/* envelope generator state */
	guint8 eg_typ;/* envelope type flag					*/
	guint8 evm;	/* envelope phase						*/
	int evc;	/* envelope counter						*/
	int eve;	/* envelope counter end point			*/
	int evs;	/* envelope counter step				*/
	int evsa;	/* envelope step for AR :AR[ksr]		*/
	int evsd;	/* envelope step for DR :DR[ksr]		*/
	int evsr;	/* envelope step for RR :RR[ksr]		*/

	/* LFO */
	guint8 ams;		/* ams flag                            */
	guint8 vib;		/* vibrate flag                        */
	/* wave selector */
	int **wavetable;
} OPL_SLOT;

/* ---------- OPL one of channel  ---------- */
typedef struct fm_opl_channel {
	OPL_SLOT SLOT[2];
	guint8 CON;			/* connection type					*/
	guint8 FB;			/* feed back       :(shift down bit)*/
	int *connect1;		/* slot1 output pointer				*/
	int *connect2;		/* slot2 output pointer				*/
	int op1_out[2];		/* slot1 output for selfeedback		*/

	/* phase generator state */
	guint32 block_fnum;	/* block+fnum						*/
	guint8 kcode;		/* key code        : KeyScaleCode	*/
	guint32 fc;			/* Freq. Increment base				*/
	guint32 ksl_base;		/* KeyScaleLevel Base step			*/
	guint8 keyon;		/* key on/off flag					*/
} OPL_CH;

/* OPL state */
typedef struct fm_opl_f {
	guint8 type;			/* chip type                         */
	int clock;			/* master clock  (Hz)                */
	int rate;			/* sampling rate (Hz)                */
	double freqbase;	/* frequency base                    */
	double TimerBase;	/* Timer base time (==sampling time) */
	guint8 address;		/* address register                  */
	guint8 status;		/* status flag                       */
	guint8 statusmask;	/* status mask                       */
	guint32 mode;			/* Reg.08 : CSM , notesel,etc.       */

	/* Timer */
	int T[2];			/* timer counter                     */
	guint8 st[2];		/* timer enable                      */

	/* FM channel slots */
	OPL_CH *P_CH;		/* pointer of CH                     */
	int	max_ch;			/* maximum channel                   */

	/* Rythm sention */
	guint8 rythm;		/* Rythm mode , key flag */

	/* time tables */
	int AR_TABLE[75];	/* atttack rate tables				*/
	int DR_TABLE[75];	/* decay rate tables				*/
	guint32 FN_TABLE[1024];/* fnumber -> increment counter		*/

	/* LFO */
	int *ams_table;
	int *vib_table;
	int amsCnt;
	int amsIncr;
	int vibCnt;
	int vibIncr;

	/* wave selector enable flag */
	guint8 wavesel;

	/* external event callback handler */
	OPL_TIMERHANDLER  TimerHandler;		/* TIMER handler   */
	int TimerParam;						/* TIMER parameter */
	OPL_IRQHANDLER    IRQHandler;		/* IRQ handler    */
	int IRQParam;						/* IRQ parameter  */
	OPL_UPDATEHANDLER UpdateHandler;	/* stream update handler   */
	int UpdateParam;					/* stream update parameter */
} FM_OPL;

/* ---------- Generic interface section ---------- */
#define OPL_TYPE_YM3526 (0)
#define OPL_TYPE_YM3812 (OPL_TYPE_WAVESEL)

void OPLBuildTables(int ENV_BITS_PARAM, int EG_ENT_PARAM);

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
void YM3812UpdateOne(FM_OPL *OPL, gint16 *buffer, int length, int interleave);
#endif
