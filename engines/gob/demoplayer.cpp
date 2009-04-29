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

#include "gob/gob.h"
#include "gob/demoplayer.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/draw.h"
#include "gob/inter.h"
#include "gob/videoplayer.h"

namespace Gob {

DemoPlayer::DemoPlayer(GobEngine *vm) : _vm(vm) {
	_doubleMode = false;
}

DemoPlayer::~DemoPlayer() {
}

bool DemoPlayer::lineStartsWith(const Common::String &line, const char *start) {
	return (strstr(line.c_str(), start) == line.c_str());
}

void DemoPlayer::init() {
	// The video player needs some fake variables
	_vm->_inter->allocateVars(32);

	// Init the screen
	_vm->_draw->initScreen();
	_vm->_draw->_cursorIndex = -1;

	_vm->_util->longDelay(200); // Letting everything settle

}

void DemoPlayer::clearScreen() {
	debugC(1, kDebugDemo, "Clearing the screen");
	_vm->_video->clearScreen();
}

void DemoPlayer::playVideo(const char *fileName) {
	uint32 waitTime = 0;
	char *file, *filePtr;

	file = filePtr = new char[strlen(fileName) + 1];
	strcpy(file, fileName);

	// Trimming spaces front
	while (*file == ' ')
		file++;

	char *spaceBack = strchr(file, ' ');
	if (spaceBack) {
		char *nextSpace = strchr(spaceBack, ' ');

		if (nextSpace)
			*nextSpace = '\0';

		*spaceBack++ = '\0';

		waitTime = atoi(spaceBack) * 100;
	}

	// WORKAROUND: The Inca2 demo wants to play cons2.imd, but that file doesn't exist.
	//             cons.imd does, however.
	if ((_vm->getGameType() == kGameTypeInca2) && (!scumm_stricmp(file, "cons2.imd")))
		strcpy(file, "cons.imd");

	debugC(1, kDebugDemo, "Playing video \"%s\"", file);

	if (_vm->_vidPlayer->primaryOpen(file)) {
		bool videoSupportsDouble =
			((_vm->_vidPlayer->getFeatures() & CoktelVideo::kFeaturesSupportsDouble) != 0);

		if (_doubleMode) {
			if (videoSupportsDouble) {
				_vm->_vidPlayer->slotSetDoubleMode(-1, true);
				playVideoNormal();
			} else
				playVideoDoubled();
		} else
			playVideoNormal();

		_vm->_vidPlayer->primaryClose();

		if (waitTime > 0)
			_vm->_util->longDelay(waitTime);
	}


	delete[] filePtr;
}

void DemoPlayer::playVideoNormal() {
	_vm->_vidPlayer->primaryPlay();
}

void DemoPlayer::playVideoDoubled() {
	const char *fileNameOpened;
	char *fileName;

	fileNameOpened = _vm->_vidPlayer->getFileName();

	fileName = new char[strlen(fileNameOpened) + 1];
	strcpy(fileName, fileNameOpened);

	_vm->_vidPlayer->primaryClose();

	if (_vm->_vidPlayer->primaryOpen(fileName, 0, -1, VideoPlayer::kFlagOtherSurface)) {
		for (int i = 0; i < _vm->_vidPlayer->getFramesCount(); i++) {
			if (_vm->_vidPlayer->primaryPlay(i, i))
				break;

			CoktelVideo::State state = _vm->_vidPlayer->getState();

			int16 w = state.right - state.left + 1;
			int16 h = state.bottom - state.top + 1;
			int16 wD = (state.left * 2) + (w * 2);
			int16 hD = (state.top * 2) + (h * 2);

			_vm->_video->drawSpriteDouble(_vm->_draw->_spritesArray[0], _vm->_draw->_frontSurface,
					state.left, state.top, state.right, state.bottom, state.left, state.top, 0);
			_vm->_draw->dirtiedRect(_vm->_draw->_frontSurface,
					state.left * 2, state.top * 2, wD, hD);
			_vm->_video->retrace();
		}
	}

	delete[] fileName;
}

void DemoPlayer::evaluateVideoMode(const char *mode) {
	debugC(2, kDebugDemo, "Video mode \"%s\"", mode);

	if (!scumm_strnicmp(mode, "VESA", 4))
		_doubleMode = false;
	else if (!scumm_strnicmp(mode, "VGA", 3) && _vm->is640())
		_doubleMode = true;
}

} // End of namespace Gob
