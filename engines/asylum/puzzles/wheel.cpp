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

#include "asylum/puzzles/wheel.h"

#include "asylum/resources/special.h"
#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"

namespace Asylum {

const GameFlag puzzleWheelFlags[32] = {
	kGameFlag253, kGameFlag257, kGameFlag259, kGameFlag254, kGameFlag258,
	kGameFlag260, kGameFlag253, kGameFlag255, kGameFlag259, kGameFlag254,
	kGameFlag256, kGameFlag260, kGameFlag253, kGameFlag255, kGameFlag257,
	kGameFlag254, kGameFlag256, kGameFlag258, kGameFlag255, kGameFlag257,
	kGameFlag259, kGameFlag256, kGameFlag258, kGameFlag260, kGameFlag253,
	kGameFlag254, kGameFlag255, kGameFlag256, kGameFlag257, kGameFlag258,
	kGameFlag259, kGameFlag260
};

const uint32 puzzleWheelContacts[24] = {
	1, 5, 7, 2, 6,
	8, 1, 3, 7, 2,
	4, 8, 1, 3, 5,
	2, 4, 6, 3, 5,
	7, 4, 6, 8
};

const uint32 puzzleWheelSparks[8] = {
	0, 6, 5, 4, 1, 3, 7, 2
};

const uint32 puzzleWheelClockResourceIndexes[16] = {
	39, 40, 42, 44, 46,
	48, 50, 52, 38, 41,
	43, 45, 47, 49, 51,
	53
};

const int16 puzzleWheelPoints[56][2] = {
	{  0,   0}, {  0,   0}, {  0,   0}, {250, 254}, {122,  24}, {208,  68}, {238, 160},
	{218, 234}, {162, 228}, { 71, 222}, { 22, 165}, { 35,  70}, {278,   0}, {536, 146},
	{122,  24}, {208,  68}, {238, 160}, {218, 234}, {162, 228}, { 71, 222}, { 22, 165},
	{ 35,  70}, {342,  87}, {342,  87}, {342,  87}, {342,  87}, {342,  87}, {342,  87},
	{342,  87}, {342,  87}, {358, 268}, {342,  87}, {342,  87}, {342,  87}, {342,  87},
	{342,  87}, {342,  87}, {342,  87}, {342,  87}, {342,  87}, {342,  87}, {342,  87},
	{342,  87}, {342,  87}, {342,  87}, {342,  87}, {342,  87}, {536, 146}, {406, 106},
	{402, 217}, {369, 128}, {368, 197}, {452, 184}, {470, 144}, {442, 116}, {347, 166}
};

const int16 puzzleWheelRects[4][4] = {
	{425, 268, 491, 407}, {358, 268, 424, 407}, {561, 251, 594, 324}, {280, 276, 310, 400}
};

PuzzleWheel::PuzzleWheel(AsylumEngine *engine) : Puzzle(engine) {
	_currentRect = -1;
	_resourceIndex = 0;
	_resourceIndexClock = 0;
	_resourceIndexLever = 13;

	_frameIndexWheel = 0;
	memset(&_frameIndexes, 0, sizeof(_frameIndexes));
	memset(&_frameIndexesSparks, -1, sizeof(_frameIndexesSparks));

	_showTurnedClock = false;
	_turnWheelRight = false;
	_moveLever = false;
	_moveChain = false;
}

PuzzleWheel::~PuzzleWheel() {
}

void PuzzleWheel::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsSint32LE(_resourceIndex);
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleWheel::init(const AsylumEvent &)  {
	getSpecial()->reset(false);

	getScreen()->setPalette(getWorld()->graphicResourceIds[1]);
	getScreen()->setGammaLevel(getWorld()->graphicResourceIds[1]);

	updateCursor();
	getCursor()->show();

	_currentRect = -2;

	memset(&_frameIndexesSparks, -1, sizeof(_frameIndexesSparks));

	for (uint32 i = 0; i < 8; i++) {
		if (_vm->isGameFlagSet((GameFlag)(kGameFlag253 + i)))
			_frameIndexes[i + 1] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[i + 14]) - 1;
		else
			_frameIndexes[i + 1] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[i + 4]) - 1;
	}

	return true;
}

void PuzzleWheel::updateScreen()  {
	getScreen()->clearGraphicsInQueue();
	getScreen()->draw(getWorld()->graphicResourceIds[0]);

	// Blinking red light
	getScreen()->draw(getWorld()->graphicResourceIds[12], (uint32)_frameIndexes[11], &puzzleWheelPoints[12]);
	_frameIndexes[11] = (_frameIndexes[11] + 1) % (int32)GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[12]);

	// Clock
	if (_showTurnedClock)
		getScreen()->draw(getWorld()->graphicResourceIds[_resourceIndexClock], (uint32)_frameIndexes[9], Common::Point(342, 87));
	else
		getScreen()->draw(getWorld()->graphicResourceIds[_resourceIndex + 22], 0, Common::Point(342, 87));

	// Chain
	getScreen()->draw(getWorld()->graphicResourceIds[3], (uint32)_frameIndexes[0], &puzzleWheelPoints[3]);

	// Update chain frame index
	if (_moveChain) {
		if (!_frameIndexes[0]) {
			getCursor()->hide();
			getSound()->playSound(getWorld()->graphicResourceIds[65]);
		}

		_frameIndexes[0] = (_frameIndexes[0] + 1) % (int32)GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[3]);

		if (!_frameIndexes[0]) {
			closeLocks();
			_moveChain = false;
			getCursor()->show();
		}
	}

	// Locks
	uint32 frameIndex = 1;
	for (uint32 i = 0; i < 8; i++) {
		ResourceId resourceIndex = 0;
		uint32 pointIndex = 0;

		if (_vm->isGameFlagSet((GameFlag)(kGameFlag253 + i))) {
			resourceIndex = 14 + i;
			pointIndex = 10 + (4 + i);
		} else {
			resourceIndex = 4 + i;
			pointIndex = 4 + i;
		}

		getScreen()->draw(getWorld()->graphicResourceIds[resourceIndex], (uint32)_frameIndexes[frameIndex], &puzzleWheelPoints[pointIndex]);

		if (_frameIndexes[frameIndex] != (int32)GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[resourceIndex]) - 1)
			++_frameIndexes[frameIndex];

		++frameIndex;
	}

	// Sparks
	for (uint32 i = 0; i < 8; i++) {
		if (_frameIndexesSparks[i] >= 0) {
			getScreen()->draw(getWorld()->graphicResourceIds[57 + i], (uint32)_frameIndexesSparks[i], &puzzleWheelPoints[48 + i]);

			if (_frameIndexesSparks[i] == (int32)GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[57 + i]) - 1)
				_frameIndexesSparks[i] = -1;
			else
				++_frameIndexesSparks[i];
		}
	}

	// Lever
	if (_resourceIndexLever == 13)
		getScreen()->draw(getWorld()->graphicResourceIds[_resourceIndexLever], (uint32)_frameIndexes[10], &puzzleWheelPoints[13]);
	else if (_resourceIndexLever == 54)
		getScreen()->draw(getWorld()->graphicResourceIds[_resourceIndexLever], (uint32)_frameIndexes[10], &puzzleWheelPoints[47]);

	// Update lever frame index
	if (_moveLever) {
		if (!_frameIndexes[10] && _resourceIndexLever == 13) {
			getCursor()->hide();
			getSound()->playSound(getWorld()->graphicResourceIds[67]);
		}

		_frameIndexes[10] = (_frameIndexes[10] + 1) % (int32)GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[_resourceIndexLever]);

		if (!_frameIndexes[10]) {
			if (_resourceIndexLever == 54) {
				_resourceIndexLever = 13;
				toggleLocks();
				_moveLever = false;
				getCursor()->show();
			} else {
				_resourceIndexLever = 54;
			}
		}
	}

	// Wheel
	getScreen()->draw(getWorld()->graphicResourceIds[30], _frameIndexWheel, &puzzleWheelPoints[30]);

	// Update wheel frame index
	if (_showTurnedClock) {
		if (!_frameIndexes[9]) {
			getCursor()->hide();
			getSound()->playSound(getWorld()->graphicResourceIds[66]);
		}

		uint32 frameCountWheel = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[30]);
		if (_turnWheelRight)
			_frameIndexWheel = (_frameIndexWheel + 1) % frameCountWheel;
		else
			_frameIndexWheel = (_frameIndexWheel + frameCountWheel - 1) % frameCountWheel;

		_frameIndexes[9] = (_frameIndexes[9] + 1) % (int32)GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[_resourceIndexClock]);

		if (!_frameIndexes[9]) {
			_showTurnedClock = false;
			getCursor()->show();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Show elements on screen
	getScene()->drawRain();

	// Check for completion
	checkFlags();
}

bool PuzzleWheel::mouseLeftDown(const AsylumEvent &) {
	switch (findRect()) {
	default:
		break;

	case 0: // Wheel right
		_frameIndexes[9] = 0;
		_turnWheelRight = true;
		_showTurnedClock = true;

		updateIndex();
		break;

	case 1: // Wheel left
		_frameIndexes[9] = 0;
		_turnWheelRight = false;
		_showTurnedClock = true;

		updateIndex();
		break;

	case 2: // Lever
		_moveLever = true;
		break;

	case 3: // Chain
		_moveChain = true;
		break;
	}

	return true;
}

bool PuzzleWheel::mouseRightDown(const AsylumEvent &) {
	getScreen()->clear();
	_vm->switchEventHandler(getScene());

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////
void PuzzleWheel::updateCursor() {
	int32 index = findRect();

	if (_currentRect != index) {
		_currentRect = index;

		getCursor()->set(getWorld()->graphicResourceIds[2], -1, (index == -1) ? kCursorAnimationNone : kCursorAnimationMirror);
	}
}

int32 PuzzleWheel::findRect() {
	for (uint32 i = 0; i < ARRAYSIZE(puzzleWheelRects); i++) {
		if (_vm->rectContains(&puzzleWheelRects[i], getCursor()->position()))
			return i;
	}

	return -1;
}

void PuzzleWheel::updateIndex() {
	if (_turnWheelRight) {
		_resourceIndexClock = puzzleWheelClockResourceIndexes[_resourceIndex];
		_resourceIndex = (_resourceIndex + 7) % ~7;
	} else {
		_resourceIndexClock = puzzleWheelClockResourceIndexes[_resourceIndex + 8];
		_resourceIndex = (_resourceIndex + 1) % ~7;
	}

}

void PuzzleWheel::checkFlags() {
	for (uint32 i = 0; i < 8; i++)
		if (!_vm->isGameFlagSet(puzzleWheelFlags[i]))
			return;

	_vm->setGameFlag(kGameFlag261);
	getScreen()->clear();
	_vm->switchEventHandler(getScene());
}

void PuzzleWheel::closeLocks() {
	for (uint32 i = 0; i < 8; i++) {
		if (!_vm->isGameFlagSet(puzzleWheelFlags[24 + i]))
			continue;

		getSound()->playSound(getWorld()->graphicResourceIds[69]);
		_vm->clearGameFlag(puzzleWheelFlags[24 + i]);
		_frameIndexes[i + 1] = 0;
	}
}

void PuzzleWheel::toggleLocks() {
	memset(&_frameIndexesSparks, -1, sizeof(_frameIndexesSparks));

	for (int32 i = 0; i < 3; i++) {
		_vm->toggleGameFlag(puzzleWheelFlags[i + 3 * _resourceIndex]);

		// Update lock frame indexes
		_frameIndexes[puzzleWheelContacts[i + 3 * _resourceIndex]] = 0;

		// Update sparks frame indexes
		_frameIndexesSparks[puzzleWheelSparks[puzzleWheelContacts[i + 3 * _resourceIndex] - 1]] = 0;

		if (_vm->isGameFlagSet(puzzleWheelFlags[i]))
			getSound()->playSound(getWorld()->graphicResourceIds[68]);
		else
			getSound()->playSound(getWorld()->graphicResourceIds[69]);
	}
}

} // End of namespace Asylum
