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
	optNone					=	0,
	optIsARMDevice			=	1 <<	0,
	optIsOS5Device			=	1 <<	1,
	optIsClieDevice			=	1 <<	2,
	optIsZodiacDevice		=	1 <<	3,
	optIsCollapsible		=	1 <<	4,
	optHasWideMode			=	1 <<	5,
	optIsLandscapeDisplay	=	1 <<	6,	// screen pitch is always based on Landscape mode (portrait/landscape)
	optHasLandscapeMode		=	1 <<	7,	// screen pitch can be based on portrait or landscape
	optHas16BitMode			=	1 <<	8,
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
		Boolean MP3;
		Boolean setDefaultTrackLength;
		UInt16 defaultTrackLength;
		UInt16 firstTrack;
	} music;

} GlobalsDataType, *GlobalsDataPtr;

extern GlobalsDataPtr gVars;

#define OPTIONS(x)	(gVars->options & (x))

#endif
