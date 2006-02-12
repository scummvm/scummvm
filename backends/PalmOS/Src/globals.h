/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2002-2006 Chris Apers - PalmOS Backend
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

#ifndef GLOBALS_H
#define GLOBALS_H

#include <VFSMgr.h>
#include "stuffs.h"

#ifdef PALMOS_68K

#include "scumm_globals.h"

enum {
	kMemScummOldCostGames = 0,
	kMemScummNewCostGames,
	kMemSimon1Games,	
	kMemSimon2Games,

	kMemGamesCount
};

#endif

enum {
	INIT_VIBRATOR	= 1 <<	0x00,
	INIT_PA1LIB		= 1 <<	0x01,
	INIT_ARM		= 1 <<	0x02,
	INIT_AUTOOFF	= 1 <<	0x03,
	INIT_GOLCD		= 1 <<	0x04
};

enum {
	FM_QUALITY_LOW = 0,
	FM_QUALITY_MED,
	FM_QUALITY_HI,
	FM_QUALITY_INI
};

typedef struct {
	// common parts
	UInt32 _4B, _2B;

	// 4 bytes part
	UInt32 startupMemory;
	UInt32 slkVersion;
	UInt32 options;
	UInt32 screenPitch;

	struct {
		FileRef	logFile;
		UInt32 cacheSize;
		UInt16 volRefNum;
		UInt16 dummy;
	} VFS;

	// 2 bytes part
	UInt16 HRrefNum;
	UInt16 slkRefNum;
	Coord screenWidth, screenHeight;			// silkarea shown
	Coord screenFullWidth, screenFullHeight;	// silkarea hidden
	Int16 autoSave;
	struct {
		Int16 on;
		Int16 off;
		Int16 showLED;
	} indicator;

	// 1 byte part
	Boolean vibrator;
	Boolean stdPalette;
	Boolean filter;
	Boolean stylusClick;
	UInt8 init;
	UInt8 palmVolume;
	UInt8 fmQuality;
	UInt8 advancedMode;

#ifdef PALMOS_68K
	// 68k only part
	struct {
		Boolean enable;
		UInt8 driver, format;
		UInt16 defaultTrackLength;
		UInt16 firstTrack;
		UInt8 volume;		
	} CD;

	DmOpenRef globals[GBVARS_COUNT];
	UInt32 memory[kMemGamesCount];
#endif

} GlobalsDataType, *GlobalsDataPtr;

extern GlobalsDataPtr gVars;

#define VARS_EXPORT()		gVars->_4B = 6; \
							gVars->_2B = 12;

#define DO_VARS(z, t, o) \
	{	Int8 *tmp = (Int8 *)gVars + o + 8; \
		for (Int8 cnt = 0; cnt < gVars->z; cnt++) \
			{ 	UInt##t val = *((UInt##t *)tmp);	\
				val = ByteSwap##t(val);	\
				*((UInt##t *)tmp) = val;	\
				tmp += (t / 8);	\
			}	\
	}

#define OPTIONS_DEF()		gVars->options

#define HWR_INIT(x)			(gVars->init & (x))
#define HWR_SET(x)			gVars->init |= (x)
#define HWR_RST(x)			gVars->init &= ~(x)
#define HWR_RSTALL()		gVars->init = 0
#define HWR_GET()			(gVars->init)

#define ARM(x)	gVars->arm[x]

#endif
