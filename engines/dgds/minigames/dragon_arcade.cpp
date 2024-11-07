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

#include "common/util.h"
#include "common/system.h"
#include "graphics/cursorman.h"

#include "dgds/dgds.h"
#include "dgds/image.h"
#include "dgds/includes.h"
#include "dgds/minigames/dragon_arcade.h"
#include "dgds/sound.h"
#include "dgds/scene.h"
#include "dgds/drawing.h"
#include "dgds/globals.h"
#include "dgds/game_palettes.h"
#include "dgds/menu.h"
#include "dgds/font.h"

namespace Dgds {

DragonArcade::DragonArcade() : _arcadeTTM(_npcState), _lastDrawnBladeHealth(-1), _lastDrawnBossHealth(-1),
	_nextRandomVal(0), _loadedArcadeStage(-1), _nextStage(0), _attemptCounter(0),
	_shouldUpdateState(0), _finishCountdown(0), _bladeState1(0), _bladePageOffset(0), _mouseButtonWentDown(0),
	_scrollXOffset(0), _nTickUpdates(0), _startDifficultyMaybe(0), _bossStateUpdateCounter(0), _npcStateResetCounter(0),
	_scrollVelocityX(0), _uint0a17(0), _currentYOffset(0), _int0b58(0), _int0b5a(0), _int0b60(0), _ttmYAdjust(0),
	_uint0be6(0), _dontMoveBladeFlag(false), _scrollXIncrement(0), _lMouseButtonState(false), _rMouseButtonState(false),
	_lastLMouseButtonState(false), _lastRMouseButtonState(false), _bladeXMove(0), _bladeHorizMoveAttempt(kBladeMoveNone),
	_currentArrowNum(0), _foundFloorY(0), _foundFloorFlag(false), _lastFloorY(0), _someMoveDirection(0),
	_haveBigGun(true), _haveBomb(true), _enemyHasSmallGun(false), _dontRedrawBgndAndWeapons(false),
	/*_arcadeNeedsBufferCopy(false), _flagInventoryOpened(false),*/ _initFinished(false), _stillLoadingScriptsMaybe(false),
	_flag40ee(false), _flag40ef(false), _bladeHasFired(false), _mouseIsAvailable(false), _isMovingStage(false),
	_bladeMoveFlag(kBladeMoveNone), _keyStateFlags(kBladeMoveNone), _bladeMoveFlagBeforeRButton(kBladeMoveNone)
{
}

static uint _getNextRandom() {
	return DgdsEngine::getInstance()->getRandom().getRandomNumber(65535);
}

void DragonArcade::finish() {
	_arcadeTTM._currentTTMNum = 0;
	_arcadeTTM.freeShapes();
	_arcadeTTM.freePages(0);
	_arcadeTTM._currentTTMNum = 1;
	_arcadeTTM.freeShapes();
	_arcadeTTM.freePages(1);
	_arcadeTTM._currentTTMNum = 2;
	_arcadeTTM.freeShapes();
	_arcadeTTM.freePages(2);
	//DgdsEngine::getInstance()->getGamePals()->freePal(arcadePal);
	_bulletImg.reset();
	_arrowImg.reset();
	_scrollImg.reset();
	_loadedArcadeStage = -1;

	DgdsEngine *engine = DgdsEngine::getInstance();
	engine->enableKeymapper();

	_initFinished = false;
	//warning("TODO: DragonArcade::finish: copy/clear some vid buffers here?");
}

bool DragonArcade::doTickUpdate() {
	// Note: original has a few buffer copy adjustments here,
	// but we never need them I think because we clear+flip every time.
	if (_finishCountdown == 0)
		return false;

	_nextRandomVal = _getNextRandom();

	updateMouseAndJoystickStates();
	updateBladeWithInputs();

	int16 floorY = findFloorUnderBlade();
	arcade2754(floorY);

	switch (_loadedArcadeStage) {
	case 0:
	case 1:
	case 2:
		updateBlade();
		arcade3e96();
		break;
	case 3:
		updateBoss();
		break;
	case 4:
		updateBlade();
		arcade4085();
		break;
	case 5:
		arcade4085();
		break;
	case 6:
		updateBoss2();
		break;
	}

	updateBullets();
	drawScrollBmp();
	runThenDrawBulletsInFlight();
	checkBladeFireAllStages();
	switch(_loadedArcadeStage) {
	case 0:
	case 1:
	case 2:
	case 4:
		checkEnemyFireStage0124();
		break;
	case 3:
		checkBossFireStage3();
		break;
	case 6:
		checkBossFireStage6();
		break;
	default:
		break;
	}
	drawHealthBars();

	// Original has delay here to reduce the arcade speed to 15 FPS.
	DgdsEngine::getInstance()->setSkipNextFrame();

	_nTickUpdates++;

	return true;
}

void DragonArcade::updateBullets() {
	for (int i = 19; i >= 0; i--) {
		if (_bullets[i]._state == kBulletHittingBlade || _bullets[i]._state == kBulletHittingEnemy) {
			_bullets[i]._state = kBulletInactive;
			continue;
		}

		if (_bullets[i]._state == kBulletFlying) {
			if (_bullets[i]._bulletType == 3) {
				_bullets[i]._y += _bullets[i]._ySpeed;
			}
			if (_bullets[i]._flipMode == kImageFlipNone) {
				_bullets[i]._x -= (_scrollXIncrement * 8 - 10);
				if (_bullets[i]._x < 321) {
					int16 collisionResult = checkBulletCollision(i);
					if (collisionResult == -1) {
						_bullets[i]._state = kBulletHittingEnemy;
					} else if (collisionResult == 1) {
						_bullets[i]._state = kBulletHittingBlade;
					}
					continue;
				}
			} else {
				_bullets[i]._x -= (_scrollXIncrement * 8 + 10);
				if (_bullets[i]._x > -41) {
					int16 collisionResult = checkBulletCollision(i);
					if (collisionResult == -1) {
						_bullets[i]._state = kBulletHittingEnemy;
					} else if (collisionResult == 1) {
						_bullets[i]._state = kBulletHittingBlade;
					}
					continue;
				}
			}
			_bullets[i]._state = kBulletInactive;
		}
	}
}

int16 DragonArcade::checkBulletCollision(int16 num) {
	int yoff = 0;
	for (int i = 19; i >= 0; i--) {
		if (_npcState[i].byte12 <= 0)
			continue;

		if (_bullets[num]._bulletType == 3) {
			yoff = 7;
		}
		if (_bullets[num]._bulletType != 1 || i == 0) {
			if (_bullets[num]._x < _npcState[i].x_11 || _npcState[i].x_12 < _bullets[num]._x ||
				_bullets[num]._y + yoff < _npcState[i].y_11 || _npcState[i].y_12 < _bullets[num]._y + yoff) {
				if (_bullets[num]._x < _npcState[i].x_21 || _npcState[i].x_22 < _bullets[num]._x ||
					_bullets[num]._y + yoff < _npcState[i].y_21 || _npcState[i].y_22 < _bullets[num]._y + yoff)
					continue;
				if (i == 0)
					return -1;

				if (_loadedArcadeStage == 3) {
					if (_bullets[num]._bulletType == 3)
						continue;
				} else {
					if (_loadedArcadeStage == 4) {
						if (_bullets[num]._bulletType == 1 || _bullets[num]._bulletType == 3)
							continue;
					} else if (_loadedArcadeStage != 6) {
						return -1;
					}
				}

				if (_bullets[num]._bulletType != 2) {
					return -1;
				}
			} else {
				if (i == 0) {
					bladeTakeHit();
					if (_npcState[0].health != 0)
						return 1;

					if (_bullets[num]._bulletType != 3) {
						return 1;
					}
					_shouldUpdateState = 3;
					return 1;
				}

				switch (_loadedArcadeStage) {
				case 0:
				case 1:
				case 2:
				case 4:
					if (_loadedArcadeStage == 4 || _npcState[i].byte12 < 30) {
						if (_bullets[num]._bulletType != 1) {
							playSfx(0x56);
							_npcState[i].byte12 = 1;
							if (_npcState[i].ttmPage < 28) {
								_npcState[i].ttmPage = 21;
							} else {
								_npcState[i].ttmPage = 49;
							}
							return 1;
						}
						break;
					}
					if (_npcState[i].byte12 == 30) {
						_flag40ee = false;
					} else {
						_flag40ef = false;
					}
					_npcState[i].byte12 = -8;
					_npcState[i].ttmPage = 33;
					break;
				case 3:
					if (_bullets[num]._bulletType != 3) {
						if (_npcState[i].byte12 == 1) {
							_npcState[i].byte12 = 7;
							_npcState[i].ttmPage = 75;
						}
						if (_haveBigGun) {
							decBossHealth();
						}
						decBossHealth();
						if (_npcState[i].health != 0) {
							return 1;
						}
						_npcState[i].byte12 = 8;
						_npcState[i].ttmPage = 79;
						setFinishCountdownIfLessThan0(0x78);
						return 1;
					}
					break;
				case 6:
					if (_bullets[num]._bulletType != 2) {
						if (_haveBigGun) {
							decBossHealthAndCheck();
						}
						decBossHealthAndCheck();
						return 1;
					}
					break;
				default:
					return 1;
				}
			}
		}
	}
	return 0;
}


static const int16 FIRE_ALLOWABLE_PAGES[] = {
	0x1A, 0x26, 0x3E, 0x74, 0x94, 0xA0, 0xB8, 0xEE
};

static const int16 FIRE_Y_OFFSETS_SMALL_GUN[] = {
	0x3A, 0x5A, 0x41, 0x4F, 0x37, 0x5A, 0x47, 0x4F
};

static const int16 FIRE_Y_OFFSETS_BIG_GUN[] = {
	0x38, 0x5E, 0x3E, 0x52, 0x40, 0x5E, 0x3D, 0x52
};

static const int16 FIRE_X_OFFSETS[] = {
	0xC4, 0xC8, 0xC8, 0xC2, 0x7A, 0x72, 0x69, 0x78
};

void DragonArcade::checkBladeFireAllStages() {
	int16 yoff;
	int16 sndno;
	ImageFlipMode flipMode;

	_bladeHasFired = false;
	if (_npcState[0].ttmNum != 0)
		return;

	for (int i = 0; i < 8; i++) {
		if (FIRE_ALLOWABLE_PAGES[i] == _npcState[0].ttmPage) {
			if (_npcState[0].ttmPage < 123) {
				flipMode = kImageFlipNone;
			} else {
				flipMode = kImageFlipH;
			}
			if (_haveBigGun) {
				yoff = FIRE_Y_OFFSETS_BIG_GUN[i];
			} else {
				yoff = FIRE_Y_OFFSETS_SMALL_GUN[i];
			}

			createBullet(FIRE_X_OFFSETS[i] + _npcState[0].x - 0xa0,
						 yoff + _arcadeTTM._startYOffset, flipMode, 0);

			// is this always 0?
			//if (INT_39e5_0c58 == 0) {
			sndno = 47;
			//} else {
			//sndno = 52;
			//}
			playSfx(sndno);
			_bladeHasFired = true;
			break;
		}
	}
}


static const int16 ENEMY_FIRE_ALLOWABLE_PAGES[] = { 3, 12, 31, 40 };
static const int16 ENEMY_FIRE_X_OFFSETS[] = { 0xB1, 0xB3, 0x77, 0x75, };
static const int16 ENEMY_FIRE_Y_OFFSETS[] = { 0x4E, 0x56, 0x4D, 0x55,};

void DragonArcade::checkEnemyFireStage0124() {
	for (int i = 9; i != 0; i--) {
		if (_npcState[i].byte12 == 0)
			continue;

		for (int j = 0; j < 4; j++) {
			if (_npcState[i].x < 340 && -20 < _npcState[i].x &&
				ENEMY_FIRE_ALLOWABLE_PAGES[j] == _npcState[i].ttmPage) {
				debug("enemy %d @ %d firing type %d on page %d", i, _npcState[i].x, j, _npcState[i].ttmPage);
				ImageFlipMode flipMode = (_npcState[i].ttmPage < 29) ? kImageFlipNone : kImageFlipH;
				createBullet(ENEMY_FIRE_X_OFFSETS[j] + _npcState[i].xx - _scrollXOffset * 8 - 0xa0,
							 ENEMY_FIRE_Y_OFFSETS[j] + _npcState[i].yy + 3, flipMode, 1);
				playSfx(0x25);
			}
		}
	}
}


void DragonArcade::checkBossFireStage3() {
	if (_npcState[1].x < 0x154 && -20 < _npcState[1].x && _npcState[1].ttmPage == 22) {
		createBullet(_npcState[1].xx - _scrollXOffset * 8 - 44,
						_npcState[1].yy + 70, kImageFlipH, 3);
		playSfx(0x2a);
	}
}

void DragonArcade::checkBossFireStage6() {
	ImageFlipMode flipMode = (_npcState[1].ttmPage < 40) ? kImageFlipNone: kImageFlipH;

	if (_npcState[1].x < 0x154 && -20 < _npcState[1].x &&
		(_npcState[1].ttmPage == 9 || _npcState[1].ttmPage == 40)) {
		createBullet(_npcState[1].xx - _scrollXOffset * 8 - 19,
						_npcState[1].yy + 86, flipMode, 2);
		playSfx(0x24);
	}
}

void DragonArcade::limitToCenterOfScreenAndUpdateCursor() {
	Common::Point lastMouse = DgdsEngine::getInstance()->getLastMouse();
	/* limit mouse coords to (x = 144-190, y = 135-180) */
	lastMouse.x = CLIP((int)lastMouse.x, 144, 190);
	lastMouse.y = CLIP((int)lastMouse.y, 135, 180);
	g_system->warpMouse(lastMouse.x, lastMouse.y);

	int16 arrowNum = (lastMouse.x - 144) / 16 + ((lastMouse.y - 136) / 16) * 3;

	if (_currentArrowNum != arrowNum && arrowNum < 9) {
		_currentArrowNum = arrowNum;
		CursorMan.replaceCursor(*(_arrowImg->getSurface(arrowNum)->surfacePtr()), 0, 0, 0, 0);
	}
	//if (g_arcadeNeedsBufferCopy == 0) {
	//  Arcade_MouseCursorDraw();
	//}
}

void DragonArcade::onKeyDown(Common::KeyState kbd) {
	switch (kbd.keycode) {
	case Common::KEYCODE_KP7:
		_keyStateFlags = static_cast<DragonBladeMoveFlag>(_keyStateFlags | kBladeMoveUp | kBladeMoveLeft);
		break;
	case Common::KEYCODE_KP8:
	case Common::KEYCODE_UP:
	case Common::KEYCODE_w:
		_keyStateFlags = static_cast<DragonBladeMoveFlag>(_keyStateFlags | kBladeMoveUp);
		break;
	case Common::KEYCODE_KP9:
		_keyStateFlags = static_cast<DragonBladeMoveFlag>(_keyStateFlags | kBladeMoveUp | kBladeMoveRight);
		break;
	case Common::KEYCODE_KP4:
	case Common::KEYCODE_LEFT:
	case Common::KEYCODE_a:
		_keyStateFlags = static_cast<DragonBladeMoveFlag>(_keyStateFlags | kBladeMoveLeft);
		break;
	case Common::KEYCODE_KP6:
	case Common::KEYCODE_RIGHT:
	case Common::KEYCODE_d:
		_keyStateFlags = static_cast<DragonBladeMoveFlag>(_keyStateFlags | kBladeMoveRight);
		break;
	case Common::KEYCODE_KP1:
		_keyStateFlags = static_cast<DragonBladeMoveFlag>(_keyStateFlags | kBladeMoveDown | kBladeMoveLeft);
		break;
	case Common::KEYCODE_KP2:
	case Common::KEYCODE_DOWN:
	case Common::KEYCODE_x:
		_keyStateFlags = static_cast<DragonBladeMoveFlag>(_keyStateFlags | kBladeMoveDown);
		break;
	case Common::KEYCODE_KP3:
		_keyStateFlags = static_cast<DragonBladeMoveFlag>(_keyStateFlags | kBladeMoveDown | kBladeMoveRight);
		break;
	case Common::KEYCODE_SPACE:
		_lMouseButtonState = true;
		break;
	case Common::KEYCODE_RETURN:
	case Common::KEYCODE_KP_ENTER:
		_rMouseButtonState = true;
		break;
	default:
		break;
	}
}

void DragonArcade::onKeyUp(Common::KeyState kbd) {
	switch (kbd.keycode) {
	case Common::KEYCODE_KP7:
		_keyStateFlags = static_cast<DragonBladeMoveFlag>(_keyStateFlags & ~(kBladeMoveUp | kBladeMoveLeft));
		break;
	case Common::KEYCODE_KP8:
	case Common::KEYCODE_UP:
	case Common::KEYCODE_w:
		_keyStateFlags = static_cast<DragonBladeMoveFlag>(_keyStateFlags & ~kBladeMoveUp);
		break;
	case Common::KEYCODE_KP9:
		_keyStateFlags = static_cast<DragonBladeMoveFlag>(_keyStateFlags & ~(kBladeMoveUp | kBladeMoveRight));
		break;
	case Common::KEYCODE_KP4:
	case Common::KEYCODE_LEFT:
	case Common::KEYCODE_a:
		_keyStateFlags = static_cast<DragonBladeMoveFlag>(_keyStateFlags & ~kBladeMoveLeft);
		break;
	case Common::KEYCODE_KP6:
	case Common::KEYCODE_RIGHT:
	case Common::KEYCODE_d:
		_keyStateFlags = static_cast<DragonBladeMoveFlag>(_keyStateFlags & ~kBladeMoveRight);
		break;
	case Common::KEYCODE_KP1:
		_keyStateFlags = static_cast<DragonBladeMoveFlag>(_keyStateFlags & ~(kBladeMoveDown | kBladeMoveLeft));
		break;
	case Common::KEYCODE_KP2:
	case Common::KEYCODE_DOWN:
	case Common::KEYCODE_x:
		_keyStateFlags = static_cast<DragonBladeMoveFlag>(_keyStateFlags & ~kBladeMoveDown);
		break;
	case Common::KEYCODE_KP3:
		_keyStateFlags = static_cast<DragonBladeMoveFlag>(_keyStateFlags & ~(kBladeMoveDown | kBladeMoveRight));
		break;
	case Common::KEYCODE_SPACE:
		_lMouseButtonState = false;
		break;
	case Common::KEYCODE_RETURN:
	case Common::KEYCODE_KP_ENTER:
		_rMouseButtonState = false;
		break;
	default:
		break;
	}
}


void DragonArcade::keyboardUpdate() {
	//warning("TODO: Keyboard update");
	// TODO: Keyboard update.
}

void DragonArcade::mouseUpdate() {
	limitToCenterOfScreenAndUpdateCursor();
	_rMouseButtonState |= DgdsEngine::getInstance()->getScene()->isRButtonDown();
	_lMouseButtonState |= DgdsEngine::getInstance()->getScene()->isLButtonDown();
	int16 arrowRow = _currentArrowNum / 3;
	if (arrowRow == 0) {
		_keyStateFlags = kBladeMoveUp;
	} else if (arrowRow == 1) {
		_keyStateFlags = kBladeMoveNone;
	} else if (arrowRow == 2) {
		_keyStateFlags = kBladeMoveDown;
	}
	if (_currentArrowNum % 3 == 0) {
		_keyStateFlags = static_cast<DragonBladeMoveFlag>(_keyStateFlags | kBladeMoveLeft);
	} else if (_currentArrowNum % 3 == 2) {
		_keyStateFlags = static_cast<DragonBladeMoveFlag>(_keyStateFlags | kBladeMoveRight);
	}
}

void DragonArcade::updateMouseAndJoystickStates() {
	_bladeXMove = 0;
	_scrollXIncrement = 0;
	//_rMouseButtonState = false;
	//_lMouseButtonState = false;
	if (!_mouseIsAvailable) {
		//if (g_optionJoystickOnButtonState != 0) {
		//	Joystick_ArcadeUpdate();
		//}
		keyboardUpdate();
	} else {
		mouseUpdate();
	}

	if (_mouseButtonWentDown != 0x80) {
		_bladeMoveFlag = kBladeMoveNone;
		if ((_keyStateFlags & kBladeMoveRight) == kBladeMoveNone) {
			if ((_keyStateFlags & kBladeMoveLeft) != kBladeMoveNone) {
				if (_someMoveDirection) {
					_someMoveDirection = 0;
				}
				_bladeMoveFlag = kBladeMoveLeft;
				if (_bladeState1 == 0) {
					_scrollVelocityX = -1;
					_bladeHorizMoveAttempt = kBladeMoveLeft;
				}
			}
		} else {
			_bladeMoveFlag = kBladeMoveRight;
			if (_bladeState1 == 0) {
				_scrollVelocityX = 1;
				_bladeHorizMoveAttempt = kBladeMoveRight;
			}
		}
		if ((_keyStateFlags & kBladeMoveUp) == kBladeMoveNone) {
			if ((_keyStateFlags & kBladeMoveDown) != kBladeMoveNone) {
				_bladeMoveFlag = static_cast<DragonBladeMoveFlag>(_bladeMoveFlag | kBladeMoveDown);
			}
		} else {
			_bladeMoveFlag = static_cast<DragonBladeMoveFlag>(_bladeMoveFlag | kBladeMoveUp);
		}
		if (_lMouseButtonState && !_lastLMouseButtonState) {
			_mouseButtonWentDown = 1;
		}
		if (_rMouseButtonState && !_lastRMouseButtonState && _bladeState1 != 2 && _bladeState1 != 1) {
			_mouseButtonWentDown = 2;
			_bladeMoveFlagBeforeRButton = _bladeMoveFlag;
		}
		_lastLMouseButtonState = _lMouseButtonState;
		_lastRMouseButtonState = _rMouseButtonState;
	}
}

int16 DragonArcade::findFloorUnderBlade() {
	updateFloorsUnderBlade();
	if (_bladeState1 == 1 || _bladeState1 == 2) {
		if ((_bladePageOffset + 56 == _npcState[0].ttmPage) ||
			(_bladePageOffset + 22 == _npcState[0].ttmPage)) {
			findFloorMatchOrMinOrMax();
		} else {
			findFloorMinGE();
			if (isFloorNotFound()) {
				_foundFloorY = -0x100;
			}
		}
	} else if ((_bladeMoveFlag & kBladeMoveDown) == kBladeMoveNone) {
		if ((_bladeMoveFlag & kBladeMoveUp) == kBladeMoveNone) {
			/* Not moving up or down */
			findFloorMatch();
			if (isFloorNotFound()) {
				findFloorMinGT();
				if (isFloorNotFound()) {
					findFloorMax();
					if (isFloorNotFound()) {
						_foundFloorY = -0x100;
					}
				}
			}
		} else {
			/* Move up */
			findFloorMax();
			if (isFloorNotFound()) {
				findFloorMatch();
				if (isFloorNotFound()) {
					findFloorMinGT();
					if (isFloorNotFound()) {
						_foundFloorY = -0x100;
					}
				}
			}
		}
	} else {
		/* Move down */
		findFloorMinGT();
		if (isFloorNotFound()) {
			findFloorMatch();
			if (isFloorNotFound()) {
				findFloorMax();
				if (isFloorNotFound()) {
					_foundFloorY = -0x100;
				}
			}
		}
	}
	return _foundFloorY;
}

bool DragonArcade::isNpcInsideXRange(int16 num) {
	return _npcState[num].x < 321 && _npcState[num].x > 1;
}

void DragonArcade::arcade16bc() {
	_bladeState1 = 5;
	_npcState[0].ttmPage = _bladePageOffset + 64;
	_arcadeTTM._startYOffset++;
	_currentYOffset = _arcadeTTM._startYOffset;
	_uint0a17++;
}

void DragonArcade::arcade1e83() {
	_scrollXOffset -= _scrollXIncrement;
	_npcState[0].x = _npcState[0].x - _bladeXMove;
}

void DragonArcade::arcade16de(int16 param) {
	if (_bladeState1 == 2) {
		_npcState[0].ttmPage = _bladePageOffset + 57;
	}
	else {
		_npcState[0].ttmPage = _bladePageOffset + 23;
	}
	_npcState[0].ttmNum = 0;
	_arcadeTTM._startYOffset = param;
	_uint0a17 = 0;
	_bladeState1 = 0;
	_ttmYAdjust = 0;
	_int0b5a = 15;
}

void DragonArcade::arcade4085() {
	for (int i = 10; i < 12; i++) {
		if (_npcStateResetCounter == 20) {
			_npcState[i].ttmPage = 0;
		}
		_npcState[i].ttmPage++;
		if (_npcState[i].ttmPage < 1 || 14 < _npcState[i].ttmPage) {
			_npcState[i].byte12 = 0;
		}
		else {
			if (_npcState[i].ttmPage == 1) {
				if (isNpcInsideXRange(i))
					playSfx(0x1d);
			}
			_npcState[i].byte12 = -5;
			if (_npcState[0].health && _npcState[i].xx - 0x10 <= _npcState[0].xx &&
				_npcState[0].xx <= _npcState[i].xx + 0x37) {
				bladeTakeHit();
			}
		}
	}

	if ((_npcStateResetCounter & 3) == 0) {
		_npcState[12].ttmPage++;
		_npcState[13].ttmPage++;
		if (0x1e < _npcState[12].ttmPage) {
			_npcState[12].ttmPage = 15;
			_npcState[13].ttmPage = 15;
		}
	}

	for (int i = 10; i < 12; i++) {
		if (_npcState[i].ttmPage == 29) {
			if (isNpcInsideXRange(i + 2))
				playSfx(0x59);
			if (_npcState[0].health && _npcState[i].yy - 0x10 <= _npcState[0].xx && _npcState[0].xx <= _npcState[i].yy + 0x23) {
				bladeTakeHit();
				bladeTakeHit();
			}
		}
	}

	_npcState[14].ttmPage++;
	if (0x35 < _npcState[14].ttmPage) {
		if (isNpcInsideXRange(0xe))
			playSfx(0x58);
		_npcState[14].ttmPage = 40;
	}
	if (_npcState[0].health != 0 && ((_bladeMoveFlag & kBladeMoveDown) == kBladeMoveNone || _arcadeTTM._startYOffset < -6)
		&& _npcState[14].xx - 0x10 <= _npcState[0].xx && _npcState[0].xx <= _npcState[14].xx + 0x23) {
		bladeTakeHit();
		bladeTakeHit();
		if (_npcState[0].health == 0) {
			/* blade dies! */
			_npcState[0].ttmNum = 2;
			_npcState[0].ttmPage = 34;
			_npcState[0].byte12 = 14;
			_bladeState1 = 14;
			_shouldUpdateState = 2;
		}
	}

	_npcStateResetCounter++;
	if (_npcStateResetCounter == 60) {
		_npcStateResetCounter = 0;
	}
}

bool DragonArcade::isFloorNotFound() {
	return abs(_foundFloorY) > 990;
}


uint16 DragonArcade::moveToNextStage() {
	int xblock = _scrollXOffset + _npcState[0].x / 8;

	switch (_loadedArcadeStage) {
	case 0:
		if (0x31 < _scrollXOffset) {
			_loadedArcadeStage = 1;
		}
		break;
	case 1:
		if (!_isMovingStage && xblock == 0x80 && 0 < _scrollVelocityX && _bladeState1 == 0) {
			_scrollXOffset -= _scrollVelocityX;
			arcade2445();
			return 1;
		}

		if (0x89 < xblock && xblock < 0x8d && 20 < _arcadeTTM._startYOffset &&
			_arcadeTTM._startYOffset < 70 && _bladePageOffset == 0) {
			_scrollXOffset = 0x89 - _npcState[0].x / 8;
			_arcadeTTM._startYOffset = -13;
			playSFX55AndStuff();
			_loadedArcadeStage = 2;
			initValuesForStage2();
			return 1;
		}
		break;
	case 2:
		if ((!_isMovingStage && xblock == 0x90 && 0 < _scrollVelocityX && _bladeState1 == 0) ||
			(!_isMovingStage && xblock == 0xe9 && 0 < _scrollVelocityX && _bladeState1 == 0)) {
			_scrollXOffset -= _scrollVelocityX;
			arcade2445();
			return 1;
		}

		if (0x99 < xblock && xblock < 0x9c && 20 < _arcadeTTM._startYOffset &&
			  _arcadeTTM._startYOffset < 70 && _bladePageOffset == 0) {
			_scrollXOffset = 0x9a - _npcState[0].x / 8;
			_arcadeTTM._startYOffset = -13;
			playSFX55AndStuff();
			return 1;
		}

		if (_scrollXOffset < 0x100) {
			if (0xf3 < xblock && xblock < 0xf6 && 30 < _arcadeTTM._startYOffset &&
				 _arcadeTTM._startYOffset < 60 && _bladePageOffset == 0 && _startDifficultyMaybe != 3) {
				_scrollXOffset = 0xf4 - _npcState[0].x / 8;
				_arcadeTTM._startYOffset = -0x1a;
				playSFX55AndStuff();
				return 1;
			}
		} else if (_bladeState1 == 0) {
			//Arcade_VidPtrs_SrcFront_DstBack();
			//ResetFlagAt_48a0();
			//SetFlagAt_48a0_andMaybeMouseUpdate();
			loadTTMScriptsForStage(3);
		}
		break;
	case 4:
		if (-2 < _arcadeTTM._startYOffset && 129 < _npcState[0].xx && _npcState[0].xx < 201 && _npcState[0].health != 0) {
			playSfx(0x57);
			setFinishCountdownIfLessThan0(0x14);
			if (_haveBigGun) {
				_npcState[0].ttmPage = 58;
			} else {
				_npcState[0].ttmPage = 54;
			}
			_bladeState1 = 13;
			_mouseButtonWentDown = 0x80;
			_npcState[0].byte12 = 13;
			_npcState[0].health = 0;
			_npcState[0].ttmNum = 2;
			return 1;
		}

		if (_scrollXOffset < 0x100) {
			if (!_isMovingStage && xblock == 0x54 && 0 < _scrollVelocityX && _bladeState1 == 0) {
				_scrollXOffset -= _scrollVelocityX;
				arcade2445();
				return 1;
			}
		} else if (_bladeState1 == 0) {
			//Arcade_VidPtrs_SrcFront_DstBack();
			//SetFlagAt_48a0_andMaybeMouseUpdate();
			//ResetFlagAt_48a0();
			loadTTMScriptsForStage(6);
		}
		break;
	case 6:
		if (!_stillLoadingScriptsMaybe && _scrollXOffset < 0x100) {
			_scrollXOffset = 0x100;
			_npcState[0].x -= 8;
			if (_npcState[0].x < 0) {
				_npcState[0].x = 0;
			}
		} else if (0x11f < xblock && _someMoveDirection == 0) {
			_someMoveDirection = 1;
		}
		break;
	default:
		break;
	}

	return 0;
}

void DragonArcade::playSFX55AndStuff() {
	_isMovingStage = false;
	playSfx(0x55);
	_bladeState1 = 7;
	_uint0a17 = 0;
	_ttmYAdjust = 0;
	_int0b5a = 15;
	_scrollVelocityX = 0;
	_npcState[0].ttmPage = _bladePageOffset + 67;
}


void DragonArcade::updateFloorsUnderBlade() {
	_floorY.clear();
	_floorFlag.clear();
	const Common::Array<ArcadeFloor> &floorData = _arcadeTTM.getFloorData();

	for (const auto &floor : floorData) {
		if (floor.x <= _npcState[0].xx && _npcState[0].xx <= floor.x + floor.width) {
			_floorY.push_back(floor.yval - 108);
			_floorFlag.push_back(floor.flag);
		}
	}
}

void DragonArcade::findFloorMinGE() {
	_foundFloorY = 999;
	for (uint i = 0; i < _floorY.size(); i++) {
		if (_currentYOffset <= _floorY[i] && _floorY[i] < _foundFloorY) {
			_foundFloorY = _floorY[i];
			_foundFloorFlag = _floorFlag[i];
		}
	}
}

void DragonArcade::findFloorMinGT() {
	_foundFloorY = 999;
	for (uint i = 0; i < _floorY.size(); i++) {
		if (_currentYOffset < _floorY[i] && _floorY[i] < _foundFloorY) {
			_foundFloorY = _floorY[i];
			_foundFloorFlag = _floorFlag[i];
		}
	}
}

void DragonArcade::findFloorMatch() {
	_foundFloorY = -999;
	for (uint i = 0; i < _floorY.size(); i++) {
		if (_floorY[i] == _currentYOffset) {
			_foundFloorY = _floorY[i];
			_foundFloorFlag = _floorFlag[i];
		}
	}
}

void DragonArcade::findFloorMax() {
	_foundFloorY = -999;
	for (uint i = 0; i < _floorY.size(); i++) {
		if (_floorY[i] < _currentYOffset && _foundFloorY < _floorY[i]) {
			_foundFloorY = _floorY[i];
			_foundFloorFlag = _floorFlag[i];
		}
	}
}

void DragonArcade::findFloorMatchOrMinOrMax() {
	findFloorMatch();
	if (isFloorNotFound()) {
		findFloorMinGT();
		if (isFloorNotFound()) {
			findFloorMax();
		}
	}
}


void DragonArcade::arcade2445() {
	_scrollVelocityX = 0;
	_bladeState1 = 6;
	_npcState[0].ttmPage += 78;
	_isMovingStage = true;
}

static const int16 STAGE_2_XX[] = {
	0x6A3, 0x6CD, 0x7C3
};

static const int16 STAGE_2_YY[] = {
	0, 0, -23
};

static const byte STAGE_2_BYTE12[] = {
	4, 5, 4
};

static const int16 STAGE_2_TTMPAGE[] = {
	30, 39, 30
};

static const int16 STAGE_2_VAL1_PART2[] = {
	0x547, 0x557, 0x567, 0x577
};

void DragonArcade::initValuesForStage2() {
	for (int i = 7; i != 0; i--) {
		_npcState[i].byte12 = 0;
	}
	for (int i = 3; i != 0; i--) {
		_npcState[i].xx = STAGE_2_XX[i - 1];
		_npcState[i].yy = STAGE_2_YY[i - 1];
		_npcState[i].byte12 = STAGE_2_BYTE12[i - 1];
		_npcState[i].ttmPage = STAGE_2_TTMPAGE[i - 1];
		_npcState[i].ttmNum = 1;
	}
	for (int i = 10; i < 14; i++) {
		_npcState[i].xx = STAGE_2_VAL1_PART2[i - 10];
	}
	_npcState[18].xx = 0x52f;
	_npcState[18].yy = -13;
	_npcState[18].byte12 = 31;
	_npcState[18].ttmPage = 32;
	_npcState[18].ttmNum = 2;
}

void DragonArcade::arcade2754(int16 floorY) {
	if (0 < _finishCountdown) {
		_finishCountdown--;
	}

	if (_finishCountdown == 0) {
		if (_npcState[0].health == 0) {
			if (_shouldUpdateState == 0) {
				_shouldUpdateState = 1;
			}
		} else {
			_shouldUpdateState = 0;
		}
	} else {
		_dontMoveBladeFlag = false;
		if (100 < _arcadeTTM._startYOffset) {
			_npcState[0].health = 0;
			if (_finishCountdown < 1) {
				setFinishCountdownIfLessThan0(20);
			} else {
				_arcadeTTM._startYOffset = 100;
			}
		}
		if (!moveToNextStage()) {
			if (floorY < _arcadeTTM._startYOffset && _currentYOffset <= floorY) {
				arcade16de(floorY);
			} else if (_lastFloorY < _arcadeTTM._startYOffset && _currentYOffset <= _lastFloorY) {
				arcade1e83();
				floorY = _lastFloorY;
				arcade16de(_lastFloorY);
			} else if (_bladeState1 == 0) {
				if (floorY == -0x100) {
					arcade16bc();
				} else if (_lastFloorY != -0x100) {
					// Pop to new floor level if close enough
					if (abs(_lastFloorY - floorY) < 16 || _nTickUpdates == 0) {
						_arcadeTTM._startYOffset = floorY;
					} else if (floorY < _lastFloorY) {
						findFloorMatch();
						if (((_bladeMoveFlag & kBladeMoveUp) == kBladeMoveNone) ||
							(_lastFloorY != _foundFloorY)) {
							findFloorMinGT();
							if (!isFloorNotFound()) {
								_arcadeTTM._startYOffset = _foundFloorY;
								floorY = _foundFloorY;
							} else {
								arcade16bc();
							}
						} else {
							_arcadeTTM._startYOffset = _lastFloorY;
							floorY = _lastFloorY;
						}
					} else if (_lastFloorY < floorY) {
						if ((_bladeMoveFlag & kBladeMoveDown) == kBladeMoveNone) {
							if (_arcadeTTM._startYOffset + 20 < floorY) {
								_bladeState1 = 1;
								_npcState[0].ttmPage = _bladePageOffset + 22;
								_arcadeTTM._startYOffset += 10;
								_uint0a17++;
							} else {
								_arcadeTTM._startYOffset = floorY;
							}
						} else {
							findFloorMatchOrMinOrMax();
							_arcadeTTM._startYOffset = _foundFloorY;
							floorY = _foundFloorY;
						}
					}
				}
			} else if ((_bladeState1 == 3 || _bladeState1 == 4) && 25 < abs(_lastFloorY - floorY) && _nTickUpdates) {
				floorY = _lastFloorY;
			}
		}
		_npcState[0].xx = _scrollXOffset * 8 + _npcState[0].x;
		_currentYOffset = _arcadeTTM._startYOffset;
		_lastFloorY = floorY;
	}
}

// Array at 0c5a in original
static const int16 NPC_RESET_COUNTS_1[4][7] {
	{ 0x8, 0x19, 0x5A, 0x78, 0x8C, 0xA5, 0xBE },
	{ 0x6, 0x28, 0x5A, 0x87, 0x96, 0xAA, 0xC0 },
	{ 0x4, 0x37, 0x69, 0x79, 0x91, 0xA0, 0xC2 },
	{ 0x2, 0x46, 0x69, 0x87, 0x9B, 0xAF, 0xC4 },
};

// Array at 0c92 in original
static const int16 NPC_RESET_COUNTS_2[4][7] {
	{ 0xA, 0x37, 0x46, 0x55, 0x91, 0xAA, 0xC8 },
	{ 0x19, 0x5F, 0x87, 0x9B, 0xB9, 0xD7, -0x1 },
	{ 0x19, 0x23, 0x69, 0x7D, 0x9B, 0xB9, 0xD7 },
	{ 0xA, 0x37, 0x46, 0x73, 0xAA, 0xC8, -0x1 },
};


void DragonArcade::arcade3e96() {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 7; j++) {
			if (_loadedArcadeStage == 0 && _flag40ee && NPC_RESET_COUNTS_1[i][j] == _npcStateResetCounter) {
				_npcState[i + 10].ttmPage = 1;
			} else if (_loadedArcadeStage == 1) {
				if (NPC_RESET_COUNTS_2[i][j] == _npcStateResetCounter) {
					_npcState[i + 4 + 10].ttmPage = 1;
				}
				if (_flag40ee && NPC_RESET_COUNTS_1[i][j] == _npcStateResetCounter) {
					_npcState[i + 10].ttmPage = 1;
				}
			} else if (_loadedArcadeStage == 2 && _flag40ef && NPC_RESET_COUNTS_2[i][j] == _npcStateResetCounter) {
				_npcState[i + 10].ttmPage = 1;
			}
		}
	}

	for (int i = 10; i < 18; i++) {
		_npcState[i].ttmPage++;
		if (_npcState[i].ttmPage < 2 || _npcState[i].ttmPage > 18) {
			_npcState[i].byte12 = 0;
		} else {
			if (_npcState[i].ttmPage == 2 && isNpcInsideXRange(i)) {
				playSfx(0x5a);
			}
			_npcState[i].byte12 = -4;
			if (_npcState[i].ttmPage < 15 &&
				_npcState[i].xx - 16 <= _npcState[0].xx &&
				_npcState[0].xx <= _npcState[i].xx + 12 && _npcState[0].health != 0 &&
				(_loadedArcadeStage != 1 || _arcadeTTM._startYOffset < -9 ||
				 (_bladeMoveFlag & kBladeMoveRight) == kBladeMoveNone)) {
				setFinishCountdownIfLessThan0(20);
				_npcState[0].ttmNum = 2;
				_npcState[0].health = 0;
				if (_haveBigGun) {
					_npcState[0].ttmPage = 25;
				} else {
					_npcState[0].ttmPage = 34;
				}
				_npcState[0].byte12 = 12;
				_bladeState1 = 12;
				_mouseButtonWentDown = 0x80;
			}
		}
	}
	_npcStateResetCounter++;
	if (_npcStateResetCounter == 221) {
		_npcStateResetCounter = 0;
	}

}

void DragonArcade::updateBlade() {
	int16 local_6 = 0;

	for (int i = 9; i > 0; i--) {
		if (_npcState[i].byte12 == 0)
			continue;

		int16 startPage = (_npcState[i].ttmPage < 30) ? 0 : 28;

		if (_npcState[i].byte13 == 0 &&
			((startPage != 0 && _npcState[i].xx < _npcState[0].xx) ||
			 (startPage == 0 && _npcState[0].xx < _npcState[i].xx))) {

			if (_npcState[i].byte12 == 4 || _npcState[i].byte12 == 5) {
				if (_npcState[i].byte12 == 4) {
					_npcState[i].byte13 = 5;
					_npcState[i].byte12 = 7;
					_npcState[i].ttmPage = startPage + 26;
				} else if (_npcState[i].byte12 == 5) {
					_npcState[i].byte13 = 7;
					_npcState[i].byte12 = 8;
					_npcState[i].ttmPage = startPage + 18;
				}
				local_6++;
			}
		}

		if (local_6 != 0)
			continue;

		switch (_npcState[i].byte12 - 1) {
		case 5:
			if ( _npcState[i].ttmPage < startPage + 11) {
				_npcState[i].ttmPage++;
				break;
			}
			_npcState[i].byte13 = 0;
			_npcState[i].byte12 = 5;
			_npcState[i].ttmPage = startPage + 11;
			break;
		case 6:
			if (_npcState[i].ttmPage < startPage + 29) {
				_npcState[i].ttmPage++;
				break;
			}
			startPage = startPage ^ 28;
			if (_npcState[i].byte13 == 7) {
				_npcState[i].byte13 = 0;
				_npcState[i].byte12 = 4;
				_npcState[i].ttmPage = startPage + 2;
				break;
			}
			_npcState[i].byte12 = 6;
			_npcState[i].ttmPage = startPage + 9;
			break;
		case 7:
			if (_npcState[i].ttmPage < startPage + 20) {
				_npcState[i].ttmPage++;
				break;
			}
			if (_npcState[i].byte13 == 4){
				_npcState[i].byte13 = 0;
				_npcState[i].byte12 = 4;
				_npcState[i].ttmPage = startPage + 2;
				break;
			}
			_npcState[i].byte12 = 7;
			_npcState[i].ttmPage = startPage + 26;
			break;
		case 0:
			if (_npcState[i].ttmPage < startPage + 23) {
				_npcState[i].ttmPage++;
			}
			break;
		case 3:
			if ((15 - i == (_nextRandomVal % 16)) &&
				 abs(_npcState[i].y - _npcState[0].y) < 36 && _npcState[0].health != 0) {
				_npcState[i].byte12 = 2;
				_npcState[i].ttmPage = startPage + 3;
			}
			break;
		case 4:
			if ((15 - i == (_nextRandomVal % 16)) &&
				  abs(_npcState[i].y - _npcState[0].y) < 36 && _bladeState1 != 8 && _bladeState1 != 9) {
				_npcState[i].byte12 = 3;
				_npcState[i].ttmPage = startPage + 12;
			}
			break;
		case 2:
			if (_npcState[i].ttmPage < startPage + 17) {
				_npcState[i].ttmPage++;
				break;
			}
			_npcState[i].byte12 = 5;
			_npcState[i].ttmPage = startPage + 11;
			break;
		case 1:
			if (_npcState[i].ttmPage < startPage + 8) {
				_npcState[i].ttmPage++;
			}
			break;
		default:
			break;
		}
	}

}


void DragonArcade::updateBoss() {
	_npcState[2].ttmPage++;
	if (29 < _npcState[2].ttmPage) {
		_npcState[2].ttmPage = 23;
	}

	int16 distToBoss = _npcState[1].x - _npcState[0].x;
	int16 absDistToBoss = abs(distToBoss);
	bool bossIsCloseY = abs(_npcState[1].y - _npcState[0].y) < 20;
	uint16 randVal = _nextRandomVal % 16;

	switch(_npcState[1].byte12 - 1) {
	case 0:
		if (bossIsCloseY && absDistToBoss < 45) {
			if (_bladeState1 != 8 && _bladeState1 != 9) {
				_npcState[1].byte12 = 5;
				_npcState[1].ttmPage = 30;
			}
		} else if (distToBoss < 0 || (_bossStateUpdateCounter < 0 && randVal == 7)) {
			_npcState[1].byte12 = 3;
			_npcState[1].ttmPage = 10;
			_bossStateUpdateCounter++;
		} else if ((bossIsCloseY && distToBoss < 70 && 0 < distToBoss && randVal == 15) || (0 < _bossStateUpdateCounter && randVal == 7)) {
			_npcState[1].byte12 = 2;
			_npcState[1].ttmPage = 3;
			_bossStateUpdateCounter--;
		} else if (_bossStateUpdateCounter == 0 && randVal == 15) {
			_npcState[1].byte12 = 4;
			_npcState[1].ttmPage = 17;
		}
		break;
	case 7:
		if (_npcState[1].ttmPage < 89) {
			_npcState[1].ttmPage++;
		} else {
			_npcState[1].byte12 = 9;
			_npcState[1].ttmPage = 67;
		}
		break;
	case 9:
		if (0x37 < _npcState[1].ttmPage) {
			decBossHealth();
		}
		if ((_npcState[1].ttmPage != 67 || (_nTickUpdates % 32) == 0) && _npcState[1].ttmPage < 74) {
			if (_npcState[1].ttmPage < 68) {
				_npcState[1].ttmPage++;
			} else if (!(_nTickUpdates & 1)) {
				setFinishCountdownIfLessThan0(15);
			} else {
				_npcState[1].ttmPage++;
			}
		}
		break;
	case 8:
		if ((_npcState[1].ttmPage != 67 || (_nTickUpdates % 32) == 0) && _npcState[1].ttmPage < 74) {
			playSfx(75);
			setFinishCountdownIfLessThan0(20);
			if (_npcState[1].ttmPage == 67) {
				_npcState[1].ttmPage = 68;
			} else if (_nTickUpdates & 1) {
				_npcState[1].ttmPage++;
			}
		}
		break;
	case 6:
		if (_npcState[1].ttmPage < 78) {
			_npcState[1].ttmPage++;
			break;
		}
		// FALL THROUGH
	case 3:
		if (_npcState[1].ttmPage < 22) {
			_npcState[1].ttmPage++;
		} else {
			_npcState[1].byte12 = 1;
			_npcState[1].ttmPage = 2;
		}
		break;
	case 2:
		if (_npcState[1].ttmPage < 16) {
			_npcState[1].ttmPage++;
			_npcState[1].xx += 6;
		} else {
			_npcState[1].byte12 = 1;
			_npcState[1].ttmPage = 2;
		}
		break;
	case 1:
		if (_npcState[1].ttmPage < 9) {
			_npcState[1].ttmPage++;
			_npcState[1].xx -= 6;
		} else {
			_npcState[1].byte12 = 5;
			_npcState[1].ttmPage = 30;
		}
		break;
	case 4:
		if (_npcState[1].ttmPage < 37) {
			_npcState[1].ttmPage++;
			if (bossIsCloseY && absDistToBoss < 50 && _uint0a17 == 0 && 33 < _npcState[1].ttmPage && _npcState[1].ttmPage < 37) {
				_npcState[0].byte12 = 10;
				_bladeState1 = 10;
				_npcState[0].ttmPage = 76;
				bladeTakeHit();
				if (_npcState[0].health == 0) {
					_shouldUpdateState = 3;
				}
			}
		} else {
			_npcState[1].byte12 = 1;
			_npcState[1].ttmPage = 2;
		}
		break;
	default:
		break;
	}

	// TODO: Is this supposed to be just be case 5 or apply to all cases?
	if (_npcState[3].byte12 == -2) {
		_npcState[3].health--;
		if (_npcState[3].health == 6) {
			_npcState[1].ttmPage = 49;
			_npcState[1].byte12 = 10;
		}
		if (_npcState[3].health == 0) {
			_npcState[3].byte12 = -3;
			_npcState[3].ttmPage = 39;
			_npcState[3].ttmNum = 1;
		}
	} else if (_npcState[3].byte12 == -3) {
		_npcState[3].ttmPage++;
		if (48 < _npcState[3].ttmPage)
			_npcState[3].byte12 = 0;
	}
}


static const int16 BOSS_2_PAGE_OFFSETS[] = { 2, 2, 14, 27, 33, 42, 52, 59 };

void DragonArcade::updateBoss2() {
	if (_someMoveDirection > 0 && 269 < _scrollXOffset + _npcState[0].x / 8 && _scrollXOffset < 282) {
		_scrollVelocityX = 1;
		_scrollXOffset++;
		_npcState[0].x -= 8;
	}

	if (_bladeState1 == 5) {
		return;
	}

	int distToBoss = _npcState[1].x - _npcState[0].x;
	int absDistToBoss = abs(distToBoss);

	switch (_npcState[1].byte12 - 1) {
	case 0:
		if (_npcState[1].x - _npcState[0].x < 1) {
			_uint0be6 = 0;
		} else {
			_uint0be6 = 31;
		}
		if ((_nextRandomVal % 16) == 15) {
			if (abs(_npcState[1].y - _npcState[0].y) > 35)
				return;

			_npcState[1].byte12 = 4;
			_npcState[1].ttmPage = _uint0be6 + 9;
		}
		if ((_nextRandomVal % 16) == 7 && absDistToBoss > 20 && _npcState[1].xx < 0x938) {
			_npcState[1].byte12 = 2;
			_npcState[1].ttmPage = _uint0be6 + 2;
		} else if (!_bladeHasFired || _npcState[1].xx < 0x939) {
			if (absDistToBoss < 30) {
				arcade34b4();
			}
		} else {
			_npcState[1].byte12 = 6;
			_npcState[1].ttmPage = _uint0be6 + 13;
		}
		break;
	case 4:
		if (_npcState[1].ttmPage < (_uint0be6 + 31)) {
			_npcState[1].ttmPage++;
		}
		break;
	case 3:
		if (_npcState[1].ttmPage < (_uint0be6 + 12)) {
			_npcState[1].ttmPage++;
		} else {
			_npcState[1].byte12 = 1;
			_npcState[1].ttmPage = _uint0be6 + 1;
		}
		break;
	case 1:
		_npcState[1].ttmPage++;
		if ((_uint0be6 + 8) <= _npcState[1].ttmPage) {
			_npcState[1].ttmPage = _uint0be6 + 2;
		}
		if (_uint0be6 == 0) {
			_npcState[1].xx += 8;
		} else {
			_npcState[1].xx -= 8;
		}
		if (absDistToBoss < 30) {
			arcade34b4();
		}
		break;
	case 5:
		if (_npcState[1].ttmPage < (int)(_uint0be6 + 20)) {
			_npcState[1].ttmPage++;
		} else {
			_npcState[1].byte12 = 7;
			_npcState[1].ttmPage = _uint0be6 + 21;
		}
		break;
	case 6:
		if (_npcState[1].ttmPage < (int)(_uint0be6 + 24)) {
			if (_nTickUpdates & 1) {
				_npcState[1].ttmPage++;
				if (_uint0be6 == 0) {
					_npcState[1].xx += 6;
				} else {
					_npcState[1].xx -= 6;
				}
			}
		} else if (absDistToBoss < 40 || _npcState[1].xx < 0x8d4) {
			_npcState[1].byte12 = 8;
			_npcState[1].ttmPage = _uint0be6 + 25;
		} else if (_nTickUpdates & 1) {
			_npcState[1].ttmPage = _uint0be6 + 21;
			if (_uint0be6 == 0) {
				_npcState[1].xx += 6;
			} else {
				_npcState[1].xx -= 6;
			}
		}
		break;
	case 7:
		if (_npcState[1].ttmPage < _uint0be6 + 27) {
			_npcState[1].ttmPage++;
		} else if (absDistToBoss < 40) {
			arcade34b4();
		} else {
			_npcState[1].byte12 = 1;
			_npcState[1].ttmPage = _uint0be6 + 1;
		}
		break;
	case 2:
	default:
		if (_stillLoadingScriptsMaybe) {
			if ((_scrollVelocityX == -1 && _npcState[1].x < 0x96) || (_scrollVelocityX == 1 && 160 < _npcState[1].x)) {
				updateXScrollOffset();
			}
			byte bossByte12 = _npcState[1].byte12;
			// Note: these never get changed? they're items in INT_TABLE_0BCE
			const int16 INT_39e5_0be0 = 58;
			const int16 INT_39e5_0bda = 32;
			if (bossByte12 == 100) {
				int16 absRand;
				if (_mouseButtonWentDown == 1) {
					_mouseButtonWentDown = 0;
					if (_npcState[1].health == 1) {
						absRand = 6;
					} else {
						while (true) {
							absRand = abs(_nextRandomVal % 4) + 4;
							if (absRand != 6)
								break;
							_nextRandomVal = _getNextRandom();
						}
					}
					if (absRand == 5 || absRand == 6 || absRand == 7) {
						decBossHealthAndCheck();
					}
					_npcState[1].ttmPage = BOSS_2_PAGE_OFFSETS[absRand] + 65;
					_npcState[1].byte12 = absRand + 100;
				} else {
					while (absRand = abs(_nextRandomVal % 16), absRand != 0 && absRand < 4) {
						bool hitBlade = false;
						if (_npcState[0].health <= _startDifficultyMaybe + 2) {
							absRand = 3;
							hitBlade = true;
						}
						if (absRand != 3)
							hitBlade = true;

						if (hitBlade) {
							_npcState[1].ttmPage = BOSS_2_PAGE_OFFSETS[absRand] + 65;
							_npcState[1].byte12 = absRand + 100;
							for (int16 i = _startDifficultyMaybe + 2; i != 0; i--)
								bladeTakeHit();
							return;
						}
						_nextRandomVal = _getNextRandom();
					}
				}
			} else if ((bossByte12 != 106 || INT_39e5_0be0 + 0x41 != _npcState[1].ttmPage) &&
					   (bossByte12 != 103 || INT_39e5_0bda + 0x41 != _npcState[1].ttmPage)) {
				// code uses 0x0b0c + byte12, but table that's actually used starts at 0xbce
				static const int16 INT_TABLE_0BCE[] = {0x2a, 0x34, 0x3b, 0x2, 0xD, 0x1A, 0x20, 0x29, 0x33, 0x3A, 0x42};
				assert(bossByte12 >= 97 && bossByte12 - 97 < ARRAYSIZE(INT_TABLE_0BCE));
				if (INT_TABLE_0BCE[bossByte12 - 97] + 0x41 == _npcState[1].ttmPage) {
					_npcState[1].byte12 = 100;
					_npcState[1].ttmPage = 67;
				} else if (bossByte12 != 100 && _nTickUpdates & 1) {
					_npcState[1].ttmPage++;
				}
			}
		}
	}
}

void DragonArcade::updateXScrollOffset() {
	int16 lastScrollOffset = _scrollXOffset;
	_scrollXOffset = CLIP(_scrollXOffset + _scrollVelocityX, 0, 282);
	if (lastScrollOffset != _scrollXOffset) {
		_scrollXIncrement += _scrollVelocityX;
	}
}

void DragonArcade::arcade34b4() {
	_npcState[0].ttmPage = -1;
	if (_npcState[0].x < 150) {
		_scrollVelocityX = -1;
	} else if (_npcState[0].x < 161) {
		_scrollVelocityX = 0;
	} else {
		_scrollVelocityX = 1;
	}

	_someMoveDirection = -1;
	// TODO: what is this?
	// UINT_39e5_0a1f = 1;
	_stillLoadingScriptsMaybe = true;
	_npcState[0].byte12 = -1;
	_npcState[1].byte12 = 100;
	_npcState[1].ttmPage = 67;
	_npcState[1].xx = _npcState[0].xx;
	_npcState[1].ttmNum = 2;
}

void DragonArcade::decBossHealth() {
	if (_npcState[1].health) {
		_npcState[1].health--;
	}
}

void DragonArcade::decBossHealthAndCheck() {
	if (_npcState[1].health) {
		_npcState[1].health--;
		if (!_npcState[1].health) {
			// boss is dead!
			if (_npcState[1].ttmPage < 32) {
				_npcState[1].ttmPage = 28;
			} else {
				_npcState[1].ttmPage = 59;
			}
			_npcState[1].byte12 = 5;
			setFinishCountdownIfLessThan0(20);
		}
	}
}

void DragonArcade::bladeTakeHit() {
	if (_npcState[0].health) {
		_npcState[0].health--;
	}

	if (!_enemyHasSmallGun && _npcState[0].health) {
		_npcState[0].health--;
	}

	if (_npcState[0].health == 0) {
		/* dead! */
		playSfx(0x4b);
		if ((_bladeState1 == 0 && _bladePageOffset + 28 <= _npcState[0].ttmPage &&
			 _npcState[0].ttmPage <= _bladePageOffset + 35) || _bladeState1 == 4) {
			_bladeState1 = 9;
			_npcState[0].ttmPage = _bladePageOffset + 103;
		} else {
			_bladeState1 = 8;
			_npcState[0].ttmPage = _bladePageOffset + 98;
		}
		setFinishCountdownIfLessThan0(15);
		_npcState[0].ttmNum = 0;
		_mouseButtonWentDown = 0x80;
	} else {
		playSfx(0x29);
	}
}


void DragonArcade::initIfNeeded() {
	if (_initFinished)
		return;

	DgdsEngine *engine = DgdsEngine::getInstance();
	engine->disableKeymapper();

	const char *ttmName;
	const char *scrollBmpName;
	const char *songName;
	if (_nextStage == 4) {
		ttmName = "path2.ttm";
		scrollBmpName = "scroll2.bmp";
		songName = "sarcade.sng";
	} else {
		ttmName = "path1.ttm";
		scrollBmpName = "scroll.bmp";
		songName = "darcade.sng";
	}

	engine->getGamePals()->loadPalette("arcade.pal");
	_scrollImg.reset(new Image(engine->getResourceManager(), engine->getDecompressor()));
	_scrollImg->loadBitmap(scrollBmpName);

	_arcadeTTM.clearDataPtrs();
	_arcadeTTM._currentTTMNum = 0;
	int16 envNum = _arcadeTTM.load(ttmName);
	_arcadeTTM.finishTTMParse(envNum);
	_arcadeTTM._doingInit = true;
	for (int i = 0; i < 8; i++) {
		_arcadeTTM.runNextPage(i + 1);
	}
	_arcadeTTM._doingInit = false;
	_arcadeTTM.freePages(0);

	_arcadeTTM.freeShapes();
	_arcadeTTM._currentTTMNum = 0;

	const char *bladeTTM = _haveBigGun ? "BIGUNBLA.TTM" : "BLADE.TTM";

	envNum = _arcadeTTM.load(bladeTTM);
	_arcadeTTM.finishTTMParse(envNum);
	_arcadeTTM.runNextPage(0);

	_bulletImg.reset(new Image(engine->getResourceManager(), engine->getDecompressor()));
	_bulletImg->loadBitmap("bullet.bmp");

	_arrowImg.reset(new Image(engine->getResourceManager(), engine->getDecompressor()));
	_arrowImg->loadBitmap("arcade.bmp");

	engine->_soundPlayer->loadMusic(songName);
	engine->_soundPlayer->playMusic(0);
	// set font to 0?
	// set text draw to 0xe?
	drawBackgroundAndWeapons();
	loadTTMScriptsForStage(_nextStage);
	_initFinished = true;
	_attemptCounter = 0;
	g_system->warpMouse(166, 158);
	_dontRedrawBgndAndWeapons = true;
	redraw();
}

void DragonArcade::updateBladeWithInputs() {
	if (_stillLoadingScriptsMaybe)
		return;

	if (_int0b5a != 0) {
		_int0b5a--;
		if (_int0b5a == 0)
			_int0b58 = 0;
	}

	if ((_bladeHorizMoveAttempt & kBladeMoveLeft) == kBladeMoveNone) {
		_bladePageOffset = 0;
	} else {
		_bladePageOffset = 122;
	}
	if (_bladeState1 == 0) {
		if (!_dontMoveBladeFlag) {
			_npcState[0].ttmPage++;
		}
		handleMouseStates();
		return;
	}

	int16 newPage = 0;

	switch (_bladeState1 - 1) {
	case 0:
		if (_bladePageOffset + 26 == _npcState[0].ttmPage) {
			_npcState[0].ttmPage = _bladePageOffset + 22;
		}
		newPage = _bladePageOffset + 25;
		break;
	case 1:
		if (_bladePageOffset + 62 == _npcState[0].ttmPage) {
			_npcState[0].ttmPage = _bladePageOffset + 56;
		}
		newPage = _bladePageOffset + 61;
		if ((_bladePageOffset + 50 < _npcState[0].ttmPage) &&
			(_npcState[0].ttmPage <= _bladePageOffset + 56)) {
			moveBladeX();
		}
		break;
	case 2:
		newPage = _bladePageOffset + 123;
		break;
	case 3:
		newPage = _bladePageOffset + 44;
		break;
	case 4:
		_npcState[0].ttmPage = _bladePageOffset + 64;
		moveBladeX();
		newPage = 999;
		break;
	case 5:
		newPage = _bladePageOffset + 97;
		break;
	case 6:
		newPage = _bladePageOffset + 77;
		if (_bladePageOffset == 0) {
			_scrollVelocityX = 1;
		} else {
			_scrollVelocityX = -1;
		}
		if (newPage <= _npcState[0].ttmPage + 2 && _npcState[0].ttmPage <= newPage) {
			_npcState[0].x = _npcState[0].x + 4;
		}
		break;
	case 7:
		if (_npcState[0].ttmPage < _bladePageOffset + 102) {
			_npcState[0].ttmPage++;
		} else {
			_npcState[0].ttmPage = _bladePageOffset + 102;
		}
		return;
	case 8:
		if (_npcState[0].ttmPage < _bladePageOffset + 108) {
			_npcState[0].ttmPage++;
		} else {
			_npcState[0].ttmPage = _bladePageOffset + 108;
		}
		return;
	case 9:
		if (_npcState[0].ttmPage < 79) {
			_npcState[0].ttmPage++;
			_scrollVelocityX = -1;
			moveBladeX();
		} else {
			handleMouseStates();
		}
		return;
	case 10:
		if (_npcState[0].ttmPage < (_haveBigGun ? 25 : 14)) {
			_npcState[0].ttmPage++;
		} else {
			_npcState[3].xx = _npcState[0].xx + 22;
			_npcState[3].yy = -32;
			_npcState[3].byte12 = -2;
			_npcState[3].ttmPage = 33;
			_npcState[3].ttmNum = 2;
			_npcState[3].health = 20;
			_npcState[0].ttmNum = 0;
			handleMouseStates();
		}
		return;
	case 11:
		if (!_haveBigGun) {
			newPage = 40;
		} else {
			newPage = 31;
		}
		if (newPage < _npcState[0].ttmPage + 1) {
			_npcState[0].ttmPage = newPage;
		} else {
			_npcState[0].ttmPage++;
		}
		return;
	case 12:
		if (!(_nTickUpdates & 1)) {
			return;
		}
		_npcState[0].ttmPage++;
		if (_haveBigGun) {
			if (_npcState[0].ttmPage > 61)
				_npcState[0].ttmPage = 61;
		} else {
			if (_npcState[0].ttmPage > 57) {
				_npcState[0].ttmPage = 57;
			}
		}
		return;
	case 13:
		if (!(_nTickUpdates & 1)) {
			return;
		}
		if (_npcState[0].ttmPage + 1 < 38) {
			_npcState[0].ttmPage++;
		} else {
			_npcState[0].ttmPage = 34;
		}
		return;
	default:
		break;
	}

	if (!_dontMoveBladeFlag)
		_npcState[0].ttmPage++;

	if (newPage < _npcState[0].ttmPage) {
		handleMouseStates();
	} else if (_uint0a17 == 0) {
		if (_bladeState1 == 1) {
			if (_bladePageOffset + 22 == _npcState[0].ttmPage) {
				_ttmYAdjust = _int0b58 * -4;
				_uint0a17 = 1;
			}
		} else if (_bladeState1 == 2 && _bladePageOffset + 56 == _npcState[0].ttmPage) {
			_ttmYAdjust = _int0b58 * -4;
			_uint0a17 = 1;
		}
	} else {
		if (_mouseButtonWentDown == 1) {
			_mouseButtonWentDown = 0;
			if (_bladeState1 == 2) {
				_npcState[0].ttmPage = _bladePageOffset + 63;
			} else if (_bladeState1 == 1) {
				_npcState[0].ttmPage = _bladePageOffset + 27;
			}
		}
		if (_bladeState1 == 1 || _bladeState1 == 2 || _bladeState1 == 5) {
			_ttmYAdjust += 2;
			_arcadeTTM._startYOffset += _ttmYAdjust;
			_npcState[0].ttmPage--;
		}
	}
}

void DragonArcade::moveBladeX() {
	if (_dontMoveBladeFlag)
		return;

	if (_someMoveDirection != 0)
		return;

	if (_scrollVelocityX < 0) {
		if (_scrollXOffset == 0) {
			if (_npcState[0].x > 0)
				_bladeXMove = -4;
		} else if (_npcState[0].x > 260) {
			_bladeXMove = -4;
			_int0b60 = 0;
		} else if (_npcState[0].x < 160) {
			_int0b60 = 1;
			_bladeXMove = 4;
			updateXScrollOffset();
		} else if (_int0b60 == 1) {
			updateXScrollOffset();
			_bladeXMove = 4;
		} else {
			_bladeXMove = -4;
		}
	} else if (_scrollVelocityX > 0) {
		if (_scrollXOffset == 282) {
			if (_npcState[0].x < SCREEN_WIDTH)
				_bladeXMove = 4;

		} else if (0xa0 < _npcState[0].x) {
			_int0b60 = -1;
			_bladeXMove = -4;
			updateXScrollOffset();
		} else if (_npcState[0].x < 60) {
			_int0b60 = 0;
			_bladeXMove = 4;
		} else if (_int0b60 != -1) {
			_bladeXMove = 4;
		} else {
			updateXScrollOffset();
			_bladeXMove = -4;
		}
	}
	_npcState[0].x += _bladeXMove;
}

void DragonArcade::handleMouseStates() {
	if (_mouseButtonWentDown == 0) {
		_bladeState1 = 0;
		if ((_bladeMoveFlag & (kBladeMoveRight | kBladeMoveLeft)) == kBladeMoveNone) {
			/* not moving up or down */
			if ((_bladeMoveFlag & kBladeMoveDown) == kBladeMoveNone) {
				_npcState[0].ttmPage = _bladePageOffset + 2;
			} else {
				_npcState[0].ttmPage = _bladePageOffset + 14;
			}
		} else {
			moveBladeX();
			if (!_foundFloorFlag) {
				if ((_npcState[0].ttmPage < _bladePageOffset + 109) ||
					(_bladePageOffset + 112 < _npcState[0].ttmPage)) {
					_npcState[0].ttmPage = _bladePageOffset + 109;
				}
			} else if ((_bladeMoveFlag & kBladeMoveDown) == kBladeMoveNone) {
				if ((_npcState[0].ttmPage < _bladePageOffset + 3) ||
					(_bladePageOffset + 10 < _npcState[0].ttmPage)) {
					_npcState[0].ttmPage = _bladePageOffset + 3;
				}
			} else if ((_npcState[0].ttmPage < _bladePageOffset + 28) ||
					   (_bladePageOffset + 35 < _npcState[0].ttmPage)) {
				_npcState[0].ttmPage = _bladePageOffset + 28;
			}
		}
	} else if (_mouseButtonWentDown == 1) {
		if (_loadedArcadeStage == 3 && _haveBomb && _npcState[1].health != 0 &&
			  25 < abs(_npcState[1].y - _npcState[0].y) && abs(_npcState[1].x - _npcState[0].x) < 40) {
			// use a bomb
			_bladeState1 = 11;
			_haveBomb = false;
			_npcState[0].ttmNum = 2;
			if (!_haveBigGun) {
				_npcState[0].ttmPage = 4;
			} else {
				_npcState[0].ttmPage = 15;
			}
		} else if ((_bladeMoveFlag & kBladeMoveDown) == kBladeMoveNone) {
			_bladeState1 = 3;
			_npcState[0].ttmPage = _bladePageOffset + 113;
		} else {
			_bladeState1 = 4;
			_npcState[0].ttmPage = _bladePageOffset + 36;
		}
	} else if (_mouseButtonWentDown == 2) {
		if ((_bladeMoveFlagBeforeRButton & (kBladeMoveLeft | kBladeMoveRight)) == kBladeMoveNone) {
			_bladeMoveFlag = static_cast<DragonBladeMoveFlag>(_bladeMoveFlagBeforeRButton | (_bladeHorizMoveAttempt & (kBladeMoveLeft | kBladeMoveRight)));
		} else {
			_bladeMoveFlag = _bladeMoveFlagBeforeRButton;
			_bladeHorizMoveAttempt = _bladeMoveFlagBeforeRButton;
		}

		if ((_bladeMoveFlag & kBladeMoveUp) == kBladeMoveNone) {
			if (_int0b58 < 4) {
				_int0b58++;
			} else {
				_int0b58 = 4;
			}
		} else {
			_int0b58 = 4;
		}

		_int0b5a = 0;
		_scrollVelocityX = 0;
		if ((_bladeMoveFlag & kBladeMoveLeft) == kBladeMoveNone) {
			_bladePageOffset = 0;
		} else {
			_bladePageOffset = 122;
		}

		if ((_bladeMoveFlagBeforeRButton & (kBladeMoveLeft | kBladeMoveRight)) == kBladeMoveNone) {
			// Not jumping left or right
			_bladeState1 = 1;
			_npcState[0].ttmPage = _bladePageOffset + 15;
			debug("Move: blade jump up -> ttm %d", _npcState[0].ttmPage);
		} else {
			// Jump to left or right
			if ((_bladeMoveFlag & kBladeMoveLeft) == kBladeMoveNone) {
				_scrollVelocityX = 1;
			} else {
				_scrollVelocityX = -1;
			}
			_isMovingStage = false;
			_bladeState1 = 2;
			_npcState[0].ttmPage = _bladePageOffset + 45;
			debug("Move: blade jump up -> ttm %d velocity %d", _npcState[0].ttmPage, _scrollVelocityX);
		}

		if ((_bladeMoveFlagBeforeRButton & kBladeMoveDown) != kBladeMoveNone) {
			findFloorMinGT();
			if (!isFloorNotFound()) {
				if (_bladeState1 == 2) {
					_npcState[0].ttmPage = _bladePageOffset + 56;
				} else {
					_npcState[0].ttmPage = _bladePageOffset + 22;
				}
				debug("Move: blade jump down -> ttm %d", _npcState[0].ttmPage);
				_arcadeTTM._startYOffset++;
				_uint0a17++;
				_currentYOffset = _arcadeTTM._startYOffset;
			}
		}
		playSfx(0x54);
		_bladeMoveFlagBeforeRButton = kBladeMoveNone;
	}
	_mouseButtonWentDown = 0;
}

void DragonArcade::resetStageState() {
	clearAllNPCStates();
	clearAllBulletStates();
	_scrollXOffset = 0;
	_nTickUpdates = 0;
	_isMovingStage = false;
	_ttmYAdjust = 0;
	_uint0a17 = 0;
	_shouldUpdateState = 0;
	_someMoveDirection = 0;
	_npcState[0].byte12 = 1;
	_npcState[0].ttmNum = 0;
	_npcState[0].health = (4 - _startDifficultyMaybe) * 3 + 20;
	_npcState[0].ttmPage = 3;
	_npcState[1].health = 0;
	_lastDrawnBladeHealth = -1;
	_lastDrawnBossHealth = -1;
	_bladeState1 = 0;
	_mouseButtonWentDown = 0;
	_bladeMoveFlag = kBladeMoveNone;
}

static const int16 STAGE_0_NPC_XX[] = {
	0x191, 0x1BB, 0x1F5,
	0x25B, 0x2D3, 0x341,
	0x535, 0x5C9, 0x623
};

static const int16 STAGE_0_NPC_YY[] = {
	0, 0, 0, -40, -40, 0, 0, 0, -40
};

static const byte STAGE_0_NPC_BYTE12[] = {
	5, 4, 4, 5, 4, 5, 4, 5, 5
};

static const int16 STAGE_4_NPC_XX_1[] = {
	0x169, 0x19D, 0x1B9,
	0x30D, 0x32D, 0x457,
	0x4DB, 0x501, -1
};

static const int16 STAGE_4_NPC_YY_1[] = {
	0, 0, 0, 0, 0, -40, 0, 0, -40
};

static const byte STAGE_4_NPC_BYTE12_1[] = {
	5, 4, 4, 5, 4, 5, 5, 4, 5
};

void DragonArcade::initValuesForStage() {
	for (int i = 9; i != 0; i--)
		_npcState[i].byte12 = 0;

	switch (_loadedArcadeStage) {
	case 0:
		for (int i = 1; i < 10; i++) {
			_npcState[i].xx = STAGE_0_NPC_XX[i - 1];
			_npcState[i].yy = STAGE_0_NPC_YY[i - 1];
			_npcState[i].byte12 = STAGE_0_NPC_BYTE12[i - 1];
			if (_npcState[i].byte12 == 5)
				_npcState[i].ttmPage = 39;
			else
				_npcState[i].ttmPage = 30;
			_npcState[i].ttmNum = 1;
		}
		initValuesForStage0();
		break;
	case 3:
		initValuesForStage3();
		break;
	case 4:
		// Note: The original also only does 8 NPCs here even though the arrays have
		// 9 values in them (note the last x value is -1).
		for (int i = 1; i < 9; i++) {
			_npcState[i].xx = STAGE_4_NPC_XX_1[i - 1];
			_npcState[i].yy = STAGE_4_NPC_YY_1[i - 1];
			_npcState[i].byte12 = STAGE_4_NPC_BYTE12_1[i - 1];
			if (_npcState[i].byte12 == 5)
				_npcState[i].ttmPage = 39;
			else
				_npcState[i].ttmPage = 30;
			_npcState[i].ttmNum = 1;
		}
		initValuesForStage4();
		break;
	case 6:
		initValuesForStage6();
		break;
	default:
		break;
	}
}

static const int16 STAGE_0_NPC2_XX_1[] = {
	0x13F, 0x150, 0x161, 0x172
};

static const int16 STAGE_0_NPC2_XX_2[] = {
	0x317, 0x328, 0x339, 0x34A
};

static const int16 STAGE_0_NPC2_TTMPAGE[] = {
	 0, 30, 15, 0
};

void DragonArcade::initValuesForStage0() {
	_npcStateResetCounter = 0;
	for (int i = 10; i < 14; i++) {
		_npcState[i].xx = STAGE_0_NPC2_XX_1[i - 10];
		_npcState[i].yy = 2;
		_npcState[i].ttmPage = STAGE_0_NPC2_TTMPAGE[i - 10];
		_npcState[i].ttmNum = 2;

		_npcState[i + 4].xx = STAGE_0_NPC2_XX_2[i - 10];
		_npcState[i + 4].yy = -37;
		_npcState[i + 4].ttmPage = STAGE_0_NPC2_TTMPAGE[i - 10];
		_npcState[i + 4].ttmNum = 2;
	}
	_flag40ee = true;
	_flag40ef = true;
	_npcState[18].xx = 0x11f;
	_npcState[18].yy = -13;
	_npcState[18].byte12 = 30;
	_npcState[18].ttmPage = 32;
	_npcState[18].ttmNum = 2;
}

void DragonArcade::initValuesForStage3() {
	clearAllNPCStates();
	_bossStateUpdateCounter = 0;
	_npcState[1].xx = 0x99c;
	_npcState[1].yy = -54;
	_npcState[1].byte12 = 1;
	_npcState[1].ttmPage = 2;
	_npcState[1].health = 20;
	_npcState[1].ttmNum = 1;
	_npcState[1].y = 300;
	_npcState[2].xx = 0x9b2;
	_npcState[2].yy = -57;
	_npcState[2].byte12 = -1;
	_npcState[2].ttmPage = 23;
	_npcState[2].health = 0;
	_npcState[2].ttmNum = 1;
}

static const int16 STAGE_4_NPC_XX[] = {
	0x1F9, 0x551, 0x362, 0x592, 0x7AF
};

static const int16 STAGE_4_NPC_YY[] = {
	8, 8, 6, 6, 6
};

static const int16 STAGE_4_ST_TTMPAGE[] = {
	0, 0, 15, 15, 40
};

static const byte STAGE_4_ST_BYTE12[] = {
	0, 0, 0xfa, 0xfa, 0xf9
};

void DragonArcade::initValuesForStage4() {
	_npcStateResetCounter = 0;
	for (int i = 10; i < 15; i++) {
		_npcState[i].xx = STAGE_4_NPC_XX[i - 10];
		_npcState[i].yy = STAGE_4_NPC_YY[i - 10];
		_npcState[i].ttmPage = STAGE_4_ST_TTMPAGE[i - 10];
		_npcState[i].byte12 = STAGE_4_ST_BYTE12[i - 10];
		_npcState[i].health = 1;
		_npcState[i].ttmNum = 2;
	}
}

void DragonArcade::initValuesForStage6() {
	clearAllNPCStates();
	_npcState[1].xx = 0x9e2;
	_npcState[1].yy = -3;
	_npcState[1].ttmPage = 1;
	_npcState[1].health = 10;
	_npcState[1].ttmNum = 1;
	_npcState[1].byte12 = 1;
	_stillLoadingScriptsMaybe = false;
}

void DragonArcade::setFinishCountdownIfLessThan0(int16 val) {
	if (_finishCountdown < 0)
		_finishCountdown = val;
}

void DragonArcade::arcadeTick() {
	DragonGlobals *globals = static_cast<DragonGlobals *>(DgdsEngine::getInstance()->getGameGlobals());
	int16 arcadeState = globals->getArcadeState();

	switch (arcadeState) {
	case 0:
		return;
	case 5: {
		initIfNeeded();
		if (doTickUpdate())
			return;

		if (_shouldUpdateState == 0) {
			globals->setArcadeState(6);
			return;
		}
		_attemptCounter++;
		checkToOpenMenu();
		globals->setArcadeState(0);
		return;
	}
	case 6:
	case 7:
	case 8:
	case 9:
		finish();
		return;
	case 10:
		// Restart? No.
		fadeInAndClearScreen();
		finish();
		globals->setArcadeState(_shouldUpdateState + 6);
		return;
	case 20:
		// Restart? Yes.
		globals->setArcadeState(30);
		return;
	case 30:
		// Do (re)start
		loadTTMScriptsForStage(_nextStage);
		// These don't seem to ever be used?
		// UINT_39e5_0d0e = 0;
		// UINT_39e5_0d10 = 0;
		globals->setArcadeState(5);
		//_arcadeNeedsBufferCopy = true;
		//flagInventoryOpened = false;
		return;
	default:
		_haveBomb = arcadeState > 20;
		if (_haveBomb) {
			arcadeState -= 20;
			globals->setArcadeState(arcadeState);
		}

		_enemyHasSmallGun = arcadeState > 10;
		if (_enemyHasSmallGun) {
			arcadeState -= 10;
			globals->setArcadeState(arcadeState);
		}

		_haveBigGun = arcadeState > 2;
		if (_haveBigGun) {
			arcadeState -= 2;
			globals->setArcadeState(arcadeState);
		}

		_nextStage = (arcadeState & 1) ? 4 : 0;

		globals->setArcadeState(5);
		return;
	}
}

void DragonArcade::loadTTMScriptsForStage(uint16 stage) {
	const char *ttm1;
	const char *ttm2;

	switch(stage) {
	case 0:
		resetStageState();
		ttm1 = "STATIONA.TTM";
		ttm2 = "FLAMDEAD.TTM";
		_npcState[0].x = 160;
		_npcState[0].xx = 160;
		_arcadeTTM._startYOffset = 0;
		break;
	case 3:
		ttm1 = "DRAGON.TTM";
		ttm2 = "GRENADE.TTM";
		break;
	case 4:
		resetStageState();
		ttm1 = "STATIONA.TTM";
		ttm2 = "AARC.TTM";
		_npcState[0].x = 140;
		_npcState[0].xx = 140;
		_arcadeTTM._startYOffset = -43;
		break;
	case 6:
		_arcadeTTM._currentNPCRunningTTM = 0;
		_arcadeTTM.runNextPage(276);
		ttm1 = "SNAKERUN.TTM";
		if (_haveBigGun)
			ttm2 = "BIGFIGHT.TTM";
		else
			ttm2 = "LITFIGHT.TTM";
		break;
	default:
		return;
	}

	if (stage != _loadedArcadeStage) {
		int16 envNum;
		_arcadeTTM._currentTTMNum = 1;
		_arcadeTTM.freeShapes();
		_arcadeTTM.freePages(1);
		// original also clears data pointers here, but we do that in freePages()
		_arcadeTTM._currentTTMNum = 2;
		_arcadeTTM.freeShapes();
		_arcadeTTM.freePages(2);
		_arcadeTTM._currentTTMNum = 1;
		envNum = _arcadeTTM.load(ttm1);
		_arcadeTTM.finishTTMParse(envNum);
		_arcadeTTM.runNextPage(0);
		_arcadeTTM._currentTTMNum = 2;
		envNum = _arcadeTTM.load(ttm2);
		_arcadeTTM.finishTTMParse(envNum);
		_arcadeTTM.runNextPage(0);
	}

	_currentYOffset = _arcadeTTM._startYOffset;
	_finishCountdown = -1;
	_stillLoadingScriptsMaybe = 0;
	_loadedArcadeStage = stage;
	initValuesForStage();
}

void DragonArcade::fadeInAndClearScreen() {
	DgdsEngine *engine = DgdsEngine::getInstance();
	for (int fade = 63; fade > 0; fade--) {
		engine->getGamePals()->setFade(0, 255, 0, fade * 4);
		g_system->updateScreen();
		g_system->delayMillis(5);
	}
	Common::Rect screenRect(SCREEN_WIDTH, SCREEN_HEIGHT);
	engine->getBackgroundBuffer().fillRect(screenRect, 0);
	engine->_compositionBuffer.fillRect(screenRect, 0);
}

void DragonArcade::drawBackgroundAndWeapons() {
	DgdsEngine *engine = DgdsEngine::getInstance();
	Image bg(engine->getResourceManager(), engine->getDecompressor());
	bg.drawScreen("BGND.SCR", engine->getBackgroundBuffer());

	Image weapons(engine->getResourceManager(), engine->getDecompressor());
	weapons.loadBitmap("W.BMP");
	if (weapons.loadedFrameCount() < 3)
		error("Dragon Arcade: Expect 3 frames in w.bmp");

	// Offsets are customized and hard-coded depending on weapon combination
	// Frames are 0 = big gun, 1 = pistol, 2 = bomb
	Graphics::ManagedSurface &dst = engine->getBackgroundBuffer();
	const Common::Rect screen(SCREEN_WIDTH, SCREEN_HEIGHT);
	if (!_haveBigGun && !_haveBomb) {
		weapons.drawBitmap(1, 267, 160, screen, dst);
	} else if (_haveBigGun && !_haveBomb) {
		weapons.drawBitmap(0, 249, 159, screen, dst);
	} else if (!_haveBigGun && _haveBomb) {
		weapons.drawBitmap(1, 258, 155, screen, dst);
		weapons.drawBitmap(2, 289, 165, screen, dst);
	} else {
		// have big gun and have bomb
		weapons.drawBitmap(0, 246, 153, screen, dst);
		weapons.drawBitmap(2, 295, 166, screen, dst);
	}
}

void DragonArcade::drawBulletHitCircles(uint16 x, uint16 y, bool colorFlag) {
	static const byte COLORS[2][3] = { {0, 1, 9}, {0, 4, 12} };

	Graphics::ManagedSurface &dst = DgdsEngine::getInstance()->_compositionBuffer;
	for (int i = 0; i < 3; i++) {
		byte col = COLORS[colorFlag][i];
		Drawing::filledCircle(x, y, 4 - i, 4 - i, &dst, col, col);
	}
}

void DragonArcade::checkToOpenMenu() {
	if (_attemptCounter < 5) {
		// Open menu 45, hightlight widget 139
		DgdsEngine::getInstance()->setMenuToTrigger(kMenuReplayArcade);
	} else {
		// Open menu 47, hightlight widget 148
		DgdsEngine::getInstance()->setMenuToTrigger(kMenuArcadeFrustrated);
	}
}

void DragonArcade::clearAllNPCStates() {
	for (uint i = 1; i < ARRAYSIZE(_npcState); i++) {
		_npcState[i].byte12 = 0;
		_npcState[i].ttmPage = -1;
	}
}

void DragonArcade::clearAllBulletStates() {
	for (uint i = 0; i < ARRAYSIZE(_bullets); i++) {
		_bullets[i]._state = kBulletInactive;
	}
}

void DragonArcade::createBullet(int16 x, int16 y, ImageFlipMode flipMode, int16 bulletType) {
	for (uint i = 0; i < ARRAYSIZE(_bullets); i++) {
		if (_bullets[i]._state == kBulletInactive) {
			_bullets[i]._state = kBulletFlying;
			_bullets[i]._x = x;
			_bullets[i]._y = y;
			_bullets[i]._flipMode = flipMode;
			_bullets[i]._bulletType = bulletType;
			if (bulletType == 3)
				_bullets[i]._ySpeed = _nextRandomVal & 3;

			break;
		}
	}
}

void DragonArcade::playSfx(int16 num) const {
	DgdsEngine::getInstance()->_soundPlayer->playSFX(num);
}

void DragonArcade::bladeTakeHitAndCheck() {
	if (_npcState[0].health)
		_npcState[0].health--;
	if (!_enemyHasSmallGun && _npcState[0].health)
		_npcState[0].health--;
	if (_npcState[0].health <= 0) {
		playSfx(75);
		if ((_bladeState1 == 0 && _bladePageOffset + 28 < _npcState[0].ttmPage && _npcState[0].ttmPage <= 35)
			|| _bladeState1 == 4) {
			_bladeState1 = 9;
			_npcState[0].ttmPage = _bladeState1 + 103;
		} else {
			_bladeState1 = 8;
			_npcState[0].ttmPage = _bladeState1 + 98;
		}
		setFinishCountdownIfLessThan0(15);
		_npcState[0].ttmNum = 0;
		_mouseButtonWentDown = 0x80;
	} else {
		playSfx(41);
	}
}

void DragonArcade::drawHealthBars() {
	DgdsEngine *engine = DgdsEngine::getInstance();

	// Note: the original here checks _npcState[0].health vs _lastDrawnBladeHealth
	// and _npcState[1].health vs _lastDrawnBossHealth to avoid redrawing every time,
	// but we clear the screen every time so just redraw it each time.

	const Common::Rect clearRect1(Common::Point(10, 155), 64, 10);
	engine->_compositionBuffer.fillRect(clearRect1, 0);

	for (int i = 1; i <= _npcState[0].health; i++) {
		int x = 8 + i * 2;
		engine->_compositionBuffer.drawLine(x, 155, x, 162, 12);
	}

	_lastDrawnBladeHealth = _npcState[0].health;

	if ((_loadedArcadeStage == 3 || _loadedArcadeStage == 6) || _lastDrawnBossHealth == -1) {
		const Common::Rect clearRect2(Common::Point(10, 167), 60, 8);
		engine->_compositionBuffer.fillRect(clearRect2, 0);

		byte color = (_loadedArcadeStage == 3) ? 2 : 9;

		for (int i = 1; i <= _npcState[1].health; i++) {
			int x = 8 + i * 2;
			engine->_compositionBuffer.drawLine(x, 167, x, 174, color);
		}
		_lastDrawnBossHealth = _npcState[1].health;
	}
}

void DragonArcade::redraw() {
	if (!_dontRedrawBgndAndWeapons)
		drawBackgroundAndWeapons();

	drawScrollBmp();
	runThenDrawBulletsInFlight();
	_lastDrawnBladeHealth = -1;
	_lastDrawnBossHealth = -1;
	drawHealthBars();
	// TODO: What are these?
	//UINT_39e5_0d10 = 0;
	//UINT_39e5_0d0e = 0;
	_dontRedrawBgndAndWeapons = 0;
	g_system->warpMouse(166, 158);
}

void DragonArcade::drawScrollBmp() {
	const Common::Rect drawWin(Common::Point(8, 8), SCREEN_WIDTH - 16, 117);
	Graphics::ManagedSurface &dst = DgdsEngine::getInstance()->_compositionBuffer;
	_scrollImg->drawScrollBitmap(drawWin.left, drawWin.top, drawWin.width(), drawWin.height(),
						_scrollXOffset, 0, drawWin, dst);
}


void DragonArcade::runThenDrawBulletsInFlight() {
	_arcadeTTM.runPagesForEachNPC(_scrollXOffset);

	const Common::Rect drawWin(Common::Point(8, 8), SCREEN_WIDTH - 16, 117);
	_arcadeTTM._currentTTMNum = _npcState[0].ttmNum;
	_npcState[0].x_11 = 0;
	_npcState[0].x_12 = 0;
	_npcState[0].x_21 = 0;
	_npcState[0].x_22 = 0;
	_npcState[0].y_11 = 0;
	_npcState[0].y_12 = 0;
	_npcState[0].y_21 = 0;
	_npcState[0].y_22 = 0;
	_arcadeTTM._drawXOffset = _npcState[0].x - 152;
	_arcadeTTM._drawYOffset = _arcadeTTM._startYOffset;
	_arcadeTTM._currentNPCRunningTTM = 0;
	if (-1 < _npcState[0].byte12) {
		_arcadeTTM.runNextPage(_npcState[0].ttmPage);
	}

	for (int i = 0; i < ARRAYSIZE(_bullets); i++) {
		int16 x = _bullets[i]._x;
		int16 y = _bullets[i]._y;
		if (_bullets[i]._state == kBulletHittingBlade) {
			drawBulletHitCircles(x, y, false);
		} else if (_bullets[i]._state == kBulletHittingEnemy) {
			drawBulletHitCircles(x, y, true);
		} else if (_bullets[i]._state == kBulletFlying) {
			int16 frameno;
			if (_bullets[i]._bulletType == 3) {
				// TODO: check this.. it's a bit weird?
				frameno = (_nextRandomVal % 3);
			} else {
				frameno = 0;
			}
			_bulletImg->drawBitmap(frameno, x, y, drawWin, DgdsEngine::getInstance()->_compositionBuffer, _bullets[i]._flipMode);
		}
	}

}

} // end namespace Dgds
