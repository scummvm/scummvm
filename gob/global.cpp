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

Global::Global(GobEngine *vm) : _vm(vm) {
	_useMouse = UNDEF;
	_mousePresent = UNDEF;

	_presentCGA = UNDEF;
	_presentEGA = UNDEF;
	_presentVGA = UNDEF;
	_presentHER = UNDEF;

	_videoMode = 0;

	/* Sound */
	_presentSound = 0x8000;	/* undefined values */
	_soundFlags = 0x8000;
	_blasterPort = 0;
	_disableSoundCfg = 0;

	//char _playingSound = 0;

	/* Mouse */
	_disableMouseCfg = 0;

	_mouseXShift = 3;
	_mouseYShift = 3;

	_mouseMaxCol = 320;
	_mouseMaxRow = 200;

	/* Language */
	_disableLangCfg = 0x8000;
	_language = 0x8000;

	/* Timer variables */
	_startTime = 0;
	_timer_delta = 1000;

	_frameWaitTime = 0;
	_startFrameTime = 0;

	/* Timer and delays */
	_delayTime = 0;

	/* Joystick */
	_useJoystick = 1;

	/* Data files */
	_packedSize = 0;
	int i;

	for (i = 0; i < MAX_DATA_FILES; i++) {
		_dataFiles[i] = 0;
		_numDataChunks[i] = 0;
		_dataFileHandles[i] = -1;
	}

	_primaryWidth = 0;
	_primaryHeight = 0;

	_sprAllocated = 0;

	_doRangeClamp = 0;

	_setAllPalette = 0;

	_oldMode = 3;
	_dontSetPalette = 0;
	_pPrimarySurfDesc = 0;

	_pPaletteDesc = 0;

	_unusedPalette1[0] = (int16)0;
	_unusedPalette1[1] = (int16)0x0b;
	_unusedPalette1[2] = (int16)0;
	_unusedPalette1[3] = (int16)0x5555;
	_unusedPalette1[4] = (int16)0xAAAA;
	_unusedPalette1[5] = (int16)0xFFFF;
	_unusedPalette1[6] = (int16)0;
	_unusedPalette1[7] = (int16)0x5555;
	_unusedPalette1[8] = (int16)0xAAAA;
	_unusedPalette1[9] = (int16)0xFFFF;
	_unusedPalette1[10] = (int16)0;
	_unusedPalette1[11] = (int16)0x5555;
	_unusedPalette1[12] = (int16)0xAAAA;
	_unusedPalette1[13] = (int16)0xFFFF;
	_unusedPalette1[14] = (int16)0;
	_unusedPalette1[15] = (int16)0x5555;
	_unusedPalette1[16] = (int16)0xAAAA;
	_unusedPalette1[17] = (int16)0xFFFF;

	for (i = 0; i < 16 ;i++)
		_unusedPalette2[i] = i;

	_vgaPalette[0].red = 0x00; _vgaPalette[0].green = 0x00; _vgaPalette[0].blue = 0x00;
	_vgaPalette[1].red = 0x00; _vgaPalette[1].green = 0x00; _vgaPalette[1].blue = 0x2a;
	_vgaPalette[2].red = 0x00; _vgaPalette[2].green = 0x2a; _vgaPalette[2].blue = 0x00;
	_vgaPalette[3].red = 0x00; _vgaPalette[3].green = 0x2a; _vgaPalette[3].blue = 0x2a;
	_vgaPalette[4].red = 0x2a; _vgaPalette[4].green = 0x00; _vgaPalette[4].blue = 0x00;
	_vgaPalette[5].red = 0x2a; _vgaPalette[5].green = 0x00; _vgaPalette[5].blue = 0x2a;
	_vgaPalette[6].red = 0x2a; _vgaPalette[6].green = 0x15; _vgaPalette[6].blue = 0x00;
	_vgaPalette[7].red = 0x2a; _vgaPalette[7].green = 0x2a; _vgaPalette[7].blue = 0x2a;
	_vgaPalette[8].red = 0x15; _vgaPalette[8].green = 0x15; _vgaPalette[8].blue = 0x15;
	_vgaPalette[9].red = 0x15; _vgaPalette[9].green = 0x15; _vgaPalette[9].blue = 0x3f;
	_vgaPalette[10].red = 0x15; _vgaPalette[10].green = 0x3f; _vgaPalette[10].blue = 0x15;
	_vgaPalette[11].red = 0x15; _vgaPalette[11].green = 0x3f; _vgaPalette[11].blue = 0x3f;
	_vgaPalette[12].red = 0x3f; _vgaPalette[12].green = 0x15; _vgaPalette[12].blue = 0x15;
	_vgaPalette[13].red = 0x3f; _vgaPalette[13].green = 0x15; _vgaPalette[13].blue = 0x3f;
	_vgaPalette[14].red = 0x3f; _vgaPalette[14].green = 0x3f; _vgaPalette[14].blue = 0x15;
	_vgaPalette[15].red = 0x3f; _vgaPalette[15].green = 0x3f; _vgaPalette[15].blue = 0x3f;

	_debugFlag = 0;
	_inVM = 0;
	_colorCount = 16;

	_inter_resStr[0] = 0;
	_inter_resVal = 0;

	_inter_variables = 0;
	_inter_execPtr = 0;
	_inter_animDataSize = 10;

	_inter_mouseX = 0;
	_inter_mouseY = 0;
}

} // End of namespace Gob
