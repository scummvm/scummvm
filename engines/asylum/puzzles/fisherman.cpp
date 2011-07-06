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

#include "asylum/puzzles/fisherman.h"

#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/screen.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"

namespace Asylum {

const Common::Point puzzleFishermanPolygons[31] = {
	Common::Point( 10,  53), Common::Point(113,  52), Common::Point(222,  46), Common::Point(328,  51),
	Common::Point(426,  51), Common::Point(523,  49), Common::Point(277, 398), Common::Point( 30,  44),
	Common::Point(112,  44), Common::Point( 93, 400), Common::Point(  0, 400), Common::Point(130,  44),
	Common::Point(210,  44), Common::Point(201, 400), Common::Point(112, 400), Common::Point(224,  44),
	Common::Point(315,  44), Common::Point(309, 400), Common::Point(219, 400), Common::Point(326,  44),
	Common::Point(411,  44), Common::Point(415, 400), Common::Point(326, 400), Common::Point(422,  44),
	Common::Point(506,  44), Common::Point(526, 400), Common::Point(434, 400), Common::Point(523,  44),
	Common::Point(607,  44), Common::Point(640, 400), Common::Point(545, 400)
};

PuzzleFisherman::PuzzleFisherman(AsylumEngine *engine) : Puzzle(engine) {
	memset(&_state, 0, sizeof(_state));

	_dword_45AAD4 = false;
	_counter = 0;

	_dword_45A12C = 0;
	_dword_45A130 = false;
}

PuzzleFisherman::~PuzzleFisherman() {
}

//////////////////////////////////////////////////////////////////////////
// Reset
//////////////////////////////////////////////////////////////////////////
void PuzzleFisherman::reset() {
	memset(&_state, 0, sizeof(_state));
	_dword_45AAD4 = false;

	// Original setups polygons here

	_dword_45A130 = false;
	_counter = 0;

	// Original resets scene fields, but since we are called during a restart, the whole scene is recreated later anyway
	/*for (uint32 i = 0; i < 6; i++) {
		getWorld()->field_E8610[i] = 0;
		getWorld()->field_E8628[i] = 0;
	}*/
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleFisherman::init(const AsylumEvent &evt)  {
	getCursor()->set(getWorld()->graphicResourceIds[47], -1, kCursorAnimationMirror, 7);

	for (uint32 i = 0; i < ARRAYSIZE(_state); i++)
		if (_vm->isGameFlagNotSet((GameFlag)(kGameFlag801 + i)))
			_state[i] = 0;

	if (_counter == 6) {
		_vm->clearGameFlag(kGameFlag619);
		_counter = 0;
	}

	_dword_45A130 = false;
	getScreen()->setPalette(getWorld()->graphicResourceIds[39]);
	getScreen()->setGammaLevel(getWorld()->graphicResourceIds[39], 0);

	return mouseLeftDown(evt);
}

bool PuzzleFisherman::update(const AsylumEvent &evt)  {
	updateCursor();

	// Draw background
	getScreen()->clearGraphicsInQueue();
	getScreen()->draw(getWorld()->graphicResourceIds[38]);

	// Draw 7 graphics
	for (uint32 i = 0; i < 6; i++) {
		if (_state[i])
			getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[40 + i], 0, puzzleFishermanPolygons[i], kDrawFlagNone, 0, 1);
	}

	getScreen()->drawGraphicsInQueue();

	_dword_45A130 = true;
	getScreen()->copyBackBufferToScreen();

	if (_dword_45AAD4 == 1) {
		++_dword_45A12C;

		if (_dword_45A12C > 5) {
			// Reset state
			memset(&_state, 0, sizeof(_state));

			for (uint32 i = 0; i < 6; i++)
				_vm->clearGameFlag((GameFlag)(kGameFlag801 + i));

			_dword_45A130 = true;
			_dword_45A12C = 0;
		}
	}

	if (_counter == 6) {
		++_dword_45A12C;

		if (_dword_45A12C > 10) {
			_dword_45A12C = 0;

			_vm->setGameFlag(kGameFlag619);
			getScreen()->setPalette(getWorld()->currentPaletteId);

			_vm->switchEventHandler(getScene());
		}
	}

	return true;
}

bool PuzzleFisherman::mouseLeftDown(const AsylumEvent &evt) {
	if (!_dword_45A130)
		return false;

	for (uint32 i = 0; i < 6; i++) {
		if (hitTest(&puzzleFishermanPolygons[i * 4 + 7], evt.mouse)) {
			if (!_state[i]) {
				getSound()->playSound(getWorld()->graphicResourceIds[9], false, Config.sfxVolume - 10);
				_state[i] = true;
				setFlags(i);
			}
		}
	}

	if (puzzleFishermanPolygons[6].x < evt.mouse.x
	 && puzzleFishermanPolygons[6].y < evt.mouse.y
	 && puzzleFishermanPolygons[6].x + 70 > evt.mouse.x
	 && puzzleFishermanPolygons[6].y + 30 > evt.mouse.y) {
		 getSound()->playSound(getWorld()->graphicResourceIds[10], false, Config.sfxVolume - 10);

		 for (uint32 i = 0; i < 6; i++)
			 _vm->clearGameFlag((GameFlag)(kGameFlag801 + i));

		 _dword_45AAD4 = true;
	}

	if (_dword_45AAD4)
		_dword_45A130 = false;

	return true;
}

bool PuzzleFisherman::mouseRightDown(const AsylumEvent &evt) {
	getCursor()->hide();
	getSharedData()->setFlag(kFlag1, true);
	getScreen()->setupPaletteAndStartFade(0, 0, 0);

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

				if (getCursor()->animation != kCursorAnimationMirror)
					getCursor()->set(getWorld()->graphicResourceIds[47], -1, kCursorAnimationMirror, 7);
			}
		}
	}

	if (found)
		return;

	if (puzzleFishermanPolygons[6].x >= mousePos.x
	 || puzzleFishermanPolygons[6].y >= mousePos.y
	 || puzzleFishermanPolygons[6].x + 70 <= mousePos.x
	 || puzzleFishermanPolygons[6].y + 30 <= mousePos.y) {
		 if (getCursor()->animation != kCursorAnimationNone)
			 getCursor()->set(getWorld()->graphicResourceIds[47], -1, kCursorAnimationNone, 7);
		 else if (getCursor()->animation != kCursorAnimationMirror)
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
			_dword_45A130 = false;
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
