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

#include "asylum/puzzles/hivemachine.h"

#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"

namespace Asylum {

const int16 keyPoints[5][2] = {
	{246, 106}, {377, 171}, {319, 250}, {220, 249}, {167, 154}
};

const int16 indicatorPoints[6][2] = {
	{260, 410}, {279, 411}, {298, 411}, {318, 412}, {338, 413}, {358, 413}
};

PuzzleHiveMachine::PuzzleHiveMachine(AsylumEngine *engine) : Puzzle(engine) {
	_counterRed = _counterGreen = _counterKey = 0;
	_rectIndex = -2;
	_frameIndex = 0;
	_frameIndex1 = 0;
	_soundingNote = kMusicalNoteNone;
	_notesNumber = 0;
	_ok = false;
}

PuzzleHiveMachine::~PuzzleHiveMachine() {
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleHiveMachine::init(const AsylumEvent &)  {
	getScreen()->setPalette(getWorld()->graphicResourceIds[9]);
	getScreen()->setGammaLevel(getWorld()->graphicResourceIds[9]);
	_rectIndex = -2;

	getCursor()->show();

	return true;
}

bool PuzzleHiveMachine::mouseLeftDown(const AsylumEvent &) {
	if (_rectIndex != -1 && _counterRed == 0) {
		_soundingNote = MusicalNote(_rectIndex);
		_melody.push_back(_soundingNote);
		_notesNumber = (_notesNumber + 1) % 7;
		_counterKey = 10;
		_frameIndex1 = 0;
		playSound();

		if (_melody.size() == 6) {
			_ok = 	(_melody[0] == kMusicalNoteB) &&
				(_melody[1] == kMusicalNoteD) &&
				(_melody[2] == kMusicalNoteD) &&
				(_melody[3] == kMusicalNoteA) &&
				(_melody[4] == kMusicalNoteF) &&
				(_melody[5] == kMusicalNoteE);
			if (!_ok) {
				_melody.clear();
				_counterRed = 30;
			} else
				_counterGreen = 10;
		}
	}

	return true;
}

bool PuzzleHiveMachine::mouseRightDown(const AsylumEvent &) {
	_notesNumber = 0;
	_melody.clear();

	getScreen()->clear();
	_vm->switchEventHandler(getScene());

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////
void PuzzleHiveMachine::updateCursor() {
	int32 index = findRect();

	if (_rectIndex == index)
		return;

	_rectIndex = index;
	if (index == -1)
		getCursor()->set(getWorld()->graphicResourceIds[12], -1, kCursorAnimationNone);
	else
		getCursor()->set(getWorld()->graphicResourceIds[12], -1);
}

int32 PuzzleHiveMachine::findRect() {
	Common::Point mouse = getCursor()->position();
	GraphicResource resource(_vm);

	for (uint32 i = 0; i < 5; ++i) {
		resource.load(getWorld()->graphicResourceIds[i + 13]);
		GraphicFrame *frame = resource.getFrame(0);
		Common::Point point(mouse.x - keyPoints[i][0], mouse.y - keyPoints[i][1]);

		if (frame->getRect().contains(point)) {
			point.x -= frame->x;
			point.y -= frame->y;
			if (frame->surface.getPixel(point.x, point.y))
				return i;
		}
	}

	return -1;
}

void PuzzleHiveMachine::updateScreen() {
	getScreen()->clearGraphicsInQueue();
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[8], 0, Common::Point(0, 0), kDrawFlagNone, 0, 2);

	if (_ok) {
		if (_counterGreen > 1)
			--_counterGreen;
		else if (_counterGreen-- == 1) {
			getSound()->playSound(getWorld()->graphicResourceIds[86], false, Config.sfxVolume - 10);
			_vm->setGameFlag(kGameFlagSolveHiveMachine);
			_vm->switchEventHandler(getScene());
		}
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[11], 0, Common::Point(271, 369), kDrawFlagNone, 0, 1);
	} else if (_counterRed) {
		if (_counterRed == 1)
			_notesNumber = 0;
		else if (_counterRed == 30)
			getSound()->playSound(getWorld()->graphicResourceIds[85], false, Config.sfxVolume - 10);
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[10], 0, Common::Point(318, 372), kDrawFlagNone, 0, 1);

		--_counterRed;
	}

	for (uint32 i = 0; i < 5; ++i) {
		uint32 resourceId = i + 13, frameIndex;
		if (_soundingNote == MusicalNote(i)) {
			resourceId += 5;
			frameIndex = _frameIndex1;
		} else
			frameIndex = _frameIndex;
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[resourceId], frameIndex, &keyPoints[i], kDrawFlagNone, 0, 1);
	}

	if (_counterKey)
		--_counterKey;
	else {
		_soundingNote = kMusicalNoteNone;
		_frameIndex1 = 0;
	}

	for (uint32 i = 0; i < _notesNumber; ++i)
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[84], 0, &indicatorPoints[i], kDrawFlagNone, 0, 1);

	_frameIndex = (_frameIndex + 1) % GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[13]);
	if (_counterKey)
		_frameIndex1 = (_frameIndex1 + 1) % GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[18]);
}

void PuzzleHiveMachine::playSound() {
	if (_soundingNote == kMusicalNoteNone)
		error("[PuzzleHiveMachine::playSound] Invalid sound resource id");

	uint32 soundMap[] = {4, 2, 3, 0, 1};
	getSound()->playSound(getWorld()->graphicResourceIds[soundMap[_soundingNote] + 23], false, Config.sfxVolume - 10);
}

} // End of namespace Asylum
