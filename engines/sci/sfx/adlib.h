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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */


#ifndef _SFX_ADLIB_H_
#define _SFX_ADLIB_H_

#include "sci/include/resource.h"


#define ADLIB_VOICES 12

typedef struct _sci_adlib_def {
	guint8 keyscale1;       /* 0-3 !*/
	guint8 freqmod1;        /* 0-15 !*/
	guint8 feedback1;       /* 0-7 !*/
	guint8 attackrate1;     /* 0-15 !*/
	guint8 sustainvol1;     /* 0-15 !*/
	guint8 envelope1;       /* t/f !*/
	guint8 decayrate1;      /* 0-15 !*/
	guint8 releaserate1;    /* 0-15 !*/
	guint8 volume1;         /* 0-63 !*/
	guint8 ampmod1;         /* t/f !*/
	guint8 vibrato1;        /* t/f !*/
	guint8 keybdscale1;     /* t/f !*/
	guint8 algorithm1;      /* 0,1 REVERSED */
	guint8 keyscale2;       /* 0-3 !*/
	guint8 freqmod2;        /* 0-15 !*/
	guint8 feedback2;       /* 0-7 UNUSED */
	guint8 attackrate2;     /* 0-15 !*/
	guint8 sustainvol2;     /* 0-15 !*/
	guint8 envelope2;       /* t/f !*/
	guint8 decayrate2;      /* 0-15 !*/
	guint8 releaserate2;    /* 0-15 !*/
	guint8 volume2;         /* 0-63 !*/
	guint8 ampmod2;         /* t/f !*/
	guint8 vibrato2;        /* t/f !*/
	guint8 keybdscale2;     /* t/f !*/
	guint8 algorithm2;      /* 0,1 UNUSED */
	guint8 waveform1;       /* 0-3 !*/
	guint8 waveform2;       /* 0-3 !*/
} adlib_def;

typedef unsigned char adlib_instr[12];

extern adlib_instr adlib_sbi[96];

void make_sbi(adlib_def *one, guint8 *buffer);
/* Converts a raw SCI adlib instrument into the adlib register format. */

#endif /* _SFX_ADLIB_H_ */
