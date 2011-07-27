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

#include "asylum/puzzles/hivemachine.h"

#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"

namespace Asylum {

const Common::Point keyPoints[] = {
	Common::Point(246, 106),
	Common::Point(377, 171),
	Common::Point(319, 250),
	Common::Point(220, 249),
	Common::Point(167, 154)
};

const Common::Point indicatorPoints[] = {
	Common::Point(260, 410),
	Common::Point(279, 411),
	Common::Point(298, 411),
	Common::Point(318, 412),
	Common::Point(338, 413),
	Common::Point(358, 413)
};

PuzzleHiveMachine::PuzzleHiveMachine(AsylumEngine *engine) : Puzzle(engine) {
	_counterRed = _counterGreen = _counterKey = 0;
	_rectIndex = -2;
	_frameIndex = 0;
	_soundingNote = kMusicalNoteNone;
	_notesNumber = 0;
	_ok = false;

	_polygons[0].points.push_back(Common::Point(306, 108));
	_polygons[0].points.push_back(Common::Point(334, 111));
	_polygons[0].points.push_back(Common::Point(340, 112));
	_polygons[0].points.push_back(Common::Point(346, 120));
	_polygons[0].points.push_back(Common::Point(352, 127));
	_polygons[0].points.push_back(Common::Point(371, 142));
	_polygons[0].points.push_back(Common::Point(363, 160));
	_polygons[0].points.push_back(Common::Point(363, 172));
	_polygons[0].points.push_back(Common::Point(362, 182));
	_polygons[0].points.push_back(Common::Point(363, 190));
	_polygons[0].points.push_back(Common::Point(348, 202));
	_polygons[0].points.push_back(Common::Point(318, 210));
	_polygons[0].points.push_back(Common::Point(292, 194));
	_polygons[0].points.push_back(Common::Point(278, 193));
	_polygons[0].points.push_back(Common::Point(268, 180));
	_polygons[0].points.push_back(Common::Point(270, 162));
	_polygons[0].points.push_back(Common::Point(266, 152));
	_polygons[0].points.push_back(Common::Point(273, 137));
	_polygons[0].points.push_back(Common::Point(283, 120));

	_polygons[1].points.push_back(Common::Point(436, 171));
	_polygons[1].points.push_back(Common::Point(464, 181));
	_polygons[1].points.push_back(Common::Point(482, 201));
	_polygons[1].points.push_back(Common::Point(472, 228));
	_polygons[1].points.push_back(Common::Point(473, 245));
	_polygons[1].points.push_back(Common::Point(460, 262));
	_polygons[1].points.push_back(Common::Point(452, 262));
	_polygons[1].points.push_back(Common::Point(416, 244));
	_polygons[1].points.push_back(Common::Point(403, 231));
	_polygons[1].points.push_back(Common::Point(378, 220));
	_polygons[1].points.push_back(Common::Point(379, 215));
	_polygons[1].points.push_back(Common::Point(387, 201));
	_polygons[1].points.push_back(Common::Point(405, 177));

	_polygons[2].points.push_back(Common::Point(357, 249));
	_polygons[2].points.push_back(Common::Point(368, 252));
	_polygons[2].points.push_back(Common::Point(407, 277));
	_polygons[2].points.push_back(Common::Point(415, 283));
	_polygons[2].points.push_back(Common::Point(422, 294));
	_polygons[2].points.push_back(Common::Point(413, 314));
	_polygons[2].points.push_back(Common::Point(413, 324));
	_polygons[2].points.push_back(Common::Point(406, 337));
	_polygons[2].points.push_back(Common::Point(386, 343));
	_polygons[2].points.push_back(Common::Point(368, 350));
	_polygons[2].points.push_back(Common::Point(354, 347));
	_polygons[2].points.push_back(Common::Point(335, 347));
	_polygons[2].points.push_back(Common::Point(325, 339));
	_polygons[2].points.push_back(Common::Point(320, 319));
	_polygons[2].points.push_back(Common::Point(325, 308));
	_polygons[2].points.push_back(Common::Point(324, 292));
	_polygons[2].points.push_back(Common::Point(329, 286));
	_polygons[2].points.push_back(Common::Point(341, 272));
	_polygons[2].points.push_back(Common::Point(347, 253));

	_polygons[3].points.push_back(Common::Point(247, 249));
	_polygons[3].points.push_back(Common::Point(260, 263));
	_polygons[3].points.push_back(Common::Point(279, 279));
	_polygons[3].points.push_back(Common::Point(298, 290));
	_polygons[3].points.push_back(Common::Point(299, 308));
	_polygons[3].points.push_back(Common::Point(303, 322));
	_polygons[3].points.push_back(Common::Point(294, 337));
	_polygons[3].points.push_back(Common::Point(286, 342));
	_polygons[3].points.push_back(Common::Point(241, 342));
	_polygons[3].points.push_back(Common::Point(232, 338));
	_polygons[3].points.push_back(Common::Point(219, 315));
	_polygons[3].points.push_back(Common::Point(224, 284));
	_polygons[3].points.push_back(Common::Point(224, 267));

	_polygons[4].points.push_back(Common::Point(222, 156));
	_polygons[4].points.push_back(Common::Point(231, 164));
	_polygons[4].points.push_back(Common::Point(237, 174));
	_polygons[4].points.push_back(Common::Point(246, 184));
	_polygons[4].points.push_back(Common::Point(258, 187));
	_polygons[4].points.push_back(Common::Point(266, 183));
	_polygons[4].points.push_back(Common::Point(275, 195));
	_polygons[4].points.push_back(Common::Point(261, 198));
	_polygons[4].points.push_back(Common::Point(253, 205));
	_polygons[4].points.push_back(Common::Point(247, 215));
	_polygons[4].points.push_back(Common::Point(243, 232));
	_polygons[4].points.push_back(Common::Point(246, 247));
	_polygons[4].points.push_back(Common::Point(226, 261));
	_polygons[4].points.push_back(Common::Point(202, 247));
	_polygons[4].points.push_back(Common::Point(186, 236));
	_polygons[4].points.push_back(Common::Point(184, 227));
	_polygons[4].points.push_back(Common::Point(175, 217));
	_polygons[4].points.push_back(Common::Point(170, 210));
	_polygons[4].points.push_back(Common::Point(171, 196));
	_polygons[4].points.push_back(Common::Point(175, 186));
	_polygons[4].points.push_back(Common::Point(177, 180));
	_polygons[4].points.push_back(Common::Point(183, 172));
	_polygons[4].points.push_back(Common::Point(201, 158));
	_polygons[4].points.push_back(Common::Point(207, 158));

	_polygons[0].boundingRect = Common::Rect(264, 106, 370, 211);
	_polygons[1].boundingRect = Common::Rect(376, 171, 483, 264);
	_polygons[2].boundingRect = Common::Rect(317, 247, 424, 352);
	_polygons[3].boundingRect = Common::Rect(217, 247, 305, 345);
	_polygons[4].boundingRect = Common::Rect(168, 154, 278, 264);
}

PuzzleHiveMachine::~PuzzleHiveMachine() {
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleHiveMachine::init(const AsylumEvent &evt)  {
	getScreen()->setPalette(getWorld()->graphicResourceIds[9]);
	getScreen()->setGammaLevel(getWorld()->graphicResourceIds[9]);
	_rectIndex = -2;

	return true;
}

bool PuzzleHiveMachine::update(const AsylumEvent &evt)  {
	updateScreen();
	updateCursor();

	return true;
}

bool PuzzleHiveMachine::mouseLeftDown(const AsylumEvent &evt) {
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

bool PuzzleHiveMachine::mouseRightDown(const AsylumEvent &evt) {
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
	for (uint32 i = 0; i < 5; ++i)
		if (_polygons[i].contains(getCursor()->position()))
			return i;

	return -1;
}

void PuzzleHiveMachine::updateScreen() {
	getScreen()->clear();
	getScreen()->clearGraphicsInQueue();
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[8], 0, Common::Point(0, 0), kDrawFlagNone, 0, 2);

	if (_ok) {
		if (_counterGreen > 1)
			--_counterGreen;
		else if (_counterGreen-- == 1)
			getSound()->playSound(getWorld()->graphicResourceIds[86], false, Config.sfxVolume - 10);
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
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[resourceId], frameIndex, keyPoints[i], kDrawFlagNone, 0, 1);
	}

	if (_counterKey)
		--_counterKey;
	else {
		_soundingNote = kMusicalNoteNone;
		_frameIndex1 = 0;
	}

	for (uint32 i = 0; i < _notesNumber; ++i)
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[84], 0, indicatorPoints[i], kDrawFlagNone, 0, 1);

	_frameIndex = (_frameIndex + 1) % GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[13]);
	if (_counterKey)
		_frameIndex1 = (_frameIndex1 + 1) % GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[18]);

	getScreen()->drawGraphicsInQueue();
	getScreen()->copyBackBufferToScreen();
}

void PuzzleHiveMachine::playSound() {
	uint32 soundMap[] = {4, 2, 3, 0, 1};
	getSound()->playSound(getWorld()->graphicResourceIds[soundMap[_soundingNote] + 23], false, Config.sfxVolume - 10);
}

} // End of namespace Asylum
