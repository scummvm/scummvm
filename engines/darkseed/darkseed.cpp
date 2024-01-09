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
		handleInput();
		updateDisplay();
		_room->update();
		_frame.draw();
		_room->draw();
		_sprites.drawSprites();
		_cursor.draw();
		_screen->makeAllDirty();
		_screen->update();
		wait();
	}
}

void DarkseedEngine::updateEvents() {
	Common::Event event;
	_isRightMouseClicked = false;
	_isLeftMouseClicked = false;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_MOUSEMOVE: _cursor.updatePosition(event.mouse.x, event.mouse.y); break;
		case Common::EVENT_RBUTTONDOWN: _isRightMouseClicked = true; break;
		case Common::EVENT_RBUTTONUP: _isRightMouseClicked = false; break;
		case Common::EVENT_LBUTTONDOWN: _isLeftMouseClicked = true; break;
		case Common::EVENT_LBUTTONUP: _isLeftMouseClicked = false; break;
		default: break;
		}
	}
}

void DarkseedEngine::wait() {
	g_system->delayMillis(10);
}

void DarkseedEngine::handleInput() {
	int currentRoomNumber = _room->_roomNumber;
	if (_player->_isAutoWalkingToBed && currentRoomNumber == 0 && _player->isAtPosition(0xdf, 0xbe)) {
		_player->_isAutoWalkingToBed = false;
		_currentTimeInSeconds = 0x7e90;
		_player->loadAnimations("bedsleep.nsp");
		setupOtherNspAnimation(1,5);
	}

	if (!isPlayingAnimation_maybe) {
		if (!_player->_playerIsChangingDirection) {
			if (currentRoomNumber == 0x39 &&_previousRoomNumber == 6) {
				_player->updateSprite();
			} else {
				if (_player->isAtWalkTarget() && !_player->BoolEnum_2c85_811c) {
					_player->updateSprite();
				} else {
					if (counter_2c85_888b >= 0 && !_player->isAtWalkTarget()) {
						counter_2c85_888b = 0;
						//						uVar7 = CONCAT11((char)(uVar7 >> 8),DAT_2c85_7dd7 + '\x01') & 0xff07;
						//						DAT_2c85_7dd7 = (char)uVar7;
						//						if (((DAT_2c85_7dd7 == '\0') || (DAT_2c85_7dd7 == '\x04')) &&
						//							((((currentRoomNumber != 0x22 && (currentRoomNumber != 0x13)) && (currentRoomNumber != 0x14)) &&
						//							  ((currentRoomNumber != 0x15 && (currentRoomNumber != 0x16)))))) {
						//							FUN_1208_0dac_sound_related(0x5c,CONCAT11((char)(uVar7 >> 8),5));
						//						}
					}
					_player->updateSprite();
				}
				if (_isLeftMouseClicked && _cursor.getY() > 0x28 && !_player->isPlayerWalking_maybe) { // prevLeftMouseButtonState == 0 &&
					if (_actionMode == PointerAction) {
						_player->calculateWalkTarget();
						_player->playerFaceWalkTarget();
					} else {
						int objNum = getObjectUnderCursor();
						if (objNum != -1) {

						}
					}
				}
//				if ((int)playerSpriteX_maybe < (int)walkTargetX) {
//					local_22 = walkTargetX - playerSpriteX_maybe;
//				}
//				else {
//					local_22 = playerSpriteX_maybe - walkTargetX;
//				}
//				if ((int)playerSpriteY_maybe < (int)walkTargetY) {
//					local_20 = walkTargetY - playerSpriteY_maybe;
//				}
//				else {
//					local_20 = playerSpriteY_maybe - walkTargetY;
//				}
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
				if (_player->isAtWalkTarget() && _player->BoolEnum_2c85_811c && _player->walkPathIndex != 0xff) {
//					if (walkPathIndex + 1 < numConnectorsInWalkPath) {
//						walkPathIndex = walkPathIndex + 1;
//						walkTargetX = *(uint *)((int)roomConnectorXPositionTbl +
//												*(int *)((int)walkPathConnectorList + walkPathIndex * 2) * 2);
//						walkTargetY = *(uint *)((int)roomConnectorYPositionTbl +
//												*(int *)((int)walkPathConnectorList + walkPathIndex * 2) * 2);
//					}
//					else {
//						walkTargetX = DAT_2c85_6dd7;
//						walkTargetY = DAT_2c85_6df1;
//						walkPathIndex = 0xff;
//					}
//					playerFaceWalkTarget();
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
//		uVar7 = playerSpriteX_maybe;
//		uVar5 = playerSpriteY_maybe;
//		if ((isPlayingAnimation_maybe == False) && (DAT_2c85_6b17 != '\0')) {
//			FUN_2022_7508();
//			uVar7 = playerSpriteX_maybe;
//			uVar5 = playerSpriteY_maybe;
//		}
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
//				calculateScaledPlayerSpriteDimensions(uVar7,uVar4,playerSpriteY_maybe);
				if ((currentRoomNumber != 0x35) || (DAT_2c85_81e0 != 2)) {
					const Sprite &playerSprite = _player->getSprite(_player->_frameIdx);
					uint8 scaledWidth = playerSprite.width;
					uint8 scaledHeight = playerSprite.height;
					_sprites.addSpriteToDrawList(
						_player->_position.x - (scaledWidth / 2),
						_player->_position.y - scaledHeight,
						&playerSprite,
						240 - _player->_position.y,
						scaledWidth,
						scaledHeight, player_sprite_related_2c85_82f3);
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

				if (nsp_sprite_scaling_y_position == 0) {
//					calculateScaledPlayerSpriteDimensions(iVar9,iVar8,playerSpriteY_maybe);
//					bVar6 = extraout_AH_00;
				}
				else {
//					calculateScaledPlayerSpriteDimensions(iVar9,iVar8,nsp_sprite_scaling_y_position);
//					bVar6 = extraout_AH;
				}

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
//						uVar7 = LONG_2c85_8116._2_2_;
//						uVar4 = LXMUL@(CONCAT22((undefined2)LONG_2c85_8116,LONG_2c85_8116._2_2_),0x2d0000);
//						uVar3 = LUMOD@(uVar4,uVar7,uVar10,uVar11);
//						iVar5 = playerSpriteX_maybe - uVar3;
//					}
//					addSpriteToDraw(iVar5,playerSpriteY_maybe - uVar2,iVar9,iVar8,
//									*(undefined2 *)((int)otherNspSpritePtr + _player->_frameIdx * 4),
//									*(undefined2 *)((int)&otherNspSpritePtr[0].Offset + _player->_frameIdx * 4),
//									0xf0 - playerSpriteY_maybe,uVar1,uVar2,uVar3 & 0xff00);
//					bVar6 = extraout_AH_01;
				} else if (!BoolEnum_2c85_985) {
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
	switch (otherNspAnimationType_maybe - 2) {
	default:
		break;
	}
}

void DarkseedEngine::updateAnimation() {
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
	default:
		error("Unhandled animation type! %d", otherNspAnimationType_maybe);
	}
}

void DarkseedEngine::advanceAnimationFrame(int nspAminIdx) {
	if (!BoolEnum_2c85_985a) {
//		LONG_2c85_8116._2_2_ = 0;
//		LONG_2c85_8116._0_2_ = 1000;
	}
	else {
//		calculateScaledPlayerSpriteDimensions(10,10,playerSpriteY_maybe);
	}
	isAnimFinished_maybe = false;
	animFrameChanged = false;
	const Obt &anim = _player->_animations.getAnimAt(nspAminIdx);
	spriteAnimCountdownTimer[nspAminIdx] = spriteAnimCountdownTimer[nspAminIdx] - 1;
	if (spriteAnimCountdownTimer[nspAminIdx] < 1) {
		animFrameChanged = true;
		animIndexTbl[nspAminIdx] = animIndexTbl[nspAminIdx] + 1;
		_player->_position.x += (int16)anim.deltaX[animIndexTbl[nspAminIdx]];
		_player->_position.y += (int16)anim.deltaY[animIndexTbl[nspAminIdx]];
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

int DarkseedEngine::getObjectUnderCursor() {
	// TODO
	return -1;
}

} // End of namespace Darkseed
