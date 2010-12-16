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

#include "asylum/puzzles/morguedoor.h"

#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"

namespace Asylum {

PuzzleMorgueDoor::PuzzleMorgueDoor(AsylumEngine *engine) : Puzzle(engine) {
	memset(&_frameCounts, 0, sizeof(_frameCounts));
	memset(&_frameIndexes, 0, sizeof(_frameIndexes));
	_data_4572A4 = false;
	_data_4572A8 = false;
	_data_4572AC = false;
	_data_4572B0 = false;

	_flag1 = false;
	_flag2 = false;
	_flag3 = false;
}

PuzzleMorgueDoor::~PuzzleMorgueDoor() {
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleMorgueDoor::init()  {
	getCursor()->set(getWorld()->graphicResourceIds[33], -1, kCursorAnimationNone, 7);

	_frameCounts[0] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[21]);
	_frameCounts[1] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[22]);
	_frameCounts[2] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[23]);
	_frameCounts[3] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[24]);
	_frameCounts[4] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[25]);
	_frameCounts[5] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[26]);
	_frameCounts[6] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[27]);
	_frameCounts[7] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[28]);
	_frameCounts[8] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[31]);
	_frameCounts[9] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[32]);

	getScreen()->setPalette(getWorld()->graphicResourceIds[20]);
	getScreen()->setGammaLevel(getWorld()->graphicResourceIds[20], 0);

	return mouseDown();
}

bool PuzzleMorgueDoor::update()  {
	updateCursor();

	// Draw elements
	getScreen()->clearGraphicsInQueue();
	getScreen()->draw(getWorld()->graphicResourceIds[19]);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[21], _frameIndexes[0], Common::Point(47, 0), 0, 0, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[22], _frameIndexes[1], Common::Point(51, 236), 0, 0, 1);

	if (_flag1)
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[23], _frameIndexes[2], Common::Point(80, 0), 0, 0, 1);

	if (_flag2)
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[24], _frameIndexes[3], Common::Point(89, 230), 0, 0, 1);

	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[25], _frameIndexes[4], Common::Point(515, 41), 0, 0, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[26], _frameIndexes[5], Common::Point(267, 190), 0, 0, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[27], _frameIndexes[6], Common::Point(388, 105), 0, 0, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[28], _frameIndexes[7], Common::Point(491, 143), 0, 0, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[29], _frameIndexes[8], Common::Point(347, 124), 0, 0, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[30], _frameIndexes[9], Common::Point(346, 339), 0, 0, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[31], _frameIndexes[10], Common::Point(276, 67), 0, 0, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[32], _frameIndexes[11], Common::Point(278, 378), 0, 0, 1);

	getScreen()->drawGraphicsInQueue();
	getScreen()->copyBackBufferToScreen();

	return true;
}

bool PuzzleMorgueDoor::key(const AsylumEvent &evt) {
	switch (evt.kbd.keycode) {
	default:
		_vm->switchEventHandler(getScene());
		break;

	case Common::KEYCODE_TAB:
		getScreen()->takeScreenshot();
		break;
	}

	return true;
}

bool PuzzleMorgueDoor::mouse(const AsylumEvent &evt) {
	switch (evt.type) {
	case Common::EVENT_LBUTTONDOWN:
		return mouseDown();
		break;

	case Common::EVENT_RBUTTONUP:
		getCursor()->hide();
		getSharedData()->setFlag(kFlag1, true);
		getScreen()->setupPaletteAndStartFade(0, 0, 0);

		_vm->switchEventHandler(getScene());
		break;
	}

	return false;
}

bool PuzzleMorgueDoor::mouseDown() {
	Common::Point mousePos = getCursor()->position();

	if (mousePos.x > 347 && mousePos.x < 357
	 && mousePos.y > 124 && mousePos.y < 154) {
		 _frameIndexes[8] = 1;
		 getSound()->playSound(getWorld()->soundResourceIds[6], false, Config.sfxVolume - 10);
	}

	if (mousePos.x > 346 && mousePos.x < 356
	 && mousePos.y > 339 && mousePos.y < 386) {
		_frameIndexes[9] = 1;
		getSound()->playSound(getWorld()->soundResourceIds[6], false, Config.sfxVolume - 10);
	}

	if (mousePos.x > 515 && mousePos.x < 605
	 && mousePos.y > 41  && mousePos.y < 120) {
		if (_frameIndexes[4] == 0) {
			_frameIndexes[4] = 1;
			getSound()->playSound(getWorld()->soundResourceIds[1], false, Config.sfxVolume - 10);
		}

		return true;
	}

	if (mousePos.x > 267 && mousePos.x < 325
	 && mousePos.y > 190 && mousePos.y < 320) {
		if (_frameIndexes[5] == 0) {
			_frameIndexes[5] = 1;
			getSound()->playSound(getWorld()->soundResourceIds[0], false, Config.sfxVolume - 10);
			_data_4572B0 = true;
		}

		return true;
	}

	if (mousePos.x > 325 && mousePos.x < 383
	 && mousePos.y > 190 && mousePos.y < 320) {
		if (_frameIndexes[5] == 0) {
			_frameIndexes[5] = 14;
			getSound()->playSound(getWorld()->soundResourceIds[0], false, Config.sfxVolume - 10);
			_data_4572B0 = false;
		}

		return true;
	}

	if (mousePos.x > 507 && mousePos.x < 556
	 && mousePos.y > 124 && mousePos.y < 177) {
		if (_frameIndexes[7] == 4) {
			getSound()->playSound(getWorld()->soundResourceIds[2], false, Config.sfxVolume - 10);
			_data_4572A8 = true;
			_data_4572AC = false;
		}

		return true;
	}

	if (mousePos.x > 556 && mousePos.x < 605
	 && mousePos.y > 124 && mousePos.y < 177) {
		if (_frameIndexes[7] == 4) {
			getSound()->playSound(getWorld()->soundResourceIds[2], false, Config.sfxVolume - 10);
			_data_4572A8 = true;
			_data_4572AC = false;
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////
void PuzzleMorgueDoor::updateCursor() {
	Common::Point mousePos = getCursor()->position();

	bool animate = false;

	if (mousePos.x > 347 && mousePos.x < 357 && mousePos.y > 124 && mousePos.y < 154)
		animate = true;

	if (mousePos.x > 346 && mousePos.x < 356 && mousePos.y > 339 && mousePos.y < 386)
		animate = true;

	if (mousePos.x > 515 && mousePos.x < 605 && mousePos.y > 41  && mousePos.y < 120)
		animate = true;

	if (mousePos.x > 267 && mousePos.x < 325 && mousePos.y > 190 && mousePos.y < 320)
		animate = true;

	if (mousePos.x > 325 && mousePos.x < 383 && mousePos.y > 190 && mousePos.y < 320)
		animate = true;

	if (mousePos.x > 507 && mousePos.x < 556 && mousePos.y > 124 && mousePos.y < 177)
		animate = true;

	if (mousePos.x > 556 && mousePos.x < 605 && mousePos.y > 124 && mousePos.y < 177)
		animate = true;

	// Default cursor
	if (animate) {
		if (getCursor()->animation != kCursorAnimationMirror) {
			getCursor()->set(getWorld()->graphicResourceIds[33], -1, kCursorAnimationMirror, 7);
		}
	} else if (getCursor()->animation != kCursorAnimationNone) {
		getCursor()->set(getWorld()->graphicResourceIds[33], -1, kCursorAnimationNone, 7);
	}
}

} // End of namespace Asylum
