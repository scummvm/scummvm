/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
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
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include <VFSMgr.h>
#include "scumm_globals.h"

enum {
	kOptNone					=	0,
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
	
	kOptDeviceProcX86		=	1 <<	0x1F	// DEBUG only
};

enum {
	kMemScummOldCostGames = 0,
	kMemScummNewCostGames,
	kMemSimon1Games,	
	kMemSimon2Games,

	kMemGamesCount
};

typedef struct {
	DmOpenRef globals[GBVARS_COUNT];
	UInt32 memory[kMemGamesCount];

	UInt32 options;

	UInt16 HRrefNum;
	UInt16 volRefNum;
	UInt16 slkRefNum;
	UInt32 slkVersion;
	Boolean skinSet;
	Boolean pinUpdate;

	FileRef	logFile;

	Boolean vibrator;
	Boolean autoReset;
	Boolean screenLocked;
	Boolean stdPalette;
	Coord screenWidth, screenHeight;			// silkarea shown
	Coord screenFullWidth, screenFullHeight;	// silkarea hidden
	UInt32 screenPitch;

	struct {
		UInt8 on;
		UInt8 off;
	} indicator;

	struct {
		UInt8 *pageAddr1;
		UInt8 *pageAddr2;
	} flipping;
	
	struct {
		Boolean MP3;
		Boolean setDefaultTrackLength;
		UInt16 defaultTrackLength;
		UInt16 firstTrack;
	} music;

} GlobalsDataType, *GlobalsDataPtr;

extern GlobalsDataPtr gVars;

#define OPTIONS_TST(x)	(gVars->options & (x))
#define OPTIONS_SET(x)	gVars->options |= (x)
#define OPTIONS_RST(x)	gVars->options &= ~(x)

#endif
