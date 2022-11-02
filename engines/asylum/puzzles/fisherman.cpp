/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "asylum/puzzles/fisherman.h"

#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/screen.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"

namespace Asylum {

const int16 puzzleFishermanPolygons[31][2] = {
	{ 10,  53}, {113,  52}, {222,  46}, {328,  51},
	{426,  51}, {523,  49}, {277, 398}, { 30,  44},
	{112,  44}, { 93, 400}, {  0, 400}, {130,  44},
	{210,  44}, {201, 400}, {112, 400}, {224,  44},
	{315,  44}, {309, 400}, {219, 400}, {326,  44},
	{411,  44}, {415, 400}, {326, 400}, {422,  44},
	{506,  44}, {526, 400}, {434, 400}, {523,  44},
	{607,  44}, {640, 400}, {545, 400}
};

PuzzleFisherman::PuzzleFisherman(AsylumEngine *engine) : Puzzle(engine) {
	memset(&_state, 0, sizeof(_state));

	_resetPressed = false;
	_counter = 0;

	_pauseTimer = 0;
	_allowClick = false;
}

PuzzleFisherman::~PuzzleFisherman() {
}

void PuzzleFisherman::saveLoadWithSerializer(Common::Serializer &s) {
	for (int i = 0; i < ARRAYSIZE(_state); i++)
		s.syncAsSint32LE(_state[i]);

	s.syncAsSint32LE(_resetPressed);
	s.syncAsSint32LE(_counter);
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleFisherman::init(const AsylumEvent &evt)  {
	getCursor()->set(getWorld()->graphicResourceIds[47], -1, kCursorAnimationMirror, 7);

	for (uint32 i = 0; i < ARRAYSIZE(_state); i++)
		if (_vm->isGameFlagNotSet((GameFlag)(kGameFlag801 + i)))
			_state[i] = false;

	if (_counter == 6) {
		_vm->clearGameFlag(kGameFlag619);
		_counter = 0;
	}

	_allowClick = false;
	getScreen()->setPalette(getWorld()->graphicResourceIds[39]);
	getScreen()->setGammaLevel(getWorld()->graphicResourceIds[39]);

	return mouseLeftDown(evt);
}

void PuzzleFisherman::updateScreen()  {
	// Draw background
	getScreen()->clearGraphicsInQueue();
	getScreen()->fillRect(0, 0, 640, 480, 251);
	getScreen()->draw(getWorld()->graphicResourceIds[38], 0, Common::Point(0, 0), kDrawFlagNone, true);

	// Draw 7 graphics
	for (uint32 i = 0; i < 6; i++) {
		if (_state[i])
			getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[40 + i], 0, &puzzleFishermanPolygons[i], kDrawFlagNone, 0, 1);
	}

	_allowClick = true;

	if (_resetPressed) {
		++_pauseTimer;

		if (_pauseTimer > 5) {
			// Reset state
			memset(&_state, 0, sizeof(_state));

			for (uint32 i = 0; i < 6; i++)
				_vm->clearGameFlag((GameFlag)(kGameFlag801 + i));

			_resetPressed = false;
			_allowClick = true;
			_pauseTimer = 0;
			// Original bug: if the Sun button was pressed last,
			// the correct order of buttons wouldn't work until another reset
			_counter = 0;
		}
	}

	if (_counter == 6) {
		++_pauseTimer;

		if (_pauseTimer > 10) {
			_pauseTimer = 0;

			_vm->setGameFlag(kGameFlag619);
			getScreen()->setPalette(getWorld()->currentPaletteId);

			_vm->switchEventHandler(getScene());
		}
	}
}

bool PuzzleFisherman::mouseLeftDown(const AsylumEvent &evt) {
	if (!_allowClick)
		return false;

	for (uint32 i = 0; i < 6; i++) {
		if (hitTest(&puzzleFishermanPolygons[i * 4 + 7], evt.mouse)) {
			if (!_state[i]) {
				getSound()->playSound(getWorld()->soundResourceIds[9], false, Config.sfxVolume - 10);
				_state[i] = true;
				setFlags(i);
			}
		}
	}

	if (puzzleFishermanPolygons[6][0] < evt.mouse.x
	 && puzzleFishermanPolygons[6][1] < evt.mouse.y
	 && puzzleFishermanPolygons[6][0] + 70 > evt.mouse.x
	 && puzzleFishermanPolygons[6][1] + 30 > evt.mouse.y) {
		 getSound()->playSound(getWorld()->soundResourceIds[10], false, Config.sfxVolume - 10);

		 for (uint32 i = 0; i < 6; i++)
			 _vm->clearGameFlag((GameFlag)(kGameFlag801 + i));

		 _resetPressed = true;
	}

	if (_resetPressed)
		_allowClick = false;

	return true;
}

bool PuzzleFisherman::mouseRightDown(const AsylumEvent &) {
	getCursor()->hide();
	getSharedData()->setFlag(kFlag1, true);
	getScreen()->stopPaletteFade(0, 0, 0);

	_vm->switchEventHandler(getScene());

	return false;
}

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////
void PuzzleFisherman::updateCursor() {
	bool found = false;
	Common::Point mousePos = getCursor()->position();

	for (uint32 i = 0; i < 6; i++) {
		if (found)
			break;

		if (hitTest(&puzzleFishermanPolygons[i * 4 + 7], mousePos)) {
			if (!_state[i]) {
				found = true;

				if (getCursor()->getAnimation() != kCursorAnimationMirror)
					getCursor()->set(getWorld()->graphicResourceIds[47], -1, kCursorAnimationMirror, 7);
			}
		}
	}

	if (found)
		return;

	if (puzzleFishermanPolygons[6][0] >= mousePos.x
	 || puzzleFishermanPolygons[6][1] >= mousePos.y
	 || puzzleFishermanPolygons[6][0] + 70 <= mousePos.x
	 || puzzleFishermanPolygons[6][1] + 30 <= mousePos.y) {
		 if (getCursor()->getAnimation() != kCursorAnimationNone)
			 getCursor()->set(getWorld()->graphicResourceIds[47], -1, kCursorAnimationNone, 7);
		 else if (getCursor()->getAnimation() != kCursorAnimationMirror)
			 getCursor()->set(getWorld()->graphicResourceIds[47], -1, kCursorAnimationMirror, 7);
	}
}

void PuzzleFisherman::setFlags(uint32 index) {
	switch (index) {
	default:
		break;

	case 0:
		_vm->setGameFlag(kGameFlag801);
		_counter = (_counter == 2) ? 3 : 0;
		break;

	case 1:
		_vm->setGameFlag(kGameFlag802);
		_counter = (_counter == 3) ? 4 : 0;
		break;

	case 2:
		_vm->setGameFlag(kGameFlag803);
		_counter = (_counter == 1) ? 2 : 0;
		break;

	case 3:
		_vm->setGameFlag(kGameFlag804);
		if (_counter == 5) {
			_allowClick = false;
			_counter = 6;
		} else {
			_counter = 0;
		}
		break;

	case 4:
		_vm->setGameFlag(kGameFlag805);
		_counter = (_counter == 0) ? 1 : 0;
		break;

	case 5:
		_vm->setGameFlag(kGameFlag806);
		_counter = (_counter == 4) ? 5 : 0;
		break;
	}
}

} // End of namespace Asylum
