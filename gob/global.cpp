/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */
#include "gob/gob.h"
#include "gob/global.h"

namespace Gob {

char pressedKeys[128];

char useMouse = UNDEF;
int16 mousePresent = UNDEF;

int16 presentCGA = UNDEF;
int16 presentEGA = UNDEF;
int16 presentVGA = UNDEF;
int16 presentHER = UNDEF;

int16 videoMode = 0;

int16 disableVideoCfg;

/* Sound */
uint16 presentSound = 0x8000;	/* undefined values */
uint16 soundFlags = 0x8000;
int16 blasterPort = 0;
int16 disableSoundCfg = 0;

//char playingSound = 0;

/* Mouse */
int16 disableMouseCfg = 0;

int16 mouseXShift = 3;
int16 mouseYShift = 3;

int16 mouseMaxCol = 320;
int16 mouseMaxRow = 200;

/* Language */
uint16 disableLangCfg = 0x8000;
uint16 language = 0x8000;

/* Timer variables */
int32 startTime = 0;
int16 timer_delta = 1000;

int16 frameWaitTime = 0;
int32 startFrameTime = 0;

/* Timer and delays */
int16 delayTime = 0;

/* Joystick */
char useJoystick = 1;

/* Files */
Common::File filesHandles[MAX_FILES];

/* Data files */
struct ChunkDesc *dataFiles[MAX_DATA_FILES];
int16 numDataChunks[MAX_DATA_FILES];
int16 dataFileHandles[MAX_DATA_FILES];
int32 chunkPos[MAX_SLOT_COUNT * MAX_DATA_FILES];
int32 chunkOffset[MAX_SLOT_COUNT * MAX_DATA_FILES];
int32 chunkSize[MAX_SLOT_COUNT * MAX_DATA_FILES];
char isCurrentSlot[MAX_SLOT_COUNT * MAX_DATA_FILES];
int32 packedSize = 0;

int16 sprAllocated = 0;

SurfaceDesc primarySurfDesc;
SurfaceDesc *pPrimarySurfDesc;

int16 primaryWidth;
int16 primaryHeight;

int16 doRangeClamp = 0;

char redPalette[256];
char greenPalette[256];
char bluePalette[256];

int16 setAllPalette = 0;

int16 oldMode = 3;
char dontSetPalette = 0;
SurfaceDesc *curPrimaryDesc = 0;
SurfaceDesc *allocatedPrimary = 0;

PalDesc *pPaletteDesc = 0;

int16 unusedPalette1[18] = {
	0, 0x0b, 0, (int16)0x5555, (int16)0xAAAA, (int16)0xFFFF, 0, (int16)0x5555,
	(int16)0xAAAA, (int16)0xFFFF, 0, (int16)0x5555,
	(int16)0xAAAA, (int16)0xFFFF, 0, (int16)0x5555, (int16)0xAAAA, (int16)0xFFFF
};

int16 unusedPalette2[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

Color vgaPalette[16] = {
	{0x00, 0x00, 0x00},
	{0x00, 0x00, 0x2a},
	{0x00, 0x2a, 0x00},
	{0x00, 0x2a, 0x2a},
	{0x2a, 0x00, 0x00},
	{0x2a, 0x00, 0x2a},
	{0x2a, 0x15, 0x00},
	{0x2a, 0x2a, 0x2a},
	{0x15, 0x15, 0x15},
	{0x15, 0x15, 0x3f},
	{0x15, 0x3f, 0x15},
	{0x15, 0x3f, 0x3f},
	{0x3f, 0x15, 0x15},
	{0x3f, 0x15, 0x3f},
	{0x3f, 0x3f, 0x15},
	{0x3f, 0x3f, 0x3f}
};

PalDesc paletteStruct;

int16 debugFlag = 0;
int16 inVM = 0;
int16 colorCount = 16;

char inter_resStr[200];
int32 inter_resVal = 0;

char *inter_variables = 0;
char *inter_execPtr = 0;
int16 inter_animDataSize = 10;

int16 inter_mouseX = 0;
int16 inter_mouseY = 0;

char *tmpPalBuffer = 0;

} // End of namespace Gob
