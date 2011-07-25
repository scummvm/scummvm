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
 */

#include "asylum/puzzles/lock.h"

#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"

namespace Asylum {

const Common::Point puzzleLockPolygons[16] = {
	Common::Point(159, 293), Common::Point(151, 318), Common::Point(179, 322), Common::Point(185, 298),
	Common::Point(212, 303), Common::Point(207, 327), Common::Point(234, 332), Common::Point(239, 308),
	Common::Point(149, 333), Common::Point(146, 365), Common::Point(172, 369), Common::Point(176, 338),
	Common::Point(202, 345), Common::Point(200, 375), Common::Point(226, 379), Common::Point(231, 347)
};

PuzzleLock::PuzzleLock(AsylumEngine *engine) : Puzzle(engine) {
	memset(&_frameIndexes, 0, sizeof(_frameIndexes));
	_incrementLock = false;
	_counter = 0;
	memset(&_ticks, 0, sizeof(_ticks));
}

PuzzleLock::~PuzzleLock() {
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleLock::init(const AsylumEvent &evt)  {
	getScreen()->clear();

	getScreen()->setPalette(getWorld()->graphicResourceIds[14]);
	getScreen()->setGammaLevel(getWorld()->graphicResourceIds[14], 0);

	_vm->setGameFlag(kGameFlag115);

	getCursor()->set(getWorld()->graphicResourceIds[41], 0, kCursorAnimationNone);

	return true;
}

bool PuzzleLock::update(const AsylumEvent &evt)  {
	updateCursor();

	// Draw screen
	getScreen()->clearGraphicsInQueue();
	getScreen()->draw(getWorld()->graphicResourceIds[13]);

	if (_frameIndexes[0] != 32 || _frameIndexes[1] != 28 || _frameIndexes[2] != 0) {
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[15], _frameIndexes[0], Common::Point(145, 292), kDrawFlagNone, 0, 1);
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[15], _frameIndexes[1], Common::Point(173, 297), kDrawFlagNone, 0, 1);
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[15], _frameIndexes[2], Common::Point(201, 302), kDrawFlagNone, 0, 1);
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[40], _frameIndexes[3], Common::Point(337, 127), kDrawFlagNone, 0, 1);

		if (_frameIndexes[4] != -1)
			getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[42], _frameIndexes[4], Common::Point(318, 102), kDrawFlagNone, 0, 1);

		if (_frameIndexes[5] != -1)
			getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[43], _frameIndexes[5], Common::Point(318, 99), kDrawFlagNone, 0, 1);

		getScreen()->drawGraphicsInQueue();
		getScreen()->copyBackBufferToScreen();
	} else {
		if (_frameIndexes[6] == 5)
			getSound()->playSound(getWorld()->soundResourceIds[15], false, Config.sfxVolume - 10);

		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[16], _frameIndexes[6], Common::Point(0, 264), kDrawFlagNone, 0, 1);

		++_counter;

		if (_counter > 9) {
			++_frameIndexes[6];

			if (_frameIndexes[6] > 7) {
				getCursor()->hide();
				getSharedData()->setFlag(kFlag1, true);
				_vm->setGameFlag(kGameFlag52);
				getSound()->playSound(getWorld()->soundResourceIds[16], false, Config.sfxVolume - 10);
				_frameIndexes[6] = 0;
				_counter = 0;

				exitPuzzle();
			} else {
				getScreen()->drawGraphicsInQueue();
				getScreen()->copyBackBufferToScreen();
				_counter = 0;
			}
		}
	}

	// Update frame indexes
	for (uint32 i = 0; i < 3; i++) {
		if (_frameIndexes[i] % ~3 && _frameIndexes[i]) {
			++_counter;

			if (_counter > 3) {
				if (_incrementLock)
					++_frameIndexes[i];
				else
					--_frameIndexes[i];

				if (_frameIndexes[i] < 0)
					_frameIndexes[i] = 43;

				if (_frameIndexes[i] > 43)
					_frameIndexes[i] = 0;

				_counter = 0;
			}
		}
	}

	for (uint32 i = 0; i < 2; i++) {
		if (_frameIndexes[i + 4] == -1) {
			_ticks[i] = _vm->getTick();

			if (rnd(1000) < 10)
				_frameIndexes[i + 4] = 0;
		} else {
			if (_vm->getTick() > _ticks[i] + 200) {
				++_frameIndexes[i + 4];

				if (_frameIndexes[i + 4] > 24)
					_frameIndexes[i + 4] = -1;

				_ticks[i] = _vm->getTick();
			}
		}
	}

	if (_frameIndexes[3]) {
		if (_vm->getTick() > _ticks[2] + 100) {
			++_frameIndexes[3];

			if (_frameIndexes[3] > 15)
				_frameIndexes[3] = 0;

			_ticks[2] = _vm->getTick();
		}
	} else {
		_ticks[2] = _vm->getTick();

		if (rnd(1000) < 10)
			_frameIndexes[3] = 1;
	}

	return true;
}

bool PuzzleLock::mouseLeftDown(const AsylumEvent &evt) {
	if (_frameIndexes[6])
		return false;

	Common::Point mousePos = evt.mouse;

	for (uint32 i = 0; i < 3; i++) {
		if (hitTest(&puzzleLockPolygons[0], mousePos, 8 + 2 * i)) {
			if (!(_frameIndexes[i] % ~3))
				getSound()->playSound(getWorld()->soundResourceIds[14], false, Config.sfxVolume - 10);

			_incrementLock = true;

			++_frameIndexes[i];

			if (_frameIndexes[i] > 43)
				_frameIndexes[i] = 0;
		}
	}

	for (uint32 i = 0; i < 3; i++) {
		if (hitTest(&puzzleLockPolygons[0], mousePos, 2 * i)) {
			if (!(_frameIndexes[i] % ~3))
				getSound()->playSound(getWorld()->soundResourceIds[14], false, Config.sfxVolume - 10);

			_incrementLock = false;

			--_frameIndexes[i];

			if (_frameIndexes[i] < 0)
				_frameIndexes[i] = 43;
		}
	}

	return true;
}

bool PuzzleLock::mouseRightDown(const AsylumEvent &evt) {
	exitPuzzle();

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////
void PuzzleLock::updateCursor() {
	Common::Point mousePos = getCursor()->position();

	for (uint32 i = 0; i < 6; i++) {
		if (hitTest(&puzzleLockPolygons[0], mousePos, 2 * i)) {
			if (getCursor()->animation != kCursorAnimationMirror)
				getCursor()->set(getWorld()->graphicResourceIds[41]);

			return ;
		}
	}

	if (getCursor()->animation == kCursorAnimationMirror)
		getCursor()->set(getWorld()->graphicResourceIds[41], 0, kCursorAnimationNone);
}

} // End of namespace Asylum
