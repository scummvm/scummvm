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
#include "alg/logic/game_johnnyrock.h"
#include "alg/scene.h"

namespace Alg {

GameJohnnyRock::GameJohnnyRock(AlgEngine *vm, const AlgGameDescription *gd) : Game(vm) {
}

GameJohnnyRock::~GameJohnnyRock() {
	for (auto item : *_difficultyIcon) {
		item->free();
		delete item;
	}
	if (_levelIcon) {
		_levelIcon->free();
		delete _levelIcon;
	}
	if (_bulletholeIcon) {
		_bulletholeIcon->free();
		delete _bulletholeIcon;
	}
}

void GameJohnnyRock::init() {
	Game::init();

	_videoPosX = 11;
	_videoPosY = 2;

	setupCursorTimer();

	if(_vm->useSingleSpeedVideos()) {
		loadLibArchive("johnroc.lib");
	} else {
		loadLibArchive("johnrocd.lib");
	}

	_sceneInfo->loadScnFile("johnroc.scn");
	_startScene = _sceneInfo->getStartScene();

	registerScriptFunctions();
	verifyScriptFunctions();

	_menuzone = new Zone("MainMenu", "GLOBALHIT");
	_menuzone->addRect(0x0C, 0xBB, 0x3C, 0xC7, nullptr, 0, "SHOTMENU", "0");

	_submenzone = new Zone("SubMenu", "GLOBALHIT");
	_submenzone->addRect(0x10, 0x0F, 0x78, 0x34, nullptr, 0, "STARTMENU", "0");
	_submenzone->addRect(0x10, 0x8E, 0x8A, 0xAF, nullptr, 0, "CONTMENU", "0");
	_submenzone->addRect(0x10, 0x3A, 0x6A, 0x5C, nullptr, 0, "RECTSAVE", "0");
	_submenzone->addRect(0x10, 0x64, 0x84, 0x99, nullptr, 0, "RECTLOAD", "0");
	_submenzone->addRect(0xD2, 0x8D, 0x12F, 0xB0, nullptr, 0, "EXITMENU", "0");
	_submenzone->addRect(0xD0, 0x35, 0x123, 0x51, nullptr, 0, "RECTEASY", "0");
	_submenzone->addRect(0xD2, 0x50, 0x125, 0x6B, nullptr, 0, "RECTAVG", "0");
	_submenzone->addRect(0xD2, 0x6D, 0x122, 0x86, nullptr, 0, "RECTHARD", "0");

	_shotSound = loadSoundFile("blow.8b");
	_emptySound = loadSoundFile("empty.8b");
	_saveSound = loadSoundFile("saved.8b");
	_loadSound = loadSoundFile("loaded.8b");
	_skullSound = loadSoundFile("money.8b");

	_gun = AlgGraphics::loadScreenCoordAniImage("gun.ani", _palette);
	_numbers = AlgGraphics::loadAniImage("numbers.ani", _palette);
	_difficultyIcon = AlgGraphics::loadAniImage("diff.ani", _palette);
	auto level = AlgGraphics::loadScreenCoordAniImage("level.ani", _palette);
	_levelIcon = (*level)[0];
	auto hole = AlgGraphics::loadScreenCoordAniImage("hole.ani", _palette);
	_bulletholeIcon = (*hole)[0];

	_background = AlgGraphics::loadVgaBackground("backgrnd.vga", _palette);
	_screen->copyRectToSurface(_background->getPixels(), _background->pitch, 0, 0, _background->w, _background->h);

	moveMouse();
}

void GameJohnnyRock::registerScriptFunctions() {
#define RECT_HIT_FUNCTION(name, func) _rectHitFuncs[name] = new JRScriptFunctionRect(this, &GameJohnnyRock::func);
	RECT_HIT_FUNCTION("DEFAULT", rectNewScene);
	RECT_HIT_FUNCTION("NEWSCENE", rectNewScene);
	RECT_HIT_FUNCTION("EXITMENU", rectExit);
	RECT_HIT_FUNCTION("CONTMENU", rectContinue);
	RECT_HIT_FUNCTION("STARTMENU", rectStart);
	RECT_HIT_FUNCTION("SHOTMENU", rectShotMenu);
	RECT_HIT_FUNCTION("RECTSAVE", rectSave);
	RECT_HIT_FUNCTION("RECTLOAD", rectLoad);
	RECT_HIT_FUNCTION("RECTEASY", rectEasy);
	RECT_HIT_FUNCTION("RECTAVG", rectAverage);
	RECT_HIT_FUNCTION("RECTHARD", rectHard);
	RECT_HIT_FUNCTION("KILLINNOCENT", rectKillInnocent);
	RECT_HIT_FUNCTION("SELCASINO", rectSelectCasino);
	RECT_HIT_FUNCTION("SELPOOLH", rectSelectPoolhall);
	RECT_HIT_FUNCTION("SELWAREHSE", rectSelectWarehouse);
	RECT_HIT_FUNCTION("SELGARAGE", rectSelectGarage);
	RECT_HIT_FUNCTION("SELMANSION", rectSelectMansion);
	RECT_HIT_FUNCTION("SELAMMO", rectSelectAmmo);
	RECT_HIT_FUNCTION("SELOFFICE", rectSelectOffice);
	RECT_HIT_FUNCTION("MANBUST", rectShotManBust);
	RECT_HIT_FUNCTION("WOMANBUST", rectShotWomanBust);
	RECT_HIT_FUNCTION("BLUEVASE", rectShotBlueVase);
	RECT_HIT_FUNCTION("CAT", rectShotCat);
	RECT_HIT_FUNCTION("INDIAN", rectShotIndian);
	RECT_HIT_FUNCTION("PLATE", rectShotPlate);
	RECT_HIT_FUNCTION("BLUEDRESSPIC", rectShotBlueDressPic);
	RECT_HIT_FUNCTION("MODERNPIC", rectShotModernPic);
	RECT_HIT_FUNCTION("MONALISA", rectShotMonaLisa);
	RECT_HIT_FUNCTION("GWASHINGTON", rectShotGWashington);
	RECT_HIT_FUNCTION("BOYINREDPIC", rectShotBoyInRedPic);
	RECT_HIT_FUNCTION("COATOFARMS", rectShotCoatOfArms);
	RECT_HIT_FUNCTION("COMBNOA0", rectShotCombinationA0);
	RECT_HIT_FUNCTION("COMBNOA1", rectShotCombinationA1);
	RECT_HIT_FUNCTION("COMBNOA2", rectShotCombinationA2);
	RECT_HIT_FUNCTION("COMBNOA3", rectShotCombinationA3);
	RECT_HIT_FUNCTION("COMBNOA4", rectShotCombinationA4);
	RECT_HIT_FUNCTION("COMBNOA5", rectShotCombinationA5);
	RECT_HIT_FUNCTION("COMBNOB0", rectShotCombinationB0);
	RECT_HIT_FUNCTION("COMBNOB1", rectShotCombinationB1);
	RECT_HIT_FUNCTION("COMBNOB2", rectShotCombinationB2);
	RECT_HIT_FUNCTION("COMBNOB3", rectShotCombinationB3);
	RECT_HIT_FUNCTION("COMBNOB4", rectShotCombinationB4);
	RECT_HIT_FUNCTION("COMBNOB5", rectShotCombinationB5);
	RECT_HIT_FUNCTION("LUCKNO0", rectShotLuckyNumber0);
	RECT_HIT_FUNCTION("LUCKNO1", rectShotLuckyNumber1);
	RECT_HIT_FUNCTION("LUCKNO2", rectShotLuckyNumber2);
	RECT_HIT_FUNCTION("LUCKNO3", rectShotLuckyNumber3);
	RECT_HIT_FUNCTION("LUCKNO4", rectShotLuckyNumber4);
	RECT_HIT_FUNCTION("LUCKNO5", rectShotLuckyNumber5);
#undef RECT_HIT_FUNCTION

#define PRE_OPS_FUNCTION(name, func) _scenePreOps[name] = new JRScriptFunctionScene(this, &GameJohnnyRock::func);
	PRE_OPS_FUNCTION("DRAWRCT", scenePsoDrawRct);
	PRE_OPS_FUNCTION("PAUSE", scenePsoPause);
	PRE_OPS_FUNCTION("FADEIN", scenePsoFadeIn);
	PRE_OPS_FUNCTION("PAUSFI", scenePsoPauseFadeIn);
	PRE_OPS_FUNCTION("PREREAD", scenePsoPreRead);
	PRE_OPS_FUNCTION("PAUSPR", scenePsoPausePreRead);
	PRE_OPS_FUNCTION("DEFAULT", scenePsoDrawRct);
	PRE_OPS_FUNCTION("DRAWRCTFDI", scenePsoDrawRctFadeIn);
#undef PRE_OPS_FUNCTION

#define INS_OPS_FUNCTION(name, func) _sceneInsOps[name] = new JRScriptFunctionScene(this, &GameJohnnyRock::func);
	INS_OPS_FUNCTION("DEFAULT", sceneIsoDoNothing);
	INS_OPS_FUNCTION("PAUSE", sceneIsoPause);
	INS_OPS_FUNCTION("SPAUSE", sceneIsoShootpastPause);
	INS_OPS_FUNCTION("STARTGAME", sceneIsoStartGame);
	INS_OPS_FUNCTION("SHOOTPAST", sceneIsoShootpast);
	INS_OPS_FUNCTION("GOTTOCASINO", sceneIsoGotoCasino);
	INS_OPS_FUNCTION("GOTTOPOOLH", sceneIsoGotoPoolhall);
	INS_OPS_FUNCTION("GOTTOWAREHSE", sceneIsoGotoWarehouse);
	INS_OPS_FUNCTION("INWAREHSE2", sceneIsoInWarehouse2);
	INS_OPS_FUNCTION("INWAREHSE3", sceneIsoInwarehouse3);
	INS_OPS_FUNCTION("GOTOGARAGE", sceneIsoGotoGarage);
	INS_OPS_FUNCTION("GOTOMANSION", sceneIsoGotoMansion);
	INS_OPS_FUNCTION("INMANSION1", sceneIsoInMansion1);
#undef INS_OPS_FUNCTION

#define NXT_SCN_FUNCTION(name, func) _sceneNxtScn[name] = new JRScriptFunctionScene(this, &GameJohnnyRock::func);
	NXT_SCN_FUNCTION("DEFAULT", sceneDefaultNxtscn);
	NXT_SCN_FUNCTION("DRAWGUN", sceneDefaultNxtscn);
	NXT_SCN_FUNCTION("DIED", sceneNxtscnDied);
	NXT_SCN_FUNCTION("BOMBDEAD", sceneNxtscnBombDead);
	NXT_SCN_FUNCTION("PIKUNDRTAKR", sceneNxtscnPickUndertaker);
	NXT_SCN_FUNCTION("CALLATTRACT", sceneNxtscnCallAttract);
	NXT_SCN_FUNCTION("PICKLUCKNO", sceneNxtscnPickLuckyNumber);
	NXT_SCN_FUNCTION("PICKMAP", sceneNxtscnPickMap);
	NXT_SCN_FUNCTION("PICKCLUE", sceneNxtscnPickClue);
	NXT_SCN_FUNCTION("MAPTIMEOUT", sceneNxtscnMapTimeout);
	NXT_SCN_FUNCTION("ENTCASINO", sceneNxtscnEnterCasino);
	NXT_SCN_FUNCTION("CASINOWHAT?", sceneNxtscnCasinoWhat);
	NXT_SCN_FUNCTION("ENTPOOLH", sceneNxtscnEnterPoolhall);
	NXT_SCN_FUNCTION("POOLHCLUE", sceneNxtscnPoolhallClue);
	NXT_SCN_FUNCTION("ENTWAREHSE", sceneNxtscnEnterWarehouse);
	NXT_SCN_FUNCTION("WAREHSECLUE", sceneNxtscnWarehouseClue);
	NXT_SCN_FUNCTION("ENTGARAGE", sceneNxtscnEnterGarage);
	NXT_SCN_FUNCTION("GARAGECLUE", sceneNxtscnGarageClue);
	NXT_SCN_FUNCTION("ENTMANSION", sceneNxtscnEnterMansion);
	NXT_SCN_FUNCTION("GIVECLUE", sceneNxtscnGiveClue);
	NXT_SCN_FUNCTION("PICKFLOWERMAN", sceneNxtscnPickFlowerMan);
	NXT_SCN_FUNCTION("RANDOMSCENE", sceneNxtscnRandomScene);
	NXT_SCN_FUNCTION("ENDRANDSCENE", sceneNxtscnEndRandScene);
	NXT_SCN_FUNCTION("SCN_KILLINNOCENT", sceneNxtscnKillInnocent);
#undef NXT_SCN_FUNCTION

	_zonePtrFb["DEFAULT"] = new JRScriptFunctionPoint(this, &GameJohnnyRock::zoneBullethole);
	_sceneShowMsg["DEFAULT"] = new JRScriptFunctionScene(this, &GameJohnnyRock::sceneSmDonothing);
	_sceneWepDwn["DEFAULT"] = new JRScriptFunctionScene(this, &GameJohnnyRock::sceneDefaultWepdwn);
	_sceneScnScr["DEFAULT"] = new JRScriptFunctionScene(this, &GameJohnnyRock::sceneDefaultScore);
	_sceneNxtFrm["DEFAULT"] = new JRScriptFunctionScene(this, &GameJohnnyRock::sceneNxtfrm);
}

void GameJohnnyRock::verifyScriptFunctions() {
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

JRScriptFunctionPoint GameJohnnyRock::getScriptFunctionZonePtrFb(Common::String name) {
	auto it = _zonePtrFb.find(name);
	if (it != _zonePtrFb.end()) {
		return *it->_value;
	} else {
		error("GameJohnnyRock::getScriptFunctionZonePtrFb(): Could not find zonePtrFb function: %s", name.c_str());
	}
}

JRScriptFunctionRect GameJohnnyRock::getScriptFunctionRectHit(Common::String name) {
	auto it = _rectHitFuncs.find(name);
	if (it != _rectHitFuncs.end()) {
		return *it->_value;
	} else {
		error("GameJohnnyRock::getScriptFunctionRectHit(): Could not find rectHit function: %s", name.c_str());
	}
}

JRScriptFunctionScene GameJohnnyRock::getScriptFunctionScene(SceneFuncType type, Common::String name) {
	JRScriptFunctionSceneMap *functionMap;
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
		error("GameJohnnyRock::getScriptFunctionScene(): Unkown scene script type: %u", type);
		break;
	}
	JRScriptFunctionSceneMap::iterator it;
	it = functionMap->find(name);
	if (it != functionMap->end()) {
		return *it->_value;
	} else {
		error("GameJohnnyRock::getScriptFunctionScene(): Could not find scene type %u function: %s", type, name.c_str());
	}
}

void GameJohnnyRock::callScriptFunctionZonePtrFb(Common::String name, Common::Point *point) {
	JRScriptFunctionPoint function = getScriptFunctionZonePtrFb(name);
	function(point);
}

void GameJohnnyRock::callScriptFunctionRectHit(Common::String name, Rect *rect) {
	JRScriptFunctionRect function = getScriptFunctionRectHit(name);
	function(rect);
}

void GameJohnnyRock::callScriptFunctionScene(SceneFuncType type, Common::String name, Scene *scene) {
	JRScriptFunctionScene function = getScriptFunctionScene(type, name);
	function(scene);
}

Common::Error GameJohnnyRock::run() {
	init();
	newGame();
	_curScene = _startScene;
	Common::String oldscene;
	while (!_vm->shouldQuit()) {
		_leftDown = false;
		oldscene = _curScene;
		_fired = false;
		Scene *scene = _sceneInfo->findScene(_curScene);
		if (!loadScene(scene)) {
			error("GameJohnnyRock::run(): Cannot find scene %s in libfile", scene->_name.c_str());
		}
		_paletteDirty = true;
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
						Rect *hitRect = nullptr;
						Zone *hitSceneZone = checkZonesV1(scene, hitRect, &firedCoords);
						if (hitSceneZone != nullptr) {
							callScriptFunctionZonePtrFb(hitSceneZone->_ptrfb, &firedCoords);
							callScriptFunctionRectHit(hitRect->_rectHit, hitRect);
						} else {
							defaultBullethole(&firedCoords);
						}
					} else {
						playSound(_emptySound);
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

bool GameJohnnyRock::fired(Common::Point *point) {
	pollEvents();
	_fired = false;
	if (!_leftDown) {
		_buttonDown = false;
		return false;
	}
	if (_leftDown && !_inMenu) {
		_leftDown = true;
	}
	if (_buttonDown) {
		if (_thisGameTimer - _machGunTimer > 3) {
			_buttonDown = false;
			_mgunCnt++;
			if (_mgunCnt > 5) {
				_mgunCnt = 0;
				_leftDown = false;
			}
		}
		return false;
	}
	_fired = true;
	point->x = _mousePos.x;
	point->y = _mousePos.y;
	_machGunTimer = _thisGameTimer;
	_buttonDown = true;
	return true;
}

void GameJohnnyRock::newGame() {
	_gameMoney = 2000;
	_shots = 400;
	_score = 0;
	_holster = false;
	updateStat();
	_retScene = "";
	_subScene = "";
}

void GameJohnnyRock::resetParams() {
	_subScene = "";
	_moneyScene = "";
	_thisMap = _rnd->getRandomNumber(2);
	_clues = 0;
	_gotThisNumber = 0;
	_gotThisClue = 0;
	_thisClue = 0;
	_office = 0;
	_casino = 0;
	_poolHall = 0;
	_warehouse = 0;
	_garage = 0;
	_mansion = 0;
	_didContinue = 0;
	_thisDifficulty = _difficulty - 1;
	_casinoType = _rnd->getRandomNumber(1);
	_poolHallType = _rnd->getRandomNumber(2);
	_warehouseType = _rnd->getRandomNumber(2);
	_garageType = _rnd->getRandomNumber(1);
	_mapTimeout = 0;
	_ammoAgain = 0;
	_combinations[0] = _rnd->getRandomNumber(5);
	_combinations[1] = _rnd->getRandomNumber(5);
	_combinations[2] = _rnd->getRandomNumber(5);
	_combinations[3] = _rnd->getRandomNumber(5);
	_whoDidIt = _rnd->getRandomNumber(3);
	_inWarehouse = 0;
	_officeCount = 0;
	_hadGoToMansion = 0;
	_randomPlaceBits = 0;
	for (int i = 0; i < 5; i++) {
		// this assigns places from _randomPlaces
		uint16 picked = pickBits(&_randomPlaceBits, 6);
		_entranceIndex[i] = picked;
	}
	_randomPlaceBits = 0;
	for (uint8 i = 5; i < 19; i++) {
		// this assigns places from _randomPlacesMR
		uint16 picked = pickBits(&_randomPlaceBits, 8);
		_entranceIndex[i] = picked;
	}
	_maxRepeat = _thisDifficulty + 4;
	_repeatRandomPlace = 0;
	_gotTo = 0;
	updateStat();
}

void GameJohnnyRock::outShots() {
	_shots = 400;
	_score = 0;
	_holster = false;
	updateStat();
}

void GameJohnnyRock::doMenu() {
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
		_leftDown = false;
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

void GameJohnnyRock::updateStat() {
	if (_score != _oldScore) {
		_oldScore = _score;
		Common::String buffer = Common::String::format("%05d", _score);
		for (int i = 0; i < 5; i++) {
			uint8 digit = buffer[i] - '0';
			AlgGraphics::drawImage(_screen, (*_numbers)[digit], (i * 7) + 0x7D, 0xBE);
		}
	}
	if (_gameMoney != _oldGameMoney) {
		_oldGameMoney = _gameMoney;
		Common::String buffer = Common::String::format("%04d", _gameMoney < 0 ? 0 : _gameMoney);
		for (int i = 0; i < 4; i++) {
			uint8 digit = buffer[i] - '0';
			AlgGraphics::drawImage(_screen, (*_numbers)[digit], (i * 7) + 0x43, 0xBE);
		}
	}
	if (_shots != _oldShots) {
		_oldShots = _shots;
		Common::String buffer = Common::String::format("%04d", _shots);
		for (int i = 0; i < 4; i++) {
			uint8 digit = buffer[i] - '0';
			AlgGraphics::drawImage(_screen, (*_numbers)[digit], (i * 7) + 0x10A, 0xBE);
		}
	}
	AlgGraphics::drawImage(_screen, (*_difficultyIcon)[_difficulty - 1], 0xBA, 0xBE);
}

void GameJohnnyRock::displayScore() {
	updateStat();
}

void GameJohnnyRock::showDifficulty(uint8 newDifficulty, bool cursor) {
	// reset menu screen
	_screen->copyRectToSurface(_background->getBasePtr(_videoPosX, _videoPosY), _background->pitch, _videoPosX, _videoPosY, _videoDecoder->getWidth(), _videoDecoder->getHeight());
	AlgGraphics::drawImageCentered(_screen, _levelIcon, _diffPos[newDifficulty][0], _diffPos[newDifficulty][1]);
	if (cursor) {
		updateCursor();
	}
}

void GameJohnnyRock::changeDifficulty(uint8 newDifficulty) {
	if (newDifficulty == _oldDifficulty) {
		return;
	}
	showDifficulty(newDifficulty, true);
	Game::adjustDifficulty(newDifficulty, _oldDifficulty);
	_oldDifficulty = newDifficulty;
	_difficulty = newDifficulty;
}

void GameJohnnyRock::updateCursor() {
	_oldWhichGun = _whichGun;
}

void GameJohnnyRock::updateMouse() {
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

void GameJohnnyRock::moveMouse() {
	if (_inMenu) {
		if (_mousePos.y > 0xB7) {
			_mousePos.y = 0xB7;
		}
		_whichGun = 8;
	} else {
		if (_mousePos.y > 0xBC) {
			_whichGun = 6;
		} else if (_whichGun > 5) {
			_whichGun = 0;
		}
	}
	updateMouse();
}

bool GameJohnnyRock::weaponDown() {
	if (_rightDown && _mousePos.y >= 0xBC && _mousePos.x >= 0x37) {
		return true;
	}
	return false;
}

bool GameJohnnyRock::saveState() {
	Common::OutSaveFile *outSaveFile;
	Common::String saveFileName = _vm->getSaveStateName(0);
	if (!(outSaveFile = g_system->getSavefileManager()->openForSaving(saveFileName))) {
		warning("GameJohnnyRock::saveState(): Can't create file '%s', game not saved", saveFileName.c_str());
		return false;
	}
	outSaveFile->writeUint32BE(MKTAG('A', 'L', 'G', 'S')); // header
	outSaveFile->writeByte(0);                             // version, unused for now
	outSaveFile->writeUint16LE(_totalDies);
	outSaveFile->writeSint16LE(_gameMoney);
	outSaveFile->writeUint16LE(_ammoAgain);
	outSaveFile->writeUint16LE(_mapTimeout);
	outSaveFile->writeByte(_luckyNumber);
	outSaveFile->writeByte(_thisMap);
	outSaveFile->writeUint16LE(_clues);
	outSaveFile->writeUint16LE(_placeBits);
	outSaveFile->writeByte(_randomCount);
	outSaveFile->writeUint16LE(_doctorBits);
	outSaveFile->writeUint16LE(_undertakerBits);
	for (int i = 0; i < 4; i++) {
		outSaveFile->writeByte(_clueTable[i]);
	}
	outSaveFile->writeUint16LE(_thisClue);
	outSaveFile->writeByte(_gotThisNumber);
	outSaveFile->writeByte(_casino);
	outSaveFile->writeByte(_poolHall);
	outSaveFile->writeByte(_warehouse);
	outSaveFile->writeByte(_garage);
	outSaveFile->writeByte(_office);
	outSaveFile->writeByte(_casinoType);
	outSaveFile->writeByte(_poolHallType);
	outSaveFile->writeByte(_warehouseType);
	outSaveFile->writeByte(_garageType);
	outSaveFile->writeByte(_mansion);
	outSaveFile->writeByte(_inWarehouse);
	outSaveFile->writeByte(_inOffice);
	outSaveFile->writeUint16LE(_gotTo);
	for (int i = 0; i < 20; i++) {
		outSaveFile->writeUint16LE(_entranceIndex[i]);
	}
	for (int i = 0; i < 10; i++) {
		outSaveFile->writeUint16LE(_randomScenesIndex[i]);
	}
	for (int i = 0; i < 4; i++) {
		outSaveFile->writeByte(_combinations[i]);
	}
	outSaveFile->writeByte(_whoDidIt);
	outSaveFile->writeByte(_hadGoToMansion);
	outSaveFile->writeUint16LE(_officeCount);
	outSaveFile->writeUint16LE(_randomPlaceBits);
	outSaveFile->writeByte(_maxRandomCount);
	outSaveFile->writeUint16LE(_gotoAfterRandom);
	outSaveFile->writeUint16LE(_repeatRandomPlace);
	outSaveFile->writeUint16LE(_maxRepeat);
	outSaveFile->writeUint16LE(_gotThisClue);
	outSaveFile->writeUint16LE(_didContinue);
	outSaveFile->writeUint16LE(_thisGameTime);
	outSaveFile->writeUint16LE(_shots);
	outSaveFile->writeSint32LE(_score);
	outSaveFile->writeByte(_holster);
	outSaveFile->writeByte(_difficulty);
	outSaveFile->writeByte(_thisDifficulty);
	outSaveFile->writeString(_moneyScene);
	outSaveFile->writeByte(0);
	outSaveFile->writeString(_curScene);
	outSaveFile->writeByte(0);
	outSaveFile->writeString(_subScene);
	outSaveFile->writeByte(0);
	outSaveFile->writeString(_retScene);
	outSaveFile->writeByte(0);
	outSaveFile->writeByte(_randomScenesSavestateIndex);
	outSaveFile->finalize();
	delete outSaveFile;
	return true;
}

bool GameJohnnyRock::loadState() {
	Common::InSaveFile *inSaveFile;
	Common::String saveFileName = _vm->getSaveStateName(0);
	if (!(inSaveFile = g_system->getSavefileManager()->openForLoading(saveFileName))) {
		debug("GameJohnnyRock::loadState(): Can't load file '%s', game not loaded", saveFileName.c_str());
		return false;
	}
	uint32 header = inSaveFile->readUint32BE();
	if (header != MKTAG('A', 'L', 'G', 'S')) {
		warning("GameJohnnyRock::loadState(): Unkown save file, header: %s", tag2str(header));
		return false;
	}
	inSaveFile->skip(1); // version, unused for now
	_totalDies = inSaveFile->readUint16LE();
	_gameMoney = inSaveFile->readSint16LE();
	_ammoAgain = inSaveFile->readUint16LE();
	_mapTimeout = inSaveFile->readUint16LE();
	_luckyNumber = inSaveFile->readByte();
	_thisMap = inSaveFile->readByte();
	_clues = inSaveFile->readUint16LE();
	_placeBits = inSaveFile->readUint16LE();
	_randomCount = inSaveFile->readByte();
	_doctorBits = inSaveFile->readUint16LE();
	_undertakerBits = inSaveFile->readUint16LE();
	for (int i = 0; i < 4; i++) {
		_clueTable[i] = inSaveFile->readByte();
	}
	_thisClue = inSaveFile->readUint16LE();
	_gotThisNumber = inSaveFile->readByte();
	_casino = inSaveFile->readByte();
	_poolHall = inSaveFile->readByte();
	_warehouse = inSaveFile->readByte();
	_garage = inSaveFile->readByte();
	_office = inSaveFile->readByte();
	_casinoType = inSaveFile->readByte();
	_poolHallType = inSaveFile->readByte();
	_warehouseType = inSaveFile->readByte();
	_garageType = inSaveFile->readByte();
	_mansion = inSaveFile->readByte();
	_inWarehouse = inSaveFile->readByte();
	_inOffice = inSaveFile->readByte();
	_gotTo = inSaveFile->readUint16LE();
	for (int i = 0; i < 20; i++) {
		_entranceIndex[i] = inSaveFile->readUint16LE();
	}
	for (int i = 0; i < 10; i++) {
		_randomScenesIndex[i] = inSaveFile->readUint16LE();
	}
	for (int i = 0; i < 4; i++) {
		_combinations[i] = inSaveFile->readByte();
	}
	_whoDidIt = inSaveFile->readByte();
	_hadGoToMansion = inSaveFile->readByte();
	_officeCount = inSaveFile->readUint16LE();
	_randomPlaceBits = inSaveFile->readUint16LE();
	_maxRandomCount = inSaveFile->readByte();
	_gotoAfterRandom = inSaveFile->readUint16LE();
	_repeatRandomPlace = inSaveFile->readUint16LE();
	_maxRepeat = inSaveFile->readUint16LE();
	_gotThisClue = inSaveFile->readUint16LE();
	_didContinue = inSaveFile->readUint16LE();
	_thisGameTime = inSaveFile->readUint16LE();
	_shots = inSaveFile->readUint16LE();
	_score = inSaveFile->readSint32LE();
	_holster = inSaveFile->readByte();
	_difficulty = inSaveFile->readByte();
	_thisDifficulty = inSaveFile->readByte();
	_moneyScene = inSaveFile->readString();
	_curScene = inSaveFile->readString();
	_subScene = inSaveFile->readString();
	_retScene = inSaveFile->readString();
	_randomScenesSavestateIndex = inSaveFile->readByte();
	delete inSaveFile;
	// find out where _random_scenes should point
	uint16 placeIndex = _entranceIndex[_randomScenesSavestateIndex];
	if (_randomScenesSavestateIndex < 5) {
		_randomScenes = _randomPlaces[placeIndex];
	} else {
		_randomScenes = _randomPlacesMR[placeIndex];
	}
	changeDifficulty(_difficulty);
	debug("lucky number: %d", (_luckyNumber + 1));
	return true;
}

void GameJohnnyRock::doMoneySound() {
	playSound(_skullSound);
}

// Misc game functions
Common::String GameJohnnyRock::numToScene(int n) {
	switch (n) {
	case 1:
		return "scene1a";
	case 10:
		return "scene10a";
	case 12:
		return "scene12a";
	case 14:
		return "scene14a";
	case 18:
		return "scene18a";
	case 63:
		return "scene63a";
	case 71:
		return "scene71a";
	case 120:
		return "scn120a";
	case 137:
		return "scn137a";
	default:
		return Common::String::format("scene%d", n);
	}
}

uint16 GameJohnnyRock::sceneToNum(Common::String sceneName) {
	Common::String temp;
	uint16 index = 4;
	if (sceneName[index] == 'e') {
		index++;
	}
	while (sceneName[index] != 'a' && sceneName[index] != 'b' && sceneName[index] != 'c' && sceneName[index] != '\0') {
		temp += sceneName[index];
		index++;
		if (index >= sceneName.size()) {
			break;
		}
	}
	return atoi(temp.c_str());
}

void GameJohnnyRock::defaultBullethole(Common::Point *point) {
	if (point->x >= 14 && point->x <= 306 && point->y >= 5 && point->y <= 169) {
		uint16 targetX = point->x - _videoPosX;
		uint16 targetY = point->y - _videoPosY;
		AlgGraphics::drawImageCentered(_videoDecoder->getVideoFrame(), _bulletholeIcon, targetX, targetY);
		updateCursor();
		_shotFired = true;
		doShot();
	}
}

uint16 GameJohnnyRock::pickBits(uint16 *bits, uint8 max) {
	// reset bits if full
	if (*bits == (0xFFFF >> (16 - max))) {
		*bits = 0;
	}
	uint8 randomNum = _rnd->getRandomNumber(max - 1);
	// find first unused bit position
	while (*bits & (1 << randomNum)) {
		randomNum++;
		if (randomNum >= max) {
			randomNum = 0;
		}
	}
	*bits |= (1 << randomNum);
	return randomNum;
}

uint16 GameJohnnyRock::pickRandomPlace(uint8 place) {
	_randomScenesSavestateIndex = place;
	uint16 placeIndex = _entranceIndex[place];
	if (place < 5) {
		_randomScenes = _randomPlaces[placeIndex];
	} else {
		_randomScenes = _randomPlacesMR[placeIndex];
	}
	_placeBits = 0;
	_randomCount = 0;
	memset(&_randomScenesIndex, 0, (10 * sizeof(uint16)));
	for (int i = 0; i < (_randomScenes[0] + 4); i++) {
		_randomScenesIndex[i] = _randomScenes[i];
	}
	if (_randomScenes[1] < 0) {
		_maxRandomCount = (_thisDifficulty * 2) - _randomScenes[1];
	} else {
		_maxRandomCount = _randomScenes[1];
	}
	uint16 index = pickBits(&_placeBits, _randomScenes[0]);
	return _randomScenes[index + 4];
}

void GameJohnnyRock::showCombination() {
	uint16 offset = (_gotThisClue * 6) + _combinations[_gotThisClue];
	_gotThisClue++;
	if (_gotThisClue == 4) {
		_mansion = 3;
	}
	_curScene = numToScene(offset + 0xDB);
}

// Script functions: Zone
void GameJohnnyRock::zoneBullethole(Common::Point *point) {
	defaultBullethole(point);
}

void GameJohnnyRock::rectShotMenu(Rect *rect) {
	doMenu();
}

void GameJohnnyRock::rectSave(Rect *rect) {
	if (saveState()) {
		doSaveSound();
	}
}

void GameJohnnyRock::rectLoad(Rect *rect) {
	if (loadState()) {
		doLoadSound();
	}
}

void GameJohnnyRock::rectContinue(Rect *rect) {
	_inMenu = false;
	_fired = false;
	if (_gameMoney < 0) {
		newGame();
		_retScene = "";
		_subScene = "";
		if (_inOffice) {
			_curScene = numToScene(_inOffice);
		} else {
			_curScene = numToScene(_thisMap + 174);
		}
		_didContinue++;
	}
	if (_shots <= 0) {
		outShots();
		_didContinue++;
	}
}

void GameJohnnyRock::rectStart(Rect *rect) {
	_inMenu = false;
	_fired = false;
	_thisDifficulty = 0;
	Scene *scene = _sceneInfo->findScene(_startScene);
	if (scene->_nxtscn == "DRAWGUN") {
		callScriptFunctionScene(NXTSCN, "DRAWGUN", scene);
	}
	_curScene = _startScene;
	resetParams();
	newGame();
	updateStat();
}

void GameJohnnyRock::rectKillInnocent(Rect *rect) {
	_inOffice = sceneToNum(_curScene);
	if (_inOffice >= 0x13) {
		_inOffice = 0;
	}
	if (!_debug_godMode) {
		_gameMoney -= 400;
	}
	if (_gameMoney < 0) {
		_subScene = "scene358";
		_retScene = "";
		_curScene = "scene151";
	} else {
		switch (_rnd->getRandomNumber(2)) {
		case 0:
			_curScene = "scene151";
			break;
		case 1:
			_curScene = "scene152";
			break;
		case 2:
			_curScene = "scene153";
			break;
		}
	}
}

void GameJohnnyRock::rectSelectCasino(Rect *rect) {
	_retScene = "";
	if (_mansion == 5 && _whoDidIt == 1) {
		_repeatRandomPlace = _maxRepeat;
		_gotoAfterRandom = 0xF5;
		_curScene = numToScene(pickRandomPlace(_maxRepeat + 5));
	} else if (_casino == 0) {
		if (_gotTo & 1) {
			_curScene = "scene19";
		} else {
			_gotoAfterRandom = 0x13;
			_curScene = numToScene(pickRandomPlace(0));
		}
	} else if (_casino == 1 || _casino == 3) {
		_curScene = "scene44";
	} else {
		_casino = 3;
		if (_rnd->getRandomBit()) { // original: (_this_game_time & 1) == 0
			_curScene = "scene64";
		} else {
			_curScene = "scene65";
		}
	}
}

void GameJohnnyRock::rectSelectPoolhall(Rect *rect) {
	_retScene = "";
	if (_mansion == 5 && _whoDidIt == 0) {
		_repeatRandomPlace = _maxRepeat;
		_gotoAfterRandom = 0xF7;
		_curScene = numToScene(pickRandomPlace(_maxRepeat + 5));
	} else if (_poolHall == 0) {
		if (_gotTo & 2) {
			_curScene = "scene66";
		} else {
			_gotoAfterRandom = 0x42;
			_curScene = numToScene(pickRandomPlace(1));
		}
	} else if (_poolHall == 1 || _poolHall == 3) {
		_curScene = "scene82";
	} else {
		_poolHall = 3;
		_curScene = "scene89";
	}
}

void GameJohnnyRock::rectSelectWarehouse(Rect *rect) {
	_retScene = "";
	if (_mansion == 5 && _whoDidIt == 2) {
		_repeatRandomPlace = _maxRepeat;
		_gotoAfterRandom = 0xF9;
		_curScene = numToScene(pickRandomPlace(_maxRepeat + 5));
	} else if (_warehouse == 0) {
		if (_inWarehouse < 2) {
			if (_gotTo & 4) {
				_curScene = "scene90";
			} else {
				_inWarehouse = 1;
				_gotoAfterRandom = 0x5A;
				_curScene = numToScene(pickRandomPlace(2));
			}
		} else if (_inWarehouse == 2) {
			_curScene = "scene93";
		} else if (_inWarehouse == 3) {
			_curScene = "scene119";
		}
	} else if (_warehouse == 1 || _warehouse == 3) {
		_curScene = "scene122";
	} else {
		_warehouse = 3;
		_curScene = "scene121";
	}
}

void GameJohnnyRock::rectSelectGarage(Rect *rect) {
	_retScene = "";
	if (_mansion == 5 && _whoDidIt == 3) {
		_repeatRandomPlace = _maxRepeat;
		_gotoAfterRandom = 0xFB;
		_curScene = numToScene(pickRandomPlace(_maxRepeat + 5));
	} else if (_garage == 0) {
		if (_gotTo & 8) {
			_curScene = "scene123";
		} else {
			_gotoAfterRandom = 0x7B;
			_curScene = numToScene(pickRandomPlace(3));
		}
	} else if (_garage == 1 || _garage == 3) {
		_curScene = "scene138";
	} else {
		_garage = 3;
		_curScene = "scene139";
	}
}

void GameJohnnyRock::rectSelectMansion(Rect *rect) {
	_placeBits = 0;
	_randomCount = 1;
	_retScene = "";
	if (_mansion == 1) {
		uint16 picked = pickBits(&_placeBits, 5);
		_curScene = numToScene(0xB8 + (picked * 2));
	} else if (_mansion == 2) {
		_curScene = "scene194";
	} else if (_mansion == 3) {
		_curScene = "scene207";
	} else if (_mansion == 4) {
		_gotThisNumber = 0;
		_curScene = "scene212";
	} else if (_mansion == 5) {
		_curScene = "scene243";
	} else {
		if (_garage == 0 || _casino == 0 || _poolHall == 0 || _warehouse == 0) {
			_curScene = "scene243";
		} else if (_gotTo & 0x10) {
			_curScene = "scene180";
		} else {
			_gotoAfterRandom = 0xB4;
			_curScene = numToScene(pickRandomPlace(4));
		}
	}
}

void GameJohnnyRock::rectSelectAmmo(Rect *rect) {
	_retScene = "";
	if (_gameMoney >= 100) {
		if (!_debug_godMode) {
			_gameMoney -= 100;
		}
		_shots += 200;
		_ammoAgain = 0;
		doMoneySound();
		_curScene = "scene178";
	} else {
		_ammoAgain++;
		if (_ammoAgain >= 2) {
			_curScene = "scene243";
		} else {
			_curScene = "scene179";
		}
	}
}

void GameJohnnyRock::rectSelectOffice(Rect *rect) {
	_retScene = "";
	if (!_office) {
		_office = 1;
		_curScene = "scene168";
	} else {
		if (_rnd->getRandomBit()) { // original: _this_game_time & 1
			_curScene = "scene243";
		} else {
			_curScene = "scene262";
		}
	}
}

void GameJohnnyRock::shotClue(uint8 clue) {
	if (_clueTable[_gotThisClue] == clue) {
		showCombination();
	} else {
		_gotThisClue = 0;
		_curScene = "scene374";
	}
}

void GameJohnnyRock::rectShotManBust(Rect *rect) {
	shotClue(0);
}

void GameJohnnyRock::rectShotWomanBust(Rect *rect) {
	shotClue(1);
}

void GameJohnnyRock::rectShotBlueVase(Rect *rect) {
	shotClue(2);
}

void GameJohnnyRock::rectShotCat(Rect *rect) {
	shotClue(3);
}

void GameJohnnyRock::rectShotIndian(Rect *rect) {
	shotClue(4);
}

void GameJohnnyRock::rectShotPlate(Rect *rect) {
	shotClue(5);
}

void GameJohnnyRock::rectShotBlueDressPic(Rect *rect) {
	shotClue(6);
}

void GameJohnnyRock::rectShotModernPic(Rect *rect) {
	shotClue(7);
}

void GameJohnnyRock::rectShotMonaLisa(Rect *rect) {
	shotClue(8);
}

void GameJohnnyRock::rectShotGWashington(Rect *rect) {
	shotClue(9);
}

void GameJohnnyRock::rectShotBoyInRedPic(Rect *rect) {
	shotClue(10);
}

void GameJohnnyRock::rectShotCoatOfArms(Rect *rect) {
	shotClue(11);
}

void GameJohnnyRock::shotCombination(uint8 combination, bool combinationB) {
	if (_combinations[_gotThisNumber] == combination) {
		_gotThisNumber++;
		if (_gotThisNumber >= 4) {
			_mansion = 5;
			_curScene = numToScene(_whoDidIt + 0xD7);
		} else {
			if (combinationB) {
				_curScene = "scene213";
			} else {
				_curScene = "scene214";
			}
		}
	} else {
		_gotThisNumber = 0;
		_curScene = "scene376";
	}
}

void GameJohnnyRock::rectShotCombinationA0(Rect *rect) {
	shotCombination(0, false);
}

void GameJohnnyRock::rectShotCombinationA1(Rect *rect) {
	shotCombination(1, false);
}

void GameJohnnyRock::rectShotCombinationA2(Rect *rect) {
	shotCombination(2, false);
}

void GameJohnnyRock::rectShotCombinationA3(Rect *rect) {
	shotCombination(3, false);
}

void GameJohnnyRock::rectShotCombinationA4(Rect *rect) {
	shotCombination(4, false);
}

void GameJohnnyRock::rectShotCombinationA5(Rect *rect) {
	shotCombination(5, false);
}

void GameJohnnyRock::rectShotCombinationB0(Rect *rect) {
	shotCombination(0, true);
}

void GameJohnnyRock::rectShotCombinationB1(Rect *rect) {
	shotCombination(1, true);
}

void GameJohnnyRock::rectShotCombinationB2(Rect *rect) {
	shotCombination(2, true);
}

void GameJohnnyRock::rectShotCombinationB3(Rect *rect) {
	shotCombination(3, true);
}

void GameJohnnyRock::rectShotCombinationB4(Rect *rect) {
	shotCombination(4, true);
}

void GameJohnnyRock::rectShotCombinationB5(Rect *rect) {
	shotCombination(5, true);
}

void GameJohnnyRock::shotLuckyNumber(uint8 number) {
	if (_luckyNumber != number || _curScene == _moneyScene) {
		return;
	}
	doMoneySound();
	_gameMoney += 100;
	_score += 500;
	_moneyScene = _curScene;
}

void GameJohnnyRock::rectShotLuckyNumber0(Rect *rect) {
	shotLuckyNumber(0);
}

void GameJohnnyRock::rectShotLuckyNumber1(Rect *rect) {
	shotLuckyNumber(1);
}

void GameJohnnyRock::rectShotLuckyNumber2(Rect *rect) {
	shotLuckyNumber(2);
}

void GameJohnnyRock::rectShotLuckyNumber3(Rect *rect) {
	shotLuckyNumber(3);
}

void GameJohnnyRock::rectShotLuckyNumber4(Rect *rect) {
	shotLuckyNumber(4);
}

void GameJohnnyRock::rectShotLuckyNumber5(Rect *rect) {
	shotLuckyNumber(5);
}

// Script functions: Scene PreOps

// Script functions: Scene Scene InsOps
void GameJohnnyRock::sceneIsoShootpast(Scene *scene) {
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

void GameJohnnyRock::sceneIsoShootpastPause(Scene *scene) {
	sceneIsoShootpast(scene);
	sceneIsoPause(scene);
}

void GameJohnnyRock::sceneIsoGotoCasino(Scene *scene) {
	_gotTo |= 1;
	sceneIsoShootpast(scene);
}

void GameJohnnyRock::sceneIsoGotoPoolhall(Scene *scene) {
	_gotTo |= 2;
	sceneIsoShootpast(scene);
}

void GameJohnnyRock::sceneIsoGotoWarehouse(Scene *scene) {
	_gotTo |= 4;
}

void GameJohnnyRock::sceneIsoInWarehouse2(Scene *scene) {
	_inWarehouse = 2;
	sceneIsoShootpast(scene);
}

void GameJohnnyRock::sceneIsoInwarehouse3(Scene *scene) {
	_inWarehouse = 3;
	sceneIsoShootpast(scene);
}

void GameJohnnyRock::sceneIsoGotoGarage(Scene *scene) {
	_gotTo |= 8;
	sceneIsoShootpast(scene);
}

void GameJohnnyRock::sceneIsoGotoMansion(Scene *scene) {
	_gotTo |= 0x10;
}

void GameJohnnyRock::sceneIsoInMansion1(Scene *scene) {
	_mansion = 1;
}

// Script functions: Scene NxtScn
void GameJohnnyRock::sceneNxtscnDied(Scene *scene) {
	uint16 sceneNum = sceneToNum(_curScene);
	_inOffice = sceneNum;
	if (sceneNum >= 0x13) {
		_inOffice = 0;
	}
	_totalDies++;
	if (!_debug_godMode) {
		_gameMoney -= 400;
	}
	if (_gameMoney < 0) {
		switch (_rnd->getRandomNumber(2)) {
		case 0:
			_curScene = "scene148";
			break;
		case 1:
			_curScene = "scene149";
			break;
		case 2:
			_curScene = "scene150";
			break;
		}
	} else {
		switch (pickBits(&_doctorBits, 8)) {
		case 0:
			_curScene = "scene140";
			break;
		case 1:
			_curScene = "scene141";
			break;
		case 2:
			_curScene = "scene142";
			break;
		case 3:
			_curScene = "scene143";
			break;
		case 4:
			_curScene = "scene144";
			break;
		case 5:
			_curScene = "scene145";
			break;
		case 6:
			_curScene = "scene146";
			break;
		case 7:
			_curScene = "scene147";
			break;
		}
	}
}

void GameJohnnyRock::sceneNxtscnBombDead(Scene *scene) {
	uint16 sceneNum = sceneToNum(_curScene);
	_inOffice = sceneNum;
	if (sceneNum >= 0x13) {
		_inOffice = 0;
	}
	_totalDies++;
	if (!_debug_godMode) {
		_gameMoney -= 400;
	}
	if (_gameMoney < 0) {
		switch (_rnd->getRandomNumber(2)) {
		case 0:
			_curScene = "scene148";
			break;
		case 1:
			_curScene = "scene149";
			break;
		case 2:
			_curScene = "scene150";
			break;
		}
	} else {
		_curScene = "scene142";
	}
}

void GameJohnnyRock::sceneNxtscnPickUndertaker(Scene *scene) {
	switch (pickBits(&_undertakerBits, 3)) {
	case 0:
		_curScene = "scene154";
		break;
	case 1:
		_curScene = "scene155";
		break;
	case 2:
		_curScene = "scene156";
		break;
	}
}

void GameJohnnyRock::sceneNxtscnCallAttract(Scene *scene) {
	resetParams();
	newGame();
	_curScene = "scn354aa";
}

void GameJohnnyRock::sceneNxtscnPickLuckyNumber(Scene *scene) {
	_luckyNumber = _rnd->getRandomNumber(5);
	debug("lucky number: %d", (_luckyNumber + 1));
	_curScene = Common::String::format("scene%d", _luckyNumber + 3);
}

void GameJohnnyRock::sceneNxtscnPickMap(Scene *scene) {
	Common::String nextScene;
	if (_gameMoney < 0) {
		switch (_rnd->getRandomNumber(2)) {
		case 0:
			_curScene = "scene148";
			break;
		case 1:
			_curScene = "scene149";
			break;
		case 2:
			_curScene = "scene150";
			break;
		}
		return;
	}
	uint16 sceneNum = sceneToNum(_curScene);
	if (sceneNum == 18) {
		_inOffice = 0;
	}
	if (_inOffice) {
		nextScene = numToScene(_inOffice);
	} else if (_officeCount < 10) {
		if ((_officeCount % 3) == 0) {
			uint16 sceneNume = _officeTable[_officeCount / 3];
			nextScene = numToScene(sceneNume);
		}
		_officeCount++;
	} else if (_thisClue == 2 && !_hadGoToMansion) {
		if (!_didContinue) {
			_thisDifficulty++;
			if (_thisDifficulty > 7)
				_thisDifficulty = 7;
		}
		_hadGoToMansion = 1;
		if (_poolHall) {
			_poolHall = 2;
			nextScene = "scene162";
		} else if (_casino) {
			_casino = 2;
			nextScene = "scene163";
		} else if (_warehouse) {
			_warehouse = 2;
			nextScene = "scene165";
		}
	} else if (_thisClue == 3 && _hadGoToMansion == 1) {
		if (_totalDies < 4) {
			_thisDifficulty++;
			if (_thisDifficulty > 7)
				_thisDifficulty = 7;
		}
		_hadGoToMansion = 2;
		if (_poolHall == 1) {
			_poolHall = 2;
			nextScene = "scene162";
		} else if (_casino == 1) {
			_casino = 2;
			nextScene = "scene163";
		} else if (_warehouse == 1) {
			_warehouse = 2;
			nextScene = "scene165";
		}
	} else if (_hadGoToMansion == 2 && _garage && _casino && _poolHall && _warehouse) {
		if (_totalDies < 5 || _didContinue <= 1) {
			_thisDifficulty++;
			if (_thisDifficulty > 7)
				_thisDifficulty = 7;
		}
		_hadGoToMansion = 3;
		nextScene = "scene166";
	}
	if (!nextScene.empty()) {
		_curScene = nextScene;
	} else {
		_curScene = numToScene(_thisMap + 174);
	}
}

void GameJohnnyRock::sceneNxtscnPickClue(Scene *scene) {
	uint16 picked = pickBits(&_clues, 12);
	_clueTable[_thisClue] = picked;
	_thisClue++;
	_curScene = numToScene(picked + 0xC3);
}

void GameJohnnyRock::sceneNxtscnMapTimeout(Scene *scene) {
	_mapTimeout++;
	if (_mapTimeout < 3) {
		_curScene = "scene360";
	} else {
		_curScene = "scene262";
	}
}

void GameJohnnyRock::sceneNxtscnEnterCasino(Scene *scene) {
	_placeBits = 0;
	_randomCount = 0;
	uint16 sceneNum;
	if (_casinoType != 0) {
		sceneNum = (pickBits(&_placeBits, 12) * 2) + 0x14;
	} else {
		sceneNum = (pickBits(&_placeBits, 8) * 2) + 0x2D;
	}
	_curScene = numToScene(sceneNum);
}

void GameJohnnyRock::sceneNxtscnCasinoWhat(Scene *scene) {
	_randomCount++;
	uint16 maxRandom = ((_thisDifficulty * 3) + 6);
	if (_randomCount > maxRandom) {
		_casino = 1;
		_curScene = "scene63a";
	} else {
		uint16 sceneNum;
		if (_casinoType != 0) {
			sceneNum = (pickBits(&_placeBits, 12) * 2) + 0x14;
		} else {
			sceneNum = (pickBits(&_placeBits, 8) * 2) + 0x2D;
		}
		_curScene = numToScene(sceneNum);
	}
}

void GameJohnnyRock::sceneNxtscnEnterPoolhall(Scene *scene) {
	if (_poolHallType == 0) {
		_curScene = "scene67";
	} else if (_poolHallType == 1) {
		_curScene = "scene73";
	} else {
		_curScene = "scene78";
	}
}

void GameJohnnyRock::sceneNxtscnPoolhallClue(Scene *scene) {
	_poolHall = 1;
	uint16 clue = pickBits(&_clues, 12);
	_clueTable[_thisClue] = clue;
	_thisClue++;
	_curScene = numToScene(clue + 0xC3);
}

void GameJohnnyRock::sceneNxtscnEnterWarehouse(Scene *scene) {
	if (_warehouseType == 0) {
		_curScene = "scene94";
	} else if (_warehouseType == 1) {
		_curScene = "scene102";
	} else {
		_curScene = "scene110";
	}
}

void GameJohnnyRock::sceneNxtscnWarehouseClue(Scene *scene) {
	_warehouse = 1;
	uint16 clue = pickBits(&_clues, 12);
	_clueTable[_thisClue] = clue;
	_thisClue++;
	_curScene = numToScene(clue + 0xC3);
}

void GameJohnnyRock::sceneNxtscnEnterGarage(Scene *scene) {
	if (_garageType != 0) {
		_curScene = "scene124";
	} else {
		_curScene = "scene131";
	}
}

void GameJohnnyRock::sceneNxtscnGarageClue(Scene *scene) {
	_garage = 1;
	uint16 clue = pickBits(&_clues, 12);
	_clueTable[_thisClue] = clue;
	_thisClue++;
	_curScene = numToScene(clue + 0xC3);
}

void GameJohnnyRock::sceneNxtscnEnterMansion(Scene *scene) {
	_mansion = 1;
	_randomCount++;
	uint16 maxRandom = ((_thisDifficulty * 2) + 7);
	if (_randomCount <= maxRandom) {
		uint16 picked = pickBits(&_placeBits, 5);
		_curScene = numToScene((picked * 2) + 0xB8);
	} else {
		_mansion = 2;
		_curScene = "scene194";
	}
}

void GameJohnnyRock::sceneNxtscnGiveClue(Scene *scene) {
	_score += 1000;
	_gameMoney += 50;
	sceneNxtscnPickMap(scene);
}

void GameJohnnyRock::sceneNxtscnPickFlowerMan(Scene *scene) {
	if (_rnd->getRandomBit()) {
		_curScene = "scene10a";
	} else {
		_curScene = "scene12a";
	}
}

void GameJohnnyRock::sceneNxtscnRandomScene(Scene *scene) {
	_randomCount++;
	if (_randomCount <= _maxRandomCount) {
		_placeBits = pickBits(&_placeBits, _randomScenes[0]);
		_curScene = numToScene(_randomScenes[_placeBits + 4]);
	} else {
		if (_randomScenes[2] != 0) {
			_curScene = numToScene(_randomScenes[2]);
		} else if (_repeatRandomPlace > 0) {
			_repeatRandomPlace--;
			_maxRepeat--;
			uint16 picked = pickRandomPlace(_repeatRandomPlace + 5);
			_curScene = numToScene(picked);
		} else {
			_curScene = numToScene(_gotoAfterRandom);
			_gotoAfterRandom = 0;
			_placeBits = 0;
			_randomCount = 1;
		}
	}
}

void GameJohnnyRock::sceneNxtscnEndRandScene(Scene *scene) {
	if (_repeatRandomPlace > 0) {
		_repeatRandomPlace--;
		_maxRepeat--;
		uint16 picked = pickRandomPlace(_repeatRandomPlace + 5);
		_curScene = numToScene(picked);
	} else {
		_curScene = numToScene(_gotoAfterRandom);
		_gotoAfterRandom = 0;
		_placeBits = 0;
		_randomCount = 1;
	}
}

void GameJohnnyRock::sceneNxtscnKillInnocent(Scene *scene) {
	if (!_debug_godMode) {
		_gameMoney -= 400;
	}
	if (_gameMoney < 0) {
		_retScene = "scene358";
		_curScene = "scene151";
	} else {
		switch (_rnd->getRandomNumber(2)) {
		case 0:
			_curScene = "scene151";
			break;
		case 1:
			_curScene = "scene152";
			break;
		case 2:
			_curScene = "scene153";
			break;
		}
	}
}

// Script functions: WepDwn
void GameJohnnyRock::sceneDefaultWepdwn(Scene *scene) {
	_inHolster = 9;
	_whichGun = 7;
	updateMouse();
}

// Debug methods
void GameJohnnyRock::debugWarpTo(int val) {
	// TODO implement
}

// Debugger methods
DebuggerJohnnyRock::DebuggerJohnnyRock(GameJohnnyRock *game) {
	_game = game;
	registerVar("drawRects", &game->_debug_drawRects);
	registerVar("godMode", &game->_debug_godMode);
	registerVar("unlimitedAmmo", &game->_debug_unlimitedAmmo);
	registerCmd("warpTo", WRAP_METHOD(DebuggerJohnnyRock, cmdWarpTo));
	registerCmd("dumpLib", WRAP_METHOD(DebuggerJohnnyRock, cmdDumpLib));
}

bool DebuggerJohnnyRock::cmdWarpTo(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: warp <int>");
		return true;
	} else {
		int val = atoi(argv[1]);
		_game->debugWarpTo(val);
		return false;
	}
}

bool DebuggerJohnnyRock::cmdDumpLib(int argc, const char **argv) {
	return _game->debug_dumpLibFile();
}

} // End of namespace Alg
