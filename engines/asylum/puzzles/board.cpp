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

#include "asylum/puzzles/board.h"

#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/screen.h"
#include "asylum/system/sound.h"
#include "asylum/system/text.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"

namespace Asylum {

PuzzleBoard::PuzzleBoard(AsylumEngine *engine) : Puzzle(engine) {
	error("[PuzzleBoard::PuzzleBoard] No puzzle data!");
}

PuzzleBoard::PuzzleBoard(AsylumEngine *engine, PuzzleData data) : Puzzle(engine) {
	_data = data;

	// Init board
	_solved = false;
	memset(&_text,       0,     sizeof(_text));
	memset(&_charUsed,   false, sizeof(_charUsed));
	memset(&_solvedText, 0,     sizeof(_solvedText));
	_rectIndex = -2;
	_soundResourceId = kResourceNone;
	_selectedSlot = -1;
	_position = 0;
}

void PuzzleBoard::reset() {
	memset(&_charUsed, false, sizeof(_charUsed));
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleBoard::init(const AsylumEvent &evt)  {
	_rectIndex = -2;
	_selectedSlot = -1;
	_solved = false;
	_soundResourceId = 0;

	getScreen()->setPalette(getWorld()->graphicResourceIds[32]);
	getScreen()->setGammaLevel(getWorld()->graphicResourceIds[32], 0);
	getText()->loadFont(getWorld()->graphicResourceIds[35]);

	// Prepare text to draw
	memset(&_text, 0, sizeof(_text));

	for (uint32 i = 0; i < _data.soundResourceSize; i++) {
		_data.soundResources[i].played = false;
		strcat((char *)&_text, getText()->get(MAKE_RESOURCE(kResourcePackText, 1068 + _data.soundResources[i].index)));
		strcat((char *)&_text, " ");
	}

	updateScreen();

	getCursor()->show();

	return true;
}

bool PuzzleBoard::updateScreen()  {
	getScreen()->clearGraphicsInQueue();

	getScreen()->draw(getWorld()->graphicResourceIds[_data.backgroundIndex]);
	drawText();

	getScreen()->drawGraphicsInQueue();
	getScreen()->copyBackBufferToScreen();

	return true;
}

bool PuzzleBoard::update(const AsylumEvent &evt) {
	updateCursor();

	if (!_solved)
		playSound();

	if (_vm->isGameFlagNotSet(_data.gameFlag)) {
		if (strcmp(_solvedText, _data.solvedText))
			return true;

		if (_solved) {
			if (!getSound()->isPlaying(MAKE_RESOURCE(kResourcePackSpeech, 1))) {
				_vm->setGameFlag(_data.gameFlag);
				getCursor()->show();
				getScreen()->clear();
				_vm->switchEventHandler(getScene());
			}
		} else {
			_solved = true;
			getCursor()->hide();
			stopSound();
			getSound()->playSound(MAKE_RESOURCE(kResourcePackSpeech, 1), false, Config.voiceVolume);
		}
	}

	return true;
}

bool PuzzleBoard::mouseRightDown(const AsylumEvent &evt) {
	if (!stopSound()) {
		getScreen()->clear();
		_vm->switchEventHandler(getScene());
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////
bool PuzzleBoard::stopSound() {
	if (_soundResourceId && getSound()->isPlaying(_soundResourceId)) {
		getSound()->stopAll(_soundResourceId);

		return true;
	}

	return false;
}

void PuzzleBoard::drawText() {
	getText()->loadFont(getWorld()->graphicResourceIds[35]);
	getText()->draw(0, 99, kTextCenter, 25, 50, 16, 590, _text);

	int32 index = 0;
	for (uint32 x = 215; x < _data.maxWidth; x += 24) {
		if (!_solvedText[index])
			break;

		getText()->setPosition(x, _selectedSlot != index ? 360 : 370);
		getText()->drawChar(_solvedText[index]);

		index += 2;
	}
}

void PuzzleBoard::playSound() {
	uint32 index = 0;
	for (index = 0; index < _data.soundResourceSize; index++) {
		if (!_data.soundResources[index].played)
			break;
	}

	if (index >= _data.soundResourceSize)
		return;

	if (!_soundResourceId || !getSound()->isPlaying(_soundResourceId)) {
		_soundResourceId = MAKE_RESOURCE(kResourcePackSharedSound, 961 + _data.soundResources[index].index);
		getSound()->playSound(_soundResourceId, false, Config.voiceVolume);
		_data.soundResources[index].played = true;
	}
}

int32 PuzzleBoard::findRect() {
	Common::Point mousePos = getCursor()->position();

	for (uint32 i = 0; i < _data.charMapSize; i++) {
		if (mousePos.x >= _data.charMap[i].position.x && mousePos.x < _data.charMap[i].position.x + 12
		 && mousePos.y >= _data.charMap[i].position.y && mousePos.y < _data.charMap[i].position.y + 18)
			if (!_charUsed[i])
				return i;
	}

	return -1;
}

int32 PuzzleBoard::checkMouse() {
	Common::Point mousePos = getCursor()->position();

	if (mousePos.x >= 215 && mousePos.x < (int16)_data.maxWidth && mousePos.y >= 360 && mousePos.y < 376) {
		uint32 index = (mousePos.x - 215) / 12;

		if (index >= ARRAYSIZE(_solvedText))
			return -1;

		if (_solvedText[index] != 0) {
			if (_data.checkForSpace) {
				if (_solvedText[index] != ' ')
					return -3;
			} else {
				return -3;
			}
		}
	}

	return -1;
}

void PuzzleBoard::updateCursor() {
	Common::Point mousePos = getCursor()->position();

	if (mousePos.y <= 350) {
		int32 index = findRect();

		if (index == -1) {
			if (getCursor()->graphicResourceId == getWorld()->graphicResourceIds[34])
				return;

			_rectIndex = index;

			getCursor()->set(getWorld()->graphicResourceIds[34]);
		} else {
			if (getCursor()->graphicResourceId == getWorld()->graphicResourceIds[33])
				return;

			if (index == _rectIndex)
				return;

			_rectIndex = index;

			getCursor()->set(getWorld()->graphicResourceIds[33]);
		}
	} else {
		if (_vm->isGameFlagSet(_data.gameFlag)) {
			if (getCursor()->graphicResourceId == getWorld()->graphicResourceIds[34])
				return;

			getCursor()->set(getWorld()->graphicResourceIds[34]);
		} else {
			int32 index = checkMouse();

			if (getCursor()->graphicResourceId == getWorld()->graphicResourceIds[33])
				return;

			if (index == _rectIndex)
				return;

			if (index == -1)
				getCursor()->set(getWorld()->graphicResourceIds[33], 0, kCursorAnimationNone);
			else
				getCursor()->set(getWorld()->graphicResourceIds[33]);
		}
	}
}

} // End of namespace Asylum
