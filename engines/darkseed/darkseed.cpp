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
}

uint32 DarkseedEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String DarkseedEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error DarkseedEngine::run() {
	initGraphics(640, 350);
	_screen = new Graphics::Screen();
	_tosText = new TosText();
	_tosText->load();
	_console = new Console(_tosText);
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

	// Set the engine's debugger console
	setDebugger(new DebugConsole(_tosText));

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	// Draw a series of boxes on screen as a sample
//	for (int i = 0; i < 100; ++i)
//		_screen->frameRect(Common::Rect(i, i, 320 - i, 200 - i), i);
//	Pal::load("art/ship.pal");
//	_screen->copyRectToSurface(left00Img.getPixels().data(), left00Img.getWidth(), left00Img.getX(), left00Img.getY(), left00Img.getWidth(), left00Img.getHeight());
//	_screen->copyRectToSurface(left01Img.getPixels().data(), left01Img.getWidth(), left01Img.getX(), left01Img.getY(), left01Img.getWidth(), left01Img.getHeight());

	Pal housePalette;
	housePalette.load("art/house.pal");
	TitleFont titleFont;
	titleFont.displayString(0x44, 0xa0, "DEVELOPING NEW WAYS TO AMAZE");
	Img house;
//	house.load("art/bdoll0.img");
//		_screen->copyRectToSurface(house.getPixels().data(), house.getWidth(), house.getX(), house.getY(), house.getWidth(), house.getHeight());
//	_screen->copyRectToSurfaceWithKey(letterD.getPixels().data(), letterD.getWidth(), 24, 24, letterD.getWidth(), letterD.getHeight(), 0);
//	_screen->copyRectToSurfaceWithKey(letterD1.getPixels().data(), letterD1.getWidth(), 24+1, 24, letterD1.getWidth(), letterD1.getHeight(), 0);

	_frame.load("cframe.pic");
	_frame.draw();

	_baseSprites.load("cbase.nsp");

	_cursor.updatePosition(0x140,0xaf);
	_cursor.setCursorType(Pointer);

	_player->_position.x = 0x87;
	_player->_position.y = 0x5b;
	_player->_direction = 1;
	_player->_frameIdx = 0;

//	Pic room;
//	room.load("bed1a.pic");
//	_screen->copyRectToSurface(room.getPixels().data(), room.getWidth(), 0x45, 0x28, room.getWidth(), room.getHeight());

//	Pal roomPal;
//	roomPal.load("room0.pal");

//	Nsp playerNsp;
//	playerNsp.load("bedsleep.nsp"); //"cplayer.nsp");
//	const Sprite &s = playerNsp.getSpriteAt(11);
//
//	_screen->copyRectToSurfaceWithKey(s.pixels.data(), s.width, 0x45 + 220, 0x28 + 40, s.width, s.height, 0xf);

	_room = new Room(0);

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

	gameloop();

	delete _room;
	delete _player;

	return Common::kNoError;
}

Common::Error DarkseedEngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

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
		counter_2c85_888b = (counter_2c85_888b + 1) & 0xff;
		if (systemTimerCounter == 5) {
			handleInput();
			updateDisplay();
			_isRightMouseClicked = false;
			_isLeftMouseClicked = false;
		}
		_room->update();
		_frame.draw();
		_room->draw();
		_sprites.drawSprites();
		_console->draw();
		_cursor.draw();
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
					}
					_cursor.setCursorType((CursorType)_actionMode);
				}
				if (_player->isAtWalkTarget() && _player->_heroMoving && _player->walkPathIndex != -1) {
					_player->walkToNextConnector();
				}
				if (_isLeftMouseClicked && _cursor.getY() < 0x29) {
					// TODO handle inventory click.
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
								// 2022:808a TODO
//								if (currentRoomNumber == 0x3b)
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
				}
				if (!isPlayingAnimation_maybe) {
					// walk to destination point
					Common::Point origPlayerPosition = _player->_position;
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
							if (_player->_position.y - _player->_walkTarget.y <= walkYDelta) {
								local_4 = _player->_position.y - _player->_walkTarget.y;
							} else {
								local_4 = walkYDelta;
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
							if (_player->_walkTarget.y - _player->_position.y <= walkYDelta) {
								local_4 = _player->_walkTarget.y - _player->_position.y;
							} else {
								local_4 = walkYDelta;
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
							_player->_position.x -= walkXDelta;
						} else if (_player->_position.x < _player->_walkTarget.x) {
							if (_player->_walkTarget.x - _player->_position.x < walkXDelta) {
								walkXDelta = _player->_walkTarget.x - _player->_position.x;
							}
							_player->_position.x += walkXDelta;
						}
						if (_player->_walkTarget.y < _player->_position.y) {
							if (_player->_position.y - _player->_walkTarget.y < walkYDelta) {
								walkYDelta = _player->_position.y - _player->_walkTarget.y;
							}
							_player->_position.y -= walkYDelta;
						} else if (_player->_position.y < _player->_walkTarget.y) {
							if (_player->_walkTarget.y - _player->_position.y < walkYDelta) {
								walkYDelta = _player->_walkTarget.y - _player->_position.y;
							}
							_player->_position.y += walkYDelta;
						}
						if (!_room->canWalkAtLocation(_player->_position.x, _player->_position.y)) {
							_player->_position = origPlayerPosition;
							_player->_walkTarget = origPlayerPosition;
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

void DarkseedEngine::changeToRoom(int newRoomNumber) {
	delete _room;
	_room = new Room(newRoomNumber);
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
	} // TODO a bunch of other room codes here.
	else if (newRoomNumber != 0x22 && (newRoomNumber < 0x13 || newRoomNumber > 0x17)) {
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
		if (_previousRoomNumber == 10 && newRoomNumber == 6) {
//			if (DAT_2c85_81a0 == 0 && _currentDay == 1) {
//				DAT_2c85_6750 = 2;
//			}
		}
	}

	_room->printRoomDescriptionText();
}

void DarkseedEngine::updateDisplay() {
	int currentRoomNumber = _room->_roomNumber;
	_sprites.clearSpriteDrawList();
//	FUN_2022_2ca7();
//	FUN_2022_413a();
	if (isPlayingAnimation_maybe == 0 ||
		(otherNspAnimationType_maybe != 6 && otherNspAnimationType_maybe != 7) || currentRoomNumber != 5) {
		DAT_2c85_8326_blit_related = 0xf0;
	}
	else {
		DAT_2c85_8326_blit_related = 0xd0;
	}

	if (currentRoomNumber != 0x22 && currentRoomNumber != 0x13 &&
		  currentRoomNumber != 0x14 && currentRoomNumber != 0x15 && currentRoomNumber != 0x16 &&
		 DAT_2c85_825c != 9 && (currentRoomNumber != 0x35 || DAT_2c85_819c != 2)) {
		if (((otherNspAnimationType_maybe == 0x3c) || (otherNspAnimationType_maybe == 0x3d)) &&
			isPlayingAnimation_maybe) {
//			uVar7 = *(undefined2 *)((int)nspWidthTbl + _player->_frameIdx * 2);
//			uVar1 = *(uint *)((int)nspHeightTbl + _player->_frameIdx * 2);
//			addSpriteToDraw(0x1af,0x42,uVar7,uVar1,*(undefined2 *)((int)nspPixelPtrTbl + _player->_frameIdx * 4),
//							*(undefined2 *)((int)&nspPixelPtrTbl[0].Offset + _player->_frameIdx * 4),0xff,uVar7,uVar1,
//							uVar1 & 0xff00);
		}
		if (!isWearingHeadBand || (headAcheMessageCounter & 1) != 0) {
			if (!isPlayingAnimation_maybe || otherNspAnimationType_maybe == 0x13
				|| otherNspAnimationType_maybe == 0x17) {
				const Sprite &playerSprite = _player->getSprite(_player->_frameIdx);
				_room->calculateScaledSpriteDimensions(playerSprite.width, playerSprite.height, _player->_position.y);
				if ((currentRoomNumber != 0x35) || (DAT_2c85_81e0 != 2)) {
					_sprites.addSpriteToDrawList(
						_player->_position.x - (scaledSpriteWidth / 2),
						_player->_position.y - scaledSpriteHeight,
						&playerSprite,
						240 - _player->_position.y,
						scaledSpriteWidth,
						scaledSpriteHeight, player_sprite_related_2c85_82f3);
				}
			} else {
				if (otherNspAnimationType_maybe == 0x25) {
//					uVar1 = (uint)BYTE_ARRAY_2c85_41e7[1];
//					uVar7 = *(undefined2 *)((int)CPlayerSpriteWidthTbl + uVar1 * 2);
//					uVar4 = *(undefined2 *)((int)CPlayerSpriteHeightTbl + uVar1 * 2);
//					calculateScaledPlayerSpriteDimensions(uVar7,uVar4,playerSpriteY_maybe);
//					addSpriteToDraw(playerSpriteX_maybe - (_curPlayerSpriteWidth & 0xff) / 2,
//									playerSpriteY_maybe - (_curPlayerSpriteHeight_maybe & 0xff),uVar7,uVar4,
//									*(undefined2 *)((int)CPlayerSpritePtrTbl + uVar1 * 4),
//									*(undefined2 *)((int)&CPlayerSpritePtrTbl[0].Offset + uVar1 * 4),0xf0 - playerSpriteY_maybe ,
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
//									*(undefined2 *)((int)&CPlayerSpritePtrTbl[0].Offset + uVar1 * 4),0xf0 - playerSpriteY_maybe ,
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
				if (otherNspAnimationType_maybe == 0x12) {
					if (trunkPushCounter == 0) {
						sprite_y_scaling_threshold_maybe = 0xcb;
					}
					else if (trunkPushCounter == 1) {
						sprite_y_scaling_threshold_maybe = 0xcb;
					}
					else if (trunkPushCounter == 2) {
						sprite_y_scaling_threshold_maybe = 0xc4;
					}
					else {
						sprite_y_scaling_threshold_maybe = 0xaf;
					}
				}

				_room->calculateScaledSpriteDimensions(
					_player->_animations.getSpriteAt(_player->_frameIdx).width,
					_player->_animations.getSpriteAt(_player->_frameIdx).height,
					nsp_sprite_scaling_y_position != 0 ? nsp_sprite_scaling_y_position : _player->_position.y);

				if (otherNspAnimationType_maybe == 3) {
//					uVar1 = _curPlayerSpriteWidth & 0xff;
//					uVar2 = _curPlayerSpriteHeight_maybe & 0xff;
//					calculateScaledPlayerSpriteDimensions
//						(*(undefined2 *)((int)otherNspWidthTbl + (_player->_frameIdx + 1) * 2),
//						 *(undefined2 *)((int)&otherNspHeightTbl + (_player->_frameIdx + 1) * 2),playerSpriteY_maybe);
//					if (otherNspAnimationType_maybe == 3) {
//						uVar3 = ((_curPlayerSpriteWidth & 0xff) + uVar1) / 2;
//						iVar5 = playerSpriteX_maybe - uVar3;
//					}
//					else {
//						uVar11 = 0;
//						uVar10 = 1000;
//						uVar7 = scaledWalkSpeed_maybe._2_2_;
//						uVar4 = LXMUL@(CONCAT22((undefined2)scaledWalkSpeed_maybe,scaledWalkSpeed_maybe._2_2_),0x2d0000);
//						uVar3 = LUMOD@(uVar4,uVar7,uVar10,uVar11);
//						iVar5 = playerSpriteX_maybe - uVar3;
//					}
//					addSpriteToDraw(iVar5,playerSpriteY_maybe - uVar2,iVar9,iVar8,
//									*(undefined2 *)((int)otherNspSpritePtr + _player->_frameIdx * 4),
//									*(undefined2 *)((int)&otherNspSpritePtr[0].Offset + _player->_frameIdx * 4),
//									0xf0 - playerSpriteY_maybe,uVar1,uVar2,uVar3 & 0xff00);
//					bVar6 = extraout_AH_01;
				} else if (!BoolEnum_2c85_985a) {
					if (otherNspAnimationType_maybe == 0x11) {
//						addSpriteToDraw(playerSpriteX_maybe - (int)otherNspWidthTbl[0] / 2,playerSpriteY_maybe - iVar8,iVar9,iVa r8,
//										*(undefined2 *)((int)otherNspSpritePtr + _player->_frameIdx * 4),
//										*(undefined2 *)((int)&otherNspSpritePtr[0].Offset + _player->_frameIdx * 4),
//										0xf0 - playerSpriteY_maybe,iVar9,iVar8,_player_sprite_related_2c85_82f3);
//						bVar6 = extraout_AH_02;
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
						_sprites.addSpriteToDrawList(x, y, &animSprite, 0xf0 - _player->_position.y, animSprite.width, animSprite.height, player_sprite_related_2c85_82f3);
//						addSpriteToDraw(x,y,iVar9,iVar8,*(undefined2 *)((int)otherNspSpritePtr + _player->_frameIdx *  4),
//										*(undefined2 *)((int)&otherNspSpritePtr[0].Offset + _player->_frameIdx * 4),
//										0xf0 - playerSpriteY_maybe,iVar9,iVar8,_player_sprite_related_2c85_82f3);
//						bVar6 = extraout_AH_03;
					} // TODO continue adding logic here. else if (....
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
				} else if (otherNspAnimationType_maybe == 0x16) {
					// TODO
				} else if (otherNspAnimationType_maybe == 4 || otherNspAnimationType_maybe == 0x15) {
					// TODO
				} else if (otherNspAnimationType_maybe == 0x27) {
					// TODO
				} else if (otherNspAnimationType_maybe == 0x2f) {
					// TODO
				} else {
					_sprites.addSpriteToDrawList(
						_player->_position.x - scaledSpriteWidth / 2,
						_player->_position.y - scaledSpriteHeight,
						&_player->_animations.getSpriteAt(_player->_frameIdx),
						0xf0 - _player->_position.y,
						scaledSpriteWidth,
						scaledSpriteHeight,
						player_sprite_related_2c85_82f3);
				}
			}
		}
	}
}

void DarkseedEngine::setupOtherNspAnimation(int nspAnimIdx, int animId) {
	assert(nspAnimIdx < 20);
	_player->_playerIsChangingDirection = false;
	nsp_sprite_scaling_y_position = 0;
	BoolEnum_2c85_985a = false;

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
	case 6 : // stairs
		nsp_sprite_scaling_y_position = 0xbe;
		BoolEnum_2c85_985a = true;
		break;
	case 7 : // stairs down
		nsp_sprite_scaling_y_position = 0xbe;
		BoolEnum_2c85_985a = true;
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
	default:
		error("Unhandled animation type! %d", otherNspAnimationType_maybe);
	}
}

void DarkseedEngine::advanceAnimationFrame(int nspAminIdx) {
	if (!BoolEnum_2c85_985a) {
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

Common::String DarkseedEngine::getRoomFilePath(const Common::String &filename) {
	if (isCdVersion()) {
		return "room/" + filename;
	}
	return filename;
}

Common::String DarkseedEngine::getPictureFilePath(const Common::String &filename) {
	if (isCdVersion()) {
		return "picture/" + filename;
	}
	return filename;
}

void DarkseedEngine::handleObjCollision(int objNum) {
	if (objNum == 35 && _objectVar[22] < 2 && _cursor.getY() > 40) {
		objNum = 22;
	}
	if (_actionMode == LookAction || _actionMode == HandAction || objNum != 115) {
		if (_cursor.getY() < 10 && _actionMode > LookAction) {
			// TODO handle inventory
			// 171d:53c3
		} else {
			switch (_actionMode) {
			case HandAction:
				break;
			case LookAction:
				eyeCode(objNum);
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

void DarkseedEngine::eyeCode(int objNum) {
	// TODO lots of custom eye code here.
	if (_cursor.getY() > 39 && objNum != 77) {
		int eyeTosIdx = _objectVar.getEyeDescriptionTosIdx(objNum);
		if (eyeTosIdx < 979 && eyeTosIdx != 0)  {
			_console->printTosText(eyeTosIdx);
		}
//		else if (978 < *(int *)((int)_eyedescriptions + objNum * 2)) {
//			genericresponse(3,objNum,*(undefined2 *)((int)_eyedescriptions + objNum * 2));
//		}
		return;
	}
}

} // End of namespace Darkseed
