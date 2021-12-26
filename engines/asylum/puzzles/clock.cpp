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

const int16 puzzleClockRects[12][4] = {
	{354, 121, 373, 142}, {384, 119, 405, 146}, {405, 135, 424, 160},
	{404, 168, 425, 193}, {389, 205, 410, 236}, {359, 240, 383, 270},
	{325, 255, 341, 284}, {294, 253, 313, 284}, {277, 237, 294, 264},
	{273, 201, 301, 235}, {290, 168, 315, 195}, {315, 133, 344, 162}
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

void PuzzleClock::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsSint32LE(_frameIndexes[2]);
	s.syncAsSint32LE(_frameIndexes[1]);
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleClock::init(const AsylumEvent &)  {
	_currentRect = -2;

	updateCursor();

	getScreen()->setPalette(getWorld()->graphicResourceIds[6]);
	getScreen()->setGammaLevel(getWorld()->graphicResourceIds[6]);

	getCursor()->show();

	_frameIndexes[1] = 6;
	_frameIndexes[2] = 10;
	_currentFrameIndex = 10;
	_showCursor = false;
	_rightButtonClicked = false;

	return true;
}

void PuzzleClock::updateScreen() {
	// Draw elements
	getScreen()->clearGraphicsInQueue();
	getScreen()->draw(getWorld()->graphicResourceIds[5]);

	getScreen()->draw(getWorld()->graphicResourceIds[2], _frameIndexes[0], Common::Point(322, 187));
	_frameIndexes[0]++;
	_frameIndexes[0] %= GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[2]);

	getScreen()->draw(getWorld()->graphicResourceIds[4], _frameIndexes[1], Common::Point(267, 109));
	getScreen()->draw(getWorld()->graphicResourceIds[3], _frameIndexes[2], Common::Point(274, 124));

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

	if (_rightButtonClicked) {
		getScreen()->clear();
		_vm->switchEventHandler(getScene());
	} else {
		if (_vm->isGameFlagSet(kGameFlag511)) {
			AsylumEvent evt;
			mouseRightDown(evt);
		}
	}
}

bool PuzzleClock::mouseLeftDown(const AsylumEvent &) {
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

bool PuzzleClock::mouseRightDown(const AsylumEvent &) {
	setFlag();
	_rightButtonClicked = true;

	return true;
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

int32 PuzzleClock::findRect() {
	for (uint32 i = 0; i < ARRAYSIZE(puzzleClockRects); i++) {
		if (_vm->rectContains(&puzzleClockRects[i], getCursor()->position()))
			return i;
	}

	return -1;
}

void PuzzleClock::setFlag() {
	if (_frameIndexes[2] == puzzleClockFrameIndexes[11])
		_vm->setGameFlag(kGameFlag511);
}

} // End of namespace Asylum
