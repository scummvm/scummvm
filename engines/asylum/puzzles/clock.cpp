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

#include "asylum/puzzles/clock.h"

#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"

namespace Asylum {

const uint32 puzzleClockFrameIndexes[28] = {
	 2,  4,  6,  8, 10, 11, 13, 15, 17, 19,
	21,  0, 19, 21,  0,  2,  4,  6,  8,  9,
	11, 13, 15, 17,  0,  0, 182, 0
};

const Common::Point puzzleClockPoints[3] = {
	Common::Point(322, 187),
	Common::Point(267, 109),
	Common::Point(274, 124)
};

const Common::Rect puzzleClockRects[12] = {
	Common::Rect(354, 121, 373, 142),
	Common::Rect(384, 119, 405, 146),
	Common::Rect(405, 135, 424, 160),
	Common::Rect(404, 168, 425, 193),
	Common::Rect(389, 205, 410, 236),
	Common::Rect(359, 240, 383, 270),
	Common::Rect(325, 255, 341, 284),
	Common::Rect(294, 253, 313, 284),
	Common::Rect(277, 237, 294, 264),
	Common::Rect(273, 201, 301, 235),
	Common::Rect(290, 168, 315, 195),
	Common::Rect(315, 133, 344, 162)
};

PuzzleClock::PuzzleClock(AsylumEngine *engine) : Puzzle(engine) {
	memset(&_frameIndexes, 0, sizeof(_frameIndexes));
	_showCursor = false;
	_rightButtonClicked = false;
	_currentRect = 0;
	_currentFrameIndex = 0;
}

PuzzleClock::~PuzzleClock() {
}

void PuzzleClock::reset() {
	_frameIndexes[0] = 0;
	_frameIndexes[1] = 6;
	_frameIndexes[2] = 10;
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleClock::init()  {
	_currentRect = -2;

	updateCursor();

	getScreen()->setPalette(getWorld()->graphicResourceIds[6]);
	getScreen()->setGammaLevel(getWorld()->graphicResourceIds[6], 0);

	getCursor()->show();

	_frameIndexes[1] = 6;
	_frameIndexes[2] = 10;
	_currentFrameIndex = 10;
	_showCursor = false;
	_rightButtonClicked = false;

	return true;
}

bool PuzzleClock::update() {
	// Draw elements
	getScreen()->clearGraphicsInQueue();
	getScreen()->draw(getWorld()->graphicResourceIds[5]);

	getScreen()->draw(getWorld()->graphicResourceIds[2], _frameIndexes[0], puzzleClockPoints[0].x, puzzleClockPoints[0].y, 0);
	_frameIndexes[0]++;
	_frameIndexes[0] %= GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[2]);

	getScreen()->draw(getWorld()->graphicResourceIds[4], _frameIndexes[1], puzzleClockPoints[1].x, puzzleClockPoints[1].y, 0);
	getScreen()->draw(getWorld()->graphicResourceIds[3], _frameIndexes[2], puzzleClockPoints[2].x, puzzleClockPoints[2].y, 0);

	if (_currentFrameIndex == _frameIndexes[2]) {
		if (_showCursor) {
			_showCursor = false;
			getCursor()->show();
			setFlag();
		}
	} else {
		++_frameIndexes[2];
		_frameIndexes[2] %= GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[3]);
	}

	getScreen()->drawGraphicsInQueue();
	getScreen()->copyBackBufferToScreen();

	if (_rightButtonClicked) {
		getScreen()->clear();
		_vm->switchEventHandler(getScene());
	} else {
		if (_vm->isGameFlagSet(kGameFlag511))
			mouseRight();
	}

	return true;
}

bool PuzzleClock::mouse(const AsylumEvent &evt) {
	switch (evt.type) {
	default:
		break;

	case Common::EVENT_RBUTTONDOWN:
		mouseRight();
		break;

	case Common::EVENT_LBUTTONDOWN:
		return mouseLeft();
		break;
	}

	return true;
}

bool PuzzleClock::mouseLeft() {
	int32 index = findRect();

	if (index == -1)
		return false;

	_currentFrameIndex = puzzleClockFrameIndexes[index];
	++_frameIndexes[2];
	_frameIndexes[2] %= GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[3]);
	_showCursor = true;

	getCursor()->hide();

	return true;
}

void PuzzleClock::mouseRight() {
	setFlag();
	_rightButtonClicked = true;
}

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////
void PuzzleClock::updateCursor() {
	int32 index = findRect();

	if (index != _currentRect) {
		_currentRect = index;

		if (index == -1)
			getCursor()->set(getWorld()->graphicResourceIds[7], -1, kCursorAnimationNone);
		else
			getCursor()->set(getWorld()->graphicResourceIds[7], -1, kCursorAnimationMirror);
	}
}

void PuzzleClock::setFlag() {
	if (_frameIndexes[2] == puzzleClockFrameIndexes[11])
		_vm->setGameFlag(kGameFlag511);
}

int32 PuzzleClock::findRect() {
	Common::Point mousePos = getCursor()->position();

	for (uint32 i = 0; i < ARRAYSIZE(puzzleClockRects); i++) {
		if (puzzleClockRects[i].contains(mousePos))
			return i;
	}

	return -1;
}

} // End of namespace Asylum
