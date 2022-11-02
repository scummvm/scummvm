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
	_moveTopGear = false;
	_moveBottomGear = false;
	_flag5 = false;
	_flag6 = false;
	_flag7 = false;
}

PuzzleMorgueDoor::~PuzzleMorgueDoor() {
}

void PuzzleMorgueDoor::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsSint32LE(_frameIndexes[kTopLeverOpened]);
	s.syncAsSint32LE(_frameIndexes[kBottomLeverOpened]);
	s.syncAsSint32LE(_frameIndexes[kBottomGear]);
	s.syncAsSint32LE(_frameIndexes[kTopLever]);
	s.syncAsSint32LE(_frameIndexes[kBottomLever]);
	s.syncAsSint32LE(_frameIndexes[kTopRightLever]);
	s.syncAsSint32LE(_frameIndexes[kTopSmallLever]);
	s.syncAsSint32LE(_frameIndexes[kBottomSmallLever]);
	s.syncAsSint32LE(_frameIndexes[kRightGear]);
	s.syncAsSint32LE(_frameIndexes[kTopGear]);
	s.syncAsSint32LE(_frameIndexes[kTopRightValve]);
	s.syncAsSint32LE(_frameIndexes[kCenterValve]);

	s.syncAsSint32LE(_data_45A9D8);
	s.syncAsSint32LE(_data_45A9DC);
	s.syncAsSint32LE(_flag6);
	s.syncAsSint32LE(_flag7);
	s.syncAsSint32LE(_topLeverOpen);
	s.syncAsSint32LE(_bottomLeverOpen);
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleMorgueDoor::init(const AsylumEvent &evt)  {
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
	getScreen()->setGammaLevel(getWorld()->graphicResourceIds[20]);

	return mouseLeftDown(evt);
}

void PuzzleMorgueDoor::updateScreen()  {
	// Draw elements
	getScreen()->clearGraphicsInQueue();
	getScreen()->fillRect(0, 0, 640, 480, 252);
	getScreen()->draw(getWorld()->graphicResourceIds[19], 0, Common::Point(0, 0), kDrawFlagNone, true);

	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[21], (uint32)_frameIndexes[kTopLever], Common::Point(47, 0), kDrawFlagNone, 0, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[22], (uint32)_frameIndexes[kBottomLever], Common::Point(51, 236), kDrawFlagNone, 0, 1);

	if (_topLeverOpen)
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[23], (uint32)_frameIndexes[kTopLeverOpened], Common::Point(80, 0), kDrawFlagNone, 0, 1);

	if (_bottomLeverOpen)
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[24], (uint32)_frameIndexes[kBottomLeverOpened], Common::Point(89, 230), kDrawFlagNone, 0, 1);

	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[25], (uint32)_frameIndexes[kTopRightValve], Common::Point(515, 41), kDrawFlagNone, 0, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[26], (uint32)_frameIndexes[kCenterValve], Common::Point(267, 190), kDrawFlagNone, 0, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[27], (uint32)_frameIndexes[kRightGear], Common::Point(388, 105), kDrawFlagNone, 0, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[28], (uint32)_frameIndexes[kTopRightLever], Common::Point(491, 143), kDrawFlagNone, 0, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[29], (uint32)_frameIndexes[kTopSmallLever], Common::Point(347, 124), kDrawFlagNone, 0, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[30], (uint32)_frameIndexes[kBottomSmallLever], Common::Point(346, 339), kDrawFlagNone, 0, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[31], (uint32)_frameIndexes[kTopGear], Common::Point(276, 67), kDrawFlagNone, 0, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[32], (uint32)_frameIndexes[kBottomGear], Common::Point(278, 378), kDrawFlagNone, 0, 1);

	updateState();
}

bool PuzzleMorgueDoor::mouseLeftDown(const AsylumEvent &evt) {
	Common::Point mousePos = evt.mouse;

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
			_data_4572A4 = true;
			_data_4572AC = true;
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

bool PuzzleMorgueDoor::mouseRightUp(const AsylumEvent &) {
	getCursor()->hide();
	getSharedData()->setFlag(kFlag1, true);
	getScreen()->stopPaletteFade(0, 0, 0);

	_vm->switchEventHandler(getScene());

	return false;
}

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////
void PuzzleMorgueDoor::updateCursor() {
	bool animate = false;
	Common::Point mousePos = getCursor()->position();

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
		if (getCursor()->getAnimation() != kCursorAnimationMirror) {
			getCursor()->set(getWorld()->graphicResourceIds[33], -1, kCursorAnimationMirror, 7);
		}
	} else if (getCursor()->getAnimation() != kCursorAnimationNone) {
		getCursor()->set(getWorld()->graphicResourceIds[33], -1, kCursorAnimationNone, 7);
	}
}

void PuzzleMorgueDoor::updateState() {
	// Move top gear
	if (_moveTopGear) {
		switch (_data_45A9D8) {
		default:
			break;

		case 1:
			--_frameIndexes[kTopGear];

			if (_frameIndexes[kTopGear] < 0) {
				_data_45A9D8 = 0;
				_moveTopGear = false;
				_frameIndexes[kTopGear] = 0;
				getSound()->stop(getWorld()->soundResourceIds[8]);
			}
			break;

		case 2:
			--_frameIndexes[kTopGear];

			if (_frameIndexes[kTopGear] < 0) {
				_data_45A9D8 = 1;
				_frameIndexes[kTopGear] = 10;
			}
			break;

		case 3:
			--_frameIndexes[kTopGear];

			if (_frameIndexes[kTopGear] < 0) {
				_data_45A9D8 = 2;
				_frameIndexes[kTopGear] = 10;
			}
			break;
		}

		if (!_flag6 || _data_45A9D8 < 3)
			_frameIndexes[kTopLever] = 5 * _data_45A9D8;
	}

	// Move bottom gear
	if (_moveBottomGear) {
		switch (_data_45A9DC) {
		default:
			break;

		case 1:
			--_frameIndexes[kBottomGear];

			if (_frameIndexes[kBottomGear] < 0) {
				_data_45A9DC = 0;
				_frameIndexes[kBottomGear] = 0;
				_moveBottomGear = false;

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
					_frameIndexes[kTopLever] = _frameCounts[kTopLever] - 1;
			} else {
				if (_frameIndexes[kBottomLever] < 15)
					_frameIndexes[kBottomLever] += 5;

				if (_data_45A9DC < 3)
					++_data_45A9DC;

				if (_data_45A9DC == 3 && _flag7)
					_frameIndexes[kBottomLever] = _frameCounts[kBottomLever] - 1;
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

				getSound()->playSound(getWorld()->soundResourceIds[5], false, Config.sfxVolume - 100, getWorld()->reverseStereo ? -3000 : 3000);
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
					if (_data_45A9D8 < 3)
						getSound()->playSound(getWorld()->soundResourceIds[7], false, Config.sfxVolume, getWorld()->reverseStereo ? 2000 : -2000);

					getSound()->playSound(getWorld()->soundResourceIds[5], false, Config.sfxVolume - 100, getWorld()->reverseStereo ? -3000 : 3000);
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
		if (_data_4572B0) {
			if (!_flag6 || _frameIndexes[kTopLever] < 15) {
				if (_flag7 && _frameIndexes[kBottomLever] >= 15) {
					_frameIndexes[kCenterValve] = 0;
					getSound()->stop(getWorld()->soundResourceIds[0]);
				} else {
					++_frameIndexes[kCenterValve];

					// Update top lever
					if (_frameIndexes[kTopLever] >= 15) {
						if (_frameIndexes[kTopLever] == 15)
							getSound()->playSound(getWorld()->soundResourceIds[4], false, Config.sfxVolume - 10);

						if (_frameIndexes[kTopLever] >= _frameCounts[kTopLever] - 1) {
							_frameIndexes[kCenterValve] = 0;
							_topLeverOpen = true;
							_flag6 = true;
						} else {
							++_frameIndexes[kTopLever];
						}
					} else {
						if (_frameIndexes[kCenterValve] >= 14)
							_frameIndexes[kTopLever] = 5 * _data_45A9D8;
						else
							_frameIndexes[kTopLever] = _frameIndexes[kCenterValve] / 3 + 5 * _data_45A9D8;
					}

					// Update bottom lever
					if (_frameIndexes[kBottomLever] >= 15) {
						if (_frameIndexes[kBottomLever] == 15)
							getSound()->playSound(getWorld()->soundResourceIds[4], false, Config.sfxVolume - 10);

						if (_frameIndexes[kBottomLever] >= _frameCounts[kBottomLever] - 1) {
							_frameIndexes[kCenterValve] = 0;
							_bottomLeverOpen = true;
							_flag7 = true;
						} else {
							++_frameIndexes[kBottomLever];
						}
					} else {
						if (_frameIndexes[kCenterValve] >= 14)
							_frameIndexes[kBottomLever] = 5 * _data_45A9DC;
						else
							_frameIndexes[kBottomLever] = _frameIndexes[kCenterValve] / 3 + 5 * _data_45A9DC;
					}
				}
			} else {
				_frameIndexes[kCenterValve] = 0;
				getSound()->stop(getWorld()->soundResourceIds[0]);
			}
			goto updateIndices;
		}

		if ((!_flag6 && _frameIndexes[kTopLever] >= 15)
		 || (!_flag7 && _frameIndexes[kBottomLever] >= 15)) {
			_frameIndexes[kCenterValve] = 0;
			getSound()->stop(getWorld()->soundResourceIds[0]);
		} else {
			--_frameIndexes[kCenterValve];

			// Top lever
			if (_frameIndexes[kTopLever] >= 15) {
				if (_frameIndexes[kTopLever] == 20)
					getSound()->playSound(getWorld()->soundResourceIds[3], false, Config.sfxVolume - 10);

				if (_frameIndexes[kTopLever] <= 15) {
					_frameIndexes[kCenterValve] = 0;
					_flag6 = false;
				} else {
					--_frameIndexes[kTopLever];
				}

				_topLeverOpen = false;
			} else {
				_frameIndexes[kTopLever] = _frameIndexes[kCenterValve] / 3 + 5 * _data_45A9D8;
			}

			// Bottom lever
			if (_frameIndexes[kBottomLever] >= 15) {
				if (_frameIndexes[kBottomLever] == 20)
					getSound()->playSound(getWorld()->soundResourceIds[3], false, Config.sfxVolume - 10);

				if (_frameIndexes[kBottomLever] <= 15) {
					_frameIndexes[kCenterValve] = 0;
					_flag7 = false;
				} else {
					--_frameIndexes[kBottomLever];
				}

				_bottomLeverOpen = false;
			} else {
				_frameIndexes[kBottomLever] = _frameIndexes[kCenterValve] / 3 + 5 * _data_45A9DC;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Adjust frame indexes
	//////////////////////////////////////////////////////////////////////////
updateIndices:
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
				getSound()->playSound(getWorld()->soundResourceIds[8], false, Config.sfxVolume - 10);
				_moveTopGear = true;
			}

			_frameIndexes[kTopGear] = 10;
		}
	}

	// Bottom small lever
	if (_frameIndexes[kBottomSmallLever] != 0) {
		++_frameIndexes[kBottomSmallLever];

		if (_frameIndexes[kBottomSmallLever] > 5) {
			_frameIndexes[kBottomSmallLever] = 0;

			if (_data_45A9DC > 0) {
				getSound()->playSound(getWorld()->soundResourceIds[8], false, Config.sfxVolume - 10);
				_moveBottomGear = true;
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
		getScreen()->stopPaletteFade(0, 0, 0);

		_vm->switchEventHandler(getScene());
	}
}

} // End of namespace Asylum
