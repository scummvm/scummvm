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
#include "alg/logic/game_bountyhunter.h"
#include "alg/scene.h"

namespace Alg {

GameBountyHunter::GameBountyHunter(AlgEngine *vm, const AlgGameDescription *gd) : Game(vm) {
}

GameBountyHunter::~GameBountyHunter() {
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
	if (_diffIcon) {
		_diffIcon->free();
		delete _diffIcon;
	}
	if (_bulletholeIcon) {
		_bulletholeIcon->free();
		delete _bulletholeIcon;
	}
	if (_playersIcon1) {
		_playersIcon1->free();
		delete _playersIcon1;
	}
	if (_playersIcon2) {
		_playersIcon2->free();
		delete _playersIcon2;
	}
	if (_textScoreIcon) {
		_textScoreIcon->free();
		delete _textScoreIcon;
	}
	if (_textMenuIcon) {
		_textMenuIcon->free();
		delete _textMenuIcon;
	}
	if (_textBlackBarIcon) {
		_textBlackBarIcon->free();
		delete _textBlackBarIcon;
	}
	for (auto item : *_bagsIcons) {
		item->free();
		delete item;
	}
	for (auto item : *_shotgun) {
		item->free();
		delete item;
	}
	delete _shotgunSound;
}

void GameBountyHunter::init() {
	Game::init();

	_videoPosX = 0;
	_videoPosY = 0;

	if (_vm->isDemo()) {
		loadLibArchive("bhdemo.lib");
	} else {
		loadLibArchive("bhds.lib");
	}

	_sceneInfo->loadScnFile("bh.scn");
	_startScene = _sceneInfo->getStartScene();

	registerScriptFunctions();
	verifyScriptFunctions();

	_menuzone = new Zone("MainMenu", "GLOBALHIT");
	_menuzone->addRect(0x0C, 0xAA, 0x38, 0xC7, nullptr, 0, "SHOTMENU", "0");

	_submenzone = new Zone("SubMenu", "GLOBALHIT");
	_submenzone->addRect(0, 0, 0x78, 0x3C, nullptr, 0, "STARTMENU", "0");
	_submenzone->addRect(0xC8, 0, 0x0140, 0x3C, nullptr, 0, "RECTLOAD", "0");
	_submenzone->addRect(0xC8, 0x3C, 0x0140, 0x78, nullptr, 0, "RECTSAVE", "0");
	_submenzone->addRect(0, 0x3C, 0x78, 0x78, nullptr, 0, "CONTMENU", "0");
	_submenzone->addRect(0, 0x78, 0x78, 0xB4, nullptr, 0, "EXITMENU", "0");
	_submenzone->addRect(0xC8, 0x78, 0x0140, 0xB4, nullptr, 0, "TOGGLEPLAYERS", "0");

	_shotSound = loadSoundFile("blow.8b");
	_emptySound = loadSoundFile("empty.8b");
	_saveSound = loadSoundFile("saved.8b");
	_loadSound = loadSoundFile("loaded.8b");
	_skullSound = loadSoundFile("skull.8b");
	_shotgunSound = loadSoundFile("shotgun.8b");

	_gun = AlgGraphics::loadScreenCoordAniImage("bh_gun.ani", _palette);
	_shotgun = AlgGraphics::loadScreenCoordAniImage("bh_buck.ani", _palette);
	_numbers = AlgGraphics::loadAniImage("bh_num.ani", _palette);
	auto bullets = AlgGraphics::loadAniImage("bh_ammo.ani", _palette);
	_shotIcon = (*bullets)[0];
	_emptyIcon = (*bullets)[1];
	auto lives = AlgGraphics::loadAniImage("bh_life.ani", _palette);
	_liveIcon = (*lives)[0];
	_deadIcon = (*lives)[1];
	auto hole = AlgGraphics::loadScreenCoordAniImage("bh_hole.ani", _palette);
	_bulletholeIcon = (*hole)[0];
	auto players = AlgGraphics::loadAniImage("bh_plyr.ani", _palette);
	_playersIcon1 = (*players)[0];
	_playersIcon2 = (*players)[1];
	auto text = AlgGraphics::loadAniImage("bh_text.ani", _palette);
	_textScoreIcon = (*text)[0];
	_textMenuIcon = (*text)[1];
	_textBlackBarIcon = (*text)[2];
	_bagsIcons = AlgGraphics::loadScreenCoordAniImage("bh_bags.ani", _palette);

	_background = AlgGraphics::loadVgaBackground("bh_menu.vga", _palette);
	_screen->copyRectToSurface(_background->getPixels(), _background->pitch, 0, 0, _background->w, _background->h);

	moveMouse();
}

void GameBountyHunter::registerScriptFunctions() {
#define RECT_HIT_FUNCTION(name, func) _rectHitFuncs[name] = new BHScriptFunctionRect(this, &GameBountyHunter::func);
	RECT_HIT_FUNCTION("DEFAULT", rectNewScene);
	RECT_HIT_FUNCTION("EXITMENU", rectExit);
	RECT_HIT_FUNCTION("CONTMENU", rectContinue);
	RECT_HIT_FUNCTION("STARTMENU", rectStart);
	RECT_HIT_FUNCTION("SHOTMENU", rectShotMenu);
	RECT_HIT_FUNCTION("RECTSAVE", rectSave);
	RECT_HIT_FUNCTION("RECTLOAD", rectLoad);
	RECT_HIT_FUNCTION("TOGGLEPLAYERS", rectTogglePlayers);
	RECT_HIT_FUNCTION("JUG", rectHitIconJug);
	RECT_HIT_FUNCTION("LANTERN", rectHitIconLantern);
	RECT_HIT_FUNCTION("SKULL", rectHitIconSkull);
	RECT_HIT_FUNCTION("WHEEL", rectHitIconWheel);
	RECT_HIT_FUNCTION("HARRY", rectHitSelectHarry);
	RECT_HIT_FUNCTION("DAN", rectHitSelectDan);
	RECT_HIT_FUNCTION("LOCO", rectHitSelectLoco);
	RECT_HIT_FUNCTION("KID", rectHitSelectKid);
	RECT_HIT_FUNCTION("KILLMAN", rectHitKillMan);
	RECT_HIT_FUNCTION("KILLWOMAN", rectHitKillMan);
	RECT_HIT_FUNCTION("KILLMAIN", rectHitDoNothing);
	RECT_HIT_FUNCTION("WNDMAIN", rectHitDoNothing);
	RECT_HIT_FUNCTION("SHOTGUN", rectHitGiveShotgun);
	RECT_HIT_FUNCTION("SHOOT3", rectHitKill3);
	RECT_HIT_FUNCTION("KILL3", rectHitKill3);
	RECT_HIT_FUNCTION("GOTOBAD", rectHitDoNothing);
	RECT_HIT_FUNCTION("GOTOTGT", rectHitDoNothing);
	RECT_HIT_FUNCTION("GOTOGUN", rectHitDoNothing);
	RECT_HIT_FUNCTION("CHKSHOT", rectHitCheckShotgun);
	RECT_HIT_FUNCTION("CHEATER", rectHitCheater);
#undef RECT_HIT_FUNCTION

#define PRE_OPS_FUNCTION(name, func) _scenePreOps[name] = new BHScriptFunctionScene(this, &GameBountyHunter::func);
	PRE_OPS_FUNCTION("DEFAULT", scenePsoDrawRct);
	PRE_OPS_FUNCTION("DRAW_RECT", scenePsoDrawRct);
	PRE_OPS_FUNCTION("FADEIN", scenePsoFadeIn);
	PRE_OPS_FUNCTION("PAUSE", scenePsoPause);
	PRE_OPS_FUNCTION("PAUSE_FADEIN", scenePsoPauseFadeIn);
	PRE_OPS_FUNCTION("DRAW_RECT_THEN_FADEIN", scenePsoDrawRctFadeIn);
	PRE_OPS_FUNCTION("SHOOTOUT", scenePsoShootout);
	PRE_OPS_FUNCTION("WNDMAIN", scenePsoWoundedMain);
	PRE_OPS_FUNCTION("GUNFIGHT", scenePsoGunfightSetup);
	PRE_OPS_FUNCTION("REFEREED", scenePsoDrawRct);
	PRE_OPS_FUNCTION("LOSELIFE", scenePsoLoseALife);
	PRE_OPS_FUNCTION("L1ASETUP", scenePsoDrawRct);
	PRE_OPS_FUNCTION("L1DSETUP", scenePsoDrawRct);
	PRE_OPS_FUNCTION("L2ASETUP", scenePsoSetupNdRandom1);
	PRE_OPS_FUNCTION("L2BSETUP", scenePsoDrawRct);
	PRE_OPS_FUNCTION("L4A1SETUP", scenePsoDrawRct);
	PRE_OPS_FUNCTION("L4A2SETUP", scenePsoDrawRct);
	PRE_OPS_FUNCTION("SETUPL3A", scenePsoDrawRct);
	PRE_OPS_FUNCTION("SET3SHOT", scenePsoDrawRct);
	PRE_OPS_FUNCTION("L3BSETUP", scenePsoDrawRct);
	PRE_OPS_FUNCTION("SETBADGUY", scenePsoSetCurrentScene);
	PRE_OPS_FUNCTION("CLRKILL3", scenePsoDrawRct);
	PRE_OPS_FUNCTION("DPAUSE", scenePsoDrawRct);
	PRE_OPS_FUNCTION("DEMO", scenePsoDrawRct);
#undef PRE_OPS_FUNCTION

#define INS_OPS_FUNCTION(name, func) _sceneInsOps[name] = new BHScriptFunctionScene(this, &GameBountyHunter::func);
	INS_OPS_FUNCTION("DEFAULT", sceneIsoDoNothing);
	INS_OPS_FUNCTION("PAUSE", sceneIsoPause);
	INS_OPS_FUNCTION("SHOOTOUT", sceneIsoShootout);
	INS_OPS_FUNCTION("LEFTDIE", sceneIsoDoNothing);
	INS_OPS_FUNCTION("SHOOTPAST", sceneIsoDoNothing);
	INS_OPS_FUNCTION("GUNFIGHT", sceneIsoShootout);
	INS_OPS_FUNCTION("REFEREED", sceneIsoDoNothing);
	INS_OPS_FUNCTION("CHECK3SHOT", sceneIsoDoNothing);
	INS_OPS_FUNCTION("SHOWHI", sceneIsoDoNothing);
	INS_OPS_FUNCTION("STARTGAME", sceneIsoDoNothing);
	INS_OPS_FUNCTION("GIVEMONEY", sceneIsoGivemoney);
	INS_OPS_FUNCTION("GETHI", sceneIsoDoNothing);
	INS_OPS_FUNCTION("DPAUSE", sceneIsoDoNothing);
	INS_OPS_FUNCTION("DEMO", sceneIsoDoNothing);
	INS_OPS_FUNCTION("RELOAD", sceneIsoDoNothing);
	INS_OPS_FUNCTION("RPAUSE", sceneIsoDoNothing);
#undef INS_OPS_FUNCTION

#define NXT_SCN_FUNCTION(name, func) _sceneNxtScn[name] = new BHScriptFunctionScene(this, &GameBountyHunter::func);
	NXT_SCN_FUNCTION("DEFAULT", sceneDefaultNxtscn);
	NXT_SCN_FUNCTION("DIED", sceneNxtscnLoseALife);
	NXT_SCN_FUNCTION("LOSE_A_LIFE", sceneNxtscnLoseALife);
	NXT_SCN_FUNCTION("CONTINUE_GAME", sceneNxtscnContinueGame);
	NXT_SCN_FUNCTION("DID_NOT_CONTINUE", sceneNxtscnDidNotContinue);
	NXT_SCN_FUNCTION("KILL_INNOCENT_MAN", sceneNxtscnKillInnocentMan);
	NXT_SCN_FUNCTION("KILL_INNOCENT_WOMAN", sceneNxtscnKillInnocentWoman);
	NXT_SCN_FUNCTION("AFTER_DIE", sceneNxtscnAfterDie);
	NXT_SCN_FUNCTION("CHECKNEXT", sceneNxtscnGotoLevelSelect);
	NXT_SCN_FUNCTION("CONTINUE_RANDOM", sceneNxtscnContinueRandom);
	NXT_SCN_FUNCTION("POPUP", sceneNxtscnContinueRandom);
	NXT_SCN_FUNCTION("L1ASETUP", sceneNxtscnInitRandomHarry1);
	NXT_SCN_FUNCTION("L1DSETUP", sceneNxtscnInitRandomHarry2);
	NXT_SCN_FUNCTION("L2ASETUP", sceneNxtscnInitRandomDan1);
	NXT_SCN_FUNCTION("L2BSETUP", sceneNxtscnInitRandomDan2);
	NXT_SCN_FUNCTION("L3ASETUP", sceneNxtscnInitRandomLoco1);
	NXT_SCN_FUNCTION("L3BSETUP", sceneNxtscnInitRandomLoco2);
	NXT_SCN_FUNCTION("L4A1SETUP", sceneNxtscnInitRandomKid1);
	NXT_SCN_FUNCTION("L4A2SETUP", sceneNxtscnInitRandomKid2);
	NXT_SCN_FUNCTION("NEXTSUB", sceneNxtscnNextSubLevel);
	NXT_SCN_FUNCTION("GOTOBAD", sceneNxtscnGotoBadGuy);
	NXT_SCN_FUNCTION("AUTOSEL", sceneNxtscnAutoSelectLevel);
	NXT_SCN_FUNCTION("SELECT_SCENARIO", sceneNxtscnSelectScenario);
	NXT_SCN_FUNCTION("FINISH_SCENARIO", sceneNxtscnFinishScenario);
	NXT_SCN_FUNCTION("GAME_WON", sceneNxtscnGameWon);
	NXT_SCN_FUNCTION("KILLMAN", sceneNxtscnKillInnocentMan);
	NXT_SCN_FUNCTION("KILLWOMAN", sceneNxtscnKillInnocentWoman);
	NXT_SCN_FUNCTION("BOTHDIE", sceneNxtscnLoseALife);
	NXT_SCN_FUNCTION("RIGHTDIE", sceneNxtscnLoseALife);
	NXT_SCN_FUNCTION("LEFTDIES", sceneNxtscnLoseALife);
	NXT_SCN_FUNCTION("KILLMAIN", sceneNxtscnKilledMain);
	NXT_SCN_FUNCTION("WNDMAIN", sceneNxtscnWoundedMain);
	NXT_SCN_FUNCTION("ENDLEVEL", sceneNxtscnEndLevel);
	NXT_SCN_FUNCTION("ENDOGAME", sceneNxtscnEndGame);
	NXT_SCN_FUNCTION("CLRHI", sceneNxtscnDoNothing);
	NXT_SCN_FUNCTION("TGTPRACT", sceneNxtscnDoNothing);
	NXT_SCN_FUNCTION("CREDITS", sceneNxtscnDoNothing);
	NXT_SCN_FUNCTION("DOMAIN", sceneNxtscnDoBreakoutMains);
	NXT_SCN_FUNCTION("RDIED", sceneNxtscnDiedRefed);
	NXT_SCN_FUNCTION("GIVESHOT", sceneNxtscnGiveShotgun);
	NXT_SCN_FUNCTION("CHK2P", sceneNxtscnCheck2Players);
	NXT_SCN_FUNCTION("SHOTSND", sceneNxtscnDoNothing);
	NXT_SCN_FUNCTION("XITCONT", sceneNxtscnDoNothing);
#undef NXT_SCN_FUNCTION

#define WEP_DWN_FUNCTION(name, func) _sceneWepDwn[name] = new BHScriptFunctionScene(this, &GameBountyHunter::func);
	WEP_DWN_FUNCTION("DEFAULT", sceneDefaultWepdwn);
	WEP_DWN_FUNCTION("GUNFIGHT", sceneDefaultWepdwn);
	WEP_DWN_FUNCTION("LOADALL", sceneDefaultWepdwn);
#undef NXT_SCN_FUNCTION

	_sceneShowMsg["DEFAULT"] = new BHScriptFunctionScene(this, &GameBountyHunter::sceneSmDonothing);
	_sceneScnScr["DEFAULT"] = new BHScriptFunctionScene(this, &GameBountyHunter::sceneDefaultScore);
	_sceneNxtFrm["DEFAULT"] = new BHScriptFunctionScene(this, &GameBountyHunter::sceneNxtfrm);
}

void GameBountyHunter::verifyScriptFunctions() {
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

BHScriptFunctionRect GameBountyHunter::getScriptFunctionRectHit(Common::String name) {
	auto it = _rectHitFuncs.find(name);
	if (it != _rectHitFuncs.end()) {
		return *it->_value;
	} else {
		error("GameBountyHunter::getScriptFunctionRectHit(): Could not find rectHit function: %s", name.c_str());
	}
}

BHScriptFunctionScene GameBountyHunter::getScriptFunctionScene(SceneFuncType type, Common::String name) {
	BHScriptFunctionSceneMap *functionMap;
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
		error("GameBountyHunter::getScriptFunctionScene(): Unkown scene script type: %u", type);
		break;
	}
	BHScriptFunctionSceneMap::iterator it;
	it = functionMap->find(name);
	if (it != functionMap->end()) {
		return *it->_value;
	} else {
		error("GameBountyHunter::getScriptFunctionScene(): Could not find scene type %u function: %s", type, name.c_str());
	}
}

void GameBountyHunter::callScriptFunctionRectHit(Common::String name, Rect *rect) {
	BHScriptFunctionRect function = getScriptFunctionRectHit(name);
	function(rect);
}

void GameBountyHunter::callScriptFunctionScene(SceneFuncType type, Common::String name, Scene *scene) {
	BHScriptFunctionScene function = getScriptFunctionScene(type, name);
	function(scene);
}

Common::Error GameBountyHunter::run() {
	init();
	newGame();
	_curScene = _startScene;
	Common::String oldscene;
	while (!_vm->shouldQuit()) {
		oldscene = _curScene;
		_fired = false;
		Scene *scene = _sceneInfo->findScene(_curScene);
		if (!loadScene(scene)) {
			error("GameBountyHunter::run(): Cannot find scene %s in libfile", scene->_name.c_str());
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
					} else {
						if (_playerShots[_player] > 0) {
							if (!_debug_unlimitedAmmo) {
								_playerShots[_player]--;
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
			}
			if (_curScene == oldscene) {
				callScriptFunctionScene(NXTFRM, scene->_nxtfrm, scene);
			}
			displayLivesLeft(0);
			displayScores(0);
			displayShotsLeft(0);
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
		if (_curScene == oldscene) {
			callScriptFunctionScene(NXTSCN, scene->_nxtscn, scene);
		}
		if (_curScene == "") {
			shutdown();
		}
	}
	return Common::kNoError;
}

void GameBountyHunter::newGame() {
	_playerLives[0] = _playerLives[1] = 3;
	_playerShots[0] = _playerShots[1] = 6;
	_playerGun[0] = _playerGun[1] = 1;
	_playerScore[0] = _playerScore[1] = 0;
	_currentSubLevelSceneId = 0x017B;
	_holster = false;
}

void GameBountyHunter::doMenu() {
	uint32 startTime = getMsTime();
	updateCursor();
	_inMenu = true;
	moveMouse();
	_videoDecoder->pauseAudio(true);
	_screen->copyRectToSurface(_background->getBasePtr(_videoPosX, _videoPosY), _background->pitch, _videoPosX, _videoPosY, _videoDecoder->getWidth(), _videoDecoder->getHeight());
	while (_inMenu && !_vm->shouldQuit()) {
		Common::Point firedCoords;
		if (fired(&firedCoords)) {
			Rect *hitMenuRect = checkZone(_submenzone, &firedCoords);
			if (hitMenuRect != nullptr) {
				callScriptFunctionRectHit(hitMenuRect->_rectHit, hitMenuRect);
			}
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

void GameBountyHunter::updateCursor() {
	updateMouse();
}

void GameBountyHunter::updateMouse() {
	if (_oldWhichGun != _whichGun) {
		Graphics::Surface *cursor = (*_gun)[_whichGun];
		if (_playerGun[0] == 2 && _whichGun < 2) {
			cursor = (*_shotgun)[_whichGun];
		}
		uint16 hotspotX = (cursor->w / 2) + 8;
		uint16 hotspotY = (cursor->h / 2) + 8;
		if (debugChannelSet(1, Alg::kAlgDebugGraphics)) {
			cursor->drawLine(0, hotspotY, cursor->w, hotspotY, 1);
			cursor->drawLine(hotspotX, 0, hotspotX, cursor->h, 1);
		}
		CursorMan.replaceCursor(cursor->getPixels(), cursor->w, cursor->h, hotspotX, hotspotY, 0);
		CursorMan.showMouse(true);
		_oldWhichGun = _whichGun;
	}
}

void GameBountyHunter::moveMouse() {
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

void GameBountyHunter::displayLivesLeft(uint8 player) {
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

void GameBountyHunter::displayScores(uint8 player) {
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

void GameBountyHunter::displayShotsLeft(uint8 player) {
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

bool GameBountyHunter::weaponDown() {
	if (_rightDown && _mousePos.y >= 0xAA && _mousePos.x >= 0x113) {
		return true;
	}
	return false;
}

bool GameBountyHunter::saveState() {
	Common::OutSaveFile *outSaveFile;
	Common::String saveFileName = _vm->getSaveStateName(0);
	if (!(outSaveFile = g_system->getSavefileManager()->openForSaving(saveFileName))) {
		warning("GameBountyHunter::saveState(): Can't create file '%s', game not saved", saveFileName.c_str());
		return false;
	}
	uint16 currentSceneNum = atoi(_curScene.c_str());
	outSaveFile->writeUint32BE(MKTAG('A', 'L', 'G', 'S')); // header
	outSaveFile->writeByte(0);                             // version, unused for now
	outSaveFile->writeByte(_currentLevel);
	outSaveFile->writeUint16LE(_currentSubLevelSceneId);
	outSaveFile->writeByte(_continuesUsed);
	outSaveFile->writeUint16LE(currentSceneNum);
	for (int i = 0; i < 2; i++) {
		outSaveFile->writeByte(_playerLives[i]);
		outSaveFile->writeByte(_playerShots[i]);
		outSaveFile->writeUint32LE(_playerScore[i]);
	}
	outSaveFile->writeByte(_unk_2ADA6);
	outSaveFile->writeByte(_numPlayers);
	outSaveFile->finalize();
	delete outSaveFile;
	return true;
}

bool GameBountyHunter::loadState() {
	Common::InSaveFile *inSaveFile;
	Common::String saveFileName = _vm->getSaveStateName(0);
	if (!(inSaveFile = g_system->getSavefileManager()->openForLoading(saveFileName))) {
		debug("GameBountyHunter::loadState(): Can't load file '%s', game not loaded", saveFileName.c_str());
		return false;
	}
	uint32 header = inSaveFile->readUint32BE();
	if (header != MKTAG('A', 'L', 'G', 'S')) {
		warning("GameBountyHunter::loadState(): Unkown save file, header: %s", tag2str(header));
		return false;
	}
	inSaveFile->skip(1); // version, unused for now
	_currentLevel = inSaveFile->readByte();
	_currentSubLevelSceneId = inSaveFile->readUint16LE();
	_continuesUsed = inSaveFile->readByte();
	_restartScene = inSaveFile->readUint16LE();
	for (int i = 0; i < 2; i++) {
		_playerLives[i] = inSaveFile->readByte();
		_playerShots[i] = inSaveFile->readByte();
		_playerScore[i] = inSaveFile->readUint32LE();
	}
	_unk_2ADA6 = inSaveFile->readByte();
	_numPlayers = inSaveFile->readByte();
	delete inSaveFile;
	_gameInProgress = true;
	return true;
}

// misc game functions
void GameBountyHunter::setNextScene(uint16 sceneId) {
	_curScene = Common::String::format("%d", sceneId);
}

void GameBountyHunter::displayShotFiredImage(Common::Point *point) {
	if (point->x >= _videoPosX && point->x <= (_videoPosX + _videoDecoder->getWidth()) && point->y >= _videoPosY && point->y <= (_videoPosY + _videoDecoder->getHeight())) {
		uint16 targetX = point->x - _videoPosX;
		uint16 targetY = point->y - _videoPosY;
		AlgGraphics::drawImageCentered(_videoDecoder->getVideoFrame(), _bulletholeIcon, targetX, targetY);
	}
}

void GameBountyHunter::enableVideoFadeIn() {
	// TODO implement
}

void GameBountyHunter::iconShotgun() {
	// TODO implement
}

void GameBountyHunter::iconReset() {
	// TODO implement
}

uint16 GameBountyHunter::beginLevel(uint8 levelNumber) {
	_currentLevel = levelNumber;
	_numSubLevelsDone = 0;
	int index = (levelNumber * 24) + (_numLevelsDone * 6) + _numSubLevelsDone;
	uint8 subLevel = _subLevelOrder[index];
	uint16 sceneIndex = (_currentLevel * 5) + subLevel;
	uint16 sceneNum = _subLevelSceneIds[sceneIndex];
	_currentSubLevelSceneId = sceneNum;
	return sceneNum;
}

uint16 GameBountyHunter::pickRandomScene(uint16 *sceneList, uint8 max) {
	if (max == 0) {
		return 0;
	}
	_randomSceneList = sceneList;
	_randomMax = max;
	_randomMask = 0;
	_randomPicked = -1;
	_randomSceneCount = 0;
	while (_randomSceneList[_randomSceneCount] != 0) {
		_randomSceneCount++;
	}
	uint16 count = _randomMax--;
	if (count > 0) {
		_randomPicked = randomUnusedInt(_randomSceneCount, &_randomMask, _randomPicked);
		return _randomSceneList[_randomPicked];
	}
	return 0;
}

uint16 GameBountyHunter::pickGunfightScene() {
	if (!_gunfightInitialized) {
		_gunfightInitialized = true;
		_gunfightMask = 0;
		_gunfightPicked = -1;
		_gunfightSceneCount = 0;
		while (_gunfightScenarios[_gunfightSceneCount] != 0) {
			_gunfightSceneCount++;
		}
	}
	_randomPicked = randomUnusedInt(_gunfightSceneCount, &_gunfightMask, _randomPicked);
	return _gunfightScenarios[_randomPicked];
}

uint16 GameBountyHunter::pickInnocentScene() {
	if (!_innocentInitialized) {
		_innocentInitialized = true;
		_innocentMask = 0;
		_innocentPicked = -1;
		_innocentSceneCount = 0;
		while (_innocentScenarios[_innocentSceneCount] != 0) {
			_innocentSceneCount++;
		}
	}
	_innocentPicked = randomUnusedInt(_innocentSceneCount, &_innocentMask, _innocentPicked);
	return _innocentScenarios[_innocentPicked];
}

uint16 GameBountyHunter::pickDeathScene() {
	if (!_deathInitialized) {
		_deathInitialized = true;
		_deathMask = 0;
		_deathPicked = -1;
		_deathSceneCount = 0;
		while (_deathScenarios[_deathSceneCount] != 0) {
			_deathSceneCount++;
		}
	}
	_deathPicked = randomUnusedInt(_deathSceneCount, &_deathMask, _deathPicked);
	return _deathScenarios[_deathPicked];
}

uint16 GameBountyHunter::timeForGunfight() {
	uint16 picked = 0;
	if (--_gunfightCount <= 0) {
		int index = (_unk_2ADA6 * 5) + (_numLevelsDone);
		_gunfightCount = _gunfightCountDown[index];
		picked = pickGunfightScene();
	}
	return picked;
}

void GameBountyHunter::waitingForShootout(uint32 drawFrame) {
	if (drawFrame != 0) {
		for (int i = 0; i < _numPlayers; i++) {
			_firstDrawFrame = drawFrame;
			_playerShots[i] = 0;
			_playerGun[i] = 0;
			displayShotsLeft(i);
		}
	}
	// TODO investigate & fix
	if (_currentFrame > _firstDrawFrame) {
		// player1 = 1;
		// player2 = 1;
	}
	/*
	if (shotsPlayer1 <= 0 && shotsPlayer2 <= 0) {
		return false;
	}
	return true;
	*/
}

void GameBountyHunter::doShotgunSound() {
	playSound(_shotgunSound);
}

// Script functions: RectHit
void GameBountyHunter::rectShotMenu(Rect *rect) {
	doMenu();
}

void GameBountyHunter::rectSave(Rect *rect) {
	if (saveState()) {
		doSaveSound();
	}
}

void GameBountyHunter::rectLoad(Rect *rect) {
	if (loadState()) {
		doLoadSound();
	}
	setNextScene(_restartScene);
	_restartScene = 0;
}

void GameBountyHunter::rectContinue(Rect *rect) {
	_inMenu = false;
	_fired = false;
	if (_gameInProgress) {
		bool canContinue = (_playerLives[0] > 0 && _playerLives[1] > 0);
		if (!canContinue) {
			int playerIndex = (_playerLives[0] > 0) ? 0 : 1;
			_continuesUsed++;
			if (_continuesUsed <= 2) {
				_playerLives[playerIndex] = 3;
				_playerScore[playerIndex] *= 0.75;
			} else {
				sceneNxtscnDidNotContinue(nullptr);
			}
		}
	}
}

void GameBountyHunter::rectStart(Rect *rect) {
	_inMenu = false;
	_fired = false;
	_gameInProgress = true;
	_restartScene = 0;
	if (_vm->isDemo()) {
		debug(5, "FIXME: Demo Next Scene is 0x45?");
		setNextScene(0x45); // TODO fix
	} else {
		setNextScene(0x45);
	}
	newGame();
}

void GameBountyHunter::rectTogglePlayers(Rect *rect) {
	if (_numPlayers == 1) {
		_numPlayers = 2;
		AlgGraphics::drawImage(_screen, _playersIcon2, 0xCE, 0x95);
		AlgGraphics::drawImage(_screen, _textBlackBarIcon, 0x78, 0xBF);
		AlgGraphics::drawImage(_screen, _textBlackBarIcon, 0x0C, 0xBF);
		displayShotsLeft(1);
		displayLivesLeft(1);
	} else {
		_numPlayers = 1;
		AlgGraphics::drawImage(_screen, _playersIcon1, 0xCE, 0x95);
		AlgGraphics::drawImage(_screen, _textScoreIcon, 0x78, 0xBF);
		AlgGraphics::drawImage(_screen, _textMenuIcon, 0x0C, 0xBF);
		AlgGraphics::drawImage(_screen, _textBlackBarIcon, 0x50, 0xBE);
	}
	doSkullSound();
	_screen->copyRectToSurface(_background->getBasePtr(_videoPosX, _videoPosY), _background->pitch, _videoPosX, _videoPosY, _videoDecoder->getWidth(), _videoDecoder->getHeight());
}

void GameBountyHunter::rectHitIconJug(Rect *rect) {
	// TODO fix
	// Icon.funcs[1](param);
	// PlaySound(4);
	// Icon.hitCount++;
}

void GameBountyHunter::rectHitIconLantern(Rect *rect) {
	// TODO fix
	// Icon.funcs[3](param);
	// PlaySound(4);
	// Icon.hitCount++;
}

void GameBountyHunter::rectHitIconSkull(Rect *rect) {
	// TODO fix
	// Icon.funcs[0](param);
	// PlaySound(4);
	// Icon.hitCount++;
}

void GameBountyHunter::rectHitIconWheel(Rect *rect) {
	// TODO fix
	// Icon.funcs[2](param);
	// PlaySound(4);
	// Icon.hitCount++;
}

void GameBountyHunter::rectHitSelectHarry(Rect *rect) {
	if (!(_levelDoneMask & 2)) {
		uint16 picked = beginLevel(0);
		setNextScene(picked);
	}
}

void GameBountyHunter::rectHitSelectDan(Rect *rect) {
	if (!(_levelDoneMask & 4)) {
		uint16 picked = beginLevel(1);
		setNextScene(picked);
	}
}

void GameBountyHunter::rectHitSelectLoco(Rect *rect) {
	if (!(_levelDoneMask & 8)) {
		uint16 picked = beginLevel(2);
		setNextScene(picked);
	}
}

void GameBountyHunter::rectHitSelectKid(Rect *rect) {
	if (!(_levelDoneMask & 0x10)) {
		uint16 picked = beginLevel(3);
		setNextScene(picked);
	}
}

void GameBountyHunter::rectHitKillMan(Rect *rect) {
	// do nothing
}

void GameBountyHunter::rectHitGiveShotgun(Rect *rect) {
	iconShotgun();
}

void GameBountyHunter::rectHitKill3(Rect *rect) {
	_count++;
	if (_count == 3) {
		_count = 0;
		rectNewScene(rect);
		// TODO verify
		// _RHONewScene(param1, param2);
	}
}

void GameBountyHunter::rectHitCheckShotgun(Rect *rect) {
	if (_playerGun[_player] == 2) {
		rectNewScene(rect);
	}
}

void GameBountyHunter::rectHitCheater(Rect *rect) {
	setNextScene(0x011A);
}

// Script functions: Scene PreOps
void GameBountyHunter::scenePsoShootout(Scene *scene) {
	waitingForShootout(atoi(scene->_preopParam.c_str()));
}

void GameBountyHunter::scenePsoWoundedMain(Scene *scene) {
	_wounded = true;
	_currentSubLevelSceneId = _moneyScenes[_currentLevel];
}

void GameBountyHunter::scenePsoGunfightSetup(Scene *scene) {
	waitingForShootout(atoi(scene->_preopParam.c_str()));
}

void GameBountyHunter::scenePsoLoseALife(Scene *scene) {
	sceneNxtscnLoseALife(scene);
}

void GameBountyHunter::scenePsoSetupNdRandom1(Scene *scene) {
	_numSubLevelsDone++;
}

void GameBountyHunter::scenePsoSetCurrentScene(Scene *scene) {
	int sceneId = atoi(scene->_preopParam.c_str());
	_currentSubLevelSceneId = sceneId;
	if (sceneId == 0) {
		uint8 index = (_currentLevel * 24) + (_numLevelsDone * 6) + _numSubLevelsDone;
		uint8 subLevel = _subLevelOrder[index];
		uint16 picked = (_currentLevel * 20) + (subLevel * 4);
		_currentSubLevelSceneId = 0x0D32 + picked;
	}
}

// Script functions: Scene InsOps
void GameBountyHunter::sceneIsoShootout(Scene *scene) {
	waitingForShootout(0);
}

void GameBountyHunter::sceneIsoGivemoney(Scene *scene) {
	const int moneyFrames[] = {0x1E8F, 0x3BB4, 0x7814, 0xA287};
	const int woundBits[] = {2, 4, 8, 0x10};
	for (int i = 0; i < _numPlayers; i++) {
		if (_currentLevel <= 3) {
			uint32 moneyFrame = moneyFrames[_currentLevel];
			// TODO investigate
			if (moneyFrame == _currentFrame && !_given) {
				if (_wounded) {
					_mainWounds |= woundBits[_currentLevel];
					int bonus = (2 ^ _numLevelsDone) * 200;
					_playerScore[i] += bonus;
				} else {
					int bonus = (2 ^ _numLevelsDone) * 100;
					_playerScore[i] += bonus;
				}
				_wounded = false;
				_given = true;
			} else if (moneyFrame != _currentFrame) {
				_given = false;
			}
		}
		displayScores(i);
	}
}

// Script functions: Scene NxtScn
void GameBountyHunter::sceneNxtscnLoseALife(Scene *scene) {
	uint16 picked = 0;
	int deadPlayerCount = 0;
	for (int i = 0; i < _numPlayers; i++) {
		_playerLives[i]--;
		displayLivesLeft(i);
		if (_playerLives[i] <= 0) {
			_playerScore[i] = (_playerScore[i] * 6) / 10;
			deadPlayerCount++;
		}
	}
	if (deadPlayerCount == 1 && _numPlayers == 2) {
		picked = _onePlayerOfTwoDead[_numSubLevelsDone & 1];
	} else if (deadPlayerCount > 0) {
		picked = _allPlayersDead;
	} else {
		picked = pickDeathScene();
	}
	setNextScene(picked);
}

void GameBountyHunter::sceneNxtscnContinueGame(Scene *scene) {
	if (_continuesUsed < 2) {
		setNextScene(0x01B6);
	} else {
		sceneNxtscnDidNotContinue(scene);
	}
}

void GameBountyHunter::sceneNxtscnDidNotContinue(Scene *scene) {
	_gameInProgress = false;
	_curScene = _startScene;
}

void GameBountyHunter::sceneNxtscnKillInnocentMan(Scene *scene) {
	uint16 picked = 0;
	_playerLives[_player]--;
	if (_playerLives[_player]) {
		picked = pickInnocentScene();
	} else {
		if (_numPlayers == 2) {
			picked = _onePlayerOfTwoDead[_numSubLevelsDone & 1];
		} else {
			picked = _allPlayersDead;
		}
	}
	setNextScene(picked);
}

void GameBountyHunter::sceneNxtscnKillInnocentWoman(Scene *scene) {
	sceneNxtscnKillInnocentMan(scene);
}

void GameBountyHunter::sceneNxtscnAfterDie(Scene *scene) {
	for (int i = 0; i < _numPlayers; i++) {
		if (_playerLives[i] <= 0) {
			_playerLives[i] = 3;
			displayLivesLeft(i);
		}
	}
	setNextScene(_currentSubLevelSceneId);
}

void GameBountyHunter::sceneNxtscnGotoLevelSelect(Scene *scene) {
	iconReset();
	uint16 picked = 0;
	if ((_levelDoneMask & 0x1E) != 0x1E) {
		picked = 0x17B;
	} else if (!(_levelDoneMask & 0x80)) {
		picked = 0x66;
	} else {
		picked = 0x61;
	}
	setNextScene(picked);
}

void GameBountyHunter::sceneNxtscnContinueRandom(Scene *scene) {
	// TODO verify
	sceneNxtscnNextSubLevel(scene);
	/*
	uint16 picked = _PickRandomScene(0, 0);
	if (picked == 0) {
		_scene_nxtscn_next_sub_level(scene);
		return;
	}
	_SetNextScene(picked);
	*/
}

void GameBountyHunter::sceneNxtscnInitRandomHarry1(Scene *scene) {
	uint16 picked = pickRandomScene(_randomScenes[0], _randomScenesPicks[0]);
	_currentSubLevelSceneId = picked;
	setNextScene(picked);
}

void GameBountyHunter::sceneNxtscnInitRandomHarry2(Scene *scene) {
	uint16 picked = pickRandomScene(_randomScenes[1], _randomScenesPicks[1]);
	setNextScene(picked);
}

void GameBountyHunter::sceneNxtscnInitRandomDan1(Scene *scene) {
	uint16 picked = 0;
	uint8 picks = _randomScenesPicks[2] + _numPlayers;
	if (_numPlayers == 2) {
		picked = pickRandomScene(_randomDan1TwoPlayer, picks);
	} else {
		picked = pickRandomScene(_randomScenes[2], picks);
	}
	_currentSubLevelSceneId = 0x0174;
	setNextScene(picked);
}

void GameBountyHunter::sceneNxtscnInitRandomDan2(Scene *scene) {
	uint16 picked = pickRandomScene(_randomScenes[3], _randomScenesPicks[3]);
	_currentSubLevelSceneId = picked;
	setNextScene(picked);
}

void GameBountyHunter::sceneNxtscnInitRandomLoco1(Scene *scene) {
	uint16 picked = pickRandomScene(_randomScenes[4], _randomScenesPicks[4]);
	setNextScene(picked);
}

void GameBountyHunter::sceneNxtscnInitRandomLoco2(Scene *scene) {
	uint16 picked = pickRandomScene(_randomScenes[5], _randomScenesPicks[5]);
	setNextScene(picked);
}

void GameBountyHunter::sceneNxtscnInitRandomKid1(Scene *scene) {
	uint16 picked = pickRandomScene(_randomScenes[6], _randomScenesPicks[6]);
	setNextScene(picked);
}

void GameBountyHunter::sceneNxtscnInitRandomKid2(Scene *scene) {
	uint16 picked = pickRandomScene(_randomScenes[7], _randomScenesPicks[7]);
	setNextScene(picked);
}

void GameBountyHunter::sceneNxtscnNextSubLevel(Scene *scene) {
	iconReset();
	_numSubLevelsDone++;
	int index = (_currentLevel * 24) + (_numLevelsDone * 6) + _numSubLevelsDone;
	uint8 subLevel = _subLevelOrder[index];
	uint16 sceneIndex = (_currentLevel * 5) + subLevel;
	uint16 picked = _subLevelSceneIds[sceneIndex];
	_currentSubLevelSceneId = picked;
	uint16 gunfightScene = timeForGunfight();
	if (gunfightScene != 0) {
		setNextScene(gunfightScene);
	}
	if (subLevel == 2) {
		if (_currentLevel == 0) {
			setNextScene(picked);
			return;
		}
		picked = _clueLevels[_currentLevel];
	}
	setNextScene(picked);
}

void GameBountyHunter::sceneNxtscnGotoBadGuy(Scene *scene) {
	iconReset();
	uint8 index = (_currentLevel * 24) + (_numLevelsDone * 6) + _numSubLevelsDone;
	uint8 subLevel = _subLevelOrder[index];
	uint16 sceneIndex = (_currentLevel * 5) + subLevel;
	uint16 picked = _subLevelSceneIds[sceneIndex];
	setNextScene(picked);
}

void GameBountyHunter::sceneNxtscnAutoSelectLevel(Scene *scene) {
	iconReset();
	uint8 i;
	for (i = 0; i < 4; i++) {
		if (!(_levelDoneMask & _mainLevelMasks[i])) {
			break;
		}
	}
	uint16 picked = beginLevel(i);
	setNextScene(picked);
}

void GameBountyHunter::sceneNxtscnSelectScenario(Scene *scene) {
	setNextScene(_currentLevel);
}

void GameBountyHunter::sceneNxtscnFinishScenario(Scene *scene) {
	// do nothing
}

void GameBountyHunter::sceneNxtscnGameWon(Scene *scene) {
	_gameInProgress = false;
	_curScene = _startScene;
}

void GameBountyHunter::sceneNxtscnKilledMain(Scene *scene) {
	_wounded = false;
	_subScene = "scene379";
}

void GameBountyHunter::sceneNxtscnWoundedMain(Scene *scene) {
	_wounded = true;
	_subScene = "scene379";
}

void GameBountyHunter::sceneNxtscnEndLevel(Scene *scene) {
	_levelDoneMask |= (2 << _currentLevel);
	_numLevelsDone++;
	if (_numLevelsDone > 1 && _unk_2ADA6 < 2) {
		_unk_2ADA6++;
	}
	_numSubLevelsDone = 0;
	_currentSubLevelSceneId = 0;
	if (_numLevelsDone == 4) {
		setNextScene(0x66);
		return;
	}
	setNextScene(0x017B);
}

void GameBountyHunter::sceneNxtscnEndGame(Scene *scene) {
	_gameInProgress = false;
	_curScene = _startScene;
}

void GameBountyHunter::sceneNxtscnDoBreakoutMains(Scene *scene) {
	uint16 picked = 0;
	if (_mainWounds & 2) {
		_mainWounds &= 0xFFFD;
		picked = 0x53;
	} else if (_mainWounds & 4) {
		_mainWounds &= 0xFFFB;
		picked = 0x50;
	} else if (_mainWounds & 8) {
		_mainWounds &= 0xFFF7;
		picked = 0x4D;
	} else if (_mainWounds & 0x10) {
		_mainWounds &= 0xFFEF;
		picked = 0x4B;
	} else {
		picked = 0x61;
	}
	setNextScene(picked);
}

void GameBountyHunter::sceneNxtscnDiedRefed(Scene *scene) {
	uint16 picked = 0;
	uint8 deadCount = 0;
	for (int i = 0; i < _numPlayers; i++) {
		_playerLives[i]--;
		displayLivesLeft(i);
		if (_playerLives[i] <= 0) {
			deadCount++;
		}
	}
	if (deadCount == 1 && _numPlayers == 2) {
		picked = _onePlayerOfTwoDead[_numSubLevelsDone & 1];
	} else if (deadCount > 0) {
		picked = _allPlayersDead;
	} else {
		picked = 0x114;
	}
	setNextScene(picked);
}

void GameBountyHunter::sceneNxtscnGiveShotgun(Scene *scene) {
	for (int i = 0; i < _numPlayers; i++) {
		_playerShots[i] = 5;
		_playerGun[i] = 2;
		displayShotsLeft(i);
	}
}

void GameBountyHunter::sceneNxtscnCheck2Players(Scene *scene) {
	if (_numPlayers == 2) {
		setNextScene(0x98);
		return;
	}
	setNextScene(0x99);
}

// Script functions: WepDwn
void GameBountyHunter::sceneDefaultWepdwn(Scene *scene) {
	if (_playerGun[_player] == 2 && _playerShots[_player] < 3) {
		_playerGun[_player] = 1;
	}
	if (_playerGun[_player] == 1 && _playerShots[_player] < 6) {
		_playerShots[_player] = 6;
	}
}

// Debug methods
void GameBountyHunter::debugWarpTo(int val) {
	// TODO implement
}

// Debugger methods
DebuggerBountyHunter::DebuggerBountyHunter(GameBountyHunter *game) {
	_game = game;
	registerVar("drawRects", &game->_debug_drawRects);
	registerVar("godMode", &game->_debug_godMode);
	registerVar("unlimitedAmmo", &game->_debug_unlimitedAmmo);
	registerCmd("warpTo", WRAP_METHOD(DebuggerBountyHunter, cmdWarpTo));
	registerCmd("dumpLib", WRAP_METHOD(DebuggerBountyHunter, cmdDumpLib));
}

bool DebuggerBountyHunter::cmdWarpTo(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: warp <int>");
		return true;
	} else {
		int val = atoi(argv[1]);
		_game->debugWarpTo(val);
		return false;
	}
}

bool DebuggerBountyHunter::cmdDumpLib(int argc, const char **argv) {
	return _game->debug_dumpLibFile();
}

} // End of namespace Alg
