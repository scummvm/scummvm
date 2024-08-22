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
#include "dgds/dragon_arcade.h"
#include "dgds/sound.h"
#include "dgds/scene.h"
#include "dgds/drawing.h"
#include "dgds/globals.h"
#include "dgds/game_palettes.h"
#include "dgds/menu.h"
#include "dgds/font.h"

namespace Dgds {

DragonArcade::DragonArcade() : _arcadeTTM(_npcState) {
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
	_initFinished = false;
	warning("TODO: DragonArcade::finish: copy/clear some vid buffers here?");
}

bool DragonArcade::doTickUpdate() {
	// TODO: Copy video buffers here?

	if (_finishCountdown == 0)
		return false;

	// TODO: Set video mask here?

	_nextRandomVal = _getNextRandom();

	if (!_arcadeNeedsBufferCopy) {
		updateMouseAndJoystickStates();
		updateBladeWithInputs();
	}

	int16 findResult = findValuesIn3d30Array();
	arcade2754(findResult);
	
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
    //setSomeVideoFlags();
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
    //do { } while (_arcadeTickDelay != 0);
    //_arcadeTickDelay = 4;
    _nTickUpdates++;
    
	return true;
}

void DragonArcade::updateBullets() {
	for (int i = 19; i > 0; i++) {
		if (_bullets[i]._state == 2 || _bullets[i]._state == 3) {
			_bullets[i]._state = kBulletInactive;
			continue;
		}
		
		if (_bullets[i]._state == kBulletFlying) {
			if (_bullets[i]._var1 == 3) {
				_bullets[i]._y += _bullets[i]._speed;
			}
			if (_bullets[i]._flipMode == 0) {
				_bullets[i]._x -= (_scrollXIncrement * 8 - 10);
				if (_bullets[i]._x < 0x141) {
					int iVar2 = checkBulletCollision(i);
					if (iVar2 != 0) {
						if (iVar2 < 1) {
							_bullets[i]._state = kBulletHittingEnemy;
						} else {
							_bullets[i]._state = kBulletHittingBlade;
						}
					}
					continue;
				}
			} else {
				_bullets[i]._x -= (_scrollXIncrement * 8 + 10);
				if (-0x29 < _bullets[i]._x) {
					int iVar2 = checkBulletCollision(i);
					if (iVar2 != 0) {
						if (iVar2 < 1) {
							_bullets[i]._state = kBulletHittingEnemy;
						} else {
							_bullets[i]._state = kBulletHittingBlade;
						}
					}
					continue;
				}
			}
			_bullets[i]._state = kBulletInactive;
		}
	}
}

int16 DragonArcade::checkBulletCollision(int16 num) {
	error("TODO: Implement me");
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
	if (_npcState[0].byte14 != 0)
		return;

	for (int i = 0; i < 8; i++) {
		if (FIRE_ALLOWABLE_PAGES[i] == _npcState[0].ttmPage) {
			if (_npcState[0].ttmPage < 0x7b) {
				flipMode = kImageFlipNone;
			} else {
				flipMode = kImageFlipV;
			}
			if (_haveBigGun == 0) {
				yoff = FIRE_Y_OFFSETS_SMALL_GUN[i];
			} else {
				yoff = FIRE_Y_OFFSETS_BIG_GUN[i];
			}

			createBullet(FIRE_X_OFFSETS[i] + _npcState[0].x - 0xa0,
						 yoff + _arcadeTTM._startYOffset, flipMode, 0);

			// is this always 0?
			//if (INT_39e5_0c58 == 0) {
			sndno = 0x2f;
			//} else {
			//sndno = 0x34;
			//}
			playSfx(sndno);
			_bladeHasFired = true;
			break;
		}
	}
}


static const int16 ENEMY_FIRE_ALLOWABLE_PAGES[] =  { 3, 0xC, 0x1F, 0x28 };
static const int16 ENEMY_FIRE_X_OFFSETS[] = { 0xB1, 0xB3, 0x77, 0x75, };
static const int16 ENEMY_FIRE_Y_OFFSETS[] = { 0x4E, 0x56, 0x4D, 0x55,};

void DragonArcade::checkEnemyFireStage0124() {
	for (int i = 9; i != 0; i--) {
		if (_npcState[i].byte12 == 0)
			continue;
		for (int j = 0; j < 4; j++) {
			if (_npcState[i].x < 0x154 && -20 < _npcState[i].x &&
				ENEMY_FIRE_ALLOWABLE_PAGES[j] == _npcState[i].ttmPage) {
				ImageFlipMode flipMode = (_npcState[i].ttmPage < 0x1d) ? kImageFlipNone : kImageFlipV;
				createBullet(ENEMY_FIRE_X_OFFSETS[j] + _npcState[i].val1 - _scrollXOffset * 8 - 0xa0,
							 ENEMY_FIRE_Y_OFFSETS[j] + _npcState[i].val2 + 3, flipMode, 1);
				playSfx(0x25);
			}
		}
	}
}


void DragonArcade::checkBossFireStage3() {
	if (_npcState[1].x < 0x154 && -20 < _npcState[1].x && _npcState[1].ttmPage == 22) {
		createBullet(_npcState[1].val1 - _scrollXOffset * 8 - 44,
						_npcState[1].val2 + 70, kImageFlipH, 3);
		playSfx(0x2a);
	}
}

void DragonArcade::checkBossFireStage6() {
	ImageFlipMode flipMode = (_npcState[1].ttmPage < 0x28) ? kImageFlipNone: kImageFlipV;

	if (_npcState[1].x < 0x154 && -20 < _npcState[1].x &&
		(_npcState[1].ttmPage == 9 || _npcState[1].ttmPage == 0x28)) {
		createBullet(_npcState[1].val1 - _scrollXOffset * 8 - 19,
						_npcState[1].val2 + 86, flipMode, 2);
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

void DragonArcade::keyboardUpdate() {
	// TODO: Keyboard update.
}

void DragonArcade::mouseUpdate() {
	limitToCenterOfScreenAndUpdateCursor();
	_rMouseButtonState = DgdsEngine::getInstance()->getScene()->isRButtonDown();
	_lMouseButtonState = DgdsEngine::getInstance()->getScene()->isLButtonDown();
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
	_rMouseButtonState = 0;
	_lMouseButtonState = 0;
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
				if (_arcadeNotMovingLeftFlag) {
					_arcadeNotMovingLeftFlag = false;
				}
				_bladeMoveFlag = kBladeMoveLeft;
				if (_bladeState1 == 0) {
					_int0b5c = -1;
					_bladeHorizMoveAttempt = 1;
				}
			}
		} else {
			_bladeMoveFlag = kBladeMoveRight;
			if (_bladeState1 == 0) {
				_int0b5c = 1;
				_bladeHorizMoveAttempt = 2;
			}
		}
		if ((_keyStateFlags & kBladeMoveUp) == kBladeMoveNone) {
			if ((_keyStateFlags & kBladeMoveDown) != kBladeMoveNone) {
				_bladeMoveFlag = static_cast<DragonBladeMoveFlag>(_bladeMoveFlag | kBladeMoveDown);
			}
		} else {
			_bladeMoveFlag = static_cast<DragonBladeMoveFlag>(_bladeMoveFlag | kBladeMoveUp);
		}
		if ((_lMouseButtonState != 0) && (_lastLMouseButtonState == 0)) {
			_mouseButtonWentDown = 1;
		}
		if (_rMouseButtonState != 0 && _lastRMouseButtonState == 0 && _bladeState1 != 2 && _bladeState1 != 1) {
			_mouseButtonWentDown = 2;
			_bladeMoveFlagBeforeRButton = _bladeMoveFlag;
		}
		_lastLMouseButtonState = _lMouseButtonState;
		_lastRMouseButtonState = _rMouseButtonState;
	}
}

int16 DragonArcade::findValuesIn3d30Array() {
	error("DragonArcade::findValuesIn3d30Array: Implement me");
}

bool DragonArcade::isNpcInsideXRange(int16 num) {
	return _npcState[num].x < 321 && _npcState[num].x > 1;
}


void DragonArcade::arcade4085() {
	error("DragonArcade::arcade4085: implement me");
}

void DragonArcade::arcade2754(int16 findResult) {
	error("DragonArcade::arcade2754: implement me");
}


static const int16 arrayC5A[4][7] {
    { 0x8, 0x19, 0x5A, 0x78, 0x8C, 0xA5, 0xBE },
    { 0x6, 0x28, 0x5A, 0x87, 0x96, 0xAA, 0xC0 },
    { 0x4, 0x37, 0x69, 0x79, 0x91, 0xA0, 0xC2 },
    { 0x2, 0x46, 0x69, 0x87, 0x9B, 0xAF, 0xC4 },
};

static const int16 arrayC92[4][7] {
	{ 0xA, 0x37, 0x46, 0x55, 0x91, 0xAA, 0xC8 },
	{ 0x19, 0x5F, 0x87, 0x9B, 0xB9, 0xD7, -0x1 },
	{ 0x19, 0x23, 0x69, 0x7D, 0x9B, 0xB9, 0xD7 },
	{ 0xA, 0x37, 0x46, 0x73,  0xAA, 0xC8,  -0x1 },
};


void DragonArcade::arcade3e96() {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 7; j++) {
			if (_loadedArcadeStage == 0) {
				if (_flag40ee && arrayC5A[i][j] == _someCounter40f0) {
					_npcState[i].ttmPage = 1;
				}
			} else if (_loadedArcadeStage == 1) {
				if (arrayC92[i][j] == _someCounter40f0) {
					_npcState[4 + i].ttmPage = 1;
				}
				if (_flag40ee && arrayC5A[i][j] == _someCounter40f0) {
					_npcState[i].ttmPage = 1;
				}
			} else if (_loadedArcadeStage == 2 && _flag40ef && arrayC92[i][j] == _someCounter40f0) {
				_npcState[i].ttmPage = 1;
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
			_npcState[i].byte12 = 0xfc;
			if (_npcState[i].ttmPage < 0xf &&
				_npcState[i].val1 - 16 <= _npcState[0].val1 &&
				_npcState[0].val1 <= _npcState[i].val1 + 12 && _npcState[0].health != 0 &&
				(_loadedArcadeStage != 1 || _arcadeTTM._startYOffset < -9 ||
				 (_bladeMoveFlag & kBladeMoveRight) == kBladeMoveNone)) {
				setFinishCountdownIfLessThan0(20);
				_npcState[0].byte14 = 2;
				_npcState[0].health = 0;
				if (_haveBigGun == 0) {
					_npcState[0].ttmPage = 0x22;
				} else {
					_npcState[0].ttmPage = 0x19;
				}
				_npcState[0].byte12 = 12;
				_bladeState1 = 0xc;
				_mouseButtonWentDown = 0x80;
			}
		}
	}
	_someCounter40f0++;
	if (_someCounter40f0 == 221) {
        _someCounter40f0 = 0;
	}

}

void DragonArcade::updateBlade() {
	int16 local_6 = 0;
	
	for (int i = 9; i > 0; i--) {
		if (_npcState[i].byte12 == 0)
			continue;

		int16 startPage = (_npcState[i].ttmPage < 30) ? 0 : 28;

		if (_npcState[i].byte13 == 0 &&
			((startPage != 0 && _npcState[i].val1 < _npcState[0].val1) ||
			 (startPage == 0 && _npcState[0].val1 < _npcState[i].val1))) {
			
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
			if ((0xf - i == (_nextRandomVal & 0xf)) &&
				 abs(_npcState[i].y - _npcState[0].y) < 36 && _npcState[0].health != 0) {
				_npcState[i].byte12 = 2;
				_npcState[i].ttmPage = startPage + 3;
			}
			break;
		case 4:
			if ((0xf - i == (_nextRandomVal & 0xf)) &&
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
	_npcState[2].ttmPage = _npcState[2].ttmPage + 1;
	if (29 < _npcState[2].ttmPage) {
		_npcState[2].ttmPage = 23;
	}

	int16 distToBoss = _npcState[1].x - _npcState[0].x;
	int16 absDistToBoss = abs(distToBoss);
	bool bossIsClose = absDistToBoss < 20;
	uint16 uVar4 = _nextRandomVal & 0xf;

	switch(_npcState[1].byte12) {
	case 0:
		if (bossIsClose && absDistToBoss < 45) {
			if (_bladeState1 != 8 && _bladeState1 != 9) {
				_npcState[1].byte12 = 5;
				_npcState[1].ttmPage = 30;
			}
		} else if (distToBoss < 0 || (_bossStateUpdateCounter < 0 && uVar4 == 7)) {
			_npcState[1].byte12 = 3;
			_npcState[1].ttmPage = 10;
			_bossStateUpdateCounter++;
		} else if ((bossIsClose && distToBoss < 70 && 0 < distToBoss && uVar4 == 0xf) || (0 < _bossStateUpdateCounter && uVar4 == 7)) {
			_npcState[1].byte12 = 2;
			_npcState[1].ttmPage = 3;
			_bossStateUpdateCounter--;
		} else if (_bossStateUpdateCounter == 0 && uVar4 == 0xf) {
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
		if ((_npcState[1].ttmPage != 67 || (_nTickUpdates & 0x1fU) == 0) && _npcState[1].ttmPage < 74) {
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
		if ((_npcState[1].ttmPage != 67 || (_nTickUpdates & 0x1fU) == 0) && _npcState[1].ttmPage < 74) {
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
			_npcState[1].val1 += 6;
		} else {
			_npcState[1].byte12 = 1;
			_npcState[1].ttmPage = 2;
		}
		break;
	case 1:
		if (_npcState[1].ttmPage < 9) {
			_npcState[1].ttmPage++;
			_npcState[1].val1 -= 6;
		} else {
			_npcState[1].byte12 = 5;
			_npcState[1].ttmPage = 30;
		}
		break;
	case 4:
		if (_npcState[1].ttmPage < 37) {
			_npcState[1].ttmPage++;
			// TODO: does UINT_39e5_0a17 ever get modified? It's set 0 in resetStageState
			if (bossIsClose && absDistToBoss < 50 && /*UINT_39e5_0a17 == 0 &&*/ 33 < _npcState[1].ttmPage && _npcState[1].ttmPage < 37) {
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
			_npcState[3].byte14 = 1;
		}
	} else if (_npcState[3].byte12 == -3) {
		_npcState[3].ttmPage++;
		if (48 < _npcState[3].ttmPage)
			_npcState[3].byte12 = 0;
	}
}


// FIXME: remove this
uint16 UINT_39e5_0be6 = 0;

static const int16 BOSS_2_PAGE_OFFSETS[] = { 2, 2, 0xe, 0x1b, 0x21, 0x2a, 0x34, 0x3b };

void DragonArcade::updateBoss2() {
	if (_arcadeNotMovingLeftFlag && 269 < _scrollXOffset + _npcState[0].x / 8 && _scrollXOffset < 282) {
		_int0b5c = 1;
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
			UINT_39e5_0be6 = 0;
		} else {
			UINT_39e5_0be6 = 31;
		}
		if ((_nextRandomVal & 0xfU) == 0xf) {
			if (abs(_npcState[1].y - _npcState[0].y) > 35)
				return;
			
			_npcState[1].byte12 = 4;
			_npcState[1].ttmPage = UINT_39e5_0be6 + 9;
		}
		if ((_nextRandomVal & 0xfU) == 7 && absDistToBoss > 20 && _npcState[1].val1 < 0x938) {
			_npcState[1].byte12 = 2;
			_npcState[1].ttmPage = UINT_39e5_0be6 + 2;
		}
		else if (_bladeHasFired == 0 || _npcState[1].val1 < 0x939) {
			if (absDistToBoss < 0x1e) {
				arcade34b4();
			}
		}
		else {
			_npcState[1].byte12 = 6;
			_npcState[1].ttmPage = UINT_39e5_0be6 + 13;
		}
		break;
	case 4:
		if (_npcState[1].ttmPage < (UINT_39e5_0be6 + 0x1f)) {
			_npcState[1].ttmPage = _npcState[1].ttmPage + 1;
		}
		break;
	case 3:
		if (_npcState[1].ttmPage < (UINT_39e5_0be6 + 12)) {
			_npcState[1].ttmPage = _npcState[1].ttmPage + 1;
		} else {
			_npcState[1].byte12 = 1;
			_npcState[1].ttmPage = UINT_39e5_0be6 + 1;
		}
		break;
	case 1:
		_npcState[1].ttmPage = _npcState[1].ttmPage + 1;
		if ((UINT_39e5_0be6 + 8) <= _npcState[1].ttmPage) {
			_npcState[1].ttmPage = UINT_39e5_0be6 + 2;
		}
		if (UINT_39e5_0be6 == 0) {
			_npcState[1].val1 = _npcState[1].val1 + 8;
		} else {
			_npcState[1].val1 = _npcState[1].val1 - 8;
		}
		if (absDistToBoss < 0x1e) {
			arcade34b4();
		}
		break;
	case 5:
		if (_npcState[1].ttmPage < (int)(UINT_39e5_0be6 + 20)) {
			_npcState[1].ttmPage = _npcState[1].ttmPage + 1;
		} else {
			_npcState[1].byte12 = 7;
			_npcState[1].ttmPage = UINT_39e5_0be6 + 21;
		}
		break;
	case 6:
		if (_npcState[1].ttmPage < (int)(UINT_39e5_0be6 + 24)) {
			if (_nTickUpdates & 1) {
				_npcState[1].ttmPage = _npcState[1].ttmPage + 1;
				if (UINT_39e5_0be6 == 0) {
					_npcState[1].val1 = _npcState[1].val1 + 6;
				} else {
					_npcState[1].val1 = _npcState[1].val1 + -6;
				}
			}
		} else if (absDistToBoss < 40 || _npcState[1].val1 < 0x8d4) {
			_npcState[1].byte12 = 8;
			_npcState[1].ttmPage = UINT_39e5_0be6 + 25;
		} else if (_nTickUpdates & 1) {
			_npcState[1].ttmPage = UINT_39e5_0be6 + 21;
			if (UINT_39e5_0be6 == 0) {
				_npcState[1].val1 = _npcState[1].val1 + 6;
			} else {
				_npcState[1].val1 = _npcState[1].val1 + -6;
			}
		}
		break;
	case 7:
		if (_npcState[1].ttmPage < (int)(UINT_39e5_0be6 + 0x1b)) {
			_npcState[1].ttmPage = _npcState[1].ttmPage + 1;
		} else if (absDistToBoss < 40) {
			arcade34b4();
		} else {
			_npcState[1].byte12 = 1;
			_npcState[1].ttmPage = UINT_39e5_0be6 + 1;
		}
		break;
	case 2:
	default:
		if (_stillLoadingScriptsMaybe != 0) {
			if ((_int0b5c == -1 && _npcState[1].x < 0x96) || (_int0b5c == 1 && 160 < _npcState[1].x)) {
				arcade1d57();
			}
			byte bossByte12 = _npcState[1].byte12;
			// TODO: do these ever get changed?
			const int16 INT_39e5_0be0 = 0x3A;
			const int16 INT_39e5_0bda = 0x20;
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
				error("TODO: Fix references to table at 0xb0c");
				/*
				if (*(int *)(bossByte12 * 2 + 0xb0c) + 0x41 == _npcState[1].ttmPage) {
					_npcState[1].byte12 = 100;
					_npcState[1].ttmPage = 67;
				} else if (bossByte12 != 100 && _nTickUpdates & 1) {
					_npcState[1].ttmPage++;
				}
				*/
			}
		}
	}
}

void DragonArcade::arcade1d57() {
	int16 lastScrollOffset = _scrollXOffset;
	_scrollXOffset = CLIP(_scrollXOffset + _int0b5c, 0, 282);
	if (lastScrollOffset != _scrollXOffset) {
		_scrollXIncrement += _int0b5c;
	}
}

void DragonArcade::arcade34b4() {
	_npcState[0].ttmPage = -1;
	if (_npcState[0].x < 150) {
		_int0b5c = -1;
	} else if (_npcState[0].x < 161) {
		_int0b5c = 0;
	} else {
		_int0b5c = 1;
	}

	_arcadeNotMovingLeftFlag = -1;
	// TODO: what is this?
	// UINT_39e5_0a1f = 1;
	_stillLoadingScriptsMaybe = 1;
	_npcState[0].byte12 = -1;
	_npcState[1].byte12 = 100;
	_npcState[1].ttmPage = 67;
	_npcState[1].val1 = _npcState[0].val1;
	_npcState[1].byte14 = 2;
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
			if (_npcState[1].ttmPage < 0x20) {
				_npcState[1].ttmPage = 0x1c;
			} else {
				_npcState[1].ttmPage = 0x3b;
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
		if ((_bladeState1 == 0 && _bladeStateOffset + 0x1c <= _npcState[0].ttmPage &&
			 _npcState[0].ttmPage <= _bladeStateOffset + 0x23) || _bladeState1 == 4) {
			_bladeState1 = 9;
			_npcState[0].ttmPage = _bladeStateOffset + 0x67;
		} else {
			_bladeState1 = 8;
			_npcState[0].ttmPage = _bladeStateOffset + 0x62;
		}
		setFinishCountdownIfLessThan0(15);
		_npcState[0].byte14 = 0;
		_mouseButtonWentDown = 0x80;
	} else {
		playSfx(0x29);
	}
}


void DragonArcade::initIfNeeded() {
	if (_initFinished)
		return;

	DgdsEngine *engine = DgdsEngine::getInstance();
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

	int16 envNum;
	_arcadeTTM._currentTTMNum = 0;
	envNum = _arcadeTTM.load(ttmName);
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
	// set font to 0?
	// set text draw to 0xe?
	drawBackgroundAndWeapons();
	loadTTMScriptsForStage(_nextStage);
	_initFinished = true;
	_arcadeModeSomethingCounter = 0;
	g_system->warpMouse(166, 158);
	_dontRedrawBgndAndWeapons = true;
	redraw();
}

void DragonArcade::updateBladeWithInputs() {
	if (_stillLoadingScriptsMaybe)
		return;
	
	error("TODO: updateBladeWithInputs: Finish me");
}

void DragonArcade::resetStageState() {
	clearAllNPCStates();
	clearAllBulletStates();
	_scrollXOffset = 0;
	_nTickUpdates = 0;
	//BYTE_39e5_0a14 = 0;
	//INT_39e5_3d18 = 0;
	//UINT_39e5_0a17 = 0;
	_shouldUpdateState = 0;
	_arcadeNotMovingLeftFlag = 0;
	_npcState[0].byte12 = 1;
	_npcState[0].byte14 = 0;
	_npcState[0].health = (4 - _startDifficultyMaybe) * 3 + 20;
	_npcState[0].ttmPage = 3;
	_npcState[1].health = 0;
	_lastDrawnBladeHealth = -1;
	_lastDrawnBossHealth = -1;
	_bladeState1 = 0;
	_mouseButtonWentDown = 0;
	_bladeMoveFlag = kBladeMoveNone;
	// TODO: Work out what those commented out variables are..
}

static const int STAGE_0_VAL1[] = {
	0x191, 0x1BB, 0x1F5,
	0x25B, 0x2D3, 0x341,
	0x535, 0x5C9, 0x623
};

static const byte STAGE_0_VAL2[] = {
	0, 0, 0, 0xd8, 0xd8, 0, 0, 0, 0xd8
};

static const byte STAGE_0_BYTE12[] = {
	5, 4, 4, 5, 4, 5, 4, 5, 5
};

static const int STAGE_4_VAL1[] = {
	0x169, 0x19D, 0x1B9,
	0x30D, 0x32D, 0x457,
	0x4DB, 0x501, -1
};

static const byte STAGE_4_VAL2[] = {
	0, 0, 0, 0, 0, 0xd8, 0, 0, 0xd8
};

static const byte STAGE_4_BYTE12[] = {
	5, 4, 4, 5, 4, 5, 5, 4, 5
};

void DragonArcade::initValuesForStage() {
	for (int i = 9; i != 0; i--)
		_npcState[i].byte12 = 0;

	switch (_loadedArcadeStage) {
	case 0:
		for (int i = 1; i < 10; i++) {
			_npcState->val1 = STAGE_0_VAL1[i - 1];
			_npcState->val2 = STAGE_0_VAL2[i - 1];
			_npcState->byte12 = STAGE_0_BYTE12[i - 1];
			if (_npcState->byte12 == 5)
				_npcState->ttmPage = 39;
			else
				_npcState->ttmPage = 30;
			initValuesForStage0();
		}
		break;
	case 3:
		initValuesForStage3();
		break;
	case 4:
		for (int i = 1; i < 10; i++) {
			_npcState->val1 = STAGE_4_VAL1[i - 1];
			_npcState->val2 = STAGE_4_VAL2[i - 1];
			_npcState->byte12 = STAGE_4_BYTE12[i - 1];
			if (_npcState->byte12 == 5)
				_npcState->ttmPage = 39;
			else
				_npcState->ttmPage = 30;
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

void DragonArcade::initValuesForStage0() {
	_someCounter40f0 = 0;
	for (int i = 0; i < 4; i++) {

	}
	_flag40ee = true;
	_flag40ef = true;
	/*
	INT_39e5_3f6c = 0x11f;
	INT_39e5_3f6e = -0xd;
	BYTE_39e5_3f7e = 0x1e;
	INT_39e5_3f7c = 0x20;
	BYTE_39e5_3f81 = 2;
	*/
	error("TODO: initValuesForStage0 Implement me");
}

void DragonArcade::initValuesForStage3() {
	clearAllNPCStates();
	_bossStateUpdateCounter = 0;
	_npcState[1].val1 = 0x99c;
	_npcState[1].val2 = -54;
	_npcState[1].byte12 = 1;
	_npcState[1].ttmPage = 2;
	_npcState[1].health = 20;
	_npcState[1].byte14 = 1;
	_npcState[1].y = 300;
	_npcState[2].val1 = 0x9b2;
	_npcState[2].val2 = -57;
	_npcState[2].byte12 = -1;
	_npcState[2].ttmPage = 23;
	_npcState[2].health = 0;
	_npcState[2].byte14 = 1;
}

void DragonArcade::initValuesForStage4() {
	_someCounter40f0 = 0;
	for (int i = 0; i < 5; i++) {
	}
	error("TODO: initValuesForStage4 Implement me");
}

void DragonArcade::initValuesForStage6() {
	clearAllNPCStates();
	_npcState[1].val1 = 0x9e2;
	_npcState[1].val2 = -3;
	_npcState[1].ttmPage = 1;
	_npcState[1].health = 10;
	_npcState[1].byte14 = 1;
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
		_arcadeModeSomethingCounter++;
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
		fadeInAndClearScreen();
		finish();
		globals->setArcadeState(_shouldUpdateState + 6);
		return;
	case 20:
		globals->setArcadeState(30);
		return;
	case 30:
        loadTTMScriptsForStage(_nextStage);
        // These don't seem to ever be used?
        // UINT_39e5_0d0e = 0;
        // UINT_39e5_0d10 = 0;
        globals->setArcadeState(5);
        _arcadeNeedsBufferCopy = true;
        _flagInventoryOpened = false;
        return;
	default:
		_haveBomb = arcadeState > 20;
		if (_haveBomb)
		  globals->setArcadeState(arcadeState - 20);

		_enemyHasSmallGun = arcadeState > 10;
		if (_enemyHasSmallGun != 0)
		  globals->setArcadeState(arcadeState - 10);

		bool _haveBigGun = arcadeState > 2;
		if (_haveBigGun != 0)
		  globals->setArcadeState(arcadeState - 2);

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
		_npcState[0].val1 = 160;
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
		_npcState[0].val1 = 140;
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
		_currentArcadeTT3Num = 1;
		_arcadeTTM.freeShapes();
		_arcadeTTM.freePages(1);
		//g_TT3ScriptDataPtrs[1][0] = nullptr;
		//g_TT3ScriptDataPtrs[1][1] = nullptr;
		_currentArcadeTT3Num = 2;
		_arcadeTTM.freeShapes();
		_arcadeTTM.freePages(2);
		_currentArcadeTT3Num = 1;
		int16 envNum = _arcadeTTM.load(ttm1);
		_arcadeTTM.finishTTMParse(envNum);
		_arcadeTTM.runNextPage(0);
		_currentArcadeTT3Num = 2;
		//g_TT3ScriptDataPtrs[2][0] = nullptr;
		//g_TT3ScriptDataPtrs[2][1] = nullptr;
		envNum = _arcadeTTM.load(ttm2);
		_arcadeTTM.finishTTMParse(envNum);
		_arcadeTTM.runNextPage(0);
	}
	//INT_39e5_0b54 = _startYOffset;
	_finishCountdown = -1;
	//INT_39e5_0be4 = 0; // this only ever gets set 0?
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

void DragonArcade::drawBulletHitCircles(uint16 x, uint16 y, bool flag) {
	static const byte COLORS[2][3] = { {0, 1, 9}, {0, 4, 12} };

	Graphics::ManagedSurface &dst = DgdsEngine::getInstance()->_compositionBuffer;
	for (int i = 0; i < 3; i++) {
		byte col = COLORS[flag][i];
		Drawing::filledCircle(x, y, 4 - i, 4 - i, &dst, col, col);
	}
}

void DragonArcade::checkToOpenMenu() {
	if (_arcadeModeSomethingCounter < 5) {
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

void DragonArcade::createBullet(int16 x, int16 y, ImageFlipMode flipMode, uint16 var1) {
	for (uint i = 0; i < ARRAYSIZE(_bullets); i++) {
		if (_bullets[i]._state == kBulletInactive) {
			_bullets[i]._state = kBulletFlying;
			_bullets[i]._x = x;
			_bullets[i]._y = x;
			_bullets[i]._flipMode = flipMode;
			_bullets[i]._var1 = var1;
			if (var1 == 3)
				_bullets[i]._speed = _nextRandomVal & 3;

			break;
		}
	}
}

void DragonArcade::playSfx(int16 num) const {
	DgdsEngine::getInstance()->_soundPlayer->playSFX(num);
}

void DragonArcade::bladeTakeHitAndCheck() {
	if (_bladeHealth)
		_bladeHealth--;
	if (!_enemyHasSmallGun && _bladeHealth)
		_bladeHealth--;
	if (_bladeHealth <= 0) {
		playSfx(75);
		if ((_bladeState1 == 0 && _bladeStateOffset + 28 < _npcState[0].ttmPage && _npcState[0].ttmPage <= 35)
			|| _bladeState1 == 4) {
			_bladeState1 = 9;
			_npcState[0].ttmPage = _bladeState1 + 103;
		} else {
			_bladeState1 = 8;
			_npcState[0].ttmPage = _bladeState1 + 98;
		}
		setFinishCountdownIfLessThan0(15);
		_npcState[0].byte14 = 0;
		_mouseButtonWentDown = 0x80;
	} else {
		playSfx(41);
	}
}

void DragonArcade::drawHealthBars() {
	DgdsEngine *engine = DgdsEngine::getInstance();

	if (_bladeHealth != _lastDrawnBladeHealth) {
		Common::Rect clearRect(Common::Point(10, 155), 64, 10);
		engine->_compositionBuffer.fillRect(clearRect, 0);

		for (int i = 1; i <= _bladeHealth; i++) {
			int x = 8 + i * 2;
			engine->_compositionBuffer.drawLine(x, 155, x, 162, 12);
		}

		_lastDrawnBladeHealth = _bladeHealth;
	}

	if (((_loadedArcadeStage == 3 || _loadedArcadeStage == 6) || _lastDrawnBossHealth == -1) && (_bossHealth != _lastDrawnBossHealth)) {
		Common::Rect clearRect(Common::Point(10, 167), 60, 8);
		engine->_compositionBuffer.fillRect(clearRect, 0);

		byte color = (_loadedArcadeStage == 3) ? 2 : 9;

		for (int i = 1; i <= _bossHealth; i++) {
			int x = 8 + i * 2;
			engine->_compositionBuffer.drawLine(x, 167, x, 174, color);
		}
		_lastDrawnBossHealth = _bossHealth;
	}
}

void DragonArcade::redraw() {
	if (!_dontRedrawBgndAndWeapons)
		drawBackgroundAndWeapons();

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


void DragonArcade::runThenDrawBulletsInFlight() {
	// TODO: Set clip window here? 8,8,124,311
	_arcadeTTM.runPagesForEachNPC(_scrollXOffset);

	const Common::Rect screenWin(SCREEN_WIDTH, SCREEN_HEIGHT);
	_arcadeTTM._currentTTMNum = _npcState[0].byte14;
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
		} else if (_bullets[i]._state == kBulletHittingBlade) {
			drawBulletHitCircles(x, y, true);
		} else if (_bullets[i]._state == kBulletFlying) {
			int16 frameno;
			if (_bullets[i]._var1 == 3) {
				// FIXME: this is a bit weird?
				frameno = (_nextRandomVal % 3);
			} else {
				frameno = 0;
			}
			_bulletImg->drawBitmap(frameno, x, y, screenWin, DgdsEngine::getInstance()->_compositionBuffer, _bullets[i]._flipMode);
		}
	}

}

} // end namespace Dgds
