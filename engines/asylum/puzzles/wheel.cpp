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

const uint32 puzzleFrameCountIndex[24] = {
	1, 5, 7, 2, 6,
	8, 1, 3, 7, 2,
	4, 8, 1, 3, 5,
	2, 4, 6, 3, 5,
	7, 4, 6, 8
};

const uint32 puzzleResourceIndexes[16] = {
	39, 40, 42, 44, 46,
	48, 50, 52, 38, 41,
	43, 45, 47, 49, 51,
	53
};

const Common::Rect puzzleWheelRects[4] = {
	Common::Rect(425, 268, 491, 407),
	Common::Rect(358, 268, 424, 407),
	Common::Rect(561, 251, 594, 324),
	Common::Rect(280, 276, 310, 400)
};

PuzzleWheel::PuzzleWheel(AsylumEngine *engine) : Puzzle(engine) {
	_currentRect = -1;
	_resourceIndex = 0;
	_resourceIndex9 = 0;
	_resourceIndex10 = 13;

	_frameIndex30 = 0;
	memset(&_frameIndexes, -1, sizeof(_frameIndexes));
	memset(&_frameCounts, 0, sizeof(_frameCounts));

	_showResource9 = false;
	_flag1 = false;
	_flag2 = false;
	_flag3 = false;
}

PuzzleWheel::~PuzzleWheel() {
}

void PuzzleWheel::reset() {
	getSpecial()->reset(true);

	_resourceIndex = 0;
	_resourceIndex10 = 13;

	_frameIndex30 = 0;

	_frameCounts[0] = 0;
	_frameCounts[9] = 0;
	_frameCounts[10] = 0;
	_frameCounts[11] = 0;

	_flag1 = false;
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleWheel::init(const AsylumEvent &evt)  {
	getSpecial()->reset(false);

	getScreen()->setPalette(getWorld()->graphicResourceIds[1]);
	getScreen()->setGammaLevel(getWorld()->graphicResourceIds[1], 0);

	updateCursor(evt);
	getCursor()->show();

	_currentRect = -2;

	memset(&_frameIndexes, -1, sizeof(_frameIndexes));

	for (uint32 i = 0; i < 8; i++) {
		if (_vm->isGameFlagSet((GameFlag)(kGameFlag253 + i)))
			_frameCounts[i + 1] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[i + 14]) - 1;
		else
			_frameCounts[i + 1] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[i + 4]) - 1;
	}

	return true;
}

bool PuzzleWheel::update(const AsylumEvent &evt)  {
	error("[PuzzleWheel::update] Not implemented!");
}

bool PuzzleWheel::mouseLeftDown(const AsylumEvent &evt) {
	switch (findRect(evt.mouse)) {
	default:
		break;

	case 0:
		_frameCounts[9] = 0;
		_flag1 = true;
		_showResource9 = true;

		updateIndex();
		break;

	case 1:
		_frameCounts[9] = 0;
		_flag1 = false;
		_showResource9 = true;

		updateIndex();
		break;

	case 2:
		_flag2 = true;
		break;

	case 3:
		_flag3 = true;
		break;
	}

	return true;
}

bool PuzzleWheel::mouseRightDown(const AsylumEvent &evt) {
	getScreen()->clear();
	_vm->switchEventHandler(getScene());

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////
void PuzzleWheel::updateCursor(const AsylumEvent &evt) {
	int32 index = findRect(evt.mouse);

	if (_currentRect != index) {
		_currentRect = index;

		getCursor()->set(getWorld()->graphicResourceIds[2], -1, (index == -1) ? kCursorAnimationNone : kCursorAnimationMirror);
	}
}

int32 PuzzleWheel::findRect(Common::Point mousePos) {
	for (uint32 i = 0; i < ARRAYSIZE(puzzleWheelRects); i++) {
		if (puzzleWheelRects[i].contains(mousePos))
			return i;
	}

	return -1;
}

void PuzzleWheel::updateIndex() {
	_resourceIndex9 = puzzleResourceIndexes[_resourceIndex + (_flag1 ? 0 : 8)];
	_resourceIndex = (_resourceIndex + (_flag1 ? 7 : 1)) % -8;
}

void PuzzleWheel::checkFlags() {
	for (uint32 i = 0; i < 8; i++)
		if (!_vm->isGameFlagSet(puzzleWheelFlags[i]))
			return;

	_vm->setGameFlag(kGameFlag261);
	getScreen()->clear();
	_vm->switchEventHandler(getScene());
}

void PuzzleWheel::playSound() {
	for (uint32 i = 0; i < 8; i++) {
		if (!_vm->isGameFlagSet(puzzleWheelFlags[24 + i]))
			continue;

		getSound()->playSound(getWorld()->graphicResourceIds[69]);
		_vm->clearGameFlag(puzzleWheelFlags[24 + i]);
		_frameCounts[i + 1] = 0;
	}
}

void PuzzleWheel::playSoundReset() {
	memset(&_frameIndexes, -1, sizeof(_frameIndexes));

	for (uint32 i = 0; i < 3; i++) {
		_vm->toggleGameFlag(puzzleWheelFlags[i + 3 * _resourceIndex]);
		_frameCounts[puzzleFrameCountIndex[i + 3 * _resourceIndex]] = 0;

		// Original game resets some unused data

		if (_vm->isGameFlagSet(puzzleWheelFlags[i]))
			getSound()->playSound(getWorld()->graphicResourceIds[68]);
		else
			getSound()->playSound(getWorld()->graphicResourceIds[69]);
	}
}

} // End of namespace Asylum
