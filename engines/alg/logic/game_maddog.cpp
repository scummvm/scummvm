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

#include "common/debug.h"
#include "common/rect.h"
#include "common/savefile.h"
#include "common/system.h"

#include "graphics/cursorman.h"

#include "alg/graphics.h"
#include "alg/logic/game_maddog.h"
#include "alg/scene.h"

namespace Alg {

GameMaddog::GameMaddog(AlgEngine *vm, const AlgGameDescription *gd) : Game(vm) {
}

GameMaddog::~GameMaddog() {
	if (_shotIcon) {
		_shotIcon->free();
		delete _shotIcon;
	}
	if (_emptyIcon) {
		_emptyIcon->free();
		delete _emptyIcon;
	}
	if (_liveIcon) {
		_liveIcon->free();
		delete _liveIcon;
	}
	if (_deadIcon) {
		_deadIcon->free();
		delete _deadIcon;
	}
	if (_reloadIcon) {
		_reloadIcon->free();
		delete _reloadIcon;
	}
	if (_drawIcon) {
		_drawIcon->free();
		delete _drawIcon;
	}
	if (_knifeIcon) {
		_knifeIcon->free();
		delete _knifeIcon;
	}
	if (_bulletholeIcon) {
		_bulletholeIcon->free();
		delete _bulletholeIcon;
	}
}

void GameMaddog::init() {
	Game::init();

	_videoPosX = 56;
	_videoPosY = 8;

	setupCursorTimer();

	loadLibArchive("maddog.lib");
	_sceneInfo->loadScnFile("maddog.scn");
	_startScene = _sceneInfo->getStartScene();

	registerScriptFunctions();
	verifyScriptFunctions();

	_menuzone = new Zone("MainMenu", "GLOBALHIT");
	_menuzone->addRect(0x0C, 0xAC, 0x3D, 0xBF, nullptr, 0, "SHOTMENU", "0");
	_menuzone->addRect(0x00, 0xA6, 0x013F, 0xC7, nullptr, 0, "DEFAULT", "0"); // _mm_bott
	_menuzone->addRect(0x00, 0x00, 0x3B, 0xC7, nullptr, 0, "DEFAULT", "0");   // _mm_left

	_submenzone = new Zone("SubMenu", "GLOBALHIT");
	_submenzone->addRect(0x8A, 0x3B, 0xC2, 0x48, nullptr, 0, "STARTBOT", "0");
	_submenzone->addRect(0x8A, 0x4E, 0xC2, 0x59, nullptr, 0, "STARTMENU", "0");
	_submenzone->addRect(0x8A, 0x60, 0xC2, 0x6B, nullptr, 0, "CONTMENU", "0");
	_submenzone->addRect(0xE3, 0x3B, 0x011B, 0x48, nullptr, 0, "RECTSAVE", "0");
	_submenzone->addRect(0xE3, 0x4E, 0x011B, 0x59, nullptr, 0, "RECTLOAD", "0");
	_submenzone->addRect(0xE3, 0x60, 0x011B, 0x6B, nullptr, 0, "EXITMENU", "0");
	_submenzone->addRect(0x42, 0x34, 0x5C, 0x4E, nullptr, 0, "RECTEASY", "0");
	_submenzone->addRect(0x42, 0x53, 0x5C, 0x70, nullptr, 0, "RECTAVG", "0");
	_submenzone->addRect(0x42, 0x72, 0x62, 0x8A, nullptr, 0, "RECTHARD", "0");

	_shotSound = loadSoundFile("blow.8b");
	_emptySound = loadSoundFile("empty.8b");
	_saveSound = loadSoundFile("saved.8b");
	_loadSound = loadSoundFile("loaded.8b");
	_skullSound = loadSoundFile("skull.8b");
	_easySound = loadSoundFile("deputy.8b");
	_avgSound = loadSoundFile("sheriff.8b");
	_hardSound = loadSoundFile("marshall.8b");

	_gun = AlgGraphics::loadScreenCoordAniImage("gun.ani", _palette);
	_numbers = AlgGraphics::loadAniImage("numbers.ani", _palette);
	auto bullet = AlgGraphics::loadAniImage("bullet.ani", _palette);
	_shotIcon = (*bullet)[0];
	_emptyIcon = (*bullet)[1];
	auto hat = AlgGraphics::loadAniImage("hat.ani", _palette);
	_liveIcon = (*hat)[0];
	_deadIcon = (*hat)[1];
	auto shootout = AlgGraphics::loadAniImage("shootout.ani", _palette);
	_reloadIcon = (*shootout)[0];
	_drawIcon = (*shootout)[1];
	auto knife = AlgGraphics::loadScreenCoordAniImage("knife.ani", _palette);
	_knifeIcon = (*knife)[0];
	auto hole = AlgGraphics::loadScreenCoordAniImage("hole.ani", _palette);
	_bulletholeIcon = (*hole)[0];

	_background = AlgGraphics::loadVgaBackground("backgrnd.vga", _palette);
	_screen->copyRectToSurface(_background->getPixels(), _background->pitch, 0, 0, _background->w, _background->h);

	moveMouse();
}

void GameMaddog::registerScriptFunctions() {
#define ZONE_PTRFB_FUNCTION(name, func) _zonePtrFb[name] = new MDScriptFunctionPoint(this, &GameMaddog::func);
	ZONE_PTRFB_FUNCTION("DEFAULT", zoneBullethole);
	ZONE_PTRFB_FUNCTION("GLOBALHIT", zoneGlobalHit);
	ZONE_PTRFB_FUNCTION("BULLETHOLE", zoneBullethole);
	ZONE_PTRFB_FUNCTION("SKULL", zoneSkullhole);
#undef ZONE_PTRFB_FUNCTION

#define RECT_HIT_FUNCTION(name, func) _rectHitFuncs[name] = new MDScriptFunctionRect(this, &GameMaddog::func);
	RECT_HIT_FUNCTION("DEFAULT", rectNewScene);
	RECT_HIT_FUNCTION("STARTMENU", rectStart);
	RECT_HIT_FUNCTION("SHOTMENU", rectShotMenu);
	RECT_HIT_FUNCTION("EXITMENU", rectExit);
	RECT_HIT_FUNCTION("CONTMENU", rectContinue);
	RECT_HIT_FUNCTION("RECTSAVE", rectSave);
	RECT_HIT_FUNCTION("RECTLOAD", rectLoad);
	RECT_HIT_FUNCTION("RECTEASY", rectEasy);
	RECT_HIT_FUNCTION("RECTAVG", rectAverage);
	RECT_HIT_FUNCTION("RECTHARD", rectHard);
	RECT_HIT_FUNCTION("STARTBOT", rectStartBottles);
	RECT_HIT_FUNCTION("HIDEFRONT", rectHideFront);
	RECT_HIT_FUNCTION("HIDEREAR", rectHideRear);
	RECT_HIT_FUNCTION("NEWSCENE", rectNewScene);
	RECT_HIT_FUNCTION("MENUSELECT", rectMenuSelect);
	RECT_HIT_FUNCTION("SKULL", rectSkull);
	RECT_HIT_FUNCTION("KILLMAN", rectKillMan);
	RECT_HIT_FUNCTION("KILLWOMAN", rectKillWoman);
	RECT_HIT_FUNCTION("PROSPSIGN", rectProspSign);
	RECT_HIT_FUNCTION("MINESIGN", rectMineSign);
	RECT_HIT_FUNCTION("MINEITEM1", rectMineItem1);
	RECT_HIT_FUNCTION("MINEITEM2", rectMineItem2);
	RECT_HIT_FUNCTION("MINEITEM3", rectMineItem3);
	RECT_HIT_FUNCTION("MINELANTERN", rectMineLantern);
	RECT_HIT_FUNCTION("SHOTHIDEOUT", rectShotHideout);
	RECT_HIT_FUNCTION("SHOTRIGHT", rectShotRight);
	RECT_HIT_FUNCTION("SHOTLEFT", rectShotLeft);
#undef RECT_HIT_FUNCTION

#define PRE_OPS_FUNCTION(name, func) _scenePreOps[name] = new MDScriptFunctionScene(this, &GameMaddog::func);
	PRE_OPS_FUNCTION("DRAWRCT", scenePsoDrawRct);
	PRE_OPS_FUNCTION("PAUSE", scenePsoPause);
	PRE_OPS_FUNCTION("PRESHOOTOUT", scenePsoShootout);
	PRE_OPS_FUNCTION("MDSHOOTOUT", scenePsoMDShootout);
	PRE_OPS_FUNCTION("FADEIN", scenePsoFadeIn);
	PRE_OPS_FUNCTION("PAUSFI", scenePsoPauseFadeIn);
	PRE_OPS_FUNCTION("PREREAD", scenePsoPreRead);
	PRE_OPS_FUNCTION("PAUSPR", scenePsoPausePreRead);
	PRE_OPS_FUNCTION("DEFAULT", scenePsoDrawRct);
#undef PRE_OPS_FUNCTION

#define INS_OPS_FUNCTION(name, func) _sceneInsOps[name] = new MDScriptFunctionScene(this, &GameMaddog::func);
	INS_OPS_FUNCTION("DEFAULT", sceneIsoDoNothing);
	INS_OPS_FUNCTION("PAUSE", sceneIsoPause);
	INS_OPS_FUNCTION("SPAUSE", sceneIsoShootPastPause);
	INS_OPS_FUNCTION("STARTGAME", sceneIsoStartGame);
	INS_OPS_FUNCTION("SHOOTPAST", sceneIsoShootPast);
	INS_OPS_FUNCTION("SKIPSALOON", sceneIsoSkipSaloon);
	INS_OPS_FUNCTION("SKIPSALOON2", sceneIsoSkipSaloon2);
	INS_OPS_FUNCTION("CHECKSALOON", sceneIsoCheckSaloon);
	INS_OPS_FUNCTION("INTOSTABLE", sceneIsoIntoStable);
	INS_OPS_FUNCTION("INTOFFICE", sceneIsoIntoOffice);
	INS_OPS_FUNCTION("INTOBANK_SP", sceneIsoIntoBank);
	INS_OPS_FUNCTION("CHKBARTNDR", sceneIsoCheckBartender);
	INS_OPS_FUNCTION("DIDHIDEOUT", sceneIsoDidHideout);
	INS_OPS_FUNCTION("DIDSIGNPOST", sceneIsoDidSignPost);
	INS_OPS_FUNCTION("DOSHOOTOUT", sceneIsoDoShootout);
	INS_OPS_FUNCTION("MDSHOOTOUT", sceneIsoMDShootout);
	INS_OPS_FUNCTION("SHOTINTO24", sceneIsoDoNothing);
	INS_OPS_FUNCTION("SHOTINTO116", sceneIsoShotInto116);
#undef INS_OPS_FUNCTION

#define NXT_SCN_FUNCTION(name, func) _sceneNxtScn[name] = new MDScriptFunctionScene(this, &GameMaddog::func);
	NXT_SCN_FUNCTION("DEFAULT", sceneDefaultNxtscn);
	NXT_SCN_FUNCTION("DRAWGUN", sceneDefaultNxtscn);
	NXT_SCN_FUNCTION("PICKBOTTLE", sceneNxtscnPickBottle);
	NXT_SCN_FUNCTION("DIED", sceneNxtscnDied);
	NXT_SCN_FUNCTION("AUTOSEL", sceneNxtscnAutoSelect);
	NXT_SCN_FUNCTION("FINSALOON", sceneNxtscnFinishSaloon);
	NXT_SCN_FUNCTION("FINOFFICE", sceneNxtscnFinishOffice);
	NXT_SCN_FUNCTION("FINSTABLE", sceneNxtscnFinishStable);
	NXT_SCN_FUNCTION("FINBANK", sceneNxtscnFinishBank);
	NXT_SCN_FUNCTION("PICSALOON", sceneNxtscnPicSaloon);
	NXT_SCN_FUNCTION("KILLMAN", sceneNxtscnKillMan);
	NXT_SCN_FUNCTION("KILLWOMAN", sceneNxtscnKillWoman);
	NXT_SCN_FUNCTION("BANK", sceneNxtscnBank);
	NXT_SCN_FUNCTION("STABLE", sceneNxtscnStable);
	NXT_SCN_FUNCTION("SAVPROSP", sceneNxtscnSavProsp);
	NXT_SCN_FUNCTION("PICKTOSS", sceneNxtscnPickToss);
	NXT_SCN_FUNCTION("HITTOSS", sceneNxtscnHitToss);
	NXT_SCN_FUNCTION("MISSTOSS", sceneNxtscnMissToss);
	NXT_SCN_FUNCTION("PICKSIGN", sceneNxtscnPickSign);
	NXT_SCN_FUNCTION("BROCKMAN", sceneNxtscnBRockMan);
	NXT_SCN_FUNCTION("LROCKMAN", sceneNxtscnLRockMan);
	NXT_SCN_FUNCTION("HOTELMEN", sceneNxtscnHotelMen);
#undef NXT_SCN_FUNCTION

	_sceneShowMsg["DEFAULT"] = new MDScriptFunctionScene(this, &GameMaddog::sceneSmDonothing);
	_sceneWepDwn["DEFAULT"] = new MDScriptFunctionScene(this, &GameMaddog::sceneDefaultWepdwn);
	_sceneScnScr["DEFAULT"] = new MDScriptFunctionScene(this, &GameMaddog::sceneDefaultScore);
	_sceneNxtFrm["DEFAULT"] = new MDScriptFunctionScene(this, &GameMaddog::sceneNxtfrm);
}

void GameMaddog::verifyScriptFunctions() {
	auto scenes = _sceneInfo->getScenes();
	for (auto scene : *scenes) {
		getScriptFunctionScene(PREOP, scene->_preop);
		getScriptFunctionScene(SHOWMSG, scene->_scnmsg);
		getScriptFunctionScene(INSOP, scene->_insop);
		getScriptFunctionScene(WEPDWN, scene->_wepdwn);
		getScriptFunctionScene(SCNSCR, scene->_scnscr);
		getScriptFunctionScene(NXTFRM, scene->_nxtfrm);
		getScriptFunctionScene(NXTSCN, scene->_nxtscn);
		for (auto zone : scene->_zones) {
			for (auto rect : zone->_rects) {
				getScriptFunctionRectHit(rect->_rectHit);
			}
		}
	}
}

MDScriptFunctionPoint GameMaddog::getScriptFunctionZonePtrFb(Common::String name) {
	auto it = _zonePtrFb.find(name);
	if (it != _zonePtrFb.end()) {
		return *it->_value;
	} else {
		error("GameMaddog::getScriptFunctionZonePtrFb(): Could not find zonePtrFb function: %s", name.c_str());
	}
}

MDScriptFunctionRect GameMaddog::getScriptFunctionRectHit(Common::String name) {
	auto it = _rectHitFuncs.find(name);
	if (it != _rectHitFuncs.end()) {
		return *it->_value;
	} else {
		error("GameMaddog::getScriptFunctionRectHit(): Could not find rectHit function: %s", name.c_str());
	}
}

MDScriptFunctionScene GameMaddog::getScriptFunctionScene(SceneFuncType type, Common::String name) {
	MDScriptFunctionSceneMap *functionMap;
	switch (type) {
	case PREOP:
		functionMap = &_scenePreOps;
		break;
	case SHOWMSG:
		functionMap = &_sceneShowMsg;
		break;
	case INSOP:
		functionMap = &_sceneInsOps;
		break;
	case WEPDWN:
		functionMap = &_sceneWepDwn;
		break;
	case SCNSCR:
		functionMap = &_sceneScnScr;
		break;
	case NXTFRM:
		functionMap = &_sceneNxtFrm;
		break;
	case NXTSCN:
		functionMap = &_sceneNxtScn;
		break;
	default:
		error("GameMaddog::getScriptFunctionScene(): Unkown scene script type: %u", type);
		break;
	}
	MDScriptFunctionSceneMap::iterator it;
	it = functionMap->find(name);
	if (it != functionMap->end()) {
		return *it->_value;
	} else {
		error("GameMaddog::getScriptFunctionScene(): Could not find scene type %u function: %s", type, name.c_str());
	}
}

void GameMaddog::callScriptFunctionZonePtrFb(Common::String name, Common::Point *point) {
	MDScriptFunctionPoint function = getScriptFunctionZonePtrFb(name);
	function(point);
}

void GameMaddog::callScriptFunctionRectHit(Common::String name, Rect *rect) {
	MDScriptFunctionRect function = getScriptFunctionRectHit(name);
	function(rect);
}

void GameMaddog::callScriptFunctionScene(SceneFuncType type, Common::String name, Scene *scene) {
	MDScriptFunctionScene function = getScriptFunctionScene(type, name);
	function(scene);
}

Common::Error GameMaddog::run() {
	init();
	newGame();
	_curScene = _startScene;
	Common::String oldscene;
	Rect *hitRect = nullptr;
	while (!_vm->shouldQuit()) {
		oldscene = _curScene;
		_fired = false;
		if (_curScene == "scene28") {
			_curScene = pickTown();
		}
		Scene *scene = _sceneInfo->findScene(_curScene);
		if (!loadScene(scene)) {
			error("GameMaddog::run(): Cannot find scene %s in libfile", scene->_name.c_str());
		}
		_nextFrameTime = getMsTime() + 100;
		callScriptFunctionScene(PREOP, scene->_preop, scene);
		_currentFrame = getFrame(scene);
		while (_currentFrame <= scene->_endFrame && _curScene == oldscene && !_vm->shouldQuit()) {
			updateMouse();
			// TODO: call scene->messageFunc
			callScriptFunctionScene(INSOP, scene->_insop, scene);
			_holster = weaponDown();
			if (_holster) {
				callScriptFunctionScene(WEPDWN, scene->_wepdwn, scene);
			}
			Common::Point firedCoords;
			if (fired(&firedCoords)) {
				if (!_holster) {
					Rect *hitGlobalRect = checkZone(_menuzone, &firedCoords);
					if (hitGlobalRect != nullptr) {
						callScriptFunctionRectHit(hitGlobalRect->_rectHit, hitGlobalRect);
					} else if (_shots > 0) {
						if (!_debug_unlimitedAmmo) {
							_shots--;
						}
						updateStat();
						hitRect = nullptr;
						Zone *hitSceneZone = checkZonesV1(scene, hitRect, &firedCoords);
						if (hitSceneZone != nullptr) {
							callScriptFunctionZonePtrFb(hitSceneZone->_ptrfb, &firedCoords);
							callScriptFunctionRectHit(hitRect->_rectHit, hitRect);
						} else {
							defaultBullethole(&firedCoords);
						}
					} else {
						playSound(_emptySound);
						_emptyCount = 3;
						_whichGun = 9;
					}
				}
			}
			if (_curScene == oldscene) {
				callScriptFunctionScene(NXTFRM, scene->_nxtfrm, scene);
			}
			displayScore();
			moveMouse();
			if (_pauseTime > 0) {
				_videoDecoder->pauseAudio(true);
			} else {
				_videoDecoder->pauseAudio(false);
			}
			if (_videoDecoder->getCurrentFrame() == 0) {
				_videoDecoder->getNextFrame();
			}
			updateScreen();
			int32 remainingMillis = _nextFrameTime - getMsTime();
			if (remainingMillis < 10) {
				if (_videoDecoder->getCurrentFrame() > 0) {
					_videoDecoder->getNextFrame();
				}
				remainingMillis = _nextFrameTime - getMsTime();
				_nextFrameTime = getMsTime() + (remainingMillis > 0 ? remainingMillis : 0) + 100;
			}
			if (remainingMillis > 0) {
				if (remainingMillis > 15) {
					remainingMillis = 15;
				}
				g_system->delayMillis(remainingMillis);
			}
			_currentFrame = getFrame(scene);
		}
		// frame limit reached or scene changed, prepare for next scene
		_hadPause = false;
		_pauseTime = 0;
		if (_retScene != "") {
			_curScene = _retScene;
			_retScene = "";
		}
		if (_subScene != "") {
			_retScene = _subScene;
			_subScene = "";
		}
		if (_curScene == oldscene) {
			callScriptFunctionScene(NXTSCN, scene->_nxtscn, scene);
		}
		if (_curScene == "") {
			shutdown();
		}
	}
	removeCursorTimer();
	return Common::kNoError;
}

void GameMaddog::newGame() {
	_shots = 6;
	_lives = 3;
	_score = 0;
	_holster = false;
	updateStat();
	_subScene = "";
}

void GameMaddog::resetParams() {
	_beenTo = 0;
	_bottles = 0;
	_bottlesMask = 0;
	_gotInto = 0;
	_hadSkull = false;
	_badMen = 0;
	_badMenBits = 0;
	_peopleKilled = 0;
	_hideOutFront = false;
	_difficulty = 1;
	_gunTime = 0;
	_proClue = 0;
	_gotClue = false;
	_hadLantern = false;
	_mapPos = 0;
	_shootOutCnt = 0;
	_maxMapPos = 0;
	_sheriffCnt = 0;
	_inShootout = false;
	_retScene = "";
	_subScene = "";
}

void GameMaddog::doMenu() {
	uint32 startTime = getMsTime();
	updateCursor();
	_inMenu = true;
	moveMouse();
	_videoDecoder->pauseAudio(true);
	_screen->copyRectToSurface(_background->getBasePtr(_videoPosX, _videoPosY), _background->pitch, _videoPosX, _videoPosY, _videoDecoder->getWidth(), _videoDecoder->getHeight());
	showDifficulty(_difficulty, false);
	while (_inMenu && !_vm->shouldQuit()) {
		Common::Point firedCoords;
		if (fired(&firedCoords)) {
			Rect *hitMenuRect = checkZone(_submenzone, &firedCoords);
			if (hitMenuRect != nullptr) {
				callScriptFunctionRectHit(hitMenuRect->_rectHit, hitMenuRect);
			}
		}
		if (_difficulty != _oldDifficulty) {
			changeDifficulty(_difficulty);
		}
		updateScreen();
		g_system->delayMillis(15);
	}
	updateCursor();
	_videoDecoder->pauseAudio(false);
	if (_hadPause) {
		uint32 endTime = getMsTime();
		uint32 timeDiff = endTime - startTime;
		_pauseTime += timeDiff;
		_nextFrameTime += timeDiff;
	}
}

void GameMaddog::updateStat() {
	if (_lives != _oldLives) {
		if (_lives > _oldLives) {
			for (uint8 i = _oldLives; i < _lives; i++) {
				AlgGraphics::drawImage(_screen, _liveIcon, _livePos[i][0], _livePos[i][1]);
			}
		} else {
			for (uint8 i = _lives; i < _oldLives; i++) {
				AlgGraphics::drawImage(_screen, _deadIcon, _livePos[i][0], _livePos[i][1]);
			}
		}
		_oldLives = _lives;
	}
	if (_shots != _oldShots) {
		if (_shots > _oldShots) {
			for (uint8 i = _oldShots; i < _shots; i++) {
				AlgGraphics::drawImage(_screen, _shotIcon, _shotPos[i][0], _shotPos[i][1]);
			}
		} else {
			for (uint8 i = _shots; i < _oldShots; i++) {
				AlgGraphics::drawImage(_screen, _emptyIcon, _shotPos[i][0], _shotPos[i][1]);
			}
		}
		_oldShots = _shots;
	}
}

void GameMaddog::changeDifficulty(uint8 newDifficulty) {
	if (newDifficulty == _oldDifficulty) {
		return;
	}
	showDifficulty(newDifficulty, true);
	Game::adjustDifficulty(newDifficulty, _oldDifficulty);
	_oldDifficulty = newDifficulty;
	_difficulty = newDifficulty;
}

void GameMaddog::showDifficulty(uint8 newDifficulty, bool cursor) {
	// reset menu screen
	_screen->copyRectToSurface(_background->getBasePtr(_videoPosX, _videoPosY), _background->pitch, _videoPosX, _videoPosY, _videoDecoder->getWidth(), _videoDecoder->getHeight());
	AlgGraphics::drawImageCentered(_screen, _knifeIcon, _diffPos[newDifficulty][0], _diffPos[newDifficulty][1]);
	if (cursor) {
		updateCursor();
	}
}

void GameMaddog::updateCursor() {
	updateMouse();
}

void GameMaddog::updateMouse() {
	if (_oldWhichGun != _whichGun) {
		Graphics::Surface *cursor = (*_gun)[_whichGun];
		uint16 hotspotX = (cursor->w / 2);
		uint16 hotspotY = (cursor->h / 2);
		if (debugChannelSet(1, Alg::kAlgDebugGraphics)) {
			cursor->drawLine(0, hotspotY, cursor->w, hotspotY, 1);
			cursor->drawLine(hotspotX, 0, hotspotX, cursor->h, 1);
		}
		CursorMan.replaceCursor(cursor->getPixels(), cursor->w, cursor->h, hotspotX, hotspotY, 0);
		CursorMan.showMouse(true);
		_oldWhichGun = _whichGun;
	}
}

void GameMaddog::moveMouse() {
	if (_mousePos.x < 59 || _inMenu) {
		_whichGun = 8;
	} else if (_mousePos.y > 166) {
		if (_inHolster == 0)
			_whichGun = 6;
		else
			_whichGun = 7;
	} else if (_whichGun > 5) {
		_whichGun = 0;
	}
	updateMouse();
}

void GameMaddog::displayScore() {
	if (_score == _oldScore) {
		return;
	}
	_oldScore = _score;
	Common::String scoreString = Common::String::format("%05d", _score);
	int posX = 0xDC;
	for (int i = 0; i < 5; i++) {
		int digit;
		if (scoreString[i] == '0') {
			digit = 9;
		} else {
			digit = scoreString[i] - '0' - 1;
		}
		AlgGraphics::drawImage(_screen, (*_numbers)[digit], posX, 0xAD);
		posX += 10;
	}
}

bool GameMaddog::weaponDown() {
	if (_rightDown && _mousePos.y > 168) {
		return true;
	}
	return false;
}

bool GameMaddog::saveState() {
	Common::OutSaveFile *outSaveFile;
	Common::String saveFileName = _vm->getSaveStateName(0);
	if (!(outSaveFile = g_system->getSavefileManager()->openForSaving(saveFileName))) {
		warning("GameMaddog::saveState(): Can't create file '%s', game not saved", saveFileName.c_str());
		return false;
	}
	outSaveFile->writeUint32BE(MKTAG('A', 'L', 'G', 'S')); // header
	outSaveFile->writeByte(0);                             // version, unused for now
	outSaveFile->writeUint16LE(_beenTo);
	outSaveFile->writeUint16LE(_gotInto);
	outSaveFile->writeByte(_hadSkull);
	outSaveFile->writeByte(_badMen);
	outSaveFile->writeByte(_badMenBits);
	outSaveFile->writeByte(_peopleKilled);
	outSaveFile->writeByte(_hideOutFront);
	outSaveFile->writeByte(_difficulty);
	outSaveFile->writeByte(_proClue);
	outSaveFile->writeByte(_gotClue);
	outSaveFile->writeByte(_hadLantern);
	outSaveFile->writeByte(_mapPos);
	outSaveFile->writeByte(_shootOutCnt);
	outSaveFile->writeSByte(_map0);
	outSaveFile->writeSByte(_map1);
	outSaveFile->writeSByte(_map2);
	outSaveFile->writeByte(_maxMapPos);
	outSaveFile->writeByte(_bartenderAlive);
	outSaveFile->writeByte(_sheriffCnt);
	outSaveFile->writeByte(_inShootout);
	outSaveFile->writeString(_curScene);
	outSaveFile->writeByte(0);
	outSaveFile->writeString(_retScene);
	outSaveFile->writeByte(0);
	outSaveFile->writeString(_subScene);
	outSaveFile->writeByte(0);
	outSaveFile->finalize();
	delete outSaveFile;
	return true;
}

bool GameMaddog::loadState() {
	Common::InSaveFile *inSaveFile;
	Common::String saveFileName = _vm->getSaveStateName(0);
	if (!(inSaveFile = g_system->getSavefileManager()->openForLoading(saveFileName))) {
		debug("GameMaddog::loadState(): Can't load file '%s', game not loaded", saveFileName.c_str());
		return false;
	}
	uint32 header = inSaveFile->readUint32BE();
	if (header != MKTAG('A', 'L', 'G', 'S')) {
		warning("GameMaddog::loadState(): Unkown save file, header: %s", tag2str(header));
		return false;
	}
	inSaveFile->skip(1); // version, unused for now
	_beenTo = inSaveFile->readUint16LE();
	_gotInto = inSaveFile->readUint16LE();
	_hadSkull = inSaveFile->readByte();
	_badMen = inSaveFile->readByte();
	_badMenBits = inSaveFile->readByte();
	_peopleKilled = inSaveFile->readByte();
	_hideOutFront = inSaveFile->readByte();
	_difficulty = inSaveFile->readByte();
	_proClue = inSaveFile->readByte();
	_gotClue = inSaveFile->readByte();
	_hadLantern = inSaveFile->readByte();
	_mapPos = inSaveFile->readByte();
	_shootOutCnt = inSaveFile->readByte();
	_map0 = inSaveFile->readSByte();
	_map1 = inSaveFile->readSByte();
	_map2 = inSaveFile->readSByte();
	_maxMapPos = inSaveFile->readByte();
	_bartenderAlive = inSaveFile->readByte();
	_sheriffCnt = inSaveFile->readByte();
	_inShootout = inSaveFile->readByte();
	_curScene = inSaveFile->readString();
	_retScene = inSaveFile->readString();
	_subScene = inSaveFile->readString();
	delete inSaveFile;
	changeDifficulty(_difficulty);
	return true;
}

// misc game functions
void GameMaddog::defaultBullethole(Common::Point *point) {
	if (point->x >= 59 && point->y <= 166) {
		uint16 targetX = point->x - _videoPosX;
		uint16 targetY = point->y - _videoPosY;
		AlgGraphics::drawImageCentered(_videoDecoder->getVideoFrame(), _bulletholeIcon, targetX, targetY);
		updateCursor();
		_shotFired = true;
		doShot();
	}
}

void GameMaddog::die() {
	Common::String newScene;
	updateStat();
	switch (_lives) {
	case 2:
		newScene = "scene150";
		break;
	case 1:
		newScene = "scene152";
		break;
	case 0:
		newScene = "scene153";
		break;
	default:
		int nextSceneNum = (_rnd->getRandomNumber(1)) + 148;
		newScene = Common::String::format("scene%d", nextSceneNum);
		break;
	}
	_curScene = newScene;
}

uint8 GameMaddog::pickRand(uint8 *bits, uint8 max) {
	uint8 randomNum, mask;
	// reset bits if full
	if (*bits == (0xFF >> (8 - max))) {
		*bits = 0;
	}
	do {
		randomNum = _rnd->getRandomNumber(max - 1);
		mask = 1 << randomNum;
	} while (*bits & mask);
	*bits |= mask;
	return randomNum * 2;
}

uint8 GameMaddog::pickBad(uint8 max) {
	return pickRand(&_badMenBits, max);
}

Common::String GameMaddog::pickTown() {
	_hadSkull = false;
	_mapPos = 0;
	_badMenBits = 0;
	_shootOutCnt++;
	if (_shootOutCnt % 5 == 0) {
		if (_shootOutCnt > 15 || _shootOutCnt == 0) {
			_shootOutCnt = 5;
		}
		// surprise showdown!
		int pickedSceneNum = (_shootOutCnt / 5) + 106;
		return Common::String::format("scene%d", pickedSceneNum);
	} else if (_beenTo == 0) {
		return "scene28";
	} else if (_beenTo & 0x100) {
		return "scene250";
	} else if (_beenTo & 0x80) {
		return "scene76";
	} else if (_beenTo & 0x40) {
		return "scene214";
	} else if (_beenTo >= 15) {
		return "scene186";
	} else {
		return Common::String::format("scene%d", _beenTo + 29);
	}
}

Common::String GameMaddog::pickMap() {
	_beenTo |= 0x20;
	uint8 randomNum = _rnd->getRandomNumber(5);
	switch (randomNum) {
	case 0:
		_map0 = 1;
		_map1 = 1;
		_map2 = 0;
		return "scene164";
	case 1:
		_map0 = 1;
		_map1 = -1;
		_map2 = 0;
		return "scene165";
	case 2:
		_map0 = -1;
		_map1 = 1;
		_map2 = 1;
		return "scene166";
	case 3:
		_map0 = -1;
		_map1 = 1;
		_map2 = -1;
		return "scene167";
	case 4:
		_map0 = -1;
		_map1 = -1;
		_map2 = 1;
		return "scene168";
	case 5:
	default:
		_map0 = -1;
		_map1 = -1;
		_map2 = -1;
		return "scene169";
	}
}

Common::String GameMaddog::pickSign() {
	int8 _mapArray[3] = {_map0, _map1, _map2};
	_mapPos++;
	if (_mapPos > _maxMapPos) {
		_maxMapPos = _mapPos;
	}
	if (_mapPos <= 2 && _mapArray[_mapPos] != 0) {
		return Common::String::format("scene%d", _mapPos + 187);
	} else {
		return "scene210";
	}
}

Common::String GameMaddog::mapRight() {
	int8 _mapArray[3] = {_map0, _map1, _map2};
	if (_mapArray[_mapPos] == -1) {
		if (_mapPos >= _maxMapPos) {
			return Common::String::format("scene%d", _fight[_mapPos]);
		} else {
			return pickSign();
		}
	} else if (_mapArray[_mapPos] == 0) {
		if (_mapPos >= _maxMapPos) {
			return Common::String::format("scene%d", _fight[_mapPos]);
		} else {
			return pickSign();
		}
	} else {
		return Common::String::format("scene%d", _ambush[_mapPos]);
	}
}

Common::String GameMaddog::mapLeft() {
	int8 _mapArray[3] = {_map0, _map1, _map2};
	if (_mapArray[_mapPos] == 1) {
		if (_mapPos >= _maxMapPos) {
			return Common::String::format("scene%d", _fight[_mapPos]);
		} else {
			return pickSign();
		}
	} else if (_mapArray[_mapPos] == -1) {
		return Common::String::format("scene%d", _ambush[_mapPos]);
	} else {
		if (_mapPos >= _maxMapPos) {
			return Common::String::format("scene%d", _fight[_mapPos]);
		} else {
			return pickSign();
		}
	}
}

// Script functions: Zone
void GameMaddog::zoneBullethole(Common::Point *point) {
	defaultBullethole(point);
}

void GameMaddog::zoneSkullhole(Common::Point *point) {
	if (point->x >= 59 && point->y <= 166) {
		uint16 targetX = point->x - _videoPosX;
		uint16 targetY = point->y - _videoPosY;
		AlgGraphics::drawImageCentered(_videoDecoder->getVideoFrame(), _bulletholeIcon, targetX, targetY);
		updateCursor();
		_shotFired = true;

		if (_hadSkull) {
			doShot();
		} else {
			doSkullSound();
		}
	}
}

// Script functions: RectHit
void GameMaddog::rectHideFront(Rect *rect) {
	if (_hideOutFront) {
		_curScene = "scene214";
	} else {
		_curScene = "scene211";
	}
}

void GameMaddog::rectHideRear(Rect *rect) {
	if (!_hideOutFront) {
		_curScene = "scene214";
	} else {
		_curScene = "scene211";
	}
}

void GameMaddog::rectMenuSelect(Rect *rect) {
	Common::String newScene;
	int cursorX = _mousePos.x;
	int cursorY = _mousePos.y;
	if (cursorX < 184) {
		if (cursorY < 88) {
			if (_beenTo & 2)
				return;
			if (_gotInto & 2) {
				newScene = "scene130";
			} else {
				newScene = "scene122";
			}
		} else {
			if (_beenTo & 8)
				return;
			if (_gotInto & 8) {
				if (_beenTo & 1) {
					newScene = "scene118";
				} else {
					newScene = "scene119";
				}
			} else {
				newScene = "scene114";
			}
		}
	} else {
		if (cursorY < 88) {
			if (_beenTo & 1)
				return;
			if (_gotInto & 1) {
				newScene = "scene69b";
			} else {
				newScene = "scene67";
			}
		} else {
			if (_beenTo & 4)
				return;
			if (_gotInto & 4) {
				sceneNxtscnBank(nullptr);
				return;
			} else {
				newScene = "scene45";
			}
		}
	}

	_curScene = newScene;
}

void GameMaddog::rectSkull(Rect *rect) {
	if (_hadSkull) {
		return;
	}
	_hadSkull = true;
	if (_beenTo < 15) {
		_shots = 9;
	} else {
		_shots = 12;
	}
	updateStat();
}

void GameMaddog::rectKillMan(Rect *rect) {
	sceneNxtscnKillMan(nullptr);
}

void GameMaddog::rectKillWoman(Rect *rect) {
	sceneNxtscnKillWoman(nullptr);
}

void GameMaddog::rectProspSign(Rect *rect) {
	if (_beenTo & 0x10) {
		return;
	}
	_gunTime = 1;
	_curScene = rect->_scene;
}

void GameMaddog::rectMineSign(Rect *rect) {
	if (_beenTo & 0x20) {
		return;
	}
	_gunTime = 1;
	_curScene = rect->_scene;
}

void GameMaddog::rectMineItem1(Rect *rect) {
	if (_proClue != 0) {
		_pauseTime = 0;
		return;
	}

	if (_hadLantern) {
		_curScene = pickMap();
	} else {
		_gotClue = true;
	}
}

void GameMaddog::rectMineItem2(Rect *rect) {
	if (_proClue != 2) {
		_pauseTime = 0;
		return;
	}

	if (_hadLantern) {
		_curScene = pickMap();
	} else {
		_gotClue = true;
	}
}

void GameMaddog::rectMineItem3(Rect *rect) {
	if (_proClue != 1) {
		_pauseTime = 0;
		return;
	}

	if (_hadLantern) {
		_curScene = pickMap();
	} else {
		_gotClue = true;
	}
}

void GameMaddog::rectMineLantern(Rect *rect) {
	_hadLantern = true;

	if (!_gotClue) {
		return;
	}

	_curScene = pickMap();
}

void GameMaddog::rectShotHideout(Rect *rect) {
	_curScene = pickSign();
}

void GameMaddog::rectShotRight(Rect *rect) {
	_curScene = mapRight();
}

void GameMaddog::rectShotLeft(Rect *rect) {
	_curScene = mapLeft();
}

void GameMaddog::rectShotMenu(Rect *rect) {
	doMenu();
}

void GameMaddog::rectContinue(Rect *rect) {
	_inMenu = false;
	_fired = false;
	if (_lives == 0) {
		newGame();
		_retScene = "";
		_curScene = pickTown();
	}
}

void GameMaddog::rectSave(Rect *rect) {
	if (saveState()) {
		doSaveSound();
	}
}

void GameMaddog::rectLoad(Rect *rect) {
	if (loadState()) {
		doLoadSound();
	}
}

void GameMaddog::rectStart(Rect *rect) {
	_inMenu = false;
	_fired = false;
	Scene *scene = _sceneInfo->findScene(_startScene);
	if (scene->_nxtscn == "DRAWGUN") {
		callScriptFunctionScene(NXTSCN, "DRAWGUN", scene);
	}
	_curScene = _startScene;
	resetParams();
	newGame();
	updateStat();
}

void GameMaddog::rectStartBottles(Rect *rect) {
	_inMenu = false;
	_fired = false;
	_curScene = "scene7";
	resetParams();
	newGame();
	updateStat();
}

// Script functions: Scene PreOps
void GameMaddog::scenePsoShootout(Scene *scene) {
	sscanf(scene->_preopParam.c_str(), "#%ldto%ld", &_minF, &_maxF);
	if (!_debug_unlimitedAmmo) {
		_shots = 0;
	}
	_inShootout = true;
	updateStat();
	AlgGraphics::drawImage(_screen, _reloadIcon, 0x40, 0xB0);
	updateCursor();
}

void GameMaddog::scenePsoMDShootout(Scene *scene) {
	sscanf(scene->_preopParam.c_str(), "#%ldto%ld", &_minF, &_maxF);
	if (!_debug_unlimitedAmmo) {
		_shots = 0;
	}
	_inShootout = true;
	updateStat();
	AlgGraphics::drawImage(_screen, _reloadIcon, 0x40, 0xB0);
	updateCursor();
}

// Script functions: Scene Scene InsOps
void GameMaddog::sceneIsoShootPast(Scene *scene) {
	if (_fired) {
		if (_retScene != "") {
			_curScene = _retScene;
			_retScene = "";
		} else if (_subScene != "") {
			_curScene = _subScene;
			_subScene = "";
		} else {
			callScriptFunctionScene(NXTSCN, scene->_nxtscn, scene);
		}
	}
}

void GameMaddog::sceneIsoShootPastPause(Scene *scene) {
	sceneIsoShootPast(scene);
	sceneIsoPause(scene);
}

void GameMaddog::sceneIsoSkipSaloon(Scene *scene) {
	if (_gotInto & 1) {
		if (_currentFrame < 7561) {
			_curScene = scene->_insopParam;
			_pauseTime = 0;
			return;
		}
	}
	if (_currentFrame > 7561) {
		_gotInto |= 1;
	}
	if (_fired && _currentFrame > 7165 && _currentFrame < 7817) {
		_curScene = scene->_insopParam;
	}
}

void GameMaddog::sceneIsoSkipSaloon2(Scene *scene) {
	Common::String insopParamTemp = scene->_insopParam;
	scene->_insopParam = Common::String::format("%u", scene->_dataParam2);
	sceneIsoPause(scene);
	scene->_insopParam = insopParamTemp;
	sceneIsoSkipSaloon(scene);
}

void GameMaddog::sceneIsoCheckSaloon(Scene *scene) {
	_gotInto |= 1;
	if (_currentFrame > 7909) {
		_bartenderAlive = false;
	} else {
		_bartenderAlive = true;
	}
}

void GameMaddog::sceneIsoIntoStable(Scene *scene) {
	_gotInto |= 2;
}

void GameMaddog::sceneIsoIntoOffice(Scene *scene) {
	_gotInto |= 8;
}

void GameMaddog::sceneIsoIntoBank(Scene *scene) {
	_gotInto |= 4;
	sceneIsoShootPast(scene);
}

void GameMaddog::sceneIsoCheckBartender(Scene *scene) {
	if (!_bartenderAlive) {
		if (scene->_dataParam1 <= (int32)_currentFrame) {
			_curScene = scene->_insopParam;
		}
	}
	if (_fired) {
		if (scene->_dataParam2 < (int32)_currentFrame) {
			callScriptFunctionScene(NXTSCN, scene->_nxtscn, scene);
		}
	}
}

void GameMaddog::sceneIsoDidHideout(Scene *scene) {
	_beenTo |= 0x80;
}

void GameMaddog::sceneIsoDidSignPost(Scene *scene) {
	_beenTo |= 0x40;
}

void GameMaddog::sceneIsoDoShootout(Scene *scene) {
	if (_currentFrame > (uint32)_minF) {
		if (_inShootout) {
			AlgGraphics::drawImage(_screen, _drawIcon, 0x40, 0xB0);
			updateCursor();
		}
		_inShootout = false;
		if (_shots > 0) {
			if (_currentFrame < (uint32)_maxF) {
				callScriptFunctionScene(NXTSCN, scene->_nxtscn, scene);
			}
		}
	}
}

void GameMaddog::sceneIsoMDShootout(Scene *scene) {
	_beenTo |= 0x100;
	sceneIsoDoShootout(scene);
}

void GameMaddog::sceneIsoShotInto116(Scene *scene) {
	uint32 targetFrame = atoi(scene->_insopParam.c_str());
	if (_fired) {
		if (_currentFrame > targetFrame) {
			callScriptFunctionScene(NXTSCN, scene->_nxtscn, scene);
		}
	}
}

// Script functions: Scene NxtScn
void GameMaddog::sceneDefaultNxtscn(Scene *scene) {
	// wipe background drawing from shootout
	_screen->copyRectToSurface(_background->getBasePtr(0x40, 0xB0), _background->pitch, 0x40, 0xB0, _reloadIcon->w, _reloadIcon->h);
	updateCursor();
	Game::sceneDefaultNxtscn(scene);
}

void GameMaddog::sceneNxtscnPickBottle(Scene *scene) {
	_bottles++;
	if (_bottles < 4) {
		int rand = pickRand(&_bottlesMask, 6);
		_curScene = Common::String::format("scene%d", rand + 11);
	} else {
		_curScene = "scene253";
	}
}

void GameMaddog::sceneNxtscnDied(Scene *scene) {
	_hadSkull = false;
	_badMenBits = 0;
	_badMen = 0;
	_gotClue = false;
	_hadLantern = false;
	if (!_debug_godMode) {
		_lives--;
	}
	if (_lives <= 0) {
		_lives = 0;
		_subScene = "scene255";
		die();
		return;
	}
	_retScene = scene->_next;
	die();
}

void GameMaddog::sceneNxtscnAutoSelect(Scene *scene) {
	Common::String newScene;
	if (!(_beenTo & 2)) {
		newScene = "scene122";
	} else if (!(_beenTo & 8)) {
		newScene = "scene114";
	} else if (!(_beenTo & 1)) {
		if (_gotInto & 1) {
			newScene = "scene69";
		} else {
			newScene = "scene67";
		}
	} else if (!(_beenTo & 4)) {
		newScene = "scene45";
	} else {
		newScene = "scene186";
	}
	_curScene = newScene;
}

void GameMaddog::sceneNxtscnFinishSaloon(Scene *scene) {
	_beenTo |= 1;
	_curScene = pickTown();
}

void GameMaddog::sceneNxtscnFinishOffice(Scene *scene) {
	_beenTo |= 8;
	_curScene = pickTown();
}

void GameMaddog::sceneNxtscnFinishStable(Scene *scene) {
	_beenTo |= 2;
	_curScene = pickTown();
}

void GameMaddog::sceneNxtscnFinishBank(Scene *scene) {
	_beenTo |= 4;
	_curScene = pickTown();
}

void GameMaddog::sceneNxtscnPicSaloon(Scene *scene) {
	if (_beenTo & 1) {
		_curScene = "scene118";
	} else {
		_curScene = "scene119";
	}
}

void GameMaddog::sceneNxtscnKillMan(Scene *scene) {
	if (!_debug_godMode) {
		_lives--;
	}
	if (_lives <= 0) {
		_lives = 0;
		_subScene = "scene212";
	} else {
		_subScene = pickTown();
	}
	updateStat();
	_badMenBits = 0;
	_badMen = 0;
	_peopleKilled++;
	if (_peopleKilled == 1) {
		_curScene = "scene155";
	} else {
		_curScene = Common::String::format("scene%d", 156 + (_peopleKilled & 1));
	}
}

void GameMaddog::sceneNxtscnKillWoman(Scene *scene) {
	if (!_debug_godMode) {
		_lives--;
	}
	if (_lives <= 0) {
		_lives = 0;
		_subScene = "scene212";
	} else {
		_subScene = pickTown();
	}
	updateStat();
	_badMenBits = 0;
	_badMen = 0;
	_peopleKilled++;
	if (_peopleKilled == 1) {
		_curScene = "scene154";
	} else {
		_curScene = Common::String::format("scene%d", 156 + (_peopleKilled & 1));
	}
}

void GameMaddog::sceneNxtscnBank(Scene *scene) {
	Common::String newScene;
	uint8 totalBadmen = (_difficulty * 2) + 6;
	_badMen++;
	if (_badMen > totalBadmen) {
		if (_badMen > totalBadmen + 2) {
			_beenTo |= 4;
			_badMenBits = 0;
			_badMen = 0;
			if (_rnd->getRandomBit()) {
				_hideOutFront = true;
				newScene = "scene49";
			} else {
				_hideOutFront = false;
				newScene = "scene48";
			}
		} else {
			newScene = "scene65";
		}
	} else {
		int nextSceneNum = pickBad(6) + 51;
		newScene = Common::String::format("scene%d", nextSceneNum);
	}
	_curScene = newScene;
}

void GameMaddog::sceneNxtscnStable(Scene *scene) {
	Common::String newScene;
	uint16 totalBadMen = (_difficulty * 2) + 6;
	totalBadMen -= (_beenTo & 8) ? 2 : 0;
	_badMen++;
	if (_badMen > totalBadMen) {
		_badMenBits = 0;
		_badMen = 0;
		newScene = "scene143";
	} else {
		int nextSceneNum = pickBad(6) + 131;
		newScene = Common::String::format("scene%d", nextSceneNum);
	}
	_curScene = newScene;
}

void GameMaddog::sceneNxtscnSavProsp(Scene *scene) {
	_gunTime = 1;
	_oldScore = -1;
	_proClue = _rnd->getRandomNumber(2);
	_beenTo |= 0x10;
	_curScene = Common::String::format("scene%d", _proClue + 160);
}

void GameMaddog::sceneNxtscnPickToss(Scene *scene) {
	int index = pickBad(7);
	_curScene = Common::String::format("scene%d", _bottleToss[index]);
}

void GameMaddog::sceneNxtscnHitToss(Scene *scene) {
	if (_lives > 0) {
		_score += 100;
	}
	sceneNxtscnMissToss(scene);
}

void GameMaddog::sceneNxtscnMissToss(Scene *scene) {
	_badMen++;
	if (_badMen <= 2) {
		_curScene = scene->_next;
	} else {
		_badMenBits = 0;
		_badMen = 0;
		_beenTo |= 0x200;
		_curScene = "scene185";
	}
}

void GameMaddog::sceneNxtscnPickSign(Scene *scene) {
	_curScene = pickSign();
}

void GameMaddog::sceneNxtscnBRockMan(Scene *scene) {
	long totalBadMen = (_difficulty * 2) + 9;
	_badMen++;
	if (_badMen > totalBadMen) {
		_badMenBits = 0;
		_badMen = 0;
		_curScene = pickSign();
	} else {
		int nextBad = pickBad(7);
		_curScene = Common::String::format("scene%d", nextBad + 229);
	}
}

void GameMaddog::sceneNxtscnLRockMan(Scene *scene) {
	long totalBadMen = (_difficulty * 2) + 4;
	_badMen++;
	if (_badMen > totalBadMen) {
		_badMenBits = 0;
		_badMen = 0;
		_curScene = pickSign();
	} else {
		int nextBad = pickBad(3);
		_curScene = Common::String::format("scene%d", nextBad + 244);
	}
}

void GameMaddog::sceneNxtscnHotelMen(Scene *scene) {
	long totalBadMen = (_difficulty * 2) + 9;
	if (_badMen >= totalBadMen) {
		_badMenBits = 0;
		_badMen = 0;
		_beenTo |= 0x100;
		_curScene = "scene250";
	} else {
		_badMen++;
		uint32 index = pickBad(5);
		_curScene = Common::String::format("scene%d", _hotelScenes[index]);
	}
}

// Script functions: WepDwn
void GameMaddog::sceneDefaultWepdwn(Scene *scene) {
	_inHolster = 9;
	_whichGun = 7;
	updateMouse();
	if (_inShootout == 0) {
		if (_beenTo >= 15) {
			if (_shots < 12) {
				_shots = 12;
			}
		} else {
			if (_shots < 6) {
				_shots = 6;
			}
		}
		updateStat();
	}
}

// Debug methods
void GameMaddog::debugWarpTo(int val) {
	switch (val) {
	case 0:
		_beenTo = 0;
		_curScene = "scene28";
		break;
	case 1:
		_beenTo = 1;
		_curScene = pickTown();
		break;
	case 2:
		_beenTo = 15;
		_curScene = pickTown();
		break;
	case 3:
		_beenTo = 575;
		// always go right
		_map0 = -1;
		_map1 = -1;
		_map2 = -1;
		_curScene = pickTown();
		break;
	case 4:
		_beenTo = 575;
		_hideOutFront = true; // go to front
		_curScene = "scene210";
		break;
	case 5:
		_beenTo = 639;
		_curScene = "scene227";
		break;
	case 6:
		_beenTo = 1023;
		_curScene = "scene250";
		break;
	default:
		break;
	}
}

// Debugger methods
DebuggerMaddog::DebuggerMaddog(GameMaddog *game) {
	_game = game;
	registerVar("drawRects", &game->_debug_drawRects);
	registerVar("godMode", &game->_debug_godMode);
	registerVar("unlimitedAmmo", &game->_debug_unlimitedAmmo);
	registerCmd("warpTo", WRAP_METHOD(DebuggerMaddog, cmdWarpTo));
	registerCmd("dumpLib", WRAP_METHOD(DebuggerMaddog, cmdDumpLib));
}

bool DebuggerMaddog::cmdWarpTo(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: warp <int>");
		return true;
	} else {
		int val = atoi(argv[1]);
		_game->debugWarpTo(val);
		return false;
	}
}

bool DebuggerMaddog::cmdDumpLib(int argc, const char **argv) {
	return _game->debug_dumpLibFile();
}

} // End of namespace Alg
