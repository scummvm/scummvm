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

#include "alg/game_spacepirates.h"
#include "alg/graphics.h"
#include "alg/scene.h"

namespace Alg {

GameSpacePirates::GameSpacePirates(AlgEngine *vm, const ADGameDescription *desc) : Game(vm) {
	if (scumm_stricmp(desc->gameId, "spiratess") == 0) {
		_libFileName = "spss.lib";
	} else if (scumm_stricmp(desc->gameId, "spiratesd") == 0) {
		_libFileName = "spds.lib";
	} else if (scumm_stricmp(desc->gameId, "spiratesdemo") == 0) {
		_libFileName = "sp.lib";
		_isDemo = true;
	}
}

GameSpacePirates::~GameSpacePirates() {
}

void GameSpacePirates::init() {
	_videoPosX = 11;
	_videoPosY = 2;

	_difficulty = 0;
	_lives = 3;
	_shots = 10;

	loadLibArchive(_libFileName);
	_sceneInfo->loadScnFile(_isDemo ? "spacepir.scn" : "sp.scn");
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

	_submenzone->addRect(0x24, 0x16, 0x64, 0x26, nullptr, 0, "STARTMENU", "0");
	_submenzone->addRect(0x24, 0x36, 0x64, 0x46, nullptr, 0, "RECTLOAD", "0");
	_submenzone->addRect(0x24, 0x56, 0x64, 0x66, nullptr, 0, "RECTSAVE", "0");
	_submenzone->addRect(0x24, 0x76, 0x64, 0x86, nullptr, 0, "CONTMENU", "0");
	_submenzone->addRect(0x24, 0x96, 0x64, 0xA6, nullptr, 0, "EXITMENU", "0");
	_submenzone->addRect(0xD5, 0x3B, 0x0115, 0x4B, nullptr, 0, "RECTEASY", "0");
	_submenzone->addRect(0xD5, 0x63, 0x0115, 0x73, nullptr, 0, "RECTAVG", "0");
	_submenzone->addRect(0xD5, 0x90, 0x0115, 0xA0, nullptr, 0, "RECTHARD", "0");

	_shotSound = _LoadSoundFile("phaser.8b");
	_emptySound = _LoadSoundFile("emptygun.8b");
	_saveSound = _LoadSoundFile("saved.8b");
	_loadSound = _LoadSoundFile("loaded.8b");
	_skullSound = _LoadSoundFile("error.8b");
	_easySound = _LoadSoundFile("difflev.8b");
	_avgSound = _LoadSoundFile("difflev.8b");
	_hardSound = _LoadSoundFile("difflev.8b");

	_gun = AlgGraphics::loadScreenCoordAniImage("gun.ani", _palette);
	_difficultyIcon = (*_gun)[1];
	_numbers = AlgGraphics::loadAniImage("numbers.ani", _palette);
	Common::Array<Graphics::Surface> *bullets = AlgGraphics::loadAniImage("bullets.ani", _palette);
	_shotIcon = (*bullets)[0];
	_emptyIcon = (*bullets)[1];
	Common::Array<Graphics::Surface> *lives = AlgGraphics::loadAniImage("lives.ani", _palette);
	_liveIcon1 = (*lives)[0];
	_liveIcon2 = (*lives)[1];
	_liveIcon3 = (*lives)[2];
	_deadIcon = (*lives)[3];
	Common::Array<Graphics::Surface> *hole = AlgGraphics::loadScreenCoordAniImage("hole.ani", _palette);
	_bulletholeIcon = (*hole)[0];

	_background = AlgGraphics::loadVgaBackground("backgrnd.vga", _palette);
	_screen->copyRectToSurface(_background->getPixels(), _background->pitch, 0, 0, _background->w, _background->h);

	_MoveMouse();
}

void GameSpacePirates::registerScriptFunctions() {
#define RECT_HIT_FUNCTION(name, func) _rectHitFuncs[name] = new SPScriptFunctionRect(this, &GameSpacePirates::func);
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
	RECT_HIT_FUNCTION("KILL_INNOCENT_PERSON", _rect_kill_innocent_person);
	RECT_HIT_FUNCTION("CONTINUE_JUNK_RINGS", _rect_continue_junk_rings);
	RECT_HIT_FUNCTION("SHOT_GRIN_REAPER", _rect_shot_grin_reaper);
	RECT_HIT_FUNCTION("SHOW_MAD_DOG", _rect_show_mad_dog);
	RECT_HIT_FUNCTION("POTT_WORLD_SHOW_CRYSTAL", _rect_pott_world_show_crystal);
	RECT_HIT_FUNCTION("SHOT_LEFT", _rect_shot_left);
	RECT_HIT_FUNCTION("SHOT_RIGHT", _rect_shot_right);
	RECT_HIT_FUNCTION("SHOT_GOLD", _rect_shot_gold);
	RECT_HIT_FUNCTION("SHOT_SILVER", _rect_shot_silver);
	RECT_HIT_FUNCTION("SELECTED_DUNE_WORLD", _rect_selected_dune_world);
	RECT_HIT_FUNCTION("SELECTED_JUNK_WORLD", _rect_selected_junk_world);
	RECT_HIT_FUNCTION("SELECTED_DRAGONS_TEETH_WORLD", _rect_selected_dragons_teeth_world);
	RECT_HIT_FUNCTION("SELECTED_VOLCANO_WORLD", _rect_selected_volcano_world);
	RECT_HIT_FUNCTION("SHOT_RED_DEATH_GRIP", _rect_shot_red_death_grip);
	RECT_HIT_FUNCTION("SHOT_BLUE_DEATH_GRIP", _rect_shot_blue_death_grip);
	RECT_HIT_FUNCTION("SHOT_GREEN_DEATH_GRIP", _rect_shot_green_death_grip);
	RECT_HIT_FUNCTION("SHOT_YELLOW", _rect_shot_yellow);
	RECT_HIT_FUNCTION("SHOT_BLUE", _rect_shot_blue);
	RECT_HIT_FUNCTION("SHOT_RED_CRYSTAL", _rect_shot_red_crystal);
	RECT_HIT_FUNCTION("SHOT_BLUE_CRYSTAL", _rect_shot_blue_crystal);
	RECT_HIT_FUNCTION("SHOT_GREEN_CRYSTAL", _rect_shot_green_crystal);
	RECT_HIT_FUNCTION("SHOT_BLACK_DRAGON_1", _rect_shot_black_dragon_1);
	RECT_HIT_FUNCTION("SHOT_BLACK_DRAGON_2", _rect_shot_black_dragon_2);
	RECT_HIT_FUNCTION("SHOT_BLACK_DRAGON_3", _rect_shot_black_dragon_3);
	RECT_HIT_FUNCTION("DO_FLYING_SKULL", _rect_do_flying_skull);
	RECT_HIT_FUNCTION("SKIP_SCENE", _rect_skip_scene);
	RECT_HIT_FUNCTION("HIT_PIRATE_SHIP", _rect_hit_pirate_ship);
#undef RECT_HIT_FUNCTION

#define PRE_OPS_FUNCTION(name, func) _scenePreOps[name] = new SPScriptFunctionScene(this, &GameSpacePirates::func);
	PRE_OPS_FUNCTION("DEFAULT", _scene_pso_drawrct);
	PRE_OPS_FUNCTION("PAUSE", _scene_pso_pause);
	PRE_OPS_FUNCTION("PAUSE_THEN_FADEIN", _scene_pso_pause_fadein);
	PRE_OPS_FUNCTION("FADEIN_VIDEO", _scene_pso_fadein_video);
	PRE_OPS_FUNCTION("FADEIN_SCREEN", _scene_pso_fadein);
	PRE_OPS_FUNCTION("SET_GOT_TO", _scene_pso_set_got_to);
	PRE_OPS_FUNCTION("SET_GOT_TO_NO_FADEIN", _scene_pso_set_got_to_no_fadein);
	PRE_OPS_FUNCTION("SET_WORLD_GOT_TO", _scene_pso_set_world_got_to);

#undef PRE_OPS_FUNCTION

#define INS_OPS_FUNCTION(name, func) _sceneInsOps[name] = new SPScriptFunctionScene(this, &GameSpacePirates::func);
	INS_OPS_FUNCTION("DEFAULT", _scene_iso_donothing);
	INS_OPS_FUNCTION("PAUSE", _scene_iso_pause);
	INS_OPS_FUNCTION("PICK_A_WORLD", _scene_iso_pick_a_world);
	INS_OPS_FUNCTION("SET_WORLD_GOT_TO", _scene_iso_set_world_got_to);
#undef INS_OPS_FUNCTION

#define NXT_SCN_FUNCTION(name, func) _sceneNxtScn[name] = new SPScriptFunctionScene(this, &GameSpacePirates::func);
	NXT_SCN_FUNCTION("DEFAULT", _scene_default_nxtscn);
	NXT_SCN_FUNCTION("GET_CHEWED_OUT", _scene_nxtscn_got_chewed_out);
	NXT_SCN_FUNCTION("RESTART_FROM_LAST", _scene_nxtscn_restart_from_last);
	NXT_SCN_FUNCTION("PLAYER_DIED", _scene_nxtscn_player_died);
	NXT_SCN_FUNCTION("MISC_ROOMS_1", _scene_nxtscn_misc_rooms_1);
	NXT_SCN_FUNCTION("PICK_DUNGEON_CLUE", _scene_nxtscn_pick_dungeon_clue);
	NXT_SCN_FUNCTION("CONTINUE_DUNGEON_CLUE", _scene_nxtscn_continue_dungeon_clue);
	NXT_SCN_FUNCTION("START_MIDSHIP_RANDOM_SCENE", _scene_nxtscn_start_midship_random_scene);
	NXT_SCN_FUNCTION("CONTINUE_MIDSHIP_RANDOM_SCENE", _scene_nxtscn_continue_midship_random_scene);
	NXT_SCN_FUNCTION("SHOW_DEATH_GRIP_BEAM_COLOR", _scene_nxtscn_show_death_grip_beam_color);
	NXT_SCN_FUNCTION("SELECT_ASTEROIDS", _scene_nxtscn_select_asteroids);
	NXT_SCN_FUNCTION("ASTEROIDS_DONE", _scene_nxtscn_asteroids_done);
	NXT_SCN_FUNCTION("DO_FLYING_SKULLS", _scene_nxtscn_do_flying_skulls);
	NXT_SCN_FUNCTION("DID_FLYING_SKULLS", _scene_nxtscn_did_flying_skulls);
	NXT_SCN_FUNCTION("SHOW_WHICH_STAR_SPLITTER", _scene_nxtscn_show_which_start_splitter);
	NXT_SCN_FUNCTION("GOTO_SELECTED_WORLD", _scene_nxtscn_goto_selected_world);
	NXT_SCN_FUNCTION("START_VOLCANO_POPUP", _scene_nxtscn_start_volcano_popup);
	NXT_SCN_FUNCTION("CONTINUE_VOLCANO_POPUP", _scene_nxtscn_continue_volcano_popup);
	NXT_SCN_FUNCTION("GIVE_FALINA_CLUE", _scene_nxtscn_give_falina_clue);
	NXT_SCN_FUNCTION("CHECK_FALINAS_CLUES", _scene_nxtscn_check_falina_clues);
	NXT_SCN_FUNCTION("SETUP_FALINA_TARGET_PRACTICE", _scene_nxtscn_setup_falina_target_practice);
	NXT_SCN_FUNCTION("CONTINUE_FALINA_TARGET_PRACTICE", _scene_nxtscn_continue_falina_target_practice);
	NXT_SCN_FUNCTION("START_DUNE_POPUP", _scene_nxtscn_start_dune_popup);
	NXT_SCN_FUNCTION("CONTINUE_DUNE_POPUP", _scene_nxtscn_continue_dune_popup);
	NXT_SCN_FUNCTION("POTT_OR_PAN_SHOOTS", _scene_nxtscn_pott_or_pan_shoots);
	NXT_SCN_FUNCTION("SETUP_POTT_TARGET_PRACTICE", _scene_nxtscn_setup_pott_target_practice);
	NXT_SCN_FUNCTION("CONTINUE_POTT_TARGET_PRACTICE", _scene_nxtscn_continue_pott_target_practice);
	NXT_SCN_FUNCTION("START_DRAGONS_TEETH_POPUP", _scene_nxtscn_start_dragons_teeth_popup);
	NXT_SCN_FUNCTION("CONTINUE_DRAGONS_TEETH_POPUP", _scene_nxtscn_continue_dragons_teeth_popup);
	NXT_SCN_FUNCTION("GRIN_REAPER_CLUE", _scene_nxtscn_grin_reaper_clue);
	NXT_SCN_FUNCTION("START_GRIN_REAPER", _scene_nxtscn_start_grin_reaper);
	NXT_SCN_FUNCTION("CONTINUE_GRIN_REAPER", _scene_nxtscn_continue_grin_reaper);
	NXT_SCN_FUNCTION("GRIN_TARGET_PRACTICE", _scene_nxtscn_grin_target_practice);
	NXT_SCN_FUNCTION("CONTINUE_GRIN_TARGET_PRACTICE", _scene_nxtscn_continue_grin_target_practice);
	NXT_SCN_FUNCTION("START_JUNK_WORLD", _scene_nxtscn_start_junk_world);
	NXT_SCN_FUNCTION("CONTINUE_JUNK_WORLD", _scene_nxtscn_continue_junk_world);
	NXT_SCN_FUNCTION("START_JUNK_RINGS", _scene_nxtscn_start_junk_rings);
	NXT_SCN_FUNCTION("SHOW_JUNK_WORLD_CRYSTAL", _scene_nxtscn_show_junk_world_crystal);
	NXT_SCN_FUNCTION("START_JUNK_WORLD_TARGET_PRACTICE", _scene_nxtscn_start_junk_world_target_practice);
	NXT_SCN_FUNCTION("CONTINUE_JUNK_WORLD_TARGET_PRACTICE", _scene_nxtscn_continue_junk_world_target_practice);
	NXT_SCN_FUNCTION("ARE_ALL_WORLDS_DONE", _scene_nxtscn_are_all_worlds_done);
	NXT_SCN_FUNCTION("START_PRACTICE_PIRATE_SHIP", _scene_nxtscn_start_practice_pirate_ship);
	NXT_SCN_FUNCTION("MORE_PRACTICE_PIRATE_SHIP", _scene_nxtscn_more_practice_pirate_ship);
	NXT_SCN_FUNCTION("PLAYER_WON", _scene_nxtscn_player_won);
#undef NXT_SCN_FUNCTION

#define MISSEDRECTS_FUNCTION(name, func) _sceneMissedRects[name] = new SPScriptFunctionScene(this, &GameSpacePirates::func);
	MISSEDRECTS_FUNCTION("DEFAULT", _scene_missedrects_default);
	MISSEDRECTS_FUNCTION("MISSED_PIRATE_SHIP", _scene_missedrects_missed_pirate_ship);
#undef MISSEDRECTS_FUNCTION

	_sceneShowMsg["DEFAULT"] = new SPScriptFunctionScene(this, &GameSpacePirates::_scene_sm_donothing);
	_sceneWepDwn["DEFAULT"] = new SPScriptFunctionScene(this, &GameSpacePirates::_scene_default_wepdwn);
	_sceneScnScr["DEFAULT"] = new SPScriptFunctionScene(this, &GameSpacePirates::_scene_default_score);
	_sceneNxtFrm["DEFAULT"] = new SPScriptFunctionScene(this, &GameSpacePirates::_scene_nxtfrm);
}

void GameSpacePirates::verifyScriptFunctions() {
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
		getScriptFunctionScene(MISSEDRECTS, scene->missedRects);
		for (size_t j = 0; j < scene->zones.size(); j++) {
			Zone *zone = scene->zones[j];
			for (size_t k = 0; k < zone->rects.size(); k++) {
				getScriptFunctionRectHit(zone->rects[k].rectHit);
			}
		}
	}
}

SPScriptFunctionRect GameSpacePirates::getScriptFunctionRectHit(Common::String name) {
	SPScriptFunctionRectMap::iterator it = _rectHitFuncs.find(name);
	if (it != _rectHitFuncs.end()) {
		return (*(*it)._value);
	} else {
		error("Could not find rectHit function: %s", name.c_str());
	}
}

SPScriptFunctionScene GameSpacePirates::getScriptFunctionScene(SceneFuncType type, Common::String name) {
	SPScriptFunctionSceneMap *functionMap;
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
	case MISSEDRECTS:
		functionMap = &_sceneMissedRects;
		break;
	default:
		error("Unkown scene script type: %u", type);
		break;
	}
	SPScriptFunctionSceneMap::iterator it;
	it = functionMap->find(name);
	if (it != functionMap->end()) {
		return (*(*it)._value);
	} else {
		error("Could not find scene type %u function: %s", type, name.c_str());
	}
}

void GameSpacePirates::callScriptFunctionRectHit(Common::String name, Rect *rect) {
	SPScriptFunctionRect function = getScriptFunctionRectHit(name);
	function(rect);
}

void GameSpacePirates::callScriptFunctionScene(SceneFuncType type, Common::String name, Scene *scene) {
	SPScriptFunctionScene function = getScriptFunctionScene(type, name);
	function(scene);
}

Common::Error GameSpacePirates::run() {
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
		_next_scene_found = false;
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
								callScriptFunctionScene(MISSEDRECTS, scene->missedRects, scene);
							}
						} else {
							_PlaySound(_emptySound);
						}
					}
				}
			}
			if (_next_scene_found && _cur_scene == oldscene) {
				callScriptFunctionScene(NXTSCN, scene->nxtscn, scene);
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

void GameSpacePirates::_NewGame() {
	_holster = false;
	_shots = 10;
}

void GameSpacePirates::_ResetParams() {
	if (_game_loaded) {
		_oldDifficulty = _difficulty;
		_lives = _lives_loaded;
		_shots = _shots_loaded;
		_score = _score_loaded;
		_difficulty = _difficulty_loaded;
		_ChangeDifficulty(_oldDifficulty);
	} else {
		_ChangeDifficulty(0);
		_random_count_asteroids = 0;
		_scene_before_flying_skulls = 0;
		_misc_rooms_count = 0;
		_player_died = 0;
		_got_to = 0;
		_selected_a_world = 0;
		_selected_world_start = 0;
		_current_world = -1;
		_world_got_to[0] = 0x91;
		_world_got_to[1] = 0x0103;
		_world_got_to[2] = 0xCE;
		_world_got_to[3] = 0x72;
		_world_done[0] = false;
		_world_done[1] = false;
		_world_done[2] = false;
		_world_done[3] = false;
		_crystal_state = 0;
		_crystals_shot = 1;
		_last_extra_life_score = 0;
	}
}

void GameSpacePirates::_DoMenu() {
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

void GameSpacePirates::_ChangeDifficulty(uint8 newDifficulty) {
	if (newDifficulty == _oldDifficulty) {
		return;
	}
	_ShowDifficulty(newDifficulty, true);
	_oldDifficulty = newDifficulty;
	_difficulty = newDifficulty;
}

void GameSpacePirates::_ShowDifficulty(uint8 newDifficulty, bool updateCursor) {
	// reset menu screen
	_screen->copyRectToSurface(_background->getBasePtr(_videoPosX, _videoPosY), _background->pitch, _videoPosX, _videoPosY, _videoDecoder->getWidth(), _videoDecoder->getHeight());
	uint16 posY = 0x31;
	if (newDifficulty == 1) {
		posY = 0x5B;
	} else if (newDifficulty == 2) {
		posY = 0x86;
	}
	AlgGraphics::drawImageCentered(_screen, &_difficultyIcon, 0x0111, posY);
	if (updateCursor) {
		_DoCursor();
	}
}

void GameSpacePirates::_DoCursor() {
	_UpdateMouse();
}

void GameSpacePirates::_UpdateMouse() {
	if (_oldWhichGun != _whichGun) {
		Graphics::PixelFormat pixelFormat = Graphics::PixelFormat::createFormatCLUT8();
		Graphics::Surface *cursor = &(*_gun)[_whichGun];
		CursorMan.popAllCursors();
		uint16 hotspotX = (cursor->w / 2) + 8;
		uint16 hotspotY = (cursor->h / 2) + 10;
		if (_whichGun == 2) {
			hotspotY += 14;
		}
		if (debugChannelSet(1, Alg::kAlgDebugGraphics)) {
			cursor->drawLine(0, hotspotY, cursor->w, hotspotY, 1);
			cursor->drawLine(hotspotX, 0, hotspotX, cursor->h, 1);
		}
		CursorMan.pushCursor(cursor->getPixels(), cursor->w, cursor->h, hotspotX, hotspotY, 0, false, &pixelFormat);
		CursorMan.showMouse(true);
		_oldWhichGun = _whichGun;
	}
}

void GameSpacePirates::_MoveMouse() {
	if (_inMenu) {
		_whichGun = 2; // in menu cursor
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

void GameSpacePirates::_DisplayLivesLeft() {
	if (_lives == _oldLives) {
		return;
	}

	int16 posY = 0x73;
	int16 posX = 0x0130;
	int16 margin = 14;
	if(_isDemo) {
		posY = 0x68;
		posX = 0x012F;
		margin = 13;
	}
	for (uint8 i = 0; i < 3; i++) {
		AlgGraphics::drawImage(_screen, &_deadIcon, posX, posY + (i * margin));
	}
	if (_lives > 2) {
		AlgGraphics::drawImage(_screen, &_liveIcon3, posX, posY + (margin * 2));
	}
	if (_lives > 1) {
		AlgGraphics::drawImage(_screen, &_liveIcon2, posX, posY + margin);
	}
	if (_lives > 0) {
		AlgGraphics::drawImage(_screen, &_liveIcon1, posX, posY);
	}
	_oldLives = _lives;
}

void GameSpacePirates::_DisplayScores() {
	if (_score == _oldScore) {
		return;
	}
	Common::String scoreString = Common::String::format("%05d", _score);
	int posX = 0x71;
	for (int i = 0; i < 5; i++) {
		uint8 digit = scoreString[i] - '0';
		AlgGraphics::drawImage(_screen, &(*_numbers)[digit], posX, 0xBF);
		posX += 7;
	}
	_oldScore = _score;
}

void GameSpacePirates::_DisplayShotsLeft() {
	if (_shots == _oldShots) {
		return;
	}
	uint16 posX = 0xAA;
	for (uint8 i = 0; i < 10; i++) {
		AlgGraphics::drawImage(_screen, &_emptyIcon, posX, 0xBF);
		posX += 8;
	}
	posX = 0xAA;
	for (uint8 i = 0; i < _shots; i++) {
		AlgGraphics::drawImage(_screen, &_shotIcon, posX, 0xBF);
		posX += 8;
	}
	_oldShots = _shots;
}

bool GameSpacePirates::_WeaponDown() {
	if (_rightDown && _mousePos.y >= 0xAA && _mousePos.x >= 0x113) {
		return true;
	}
	return false;
}

bool GameSpacePirates::_SaveState() {
	Scene *scene = _sceneInfo->findScene(_cur_scene);
	uint16 sceneNum = _SceneToNumber(scene);
	if ((sceneNum < 0xAC || sceneNum > 0xB9) && sceneNum != 0x6F) {
		Common::OutSaveFile *outSaveFile;
		Common::String saveFileName = _vm->getSaveStateName(0);
		if (!(outSaveFile = g_system->getSavefileManager()->openForSaving(saveFileName))) {
			warning("Can't create file '%s', game not saved", saveFileName.c_str());
			return false;
		}
		outSaveFile->writeUint32BE(MKTAG('A', 'L', 'G', 'S')); // header
		outSaveFile->writeByte(0);                             // version, unused for now
		outSaveFile->writeSByte(_lives);
		outSaveFile->writeUint16LE(_shots);
		outSaveFile->writeUint32LE(_score);
		outSaveFile->writeByte(_difficulty);
		outSaveFile->writeUint16LE(sceneNum);
		for (uint8 i = 0; i < 9; i++) {
			outSaveFile->writeUint16LE(_random_scenes_values[i]);
			outSaveFile->writeByte(_random_scenes_used[i]);
		}
		for (uint8 i = 0; i < 4; i++) {
			outSaveFile->writeByte(_world_done[i]);
			outSaveFile->writeUint16LE(_world_got_to[i]);
		}
		outSaveFile->writeByte(_max_random);
		outSaveFile->writeByte(_random_count_asteroids);
		outSaveFile->writeUint16LE(_scene_before_flying_skulls);
		outSaveFile->writeByte(_misc_rooms_count);
		outSaveFile->writeByte(_random_count_midship);
		outSaveFile->writeByte(_random_count);
		outSaveFile->writeUint16LE(_got_to);
		outSaveFile->writeByte(_player_died);
		outSaveFile->writeUint16LE(_clue);
		outSaveFile->writeByte(_shot_color);
		outSaveFile->writeByte(_shot_direction);
		outSaveFile->writeUint16LE(_picked_start_splitter);
		outSaveFile->writeSByte(_current_world);
		outSaveFile->writeByte(_selected_a_world);
		outSaveFile->writeUint16LE(_selected_world_start);
		outSaveFile->writeByte(_crystal_state);
		outSaveFile->writeByte(_shot_grin_reaper_count);
		outSaveFile->writeByte(_crystals_shot);
		outSaveFile->writeUint32LE(_last_extra_life_score);
		outSaveFile->finalize();
		delete outSaveFile;
		return true;
	}
	return false;
}

bool GameSpacePirates::_LoadState() {
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
	_lives = inSaveFile->readSByte();
	_shots = inSaveFile->readUint16LE();
	_score = inSaveFile->readUint32LE();
	_difficulty = inSaveFile->readByte();
	uint16 sceneNum = inSaveFile->readUint16LE();
	for (uint8 i = 0; i < 9; i++) {
		_random_scenes_values[i] = inSaveFile->readUint16LE();
		_random_scenes_used[i] = inSaveFile->readByte();
	}
	for (uint8 i = 0; i < 4; i++) {
		_world_done[i] = inSaveFile->readByte();
		_world_got_to[i] = inSaveFile->readUint16LE();
	}
	_max_random = inSaveFile->readByte();
	_random_count_asteroids = inSaveFile->readByte();
	_scene_before_flying_skulls = inSaveFile->readUint16LE();
	_misc_rooms_count = inSaveFile->readByte();
	_random_count_midship = inSaveFile->readByte();
	_random_count = inSaveFile->readByte();
	_got_to = inSaveFile->readUint16LE();
	_player_died = inSaveFile->readByte();
	_clue = inSaveFile->readUint16LE();
	_shot_color = inSaveFile->readByte();
	_shot_direction = inSaveFile->readByte();
	_picked_start_splitter = inSaveFile->readUint16LE();
	_current_world = inSaveFile->readSByte();
	_selected_a_world = inSaveFile->readByte();
	_selected_world_start = inSaveFile->readUint16LE();
	_crystal_state = inSaveFile->readByte();
	_shot_grin_reaper_count = inSaveFile->readByte();
	_crystals_shot = inSaveFile->readByte();
	_last_extra_life_score = inSaveFile->readUint32LE();
	delete inSaveFile;
	_game_loaded = true;
	_gameInProgress = true;
	_lives_loaded = _lives;
	_shots_loaded = _shots;
	_score_loaded = _score;
	_difficulty_loaded = _difficulty;
	_cur_scene = Common::String::format("scene%d", sceneNum);
	_ChangeDifficulty(_difficulty);
	if (!(_world_done[0] && _world_done[1] && _world_done[2] && _world_done[3]) && _current_world != -1) {
		_cur_scene = "scene110";
	}
	return true;
}

// misc game functions
void GameSpacePirates::_PlayErrorSound() {
	_PlaySound(_skullSound);
}

void GameSpacePirates::_DisplayShotFiredImage() {
	_DisplayShotFiredImage(&_mousePos);
}

void GameSpacePirates::_DisplayShotFiredImage(Common::Point *point) {
	if (point->x >= _videoPosX && point->x <= (_videoPosX + _videoDecoder->getWidth()) && point->y >= _videoPosY && point->y <= (_videoPosY + _videoDecoder->getHeight())) {
		uint16 targetX = point->x - _videoPosX - 4;
		uint16 targetY = point->y - _videoPosY - 4;
		AlgGraphics::drawImageCentered(_videoDecoder->getVideoFrame(), &_bulletholeIcon, targetX, targetY);
	}
}

void GameSpacePirates::_DisplayShotLine(uint16 startX, uint16 startY, uint16 endX, uint16 endY) {
	int16 currentX = startX;
	int16 currentY = startY;
	int16 deltaX = endX - startX;
	int16 deltaY = endY - startY;
	int16 stepX, stepY;
	int16 steep = 0, accum = 0, error = 0;
	if (deltaX > 0)
		stepX = 1;
	else if (deltaX == 0)
		stepX = 0;
	else
		stepX = -1;
	if (deltaY > 0)
		stepY = 1;
	else if (deltaY == 0)
		stepY = 0;
	else
		stepY = -1;
	deltaX = abs(deltaX);
	deltaY = abs(deltaY);
	steep = (deltaX > deltaY) ? deltaX : deltaY;
	for (int16 count = 0; count <= steep; count++) {
		if (count % 10 == 0) {
			Common::Point position = Common::Point(currentX, currentY);
			_DisplayShotFiredImage(&position);
		}
		accum += deltaX;
		error += deltaY;
		if (accum > steep) {
			accum -= steep;
			currentX += stepX;
		}
		if (error > steep) {
			error -= steep;
			currentY += stepY;
		}
	}
}

void GameSpacePirates::_DisplayMultipleShotLines() {
	_DisplayShotLine(0x73, 0x37, _mousePos.x, _mousePos.y);
	_DisplayShotLine(0x20, 0x9B, _mousePos.x, _mousePos.y);
	_DisplayShotLine(0x73, 0xFB, _mousePos.x, _mousePos.y);
}

void GameSpacePirates::_EnableVideoFadeIn() {
	// TODO implement
}

uint16 GameSpacePirates::_SceneToNumber(Scene *scene) {
	return atoi(scene->name.substr(5).c_str());
}

uint16 GameSpacePirates::_RandomUnusedScene(uint8 max) {
	bool found = 0;
	uint8 random = 0;
	for (uint8 i = 0; i < max && !found; i++) {
		random = _rnd->getRandomNumber(max - 1);
		if (_random_scenes_used[random] == 0) {
			found = true;
		}
	}
	if (!found) {
		for (uint8 i = 0; i < max && !found; i++) {
			if (_random_scenes_used[i] == 0) {
				found = true;
				random = i;
				break;
			}
		}
	}
	_random_scenes_used[random] = 1;
	return _random_scenes_values[random];
}

uint16 GameSpacePirates::_RandomNumberInRange(uint16 min, uint16 max) {
	uint16 random;
	do {
		random = _rnd->getRandomNumberRng(min, max);
	} while (random == _random_picked);
	_random_picked = random;
	return random;
}

uint16 GameSpacePirates::_PickCrystalScene(uint16 scene1, uint16 scene2, uint16 scene3) {
	uint16 picked = 0;
	if (_crystal_state == 7) {
		picked = 110;
	} else {
		if ((_crystal_state & 1) == 0) {
			picked = scene1;
			_crystal_state |= 1;
		} else if ((_crystal_state & 2) == 0) {
			picked = scene2;
			_crystal_state |= 2;
		} else if ((_crystal_state & 4) == 0) {
			picked = scene3;
			_crystal_state |= 4;
		}
	}
	return picked;
}

// Script functions: RectHit
void GameSpacePirates::_rect_shotmenu(Rect *rect) {
	_DoMenu();
}

void GameSpacePirates::_rect_save(Rect *rect) {
	if (_SaveState()) {
		_DoSaveSound();
	}
}

void GameSpacePirates::_rect_load(Rect *rect) {
	if (_LoadState()) {
		_DoLoadSound();
	}
}

void GameSpacePirates::_rect_continue(Rect *rect) {
	_inMenu = false;
	_fired = false;
}

void GameSpacePirates::_rect_start(Rect *rect) {
	_ChangeDifficulty(0);
	_game_loaded = false;
	_inMenu = false;
	_fired = false;
	_gameInProgress = true;
	_cur_scene = _isDemo ? "scene62" : "scene187";
	_ResetParams();
	_NewGame();
}

void GameSpacePirates::_rect_easy(Rect *rect) {
	_DoDiffSound(1);
	_difficulty = 0;
}

void GameSpacePirates::_rect_average(Rect *rect) {
	_DoDiffSound(2);
	_difficulty = 1;
}

void GameSpacePirates::_rect_hard(Rect *rect) {
	_DoDiffSound(3);
	_difficulty = 2;
}

void GameSpacePirates::_rect_default(Rect *rect) {
	_DisplayShotFiredImage();
	_DoShot();
	_score += rect->score;
	if (_score - _last_extra_life_score >= 1500 && _lives < 3) {
		_lives++;
		_last_extra_life_score = _score;
	}
	if (!rect->scene.empty()) {
		_next_scene_found = true;
		_cur_scene = rect->scene;
	}
}

void GameSpacePirates::_rect_kill_innocent_person(Rect *rect) {
	_DisplayShotFiredImage();
	_DoShot();
	if (!_debug_godMode) {
		_lives--;
	}
	_next_scene_found = true;
	_player_died = true;
	if (_isDemo) {
		_cur_scene = "scene185";
		return;
	}
	Scene *scene = _sceneInfo->findScene(rect->scene);
	uint16 picked = _SceneToNumber(scene);
	if (picked == 0) {
		picked = _RandomNumberInRange(0xB7, 0xB9);
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_rect_continue_junk_rings(Rect *rect) {
	_DisplayShotFiredImage();
	_DoShot();
	_random_count++;
	uint16 picked = 0;
	if (_random_count >= 10) {
		if (_crystal_state == 7) {
			picked = 0x124;
		} else {
			picked = 0x119;
		}
	} else {
		picked = _RandomNumberInRange(0x011E, 0x0122);
	}
	_next_scene_found = true;
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_rect_shot_grin_reaper(Rect *rect) {
	_DisplayShotFiredImage();
	_DoShot();
	_shot_grin_reaper_count++;
	uint16 picked = 0;
	if (_clue - 223 <= _shot_grin_reaper_count) {
		_next_scene_found = true;
		if (_crystal_state == 7) {
			picked = 0xF9;
		} else {
			picked = _PickCrystalScene(0xF6, 0xF7, 0xF8);
		}
	} else {
		picked = 0;
	}
	if (picked != 0) {
		_cur_scene = Common::String::format("scene%d", picked);
	}
}

void GameSpacePirates::_rect_show_mad_dog(Rect *rect) {
	_DisplayShotFiredImage();
	_DoShot();
	_next_scene_found = true;
	_cur_scene = "scene354";
}

void GameSpacePirates::_rect_pott_world_show_crystal(Rect *rect) {
	_DisplayShotFiredImage();
	_DoShot();
	uint16 picked = 0;
	if (_crystal_state == 7) {
		picked = 0xA6;
	} else {
		picked = _PickCrystalScene(0xA3, 0xA4, 0xA5);
	}
	_next_scene_found = true;
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_rect_shot_left(Rect *rect) {
	_DisplayShotFiredImage();
	_DoShot();
	_shot_direction = 1;
	_next_scene_found = true;
}

void GameSpacePirates::_rect_shot_right(Rect *rect) {
	_DisplayShotFiredImage();
	_DoShot();
	_shot_direction = 2;
	_next_scene_found = true;
}

void GameSpacePirates::_rect_shot_gold(Rect *rect) {
	_DisplayShotFiredImage();
	_DoShot();
	_shot_color = 3;
	_next_scene_found = true;
	_cur_scene = rect->scene;
}

void GameSpacePirates::_rect_shot_silver(Rect *rect) {
	_DisplayShotFiredImage();
	_DoShot();
	_shot_color = 4;
	_next_scene_found = true;
	_cur_scene = rect->scene;
}

void GameSpacePirates::_rect_selected_dune_world(Rect *rect) {
	_DisplayShotFiredImage();
	_DoShot();
	if (!_world_done[0]) {
		_selected_a_world = true;
		_current_world = 0;
		_selected_world_start = _world_got_to[0];
		_cur_scene = "scene111";
	} else {
		_PlayErrorSound();
	}
	_shots++;
}

void GameSpacePirates::_rect_selected_junk_world(Rect *rect) {
	_DisplayShotFiredImage();
	_DoShot();
	if (!_world_done[1]) {
		_selected_a_world = true;
		_current_world = 1;
		_selected_world_start = _world_got_to[1];
		_cur_scene = "scene111";
	} else {
		_PlayErrorSound();
	}
	_shots++;
}

void GameSpacePirates::_rect_selected_dragons_teeth_world(Rect *rect) {
	_DisplayShotFiredImage();
	_DoShot();
	if (!_world_done[2]) {
		_selected_a_world = true;
		_current_world = 2;
		_selected_world_start = _world_got_to[2];
		_cur_scene = "scene111";
	} else {
		_PlayErrorSound();
	}
	_shots++;
}

void GameSpacePirates::_rect_selected_volcano_world(Rect *rect) {
	_DisplayShotFiredImage();
	_DoShot();
	if (!_world_done[3]) {
		_selected_a_world = true;
		_current_world = 3;
		_selected_world_start = _world_got_to[3];
		_cur_scene = "scene111";
	} else {
		_PlayErrorSound();
	}
	_shots++;
}

void GameSpacePirates::_rect_shot_red_death_grip(Rect *rect) {
	uint16 picked = 0;
	_next_scene_found = true;
	if (_clue == 0x36) {
		_DisplayShotFiredImage();
		_DoShot();
		picked = 0x5A;
	} else {
		_rect_kill_innocent_person(rect);
		picked = _RandomNumberInRange(0xB7, 0xB9);
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_rect_shot_blue_death_grip(Rect *rect) {
	uint16 picked = 0;
	_next_scene_found = true;
	if (_clue == 0x38) {
		_DisplayShotFiredImage();
		_DoShot();
		picked = 0x5C;
	} else {
		_rect_kill_innocent_person(rect);
		picked = _RandomNumberInRange(0xB7, 0xB9);
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_rect_shot_green_death_grip(Rect *rect) {
	uint16 picked = 0;
	_next_scene_found = true;
	if (_clue == 0x37) {
		_DisplayShotFiredImage();
		_DoShot();
		picked = 0x5B;
	} else {
		_rect_kill_innocent_person(rect);
		picked = _RandomNumberInRange(0xB7, 0xB9);
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_rect_shot_yellow(Rect *rect) {
	_DisplayShotFiredImage();
	_DoShot();
	_shot_color = 0x0F;
	_next_scene_found = true;
}

void GameSpacePirates::_rect_shot_blue(Rect *rect) {
	_DisplayShotFiredImage();
	_DoShot();
	_shot_color = 0x0E;
	_next_scene_found = true;
}

void GameSpacePirates::_rect_shot_red_crystal(Rect *rect) {
	uint16 picked = 0;
	_DisplayShotFiredImage();
	_DoShot();
	Scene *scene = _sceneInfo->findScene(_cur_scene);
	if (_crystals_shot == 1) {
		if (_picked_start_splitter == 0x6A) {
			picked = 0xC6;
			_crystals_shot++;
		} else {
			_cur_scene = scene->next;
			return;
		}
	} else if (_crystals_shot == 2) {
		if (_picked_start_splitter == 0x6C) {
			picked = 0xC5;
			_crystals_shot++;
		} else {
			_cur_scene = scene->next;
			return;
		}
	} else if (_crystals_shot == 3) {
		if (_picked_start_splitter == 0x6B) {
			picked = 0x14E;
		} else {
			_cur_scene = scene->next;
			return;
		}
	}
	_next_scene_found = true;
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_rect_shot_blue_crystal(Rect *rect) {
	uint16 picked = 0;
	_DisplayShotFiredImage();
	_DoShot();
	Scene *scene = _sceneInfo->findScene(_cur_scene);
	if (_crystals_shot == 1) {
		if (_picked_start_splitter == 0x6C) {
			picked = 0xC4;
			_crystals_shot++;
		} else {
			_cur_scene = scene->next;
			return;
		}
	} else if (_crystals_shot == 2) {
		if (_picked_start_splitter == 0x6B) {
			picked = 0xC2;
			_crystals_shot++;
		} else {
			_cur_scene = scene->next;
			return;
		}
	} else if (_crystals_shot == 3) {
		if (_picked_start_splitter == 0x6A) {
			picked = 0x14E;
		} else {
			_cur_scene = scene->next;
			return;
		}
	}
	_next_scene_found = true;
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_rect_shot_green_crystal(Rect *rect) {
	uint16 picked = 0;
	_DisplayShotFiredImage();
	_DoShot();
	Scene *scene = _sceneInfo->findScene(_cur_scene);
	if (_crystals_shot == 1) {
		if (_picked_start_splitter == 0x6B) {
			picked = 0xC1;
			_crystals_shot++;
		} else {
			_cur_scene = scene->next;
			return;
		}
	} else if (_crystals_shot == 2) {
		if (_picked_start_splitter == 0x6A) {
			picked = 0xC7;
			_crystals_shot++;
		} else {
			_cur_scene = scene->next;
			return;
		}
	} else if (_crystals_shot == 3) {
		if (_picked_start_splitter == 0x6C) {
			picked = 0x14E;
		} else {
			_cur_scene = scene->next;
			return;
		}
	}
	_next_scene_found = true;
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_rect_shot_black_dragon_1(Rect *rect) {
	_DisplayShotFiredImage();
	_DoShot();
	_score += rect->score;
	_next_scene_found = true;
	_cur_scene = "scene203";
}

void GameSpacePirates::_rect_shot_black_dragon_2(Rect *rect) {
	_DisplayShotFiredImage();
	_DoShot();
	_score += rect->score;
	_next_scene_found = true;
	_cur_scene = "scene204";
}

void GameSpacePirates::_rect_shot_black_dragon_3(Rect *rect) {
	_DisplayShotFiredImage();
	_DoShot();
	_score += rect->score;
	_next_scene_found = true;
	_cur_scene = "scene335";
}

void GameSpacePirates::_rect_do_flying_skull(Rect *rect) {
	_DisplayShotFiredImage();
	_DoShot();
	_next_scene_found = true;
	Scene *scene = _sceneInfo->findScene(rect->scene);
	_scene_before_flying_skulls = _SceneToNumber(scene);
	uint16 picked = _RandomNumberInRange(0x014A, 0x014D);
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_rect_skip_scene(Rect *rect) {
	_DisplayShotFiredImage();
	_DoShot();
	_next_scene_found = true;
	Scene *scene = _sceneInfo->findScene(_cur_scene);
	_cur_scene = scene->next;
}

void GameSpacePirates::_rect_hit_pirate_ship(Rect *rect) {
	_DisplayShotFiredImage();
	_DoShot();
	_DisplayMultipleShotLines();
	_score += rect->score;
	_next_scene_found = true;
}

// Script functions: Scene PreOps
void GameSpacePirates::_scene_pso_fadein_video(Scene *scene) {
	// TODO implement
}

void GameSpacePirates::_scene_pso_set_got_to(Scene *scene) {
	_EnableVideoFadeIn();
	_got_to = _SceneToNumber(scene);
}

void GameSpacePirates::_scene_pso_set_got_to_no_fadein(Scene *scene) {
	_got_to = _SceneToNumber(scene);
}

void GameSpacePirates::_scene_pso_set_world_got_to(Scene *scene) {
	_EnableVideoFadeIn();
	uint16 sceneNum = _SceneToNumber(scene);
	_world_got_to[_current_world] = sceneNum;
}

// Script functions: Scene InsOps
void GameSpacePirates::_scene_iso_pick_a_world(Scene *scene) {
	Zone *zone = scene->zones[0];
	uint8 world = 3;
	for (size_t i = 0; i < zone->rects.size(); i++) {
		if (_world_done[world]) {
			uint16 centerX = zone->rects[i].left + (zone->rects[i].width() / 2);
			uint16 centerY = zone->rects[i].top + (zone->rects[i].height() / 2);
			AlgGraphics::drawImageCentered(_videoDecoder->getVideoFrame(), &(*_gun)[2], centerX - 16, centerY - 24);
		}
		world--;
	}
	if (_world_done[0] && _world_done[1] && _world_done[2] && _world_done[3]) {
		_current_world = -1;
		_next_scene_found = true;
	} else if (_selected_a_world) {
		if (_world_done[_current_world]) {
			_current_world = -1;
		}
	} else {
		_current_world = -1;
	}
}

void GameSpacePirates::_scene_iso_set_world_got_to(Scene *scene) {
	uint16 sceneNum = _SceneToNumber(scene);
	_world_got_to[_current_world] = sceneNum;
}

// Script functions: Scene NxtScn
void GameSpacePirates::_scene_nxtscn_got_chewed_out(Scene *scene) {
	uint16 picked = _RandomNumberInRange(0xB7, 0xB9);
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_scene_nxtscn_restart_from_last(Scene *scene) {
	uint16 picked = 0;
	if (!_player_died) {
		_cur_scene = scene->next;
	} else {
		if (_lives > 0) {
			picked = _got_to;
			_player_died = false;
		} else {
			_next_scene_found = true;
			_gameInProgress = false;
			_shots = 0;
			picked = 368;
		}
		_cur_scene = Common::String::format("scene%d", picked);
	}
}

void GameSpacePirates::_scene_nxtscn_player_died(Scene *scene) {
	uint16 picked = 0;
	_player_died = true;
	if (!_debug_godMode) {
		_lives--;
	}
	if (_lives > 0) {
		if (_isDemo) {
			picked = 178;
		} else {
			picked = _RandomNumberInRange(0xB2, 0xB4);
		}
	} else {
		if (_isDemo) {
			picked = 172;
		} else {
			uint8 random = _rnd->getRandomNumber(9);
			if (random < 5) {
				picked = _RandomNumberInRange(0xB5, 0xB6);
			} else {
				picked = _RandomNumberInRange(0xAC, 0xB1);
			}
		}
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_scene_nxtscn_misc_rooms_1(Scene *scene) {
	uint16 picked = 0;
	if (_misc_rooms_count == 0) {
		_picked_misc_rooms = 0;
		while (1) {
			uint8 random = _rnd->getRandomNumber(9);
			if (random <= 5) {
				picked = 0x1F;
			} else {
				picked = 0x21;
			}
			if (_picked_misc_rooms != picked) {
				_picked_misc_rooms = picked;
				break;
			}
		}
	} else {
		if (_misc_rooms_count < 2) {
			if (_picked_misc_rooms == 0x1F) {
				picked = 0x21;
			} else if (_picked_misc_rooms == 0x21) {
				picked = 0x1F;
			}
		} else {
			_cur_scene = scene->next;
			return;
		}
	}
	_misc_rooms_count++;
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_scene_nxtscn_pick_dungeon_clue(Scene *scene) {
	_clue = _RandomNumberInRange(14, 15);
	_shot_color = 0;
	_cur_scene = Common::String::format("scene%d", _clue);
}

void GameSpacePirates::_scene_nxtscn_continue_dungeon_clue(Scene *scene) {
	uint16 picked = 0;
	if (_clue == _shot_color) {
		if (_shot_color == 14) {
			picked = 0x15;
		} else {
			picked = 0x16;
		}
	} else {
		_scene_nxtscn_player_died(scene);
		picked = 0xAE;
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_scene_nxtscn_start_midship_random_scene(Scene *scene) {
	_max_random = 5;
	_random_count_midship = 0;
	_random_scenes_values[0] = 0x25;
	_random_scenes_values[1] = 0x27;
	_random_scenes_values[2] = 0x29;
	_random_scenes_values[3] = 0x2B;
	_random_scenes_values[4] = 0x2D;
	memset(&_random_scenes_used, 0, (9 * sizeof(uint8)));
	uint16 picked = _RandomUnusedScene(5);
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_scene_nxtscn_continue_midship_random_scene(Scene *scene) {
	uint16 picked = 0;
	_random_count_midship++;
	if (((_difficulty * 5) + 5) > _random_count_midship) {
		memset(&_random_scenes_used, 0, (9 * sizeof(uint8)));
		picked = _RandomUnusedScene(_max_random);
		_cur_scene = Common::String::format("scene%d", picked);
	} else {
		_cur_scene = scene->next;
	}
}

void GameSpacePirates::_scene_nxtscn_show_death_grip_beam_color(Scene *scene) {
	_clue = _RandomNumberInRange(0x36, 0x38);
	_shot_color = 0;
	_cur_scene = Common::String::format("scene%d", _clue);
}

void GameSpacePirates::_scene_nxtscn_select_asteroids(Scene *scene) {
	uint16 picked = _RandomNumberInRange(0x013E, 0x0142);
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_scene_nxtscn_asteroids_done(Scene *scene) {
	uint16 picked = 0;
	if (_random_count_asteroids < 4) {
		_random_count_asteroids++;
		picked = _RandomNumberInRange(0x013E, 0x0142);
	} else {
		_scene_nxtscn_do_flying_skulls(scene);
		return;
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_scene_nxtscn_do_flying_skulls(Scene *scene) {
	_scene_before_flying_skulls = _SceneToNumber(scene);
	uint16 picked = _RandomNumberInRange(0x014A, 0x014D);
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_scene_nxtscn_did_flying_skulls(Scene *scene) {
	uint16 picked = 0;
	switch (_scene_before_flying_skulls) {
	case 0x8A:
	case 0x8B:
	case 0x8C:
	case 0x8D:
	case 0x8E:
	case 0x8F:
	case 0x90:
	case 0xA8:
	case 0xA9:
	case 0xAB:
	case 0x128:
	case 0xFA:
	case 0xFE:
	case 0x126:
	case 0x12A:
	case 0x12B:
	case 0x12D:
	case 0x12F:
	case 0x131:
	case 0x132:
	case 0x133:
		picked = 0x6E;
		break;
	case 0xBB:
	case 0x143:
	case 0x144:
	case 0x145:
	case 0x146:
	case 0x148:
	case 0x149:
	case 0x14A:
	case 0x14B:
		picked = 0x18;
		break;
	case 0xCD:
		picked = 0x136;
		break;
	default:
		picked = 0;
		break;
	}
	assert(picked != 0);
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_scene_nxtscn_show_which_start_splitter(Scene *scene) {
	uint16 picked = _RandomNumberInRange(0x6A, 0x6C);
	_picked_start_splitter = picked;
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_scene_nxtscn_goto_selected_world(Scene *scene) {
	_selected_a_world = false;
	_cur_scene = Common::String::format("scene%d", _selected_world_start);
}

void GameSpacePirates::_scene_nxtscn_start_volcano_popup(Scene *scene) {
	_max_random = 3;
	_random_count = 0;
	_random_scenes_values[0] = 0x73;
	_random_scenes_values[1] = 0x75;
	_random_scenes_values[2] = 0x77;
	memset(&_random_scenes_used, 0, (9 * sizeof(uint8)));
	uint16 picked = _RandomUnusedScene(3);
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_scene_nxtscn_continue_volcano_popup(Scene *scene) {
	_random_count++;
	if (_random_count < 3) {
		uint16 picked = _RandomUnusedScene(_max_random);
		_cur_scene = Common::String::format("scene%d", picked);
	} else {
		_cur_scene = scene->next;
	}
}

void GameSpacePirates::_scene_nxtscn_give_falina_clue(Scene *scene) {
	_clue = _RandomNumberInRange(0x7F, 0x82);
	_shot_color = 0;
	_shot_direction = 0;
	_cur_scene = Common::String::format("scene%d", _clue);
}

void GameSpacePirates::_scene_nxtscn_check_falina_clues(Scene *scene) {
	uint16 picked = 0;
	if (_clue == 0x7F) {
		if (_shot_color == 3 && _shot_direction == 1) {
			if (_crystal_state == 7) {
				picked = 0x89;
			} else {
				picked = _PickCrystalScene(0x86, 0x87, 0x88);
			}
		} else {
			_cur_scene = scene->next;
			return;
		}
	} else if (_clue == 0x80) {
		if (_shot_color == 3 && _shot_direction == 2) {
			if (_crystal_state == 7) {
				picked = 0x89;
			} else {
				picked = _PickCrystalScene(0x86, 0x87, 0x88);
			}
		} else {
			_cur_scene = scene->next;
			return;
		}
	} else if (_clue == 0x81) {
		if (_shot_color == 4 && _shot_direction == 2) {
			if (_crystal_state == 7) {
				picked = 0x89;
			} else {
				picked = _PickCrystalScene(0x86, 0x87, 0x88);
			}
		} else {
			_cur_scene = scene->next;
			return;
		}
	} else if (_clue == 0x82) {
		if (_shot_color == 4 && _shot_direction == 1) {
			if (_crystal_state == 7) {
				picked = 0x89;
			} else {
				picked = _PickCrystalScene(0x86, 0x87, 0x88);
			}
		} else {
			_cur_scene = scene->next;
			return;
		}
	} else {
		_cur_scene = scene->next;
		return;
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_scene_nxtscn_setup_falina_target_practice(Scene *scene) {
	_random_count = 0;
	uint16 picked = _RandomNumberInRange(0x8A, 0x90);
	if (picked == 0x8D) {
		picked++;
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_scene_nxtscn_continue_falina_target_practice(Scene *scene) {
	uint16 picked = 0;
	_random_count++;
	if (_random_count < 6) {
		picked = _RandomNumberInRange(0x8A, 0x90);
		if (picked == 0x8D) {
			picked++;
		}
		_cur_scene = Common::String::format("scene%d", picked);
	} else {
		_scene_nxtscn_do_flying_skulls(scene);
		_world_done[3] = true;
	}
}

void GameSpacePirates::_scene_nxtscn_start_dune_popup(Scene *scene) {
	_max_random = 4;
	_random_count = 0;
	_random_scenes_values[0] = 0x93;
	_random_scenes_values[1] = 0x95;
	_random_scenes_values[2] = 0x97;
	_random_scenes_values[3] = 0x99;
	memset(&_random_scenes_used, 0, (9 * sizeof(uint8)));
	uint16 picked = _RandomUnusedScene(4);
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_scene_nxtscn_continue_dune_popup(Scene *scene) {
	_random_count++;
	if (((_difficulty * 4) + 4) > _random_count) {
		memset(&_random_scenes_used, 0, (9 * sizeof(uint8)));
		uint16 picked = _RandomUnusedScene(_max_random);
		_cur_scene = Common::String::format("scene%d", picked);
		return;
	}
	_cur_scene = scene->next;
}

void GameSpacePirates::_scene_nxtscn_pott_or_pan_shoots(Scene *scene) {
	uint8 random = _rnd->getRandomNumber(9);
	uint16 picked = (random > 5) ? 0xA2 : 0xA1;
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_scene_nxtscn_setup_pott_target_practice(Scene *scene) {
	uint16 picked = 0;
	_random_count = 0;
	uint8 random = _rnd->getRandomNumber(9);
	if (random < 5) {
		picked = 0xA7;
	} else {
		picked = 0xAA;
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_scene_nxtscn_continue_pott_target_practice(Scene *scene) {
	uint16 picked = 0;
	_random_count++;
	if (_random_count < 6) {
		uint8 random = _rnd->getRandomNumber(9);
		if (random <= 5) {
			picked = 0xA7;
		} else {
			picked = 0xAA;
		}
	} else {
		_scene_nxtscn_do_flying_skulls(scene);
		_world_done[0] = true;
		return;
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_scene_nxtscn_start_dragons_teeth_popup(Scene *scene) {
	_max_random = 9;
	_random_count = 0;
	_random_scenes_values[0] = 0xCF;
	_random_scenes_values[1] = 0xD2;
	_random_scenes_values[2] = 0xD4;
	_random_scenes_values[3] = 0xD6;
	_random_scenes_values[4] = 0xD8;
	_random_scenes_values[5] = 0xDA;
	_random_scenes_values[6] = 0xDC;
	_random_scenes_values[7] = 0xDE;
	_random_scenes_values[8] = 0xE0;
	memset(&_random_scenes_used, 0, (9 * sizeof(uint8)));
	uint16 picked = _RandomUnusedScene(9);
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_scene_nxtscn_continue_dragons_teeth_popup(Scene *scene) {
	if (_SceneToNumber(scene) != 0x14F) {
		_random_count++;
	}
	if (((_difficulty * 9) + 9) > _random_count) {
		memset(&_random_scenes_used, 0, (9 * sizeof(uint8)));
		uint16 picked = _RandomUnusedScene(_max_random);
		_cur_scene = Common::String::format("scene%d", picked);
		return;
	}
	_cur_scene = scene->next;
}

void GameSpacePirates::_scene_nxtscn_grin_reaper_clue(Scene *scene) {
	_shot_grin_reaper_count = 0;
	_clue = _RandomNumberInRange(0xE2, 0xE7);
	_shot_color = 0;
	_cur_scene = Common::String::format("scene%d", _clue);
}

void GameSpacePirates::_scene_nxtscn_start_grin_reaper(Scene *scene) {
	_max_random = 8;
	_random_count = 0;
	_random_scenes_values[0] = 0xEC;
	_random_scenes_values[1] = 0xED;
	_random_scenes_values[2] = 0xEE;
	_random_scenes_values[3] = 0xEF;
	_random_scenes_values[4] = 0xF0;
	_random_scenes_values[5] = 0xF1;
	_random_scenes_values[6] = 0xF2;
	_random_scenes_values[7] = 0xF3;
	memset(&_random_scenes_used, 0, (9 * sizeof(uint8)));
	_random_scenes_used[0] = 1;
	_cur_scene = Common::String::format("scene%d", _random_scenes_values[0]);
}

void GameSpacePirates::_scene_nxtscn_continue_grin_reaper(Scene *scene) {
	_random_count++;
	uint16 picked = 0;
	if (_random_count >= 8 || (_clue - 223) <= _random_count) {
		_cur_scene = scene->next;
	} else {
		picked = _RandomUnusedScene(_max_random);
		_cur_scene = Common::String::format("scene%d", picked);
	}
}

void GameSpacePirates::_scene_nxtscn_grin_target_practice(Scene *scene) {
	uint16 picked = 0;
	_random_count = 0;
	picked = _RandomNumberInRange(0xFB, 0x0101);
	if (picked == 0xFE) {
		picked++;
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_scene_nxtscn_continue_grin_target_practice(Scene *scene) {
	uint16 picked = 0;
	_random_count++;
	if (_random_count < 5) {
		picked = _RandomNumberInRange(0xFB, 0x0101);
		if (picked == 0xFE) {
			picked++;
		}
	} else {
		_scene_nxtscn_do_flying_skulls(scene);
		_world_done[2] = true;
		return;
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_scene_nxtscn_start_junk_world(Scene *scene) {
	_max_random = 6;
	_random_count = 0;
	_random_scenes_values[0] = 0x0105;
	_random_scenes_values[1] = 0x0107;
	_random_scenes_values[2] = 0x0109;
	_random_scenes_values[3] = 0x010B;
	_random_scenes_values[4] = 0x010D;
	_random_scenes_values[5] = 0x010F;
	memset(&_random_scenes_used, 0, (9 * sizeof(uint8)));
	uint16 picked = _RandomUnusedScene(6);
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_scene_nxtscn_continue_junk_world(Scene *scene) {
	uint16 picked = 0;
	_random_count++;
	if (((_difficulty * 6) + 6) > _random_count) {
		memset(&_random_scenes_used, 0, (9 * sizeof(uint8)));
		picked = _RandomUnusedScene(_max_random);
	} else {
		_cur_scene = scene->next;
		return;
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_scene_nxtscn_start_junk_rings(Scene *scene) {
	_random_count = 0;
	uint16 picked = _RandomNumberInRange(0x011E, 0x0122);
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_scene_nxtscn_show_junk_world_crystal(Scene *scene) {
	uint16 picked = _PickCrystalScene(0x11D, 0x11C, 0x11B);
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_scene_nxtscn_start_junk_world_target_practice(Scene *scene) {
	_max_random = 6;
	_random_count = 0;
	_random_scenes_values[0] = 0x125;
	_random_scenes_values[1] = 0x127;
	_random_scenes_values[2] = 0x129;
	_random_scenes_values[3] = 0x12C;
	_random_scenes_values[4] = 0x12E;
	_random_scenes_values[5] = 0x130;
	memset(&_random_scenes_used, 0, (9 * sizeof(uint8)));
	uint16 picked = _RandomUnusedScene(6);
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_scene_nxtscn_continue_junk_world_target_practice(Scene *scene) {
	_random_count++;
	if (_random_count == 1) {
		_target_practice_reset = false;
	}
	if (_random_count >= 0xC) {
		_scene_nxtscn_do_flying_skulls(scene);
		_world_done[1] = true;
		return;
	}
	if (_random_count >= 6 && !_target_practice_reset) {
		memset(&_random_scenes_used, 0, (9 * sizeof(uint8)));
		_target_practice_reset = true;
	}
	uint16 picked = _RandomUnusedScene(_max_random);
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_scene_nxtscn_are_all_worlds_done(Scene *scene) {
	if (_world_done[0] && _world_done[1] && _world_done[2] && _world_done[3]) {
		_cur_scene = "scene192";
	} else {
		_cur_scene = scene->next;
	}
}

void GameSpacePirates::_scene_nxtscn_start_practice_pirate_ship(Scene *scene) {
	_random_count = 0;
	uint16 picked = _RandomNumberInRange(0xBE, 0xBF);
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::_scene_nxtscn_more_practice_pirate_ship(Scene *scene) {
	_random_count++;
	if (_random_count < 5) {
		uint16 picked = _RandomNumberInRange(0xBE, 0xBF);
		_cur_scene = Common::String::format("scene%d", picked);
	} else {
		_cur_scene = scene->next;
	}
}

void GameSpacePirates::_scene_nxtscn_player_won(Scene *scene) {
	_gameInProgress = false;
	_cur_scene = scene->next;
}

// Script functions: MissedRect
void GameSpacePirates::_scene_missedrects_default(Scene *scene) {
	// do nothing
}

void GameSpacePirates::_scene_missedrects_missed_pirate_ship(Scene *scene) {
	_DisplayMultipleShotLines();
}

// Script functions: WepDwn
void GameSpacePirates::_scene_default_wepdwn(Scene *scene) {
	_shots = 10;
}

// Debug methods
void GameSpacePirates::debug_warpTo(int val) {
	// TODO implement
}

// Debugger methods
DebuggerSpacePirates::DebuggerSpacePirates(GameSpacePirates *game) : GUI::Debugger() {
	_game = game;
	registerVar("drawRects", &game->_debug_drawRects);
	registerVar("godMode", &game->_debug_godMode);
	registerVar("unlimitedAmmo", &game->_debug_unlimitedAmmo);
	registerCmd("warpTo", WRAP_METHOD(DebuggerSpacePirates, cmdWarpTo));
	registerCmd("dumpLib", WRAP_METHOD(DebuggerSpacePirates, cmdDumpLib));
}

bool DebuggerSpacePirates::cmdWarpTo(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: warp <int>");
		return true;
	} else {
		int val = atoi(argv[1]);
		_game->debug_warpTo(val);
		return false;
	}
}

bool DebuggerSpacePirates::cmdDumpLib(int argc, const char **argv) {
	return _game->debug_dumpLibFile();
}

} // End of namespace Alg
