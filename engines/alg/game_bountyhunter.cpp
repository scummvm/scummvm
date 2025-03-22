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
#include "graphics/pixelformat.h"

#include "alg/game_bountyhunter.h"
#include "alg/graphics.h"
#include "alg/scene.h"

namespace Alg {

GameBountyHunter::GameBountyHunter(AlgEngine *vm, const ADGameDescription *desc) : Game(vm) {
	if (scumm_stricmp(desc->gameId, "lbhunter") == 0) {
		_libFileName = "bhds.lib";
	} else if (scumm_stricmp(desc->gameId, "lbhunterdemo") == 0) {
		_libFileName = "bhdemo.lib";
		_isDemo = true;
	}
}

GameBountyHunter::~GameBountyHunter() {
}

void GameBountyHunter::init() {
	_videoPosX = 0;
	_videoPosY = 0;

	loadLibArchive(_libFileName);
	_sceneInfo->loadScnFile("bh.scn");
	_startscene = _sceneInfo->getStartScene();

	registerScriptFunctions();
	verifyScriptFunctions();

	_menuzone = new Zone();
	_menuzone->name = "MainMenu";
	_menuzone->ptrfb = "GLOBALHIT";

	_menuzone->addRect(0x0C, 0xAA, 0x38, 0xC7, nullptr, 0, "SHOTMENU", "0");

	_submenzone = new Zone();
	_submenzone->name = "SubMenu";
	_submenzone->ptrfb = "GLOBALHIT";

	_submenzone->addRect(0, 0, 0x78, 0x3C, nullptr, 0, "STARTMENU", "0");
	_submenzone->addRect(0xC8, 0, 0x0140, 0x3C, nullptr, 0, "RECTLOAD", "0");
	_submenzone->addRect(0xC8, 0x3C, 0x0140, 0x78, nullptr, 0, "RECTSAVE", "0");
	_submenzone->addRect(0, 0x3C, 0x78, 0x78, nullptr, 0, "CONTMENU", "0");
	_submenzone->addRect(0, 0x78, 0x78, 0xB4, nullptr, 0, "EXITMENU", "0");
	_submenzone->addRect(0xC8, 0x78, 0x0140, 0xB4, nullptr, 0, "TOGGLEPLAYERS", "0");

	_shotSound = _LoadSoundFile("blow.8b");
	_emptySound = _LoadSoundFile("empty.8b");
	_saveSound = _LoadSoundFile("saved.8b");
	_loadSound = _LoadSoundFile("loaded.8b");
	_skullSound = _LoadSoundFile("skull.8b");
	_shotgunSound = _LoadSoundFile("shotgun.8b");

	_gun = AlgGraphics::loadScreenCoordAniImage("bh_gun.ani", _palette);
	_shotgun = AlgGraphics::loadScreenCoordAniImage("bh_buck.ani", _palette);
	_numbers = AlgGraphics::loadAniImage("bh_num.ani", _palette);
	Common::Array<Graphics::Surface> *bullets = AlgGraphics::loadAniImage("bh_ammo.ani", _palette);
	_shotIcon = (*bullets)[0];
	_emptyIcon = (*bullets)[1];
	Common::Array<Graphics::Surface> *lives = AlgGraphics::loadAniImage("bh_life.ani", _palette);
	_liveIcon = (*lives)[0];
	_deadIcon = (*lives)[1];
	Common::Array<Graphics::Surface> *hole = AlgGraphics::loadScreenCoordAniImage("bh_hole.ani", _palette);
	_bulletholeIcon = (*hole)[0];
	Common::Array<Graphics::Surface> *players = AlgGraphics::loadAniImage("bh_plyr.ani", _palette);
	_playersIcon1 = (*players)[0];
	_playersIcon2 = (*players)[1];
	Common::Array<Graphics::Surface> *text = AlgGraphics::loadAniImage("bh_text.ani", _palette);
	_textScoreIcon = (*text)[0];
	_textMenuIcon = (*text)[1];
	_textBlackBarIcon = (*text)[2];
	_bagsIcons = AlgGraphics::loadScreenCoordAniImage("bh_bags.ani", _palette);

	_background = AlgGraphics::loadVgaBackground("bh_menu.vga", _palette);
	_screen->copyRectToSurface(_background->getPixels(), _background->pitch, 0, 0, _background->w, _background->h);

	_MoveMouse();
}

void GameBountyHunter::registerScriptFunctions() {
#define RECT_HIT_FUNCTION(name, func) _rectHitFuncs[name] = new BHScriptFunctionRect(this, &GameBountyHunter::func);
	RECT_HIT_FUNCTION("DEFAULT", _rect_newscene);
	RECT_HIT_FUNCTION("EXITMENU", _rect_exit);
	RECT_HIT_FUNCTION("CONTMENU", _rect_continue);
	RECT_HIT_FUNCTION("STARTMENU", _rect_start);
	RECT_HIT_FUNCTION("SHOTMENU", _rect_shotmenu);
	RECT_HIT_FUNCTION("RECTSAVE", _rect_save);
	RECT_HIT_FUNCTION("RECTLOAD", _rect_load);
	RECT_HIT_FUNCTION("TOGGLEPLAYERS", _rect_toggle_players);
	RECT_HIT_FUNCTION("JUG", _rect_hit_icon_jug);
	RECT_HIT_FUNCTION("LANTERN", _rect_hit_icon_lantern);
	RECT_HIT_FUNCTION("SKULL", _rect_hit_icon_skull);
	RECT_HIT_FUNCTION("WHEEL", _rect_hit_icon_wheel);
	RECT_HIT_FUNCTION("HARRY", _rect_hit_select_harry);
	RECT_HIT_FUNCTION("DAN", _rect_hit_select_dan);
	RECT_HIT_FUNCTION("LOCO", _rect_hit_select_loco);
	RECT_HIT_FUNCTION("KID", _rect_hit_select_kid);
	RECT_HIT_FUNCTION("KILLMAN", _rect_hit_kill_man);
	RECT_HIT_FUNCTION("KILLWOMAN", _rect_hit_kill_man);
	RECT_HIT_FUNCTION("KILLMAIN", _rect_hit_donothing);
	RECT_HIT_FUNCTION("WNDMAIN", _rect_hit_donothing);
	RECT_HIT_FUNCTION("SHOTGUN", _rect_hit_give_shotgun);
	RECT_HIT_FUNCTION("SHOOT3", _rect_hit_kill3);
	RECT_HIT_FUNCTION("KILL3", _rect_hit_kill3);
	RECT_HIT_FUNCTION("GOTOBAD", _rect_hit_donothing);
	RECT_HIT_FUNCTION("GOTOTGT", _rect_hit_donothing);
	RECT_HIT_FUNCTION("GOTOGUN", _rect_hit_donothing);
	RECT_HIT_FUNCTION("CHKSHOT", _rect_hit_check_shotgun);
	RECT_HIT_FUNCTION("CHEATER", _rect_hit_cheater);
#undef RECT_HIT_FUNCTION

#define PRE_OPS_FUNCTION(name, func) _scenePreOps[name] = new BHScriptFunctionScene(this, &GameBountyHunter::func);
	PRE_OPS_FUNCTION("DEFAULT", _scene_pso_drawrct);
	PRE_OPS_FUNCTION("DRAW_RECT", _scene_pso_drawrct);
	PRE_OPS_FUNCTION("FADEIN", _scene_pso_fadein);
	PRE_OPS_FUNCTION("PAUSE", _scene_pso_pause);
	PRE_OPS_FUNCTION("PAUSE_FADEIN", _scene_pso_pause_fadein);
	PRE_OPS_FUNCTION("DRAW_RECT_THEN_FADEIN", _scene_pso_drawrct_fadein);
	PRE_OPS_FUNCTION("SHOOTOUT", _scene_pso_shootout);
	PRE_OPS_FUNCTION("WNDMAIN", _scene_pso_wounded_main);
	PRE_OPS_FUNCTION("GUNFIGHT", _scene_pso_gunfight_setup);
	PRE_OPS_FUNCTION("REFEREED", _scene_pso_drawrct);
	PRE_OPS_FUNCTION("LOSELIFE", _scene_pso_lose_a_life);
	PRE_OPS_FUNCTION("L1ASETUP", _scene_pso_drawrct);
	PRE_OPS_FUNCTION("L1DSETUP", _scene_pso_drawrct);
	PRE_OPS_FUNCTION("L2ASETUP", _scene_pso_setup_ndrandom1);
	PRE_OPS_FUNCTION("L2BSETUP", _scene_pso_drawrct);
	PRE_OPS_FUNCTION("L4A1SETUP", _scene_pso_drawrct);
	PRE_OPS_FUNCTION("L4A2SETUP", _scene_pso_drawrct);
	PRE_OPS_FUNCTION("SETUPL3A", _scene_pso_drawrct);
	PRE_OPS_FUNCTION("SET3SHOT", _scene_pso_drawrct);
	PRE_OPS_FUNCTION("L3BSETUP", _scene_pso_drawrct);
	PRE_OPS_FUNCTION("SETBADGUY", _scene_pso_set_current_scene);
	PRE_OPS_FUNCTION("CLRKILL3", _scene_pso_drawrct);
	PRE_OPS_FUNCTION("DPAUSE", _scene_pso_drawrct);
	PRE_OPS_FUNCTION("DEMO", _scene_pso_drawrct);
#undef PRE_OPS_FUNCTION

#define INS_OPS_FUNCTION(name, func) _sceneInsOps[name] = new BHScriptFunctionScene(this, &GameBountyHunter::func);
	INS_OPS_FUNCTION("DEFAULT", _scene_iso_donothing);
	INS_OPS_FUNCTION("PAUSE", _scene_iso_pause);
	INS_OPS_FUNCTION("SHOOTOUT", _scene_iso_shootout);
	INS_OPS_FUNCTION("LEFTDIE", _scene_iso_donothing);
	INS_OPS_FUNCTION("SHOOTPAST", _scene_iso_donothing);
	INS_OPS_FUNCTION("GUNFIGHT", _scene_iso_shootout);
	INS_OPS_FUNCTION("REFEREED", _scene_iso_donothing);
	INS_OPS_FUNCTION("CHECK3SHOT", _scene_iso_donothing);
	INS_OPS_FUNCTION("SHOWHI", _scene_iso_donothing);
	INS_OPS_FUNCTION("STARTGAME", _scene_iso_donothing);
	INS_OPS_FUNCTION("GIVEMONEY", _scene_iso_givemoney);
	INS_OPS_FUNCTION("GETHI", _scene_iso_donothing);
	INS_OPS_FUNCTION("DPAUSE", _scene_iso_donothing);
	INS_OPS_FUNCTION("DEMO", _scene_iso_donothing);
	INS_OPS_FUNCTION("RELOAD", _scene_iso_donothing);
	INS_OPS_FUNCTION("RPAUSE", _scene_iso_donothing);
#undef INS_OPS_FUNCTION

#define NXT_SCN_FUNCTION(name, func) _sceneNxtScn[name] = new BHScriptFunctionScene(this, &GameBountyHunter::func);
	NXT_SCN_FUNCTION("DEFAULT", _scene_default_nxtscn);
	NXT_SCN_FUNCTION("DIED", _scene_nxtscn_lose_a_life);
	NXT_SCN_FUNCTION("LOSE_A_LIFE", _scene_nxtscn_lose_a_life);
	NXT_SCN_FUNCTION("CONTINUE_GAME", _scene_nxtscn_continue_game);
	NXT_SCN_FUNCTION("DID_NOT_CONTINUE", _scene_nxtscn_did_not_continue);
	NXT_SCN_FUNCTION("KILL_INNOCENT_MAN", _scene_nxtscn_kill_innocent_man);
	NXT_SCN_FUNCTION("KILL_INNOCENT_WOMAN", _scene_nxtscn_kill_innocent_woman);
	NXT_SCN_FUNCTION("AFTER_DIE", _scene_nxtscn_after_die);
	NXT_SCN_FUNCTION("CHECKNEXT", _scene_nxtscn_goto_level_select);
	NXT_SCN_FUNCTION("CONTINUE_RANDOM", _scene_nxtscn_continue_random);
	NXT_SCN_FUNCTION("POPUP", _scene_nxtscn_continue_random);
	NXT_SCN_FUNCTION("L1ASETUP", _scene_nxtscn_init_random_harry1);
	NXT_SCN_FUNCTION("L1DSETUP", _scene_nxtscn_init_random_harry2);
	NXT_SCN_FUNCTION("L2ASETUP", _scene_nxtscn_init_random_dan1);
	NXT_SCN_FUNCTION("L2BSETUP", _scene_nxtscn_init_random_dan2);
	NXT_SCN_FUNCTION("L3ASETUP", _scene_nxtscn_init_random_loco1);
	NXT_SCN_FUNCTION("L3BSETUP", _scene_nxtscn_init_random_loco2);
	NXT_SCN_FUNCTION("L4A1SETUP", _scene_nxtscn_init_random_kid1);
	NXT_SCN_FUNCTION("L4A2SETUP", _scene_nxtscn_init_random_kid2);
	NXT_SCN_FUNCTION("NEXTSUB", _scene_nxtscn_next_sub_level);
	NXT_SCN_FUNCTION("GOTOBAD", _scene_nxtscn_goto_bad_guy);
	NXT_SCN_FUNCTION("AUTOSEL", _scene_nxtscn_auto_select_level);
	NXT_SCN_FUNCTION("SELECT_SCENARIO", _scene_nxtscn_select_scenario);
	NXT_SCN_FUNCTION("FINISH_SCENARIO", _scene_nxtscn_finish_scenario);
	NXT_SCN_FUNCTION("GAME_WON", _scene_nxtscn_game_won);
	NXT_SCN_FUNCTION("KILLMAN", _scene_nxtscn_kill_innocent_man);
	NXT_SCN_FUNCTION("KILLWOMAN", _scene_nxtscn_kill_innocent_woman);
	NXT_SCN_FUNCTION("BOTHDIE", _scene_nxtscn_lose_a_life);
	NXT_SCN_FUNCTION("RIGHTDIE", _scene_nxtscn_lose_a_life);
	NXT_SCN_FUNCTION("LEFTDIES", _scene_nxtscn_lose_a_life);
	NXT_SCN_FUNCTION("KILLMAIN", _scene_nxtscn_killed_main);
	NXT_SCN_FUNCTION("WNDMAIN", _scene_nxtscn_wounded_main);
	NXT_SCN_FUNCTION("ENDLEVEL", _scene_nxtscn_end_level);
	NXT_SCN_FUNCTION("ENDOGAME", _scene_nxtscn_end_game);
	NXT_SCN_FUNCTION("CLRHI", _scene_nxtscn_donothing);
	NXT_SCN_FUNCTION("TGTPRACT", _scene_nxtscn_donothing);
	NXT_SCN_FUNCTION("CREDITS", _scene_nxtscn_donothing);
	NXT_SCN_FUNCTION("DOMAIN", _scene_nxtscn_do_breakout_mains);
	NXT_SCN_FUNCTION("RDIED", _scene_nxtscn_died_refed);
	NXT_SCN_FUNCTION("GIVESHOT", _scene_nxtscn_give_shotgun);
	NXT_SCN_FUNCTION("CHK2P", _scene_nxtscn_check_2players);
	NXT_SCN_FUNCTION("SHOTSND", _scene_nxtscn_donothing);
	NXT_SCN_FUNCTION("XITCONT", _scene_nxtscn_donothing);
#undef NXT_SCN_FUNCTION

#define WEP_DWN_FUNCTION(name, func) _sceneWepDwn[name] = new BHScriptFunctionScene(this, &GameBountyHunter::func);
	WEP_DWN_FUNCTION("DEFAULT", _scene_default_wepdwn);
	WEP_DWN_FUNCTION("GUNFIGHT", _scene_default_wepdwn);
	WEP_DWN_FUNCTION("LOADALL", _scene_default_wepdwn);
#undef NXT_SCN_FUNCTION

	_sceneShowMsg["DEFAULT"] = new BHScriptFunctionScene(this, &GameBountyHunter::_scene_sm_donothing);
	_sceneScnScr["DEFAULT"] = new BHScriptFunctionScene(this, &GameBountyHunter::_scene_default_score);
	_sceneNxtFrm["DEFAULT"] = new BHScriptFunctionScene(this, &GameBountyHunter::_scene_nxtfrm);
}

void GameBountyHunter::verifyScriptFunctions() {
	Common::Array<Scene *> *scenes = _sceneInfo->getScenes();
	for (size_t i = 0; i < scenes->size(); i++) {
		Scene *scene = (*scenes)[i];
		getScriptFunctionScene(PREOP, scene->preop);
		getScriptFunctionScene(SHOWMSG, scene->scnmsg);
		getScriptFunctionScene(INSOP, scene->insop);
		getScriptFunctionScene(WEPDWN, scene->wepdwn);
		getScriptFunctionScene(SCNSCR, scene->scnscr);
		getScriptFunctionScene(NXTFRM, scene->nxtfrm);
		getScriptFunctionScene(NXTSCN, scene->nxtscn);
		for (size_t j = 0; j < scene->zones.size(); j++) {
			Zone *zone = scene->zones[j];
			for (size_t k = 0; k < zone->rects.size(); k++) {
				getScriptFunctionRectHit(zone->rects[k].rectHit);
			}
		}
	}
}

BHScriptFunctionRect GameBountyHunter::getScriptFunctionRectHit(Common::String name) {
	BHScriptFunctionRectMap::iterator it = _rectHitFuncs.find(name);
	if (it != _rectHitFuncs.end()) {
		return (*(*it)._value);
	} else {
		error("Could not find rectHit function: %s", name.c_str());
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
		error("Unkown scene script type: %u", type);
		break;
	}
	BHScriptFunctionSceneMap::iterator it;
	it = functionMap->find(name);
	if (it != functionMap->end()) {
		return (*(*it)._value);
	} else {
		error("Could not find scene type %u function: %s", type, name.c_str());
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
	_NewGame();
	_cur_scene = _startscene;
	Common::String oldscene;
	while (!_vm->shouldQuit()) {
		oldscene = _cur_scene;
		_SetFrame();
		_fired = false;
		Scene *scene = _sceneInfo->findScene(_cur_scene);
		if (!loadScene(scene)) {
			error("Cannot find scene %s in libfile", scene->name.c_str());
		}
		_sceneSkipped = false;
		Audio::PacketizedAudioStream *audioStream = _videoDecoder->getAudioStream();
		g_system->getMixer()->stopHandle(_sceneAudioHandle);
		g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &_sceneAudioHandle, audioStream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
		_paletteDirty = true;
		_nextFrameTime = _GetMsTime() + 100;
		callScriptFunctionScene(PREOP, scene->preop, scene);
		_currentFrame = _GetFrame(scene);
		while (_currentFrame <= scene->endFrame && _cur_scene == oldscene && !_vm->shouldQuit()) {
			_UpdateMouse();
			callScriptFunctionScene(SHOWMSG, scene->scnmsg, scene);
			callScriptFunctionScene(INSOP, scene->insop, scene);
			_holster = _WeaponDown();
			if (_holster) {
				callScriptFunctionScene(WEPDWN, scene->wepdwn, scene);
			}
			Common::Point firedCoords;
			if (__Fired(&firedCoords)) {
				if (!_holster) {
					Rect *hitGlobalRect = _CheckZone(_menuzone, &firedCoords);
					if (hitGlobalRect != nullptr) {
						callScriptFunctionRectHit(hitGlobalRect->rectHit, hitGlobalRect);
					} else {
						if (_playerShots[_player] > 0) {
							if (!_debug_unlimitedAmmo) {
								_playerShots[_player]--;
							}
							_DisplayShotFiredImage(&firedCoords);
							_DoShot();
							Rect *hitRect = nullptr;
							Zone *hitSceneZone = _CheckZonesV2(scene, hitRect, &firedCoords);
							if (hitSceneZone != nullptr) {
								callScriptFunctionRectHit(hitRect->rectHit, hitRect);
							} else {
								int8 skip = _SkipToNewScene(scene);
								if (skip == -1) {
									callScriptFunctionScene(NXTSCN, scene->nxtscn, scene);
								} else if (skip == 1) {
									if (scene->dataParam4 > 0) {
										uint32 framesToSkip = (scene->dataParam4 - _currentFrame) / _videoFrameSkip;
										_videoDecoder->skipNumberOfFrames(framesToSkip);
									} else {
										callScriptFunctionScene(NXTSCN, scene->nxtscn, scene);
									}
								}
							}
						} else {
							_PlaySound(_emptySound);
						}
					}
				}
			}
			if (_cur_scene == oldscene) {
				callScriptFunctionScene(NXTFRM, scene->nxtfrm, scene);
			}
			_DisplayLivesLeft(0);
			_DisplayScores(0);
			_DisplayShotsLeft(0);
			_MoveMouse();
			if (_pauseTime > 0) {
				g_system->getMixer()->pauseHandle(_sceneAudioHandle, true);
			} else {
				g_system->getMixer()->pauseHandle(_sceneAudioHandle, false);
			}
			if (_videoDecoder->getCurrentFrame() == 0) {
				_videoDecoder->getNextFrame();
			}
			int32 remainingMillis = _nextFrameTime - _GetMsTime();
			if (remainingMillis < 10) {
				if (_videoDecoder->getCurrentFrame() > 0) {
					_videoDecoder->getNextFrame();
				}
				remainingMillis = _nextFrameTime - _GetMsTime();
				_nextFrameTime = _GetMsTime() + (remainingMillis > 0 ? remainingMillis : 0) + 100;
			}
			if (remainingMillis > 0) {
				if (remainingMillis > 15) {
					remainingMillis = 15;
				}
				g_system->delayMillis(remainingMillis);
			}
			_currentFrame = _GetFrame(scene);
			updateScreen();
		}
		// frame limit reached or scene changed, prepare for next scene
		_hadPause = false;
		_pauseTime = 0;
		if (_cur_scene == oldscene) {
			callScriptFunctionScene(NXTSCN, scene->nxtscn, scene);
		}
		if (_cur_scene == "") {
			_vm->quitGame();
		}
	}
	return Common::kNoError;
}

void GameBountyHunter::_NewGame() {
	_playerLives[0] = _playerLives[1] = 3;
	_playerShots[0] = _playerShots[1] = 6;
	_playerGun[0] = _playerGun[1] = 1;
	_playerScore[0] = _playerScore[1] = 0;
	_currentSubLevelSceneId = 0x017B;
	_holster = false;
}

void GameBountyHunter::_DoMenu() {
	uint32 startTime = _GetMsTime();
	_RestoreCursor();
	_DoCursor();
	_inMenu = true;
	_MoveMouse();
	g_system->getMixer()->pauseHandle(_sceneAudioHandle, true);
	_screen->copyRectToSurface(_background->getBasePtr(_videoPosX, _videoPosY), _background->pitch, _videoPosX, _videoPosY, _videoDecoder->getWidth(), _videoDecoder->getHeight());
	while (_inMenu && !_vm->shouldQuit()) {
		Common::Point firedCoords;
		if (__Fired(&firedCoords)) {
			Rect *hitMenuRect = _CheckZone(_submenzone, &firedCoords);
			if (hitMenuRect != nullptr) {
				callScriptFunctionRectHit(hitMenuRect->rectHit, hitMenuRect);
			}
		}
		g_system->delayMillis(15);
		updateScreen();
	}
	_RestoreCursor();
	_DoCursor();
	g_system->getMixer()->pauseHandle(_sceneAudioHandle, false);
	if (_hadPause) {
		unsigned long endTime = _GetMsTime();
		unsigned long timeDiff = endTime - startTime;
		_pauseTime += timeDiff;
		_nextFrameTime += timeDiff;
	}
}

void GameBountyHunter::_DoCursor() {
	_UpdateMouse();
}

void GameBountyHunter::_UpdateMouse() {
	if (_oldWhichGun != _whichGun) {
		Graphics::PixelFormat pixelFormat = Graphics::PixelFormat::createFormatCLUT8();
		Graphics::Surface *cursor = &(*_gun)[_whichGun];
		if (_playerGun[0] == 2 && _whichGun < 2) {
			cursor = &(*_shotgun)[_whichGun];
		}
		CursorMan.popAllCursors();
		uint16 hotspotX = (cursor->w / 2) + 8;
		uint16 hotspotY = (cursor->h / 2) + 8;
		if (debugChannelSet(1, Alg::kAlgDebugGraphics)) {
			cursor->drawLine(0, hotspotY, cursor->w, hotspotY, 1);
			cursor->drawLine(hotspotX, 0, hotspotX, cursor->h, 1);
		}
		CursorMan.pushCursor(cursor->getPixels(), cursor->w, cursor->h, hotspotX, hotspotY, 0, false, &pixelFormat);
		CursorMan.showMouse(true);
		_oldWhichGun = _whichGun;
	}
}

void GameBountyHunter::_MoveMouse() {
	if (_inMenu) {
		_whichGun = 3; // in menu cursor
	} else {
		// disabled for now, because glitchy
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
	_UpdateMouse();
}

void GameBountyHunter::_DisplayLivesLeft(uint8 player) {
	if (_lives == _oldLives) {
		return;
	}
	int posY = 0x67;
	for (uint8 i = 0; i < 3; i++) {
		AlgGraphics::drawImage(_screen, &_deadIcon, 0x12F, posY);
		posY += 0xE;
	}
	posY = 0x67;
	for (uint8 i = 0; i < _lives; i++) {
		AlgGraphics::drawImage(_screen, &_liveIcon, 0x12F, posY);
		posY += 0xE;
	}
	_oldLives = _lives;
}

void GameBountyHunter::_DisplayScores(uint8 player) {
	if (_score == _oldScore) {
		return;
	}
	Common::String scoreString = Common::String::format("%05d", _score);
	int posX = 0x9B;
	for (int i = 0; i < 5; i++) {
		uint8 digit = scoreString[i] - '0';
		AlgGraphics::drawImage(_screen, &(*_numbers)[digit], posX, 0xBF);
		posX += 7;
	}
	_oldScore = _score;
}

void GameBountyHunter::_DisplayShotsLeft(uint8 player) {
	if (_shots == _oldShots) {
		return;
	}
	uint16 posX = 0xEE;
	for (uint8 i = 0; i < 10; i++) {
		AlgGraphics::drawImage(_screen, &_emptyIcon, posX, 0xBE);
		posX += 5;
	}
	posX = 0xEE;
	for (uint8 i = 0; i < _shots; i++) {
		AlgGraphics::drawImage(_screen, &_shotIcon, posX, 0xBE);
		posX += 5;
	}
	_oldShots = _shots;
}

bool GameBountyHunter::_WeaponDown() {
	if (_rightDown && _mousePos.y >= 0xAA && _mousePos.x >= 0x113) {
		return true;
	}
	return false;
}

bool GameBountyHunter::_SaveState() {
	Common::OutSaveFile *outSaveFile;
	Common::String saveFileName = _vm->getSaveStateName(0);
	if (!(outSaveFile = g_system->getSavefileManager()->openForSaving(saveFileName))) {
		warning("Can't create file '%s', game not saved", saveFileName.c_str());
		return false;
	}
	uint16 currentSceneNum = atoi(_cur_scene.c_str());
	outSaveFile->writeUint32BE(MKTAG('A', 'L', 'G', 'S')); // header
	outSaveFile->writeByte(0);                             // version, unused for now
	outSaveFile->writeByte(_currentLevel);
	outSaveFile->writeUint16LE(_currentSubLevelSceneId);
	outSaveFile->writeByte(_continuesUsed);
	outSaveFile->writeUint16LE(currentSceneNum);
	for (uint8 i = 0; i < 2; i++) {
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

bool GameBountyHunter::_LoadState() {
	Common::InSaveFile *inSaveFile;
	Common::String saveFileName = _vm->getSaveStateName(0);
	if (!(inSaveFile = g_system->getSavefileManager()->openForLoading(saveFileName))) {
		debug("Can't load file '%s', game not loaded", saveFileName.c_str());
		return false;
	}
	uint32 header = inSaveFile->readUint32BE();
	if (header != MKTAG('A', 'L', 'G', 'S')) {
		warning("Unkown save file, header: %d", header);
		return false;
	}
	inSaveFile->skip(1); // version, unused for now
	_currentLevel = inSaveFile->readByte();
	_currentSubLevelSceneId = inSaveFile->readUint16LE();
	_continuesUsed = inSaveFile->readByte();
	_restartScene = inSaveFile->readUint16LE();
	for (uint8 i = 0; i < 2; i++) {
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
void GameBountyHunter::_SetNextScene(uint16 sceneId) {
	_cur_scene = Common::String::format("%d", sceneId);
}

void GameBountyHunter::_DisplayShotFiredImage(Common::Point *point) {
	if (point->x >= _videoPosX && point->x <= (_videoPosX + _videoDecoder->getWidth()) && point->y >= _videoPosY && point->y <= (_videoPosY + _videoDecoder->getHeight())) {
		uint16 targetX = point->x - _videoPosX;
		uint16 targetY = point->y - _videoPosY;
		AlgGraphics::drawImageCentered(_videoDecoder->getVideoFrame(), &_bulletholeIcon, targetX, targetY);
	}
}

void GameBountyHunter::_EnableVideoFadeIn() {
	// TODO implement
}

void GameBountyHunter::_IconShotgun() {
	// TODO implement
}

void GameBountyHunter::_IconReset() {
	// TODO implement
}

uint16 GameBountyHunter::_BeginLevel(uint8 levelNumber) {
	_currentLevel = levelNumber;
	_numSubLevelsDone = 0;
	int index = (levelNumber * 24) + (_numLevelsDone * 6) + _numSubLevelsDone;
	uint8 subLevel = _subLevelOrder[index];
	uint16 sceneIndex = (_currentLevel * 5) + subLevel;
	uint16 sceneNum = _subLevelSceneIds[sceneIndex];
	_currentSubLevelSceneId = sceneNum;
	return sceneNum;
}

uint16 GameBountyHunter::_RandomUnusedInt(uint8 max, uint16 *mask, uint16 exclude) {
	if (max == 1) {
		return 0;
	}
	// reset mask if full
	uint16 fullMask = 0xFFFF >> (16 - max);
	if (*mask == fullMask) {
		*mask = 0;
	}
	uint16 random = 0;
	// find an unused random number
	while (1) {
		random = _rnd->getRandomNumber(max - 1);
		// check if bit is already used
		unsigned int bit = 1 << random;
		if (!((*mask & bit) || random == exclude)) {
			// set the bit in mask
			*mask |= bit;
			break;
		}
	}
	return random;
}

uint16 GameBountyHunter::_PickRandomScene(const uint16 *sceneList, uint8 max) {
	if (max == 0) {
		return 0;
	}
	_random_scene_list = const_cast<uint16 *>(sceneList);
	_random_max = max;
	_random_mask = 0;
	_random_picked = -1;
	_random_scene_count = 0;
	while (_random_scene_list[_random_scene_count] != 0) {
		_random_scene_count++;
	}
	unsigned short count = _random_max--;
	if (count > 0) {
		_random_picked = _RandomUnusedInt(_random_scene_count, &_random_mask, _random_picked);
		return _random_scene_list[_random_picked];
	}
	return 0;
}

uint16 GameBountyHunter::_PickGunfightScene() {
	if (!_gunfight_initialized) {
		_gunfight_initialized = true;
		_gunfight_mask = 0;
		_gunfight_picked = -1;
		_gunfight_scene_count = 0;
		while (_gunfightScenarios[_gunfight_scene_count] != 0) {
			_gunfight_scene_count++;
		}
	}
	_random_picked = _RandomUnusedInt(_gunfight_scene_count, &_gunfight_mask, _random_picked);
	return _gunfightScenarios[_random_picked];
}

uint16 GameBountyHunter::_PickInnocentScene() {
	if (!_innocent_initialized) {
		_innocent_initialized = true;
		_innocent_mask = 0;
		_innocent_picked = -1;
		_innocent_scene_count = 0;
		while (_innocentScenarios[_innocent_scene_count] != 0) {
			_innocent_scene_count++;
		}
	}
	_innocent_picked = _RandomUnusedInt(_innocent_scene_count, &_innocent_mask, _innocent_picked);
	return _innocentScenarios[_innocent_picked];
}

uint16 GameBountyHunter::_PickDeathScene() {
	if (!_death_initialized) {
		_death_initialized = true;
		_death_mask = 0;
		_death_picked = -1;
		_death_scene_count = 0;
		while (_deathScenarios[_death_scene_count] != 0) {
			_death_scene_count++;
		}
	}
	_death_picked = _RandomUnusedInt(_death_scene_count, &_death_mask, _death_picked);
	return _deathScenarios[_death_picked];
}

uint16 GameBountyHunter::_TimeForGunfight() {
	uint16 picked = 0;
	if (--_gunfightCount <= 0) {
		int index = (_unk_2ADA6 * 5) + (_numLevelsDone);
		_gunfightCount = _gunfightCountDown[index];
		picked = _PickGunfightScene();
	}
	return picked;
}

void GameBountyHunter::_WaitingForShootout(uint32 drawFrame) {
	if (drawFrame != 0) {
		for (uint8 i = 0; i < _numPlayers; i++) {
			_firstDrawFrame = drawFrame;
			_playerShots[i] = 0;
			_playerGun[i] = 0;
			_DisplayShotsLeft(i);
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

void GameBountyHunter::_DoShotgunSound() {
	_PlaySound(_shotgunSound);
}

// Script functions: RectHit
void GameBountyHunter::_rect_shotmenu(Rect *rect) {
	_DoMenu();
}

void GameBountyHunter::_rect_save(Rect *rect) {
	if (_SaveState()) {
		_DoSaveSound();
	}
}

void GameBountyHunter::_rect_load(Rect *rect) {
	if (_LoadState()) {
		_DoLoadSound();
	}
	_SetNextScene(_restartScene);
	_restartScene = 0;
}

void GameBountyHunter::_rect_continue(Rect *rect) {
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
				_scene_nxtscn_did_not_continue(nullptr);
			}
		}
	}
}

void GameBountyHunter::_rect_start(Rect *rect) {
	_inMenu = false;
	_fired = false;
	_gameInProgress = true;
	_restartScene = 0;
	if (_isDemo) {
		_SetNextScene(0x45); // TODO fix
	} else {
		_SetNextScene(0x45);
	}
	_NewGame();
}

void GameBountyHunter::_rect_toggle_players(Rect *rect) {
	if (_numPlayers == 1) {
		_numPlayers = 2;
		AlgGraphics::drawImage(_screen, &_playersIcon2, 0xCE, 0x95);
		AlgGraphics::drawImage(_screen, &_textBlackBarIcon, 0x78, 0xBF);
		AlgGraphics::drawImage(_screen, &_textBlackBarIcon, 0x0C, 0xBF);
		_DisplayShotsLeft(1);
		_DisplayLivesLeft(1);
	} else {
		_numPlayers = 1;
		AlgGraphics::drawImage(_screen, &_playersIcon1, 0xCE, 0x95);
		AlgGraphics::drawImage(_screen, &_textScoreIcon, 0x78, 0xBF);
		AlgGraphics::drawImage(_screen, &_textMenuIcon, 0x0C, 0xBF);
		AlgGraphics::drawImage(_screen, &_textBlackBarIcon, 0x50, 0xBE);
	}
	_DoSkullSound();
	_screen->copyRectToSurface(_background->getBasePtr(_videoPosX, _videoPosY), _background->pitch, _videoPosX, _videoPosY, _videoDecoder->getWidth(), _videoDecoder->getHeight());
}

void GameBountyHunter::_rect_hit_icon_jug(Rect *rect) {
	// TODO fix
	// Icon.funcs[1](param);
	// PlaySound(4);
	// Icon.hitCount++;
}

void GameBountyHunter::_rect_hit_icon_lantern(Rect *rect) {
	// TODO fix
	// Icon.funcs[3](param);
	// PlaySound(4);
	// Icon.hitCount++;
}

void GameBountyHunter::_rect_hit_icon_skull(Rect *rect) {
	// TODO fix
	// Icon.funcs[0](param);
	// PlaySound(4);
	// Icon.hitCount++;
}

void GameBountyHunter::_rect_hit_icon_wheel(Rect *rect) {
	// TODO fix
	// Icon.funcs[2](param);
	// PlaySound(4);
	// Icon.hitCount++;
}

void GameBountyHunter::_rect_hit_select_harry(Rect *rect) {
	if (!(_levelDoneMask & 2)) {
		uint16 picked = _BeginLevel(0);
		_SetNextScene(picked);
	}
}

void GameBountyHunter::_rect_hit_select_dan(Rect *rect) {
	if (!(_levelDoneMask & 4)) {
		uint16 picked = _BeginLevel(1);
		_SetNextScene(picked);
	}
}

void GameBountyHunter::_rect_hit_select_loco(Rect *rect) {
	if (!(_levelDoneMask & 8)) {
		uint16 picked = _BeginLevel(2);
		_SetNextScene(picked);
	}
}

void GameBountyHunter::_rect_hit_select_kid(Rect *rect) {
	if (!(_levelDoneMask & 0x10)) {
		uint16 picked = _BeginLevel(3);
		_SetNextScene(picked);
	}
}

void GameBountyHunter::_rect_hit_kill_man(Rect *rect) {
	// do nothing
}

void GameBountyHunter::_rect_hit_give_shotgun(Rect *rect) {
	_IconShotgun();
}

void GameBountyHunter::_rect_hit_kill3(Rect *rect) {
	_count++;
	if (_count == 3) {
		_count = 0;
		_rect_newscene(rect);
		// TODO verify
		// _RHONewScene(param1, param2);
	}
}

void GameBountyHunter::_rect_hit_check_shotgun(Rect *rect) {
	if (_playerGun[_player] == 2) {
		_rect_newscene(rect);
	}
}

void GameBountyHunter::_rect_hit_cheater(Rect *rect) {
	_SetNextScene(0x011A);
}

// Script functions: Scene PreOps
void GameBountyHunter::_scene_pso_shootout(Scene *scene) {
	_WaitingForShootout(atoi(scene->preopParam.c_str()));
}

void GameBountyHunter::_scene_pso_wounded_main(Scene *scene) {
	_wounded = true;
	_currentSubLevelSceneId = _moneyScenes[_currentLevel];
}

void GameBountyHunter::_scene_pso_gunfight_setup(Scene *scene) {
	_WaitingForShootout(atoi(scene->preopParam.c_str()));
}

void GameBountyHunter::_scene_pso_lose_a_life(Scene *scene) {
	_scene_nxtscn_lose_a_life(scene);
}

void GameBountyHunter::_scene_pso_setup_ndrandom1(Scene *scene) {
	_numSubLevelsDone++;
}

void GameBountyHunter::_scene_pso_set_current_scene(Scene *scene) {
	int sceneId = atoi(scene->preopParam.c_str());
	_currentSubLevelSceneId = sceneId;
	if (sceneId == 0) {
		uint8 index = (_currentLevel * 24) + (_numLevelsDone * 6) + _numSubLevelsDone;
		uint8 subLevel = _subLevelOrder[index];
		uint16 picked = (_currentLevel * 20) + (subLevel * 4);
		_currentSubLevelSceneId = 0x0D32 + picked;
	}
}

// Script functions: Scene InsOps
void GameBountyHunter::_scene_iso_shootout(Scene *scene) {
	_WaitingForShootout(0);
}

void GameBountyHunter::_scene_iso_givemoney(Scene *scene) {
	const int moneyFrames[] = {0x1E8F, 0x3BB4, 0x7814, 0xA287};
	const int woundBits[] = {2, 4, 8, 0x10};
	for (uint8 i = 0; i < _numPlayers; i++) {
		if (_currentLevel <= 3) {
			unsigned long moneyFrame = moneyFrames[_currentLevel];
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
				_wounded = 0;
				_given = 1;
			} else if (moneyFrame != _currentFrame) {
				_given = 0;
			}
		}
		_DisplayScores(i);
	}
}

// Script functions: Scene NxtScn
void GameBountyHunter::_scene_nxtscn_lose_a_life(Scene *scene) {
	uint16 picked = 0;
	int deadPlayerCount = 0;
	for (int i = 0; i < _numPlayers; i++) {
		_playerLives[i]--;
		_DisplayLivesLeft(i);
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
		picked = _PickDeathScene();
	}
	_SetNextScene(picked);
}

void GameBountyHunter::_scene_nxtscn_continue_game(Scene *scene) {
	if (_continuesUsed < 2) {
		_SetNextScene(0x01B6);
	} else {
		_scene_nxtscn_did_not_continue(scene);
	}
}

void GameBountyHunter::_scene_nxtscn_did_not_continue(Scene *scene) {
	_gameInProgress = false;
	_cur_scene = _startscene;
}

void GameBountyHunter::_scene_nxtscn_kill_innocent_man(Scene *scene) {
	uint16 picked = 0;
	_playerLives[_player]--;
	if (_playerLives[_player]) {
		picked = _PickInnocentScene();
	} else {
		if (_numPlayers == 2) {
			picked = _onePlayerOfTwoDead[_numSubLevelsDone & 1];
		} else {
			picked = _allPlayersDead;
		}
	}
	_SetNextScene(picked);
}

void GameBountyHunter::_scene_nxtscn_kill_innocent_woman(Scene *scene) {
	_scene_nxtscn_kill_innocent_man(scene);
}

void GameBountyHunter::_scene_nxtscn_after_die(Scene *scene) {
	for (uint8 i = 0; i < _numPlayers; i++) {
		if (_playerLives[i] <= 0) {
			_playerLives[i] = 3;
			_DisplayLivesLeft(i);
		}
	}
	_SetNextScene(_currentSubLevelSceneId);
}

void GameBountyHunter::_scene_nxtscn_goto_level_select(Scene *scene) {
	_IconReset();
	uint16 picked = 0;
	if ((_levelDoneMask & 0x1E) != 0x1E) {
		picked = 0x17B;
	} else if (!(_levelDoneMask & 0x80)) {
		picked = 0x66;
	} else {
		picked = 0x61;
	}
	_SetNextScene(picked);
}

void GameBountyHunter::_scene_nxtscn_continue_random(Scene *scene) {
	// TODO verify
	_scene_nxtscn_next_sub_level(scene);
	/*
	uint16 picked = _PickRandomScene(0, 0);
	if (picked == 0) {
		_scene_nxtscn_next_sub_level(scene);
		return;
	}
	_SetNextScene(picked);
	*/
}

void GameBountyHunter::_scene_nxtscn_init_random_harry1(Scene *scene) {
	uint16 picked = _PickRandomScene(_randomScenes[0], _randomScenesPicks[0]);
	_currentSubLevelSceneId = picked;
	_SetNextScene(picked);
}

void GameBountyHunter::_scene_nxtscn_init_random_harry2(Scene *scene) {
	uint16 picked = _PickRandomScene(_randomScenes[1], _randomScenesPicks[1]);
	_SetNextScene(picked);
}

void GameBountyHunter::_scene_nxtscn_init_random_dan1(Scene *scene) {
	uint16 picked = 0;
	uint8 picks = _randomScenesPicks[2] + _numPlayers;
	if (_numPlayers == 2) {
		picked = _PickRandomScene(_randomDan1TwoPlayer, picks);
	} else {
		picked = _PickRandomScene(_randomScenes[2], picks);
	}
	_currentSubLevelSceneId = 0x0174;
	_SetNextScene(picked);
}

void GameBountyHunter::_scene_nxtscn_init_random_dan2(Scene *scene) {
	uint16 picked = _PickRandomScene(_randomScenes[3], _randomScenesPicks[3]);
	_currentSubLevelSceneId = picked;
	_SetNextScene(picked);
}

void GameBountyHunter::_scene_nxtscn_init_random_loco1(Scene *scene) {
	uint16 picked = _PickRandomScene(_randomScenes[4], _randomScenesPicks[4]);
	_SetNextScene(picked);
}

void GameBountyHunter::_scene_nxtscn_init_random_loco2(Scene *scene) {
	uint16 picked = _PickRandomScene(_randomScenes[5], _randomScenesPicks[5]);
	_SetNextScene(picked);
}

void GameBountyHunter::_scene_nxtscn_init_random_kid1(Scene *scene) {
	uint16 picked = _PickRandomScene(_randomScenes[6], _randomScenesPicks[6]);
	_SetNextScene(picked);
}

void GameBountyHunter::_scene_nxtscn_init_random_kid2(Scene *scene) {
	uint16 picked = _PickRandomScene(_randomScenes[7], _randomScenesPicks[7]);
	_SetNextScene(picked);
}

void GameBountyHunter::_scene_nxtscn_next_sub_level(Scene *scene) {
	_IconReset();
	_numSubLevelsDone++;
	int index = (_currentLevel * 24) + (_numLevelsDone * 6) + _numSubLevelsDone;
	uint8 subLevel = _subLevelOrder[index];
	uint16 sceneIndex = (_currentLevel * 5) + subLevel;
	uint16 picked = _subLevelSceneIds[sceneIndex];
	_currentSubLevelSceneId = picked;
	uint16 gunfightScene = _TimeForGunfight();
	if (gunfightScene != 0) {
		_SetNextScene(gunfightScene);
	}
	if (subLevel == 2) {
		if (_currentLevel == 0) {
			_SetNextScene(picked);
			return;
		}
		picked = _clueLevels[_currentLevel];
	}
	_SetNextScene(picked);
}

void GameBountyHunter::_scene_nxtscn_goto_bad_guy(Scene *scene) {
	_IconReset();
	uint8 index = (_currentLevel * 24) + (_numLevelsDone * 6) + _numSubLevelsDone;
	uint8 subLevel = _subLevelOrder[index];
	uint16 sceneIndex = (_currentLevel * 5) + subLevel;
	uint16 picked = _subLevelSceneIds[sceneIndex];
	_SetNextScene(picked);
}

void GameBountyHunter::_scene_nxtscn_auto_select_level(Scene *scene) {
	_IconReset();
	uint8 i;
	for (i = 0; i < 4; i++) {
		if (!(_levelDoneMask & _mainLevelMasks[i])) {
			break;
		}
	}
	uint16 picked = _BeginLevel(i);
	_SetNextScene(picked);
}

void GameBountyHunter::_scene_nxtscn_select_scenario(Scene *scene) {
	_SetNextScene(_currentLevel);
}

void GameBountyHunter::_scene_nxtscn_finish_scenario(Scene *scene) {
	// do nothing
}

void GameBountyHunter::_scene_nxtscn_game_won(Scene *scene) {
	_gameInProgress = false;
	_cur_scene = _startscene;
}

void GameBountyHunter::_scene_nxtscn_killed_main(Scene *scene) {
	_wounded = false;
	_sub_scene = "scene379";
}

void GameBountyHunter::_scene_nxtscn_wounded_main(Scene *scene) {
	_wounded = true;
	_sub_scene = "scene379";
}

void GameBountyHunter::_scene_nxtscn_end_level(Scene *scene) {
	switch (_currentLevel) {
	case 0:
		_levelDoneMask |= 0x02;
		break;
	case 1:
		_levelDoneMask |= 0x04;
		break;
	case 2:
		_levelDoneMask |= 0x08;
		break;
	case 3:
		_levelDoneMask |= 0x10;
		break;
	default:
		return;
	}
	_numLevelsDone++;
	if (_numLevelsDone > 1 && _unk_2ADA6 < 2) {
		_unk_2ADA6++;
	}
	_numSubLevelsDone = 0;
	_currentSubLevelSceneId = 0;
	if (_numLevelsDone == 4) {
		_SetNextScene(0x66);
		return;
	}
	_SetNextScene(0x017B);
}

void GameBountyHunter::_scene_nxtscn_end_game(Scene *scene) {
	_gameInProgress = false;
	_cur_scene = _startscene;
}

void GameBountyHunter::_scene_nxtscn_do_breakout_mains(Scene *scene) {
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
	_SetNextScene(picked);
}

void GameBountyHunter::_scene_nxtscn_died_refed(Scene *scene) {
	uint16 picked = 0;
	uint8 deadCount = 0;
	for (int i = 0; i < _numPlayers; i++) {
		_playerLives[i]--;
		_DisplayLivesLeft(i);
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
	_SetNextScene(picked);
}

void GameBountyHunter::_scene_nxtscn_give_shotgun(Scene *scene) {
	for (uint8 i = 0; i < _numPlayers; i++) {
		_playerShots[i] = 5;
		_playerGun[i] = 2;
		_DisplayShotsLeft(i);
	}
}

void GameBountyHunter::_scene_nxtscn_check_2players(Scene *scene) {
	if (_numPlayers == 2) {
		_SetNextScene(0x98);
		return;
	}
	_SetNextScene(0x99);
}

// Script functions: WepDwn
void GameBountyHunter::_scene_default_wepdwn(Scene *scene) {
	if (_playerGun[_player] == 2 && _playerShots[_player] < 3) {
		_playerGun[_player] = 1;
	}
	if (_playerGun[_player] == 1 && _playerShots[_player] < 6) {
		_playerShots[_player] = 6;
	}
}

// Debug methods
void GameBountyHunter::debug_warpTo(int val) {
	// TODO implement
}

// Debugger methods
DebuggerBountyHunter::DebuggerBountyHunter(GameBountyHunter *game) : GUI::Debugger() {
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
		_game->debug_warpTo(val);
		return false;
	}
}

bool DebuggerBountyHunter::cmdDumpLib(int argc, const char **argv) {
	return _game->debug_dumpLibFile();
}

} // End of namespace Alg
