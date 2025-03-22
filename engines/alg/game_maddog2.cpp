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

#include "alg/game_maddog2.h"
#include "alg/graphics.h"
#include "alg/scene.h"

namespace Alg {

GameMaddog2::GameMaddog2(AlgEngine *vm, const ADGameDescription *desc) : Game(vm) {
	if (scumm_stricmp(desc->gameId, "maddog2s") == 0) {
		_libFileName = "maddog2.lib";
	} else if (scumm_stricmp(desc->gameId, "maddog2d") == 0) {
		_libFileName = "maddog2d.lib";
	}
}

GameMaddog2::~GameMaddog2() {
}

void GameMaddog2::init() {
	_videoPosX = 11;
	_videoPosY = 2;

	_SetupCursorTimer();

	loadLibArchive(_libFileName);
	_sceneInfo->loadScnFile("maddog2.scn");
	_startscene = _sceneInfo->getStartScene();

	registerScriptFunctions();
	verifyScriptFunctions();

	_menuzone = new Zone();
	_menuzone->name = "MainMenu";
	_menuzone->ptrfb = "GLOBALHIT";

	_menuzone->addRect(0x0C, 0xAA, 0x38, 0xC7, nullptr, 0, "SHOTMENU", "0");
	_menuzone->addRect(0x08, 0xA9, 0x013C, 0xC7, nullptr, 0, "DEFAULT", "0"); // _mm_bott

	_submenzone = new Zone();
	_submenzone->name = "SubMenu";
	_submenzone->ptrfb = "GLOBALHIT";

	_submenzone->addRect(0x2F, 0x16, 0x64, 0x2B, nullptr, 0, "STARTMENU", "0");
	_submenzone->addRect(0x2F, 0xA0, 0x8D, 0xC7, nullptr, 0, "CONTMENU", "0");
	_submenzone->addRect(0x2F, 0x40, 0x64, 0x54, nullptr, 0, "RECTSAVE", "0");
	_submenzone->addRect(0x2F, 0x6E, 0x7B, 0x86, nullptr, 0, "RECTLOAD", "0");
	_submenzone->addRect(0xEC, 0x15, 0x0122, 0x2C, nullptr, 0, "EXITMENU", "0");
	_submenzone->addRect(0xAD, 0x58, 0xF2, 0x70, nullptr, 0, "RECTEASY", "0");
	_submenzone->addRect(0xBC, 0x78, 0xF2, 0x93, nullptr, 0, "RECTAVG", "0");
	_submenzone->addRect(0xB8, 0x9D, 0xF2, 0xC7, nullptr, 0, "RECTHARD", "0");

	_shotSound = _LoadSoundFile("blow.8b");
	_emptySound = _LoadSoundFile("empty.8b");
	_saveSound = _LoadSoundFile("saved.8b");
	_loadSound = _LoadSoundFile("loaded.8b");
	_skullSound = _LoadSoundFile("skull.8b");

	_gun = AlgGraphics::loadScreenCoordAniImage("gun.ani", _palette);
	_numbers = AlgGraphics::loadAniImage("numbers.ani", _palette);
	Common::Array<Graphics::Surface> *bullet = AlgGraphics::loadAniImage("bullet.ani", _palette);
	_shotIcon = (*bullet)[0];
	_emptyIcon = (*bullet)[1];
	Common::Array<Graphics::Surface> *hat = AlgGraphics::loadAniImage("hat.ani", _palette);
	_liveIcon = (*hat)[0];
	_deadIcon = (*hat)[1];
	Common::Array<Graphics::Surface> *shootout = AlgGraphics::loadAniImage("shootout.ani", _palette);
	_reloadIcon = (*shootout)[0];
	_drawIcon = (*shootout)[1];
	Common::Array<Graphics::Surface> *knife = AlgGraphics::loadScreenCoordAniImage("knife.ani", _palette);
	_knifeIcon = (*knife)[0];
	Common::Array<Graphics::Surface> *hole = AlgGraphics::loadAniImage("hole.ani", _palette);
	_bulletholeIcon = (*hole)[0];

	_background = AlgGraphics::loadVgaBackground("backgrnd.vga", _palette);
	_screen->copyRectToSurface(_background->getPixels(), _background->pitch, 0, 0, _background->w, _background->h);

	_MoveMouse();
}

void GameMaddog2::registerScriptFunctions() {
#define ZONE_PTRFB_FUNCTION(name, func) _zonePtrFb[name] = new MD2ScriptFunctionPoint(this, &GameMaddog2::func);
	ZONE_PTRFB_FUNCTION("DEFAULT", _zone_bullethole);
	ZONE_PTRFB_FUNCTION("BULLETHOLE", _zone_bullethole);
	ZONE_PTRFB_FUNCTION("SKULL", _zone_skullhole);
#undef ZONE_PTRFB_FUNCTION

#define RECT_HIT_FUNCTION(name, func) _rectHitFuncs[name] = new MD2ScriptFunctionRect(this, &GameMaddog2::func);
	RECT_HIT_FUNCTION("DEFAULT", _rect_newscene);
	RECT_HIT_FUNCTION("NEWSCENE", _rect_newscene);
	RECT_HIT_FUNCTION("EXITMENU", _rect_exit);
	RECT_HIT_FUNCTION("CONTMENU", _rect_continue);
	RECT_HIT_FUNCTION("STARTMENU", _rect_start);
	RECT_HIT_FUNCTION("SHOTMENU", _rect_shotmenu);
	RECT_HIT_FUNCTION("RECTSAVE", _rect_save);
	RECT_HIT_FUNCTION("RECTLOAD", _rect_load);
	RECT_HIT_FUNCTION("RECTEASY", _rect_easy);
	RECT_HIT_FUNCTION("RECTAVG", _rect_average);
	RECT_HIT_FUNCTION("RECTHARD", _rect_hard);
	RECT_HIT_FUNCTION("SKULL", _rect_skull);
	RECT_HIT_FUNCTION("KILLINNOCENTMAN", _rect_killinnocentman);
	RECT_HIT_FUNCTION("KILLINNOCENTWOMAN", _rect_killinnocentwoman);
	RECT_HIT_FUNCTION("SELECTBEAVER", _rect_selectbeaver);
	RECT_HIT_FUNCTION("SELECTBONNIE", _rect_selectbonnie);
	RECT_HIT_FUNCTION("SELECTPROFESSOR", _rect_selectprofessor);
	RECT_HIT_FUNCTION("SHOTAMMO", _rect_shotammo);
	RECT_HIT_FUNCTION("SHOTGIN", _rect_shotgin);
	RECT_HIT_FUNCTION("SHOTLANTERN", _rect_shotlantern);
	RECT_HIT_FUNCTION("SHOOTSKULL", _rect_shootskull);

#undef RECT_HIT_FUNCTION

#define PRE_OPS_FUNCTION(name, func) _scenePreOps[name] = new MD2ScriptFunctionScene(this, &GameMaddog2::func);
	PRE_OPS_FUNCTION("DRAWRCT", _scene_pso_drawrct);
	PRE_OPS_FUNCTION("PAUSE", _scene_pso_pause);
	PRE_OPS_FUNCTION("FADEIN", _scene_pso_fadein);
	PRE_OPS_FUNCTION("PAUSFI", _scene_pso_pause_fadein);
	PRE_OPS_FUNCTION("PREREAD", _scene_pso_preread);
	PRE_OPS_FUNCTION("PAUSPR", _scene_pso_pause_preread);
	PRE_OPS_FUNCTION("DEFAULT", _scene_pso_drawrct);
	PRE_OPS_FUNCTION("DRAWRCTFDI", _scene_pso_drawrct_fadein);
	PRE_OPS_FUNCTION("PRESHOOTOUT", _scene_pso_shootout);
	PRE_OPS_FUNCTION("PREMDSHOOTOUT", _scene_pso_mdshootout);
#undef PRE_OPS_FUNCTION

#define INS_OPS_FUNCTION(name, func) _sceneInsOps[name] = new MD2ScriptFunctionScene(this, &GameMaddog2::func);
	INS_OPS_FUNCTION("DEFAULT", _scene_iso_donothing);
	INS_OPS_FUNCTION("PAUSE", _scene_iso_pause);
	INS_OPS_FUNCTION("SPAUSE", _scene_iso_spause);
	INS_OPS_FUNCTION("STARTGAME", _scene_iso_startgame);
	INS_OPS_FUNCTION("SHOOTPAST", _scene_iso_shootpast);
	INS_OPS_FUNCTION("STAGECOACH", _scene_iso_stagecoach);
	INS_OPS_FUNCTION("DIFFERENTPADRES", _scene_iso_differentpadres);
	INS_OPS_FUNCTION("DIFFERENTPADRESPAS", _scene_iso_differentpadrespas);
	INS_OPS_FUNCTION("DONTPOPNEXT", _scene_iso_dontpopnext);
	INS_OPS_FUNCTION("GETINTOROCK", _scene_iso_getintorock);
	INS_OPS_FUNCTION("BENATCAVE", _scene_iso_benatcave);
	INS_OPS_FUNCTION("SKULLATCAVE", _scene_iso_skullatcave);
	INS_OPS_FUNCTION("STARTOFTRAIN", _scene_iso_startoftrain);
	INS_OPS_FUNCTION("MISSION", _scene_iso_mission);
	INS_OPS_FUNCTION("MDSHOOTOUT", _scene_iso_mdshootout);
	INS_OPS_FUNCTION("STARTOFBOARDINGHOUSE", _scene_iso_startofboardinghouse);
	INS_OPS_FUNCTION("DONTCONTINUE", _scene_iso_dontcontinue);
	INS_OPS_FUNCTION("DOSHOOTOUT", _scene_iso_doshootout);
#undef INS_OPS_FUNCTION

#define NXT_SCN_FUNCTION(name, func) _sceneNxtScn[name] = new MD2ScriptFunctionScene(this, &GameMaddog2::func);
	NXT_SCN_FUNCTION("DEFAULT", _scene_default_nxtscn);
	NXT_SCN_FUNCTION("DIED", _scene_nxtscn_died);
	NXT_SCN_FUNCTION("KILLINNOCENTMAN", _scene_nxtscn_killinnocentman);
	NXT_SCN_FUNCTION("KILLINNOCENTWOMAN", _scene_nxtscn_killinnocentwoman);
	NXT_SCN_FUNCTION("KILLGUIDE", _scene_nxtscn_killguide);
	NXT_SCN_FUNCTION("SCN_SHOOTSKULL", _scene_nxtscn_shootskull);
	NXT_SCN_FUNCTION("CALLATTRACT", _scene_nxtscn_callattract);
	NXT_SCN_FUNCTION("PICKUNDERTAKER", _scene_nxtscn_pickundertaker);
	NXT_SCN_FUNCTION("CHOOSEPADRE", _scene_nxtscn_choosepadre);
	NXT_SCN_FUNCTION("SELECTGUIDE", _scene_nxtscn_selectguide);
	NXT_SCN_FUNCTION("SAVEBONNIE", _scene_nxtscn_savebonnie);
	NXT_SCN_FUNCTION("FINISHBONNIE", _scene_nxtscn_finishbonnie);
	NXT_SCN_FUNCTION("SHOWGGCLUE", _scene_nxtscn_showggclue);
	NXT_SCN_FUNCTION("BBAFTERCLUE", _scene_nxtscn_bbafterclue);
	NXT_SCN_FUNCTION("ASFARSHEGOES", _scene_nxtscn_asfarshegoes);
	NXT_SCN_FUNCTION("SAVEBEAVER", _scene_nxtscn_savebeaver);
	NXT_SCN_FUNCTION("FINISHBEAVER", _scene_nxtscn_finishbeaver);
	NXT_SCN_FUNCTION("TOGATLINGUNSBCLUE", _scene_nxtscn_togatlingunsbclue);
	NXT_SCN_FUNCTION("TOGUIDEAFTERCLUE", _scene_nxtscn_toguideafterclue);
	NXT_SCN_FUNCTION("TOGUIDECAVE", _scene_nxtscn_toguidecave);
	NXT_SCN_FUNCTION("INITRANDOMVILLAGE", _scene_nxtscn_initrandomvillage);
	NXT_SCN_FUNCTION("PICKVILLAGESCENES", _scene_nxtscn_pickvillagescenes);
	NXT_SCN_FUNCTION("SAVEPROFESSOR", _scene_nxtscn_saveprofessor);
	NXT_SCN_FUNCTION("FINISHPROFESSOR", _scene_nxtscn_finishprofessor);
	NXT_SCN_FUNCTION("TOGATLINGUNTPCLUE", _scene_nxtscn_togatlinguntpclue);
	NXT_SCN_FUNCTION("TPAFTERCLUE", _scene_nxtscn_tpafterclue);
	NXT_SCN_FUNCTION("FINISHGATLINGUN1", _scene_nxtscn_finishgatlingun1);
	NXT_SCN_FUNCTION("FINISHGUYATGG", _scene_nxtscn_finishguyatgg);
	NXT_SCN_FUNCTION("FINISHGATLINGUN2", _scene_nxtscn_finishgatlingun2);
	NXT_SCN_FUNCTION("HOWWEDID", _scene_nxtscn_howwedid);
	NXT_SCN_FUNCTION("PLAYERWON", _scene_nxtscn_playerwon);
	NXT_SCN_FUNCTION("BACKTONXTGUIDE", _scene_nxtscn_backtonxtguide);
	NXT_SCN_FUNCTION("FINISHGENERICSCENE", _scene_nxtscn_finishgenericscene);
	NXT_SCN_FUNCTION("INITRANDOMCOWBOYS", _scene_nxtscn_initrandomcowboys);
	NXT_SCN_FUNCTION("TOCOWBOYSCENES", _scene_nxtscn_tocowboyscenes);
	NXT_SCN_FUNCTION("INITRANDOMFARMYARD", _scene_nxtscn_initrandomfarmyard);
	NXT_SCN_FUNCTION("TOFARMYARDSCENES", _scene_nxtscn_tofarmyardscenes);
	NXT_SCN_FUNCTION("INITRANDOMCAVE", _scene_nxtscn_initrandomcave);
	NXT_SCN_FUNCTION("TOCAVESCENES", _scene_nxtscn_tocavescenes);
	NXT_SCN_FUNCTION("PICKSKULLATCAVE", _scene_nxtscn_pickskullatcave);
	NXT_SCN_FUNCTION("DRAWGUN", _scene_nxtscn_drawgun);
#undef NXT_SCN_FUNCTION

	_sceneShowMsg["DEFAULT"] = new MD2ScriptFunctionScene(this, &GameMaddog2::_scene_sm_donothing);
	_sceneWepDwn["DEFAULT"] = new MD2ScriptFunctionScene(this, &GameMaddog2::_scene_default_wepdwn);
	_sceneScnScr["DEFAULT"] = new MD2ScriptFunctionScene(this, &GameMaddog2::_scene_default_score);
	_sceneNxtFrm["DEFAULT"] = new MD2ScriptFunctionScene(this, &GameMaddog2::_scene_nxtfrm);
}

void GameMaddog2::verifyScriptFunctions() {
	Common::Array<Scene *> *scenes = _sceneInfo->getScenes();
	for (size_t i = 0; i < scenes->size(); i++) {
		Scene *scene = (*scenes)[i];
		getScriptFunctionScene(PREOP, scene->preop);
		// TODO: SHOWMSG
		getScriptFunctionScene(INSOP, scene->insop);
		getScriptFunctionScene(WEPDWN, scene->wepdwn);
		getScriptFunctionScene(SCNSCR, scene->scnscr);
		getScriptFunctionScene(NXTFRM, scene->nxtfrm);
		getScriptFunctionScene(NXTSCN, scene->nxtscn);
		for (size_t j = 0; j < scene->zones.size(); j++) {
			Zone *zone = scene->zones[j];
			getScriptFunctionZonePtrFb(zone->ptrfb);
			for (size_t k = 0; k < zone->rects.size(); k++) {
				getScriptFunctionRectHit(zone->rects[k].rectHit);
			}
		}
	}
}

MD2ScriptFunctionPoint GameMaddog2::getScriptFunctionZonePtrFb(Common::String name) {
	MD2ScriptFunctionPointMap::iterator it = _zonePtrFb.find(name);
	if (it != _zonePtrFb.end()) {
		return (*(*it)._value);
	} else {
		error("Could not find zonePtrFb function: %s", name.c_str());
	}
}

MD2ScriptFunctionRect GameMaddog2::getScriptFunctionRectHit(Common::String name) {
	MD2ScriptFunctionRectMap::iterator it = _rectHitFuncs.find(name);
	if (it != _rectHitFuncs.end()) {
		return (*(*it)._value);
	} else {
		error("Could not find rectHit function: %s", name.c_str());
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
		error("Unkown scene script type: %u", type);
		break;
	}
	MD2ScriptFunctionSceneMap::iterator it;
	it = functionMap->find(name);
	if (it != functionMap->end()) {
		return (*(*it)._value);
	} else {
		error("Could not find scene type %u function: %s", type, name.c_str());
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
		Audio::PacketizedAudioStream *audioStream = _videoDecoder->getAudioStream();
		g_system->getMixer()->stopHandle(_sceneAudioHandle);
		g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &_sceneAudioHandle, audioStream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
		_paletteDirty = true;
		_nextFrameTime = _GetMsTime() + 100;
		callScriptFunctionScene(PREOP, scene->preop, scene);
		_currentFrame = _GetFrame(scene);
		while (_currentFrame <= scene->endFrame && _cur_scene == oldscene && !_vm->shouldQuit()) {
			_UpdateMouse();
			// TODO: call scene->messageFunc
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
						if (_shots > 0) {
							if (!_debug_unlimitedAmmo) {
								_shots--;
							}
							_UpdateStat();
							Rect *hitRect = nullptr;
							Zone *hitSceneZone = _CheckZonesV1(scene, hitRect, &firedCoords);
							if (hitSceneZone != nullptr) {
								callScriptFunctionZonePtrFb(hitSceneZone->ptrfb, &firedCoords);
								callScriptFunctionRectHit(hitRect->rectHit, hitRect);
							} else {
								_default_bullethole(&firedCoords);
							}
						} else {
							_PlaySound(_emptySound);
							_emptyCount = 3;
							_whichGun = 9;
						}
					}
				}
			}
			if (_cur_scene == oldscene) {
				callScriptFunctionScene(NXTFRM, scene->nxtfrm, scene);
			}
			_DisplayScore();
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
		if (_ret_scene != "") {
			_cur_scene = _ret_scene;
			_ret_scene = "";
		}
		if (_sub_scene != "") {
			_ret_scene = _sub_scene;
			_sub_scene = "";
		}
		if (_cur_scene == oldscene) {
			callScriptFunctionScene(NXTSCN, scene->nxtscn, scene);
		}
		if (_cur_scene == "") {
			_vm->quitGame();
		}
	}
	_RemoveCursorTimer();
	return Common::kNoError;
}

void GameMaddog2::_NewGame() {
	_shots = 6;
	_lives = 3;
	_score = 0;
	_holster = false;
	_UpdateStat();
	_sub_scene = "";
}

void GameMaddog2::_ResetParams() {
	_ret_scene = "";
	_sub_scene = "";
	_last_scene = "";
	_lives = 3;
	_shots = 6;
	_score = 0;
	_which_padre = 0;
	_which_gatlin_gun = _rnd->getRandomNumber(2);
	_got_to = 0x22;
	_last_shoot_out = -1;
	_sb_got_to = 0;
	_bb_got_to = 0;
	_tp_got_to = 0;
	_shootout_from_die = false;
	_was_a_shootout = false;
	_shoot_out_cnt = 0;
	_this_guide = -1;
	_done_guide = 0;
	_total_dies = 0;
	_had_skull = 0;
	_in_shootout = 0;
	_UpdateStat();
}

void GameMaddog2::_DoMenu() {
	uint32 startTime = _GetMsTime();
	_RestoreCursor();
	_DoCursor();
	_inMenu = true;
	_MoveMouse();
	g_system->getMixer()->pauseHandle(_sceneAudioHandle, true);
	_screen->copyRectToSurface(_background->getBasePtr(_videoPosX, _videoPosY), _background->pitch, _videoPosX, _videoPosY, _videoDecoder->getWidth(), _videoDecoder->getHeight());
	_ShowDifficulty(_difficulty, false);
	while (_inMenu && !_vm->shouldQuit()) {
		Common::Point firedCoords;
		if (__Fired(&firedCoords)) {
			Rect *hitMenuRect = _CheckZone(_submenzone, &firedCoords);
			if (hitMenuRect != nullptr) {
				callScriptFunctionRectHit(hitMenuRect->rectHit, hitMenuRect);
			}
		}
		if (_difficulty != _oldDifficulty) {
			_ChangeDifficulty(_difficulty);
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

void GameMaddog2::_UpdateStat() {
	if (_lives != _oldLives) {
		if (_lives > _oldLives) {
			for (uint8 i = _oldLives; i < _lives; i++) {
				AlgGraphics::drawImage(_screen, &_liveIcon, _livepos[i][0], _livepos[i][1]);
			}
		} else {
			for (uint8 i = _lives; i < _oldLives; i++) {
				AlgGraphics::drawImage(_screen, &_deadIcon, _livepos[i][0], _livepos[i][1]);
			}
		}
		_oldLives = _lives;
	}
	if (_shots != _oldShots) {
		if (_shots > _oldShots) {
			for (uint8 i = _oldShots; i < _shots; i++) {
				AlgGraphics::drawImage(_screen, &_shotIcon, _shotpos[i][0], _shotpos[i][1]);
			}
		} else {
			for (uint8 i = _shots; i < _oldShots; i++) {
				AlgGraphics::drawImage(_screen, &_emptyIcon, _shotpos[i][0], _shotpos[i][1]);
			}
		}
		_oldShots = _shots;
	}
}

void GameMaddog2::_ChangeDifficulty(uint8 newDifficulty) {
	if (newDifficulty == _oldDifficulty) {
		return;
	}
	_ShowDifficulty(newDifficulty, true);
	Game::_AdjustDifficulty(newDifficulty, _oldDifficulty);
	_oldDifficulty = newDifficulty;
	_difficulty = newDifficulty;
}

void GameMaddog2::_ShowDifficulty(uint8 newDifficulty, bool updateCursor) {
	// reset menu screen
	_screen->copyRectToSurface(_background->getBasePtr(_videoPosX, _videoPosY), _background->pitch, _videoPosX, _videoPosY, _videoDecoder->getWidth(), _videoDecoder->getHeight());
	AlgGraphics::drawImageCentered(_screen, &_knifeIcon, _diffpos[newDifficulty - 1][0], _diffpos[newDifficulty - 1][1]);
	if (updateCursor) {
		_DoCursor();
	}
}

void GameMaddog2::_DoCursor() {
	_UpdateMouse();
}

void GameMaddog2::_UpdateMouse() {
	if (_oldWhichGun != _whichGun) {
		Graphics::PixelFormat pixelFormat = Graphics::PixelFormat::createFormatCLUT8();
		Graphics::Surface *cursor = &(*_gun)[_whichGun];
		CursorMan.popAllCursors();
		uint16 hotspotX = (cursor->w / 2);
		uint16 hotspotY = (cursor->h / 2);
		if (debugChannelSet(1, Alg::kAlgDebugGraphics)) {
			cursor->drawLine(0, hotspotY, cursor->w, hotspotY, 1);
			cursor->drawLine(hotspotX, 0, hotspotX, cursor->h, 1);
		}
		CursorMan.pushCursor(cursor->getPixels(), cursor->w, cursor->h, hotspotX, hotspotY, 0, false, &pixelFormat);
		CursorMan.showMouse(true);
		_oldWhichGun = _whichGun;
	}
}

void GameMaddog2::_MoveMouse() {
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
	_UpdateMouse();
}

void GameMaddog2::_DisplayScore() {
	if (_score == _oldScore) {
		return;
	}
	_oldScore = _score;
	Common::String scoreString = Common::String::format("%05d", _score);
	int posX = 0xE6;
	for (int i = 0; i < 5; i++) {
		uint8 digit = scoreString[i] - '0';
		AlgGraphics::drawImage(_screen, &(*_numbers)[digit], posX, 0xBE);
		posX += 7;
	}
}

bool GameMaddog2::_WeaponDown() {
	if (_rightDown && _mousePos.y >= 0xAA && _mousePos.x >= 0x113) {
		return true;
	}
	return false;
}

bool GameMaddog2::_SaveState() {
	Common::OutSaveFile *outSaveFile;
	Common::String saveFileName = _vm->getSaveStateName(0);
	if (!(outSaveFile = g_system->getSavefileManager()->openForSaving(saveFileName))) {
		warning("Can't create file '%s', game not saved", saveFileName.c_str());
		return false;
	}
	outSaveFile->writeUint32BE(MKTAG('A', 'L', 'G', 'S')); // header
	outSaveFile->writeByte(0);                             // version, unused for now
	outSaveFile->writeUint16LE(_total_dies);
	outSaveFile->writeUint16LE(_got_to);
	outSaveFile->writeUint16LE(_sb_got_to);
	outSaveFile->writeUint16LE(_bb_got_to);
	outSaveFile->writeUint16LE(_tp_got_to);
	outSaveFile->writeSint16LE(_this_guide);
	outSaveFile->writeUint16LE(_done_guide);
	outSaveFile->writeUint16LE(_had_skull);
	outSaveFile->writeUint16LE(_shoot_out_cnt);
	outSaveFile->writeUint16LE(_shoot_out_bits);
	outSaveFile->writeSint16LE(_last_shoot_out);
	outSaveFile->writeUint16LE(_start_lives);
	outSaveFile->writeByte(_was_a_shootout);
	outSaveFile->writeByte(_shootout_from_die);
	outSaveFile->writeUint16LE(_which_padre);
	outSaveFile->writeUint16LE(_which_gatlin_gun);
	outSaveFile->writeUint16LE(_lives);
	outSaveFile->writeUint16LE(_score);
	outSaveFile->writeUint16LE(_holster);
	outSaveFile->writeUint16LE(_in_shootout);
	outSaveFile->writeUint16LE(_difficulty);
	outSaveFile->writeString(_last_scene);
	outSaveFile->writeByte(0);
	outSaveFile->writeString(_cur_scene);
	outSaveFile->writeByte(0);
	outSaveFile->writeString(_sub_scene);
	outSaveFile->writeByte(0);
	outSaveFile->writeString(_ret_scene);
	outSaveFile->writeByte(0);
	outSaveFile->finalize();
	delete outSaveFile;
	return true;
}

bool GameMaddog2::_LoadState() {
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
	_total_dies = inSaveFile->readUint16LE();
	_got_to = inSaveFile->readUint16LE();
	_sb_got_to = inSaveFile->readUint16LE();
	_bb_got_to = inSaveFile->readUint16LE();
	_tp_got_to = inSaveFile->readUint16LE();
	_this_guide = inSaveFile->readSint16LE();
	_done_guide = inSaveFile->readUint16LE();
	_had_skull = inSaveFile->readUint16LE();
	_shoot_out_cnt = inSaveFile->readUint16LE();
	_shoot_out_bits = inSaveFile->readUint16LE();
	_last_shoot_out = inSaveFile->readSint16LE();
	_start_lives = inSaveFile->readUint16LE();
	_was_a_shootout = inSaveFile->readByte();
	_shootout_from_die = inSaveFile->readByte();
	_which_padre = inSaveFile->readUint16LE();
	_which_gatlin_gun = inSaveFile->readUint16LE();
	_lives = inSaveFile->readUint16LE();
	_score = inSaveFile->readUint16LE();
	_holster = inSaveFile->readUint16LE();
	_in_shootout = inSaveFile->readUint16LE();
	_difficulty = inSaveFile->readUint16LE();
	_last_scene = inSaveFile->readString();
	_cur_scene = inSaveFile->readString();
	_sub_scene = inSaveFile->readString();
	_ret_scene = inSaveFile->readString();
	delete inSaveFile;
	_ChangeDifficulty(_difficulty);
	return true;
}

// misc game functions
Common::String GameMaddog2::_NumtoScene(int n) {
	switch (n) {
	case 1:
		return "scene1aa";
	case 31:
		return "scen31a";
	case 34:
		return "scen34a";
	case 41:
		return "scen41a";
	case 42:
		return "scen42a";
	case 67:
		return "scen67a";
	case 85:
		return "scen85a";
	case 106:
		return "scen106a";
	case 118:
		return "scen118a";
	case 171:
		return "scen171a";
	case 180:
		return "scen180a";
	case 181:
		return "scen181a";
	case 182:
		return "scen182a";
	case 197:
		return "scen197a";
	case 199:
		return "scen199a";
	case 201:
		return "scen201a";
	case 203:
		return "scen203a";
	case 227:
		return "scen227a";
	case 244:
		return "scen244a";
	case 253:
		return "scen253a";
	case 287:
		return "scen287a";
	case 288:
		return "scen288a";
	case 295:
		return "scen295a";
	case 296:
		return "scen296a";
	default:
		return Common::String::format("scene%d", n);
	}
}

uint16 GameMaddog2::_ScenetoNum(Common::String sceneName) {
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

void GameMaddog2::_default_bullethole(Common::Point *point) {
	if (point->x >= 14 && point->x <= 306 && point->y >= 5 && point->y <= 169) {
		_RestoreCursor();
		uint16 targetX = point->x - _videoPosX;
		uint16 targetY = point->y - _videoPosY;
		AlgGraphics::drawImageCentered(_videoDecoder->getVideoFrame(), &_bulletholeIcon, targetX, targetY);
		_DoCursor();
		_shotFired = true;
		_DoShot();
	}
}

uint16 GameMaddog2::_die() {
	_UpdateStat();
	uint8 random = _rnd->getRandomNumber(9);
	if (random >= 4 && _lives >= 3) {
		return _pick_bits(&_die_bits, 6);
	}
	if (_lives == 2) {
		return 0x9D;
	} else if (_lives == 1) {
		return 0x9E;
	}
	return 0x9A;
}

uint16 GameMaddog2::_pick_bits(uint16 *bits, uint8 max) {
	// reset mask if full
	unsigned int fullMask = 0xFFFF >> (16 - max);
	if (fullMask == _pick_mask) {
		_pick_mask = 0;
	}
	*bits |= _pick_mask;
	if (fullMask == *bits) {
		*bits = _pick_mask;
		if (fullMask == *bits) {
			_pick_mask = 0;
			*bits = 0;
		}
	}
	uint16 random = _rnd->getRandomNumber(max - 1);
	// find an unused bit
	while (1) {
		uint16 bitMask = 1 << random;
		// if bit is already used or matches _last_pick, try next position
		if ((*bits & bitMask) || random == _last_pick) {
			random++;
			if (random >= max) {
				random = 0;
				_last_pick = 0xFFFF;
			}
			continue;
		}
		// found an unused bit
		break;
	}
	*bits |= (1 << random);
	_last_pick = random;
	_pick_mask = 0;
	return random;
}

uint16 GameMaddog2::_pick_shootout() {
	_shoot_out_cnt = 0;
	_was_a_shootout = true;
	_last_pick = _last_shoot_out;
	if (_difficulty == 1) {
		_last_shoot_out = _pick_bits(&_shoot_out_bits, 5);
		return _ez_shoot_outs[_last_shoot_out];
	} else {
		_last_shoot_out = _pick_bits(&_shoot_out_bits, 6);
		return _shoot_outs[_last_shoot_out];
	}
}

void GameMaddog2::_next_sb() {
	_shootout_from_die = false;
	_had_skull = false;
	_random_count = 0;
	if (_was_a_shootout) {
		_was_a_shootout = false;
	} else {
		_sb_got_to++;
	}
	if (_sb_scenes[_sb_got_to] == 0x87) {
		_place_bits = 0;
		_pick_mask = 0;
		_gg_pick_man();
		// _scene_pso_fadein(cur_scene);
	} else {
		if (_sb_got_to == 7 || _sb_got_to == 9) {
			_cur_scene = _NumtoScene(_sb_scenes[_sb_got_to]);
		} else {
			_shoot_out_cnt++;
			if (_shoot_out_cnt <= 3) {
				_cur_scene = _NumtoScene(_sb_scenes[_sb_got_to]);
				// _scene_pso_fadein(cur_scene);
			} else {
				_shootout_from_die = false;
				_cur_scene = _NumtoScene(_pick_shootout());
			}
		}
	}
}

void GameMaddog2::_next_bb() {
	_shootout_from_die = false;
	_had_skull = false;
	_random_count = 0;
	if (_was_a_shootout) {
		_was_a_shootout = false;
	} else {
		_bb_got_to++;
	}
	if (_bb_scenes[_bb_got_to] == 0x87) {
		_place_bits = 0;
		_pick_mask = 0;
		_gg_pick_man();
		// _scene_pso_fadein(cur_scene);
	} else {
		if (_bb_got_to == 7 || _bb_got_to == 9) {
			_cur_scene = _NumtoScene(_bb_scenes[_bb_got_to]);
		} else {
			_shoot_out_cnt++;
			if (_shoot_out_cnt <= 3) {
				_cur_scene = _NumtoScene(_bb_scenes[_bb_got_to]);
				// _scene_pso_fadein(cur_scene);
			} else {
				_shootout_from_die = false;
				_cur_scene = _NumtoScene(_pick_shootout());
			}
		}
	}
}

void GameMaddog2::_next_tp() {
	_shootout_from_die = false;
	_had_skull = false;
	_random_count = 0;
	if (_was_a_shootout) {
		_was_a_shootout = false;
	} else {
		_tp_got_to++;
	}
	if (_tp_scenes[_tp_got_to] == 0xDC) {
		_place_bits = 0;
		_pick_mask = 0;
	}
	if (_tp_scenes[_tp_got_to] == 0x87) {
		_place_bits = 0;
		_pick_mask = 0;
		_gg_pick_man();
		// _scene_pso_fadein(cur_scene);
	} else {
		if (_tp_got_to == 7 || _tp_got_to == 9) {
			_cur_scene = _NumtoScene(_tp_scenes[_tp_got_to]);
		} else {
			_shoot_out_cnt++;
			if (_shoot_out_cnt <= 3) {
				_cur_scene = _NumtoScene(_tp_scenes[_tp_got_to]);
				// _scene_pso_fadein(cur_scene);
			} else {
				_shootout_from_die = false;
				_cur_scene = _NumtoScene(_pick_shootout());
			}
		}
	}
}

void GameMaddog2::_gg_pick_man() {
	_random_count++;
	uint8 totalRandom = ((_difficulty - 1) * 2) + 3;
	if (_random_count < totalRandom) {
		uint16 index = _pick_bits(&_place_bits, 3);
		_cur_scene = _NumtoScene(_gg_scenes[index]);
	} else {
		_cur_scene = "scene139";
	}
}

void GameMaddog2::_generic_next() {
	if (_shootout_from_die && _got_to == 0x32) {
		_shootout_from_die = false;
		_cur_scene = "scene50";
		return;
	}
	_had_skull = false;
	_shootout_from_die = false;
	switch (_got_to) {
	case 227:
		_cur_scene = "scen227a";
		break;
	case 238:
		_cur_scene = "scene238";
		break;
	case 244:
		_cur_scene = "scen244a";
		break;
	case 254:
		_cur_scene = "scene254";
		break;
	case 287:
		_cur_scene = "scen287a";
		break;
	default:
		if (_this_guide == 0) {
			_next_sb();
			return;
		} else if (_this_guide == 1) {
			_next_bb();
			return;
		} else if (_this_guide == 2) {
			_next_tp();
			return;
		}
	}
}

void GameMaddog2::_player_won() {
	_done_guide |= (1 << _this_guide);
	if (_total_dies < _start_lives || _done_guide != 7) {
		_cur_scene = "scene290";
	} else {
		_cur_scene = "scene291";
	}
}

// Script functions: Zone
void GameMaddog2::_zone_bullethole(Common::Point *point) {
	_default_bullethole(point);
}

void GameMaddog2::_zone_skullhole(Common::Point *point) {
	if (point->x >= 14 && point->x <= 306 && point->y >= 5 && point->y <= 169) {
		uint16 targetX = point->x - _videoPosX;
		uint16 targetY = point->y - _videoPosY;
		_RestoreCursor();
		AlgGraphics::drawImageCentered(_videoDecoder->getVideoFrame(), &_bulletholeIcon, targetX, targetY);
		_DoCursor();
		_shotFired = true;

		if (_had_skull) {
			_DoShot();
		} else {
			_DoSkullSound();
		}
	}
}

// Script functions: RectHit
void GameMaddog2::_rect_skull(Rect *rect) {
	if (_had_skull) {
		return;
	}
	_had_skull = true;
	_shots = 12;
	_score += 1000;
	_UpdateStat();
}

void GameMaddog2::_rect_killinnocentman(Rect *rect) {
	_total_dies++;
	_was_a_shootout = false;
	if (!_debug_godMode) {
		_lives--;
	}
	if (_lives <= 0) {
		_shoot_out_cnt++;
	}
	_UpdateStat();
	_cur_scene = "scene153";
}

void GameMaddog2::_rect_killinnocentwoman(Rect *rect) {
	_total_dies++;
	_was_a_shootout = false;
	if (!_debug_godMode) {
		_lives--;
	}
	if (_lives <= 0) {
		_shoot_out_cnt++;
	}
	_UpdateStat();
	_cur_scene = "scene156";
}

void GameMaddog2::_rect_selectbeaver(Rect *rect) {
	if (_done_guide & 1) {
		return;
	}
	_was_a_shootout = false;
	_this_guide = 0;
	_random_count = 0;
	_place_bits = 0;
	if (_sb_scenes[_sb_got_to] == 0x87) {
		_pick_mask = 0;
		_gg_pick_man();
		// scene_pso_fadein(cur_scene);
	} else {
		_cur_scene = _NumtoScene(_sb_scenes[_sb_got_to]);
		// scene_pso_fadein(cur_scene);
	}
}

void GameMaddog2::_rect_selectbonnie(Rect *rect) {
	if (_done_guide & 2) {
		return;
	}
	_was_a_shootout = false;
	_this_guide = 1;
	_random_count = 0;
	_place_bits = 0;
	if (_bb_scenes[_bb_got_to] == 0x87) {
		_pick_mask = 0;
		_gg_pick_man();
		// scene_pso_fadein(cur_scene);
	} else {
		_cur_scene = _NumtoScene(_bb_scenes[_bb_got_to]);
		// scene_pso_fadein(cur_scene);
	}
}

void GameMaddog2::_rect_selectprofessor(Rect *rect) {
	if (_done_guide & 4) {
		return;
	}
	_was_a_shootout = false;
	_this_guide = 2;
	_random_count = 0;
	_place_bits = 0;
	if (_tp_scenes[_tp_got_to] == 0x87) {
		_pick_mask = 0;
		_gg_pick_man();
		// scene_pso_fadein(cur_scene);
	} else {
		_cur_scene = _NumtoScene(_tp_scenes[_tp_got_to]);
		// scene_pso_fadein(cur_scene);
	}
}

void GameMaddog2::_rect_shotammo(Rect *rect) {
	if (_which_gatlin_gun == 0) {
		_cur_scene = "scene140";
	} else {
		switch (_this_guide) {
		case 0:
			_sb_got_to = 5;
			break;
		case 1:
			_bb_got_to = 5;
			break;
		case 2:
			_tp_got_to = 5;
			break;
		}
		_cur_scene = "scene299";
	}
}

void GameMaddog2::_rect_shotgin(Rect *rect) {
	if (_which_gatlin_gun == 1) {
		_cur_scene = "scene140";
	} else {
		switch (_this_guide) {
		case 0:
			_sb_got_to = 5;
			break;
		case 1:
			_bb_got_to = 5;
			break;
		case 2:
			_tp_got_to = 5;
			break;
		}
		_cur_scene = "scene299";
	}
}

void GameMaddog2::_rect_shotlantern(Rect *rect) {
	if (_which_gatlin_gun == 2) {
		_cur_scene = "scene140";
	} else {
		switch (_this_guide) {
		case 0:
			_sb_got_to = 5;
			break;
		case 1:
			_bb_got_to = 5;
			break;
		case 2:
			_tp_got_to = 5;
			break;
		}
		_cur_scene = "scene299";
	}
}

void GameMaddog2::_rect_shootskull(Rect *rect) {
	_scene_nxtscn_shootskull(nullptr);
}

void GameMaddog2::_rect_shotmenu(Rect *rect) {
	_DoMenu();
}

void GameMaddog2::_rect_save(Rect *rect) {
	if(_SaveState()) {
		_DoSaveSound();
	}
}

void GameMaddog2::_rect_load(Rect *rect) {
	if(_LoadState()) {
		_DoLoadSound();
	}
}

void GameMaddog2::_rect_continue(Rect *rect) {
	_inMenu = false;
	_fired = false;
	if (_lives <= 0) {
		_cur_scene = _last_scene;
		_sub_scene = "";
		_ret_scene = "";
		_NewGame();
		_had_skull = false;
		_shootout_from_die = false;
		_was_a_shootout = false;
	} else {
		_UpdateStat();
	}
}

void GameMaddog2::_rect_start(Rect *rect) {
	_inMenu = false;
	_fired = false;
	Scene *scene = _sceneInfo->findScene(_startscene);
	if (scene->nxtscn == "DRAWGUN") {
		callScriptFunctionScene(NXTSCN, "DRAWGUN", scene);
	}
	_cur_scene = _startscene;
	_ResetParams();
	_NewGame();
	_UpdateStat();
}

// Script functions: Scene PreOps
void GameMaddog2::_scene_pso_shootout(Scene *scene) {
	sscanf(scene->preopParam.c_str(), "#%ldto%ld", &_minF, &_maxF);
	if (!_debug_unlimitedAmmo) {
		_shots = 0;
	}
	_in_shootout = true;
	_UpdateStat();
	_RestoreCursor();
	AlgGraphics::drawImage(_screen, &_reloadIcon, 0x37, 0xBE);
	_DoCursor();
}

void GameMaddog2::_scene_pso_mdshootout(Scene *scene) {
	sscanf(scene->preopParam.c_str(), "#%ldto%ld", &_minF, &_maxF);
	if (!_debug_unlimitedAmmo) {
		_shots = 0;
	}
	_in_shootout = true;
	_UpdateStat();
	_RestoreCursor();
	AlgGraphics::drawImage(_screen, &_reloadIcon, 0x37, 0xBE);
	_DoCursor();
}

// Script functions: Scene Scene InsOps
void GameMaddog2::_scene_iso_shootpast(Scene *scene) {
	if (_lives <= 0) {
		return;
	}
	if (_fired) {
		if (_ret_scene != "") {
			_cur_scene = _ret_scene;
			_ret_scene = "";
		} else if (_sub_scene != "") {
			_cur_scene = _sub_scene;
			_sub_scene = "";
		} else {
			callScriptFunctionScene(NXTSCN, scene->nxtscn, scene);
		}
	}
}

void GameMaddog2::_scene_iso_spause(Scene *scene) {
	_scene_iso_shootpast(scene);
	_scene_iso_pause(scene);
}

void GameMaddog2::_scene_iso_stagecoach(Scene *scene) {
	_had_skull = false;
	_last_scene = "scen34a";
	_scene_iso_pause(scene);
}

void GameMaddog2::_scene_iso_differentpadres(Scene *scene) {
	_got_to = 0x32;
	_last_scene = "scene50";
}

void GameMaddog2::_scene_iso_differentpadrespas(Scene *scene) {
	_got_to = 0x32;
	_last_scene = "scene50";
	_scene_iso_pause(scene);
}

void GameMaddog2::_scene_iso_dontpopnext(Scene *scene) {
	_last_pick = 0;
	_last_scene = "scene135";
}

void GameMaddog2::_scene_iso_getintorock(Scene *scene) {
	_last_scene = "scene50";
}

void GameMaddog2::_scene_iso_benatcave(Scene *scene) {
	_got_to = _ScenetoNum(_cur_scene);
	_last_scene = "scen227a";
	_scene_iso_shootpast(scene);
}

void GameMaddog2::_scene_iso_skullatcave(Scene *scene) {
	_got_to = _ScenetoNum(_cur_scene);
	_last_scene = "scene238";
}

void GameMaddog2::_scene_iso_startoftrain(Scene *scene) {
	_got_to = _ScenetoNum(_cur_scene);
	_last_scene = "scen244a";
}

void GameMaddog2::_scene_iso_mission(Scene *scene) {
	_got_to = _ScenetoNum(_cur_scene);
	_last_scene = "scene254";
}

void GameMaddog2::_scene_iso_startofboardinghouse(Scene *scene) {
	_got_to = _ScenetoNum(_cur_scene);
	_last_scene = "scen295a";
}

void GameMaddog2::_scene_iso_dontcontinue(Scene *scene) {
	_scene_nxtscn_callattract(scene);
}

void GameMaddog2::_scene_iso_mdshootout(Scene *scene) {
	_got_to = _ScenetoNum(_cur_scene);
	_last_scene = "scen287a";
	_scene_iso_doshootout(scene);
}

void GameMaddog2::_scene_iso_doshootout(Scene *scene) {
	if (_currentFrame < (uint32)_minF) {
		if (!_debug_unlimitedAmmo) {
			_shots = 0;
		}
		return;
	}
	if (_in_shootout) {
		_RestoreCursor();
		AlgGraphics::drawImage(_screen, &_drawIcon, 0x37, 0xBE);
		_DoCursor();
	}
	_in_shootout = false;
	if (_shots > 0) {
		if (_currentFrame < (uint32)_maxF) {
			callScriptFunctionScene(NXTSCN, scene->nxtscn, scene);
		}
	}
}

// Script functions: Scene NxtScn
void GameMaddog2::_scene_default_nxtscn(Scene *scene) {
	// wipe background drawing from shootout
	_screen->copyRectToSurface(_background->getBasePtr(0x37, 0xBE), _background->pitch, 0x37, 0xBE, _reloadIcon.w, _reloadIcon.h);
	Game::_scene_default_nxtscn(scene);
}

void GameMaddog2::_scene_nxtscn_drawgun(Scene *scene) {
	_RestoreCursor();
	_DoCursor();
	_scene_default_nxtscn(scene);
}

void GameMaddog2::_scene_nxtscn_died(Scene *scene) {
	_total_dies++;
	_shootout_from_die = false;
	_was_a_shootout = false;
	if (!_debug_godMode) {
		_lives--;
	}
	_UpdateStat();
	_cur_scene = _NumtoScene(_die());
}

void GameMaddog2::_scene_nxtscn_killinnocentman(Scene *scene) {
	_total_dies++;
	_was_a_shootout = false;
	if (!_debug_godMode) {
		_lives--;
	}
	_shoot_out_cnt++;
	_UpdateStat();
	_cur_scene = "scene153";
}

void GameMaddog2::_scene_nxtscn_killinnocentwoman(Scene *scene) {
	_total_dies++;
	_was_a_shootout = false;
	if (!_debug_godMode) {
		_lives--;
	}
	_shoot_out_cnt++;
	_UpdateStat();
	_cur_scene = "scene156";
}

void GameMaddog2::_scene_nxtscn_killguide(Scene *scene) {
	if (!_debug_godMode) {
		_lives--;
	}
	_UpdateStat();
	_cur_scene = "scene160";
}

void GameMaddog2::_scene_nxtscn_shootskull(Scene *scene) {
	if (_had_skull) {
		return;
	}
	_had_skull = 1;
	_DoSkullSound();
	_shots = 12;
	_score += 1000;
	_UpdateStat();
	_cur_scene = "scene293";
}

void GameMaddog2::_scene_nxtscn_callattract(Scene *scene) {
	_ResetParams();
	_cur_scene = "scene1aa";
}

void GameMaddog2::_scene_nxtscn_pickundertaker(Scene *scene) {
	if (_lives <= 0) {
		_cur_scene = "scene311";
		return;
	}
	if (_got_to == 0x127 || _got_to == 0x22) {
		_cur_scene = _NumtoScene(_got_to);
	} else {
		if (_which_padre == 0) {
			_which_padre = 2;
			_got_to = 0x32;
			if (_rnd->getRandomBit()) {
				_cur_scene = "scene49";
			} else {
				_cur_scene = "scene47";
			}
		} else {
			_shoot_out_cnt++;
			if (_shoot_out_cnt > 3) {
				_shootout_from_die = true;
				_cur_scene = _NumtoScene(_pick_shootout());
			} else {
				_cur_scene = _NumtoScene(_got_to);
			}
		}
	}
}

void GameMaddog2::_scene_nxtscn_choosepadre(Scene *scene) {
	_had_skull = false;
	if (_which_padre == 0) {
		_got_to = 0x32;
		_which_padre = 1;
		_cur_scene = "scene45";
	} else if (_which_padre == 1) {
		_got_to = 0x32;
		_which_padre = 2;
		if (_rnd->getRandomBit()) {
			_cur_scene = "scene49";
		} else {
			_cur_scene = "scene47";
		}
	} else {
		_got_to = 0x32;
		_cur_scene = _NumtoScene(0x32);
	}
}

void GameMaddog2::_scene_nxtscn_selectguide(Scene *scene) {
	_was_a_shootout = false;
	if (_this_guide < 0) {
		_this_guide = _rnd->getRandomNumber(2);
	}
	_random_count = 0;
	_place_bits = 0;
	if (_done_guide & (1 << _this_guide)) {
		_this_guide = 0;
		while (_this_guide < 3) {
			if (!(_done_guide & (1 << _this_guide))) {
				break;
			}
			_this_guide++;
		}
	}
	switch (_this_guide) {
	case 0:
		_cur_scene = _NumtoScene(_sb_scenes[_sb_got_to]);
		break;
	case 1:
		_cur_scene = _NumtoScene(_bb_scenes[_bb_got_to]);
		break;
	case 2:
		_cur_scene = _NumtoScene(_tp_scenes[_tp_got_to]);
		break;
	}
}

void GameMaddog2::_scene_nxtscn_savebonnie(Scene *scene) {
	_next_bb();
}

void GameMaddog2::_scene_nxtscn_finishbonnie(Scene *scene) {
	_next_bb();
}

void GameMaddog2::_scene_nxtscn_showggclue(Scene *scene) {
	_shootout_from_die = false;
	_cur_scene = _NumtoScene(_bb_clue[_which_gatlin_gun]);
}

void GameMaddog2::_scene_nxtscn_bbafterclue(Scene *scene) {
	_shootout_from_die = false;
	_random_count = 0;
	_bb_got_to = 6;
	_cur_scene = _NumtoScene(_bb_scenes[_bb_got_to]);
}

void GameMaddog2::_scene_nxtscn_asfarshegoes(Scene *scene) {
	_shootout_from_die = false;
	_random_count = 0;
	_got_to = 12;
	_cur_scene = _NumtoScene(_bb_scenes[_got_to]);
}

void GameMaddog2::_scene_nxtscn_savebeaver(Scene *scene) {
	_next_sb();
}

void GameMaddog2::_scene_nxtscn_finishbeaver(Scene *scene) {
	_next_sb();
}

void GameMaddog2::_scene_nxtscn_togatlingunsbclue(Scene *scene) {
	_shootout_from_die = false;
	_cur_scene = _NumtoScene(_sb_clue[_which_gatlin_gun]);
}

void GameMaddog2::_scene_nxtscn_toguideafterclue(Scene *scene) {
	_shootout_from_die = false;
	_random_count = 0;
	_sb_got_to = 6;
	_cur_scene = _NumtoScene(_sb_scenes[_sb_got_to]);
}

void GameMaddog2::_scene_nxtscn_toguidecave(Scene *scene) {
	_shootout_from_die = false;
	_random_count = 0;
	_sb_got_to = 12;
	_got_to = _sb_scenes[_sb_got_to];
	_cur_scene = _NumtoScene(_got_to);
}

void GameMaddog2::_scene_nxtscn_initrandomvillage(Scene *scene) {
	_shootout_from_die = false;
	_place_bits = 0;
	_random_count = 0;
	int index = _pick_bits(&_place_bits, 6);
	_cur_scene = _NumtoScene(_village_scenes[index]);
}

void GameMaddog2::_scene_nxtscn_pickvillagescenes(Scene *scene) {
	_random_count++;
	uint8 totalRandom = ((_difficulty - 1) * 2) + 5;
	if (_random_count < totalRandom) {
		int index = _pick_bits(&_place_bits, 6);
		_cur_scene = _NumtoScene(_village_scenes[index]);
	} else {
		_cur_scene = "scene100";
	}
}

void GameMaddog2::_scene_nxtscn_saveprofessor(Scene *scene) {
	_next_tp();
}

void GameMaddog2::_scene_nxtscn_finishprofessor(Scene *scene) {
	_next_tp();
}

void GameMaddog2::_scene_nxtscn_togatlinguntpclue(Scene *scene) {
	_shootout_from_die = false;
	_cur_scene = _NumtoScene(_tp_clue[_which_gatlin_gun]);
}

void GameMaddog2::_scene_nxtscn_tpafterclue(Scene *scene) {
	_shootout_from_die = false;
	_random_count = 0;
	_tp_got_to = 6;
	_cur_scene = _NumtoScene(_tp_scenes[_tp_got_to]);
}

void GameMaddog2::_scene_nxtscn_finishgatlingun1(Scene *scene) {
	_shootout_from_die = false;
	_random_count = 0;
	_tp_got_to = 12;
	_got_to = _tp_scenes[_tp_got_to];
	_cur_scene = _NumtoScene(_got_to);
}

void GameMaddog2::_scene_nxtscn_finishguyatgg(Scene *scene) {
	_gg_pick_man();
}

void GameMaddog2::_scene_nxtscn_finishgatlingun2(Scene *scene) {
	_shootout_from_die = false;
	_scene_nxtscn_finishgenericscene(scene);
}

void GameMaddog2::_scene_nxtscn_howwedid(Scene *scene) {
	_player_won();
}

void GameMaddog2::_scene_nxtscn_playerwon(Scene *scene) {
	_cur_scene = "scene1aa";
}

void GameMaddog2::_scene_nxtscn_backtonxtguide(Scene *scene) {
	_done_guide |= (1 << _this_guide);
	_got_to = 0x32;
	_cur_scene = _NumtoScene(_got_to);
}

void GameMaddog2::_scene_nxtscn_finishgenericscene(Scene *scene) {
	_generic_next();
}

void GameMaddog2::_scene_nxtscn_initrandomcowboys(Scene *scene) {
	_place_bits = 0;
	_random_count = 0;
	uint16 picked = _pick_bits(&_place_bits, 7);
	_cur_scene = _NumtoScene(_cowboy_scenes[picked]);
}

void GameMaddog2::_scene_nxtscn_tocowboyscenes(Scene *scene) {
	_random_count++;
	uint8 totalRandom = ((_difficulty - 1) * 2) + 7;
	if (_random_count < totalRandom) {
		uint16 picked = _pick_bits(&_place_bits, 7);
		_cur_scene = _NumtoScene(_cowboy_scenes[picked]);
	} else {
		_generic_next();
	}
}

void GameMaddog2::_scene_nxtscn_initrandomfarmyard(Scene *scene) {
	_place_bits = 0;
	_random_count = 0;
	uint16 picked = _pick_bits(&_place_bits, 4);
	_cur_scene = _NumtoScene(_farmyard_scenes[picked]);
}

void GameMaddog2::_scene_nxtscn_tofarmyardscenes(Scene *scene) {
	_random_count++;
	uint8 totalRandom = ((_difficulty - 1) * 2) + 5;
	if (_random_count < totalRandom) {
		uint16 picked = _pick_bits(&_place_bits, 4);
		_cur_scene = _NumtoScene(_farmyard_scenes[picked]);
	} else {
		_generic_next();
	}
}

void GameMaddog2::_scene_nxtscn_initrandomcave(Scene *scene) {
	_place_bits = 0;
	_random_count = 0;
	uint16 picked = _pick_bits(&_place_bits, 5);
	_cur_scene = _NumtoScene(_cave_scenes[picked]);
}

void GameMaddog2::_scene_nxtscn_tocavescenes(Scene *scene) {
	_random_count++;
	uint8 totalRandom = ((_difficulty - 1) * 2) + 8;
	if (_random_count < totalRandom) {
		uint16 picked = _pick_bits(&_place_bits, 5);
		_cur_scene = _NumtoScene(_cave_scenes[picked]);
	} else {
		_got_to = 0xEE;
		_cur_scene = _NumtoScene(_got_to);
	}
}

void GameMaddog2::_scene_nxtscn_pickskullatcave(Scene *scene) {
	switch (_rnd->getRandomNumber(2)) {
	case 0:
		_cur_scene = "scene239";
		break;
	case 1:
		_cur_scene = "scene240";
		break;
	case 2:
		_cur_scene = "scene242";
		break;
	}
}

// Script functions: WepDwn
void GameMaddog2::_scene_default_wepdwn(Scene *scene) {
	_inHolster = 9;
	_whichGun = 7;
	_UpdateMouse();
	if (!_in_shootout) {
		if (_shots < 6) {
			_shots = 6;
		}
		_UpdateStat();
	}
}


// Debug methods
void GameMaddog2::debug_warpTo(int val) {
	// TODO implement
}

// Debugger methods
DebuggerMaddog2::DebuggerMaddog2(GameMaddog2 *game) : GUI::Debugger() {
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
		_game->debug_warpTo(val);
		return false;
	}
}

bool DebuggerMaddog2::cmdDumpLib(int argc, const char **argv) {
	return _game->debug_dumpLibFile();
}

} // End of namespace Alg
