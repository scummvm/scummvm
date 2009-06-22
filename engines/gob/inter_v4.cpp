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

#include "common/endian.h"
#include "common/file.h"

#include "gob/gob.h"
#include "gob/inter.h"
#include "gob/helper.h"
#include "gob/global.h"
#include "gob/draw.h"
#include "gob/game.h"
#include "gob/parse.h"
#include "gob/videoplayer.h"
#include "gob/sound/sound.h"

namespace Gob {

#define OPCODEVER Inter_v4
#define OPCODEDRAW(i, x)  _opcodesDraw[i]._OPCODEDRAW(OPCODEVER, x)
#define OPCODEFUNC(i, x)  _opcodesFunc[i]._OPCODEFUNC(OPCODEVER, x)
#define OPCODEGOB(i, x)   _opcodesGob[i]._OPCODEGOB(OPCODEVER, x)

Inter_v4::Inter_v4(GobEngine *vm) : Inter_v3(vm) {
}

void Inter_v4::setupOpcodesDraw() {
	Inter_v3::setupOpcodesDraw();

	OPCODEDRAW(0x80, o4_initScreen);
	OPCODEDRAW(0x83, o4_playVmdOrMusic);
}

void Inter_v4::setupOpcodesFunc() {
	Inter_v3::setupOpcodesFunc();
}

void Inter_v4::setupOpcodesGob() {
}

void Inter_v4::o4_initScreen() {
	int16 offY;
	int16 videoMode;
	int16 width, height;

	offY = load16();

	videoMode = offY & 0xFF;
	offY = (offY >> 8) & 0xFF;

	width = _vm->_parse->parseValExpr();
	height = _vm->_parse->parseValExpr();

	_vm->_video->clearScreen();

	_vm->_global->_fakeVideoMode = videoMode;

	// Some versions require this
	if (videoMode == 0xD)
		videoMode = _vm->_mode;

	if ((videoMode == _vm->_global->_videoMode) && (width == -1))
		return;

	if (width > 0)
		_vm->_video->_surfWidth = width;
	if (height > 0)
		_vm->_video->_surfHeight = height;

	_vm->_video->_screenDeltaX = 0;
	if (_vm->_video->_surfWidth < _vm->_width)
		_vm->_video->_screenDeltaX = (_vm->_width - _vm->_video->_surfWidth) / 2;

	_vm->_global->_mouseMinX = _vm->_video->_screenDeltaX;
	_vm->_global->_mouseMaxX = _vm->_video->_screenDeltaX + _vm->_video->_surfWidth - 1;

	_vm->_video->_splitStart = _vm->_video->_surfHeight - offY;

	_vm->_video->_splitHeight1 = MIN<int16>(_vm->_height, _vm->_video->_surfHeight);
	_vm->_video->_splitHeight2 = offY;

	if ((_vm->_video->_surfHeight + offY) < _vm->_height)
		_vm->_video->_screenDeltaY = (_vm->_height - (_vm->_video->_surfHeight + offY)) / 2;
	else
		_vm->_video->_screenDeltaY = 0;

	_vm->_global->_mouseMaxY = (_vm->_video->_surfHeight + _vm->_video->_screenDeltaY) - offY - 1;
	_vm->_global->_mouseMinY = _vm->_video->_screenDeltaY;

	_vm->_draw->closeScreen();
	_vm->_util->clearPalette();
	memset(_vm->_global->_redPalette, 0, 256);
	memset(_vm->_global->_greenPalette, 0, 256);
	memset(_vm->_global->_bluePalette, 0, 256);

	_vm->_video->_splitSurf.reset();
	_vm->_draw->_spritesArray[24].reset();
	_vm->_draw->_spritesArray[25].reset();

	_vm->_global->_videoMode = videoMode;
	_vm->_video->initPrimary(videoMode);
	WRITE_VAR(15, _vm->_global->_fakeVideoMode);

	_vm->_global->_setAllPalette = true;

	_vm->_util->setMousePos(_vm->_global->_inter_mouseX,
			_vm->_global->_inter_mouseY);
	_vm->_util->clearPalette();

	_vm->_draw->initScreen();

	_vm->_util->setScrollOffset();

	if (offY > 0) {
		_vm->_draw->_spritesArray[24] = SurfaceDescPtr(new SurfaceDesc(videoMode, _vm->_width, offY));
		_vm->_draw->_spritesArray[25] = SurfaceDescPtr(new SurfaceDesc(videoMode, _vm->_width, offY));
		_vm->_video->_splitSurf = _vm->_draw->_spritesArray[25];
	}
}

void Inter_v4::o4_playVmdOrMusic() {
	char fileName[128];
	int16 x, y;
	int16 startFrame;
	int16 lastFrame;
	int16 breakKey;
	int16 flags;
	int16 palStart;
	int16 palEnd;
	uint16 palCmd;
	bool close;

	evalExpr(0);
	strncpy0(fileName, _vm->_parse->getResultStr(), 127);

	// WORKAROUND: The nut rolling animation in the administration center
	// in Woodruff is called "noixroul", but the scripts think it's "noixroule".
	if ((_vm->getGameType() == kGameTypeWoodruff) &&
			(!scumm_stricmp(fileName, "noixroule")))
		strcpy(fileName, "noixroul");

	x = _vm->_parse->parseValExpr();
	y = _vm->_parse->parseValExpr();
	startFrame = _vm->_parse->parseValExpr();
	lastFrame = _vm->_parse->parseValExpr();
	breakKey = _vm->_parse->parseValExpr();
	flags = _vm->_parse->parseValExpr();
	palStart = _vm->_parse->parseValExpr();
	palEnd = _vm->_parse->parseValExpr();
	palCmd = 1 << (flags & 0x3F);

	debugC(1, kDebugVideo, "Playing video \"%s\" @ %d+%d, frames %d - %d, "
			"paletteCmd %d (%d - %d), flags %X", fileName, x, y, startFrame, lastFrame,
			palCmd, palStart, palEnd, flags);

	close = false;
	if (lastFrame == -1) {
		close = true;
	} else if (lastFrame == -2) {
	} else if (lastFrame == -3) {

		_vm->_mult->_objects[startFrame].pAnimData->animation = -startFrame - 1;

		if (_vm->_mult->_objects[startFrame].videoSlot > 0)
			_vm->_vidPlayer->slotClose(_vm->_mult->_objects[startFrame].videoSlot - 1);

		int slot = _vm->_vidPlayer->slotOpen(fileName);

		_vm->_mult->_objects[startFrame].videoSlot = slot + 1;

		if (x == -1) {
			*_vm->_mult->_objects[startFrame].pPosX = _vm->_vidPlayer->getDefaultX(slot);
			*_vm->_mult->_objects[startFrame].pPosY = _vm->_vidPlayer->getDefaultY(slot);
		} else {
			*_vm->_mult->_objects[startFrame].pPosX = x;
			*_vm->_mult->_objects[startFrame].pPosY = y;
		}

		return;
	} else if (lastFrame == -4) {
		warning("Woodruff Stub: Video/Music command -4: Play background video %s", fileName);
		return;
	} else if (lastFrame == -5) {
		_vm->_sound->bgStop();
		return;
	} else if (lastFrame == -6) {
		return;
	} else if (lastFrame == -7) {
		return;
	} else if (lastFrame == -8) {
		warning("Woodruff Stub: Video/Music command -8: Play background video %s", fileName);
		return;
	} else if (lastFrame == -9) {
		_vm->_sound->bgStop();
		_vm->_sound->bgSetPlayMode(BackgroundAtmosphere::kPlayModeRandom);
		_vm->_sound->bgPlay(fileName, "SND", SOUND_SND, palStart);
		return;
	} else if (lastFrame < 0) {
		warning("Unknown Video/Music command: %d, %s", lastFrame, fileName);
		return;
	}

	if (startFrame == -2) {
		startFrame = 0;
		lastFrame = -1;
		close = false;
	}

	if ((fileName[0] != 0) && !_vm->_vidPlayer->primaryOpen(fileName, x, y, flags)) {
		WRITE_VAR(11, (uint32) -1);
		return;
	}

	if (startFrame >= 0) {
		_vm->_game->_preventScroll = true;
		_vm->_vidPlayer->primaryPlay(startFrame, lastFrame, breakKey, palCmd, palStart, palEnd, 0);
		_vm->_game->_preventScroll = false;
	}

	if (close)
		_vm->_vidPlayer->primaryClose();
}

} // End of namespace Gob
