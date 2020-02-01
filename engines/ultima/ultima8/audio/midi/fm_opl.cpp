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

#include "ultima/ultima8/misc/pent_include.h"

#ifdef USE_FMOPL_MIDI

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <cmath>
#include <iostream>
#include "fmopl.h"

namespace Ultima8 {
namespace FMOpl_Pentagram {

#ifndef UNDER_CE
using std::malloc;
using std::free;
#endif

#ifndef PI
#define PI 3.14159265358979323846
#endif

/* -------------------- preliminary define section --------------------- */
/* attack/decay rate time rate */
#define OPL_ARRATE     141280  /* RATE 4 =  2826.24ms @ 3.6MHz */
#define OPL_DRRATE    1956000  /* RATE 4 = 39280.64ms @ 3.6MHz */

#define FREQ_BITS 24            /* frequency turn          */

/* counter bits = 20 , octerve 7 */
#define FREQ_RATE   (1<<(FREQ_BITS-20))
#define TL_BITS    (FREQ_BITS+2)

/* final output shift , limit minimum and maximum */
#define OPL_OUTSB   (TL_BITS+3-16)      /* OPL output final shift 16bit */
#define OPL_MAXOUT (0x7fff<<OPL_OUTSB)
#define OPL_MINOUT (-0x8000<<OPL_OUTSB)

/* -------------------- quality selection --------------------- */

/* sinwave entries */
/* used static memory = SIN_ENT * 4 (byte) */
#define SIN_ENT 2048

/* output level entries (envelope,sinwave) */
/* envelope counter lower bits */
#define ENV_BITS 16
/* envelope output entries */
#define EG_ENT   4096
/* used dynamic memory = EG_ENT*4*4(byte)or EG_ENT*6*4(byte) */
/* used static  memory = EG_ENT*4 (byte)                     */

#define EG_OFF   ((2*EG_ENT)<<ENV_BITS)  /* OFF          */
#define EG_DED   EG_OFF
#define EG_DST   (EG_ENT<<ENV_BITS)      /* DECAY  START */
#define EG_AED   EG_DST
#define EG_AST   0                       /* ATTACK START */

#define EG_STEP (96.0/EG_ENT) /* OPL is 0.1875 dB step  */

/* LFO table entries */
#define VIB_ENT 512
#define VIB_SHIFT (32-9)
#define AMS_ENT 512
#define AMS_SHIFT (32-9)

#define VIB_RATE 256

/* -------------------- local defines , macros --------------------- */

/* register number to channel number , slot offset */
#define SLOT1 0
#define SLOT2 1

/* envelope phase */
#define ENV_MOD_RR  0x00
#define ENV_MOD_DR  0x01
#define ENV_MOD_AR  0x02

/* -------------------- tables --------------------- */
static const int slot_array[32] = {
	0, 2, 4, 1, 3, 5, -1, -1,
	6, 8, 10, 7, 9, 11, -1, -1,
	12, 14, 16, 13, 15, 17, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1
};

#define SC(mydb) (static_cast<uint32>(mydb / (EG_STEP/2)))

static const uint32 KSL_TABLE[8 * 16] = {
	/* OCT 0 */
	SC(0.000), SC(0.000), SC(0.000), SC(0.000),
	SC(0.000), SC(0.000), SC(0.000), SC(0.000),
	SC(0.000), SC(0.000), SC(0.000), SC(0.000),
	SC(0.000), SC(0.000), SC(0.000), SC(0.000),
	/* OCT 1 */
	SC(0.000), SC(0.000), SC(0.000), SC(0.000),
	SC(0.000), SC(0.000), SC(0.000), SC(0.000),
	SC(0.000), SC(0.750), SC(1.125), SC(1.500),
	SC(1.875), SC(2.250), SC(2.625), SC(3.000),
	/* OCT 2 */
	SC(0.000), SC(0.000), SC(0.000), SC(0.000),
	SC(0.000), SC(1.125), SC(1.875), SC(2.625),
	SC(3.000), SC(3.750), SC(4.125), SC(4.500),
	SC(4.875), SC(5.250), SC(5.625), SC(6.000),
	/* OCT 3 */
	SC(0.000), SC(0.000), SC(0.000), SC(1.875),
	SC(3.000), SC(4.125), SC(4.875), SC(5.625),
	SC(6.000), SC(6.750), SC(7.125), SC(7.500),
	SC(7.875), SC(8.250), SC(8.625), SC(9.000),
	/* OCT 4 */
	SC(0.000), SC(0.000), SC(3.000), SC(4.875),
	SC(6.000), SC(7.125), SC(7.875), SC(8.625),
	SC(9.000), SC(9.750), SC(10.125), SC(10.500),
	SC(10.875), SC(11.250), SC(11.625), SC(12.000),
	/* OCT 5 */
	SC(0.000), SC(3.000), SC(6.000), SC(7.875),
	SC(9.000), SC(10.125), SC(10.875), SC(11.625),
	SC(12.000), SC(12.750), SC(13.125), SC(13.500),
	SC(13.875), SC(14.250), SC(14.625), SC(15.000),
	/* OCT 6 */
	SC(0.000), SC(6.000), SC(9.000), SC(10.875),
	SC(12.000), SC(13.125), SC(13.875), SC(14.625),
	SC(15.000), SC(15.750), SC(16.125), SC(16.500),
	SC(16.875), SC(17.250), SC(17.625), SC(18.000),
	/* OCT 7 */
	SC(0.000), SC(9.000), SC(12.000), SC(13.875),
	SC(15.000), SC(16.125), SC(16.875), SC(17.625),
	SC(18.000), SC(18.750), SC(19.125), SC(19.500),
	SC(19.875), SC(20.250), SC(20.625), SC(21.000)
};
#undef SC


/* sustain lebel table (3db per step) */
/* 0 - 15: 0, 3, 6, 9,12,15,18,21,24,27,30,33,36,39,42,93 (dB)*/
#define SC(db) static_cast<int>(db*((3/EG_STEP)*(1<<ENV_BITS)))+EG_DST
static const int SL_TABLE[16] = {
	SC(0), SC(1), SC(2), SC(3), SC(4), SC(5), SC(6), SC(7),
	SC(8), SC(9), SC(10), SC(11), SC(12), SC(13), SC(14), SC(31)
};
#undef SC

#define TL_MAX (EG_ENT*2) /* limit(tl + ksr + envelope) + sinwave */
/* TotalLevel : 48 24 12  6  3 1.5 0.75 (dB) */
/* TL_TABLE[ 0      to TL_MAX          ] : plus  section */
/* TL_TABLE[ TL_MAX to TL_MAX+TL_MAX-1 ] : minus section */
static int *TL_TABLE;

/* pointers to TL_TABLE with sinwave output offset */
static int **SIN_TABLE;

/* LFO table */
static int *AMS_TABLE;
static int *VIB_TABLE;

/* envelope output curve table */
/* attack + decay + OFF */
static int ENV_CURVE[2 * EG_ENT + 1];

/* multiple table */
#define ML(a) static_cast<int>(a*2)
static const uint32 MUL_TABLE[16] = {
	/* 1/2, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15 */
	ML(0.50), ML(1.00), ML(2.00),  ML(3.00), ML(4.00), ML(5.00), ML(6.00), ML(7.00),
	ML(8.00), ML(9.00), ML(10.00), ML(10.00), ML(12.00), ML(12.00), ML(15.00), ML(15.00)
};
#undef ML

/* dummy attack / decay rate ( when rate == 0 ) */
static int RATE_0[16] =
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

/* -------------------- static state --------------------- */

/* lock level of common table */
static int num_lock = 0;

/* work table */
static void *cur_chip = NULL;   /* current chip point */
/* currenct chip state */
/* static OPLSAMPLE  *bufL,*bufR; */
static OPL_CH *S_CH;
static OPL_CH *E_CH;
OPL_SLOT *SLOT7_1, *SLOT7_2, *SLOT8_1, *SLOT8_2;

static int outd[1];
static int ams;
static int vib;
int  *ams_table;
int  *vib_table;
static int amsIncr;
static int vibIncr;
static int feedback2;       /* connect for SLOT 2 */

/* --------------------- subroutines  --------------------- */

inline int Limit(int val, int max, int min) {
	if (val > max)
		val = max;
	else if (val < min)
		val = min;

	return val;
}

/* status set and IRQ handling */
inline void OPL_STATUS_SET(FM_OPL *OPL, int flag) {
	/* set status flag */
	OPL->status |= flag;
	if (!(OPL->status & 0x80)) {
		if (OPL->status & OPL->statusmask) {
			/* IRQ on */
			OPL->status |= 0x80;
			/* callback user interrupt handler (IRQ is OFF to ON) */
			if (OPL->IRQHandler)(OPL->IRQHandler)(OPL->IRQParam, 1);
		}
	}
}

/* status reset and IRQ handling */
inline void OPL_STATUS_RESET(FM_OPL *OPL, int flag) {
	/* reset status flag */
	OPL->status &= ~flag;
	if ((OPL->status & 0x80)) {
		if (!(OPL->status & OPL->statusmask)) {
			OPL->status &= 0x7f;
			/* callback user interrupt handler (IRQ is ON to OFF) */
			if (OPL->IRQHandler)(OPL->IRQHandler)(OPL->IRQParam, 0);
		}
	}
}

/* IRQ mask set */
inline void OPL_STATUSMASK_SET(FM_OPL *OPL, int flag) {
	OPL->statusmask = flag;
	/* IRQ handling check */
	OPL_STATUS_SET(OPL, 0);
	OPL_STATUS_RESET(OPL, 0);
}

/* ----- key on  ----- */
inline void OPL_KEYON(OPL_SLOT *SLOT) {
	/* sin wave restart */
	SLOT->Cnt = 0;
	/* set attack */
	SLOT->evm = ENV_MOD_AR;
	SLOT->evs = SLOT->evsa;
	SLOT->evc = EG_AST;
	SLOT->eve = EG_AED;
}
/* ----- key off ----- */
inline void OPL_KEYOFF(OPL_SLOT *SLOT) {
	if (SLOT->evm > ENV_MOD_RR) {
		/* set envelope counter from envleope output */
		SLOT->evm = ENV_MOD_RR;
		if (!(SLOT->evc & EG_DST))
			//SLOT->evc = (ENV_CURVE[SLOT->evc>>ENV_BITS]<<ENV_BITS) + EG_DST;
			SLOT->evc = EG_DST;
		SLOT->eve = EG_DED;
		SLOT->evs = SLOT->evsr;
	}
}

/* ---------- calcrate Envelope Generator & Phase Generator ---------- */
/* return : envelope output */
inline uint32 OPL_CALC_SLOT(OPL_SLOT *SLOT) {
	/* calcrate envelope generator */
	if ((SLOT->evc += SLOT->evs) >= SLOT->eve) {
		switch (SLOT->evm) {
		case ENV_MOD_AR: /* ATTACK -> DECAY1 */
			/* next DR */
			SLOT->evm = ENV_MOD_DR;
			SLOT->evc = EG_DST;
			SLOT->eve = SLOT->SL;
			SLOT->evs = SLOT->evsd;
			break;
		case ENV_MOD_DR: /* DECAY -> SL or RR */
			SLOT->evc = SLOT->SL;
			SLOT->eve = EG_DED;
			if (SLOT->eg_typ) {
				SLOT->evs = 0;
			} else {
				SLOT->evm = ENV_MOD_RR;
				SLOT->evs = SLOT->evsr;
			}
			break;
		case ENV_MOD_RR: /* RR -> OFF */
			SLOT->evc = EG_OFF;
			SLOT->eve = EG_OFF + 1;
			SLOT->evs = 0;
			break;
		}
	}
	/* calcrate envelope */
	return SLOT->TLL + ENV_CURVE[SLOT->evc >> ENV_BITS] + (SLOT->ams ? ams : 0);
}

/* set algorythm connection */
static void set_algorythm(OPL_CH *CH) {
	int *carrier = &outd[0];
	CH->connect1 = CH->CON ? carrier : &feedback2;
	CH->connect2 = carrier;
}

/* ---------- frequency counter for operater update ---------- */
inline void CALC_FCSLOT(OPL_CH *CH, OPL_SLOT *SLOT) {
	int ksr;

	/* frequency step counter */
	SLOT->Incr = CH->fc * SLOT->mul;
	ksr = CH->kcode >> SLOT->KSR;

	if (SLOT->ksr != ksr) {
		SLOT->ksr = ksr;
		/* attack , decay rate recalcration */
		SLOT->evsa = SLOT->AR[ksr];
		SLOT->evsd = SLOT->DR[ksr];
		SLOT->evsr = SLOT->RR[ksr];
	}
	SLOT->TLL = SLOT->TL + (CH->ksl_base >> SLOT->ksl);
}

/* set multi,am,vib,EG-TYP,KSR,mul */
inline void set_mul(FM_OPL *OPL, int slot, int v) {
	OPL_CH   *CH   = &OPL->P_CH[slot / 2];
	OPL_SLOT *SLOT = &CH->SLOT[slot & 1];

	SLOT->mul    = MUL_TABLE[v & 0x0f];
	SLOT->KSR    = (v & 0x10) ? 0 : 2;
	SLOT->eg_typ = (v & 0x20) >> 5;
	SLOT->vib    = (v & 0x40);
	SLOT->ams    = (v & 0x80);
	CALC_FCSLOT(CH, SLOT);
}

/* set ksl & tl */
inline void set_ksl_tl(FM_OPL *OPL, int slot, int v) {
	OPL_CH   *CH   = &OPL->P_CH[slot / 2];
	OPL_SLOT *SLOT = &CH->SLOT[slot & 1];
	int ksl = v >> 6; /* 0 / 1.5 / 3 / 6 db/OCT */

	SLOT->ksl = ksl ? 3 - ksl : 31;
	SLOT->TL  = (int)((v & 0x3f) * (0.75 / EG_STEP)); /* 0.75db step */

	if (!(OPL->mode & 0x80)) {
		/* not CSM latch total level */
		SLOT->TLL = SLOT->TL + (CH->ksl_base >> SLOT->ksl);
	}
}

/* set attack rate & decay rate  */
inline void set_ar_dr(FM_OPL *OPL, int slot, int v) {
	OPL_CH   *CH   = &OPL->P_CH[slot / 2];
	OPL_SLOT *SLOT = &CH->SLOT[slot & 1];
	int ar = v >> 4;
	int dr = v & 0x0f;

	SLOT->AR = ar ? &OPL->AR_TABLE[ar << 2] : RATE_0;
	SLOT->evsa = SLOT->AR[SLOT->ksr];
	if (SLOT->evm == ENV_MOD_AR) SLOT->evs = SLOT->evsa;

	SLOT->DR = dr ? &OPL->DR_TABLE[dr << 2] : RATE_0;
	SLOT->evsd = SLOT->DR[SLOT->ksr];
	if (SLOT->evm == ENV_MOD_DR) SLOT->evs = SLOT->evsd;
}

/* set sustain level & release rate */
inline void set_sl_rr(FM_OPL *OPL, int slot, int v) {
	OPL_CH   *CH   = &OPL->P_CH[slot / 2];
	OPL_SLOT *SLOT = &CH->SLOT[slot & 1];
	int sl = v >> 4;
	int rr = v & 0x0f;

	SLOT->SL = SL_TABLE[sl];
	if (SLOT->evm == ENV_MOD_DR) SLOT->eve = SLOT->SL;
	SLOT->RR = &OPL->DR_TABLE[rr << 2];
	SLOT->evsr = SLOT->RR[SLOT->ksr];
	if (SLOT->evm == ENV_MOD_RR) SLOT->evs = SLOT->evsr;
}

/* operator output calcrator */
#define OP_OUT(slot,env,con)   slot->wavetable[((slot->Cnt+con)/(0x1000000/SIN_ENT))&(SIN_ENT-1)][env]
/* ---------- calcrate one of channel ---------- */
inline void OPL_CALC_CH(OPL_CH *CH) {
	uint32 env_out;
	OPL_SLOT *SLOT;

	feedback2 = 0;
	/* SLOT 1 */
	SLOT = &CH->SLOT[SLOT1];
	env_out = OPL_CALC_SLOT(SLOT);
	if (env_out < EG_ENT - 1) {
		/* PG */
		if (SLOT->vib) SLOT->Cnt += (SLOT->Incr * vib / VIB_RATE);
		else          SLOT->Cnt += SLOT->Incr;
		/* connectoion */
		if (CH->FB) {
			int feedback1 = (CH->op1_out[0] + CH->op1_out[1]) >> CH->FB;
			CH->op1_out[1] = CH->op1_out[0];
			*CH->connect1 += CH->op1_out[0] = OP_OUT(SLOT, env_out, feedback1);
		} else {
			*CH->connect1 += OP_OUT(SLOT, env_out, 0);
		}
	} else {
		CH->op1_out[1] = CH->op1_out[0];
		CH->op1_out[0] = 0;
	}
	/* SLOT 2 */
	SLOT = &CH->SLOT[SLOT2];
	env_out = OPL_CALC_SLOT(SLOT);
	if (env_out < EG_ENT - 1) {
		/* PG */
		if (SLOT->vib) SLOT->Cnt += (SLOT->Incr * vib / VIB_RATE);
		else          SLOT->Cnt += SLOT->Incr;
		/* connectoion */
		outd[0] += OP_OUT(SLOT, env_out, feedback2);
	}
}

/* ---------- calcrate rythm block ---------- */
#define WHITE_NOISE_db 6.0
inline void OPL_CALC_RH(OPL_CH *CH) {
	uint32 env_tam, env_sd, env_top, env_hh;
	int whitenoise = int((std::rand() & 1) * (WHITE_NOISE_db / EG_STEP));
	int tone8;

	OPL_SLOT *SLOT;
	int env_out;

	/* BD : same as FM serial mode and output level is large */
	feedback2 = 0;
	/* SLOT 1 */
	SLOT = &CH[6].SLOT[SLOT1];
	env_out = OPL_CALC_SLOT(SLOT);
	if (env_out < EG_ENT - 1) {
		/* PG */
		if (SLOT->vib) SLOT->Cnt += (SLOT->Incr * vib / VIB_RATE);
		else          SLOT->Cnt += SLOT->Incr;
		/* connectoion */
		if (CH[6].FB) {
			int feedback1 = (CH[6].op1_out[0] + CH[6].op1_out[1]) >> CH[6].FB;
			CH[6].op1_out[1] = CH[6].op1_out[0];
			feedback2 = CH[6].op1_out[0] = OP_OUT(SLOT, env_out, feedback1);
		} else {
			feedback2 = OP_OUT(SLOT, env_out, 0);
		}
	} else {
		feedback2 = 0;
		CH[6].op1_out[1] = CH[6].op1_out[0];
		CH[6].op1_out[0] = 0;
	}
	/* SLOT 2 */
	SLOT = &CH[6].SLOT[SLOT2];
	env_out = OPL_CALC_SLOT(SLOT);
	if (env_out < EG_ENT - 1) {
		/* PG */
		if (SLOT->vib) SLOT->Cnt += (SLOT->Incr * vib / VIB_RATE);
		else          SLOT->Cnt += SLOT->Incr;
		/* connectoion */
		outd[0] += OP_OUT(SLOT, env_out, feedback2) * 2;
	}

	// SD  (17) = mul14[fnum7] + white noise
	// TAM (15) = mul15[fnum8]
	// TOP (18) = fnum6(mul18[fnum8]+whitenoise)
	// HH  (14) = fnum7(mul18[fnum8]+whitenoise) + white noise
	env_sd = OPL_CALC_SLOT(SLOT7_2) + whitenoise;
	env_tam = OPL_CALC_SLOT(SLOT8_1);
	env_top = OPL_CALC_SLOT(SLOT8_2);
	env_hh = OPL_CALC_SLOT(SLOT7_1) + whitenoise;

	/* PG */
	if (SLOT7_1->vib) SLOT7_1->Cnt += (2 * SLOT7_1->Incr * vib / VIB_RATE);
	else             SLOT7_1->Cnt += 2 * SLOT7_1->Incr;
	if (SLOT7_2->vib) SLOT7_2->Cnt += ((CH[7].fc * 8) * vib / VIB_RATE);
	else             SLOT7_2->Cnt += (CH[7].fc * 8);
	if (SLOT8_1->vib) SLOT8_1->Cnt += (SLOT8_1->Incr * vib / VIB_RATE);
	else             SLOT8_1->Cnt += SLOT8_1->Incr;
	if (SLOT8_2->vib) SLOT8_2->Cnt += ((CH[8].fc * 48) * vib / VIB_RATE);
	else             SLOT8_2->Cnt += (CH[8].fc * 48);

	tone8 = OP_OUT(SLOT8_2, whitenoise, 0);

	/* SD */
	if (env_sd < EG_ENT - 1)
		outd[0] += OP_OUT(SLOT7_1, env_sd, 0) * 8;
	/* TAM */
	if (env_tam < EG_ENT - 1)
		outd[0] += OP_OUT(SLOT8_1, env_tam, 0) * 2;
	/* TOP-CY */
	if (env_top < EG_ENT - 1)
		outd[0] += OP_OUT(SLOT7_2, env_top, tone8) * 2;
	/* HH */
	if (env_hh  < EG_ENT - 1)
		outd[0] += OP_OUT(SLOT7_2, env_hh, tone8) * 2;
}

/* ----------- initialize time tabls ----------- */
static void init_timetables(FM_OPL *OPL , int ARRATE , int DRRATE) {
	int i;
	double rate;

	/* make attack rate & decay rate tables */
	for (i = 0; i < 4; i++) OPL->AR_TABLE[i] = OPL->DR_TABLE[i] = 0;
	for (i = 4; i <= 60; i++) {
		rate  = OPL->freqbase;                      /* frequency rate */
		if (i < 60) rate *= 1.0 + (i & 3) * 0.25; /* b0-1 : x1 , x1.25 , x1.5 , x1.75 */
		rate *= 1 << ((i >> 2) - 1);                /* b2-5 : shift bit */
		rate *= (double)(EG_ENT << ENV_BITS);
		OPL->AR_TABLE[i] = (int)(rate / ARRATE);
		OPL->DR_TABLE[i] = (int)(rate / DRRATE);
	}
	for (i = 60; i < 76; i++) {
		OPL->AR_TABLE[i] = EG_AED - 1;
		OPL->DR_TABLE[i] = OPL->DR_TABLE[60];
	}
}

/* ---------- generic table initialize ---------- */
static int OPLOpenTable(void) {
	int s, t;
	double rate;
	int i, j;
	double pom;

	/* allocate dynamic tables */
	if ((TL_TABLE = (int *)malloc(TL_MAX * 2 * sizeof(int))) == NULL)
		return 0;
	if ((SIN_TABLE = (int **)malloc(SIN_ENT * 4 * sizeof(int *))) == NULL) {
		free(TL_TABLE);
		return 0;
	}
	if ((AMS_TABLE = (int *)malloc(AMS_ENT * 2 * sizeof(int))) == NULL) {
		free(TL_TABLE);
		free(SIN_TABLE);
		return 0;
	}
	if ((VIB_TABLE = (int *)malloc(VIB_ENT * 2 * sizeof(int))) == NULL) {
		free(TL_TABLE);
		free(SIN_TABLE);
		free(AMS_TABLE);
		return 0;
	}
	/* make total level table */
	for (t = 0; t < EG_ENT - 1 ; t++) {
		rate = ((1 << TL_BITS) - 1) / std::pow(10.0, EG_STEP * t / 20); /* dB -> voltage */
		TL_TABLE[       t] = (int)rate;
		TL_TABLE[TL_MAX + t] = -TL_TABLE[t];
	}
	/* fill volume off area */
	for (t = EG_ENT - 1; t < TL_MAX ; t++) {
		TL_TABLE[t] = TL_TABLE[TL_MAX + t] = 0;
	}

	/* make sinwave table (total level offet) */
	/* degree 0 = degree 180                   = off */
	SIN_TABLE[0] = SIN_TABLE[SIN_ENT / 2]         = &TL_TABLE[EG_ENT - 1];
	for (s = 1; s <= SIN_ENT / 4; s++) {
		pom = std::sin(2 * PI * s / SIN_ENT); /* sin     */
		pom = 20 * std::log10(1 / pom); /* decibel */
		j = int(pom / EG_STEP);         /* TL_TABLE steps */

		/* degree 0   -  90    , degree 180 -  90 : plus section */
		SIN_TABLE[          s] = SIN_TABLE[SIN_ENT / 2 - s] = &TL_TABLE[j];
		/* degree 180 - 270    , degree 360 - 270 : minus section */
		SIN_TABLE[SIN_ENT / 2 + s] = SIN_TABLE[SIN_ENT  - s] = &TL_TABLE[TL_MAX + j];
	}
	for (s = 0; s < SIN_ENT; s++) {
		SIN_TABLE[SIN_ENT * 1 + s] = s < (SIN_ENT / 2) ? SIN_TABLE[s] : &TL_TABLE[EG_ENT];
		SIN_TABLE[SIN_ENT * 2 + s] = SIN_TABLE[s % (SIN_ENT / 2)];
		SIN_TABLE[SIN_ENT * 3 + s] = (s / (SIN_ENT / 4)) & 1 ? &TL_TABLE[EG_ENT] : SIN_TABLE[SIN_ENT * 2 + s];
	}

	/* envelope counter -> envelope output table */
	for (i = 0; i < EG_ENT; i++) {
		/* ATTACK curve */
		pom = std::pow(((double)(EG_ENT - 1 - i) / EG_ENT) , 8) * EG_ENT;
		/* if( pom >= EG_ENT ) pom = EG_ENT-1; */
		ENV_CURVE[i] = (int)pom;
		/* DECAY ,RELEASE curve */
		ENV_CURVE[(EG_DST >> ENV_BITS) + i] = i;
	}
	/* off */
	ENV_CURVE[EG_OFF >> ENV_BITS] = EG_ENT - 1;
	/* make LFO ams table */
	for (i = 0; i < AMS_ENT; i++) {
		pom = (1.0 + std::sin(2 * PI * i / AMS_ENT)) / 2; /* sin */
		AMS_TABLE[i]         = (int)((1.0 / EG_STEP) * pom); /* 1dB   */
		AMS_TABLE[AMS_ENT + i] = (int)((4.8 / EG_STEP) * pom); /* 4.8dB */
	}
	/* make LFO vibrate table */
	for (i = 0; i < VIB_ENT; i++) {
		/* 100cent = 1seminote = 6% ?? */
		pom = (double)VIB_RATE * 0.06 * std::sin(2 * PI * i / VIB_ENT); /* +-100sect step */
		VIB_TABLE[i]         = (int)(VIB_RATE + (pom * 0.07)); /* +- 7cent */
		VIB_TABLE[VIB_ENT + i] = (int)(VIB_RATE + (pom * 0.14)); /* +-14cent */
	}
	return 1;
}


static void OPLCloseTable(void) {
	free(TL_TABLE);
	free(SIN_TABLE);
	free(AMS_TABLE);
	free(VIB_TABLE);
}

/* CSM Key Controll */
inline void CSMKeyControll(OPL_CH *CH) {
	OPL_SLOT *slot1 = &CH->SLOT[SLOT1];
	OPL_SLOT *slot2 = &CH->SLOT[SLOT2];
	/* all key off */
	OPL_KEYOFF(slot1);
	OPL_KEYOFF(slot2);
	/* total level latch */
	slot1->TLL = slot1->TL + (CH->ksl_base >> slot1->ksl);
	slot1->TLL = slot1->TL + (CH->ksl_base >> slot1->ksl);
	/* key on */
	CH->op1_out[0] = CH->op1_out[1] = 0;
	OPL_KEYON(slot1);
	OPL_KEYON(slot2);
}

/* ---------- opl initialize ---------- */
static void OPL_initalize(FM_OPL *OPL) {
	int fn;

	/* frequency base */
	OPL->freqbase = (OPL->rate) ? ((double)OPL->clock / OPL->rate) / 72  : 0;
	/* Timer base time */
	OPL->TimerBase = 1.0 / ((double)OPL->clock / 72.0);
	/* make time tables */
	init_timetables(OPL , OPL_ARRATE , OPL_DRRATE);
	/* make fnumber -> increment counter table */
	for (fn = 0 ; fn < 1024 ; fn++) {
		OPL->FN_TABLE[fn] = (uint32)(OPL->freqbase * fn * FREQ_RATE * (1 << 7) / 2);
	}
	/* LFO freq.table */
	OPL->amsIncr = (int)(OPL->rate ? (double)AMS_ENT * (1 << AMS_SHIFT) / OPL->rate * 3.7 * ((double)OPL->clock / 3600000) : 0);
	OPL->vibIncr = (int)(OPL->rate ? (double)VIB_ENT * (1 << VIB_SHIFT) / OPL->rate * 6.4 * ((double)OPL->clock / 3600000) : 0);
}

/* ---------- write a OPL registers ---------- */
void OPLWriteReg(FM_OPL *OPL, int r, int v) {
	OPL_CH *CH;
	int slot;
	uint32 block_fnum;

	switch (r & 0xe0) {
	case 0x00: /* 00-1f:controll */
		switch (r & 0x1f) {
		case 0x01:
			/* wave selector enable */
			if (OPL->type & OPL_TYPE_WAVESEL) {
				OPL->wavesel = v & 0x20;
				if (!OPL->wavesel) {
					/* preset compatible mode */
					int c;
					for (c = 0; c < OPL->max_ch; c++) {
						OPL->P_CH[c].SLOT[SLOT1].wavetable = &SIN_TABLE[0];
						OPL->P_CH[c].SLOT[SLOT2].wavetable = &SIN_TABLE[0];
					}
				}
			}
			return;
		case 0x02:  /* Timer 1 */
			OPL->T[0] = (256 - v) * 4;
			break;
		case 0x03:  /* Timer 2 */
			OPL->T[1] = (256 - v) * 16;
			return;
		case 0x04:  /* IRQ clear / mask and Timer enable */
			if (v & 0x80) {
				/* IRQ flag clear */
				OPL_STATUS_RESET(OPL, 0x7f);
			} else {
				/* set IRQ mask ,timer enable*/
				uint8 st1 = v & 1;
				uint8 st2 = (v >> 1) & 1;
				/* IRQRST,T1MSK,t2MSK,EOSMSK,BRMSK,x,ST2,ST1 */
				OPL_STATUS_RESET(OPL, v & 0x78);
				OPL_STATUSMASK_SET(OPL, ((~v) & 0x78) | 0x01);
				/* timer 2 */
				if (OPL->st[1] != st2) {
					double interval = st2 ? (double)OPL->T[1] * OPL->TimerBase : 0.0;
					OPL->st[1] = st2;
					if (OPL->TimerHandler)(OPL->TimerHandler)(OPL->TimerParam + 1, interval);
				}
				/* timer 1 */
				if (OPL->st[0] != st1) {
					double interval = st1 ? (double)OPL->T[0] * OPL->TimerBase : 0.0;
					OPL->st[0] = st1;
					if (OPL->TimerHandler)(OPL->TimerHandler)(OPL->TimerParam + 0, interval);
				}
			}
			return;
		}
		break;
	case 0x20:  /* am,vib,ksr,eg type,mul */
		slot = slot_array[r & 0x1f];
		if (slot == -1) return;
		set_mul(OPL, slot, v);
		return;
	case 0x40:
		slot = slot_array[r & 0x1f];
		if (slot == -1) return;
		set_ksl_tl(OPL, slot, v);
		return;
	case 0x60:
		slot = slot_array[r & 0x1f];
		if (slot == -1) return;
		set_ar_dr(OPL, slot, v);
		return;
	case 0x80:
		slot = slot_array[r & 0x1f];
		if (slot == -1) return;
		set_sl_rr(OPL, slot, v);
		return;
	case 0xa0:
		switch (r) {
		case 0xbd:
			/* amsep,vibdep,r,bd,sd,tom,tc,hh */
		{
			uint8 rkey = OPL->rythm ^ v;
			OPL->ams_table = &AMS_TABLE[v & 0x80 ? AMS_ENT : 0];
			OPL->vib_table = &VIB_TABLE[v & 0x40 ? VIB_ENT : 0];
			OPL->rythm  = v & 0x3f;
			if (OPL->rythm & 0x20) {
				/* BD key on/off */
				if (rkey & 0x10) {
					if (v & 0x10) {
						OPL->P_CH[6].op1_out[0] = OPL->P_CH[6].op1_out[1] = 0;
						OPL_KEYON(&OPL->P_CH[6].SLOT[SLOT1]);
						OPL_KEYON(&OPL->P_CH[6].SLOT[SLOT2]);
					} else {
						OPL_KEYOFF(&OPL->P_CH[6].SLOT[SLOT1]);
						OPL_KEYOFF(&OPL->P_CH[6].SLOT[SLOT2]);
					}
				}
				/* SD key on/off */
				if (rkey & 0x08) {
					if (v & 0x08) OPL_KEYON(&OPL->P_CH[7].SLOT[SLOT2]);
					else       OPL_KEYOFF(&OPL->P_CH[7].SLOT[SLOT2]);
				}/* TAM key on/off */
				if (rkey & 0x04) {
					if (v & 0x04) OPL_KEYON(&OPL->P_CH[8].SLOT[SLOT1]);
					else       OPL_KEYOFF(&OPL->P_CH[8].SLOT[SLOT1]);
				}
				/* TOP-CY key on/off */
				if (rkey & 0x02) {
					if (v & 0x02) OPL_KEYON(&OPL->P_CH[8].SLOT[SLOT2]);
					else       OPL_KEYOFF(&OPL->P_CH[8].SLOT[SLOT2]);
				}
				/* HH key on/off */
				if (rkey & 0x01) {
					if (v & 0x01) OPL_KEYON(&OPL->P_CH[7].SLOT[SLOT1]);
					else       OPL_KEYOFF(&OPL->P_CH[7].SLOT[SLOT1]);
				}
			}
		}
		return;
		}
		/* keyon,block,fnum */
		if ((r & 0x0f) > 8) return;
		CH = &OPL->P_CH[r & 0x0f];
		if (!(r & 0x10)) {
			/* a0-a8 */
			block_fnum  = (CH->block_fnum & 0x1f00) | v;
		} else {
			/* b0-b8 */
			int keyon = (v >> 5) & 1;
			block_fnum = ((v & 0x1f) << 8) | (CH->block_fnum & 0xff);
			if (CH->keyon != keyon) {
				if ((CH->keyon = keyon)) {
					CH->op1_out[0] = CH->op1_out[1] = 0;
					OPL_KEYON(&CH->SLOT[SLOT1]);
					OPL_KEYON(&CH->SLOT[SLOT2]);
				} else {
					OPL_KEYOFF(&CH->SLOT[SLOT1]);
					OPL_KEYOFF(&CH->SLOT[SLOT2]);
				}
			}
		}
		/* update */
		if (CH->block_fnum != block_fnum) {
			int blockRv = 7 - (block_fnum >> 10);
			int fnum   = block_fnum & 0x3ff;
			CH->block_fnum = block_fnum;

			CH->ksl_base = KSL_TABLE[block_fnum >> 6];
			CH->fc = OPL->FN_TABLE[fnum] >> blockRv;
			CH->kcode = CH->block_fnum >> 9;
			if ((OPL->mode & 0x40) && CH->block_fnum & 0x100) CH->kcode |= 1;
			CALC_FCSLOT(CH, &CH->SLOT[SLOT1]);
			CALC_FCSLOT(CH, &CH->SLOT[SLOT2]);
		}
		return;
	case 0xc0:
		/* FB,C */
		if ((r & 0x0f) > 8) return;
		CH = &OPL->P_CH[r & 0x0f];
		{
			int feedback = (v >> 1) & 7;
			CH->FB   = feedback ? (8 + 1) - feedback : 0;
			CH->CON = v & 1;
			set_algorythm(CH);
		}
		return;
	case 0xe0: /* wave type */
		slot = slot_array[r & 0x1f];
		if (slot == -1) return;
		CH = &OPL->P_CH[slot / 2];
		if (OPL->wavesel) {
			CH->SLOT[slot & 1].wavetable = &SIN_TABLE[(v & 0x03) * SIN_ENT];
		}
		return;
	}
}

/* lock/unlock for common table */
static int OPL_LockTable(void) {
	num_lock++;
	if (num_lock > 1) return 0;
	/* first time */
	cur_chip = NULL;
	/* allocate total level table (128kb space) */
	if (!OPLOpenTable()) {
		num_lock--;
		return -1;
	}
	return 0;
}

static void OPL_UnLockTable(void) {
	if (num_lock) num_lock--;
	if (num_lock) return;
	/* last time */
	cur_chip = NULL;
	OPLCloseTable();
}

/*******************************************************************************/
/*      YM3812 local section                                                   */
/*******************************************************************************/

/* ---------- update one of chip ----------- */
void YM3812UpdateOne_Mono(FM_OPL *OPL, int16 *buffer, int length) {
	int i;
	int data;
	int16 *buf = buffer;
	uint32 amsCnt  = OPL->amsCnt;
	uint32 vibCnt  = OPL->vibCnt;
	uint8 rythm = OPL->rythm & 0x20;
	OPL_CH *CH, *R_CH;

	if ((void *)OPL != cur_chip) {
		cur_chip = (void *)OPL;
		/* channel pointers */
		S_CH = OPL->P_CH;
		E_CH = &S_CH[9];
		/* rythm slot */
		SLOT7_1 = &S_CH[7].SLOT[SLOT1];
		SLOT7_2 = &S_CH[7].SLOT[SLOT2];
		SLOT8_1 = &S_CH[8].SLOT[SLOT1];
		SLOT8_2 = &S_CH[8].SLOT[SLOT2];
		/* LFO state */
		amsIncr = OPL->amsIncr;
		vibIncr = OPL->vibIncr;
		ams_table = OPL->ams_table;
		vib_table = OPL->vib_table;
	}
	R_CH = rythm ? &S_CH[6] : E_CH;
	for (i = 0; i < length ; i++) {
		/*            channel A         channel B         channel C      */
		/* LFO */
		ams = ams_table[(amsCnt += amsIncr) >> AMS_SHIFT];
		vib = vib_table[(vibCnt += vibIncr) >> VIB_SHIFT];
		outd[0] = 0;
		/* FM part */
		for (CH = S_CH ; CH < R_CH ; CH++)
			OPL_CALC_CH(CH);
		/* Rythn part */
		if (rythm)
			OPL_CALC_RH(S_CH);
		/* limit check */
		data = Limit(outd[0] , OPL_MAXOUT, OPL_MINOUT);
		/* store to sound buffer */
		buf[i] = data >> OPL_OUTSB;
	}

	OPL->amsCnt = amsCnt;
	OPL->vibCnt = vibCnt;
}

void YM3812UpdateOne_Stereo(FM_OPL *OPL, int16 *buffer, int length) {
	int i;
	int data;
	int left;
	int right;
	int16 *buf = buffer;
	uint32 amsCnt  = OPL->amsCnt;
	uint32 vibCnt  = OPL->vibCnt;
	uint8 rythm = OPL->rythm & 0x20;
	OPL_CH *CH, *R_CH;

	if ((void *)OPL != cur_chip) {
		cur_chip = (void *)OPL;
		/* channel pointers */
		S_CH = OPL->P_CH;
		E_CH = &S_CH[9];
		/* rythm slot */
		SLOT7_1 = &S_CH[7].SLOT[SLOT1];
		SLOT7_2 = &S_CH[7].SLOT[SLOT2];
		SLOT8_1 = &S_CH[8].SLOT[SLOT1];
		SLOT8_2 = &S_CH[8].SLOT[SLOT2];
		/* LFO state */
		amsIncr = OPL->amsIncr;
		vibIncr = OPL->vibIncr;
		ams_table = OPL->ams_table;
		vib_table = OPL->vib_table;
	}
	R_CH = rythm ? &S_CH[6] : E_CH;
	for (i = 0; i < length ; i++) {
		/*            channel A         channel B         channel C      */
		/* LFO */
		ams = ams_table[(amsCnt += amsIncr) >> AMS_SHIFT];
		vib = vib_table[(vibCnt += vibIncr) >> VIB_SHIFT];
		left = 0;
		right = 0;
		/* FM part */
		for (CH = S_CH ; CH < R_CH ; CH++) {
			outd[0] = 0;
			OPL_CALC_CH(CH);
			if (CH->PAN <= 64) left += outd[0];
			else left += (outd[0] >> 6) * (127 - CH->PAN);
			if (CH->PAN >= 64) right += outd[0];
			else right += (outd[0] >> 6) * (CH->PAN);
		}
		/* Rythn part */
		if (rythm) {
			outd[0] = 0;
			OPL_CALC_RH(S_CH);
			left += outd[0];
			right += outd[0];
		}
		/* limit check */
		data = Limit(left , OPL_MAXOUT, OPL_MINOUT);
		/* store to sound buffer */
		buf[i * 2] = data >> OPL_OUTSB;

		/* limit check */
		data = Limit(right , OPL_MAXOUT, OPL_MINOUT);
		/* store to sound buffer */
		buf[i * 2 + 1] = data >> OPL_OUTSB;
	}

	OPL->amsCnt = amsCnt;
	OPL->vibCnt = vibCnt;
}

/* ---------- reset a chip ---------- */
void OPLResetChip(FM_OPL *OPL) {
	int c, s;
	int i;

	/* reset chip */
	OPL->mode   = 0;    /* normal mode */
	OPL_STATUS_RESET(OPL, 0x7f);
	/* reset with register write */
	OPLWriteReg(OPL, 0x01, 0); /* wabesel disable */
	OPLWriteReg(OPL, 0x02, 0); /* Timer1 */
	OPLWriteReg(OPL, 0x03, 0); /* Timer2 */
	OPLWriteReg(OPL, 0x04, 0); /* IRQ mask clear */
	for (i = 0xff ; i >= 0x20 ; i--) OPLWriteReg(OPL, i, 0);
	/* reset OPerator paramater */
	for (c = 0 ; c < OPL->max_ch ; c++) {
		OPL_CH *CH = &OPL->P_CH[c];
		OPL->P_CH[c].PAN = 64;
		for (s = 0 ; s < 2 ; s++) {
			/* wave table */
			CH->SLOT[s].wavetable = &SIN_TABLE[0];
			/* CH->SLOT[s].evm = ENV_MOD_RR; */
			CH->SLOT[s].evc = EG_OFF;
			CH->SLOT[s].eve = EG_OFF + 1;
			CH->SLOT[s].evs = 0;
		}
	}
}

/* ----------  Create a virtual YM3812 ----------       */
/* 'rate'  is sampling rate and 'bufsiz' is the size of the  */
FM_OPL *OPLCreate(int type, int clock, int rate) {
	char *ptr;
	FM_OPL *OPL;
	int state_size;
	int max_ch = 9; /* normaly 9 channels */

	if (OPL_LockTable() == -1) return NULL;
	/* allocate OPL state space */
	state_size  = sizeof(FM_OPL);
	state_size += sizeof(OPL_CH) * max_ch;

	/* allocate memory block */
	ptr = (char *)malloc(state_size);
	if (ptr == NULL) return NULL;

	/* clear */
	memset(ptr, 0, state_size);
	OPL        = (FM_OPL *)ptr;
	ptr += sizeof(FM_OPL);
	OPL->P_CH  = (OPL_CH *)ptr;
	ptr += sizeof(OPL_CH) * max_ch;

	/* set channel state pointer */
	OPL->type  = type;
	OPL->clock = clock;
	OPL->rate  = rate;
	OPL->max_ch = max_ch;

	/* init grobal tables */
	OPL_initalize(OPL);

	/* reset chip */
	OPLResetChip(OPL);
	return OPL;
}

/* ----------  Destroy one of vietual YM3812 ----------       */
void OPLDestroy(FM_OPL *OPL) {
	OPL_UnLockTable();
	free(OPL);
}

/* ----------  Option handlers ----------       */

void OPLSetTimerHandler(FM_OPL *OPL, OPL_TIMERHANDLER TimerHandler, int channelOffset) {
	OPL->TimerHandler   = TimerHandler;
	OPL->TimerParam = channelOffset;
}
void OPLSetIRQHandler(FM_OPL *OPL, OPL_IRQHANDLER IRQHandler, int param) {
	OPL->IRQHandler     = IRQHandler;
	OPL->IRQParam = param;
}
void OPLSetUpdateHandler(FM_OPL *OPL, OPL_UPDATEHANDLER UpdateHandler, int param) {
	OPL->UpdateHandler = UpdateHandler;
	OPL->UpdateParam = param;
}

/* ---------- YM3812 I/O interface ---------- */
int OPLWrite(FM_OPL *OPL, int a, int v) {
	if (!(a & 1)) {
		/* address port */
		OPL->address = v & 0xff;
	} else {
		/* data port */
		if (OPL->UpdateHandler) OPL->UpdateHandler(OPL->UpdateParam, 0);
		OPLWriteReg(OPL, OPL->address, v);
	}
	return OPL->status >> 7;
}

unsigned char OPLRead(FM_OPL *OPL, int a) {
	if (!(a & 1)) {
		/* status port */
		return OPL->status & (OPL->statusmask | 0x80);
	}
	/* data port */
	switch (OPL->address) {
	case 0x05: /* KeyBoard IN */
		PERR(("OPL:read unmapped KEYBOARD port\n"));
		return 0;
	case 0x19: /* I/O DATA    */
		PERR(("OPL:read unmapped I/O port\n"));
		return 0;
	case 0x1a: /* PCM-DATA    */
		return 0;
	}
	return 0;
}

int OPLTimerOver(FM_OPL *OPL, int c) {
	if (c) {
		/* Timer B */
		OPL_STATUS_SET(OPL, 0x20);
	} else {
		/* Timer A */
		OPL_STATUS_SET(OPL, 0x40);
		/* CSM mode key,TL controll */
		if (OPL->mode & 0x80) {
			/* CSM mode total level latch and auto key on */
			int ch;
			if (OPL->UpdateHandler) OPL->UpdateHandler(OPL->UpdateParam, 0);
			for (ch = 0; ch < 9; ch++)
				CSMKeyControll(&OPL->P_CH[ch]);
		}
	}
	/* reload timer */
	if (OPL->TimerHandler)(OPL->TimerHandler)(OPL->TimerParam + c, (double)OPL->T[c]*OPL->TimerBase);
	return OPL->status >> 7;
}

void OPLSetPan(FM_OPL *OPL, int c, int pan) {
	if (c > 0 && c < OPL->max_ch)
		OPL->P_CH[c].PAN = pan;
}

} // End of namespace FMOpl_Pentagram
} // End of namespace Ultima8

#endif

