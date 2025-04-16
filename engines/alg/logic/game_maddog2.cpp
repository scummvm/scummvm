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
#include "alg/logic/game_maddog2.h"
#include "alg/scene.h"

namespace Alg {

GameMaddog2::GameMaddog2(AlgEngine *vm, const AlgGameDescription *gd) : Game(vm) {
}

GameMaddog2::~GameMaddog2() {
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

void GameMaddog2::init() {
	Game::init();

	_videoPosX = 11;
	_videoPosY = 2;

	setupCursorTimer();

	if(_vm->useSingleSpeedVideos()) {
		loadLibArchive("maddog2.lib");
	} else {
		loadLibArchive("maddog2d.lib");
	}

	_sceneInfo->loadScnFile("maddog2.scn");
	_startScene = _sceneInfo->getStartScene();

	registerScriptFunctions();
	verifyScriptFunctions();

	_menuzone = new Zone("MainMenu", "GLOBALHIT");
	_menuzone->addRect(0x0C, 0xAA, 0x38, 0xC7, nullptr, 0, "SHOTMENU", "0");
	_menuzone->addRect(0x08, 0xA9, 0x013C, 0xC7, nullptr, 0, "DEFAULT", "0"); // _mm_bott

	_submenzone = new Zone("SubMenu", "GLOBALHIT");
	_submenzone->addRect(0x2F, 0x16, 0x64, 0x2B, nullptr, 0, "STARTMENU", "0");
	_submenzone->addRect(0x2F, 0xA0, 0x8D, 0xC7, nullptr, 0, "CONTMENU", "0");
	_submenzone->addRect(0x2F, 0x40, 0x64, 0x54, nullptr, 0, "RECTSAVE", "0");
	_submenzone->addRect(0x2F, 0x6E, 0x7B, 0x86, nullptr, 0, "RECTLOAD", "0");
	_submenzone->addRect(0xEC, 0x15, 0x0122, 0x2C, nullptr, 0, "EXITMENU", "0");
	_submenzone->addRect(0xAD, 0x58, 0xF2, 0x70, nullptr, 0, "RECTEASY", "0");
	_submenzone->addRect(0xBC, 0x78, 0xF2, 0x93, nullptr, 0, "RECTAVG", "0");
	_submenzone->addRect(0xB8, 0x9D, 0xF2, 0xC7, nullptr, 0, "RECTHARD", "0");

	_shotSound = loadSoundFile("blow.8b");
	_emptySound = loadSoundFile("empty.8b");
	_saveSound = loadSoundFile("saved.8b");
	_loadSound = loadSoundFile("loaded.8b");
	_skullSound = loadSoundFile("skull.8b");

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
	auto hole = AlgGraphics::loadAniImage("hole.ani", _palette);
	_bulletholeIcon = (*hole)[0];

	_background = AlgGraphics::loadVgaBackground("backgrnd.vga", _palette);
	_screen->copyRectToSurface(_background->getPixels(), _background->pitch, 0, 0, _background->w, _background->h);

	moveMouse();
}

void GameMaddog2::registerScriptFunctions() {
#define ZONE_PTRFB_FUNCTION(name, func) _zonePtrFb[name] = new MD2ScriptFunctionPoint(this, &GameMaddog2::func);
	ZONE_PTRFB_FUNCTION("DEFAULT", zoneBullethole);
	ZONE_PTRFB_FUNCTION("BULLETHOLE", zoneBullethole);
	ZONE_PTRFB_FUNCTION("SKULL", zoneSkullhole);
#undef ZONE_PTRFB_FUNCTION

#define RECT_HIT_FUNCTION(name, func) _rectHitFuncs[name] = new MD2ScriptFunctionRect(this, &GameMaddog2::func);
	RECT_HIT_FUNCTION("DEFAULT", rectNewScene);
	RECT_HIT_FUNCTION("NEWSCENE", rectNewScene);
	RECT_HIT_FUNCTION("EXITMENU", rectExit);
	RECT_HIT_FUNCTION("CONTMENU", rectContinue);
	RECT_HIT_FUNCTION("STARTMENU", rectStart);
	RECT_HIT_FUNCTION("SHOTMENU", rectShotmenu);
	RECT_HIT_FUNCTION("RECTSAVE", rectSave);
	RECT_HIT_FUNCTION("RECTLOAD", rectLoad);
	RECT_HIT_FUNCTION("RECTEASY", rectEasy);
	RECT_HIT_FUNCTION("RECTAVG", rectAverage);
	RECT_HIT_FUNCTION("RECTHARD", rectHard);
	RECT_HIT_FUNCTION("SKULL", rectSkull);
	RECT_HIT_FUNCTION("KILLINNOCENTMAN", rectKillInnocentMan);
	RECT_HIT_FUNCTION("KILLINNOCENTWOMAN", rectKillInnocentWoman);
	RECT_HIT_FUNCTION("SELECTBEAVER", rectSelectBeaver);
	RECT_HIT_FUNCTION("SELECTBONNIE", rectSelectBonnie);
	RECT_HIT_FUNCTION("SELECTPROFESSOR", rectSelectProfessor);
	RECT_HIT_FUNCTION("SHOTAMMO", rectShotAmmo);
	RECT_HIT_FUNCTION("SHOTGIN", rectShotGin);
	RECT_HIT_FUNCTION("SHOTLANTERN", rectShotLantern);
	RECT_HIT_FUNCTION("SHOOTSKULL", rectShootSkull);

#undef RECT_HIT_FUNCTION

#define PRE_OPS_FUNCTION(name, func) _scenePreOps[name] = new MD2ScriptFunctionScene(this, &GameMaddog2::func);
	PRE_OPS_FUNCTION("DRAWRCT", scenePsoDrawRct);
	PRE_OPS_FUNCTION("PAUSE", scenePsoPause);
	PRE_OPS_FUNCTION("FADEIN", scenePsoFadeIn);
	PRE_OPS_FUNCTION("PAUSFI", scenePsoPauseFadeIn);
	PRE_OPS_FUNCTION("PREREAD", scenePsoPreRead);
	PRE_OPS_FUNCTION("PAUSPR", scenePsoPausePreRead);
	PRE_OPS_FUNCTION("DEFAULT", scenePsoDrawRct);
	PRE_OPS_FUNCTION("DRAWRCTFDI", scenePsoDrawRctFadeIn);
	PRE_OPS_FUNCTION("PRESHOOTOUT", scenePsoShootout);
	PRE_OPS_FUNCTION("PREMDSHOOTOUT", scenePsoMDShootout);
#undef PRE_OPS_FUNCTION

#define INS_OPS_FUNCTION(name, func) _sceneInsOps[name] = new MD2ScriptFunctionScene(this, &GameMaddog2::func);
	INS_OPS_FUNCTION("DEFAULT", sceneIsoDoNothing);
	INS_OPS_FUNCTION("PAUSE", sceneIsoPause);
	INS_OPS_FUNCTION("SPAUSE", sceneIsoShootpastPause);
	INS_OPS_FUNCTION("STARTGAME", sceneIsoStartGame);
	INS_OPS_FUNCTION("SHOOTPAST", sceneIsoShootpast);
	INS_OPS_FUNCTION("STAGECOACH", sceneIsoStagecoach);
	INS_OPS_FUNCTION("DIFFERENTPADRES", sceneIsoDifferentPadres);
	INS_OPS_FUNCTION("DIFFERENTPADRESPAS", sceneIsoDifferentPadresPause);
	INS_OPS_FUNCTION("DONTPOPNEXT", sceneIsoDontPopNext);
	INS_OPS_FUNCTION("GETINTOROCK", sceneIsoGetIntoRock);
	INS_OPS_FUNCTION("BENATCAVE", sceneIsoBenAtCave);
	INS_OPS_FUNCTION("SKULLATCAVE", sceneIsoSkullAtCave);
	INS_OPS_FUNCTION("STARTOFTRAIN", sceneIsoStartOfTrain);
	INS_OPS_FUNCTION("MISSION", sceneIsoMission);
	INS_OPS_FUNCTION("MDSHOOTOUT", sceneIsoMDShootout);
	INS_OPS_FUNCTION("STARTOFBOARDINGHOUSE", sceneIsoStartOfBoardinghouse);
	INS_OPS_FUNCTION("DONTCONTINUE", sceneIsoDontContinue);
	INS_OPS_FUNCTION("DOSHOOTOUT", sceneIsoDoShootout);
#undef INS_OPS_FUNCTION

#define NXT_SCN_FUNCTION(name, func) _sceneNxtScn[name] = new MD2ScriptFunctionScene(this, &GameMaddog2::func);
	NXT_SCN_FUNCTION("DEFAULT", sceneDefaultNxtscn);
	NXT_SCN_FUNCTION("DIED", sceneNxtscnDied);
	NXT_SCN_FUNCTION("KILLINNOCENTMAN", sceneNxtscnKillInnocentMan);
	NXT_SCN_FUNCTION("KILLINNOCENTWOMAN", sceneNxtscnKillInnocentWoman);
	NXT_SCN_FUNCTION("KILLGUIDE", sceneNxtscnKillGuide);
	NXT_SCN_FUNCTION("SCN_SHOOTSKULL", sceneNxtscnShootSkull);
	NXT_SCN_FUNCTION("CALLATTRACT", sceneNxtscnCallAttract);
	NXT_SCN_FUNCTION("PICKUNDERTAKER", sceneNxtscnPickUndertaker);
	NXT_SCN_FUNCTION("CHOOSEPADRE", sceneNxtscnChoosePadre);
	NXT_SCN_FUNCTION("SELECTGUIDE", sceneNxtscnSelectGuide);
	NXT_SCN_FUNCTION("SAVEBONNIE", sceneNxtscnSaveBonnie);
	NXT_SCN_FUNCTION("FINISHBONNIE", sceneNxtscnFinishBonnie);
	NXT_SCN_FUNCTION("SHOWGGCLUE", sceneNxtscnShowGGClue);
	NXT_SCN_FUNCTION("BBAFTERCLUE", sceneNxtscnBBAfterClue);
	NXT_SCN_FUNCTION("ASFARSHEGOES", sceneNxtscnAsFarSheGoes);
	NXT_SCN_FUNCTION("SAVEBEAVER", sceneNxtscnSaveBeaver);
	NXT_SCN_FUNCTION("FINISHBEAVER", sceneNxtscnFinishBeaver);
	NXT_SCN_FUNCTION("TOGATLINGUNSBCLUE", sceneNxtscnToGatlingGunSBClue);
	NXT_SCN_FUNCTION("TOGUIDEAFTERCLUE", sceneNxtscnToGuideafterClue);
	NXT_SCN_FUNCTION("TOGUIDECAVE", sceneNxtscnToGuideCave);
	NXT_SCN_FUNCTION("INITRANDOMVILLAGE", sceneNxtscnInitRandomVillage);
	NXT_SCN_FUNCTION("PICKVILLAGESCENES", sceneNxtscnPickVillageScenes);
	NXT_SCN_FUNCTION("SAVEPROFESSOR", sceneNxtscnSaveProfessor);
	NXT_SCN_FUNCTION("FINISHPROFESSOR", sceneNxtscnFinishProfessor);
	NXT_SCN_FUNCTION("TOGATLINGUNTPCLUE", sceneNxtscnToGatlingGunTPClue);
	NXT_SCN_FUNCTION("TPAFTERCLUE", sceneNxtscnTPAfterClue);
	NXT_SCN_FUNCTION("FINISHGATLINGUN1", sceneNxtscnFinishGatlingGun1);
	NXT_SCN_FUNCTION("FINISHGUYATGG", sceneNxtscnFinishGuyAtGG);
	NXT_SCN_FUNCTION("FINISHGATLINGUN2", sceneNxtscnFinishGatlingGun2);
	NXT_SCN_FUNCTION("HOWWEDID", sceneNxtscnHowWeDid);
	NXT_SCN_FUNCTION("PLAYERWON", sceneNxtscnPlayerWon);
	NXT_SCN_FUNCTION("BACKTONXTGUIDE", sceneNxtscnBackToNextGuide);
	NXT_SCN_FUNCTION("FINISHGENERICSCENE", sceneNxtscnFinishGenericScene);
	NXT_SCN_FUNCTION("INITRANDOMCOWBOYS", sceneNxtscnInitRandomCowboys);
	NXT_SCN_FUNCTION("TOCOWBOYSCENES", sceneNxtscnToCowboyScenes);
	NXT_SCN_FUNCTION("INITRANDOMFARMYARD", sceneNxtscnInitRandomFarmyard);
	NXT_SCN_FUNCTION("TOFARMYARDSCENES", sceneNxtscnToFarmyardScenes);
	NXT_SCN_FUNCTION("INITRANDOMCAVE", sceneNxtscnInitRandomCave);
	NXT_SCN_FUNCTION("TOCAVESCENES", sceneNxtscnToCaveScenes);
	NXT_SCN_FUNCTION("PICKSKULLATCAVE", sceneNxtscnPickSkullAtCave);
	NXT_SCN_FUNCTION("DRAWGUN", sceneNxtscnDrawGun);
#undef NXT_SCN_FUNCTION

	_sceneShowMsg["DEFAULT"] = new MD2ScriptFunctionScene(this, &GameMaddog2::sceneSmDonothing);
	_sceneWepDwn["DEFAULT"] = new MD2ScriptFunctionScene(this, &GameMaddog2::sceneDefaultWepdwn);
	_sceneScnScr["DEFAULT"] = new MD2ScriptFunctionScene(this, &GameMaddog2::sceneDefaultScore);
	_sceneNxtFrm["DEFAULT"] = new MD2ScriptFunctionScene(this, &GameMaddog2::sceneNxtfrm);
}

void GameMaddog2::verifyScriptFunctions() {
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

MD2ScriptFunctionPoint GameMaddog2::getScriptFunctionZonePtrFb(Common::String name) {
	auto it = _zonePtrFb.find(name);
	if (it != _zonePtrFb.end()) {
		return *it->_value;
	} else {
		error("GameMaddog2::getScriptFunctionZonePtrFb(): Could not find zonePtrFb function: %s", name.c_str());
	}
}

MD2ScriptFunctionRect GameMaddog2::getScriptFunctionRectHit(Common::String name) {
	auto it = _rectHitFuncs.find(name);
	if (it != _rectHitFuncs.end()) {
		return *it->_value;
	} else {
		error("GameMaddog2::getScriptFunctionRectHit(): Could not find rectHit function: %s", name.c_str());
	}
}

MD2ScriptFunctionScene GameMaddog2::getScriptFunctionScene(SceneFuncType type, Common::String name) {
	MD2ScriptFunctionSceneMap *functionMap;
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
		error("GameMaddog2::getScriptFunctionScene(): Unkown scene script type: %u", type);
		break;
	}
	MD2ScriptFunctionSceneMap::iterator it;
	it = functionMap->find(name);
	if (it != functionMap->end()) {
		return *it->_value;
	} else {
		error("GameMaddog2::getScriptFunctionScene(): Could not find scene type %u function: %s", type, name.c_str());
	}
}

void GameMaddog2::callScriptFunctionZonePtrFb(Common::String name, Common::Point *point) {
	MD2ScriptFunctionPoint function = getScriptFunctionZonePtrFb(name);
	function(point);
}

void GameMaddog2::callScriptFunctionRectHit(Common::String name, Rect *rect) {
	MD2ScriptFunctionRect function = getScriptFunctionRectHit(name);
	function(rect);
}

void GameMaddog2::callScriptFunctionScene(SceneFuncType type, Common::String name, Scene *scene) {
	MD2ScriptFunctionScene function = getScriptFunctionScene(type, name);
	function(scene);
}

Common::Error GameMaddog2::run() {
	init();
	newGame();
	_curScene = _startScene;
	Common::String oldscene;
	while (!_vm->shouldQuit()) {
		oldscene = _curScene;
		_fired = false;
		Scene *scene = _sceneInfo->findScene(_curScene);
		if (!loadScene(scene)) {
			error("GameMaddog2::run(): Cannot find scene %s in libfile", scene->_name.c_str());
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

void GameMaddog2::newGame() {
	_shots = 6;
	_lives = 3;
	_score = 0;
	_holster = false;
	updateStat();
	_subScene = "";
}

void GameMaddog2::resetParams() {
	_retScene = "";
	_subScene = "";
	_lastScene = "";
	_lives = 3;
	_shots = 6;
	_score = 0;
	_whichPadre = 0;
	_whichGatlingGun = _rnd->getRandomNumber(2);
	_gotTo = 0x22;
	_lastShootOut = -1;
	_sbGotTo = 0;
	_bbGotTo = 0;
	_tpGotTo = 0;
	_shootoutFromDie = false;
	_wasAShootout = false;
	_shootOutCnt = 0;
	_thisGuide = -1;
	_doneGuide = 0;
	_totalDies = 0;
	_hadSkull = false;
	_inShootout = false;
	updateStat();
}

void GameMaddog2::doMenu() {
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

void GameMaddog2::updateStat() {
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

void GameMaddog2::changeDifficulty(uint8 newDifficulty) {
	if (newDifficulty == _oldDifficulty) {
		return;
	}
	showDifficulty(newDifficulty, true);
	Game::adjustDifficulty(newDifficulty, _oldDifficulty);
	_oldDifficulty = newDifficulty;
	_difficulty = newDifficulty;
}

void GameMaddog2::showDifficulty(uint8 newDifficulty, bool cursor) {
	// reset menu screen
	_screen->copyRectToSurface(_background->getBasePtr(_videoPosX, _videoPosY), _background->pitch, _videoPosX, _videoPosY, _videoDecoder->getWidth(), _videoDecoder->getHeight());
	AlgGraphics::drawImageCentered(_screen, _knifeIcon, _diffPos[newDifficulty - 1][0], _diffPos[newDifficulty - 1][1]);
	if (cursor) {
		updateCursor();
	}
}

void GameMaddog2::updateCursor() {
	updateMouse();
}

void GameMaddog2::updateMouse() {
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

void GameMaddog2::moveMouse() {
	if (_inMenu) {
		_whichGun = 8;
	} else {
		if (_mousePos.y >= 0xAA) {
			if (_mousePos.x >= 0x113) {
				if (_inHolster == 0) {
					_whichGun = 6;
				} else {
					_whichGun = 7;
				}
			} else if (_mousePos.x <= 0x37) {
				_whichGun = 0xA;
			} else {
				_whichGun = 0;
			}
		} else if (_whichGun > 5) {
			_whichGun = 0;
		}
	}
	updateMouse();
}

void GameMaddog2::displayScore() {
	if (_score == _oldScore) {
		return;
	}
	_oldScore = _score;
	Common::String scoreString = Common::String::format("%05d", _score);
	int posX = 0xE6;
	for (int i = 0; i < 5; i++) {
		uint8 digit = scoreString[i] - '0';
		AlgGraphics::drawImage(_screen, (*_numbers)[digit], posX, 0xBE);
		posX += 7;
	}
}

bool GameMaddog2::weaponDown() {
	if (_rightDown && _mousePos.y >= 0xAA && _mousePos.x >= 0x113) {
		return true;
	}
	return false;
}

bool GameMaddog2::saveState() {
	Common::OutSaveFile *outSaveFile;
	Common::String saveFileName = _vm->getSaveStateName(0);
	if (!(outSaveFile = g_system->getSavefileManager()->openForSaving(saveFileName))) {
		warning("GameMaddog2::saveState(): Can't create file '%s', game not saved", saveFileName.c_str());
		return false;
	}
	outSaveFile->writeUint32BE(MKTAG('A', 'L', 'G', 'S')); // header
	outSaveFile->writeByte(0);                             // version, unused for now
	outSaveFile->writeUint16LE(_totalDies);
	outSaveFile->writeUint16LE(_gotTo);
	outSaveFile->writeUint16LE(_sbGotTo);
	outSaveFile->writeUint16LE(_bbGotTo);
	outSaveFile->writeUint16LE(_tpGotTo);
	outSaveFile->writeSint16LE(_thisGuide);
	outSaveFile->writeUint16LE(_doneGuide);
	outSaveFile->writeUint16LE(_hadSkull);
	outSaveFile->writeUint16LE(_shootOutCnt);
	outSaveFile->writeUint16LE(_shootOutBits);
	outSaveFile->writeSint16LE(_lastShootOut);
	outSaveFile->writeUint16LE(_startLives);
	outSaveFile->writeByte(_wasAShootout);
	outSaveFile->writeByte(_shootoutFromDie);
	outSaveFile->writeUint16LE(_whichPadre);
	outSaveFile->writeUint16LE(_whichGatlingGun);
	outSaveFile->writeUint16LE(_lives);
	outSaveFile->writeUint16LE(_score);
	outSaveFile->writeUint16LE(_holster);
	outSaveFile->writeUint16LE(_inShootout);
	outSaveFile->writeUint16LE(_difficulty);
	outSaveFile->writeString(_lastScene);
	outSaveFile->writeByte(0);
	outSaveFile->writeString(_curScene);
	outSaveFile->writeByte(0);
	outSaveFile->writeString(_subScene);
	outSaveFile->writeByte(0);
	outSaveFile->writeString(_retScene);
	outSaveFile->writeByte(0);
	outSaveFile->finalize();
	delete outSaveFile;
	return true;
}

bool GameMaddog2::loadState() {
	Common::InSaveFile *inSaveFile;
	Common::String saveFileName = _vm->getSaveStateName(0);
	if (!(inSaveFile = g_system->getSavefileManager()->openForLoading(saveFileName))) {
		debug("GameMaddog2::loadState(): Can't load file '%s', game not loaded", saveFileName.c_str());
		return false;
	}
	uint32 header = inSaveFile->readUint32BE();
	if (header != MKTAG('A', 'L', 'G', 'S')) {
		warning("GameMaddog2::loadState(): Unkown save file, header: %s", tag2str(header));
		return false;
	}
	inSaveFile->skip(1); // version, unused for now
	_totalDies = inSaveFile->readUint16LE();
	_gotTo = inSaveFile->readUint16LE();
	_sbGotTo = inSaveFile->readUint16LE();
	_bbGotTo = inSaveFile->readUint16LE();
	_tpGotTo = inSaveFile->readUint16LE();
	_thisGuide = inSaveFile->readSint16LE();
	_doneGuide = inSaveFile->readUint16LE();
	_hadSkull = inSaveFile->readUint16LE();
	_shootOutCnt = inSaveFile->readUint16LE();
	_shootOutBits = inSaveFile->readUint16LE();
	_lastShootOut = inSaveFile->readSint16LE();
	_startLives = inSaveFile->readUint16LE();
	_wasAShootout = inSaveFile->readByte();
	_shootoutFromDie = inSaveFile->readByte();
	_whichPadre = inSaveFile->readUint16LE();
	_whichGatlingGun = inSaveFile->readUint16LE();
	_lives = inSaveFile->readUint16LE();
	_score = inSaveFile->readUint16LE();
	_holster = inSaveFile->readUint16LE();
	_inShootout = inSaveFile->readUint16LE();
	_difficulty = inSaveFile->readUint16LE();
	_lastScene = inSaveFile->readString();
	_curScene = inSaveFile->readString();
	_subScene = inSaveFile->readString();
	_retScene = inSaveFile->readString();
	delete inSaveFile;
	changeDifficulty(_difficulty);
	return true;
}

// misc game functions
Common::String GameMaddog2::numToScene(int n) {
	switch (n) {
	case 1:
	case 31:
	case 34:
	case 41:
	case 42:
	case 67:
	case 85:
	case 106:
	case 118:
	case 171:
	case 180:
	case 181:
	case 182:
	case 197:
	case 199:
	case 201:
	case 203:
	case 227:
	case 244:
	case 253:
	case 287:
	case 288:
	case 295:
	case 296:
		return Common::String::format("scen%da", n);
	default:
		return Common::String::format("scene%d", n);
	}
}

uint16 GameMaddog2::sceneToNum(Common::String sceneName) {
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

void GameMaddog2::defaultBullethole(Common::Point *point) {
	if (point->x >= 14 && point->x <= 306 && point->y >= 5 && point->y <= 169) {
		uint16 targetX = point->x - _videoPosX;
		uint16 targetY = point->y - _videoPosY;
		AlgGraphics::drawImageCentered(_videoDecoder->getVideoFrame(), _bulletholeIcon, targetX, targetY);
		updateCursor();
		_shotFired = true;
		doShot();
	}
}

uint16 GameMaddog2::die() {
	updateStat();
	uint8 randomNum = _rnd->getRandomNumber(9);
	if (randomNum >= 4 && _lives >= 3) {
		uint16 picked = pickBits(&_dieBits, 6);
		return _dieScenes[picked];
	}
	if (_lives == 2) {
		return 0x9D;
	} else if (_lives == 1) {
		return 0x9E;
	}
	return 0x9A;
}

uint16 GameMaddog2::pickBits(uint16 *bits, uint8 max) {
	// reset mask if full
	uint16 fullMask = 0xFFFF >> (16 - max);
	if (fullMask == _pickMask) {
		_pickMask = 0;
	}
	*bits |= _pickMask;
	if (fullMask == *bits) {
		*bits = _pickMask;
		if (fullMask == *bits) {
			_pickMask = 0;
			*bits = 0;
		}
	}
	uint16 randomNum = _rnd->getRandomNumber(max - 1);
	// find an unused bit
	while (true) {
		uint16 bitMask = 1 << randomNum;
		// if bit is already used or matches _lastPick, try next position
		if ((*bits & bitMask) || randomNum == _lastPick) {
			randomNum++;
			if (randomNum >= max) {
				randomNum = 0;
				_lastPick = 0xFFFF;
			}
			continue;
		}
		// found an unused bit
		break;
	}
	*bits |= (1 << randomNum);
	_lastPick = randomNum;
	_pickMask = 0;
	return randomNum;
}

uint16 GameMaddog2::pickShootout() {
	_shootOutCnt = 0;
	_wasAShootout = true;
	_lastPick = _lastShootOut;
	if (_difficulty == 1) {
		_lastShootOut = pickBits(&_shootOutBits, 5);
		return _ezShootOuts[_lastShootOut];
	} else {
		_lastShootOut = pickBits(&_shootOutBits, 6);
		return _shootOuts[_lastShootOut];
	}
}

void GameMaddog2::nextSB() {
	_shootoutFromDie = false;
	_hadSkull = false;
	_randomCount = 0;
	if (_wasAShootout) {
		_wasAShootout = false;
	} else {
		_sbGotTo++;
	}
	if (_sbScenes[_sbGotTo] == 0x87) {
		_placeBits = 0;
		_pickMask = 0;
		ggPickMan();
		// _scene_pso_fadein(cur_scene);
	} else {
		if (_sbGotTo == 7 || _sbGotTo == 9) {
			_curScene = numToScene(_sbScenes[_sbGotTo]);
		} else {
			_shootOutCnt++;
			if (_shootOutCnt <= 3) {
				_curScene = numToScene(_sbScenes[_sbGotTo]);
				// _scene_pso_fadein(cur_scene);
			} else {
				_shootoutFromDie = false;
				_curScene = numToScene(pickShootout());
			}
		}
	}
}

void GameMaddog2::nextBB() {
	_shootoutFromDie = false;
	_hadSkull = false;
	_randomCount = 0;
	if (_wasAShootout) {
		_wasAShootout = false;
	} else {
		_bbGotTo++;
	}
	if (_bbScenes[_bbGotTo] == 0x87) {
		_placeBits = 0;
		_pickMask = 0;
		ggPickMan();
		// _scene_pso_fadein(cur_scene);
	} else {
		if (_bbGotTo == 7 || _bbGotTo == 9) {
			_curScene = numToScene(_bbScenes[_bbGotTo]);
		} else {
			_shootOutCnt++;
			if (_shootOutCnt <= 3) {
				_curScene = numToScene(_bbScenes[_bbGotTo]);
				// _scene_pso_fadein(cur_scene);
			} else {
				_shootoutFromDie = false;
				_curScene = numToScene(pickShootout());
			}
		}
	}
}

void GameMaddog2::nextTP() {
	_shootoutFromDie = false;
	_hadSkull = false;
	_randomCount = 0;
	if (_wasAShootout) {
		_wasAShootout = false;
	} else {
		_tpGotTo++;
	}
	if (_tpScenes[_tpGotTo] == 0xDC) {
		_placeBits = 0;
		_pickMask = 0;
	}
	if (_tpScenes[_tpGotTo] == 0x87) {
		_placeBits = 0;
		_pickMask = 0;
		ggPickMan();
		// _scene_pso_fadein(cur_scene);
	} else {
		if (_tpGotTo == 7 || _tpGotTo == 9) {
			_curScene = numToScene(_tpScenes[_tpGotTo]);
		} else {
			_shootOutCnt++;
			if (_shootOutCnt <= 3) {
				_curScene = numToScene(_tpScenes[_tpGotTo]);
				// _scene_pso_fadein(cur_scene);
			} else {
				_shootoutFromDie = false;
				_curScene = numToScene(pickShootout());
			}
		}
	}
}

void GameMaddog2::ggPickMan() {
	_randomCount++;
	uint8 totalRandom = ((_difficulty - 1) * 2) + 3;
	if (_randomCount < totalRandom) {
		uint16 index = pickBits(&_placeBits, 3);
		_curScene = numToScene(_ggScenes[index]);
	} else {
		_curScene = "scene139";
	}
}

void GameMaddog2::genericNext() {
	if (_shootoutFromDie && _gotTo == 0x32) {
		_shootoutFromDie = false;
		_curScene = "scene50";
		return;
	}
	_hadSkull = false;
	_shootoutFromDie = false;
	switch (_gotTo) {
	case 227:
		_curScene = "scen227a";
		break;
	case 238:
		_curScene = "scene238";
		break;
	case 244:
		_curScene = "scen244a";
		break;
	case 254:
		_curScene = "scene254";
		break;
	case 287:
		_curScene = "scen287a";
		break;
	default:
		if (_thisGuide == 0) {
			nextSB();
			return;
		} else if (_thisGuide == 1) {
			nextBB();
			return;
		} else if (_thisGuide == 2) {
			nextTP();
			return;
		}
	}
}

void GameMaddog2::playerWon() {
	_doneGuide |= (1 << _thisGuide);
	if (_totalDies < _startLives || _doneGuide != 7) {
		_curScene = "scene290";
	} else {
		_curScene = "scene291";
	}
}

// Script functions: Zone
void GameMaddog2::zoneBullethole(Common::Point *point) {
	defaultBullethole(point);
}

void GameMaddog2::zoneSkullhole(Common::Point *point) {
	if (point->x >= 14 && point->x <= 306 && point->y >= 5 && point->y <= 169) {
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
void GameMaddog2::rectSkull(Rect *rect) {
	if (_hadSkull) {
		return;
	}
	_hadSkull = true;
	_shots = 12;
	_score += 1000;
	updateStat();
}

void GameMaddog2::rectKillInnocentMan(Rect *rect) {
	_totalDies++;
	_wasAShootout = false;
	if (!_debug_godMode) {
		_lives--;
	}
	if (_lives <= 0) {
		_shootOutCnt++;
	}
	updateStat();
	_curScene = "scene153";
}

void GameMaddog2::rectKillInnocentWoman(Rect *rect) {
	_totalDies++;
	_wasAShootout = false;
	if (!_debug_godMode) {
		_lives--;
	}
	if (_lives <= 0) {
		_shootOutCnt++;
	}
	updateStat();
	_curScene = "scene156";
}

void GameMaddog2::rectSelectBeaver(Rect *rect) {
	if (_doneGuide & 1) {
		return;
	}
	_wasAShootout = false;
	_thisGuide = 0;
	_randomCount = 0;
	_placeBits = 0;
	if (_sbScenes[_sbGotTo] == 0x87) {
		_pickMask = 0;
		ggPickMan();
		// scene_pso_fadein(cur_scene);
	} else {
		_curScene = numToScene(_sbScenes[_sbGotTo]);
		// scene_pso_fadein(cur_scene);
	}
}

void GameMaddog2::rectSelectBonnie(Rect *rect) {
	if (_doneGuide & 2) {
		return;
	}
	_wasAShootout = false;
	_thisGuide = 1;
	_randomCount = 0;
	_placeBits = 0;
	if (_bbScenes[_bbGotTo] == 0x87) {
		_pickMask = 0;
		ggPickMan();
		// scene_pso_fadein(cur_scene);
	} else {
		_curScene = numToScene(_bbScenes[_bbGotTo]);
		// scene_pso_fadein(cur_scene);
	}
}

void GameMaddog2::rectSelectProfessor(Rect *rect) {
	if (_doneGuide & 4) {
		return;
	}
	_wasAShootout = false;
	_thisGuide = 2;
	_randomCount = 0;
	_placeBits = 0;
	if (_tpScenes[_tpGotTo] == 0x87) {
		_pickMask = 0;
		ggPickMan();
		// scene_pso_fadein(cur_scene);
	} else {
		_curScene = numToScene(_tpScenes[_tpGotTo]);
		// scene_pso_fadein(cur_scene);
	}
}

void GameMaddog2::rectShotAmmo(Rect *rect) {
	if (_whichGatlingGun == 0) {
		_curScene = "scene140";
	} else {
		switch (_thisGuide) {
		case 0:
			_sbGotTo = 5;
			break;
		case 1:
			_bbGotTo = 5;
			break;
		case 2:
			_tpGotTo = 5;
			break;
		}
		_curScene = "scene299";
	}
}

void GameMaddog2::rectShotGin(Rect *rect) {
	if (_whichGatlingGun == 1) {
		_curScene = "scene140";
	} else {
		switch (_thisGuide) {
		case 0:
			_sbGotTo = 5;
			break;
		case 1:
			_bbGotTo = 5;
			break;
		case 2:
			_tpGotTo = 5;
			break;
		}
		_curScene = "scene299";
	}
}

void GameMaddog2::rectShotLantern(Rect *rect) {
	if (_whichGatlingGun == 2) {
		_curScene = "scene140";
	} else {
		switch (_thisGuide) {
		case 0:
			_sbGotTo = 5;
			break;
		case 1:
			_bbGotTo = 5;
			break;
		case 2:
			_tpGotTo = 5;
			break;
		}
		_curScene = "scene299";
	}
}

void GameMaddog2::rectShootSkull(Rect *rect) {
	sceneNxtscnShootSkull(nullptr);
}

void GameMaddog2::rectShotmenu(Rect *rect) {
	doMenu();
}

void GameMaddog2::rectSave(Rect *rect) {
	if (saveState()) {
		doSaveSound();
	}
}

void GameMaddog2::rectLoad(Rect *rect) {
	if (loadState()) {
		doLoadSound();
	}
}

void GameMaddog2::rectContinue(Rect *rect) {
	_inMenu = false;
	_fired = false;
	if (_lives <= 0) {
		_curScene = _lastScene;
		_subScene = "";
		_retScene = "";
		newGame();
		_hadSkull = false;
		_shootoutFromDie = false;
		_wasAShootout = false;
	} else {
		updateStat();
	}
}

void GameMaddog2::rectStart(Rect *rect) {
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

// Script functions: Scene PreOps
void GameMaddog2::scenePsoShootout(Scene *scene) {
	sscanf(scene->_preopParam.c_str(), "#%ldto%ld", &_minF, &_maxF);
	if (!_debug_unlimitedAmmo) {
		_shots = 0;
	}
	_inShootout = true;
	updateStat();
	AlgGraphics::drawImage(_screen, _reloadIcon, 0x37, 0xBE);
	updateCursor();
}

void GameMaddog2::scenePsoMDShootout(Scene *scene) {
	sscanf(scene->_preopParam.c_str(), "#%ldto%ld", &_minF, &_maxF);
	if (!_debug_unlimitedAmmo) {
		_shots = 0;
	}
	_inShootout = true;
	updateStat();
	AlgGraphics::drawImage(_screen, _reloadIcon, 0x37, 0xBE);
	updateCursor();
}

// Script functions: Scene Scene InsOps
void GameMaddog2::sceneIsoShootpast(Scene *scene) {
	if (_lives <= 0) {
		return;
	}
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

void GameMaddog2::sceneIsoShootpastPause(Scene *scene) {
	sceneIsoShootpast(scene);
	sceneIsoPause(scene);
}

void GameMaddog2::sceneIsoStagecoach(Scene *scene) {
	_hadSkull = false;
	_lastScene = "scen34a";
	sceneIsoPause(scene);
}

void GameMaddog2::sceneIsoDifferentPadres(Scene *scene) {
	_gotTo = 0x32;
	_lastScene = "scene50";
}

void GameMaddog2::sceneIsoDifferentPadresPause(Scene *scene) {
	_gotTo = 0x32;
	_lastScene = "scene50";
	sceneIsoPause(scene);
}

void GameMaddog2::sceneIsoDontPopNext(Scene *scene) {
	_lastPick = 0;
	_lastScene = "scene135";
}

void GameMaddog2::sceneIsoGetIntoRock(Scene *scene) {
	_lastScene = "scene50";
}

void GameMaddog2::sceneIsoBenAtCave(Scene *scene) {
	_gotTo = sceneToNum(_curScene);
	_lastScene = "scen227a";
	sceneIsoShootpast(scene);
}

void GameMaddog2::sceneIsoSkullAtCave(Scene *scene) {
	_gotTo = sceneToNum(_curScene);
	_lastScene = "scene238";
}

void GameMaddog2::sceneIsoStartOfTrain(Scene *scene) {
	_gotTo = sceneToNum(_curScene);
	_lastScene = "scen244a";
}

void GameMaddog2::sceneIsoMission(Scene *scene) {
	_gotTo = sceneToNum(_curScene);
	_lastScene = "scene254";
}

void GameMaddog2::sceneIsoStartOfBoardinghouse(Scene *scene) {
	_gotTo = sceneToNum(_curScene);
	_lastScene = "scen295a";
}

void GameMaddog2::sceneIsoDontContinue(Scene *scene) {
	sceneNxtscnCallAttract(scene);
}

void GameMaddog2::sceneIsoMDShootout(Scene *scene) {
	_gotTo = sceneToNum(_curScene);
	_lastScene = "scen287a";
	sceneIsoDoShootout(scene);
}

void GameMaddog2::sceneIsoDoShootout(Scene *scene) {
	if (_currentFrame < (uint32)_minF) {
		if (!_debug_unlimitedAmmo) {
			_shots = 0;
		}
		return;
	}
	if (_inShootout) {
		AlgGraphics::drawImage(_screen, _drawIcon, 0x37, 0xBE);
		updateCursor();
	}
	_inShootout = false;
	if (_shots > 0) {
		if (_currentFrame < (uint32)_maxF) {
			callScriptFunctionScene(NXTSCN, scene->_nxtscn, scene);
		}
	}
}

// Script functions: Scene NxtScn
void GameMaddog2::sceneDefaultNxtscn(Scene *scene) {
	// wipe background drawing from shootout
	_screen->copyRectToSurface(_background->getBasePtr(0x37, 0xBE), _background->pitch, 0x37, 0xBE, _reloadIcon->w, _reloadIcon->h);
	Game::sceneDefaultNxtscn(scene);
}

void GameMaddog2::sceneNxtscnDrawGun(Scene *scene) {
	updateCursor();
	sceneDefaultNxtscn(scene);
}

void GameMaddog2::sceneNxtscnDied(Scene *scene) {
	_totalDies++;
	_shootoutFromDie = false;
	_wasAShootout = false;
	if (!_debug_godMode) {
		_lives--;
	}
	updateStat();
	_curScene = numToScene(die());
}

void GameMaddog2::sceneNxtscnKillInnocentMan(Scene *scene) {
	_totalDies++;
	_wasAShootout = false;
	if (!_debug_godMode) {
		_lives--;
	}
	_shootOutCnt++;
	updateStat();
	_curScene = "scene153";
}

void GameMaddog2::sceneNxtscnKillInnocentWoman(Scene *scene) {
	_totalDies++;
	_wasAShootout = false;
	if (!_debug_godMode) {
		_lives--;
	}
	_shootOutCnt++;
	updateStat();
	_curScene = "scene156";
}

void GameMaddog2::sceneNxtscnKillGuide(Scene *scene) {
	if (!_debug_godMode) {
		_lives--;
	}
	updateStat();
	_curScene = "scene160";
}

void GameMaddog2::sceneNxtscnShootSkull(Scene *scene) {
	if (_hadSkull) {
		return;
	}
	_hadSkull = true;
	doSkullSound();
	_shots = 12;
	_score += 1000;
	updateStat();
	_curScene = "scene293";
}

void GameMaddog2::sceneNxtscnCallAttract(Scene *scene) {
	resetParams();
	_curScene = "scene1aa";
}

void GameMaddog2::sceneNxtscnPickUndertaker(Scene *scene) {
	if (_lives <= 0) {
		_curScene = "scene311";
		return;
	}
	if (_gotTo == 0x127 || _gotTo == 0x22) {
		_curScene = numToScene(_gotTo);
	} else {
		if (_whichPadre == 0) {
			_whichPadre = 2;
			_gotTo = 0x32;
			if (_rnd->getRandomBit()) {
				_curScene = "scene49";
			} else {
				_curScene = "scene47";
			}
		} else {
			_shootOutCnt++;
			if (_shootOutCnt > 3) {
				_shootoutFromDie = true;
				_curScene = numToScene(pickShootout());
			} else {
				_curScene = numToScene(_gotTo);
			}
		}
	}
}

void GameMaddog2::sceneNxtscnChoosePadre(Scene *scene) {
	_hadSkull = false;
	if (_whichPadre == 0) {
		_gotTo = 0x32;
		_whichPadre = 1;
		_curScene = "scene45";
	} else if (_whichPadre == 1) {
		_gotTo = 0x32;
		_whichPadre = 2;
		if (_rnd->getRandomBit()) {
			_curScene = "scene49";
		} else {
			_curScene = "scene47";
		}
	} else {
		_gotTo = 0x32;
		_curScene = numToScene(0x32);
	}
}

void GameMaddog2::sceneNxtscnSelectGuide(Scene *scene) {
	_wasAShootout = false;
	if (_thisGuide < 0) {
		_thisGuide = _rnd->getRandomNumber(2);
	}
	_randomCount = 0;
	_placeBits = 0;
	if (_doneGuide & (1 << _thisGuide)) {
		_thisGuide = 0;
		while (_thisGuide < 3) {
			if (!(_doneGuide & (1 << _thisGuide))) {
				break;
			}
			_thisGuide++;
		}
	}
	switch (_thisGuide) {
	case 0:
		_curScene = numToScene(_sbScenes[_sbGotTo]);
		break;
	case 1:
		_curScene = numToScene(_bbScenes[_bbGotTo]);
		break;
	case 2:
		_curScene = numToScene(_tpScenes[_tpGotTo]);
		break;
	}
}

void GameMaddog2::sceneNxtscnSaveBonnie(Scene *scene) {
	nextBB();
}

void GameMaddog2::sceneNxtscnFinishBonnie(Scene *scene) {
	nextBB();
}

void GameMaddog2::sceneNxtscnShowGGClue(Scene *scene) {
	_shootoutFromDie = false;
	_curScene = numToScene(_bbClue[_whichGatlingGun]);
}

void GameMaddog2::sceneNxtscnBBAfterClue(Scene *scene) {
	_shootoutFromDie = false;
	_randomCount = 0;
	_bbGotTo = 6;
	_curScene = numToScene(_bbScenes[_bbGotTo]);
}

void GameMaddog2::sceneNxtscnAsFarSheGoes(Scene *scene) {
	_shootoutFromDie = false;
	_randomCount = 0;
	_gotTo = 12;
	_curScene = numToScene(_bbScenes[_gotTo]);
}

void GameMaddog2::sceneNxtscnSaveBeaver(Scene *scene) {
	nextSB();
}

void GameMaddog2::sceneNxtscnFinishBeaver(Scene *scene) {
	nextSB();
}

void GameMaddog2::sceneNxtscnToGatlingGunSBClue(Scene *scene) {
	_shootoutFromDie = false;
	_curScene = numToScene(_sbClue[_whichGatlingGun]);
}

void GameMaddog2::sceneNxtscnToGuideafterClue(Scene *scene) {
	_shootoutFromDie = false;
	_randomCount = 0;
	_sbGotTo = 6;
	_curScene = numToScene(_sbScenes[_sbGotTo]);
}

void GameMaddog2::sceneNxtscnToGuideCave(Scene *scene) {
	_shootoutFromDie = false;
	_randomCount = 0;
	_sbGotTo = 12;
	_gotTo = _sbScenes[_sbGotTo];
	_curScene = numToScene(_gotTo);
}

void GameMaddog2::sceneNxtscnInitRandomVillage(Scene *scene) {
	_shootoutFromDie = false;
	_placeBits = 0;
	_randomCount = 0;
	int index = pickBits(&_placeBits, 6);
	_curScene = numToScene(_villageScenes[index]);
}

void GameMaddog2::sceneNxtscnPickVillageScenes(Scene *scene) {
	_randomCount++;
	uint8 totalRandom = ((_difficulty - 1) * 2) + 5;
	if (_randomCount < totalRandom) {
		int index = pickBits(&_placeBits, 6);
		_curScene = numToScene(_villageScenes[index]);
	} else {
		_curScene = "scene100";
	}
}

void GameMaddog2::sceneNxtscnSaveProfessor(Scene *scene) {
	nextTP();
}

void GameMaddog2::sceneNxtscnFinishProfessor(Scene *scene) {
	nextTP();
}

void GameMaddog2::sceneNxtscnToGatlingGunTPClue(Scene *scene) {
	_shootoutFromDie = false;
	_curScene = numToScene(_tpClue[_whichGatlingGun]);
}

void GameMaddog2::sceneNxtscnTPAfterClue(Scene *scene) {
	_shootoutFromDie = false;
	_randomCount = 0;
	_tpGotTo = 6;
	_curScene = numToScene(_tpScenes[_tpGotTo]);
}

void GameMaddog2::sceneNxtscnFinishGatlingGun1(Scene *scene) {
	_shootoutFromDie = false;
	_randomCount = 0;
	_tpGotTo = 12;
	_gotTo = _tpScenes[_tpGotTo];
	_curScene = numToScene(_gotTo);
}

void GameMaddog2::sceneNxtscnFinishGuyAtGG(Scene *scene) {
	ggPickMan();
}

void GameMaddog2::sceneNxtscnFinishGatlingGun2(Scene *scene) {
	_shootoutFromDie = false;
	sceneNxtscnFinishGenericScene(scene);
}

void GameMaddog2::sceneNxtscnHowWeDid(Scene *scene) {
	playerWon();
}

void GameMaddog2::sceneNxtscnPlayerWon(Scene *scene) {
	_curScene = "scene1aa";
}

void GameMaddog2::sceneNxtscnBackToNextGuide(Scene *scene) {
	_doneGuide |= (1 << _thisGuide);
	_gotTo = 0x32;
	_curScene = numToScene(_gotTo);
}

void GameMaddog2::sceneNxtscnFinishGenericScene(Scene *scene) {
	genericNext();
}

void GameMaddog2::sceneNxtscnInitRandomCowboys(Scene *scene) {
	_placeBits = 0;
	_randomCount = 0;
	uint16 picked = pickBits(&_placeBits, 7);
	_curScene = numToScene(_cowboyScenes[picked]);
}

void GameMaddog2::sceneNxtscnToCowboyScenes(Scene *scene) {
	_randomCount++;
	uint8 totalRandom = ((_difficulty - 1) * 2) + 7;
	if (_randomCount < totalRandom) {
		uint16 picked = pickBits(&_placeBits, 7);
		_curScene = numToScene(_cowboyScenes[picked]);
	} else {
		genericNext();
	}
}

void GameMaddog2::sceneNxtscnInitRandomFarmyard(Scene *scene) {
	_placeBits = 0;
	_randomCount = 0;
	uint16 picked = pickBits(&_placeBits, 4);
	_curScene = numToScene(_farmyardScenes[picked]);
}

void GameMaddog2::sceneNxtscnToFarmyardScenes(Scene *scene) {
	_randomCount++;
	uint8 totalRandom = ((_difficulty - 1) * 2) + 5;
	if (_randomCount < totalRandom) {
		uint16 picked = pickBits(&_placeBits, 4);
		_curScene = numToScene(_farmyardScenes[picked]);
	} else {
		genericNext();
	}
}

void GameMaddog2::sceneNxtscnInitRandomCave(Scene *scene) {
	_placeBits = 0;
	_randomCount = 0;
	uint16 picked = pickBits(&_placeBits, 5);
	_curScene = numToScene(_caveScenes[picked]);
}

void GameMaddog2::sceneNxtscnToCaveScenes(Scene *scene) {
	_randomCount++;
	uint8 totalRandom = ((_difficulty - 1) * 2) + 8;
	if (_randomCount < totalRandom) {
		uint16 picked = pickBits(&_placeBits, 5);
		_curScene = numToScene(_caveScenes[picked]);
	} else {
		_gotTo = 0xEE;
		_curScene = numToScene(_gotTo);
	}
}

void GameMaddog2::sceneNxtscnPickSkullAtCave(Scene *scene) {
	switch (_rnd->getRandomNumber(2)) {
	case 0:
		_curScene = "scene239";
		break;
	case 1:
		_curScene = "scene240";
		break;
	case 2:
		_curScene = "scene242";
		break;
	}
}

// Script functions: WepDwn
void GameMaddog2::sceneDefaultWepdwn(Scene *scene) {
	_inHolster = 9;
	_whichGun = 7;
	updateMouse();
	if (!_inShootout) {
		if (_shots < 6) {
			_shots = 6;
		}
		updateStat();
	}
}

// Debug methods
void GameMaddog2::debugWarpTo(int val) {
	// TODO implement
}

// Debugger methods
DebuggerMaddog2::DebuggerMaddog2(GameMaddog2 *game) {
	_game = game;
	registerVar("drawRects", &game->_debug_drawRects);
	registerVar("godMode", &game->_debug_godMode);
	registerVar("unlimitedAmmo", &game->_debug_unlimitedAmmo);
	registerCmd("warpTo", WRAP_METHOD(DebuggerMaddog2, cmdWarpTo));
	registerCmd("dumpLib", WRAP_METHOD(DebuggerMaddog2, cmdDumpLib));
}

bool DebuggerMaddog2::cmdWarpTo(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: warp <int>");
		return true;
	} else {
		int val = atoi(argv[1]);
		_game->debugWarpTo(val);
		return false;
	}
}

bool DebuggerMaddog2::cmdDumpLib(int argc, const char **argv) {
	return _game->debug_dumpLibFile();
}

} // End of namespace Alg
