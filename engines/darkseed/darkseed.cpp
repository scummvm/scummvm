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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "darkseed/darkseed.h"
#include "anm.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "darkseed/console.h"
#include "darkseed/detection.h"
#include "debugconsole.h"
#include "engines/util.h"
#include "graphics/palette.h"
#include "img.h"
#include "nsp.h"
#include "pal.h"
#include "pic.h"
#include "room.h"
#include "titlefont.h"

namespace Darkseed {

DarkseedEngine *g_engine;

DarkseedEngine::DarkseedEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("Darkseed") {
	g_engine = this;
}

DarkseedEngine::~DarkseedEngine() {
	delete _screen;
	delete _sound;
}

uint32 DarkseedEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String DarkseedEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error DarkseedEngine::run() {
	initGraphics(640, 350);
	_sound = new Sound(_mixer);
	_screen = new Graphics::Screen();
	_tosText = new TosText();
	_tosText->load();
	_console = new Console(_tosText, _sound);
	_player = new Player();

	Img left00Img;
	left00Img.load("art/left00.img");
	Img left01Img;
	left01Img.load("art/left01.img");

	Anm lettersAnm;
	lettersAnm.load("art/letters.anm");
	Img letterD;
	lettersAnm.getImg(6, letterD);
	Img letterD1;
	lettersAnm.getImg(7, letterD1);

	Pal housePalette;
	housePalette.load("art/house.pal");
	TitleFont titleFont;
	titleFont.displayString(0x44, 0xa0, "DEVELOPING NEW WAYS TO AMAZE");

	// Set the engine's debugger console
	setDebugger(new DebugConsole(_tosText));

	_frame.load("cframe.pic");

	_baseSprites.load("cbase.nsp");

	_cursor.updatePosition(0x140,0xaf);
	_cursor.setCursorType(Pointer);

	_player->_position.x = 0x87;
	_player->_position.y = 0x5b;
	_player->_direction = 1;
	_player->_frameIdx = 0;

	_room = new Room(0);

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1) {
		(void)loadGameState(saveSlot);
	} else {
		if (prefsCutsceneId == 'I' || ((prefsCutsceneId == 'S' || prefsCutsceneId == 'B' || prefsCutsceneId == 'C') &&
			  _room->_roomNumber == 0)) {
			_player->loadAnimations("bedsleep.nsp");
			_player->_position.x = 0x87;
			_player->_position.y = 0x5b;
			_player->_frameIdx = 0;
			_player->_direction = 1;
			setupOtherNspAnimation(0, 1);
	//		bVar1 = true;
			if (_currentDay == 1) {
				_console->printTosText(8);
			} else if (_currentDay == 2) {
				_console->printTosText(0xc);
			} else if (_currentDay == 3) {
				_console->printTosText(0xe);
			}
		}
	}

	gameloop();

	delete _room;
	delete _player;

	return Common::kNoError;
}

Common::Error DarkseedEngine::syncGame(Common::Serializer &s) {
	if (_room == nullptr) {
		_room = new Room(0);
	}
	if (_objectVar.sync(s).getCode() != Common::kNoError) {
		error("Failed to sync objects");
	}
	if (_inventory.sync(s).getCode() != Common::kNoError) {
		error("Failed to sync inventory");
	}
	s.syncAsUint32LE(_currentTimeInSeconds);
	s.syncAsByte(_currentDay);
	s.syncAsByte(_room->_roomNumber);
	s.syncAsByte(_previousRoomNumber);
	s.syncAsSint16LE(_player->_position.x);
	s.syncAsSint16LE(_player->_position.y);

	if (_sound->sync(s).getCode() != Common::kNoError) {
		error("Failed to sync sound");
	}
	return Common::kNoError;
}
void DarkseedEngine::fadeOut() {
}

void DarkseedEngine::fadeIn() {

}

void DarkseedEngine::fadeInner(int startValue, int endValue, int increment) {

}

void DarkseedEngine::gameloop() {
	while (!shouldQuit()) {
		updateEvents();
		if (_fullscreenPic) {
			if (_isLeftMouseClicked || _isRightMouseClicked) {
				_isRightMouseClicked = false;
				_isLeftMouseClicked = false;
				delete _fullscreenPic;
				_fullscreenPic = nullptr;
				_room->restorePalette();
			}
		}
		counter_2c85_888b = (counter_2c85_888b + 1) & 0xff;
		if (systemTimerCounter == 5) {
			handleInput();
			if (_objectVar[1] != 0) {
				if (_room->_roomNumber == 30) {
					if (!_inventory.hasObject(18)) {
						_objectVar[1]--;
					}
				} else {
					_objectVar[1]--;
				}
			}
			closeShops();
			if (_room->_roomNumber == 57 && _objectVar.getMoveObjectRoom(28) == 255 && _previousRoomNumber == 54) {
				if (_objectVar.getMoveObjectRoom(28) == 255) {
					if (_objectVar[56] == 4) {
						playSound(21,5,-1);
					}
					if (_objectVar[56] == 6) {
//						LoadModeSong(7);
						playSound(0,6,-1);
						stuffPlayer();
					}
				} else {
//					dcopanim(); TODO annoyingly this animation runs inside this function.
					changeToRoom(59); // TODO implement don't place hero flag.
					_player->_position = {320, 200};
					_player->updateSprite();
					_inventory.gotoJailLogic();

					playSound(0,6,-1);
				}
			}
			if (_currentTimeInSeconds > 35999 && _currentTimeInSeconds < 36005 &&
				((_currentDay == 1 || (_currentDay == 2 && _objectVar[6] != 0)) ||
				  (_currentDay == 3 && _objectVar[29] != 0))) {
				_objectVar.setObjectRunningCode(140, 1); // package delivered.
			}
			if (_room->_roomNumber == 52 && _objectVar[79] == 0) {
				_objectVar.setObjectRunningCode(79, 1);
			}
			if (_room->_roomNumber == 55) {
				_objectVar.setObjectRunningCode(58, 1);
			}
			int prevTime = _currentTimeInSeconds;
			if (_currentTimeInSeconds * 2 != _fttime) {
				_fttime = _currentTimeInSeconds * 2;
			}
			if (!_room->isGiger()) {
				_fttime += 4;
			} else {
				_fttime += 9;
			}
			_currentTimeInSeconds = _fttime / 2;
			if ((_currentTimeInSeconds == 46800 || _currentTimeInSeconds == 46801) && _currentDay == 2 && _room->_roomNumber != 34) {
				_objectVar[62] = 0;
			}
			if (_currentDay == 2 && _currentTimeInSeconds > 64799 && prevTime < 64800 && _objectVar[141] == 4) {
				initDelbertAtSide();
			}
			if (_currentDay == 1 && _currentTimeInSeconds == 64800 && _room->_roomNumber != 16) {
				_objectVar.setMoveObjectRoom(7, 253); // remove scotch from shop.
			}
			if (_timeAdvanceEventSelected && _currentTimeInSeconds < 79200 && !isPlayingAnimation_maybe && !_player->_isAutoWalkingToBed && !_player->_herowaiting) {
				_timeAdvanceEventSelected = false;
				if (((_room->_roomNumber == 30) || ((0 < _objectVar[141] && (_objectVar[141] < 4)))) ||
					(((_room->_roomNumber == 31 || (_room->_roomNumber == 32)) &&
					  (((((_objectVar[141] == 9 || (_objectVar[141] == 6)) || (_objectVar[141] ==  8))
						 || ((_objectVar[141] == 7 || (_objectVar[141] == 12)))) ||
						((_objectVar[141] == 10 || (_objectVar[141] == 5)))))))) {
					if (_room->_roomNumber == 30) {
						_console->printTosText(943);
					}
				} else {
					_currentTimeInSeconds += 3600;
					_currentTimeInSeconds -= (_currentTimeInSeconds % 3600);
					_console->printTosText(942);
					if (_currentDay == 2 && _currentTimeInSeconds == 64800 && _objectVar[141] == 4) {
						initDelbertAtSide();
					}
				}
			}
			if ((_room->_roomNumber < 10 || _room->_roomNumber == 61 || _room->_roomNumber == 62) && _currentTimeInSeconds % 3600 == 0) {
				if (_room->_roomNumber == 7) {
					playSound(45,5,-1);
				}
				else {
					playSound(46,5,-1);
				}
			}
			_room->darkenSky();
			// TODO lots of logic
			if (_currentTimeInSeconds > 79199 && !_player->_isAutoWalkingToBed) {
				if (_room->isOutside() && _room->_roomNumber != 30) {
					_inventory.endOfDayOutsideLogic();
				}
				if (!isPlayingAnimation_maybe) {
					if (_room->_roomNumber == 30) {
						gotosleepinjail();
					} else {
						if (_room->_roomNumber != 10) {
							// TODO release memory.
						}
//						makeroomname(); TODO
//						GetLocationSprites((int)&_file_name);
						if (!_room->isGiger()) {
							if (_currentDay == 3) {
								_console->printTosText(749);
							}
							_player->loadAnimations("cliedown.nsp");
						} else {
							_player->loadAnimations("gliedown.nsp");
						}
						setupOtherNspAnimation(0, 3);
					}
				}
			}
			if (_room->_roomNumber == 30 && _objectVar[1] == 0 && (otherNspAnimationType_maybe != 40 || !isPlayingAnimation_maybe || _currentTimeInSeconds > 79199)) {
				gotosleepinjail();
			}
			updateDisplay(); // Aka serviceRoom()
			_isRightMouseClicked = false;
			_isLeftMouseClicked = false;
			updateHeadache();
		}
		_room->update();
		_frame.draw();
		if (_fullscreenPic) {
			_fullscreenPic->draw(0x45, 0x28);
		} else {
			_room->draw();
			_inventory.draw();
			_sprites.drawSprites();
			_player->draw();
			_console->draw();
			_cursor.draw();
		}
		_screen->makeAllDirty();
		_screen->update();
		wait();
	}
}

void DarkseedEngine::updateEvents() {
	Common::Event event;
//	_isRightMouseClicked = false;
//	_isLeftMouseClicked = false;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_MOUSEMOVE: _cursor.updatePosition(event.mouse.x, event.mouse.y); break;
		case Common::EVENT_RBUTTONDOWN: _isRightMouseClicked = true; break;
//		case Common::EVENT_RBUTTONUP: _isRightMouseClicked = false; break;
		case Common::EVENT_LBUTTONDOWN: _isLeftMouseClicked = true; break;
//		case Common::EVENT_LBUTTONUP: _isLeftMouseClicked = false; break;
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == Common::KEYCODE_t) {
				_timeAdvanceEventSelected = true;
			}
			break;
		default: break;
		}
	}
}

void DarkseedEngine::wait() {
	g_system->delayMillis(16);
	systemTimerCounter++;
	if (systemTimerCounter == 6) {
		systemTimerCounter = 0;
	}
}

static constexpr uint8 walkToDirTbl[] =
	{0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	 0x04, 0x04, 0x04, 0x01, 0x04, 0x04, 0x04, 0x00,
	 0x04, 0x04, 0x04, 0x04, 0x01, 0x04, 0x01, 0x00,
	 0x00, 0x00, 0x04, 0x01, 0x04, 0x04, 0x04, 0x04,
	 0x04, 0x04, 0x04, 0x04, 0x00, 0x04, 0x00, 0x03,
	 0x01, 0x04, 0x04, 0x04, 0x00, 0x04, 0x04, 0x00,
	 0x04, 0x01, 0x04, 0x04, 0x04, 0x00, 0x04, 0x04,
	 0x04, 0x04, 0x00, 0x00, 0x00, 0x04, 0x04, 0x04,
	 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x04, 0x04,
	 0x03, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x03,
	 0x03, 0x03, 0x03, 0x03, 0x01, 0x01, 0x03, 0x03,
	 0x03, 0x03, 0x01, 0x04, 0x03, 0x00, 0x04, 0x00,
	 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04,
	 0x04, 0x04, 0x03, 0x04, 0x04, 0x01, 0x00, 0x04,
	 0x04, 0x04, 0x04, 0x03, 0x04, 0x04, 0x04, 0x01,
	 0x04, 0x04, 0x03, 0x04, 0x04, 0x00, 0x01, 0x00,
	 0x04, 0x04, 0x04, 0x03, 0x04, 0x01, 0x04, 0x00,
	 0x00, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00,
	 0x04, 0x04, 0x04, 0x04, 0x00, 0x04, 0x00, 0x04,
	 0x00, 0x04, 0x00, 0x04, 0x04, 0x04, 0x04, 0x04,
	 0x04, 0x04, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00,
	 0x04, 0x04, 0x04, 0x04, 0x00, 0x04, 0x04, 0x04,
	 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x04,
	 0x04, 0x04, 0x01, 0x04, 0x03, 0x00, 0x04};

static constexpr int16 walkToXTbl[] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 212, 0, 0, 0, 253,
	0, 0, 0, 0, 464, 0, 405, 325,
	325, 325, 0, 505, 0, 0, 0, 0,
	0, 0, 0, 0, 489, 0, 340, 300,
	340, 0, 0, 0, 218, 0, 0, 391,
	0, 308, 0, 0, 0, 484, 0, 0,
	0, 0, 288, 288, 288, 0, 0, 0,
	0, 0, 0, 0, 0, 437, 0, 0,
	437, 0, 0, 0, 0, 0, 0, 338,
	480, 159, 316, 387, 390, 466, 218, 331,
	316, 230, 346, 0, 153, 534, 0, 156,
	0, 0, 0, 0, 0, 0, 297, 0,
	0, 0, 0, 201, 0, 457, 327, 0,
	0, 0, 0, 223, 0, 0, 0, 477,
	0, 0, 202, 0, 0, 221, 464, 369,
	0, 0, 0, 179, 0, 142, 0, 156,
	293, 359, 0, 260, 463, 269, 208, 156,
	0, 0, 0, 0, 251, 0, 357, 0,
	428, 0, 357, 0, 0, 0, 0, 0,
	0, 0, 210, 244, 212, 202, 348, 0,
	0, 0, 0, 0, 389, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 465, 0,
	0, 0, 458, 0, 210, 260, 0
};

static constexpr uint8 walkToYTbl[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0xd7, 0x00, 0x00, 0x00, 0xcf,
	 0x00, 0x00, 0x00, 0x00, 0xd8, 0x00, 0xaa, 0xe1,
	 0xe1, 0xe1, 0x00, 0xc8, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0xd5, 0x00, 0xd4, 0xb7,
	 0xd9, 0x00, 0x00, 0x00, 0xc6, 0x00, 0x00, 0xd3,
	 0x00, 0xb8, 0x00, 0x00, 0x00, 0xaa, 0x00, 0x00,
	 0x00, 0x00, 0xcd, 0xcd, 0xcd, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0xdf, 0x00, 0x00,
	 0xd2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb6,
	 0xdf, 0xbc, 0xa2, 0xa0, 0xa1, 0xaa, 0xdd, 0xcd,
	 0xd1, 0xd8, 0xe2, 0x00, 0xd7, 0xd5, 0x00, 0xaa,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc5, 0x00,
	 0x00, 0x00, 0x00, 0xb8, 0x00, 0xbd, 0xb4, 0x00,
	 0x00, 0x00, 0x00, 0xbe, 0x00, 0x00, 0x00, 0xdb,
	 0x00, 0x00, 0xee, 0x00, 0x00, 0xc5, 0xd9, 0xd8,
	 0x00, 0x00, 0x00, 0xcc, 0x00, 0xd8, 0x00, 0xb6,
	 0xc6, 0xc8, 0x00, 0xd3, 0xba, 0xd2, 0xd2, 0xaa,
	 0x00, 0x00, 0x00, 0x00, 0xe1, 0x00, 0xd6, 0x00,
	 0xe0, 0x00, 0xab, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0xe8, 0xb9, 0xd7, 0xcf, 0xb2, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0xbf, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbc, 0x00,
	 0x00, 0x00, 0xec, 0x00, 0xe8, 0xd3, 0x00
};

void DarkseedEngine::handleInput() {
	// FUN_2022_762b_handles_mouse_click_action
	int currentRoomNumber = _room->_roomNumber;
	if (_player->_isAutoWalkingToBed && currentRoomNumber == 0 && _player->isAtPosition(0xdf, 0xbe)) {
		_player->_isAutoWalkingToBed = false;
		_currentTimeInSeconds = 0x7e90;
		_player->loadAnimations("bedsleep.nsp");
		setupOtherNspAnimation(1,5);
	}

	if (!isPlayingAnimation_maybe) {
		if (!_player->_playerIsChangingDirection) {
			if (currentRoomNumber == 0x39 &&_previousRoomNumber == 0x36) {
				_player->updateSprite();
			} else {
				if (_player->isAtWalkTarget() && !_player->_heroMoving) {
					_player->updateSprite();
				} else {
					if (counter_2c85_888b >= 0 && !_player->isAtWalkTarget()) {
						counter_2c85_888b = 0;
						_player->playerWalkFrameIdx = (_player->playerWalkFrameIdx + 1) % 8;
						if ((_player->playerWalkFrameIdx == 0 || _player->playerWalkFrameIdx == 4)
							&& currentRoomNumber != 0x22 && currentRoomNumber != 0x13
							&& currentRoomNumber != 0x14 && currentRoomNumber != 0x15
							&& currentRoomNumber != 16) {
							//TODO
						//							FUN_1208_0dac_sound_related(0x5c,CONCAT11((char)(uVar7 >> 8),5));
						}
					}
					_player->updateSprite();
				}
				if (_isLeftMouseClicked && _cursor.getY() > 0x28 && !_player->isPlayerWalking_maybe) { // prevLeftMouseButtonState == 0 &&
					if (_actionMode == PointerAction) {
						_player->calculateWalkTarget();
						_player->playerFaceWalkTarget();
					} else {
						int roomObjIdx = _room->getObjectUnderCursor();
						if (roomObjIdx != -1) {
							// 2022:77ce
							// TODO walk player to object.
							_player->_walkToSequence = true;
							_player->_walkToSequencePoint = _cursor.getPosition();
							_player->_sequenceRotation = -1;
							Common::Point currentCursorPos = _cursor.getPosition();
							int objNum = _room->_roomObj[roomObjIdx].objNum;
							if (walkToDirTbl[objNum] != 4) {
								_player->_sequenceRotation = walkToDirTbl[objNum];
								_cursor.updatePosition(walkToXTbl[objNum], walkToYTbl[objNum]);
							}
							if (objNum == 142 && _room->_roomNumber == 2) {
								_player->_sequenceRotation = 0;
								_cursor.updatePosition(347, 189);
							} else if (objNum == 53 && _room->_roomNumber == 15) {
								_player->_sequenceRotation = 0;
								_cursor.updatePosition(369, 216);
							} else if (objNum == 114) {
								if (_cursor.getX() < 321) {
									_player->_sequenceRotation = 3;
									_cursor.updatePosition(169, 178);
								} else {
									_player->_sequenceRotation = 1;
									_cursor.updatePosition(362, 198);
								}
							} else if (objNum == 189 || (objNum == 64 && _room->_roomNumber == 30)) {
								_player->_sequenceRotation = 1;
								_cursor.updatePosition(405, 208);
							} else if (objNum == 50 || objNum == 85 || (objNum >= 163 && objNum <= 168)) {
								_player->_sequenceRotation = 3;
								_cursor.updatePosition(228, 211);
							} else if (objNum == 51 || objNum == 187) {
								_player->_sequenceRotation = 1;
								_cursor.updatePosition(380, 211);
							} else if (objNum == 116 && _actionMode == Unk19Action) {
								_player->_sequenceRotation = 1;
								_cursor.updatePosition(285, 233);
							} else if (objNum == 137) {
								_player->_sequenceRotation = 1;
								if (_actionMode == Unk27Action) {
									_cursor.updatePosition(354, 175);
								} else {
									_cursor.updatePosition(409, 173);
								}
							} else if (objNum == 112 || objNum == 111) {
							_player->_sequenceRotation = 1;
							_cursor.updatePosition(464, 191);
							} else if (objNum == 138 || objNum == 7 || objNum == 152) {
								_player->_sequenceRotation = 1;
								_cursor.updatePosition(292, 208);
							} else if (objNum == 22 || objNum == 42 ||
									   (objNum == 35 && _objectVar[22] < 2 && _cursor.getY() > 40)) {
								_player->_sequenceRotation = 1;
								if (_objectVar[22] == 0 || _objectVar[22] == 1) {
									_cursor.updatePosition(437, 203);
								}
								if (_objectVar[22] == 2) {
									_cursor.updatePosition(427, 196);
								}
								if (_objectVar[22] > 2) {
									_cursor.updatePosition(394, 175);
								}
							}

							if (objNum == 102 && _objectVar[23] != 0 && _actionMode == HandAction) {
								_player->_sequenceRotation = 0;
								_cursor.updatePosition(331, 195);
							} else if (objNum < 104 || objNum > 108) {
								if (objNum == 78) {
									if (_room->_roomNumber == 2) {
										_player->_sequenceRotation = 3;
										_cursor.updatePosition(152, 239);
									} else {
										_player->_sequenceRotation = 1;
										_cursor.updatePosition(497, 220);
									}
								} else if (objNum == 59) {
									if (_room->_roomNumber == 3) {
										_player->_sequenceRotation = 3;
										_cursor.updatePosition(145, 239);
									} else {
										_player->_sequenceRotation = 1;
										_cursor.updatePosition(520, 229);
									}
								}
							} else {
								_player->_sequenceRotation = 3;
								_cursor.updatePosition(_room->_roomObj[roomObjIdx].xOffset + 30, 206);
							}
							if (_room->_roomNumber == 34 || (_room->_roomNumber > 18 && _room->_roomNumber < 24)) {
								_player->_walkTarget = _player->_position;
							} else if (_cursor.getPosition() != _player->_position) {
								_player->calculateWalkTarget();
							}

							_cursor.updatePosition(currentCursorPos.x, currentCursorPos.y);
							_player->playerFaceWalkTarget();
							_player->isPlayerWalking_maybe = true;
						}
					}
				}
				int xDistToTarget = ABS(_player->_walkTarget.x - _player->_position.x);
				int yDistToTarget = ABS(_player->_walkTarget.y - _player->_position.y);

				if (_isRightMouseClicked && !_player->isPlayerWalking_maybe) {
					if (_actionMode == LookAction) {
						_actionMode = PointerAction;
					} else if (_actionMode == PointerAction) {
						_actionMode = HandAction;
					} else if (_actionMode == HandAction) {
						_actionMode = LookAction;
					} else {
						_actionMode = PointerAction;
					}
					_cursor.setCursorType((CursorType)_actionMode);
				}
				if (_player->isAtWalkTarget() && _player->_heroMoving && _player->walkPathIndex != -1) {
					_player->walkToNextConnector();
				}
				if (_isLeftMouseClicked && _cursor.getY() < 41) {
					_inventory.handleClick();
				}
				_room->calculateScaledSpriteDimensions(_player->getWidth(), _player->getHeight(), _player->_position.y);

				if (_player->isAtWalkTarget() && _player->_heroMoving && !_player->isPlayerWalking_maybe) {
					if (useDoorTarget) {
						_player->changeDirection(_player->_direction, targetPlayerDirection);
						useDoorTarget = false;
						_doorEnabled = true;
						return;
					}
					_player->_heroMoving = false;
					if (useDoorTarget || _doorEnabled) {
						for (int i = 0; i < _room->room1.size(); i++) {
							RoomExit &roomExit = _room->room1[i];
							if (roomExit.roomNumber != 0xff
								&& roomExit.x < scaledSpriteWidth / 2 + _player->_position.x
								&& _player->_position.x - scaledSpriteWidth / 2 < roomExit.x + roomExit.width
								&& roomExit.y < _player->_position.y
								&& _player->_position.y - scaledSpriteHeight < roomExit.y + roomExit.height
								&& roomExit.direction == _player->_direction) {
								bool bVar = true;
								if (currentRoomNumber == 64 && roomExit.roomNumber == 64) {
									bVar = false;
									_console->printTosText(703);
								}
								if (currentRoomNumber == 67 && roomExit.roomNumber == 0) {
									bVar = false;
									_console->printTosText(902);
								}
								if ((currentRoomNumber == 59 && _objectVar[190] < 2) ||
									(currentRoomNumber == 61 && _objectVar[22] < 3 && roomExit.roomNumber == 13) ||
									(currentRoomNumber == 7 && roomExit.roomNumber == 38 && _objectVar[137] == 0) ||
									(currentRoomNumber == 46 && roomExit.roomNumber == 60 && _objectVar[57] == 1) ||
									(currentRoomNumber == 7 && roomExit.roomNumber == 38 && _objectVar[57] == 1) ||
									(currentRoomNumber == 13 && roomExit.roomNumber == 31 && _objectVar[23] != 1) ||
									(currentRoomNumber == 2 && roomExit.roomNumber == 0 && _objectVar[78] != 2) ||
									(currentRoomNumber == 0 && roomExit.roomNumber == 2 && _objectVar[78] != 2) ||
									(currentRoomNumber == 32 && roomExit.roomNumber == 13 && _objectVar[23] != 1) ||
									(currentRoomNumber == 13 && roomExit.roomNumber == 32 && _objectVar[23] != 1) ||
									(currentRoomNumber == 39 && roomExit.roomNumber == 46 && _objectVar[117] == 0) ||
									(currentRoomNumber == 3 && roomExit.roomNumber == 9 && _objectVar[59] != 2) ||
									(currentRoomNumber == 9 && roomExit.roomNumber == 3 && _objectVar[59] != 2)) {
									bVar = false;
								}
								if (_currentTimeInSeconds > 64800 &&
									((currentRoomNumber == 11 && roomExit.roomNumber == 15) ||
									 (currentRoomNumber == 11 && roomExit.roomNumber == 16) ||
									 (currentRoomNumber == 12 && roomExit.roomNumber == 17))) {
									bVar = false;
									_console->printTosText(936);
								}
								if (currentRoomNumber == 7 && roomExit.roomNumber == 38 && bVar) {
									_player->loadAnimations("mirror.nsp");
									setupOtherNspAnimation(0,27);
									return;
								}
								if ((currentRoomNumber == 41 && roomExit.roomNumber == 44 && bVar) ||
									(currentRoomNumber == 44 && roomExit.roomNumber == 41 && bVar)) {
									_player->loadAnimations("beamer.nsp");
									setupOtherNspAnimation(0,57);
									return;
								}
								if (currentRoomNumber == 10 && roomExit.roomNumber == 6 && bVar && !_player->_isAutoWalkingToBed) {
									_player->loadAnimations("rm10strs.nsp");
									setupOtherNspAnimation(0,53);
									return;
								}
								if (currentRoomNumber == 38 && roomExit.roomNumber == 7 && bVar) {
									_player->loadAnimations("darkin.nsp");
									setupOtherNspAnimation(0,41);
									return;
								}
								if (_objectVar[52] == 1 &&
									((currentRoomNumber == 12 && roomExit.roomNumber == 17) || (currentRoomNumber == 11 && (roomExit.roomNumber == 15 || roomExit.roomNumber == 16)))
									) {
									_console->printTosText(437);
									bVar = false;
									// TODO the original sets roomExit to number 8 here.
								} else if (currentRoomNumber == 11 && roomExit.roomNumber == 15 && bVar) {
									_player->loadAnimations("rm11strs.nsp");
									setupOtherNspAnimation(0,55);
									return;
								}
								if (currentRoomNumber == 14 && roomExit.roomNumber == 35 && _objectVar[99] != 1) {
									bVar = false;
									_console->printTosText(937);
								}
								if (bVar && (
												(currentRoomNumber == 6 && roomExit.roomNumber == 10) ||
												(currentRoomNumber == 11 && roomExit.roomNumber == 16) ||
												(currentRoomNumber == 12 && roomExit.roomNumber == 17)
												)
									) {
									_player->loadAnimations("opendoor.nsp");
									setupOtherNspAnimation(0,14);
									// FUN_1208_0dac_sound_related(10,CONCAT11(extraout_AH,5));
									return;
								}
								if (currentRoomNumber == 6 && roomExit.roomNumber == 5 && bVar) {
									_player->_position.x = 346;
									_player->_position.y = 176;
									_player->loadAnimations("stairs.nsp");
									setupOtherNspAnimation(1,6);
									return;
								}
								if (currentRoomNumber == 33 && roomExit.roomNumber == 34 && bVar) {
									_player->loadAnimations("opendoor.nsp");
									setupOtherNspAnimation(0,25);
									// FUN_1208_0dac_sound_related(24,CONCAT11(extraout_AH,5));
									return;
								}
								if (currentRoomNumber == 5 && roomExit.roomNumber == 6 && bVar) {
									if (_player->_position.x == 466 && _player->_position.y == 195) {
										_player->loadAnimations("stairs.nsp");
										_player->_position.x -= 35;
										setupOtherNspAnimation(3,7);
										return;
									}
									bVar = false;
								}
								if (currentRoomNumber == 40 && roomExit.roomNumber == 41 && _objectVar[59] != 2) {
									_console->printTosText(775);
									bVar = false;
								}
								if (currentRoomNumber == 44 && roomExit.roomNumber == 43 && _objectVar[78] != 2) {
									_console->printTosText(775);
									bVar = false;
								}

								if (bVar) {
									if (currentRoomNumber != 0x22 && (currentRoomNumber < 0x13 || currentRoomNumber > 0x17)) {
										_player->_playerIsChangingDirection = false;
										_player->_heroMoving = false;
										_player->updateSprite();
										updateDisplay();
										_previousRoomNumber = currentRoomNumber;
//										currentRoomNumber = *(byte *)((int)&roomExitTbl[0].roomNumber + iVar7 * 0xb);
//										if (((isAutoWalkingToBed != False) && (DAT_2c85_8254 == 2)) && (currentRoomNumber == 10)) {
//											FUN_171d_0c6e();
//										}
										changeToRoom(roomExit.roomNumber);
//										if ((isAutoWalkingToBed != False) &&
//											((currentRoomNumber != 5 ||
//											  (lVar9 = CONCAT22(playerSpriteX_long._2_2_,(uint)playerSpriteX_long),
//											   lVar10 = CONCAT22(playerSpriteY_long._2_2_,(uint)playerSpriteY_long), previousRoomNumber != 0x3d ))))
//										{
//											_player->updateBedAutoWalkSequence();
//											uVar4 = playerSpriteX_maybe;
//											uVar3 = playerSpriteY_maybe;
//											lVar9 = CONCAT22(playerSpriteX_long._2_2_,(uint)playerSpriteX_long);
//											lVar10 = CONCAT22(playerSpriteY_long._2_2_,(uint)playerSpriteY_long);
//										}
										return;
									}
								}
							}
						}
					}
				}
				if (_player->isAtWalkTarget() && _player->isPlayerWalking_maybe) {
					if (_player->_sequenceRotation != -1) {
						_player->changeDirection(_player->_direction, _player->_sequenceRotation);
						_player->updateSprite();
						_player->_sequenceRotation = -1;
						return;
					}
					_player->_heroMoving = false;
					_player->isPlayerWalking_maybe = false;
					// TODO complete at final destination logic. 2022:879d
					Common::Point currentCursorPos = _cursor.getPosition();
					if (_player->_walkToSequence) {
						_cursor.setPosition(_player->_walkToSequencePoint);
						_player->_walkToSequence = false;
					}
					int objIdx = _room->getObjectUnderCursor();
					_cursor.setPosition(currentCursorPos);
					if (objIdx != -1) {
					int objType = _room->_roomObj[objIdx].type;
					int objNum = _room->_roomObj[objIdx].objNum;
						if (((objType != 4 && objType != 0 && objType < 10) || objNum > 5 || _room->_collisionType != 0)) {
							if (_room->_collisionType == 0) {
								handleObjCollision(objNum);
							} else {
								handleObjCollision(objIdx); // TODO is this correct?
							}
						}
					}
					if (objIdx == -1) {
						_console->printTosText(938);
						if (_actionMode > 3) {
							_actionMode = PointerAction;
							_cursor.setCursorType((CursorType)_actionMode);
						}
					} else {
						if (_actionMode > 3) {
							_actionMode = PointerAction;
							_cursor.setCursorType((CursorType)_actionMode);
						}
					}
				}
				if (!isPlayingAnimation_maybe) {
					// walk to destination point
					int walkXDelta = 0;
					int walkYDelta = 0;
					int local_a = scaledWalkSpeed_maybe * 16;
					if ( _player->_direction == 0 || _player->_direction == 2) {
						local_a = local_a / 3;
					}
					if (local_a < 1000) {
						local_a = 1000;
					}
					if (yDistToTarget < xDistToTarget) {
						walkXDelta = local_a;
						if (yDistToTarget == 0) {
							walkYDelta = 0;
						} else {
							walkYDelta = (local_a * yDistToTarget) / xDistToTarget;
						}
					} else {
						walkYDelta = local_a;
						if (xDistToTarget == 0) {
							walkXDelta = 0;
						} else {
							walkXDelta = (local_a * xDistToTarget) / yDistToTarget;
						}
					}
					if (walkXDelta != 0) {
						walkXDelta = walkXDelta / 1000;
					}
					if (walkYDelta != 0) {
						walkYDelta = walkYDelta / 1000;
					}
					if (!_room->canWalkAtLocation(_player->_walkTarget.x, _player->_walkTarget.y) || _player->isAtWalkTarget()) {
						bool bVar1 = false;
						bool bVar2 = false;
						if ((walkYDelta == 0 && _player->_position.y != _player->_walkTarget.y) ||
							_player->_position.y == _player->_walkTarget.y) {
							bVar2 = true;
						}
						if ((walkXDelta == 0 && _player->_position.x != _player->_walkTarget.x) ||
							_player->_position.x == _player->_walkTarget.x) {
							bVar1 = true;
						}
						int local_6 = 0;
						int local_4 = 0;
						if (_player->_walkTarget.x < _player->_position.x) {
							if (_player->_position.x - _player->_walkTarget.x <= walkXDelta) {
								local_6 = _player->_position.x - _player->_walkTarget.x;
							} else {
								local_6 = walkXDelta;
							}
							while (!bVar1 && local_6 > 0) {
								if (!_room->canWalkAtLocation(_player->_position.x - local_6 - 1, _player->_position.y)) {
									local_6--;
								} else {
									_player->_position.x -= local_6;
									_player->_heroMoving = true;
									bVar1 = true;
								}
							}
						} else if (_player->_position.x < _player->_walkTarget.x) {
							if (_player->_walkTarget.x - _player->_position.x <= walkXDelta) {
								local_6 = _player->_walkTarget.x - _player->_position.x;
							} else {
								local_6 = walkXDelta;
							}
							while (!bVar1 && local_6 > 0) {
								if (!_room->canWalkAtLocation(_player->_position.x + local_6 + 1, _player->_position.y)) {
									local_6--;
								} else {
									_player->_position.x += local_6;
									_player->_heroMoving = true;
									bVar1 = true;
								}
							}
						}
						if (_player->_walkTarget.y < _player->_position.y) {
							if (walkYDelta < _player->_position.y - _player->_walkTarget.y) {
								local_4 = walkYDelta;
							} else {
								local_4 = _player->_position.y - _player->_walkTarget.y;
							}
							while (!bVar2 && local_4 > 0) {
								int local_34 = (_player->_position.y - local_4) - 1;
								if (local_34 > 0xee) {
									local_34 = 0xee;
								}
								if (!_room->canWalkAtLocation(_player->_position.x, (local_34 - local_4) - 2)) {
									local_4--;
								} else {
									_player->_position.y -= local_4;
									_player->_heroMoving = true;
									bVar2 = true;
								}
							}
						} else if (_player->_position.y < _player->_walkTarget.y) {
							if (walkYDelta < _player->_walkTarget.y - _player->_position.y) {
								local_4 = walkYDelta;
							} else {
								local_4 = _player->_walkTarget.y - _player->_position.y;
							}
							while (!bVar2 && local_4 > 0) {
								if (!_room->canWalkAtLocation(_player->_position.x, _player->_position.y + local_4 + 2)) {
									local_4--;
								} else {
									_player->_position.y += local_4;
									_player->_heroMoving = true;
									bVar2 = true;
								}
							}
						}

						if (!bVar1 || !bVar2) {
							_player->_walkTarget = _player->_position;
						}
						_doorEnabled = false;
						if (_player->_isAutoWalkingToBed && _player->isAtWalkTarget()) {
							_player->updateBedAutoWalkSequence();
						}
					} else {
						if (_player->_walkTarget.x < _player->_position.x) {
							if (_player->_position.x - _player->_walkTarget.x < walkXDelta) {
								walkXDelta = _player->_position.x - _player->_walkTarget.x;
							}
							_player->_positionLong.x -= walkXDelta;
						} else if (_player->_position.x < _player->_walkTarget.x) {
							if (_player->_walkTarget.x - _player->_position.x < walkXDelta) {
								walkXDelta = _player->_walkTarget.x - _player->_position.x;
							}
							_player->_positionLong.x += walkXDelta;
						}
						if (_player->_walkTarget.y < _player->_position.y) {
							if (_player->_position.y - _player->_walkTarget.y < walkYDelta) {
								walkYDelta = _player->_position.y - _player->_walkTarget.y;
							}
							_player->_positionLong.y -= walkYDelta;
						} else if (_player->_position.y < _player->_walkTarget.y) {
							if (_player->_walkTarget.y - _player->_position.y < walkYDelta) {
								walkYDelta = _player->_walkTarget.y - _player->_position.y;
							}
							_player->_positionLong.y += walkYDelta;
						}
						if (!_room->canWalkAtLocation(_player->_positionLong.x, _player->_positionLong.y)) {
							_player->_walkTarget = _player->_position;
							_player->_positionLong = _player->_position;
						} else {
							_player->_position = _player->_positionLong;
						}
					}
				}
//				else if (_isLeftMouseClicked) {
//					// TODO do actions here.
//					handlePointerAction();
//				}
			}
		} else {
			// turn player around.
			_player->playerSpriteWalkIndex_maybe = (int16)((_player->playerSpriteWalkIndex_maybe + _player->playerWalkFrameDeltaOffset) & 7);
			if (_player->playerSpriteWalkIndex_maybe == _player->playerNewFacingDirection_maybe) {
				_player->_playerIsChangingDirection = false;
				_player->_direction = _player->playerNewFacingDirection_maybe / 2;
			}
			_player->updateSprite();
		}
	} else {
		updateAnimation();
		if (!isPlayingAnimation_maybe && _player->_isAutoWalkingToBed) {
			_player->updateBedAutoWalkSequence();
		}
	}
}

void DarkseedEngine::handlePointerAction() {
	if (_cursor.getCursorType() == ConnectorEntrance) {
		int newRoomNumber = _room->getExitRoomNumberAtPoint(_cursor.getX(), _cursor.getY());
		if (newRoomNumber >= 0) {
			changeToRoom(newRoomNumber);
		}
	}
}

void DarkseedEngine::loadRoom(int roomNumber) {
//	*(undefined *)&_erasemenu = 1; TODO do we need these?
//	*(undefined2 *)&_gShipOff = 0;
	_sound->waitForSpeech();
	if (roomNumber == 33 && _objectVar[62] == 101) {
		_objectVar[62] = 0;
	}
	_printedcomeheredawson = false;
	_objectVar.setObjectRunningCode(53, 0);
	_objectVar[56] = 0;
	_objectVar.setObjectRunningCode(72, 0);
	for (int i = 31; i < 34; i++) {
		if (_objectVar.getMoveObjectRoom(i) == 99) {
			_objectVar.setMoveObjectRoom(i, 10);
		}
	}
	if (_objectVar[28] == 1) {
		_objectVar[28] = 2;
	}
	if (_objectVar[29] == 1) {
		_objectVar[29] = 2;
	}
	if (_objectVar[141] == 10) {
		_objectVar[141] = 11;
	}

	delete _room;
	_room = new Room(roomNumber); // getroomstuff

	if (roomNumber == 46 && _previousRoomNumber == 60 && _objectVar[57] == 1) {
		return;
	}
	if (roomNumber == 15 && _objectVar.getMoveObjectRoom(28) != 255) {
		setupOtherNspAnimation(1, 39);
	}
}

void DarkseedEngine::changeToRoom(int newRoomNumber) {
	_objectVar[99] = 0;
	_objectVar[66] = 0;
	_objectVar[67] = 0;
	_objectVar[68] = 0;

	if (_objectVar[53] == 3) {
		_objectVar[53] = 0;
	}

	loadRoom(newRoomNumber);

	_room->darkenSky();
	if (_room->_roomNumber == 54) {
		_objectVar[21] = 0;
	}
	if ((_room->_roomNumber == 9 && _previousRoomNumber == 3) || (_room->_roomNumber == 3 && _previousRoomNumber == 9)) {
		_objectVar[59] = 1;
	}
	if ((_room->_roomNumber == 0 && _previousRoomNumber == 2) || (_room->_roomNumber == 2 && _previousRoomNumber == 0)) {
		_objectVar[78] = 1;
	}
	if (newRoomNumber == 5 && _previousRoomNumber == 6) {
		_player->loadAnimations("stairs.nsp");
		_player->_position.x = 0x174;
		_player->_position.y = 0x100;
		setupOtherNspAnimation(2,6);
	} else if (newRoomNumber == 6 && _previousRoomNumber == 5) {
		_player->loadAnimations("stairs.nsp");
		_player->_position.x = 0x19f;
		_player->_position.y = 0x8c;
		setupOtherNspAnimation(0,7);
	} else if (newRoomNumber == 32 && _previousRoomNumber == 13) {
		_player->loadAnimations("slide.nsp");
		setupOtherNspAnimation(0,16);
	} else if (newRoomNumber == 10 && _previousRoomNumber == 6) {
		_player->loadAnimations("rm10strs.nsp");
		setupOtherNspAnimation(1,54);
	} else if (newRoomNumber == 11 && _previousRoomNumber == 15) {
		_player->loadAnimations("rm11strs.nsp");
		setupOtherNspAnimation(1,56);
	} else if (newRoomNumber == 46 && _previousRoomNumber == 60 && _objectVar[57] == 1) {
		_console->printTosText(62);
		// TODO wait logic here.
		g_engine->playCutscene("E");
		return;
	} else if (newRoomNumber == 7 && _previousRoomNumber == 38) {
		_player->loadAnimations("mirror.nsp");
		setupOtherNspAnimation(1,28);
		if (_objectVar[47] == 0 && _currentDay == 3 && _objectVar[51] == 1) {
			_objectVar.setObjectRunningCode(47, 2);
		}
	} else if ((newRoomNumber == 38) && (_previousRoomNumber == 7)) {
		_player->loadAnimations("darkout.nsp");
		setupOtherNspAnimation(1, 42);
	} else if ((_previousRoomNumber == 2) && (newRoomNumber == 3)) {
		_player->loadAnimations("rtladder.nsp");
		setupOtherNspAnimation(1, 13);
	} else if ((_previousRoomNumber == 61) && (newRoomNumber == 5)) {
		_player->loadAnimations("ltladder.nsp");
		setupOtherNspAnimation(1, 11);
	} else if ((_previousRoomNumber == 41 && newRoomNumber == 44) || (_previousRoomNumber == 44 && newRoomNumber == 41)) {
		_player->loadAnimations("beamer.nsp");
		setupOtherNspAnimation(1, 58);
		for (auto &exit : _room->room1) {
			if (exit.roomNumber == _previousRoomNumber) {
				_player->_position.x = exit.x + exit.width / 2;
				_player->_position.y = exit.y;
				_player->_direction = exit.direction;
				_player->updatePlayerPositionAfterRoomChange();
				_player->_walkTarget = _player->_position;
			}
		}
	} else if (newRoomNumber != 0x22 && (newRoomNumber < 0x13 || newRoomNumber > 0x17)) {
		for (int i = 0; i < _room->room1.size(); i++) {
			const RoomExit &roomExit = _room->room1[i];
			if (roomExit.roomNumber == _previousRoomNumber) {
				_player->_position.x = roomExit.x + roomExit.width / 2;
				_player->_position.y = roomExit.y;
				_player->_direction = roomExit.direction ^ 2;
				_player->updatePlayerPositionAfterRoomChange();
				_player->_walkTarget = _player->_position;
			}
		}
		if (_previousRoomNumber == 10 && newRoomNumber == 6 && _objectVar[47] == 0 && _currentDay == 1) {
			_objectVar.setObjectRunningCode(47, 2);
		}
	}

	if (!g_engine->isPlayingAnimation_maybe) {
		g_engine->_player->updateSprite();
	}
	_room->initRoom();
	updateDisplay();
	// TODO load room song.
	_room->printRoomDescriptionText();
}

void DarkseedEngine::debugTeleportToRoom(int newRoomNumber, int entranceNumber) {
	delete _room;
	_room = new Room(newRoomNumber);

	if (entranceNumber < _room->room1.size()) {
		const RoomExit &roomExit = _room->room1[entranceNumber];
		_player->_position.x = roomExit.x + roomExit.width / 2;
		_player->_position.y = roomExit.y;
		_player->_direction = roomExit.direction ^ 2;
		_player->updatePlayerPositionAfterRoomChange();
		_player->_walkTarget = _player->_position;
	}
	g_engine->updateDisplay();
}

void DarkseedEngine::updateDisplay() { // AKA ServiceRoom
	int currentRoomNumber = _room->_roomNumber;
	_sprites.clearSpriteDrawList();
	_room->runRoomObjects();
	runObjects();
	if (isPlayingAnimation_maybe == 0 ||
		(otherNspAnimationType_maybe != 6 && otherNspAnimationType_maybe != 7) || currentRoomNumber != 5) {
		_frameBottom = 240;
	}
	else {
		_frameBottom = 208;
	}

	if (currentRoomNumber != 0x22 && currentRoomNumber != 0x13 &&
		  currentRoomNumber != 0x14 && currentRoomNumber != 0x15 && currentRoomNumber != 0x16 &&
		 _objectVar[141] != 9 && (currentRoomNumber != 0x35 || _objectVar[45] != 2)) {
		if (((otherNspAnimationType_maybe == 60) || (otherNspAnimationType_maybe == 61)) &&
			isPlayingAnimation_maybe) {
			const Sprite &sprite = _room->_locationSprites.getSpriteAt(_player->_frameIdx);
			_sprites.addSpriteToDrawList(431, 66, &sprite, 255, sprite.width, sprite.height, false);
		}
		if (_objectVar[OBJ_21_HEADBAND] == 0 || (headAcheMessageCounter & 1) != 0) {
			if (!isPlayingAnimation_maybe || otherNspAnimationType_maybe == 19
				|| otherNspAnimationType_maybe == 23) {
				const Sprite &playerSprite = _player->getSprite(_player->_frameIdx);
				_room->calculateScaledSpriteDimensions(playerSprite.width, playerSprite.height, _player->_position.y);
				if ((currentRoomNumber != 53) || (_objectVar[79] != 2)) {
					_sprites.addSpriteToDrawList(
						_player->_position.x - (scaledSpriteWidth / 2),
						_player->_position.y - scaledSpriteHeight,
						&playerSprite,
						240 - _player->_position.y,
						scaledSpriteWidth,
						scaledSpriteHeight, player_sprite_related_2c85_82f3);
				}
			} else {
				if (otherNspAnimationType_maybe == 37) {
					error("anim: 37");
//					uVar1 = (uint)BYTE_ARRAY_2c85_41e7[1];
//					uVar7 = *(undefined2 *)((int)CPlayerSpriteWidthTbl + uVar1 * 2);
//					uVar4 = *(undefined2 *)((int)CPlayerSpriteHeightTbl + uVar1 * 2);
//					calculateScaledPlayerSpriteDimensions(uVar7,uVar4,playerSpriteY_maybe);
//					addSpriteToDraw(playerSpriteX_maybe - (_curPlayerSpriteWidth & 0xff) / 2,
//									playerSpriteY_maybe - (_curPlayerSpriteHeight_maybe & 0xff),uVar7,uVar4,
//									*(undefined2 *)((int)CPlayerSpritePtrTbl + uVar1 * 4),
//									*(undefined2 *)((int)&CPlayerSpritePtrTbl[0].Offset + uVar1 * 4),240 - playerSpriteY_maybe ,
//									_curPlayerSpriteWidth,_curPlayerSpriteHeight_maybe,_player_sprite_related_2c85_82f3);
				}
				if (otherNspAnimationType_maybe == 0x27 || otherNspAnimationType_maybe == 0x3b ||
					 otherNspAnimationType_maybe == 0x3c || otherNspAnimationType_maybe == 0x3d) {
//					uVar1 = (uint)BYTE_ARRAY_2c85_41e7[1];
//					uVar7 = *(undefined2 *)((int)CPlayerSpriteWidthTbl + uVar1 * 2);
//					uVar4 = *(undefined2 *)((int)CPlayerSpriteHeightTbl + uVar1 * 2);
//					calculateScaledPlayerSpriteDimensions(uVar7,uVar4,playerSpriteY_maybe);
//					addSpriteToDraw(playerSpriteX_maybe - (_curPlayerSpriteWidth & 0xff) / 2,
//									playerSpriteY_maybe - (_curPlayerSpriteHeight_maybe & 0xff),uVar7,uVar4,
//									*(undefined2 *)((int)CPlayerSpritePtrTbl + uVar1 * 4),
//									*(undefined2 *)((int)&CPlayerSpritePtrTbl[0].Offset + uVar1 * 4),240 - playerSpriteY_maybe ,
//									_curPlayerSpriteWidth,_curPlayerSpriteHeight_maybe,_player_sprite_related_2c85_82f3);
//					if ((otherNspAnimationType_maybe == 0x3c) || (otherNspAnimationType_maybe == 0x3d)) {
//						sprite_y_scaling_threshold_maybe = 0xf0;
//						drawInventory();
//						updateScreen();
//						DAT_2c85_985d = cursorYPosition;
//						return;
//					}
				}
//				iVar9 = *(int *)((int)otherNspWidthTbl + _player->_frameIdx * 2);
//				iVar8 = *(int *)((int)&otherNspHeightTbl + _player->_frameIdx * 2);
				if (otherNspAnimationType_maybe == 18) {
					if (_objectVar[22] == 0) {
						sprite_y_scaling_threshold_maybe = 0xcb;
					} else if (_objectVar[22] == 1) {
						sprite_y_scaling_threshold_maybe = 0xcb;
					} else if (_objectVar[22] == 2) {
						sprite_y_scaling_threshold_maybe = 0xc4;
					} else {
						sprite_y_scaling_threshold_maybe = 0xaf;
					}
				}

				_room->calculateScaledSpriteDimensions(
					_player->_animations.getSpriteAt(_player->_frameIdx).width,
					_player->_animations.getSpriteAt(_player->_frameIdx).height,
					nsp_sprite_scaling_y_position != 0 ? nsp_sprite_scaling_y_position : _player->_position.y);

				if (otherNspAnimationType_maybe == 3) { // fall unconscious outside.
					int curScaledWidth = g_engine->scaledSpriteWidth;
					int curScaledHeight = g_engine->scaledSpriteHeight;
					_room->calculateScaledSpriteDimensions(
						_player->_animations.getSpriteAt(_player->_frameIdx+1).width,
						_player->_animations.getSpriteAt(_player->_frameIdx+1).height, _player->_position.y);
					const Sprite &animSprite = _player->_animations.getSpriteAt(_player->_frameIdx);
					_sprites.addSpriteToDrawList(_player->_position.x - (curScaledWidth + g_engine->scaledSpriteWidth) / 2, _player->_position.y - curScaledHeight, &animSprite, 240 - _player->_position.y, curScaledWidth, curScaledHeight, false);
				} else if (!_scaleSequence) {
					if (otherNspAnimationType_maybe == 17) { // open trunk
						const Sprite &animSprite = _player->_animations.getSpriteAt(_player->_frameIdx);
						_sprites.addSpriteToDrawList(_player->_position.x - animSprite.width / 2, _player->_position.y - animSprite.height, &animSprite, 240 - _player->_position.y, animSprite.width, animSprite.height, player_sprite_related_2c85_82f3);
					} else if (otherNspAnimationType_maybe == 5 || otherNspAnimationType_maybe == 1) {
						int x = 0xa6;
						int y = 0x69;
						if (_player->_frameIdx < 4) {
							x = 0x75;
							y = 0x71;
						}
						else if (_player->_frameIdx == 4) {
							x = 0x75;
							y = 0x69;
						}
						else if (_player->_frameIdx == 7) {
							x = 0xa6;
							y = 0x5b;
						}

						/* bed wake sequence is played here. */
						const Sprite &animSprite = _player->_animations.getSpriteAt(_player->_frameIdx);
						_sprites.addSpriteToDrawList(x, y, &animSprite, 240 - _player->_position.y, animSprite.width, animSprite.height, player_sprite_related_2c85_82f3);
					} else if (otherNspAnimationType_maybe == 43 || otherNspAnimationType_maybe == 44) {
						const Sprite &animSprite = _player->_animations.getSpriteAt(_player->_frameIdx);
						_sprites.addSpriteToDrawList(303, 105, &animSprite, 240 - _player->_position.y, animSprite.width, animSprite.height, player_sprite_related_2c85_82f3);
					} else if (otherNspAnimationType_maybe == 62) {
						// TODO
						error("anim display 62");
					} else if (otherNspAnimationType_maybe == 45 || otherNspAnimationType_maybe == 46) { // pull lever
						const Sprite &animSprite = _player->_animations.getSpriteAt(_player->_frameIdx);
						_sprites.addSpriteToDrawList(446, 124, &animSprite, 240 - _player->_position.y, animSprite.width, animSprite.height, player_sprite_related_2c85_82f3);
					} else if (otherNspAnimationType_maybe == 36) {
						const Sprite &animSprite = _player->_animations.getSpriteAt(_player->_frameIdx);
						_sprites.addSpriteToDrawList(339, 78, &animSprite, 240 - _player->_position.y, animSprite.width, animSprite.height, player_sprite_related_2c85_82f3);
					} else if (otherNspAnimationType_maybe == 59) {
						const Sprite &animSprite = _player->_animations.getSpriteAt(_player->_frameIdx);
						_sprites.addSpriteToDrawList(433, 91, &animSprite, 240 - _player->_position.y, animSprite.width, animSprite.height, player_sprite_related_2c85_82f3);
					} else if (otherNspAnimationType_maybe == 37) {
						const Sprite &animSprite = _player->_animations.getSpriteAt(_player->_frameIdx);
						_sprites.addSpriteToDrawList(428, 78, &animSprite, 240 - _player->_position.y, animSprite.width, animSprite.height, player_sprite_related_2c85_82f3);
					} else if (otherNspAnimationType_maybe == 10 || otherNspAnimationType_maybe == 11) {
						const Sprite &animSprite = _player->_animations.getSpriteAt(_player->_frameIdx);
						_sprites.addSpriteToDrawList(118, 62, &animSprite, 240 - _player->_position.y, animSprite.width, animSprite.height, false);
					} else if (otherNspAnimationType_maybe == 12 || otherNspAnimationType_maybe == 13) {
						const Sprite &animSprite = _player->_animations.getSpriteAt(_player->_frameIdx);
						_sprites.addSpriteToDrawList(407, 73, &animSprite, 240 - _player->_position.y, animSprite.width, animSprite.height, false);
					} else if (otherNspAnimationType_maybe == 20) {
						const Sprite &animSprite = _player->_animations.getSpriteAt(_player->_frameIdx);
						_sprites.addSpriteToDrawList((_player->_position.x - animSprite.width / 2) - 4, _player->_position.y - animSprite.height, &animSprite, 240 - _player->_position.y, animSprite.width, animSprite.height, player_sprite_related_2c85_82f3);
					} else if (otherNspAnimationType_maybe < 30 || otherNspAnimationType_maybe > 34) {
						if (otherNspAnimationType_maybe == 40) {
							error("anim 40 display"); // TODO
						} else if (otherNspAnimationType_maybe < 48 || otherNspAnimationType_maybe > 52) {
							if (otherNspAnimationType_maybe == 35) {
								const Sprite &animSprite = _player->_animations.getSpriteAt(_player->_frameIdx);
								_sprites.addSpriteToDrawList(_player->_position.x - 10, _player->_position.y - animSprite.height, &animSprite, 240 - _player->_position.y, animSprite.width, animSprite.height, player_sprite_related_2c85_82f3);
							} else if (otherNspAnimationType_maybe >= 53 && otherNspAnimationType_maybe <= 56) {
								const Sprite &animSprite = _player->_animations.getSpriteAt(_player->_frameIdx);
								_sprites.addSpriteToDrawList(_player->_position.x, _player->_position.y, &animSprite, 240 - _player->_position.y, animSprite.width, animSprite.height, player_sprite_related_2c85_82f3);
							} else if (otherNspAnimationType_maybe == 57 || otherNspAnimationType_maybe == 58) {
								const Sprite &animSprite = _player->_animations.getSpriteAt(_player->_frameIdx);
								Common::Point spritePos = {344,57};
								if (_room->_roomNumber == 41) {
									spritePos = {295,46};
								}
								_sprites.addSpriteToDrawList(spritePos.x, spritePos.y, &animSprite, 240 - _player->_position.y, animSprite.width, animSprite.height, player_sprite_related_2c85_82f3);
							} else {
								const Sprite &animSprite = _player->_animations.getSpriteAt(_player->_frameIdx);
								_sprites.addSpriteToDrawList(_player->_position.x, _player->_position.y, &animSprite, 240 - _player->_position.y, animSprite.width, animSprite.height, player_sprite_related_2c85_82f3);
							}
						} else {
							const Sprite &animSprite = _player->_animations.getSpriteAt(_player->_frameIdx);
							_sprites.addSpriteToDrawList(_player->_position.x - animSprite.width / 2, _player->_position.y - animSprite.height, &animSprite, 240 - _player->_position.y, animSprite.width, animSprite.height, player_sprite_related_2c85_82f3);
						}
					} else {
						// drinking water in bathroom
						const Sprite &animSprite = _player->_animations.getSpriteAt(_player->_frameIdx);
						_sprites.addSpriteToDrawList(448, 97, &animSprite, 240 - _player->_position.y, animSprite.width, animSprite.height, player_sprite_related_2c85_82f3);
						const Sprite &legsSprite = _player->_animations.getSpriteAt(0);
						_sprites.addSpriteToDrawList(451, 160, &legsSprite, 240 - _player->_position.y, legsSprite.width, legsSprite.height, player_sprite_related_2c85_82f3);

					}
				} else if (otherNspAnimationType_maybe == 6) {
					// stairs up
					_sprites.addSpriteToDrawList(
						_player->_position.x - scaledSpriteWidth / 2,
						_player->_position.y - scaledSpriteHeight,
						&_player->_animations.getSpriteAt(_player->_frameIdx),
						0xff,
						scaledSpriteWidth,
						scaledSpriteHeight,
						player_sprite_related_2c85_82f3);
				} else if (otherNspAnimationType_maybe == 22) {
					error("anim: 22"); // TODO
				} else if (otherNspAnimationType_maybe == 4 || otherNspAnimationType_maybe == 21) {
					error("anim: 4 || 21"); // TODO
				} else if (otherNspAnimationType_maybe == 39) {
					error("anim: 39"); // TODO
				} else if (otherNspAnimationType_maybe == 47) {
					error("anim: 47"); // TODO
				} else {
					_sprites.addSpriteToDrawList(
						_player->_position.x - scaledSpriteWidth / 2,
						_player->_position.y - scaledSpriteHeight,
						&_player->_animations.getSpriteAt(_player->_frameIdx),
						240 - _player->_position.y,
						scaledSpriteWidth,
						scaledSpriteHeight,
						player_sprite_related_2c85_82f3);
				}
				if (isPlayingAnimation_maybe && otherNspAnimationType_maybe == 14 && animIndexTbl[0] > 1) {
					if (_objectVar.getObjectRunningCode(140) == 0 || _room->_roomNumber != 6) {
						if (_room->_roomNumber == 6 && _player->_isAutoWalkingToBed) {
							const Sprite &animSprite = _player->_animations.getSpriteAt(8);
							_sprites.addSpriteToDrawList(132, 70, &animSprite, 255, animSprite.width, animSprite.height, player_sprite_related_2c85_82f3);
						} else if (_room->_roomNumber == 6) {
							const Sprite &animSprite = _player->_animations.getSpriteAt(2);
							_sprites.addSpriteToDrawList(132, 70, &animSprite, 255, animSprite.width, animSprite.height, player_sprite_related_2c85_82f3);
						} else if (_room->_roomNumber == 11) {
							const Sprite &animSprite = _player->_animations.getSpriteAt(6);
							_sprites.addSpriteToDrawList(369, 119, &animSprite, 255, animSprite.width, animSprite.height, player_sprite_related_2c85_82f3);
						} else if (_room->_roomNumber == 12) {
							const Sprite &animSprite = _player->_animations.getSpriteAt(6);
							_sprites.addSpriteToDrawList(252, 121, &animSprite, 255, animSprite.width, animSprite.height, player_sprite_related_2c85_82f3);
						}
					} else {
						const Sprite &animSprite = _player->_animations.getSpriteAt(3);
						_sprites.addSpriteToDrawList(132, 70, &animSprite, 255, animSprite.width, animSprite.height, player_sprite_related_2c85_82f3);
					}
				}
			}
		}
	}
}

void DarkseedEngine::setupOtherNspAnimation(int nspAnimIdx, int animId) {
	assert(nspAnimIdx < 20);
	_player->_playerIsChangingDirection = false;
	nsp_sprite_scaling_y_position = 0;
	_scaleSequence = false;

	animIndexTbl[nspAnimIdx] = 0;
	spriteAnimCountdownTimer[nspAnimIdx] = _player->_animations.getAnimAt(nspAnimIdx).frameDuration[0];
	isPlayingAnimation_maybe = true;
	otherNspAnimationType_maybe = animId;
	_player->_frameIdx = _player->_animations.getAnimAt(nspAnimIdx).frameNo[0];

	player_sprite_related_2c85_82f3 = 0;
	isAnimFinished_maybe = false;

	if (otherNspAnimationType_maybe < 2) {
		return;
	}

	// TODO big switch here to init the different animation types.
	switch (otherNspAnimationType_maybe) {
	case 2:
		_player->_position.x = 188;
		_player->_position.y = 94;
		break;
	case 3:
		// TODO
//		if ((_SoundDevice != '\x01') && ((char)_currentDay == '\x03')) {
//			LoadModeSong(7);
//			PlaySound(0,6,-1);
//		}
		_scaleSequence = 1;
		break;
	case 4:
	case 14:
	case 15:
	case 18:
	case 21:
	case 22:
	case 24:
	case 25:
	case 65:
		_scaleSequence = 1;
		break;
	case 6 : // stairs
		nsp_sprite_scaling_y_position = 0xbe;
		_scaleSequence = true;
		break;
	case 7 : // stairs down
		nsp_sprite_scaling_y_position = 0xbe;
		_scaleSequence = true;
		break;
	case 8 :
		_player->_position.x = 249;
		_player->_position.y = 92;
		break;
	case 16 :
		_player->_position.x = 324;
		_player->_position.y = 50;
		break;
	case 20:
		spriteAnimCountdownTimer[3] = 3;
		break;
	case 26 :
		_player->_position.x = 324;
		_player->_position.y = 135;
		break;
	case 27 :
		_player->_position.x = 397;
		_player->_position.y = 84;
		break;
	case 28 :
		_player->_position.x = 397;
		_player->_position.y = 77;
		break;
	case 41 :
	case 42 :
		_player->_position.x = 432;
		_player->_position.y = 78;
		break;
	case 44:
	case 46:
		playSound(30,5,-1);
		break;
	case 53 :
	case 54 :
		_player->_position.x = 308;
		_player->_position.y = 160;
		break;
	case 55 :
		_player->_position.x = 150;
		_player->_position.y = 104;
		break;
	case 56 :
		_player->_position.x = 142;
		_player->_position.y = 104;
		break;
	case 57:
	case 58:
		playSound(48,5,-1);
		break;
	case 63:
		_player->_position.x = 249;
		_player->_position.y = 92;
		_phoneStatus = 1;
		break;
	default:
		break;
	}
}

void DarkseedEngine::updateAnimation() {
	int currentRoomNumber = _room->_roomNumber;
	switch (otherNspAnimationType_maybe) {
	case 0: break;
	case 1 : // sleep wake anim
		advanceAnimationFrame(0);
		if (!isAnimFinished_maybe) {
			_player->_frameIdx = _player->_animations.getAnimAt(0).frameNo[_player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]]];
		}
		else {
			_player->_position.x = 0xdf;
			_player->_position.y = 0xbe;
			_player->_walkTarget.x = 0xdf;
			_player->_walkTarget.y = 0xbe;
			_player->updateSprite();
		}
		break;
	case 2:
		advanceAnimationFrame(0);
		if (!isAnimFinished_maybe) {
			_player->_frameIdx = _player->_animations.getAnimAt(0).frameNo[_player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]]];
		} else {
			_player->_position.x = 218;
			_player->_position.y = 198;
			_player->_direction = 2;
			_player->updateSprite();
			_player->_walkTarget.x = 218;
			_player->_walkTarget.y = 198;
			_objectVar[52] = 0;
//			StopVOC(); TODO
		}
		break;
	case 3:
		advanceAnimationFrame(0);
		if (!isAnimFinished_maybe) {
			_player->_frameIdx = _player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]];
		} else {
			if (_room->isGiger()) {
				stuffPlayer();
			} else {
				if (_room->isOutside() && _currentTimeInSeconds > 61200) {
					_room->restorePalette();
				}
				gotonextmorning();
				playDayChangeCutscene();
			}
		}
		break;
	case 5: // goto sleep animation
		_player->_position.x = 135;
		_player->_position.y = 91;
		advanceAnimationFrame(1);
		if (!isAnimFinished_maybe) {
			_player->_frameIdx = _player->_animations.getAnimAt(1).frameNo[_player->_animations.getAnimAt(1).frameNo[animIndexTbl[1]]];
		}
		else {
			gotonextmorning(); // TODO there might be some extra logic required from original function
			_player->_position.x = 242;
			_player->_position.y = 187;
			playDayChangeCutscene();
		}
		_player->_position.x = 242;
		_player->_position.y = 187;
		break;
	case 6: // stairs up
		if (currentRoomNumber == 6) {
			advanceAnimationFrame(1);
		} else {
			advanceAnimationFrame(2);
		}
		if (animFrameChanged && ((currentRoomNumber == 6 && animIndexTbl[1] == 1) || (currentRoomNumber == 5 && animIndexTbl[2] == 1))) {
//			FUN_1208_0dac_sound_related(0xd,CONCAT11(uVar4,5));
		}
		if (!isAnimFinished_maybe) {
			if (currentRoomNumber == 6) {
				_player->_frameIdx = _player->_animations.getAnimAt(1).frameNo[_player->_animations.getAnimAt(1).frameNo[animIndexTbl[1]]];
			} else {
				_player->_frameIdx = _player->_animations.getAnimAt(2).frameNo[_player->_animations.getAnimAt(2).frameNo[animIndexTbl[2]]];
			}
		} else {
			if (currentRoomNumber == 6) {
				_previousRoomNumber = 6;
				changeToRoom(5);
			}
//			if (isAutoWalkingToBed != False) {
//				_player->updateBedAutoWalkSequence();
//			}
		}
		break;
	case 7: // stairs down
		if (currentRoomNumber == 5) {
			advanceAnimationFrame(3);
			if (animFrameChanged && animIndexTbl[3] == 1) {
				// FUN_1208_0dac_sound_related(0xd,CONCAT11(extraout_AH_05,5));
			}
			if (!isAnimFinished_maybe) {
				_player->_frameIdx = _player->_animations.getAnimAt(3).frameNo[_player->_animations.getAnimAt(3).frameNo[animIndexTbl[3]]];
			} else {
				_previousRoomNumber = 5;
				changeToRoom(6);
			}
		} else {
			advanceAnimationFrame(0);
			if (animFrameChanged && animIndexTbl[0] == 1) {
				// FUN_1208_0dac_sound_related(0xd,CONCAT11(extraout_AH_05,5));
			}
			if (!isAnimFinished_maybe) {
				_player->_frameIdx = _player->_animations.getAnimAt(0).frameNo[_player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]]];
			}
		}
		break;
	case 8: //phone call
	case 63:
		if (otherNspAnimationType_maybe == 8) {
			advanceAnimationFrame(0);
		} else {
			advanceAnimationFrame(2);
			if (_phoneStatus == 1) {
				_phoneStatus = 2;
			}
		}
		if (!isAnimFinished_maybe) {
			if (otherNspAnimationType_maybe == 8) {
				_player->_frameIdx = _player->_animations.getAnimAt(0).frameNo[_player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]]];
			} else {
				_player->_frameIdx = _player->_animations.getAnimAt(2).frameNo[_player->_animations.getAnimAt(2).frameNo[animIndexTbl[2]]];
			}
		} else {
			if (otherNspAnimationType_maybe == 63) {
				_console->printTosText(44);
				setupOtherNspAnimation(1, 64);
			} else {
				_objectVar.setObjectRunningCode(47, 0);
				if (_currentDay == 1) {
					_console->printTosText(904);
					_objectVar[47] = 1;
					_objectVar[46] = 1;
				}
				else {
					_objectVar[51] = 2;
					_objectVar[47] = 1;
					_console->printTosText(922);
				}
				_sound->waitForSpeech();
				setupOtherNspAnimation(1, 9);
			}
		}
		break;
	case 9: // hang up phone
	case 64:
		advanceAnimationFrame(1);
		if (!isAnimFinished_maybe) {
			_player->_frameIdx = _player->_animations.getAnimAt(1).frameNo[_player->_animations.getAnimAt(1).frameNo[animIndexTbl[1]]];
		} else {
			_player->_position.x = 300;
			_player->_position.y = 183;
			_player->_walkTarget.x = 300;
			_player->_walkTarget.y = 183;
			_player->_direction = 3;
			_player->updateSprite();
		}
		break;
	case 11:
		advanceAnimationFrame(1);
		if (!isAnimFinished_maybe) {
			_player->_frameIdx = _player->_animations.getAnimAt(1).frameNo[animIndexTbl[1]];
		} else {
			_player->_position.x = 175;
			_player->_position.y = 200;
			_player->_walkTarget.x = 175;
			_player->_walkTarget.y = 200;
			_player->updateSprite();
			if (_player->_isAutoWalkingToBed) {
				_player->setplayertowardsbedroom();
			}
		}
		break;
	case 12:
		advanceAnimationFrame(0);
		if (!isAnimFinished_maybe) {
			_player->_frameIdx = _player->_animations.getAnimAt(0).frameNo[_player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]]];
		} else {
			_player->_position.x = 360;
			_player->_position.y = 238;
			_player->_walkTarget.x = 360;
			_player->_walkTarget.y = 238;
			_player->_direction = 3;
			changeToRoom(2);
		}
		break;
	case 13:
		advanceAnimationFrame(1);
		if (!isAnimFinished_maybe) {
			_player->_frameIdx = _player->_animations.getAnimAt(1).frameNo[animIndexTbl[1]];
		} else {
			_player->_position.x = 450;
			_player->_position.y = 217;
			_player->_walkTarget.x = 450;
			_player->_walkTarget.y = 217;
			_player->_direction = 3;
			_player->updateSprite();
		}
		break;
	case 16:
		advanceAnimationFrame(0);
		if (!isAnimFinished_maybe) {
			_player->_frameIdx = _player->_animations.getAnimAt(0).frameNo[_player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]]];
		} else {
			_player->_position.x = 336;
			_player->_position.y = 195;
			_player->_walkTarget.x = 336;
			_player->_walkTarget.y = 195;
			_player->_direction = 1;
			_player->updateSprite();
		}
		break;
	case 17: // open trunk with crowbar
		advanceAnimationFrame(0);
		if (!isAnimFinished_maybe) {
			_player->_frameIdx = _player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]];
		}
		if (animIndexTbl[0] == 5 && animFrameChanged) {
			playSound(31,5,-1);
			_console->printTosText(666);
			if (_objectVar[42] == 0) {
				_objectVar[42] = 1;
			} else {
				_objectVar[42] = 3;
			}
		}
		break;
	case 18: // push trunk
		advanceAnimationFrame(0);
		if (!isAnimFinished_maybe) {
			_player->_frameIdx = _player->_animations.getAnimAt(0).frameNo[_player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]]];
		}
		if (_player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]] == 3 && animFrameChanged) {
			_console->printTosText(_objectVar[22] + 662);
			_objectVar[22] = _objectVar[22] + 1;
			if (_objectVar[22] == 3) {
				_room->loadRoom61AWalkableLocations();
			}
		}
		break;
	case 20: // receive card from delbert
		advanceAnimationFrame(3);
		if (!isAnimFinished_maybe) {
			_player->_frameIdx = _player->_animations.getAnimAt(3).frameNo[animIndexTbl[3]];
		} else {
			_inventory.addItem(18);
		}
		break;
	case 10:
		advanceAnimationFrame(0);
		if (!isAnimFinished_maybe) {
			_player->_frameIdx = _player->_animations.getAnimAt(0).frameNo[_player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]]];
		} else {
			_previousRoomNumber = _room->_roomNumber;
			changeToRoom(61);
		}
		break;
	case 14:
	case 15:
	case 24:
	case 25:
		// Open doors
		advanceAnimationFrame(0);
		if (!isAnimFinished_maybe) {
			_player->_frameIdx = _player->_animations.getAnimAt(0).frameNo[_player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]]];
		} else {
			_player->updateSprite();
			if (otherNspAnimationType_maybe == 14) {
				// TODO
				if (_objectVar.getObjectRunningCode(140) == 0 || _room->_roomNumber != 6) {
					_previousRoomNumber = _room->_roomNumber;
					int newRoomNumber = _previousRoomNumber;
					if (_room->_roomNumber == 6) {
						if (_player->_isAutoWalkingToBed && _objectVar[137] == 2) {
							 wongame();
						}
						newRoomNumber = 10;
					} else if (_room->_roomNumber == 10) {
						newRoomNumber = 6;
					} else if (_room->_roomNumber == 11) {
						if (_player->_position.x < 250) {
							newRoomNumber = 15;
						} else {
							newRoomNumber = 16;
						}
					} else {
						newRoomNumber = 17;
					}
					changeToRoom(newRoomNumber);
				} else {
					_objectVar.setObjectRunningCode(140, 0);
					getPackageObj(_currentDay);
				}
			}
			if (otherNspAnimationType_maybe == 25) {
				_previousRoomNumber = 33;
				changeToRoom(34);
			}
		}
		break;
	case 26: // climb down rope.
		advanceAnimationFrame(1);
		if (!isAnimFinished_maybe) {
			_player->_frameIdx = _player->_animations.getAnimAt(1).frameNo[_player->_animations.getAnimAt(1).frameNo[animIndexTbl[1]]];
		} else {
			_previousRoomNumber = _room->_roomNumber;
			changeToRoom(32);
		}
		break;
	case 27:
		advanceAnimationFrame(0);
		if (!isAnimFinished_maybe) {
			_player->_frameIdx = _player->_animations.getAnimAt(0).frameNo[_player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]]];
		} else {
			_previousRoomNumber = _room->_roomNumber;
			changeToRoom(38);
		}
		break;
	case 28: // step out of mirror normal world
		advanceAnimationFrame(1);
		if (!isAnimFinished_maybe) {
			_player->_frameIdx = _player->_animations.getAnimAt(1).frameNo[animIndexTbl[1]];
		} else {
			_player->_position.x = 418;
			_player->_position.y = 170;
			_player->_direction = 3;
			_player->updateSprite();
			_player->_walkTarget = _player->_position;
		}
		break;
	case 30:
	case 31: {
		int animIdx = otherNspAnimationType_maybe - 30;
		advanceAnimationFrame(animIdx);
		if (isPlayingAnimation_maybe) {
			_player->_frameIdx = _player->_animations.getAnimAt(animIdx).frameNo[animIndexTbl[animIdx]];
		}
		if (isAnimFinished_maybe && otherNspAnimationType_maybe == 30) {
			setupOtherNspAnimation(1, 31);
		}
		break;
	}
	case 32:
	case 33:
	case 34: {
		_objectVar[112] = 1;
		int animIdx = otherNspAnimationType_maybe - 30;
		advanceAnimationFrame(animIdx);
		// TODO play sfx.
		if (isPlayingAnimation_maybe) {
			_player->_frameIdx = _player->_animations.getAnimAt(animIdx).frameNo[animIndexTbl[animIdx]];
		}
		if (isAnimFinished_maybe && (otherNspAnimationType_maybe == 32 || otherNspAnimationType_maybe == 33)) {
			setupOtherNspAnimation(otherNspAnimationType_maybe - 29, otherNspAnimationType_maybe + 1);
		}
		break;
	}
	case 35: // pay shopkeeper
		advanceAnimationFrame(6);
		if (!isAnimFinished_maybe) {
			_player->_frameIdx = _player->_animations.getAnimAt(6).frameNo[animIndexTbl[6]];
		} else {
			_objectVar[8]++;
			if (_objectVar[8] > 2) {
				_inventory.removeItem(8);
			}
			_objectVar[138]++;
			_console->printTosText(927);
		}
		break;
	case 41:
		advanceAnimationFrame(0);
		if (!isAnimFinished_maybe) {
			_player->_frameIdx = _player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]];
		} else {
			_previousRoomNumber = _room->_roomNumber;
			changeToRoom(7);
		}
		break;
	case 42: // step out of mirror giger world
		advanceAnimationFrame(1);
		if (!isAnimFinished_maybe) {
			_player->_frameIdx = _player->_animations.getAnimAt(1).frameNo[animIndexTbl[1]];
		} else {
			_player->_position.x = 456;
			_player->_position.y = 173;
			_player->_direction = 3;
			_player->updateSprite();
			_player->_walkTarget = _player->_position;
		}
		break;
	case 43:
	case 44:
	case 45:
	case 46: {
		int iVar4 = 0;
		if ((otherNspAnimationType_maybe == 44) || (otherNspAnimationType_maybe == 46)) {
			iVar4 = 1;
		}
		advanceAnimationFrame(iVar4);
		_player->_frameIdx = _player->_animations.getAnimAt(iVar4).frameNo[_player->_animations.getAnimAt(iVar4).frameNo[animIndexTbl[iVar4]]];
//		_HeroSpr = (uint) * (byte *)((int)&DAT_1060_7eb8 + *(int *)((int)&_ObjFrame + iVar4 * 2) + iVar4 * 202);
		if (!isAnimFinished_maybe || (otherNspAnimationType_maybe != 46 && otherNspAnimationType_maybe != 44)) {
			if (otherNspAnimationType_maybe == 45) {
				_objectVar[117] = 1;
			} else if (isAnimFinished_maybe) {
				if ((_objectVar[71] == 2) && (_objectVar[44] != 0)) {
					_console->printTosText(896);
					_objectVar[57] = 1;
				} else {
					_console->printTosText(897);
				}
			}
		} else {
//			LoadModeSong(7); TODO
			playSound(0, 6, -1);
			stuffPlayer();
		}
		break;
	}
	case 47:
		_room->runAnim47();
		break;
	case 48:
	case 49:
	case 50:
	case 51:
	case 52:
		// open/close cupbords, oven
		advanceAnimationFrame(0);
		if (!isAnimFinished_maybe) {
			_player->_frameIdx = _player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]];
		}
		else {
			if (otherNspAnimationType_maybe == 52) {
				_objectVar[108] = (_objectVar[108] == 0) ? 1 : 0;
			}
			else {
				_objectVar[56 + otherNspAnimationType_maybe] = _objectVar[56 + otherNspAnimationType_maybe] == 0 ? 1 : 0;
			}
			playSound(42,5,-1);
		}
		break;
	case 53 :
	case 54 :
	case 55 :
	case 56 :
		advanceAnimationFrame((otherNspAnimationType_maybe - 53) & 1);
		if (!isAnimFinished_maybe) {
			_player->_frameIdx = _player->_animations.getAnimAt(0).frameNo[_player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]]];
		} else {
			switch (otherNspAnimationType_maybe) {
			case 53 :
				_previousRoomNumber = _room->_roomNumber;
				changeToRoom(6);
				break;
			case 54 :
				_player->_position.x = 322;
				_player->_position.y = 220;
				_player->_walkTarget = _player->_position;
				_player->_direction = 2;
				_player->updateSprite();
				break;
			case 55 :
				_previousRoomNumber = _room->_roomNumber;
				changeToRoom(15);
				break;
			case 56 :
				_player->_position.x = 162;
				_player->_position.y = 206;
				_player->_walkTarget = _player->_position;
				_player->_direction = 2;
				_player->updateSprite();
				break;
			}
		}
		if (animFrameChanged && animIndexTbl[0] == 1) {
			// FUN_1208_0dac_sound_related(0xd,CONCAT11(extraout_AH_05,5));
			// PlaySound(1,5,-1);
		}
		break;
	case 57:
	case 58:
		advanceAnimationFrame((otherNspAnimationType_maybe - 57) & 1);
		if (!isAnimFinished_maybe) {
			_player->_frameIdx = _player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]]; //_player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]]];
		} else if (otherNspAnimationType_maybe == 57) {
			_previousRoomNumber = _room->_roomNumber;
			changeToRoom(_room->_roomNumber == 41 ? 44 : 41);
		} else {
			_player->_direction = 2;
		}
		break;
	case 65:
		advanceAnimationFrame(0);
		if (!isAnimFinished_maybe) {
			_player->_frameIdx = _player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]];
		} else {
			_player->updateSprite();
		}
		break;
	default:
		error("Unhandled animation type! %d", otherNspAnimationType_maybe);
	}
}

void DarkseedEngine::advanceAnimationFrame(int nspAminIdx) {
	if (!_scaleSequence) {
		scaledWalkSpeed_maybe = 1000;
	}
	else {
		_room->calculateScaledSpriteDimensions(10, 10, _player->_position.y);
	}
	isAnimFinished_maybe = false;
	animFrameChanged = false;
	const Obt &anim = _player->_animations.getAnimAt(nspAminIdx);
	spriteAnimCountdownTimer[nspAminIdx] = spriteAnimCountdownTimer[nspAminIdx] - 1;
	if (spriteAnimCountdownTimer[nspAminIdx] < 1) {
		animFrameChanged = true;
		animIndexTbl[nspAminIdx] = animIndexTbl[nspAminIdx] + 1;
		_player->_position.x += ((int16)anim.deltaX[animIndexTbl[nspAminIdx]] * scaledWalkSpeed_maybe) / 1000;
		_player->_position.y += ((int16)anim.deltaY[animIndexTbl[nspAminIdx]] * scaledWalkSpeed_maybe) / 1000;
		if (animIndexTbl[nspAminIdx] == anim.numFrames) {
			animIndexTbl[nspAminIdx] = 0;
			isAnimFinished_maybe = true;
			isPlayingAnimation_maybe = false;
			_player->_walkTarget.x = _player->_position.x;
			_player->_walkTarget.y = _player->_position.y;
			_player->updateSprite();
		}
		spriteAnimCountdownTimer[nspAminIdx] = anim.frameDuration[animIndexTbl[nspAminIdx]];
	}
}

Common::Path DarkseedEngine::getRoomFilePath(const Common::Path &filename) {
	if (isCdVersion()) {
		return Common::Path("room").join(filename);
	}
	return filename;
}

Common::Path DarkseedEngine::getPictureFilePath(const Common::Path &filename) {
	if (isCdVersion()) {
		return Common::Path("picture").join(filename);
	}
	return filename;
}

void DarkseedEngine::handleObjCollision(int targetObjNum) {
	if (targetObjNum == 35 && _objectVar[22] < 2 && _cursor.getY() > 40) {
		targetObjNum = 22;
	}
	if (_actionMode == LookAction || _actionMode == HandAction || targetObjNum != 115) {
		if (_cursor.getY() < 10 && _actionMode > LookAction) {
			// TODO handle inventory
			// 171d:53c3
		} else {
			switch (_actionMode) {
			case HandAction:
				useCode(targetObjNum);
				break;
			case LookAction:
				lookCode(targetObjNum);
				break;
			case 5:
				useCrowBar(targetObjNum);
				break;
			case 6:
			case 12:
			case 29:
				useCodeJournal(_actionMode, targetObjNum);
				break;
			case 7:
			case 36:
			case 37:
			case 38:
			case 39:
			case 40:
				useCodeShopItems(_actionMode, targetObjNum);
				break;
			case 8:
				useCodeMoney(targetObjNum);
				break;
				// TODO lots of extra switch cases here for inventory usages.
			case 14:
				useCodeGloves(targetObjNum);
				break;
			// TODO lots of extra switch cases here for inventory usages.
			default:
				break;
			}
		}
	} else {
		// TODO
		// 171d:51dc
	}
}

void DarkseedEngine::useCode(int objNum) {
	debug("useCode: objNum = %d", objNum);

	if (objNum == 141) {
		_console->addTextLine("You touch Delbert...");
		return;
	}
	if (objNum == 189) {
		_console->addTextLine("You touch the cold iron bars.");
		return;
	}
	if (objNum == 42) {
		switch(_objectVar[42]) {
		case 0:
			_console->printTosText(653);
			break;
		case 1:
			_console->printTosText(655);
			_objectVar[42] = 0;
			break;
		case 2:
			_console->printTosText(660);
			_inventory.addItem(6);
			_objectVar[42] = 3;
			break;
		case 3:
			_console->printTosText(655);
			_objectVar[42] = 4;
			break;
		case 4:
			_console->printTosText(653);
		}
		return;
	}
	if (objNum == 9) {
		_room->_collisionType = 0;
		_room->removeObjectFromRoom(9);
		_objectVar.setMoveObjectRoom(9, 100);
		showFullscreenPic(g_engine->isCdVersion() ? "paper_c.pic" : "paper-c.pic");
		return;
	}
	if (objNum == 35) {
		_objectVar[35] = 3600;
	}
	if (objNum == 47) {
		_player->loadAnimations("phone.nsp");
		if (_objectVar.getObjectRunningCode(47) == 0) {
			setupOtherNspAnimation(2,63);
		}
		else {
			setupOtherNspAnimation(0,8);
		}
		_player->_direction = 3;
	}
	if (objNum == 194) {
		if (_objectVar[53] == 2) {
			_console->printTosText(948);
			for (int i = 0; i < Objects::MAX_MOVED_OBJECTS; i++) {
				if (_objectVar.getMoveObjectRoom(i) == 252) {
					_objectVar.setMoveObjectRoom(i, 254);
					_inventory.addItem(i);
				}
			}
			_objectVar[53] = 3;
		} else {
			_console->printTosText(566);
		}
		return;
	}
	if (objNum == 113) {
		bool foundMatch = false;
		for (int i = 0; i < Objects::MAX_MOVED_OBJECTS; i++) {
			if (_objectVar.getMoveObjectRoom(i) == 250) {
				_objectVar.setMoveObjectRoom(i, 254);
				_inventory.addItem(i);
				foundMatch = true;
			}
		}
		if (foundMatch) {
			_objectVar[113] = 0;
			_console->printTosText(948);
		} else {
			_console->printTosText(751);
		}
	}
	if (objNum == 123) {
		if (_currentTimeInSeconds < 61200) {
			_console->printTosText(119);
		} else {
			_player->loadAnimations("bedsleep.nsp");
			setupOtherNspAnimation(1, 5);
		}
		return;
	}
	if (objNum == 71 && _objectVar[71] == 2) { // car ignition
		_objectVar[71] = 0;
		_console->printTosText(949);
		_inventory.addItem(26);
		return;
	}
	if ((162 < objNum) && (objNum < 169)) {
		playSound(47,5,-1);
	}
	if (objNum == 175) {
		playSound(39,5,-1);
//		while (iVar2 = VOCPlaying(), iVar2 != 0) {
//			VOCPoll();
//		}
		_console->printTosText(719);
		return;
	}
	if (objNum == 187) {
		if (_objectVar[187] == 0) {
			_console->printTosText(856);
			setupOtherNspAnimation(0, 60);
		} else {
			setupOtherNspAnimation(1, 61);
		}
		return;
	}
	if (objNum == 137 || objNum == 30) {
		if (_objectVar[30] == 0) {
			_objectVar[30] = 1;
			_room->_collisionType = 0;
			_room->removeObjectFromRoom(30);
			_objectVar.setMoveObjectRoom(30, 100);
			showFullscreenPic(g_engine->isCdVersion() ? "note_c.pic" : "note-c.pic");
		} else {
			_console->printTosText(296);
		}
		return;
	}
	if (_room->_roomNumber == 21 && objNum == 12) {
		if (_objectVar[12] == 0 && _objectVar[10] == 1) {
			_objectVar[12] = 1;
			_inventory.addItem(12);
			_console->printTosText(568);
		} else {
			_console->printTosText(566);
		}
		return;
	}
	if (objNum == 46) {
		if (_objectVar[46] == 1) {
			setupOtherNspAnimation(0,19);
		} else {
			_console->printTosText(539);
		}
		return;
	}
	if (objNum != 7 && objNum != 36 && objNum != 37 && objNum != 38 && objNum != 39 && objNum != 40) {
		int handTosIdx = _objectVar.getHandDescriptionTosIdx(objNum);
		if (handTosIdx != 0 && handTosIdx < 979) {
			_console->printTosText(handTosIdx);
		} else if (handTosIdx > 978) {
			// TODO genericresponse
		}
		if (objNum == 80) {
			_console->printTosText(553 + (_objectVar[80] & 1));
			_objectVar[80] = (_objectVar[80] & 2) + ((_objectVar[80] & 1) == 0 ? 1 : 0);
			return;
		}
		if ((objNum == 59 || objNum == 78) && _objectVar[34] == 0) {
			return;
		}
		if (objNum == 100) {
			if (_objectVar[100] == 0 || _objectVar[100] == 1 || _objectVar[100] == 3) {
				_console->printTosText(140);
			} else if (_objectVar[100] == 2) {
				_objectVar[100] = 3;
				_inventory.addItem(10);
				_objectVar[10] = 254;
				_console->printTosText(142);
			}
			return;
		}
		if (objNum == 114) {
			gancanim();
			return;
		}
		if ((objNum == 28) && (_objectVar[28] == 2)) {
			_console->addTextLine("The sergeant says 'Nice gun eh? It's a Browning'");
			return;
		}
		if (objNum > 103 && objNum < 111) {
			if (objNum < 110) {
				_player->loadAnimations("opendrwr.nsp");
				if (objNum == 108) {
					if (_objectVar[108] == 0) {
						_console->printTosText(383);
					}
					else {
						_console->printTosText(385);
					}
					setupOtherNspAnimation(0, 52);
				}
				else {
					if (_objectVar[objNum] == 0) {
						_console->printTosText(371);
					}
					else {
						_console->printTosText(373);
					}
					setupOtherNspAnimation(0,objNum - 56);
				}
			}
			else {
				if (_objectVar[objNum] == 0) {
					_console->printTosText(389);
				}
				else {
					_console->printTosText(391);
				}
				_objectVar[objNum] = _objectVar[objNum] == 0 ? 1 : 0;
			}
			return;
		}
		if (objNum == 111) { // tap
			_player->loadAnimations("aspirin.nsp");
			setupOtherNspAnimation(0,30);
			_console->printTosText(242);
			return;
		}
		if (objNum == 112) { // mirror cabinet
			_player->loadAnimations("aspirin.nsp");
			setupOtherNspAnimation(2,32);
			return;
		}
		if (objNum > 30 && objNum < 34) {
			_room->removeObjectFromRoom(objNum);
			getPackageObj(objNum - 30);
			return;
		}
		if ((objNum == 14) && (_objectVar[86] == 0)) {
			_objectVar[86] = 1;
			return;
		}
		if ((objNum == 102) && (_objectVar[23] == 1)) {
			_player->loadAnimations("slide.nsp");
			setupOtherNspAnimation(1, 26);
			return;
		}
		if (objNum == 101) {
			switch(_objectVar[101]) {
			case 0:
				_player->loadAnimations("opendoor.nsp");
				setupOtherNspAnimation(0,65);
				playSound(31,5,-1);
				_objectVar[101] = 1;
				_console->printTosText(733);
				break;
			case 1:
				_player->loadAnimations("opendoor.nsp");
				setupOtherNspAnimation(0,65);
				playSound(31,5,-1);
				_objectVar[101] = 0;
				_console->printTosText(737);
				break;
			case 2:
				_inventory.addItem(5);
				_console->printTosText(950);
				_objectVar[101] = 3;
				return;
			case 3:
				_player->loadAnimations("opendoor.nsp");
				setupOtherNspAnimation(0,65);
				playSound(31,5,-1);
				_objectVar[101] = 4;
				_console->printTosText(737);
				break;
			case 4:
				_player->loadAnimations("opendoor.nsp");
				setupOtherNspAnimation(0,65);
				playSound(31,5,-1);
				_objectVar[101] = 3;
				_console->printTosText(733);
				break;
			default:
				break;
			}
		}
		if (objNum == 81) {
			if (_objectVar[81] == 0) {
				_console->printTosText(951);
				for (int i = 0; i < Objects::MAX_MOVED_OBJECTS; i++) {
					if (_objectVar.getMoveObjectRoom(i) == 251) {
						_objectVar.setMoveObjectRoom(i, 254);
						_inventory.addItem(i);
					}
				}
				_objectVar[81] = 1;
			} else {
				_console->printTosText(952);
			}
			return;
		}
		if (objNum == 25) {
			if (1 < _objectVar[80]) {
				if (_objectVar[25] == 0) {
					_objectVar[25] = 1;
					_console->printTosText(953);
					_inventory.addItem(25);
				} else if (_objectVar[25] == 2) {
					_objectVar[26] = 1;
					_console->printTosText(954);
					_inventory.addItem(26);
					_objectVar[25] = _objectVar[25] + 1;
				} else if (_objectVar[25] == 1 || _objectVar[25] == 3 ||
						 _objectVar[25] == 101 || _objectVar[25] == 103) {
					_console->printTosText(694);
				}
			}
			return;
		}
		if (objNum < 42 && objNum != 22 && (objNum != 7 || _objectVar[7] == 1) &&
			objNum != 31 && objNum != 33 && objNum != 32 && objNum != 86) {
			_console->printTosText(955); // "You pick up the "
			_console->addToCurrentLine(Common::String::format("%s.", _objectVar.getObjectName(objNum)));
			_inventory.addItem(objNum);
			_room->_collisionType = 0;
			if (((objNum != 25) || (_objectVar[25] == 1)) || (_objectVar[25] == 101)) {
				_room->removeObjectFromRoom(objNum);
			}
			_objectVar.setMoveObjectRoom(objNum, 254);
			if (objNum == 28) {
				_objectVar[28] = 1;
			}
		} else if (objNum == 86) {
			_objectVar[86] = _objectVar[86] == 0 ? 1 : 0;
			playSound(42,5,-1);
		} else if (objNum == 22) {
			if (_objectVar[22] < 4) {
				_player->loadAnimations("push.nsp");
				setupOtherNspAnimation(0, 18);
			} else {
				_console->printTosText(665);
			}
		} else if (objNum == 165) {
			if (_objectVar[165] == 0) {
				_objectVar[165] = 1;
				_console->printTosText(639);
				_inventory.addItem(13);
			} else {
				_console->printTosText(640);
			}
		} else if (objNum == 57) {
			_player->loadAnimations("instrshp.nsp");
			if (_objectVar[71] == 2) {
				_console->printTosText(4);
				setupOtherNspAnimation(1, 44);
			} else {
				setupOtherNspAnimation(0, 43);
			}
		} else if (objNum == 117) {
			_player->loadAnimations("obslev.nsp");
			setupOtherNspAnimation(1, 46);
		} else if (objNum == 62) {
			if (((_objectVar[49] == 0) || (1 < _objectVar[62])) || (_currentDay != 1)) {
				if (((_objectVar[49] == 0) || (1 < _objectVar[62])) || (_currentDay != 2)) {
					if (((_objectVar[49] == 0) || (1 < _objectVar[62])) || (_currentDay != 3)) {
						if (_objectVar[62] < 101) {
//							LoadModeSong(9);
							playSound(0,6,-1);
							_objectVar[62] = 101;
							_console->addTextLine("You turn on the music.");
						} else if (_objectVar[62] == 101) {
//							KillAllSound();
//							TerminateRoomSong();
							_objectVar[62] = 100;
							_console->addTextLine("You turn off the music.");
						}
					} else {
						_objectVar[62] = _objectVar[62] + 1;
						_console->printTosText(921);
					}
				} else {
					_objectVar[62] = _objectVar[62] + 1;
					_console->printTosText(920);
				}
			} else {
				_objectVar[62] = _objectVar[62] + 1;
				_console->printTosText(919);
			}
		} else if (objNum == 63) {
//			Ordinal_184(unaff_CS,(int)&_file_name,4192,(int)s_room33.nnn_1060_319f);
//			_Room = 33;
//			InitRoom();
//			LoadRoomAndDumpPic();
//			SetHeroSpriteNumber();
		} else if (objNum == 83) {
//			Ordinal_184(unaff_CS,(int)&_file_name,4192,(int)s_room28.nnn_1060_31ab);
//			_Room = 28;
//			InitRoom();
//			LoadRoomAndDumpPic();
//			SetHeroSpriteNumber();
		} else if (objNum == 82) {
			if (_room->_roomNumber == 41) {
				_room->_roomNumber = 44;
				_console->printTosText(956);
			}
			else {
				_room->_roomNumber = 41;
				_console->printTosText(957);
			}
//			LoadRoomAndDumpPic();
//			InitRoom();
//			SetHeroSpriteNumber();
		} else if (objNum == 52) {
//			_NoScroll = 0;
			if (_objectVar[52] == 1) {
				_player->loadAnimations("shower.nsp");
				setupOtherNspAnimation(0, 2);
				_console->printTosText(193);
				playSound(2,4,-1);
			} else {
				_console->printTosText(194);
			}
		} else if (objNum == 59) {
			if (_objectVar[59] == 2) {
				_objectVar[59] = 1;
				if (_room->_roomNumber == 0 || _room->_roomNumber == 9) {
					_console->printTosText(409);
				}
			}
			else {
				if (_room->_roomNumber == 0 || _room->_roomNumber == 9) {
					_console->printTosText(407);
				}
				_objectVar[59] = 2;
			}
		} else if (objNum == 78) {
			if (_objectVar[78] == 2) {
				_objectVar[78] = 1;
				if (_room->_roomNumber == 0 || _room->_roomNumber == 9) {
					_console->printTosText(409);
				}
			}
			else {
				playSound(41,5,-1);
				if (_room->_roomNumber == 0 || _room->_roomNumber == 9) {
					_console->printTosText(32);
				}
				_objectVar[78] = 2;
			}
		} else if (objNum == 139) {
			_player->loadAnimations("ltladder.nsp");
			setupOtherNspAnimation(0,10);
		} else if (objNum == 76) {
//			_NoScroll = 0;
//			Ordinal_184(unaff_CS,(int)&_file_name,4192,(int)s_room3.nnn_1060_31e1);
//			_Room = 3;
//			LoadRoomAndDumpPic();
//			InitRoom();
//			SetHeroSpriteNumber();
		} else if (objNum == 77) {
			_player->loadAnimations("rtladder.nsp");
			setupOtherNspAnimation(0, 12);
		} else if (objNum == 66 && _objectVar[68] == 0) {
			if (_objectVar[12] == 2) {
				playSound(5,5,-1);
				if (_objectVar[66] == 0) {
					if (_objectVar[67] == 0 && _objectVar[68] == 0) {
						_objectVar[66] = 1;
					} else {
						_objectVar[66] = 2;
					}
				} else {
					_objectVar[66] = 0;
				}
			} else {
				_console->addTextLine("You touch the surface of the ornate sigil.");
			}
		} else if (objNum == 67 && _objectVar[68] == 0) {
			if (_objectVar[12] == 2) {
				playSound(5,5,-1);
				if (_objectVar[67] == 0) {
					if (_objectVar[66] == 1 && _objectVar[68] == 0) {
						_objectVar[67] = 1;
					} else {
						_objectVar[67] = 2;
					}
				} else {
					_objectVar[67] = 0;
				}
			} else {
				_console->addTextLine("You touch the surface of the ornate sigil.");
			}
		} else if ((objNum == 68) && (_objectVar[68] == 0)) {
			if (_objectVar[12] == 2) {
				if (true) {
					if ((_objectVar[66] == 1) && (_objectVar[67] == 1)) {
						playSound(13,5,-1);
						_objectVar[68] = 1;
						setupOtherNspAnimation(0,23);
					} else {
						_objectVar[68] = 2;
					}
				} else {
					_objectVar[68] = 0;
				}
			} else {
				_console->addTextLine("You touch the surface of the ornate sigil.");
			}
		}
		else if (objNum == 84) {
			_console->printTosText(566);
		}
//		_NoScroll = 0;
		return;
	}
	if (_objectVar[138] == 0) {
		_console->printTosText(906);
	}
	else {
		_inventory.addItem(objNum);
		_room->removeObjectFromRoom(objNum);
		_objectVar[138] = 0;
		updateDisplay();
		if ((objNum == 7) && _currentDay == 1) {
			_console->printTosText(905);
			_objectVar[141] = 1;
			_player->loadAnimations("delstore.nsp");
			_objectVar.setMoveObjectPosition(141, {60, 221});
			_objectVar.setMoveObjectRoom(7, 100);
			_objectVar[7] = 1;
		}
	}
}

void DarkseedEngine::useCodeGloves(int16 targetObjNum) {
	if (targetObjNum == 113) {
		putobjunderpillow(14);
		return;
	}

	int16 tosIdx = _objectVar.getUseGlovesTosIdx(targetObjNum);
	if (tosIdx != 0) {
		if (tosIdx < 979) {
			_console->printTosText(tosIdx);
		} else {
			genericresponse(14,targetObjNum, tosIdx);
		}
	}
	if (targetObjNum == 57) {
		_player->loadAnimations("instrshp.nsp");
		setupOtherNspAnimation(0, 43);
	} else if (targetObjNum == 117) {
		_player->loadAnimations("obslev.nsp");
		setupOtherNspAnimation(0, 45);
	}
}

void DarkseedEngine::useCodeMoney(int16 targetObjNum) {
	if ((targetObjNum != 138) && (targetObjNum != 152)) {
		int16 tosIdx = _objectVar.getUseMoneyTosIdx(targetObjNum);
		if (tosIdx == 0) {
			if (targetObjNum == 7) {
				_console->printTosText(961);
			} else if (targetObjNum == 113) {
				putobjunderpillow(8);
			}
		} else if (tosIdx < 979) {
			_console->printTosText(tosIdx);
		} else {
			genericresponse(8, targetObjNum, tosIdx);
		}
		return;
	}
	if (_objectVar[138] == 0) {
		_player->loadAnimations("givclerk.nsp");
		setupOtherNspAnimation(6, 35);
	} else {
		_console->addTextLine("Choose an item before giving clerk more money.");
	}
}

void DarkseedEngine::useCodeJournal(int16 actionObjNum, int16 targetObjNum) {
	if (targetObjNum == 113) {
		putobjunderpillow(actionObjNum);
	} else if ((actionObjNum == 6) && (targetObjNum == 47)) {
		_console->printTosText(46);
	} else if ((actionObjNum == 6) && (targetObjNum == 100)) {
		_console->printTosText(145);
	} else if ((actionObjNum == 6) && (targetObjNum == 136)) {
		_console->printTosText(999);
	} else {
		int16 tosIdx = _objectVar.getUseJournalTosIdx(targetObjNum);
		if (tosIdx != 0) {
			if (tosIdx < 979) {
				_console->printTosText(tosIdx);
			}
			else {
				genericresponse(6,targetObjNum, tosIdx);
			}
		}
	}
}

void DarkseedEngine::useCodeShopItems(int16 actionObjNum, int16 targetObjNum) {
	if (actionObjNum == 7) {
		if (targetObjNum == 44) {
			_player->loadAnimations("opendoor.nsp");
			setupOtherNspAnimation(0, 24);
			_objectVar[44] = 3600;
			playSound(32, 5, -1);
			_inventory.removeItem(7);
			_console->printTosText(730);
		} else if ((targetObjNum == 141) && (_objectVar[141] == 7)) {
			_objectVar[141] = 8;
		} else if (targetObjNum == 47) {
			_console->printTosText(54);
		} else if (targetObjNum == 127) {
			_console->printTosText(106);
		} else if (targetObjNum == 123) {
			_console->printTosText(129);
		} else if (targetObjNum == 100) {
			_console->printTosText(153);
		} else if (targetObjNum == 143) {
			_console->printTosText(189);
		} else if (targetObjNum == 52) {
			_console->printTosText(202);
		} else if (targetObjNum == 145) {
			_console->printTosText(226);
		} else if (targetObjNum == 137) {
			_console->printTosText(300);
		} else if (targetObjNum == 126) {
			_console->printTosText(312);
		} else if (targetObjNum == 103 || targetObjNum == 151) {
			_console->printTosText(362);
		} else if (targetObjNum == 109) {
			_console->printTosText(965);
		} else if (targetObjNum == 61) {
			_console->printTosText(460);
		} else if (targetObjNum == 53) {
			_console->printTosText(489);
			throwmikeinjail();
		} else if (targetObjNum == 46) {
			_console->printTosText(542);
		} else if (targetObjNum < 87 || 98 < targetObjNum) {
			if (targetObjNum < 163 || 168 < targetObjNum) {
				if (targetObjNum == 171) {
					_console->printTosText(679);
				} else if (targetObjNum == 129) {
					genericresponse(7, 129, 998);
				} else if (targetObjNum == 108) {
					_console->printTosText(386);
				} else if (targetObjNum == 110) {
					_console->printTosText(392);
				} else if (targetObjNum == 85) {
					_console->printTosText(434);
				} else if (targetObjNum == 194) {
					genericresponse(7, 194, 989);
				} else if (targetObjNum == 156) {
					_console->printTosText(550);
				} else if (targetObjNum == 172) {
					genericresponse(7, 172, 988);
				} else if (targetObjNum == 71) {
					genericresponse(7, 71, 987);
				} else if (targetObjNum == 101) {
					genericresponse(7, 101, 986);
				} else if (targetObjNum == 120) {
					_console->printTosText(804);
				} else if (targetObjNum == 184) {
					genericresponse(7, 184, 980);
				} else if (targetObjNum == 185) {
					genericresponse(7, 185, 982);
				} else if (targetObjNum == 121) {
					genericresponse(7, 121, 990);
				} else {
					genericresponse(7, targetObjNum, 999);
				}
			} else {
				_console->printTosText(632);
			}
		} else {
			_console->printTosText(585);
		}
		return;
	}
	if (actionObjNum == 38 && targetObjNum == 78) {
		_console->printTosText(42);
		return;
	}
	if (actionObjNum == 38 && targetObjNum == 47) {
		_console->printTosText(60);
		return;
	}
	if (actionObjNum == 36 && targetObjNum == 125) {
		_console->printTosText(77);
		return;
	}
	if (actionObjNum == 38 && targetObjNum == 125) {
		_console->printTosText(78);
		return;
	}
	if (actionObjNum == 38 &&
		(((((targetObjNum == 126 || targetObjNum == 126) || targetObjNum == 197) ||
		   ((targetObjNum == 131 || (targetObjNum == 89)))) ||
		  targetObjNum == 171))) {
		_console->printTosText(89);
		return;
	}
	if (actionObjNum == 37 && targetObjNum == 124) {
		_console->printTosText(28);
	} else if (actionObjNum == 38 && targetObjNum == 127) {
		_console->printTosText(116);
	} else if ((actionObjNum == 36 || actionObjNum == 37) && targetObjNum == 123) {
		_console->printTosText(135);
	} else if (actionObjNum == 38 && (targetObjNum == 123 || targetObjNum == 318)) {
		_console->printTosText(320);
	} else if (targetObjNum == 129) {
		genericresponse(actionObjNum, 129, 998);
	} else if ((actionObjNum == 37) &&
			   ((((((targetObjNum == 142 || (targetObjNum == 124)) ||
					((targetObjNum == 137 || (((targetObjNum == 148 || (targetObjNum == 109)) || (targetObjNum == 59)))))) ||
				   ((targetObjNum == 66 || (targetObjNum == 67)))) ||
				  (targetObjNum == 68)) ||
				 ((((targetObjNum == 133 || (targetObjNum == 194)) ||
					((targetObjNum == 46 ||
					  (((((targetObjNum == 156 || (targetObjNum == 158)) || (targetObjNum == 159)) ||
						 ((targetObjNum == 161 || (targetObjNum == 162)))) ||
						(targetObjNum == 22)))))) ||
				   (((targetObjNum == 42 || (targetObjNum == 35)) ||
					 ((targetObjNum == 196 ||
					   ((((targetObjNum == 170 || (targetObjNum == 172)) || (targetObjNum == 25)) ||
						 (((targetObjNum == 71 || (targetObjNum == 101)) || (targetObjNum == 179)))))))))))))) {
		_console->printTosText(180);
	} else if ((actionObjNum == 37) && (targetObjNum == 52)) {
		_console->printTosText(211);
	} else if ((actionObjNum == 37) && (targetObjNum == 144)) {
		_console->printTosText(218);
	} else if ((actionObjNum == 37) && (targetObjNum == 145)) {
		_console->printTosText(229);
	} else if ((actionObjNum == 38) && (targetObjNum == 145)) {
		_console->printTosText(230);
	} else if (targetObjNum == 112) {
		genericresponse(actionObjNum, 112, 996);
	} else if ((actionObjNum == 38) && (targetObjNum == 49)) {
		_console->printTosText(251);
	} else if ((targetObjNum == 147) && (actionObjNum != 7)) {
		_console->printTosText(319);
	} else if (targetObjNum == 197) {
		genericresponse(actionObjNum, 197, 993);
	} else if ((actionObjNum == 38) && (targetObjNum == 150)) {
		_console->printTosText(354);
	} else if (targetObjNum == 109) {
		if (actionObjNum == 38) {
			_console->printTosText(375);
		} else {
			_console->printTosText(374);
		}
	} else if (targetObjNum == 130) {
		genericresponse(actionObjNum, 130, 991);
	} else if (targetObjNum == 108) {
		_console->printTosText(386);
	} else if (targetObjNum == 110) {
		_console->printTosText(392);
	} else if ((targetObjNum == 131) && (actionObjNum != 7)) {
		_console->printTosText(405);
	} else if (targetObjNum == 85) {
		_console->printTosText(434);
	} else if (targetObjNum == 121) {
		if (actionObjNum == 38) {
			_console->printTosText(445);
		} else if (actionObjNum == 40) {
			_console->printTosText(446);
		} else {
			genericresponse(actionObjNum, 121, 990);
		}
	} else if ((actionObjNum == 37) && (targetObjNum == 122)) {
		_console->printTosText(451);
	} else if ((actionObjNum == 38) || (targetObjNum != 61)) {
		if ((actionObjNum == 36) && (targetObjNum == 133)) {
			_console->printTosText(476);
		} else if (((actionObjNum == 36) || (actionObjNum == 40)) && (targetObjNum == 53)) {
			_console->printTosText(491);
		} else if ((actionObjNum == 37) && (targetObjNum == 53)) {
			_console->printTosText(492);
		} else if (targetObjNum == 194) {
			genericresponse(actionObjNum, 194, 989);
		} else if (targetObjNum == 153) {
			_console->printTosText(405);
		} else if ((actionObjNum == 37) && (targetObjNum == 154)) {
			_console->printTosText(535);
		} else if ((actionObjNum == 36) && (targetObjNum == 46)) {
			_console->printTosText(405);
		} else if ((actionObjNum == 38) && (targetObjNum == 46)) {
			_console->printTosText(545);
		} else if ((actionObjNum == 40) && (targetObjNum == 46)) {
			_console->printTosText(405);
		} else if (targetObjNum == 156) {
			_console->printTosText(550);
		} else if (targetObjNum == 159) {
			_console->printTosText(577);
		} else if ((targetObjNum < 87) || (98 < targetObjNum)) {
			if ((actionObjNum == 37) && ((162 < targetObjNum && (targetObjNum < 169)))) {
				_console->printTosText(634);
			} else if ((actionObjNum == 38) && ((162 < targetObjNum && (targetObjNum < 169)))) {
				_console->printTosText(635);
			} else if (targetObjNum == 172) {
				genericresponse(actionObjNum, 172, 988);
			} else if ((actionObjNum == 37) && (targetObjNum == 174)) {
				_console->printTosText(690);
			} else if ((actionObjNum == 38) && (targetObjNum == 25)) {
				_console->printTosText(696);
			} else if ((actionObjNum == 40) && (targetObjNum == 26)) {
				_console->printTosText(700);
			} else if (targetObjNum == 71) {
				genericresponse(actionObjNum, 71, 987);
			} else if ((actionObjNum == 39) && (targetObjNum == 44)) {
				_console->printTosText(731);
			} else if (targetObjNum == 101) {
				genericresponse(actionObjNum, 101, 986);
			} else if (targetObjNum == 113) {
				_console->printTosText(753);
			} else if (targetObjNum == 41) {
				_console->printTosText(757);
			} else if ((actionObjNum == 36) && (targetObjNum == 179)) {
				_console->printTosText(764);
			} else if ((actionObjNum == 38) && (targetObjNum == 179)) {
				_console->printTosText(762);
			} else if ((actionObjNum == 38) && (targetObjNum == 180)) {
				_console->printTosText(769);
			} else if (targetObjNum == 120) {
				_console->printTosText(804);
			} else if ((actionObjNum == 38) && (targetObjNum == 119)) {
				_console->printTosText(808);
			} else if ((actionObjNum == 38) && (targetObjNum == 114)) {
				genericresponse(38, 114, 984);
			} else if (targetObjNum == 184) {
				genericresponse(actionObjNum, 184, 980);
			} else if (targetObjNum == 185) {
				genericresponse(actionObjNum, 185, 982);
			} else if ((actionObjNum == 38) && (targetObjNum == 79)) {
				_console->printTosText(833);
			} else if ((targetObjNum == 79) && (actionObjNum != 7)) {
				_console->printTosText(836);
			} else if ((actionObjNum == 38) && (targetObjNum == 58)) {
				_console->printTosText(849);
			} else {
				genericresponse(actionObjNum, targetObjNum, 999);
			}
		} else {
			_console->printTosText(587);
		}
	} else {
		_console->printTosText(462);
	}
}

void DarkseedEngine::useCrowBar(int16 targetObjNum) {
	int16 tosIdx = _objectVar.getUseCrowbarTosIdx(targetObjNum);
	if (tosIdx != 0) {
		if (tosIdx < 979) {
			_console->printTosText(tosIdx);
		} else {
			genericresponse(5,targetObjNum, tosIdx);
		}
	}
	if (targetObjNum == 42) {
		if ((_objectVar[42] == 0) || (_objectVar[42] == 4)) {
			_player->loadAnimations("crowbar.nsp");
			setupOtherNspAnimation(0, 17);
		} else {
			_console->printTosText(962);
		}
	}
}


void DarkseedEngine::lookCode(int objNum) {
	if (objNum == 71 && _objectVar[71] == 2) {
		_console->addTextLine("You see the car keys in the ignition.");
		return;
	}
	if (objNum == 189) {
		_console->addTextLine("You see the iron bars of your cell.");
		return;
	}
	if (objNum == 141) {
		_console->addTextLine("You see Delbert, not much to look at.");
		return;
	}
	if (objNum == 42) {
		switch(_objectVar[42]) {
		case 0:
			_console->printTosText(652);
			break;
		case 1:
			_console->printTosText(659);
			_objectVar[42] = 2;
			break;
		case 2:
			_console->printTosText(659);
			break;
		case 3:
			_console->printTosText(658);
			break;
		case 4:
			_console->printTosText(652);
			break;
		default:
			break;
		}
		return;
	}
	if (objNum == 101) {
		switch(_objectVar[101]) {
		case 0:
		case 4:
			_console->printTosText(732);
			break;
		case 1:
		case 2:
			_console->printTosText(734);
			_objectVar[101] = 2;
			break;
		case 3:
			_console->printTosText(735);
			break;
		default:
			break;
		}
		return;
	}
	if (objNum == 25 && _cursor.getY() > 40) {
		if (_objectVar[80] > 1) {
			if (_objectVar[25] == 0 || _objectVar[25] == 100) {
				_console->printTosText(691);
			}
			else if (_objectVar[25] == 1 || _objectVar[25] == 2) {
				_objectVar[25] = 2;
				_console->printTosText(697);
			}
			else {
				_console->printTosText(693);
			}
		}
		return;
	}
	if (objNum == 138) {
		_console->addTextLine("You see the clerk.");
		return;
	}
	if (objNum == 86 && _objectVar[86] != 0) {
		_console->addTextLine("You see the open glove box.");
		return;
	}
	if (objNum == 9) {
		_room->_collisionType = 0;
		_room->removeObjectFromRoom(9);
		_objectVar.setMoveObjectRoom(9, 100);
		showFullscreenPic(g_engine->isCdVersion() ? "paper_c.pic" : "paper-c.pic");
		return;
	}
	if (objNum == 30) {
		_objectVar[30] = 1;
		_room->_collisionType = 0;
		_room->removeObjectFromRoom(30);
		_objectVar.setMoveObjectRoom(30, 100);
		showFullscreenPic(g_engine->isCdVersion() ? "note_c.pic" : "note-c.pic");
		return;
	}
	if (objNum == 194) {
		if (_objectVar[53] == 2) {
			_console->printTosText(494);
		}
		else {
			_console->printTosText(496);
		}
		return;
	}
	if (objNum > 103 && objNum < 111) {
		int baseIdx = 370;
		if (objNum == 108) {
			baseIdx = 382;
		} else if (objNum == 110) {
			baseIdx = 388;
		}
		_console->printTosText(baseIdx + _objectVar[objNum] * 2);
		return;
	}
	if (objNum == 137) {
		if (_objectVar[30] == 0) {
			_console->printTosText(293);
		} else {
			_console->printTosText(295);
		}
		return;
	}
	if (objNum == 59) {
		if (_objectVar[59] == 2) {
			_console->printTosText(408);
		} else {
			_console->printTosText(31);
		}
	}
	if (objNum == 78) {
		if (_objectVar[78] == 2) {
			_console->printTosText(408);
		} else {
			_console->printTosText(406);
		}
	}
	if (objNum == 48 && _objectVar[48] == 0) {
		sargoanim();
		return;
	}
	if (objNum == 35 && _cursor.getY() < 40) {
		if (_objectVar[35] == 0) {
			_console->printTosText(670);
		} else {
			printTime();
		}
		return;
	}
	if (objNum == 46) {
		if (_objectVar[46] == 1) {
			_console->printTosText(538);
		} else {
			_console->printTosText(536);
		}
	}
	if (objNum == 84) {
		_console->printTosText(565);
	}
	if ((objNum == 14) && (_objectVar[86] == 0)) {
		return;
	}
	if (objNum == 51) {
		if (_objectVar[187] == 0) {
			_console->printTosText(851);
		}
		else if (_objectVar[51] == 0) {
			_console->printTosText(853);
			_objectVar[51] = 1;
			keeperanim();
			_objectVar.setObjectRunningCode(72, 1);
			_inventory.addItem(24);
			_console->printTosText(959);
		}
		else {
			_console->printTosText(960);
		}
	}
	if (objNum == 55) {
		_console->printTosText(776);
		showFullscreenPic("diagram.pic");
		return;
	}
	if (objNum == 34) {
		_objectVar[34] = 1;
		showFullscreenPic(_room->isGiger() ? "gbprint1.pic" : "bprint1.pic");
		return;
	}
	if (objNum == 18 && _cursor.getY() < 40) {
		showFullscreenPic(_room->isGiger() ? "gbcard01.pic" : "cbcard01.pic");
		return;
	}
	if (objNum == 6 && _cursor.getY() < 40) {
		showFullscreenPic(_room->isGiger() ? "gdiary.pic" : "cdiary.pic");
		_objectVar[6] = 1;
		return;
	}
	if (objNum == 12) {
		if (_cursor.getY() < 40) {
			showFullscreenPic(_room->isGiger() ? "gjourn01.pic" : "cjourn01.pic");
			_objectVar[12] = 2;
		} else if (_objectVar[12] < 2) {
			_console->printTosText(567);
		} else {
			_console->printTosText(566);
		}
		return;
	}
	if (objNum == 29) {
		showFullscreenPic(_room->isGiger() ? "gjour201.pic" : "cjour201.pic");
		if (_objectVar[29] == 0) {
			_objectVar[29] = 1;
		}
		return;
	}
	if (objNum == 10) {
		showFullscreenPic(_room->isGiger() ? "glcard.pic" : "clcard.pic");
		return;
	}
	if (objNum == 100) {
		if (_objectVar[100] == 0) {
			_console->printTosText(139);
			_objectVar[100] = 1;
		} else if (_objectVar[100] == 1 || _objectVar[100] == 2) {
			_objectVar[100] = 2;
			_console->printTosText(141);
		} else if (_objectVar[100] == 3) {
			_console->printTosText(143);
		}
		return;
	}
	if (objNum == 103) {
		printTime();
	}
	if (_cursor.getY() > 39 && objNum != 77) {
		int eyeTosIdx = _objectVar.getEyeDescriptionTosIdx(objNum);
		if (eyeTosIdx < 979 && eyeTosIdx != 0)  {
			_console->printTosText(eyeTosIdx);
		}
		// TODO
//		else if (978 < *(int *)((int)_eyedescriptions + objNum * 2)) {
//			genericresponse(3,objNum,*(undefined2 *)((int)_eyedescriptions + objNum * 2));
//		}
		return;
	}
	_console->addTextLine(Common::String::format("You see the %s.", _objectVar.getObjectName(objNum)));
}

void DarkseedEngine::wongame() {
	// TODO
}

void DarkseedEngine::printTime() {
	_console->printTosText(958);
	int hour = g_engine->_currentTimeInSeconds / 60 / 60 + 1;
	_console->addToCurrentLine(Common::String::format("%d: %02d %s", hour % 12, (g_engine->_currentTimeInSeconds / 60) % 60, hour < 12 ? "a.m." : "p.m."));
}

void DarkseedEngine::showFullscreenPic(const Common::Path &filename) {
	if (_fullscreenPic) {
		delete _fullscreenPic;
	}
	_fullscreenPic = new Pic();
	if(!_fullscreenPic->load(filename)) {
		delete _fullscreenPic;
		_fullscreenPic = nullptr;
		error("Failed to load %s", filename.toString().c_str());
	}
	Common::String filePathStr = filename.toString();
	debug("Loaded %s", filePathStr.c_str());
	Common::Path palFilename = Common::Path(filePathStr.substr(0, filePathStr.size() - 4) + ".pal");
	Pal pal;
	pal.load(palFilename);
}

void DarkseedEngine::keeperanim() {
	// TODO
}

void DarkseedEngine::sargoanim() {
	// TODO
}

void DarkseedEngine::playCutscene(const Common::String cutsceneId) {
	debug("Play Cutscene %s", cutsceneId.c_str()); // TODO play cutscenes.
}

void DarkseedEngine::gancanim() {
	// TODO
}

void DarkseedEngine::getPackageObj(int packageType) {
	_console->printTosText(424);
	if (packageType == 1) {
		playCutscene("D");
	}
	if (packageType == 2) {
		_inventory.addItem(15);
		showFullscreenPic("shard.pic");
	}
	if (packageType == 3) {
		_inventory.addItem(20);
		showFullscreenPic("handle.pic");
	}
}

void DarkseedEngine::playSound(int16 unk, uint8 unk1, int16 unk2) {
	// TODO...
}

void DarkseedEngine::nextFrame(int nspAminIdx) {
	isAnimFinished_maybe = false;
	spriteAnimCountdownTimer[nspAminIdx]--;
	_FrameAdvanced = false;
	if (spriteAnimCountdownTimer[nspAminIdx] < 1) {
		const Obt &anim = _player->_animations.getAnimAt(nspAminIdx);
		animIndexTbl[nspAminIdx]++;
		_FrameAdvanced = true;
		if (animIndexTbl[nspAminIdx] == anim.numFrames) {
			animIndexTbl[nspAminIdx] = 0;
			isAnimFinished_maybe = true;
		}
		spriteAnimCountdownTimer[nspAminIdx] = anim.frameDuration[animIndexTbl[nspAminIdx]];
	}
}

void DarkseedEngine::stuffPlayer() {
	// TODO
}
void DarkseedEngine::putobjunderpillow(int objNum) {
	// TODO
}

void DarkseedEngine::genericresponse(int16 useObjNum, int16 targetObjNum, int16 tosIdx) {
	// TODO
}

void DarkseedEngine::updateHeadache() {
	headAcheMessageCounter++;
	headAcheMessageCounter &= 63;
	if (headAcheMessageCounter == 0) {
		headacheMessageIdx++;
		if (headacheMessageIdx > 4) {
			headacheMessageIdx = 0;
		}
		if (_objectVar[112] == 0 && !_sound->isPlayingSpeech()) {
			_console->printTosText(headacheMessageIdx + 9);
		}
	}
}

void DarkseedEngine::gotosleepinjail() {
	gotonextmorning();
	playDayChangeCutscene();
}

void DarkseedEngine::gotonextmorning() {
	_currentDay++;
	_objectVar[47] = 0;
	_objectVar[62] = 0;
	_objectVar[112] = 0;
	_objectVar[52] = 1;
	_currentTimeInSeconds = 32400;
	_sound->resetSpeech();
	if (_currentDay == 2) {
		_objectVar.setMoveObjectRoom(7, 253);
	} else if (_currentDay == 3) {
		_objectVar.setMoveObjectRoom(7, 255);
	}
}

void DarkseedEngine::playDayChangeCutscene() {
	if (_currentDay == 4) {
		playCutscene("Y");
	} else if (_currentDay == 2) {
		playCutscene("B");
	} else if (_currentDay == 3) {
		playCutscene("C");
	}
}

void DarkseedEngine::closeShops() {
	if (_currentTimeInSeconds > 68400) {
		if (_room->_roomNumber == 15) {
			_previousRoomNumber = 15;
			_console->printTosText(79);
			changeToRoom(11);
		}
		if (_room->_roomNumber == 16) {
			_previousRoomNumber = 16;
			_console->printTosText(80);
			changeToRoom(11);
		}
		if (_room->_roomNumber == 28 || (_room->_roomNumber > 16 && _room->_roomNumber < 24)) {
			_previousRoomNumber = 17;
			_console->printTosText(98);
			changeToRoom(12);
		}
	}
}

void DarkseedEngine::initDelbertAtSide() {
	_objectVar[141] = 12;
	_objectVar.setMoveObjectX(141, 563);
	if (!isPlayingAnimation_maybe || otherNspAnimationType_maybe != 26) {
		_player->_heroMoving = false;
		_player->_walkTarget = _player->_position;
		//*(undefined *)&_ActionToPerform = 0; TODO
	}
}

void DarkseedEngine::throwmikeinjail() {
	_console->printTosText(912);
	if (_room->_roomNumber == 15) {
		playSound(0,6,-1);
	}
	_player->_position = {250, 200};
	isPlayingAnimation_maybe = false;
	_player->_frameIdx = 26;
	changeToRoom(30);
	_objectVar.setMoveObjectRoom(28, 255);
	_inventory.removeItem(28);
	_objectVar[1] = 500;
	_objectVar[88] = 0;
	_objectVar.setObjectRunningCode(72, 0);
}

void DarkseedEngine::runObjects() {
	if (((g_engine->_objectVar[44] != 0) && (g_engine->_objectVar[71] == 2))) {
		g_engine->_objectVar[44] -= 2;
		if (g_engine->_objectVar[44] == 0) {
			g_engine->_objectVar[71] = 2;
		}
	}

	if (g_engine->_objectVar[35] != 0) {
		g_engine->_objectVar[35]--;
	}
	if (((_room->_roomNumber == 46) || (_room->_roomNumber == 60)) &&
		(((_SoundTimer & 15) == 0 && (g_engine->_objectVar[57] == 1)))) {
		playSound(9,5,-1);
	}
	if ((_room->_roomNumber == 12) && (_SoundTimer > 5)) {
		_SoundTimer = 0;
	}
	if (((_room->_roomNumber == 8) && (_SoundTimer > 5)) && (g_engine->_objectVar[110] != 0)) {
		_SoundTimer = 0;
	}
	if ((_room->_roomNumber == 38) && ((_SoundTimer & 31) == 0)) {
		playSound(23,5,-1);
	}
	if ((_room->_roomNumber == 45) && ((_SoundTimer & 63) == 0)) {
		playSound(10,5,-1);
	}

	int16 delbertSpriteIdx = 0;
	if (_objectVar[141] == 8 && _room->_roomNumber == 31) {
		// Fido brings back stick after player gives delbert the scotch.
		_player->_herowaiting = true;
		if (_objectVar[45] == 3) {
			_room->loadLocationSprites("deldrink.nsp");
			_objectVar[141] = 9;
			_room->_locObjFrame[0] = 0;
			_room->_locObjFrameTimer[0] = 2;
			delbertSpriteIdx = _room->_locationSprites.getAnimAt(0).frameNo[_room->_locObjFrame[0]];
		} else {
			delbertSpriteIdx = _room->_locationSprites.getAnimAt(0).frameNo[_room->_locObjFrame[0]];
			if (delbertSpriteIdx != 9) {
				_room->advanceFrame(0);
				delbertSpriteIdx = _room->_locationSprites.getAnimAt(0).frameNo[_room->_locObjFrame[0]];
				delthrowstick(delbertSpriteIdx);
			}
			if (_objectVar[141] == 8) {
				const Sprite &sprite = _room->_locationSprites.getSpriteAt(delbertSpriteIdx);
				delthrowstick(delbertSpriteIdx);
				g_engine->_sprites.addSpriteToDrawList(145, 140, &sprite, 240 - (sprite.height + 140), sprite.width, sprite.height, false);
			}
		}
	}
	if (_objectVar[141] == 9) {
		// delbert drinks scotch
		_room->advanceFrame(0);
		delbertSpriteIdx = _room->_locationSprites.getAnimAt(0).frameNo[_room->_locObjFrame[0]];
		if (_FrameAdvanced && delbertSpriteIdx == 5) {
			playSound(32, 5, -1);
		}
		if (_room->_ObjRestarted) {
			_objectVar[141] = 10;
			_console->printTosText(910);
			_inventory.removeItem(7);
		}
		Common::Point delbertPos = {125, 140};
		_objectVar.setMoveObjectPosition(141, delbertPos);
		if (_objectVar[141] == 9) {
			const Sprite &sprite = _room->_locationSprites.getSpriteAt(delbertSpriteIdx);
			g_engine->_sprites.addSpriteToDrawList(125, 140, &sprite, 240 - (sprite.height + 140), sprite.width, sprite.height, false);
		}
	}
	if (_objectVar[141] == 10 && _room->_roomNumber == 31) {
		// delbert walks off screen.
		_room->advanceFrame(1);
		delbertSpriteIdx = _room->_locationSprites.getAnimAt(1).frameNo[_room->_locObjFrame[1]];
		Common::Point delbertPos = _objectVar.getMoveObjectPosition(141);
		if (_FrameAdvanced) {
			delbertPos.x += 10;
			_objectVar.setMoveObjectPosition(141, delbertPos);
		}
		if (delbertPos.x < 560) {
			const Sprite &sprite = _room->_locationSprites.getSpriteAt(delbertSpriteIdx);
			g_engine->_sprites.addSpriteToDrawList(delbertPos.x, 140, &sprite, 240 - (sprite.height + 140), sprite.width, sprite.height, false);
			_room->updateRoomObj(141,delbertPos.x,sprite.width,140,sprite.height);
		} else {
			_objectVar[141] = 11;
			_room->removeObjectFromRoom(141);
			_player->_herowaiting = false;
		}
	}
	if (_objectVar[141] == 7 && _room->_roomNumber == 31) {
		_room->advanceFrame(0);
		delbertSpriteIdx = _room->_locationSprites.getAnimAt(0).frameNo[_room->_locObjFrame[0]];
		const Sprite &sprite = _room->_locationSprites.getSpriteAt(delbertSpriteIdx);
		delthrowstick(delbertSpriteIdx);
		g_engine->_sprites.addSpriteToDrawList(145, 140, &sprite, 240 - (sprite.height + 140), sprite.width, sprite.height, false);
		_room->updateRoomObj(141,145,sprite.width,140,sprite.height);
	}
	if ((_objectVar[141] == 7 || _objectVar[141] == 8) && _room->_roomNumber == 31) {
		if (delbertSpriteIdx > 7 && _objectVar[45] == 0) {
			Common::Point stickPosition = _objectVar.getMoveObjectPosition(19);
			if (stickPosition.y < 205 && _yvec != 100) {
				stickPosition.x += 12;
				stickPosition.y += _yvec;
				_objectVar.setMoveObjectPosition(19, stickPosition);
				_yvec++;
			}
			if (stickPosition.y > 205) {
				stickPosition = {495, 205};
				_objectVar.setMoveObjectPosition(19, stickPosition);
				_yvec = 100;
			}
		}
		if (_objectVar[45] < 3 && _room->_roomNumber == 31 && _objectVar[141] > 6) {
			_objectVar.setMoveObjectRoom(19, 100);
			_room->advanceFrame(_objectVar[45] + 1);
			Common::Point stickPosition = _objectVar.getMoveObjectPosition(19);
			if (_room->_ObjRestarted && _objectVar[45] < 2) {
				_objectVar[45] = (_objectVar[45] == 0) ? 1 : 0;
				if (_objectVar[45] == 0) {
					stickPosition = {230, 205};
				} else {
					stickPosition.x = 1000;
				}
				_objectVar.setMoveObjectPosition(19, stickPosition);
			}
			if (stickPosition.x < 1000) {
				const Sprite &stickSprite = _baseSprites.getSpriteAt(19);
				g_engine->_sprites.addSpriteToDrawList(stickPosition.x, stickPosition.y, &stickSprite, 255, stickSprite.width, stickSprite.height, false);
			}
			Common::Point fidoPosition = _objectVar.getMoveObjectPosition(45);
			int16 fidoSpriteIdx = _room->_locationSprites.getAnimAt(_objectVar[45] + 1).frameNo[_room->_locObjFrame[_objectVar[45] + 1]];
			if (_FrameAdvanced) {
				if (_objectVar[45] == 2) {
					fidoPosition.x += 30;
					_objectVar.setMoveObjectPosition(45, fidoPosition);
					if (fidoPosition.x > 564) {
						_objectVar.setMoveObjectRoom(19, 31);
						_objectVar[45]++;
					}
				} else if (_objectVar[45] == 0) {
					if (fidoSpriteIdx > 11 && fidoSpriteIdx < 18) {
						fidoPosition.x += 30;
						_objectVar.setMoveObjectPosition(45, fidoPosition);
					}
				} else if (fidoSpriteIdx > 19 && fidoSpriteIdx < 26) {
					fidoPosition.x -= 30;
					_objectVar.setMoveObjectPosition(45, fidoPosition);
				}
			}
			if (_objectVar[45] < 3) {
				bool flipFidoSprite = false;
				if ((_objectVar[45] == 0) && (_room->_locObjFrame[1] < 6)) {
					flipFidoSprite = true;
				}
				if ((_objectVar[45]== 1) && (7 < _room->_locObjFrame[2])) {
					flipFidoSprite = true;
				}
				if (((_objectVar[45] == 1) && (_room->_locObjFrame[2] == 9)) && (_objectVar[141] ==  8)
				) {
					stickPosition = {230, 205};
					_objectVar.setMoveObjectPosition(19, stickPosition);
					_objectVar[45] = 2;
				}
				const Sprite &fidoSprite = _room->_locationSprites.getSpriteAt(fidoSpriteIdx);
				g_engine->_sprites.addSpriteToDrawList(fidoPosition.x, 173, &fidoSprite, 240 - (fidoSprite.height + 173), fidoSprite.width, fidoSprite.height, flipFidoSprite);
			}
		}
	}
	if (_objectVar[141] > 0 && _objectVar[141] < 4) {
		_player->_herowaiting = true;
		if (_player->_walkToSequence == false && _player->_position.x != 290 && _player->_position.y != 209) {
			_player->_walkTarget = {290, 209};
			// TODO reset walk path here
			_player->playerFaceWalkTarget();
		}
		if (_player->_position.x == 290 && _player->_position.y == 209 && !_player->_playerIsChangingDirection && _player->_direction != 3) {
			_player->changeDirection(_player->_direction, 3);
		}
		isAnimFinished_maybe = false;
		if (!_sound->isPlayingSpeech() && (_objectVar[141] != 2 || _delbertspeech > 64)) {
			nextFrame(_objectVar[141] - 1);
		}
		if (isAnimFinished_maybe) {
			_objectVar[141]++;
			if (_objectVar[141] == 2) {
				_delbertspeech = 63;
				_console->printTosText(907);
//				if (_gGameSound == 0) { TODO
//					waitxticks(240);
//				}
			} else if (_objectVar[141] == 4) {
				_player->_herowaiting = false;
			}
			isAnimFinished_maybe = false;
		}
		if (!_sound->isPlayingSpeech() && _objectVar[141] == 2) {
			if (_delbertspeech < 65) {
				_delbertspeech++;
			}
			if (_delbertspeech == 64) {
				_console->printTosText(908);
				_sound->waitForSpeech();
			}
			else if (*(char *)&_delbertspeech == 65) {
				setupOtherNspAnimation(3, 20);
				spriteAnimCountdownTimer[1] = 3;
				_delbertspeech = 72;
			}
		}
		const Sprite &sprite = _player->_animations.getSpriteAt(_player->_animations.getAnimAt(_objectVar[141] - 1).frameNo[animIndexTbl[_objectVar[141] - 1]]);
		Common::Point delbertPosition = _objectVar.getMoveObjectPosition(141);
		if (_FrameAdvanced) {
			if (_objectVar[141] == 1) {
				delbertPosition.x += 15;
				delbertPosition.y -= 1;
			} else if (_objectVar[141] == 3) {
				delbertPosition.x -= 15;
				delbertPosition.y += 1;
			}
			_objectVar.setMoveObjectPosition(141, delbertPosition);
		}
		int spriteX = delbertPosition.x;
		if (_objectVar[141] == 2) {
			spriteX += 23;
		}
		int spriteY = delbertPosition.y - sprite.height;
		if (_objectVar[141] < 4) {
			g_engine->_sprites.addSpriteToDrawList(spriteX, spriteY, &sprite, 240 - delbertPosition.y, sprite.width, sprite.height, _objectVar[141] == 3 ? true : false);
		}
	}
	if (_room->_roomNumber == 32 && _currentDay == 2 && _currentTimeInSeconds > 64799 && (_objectVar[141] == 5 || _objectVar[141] == 6)) {
		// walk delbert off into garden.
		_room->advanceFrame(_objectVar[141] - 4);
		if (_room->_ObjRestarted) {
			_objectVar[141]++;
			if (_objectVar[141] == 7) {
				_player->_herowaiting = false;
			}
		}
		Common::Point delbertPosition = _objectVar.getMoveObjectPosition(141);
		const Sprite &sprite = _room->_locationSprites.getSpriteAt(_room->_locationSprites.getAnimAt(_objectVar[141] - 4).frameNo[_room->_locObjFrame[_objectVar[141] - 4]]);
		if (_FrameAdvanced && _objectVar[141] > 5) {
			delbertPosition.x += 8;
			_objectVar.setMoveObjectX(141, delbertPosition.x);
		}
		if (_objectVar[141] != 7) {
			g_engine->_sprites.addSpriteToDrawList(delbertPosition.x, 135, &sprite, 240 - (sprite.height + 132), sprite.width, sprite.height, false);
		}
	}
	if (_room->_roomNumber == 32 && _currentDay == 2 && _currentTimeInSeconds > 64799 && _objectVar[141] == 4) {
		_objectVar.setMoveObjectX(141, 395);
		const Sprite &sprite = _room->_locationSprites.getSpriteAt(0);
		g_engine->_sprites.addSpriteToDrawList(395, 132, &sprite, 240 - (sprite.height + 132), sprite.width, sprite.height, false);
		moveplayertodelbert();
		if (!_player->_playerIsChangingDirection && !_player->_heroMoving) {
			_console->printTosText(909);
			_objectVar[141]++;
			_player->_herowaiting = true;
		}
	}
	if (_room->_roomNumber == 32 && _currentDay == 2 && _objectVar[141] == 12) {
		Common::Point delbertPosition = _objectVar.getMoveObjectPosition(141);
		if(_room->advanceFrame(2)) {
			delbertPosition.x -= 8;
			_objectVar.setMoveObjectX(141, delbertPosition.x);
		}
		const Sprite &sprite = _room->_locationSprites.getSpriteAt(_room->_locationSprites.getAnimAt(2).frameNo[_room->_locObjFrame[2]]);
		g_engine->_sprites.addSpriteToDrawList(delbertPosition.x, 135, &sprite, 240 - (sprite.height + 135), sprite.width, sprite.height, true);
		if (delbertPosition.x < 396) {
			_room->_locObjFrameTimer[2] = _room->_locationSprites.getAnimAt(2).frameDuration[_room->_locObjFrame[2]];
			_room->_locObjFrame[2] = 0;
			_objectVar[141] = 4;
		}
		moveplayertodelbert();
	}
	if (_objectVar.getObjectRunningCode(72) == 1 &&
		(_room->_roomNumber < 10 || _room->_roomNumber == 60 || _room->_roomNumber == 61)) {
		_objectVar.setObjectRunningCode(72, 2);
		_console->printTosText(933);
	}
	// jail sargent
	if (_room->_roomNumber == 30 && (_objectVar.getObjectRunningCode(53) == 1 || _objectVar.getObjectRunningCode(53) == 2)
			  && (otherNspAnimationType_maybe != 40 || !isPlayingAnimation_maybe)) {
		if (_objectVar.getObjectRunningCode(53) == 1) {
			int oldFrame = animIndexTbl[0];
			nextFrame(0);
			if (isAnimFinished_maybe) {
				_objectVar.setObjectRunningCode(53, 2);
				animIndexTbl[0] = oldFrame;
			}
		}
		_room->removeObjectFromRoom(189);
		const Sprite &sprite = _player->_animations.getSpriteAt(_player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]]);
		g_engine->_sprites.addSpriteToDrawList(463, 99, &sprite, 255, sprite.width, sprite.height, false);
		_room->updateRoomObj(64,467,200,99,200);
	}
	if ((((!isPlayingAnimation_maybe || otherNspAnimationType_maybe != 39) && _room->_roomNumber == 10) &&
		 _objectVar.getObjectRunningCode(72) != 0) &&
		((_currentDay == 3 && _currentTimeInSeconds > 39600 && _objectVar[57] == 0) ||
		  _objectVar[88] != 0)) {
		if (_player->_position.x == 322 && _player->_position.y == 226) {
			setupOtherNspAnimation(1, 39);
		} else {
			const Sprite &sprite = _room->_locationSprites.getSpriteAt(4);
			_room->calculateScaledSpriteDimensions(sprite.width, sprite.height, 224);
			g_engine->_sprites.addSpriteToDrawList(348 - scaledSpriteWidth, 224 - scaledSpriteHeight, &sprite, 224, scaledSpriteWidth, scaledSpriteHeight, false);
			if (_player->_heroMoving && _player->_playerIsChangingDirection == 0 && _player->_direction != 1 &&
				(_player->_position.x < 368 || _player->_position.y < 200) &&
				   (!isPlayingAnimation_maybe || otherNspAnimationType_maybe == 53) && _player->_walkTarget.x != 322 &&
				  _player->_walkTarget.y != 226) {
				_player->_heroMoving = false;
			}
			if (_player->_position.x < 369 && !_player->_heroMoving &&
				 (!isPlayingAnimation_maybe || otherNspAnimationType_maybe == 53) &&
				(_player->_position.x != 322 || _player->_position.y != 226)) {
				if (isPlayingAnimation_maybe && otherNspAnimationType_maybe == 53) {
					isPlayingAnimation_maybe = false;
				}
				if (!_player->_heroMoving) {
					Common::Point oldCursor = g_engine->_cursor.getPosition();
					Common::Point newTarget = {322,226};
					g_engine->_cursor.setPosition(newTarget);
					_player->calculateWalkTarget();
					g_engine->_cursor.setPosition(oldCursor);
					_player->playerFaceWalkTarget();
					if (!_printedcomeheredawson) {
						_console->printTosText(934);
						_printedcomeheredawson = true;
					}
				}
			}
		}
	}
	if (_objectVar[79] == 0 && _room->_roomNumber == 53) { // evil fido
		if (_player->_heroMoving && _player->_walkTarget.x > 279 && _player->_walkTarget.y < 220) {
//			_player->_CurrentConnector = 255; TODO do we need this?
			_player->_walkTarget = {323, 202};
		}
		if (_player->_position.x == 323 && _player->_position.y == 202) {
			_objectVar[79] = 4;
			playSound(20, 5, -1);
		}
	}
	if (((_objectVar.getObjectRunningCode(58) != 0) && (_objectVar[21] == 0)) && (_room->_roomNumber == 55)) {
		if (_player->_position.x == 369) {
			rundrekethsequence();
		}
		if (_player->_heroMoving && _player->_walkTarget.x > 309) {
			_player->_walkTarget = {369, 219};
		}
	}
	// phone ringing logic
	if (_objectVar._objectRunningCode[47] != 0 && (_currentDay == 1 || _currentDay == 3)) {
		_objectVar._objectRunningCode[47] += 2;
		switch (_objectVar._objectRunningCode[47]) {
		case 120:
		case 300:
		case 480:
		case 660:
		case 900:
			if (_room->_roomNumber < 10 || _room->_roomNumber == 61 || _room->_roomNumber == 62) {
				if (_room->_roomNumber == 0) {
					playSound(24, 5, -1);
				} else {
					playSound(25, 5, -1);
				}
				_console->addTextLine("The phone is ringing.");
			}
			break;
		case 1080:
			_objectVar._objectRunningCode[47] = 0;
			_objectVar[47] = 1;
			break;
		default:
			break;
		}
	}
	// handle package logic.
	if (_objectVar._objectRunningCode[140] != 0) {
		switch(_currentTimeInSeconds - 36000) {
		case 120:
		case 121:
		case 420:
		case 421:
		case 720:
		case 721:
			if (((_room->_roomNumber < 10) || (_room->_roomNumber == 61)) || (_room->_roomNumber == 62)) {
				if (_room->_roomNumber == 6) {
//					FUN_1208_0dac_sound_related(93,5); TODO floppy sound
					playSound(27, 5, -1);
				}
				else {
					playSound(29, 5, -1);
//					FUN_1208_0dac_sound_related(95,5); TODO floppy sound
				}
				_console->addTextLine("The doorbell is ringing.");
			}
			break;
		case 900:
		case 901:
			leavepackage();
			break;
		}
		if (_currentTimeInSeconds - 36000 > 901 && _objectVar._objectRunningCode[140] != 0) {
			leavepackage();
		}
	}
}

void DarkseedEngine::moveplayertodelbert() {
	if (_room->_roomNumber == 32 && !isPlayingAnimation_maybe) {
		_player->_herowaiting = true;
		if (!_player->_heroMoving && _player->_position.x != 364 && _player->_position.y != 198) {
			Common::Point oldCursor = g_engine->_cursor.getPosition();
			Common::Point newTarget = {364,198};
			g_engine->_cursor.setPosition(newTarget);
			_player->calculateWalkTarget();
			g_engine->_cursor.setPosition(oldCursor);
			_player->playerFaceWalkTarget();
		}
		if (_player->_position.x == 364 && _player->_position.y == 198 && !_player->_playerIsChangingDirection && _player->_direction != 1) {
			_player->changeDirection(_player->_direction, 1);
			_player->_walkTarget = _player->_position;
			_player->_heroMoving = false;
		}
	}
}

void DarkseedEngine::rundrekethsequence() {
	// TODO
	error("implement rundrekethsequence()");
}

void DarkseedEngine::delthrowstick(int16 spriteNum) {
	if (_FrameAdvanced && spriteNum == 8) {
		Common::Point stickPos = {265, 150};
		_objectVar.setMoveObjectPosition(19, stickPos);
		_yvec = -6;
		playSound(3, 1, -1);
	}
	if (_room->_locObjFrame[0] == 2) {
		_objectVar.setMoveObjectX(19, 1000);
	}
}

void DarkseedEngine::leavepackage() {
	int packageObjNum = _currentDay + 30;
	int16 xPos = _currentDay * 10 + 299;
	Common::Point packagePos = {xPos, 191};
	_objectVar.setMoveObjectPosition(packageObjNum, packagePos);
	if (_room->_roomNumber == 10) {
		_objectVar.setMoveObjectRoom(packageObjNum, 99);
	} else {
		_objectVar.setMoveObjectRoom(packageObjNum, 10);
	}
//	_MoveObjectDepth[packageObjNum] = 0; TODO do we need this? It doesn't appear to be used.
	_objectVar._objectRunningCode[140] = 0;
}

} // End of namespace Darkseed
