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

#include "asylum/puzzles/timemachine.h"

#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"

namespace Asylum {

const Common::Rect puzzleTimeMachineRects[10] = {
	Common::Rect(  0, 241,  20, 276),
	Common::Rect(  0, 285,  20, 320),
	Common::Rect(117, 245, 137, 280),
	Common::Rect(117, 284, 137, 319),
	Common::Rect(236, 246, 256, 281),
	Common::Rect(236, 290, 256, 325),
	Common::Rect(356, 245, 376, 280),
	Common::Rect(356, 287, 376, 322),
	Common::Rect(476, 248, 496, 283),
	Common::Rect(475, 290, 495, 325)
};

const Common::Point puzzleTimeMachinePoints[6] = {
	Common::Point(-65,  -30),
	Common::Point(-20,  -68),
	Common::Point( 25, -106),
	Common::Point( 70, -144),
	Common::Point(115, -182),
	Common::Point(-65,  -30)
};

PuzzleTimeMachine::PuzzleTimeMachine(AsylumEngine *engine) : Puzzle(engine) {
	_leftButtonClicked = false;
	_counter = 0;
	_counter2 = 0;
	_currentFrameIndex = 0;
	memset(&_frameIndexes, 0, sizeof(_frameIndexes));
	memset(&_frameCounts, 0, sizeof(_frameCounts));
	memset(&_frameIncrements, 0, sizeof(_frameIncrements));
	memset(&_state, 0, sizeof(_state));

	_data_4572BC = false;
	_data_4572CC = false;

	_data_45AAA8 = 0;
	_data_45AAAC = 0;
}

PuzzleTimeMachine::~PuzzleTimeMachine() {
}

void PuzzleTimeMachine::reset() {
	_frameIndexes[0] = 0;
	_frameIndexes[1] = 4;
	_frameIndexes[2] = 20;
	_frameIndexes[3] = 16;
	_frameIndexes[4] = 20;
	memset(&_state, 0, sizeof(_state));
	_data_45AAAC = 0;

	// Reset point only (the other values are static)
	_point = Common::Point(-65, -30);
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleTimeMachine::init(const AsylumEvent &evt)  {
	_counter = 0;
	getCursor()->set(getWorld()->graphicResourceIds[62], -1, kCursorAnimationMirror, 7);

	_frameCounts[0] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[35]);
	_frameCounts[1] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[36]);
	_frameCounts[2] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[37]);
	_frameCounts[3] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[38]);
	_frameCounts[4] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[39]);
	_frameCounts[5] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[40]);

	getScreen()->setPalette(getWorld()->graphicResourceIds[41]);
	getScreen()->setGammaLevel(getWorld()->graphicResourceIds[41]);

	mouseLeftDown(evt);

	return true;
}

bool PuzzleTimeMachine::update(const AsylumEvent &evt)  {
	updateCursor();

	// Draw screen elements
	getScreen()->clearGraphicsInQueue();
	getScreen()->draw(getWorld()->graphicResourceIds[34]);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[35], _frameIndexes[0], Common::Point(23, 215),  kDrawFlagNone, 0, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[36], _frameIndexes[1], Common::Point(70, 217),  kDrawFlagNone, 0, 2);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[37], _frameIndexes[2], Common::Point(189, 217), kDrawFlagNone, 0, 3);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[38], _frameIndexes[3], Common::Point(309, 218), kDrawFlagNone, 0, 4);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[39], _frameIndexes[4], Common::Point(429, 212), kDrawFlagNone, 0, 5);

	if (_frameIndexes[0] != 28 || _frameIndexes[1] || _frameIndexes[2] || _frameIndexes[3] || _frameIndexes[4]) {
		_leftButtonClicked = true;
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[43], 0, Common::Point(599, 220), kDrawFlagNone, 0, 5);
	} else {
		getSound()->stop(getWorld()->soundResourceIds[17]);
		getSound()->stop(getWorld()->soundResourceIds[16]);

		if (_vm->isGameFlagNotSet(kGameFlag925))
			getSound()->playSound(getWorld()->soundResourceIds[18]);

		_vm->setGameFlag(kGameFlag925);

		++_counter;
	}

	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[40], _frameIndexes[5], _point, kDrawFlagNone, 0, 1);

	//////////////////////////////////////////////////////////////////////////
	// Show all buttons
	for (uint32 i = 0; i < ARRAYSIZE(puzzleTimeMachineRects); i += 2) {
		if (_state[i / 2] != -1)
			getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[44 + i], 0, Common::Point(puzzleTimeMachineRects[i].left, puzzleTimeMachineRects[i].top), kDrawFlagNone, 0, 5);
	}

	for (uint32 i = 1; i < ARRAYSIZE(puzzleTimeMachineRects); i += 2) {
		if (_state[i / 2] != 1)
			getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[45 + i], 0, Common::Point(puzzleTimeMachineRects[i].left, puzzleTimeMachineRects[i].top), kDrawFlagNone, 0, 5);
	}

	// Draw to screen
	getScreen()->drawGraphicsInQueue();
	getScreen()->copyBackBufferToScreen();

	// Check for puzzle completion
	if (_counter > 30 && _vm->isGameFlagSet(kGameFlag925)) {
		getCursor()->hide();
		getSharedData()->setFlag(kFlag1, true);
		getScreen()->stopPaletteFade(0, 0, 0);

		_vm->switchEventHandler(getScene());
	}

	// Update frame indexes
	if (_currentFrameIndex == 0 && _frameIncrements[0] != 0) {
		_data_4572BC = 0;
		_data_4572CC = 0;

		_frameIndexes[5] += _frameIncrements[0];

		if (_counter2 > 4) {
			_counter2 = 0;

			if (!getSound()->isPlaying(getWorld()->soundResourceIds[14]))
				getSound()->playSound(getWorld()->soundResourceIds[14]);

			_frameIndexes[0] += _frameIncrements[0];
		} else {
			++_counter2;
		}
	} else {
		warning("[PuzzleTimeMachine::update] Not implemented!");
	}

	// Reset frame increments & state
	if (_counter2 == 0 && _data_4572CC == 0 && _leftButtonClicked && _data_45AAA8 == 0 && _currentFrameIndex == _data_45AAAC) {
		for (uint32 i = 0; i < 5; i++) {
			if (!(_frameIndexes[i] % ~3))
				_frameIncrements[i] = 0;
		}

		memset(&_state, 0, sizeof(_state));
	}

	// Adjust frame indexes
	for (uint32 i = 0; i < ARRAYSIZE(_frameIndexes); i++) {
		if (_frameIndexes[i] >= (int32)_frameCounts[i])
			_frameIndexes[i] = 0;
	}

	return true;
}

bool PuzzleTimeMachine::mouseLeftDown(const AsylumEvent &evt) {
	if (_vm->isGameFlagSet(kGameFlag925))
		return true;

	_leftButtonClicked = false;

	int32 index = -1;
	for (uint32 i = 0; i < ARRAYSIZE(puzzleTimeMachineRects); i++) {
		if (puzzleTimeMachineRects[i].contains(evt.mouse)) {
			index =  i;

			break;
		}
	}

	if (index == -1)
		return true;

	getSound()->playSound(getWorld()->soundResourceIds[15]);

	_data_4572CC = true;
	_data_4572BC = true;

	if (index % ~1 == 1) {
		_frameIncrements[index] = 1;
		_state[index] = 1;
	} else {
		_frameIncrements[index] = -1;
		_state[index] = -1;
	}

	return true;
}

bool PuzzleTimeMachine::mouseLeftUp(const AsylumEvent &evt) {
	_leftButtonClicked = true;

	return true;
}

bool PuzzleTimeMachine::mouseRightDown(const AsylumEvent &evt) {
	getCursor()->hide();
	getSharedData()->setFlag(kFlag1, true);
	getScreen()->stopPaletteFade(0, 0, 0);
	_vm->switchEventHandler(getScene());

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////
void PuzzleTimeMachine::updateCursor() {
	for (uint32 i = 0; i < ARRAYSIZE(puzzleTimeMachineRects); i++) {
		if (puzzleTimeMachineRects[i].contains(getCursor()->position())) {
			if (getCursor()->animation != kCursorAnimationMirror)
				getCursor()->set(getWorld()->graphicResourceIds[62], -1, kCursorAnimationMirror, 7);

			return;
		}
	}

	if (getCursor()->animation)
		getCursor()->set(getWorld()->graphicResourceIds[62], -1, kCursorAnimationNone, 7);
}

} // End of namespace Asylum
