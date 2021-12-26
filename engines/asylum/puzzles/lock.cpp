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

#include "asylum/puzzles/lock.h"

#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"

namespace Asylum {

const int16 puzzleLockPolygons[16][2] = {
	{159, 293}, {151, 318}, {179, 322}, {185, 298},
	{212, 303}, {207, 327}, {234, 332}, {239, 308},
	{149, 333}, {146, 365}, {172, 369}, {176, 338},
	{202, 345}, {200, 375}, {226, 379}, {231, 347}
};

PuzzleLock::PuzzleLock(AsylumEngine *engine) : Puzzle(engine) {
	memset(&_frameIndexes, 0, sizeof(_frameIndexes));
	_frameIndexes[0] = _frameIndexes[1] = _frameIndexes[2] = 4;
	_incrementLock = false;
	_counter = 0;
	memset(&_ticks, 0, sizeof(_ticks));
}

PuzzleLock::~PuzzleLock() {
}

void PuzzleLock::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsSint32LE(_frameIndexes[0]);
	s.syncAsSint32LE(_frameIndexes[1]);
	s.syncAsSint32LE(_frameIndexes[2]);
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleLock::init(const AsylumEvent &)  {
	getScreen()->clear();

	getScreen()->setPalette(getWorld()->graphicResourceIds[14]);
	getScreen()->setGammaLevel(getWorld()->graphicResourceIds[14]);

	_vm->setGameFlag(kGameFlag115);

	getCursor()->set(getWorld()->graphicResourceIds[41], 0, kCursorAnimationNone);

	return true;
}

bool PuzzleLock::update(const AsylumEvent &)  {
	updateCursor();

	// Draw screen
	getScreen()->clearGraphicsInQueue();
	getScreen()->fillRect(0, 0, 640, 480, 252);
	getScreen()->draw(getWorld()->graphicResourceIds[13], 0, Common::Point(0, 0), kDrawFlagNone, true);

	if (_frameIndexes[0] != 32 || _frameIndexes[1] != 28 || _frameIndexes[2] != 0) {
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[15], (uint32)_frameIndexes[0], Common::Point(145, 292), kDrawFlagNone, 0, 1);
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[15], (uint32)_frameIndexes[1], Common::Point(173, 297), kDrawFlagNone, 0, 1);
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[15], (uint32)_frameIndexes[2], Common::Point(201, 302), kDrawFlagNone, 0, 1);
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[40], (uint32)_frameIndexes[3], Common::Point(337, 127), kDrawFlagNone, 0, 1);

		if (_frameIndexes[4] != -1)
			getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[42], (uint32)_frameIndexes[4], Common::Point(318, 102), kDrawFlagNone, 0, 1);

		if (_frameIndexes[5] != -1)
			getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[43], (uint32)_frameIndexes[5], Common::Point(318, 99), kDrawFlagNone, 0, 1);

		getScreen()->drawGraphicsInQueue();
		getScreen()->copyBackBufferToScreen();
	} else {
		if (_frameIndexes[6] == 5)
			getSound()->playSound(getWorld()->soundResourceIds[15], false, Config.sfxVolume - 10);

		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[16], (uint32)_frameIndexes[6], Common::Point(0, 264), kDrawFlagNone, 0, 1);

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
				getSharedData()->reset();

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

bool PuzzleLock::mouseRightDown(const AsylumEvent &) {
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
			if (getCursor()->getAnimation() != kCursorAnimationMirror)
				getCursor()->set(getWorld()->graphicResourceIds[41]);

			return ;
		}
	}

	if (getCursor()->getAnimation() == kCursorAnimationMirror)
		getCursor()->set(getWorld()->graphicResourceIds[41], 0, kCursorAnimationNone);
}

} // End of namespace Asylum
