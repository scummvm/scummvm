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
#include "dgds/dgds.h"
#include "dgds/image.h"
#include "dgds/includes.h"
#include "dgds/dragon_arcade.h"
#include "dgds/sound.h"
#include "dgds/drawing.h"
#include "dgds/globals.h"
#include "dgds/game_palettes.h"
#include "dgds/menu.h"
#include "dgds/font.h"

namespace Dgds {

DragonArcade::DragonArcade() {
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
		return 0;

	// TODO: Set video mask here?

	_nextRandomVal = DgdsEngine::getInstance()->getRandom().getRandomNumber(65535);

	error("DragonArcade::doTickUpdate: finish me");
	
	return false;
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


void DragonArcade::resetStageState() {
	clearAllNPCStates();
	clearAllBulletStates();
	_scrollXOffset = 0;
	_nTickUpdates = 0;
	//INT_39e5_0a34 = 0;
	//BYTE_39e5_0a14 = 0;
	//INT_39e5_3d18 = 0;
	//UINT_39e5_0a17 = 0;
	_shouldUpdateState = 0;
	//INT_39e5_3fa8 = 0;
	_npcState[0].byte12 = 1;
	_npcState[0].byte14 = 0;
	_npcState[0].health = (4 - _startDifficultyMaybe) * 3 + 20;
	_npcState[0].npcState = 3;
	_npcState[1].health = 0;
	_lastDrawnBladeHealth = -1;
	_lastDrawnBossHealth = -1;
	_bladeState1 = 0;
	_mouseButtonWentDown = 0;
	_bladeMoveFlag = kBladeMoveNone;
	// TODO: Work out what those commented out variables are..
	error("DragonArcade::resetStageState: TODO: Finish me.");
}

void DragonArcade::initValuesForStage() {
	error("DragonArcade::initValuesForStage: TODO: Implement me.");
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
		_startYOffset = 0;
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
		_startYOffset = -43;
		break;
    case 6:
		_currentNPCRunningTTM = 0;
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
		_npcState[i].npcState = -1;
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
		if ((_bladeState1 == 0 && _bladeStateOffset + 28 < _npcState[0].npcState && _npcState[0].npcState <= 35)
			|| _bladeState1 == 4) {
			_bladeState1 = 9;
			_npcState[0].npcState = _bladeState1 + 103;
		} else {
			_bladeState1 = 8;
			_npcState[0].npcState = _bladeState1 + 98;
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
	_drawXOffset = _npcState[0].x - 152;
	_drawYOffset = _startYOffset;
	_currentNPCRunningTTM = 0;
	if (-1 < _npcState[0].byte12) {
		_arcadeTTM.runNextPage(_npcState[0].npcState);
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

	error("DragonArcade::runThenDrawBulletsInFlight: TODO: implement the first half here.");
}

} // end namespace Dgds
