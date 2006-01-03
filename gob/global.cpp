/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2005 The ScummVM project
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

Global::Global(GobEngine *vm) : _vm(vm) {
	useMouse = UNDEF;
	mousePresent = UNDEF;

	presentCGA = UNDEF;
	presentEGA = UNDEF;
	presentVGA = UNDEF;
	presentHER = UNDEF;

	videoMode = 0;

	/* Sound */
	presentSound = 0x8000;	/* undefined values */
	soundFlags = 0x8000;
	blasterPort = 0;
	disableSoundCfg = 0;

	//char playingSound = 0;

	/* Mouse */
	disableMouseCfg = 0;

	mouseXShift = 3;
	mouseYShift = 3;

	mouseMaxCol = 320;
	mouseMaxRow = 200;

	/* Language */
	disableLangCfg = 0x8000;
	language = 0x8000;

	/* Timer variables */
	startTime = 0;
	timer_delta = 1000;

	frameWaitTime = 0;
	startFrameTime = 0;

	/* Timer and delays */
	delayTime = 0;

	/* Joystick */
	useJoystick = 1;

	/* Data files */
	packedSize = 0;
	for (int i = 0; i < MAX_DATA_FILES; i++) {
		dataFiles[i] = 0;
		numDataChunks[i] = 0;
		dataFileHandles[i] = -1;
	}

	primaryWidth = 0;
	primaryHeight = 0;

	sprAllocated = 0;

	doRangeClamp = 0;

	setAllPalette = 0;

	oldMode = 3;
	dontSetPalette = 0;
	curPrimaryDesc = 0;
	allocatedPrimary = 0;
	pPrimarySurfDesc = 0;

	pPaletteDesc = 0;

	unusedPalette1[0] = (int16)0;
	unusedPalette1[1] = (int16)0x0b;
	unusedPalette1[2] = (int16)0;
	unusedPalette1[3] = (int16)0x5555;
	unusedPalette1[4] = (int16)0xAAAA;
	unusedPalette1[5] = (int16)0xFFFF;
	unusedPalette1[6] = (int16)0;
	unusedPalette1[7] = (int16)0x5555;
	unusedPalette1[8] = (int16)0xAAAA;
	unusedPalette1[9] = (int16)0xFFFF;
	unusedPalette1[10] = (int16)0;
	unusedPalette1[11] = (int16)0x5555;
	unusedPalette1[12] = (int16)0xAAAA;
	unusedPalette1[13] = (int16)0xFFFF;
	unusedPalette1[14] = (int16)0;
	unusedPalette1[15] = (int16)0x5555;
	unusedPalette1[16] = (int16)0xAAAA;
	unusedPalette1[17] = (int16)0xFFFF;

	for (int i = 0; i < 16 ;i++)
		unusedPalette2[i] = i;

	vgaPalette[0].red = 0x00; vgaPalette[0].green = 0x00; vgaPalette[0].blue = 0x00;
	vgaPalette[1].red = 0x00; vgaPalette[1].green = 0x00; vgaPalette[1].blue = 0x2a;
	vgaPalette[2].red = 0x00; vgaPalette[2].green = 0x2a; vgaPalette[2].blue = 0x00;
	vgaPalette[3].red = 0x00; vgaPalette[3].green = 0x2a; vgaPalette[3].blue = 0x2a;
	vgaPalette[4].red = 0x2a; vgaPalette[4].green = 0x00; vgaPalette[4].blue = 0x00;
	vgaPalette[5].red = 0x2a; vgaPalette[5].green = 0x00; vgaPalette[5].blue = 0x2a;
	vgaPalette[6].red = 0x2a; vgaPalette[6].green = 0x15; vgaPalette[6].blue = 0x00;
	vgaPalette[7].red = 0x2a; vgaPalette[7].green = 0x2a; vgaPalette[7].blue = 0x2a;
	vgaPalette[8].red = 0x15; vgaPalette[8].green = 0x15; vgaPalette[8].blue = 0x15;
	vgaPalette[9].red = 0x15; vgaPalette[9].green = 0x15; vgaPalette[9].blue = 0x3f;
	vgaPalette[10].red = 0x15; vgaPalette[10].green = 0x3f; vgaPalette[10].blue = 0x15;
	vgaPalette[11].red = 0x15; vgaPalette[11].green = 0x3f; vgaPalette[11].blue = 0x3f;
	vgaPalette[12].red = 0x3f; vgaPalette[12].green = 0x15; vgaPalette[12].blue = 0x15;
	vgaPalette[13].red = 0x3f; vgaPalette[13].green = 0x15; vgaPalette[13].blue = 0x3f;
	vgaPalette[14].red = 0x3f; vgaPalette[14].green = 0x3f; vgaPalette[14].blue = 0x15;
	vgaPalette[15].red = 0x3f; vgaPalette[15].green = 0x3f; vgaPalette[15].blue = 0x3f;

	debugFlag = 0;
	inVM = 0;
	colorCount = 16;

	inter_resStr[0] = 0;
	inter_resVal = 0;

	inter_variables = 0;
	inter_execPtr = 0;
	inter_animDataSize = 10;

	inter_mouseX = 0;
	inter_mouseY = 0;

	tmpPalBuffer = 0;
}

} // End of namespace Gob
