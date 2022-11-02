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

#include "asylum/puzzles/timemachine.h"

#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"

namespace Asylum {

const int16 puzzleTimeMachineRects[10][4] = {
	{  0, 241,  20, 276}, {  0, 285,  20, 320},
	{117, 245, 137, 280}, {117, 284, 137, 319},
	{236, 246, 256, 281}, {236, 290, 256, 325},
	{356, 245, 376, 280}, {356, 287, 376, 322},
	{476, 248, 496, 283}, {475, 290, 495, 325}
};

const int16 puzzleTimeMachinePoints[5][2] = {
	{-65,  -30}, {-20,  -68}, { 25, -106}, { 70, -144}, {115, -182}
};

PuzzleTimeMachine::PuzzleTimeMachine(AsylumEngine *engine) : Puzzle(engine) {
	_leftButtonClicked = true;
	_counter = 0;

	memset(&_frameIndexes,		0, sizeof(_frameIndexes));
	memset(&_frameCounts,		0, sizeof(_frameCounts));
	memset(&_frameIncrements,	0, sizeof(_frameIncrements));
	memset(&_state,				0, sizeof(_state));

	_data_45AAA8 = _data_45AAAC = 0;
	_currentFrameIndex = 0;

	reset();
}

PuzzleTimeMachine::~PuzzleTimeMachine() {
}

void PuzzleTimeMachine::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsSint32LE(_frameIndexes[0]);
	s.syncAsSint32LE(_frameIndexes[1]);
	s.syncAsSint32LE(_frameIndexes[2]);
	s.syncAsSint32LE(_frameIndexes[3]);
	s.syncAsSint32LE(_frameIndexes[4]);

	s.skip(4); // Unused

	s.syncBytes((byte *)&_state, sizeof(_state));
	s.skip(3);            // We only use 5 elements i the state array

	s.skip(5 * 4 * 2);    // The original saves 4 points that are static data

	s.syncAsSint32LE(_point.x);
	s.syncAsSint32LE(_point.y);

	for (int i = 0; i < 5; i++)
		s.syncAsSint32LE(_frameIncrements[i]);

	s.syncAsSint32LE(_currentFrameIndex);

	s.syncAsSint32LE(_data_45AAA8);
	s.syncAsSint32LE(_data_45AAAC);

	s.syncAsSint32LE(_frameIndexes[5]);
}

void PuzzleTimeMachine::reset() {
	_frameIndexes[0] =  0;
	_frameIndexes[1] =  4;
	_frameIndexes[2] = 20;
	_frameIndexes[3] = 16;
	_frameIndexes[4] = 20;

	_index  = -1;
	_index2 = 0;
	_point.x = _newPoint.x = puzzleTimeMachinePoints[0][0];
	_point.y = _newPoint.y = puzzleTimeMachinePoints[0][1];
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleTimeMachine::init(const AsylumEvent &evt) {
	getCursor()->set(getWorld()->graphicResourceIds[62], -1, kCursorAnimationMirror, 7);

	_frameCounts[0] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[35]);
	_frameCounts[1] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[36]);
	_frameCounts[2] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[37]);
	_frameCounts[3] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[38]);
	_frameCounts[4] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[39]);
	_frameCounts[5] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[40]);

	getScreen()->setPalette(getWorld()->graphicResourceIds[41]);
	getScreen()->setGammaLevel(getWorld()->graphicResourceIds[41]);

	return true;
}

void PuzzleTimeMachine::updateScreen()  {
	// Draw screen elements
	getScreen()->clearGraphicsInQueue();
	getScreen()->fillRect(0, 0, 640, 480, 115);
	getScreen()->draw(getWorld()->graphicResourceIds[34], 0, Common::Point(0, 0), kDrawFlagNone, true);

	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[35], _frameIndexes[0], Common::Point( 23, 215), kDrawFlagNone, 0, 3);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[36], _frameIndexes[1], Common::Point( 70, 217), kDrawFlagNone, 0, 3);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[37], _frameIndexes[2], Common::Point(189, 217), kDrawFlagNone, 0, 3);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[38], _frameIndexes[3], Common::Point(309, 218), kDrawFlagNone, 0, 3);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[39], _frameIndexes[4], Common::Point(429, 212), kDrawFlagNone, 0, 3);

	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[40], _frameIndexes[5], _point, kDrawFlagNone, 0, 1);
	if (_point.x < _newPoint.x)
		_point += Common::Point(15, -12 - (abs((double)(_point.x - _newPoint.x)) > 15 ? 1 : 0));
	else if (_point.x > _newPoint.x)
		_point -= Common::Point(15, -12 - (abs((double)(_point.x - _newPoint.x)) > 15 ? 1 : 0));

	if (_frameIndexes[0] != 28 || _frameIndexes[1] || _frameIndexes[2] || _frameIndexes[3] || _frameIndexes[4]) {
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[43], 0, Common::Point(599, 220), kDrawFlagNone, 0, 2);
	} else {
		getSound()->stop(getWorld()->soundResourceIds[17]);
		getSound()->stop(getWorld()->soundResourceIds[16]);

		if (_vm->isGameFlagNotSet(kGameFlag925))
			getSound()->playSound(getWorld()->soundResourceIds[18]);

		_vm->setGameFlag(kGameFlag925);

		++_counter;
	}

	//////////////////////////////////////////////////////////////////////////
	// Show all buttons
	for (uint32 i = 0; i < ARRAYSIZE(puzzleTimeMachineRects); i += 2) {
		if ((uint32)_index != i || _leftButtonClicked)
			getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[44 + i], 0, Common::Point(puzzleTimeMachineRects[i][0], puzzleTimeMachineRects[i][1]), kDrawFlagNone, 0, 5);
	}

	for (uint32 i = 1; i < ARRAYSIZE(puzzleTimeMachineRects); i += 2) {
		if ((uint32)_index != i || _leftButtonClicked)
			getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[44 + i], 0, Common::Point(puzzleTimeMachineRects[i][0], puzzleTimeMachineRects[i][1]), kDrawFlagNone, 0, 5);
	}

	_leftButtonClicked = true;

	// Check for puzzle completion
	if (_counter > 30 && _vm->isGameFlagSet(kGameFlag925)) {
		getCursor()->hide();
		getSharedData()->setFlag(kFlag1, true);
		getScreen()->stopPaletteFade(0, 0, 0);

		_vm->switchEventHandler(getScene());
	}

	// Update frame indexes & increments
	if (_index != -1) {
		_frameIndexes[_index / 2] += _frameIncrements[_index / 2];
		if (_frameIndexes[_index / 2] < 0) {
			_frameIndexes[_index / 2]	= _frameCounts[_index / 2] - 1;
		} else if (_frameIndexes[_index / 2] > (int32)_frameCounts[_index / 2] - 1) {
			_frameIndexes[_index / 2]	= 0;
			_frameIncrements[_index / 2]	= 0;
		} else if (!(_frameIndexes[_index / 2] % 4)) {
			getSound()->playSound(getWorld()->soundResourceIds[15]);
			_frameIncrements[_index / 2]	= 0;
			_index				= -1;
		}

		_frameIndexes[5] = (_frameIndexes[5] + 1) % _frameCounts[5];
	}
}

bool PuzzleTimeMachine::mouseLeftDown(const AsylumEvent &evt) {
	if (_vm->isGameFlagSet(kGameFlag925))
		return true;

	_leftButtonClicked = false;

	int32 index = -1;
	for (uint32 i = 0; i < ARRAYSIZE(puzzleTimeMachineRects); i++) {
		if (_vm->rectContains(&puzzleTimeMachineRects[i], evt.mouse)) {
			index = i;

			break;
		}
	}

	if (index == -1)
		return true;

	getSound()->playSound(getWorld()->soundResourceIds[14]);
	if ((_index2 / 2) != (uint32)index / 2) {
		getSound()->playSound(getWorld()->soundResourceIds[16]);
		_newPoint.x = puzzleTimeMachinePoints[index / 2][0];
		_newPoint.y = puzzleTimeMachinePoints[index / 2][1];
	}

	if (index % 2)
		_frameIncrements[index / 2] = 1;
	else
		_frameIncrements[index / 2] = -1;

	_index = _index2 = index;

	return true;
}

bool PuzzleTimeMachine::mouseRightDown(const AsylumEvent &) {
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
		if (_vm->rectContains(&puzzleTimeMachineRects[i], getCursor()->position())) {
			if (getCursor()->getAnimation() != kCursorAnimationMirror)
				getCursor()->set(getWorld()->graphicResourceIds[62], -1, kCursorAnimationMirror, 7);

			return;
		}
	}

	if (getCursor()->getAnimation())
		getCursor()->set(getWorld()->graphicResourceIds[62], -1, kCursorAnimationNone, 7);
}

} // End of namespace Asylum
