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
	_useCode = new UseCode(_console, _player, _objectVar, _inventory);

	// Set the engine's debugger console
	setDebugger(new DebugConsole(_tosText));

	_frame.load("cframe.pic");

	_baseSprites.load("cbase.nsp");

	_cursor.updatePosition(0x140,0xaf);
	_cursor.setCursorType(Pointer);
	_cursor.showCursor(true);

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
		_cutscene.play('I');
	}

	while (!shouldQuit()) {
		gameloop();
		_restartGame = false;
		if (!shouldQuit()) {
			_cutscene.play('I');
		}
	}

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
	_fadeDirection = FadeDirection::OUT;
	_fadeStepCounter = 0;
	_fadeTempPalette.loadFromScreen();
}

void DarkseedEngine::fadeIn(const Pal &palette) {
	_fadeDirection = FadeDirection::IN;
	_fadeStepCounter = 0;
	_fadeTargetPalette.load(palette);
	_fadeTempPalette.clear();
	_fadeTempPalette.installPalette();
}

bool DarkseedEngine::fadeStep() {
	if (_fadeStepCounter < 32) {
		_fadeTempPalette.updatePalette(_fadeDirection == FadeDirection::OUT ? -8 : 8, _fadeTargetPalette);
		_fadeStepCounter++;
	}
	return _fadeStepCounter < 32;
}

void DarkseedEngine::gameloop() {
	while (!shouldQuit() && !_restartGame) {
		updateEvents();
		if (_redrawFrame) {
			_redrawFrame = false;
			_screen->makeAllDirty();
			_frame.draw();
		}
		if (_fullscreenPic) {
			if (_isLeftMouseClicked || _isRightMouseClicked) {
				_isRightMouseClicked = false;
				_isLeftMouseClicked = false;
				removeFullscreenPic();
			}
		}
		counter_2c85_888b = (counter_2c85_888b + 1) & 0xff;
		if (_cutscene.isPlaying()) {
//			if (systemTimerCounter == 5) {
				_cutscene.update();
//			}
		} else if (systemTimerCounter == 5) {
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
			if (_room->_roomNumber == 57 && _previousRoomNumber == 54) {
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
					dcopanim();
					changeToRoom(59, true);
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
			if (_currentDay < 3 && _currentTimeInSeconds > 79199 && !_player->_isAutoWalkingToBed &&
				(_room->_roomNumber < 10 || _room->_roomNumber == 13 || _room->_roomNumber == 61 || _room->_roomNumber == 62)) {
				_player->_walkToSequence = false;
				_player->_actionToPerform = false;
				_player->_isAutoWalkingToBed = true;
				_player->setplayertowardsbedroom();
				_console->printTosText(944);
			}
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

			_screen->addDirtyRect({{0x45, 0x28}, 501, 200});

			if (!_cutscene.isPlaying()) {
				if (_fullscreenPic) {
					_fullscreenPic->draw(0x45, 0x28);
				} else {
					_room->draw();
					_inventory.draw();
					_sprites.drawSprites();
					_player->draw();
					_console->draw();
				}
			}

//			if (((*(int *)&_CursorX < 70) || (570 < *(int *)&_CursorX)) && (*(int *)&_DrawCursorNum < 90)) { TODO do we need this restriction?
//				zeromousebuttons();
//			}
			if (_player->_isAutoWalkingToBed ||
				(_room->_roomNumber == 10 && _player->_position.x < 369 &&
				  ((_currentDay == 3 && _currentTimeInSeconds > 43200) || _objectVar[88] != 0))) {
				zeromousebuttons();
			}
			if ((_objectVar[141] > 0 && _objectVar[141] < 4) ||
				_objectVar[141] == 9 ||
				  _player->_herowaiting || _objectVar[141] == 10 || _objectVar[141] == 8)  {
				zeromousebuttons();
			}
			if (_room->_roomNumber == 32 && _currentDay == 2 &&
				_currentTimeInSeconds > 64799 && _currentTimeInSeconds < 68401 &&
				  (_objectVar[141] == 5 || _objectVar[141] == 6 || _objectVar[141] == 4
					 || _objectVar[141] == 12) && !isPlayingAnimation_maybe) {
				zeromousebuttons();
			}
			updateHeadache();
			handleInput();
			_isRightMouseClicked = false;
			_isLeftMouseClicked = false;
		}
		_room->update();

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
				if (_isLeftMouseClicked && _cursor.getY() > 0x28 && !_player->_actionToPerform) { // prevLeftMouseButtonState == 0 &&
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
							_player->_actionToPerform = true;
						}
					}
				}
				int xDistToTarget = ABS(_player->_walkTarget.x - _player->_position.x);
				int yDistToTarget = ABS(_player->_walkTarget.y - _player->_position.y);

				if (_isRightMouseClicked && !_player->_actionToPerform) {
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

				if (_player->isAtWalkTarget() && _player->_heroMoving && !_player->_actionToPerform) {
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
				if (_player->isAtWalkTarget() && _player->_actionToPerform) {
					if (_player->_sequenceRotation != -1) {
						_player->changeDirection(_player->_direction, _player->_sequenceRotation);
						_player->updateSprite();
						_player->_sequenceRotation = -1;
						return;
					}
					_player->_heroMoving = false;
					_player->_actionToPerform = false;
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
							_player->setplayertowardsbedroom();
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
			_player->setplayertowardsbedroom();
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

	updateBaseSprites();

	if (roomNumber == 46 && _previousRoomNumber == 60 && _objectVar[57] == 1) {
		return;
	}
	if (roomNumber == 15 && _objectVar.getMoveObjectRoom(28) != 255) {
		setupOtherNspAnimation(1, 39);
	}
}

void DarkseedEngine::changeToRoom(int newRoomNumber, bool placeDirectly) { // AKA LoadNewRoom
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
		_cutscene.play('E');
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
	} else if (!placeDirectly && newRoomNumber != 0x22 && (newRoomNumber < 0x13 || newRoomNumber > 0x17)) {
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

	updateBaseSprites();

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
					const Sprite &playerSprite = _player->getSprite(26);
					_room->calculateScaledSpriteDimensions(playerSprite.width, playerSprite.height, _player->_position.y);
					_sprites.addSpriteToDrawList(
						_player->_position.x,
						_player->_position.y - scaledSpriteHeight,
						&playerSprite,
						240 - _player->_position.y,
						scaledSpriteWidth,
						scaledSpriteHeight, player_sprite_related_2c85_82f3);
				}
				if (otherNspAnimationType_maybe == 39 || otherNspAnimationType_maybe == 59 ||
					 otherNspAnimationType_maybe == 60 || otherNspAnimationType_maybe == 61) {
					const Sprite &playerSprite = _player->getSprite(26);
					_room->calculateScaledSpriteDimensions(playerSprite.width, playerSprite.height, _player->_position.y);
					_sprites.addSpriteToDrawList(
						_player->_position.x,
						_player->_position.y - scaledSpriteHeight,
						&playerSprite,
						240 - _player->_position.y,
						scaledSpriteWidth,
						scaledSpriteHeight, player_sprite_related_2c85_82f3);
					if (otherNspAnimationType_maybe == 60 || otherNspAnimationType_maybe == 61) {
						sprite_y_scaling_threshold_maybe = 240;
//						DrawObjectsMenu(); TODO do we need this logic?
//						UpdateAllVideo();
//						*(undefined2 *)&_LastTickCursorY = *(undefined2 *)&_CursorY;
						return;
					}
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

				if (_player->_animations.containsSpriteAt(_player->_frameIdx)) { // we need this check as the original logic calls the function below when it isn't initialised
					_room->calculateScaledSpriteDimensions(
						_player->_animations.getSpriteAt(_player->_frameIdx).width,
						_player->_animations.getSpriteAt(_player->_frameIdx).height,
						nsp_sprite_scaling_y_position != 0 ? nsp_sprite_scaling_y_position : _player->_position.y);
				}

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
					} else if (otherNspAnimationType_maybe == 62) { // sargent approaches jail cell.
						const Sprite &animSprite = _player->_animations.getSpriteAt(_player->_frameIdx);
						_sprites.addSpriteToDrawList(_player->_position.x - animSprite.width / 2, _player->_position.y - animSprite.height, &animSprite, 240 - _player->_position.y, animSprite.width, animSprite.height, player_sprite_related_2c85_82f3);
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
							const Sprite &animSprite = _player->_animations.getSpriteAt(_player->_frameIdx);
							_sprites.addSpriteToDrawList(373, 99, &animSprite, 240 - _player->_position.y, animSprite.width, animSprite.height, player_sprite_related_2c85_82f3);
							const Sprite &legsSprite = _player->_animations.getSpriteAt(12);
							_sprites.addSpriteToDrawList(373, 99 + animSprite.height, &legsSprite, 240 - _player->_position.y, legsSprite.width, legsSprite.height, player_sprite_related_2c85_82f3);
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
				} else if (otherNspAnimationType_maybe == 22) { // dig grave
					_sprites.addSpriteToDrawList(
						_player->_position.x - ((scaledWalkSpeed_maybe * 30) / 1000),
						_player->_position.y - scaledSpriteHeight,
						&_player->_animations.getSpriteAt(_player->_frameIdx),
						240 - _player->_position.y,
						scaledSpriteWidth,
						scaledSpriteHeight,
						player_sprite_related_2c85_82f3);
				} else if (otherNspAnimationType_maybe == 4 || otherNspAnimationType_maybe == 21) { // dig grave
					_sprites.addSpriteToDrawList(
						_player->_position.x - ((scaledWalkSpeed_maybe * 95) / 1000),
						_player->_position.y - scaledSpriteHeight,
						&_player->_animations.getSpriteAt(_player->_frameIdx),
						240 - _player->_position.y,
						scaledSpriteWidth,
						scaledSpriteHeight,
						player_sprite_related_2c85_82f3);
				} else if (otherNspAnimationType_maybe == 39) {
					int16 spriteX = 110;
					int16 spriteY = _player->_position.y;
					if (_room->_roomNumber == 10) {
						spriteX = 330;
						spriteY = 224;
					}
					const Sprite &sprite = _room->_locationSprites.getSpriteAt(_player->_frameIdx);
					_room->calculateScaledSpriteDimensions(
						sprite.width,
						sprite.height,
						spriteX);
					_sprites.addSpriteToDrawList(
						spriteX,
						spriteY - scaledSpriteHeight,
						&sprite,
						240 - _player->_position.y,
						scaledSpriteWidth,
						scaledSpriteHeight,
						player_sprite_related_2c85_82f3);
				} else if (otherNspAnimationType_maybe == 47) {
					const Sprite &sprite = _room->_locationSprites.getSpriteAt(_player->_frameIdx);
					_room->calculateScaledSpriteDimensions(
						sprite.width,
						sprite.height,
						_player->_position.y);
					_sprites.addSpriteToDrawList(
						_player->_position.x,
						_player->_position.y - scaledSpriteHeight,
						&sprite,
						240 - _player->_position.y,
						scaledSpriteWidth,
						scaledSpriteHeight,
						player_sprite_related_2c85_82f3);
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
	if (_player->_animations.getTotalAnim() > nspAnimIdx) {
		spriteAnimCountdownTimer[nspAnimIdx] = _player->_animations.getAnimAt(nspAnimIdx).frameDuration[0];
		_player->_frameIdx = _player->_animations.getAnimAt(nspAnimIdx).frameNo[0];
	} else {
		spriteAnimCountdownTimer[nspAnimIdx] = 0;
		_player->_frameIdx = 0;
	}
	isPlayingAnimation_maybe = true;
	otherNspAnimationType_maybe = animId;

	player_sprite_related_2c85_82f3 = 0;
	_ObjRestarted = false;

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
	case 19:
	case 23:
		_scaleSequence = true;
		_player->_frameIdx = 24;
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
	case 39:
		_scaleSequence = true;
		_player->_frameIdx = _room->_locationSprites.getAnimAt(nspAnimIdx).frameNo[0];
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
	case 47:
		_scaleSequence = true;
		_player->_frameIdx = _room->_locationSprites.getAnimAt(nspAnimIdx).frameNo[0];
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
		if (!_ObjRestarted) {
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
		if (!_ObjRestarted) {
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
		if (!_ObjRestarted) {
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
	case 4:
	case 21:
	case 22: // dig up grave
		advanceAnimationFrame(0);
		if (_player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]] % 5 == 3 &&
			_player->_animations.getAnimAt(0).frameDuration[animIndexTbl[0]] == spriteAnimCountdownTimer[0]) {
			playSound(14, 5, -1);
		}
		if (!_ObjRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]];
		} else {
			isPlayingAnimation_maybe = false;
			_objectVar[52] = 1;
			if (otherNspAnimationType_maybe == 21 && _objectVar[151] != 0) {
				if (_objectVar[87] == 0) {
					_objectVar[88] = 1;
					_objectVar[87] = 1;
					_console->printTosText(581);
					_inventory.addItem(29);
				} else {
					_console->printTosText(582);
				}
			} else {
				_objectVar[88] = 1;
				_console->printTosText(580);
			}
		}
		break;
	case 5: // goto sleep animation
		_player->_position.x = 135;
		_player->_position.y = 91;
		advanceAnimationFrame(1);
		if (!_ObjRestarted) {
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
		if (_FrameAdvanced && ((currentRoomNumber == 6 && animIndexTbl[1] == 1) || (currentRoomNumber == 5 && animIndexTbl[2] == 1))) {
//			FUN_1208_0dac_sound_related(0xd,CONCAT11(uVar4,5));
		}
		if (!_ObjRestarted) {
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
			if (_player->_isAutoWalkingToBed) {
				_player->setplayertowardsbedroom();
			}
		}
		break;
	case 7: // stairs down
		if (currentRoomNumber == 5) {
			advanceAnimationFrame(3);
			if (_FrameAdvanced && animIndexTbl[3] == 1) {
				// FUN_1208_0dac_sound_related(0xd,CONCAT11(extraout_AH_05,5));
			}
			if (!_ObjRestarted) {
				_player->_frameIdx = _player->_animations.getAnimAt(3).frameNo[_player->_animations.getAnimAt(3).frameNo[animIndexTbl[3]]];
			} else {
				_previousRoomNumber = 5;
				changeToRoom(6);
			}
		} else {
			advanceAnimationFrame(0);
			if (_FrameAdvanced && animIndexTbl[0] == 1) {
				// FUN_1208_0dac_sound_related(0xd,CONCAT11(extraout_AH_05,5));
			}
			if (!_ObjRestarted) {
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
		if (!_ObjRestarted) {
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
		if (!_ObjRestarted) {
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
		if (!_ObjRestarted) {
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
		if (!_ObjRestarted) {
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
		if (!_ObjRestarted) {
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
	case 16: // climb down rope
		advanceAnimationFrame(0);
		if (!_ObjRestarted) {
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
		if (!_ObjRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]];
		}
		if (animIndexTbl[0] == 5 && _FrameAdvanced) {
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
		if (!_ObjRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(0).frameNo[_player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]]];
		}
		if (_player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]] == 3 && _FrameAdvanced) {
			_console->printTosText(_objectVar[22] + 662);
			_objectVar[22] = _objectVar[22] + 1;
			if (_objectVar[22] == 3) {
				_room->loadRoom61AWalkableLocations();
			}
		}
		break;
	case 19: // pickup book from library
		_player->_frameIdx = 24;
		if (_ObjRestarted) {
			isPlayingAnimation_maybe = true;
			_objectVar[46] = 2;
			libanim(true);
		}
		break;
	case 20: // receive card from delbert
		advanceAnimationFrame(3);
		if (!_ObjRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(3).frameNo[animIndexTbl[3]];
		} else {
			_inventory.addItem(18);
		}
		break;
	case 23:
		_player->_frameIdx = 24;
		if (_ObjRestarted != 0) {
			isPlayingAnimation_maybe = false;
			_objectVar[99] = 1;
			_console->printTosText(468);
		}
		break;
	case 10:
		advanceAnimationFrame(0);
		if (!_ObjRestarted) {
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
		if (!_ObjRestarted) {
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
	case 26: // climb up rope.
		advanceAnimationFrame(1);
		if (!_ObjRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(1).frameNo[animIndexTbl[1]];
		} else {
			_previousRoomNumber = _room->_roomNumber;
			changeToRoom(13);
		}
		break;
	case 27:
		advanceAnimationFrame(0);
		if (!_ObjRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(0).frameNo[_player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]]]; // TODO check if this is correct.
		} else {
			_previousRoomNumber = _room->_roomNumber;
			changeToRoom(38);
		}
		break;
	case 28: // step out of mirror normal world
		advanceAnimationFrame(1);
		if (!_ObjRestarted) {
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
		if (_ObjRestarted && otherNspAnimationType_maybe == 30) {
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
		if (_ObjRestarted && (otherNspAnimationType_maybe == 32 || otherNspAnimationType_maybe == 33)) {
			setupOtherNspAnimation(otherNspAnimationType_maybe - 29, otherNspAnimationType_maybe + 1);
		}
		break;
	}
	case 35: // pay shopkeeper
		advanceAnimationFrame(6);
		if (!_ObjRestarted) {
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
	case 36:
	case 37: // smash mirror
		advanceAnimationFrame(otherNspAnimationType_maybe - 36);
		_player->_frameIdx = _player->_animations.getAnimAt(otherNspAnimationType_maybe - 36).frameNo[animIndexTbl[otherNspAnimationType_maybe - 36]];
		if (_FrameAdvanced && otherNspAnimationType_maybe == 36 && _player->_frameIdx == 4) {
			playSound(28, 5, -1);
		}
		if (_ObjRestarted) {
			if (otherNspAnimationType_maybe == 36) {
				setupOtherNspAnimation(1, 37);
			} else {
				_objectVar[137] = 2;
				_player->_isAutoWalkingToBed = true;
				_player->setplayertowardsbedroom();
			}
		}
		break;
	case 38:
		advanceAnimationFrame(0);
		if (!_ObjRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]];
		} else {
			stuffPlayer();
		}
		break;
	case 39: // Arrest Mike.
		_room->advanceFrame(1);
		if (!_ObjRestarted) {
			_player->_frameIdx = _room->_locationSprites.getAnimAt(1).frameNo[_room->_locObjFrame[1]];
		}
		else {
			throwmikeinjail();
		}
		break;
	case 40: // give Delbert's card to cop.
		advanceAnimationFrame(1);
		if (!_ObjRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(1).frameNo[animIndexTbl[1]];
		} else {
			_console->printTosText(61);
			_console->draw();
			_screen->updateScreen();
			waitxticks(60);
			_previousRoomNumber = _room->_roomNumber;
			_player->_position.x = 240;
			_player->_position.y = 200;
			_player->updateSprite();
			isPlayingAnimation_maybe = false;
			changeToRoom(15, true);
			_inventory.removeItem(41);
			_inventory.removeItem(18);
			_objectVar.setMoveObjectRoom(41, 255);
		}
		break;
	case 41:
		advanceAnimationFrame(0);
		if (!_ObjRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]];
		} else {
			_previousRoomNumber = _room->_roomNumber;
			changeToRoom(7);
		}
		break;
	case 42: // step out of mirror giger world
		advanceAnimationFrame(1);
		if (!_ObjRestarted) {
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
		if (!_ObjRestarted || (otherNspAnimationType_maybe != 46 && otherNspAnimationType_maybe != 44)) {
			if (otherNspAnimationType_maybe == 45) {
				_objectVar[117] = 1;
			} else if (_ObjRestarted) {
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
		_room->mikeStickThrowAnim();
		break;
	case 48:
	case 49:
	case 50:
	case 51:
	case 52:
		// open/close cupboards, oven
		advanceAnimationFrame(0);
		if (!_ObjRestarted) {
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
		if (!_ObjRestarted) {
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
		if (_FrameAdvanced && animIndexTbl[0] == 1) {
			// FUN_1208_0dac_sound_related(0xd,CONCAT11(extraout_AH_05,5));
			// PlaySound(1,5,-1);
		}
		break;
	case 57:
	case 58:
		advanceAnimationFrame((otherNspAnimationType_maybe - 57) & 1);
		if (!_ObjRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]]; //_player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]]];
		} else if (otherNspAnimationType_maybe == 57) {
			_previousRoomNumber = _room->_roomNumber;
			changeToRoom(_room->_roomNumber == 41 ? 44 : 41);
		} else {
			_player->_direction = 2;
		}
		break;
	case 59: // use shard on mirror
		advanceAnimationFrame(0);
		if (!_ObjRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]];
		} else {
			_player->_direction = 1;
			_player->updateSprite();
		}
		if (_FrameAdvanced && _player->_frameIdx == 3) {
			playSound(26, 5, -1);
		}
		break;
	case 60: // turn on alien computer
		_room->advanceFrame(0);
		if (!_ObjRestarted) {
			_player->_frameIdx = _room->_locationSprites.getAnimAt(0).frameNo[_room->_locObjFrame[0]];
		} else {
			isPlayingAnimation_maybe = false;
			_objectVar[187] = 1;
		}
		break;
	case 61: // turn off alien computer
		_room->advanceFrame(1);
		if (!_ObjRestarted) {
			_player->_frameIdx = _room->_locationSprites.getAnimAt(1).frameNo[_room->_locObjFrame[1]];
		} else {
			isPlayingAnimation_maybe = false;
		}
		_objectVar[187] = 0;
		break;
	case 62: // bang cup against bars
		advanceAnimationFrame(0);
		if (!_ObjRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]];
		} else {
			bool bVar5 = _objectVar.getMoveObjectRoom(18) == 250;
			if (!bVar5) {
				if (_inventory.hasObject(18)) {
					bVar5 = true;
				}
			}
			if (bVar5) {
				isPlayingAnimation_maybe = false;
				_objectVar.setObjectRunningCode(53, 1);
				_player->loadAnimations("copcard.nsp");
				animIndexTbl[0] = 0;
				spriteAnimCountdownTimer[0] = 3;
				_objectVar[1] = 2000;
			}
			else {
				_console->addTextLine("The cops ignore your demands for attention.");
			}
		}
		break;
	case 65:
		advanceAnimationFrame(0);
		if (!_ObjRestarted) {
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
	_ObjRestarted = false;
	_FrameAdvanced = false;
	const Obt &anim = _player->_animations.getAnimAt(nspAminIdx);
	spriteAnimCountdownTimer[nspAminIdx] = spriteAnimCountdownTimer[nspAminIdx] - 1;
	if (spriteAnimCountdownTimer[nspAminIdx] < 1) {
		_FrameAdvanced = true;
		animIndexTbl[nspAminIdx] = animIndexTbl[nspAminIdx] + 1;
		_player->_position.x += ((int16)anim.deltaX[animIndexTbl[nspAminIdx]] * scaledWalkSpeed_maybe) / 1000;
		_player->_position.y += ((int16)anim.deltaY[animIndexTbl[nspAminIdx]] * scaledWalkSpeed_maybe) / 1000;
		if (animIndexTbl[nspAminIdx] == anim.numFrames) {
			animIndexTbl[nspAminIdx] = 0;
			_ObjRestarted = true;
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
		if (_cursor.getY() < 40 && _actionMode > LookAction) {
			if (_objectVar[80] < 2 ||
				((_actionMode != 25 || targetObjNum != 19) && (_actionMode != 19 || targetObjNum != 25))) {
				if ((_actionMode == 25 && targetObjNum == 20) || (_actionMode == 20 && targetObjNum == 25)) {
					if (_objectVar[25] > 99) { // Assemble special hammer
						_console->printTosText(969);
						_inventory.removeItem(20);
						_inventory.removeItem(25);
						updateDisplay();
						playSound(17,5,-1);
						_inventory.addItem(27);
//						waitxticks(30); TODO do we need to add delay here?
						return;
					}
					_useCode->genericresponse(_actionMode, targetObjNum, 999);
				} else {
					_useCode->genericresponse(_actionMode, targetObjNum, 999);
				}
			} else {
				_console->printTosText(695);
			}
			return;
		} else {
			switch (_actionMode) {
			case HandAction:
				_useCode->useCode(targetObjNum);
				break;
			case LookAction:
				lookCode(targetObjNum);
				break;
			case 5:
				_useCode->useCrowBar(targetObjNum);
				break;
			case 6:
			case 12:
			case 29:
				_useCode->useCodeJournal(_actionMode, targetObjNum);
				break;
			case 7:
			case 36:
			case 37:
			case 38:
			case 39:
			case 40:
				_useCode->useCodeShopItems(_actionMode, targetObjNum);
				break;
			case 8:
				_useCode->useCodeMoney(targetObjNum);
				break;
			case 9:
				_useCode->useCodeNewspaper(targetObjNum);
				break;
			case 10:
				_useCode->useCodeLibraryCard(targetObjNum);
				break;
			case 11:
				_useCode->useCodeBobbyPin(targetObjNum);
				break;
			case 13:
			case 26:
				_useCode->useCodeKeys(_actionMode, targetObjNum);
				break;
			case 14:
				_useCode->useCodeGloves(targetObjNum);
				break;
			case 15:
				_useCode->useCodeMirrorShard(targetObjNum);
				break;
			case 16:
				_useCode->useCodeBinoculars(targetObjNum);
				break;
			case 17:
				_useCode->useCodeShovel(targetObjNum);
				break;
			case 18:
				_useCode->useCodeDelbertsCard(targetObjNum);
				break;
			case 19:
				_useCode->useCodeStick(targetObjNum);
				break;
			case 20:
				_useCode->useCodeAxeHandle(targetObjNum);
				break;
			case 23:
				_useCode->useCodeRope(targetObjNum);
				break;
			case 24:
				_useCode->useCodeMicroFilm(targetObjNum);
				break;
			case 25:
			case 27:
				_useCode->useCodeSpecialHammer(_actionMode, targetObjNum);
				break;
			case 28:
				_useCode->useCodeGun(targetObjNum);
				break;
			case 30:
				_useCode->useCodeMoversNote(targetObjNum);
				break;
			case 34:
				_useCode->useCodeBluePrints(targetObjNum);
				break;
			case 35:
				_useCode->useCodeWatch(targetObjNum);
				break;
			case 41:
				_useCode->useCodeTinCup(targetObjNum);
				break;
			case 85: // this code appears to be wrong and unused.
				_useCode->useCodeEmptyUrn(targetObjNum);
				break;
			default:
				break;
			}
		}
	} else {
		_player->loadAnimations("pownex.nsp");
		animIndexTbl[0] = 0;
		spriteAnimCountdownTimer[0] = _player->_animations.getAnimAt(0).frameDuration[0];
		_ObjRestarted = false;
		int16 frameIdx = 0;
		int16 prevFrameIdx = 0;
		while(!_ObjRestarted) {
			prevFrameIdx = frameIdx;
			for (int i = 0; i < 6; i++) {
				wait();
			}

			_sprites.clearSpriteDrawList();
			_frame.draw();
			_console->draw();

			advanceAnimationFrame(0);
			frameIdx = _player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]];
			const Sprite &sprite = _player->_animations.getSpriteAt(frameIdx);
			_sprites.addSpriteToDrawList(152, 89, &sprite, 255, sprite.width, sprite.height, false);
			_room->draw();
			_sprites.drawSprites();

			_screen->makeAllDirty();
			_screen->update();

		}

		_ObjRestarted = false;
		while(!_ObjRestarted) {
			for (int i = 0; i < 6; i++) {
				wait();
			}
			_sprites.clearSpriteDrawList();
			_frame.draw();
			_console->draw();

			advanceAnimationFrame(1);

			const Sprite &sprite = _player->_animations.getSpriteAt(_player->_animations.getAnimAt(1).frameNo[animIndexTbl[1]]);
			_sprites.addSpriteToDrawList(152, 129, &sprite, 255, sprite.width, sprite.height, false);

			const Sprite &mikeSprite = _player->_animations.getSpriteAt(prevFrameIdx);
			_sprites.addSpriteToDrawList(152, 89, &mikeSprite, 255, mikeSprite.width, mikeSprite.height, false);

			_room->draw();
			_sprites.drawSprites();

			_screen->makeAllDirty();
			_screen->update();
		}

		if (_actionMode == 25) {
			if (_objectVar[25] < 100) {
				_objectVar[25] = _objectVar[25] + 100;
			}
			_console->printTosText(930);
		} else if (_actionMode == 27) {
			_console->printTosText(929);
		}
		else {
			_console->printTosText(967);
			_console->addToCurrentLine(_objectVar.getObjectName(_actionMode)); // TODO remove newline after object name
			_console->printTosText(968);
			_inventory.removeItem(_actionMode);
		}
		if (_actionMode > 4) {
			_cursor.setCursorType(Pointer);
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
		if (eyeTosIdx > 978) {
			_useCode->genericresponse(3, objNum, eyeTosIdx);
		}
		return;
	}
	_console->addTextLine(Common::String::format("You see the %s.", _objectVar.getObjectName(objNum)));
}

void DarkseedEngine::wongame() {
	error("implement wongame()"); // TODO
	//	_cutscene.play('Z');
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
	pal.load(g_engine->getPictureFilePath(palFilename));
	_fullscreenPic->draw(0x45, 0x28);
	_screen->addDirtyRect({{0x45, 0x28}, 501, 200});
}

void DarkseedEngine::getPackageObj(int packageType) {
	_console->printTosText(424);
	if (packageType == 1) {
		_cutscene.play('D');
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
	_ObjRestarted = false;
	spriteAnimCountdownTimer[nspAminIdx]--;
	_FrameAdvanced = false;
	if (spriteAnimCountdownTimer[nspAminIdx] < 1) {
		const Obt &anim = _player->_animations.getAnimAt(nspAminIdx);
		animIndexTbl[nspAminIdx]++;
		_FrameAdvanced = true;
		if (animIndexTbl[nspAminIdx] == anim.numFrames) {
			animIndexTbl[nspAminIdx] = 0;
			_ObjRestarted = true;
		}
		spriteAnimCountdownTimer[nspAminIdx] = anim.frameDuration[animIndexTbl[nspAminIdx]];
	}
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
		_cutscene.play('Y');
	} else if (_currentDay == 2) {
		_cutscene.play('B');
	} else if (_currentDay == 3) {
		_cutscene.play('C');
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
		_player->_actionToPerform = false;
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
		if (_ObjRestarted) {
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
			if (_ObjRestarted && _objectVar[45] < 2) {
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
		_ObjRestarted = false;
		if (!_sound->isPlayingSpeech() && (_objectVar[141] != 2 || _delbertspeech > 64)) {
			nextFrame(_objectVar[141] - 1);
		}
		if (_ObjRestarted) {
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
			_ObjRestarted = false;
		}
		if (!_sound->isPlayingSpeech() && _objectVar[141] == 2) {
			if (_delbertspeech < 65) {
				_delbertspeech++;
			}
			if (_delbertspeech == 64) {
				_console->printTosText(908);
				_sound->waitForSpeech();
			}
			else if (_delbertspeech == 65) {
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
		if (_ObjRestarted) {
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
			if (_ObjRestarted) {
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
			setupOtherNspAnimation(1, 39); // arrest mike.
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
	error("implement rundrekethsequence()"); // TODO
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

static constexpr uint8 libList[100] = {
	5, 6, 7, 8,
	9, 10, 9, 8,
	7, 6, 5, 6,
	7, 6, 7, 8,
	7, 6, 5, 6,
	5, 6, 7, 8,
	9, 10, 9, 8,
	7, 6, 5, 6,
	7, 6, 7, 8,
	7, 6, 5, 6,
	5, 6, 7, 8,
	9, 10, 9, 8,
	7, 6, 5, 6,
	5, 6, 7, 6,
	7, 8, 7, 6,
	5, 6, 7, 6,
	7, 8, 7, 6,
	5, 6, 5, 6,
	5, 6, 7, 6,
	7, 8, 7, 6,
	5, 6, 7, 8,
	9, 10, 9, 8,
	7, 6, 5, 6,
	7, 8, 9, 10,
	9, 8, 7, 6
};

void DarkseedEngine::libanim(bool pickingUpReservedBook) {
	_player->loadAnimations("libparts.nsp");
	showFullscreenPic("libinlib.pic");

	_console->printTosText(pickingUpReservedBook ? 928 : 924);

	spriteAnimCountdownTimer[0] = _player->_animations.getAnimAt(0).frameDuration[0];
	uint8 lipsIdx = 0;
	while (_sound->isPlayingSpeech()) {
		_sprites.clearSpriteDrawList();
		_frame.draw();
		if (_fullscreenPic) {
			_fullscreenPic->draw(0x45, 0x28);
		}
		_console->draw();

		advanceAnimationFrame(0);
		const Sprite &eyesSprite = _player->_animations.getSpriteAt( _player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]]);
		g_engine->_sprites.addSpriteToDrawList(255, 114, &eyesSprite, 255, eyesSprite.width, eyesSprite.height, false);
		advanceAnimationFrame(1);

		const Sprite &mouthSprite = _player->_animations.getSpriteAt(libList[lipsIdx]);
		g_engine->_sprites.addSpriteToDrawList(255, 154, &mouthSprite, 255, mouthSprite.width, mouthSprite.height, false);

		_sprites.drawSprites();

		_screen->makeAllDirty();
		_screen->update();

		lipsIdx++;
		if (lipsIdx == 100) {
			lipsIdx = 0;
		}

		for (int i = 0; i < 6; i++) {
			wait();
		}
	}

	removeFullscreenPic();

	if (pickingUpReservedBook) {
		_objectVar[49] = 1;
		_objectVar[62] = 0;
		_cutscene.play('G');
	}
}

static constexpr uint8 dcopList[100] = {
	0, 1, 2, 3,
	2, 1, 2, 3,
	2, 1, 0, 1,
	2, 3, 2, 3,
	2, 3, 2, 1,
	0, 1, 2, 1,
	2, 1, 2, 3,
	2, 1, 0, 1,
	2, 3, 2, 1,
	2, 3, 2, 1,
	0, 1, 2, 3,
	2, 3, 2, 3,
	2, 1, 0, 1,
	2, 1, 2, 1,
	2, 3, 2, 1,
	0, 1, 2, 1,
	2, 1, 2, 3,
	2, 1, 0, 1,
	2, 3, 2, 3,
	2, 3, 2, 1,
	0, 1, 2, 1,
	2, 1, 2, 3,
	2, 1, 0, 1,
	2, 1, 2, 1,
	2, 3, 2, 1};

void DarkseedEngine::dcopanim() {
	_player->loadAnimations("dcopb.nsp");
	showFullscreenPic("dcopb.pic");
	animIndexTbl[0] = 0;
	spriteAnimCountdownTimer[0] = _player->_animations.getAnimAt(0).frameDuration[0];

	_sprites.clearSpriteDrawList();
	_console->printTosText(923);

	uint8 lipsIdx = 0;
	while (_sound->isPlayingSpeech()) {
		_sprites.clearSpriteDrawList();

		if (_fullscreenPic) {
			_fullscreenPic->draw(0x45, 0x28);
		}
		advanceAnimationFrame(0);
		const Sprite &dcopSprite = _player->_animations.getSpriteAt(dcopList[lipsIdx]);
		g_engine->_sprites.addSpriteToDrawList(310, 180, &dcopSprite, 255, dcopSprite.width, dcopSprite.height, false);
		_sprites.drawSprites();

		_console->draw();
		_screen->makeAllDirty();
		_screen->update();

		lipsIdx++;
		if (lipsIdx == 100) {
			lipsIdx = 0;
		}

		for (int i = 0; i < 6; i++) {
			wait();
		}
	}
	removeFullscreenPic();
}

static constexpr uint8 keeperList[250] = {
	10, 11, 12, 13,
	12, 12, 13, 10,
	11, 10, 10, 11,
	12, 13, 12, 12,
	13, 10, 11, 10,
	10, 10, 11, 11,
	12, 12, 11, 12,
	12, 13, 12, 12,
	12, 13, 13, 12,
	13, 12, 11, 12,
	13, 12, 11, 10,
	11, 12, 13, 10,
	11, 10, 10, 11,
	12, 13, 12, 12,
	13, 10, 11, 10,
	0, 0, 1, 1,
	2, 2, 3, 3,
	4, 4, 5, 5,
	6, 6, 7, 7,
	8, 8, 9, 9,
	10, 10, 11, 11,
	12, 12, 11, 12,
	12, 13, 12, 12,
	12, 13, 13, 12,
	13, 12, 11, 10,
	10, 11, 12, 13,
	12, 12, 13, 10,
	11, 10, 10, 11,
	12, 13, 12, 12,
	13, 10, 11, 10,
	10, 11, 12, 13,
	12, 12, 13, 10,
	11, 10, 10, 11,
	12, 13, 12, 12,
	13, 10, 11, 10,
	10, 11, 12, 13,
	12, 12, 13, 10,
	11, 10, 10, 10,
	11, 11, 12, 12,
	11, 12, 12, 13,
	12, 12, 12, 13,
	13, 12, 13, 12,
	11, 10, 10, 11,
	12, 13, 12, 12,
	13, 10, 11, 10,
	10, 11, 12, 13,
	12, 12, 13, 10,
	11, 10, 10, 11,
	12, 13, 12, 12,
	13, 10, 11, 10,
	10, 10, 11, 11,
	12, 12, 11, 12,
	12, 13, 12, 12,
	12, 13, 13, 12,
	13, 12, 11, 10,
	0, 1, 2, 3,
	4, 5, 6, 7,
	8, 9, 10, 11,
	12, 13, 12, 12,
	13, 10, 11, 10,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0};

void DarkseedEngine::keeperanim() {
	_cursor.showCursor(false);
	_player->loadAnimations("keeper.nsp");
	showFullscreenPic("keeper.pic");
	animIndexTbl[0] = 0;
	spriteAnimCountdownTimer[0] = _player->_animations.getAnimAt(0).frameDuration[0];

	_console->printTosText(913);

	uint8 dialogIdx = 73;
	uint8 lipsIdx = 0;
	while (_sound->isPlayingSpeech() || dialogIdx < 76) {
		_sprites.clearSpriteDrawList();

		if (_fullscreenPic) {
			_fullscreenPic->draw(0x45, 0x28);
		}
		advanceAnimationFrame(0);
		const Sprite &keeperSprite = _player->_animations.getSpriteAt(keeperList[lipsIdx]);
		g_engine->_sprites.addSpriteToDrawList(254, 117, &keeperSprite, 255, keeperSprite.width, keeperSprite.height, false);
		_sprites.drawSprites();

		_console->draw();
		_screen->makeAllDirty();
		_screen->update();

		lipsIdx++;
		if (lipsIdx == 250) {
			lipsIdx = 0;
		}

		if (!_sound->isPlayingSpeech()) {
			dialogIdx++;
			if (dialogIdx == 74) {
				_console->printTosText(914);
			} else if (dialogIdx == 75) {
				_console->printTosText(915);
			}
		}
		waitxticks(1);
	}
	removeFullscreenPic();
	_cursor.showCursor(true);
}

void DarkseedEngine::stuffPlayer() {
	_cursor.showCursor(false);
	_player->loadAnimations("labparts.nsp");
	showFullscreenPic("lab.pic");
	const Sprite &alienSprite = _player->_animations.getSpriteAt(8);

	bool updateCounter = false;
	int counter = 0;
	while (counter < 8) {
		_sprites.clearSpriteDrawList();

		if (_fullscreenPic) {
			_fullscreenPic->draw(0x45, 0x28);
		}
		const Sprite &mikeSprite = _player->_animations.getSpriteAt(counter);
		g_engine->_sprites.addSpriteToDrawList(103, 93, &mikeSprite, 255, mikeSprite.width, mikeSprite.height, false);
		g_engine->_sprites.addSpriteToDrawList(226, 100, &alienSprite, 255, alienSprite.width, alienSprite.height, false);

		_sprites.drawSprites();

		_console->draw();
		_screen->makeAllDirty();
		_screen->update();

		updateCounter = !updateCounter;
		if (updateCounter) {
			counter++;
		}
		waitxticks(1);
	}
	waitxticks(3);
	removeFullscreenPic();
	_sprites.clearSpriteDrawList();
	_cursor.showCursor(true);
	_cutscene.play('Z');
}

static constexpr uint8 sargoList[100] = {
	0, 1, 2, 3,
	4, 3, 2, 1,
	0, 1, 0, 1,
	2, 3, 2, 3,
	2, 3, 2, 1,
	0, 1, 2, 3,
	4, 3, 4, 3,
	2, 1, 0, 1,
	2, 3, 4, 3,
	2, 1, 0, 1,
	0, 1, 2, 3,
	2, 3, 2, 3,
	2, 1, 0, 1,
	2, 3, 2, 1,
	0, 4, 3, 2,
	1, 1, 2, 3,
	4, 3, 2, 1,
	0, 1, 0, 1,
	2, 3, 2, 3,
	2, 3, 2, 1,
	2, 1, 0, 0,
	1, 2, 3, 2,
	1, 0, 1, 2,
	3, 4, 3, 2,
	3, 2, 1, 1};

void DarkseedEngine::sargoanim() {
	_cursor.showCursor(false);
	_player->loadAnimations("sargo.nsp");
	showFullscreenPic("sargo.pic");
	animIndexTbl[0] = 0;
	spriteAnimCountdownTimer[0] = _player->_animations.getAnimAt(0).frameDuration[0];

	_console->printTosText(916);

	uint8 dialogIdx = 79;
	uint8 lipsIdx = 0;
	while (_sound->isPlayingSpeech() || dialogIdx < 81) {
		_sprites.clearSpriteDrawList();

		if (_fullscreenPic) {
			_fullscreenPic->draw(0x45, 0x28);
		}
		advanceAnimationFrame(0);
		const Sprite &sargoSprite = _player->_animations.getSpriteAt(sargoList[lipsIdx]);
		g_engine->_sprites.addSpriteToDrawList(334, 160, &sargoSprite, 255, sargoSprite.width, sargoSprite.height, false);
		_sprites.drawSprites();

		_console->draw();
		_screen->makeAllDirty();
		_screen->update();

		lipsIdx++;
		if (lipsIdx == 100) {
			lipsIdx = 0;
		}

		if (!_sound->isPlayingSpeech()) {
			dialogIdx++;
			if (dialogIdx == 80) {
				_console->printTosText(917);
			}
		}
		waitxticks(1);
	}
	removeFullscreenPic();
	_cursor.showCursor(true);
}

void DarkseedEngine::removeFullscreenPic() {
	if (_fullscreenPic) {
		delete _fullscreenPic;
		_fullscreenPic = nullptr;
		_room->restorePalette();
	}
}

void DarkseedEngine::zeromousebuttons() {
	_isLeftMouseClicked = false;
	_isRightMouseClicked = false;
}

void DarkseedEngine::updateBaseSprites() {
	if (!_room->isGiger() == _normalWorldSpritesLoaded) {
		// no need to change graphics
		return;
	}

	if (_room->isGiger()) {
		_baseSprites.load("gbase.nsp");
		_frame.load("gframe.pic");
		_normalWorldSpritesLoaded = false;
	} else {
		_baseSprites.load("cbase.nsp");
		_frame.load("cframe.pic");
		_normalWorldSpritesLoaded = true;
	}
	_redrawFrame = true;
}

void DarkseedEngine::waitxticks(int ticks) {
	for (int i = 0; i < ticks * 6; i++) {
		updateEvents();
		_room->update();
		_screen->update();
		wait();
	}
}
void DarkseedEngine::restartGame() {
	_restartGame = true;
}

void DarkseedEngine::newGame() {
	_redrawFrame = true;
	_cursor.showCursor(true);
	_sprites.clearSpriteDrawList();
	removeFullscreenPic();
	_inventory.reset();
	_sound->resetSpeech();
	_objectVar.reset();
	_room->_roomNumber = 0;
	changeToRoom(0);
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

} // End of namespace Darkseed
