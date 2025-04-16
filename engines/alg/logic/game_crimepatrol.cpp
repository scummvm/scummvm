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
#include "alg/logic/game_crimepatrol.h"
#include "alg/scene.h"

namespace Alg {

GameCrimePatrol::GameCrimePatrol(AlgEngine *vm, const AlgGameDescription *gd) : Game(vm) {
}

GameCrimePatrol::~GameCrimePatrol() {
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
	if (_difficultyIcon) {
		_difficultyIcon->free();
		delete _difficultyIcon;
	}
	if (_bulletholeIcon) {
		_bulletholeIcon->free();
		delete _bulletholeIcon;
	}
}

void GameCrimePatrol::init() {
	Game::init();

	_videoPosX = 11;
	_videoPosY = 2;

	if (_vm->isDemo()) {
		loadLibArchive("cp.lib");
	} else if(_vm->useSingleSpeedVideos()) {
		loadLibArchive("cpss.lib");
	} else {
		loadLibArchive("cpds.lib");
	}

	_sceneInfo->loadScnFile("cp.scn");
	_startScene = _sceneInfo->getStartScene();

	registerScriptFunctions();
	verifyScriptFunctions();

	_menuzone = new Zone("MainMenu", "GLOBALHIT");
	_menuzone->addRect(0x0C, 0xAA, 0x38, 0xC7, nullptr, 0, "SHOTMENU", "0");

	_submenzone = new Zone("SubMenu", "GLOBALHIT");
	_submenzone->addRect(0x1C, 0x11, 0x5D, 0x20, nullptr, 0, "STARTMENU", "0");
	_submenzone->addRect(0x1C, 0x31, 0x5D, 0x40, nullptr, 0, "RECTLOAD", "0");
	_submenzone->addRect(0x1C, 0x51, 0x5D, 0x60, nullptr, 0, "RECTSAVE", "0");
	_submenzone->addRect(0x1C, 0x71, 0x5D, 0x80, nullptr, 0, "CONTMENU", "0");
	_submenzone->addRect(0x1C, 0x91, 0x5D, 0xA0, nullptr, 0, "EXITMENU", "0");
	_submenzone->addRect(0xDD, 0x3C, 0x010A, 0x4B, nullptr, 0, "RECTEASY", "0");
	_submenzone->addRect(0xDD, 0x5C, 0x010A, 0x6B, nullptr, 0, "RECTAVG", "0");
	_submenzone->addRect(0xDD, 0x7C, 0x010A, 0x8B, nullptr, 0, "RECTHARD", "0");

	_shotSound = loadSoundFile("blow.8b");
	_emptySound = loadSoundFile("empty.8b");
	_saveSound = loadSoundFile("saved.8b");
	_loadSound = loadSoundFile("loaded.8b");
	_skullSound = loadSoundFile("skull.8b");

	_gun = AlgGraphics::loadScreenCoordAniImage("gun.ani", _palette);
	_numbers = AlgGraphics::loadAniImage("numbers.ani", _palette);
	auto bullets = AlgGraphics::loadAniImage("bullets.ani", _palette);
	_shotIcon = (*bullets)[0];
	_emptyIcon = (*bullets)[1];
	auto lives = AlgGraphics::loadAniImage("lives.ani", _palette);
	_liveIcon = (*lives)[0];
	_deadIcon = (*lives)[1];
	auto difficlt = AlgGraphics::loadScreenCoordAniImage("difficlt.ani", _palette);
	_difficultyIcon = (*difficlt)[0];
	auto hole = AlgGraphics::loadScreenCoordAniImage("hole.ani", _palette);
	_bulletholeIcon = (*hole)[0];

	_background = AlgGraphics::loadVgaBackground("cp_menu.vga", _palette);
	_screen->copyRectToSurface(_background->getPixels(), _background->pitch, 0, 0, _background->w, _background->h);

	moveMouse();
}

void GameCrimePatrol::registerScriptFunctions() {
#define RECT_HIT_FUNCTION(name, func) _rectHitFuncs[name] = new CPScriptFunctionRect(this, &GameCrimePatrol::func);
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
	RECT_HIT_FUNCTION("TARGET_PRACTICE", rectTargetPractice);
	RECT_HIT_FUNCTION("SELECT_TARGET_PRACTICE", rectSelectTargetPractice);
	RECT_HIT_FUNCTION("SELECT_GANG_FIGHT", rectSelectGangFight);
	RECT_HIT_FUNCTION("SELECT_WAREHOUSE", rectSelectWarehouse);
	RECT_HIT_FUNCTION("SELECT_WESTCOAST_SOUND", rectSelectWestcoastSound);
	RECT_HIT_FUNCTION("SELECT_DRUG_DEAL", rectSelectDrugDeal);
	RECT_HIT_FUNCTION("SELECT_CAR_RING", rectSelectCarRing);
	RECT_HIT_FUNCTION("SELECT_BAR", rectSelectBar);
	RECT_HIT_FUNCTION("SELECT_BANK", rectSelectBank);
	RECT_HIT_FUNCTION("SELECT_CRACK_HOUSE", rectSelectCrackHouse);
	RECT_HIT_FUNCTION("SELECT_METH_LAB", rectSelectMethLab);
	RECT_HIT_FUNCTION("SELECT_AIRPLANE", rectSelectAirplane);
	RECT_HIT_FUNCTION("SELECT_NUKE_TRANSPORT", rectSelectNukeTransport);
	RECT_HIT_FUNCTION("SELECT_AIRPORT", rectSelectAirport);
	RECT_HIT_FUNCTION("KILL_INNOCENT_MAN", rectKillInnocentMan);
#undef RECT_HIT_FUNCTION

#define PRE_OPS_FUNCTION(name, func) _scenePreOps[name] = new CPScriptFunctionScene(this, &GameCrimePatrol::func);
	PRE_OPS_FUNCTION("DEFAULT", scenePsoDrawRct);
	PRE_OPS_FUNCTION("PAUSE", scenePsoPause);
	PRE_OPS_FUNCTION("FADEIN", scenePsoFadeIn);
	PRE_OPS_FUNCTION("PAUSE_FADEIN", scenePsoPauseFadeIn);
	PRE_OPS_FUNCTION("WAREHOUSE_GOT_TO", scenePsoWarehouseGotTo);
	PRE_OPS_FUNCTION("GANG_FIGHT_GOT_TO", scenePsoGangFightGotTo);
	PRE_OPS_FUNCTION("WESTCOAST_SOUND_GOT_TO", scenePsoWestcoastSoundGotTo);
	PRE_OPS_FUNCTION("DRUG_DEAL_GOT_TO", scenePsoDrugDealGotTo);
	PRE_OPS_FUNCTION("CAR_RING_GOT_TO", scenePsoCarRingGotTo);
	PRE_OPS_FUNCTION("BANK_GOT_TO", scenePsoBankGotTo);
	PRE_OPS_FUNCTION("CRACK_HOUSE_GOT_TO", scenePsoCrackHouseGotTo);
	PRE_OPS_FUNCTION("METH_LAB_GOT_TO", scenePsoMethLabGotTo);
	PRE_OPS_FUNCTION("AIRPLANE_GOT_TO", scenePsoAirplaneGotTo);
	PRE_OPS_FUNCTION("AIRPORT_GOT_TO", scenePsoAirportGotTo);
	PRE_OPS_FUNCTION("NUKE_TRANSPORT_GOT_TO", scenePsoNukeTransportGotTo);
	PRE_OPS_FUNCTION("POWER_PLANT_GOT_TO", scenePsoPowerPlantGotTo);
#undef PRE_OPS_FUNCTION

#define INS_OPS_FUNCTION(name, func) _sceneInsOps[name] = new CPScriptFunctionScene(this, &GameCrimePatrol::func);
	INS_OPS_FUNCTION("DEFAULT", sceneIsoDoNothing);
	INS_OPS_FUNCTION("PAUSE", sceneIsoPause);
#undef INS_OPS_FUNCTION

#define NXT_SCN_FUNCTION(name, func) _sceneNxtScn[name] = new CPScriptFunctionScene(this, &GameCrimePatrol::func);
	NXT_SCN_FUNCTION("DEFAULT", sceneDefaultNxtscn);
	NXT_SCN_FUNCTION("GAME_WON", sceneNxtscnGameWon);
	NXT_SCN_FUNCTION("LOSE_A_LIFE", sceneNxtscnLoseALife);
	NXT_SCN_FUNCTION("DID_NOT_CONTINUE", sceneNxtscnDidNotContinue);
	NXT_SCN_FUNCTION("KILL_INNOCENT_MAN", sceneNxtscnKillInnocentMan);
	NXT_SCN_FUNCTION("KILL_INNOCENT_WOMAN", sceneNxtscnKillInnocentWoman);
	NXT_SCN_FUNCTION("AFTER_DIE", sceneNxtscnAfterDie);
	NXT_SCN_FUNCTION("SELECT_LANGUAGE_1", sceneNxtscnSelectLanguage1);
	NXT_SCN_FUNCTION("SELECT_LANGUAGE_2", sceneNxtscnSelectLanguage2);
	NXT_SCN_FUNCTION("INIT_RANDOM_TARGET_PRACTICE", sceneNxtscnInitRandomTargetPractice);
	NXT_SCN_FUNCTION("CONTINUE_TARGET_PRACTICE", sceneNxtscnContinueTargetPractice);
	NXT_SCN_FUNCTION("SELECT_ROOKIE_SCENARIO", sceneNxtscnSelectRookieScenario);
	NXT_SCN_FUNCTION("FINISH_GANG_FIGHT", sceneNxtscnFinishGangFight);
	NXT_SCN_FUNCTION("FINISH_WESTCOAST_SOUND", sceneNxtscnFinishWestcoastSound);
	NXT_SCN_FUNCTION("FINISH_WAREHOUSE", sceneNxtscnFinishWarehouse);
	NXT_SCN_FUNCTION("INIT_RANDOM_WAREHOUSE", sceneNxtscnInitRandomWarehouse);
	NXT_SCN_FUNCTION("CONTINUE_WAREHOUSE", sceneNxtscnContinueWarehouse);
	NXT_SCN_FUNCTION("SELECT_UNDERCOVER_SCENARIO", sceneNxtscnSelectUndercoverScenario);
	NXT_SCN_FUNCTION("FINISH_DRUG_DEAL", sceneNxtscnFinishDrugDeal);
	NXT_SCN_FUNCTION("INIT_RANDOM_CAR_RING_LEADER", sceneNxtscnInitRandomCarRingLeader);
	NXT_SCN_FUNCTION("CONTINUE_CAR_RING_LEADER_1", sceneNxtscnContinueCarRingLeader1);
	NXT_SCN_FUNCTION("CONTINUE_CAR_RING_LEADER_2", sceneNxtscnContinueCarRingLeader2);
	NXT_SCN_FUNCTION("INIT_RANDOM_CAR_RING", sceneNxtscnInitRandomCarRing);
	NXT_SCN_FUNCTION("CONTINUE_CAR_RING", sceneNxtscnContinueCarRing);
	NXT_SCN_FUNCTION("FINISH_CAR_RING", sceneNxtscnFinishCarRing);
	NXT_SCN_FUNCTION("FINISH_BAR", sceneNxtscnFinishBar);
	NXT_SCN_FUNCTION("FINISH_BANK", sceneNxtscnFinishBank);
	NXT_SCN_FUNCTION("FINISH_CRACK_HOUSE", sceneNxtscnFinishCrackHouse);
	NXT_SCN_FUNCTION("FINISH_METH_LAB", sceneNxtscnFinishMethLab);
	NXT_SCN_FUNCTION("FINISH_AIRPLANE", sceneNxtscnFinishAirplane);
	NXT_SCN_FUNCTION("FINISH_AIRPORT", sceneNxtscnFinishAirport);
	NXT_SCN_FUNCTION("FINISH_NUKE_TRANSPORT", sceneNxtscnFinishNukeTransport);
	NXT_SCN_FUNCTION("INIT_RANDOM_BAR", sceneNxtscnInitRandomBar);
	NXT_SCN_FUNCTION("CONTINUE_BAR", sceneNxtscnContinueBar);
	NXT_SCN_FUNCTION("SELECT_SWAT_SCENARIO", sceneNxtscnSelectSwatScenario);
	NXT_SCN_FUNCTION("INIT_RANDOM_BANK", sceneNxtscnInitRandomBank);
	NXT_SCN_FUNCTION("CONTINUE_BANK", sceneNxtscnContinueBank);
	NXT_SCN_FUNCTION("INIT_RANDOM_METH_LAB", sceneNxtscnInitRandomMethLab);
	NXT_SCN_FUNCTION("CONTINUE_METH_LAB", sceneNxtscnContinueMethLab);
	NXT_SCN_FUNCTION("SELECT_DELTA_SCENARIO", sceneNxtscnSelectDeltaScenario);
	NXT_SCN_FUNCTION("PICK_RANDOM_RAPPELLER", sceneNxtscnPickRandomRapeller);
	NXT_SCN_FUNCTION("INIT_RANDOM_AIRPLANE", sceneNxtscnInitRandomAirplane);
	NXT_SCN_FUNCTION("CONTINUE_AIRPLANE", sceneNxtscnContinueAirplane);
	NXT_SCN_FUNCTION("PICK_RANDOM_AIRPLANE_FRONT", sceneNxtscnPickRandomAirplaneFront);
	NXT_SCN_FUNCTION("INIT_RANDOM_AIRPORT", sceneNxtscnInitRandomAirport);
	NXT_SCN_FUNCTION("CONTINUE_AIRPORT", sceneNxtscnContinueAirport);
	NXT_SCN_FUNCTION("INIT_RANDOM_NUKE_TRANSPORT", sceneNxtscnInitRandomNukeTransport);
	NXT_SCN_FUNCTION("CONTINUE_NUKE_TRANSPORT", sceneNxtscnContinueNukeTransport);
	NXT_SCN_FUNCTION("INIT_RANDOM_POWERPLANT", sceneNxtscnInitRandomPowerplant);
	NXT_SCN_FUNCTION("CONTINUE_POWERPLANT", sceneNxtscnContinuePowerplant);
#undef NXT_SCN_FUNCTION

	_sceneShowMsg["DEFAULT"] = new CPScriptFunctionScene(this, &GameCrimePatrol::sceneSmDonothing);
	_sceneWepDwn["DEFAULT"] = new CPScriptFunctionScene(this, &GameCrimePatrol::sceneDefaultWepdwn);
	_sceneScnScr["DEFAULT"] = new CPScriptFunctionScene(this, &GameCrimePatrol::sceneDefaultScore);
	_sceneNxtFrm["DEFAULT"] = new CPScriptFunctionScene(this, &GameCrimePatrol::sceneNxtfrm);
}

void GameCrimePatrol::verifyScriptFunctions() {
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

CPScriptFunctionRect GameCrimePatrol::getScriptFunctionRectHit(Common::String name) {
	auto it = _rectHitFuncs.find(name);
	if (it != _rectHitFuncs.end()) {
		return *it->_value;
	} else {
		error("GameCrimePatrol::getScriptFunctionRectHit(): Could not find rectHit function: %s", name.c_str());
	}
}

CPScriptFunctionScene GameCrimePatrol::getScriptFunctionScene(SceneFuncType type, Common::String name) {
	CPScriptFunctionSceneMap *functionMap;
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
		error("GameCrimePatrol::getScriptFunctionScene(): Unkown scene script type: %u", type);
		break;
	}
	CPScriptFunctionSceneMap::iterator it;
	it = functionMap->find(name);
	if (it != functionMap->end()) {
		return *it->_value;
	} else {
		error("GameCrimePatrol::getScriptFunctionScene(): Could not find scene type %u function: %s", type, name.c_str());
	}
}

void GameCrimePatrol::callScriptFunctionRectHit(Common::String name, Rect *rect) {
	CPScriptFunctionRect function = getScriptFunctionRectHit(name);
	function(rect);
}

void GameCrimePatrol::callScriptFunctionScene(SceneFuncType type, Common::String name, Scene *scene) {
	CPScriptFunctionScene function = getScriptFunctionScene(type, name);
	function(scene);
}

Common::Error GameCrimePatrol::run() {
	init();
	newGame();
	_curScene = _startScene;
	Common::String oldscene;
	while (!_vm->shouldQuit()) {
		oldscene = _curScene;
		_fired = false;
		Scene *scene = _sceneInfo->findScene(_curScene);
		if (!loadScene(scene)) {
			if (scene->_nxtscn == "CONTINUE_TARGET_PRACTICE") {
				callScriptFunctionScene(NXTSCN, scene->_nxtscn, scene);
			} else {
				error("GameCrimePatrol::run(): Cannot find scene %s in libfile", scene->_name.c_str());
			}
		}
		_sceneSkipped = false;
		_paletteDirty = true;
		_nextFrameTime = getMsTime() + 100;
		callScriptFunctionScene(PREOP, scene->_preop, scene);
		_currentFrame = getFrame(scene);
		while (_currentFrame <= scene->_endFrame && _curScene == oldscene && !_vm->shouldQuit()) {
			updateMouse();
			callScriptFunctionScene(SHOWMSG, scene->_scnmsg, scene);
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
						displayShotFiredImage(&firedCoords);
						doShot();
						Rect *hitRect = nullptr;
						Zone *hitSceneZone = checkZonesV2(scene, hitRect, &firedCoords);
						if (hitSceneZone != nullptr) {
							callScriptFunctionRectHit(hitRect->_rectHit, hitRect);
						} else {
							int8 skip = skipToNewScene(scene);
							if (skip == -1) {
								callScriptFunctionScene(NXTSCN, scene->_nxtscn, scene);
							} else if (skip == 1) {
								if (scene->_dataParam4 > 0) {
									uint32 framesToSkip = (scene->_dataParam4 - _currentFrame) / _videoFrameSkip;
									_videoDecoder->skipNumberOfFrames(framesToSkip);
								} else {
									callScriptFunctionScene(NXTSCN, scene->_nxtscn, scene);
								}
							}
						}
					} else {
						playSound(_emptySound);
					}
				}
			}
			if (_curScene == oldscene) {
				callScriptFunctionScene(NXTFRM, scene->_nxtfrm, scene);
			}
			displayLivesLeft();
			displayScores();
			displayShotsLeft();
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
			debugDrawPracticeRects();
		}
		// frame limit reached or scene changed, prepare for next scene
		_hadPause = false;
		_pauseTime = 0;
		if (_curScene == oldscene) {
			callScriptFunctionScene(NXTSCN, scene->_nxtscn, scene);
		}
		if (_curScene == "") {
			shutdown();
		}
	}
	return Common::kNoError;
}

void GameCrimePatrol::newGame() {
	_shots = 10;
	_lives = 3;
	_holster = false;
}

void GameCrimePatrol::resetParams() {
	// fill _gotTo with start scenes
	// 0 in _gotTo array means the level is finished
	for (int i = 0; i < 15; i++) {
		_gotTo[i] = _levelScenes[i][0];
	}
}

void GameCrimePatrol::doMenu() {
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

void GameCrimePatrol::changeDifficulty(uint8 newDifficulty) {
	if (newDifficulty == _oldDifficulty) {
		return;
	}
	showDifficulty(newDifficulty, true);
	_oldDifficulty = newDifficulty;
	_difficulty = newDifficulty;
}

void GameCrimePatrol::showDifficulty(uint8 newDifficulty, bool cursor) {
	// reset menu screen
	_screen->copyRectToSurface(_background->getBasePtr(_videoPosX, _videoPosY), _background->pitch, _videoPosX, _videoPosY, _videoDecoder->getWidth(), _videoDecoder->getHeight());
	uint16 posY = 0x45 + ((newDifficulty - 1) * 0x21);
	AlgGraphics::drawImageCentered(_screen, _difficultyIcon, 0x0115, posY);
	if (cursor) {
		updateCursor();
	}
}

void GameCrimePatrol::updateCursor() {
	updateMouse();
}

void GameCrimePatrol::updateMouse() {
	if (_oldWhichGun != _whichGun) {
		Graphics::Surface *cursor = (*_gun)[_whichGun];
		uint16 hotspotX = (cursor->w / 2) + 3;
		uint16 hotspotY = (cursor->h / 2) + 3;
		if (debugChannelSet(1, Alg::kAlgDebugGraphics)) {
			cursor->drawLine(0, hotspotY, cursor->w, hotspotY, 1);
			cursor->drawLine(hotspotX, 0, hotspotX, cursor->h, 1);
		}
		CursorMan.replaceCursor(cursor->getPixels(), cursor->w, cursor->h, hotspotX, hotspotY, 0);
		CursorMan.showMouse(true);
		_oldWhichGun = _whichGun;
	}
}

void GameCrimePatrol::moveMouse() {
	if (_inMenu) {
		_whichGun = 3; // in menu cursor
	} else {
		// TODO: disabled for now, because glitchy
		/*
		uint16 x = _mousePos.x;
		uint16 y = _mousePos.y;
		if (x < 13) {x = 13;}
		if (x > 286) {x = 286;}
		if (y < 3) {y = 3;}
		if (y > 166) {y = 166;}
		if (_mousePos.x != x || _mousePos.y != y) {
			_mousePos.x = x;
			_mousePos.y = y;
			g_system->warpMouse(x, y);
		}
		*/
		if (_mousePos.y >= 0xA3 && _mousePos.x >= 0xF0) {
			_whichGun = 1; // holster
		} else if (_mousePos.y >= 0xA3 && _mousePos.x <= 0x43) {
			_whichGun = 2; // menu button cursor
		} else {
			_whichGun = 0; // regular gun
		}
	}
	updateMouse();
}

void GameCrimePatrol::displayLivesLeft() {
	if (_lives == _oldLives) {
		return;
	}
	int posY = 0x67;
	for (int i = 0; i < 3; i++) {
		AlgGraphics::drawImage(_screen, _deadIcon, 0x12F, posY);
		posY += 0xE;
	}
	posY = 0x67;
	for (int i = 0; i < _lives; i++) {
		AlgGraphics::drawImage(_screen, _liveIcon, 0x12F, posY);
		posY += 0xE;
	}
	_oldLives = _lives;
}

void GameCrimePatrol::displayScores() {
	if (_score == _oldScore) {
		return;
	}
	Common::String scoreString = Common::String::format("%05d", _score);
	int posX = 0x9B;
	for (int i = 0; i < 5; i++) {
		uint8 digit = scoreString[i] - '0';
		AlgGraphics::drawImage(_screen, (*_numbers)[digit], posX, 0xBF);
		posX += 7;
	}
	_oldScore = _score;
}

void GameCrimePatrol::displayShotsLeft() {
	if (_shots == _oldShots) {
		return;
	}
	uint16 posX = 0xEE;
	for (int i = 0; i < 10; i++) {
		AlgGraphics::drawImage(_screen, _emptyIcon, posX, 0xBE);
		posX += 5;
	}
	posX = 0xEE;
	for (int i = 0; i < _shots; i++) {
		AlgGraphics::drawImage(_screen, _shotIcon, posX, 0xBE);
		posX += 5;
	}
	_oldShots = _shots;
}

bool GameCrimePatrol::weaponDown() {
	if (_rightDown && _mousePos.y >= 0xAA && _mousePos.x >= 0x113) {
		return true;
	}
	return false;
}

bool GameCrimePatrol::saveState() {
	Common::OutSaveFile *outSaveFile;
	Common::String saveFileName = _vm->getSaveStateName(0);
	if (!(outSaveFile = g_system->getSavefileManager()->openForSaving(saveFileName))) {
		warning("GameCrimePatrol::saveState(): Can't create file '%s', game not saved", saveFileName.c_str());
		return false;
	}
	outSaveFile->writeUint32BE(MKTAG('A', 'L', 'G', 'S')); // header
	outSaveFile->writeByte(0);                             // version, unused for now
	outSaveFile->writeSByte(_stage);
	for (int i = 0; i < 15; i++) {
		outSaveFile->writeUint16LE(_gotTo[i]);
	}
	outSaveFile->writeSint32LE(_score);
	outSaveFile->writeUint16LE(_shots);
	outSaveFile->writeSByte(_lives);
	outSaveFile->finalize();
	delete outSaveFile;
	return true;
}

bool GameCrimePatrol::loadState() {
	Common::InSaveFile *inSaveFile;
	Common::String saveFileName = _vm->getSaveStateName(0);
	if (!(inSaveFile = g_system->getSavefileManager()->openForLoading(saveFileName))) {
		debug("GameCrimePatrol::loadState(): Can't load file '%s', game not loaded", saveFileName.c_str());
		return false;
	}
	uint32 header = inSaveFile->readUint32BE();
	if (header != MKTAG('A', 'L', 'G', 'S')) {
		warning("GameCrimePatrol::loadState(): Unkown save file, header: %s", tag2str(header));
		return false;
	}
	inSaveFile->skip(1); // version, unused for now
	_stage = inSaveFile->readSByte();
	for (int i = 0; i < 15; i++) {
		_gotTo[i] = inSaveFile->readUint16LE();
	}
	_score = inSaveFile->readSint32LE();
	_shots = inSaveFile->readUint16LE();
	_lives = inSaveFile->readSByte();
	delete inSaveFile;
	_gameInProgress = true;
	sceneNxtscnGeneric(_stage);
	return true;
}

// misc game functions
void GameCrimePatrol::displayShotFiredImage(Common::Point *point) {
	if (point->x >= _videoPosX && point->x <= (_videoPosX + _videoDecoder->getWidth()) && point->y >= _videoPosY && point->y <= (_videoPosY + _videoDecoder->getHeight())) {
		uint16 targetX = point->x - _videoPosX;
		uint16 targetY = point->y - _videoPosY;
		AlgGraphics::drawImageCentered(_videoDecoder->getVideoFrame(), _bulletholeIcon, targetX, targetY);
	}
}

void GameCrimePatrol::enableVideoFadeIn() {
	// TODO implement
}

uint16 GameCrimePatrol::sceneToNumber(Scene *scene) {
	return atoi(scene->_name.substr(5).c_str());
}

uint16 GameCrimePatrol::pickRandomScene(uint8 index, uint8 max) {
	if (max != 0) {
		_randomMax = max;
		_randomMask = 0;
		_randomPicked = 0;
		_randomSceneCount = 0;
		while (_levelScenes[index][_randomSceneCount] != 0) {
			_randomSceneCount++;
		}
	}
	uint16 count = _randomMax--;
	if (count > 0) {
		_randomPicked = randomUnusedInt(_randomSceneCount, &_randomMask, _randomPicked);
		return _levelScenes[index][_randomPicked];
	}
	return 0;
}

uint16 GameCrimePatrol::pickDeathScene() {
	if (_stage != _oldStage) {
		_oldStage = _stage;
		_deathMask = 0;
		_deathPicked = -1;
		_deathSceneCount = 0;
		while (_diedScenesByStage[_stage][_deathSceneCount] != 0) {
			_deathSceneCount++;
		}
	}
	_deathPicked = randomUnusedInt(_deathSceneCount, &_deathMask, _deathPicked);
	return _diedScenesByStage[_stage][_deathPicked];
}

void GameCrimePatrol::sceneNxtscnGeneric(uint8 index) {
	uint16 nextSceneId = 0;
	_gotTo[index] = 0;
	if (_gotTo[0] || _gotTo[1] || _gotTo[3] || _gotTo[2]) {
		nextSceneId = _stageStartScenes[0];
	} else if (_gotTo[4] || _gotTo[5] || _gotTo[6]) {
		if (_stage == 1) {
			nextSceneId = _stageStartScenes[1];
		} else {
			_stage = 1;
			nextSceneId = 0x50;
		}
	} else if (_gotTo[7] || _gotTo[8] || _gotTo[9]) {
		if (_stage == 2) {
			nextSceneId = _stageStartScenes[2];
		} else {
			_stage = 2;
			nextSceneId = 0x81;
		}
	} else if (_gotTo[10] || _gotTo[11] || _gotTo[12]) {
		if (_stage == 3) {
			nextSceneId = _stageStartScenes[3];
		} else {
			_stage = 3;
			nextSceneId = 0x014B;
		}
	} else if (_gotTo[13]) {
		_stage = 4;
		nextSceneId = 0x018F;
	} else {
		nextSceneId = 0x21;
	}
	_curScene = Common::String::format("scene%d", nextSceneId);
}

void GameCrimePatrol::rectSelectGeneric(uint8 index) {
	if (_gotTo[index] > 0) {
		_curScene = Common::String::format("scene%d", _gotTo[index]);
	}
}

void GameCrimePatrol::sceneIsoGotToGeneric(uint8 index, uint16 sceneId) {
	_gotTo[index] = sceneId;
}

// Script functions: RectHit
void GameCrimePatrol::rectShotMenu(Rect *rect) {
	doMenu();
}

void GameCrimePatrol::rectSave(Rect *rect) {
	if (saveState()) {
		doSaveSound();
	}
}

void GameCrimePatrol::rectLoad(Rect *rect) {
	if (loadState()) {
		doLoadSound();
	}
}

void GameCrimePatrol::rectContinue(Rect *rect) {
	_inMenu = false;
	_fired = false;
	if (_lives <= 0) {
		_score = (int32)(_score * 0.7f);
		uint16 returnScene = _stageStartScenes[_stage];
		_curScene = Common::String::format("scene%d", returnScene);
		newGame();
	}
}

void GameCrimePatrol::rectStart(Rect *rect) {
	_inMenu = false;
	_fired = false;
	_gameInProgress = true;
	if (_vm->isDemo()) {
		_curScene = "scene39";
		_gotTo[1] = 39;
	} else {
		_curScene = Common::String::format("scene%d", _stageStartScenes[0]);
	}
	resetParams();
	newGame();
}

void GameCrimePatrol::rectTargetPractice(Rect *rect) {
	uint16 nextScene = 0;
	Scene *scene = _sceneInfo->findScene(_curScene);
	if (_levelScenes[0][0] == sceneToNumber(scene)) {
		_practiceMask = 0x1F;
	}
	if (_practiceMask == 0) {
		_practiceMask = 0x1F;
	}
	for (int i = 0; i < 5; i++) {
		if (_mousePos.x <= _practiceTargetLeft[i] || _mousePos.x >= _practiceTargetRight[i] ||
			_mousePos.y <= _practiceTargetTop[i] || _mousePos.y >= _practiceTargetBottom[i]) {
			// did not hit target
			continue;
		}
		uint8 mask = 1 << i;
		if (!(_practiceMask & mask)) {
			// target was already hit before
			continue;
		}
		// did hit target
		_score += scene->_scnscrParam == 0 ? 50 : scene->_scnscrParam;
		_practiceMask ^= mask;
		uint8 inverted = _practiceMask ^ 0x1F;
		if (_practiceMask == 0) {
			nextScene = 432;
		} else {
			nextScene = 401 + inverted;
		}
		break;
	}
	if (nextScene != 0) {
		_curScene = Common::String::format("scene%d", nextScene);
	}
}

void GameCrimePatrol::rectSelectTargetPractice(Rect *rect) {
	rectSelectGeneric(0);
	_gotTo[0] = 0;
}

void GameCrimePatrol::rectSelectGangFight(Rect *rect) {
	_gotTo[0] = 0;
	rectSelectGeneric(1);
}

void GameCrimePatrol::rectSelectWarehouse(Rect *rect) {
	_gotTo[0] = 0;
	rectSelectGeneric(2);
}

void GameCrimePatrol::rectSelectWestcoastSound(Rect *rect) {
	_gotTo[0] = 0;
	rectSelectGeneric(3);
}

void GameCrimePatrol::rectSelectDrugDeal(Rect *rect) {
	rectSelectGeneric(4);
}
void GameCrimePatrol::rectSelectCarRing(Rect *rect) {
	rectSelectGeneric(5);
}
void GameCrimePatrol::rectSelectBar(Rect *rect) {
	rectSelectGeneric(6);
}
void GameCrimePatrol::rectSelectBank(Rect *rect) {
	rectSelectGeneric(7);
}
void GameCrimePatrol::rectSelectCrackHouse(Rect *rect) {
	rectSelectGeneric(9);
}

void GameCrimePatrol::rectSelectMethLab(Rect *rect) {
	rectSelectGeneric(8);
}

void GameCrimePatrol::rectSelectAirplane(Rect *rect) {
	rectSelectGeneric(10);
}

void GameCrimePatrol::rectSelectNukeTransport(Rect *rect) {
	rectSelectGeneric(11);
}

void GameCrimePatrol::rectSelectAirport(Rect *rect) {
	rectSelectGeneric(12);
}

void GameCrimePatrol::rectKillInnocentMan(Rect *rect) {
}

// Script functions: Scene PreOps
void GameCrimePatrol::scenePsoWarehouseGotTo(Scene *scene) {
	uint16 sceneId = sceneToNumber(scene);
	sceneIsoGotToGeneric(2, sceneId);
	enableVideoFadeIn();
}

void GameCrimePatrol::scenePsoGangFightGotTo(Scene *scene) {
	uint16 sceneId = sceneToNumber(scene);
	sceneIsoGotToGeneric(1, sceneId);
	enableVideoFadeIn();
}

void GameCrimePatrol::scenePsoWestcoastSoundGotTo(Scene *scene) {
	sceneIsoGotToGeneric(3, 456);
	enableVideoFadeIn();
}

void GameCrimePatrol::scenePsoDrugDealGotTo(Scene *scene) {
	uint16 sceneId = sceneToNumber(scene);
	sceneIsoGotToGeneric(4, sceneId);
	enableVideoFadeIn();
}

void GameCrimePatrol::scenePsoCarRingGotTo(Scene *scene) {
	uint16 sceneId = sceneToNumber(scene);
	sceneIsoGotToGeneric(5, sceneId);
	enableVideoFadeIn();
}

void GameCrimePatrol::scenePsoBankGotTo(Scene *scene) {
	uint16 sceneId = sceneToNumber(scene);
	sceneIsoGotToGeneric(7, sceneId);
	enableVideoFadeIn();
}

void GameCrimePatrol::scenePsoCrackHouseGotTo(Scene *scene) {
	uint16 sceneId = sceneToNumber(scene);
	sceneIsoGotToGeneric(9, sceneId);
	enableVideoFadeIn();
}

void GameCrimePatrol::scenePsoMethLabGotTo(Scene *scene) {
	uint16 sceneId = sceneToNumber(scene);
	sceneIsoGotToGeneric(8, sceneId);
	enableVideoFadeIn();
}

void GameCrimePatrol::scenePsoAirplaneGotTo(Scene *scene) {
	uint16 sceneId = sceneToNumber(scene);
	sceneIsoGotToGeneric(10, sceneId);
	enableVideoFadeIn();
}

void GameCrimePatrol::scenePsoAirportGotTo(Scene *scene) {
	uint16 sceneId = sceneToNumber(scene);
	sceneIsoGotToGeneric(12, sceneId);
	enableVideoFadeIn();
}

void GameCrimePatrol::scenePsoNukeTransportGotTo(Scene *scene) {
	uint16 sceneId = sceneToNumber(scene);
	sceneIsoGotToGeneric(11, sceneId);
	enableVideoFadeIn();
}

void GameCrimePatrol::scenePsoPowerPlantGotTo(Scene *scene) {
	uint16 sceneId = sceneToNumber(scene);
	sceneIsoGotToGeneric(13, sceneId);
	_finalStageScene = sceneId;
	enableVideoFadeIn();
}

// Script functions: Scene NxtScn
void GameCrimePatrol::sceneNxtscnLoseALife(Scene *scene) {
	uint16 picked = 0;
	if (!_debug_godMode) {
		_lives--;
	}
	if (_vm->isDemo()) {
		_curScene = "scene39";
		return;
	} else if (_lives > 0) {
		displayLivesLeft();
		picked = pickDeathScene();
	} else {
		picked = _deadScenes[_stage];
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::sceneNxtscnGameWon(Scene *scene) {
	_gameInProgress = false;
	_curScene = _startScene;
}

void GameCrimePatrol::sceneNxtscnDidNotContinue(Scene *scene) {
	_gameInProgress = false;
	_curScene = _startScene;
}

void GameCrimePatrol::sceneNxtscnKillInnocentMan(Scene *scene) {
	uint16 picked = 0;
	if (!_debug_godMode) {
		_lives--;
	}
	if (_lives > 0) {
		picked = _stageStartScenes[_stage];
	} else {
		picked = _deadScenes[_stage];
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::sceneNxtscnKillInnocentWoman(Scene *scene) {
	uint16 picked = 0;
	if (!_debug_godMode) {
		_lives--;
	}
	if (_lives > 0) {
		picked = _stageStartScenes[_stage];
	} else {
		picked = _deadScenes[_stage];
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::sceneNxtscnAfterDie(Scene *scene) {
	uint16 picked = _stageStartScenes[_stage];
	_curScene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::sceneNxtscnSelectLanguage1(Scene *scene) {
	// do nothing
}

void GameCrimePatrol::sceneNxtscnSelectLanguage2(Scene *scene) {
	// do nothing
}

void GameCrimePatrol::sceneNxtscnInitRandomTargetPractice(Scene *scene) {
	uint16 picked = pickRandomScene(14, 6);
	_curScene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::sceneNxtscnContinueTargetPractice(Scene *scene) {
	uint16 picked = pickRandomScene(14, 0);
	if (picked == 0) {
		sceneIsoGotToGeneric(0, 1);
		sceneNxtscnGeneric(0);
	} else {
		_curScene = Common::String::format("scene%d", picked);
	}
}

void GameCrimePatrol::sceneNxtscnSelectRookieScenario(Scene *scene) {
	uint16 picked = 0;
	if (_gotTo[0] > 0) {
		picked = _gotTo[0];
		_gotTo[0] = 0;
	} else if (_gotTo[3] > 0) {
		picked = _gotTo[3];
	} else if (_gotTo[1] > 0) {
		picked = _gotTo[1];
	} else if (_gotTo[2] > 0) {
		picked = _gotTo[2];
	} else {
		picked = _stageStartScenes[1];
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::sceneNxtscnSelectUndercoverScenario(Scene *scene) {
	uint16 picked = 0;
	if (_gotTo[4] > 0) {
		picked = _gotTo[4];
	} else if (_gotTo[5] > 0) {
		picked = _gotTo[5];
	} else if (_gotTo[6] > 0) {
		picked = _gotTo[6];
	} else {
		picked = _stageStartScenes[2];
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::sceneNxtscnSelectSwatScenario(Scene *scene) {
	uint16 picked = 0;
	if (_gotTo[8] > 0) {
		picked = _gotTo[8];
	} else if (_gotTo[7] > 0) {
		picked = _gotTo[7];
	} else if (_gotTo[9] > 0) {
		picked = _gotTo[9];
	} else {
		picked = _stageStartScenes[3];
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::sceneNxtscnSelectDeltaScenario(Scene *scene) {
	uint16 picked = 0;
	if (_gotTo[10] > 0) {
		picked = _gotTo[10];
	} else if (_gotTo[11] > 0) {
		picked = _gotTo[11];
	} else if (_gotTo[12] > 0) {
		picked = _gotTo[12];
	} else {
		picked = _finalStageScene;
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::sceneNxtscnFinishGangFight(Scene *scene) {
	if (_vm->isDemo()) {
		_curScene = _startScene;
		return;
	}
	sceneNxtscnGeneric(1);
}

void GameCrimePatrol::sceneNxtscnFinishWestcoastSound(Scene *scene) {
	sceneNxtscnGeneric(3);
}

void GameCrimePatrol::sceneNxtscnFinishWarehouse(Scene *scene) {
	sceneNxtscnGeneric(2);
}

void GameCrimePatrol::sceneNxtscnInitRandomWarehouse(Scene *scene) {
	uint16 picked = pickRandomScene(15, (_difficulty * 2) + 5);
	_curScene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::sceneNxtscnContinueWarehouse(Scene *scene) {
	uint16 picked = pickRandomScene(15, 0);
	if (picked == 0) {
		picked = 0x43;
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::sceneNxtscnFinishDrugDeal(Scene *scene) {
	sceneNxtscnGeneric(4);
}

void GameCrimePatrol::sceneNxtscnInitRandomCarRingLeader(Scene *scene) {
	uint16 picked = pickRandomScene(16, 2);
	_curScene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::sceneNxtscnContinueCarRingLeader1(Scene *scene) {
	uint16 picked = pickRandomScene(16, 0);
	if (picked == 0) {
		picked = 0x67;
	} else {
		picked = 0x63;
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::sceneNxtscnContinueCarRingLeader2(Scene *scene) {
	uint16 picked = pickRandomScene(16, 0);
	if (picked == 0) {
		picked = 0x67;
	} else {
		picked = 0x66;
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::sceneNxtscnInitRandomCarRing(Scene *scene) {
	uint16 picked = pickRandomScene(17, (_difficulty * 2) + 8);
	_curScene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::sceneNxtscnContinueCarRing(Scene *scene) {
	uint16 picked = pickRandomScene(17, 0);
	if (picked == 0) {
		picked = 0x74;
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::sceneNxtscnFinishCarRing(Scene *scene) {
	sceneNxtscnGeneric(5);
}

void GameCrimePatrol::sceneNxtscnFinishBar(Scene *scene) {
	sceneNxtscnGeneric(6);
}

void GameCrimePatrol::sceneNxtscnFinishBank(Scene *scene) {
	sceneNxtscnGeneric(7);
}

void GameCrimePatrol::sceneNxtscnFinishCrackHouse(Scene *scene) {
	sceneNxtscnGeneric(9);
}

void GameCrimePatrol::sceneNxtscnFinishMethLab(Scene *scene) {
	sceneNxtscnGeneric(8);
}

void GameCrimePatrol::sceneNxtscnFinishAirplane(Scene *scene) {
	sceneNxtscnGeneric(10);
}

void GameCrimePatrol::sceneNxtscnFinishAirport(Scene *scene) {
	sceneNxtscnGeneric(12);
}

void GameCrimePatrol::sceneNxtscnFinishNukeTransport(Scene *scene) {
	sceneNxtscnGeneric(11);
}

void GameCrimePatrol::sceneNxtscnInitRandomBar(Scene *scene) {
	uint16 picked = pickRandomScene(18, (_difficulty * 2) + 9);
	_curScene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::sceneNxtscnContinueBar(Scene *scene) {
	uint16 picked = pickRandomScene(18, 0);
	if (picked == 0) {
		picked = 0x92;
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::sceneNxtscnInitRandomBank(Scene *scene) {
	uint16 picked = pickRandomScene(19, (_difficulty * 2) + 8);
	_curScene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::sceneNxtscnContinueBank(Scene *scene) {
	uint16 picked = pickRandomScene(19, 0);
	if (picked == 0) {
		picked = 0xA8;
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::sceneNxtscnInitRandomMethLab(Scene *scene) {
	uint16 picked = pickRandomScene(20, (_difficulty * 2) + 8);
	_curScene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::sceneNxtscnContinueMethLab(Scene *scene) {
	uint16 picked = pickRandomScene(20, 0);
	if (picked == 0) {
		picked = 0xD0;
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::sceneNxtscnPickRandomRapeller(Scene *scene) {
	uint16 picked = pickRandomScene(21, 1);
	_curScene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::sceneNxtscnInitRandomAirplane(Scene *scene) {
	uint16 picked = pickRandomScene(22, (_difficulty * 2) + 8);
	_curScene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::sceneNxtscnContinueAirplane(Scene *scene) {
	uint16 picked = pickRandomScene(22, 0);
	if (picked == 0) {
		picked = 0x108;
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::sceneNxtscnPickRandomAirplaneFront(Scene *scene) {
	uint16 picked = pickRandomScene(23, 1);
	_curScene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::sceneNxtscnInitRandomAirport(Scene *scene) {
	uint16 picked = pickRandomScene(24, (_difficulty * 2) + 8);
	_curScene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::sceneNxtscnContinueAirport(Scene *scene) {
	uint16 picked = pickRandomScene(24, 0);
	if (picked == 0) {
		picked = 0x12D;
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::sceneNxtscnInitRandomNukeTransport(Scene *scene) {
	uint16 picked = pickRandomScene(25, (_difficulty * 2) + 8);
	_curScene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::sceneNxtscnContinueNukeTransport(Scene *scene) {
	uint16 picked = pickRandomScene(25, 0);
	if (picked == 0) {
		picked = 0x147;
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::sceneNxtscnInitRandomPowerplant(Scene *scene) {
	uint16 picked = pickRandomScene(26, (_difficulty * 2) + 8);
	_curScene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::sceneNxtscnContinuePowerplant(Scene *scene) {
	uint16 picked = pickRandomScene(26, 0);
	if (picked == 0) {
		picked = 0x169;
	}
	_curScene = Common::String::format("scene%d", picked);
}

// Script functions: WepDwn
void GameCrimePatrol::sceneDefaultWepdwn(Scene *scene) {
	_shots = 10;
}

// Debug methods
void GameCrimePatrol::debugWarpTo(int val) {
	// TODO implement
}

void GameCrimePatrol::debugDrawPracticeRects() {
	if (_debug_drawRects || debugChannelSet(1, Alg::kAlgDebugGraphics)) {
		Scene *scene = _sceneInfo->findScene(_curScene);
		if (scene->_zones.size() > 0) {
			if (scene->_zones[0]->_name == "zone283") {
				for (int i = 0; i < 5; i++) {
					uint16 left = _practiceTargetLeft[i] - _videoPosX;
					uint16 right = _practiceTargetRight[i] - _videoPosX;
					uint16 top = _practiceTargetTop[i] - _videoPosY;
					uint16 bottom = _practiceTargetBottom[i] - _videoPosY;
					_videoDecoder->getVideoFrame()->drawLine(left, top, right, top, 1);
					_videoDecoder->getVideoFrame()->drawLine(left, top, left, bottom, 1);
					_videoDecoder->getVideoFrame()->drawLine(right, bottom, right, top, 1);
					_videoDecoder->getVideoFrame()->drawLine(right, bottom, left, bottom, 1);
				}
			}
		}
	}
}

// Debugger methods
DebuggerCrimePatrol::DebuggerCrimePatrol(GameCrimePatrol *game) {
	_game = game;
	registerVar("drawRects", &game->_debug_drawRects);
	registerVar("godMode", &game->_debug_godMode);
	registerVar("unlimitedAmmo", &game->_debug_unlimitedAmmo);
	registerCmd("warpTo", WRAP_METHOD(DebuggerCrimePatrol, cmdWarpTo));
	registerCmd("dumpLib", WRAP_METHOD(DebuggerCrimePatrol, cmdDumpLib));
}

bool DebuggerCrimePatrol::cmdWarpTo(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: warp <int>");
		return true;
	} else {
		int val = atoi(argv[1]);
		_game->debugWarpTo(val);
		return false;
	}
}

bool DebuggerCrimePatrol::cmdDumpLib(int argc, const char **argv) {
	return _game->debug_dumpLibFile();
}

} // End of namespace Alg
