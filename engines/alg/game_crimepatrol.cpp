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

#include "alg/game_crimepatrol.h"
#include "alg/graphics.h"
#include "alg/scene.h"

namespace Alg {

GameCrimePatrol::GameCrimePatrol(AlgEngine *vm, const ADGameDescription *desc) : Game(vm) {
	if (scumm_stricmp(desc->gameId, "cpatrols") == 0) {
		_libFileName = "cpss.lib";
	} else if (scumm_stricmp(desc->gameId, "cpatrold") == 0) {
		_libFileName = "cpds.lib";
	} else if (scumm_stricmp(desc->gameId, "cpatroldemo") == 0) {
		_libFileName = "cp.lib";
		_isDemo = true;
	}
}

GameCrimePatrol::~GameCrimePatrol() {
}

void GameCrimePatrol::init() {
	_videoPosX = 11;
	_videoPosY = 2;

	loadLibArchive(_libFileName);
	_sceneInfo->loadScnFile("cp.scn");
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

	_submenzone->addRect(0x1C, 0x11, 0x5D, 0x20, nullptr, 0, "STARTMENU", "0");
	_submenzone->addRect(0x1C, 0x31, 0x5D, 0x40, nullptr, 0, "RECTLOAD", "0");
	_submenzone->addRect(0x1C, 0x51, 0x5D, 0x60, nullptr, 0, "RECTSAVE", "0");
	_submenzone->addRect(0x1C, 0x71, 0x5D, 0x80, nullptr, 0, "CONTMENU", "0");
	_submenzone->addRect(0x1C, 0x91, 0x5D, 0xA0, nullptr, 0, "EXITMENU", "0");
	_submenzone->addRect(0xDD, 0x3C, 0x010A, 0x4B, nullptr, 0, "RECTEASY", "0");
	_submenzone->addRect(0xDD, 0x5C, 0x010A, 0x6B, nullptr, 0, "RECTAVG", "0");
	_submenzone->addRect(0xDD, 0x7C, 0x010A, 0x8B, nullptr, 0, "RECTHARD", "0");

	_shotSound = _LoadSoundFile("blow.8b");
	_emptySound = _LoadSoundFile("empty.8b");
	_saveSound = _LoadSoundFile("saved.8b");
	_loadSound = _LoadSoundFile("loaded.8b");
	_skullSound = _LoadSoundFile("skull.8b");

	_gun = AlgGraphics::loadScreenCoordAniImage("gun.ani", _palette);
	_numbers = AlgGraphics::loadAniImage("numbers.ani", _palette);
	Common::Array<Graphics::Surface> *bullets = AlgGraphics::loadAniImage("bullets.ani", _palette);
	_shotIcon = (*bullets)[0];
	_emptyIcon = (*bullets)[1];
	Common::Array<Graphics::Surface> *lives = AlgGraphics::loadAniImage("lives.ani", _palette);
	_liveIcon = (*lives)[0];
	_deadIcon = (*lives)[1];
	Common::Array<Graphics::Surface> *difficlt = AlgGraphics::loadScreenCoordAniImage("difficlt.ani", _palette);
	_difficultyIcon = (*difficlt)[0];
	Common::Array<Graphics::Surface> *hole = AlgGraphics::loadScreenCoordAniImage("hole.ani", _palette);
	_bulletholeIcon = (*hole)[0];

	_background = AlgGraphics::loadVgaBackground("cp_menu.vga", _palette);
	_screen->copyRectToSurface(_background->getPixels(), _background->pitch, 0, 0, _background->w, _background->h);

	_MoveMouse();
}

void GameCrimePatrol::registerScriptFunctions() {
#define RECT_HIT_FUNCTION(name, func) _rectHitFuncs[name] = new CPScriptFunctionRect(this, &GameCrimePatrol::func);
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
	RECT_HIT_FUNCTION("TARGET_PRACTICE", _rect_target_practice);
	RECT_HIT_FUNCTION("SELECT_TARGET_PRACTICE", _rect_select_target_practice);
	RECT_HIT_FUNCTION("SELECT_GANG_FIGHT", _rect_select_gang_fight);
	RECT_HIT_FUNCTION("SELECT_WAREHOUSE", _rect_select_warehouse);
	RECT_HIT_FUNCTION("SELECT_WESTCOAST_SOUND", _rect_select_westcoast_sound);
	RECT_HIT_FUNCTION("SELECT_DRUG_DEAL", _rect_select_drug_deal);
	RECT_HIT_FUNCTION("SELECT_CAR_RING", _rect_select_car_ring);
	RECT_HIT_FUNCTION("SELECT_BAR", _rect_select_bar);
	RECT_HIT_FUNCTION("SELECT_BANK", _rect_select_bank);
	RECT_HIT_FUNCTION("SELECT_CRACK_HOUSE", _rect_select_crack_house);
	RECT_HIT_FUNCTION("SELECT_METH_LAB", _rect_select_meth_lab);
	RECT_HIT_FUNCTION("SELECT_AIRPLANE", _rect_select_airplane);
	RECT_HIT_FUNCTION("SELECT_NUKE_TRANSPORT", _rect_select_nuke_transport);
	RECT_HIT_FUNCTION("SELECT_AIRPORT", _rect_select_airport);
	RECT_HIT_FUNCTION("KILL_INNOCENT_MAN", _rect_kill_innocent_man);
#undef RECT_HIT_FUNCTION

#define PRE_OPS_FUNCTION(name, func) _scenePreOps[name] = new CPScriptFunctionScene(this, &GameCrimePatrol::func);
	PRE_OPS_FUNCTION("DEFAULT", _scene_pso_drawrct);
	PRE_OPS_FUNCTION("PAUSE", _scene_pso_pause);
	PRE_OPS_FUNCTION("FADEIN", _scene_pso_fadein);
	PRE_OPS_FUNCTION("PAUSE_FADEIN", _scene_pso_pause_fadein);
	PRE_OPS_FUNCTION("WAREHOUSE_GOT_TO", _scene_pso_warehouse_got_to);
	PRE_OPS_FUNCTION("GANG_FIGHT_GOT_TO", _scene_pso_gang_fight_got_to);
	PRE_OPS_FUNCTION("WESTCOAST_SOUND_GOT_TO", _scene_pso_westcoast_sound_got_to);
	PRE_OPS_FUNCTION("DRUG_DEAL_GOT_TO", _scene_pso_drug_deal_got_to);
	PRE_OPS_FUNCTION("CAR_RING_GOT_TO", _scene_pso_car_ring_got_to);
	PRE_OPS_FUNCTION("BANK_GOT_TO", _scene_pso_bank_got_to);
	PRE_OPS_FUNCTION("CRACK_HOUSE_GOT_TO", _scene_pso_crack_house_got_to);
	PRE_OPS_FUNCTION("METH_LAB_GOT_TO", _scene_pso_meth_lab_got_to);
	PRE_OPS_FUNCTION("AIRPLANE_GOT_TO", _scene_pso_airplane_got_to);
	PRE_OPS_FUNCTION("AIRPORT_GOT_TO", _scene_pso_airport_got_to);
	PRE_OPS_FUNCTION("NUKE_TRANSPORT_GOT_TO", _scene_pso_nuke_transport_got_to);
	PRE_OPS_FUNCTION("POWER_PLANT_GOT_TO", _scene_pso_power_plant_got_to);
#undef PRE_OPS_FUNCTION

#define INS_OPS_FUNCTION(name, func) _sceneInsOps[name] = new CPScriptFunctionScene(this, &GameCrimePatrol::func);
	INS_OPS_FUNCTION("DEFAULT", _scene_iso_donothing);
	INS_OPS_FUNCTION("PAUSE", _scene_iso_pause);
#undef INS_OPS_FUNCTION

#define NXT_SCN_FUNCTION(name, func) _sceneNxtScn[name] = new CPScriptFunctionScene(this, &GameCrimePatrol::func);
	NXT_SCN_FUNCTION("DEFAULT", _scene_default_nxtscn);
	NXT_SCN_FUNCTION("GAME_WON", _scene_nxtscn_game_won);
	NXT_SCN_FUNCTION("LOSE_A_LIFE", _scene_nxtscn_lose_a_life);
	NXT_SCN_FUNCTION("DID_NOT_CONTINUE", _scene_nxtscn_did_not_continue);
	NXT_SCN_FUNCTION("KILL_INNOCENT_MAN", _scene_nxtscn_kill_innocent_man);
	NXT_SCN_FUNCTION("KILL_INNOCENT_WOMAN", _scene_nxtscn_kill_innocent_woman);
	NXT_SCN_FUNCTION("AFTER_DIE", _scene_nxtscn_after_die);
	NXT_SCN_FUNCTION("SELECT_LANGUAGE_1", _scene_nxtscn_select_language_1);
	NXT_SCN_FUNCTION("SELECT_LANGUAGE_2", _scene_nxtscn_select_language_2);
	NXT_SCN_FUNCTION("INIT_RANDOM_TARGET_PRACTICE", _scene_nxtscn_init_random_target_practice);
	NXT_SCN_FUNCTION("CONTINUE_TARGET_PRACTICE", _scene_nxtscn_continue_target_practice);
	NXT_SCN_FUNCTION("SELECT_ROOKIE_SCENARIO", _scene_nxtscn_select_rookie_scenario);
	NXT_SCN_FUNCTION("FINISH_GANG_FIGHT", _scene_nxtscn_finish_gang_fight);
	NXT_SCN_FUNCTION("FINISH_WESTCOAST_SOUND", _scene_nxtscn_finish_westcoast_sound);
	NXT_SCN_FUNCTION("FINISH_WAREHOUSE", _scene_nxtscn_finish_warehouse);
	NXT_SCN_FUNCTION("INIT_RANDOM_WAREHOUSE", _scene_nxtscn_init_random_warehouse);
	NXT_SCN_FUNCTION("CONTINUE_WAREHOUSE", _scene_nxtscn_continue_warehouse);
	NXT_SCN_FUNCTION("SELECT_UNDERCOVER_SCENARIO", _scene_nxtscn_select_undercover_scenario);
	NXT_SCN_FUNCTION("FINISH_DRUG_DEAL", _scene_nxtscn_finish_drug_deal);
	NXT_SCN_FUNCTION("INIT_RANDOM_CAR_RING_LEADER", _scene_nxtscn_init_random_car_ring_leader);
	NXT_SCN_FUNCTION("CONTINUE_CAR_RING_LEADER_1", _scene_nxtscn_continue_car_ring_leader_1);
	NXT_SCN_FUNCTION("CONTINUE_CAR_RING_LEADER_2", _scene_nxtscn_continue_car_ring_leader_2);
	NXT_SCN_FUNCTION("INIT_RANDOM_CAR_RING", _scene_nxtscn_init_random_car_ring);
	NXT_SCN_FUNCTION("CONTINUE_CAR_RING", _scene_nxtscn_continue_car_ring);
	NXT_SCN_FUNCTION("FINISH_CAR_RING", _scene_nxtscn_finish_car_ring);
	NXT_SCN_FUNCTION("FINISH_BAR", _scene_nxtscn_finish_bar);
	NXT_SCN_FUNCTION("FINISH_BANK", _scene_nxtscn_finish_bank);
	NXT_SCN_FUNCTION("FINISH_CRACK_HOUSE", _scene_nxtscn_finish_crack_house);
	NXT_SCN_FUNCTION("FINISH_METH_LAB", _scene_nxtscn_finish_meth_lab);
	NXT_SCN_FUNCTION("FINISH_AIRPLANE", _scene_nxtscn_finish_airplane);
	NXT_SCN_FUNCTION("FINISH_AIRPORT", _scene_nxtscn_finish_airport);
	NXT_SCN_FUNCTION("FINISH_NUKE_TRANSPORT", _scene_nxtscn_finish_nuke_transport);
	NXT_SCN_FUNCTION("INIT_RANDOM_BAR", _scene_nxtscn_init_random_bar);
	NXT_SCN_FUNCTION("CONTINUE_BAR", _scene_nxtscn_continue_bar);
	NXT_SCN_FUNCTION("SELECT_SWAT_SCENARIO", _scene_nxtscn_select_swat_scenario);
	NXT_SCN_FUNCTION("INIT_RANDOM_BANK", _scene_nxtscn_init_random_bank);
	NXT_SCN_FUNCTION("CONTINUE_BANK", _scene_nxtscn_continue_bank);
	NXT_SCN_FUNCTION("INIT_RANDOM_METH_LAB", _scene_nxtscn_init_random_meth_lab);
	NXT_SCN_FUNCTION("CONTINUE_METH_LAB", _scene_nxtscn_continue_meth_lab);
	NXT_SCN_FUNCTION("SELECT_DELTA_SCENARIO", _scene_nxtscn_select_delta_scenario);
	NXT_SCN_FUNCTION("PICK_RANDOM_RAPPELLER", _scene_nxtscn_pick_random_rapeller);
	NXT_SCN_FUNCTION("INIT_RANDOM_AIRPLANE", _scene_nxtscn_init_random_airplane);
	NXT_SCN_FUNCTION("CONTINUE_AIRPLANE", _scene_nxtscn_continue_airplane);
	NXT_SCN_FUNCTION("PICK_RANDOM_AIRPLANE_FRONT", _scene_nxtscn_pick_random_airplane_front);
	NXT_SCN_FUNCTION("INIT_RANDOM_AIRPORT", _scene_nxtscn_init_random_airport);
	NXT_SCN_FUNCTION("CONTINUE_AIRPORT", _scene_nxtscn_continue_airport);
	NXT_SCN_FUNCTION("INIT_RANDOM_NUKE_TRANSPORT", _scene_nxtscn_init_random_nuke_transport);
	NXT_SCN_FUNCTION("CONTINUE_NUKE_TRANSPORT", _scene_nxtscn_continue_nuke_transport);
	NXT_SCN_FUNCTION("INIT_RANDOM_POWERPLANT", _scene_nxtscn_init_random_powerplant);
	NXT_SCN_FUNCTION("CONTINUE_POWERPLANT", _scene_nxtscn_continue_powerplant);
#undef NXT_SCN_FUNCTION

	_sceneShowMsg["DEFAULT"] = new CPScriptFunctionScene(this, &GameCrimePatrol::_scene_sm_donothing);
	_sceneWepDwn["DEFAULT"] = new CPScriptFunctionScene(this, &GameCrimePatrol::_scene_default_wepdwn);
	_sceneScnScr["DEFAULT"] = new CPScriptFunctionScene(this, &GameCrimePatrol::_scene_default_score);
	_sceneNxtFrm["DEFAULT"] = new CPScriptFunctionScene(this, &GameCrimePatrol::_scene_nxtfrm);
}

void GameCrimePatrol::verifyScriptFunctions() {
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

CPScriptFunctionRect GameCrimePatrol::getScriptFunctionRectHit(Common::String name) {
	CPScriptFunctionRectMap::iterator it = _rectHitFuncs.find(name);
	if (it != _rectHitFuncs.end()) {
		return (*(*it)._value);
	} else {
		error("Could not find rectHit function: %s", name.c_str());
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
		error("Unkown scene script type: %u", type);
		break;
	}
	CPScriptFunctionSceneMap::iterator it;
	it = functionMap->find(name);
	if (it != functionMap->end()) {
		return (*(*it)._value);
	} else {
		error("Could not find scene type %u function: %s", type, name.c_str());
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
	_NewGame();
	_cur_scene = _startscene;
	Common::String oldscene;
	while (!_vm->shouldQuit()) {
		oldscene = _cur_scene;
		_SetFrame();
		_fired = false;
		Scene *scene = _sceneInfo->findScene(_cur_scene);
		if (!loadScene(scene)) {
			if (scene->nxtscn == "CONTINUE_TARGET_PRACTICE") {
				callScriptFunctionScene(NXTSCN, scene->nxtscn, scene);
			} else {
				error("Cannot find scene %s in libfile", scene->name.c_str());
			}
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
						if (_shots > 0) {
							if (!_debug_unlimitedAmmo) {
								_shots--;
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
			_DisplayLivesLeft();
			_DisplayScores();
			_DisplayShotsLeft();
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
			debug_drawPracticeRects();
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

void GameCrimePatrol::_NewGame() {
	_shots = 10;
	_lives = 3;
	_holster = false;
}

void GameCrimePatrol::_ResetParams() {
	// fill _got_to with start scenes
	// 0 in _got_to array means the level is finished
	for (uint8 i = 0; i < 15; i++) {
		_got_to[i] = _level_scenes[i][0];
	}
}

void GameCrimePatrol::_DoMenu() {
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

void GameCrimePatrol::_ChangeDifficulty(uint8 newDifficulty) {
	if (newDifficulty == _oldDifficulty) {
		return;
	}
	_ShowDifficulty(newDifficulty, true);
	_oldDifficulty = newDifficulty;
	_difficulty = newDifficulty;
}

void GameCrimePatrol::_ShowDifficulty(uint8 newDifficulty, bool updateCursor) {
	// reset menu screen
	_screen->copyRectToSurface(_background->getBasePtr(_videoPosX, _videoPosY), _background->pitch, _videoPosX, _videoPosY, _videoDecoder->getWidth(), _videoDecoder->getHeight());
	uint16 posY = 0x45 + ((newDifficulty - 1) * 0x21);
	AlgGraphics::drawImageCentered(_screen, &_difficultyIcon, 0x0115, posY);
	if (updateCursor) {
		_DoCursor();
	}
}

void GameCrimePatrol::_DoCursor() {
	_UpdateMouse();
}

void GameCrimePatrol::_UpdateMouse() {
	if (_oldWhichGun != _whichGun) {
		Graphics::PixelFormat pixelFormat = Graphics::PixelFormat::createFormatCLUT8();
		Graphics::Surface *cursor = &(*_gun)[_whichGun];
		CursorMan.popAllCursors();
		uint16 hotspotX = (cursor->w / 2) + 3;
		uint16 hotspotY = (cursor->h / 2) + 3;
		if (debugChannelSet(1, Alg::kAlgDebugGraphics)) {
			cursor->drawLine(0, hotspotY, cursor->w, hotspotY, 1);
			cursor->drawLine(hotspotX, 0, hotspotX, cursor->h, 1);
		}
		CursorMan.pushCursor(cursor->getPixels(), cursor->w, cursor->h, hotspotX, hotspotY, 0, false, &pixelFormat);
		CursorMan.showMouse(true);
		_oldWhichGun = _whichGun;
	}
}

void GameCrimePatrol::_MoveMouse() {
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

void GameCrimePatrol::_DisplayLivesLeft() {
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

void GameCrimePatrol::_DisplayScores() {
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

void GameCrimePatrol::_DisplayShotsLeft() {
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

bool GameCrimePatrol::_WeaponDown() {
	if (_rightDown && _mousePos.y >= 0xAA && _mousePos.x >= 0x113) {
		return true;
	}
	return false;
}

bool GameCrimePatrol::_SaveState() {
	Common::OutSaveFile *outSaveFile;
	Common::String saveFileName = _vm->getSaveStateName(0);
	if (!(outSaveFile = g_system->getSavefileManager()->openForSaving(saveFileName))) {
		warning("Can't create file '%s', game not saved", saveFileName.c_str());
		return false;
	}
	outSaveFile->writeUint32BE(MKTAG('A', 'L', 'G', 'S')); // header
	outSaveFile->writeByte(0);                             // version, unused for now
	outSaveFile->writeSByte(_stage);
	for (uint8 i = 0; i < 15; i++) {
		outSaveFile->writeUint16LE(_got_to[i]);
	}
	outSaveFile->writeSint32LE(_score);
	outSaveFile->writeUint16LE(_shots);
	outSaveFile->writeSByte(_lives);
	outSaveFile->finalize();
	delete outSaveFile;
	return true;
}

bool GameCrimePatrol::_LoadState() {
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
	_stage = inSaveFile->readSByte();
	for (uint8 i = 0; i < 15; i++) {
		_got_to[i] = inSaveFile->readUint16LE();
	}
	_score = inSaveFile->readSint32LE();
	_shots = inSaveFile->readUint16LE();
	_lives = inSaveFile->readSByte();
	delete inSaveFile;
	_gameInProgress = true;
	_scene_nxtscn_generic(_stage);
	return true;
}

// misc game functions
void GameCrimePatrol::_DisplayShotFiredImage(Common::Point *point) {
	if (point->x >= _videoPosX && point->x <= (_videoPosX + _videoDecoder->getWidth()) && point->y >= _videoPosY && point->y <= (_videoPosY + _videoDecoder->getHeight())) {
		uint16 targetX = point->x - _videoPosX;
		uint16 targetY = point->y - _videoPosY;
		AlgGraphics::drawImageCentered(_videoDecoder->getVideoFrame(), &_bulletholeIcon, targetX, targetY);
	}
}

void GameCrimePatrol::_EnableVideoFadeIn() {
	// TODO implement
}

uint16 GameCrimePatrol::_SceneToNumber(Scene *scene) {
	return atoi(scene->name.substr(5).c_str());
}

uint16 GameCrimePatrol::_RandomUnusedInt(uint8 max, uint16 *mask, uint16 exclude) {
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

uint16 GameCrimePatrol::_PickRandomScene(uint8 index, uint8 max) {
	if (max != 0) {
		_random_max = max;
		_random_mask = 0;
		_random_picked = 0;
		_random_scene_count = 0;
		while (_level_scenes[index][_random_scene_count] != 0) {
			_random_scene_count++;
		}
	}
	unsigned short count = _random_max--;
	if (count > 0) {
		_random_picked = _RandomUnusedInt(_random_scene_count, &_random_mask, _random_picked);
		return _level_scenes[index][_random_picked];
	}
	return 0;
}

uint16 GameCrimePatrol::_PickDeathScene() {
	if (_stage != _old_stage) {
		_old_stage = _stage;
		_death_mask = 0;
		_death_picked = -1;
		_death_scene_count = 0;
		while (_died_scenes_by_stage[_stage][_death_scene_count] != 0) {
			_death_scene_count++;
		}
	}
	_death_picked = _RandomUnusedInt(_death_scene_count, &_death_mask, _death_picked);
	return _died_scenes_by_stage[_stage][_death_picked];
}

void GameCrimePatrol::_scene_nxtscn_generic(uint8 index) {
	uint16 nextSceneId = 0;
	_got_to[index] = 0;
	if (_got_to[0] || _got_to[1] || _got_to[3] || _got_to[2]) {
		nextSceneId = _stage_start_scenes[0];
	} else if (_got_to[4] || _got_to[5] || _got_to[6]) {
		if (_stage == 1) {
			nextSceneId = _stage_start_scenes[1];
		} else {
			_stage = 1;
			nextSceneId = 0x50;
		}
	} else if (_got_to[7] || _got_to[8] || _got_to[9]) {
		if (_stage == 2) {
			nextSceneId = _stage_start_scenes[2];
		} else {
			_stage = 2;
			nextSceneId = 0x81;
		}
	} else if (_got_to[10] || _got_to[11] || _got_to[12]) {
		if (_stage == 3) {
			nextSceneId = _stage_start_scenes[3];
		} else {
			_stage = 3;
			nextSceneId = 0x014B;
		}
	} else if (_got_to[13]) {
		_stage = 4;
		nextSceneId = 0x018F;
	} else {
		nextSceneId = 0x21;
	}
	_cur_scene = Common::String::format("scene%d", nextSceneId);
}

void GameCrimePatrol::_rect_select_generic(uint8 index) {
	if (_got_to[index] > 0) {
		_cur_scene = Common::String::format("scene%d", _got_to[index]);
	}
}

void GameCrimePatrol::_scene_iso_got_to_generic(uint8 index, uint16 sceneId) {
	_got_to[index] = sceneId;
}

// Script functions: RectHit
void GameCrimePatrol::_rect_shotmenu(Rect *rect) {
	_DoMenu();
}

void GameCrimePatrol::_rect_save(Rect *rect) {
	if(_SaveState()) {
		_DoSaveSound();
	}
}

void GameCrimePatrol::_rect_load(Rect *rect) {
	if(_LoadState()) {
		_DoLoadSound();
	}
}

void GameCrimePatrol::_rect_continue(Rect *rect) {
	_inMenu = false;
	_fired = false;
	if (_lives <= 0) {
		_score = (int32)(_score * 0.7f);
		uint16 returnScene = _stage_start_scenes[_stage];
		_cur_scene = Common::String::format("scene%d", returnScene);
		_NewGame();
	}
}

void GameCrimePatrol::_rect_start(Rect *rect) {
	_inMenu = false;
	_fired = false;
	_gameInProgress = true;
	if (_isDemo) {
		_cur_scene = "scene39";
		_got_to[1] = 39;
	} else {
		_cur_scene = Common::String::format("scene%d", _stage_start_scenes[0]);
	}
	_ResetParams();
	_NewGame();
}

void GameCrimePatrol::_rect_target_practice(Rect *rect) {
	uint16 nextScene = 0;
	Scene *scene = _sceneInfo->findScene(_cur_scene);
	if (_level_scenes[0][0] == _SceneToNumber(scene)) {
		_practice_mask = 0x1F;
	}
	if (_practice_mask == 0) {
		_practice_mask = 0x1F;
	}
	for (uint8 i = 0; i < 5; i++) {
		if (_mousePos.x <= _practice_target_left[i] || _mousePos.x >= _practice_target_right[i] ||
			_mousePos.y <= _practice_target_top[i] || _mousePos.y >= _practice_target_bottom[i]) {
			// did not hit target
			continue;
		}
		uint8 mask = 1 << i;
		if (!(_practice_mask & mask)) {
			// target was already hit before
			continue;
		}
		// did hit target
		_score += scene->scnscrParam == 0 ? 50 : scene->scnscrParam;
		_practice_mask ^= mask;
		uint8 inverted = _practice_mask ^ 0x1F;
		if (_practice_mask == 0) {
			nextScene = 432;
		} else {
			nextScene = 401 + inverted;
		}
		break;
	}
	if (nextScene != 0) {
		_cur_scene = Common::String::format("scene%d", nextScene);
	}
}

void GameCrimePatrol::_rect_select_target_practice(Rect *rect) {
	_rect_select_generic(0);
	_got_to[0] = 0;
}

void GameCrimePatrol::_rect_select_gang_fight(Rect *rect) {
	_got_to[0] = 0;
	_rect_select_generic(1);
}

void GameCrimePatrol::_rect_select_warehouse(Rect *rect) {
	_got_to[0] = 0;
	_rect_select_generic(2);
}

void GameCrimePatrol::_rect_select_westcoast_sound(Rect *rect) {
	_got_to[0] = 0;
	_rect_select_generic(3);
}

void GameCrimePatrol::_rect_select_drug_deal(Rect *rect) {
	_rect_select_generic(4);
}
void GameCrimePatrol::_rect_select_car_ring(Rect *rect) {
	_rect_select_generic(5);
}
void GameCrimePatrol::_rect_select_bar(Rect *rect) {
	_rect_select_generic(6);
}
void GameCrimePatrol::_rect_select_bank(Rect *rect) {
	_rect_select_generic(7);
}
void GameCrimePatrol::_rect_select_crack_house(Rect *rect) {
	_rect_select_generic(9);
}

void GameCrimePatrol::_rect_select_meth_lab(Rect *rect) {
	_rect_select_generic(8);
}

void GameCrimePatrol::_rect_select_airplane(Rect *rect) {
	_rect_select_generic(10);
}

void GameCrimePatrol::_rect_select_nuke_transport(Rect *rect) {
	_rect_select_generic(11);
}

void GameCrimePatrol::_rect_select_airport(Rect *rect) {
	_rect_select_generic(12);
}

void GameCrimePatrol::_rect_kill_innocent_man(Rect *rect) {
}

// Script functions: Scene PreOps
void GameCrimePatrol::_scene_pso_warehouse_got_to(Scene *scene) {
	uint16 sceneId = _SceneToNumber(scene);
	_scene_iso_got_to_generic(2, sceneId);
	_EnableVideoFadeIn();
}

void GameCrimePatrol::_scene_pso_gang_fight_got_to(Scene *scene) {
	uint16 sceneId = _SceneToNumber(scene);
	_scene_iso_got_to_generic(1, sceneId);
	_EnableVideoFadeIn();
}

void GameCrimePatrol::_scene_pso_westcoast_sound_got_to(Scene *scene) {
	_scene_iso_got_to_generic(3, 456);
	_EnableVideoFadeIn();
}

void GameCrimePatrol::_scene_pso_drug_deal_got_to(Scene *scene) {
	uint16 sceneId = _SceneToNumber(scene);
	_scene_iso_got_to_generic(4, sceneId);
	_EnableVideoFadeIn();
}

void GameCrimePatrol::_scene_pso_car_ring_got_to(Scene *scene) {
	uint16 sceneId = _SceneToNumber(scene);
	_scene_iso_got_to_generic(5, sceneId);
	_EnableVideoFadeIn();
}

void GameCrimePatrol::_scene_pso_bank_got_to(Scene *scene) {
	uint16 sceneId = _SceneToNumber(scene);
	_scene_iso_got_to_generic(7, sceneId);
	_EnableVideoFadeIn();
}

void GameCrimePatrol::_scene_pso_crack_house_got_to(Scene *scene) {
	uint16 sceneId = _SceneToNumber(scene);
	_scene_iso_got_to_generic(9, sceneId);
	_EnableVideoFadeIn();
}

void GameCrimePatrol::_scene_pso_meth_lab_got_to(Scene *scene) {
	uint16 sceneId = _SceneToNumber(scene);
	_scene_iso_got_to_generic(8, sceneId);
	_EnableVideoFadeIn();
}

void GameCrimePatrol::_scene_pso_airplane_got_to(Scene *scene) {
	uint16 sceneId = _SceneToNumber(scene);
	_scene_iso_got_to_generic(10, sceneId);
	_EnableVideoFadeIn();
}

void GameCrimePatrol::_scene_pso_airport_got_to(Scene *scene) {
	uint16 sceneId = _SceneToNumber(scene);
	_scene_iso_got_to_generic(12, sceneId);
	_EnableVideoFadeIn();
}

void GameCrimePatrol::_scene_pso_nuke_transport_got_to(Scene *scene) {
	uint16 sceneId = _SceneToNumber(scene);
	_scene_iso_got_to_generic(11, sceneId);
	_EnableVideoFadeIn();
}

void GameCrimePatrol::_scene_pso_power_plant_got_to(Scene *scene) {
	uint16 sceneId = _SceneToNumber(scene);
	_scene_iso_got_to_generic(13, sceneId);
	_final_stage_scene = sceneId;
	_EnableVideoFadeIn();
}

// Script functions: Scene NxtScn
void GameCrimePatrol::_scene_nxtscn_lose_a_life(Scene *scene) {
	uint16 picked = 0;
	if (!_debug_godMode) {
		_lives--;
	}
	if (_isDemo) {
		_cur_scene = "scene39";
		return;
	} else if (_lives > 0) {
		_DisplayLivesLeft();
		picked = _PickDeathScene();
	} else {
		picked = _dead_scenes[_stage];
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::_scene_nxtscn_game_won(Scene *scene) {
	_gameInProgress = false;
	_cur_scene = _startscene;
}

void GameCrimePatrol::_scene_nxtscn_did_not_continue(Scene *scene) {
	_gameInProgress = false;
	_cur_scene = _startscene;
}

void GameCrimePatrol::_scene_nxtscn_kill_innocent_man(Scene *scene) {
	uint16 picked = 0;
	if (!_debug_godMode) {
		_lives--;
	}
	if (_lives > 0) {
		picked = _stage_start_scenes[_stage];
	} else {
		picked = _dead_scenes[_stage];
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::_scene_nxtscn_kill_innocent_woman(Scene *scene) {
	uint16 picked = 0;
	if (!_debug_godMode) {
		_lives--;
	}
	if (_lives > 0) {
		picked = _stage_start_scenes[_stage];
	} else {
		picked = _dead_scenes[_stage];
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::_scene_nxtscn_after_die(Scene *scene) {
	uint16 picked = _stage_start_scenes[_stage];
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::_scene_nxtscn_select_language_1(Scene *scene) {
	// do nothing
}

void GameCrimePatrol::_scene_nxtscn_select_language_2(Scene *scene) {
	// do nothing
}

void GameCrimePatrol::_scene_nxtscn_init_random_target_practice(Scene *scene) {
	uint16 picked = _PickRandomScene(14, 6);
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::_scene_nxtscn_continue_target_practice(Scene *scene) {
	uint16 picked = _PickRandomScene(14, 0);
	if (picked == 0) {
		_scene_iso_got_to_generic(0, 1);
		_scene_nxtscn_generic(0);
	} else {
		_cur_scene = Common::String::format("scene%d", picked);
	}
}

void GameCrimePatrol::_scene_nxtscn_select_rookie_scenario(Scene *scene) {
	uint16 picked = 0;
	if (_got_to[0] > 0) {
		picked = _got_to[0];
		_got_to[0] = 0;
	} else if (_got_to[3] > 0) {
		picked = _got_to[3];
	} else if (_got_to[1] > 0) {
		picked = _got_to[1];
	} else if (_got_to[2] > 0) {
		picked = _got_to[2];
	} else {
		picked = _stage_start_scenes[1];
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::_scene_nxtscn_select_undercover_scenario(Scene *scene) {
	uint16 picked = 0;
	if (_got_to[4] > 0) {
		picked = _got_to[4];
	} else if (_got_to[5] > 0) {
		picked = _got_to[5];
	} else if (_got_to[6] > 0) {
		picked = _got_to[6];
	} else {
		picked = _stage_start_scenes[2];
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::_scene_nxtscn_select_swat_scenario(Scene *scene) {
	uint16 picked = 0;
	if (_got_to[8] > 0) {
		picked = _got_to[8];
	} else if (_got_to[7] > 0) {
		picked = _got_to[7];
	} else if (_got_to[9] > 0) {
		picked = _got_to[9];
	} else {
		picked = _stage_start_scenes[3];
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::_scene_nxtscn_select_delta_scenario(Scene *scene) {
	uint16 picked = 0;
	if (_got_to[10] > 0) {
		picked = _got_to[10];
	} else if (_got_to[11] > 0) {
		picked = _got_to[11];
	} else if (_got_to[12] > 0) {
		picked = _got_to[12];
	} else {
		picked = _final_stage_scene;
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::_scene_nxtscn_finish_gang_fight(Scene *scene) {
	if (_isDemo) {
		_cur_scene = _startscene;
		return;
	}
	_scene_nxtscn_generic(1);
}

void GameCrimePatrol::_scene_nxtscn_finish_westcoast_sound(Scene *scene) {
	_scene_nxtscn_generic(3);
}

void GameCrimePatrol::_scene_nxtscn_finish_warehouse(Scene *scene) {
	_scene_nxtscn_generic(2);
}

void GameCrimePatrol::_scene_nxtscn_init_random_warehouse(Scene *scene) {
	uint16 picked = _PickRandomScene(15, (_difficulty * 2) + 5);
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::_scene_nxtscn_continue_warehouse(Scene *scene) {
	uint16 picked = _PickRandomScene(15, 0);
	if (picked == 0) {
		picked = 0x43;
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::_scene_nxtscn_finish_drug_deal(Scene *scene) {
	_scene_nxtscn_generic(4);
}

void GameCrimePatrol::_scene_nxtscn_init_random_car_ring_leader(Scene *scene) {
	uint16 picked = _PickRandomScene(16, 2);
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::_scene_nxtscn_continue_car_ring_leader_1(Scene *scene) {
	uint16 picked = _PickRandomScene(16, 0);
	if (picked == 0) {
		picked = 0x67;
	} else {
		picked = 0x63;
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::_scene_nxtscn_continue_car_ring_leader_2(Scene *scene) {
	uint16 picked = _PickRandomScene(16, 0);
	if (picked == 0) {
		picked = 0x67;
	} else {
		picked = 0x66;
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::_scene_nxtscn_init_random_car_ring(Scene *scene) {
	uint16 picked = _PickRandomScene(17, (_difficulty * 2) + 8);
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::_scene_nxtscn_continue_car_ring(Scene *scene) {
	uint16 picked = _PickRandomScene(17, 0);
	if (picked == 0) {
		picked = 0x74;
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::_scene_nxtscn_finish_car_ring(Scene *scene) {
	_scene_nxtscn_generic(5);
}

void GameCrimePatrol::_scene_nxtscn_finish_bar(Scene *scene) {
	_scene_nxtscn_generic(6);
}

void GameCrimePatrol::_scene_nxtscn_finish_bank(Scene *scene) {
	_scene_nxtscn_generic(7);
}

void GameCrimePatrol::_scene_nxtscn_finish_crack_house(Scene *scene) {
	_scene_nxtscn_generic(9);
}

void GameCrimePatrol::_scene_nxtscn_finish_meth_lab(Scene *scene) {
	_scene_nxtscn_generic(8);
}

void GameCrimePatrol::_scene_nxtscn_finish_airplane(Scene *scene) {
	_scene_nxtscn_generic(10);
}

void GameCrimePatrol::_scene_nxtscn_finish_airport(Scene *scene) {
	_scene_nxtscn_generic(12);
}

void GameCrimePatrol::_scene_nxtscn_finish_nuke_transport(Scene *scene) {
	_scene_nxtscn_generic(11);
}

void GameCrimePatrol::_scene_nxtscn_init_random_bar(Scene *scene) {
	uint16 picked = _PickRandomScene(18, (_difficulty * 2) + 9);
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::_scene_nxtscn_continue_bar(Scene *scene) {
	uint16 picked = _PickRandomScene(18, 0);
	if (picked == 0) {
		picked = 0x92;
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::_scene_nxtscn_init_random_bank(Scene *scene) {
	uint16 picked = _PickRandomScene(19, (_difficulty * 2) + 8);
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::_scene_nxtscn_continue_bank(Scene *scene) {
	uint16 picked = _PickRandomScene(19, 0);
	if (picked == 0) {
		picked = 0xA8;
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::_scene_nxtscn_init_random_meth_lab(Scene *scene) {
	uint16 picked = _PickRandomScene(20, (_difficulty * 2) + 8);
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::_scene_nxtscn_continue_meth_lab(Scene *scene) {
	uint16 picked = _PickRandomScene(20, 0);
	if (picked == 0) {
		picked = 0xD0;
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::_scene_nxtscn_pick_random_rapeller(Scene *scene) {
	uint16 picked = _PickRandomScene(21, 1);
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::_scene_nxtscn_init_random_airplane(Scene *scene) {
	uint16 picked = _PickRandomScene(22, (_difficulty * 2) + 8);
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::_scene_nxtscn_continue_airplane(Scene *scene) {
	uint16 picked = _PickRandomScene(22, 0);
	if (picked == 0) {
		picked = 0x108;
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::_scene_nxtscn_pick_random_airplane_front(Scene *scene) {
	uint16 picked = _PickRandomScene(23, 1);
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::_scene_nxtscn_init_random_airport(Scene *scene) {
	uint16 picked = _PickRandomScene(24, (_difficulty * 2) + 8);
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::_scene_nxtscn_continue_airport(Scene *scene) {
	uint16 picked = _PickRandomScene(24, 0);
	if (picked == 0) {
		picked = 0x12D;
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::_scene_nxtscn_init_random_nuke_transport(Scene *scene) {
	uint16 picked = _PickRandomScene(25, (_difficulty * 2) + 8);
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::_scene_nxtscn_continue_nuke_transport(Scene *scene) {
	uint16 picked = _PickRandomScene(25, 0);
	if (picked == 0) {
		picked = 0x147;
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::_scene_nxtscn_init_random_powerplant(Scene *scene) {
	uint16 picked = _PickRandomScene(26, (_difficulty * 2) + 8);
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameCrimePatrol::_scene_nxtscn_continue_powerplant(Scene *scene) {
	uint16 picked = _PickRandomScene(26, 0);
	if (picked == 0) {
		picked = 0x169;
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

// Script functions: WepDwn
void GameCrimePatrol::_scene_default_wepdwn(Scene *scene) {
	_shots = 10;
}

// Debug methods
void GameCrimePatrol::debug_warpTo(int val) {
	// TODO implement
}

void GameCrimePatrol::debug_drawPracticeRects() {
	if (_debug_drawRects || debugChannelSet(1, Alg::kAlgDebugGraphics)) {
		Scene *scene = _sceneInfo->findScene(_cur_scene);
		if (scene->zones.size() > 0) {
			if (scene->zones[0]->name == "zone283") {
				for (uint8 i = 0; i < 5; i++) {
					uint16 left = _practice_target_left[i] - _videoPosX;
					uint16 right = _practice_target_right[i] - _videoPosX;
					uint16 top = _practice_target_top[i] - _videoPosY;
					uint16 bottom = _practice_target_bottom[i] - _videoPosY;
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
DebuggerCrimePatrol::DebuggerCrimePatrol(GameCrimePatrol *game) : GUI::Debugger() {
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
		_game->debug_warpTo(val);
		return false;
	}
}

bool DebuggerCrimePatrol::cmdDumpLib(int argc, const char **argv) {
	return _game->debug_dumpLibFile();
}

} // End of namespace Alg
