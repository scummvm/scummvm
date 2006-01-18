/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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
 * $Header$
 *
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include <VFSMgr.h>
#include "scumm_globals.h"
#include "arm/pnodefs.h"

enum {
	kOptNone				=	0,
	kOptDeviceARM			=	1 <<	0x00,
	kOptDeviceOS5			=	1 <<	0x01,
	kOptDeviceClie			=	1 <<	0x02,
	kOptDeviceZodiac		=	1 <<	0x03,
	kOptModeWide			=	1 <<	0x04,
	kOptModeLandscape		=	1 <<	0x05,
	kOptMode16Bit			=	1 <<	0x06,
	kOptModeHiDensity		=	1 <<	0x07,
	kOptCollapsible			=	1 <<	0x08,
	kOptDisableOnScrDisp	=	1 <<	0x09,
	kOpt5WayNavigator		=	1 <<	0x0A,
	kOptPalmSoundAPI		=	1 <<	0x0B,
	kOptSonyPa1LibAPI		=	1 <<	0x0C,

	kOptDeviceProcX86		=	1 <<	0x1F	// DEBUG only
};

enum {
	kMemScummOldCostGames = 0,
	kMemScummNewCostGames,
	kMemSimon1Games,
	kMemSimon2Games,

	kMemGamesCount
};

enum {
	INIT_VIBRATOR	= 1 <<	0x00,
	INIT_PA1LIB		= 1 <<	0x01,
	INIT_ARM		= 1 <<	0x02,
	INIT_AUTOOFF	= 1 <<	0x03
};

typedef struct {
	char headerBuffer[sizeof(PnoEntryHeader) + 2];
	PnoEntryHeader *alignedHeader;
	PnoDescriptor pnoDesc;
} PNOInitType;

typedef struct {
	DmOpenRef globals[GBVARS_COUNT];
	UInt32 memory[kMemGamesCount];

	UInt8 init;
	UInt32 options;

	UInt16 HRrefNum;
	UInt16 volRefNum;
	UInt16 slkRefNum;
	UInt32 slkVersion;

	FileRef	logFile;

	Boolean vibrator;
	Boolean screenLocked;
	Boolean stdPalette;
	Boolean filter;
	Coord screenWidth, screenHeight;			// silkarea shown
	Coord screenFullWidth, screenFullHeight;	// silkarea hidden
	UInt32 screenPitch;

	PNOInitType arm[ARM_COUNT];

	struct {
		UInt8 on;
		UInt8 off;
	} indicator;

	struct {
		UInt8 *pageAddr1;
		UInt8 *pageAddr2;
	} flipping;

	struct {
		Boolean enable;
		UInt8 driver, format;
		UInt16 defaultTrackLength;
		UInt16 firstTrack;
		UInt16 volume;
	} CD;

} GlobalsDataType, *GlobalsDataPtr;

extern GlobalsDataPtr gVars;

#define OPTIONS_TST(x)	(gVars->options & (x))
#define OPTIONS_SET(x)	gVars->options |= (x)
#define OPTIONS_RST(x)	gVars->options &= ~(x)

#define HWR_INIT(x)			(gVars->init & (x))
#define HWR_SET(x)			gVars->init |= (x)
#define HWR_RST(x)			gVars->init &= ~(x)
#define HWR_RSTALL()		gVars->init = 0
#define HWR_GET()			(gVars->init)

#define ARM(x)	gVars->arm[x]

#endif
