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

	_data_45A9D8 = 0;
	_data_45A9DC = 0;

	_topLeverOpen = false;
	_bottomLeverOpen = false;
	_flag3 = false;
	_flag4 = false;
	_flag5 = false;
	_flag6 = false;
	_flag7 = false;
}

PuzzleMorgueDoor::~PuzzleMorgueDoor() {
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleMorgueDoor::init()  {
	getCursor()->set(getWorld()->graphicResourceIds[33], -1, kCursorAnimationNone, 7);

	_frameCounts[kTopLever] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[21]);
	_frameCounts[kBottomLever] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[22]);
	_frameCounts[kTopLeverOpened] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[23]);
	_frameCounts[kBottomLeverOpened] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[24]);
	_frameCounts[kTopRightValve] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[25]);
	_frameCounts[kCenterValve] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[26]);
	_frameCounts[kRightGear] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[27]);
	_frameCounts[kTopRightLever] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[28]);
	_frameCounts[kTopGear] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[31]);
	_frameCounts[kBottomGear] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[32]);

	getScreen()->setPalette(getWorld()->graphicResourceIds[20]);
	getScreen()->setGammaLevel(getWorld()->graphicResourceIds[20], 0);

	return mouseDown();
}

bool PuzzleMorgueDoor::update()  {
	updateCursor();

	// Draw elements
	getScreen()->clearGraphicsInQueue();
	getScreen()->draw(getWorld()->graphicResourceIds[19]);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[21], _frameIndexes[kTopLever], Common::Point(47, 0), 0, 0, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[22], _frameIndexes[kBottomLever], Common::Point(51, 236), 0, 0, 1);

	if (_topLeverOpen)
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[23], _frameIndexes[kTopLeverOpened], Common::Point(80, 0), 0, 0, 1);

	if (_bottomLeverOpen)
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[24], _frameIndexes[kBottomLeverOpened], Common::Point(89, 230), 0, 0, 1);

	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[25], _frameIndexes[kTopRightValve], Common::Point(515, 41), 0, 0, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[26], _frameIndexes[kCenterValve], Common::Point(267, 190), 0, 0, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[27], _frameIndexes[kRightGear], Common::Point(388, 105), 0, 0, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[28], _frameIndexes[kTopRightLever], Common::Point(491, 143), 0, 0, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[29], _frameIndexes[kTopSmallLever], Common::Point(347, 124), 0, 0, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[30], _frameIndexes[kBottomSmallLever], Common::Point(346, 339), 0, 0, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[31], _frameIndexes[kTopGear], Common::Point(276, 67), 0, 0, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[32], _frameIndexes[kBottomGear], Common::Point(278, 378), 0, 0, 1);

	getScreen()->drawGraphicsInQueue();
	getScreen()->copyBackBufferToScreen();

	updateState();

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

	// Top small lever
	if (mousePos.x > 347 && mousePos.x < 357
	 && mousePos.y > 124 && mousePos.y < 154) {
		 _frameIndexes[kTopSmallLever] = 1;
		 getSound()->playSound(getWorld()->soundResourceIds[6], false, Config.sfxVolume - 10);
	}

	// Bottom small lever
	if (mousePos.x > 346 && mousePos.x < 356
	 && mousePos.y > 339 && mousePos.y < 386) {
		_frameIndexes[kBottomSmallLever] = 1;
		getSound()->playSound(getWorld()->soundResourceIds[6], false, Config.sfxVolume - 10);
	}

	// Top Right Valve
	if (mousePos.x > 515 && mousePos.x < 605
	 && mousePos.y > 41  && mousePos.y < 120) {
		if (_frameIndexes[kTopRightValve] == 0) {
			_frameIndexes[kTopRightValve] = 1;
			getSound()->playSound(getWorld()->soundResourceIds[1], false, Config.sfxVolume - 10);
		}

		return true;
	}

	// Center Valve turning left
	if (mousePos.x > 267 && mousePos.x < 325
	 && mousePos.y > 190 && mousePos.y < 320) {
		if (_frameIndexes[kCenterValve] == 0) {
			_frameIndexes[kCenterValve] = 1;
			getSound()->playSound(getWorld()->soundResourceIds[0], false, Config.sfxVolume - 10);
			_data_4572B0 = true;
		}

		return true;
	}

	// Center Valve turning right
	if (mousePos.x > 325 && mousePos.x < 383
	 && mousePos.y > 190 && mousePos.y < 320) {
		if (_frameIndexes[kCenterValve] == 0) {
			_frameIndexes[kCenterValve] = 14;
			getSound()->playSound(getWorld()->soundResourceIds[0], false, Config.sfxVolume - 10);
			_data_4572B0 = false;
		}

		return true;
	}

	// Top right lever moving left
	if (mousePos.x > 507 && mousePos.x < 556
	 && mousePos.y > 124 && mousePos.y < 177) {
		if (_frameIndexes[kTopRightLever] == 4) {
			getSound()->playSound(getWorld()->soundResourceIds[2], false, Config.sfxVolume - 10);
			_data_4572A8 = true;
			_data_4572AC = false;
		}

		return true;
	}

	// Top right lever moving right
	if (mousePos.x > 556 && mousePos.x < 605
	 && mousePos.y > 124 && mousePos.y < 177) {
		if (_frameIndexes[kTopRightLever] == 4) {
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

void PuzzleMorgueDoor::updateState() {
	if (_flag3) {
		warning("[PuzzleMorgueDoor::updateState] Not implemented (flag 3)!");
	}

	if (_flag4) {
		switch (_data_45A9DC) {
		default:
			break;

		case 1:
			--_frameIndexes[kBottomGear];

			if (_frameIndexes[kBottomGear] < 0) {
				_data_45A9DC = 0;
				_frameIndexes[kBottomGear] = 0;

				getSound()->stop(getWorld()->soundResourceIds[8]);
			}
			break;

		case 2:
			--_frameIndexes[kBottomGear];

			if (_frameIndexes[kBottomGear] < 0) {
				_data_45A9DC = 1;
				_frameIndexes[kBottomGear] = 10;
			}
			break;

		case 3:
			--_frameIndexes[kBottomGear];

			if (_frameIndexes[kBottomGear] < 0) {
				_data_45A9DC = 2;
				_frameIndexes[kBottomGear] = 10;
			}
			break;
		}

		if (!_flag7 || _data_45A9DC < 3)
			_frameIndexes[kBottomLever] = 5 * _data_45A9DC;
	}

	// Update gears
	if (_flag5) {
		--_frameIndexes[kRightGear];

		if (_data_4572AC) {
			if (_data_45A9D8 < 3)
				_frameIndexes[kTopGear] = 10 * (14 - _frameIndexes[kRightGear]) / 14;
		} else {
			if (_data_45A9DC < 3)
				_frameIndexes[kBottomGear] = 10 * (14 - _frameIndexes[kRightGear]) / 14;
		}

		if (!_frameIndexes[kRightGear]) {
			_flag5 = false;

			if (_data_4572AC) {
				if (_frameIndexes[kTopLever] < 15)
					_frameIndexes[kTopLever] += 5;

				if (_data_45A9D8 < 3)
					++_data_45A9D8;

				if (_data_45A9D8 == 3 && _flag6)
					--_frameIndexes[kTopLever];
			} else {
				if (_frameIndexes[kBottomLever] < 15)
					_frameIndexes[kBottomLever] += 5;

				if (_data_45A9DC < 3)
					++_data_45A9DC;

				if (_data_45A9DC == 3 && _flag7)
					--_frameIndexes[kBottomLever];
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Update levers & gears
	if (_frameIndexes[kTopRightValve] % 3 == 1 || (_frameIndexes[kTopRightValve] == 15 && _frameIndexes[kRightGear] > 13))
		if (_frameIndexes[kRightGear] < 14)
			++_frameIndexes[kRightGear];

	if (_frameIndexes[kRightGear] > 14 && _frameIndexes[kTopRightValve] == 4)
		_frameIndexes[kRightGear] = 14;

	if (_data_4572A4) {
		--_frameIndexes[kTopRightLever];

		if (_frameIndexes[kTopRightLever] < 0) {
			_frameIndexes[kTopRightLever] = 0;
			_data_4572A4 = false;

			if (_frameIndexes[kRightGear] == 14) {
				if (_data_45A9D8 < 3)
					getSound()->playSound(getWorld()->soundResourceIds[7], false, Config.sfxVolume, getWorld()->reverseStereo ? 2000 : -2000);

				getSound()->playSound(getWorld()->soundResourceIds[7], false, Config.sfxVolume - 100, getWorld()->reverseStereo ? -3000 : 3000);
				_flag5 = true;
			}
		}
	} else {
		if (_data_4572A8) {
			++_frameIndexes[kTopRightLever];

			if (_frameIndexes[kTopRightLever] > 7) {
				_frameIndexes[kTopRightLever] = 7;
				_data_4572A8 = false;

				if (_frameIndexes[kRightGear] == 14) {
					if (_data_45A9DC < 3)
						getSound()->playSound(getWorld()->soundResourceIds[7], false, Config.sfxVolume, getWorld()->reverseStereo ? 2000 : -2000);

					getSound()->playSound(getWorld()->soundResourceIds[7], false, Config.sfxVolume - 100, getWorld()->reverseStereo ? -3000 : 3000);
					_flag5 = true;
				}
			}
		} else {
			if (_frameIndexes[kTopRightLever] > 4)
				--_frameIndexes[kTopRightLever];
			else if (_frameIndexes[kTopRightLever] < 4)
				++_frameIndexes[kTopRightLever];
		}
	}

	// Left valve
	if (_frameIndexes[kTopRightValve])
		++_frameIndexes[kTopRightValve];

	if (_frameIndexes[kCenterValve]) {
		warning("[PuzzleMorgueDoor::updateState] Not implemented (centerValve)!");
	}

	//////////////////////////////////////////////////////////////////////////
	// Adjust frame indexes
	//////////////////////////////////////////////////////////////////////////
	if (_frameIndexes[kTopGear] > _frameCounts[kTopGear] - 1)
		_frameIndexes[kTopGear] = 0;

	if (_frameIndexes[kBottomGear] > _frameCounts[kBottomGear] - 1)
		_frameIndexes[kBottomGear] = 0;

	if (_frameIndexes[kBottomLever] > _frameCounts[kBottomLever] - 1)
		_frameIndexes[kBottomLever] = 0;

	if (_frameIndexes[kTopLeverOpened] > _frameCounts[kTopLeverOpened] - 1 || _frameIndexes[kTopLeverOpened] < 0)
		_frameIndexes[kTopLeverOpened] = 0;

	if (_frameIndexes[kBottomLeverOpened] > _frameCounts[kBottomLeverOpened] - 1 || _frameIndexes[kBottomLeverOpened] < 0)
		_frameIndexes[kBottomLeverOpened] = 0;

	if (_frameIndexes[kTopRightValve] > _frameCounts[kTopRightValve] - 1 || _frameIndexes[kTopRightValve] < 0)
		_frameIndexes[kTopRightValve] = 0;

	if (_frameIndexes[kCenterValve] > _frameCounts[kCenterValve] - 1)
		_frameIndexes[kCenterValve] = 0;

	if (_frameIndexes[kRightGear] > _frameCounts[kRightGear] - 1)
		_frameIndexes[kRightGear] = 0;

	if (_frameIndexes[kTopRightLever] > _frameCounts[kTopRightLever] - 1)
		_frameIndexes[kTopRightLever] = 0;

	// Top small lever
	if (_frameIndexes[kTopSmallLever] != 0) {
		++_frameIndexes[kTopSmallLever];

		if (_frameIndexes[kTopSmallLever] > 5) {
			_frameIndexes[kTopSmallLever] = 0;

			if (_data_45A9D8 > 0) {
				getSound()->playSound(getWorld()->graphicResourceIds[8], false, Config.sfxVolume - 10);
				_flag3 = true;
			}

			_frameIndexes[10] = 10;
		}
	}

	// Bottom small lever
	if (_frameIndexes[kBottomSmallLever] != 0) {
		++_frameIndexes[kBottomSmallLever];

		if (_frameIndexes[kBottomSmallLever] > 5) {
			_frameIndexes[kBottomSmallLever] = 0;

			if (_data_45A9DC > 0) {
				getSound()->playSound(getWorld()->graphicResourceIds[8], false, Config.sfxVolume - 10);
				_flag4 = true;
			}
		}
	}

	if (_flag6 && _flag7) {
		_vm->setGameFlag(kGameFlag608);
		_vm->setGameFlag(kGameFlag384);
		_vm->setGameFlag(kGameFlag391);

		// Original waits for 2000 ticks
		g_system->delayMillis(2000);

		getCursor()->hide();
		getSharedData()->setFlag(kFlag1, true);
		getScreen()->setupPaletteAndStartFade(0, 0, 0);

		_vm->switchEventHandler(getScene());
	}
}

} // End of namespace Asylum
