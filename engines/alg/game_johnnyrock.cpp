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

#include "alg/game_johnnyrock.h"
#include "alg/graphics.h"
#include "alg/scene.h"

namespace Alg {

GameJohnnyRock::GameJohnnyRock(AlgEngine *vm, const ADGameDescription *desc) : Game(vm) {
	if (scumm_stricmp(desc->gameId, "johnrocs") == 0) {
		_libFileName = "johnroc.lib";
	} else if (scumm_stricmp(desc->gameId, "johnrocd") == 0) {
		_libFileName = "johnrocd.lib";
	}
}

GameJohnnyRock::~GameJohnnyRock() {
}

void GameJohnnyRock::init() {
	_videoPosX = 11;
	_videoPosY = 2;

	_SetupCursorTimer();

	loadLibArchive(_libFileName);
	_sceneInfo->loadScnFile("johnroc.scn");
	_startscene = _sceneInfo->getStartScene();

	registerScriptFunctions();
	verifyScriptFunctions();

	_menuzone = new Zone();
	_menuzone->name = "MainMenu";
	_menuzone->ptrfb = "GLOBALHIT";

	_menuzone->addRect(0x0C, 0xBB, 0x3C, 0xC7, nullptr, 0, "SHOTMENU", "0");

	_submenzone = new Zone();
	_submenzone->name = "SubMenu";
	_submenzone->ptrfb = "GLOBALHIT";

	_submenzone->addRect(0x10, 0x0F, 0x78, 0x34, nullptr, 0, "STARTMENU", "0");
	_submenzone->addRect(0x10, 0x8E, 0x8A, 0xAF, nullptr, 0, "CONTMENU", "0");
	_submenzone->addRect(0x10, 0x3A, 0x6A, 0x5C, nullptr, 0, "RECTSAVE", "0");
	_submenzone->addRect(0x10, 0x64, 0x84, 0x99, nullptr, 0, "RECTLOAD", "0");
	_submenzone->addRect(0xD2, 0x8D, 0x12F, 0xB0, nullptr, 0, "EXITMENU", "0");
	_submenzone->addRect(0xD0, 0x35, 0x123, 0x51, nullptr, 0, "RECTEASY", "0");
	_submenzone->addRect(0xD2, 0x50, 0x125, 0x6B, nullptr, 0, "RECTAVG", "0");
	_submenzone->addRect(0xD2, 0x6D, 0x122, 0x86, nullptr, 0, "RECTHARD", "0");

	_shotSound = _LoadSoundFile("blow.8b");
	_emptySound = _LoadSoundFile("empty.8b");
	_saveSound = _LoadSoundFile("saved.8b");
	_loadSound = _LoadSoundFile("loaded.8b");
	_skullSound = _LoadSoundFile("money.8b");

	_gun = AlgGraphics::loadScreenCoordAniImage("gun.ani", _palette);
	_numbers = AlgGraphics::loadAniImage("numbers.ani", _palette);
	_difficultyIcon = AlgGraphics::loadAniImage("diff.ani", _palette);
	Common::Array<Graphics::Surface> *level = AlgGraphics::loadScreenCoordAniImage("level.ani", _palette);
	_levelIcon = (*level)[0];
	Common::Array<Graphics::Surface> *hole = AlgGraphics::loadScreenCoordAniImage("hole.ani", _palette);
	_bulletholeIcon = (*hole)[0];

	_background = AlgGraphics::loadVgaBackground("backgrnd.vga", _palette);
	_screen->copyRectToSurface(_background->getPixels(), _background->pitch, 0, 0, _background->w, _background->h);

	_MoveMouse();
}

void GameJohnnyRock::registerScriptFunctions() {
#define RECT_HIT_FUNCTION(name, func) _rectHitFuncs[name] = new JRScriptFunctionRect(this, &GameJohnnyRock::func);
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
	RECT_HIT_FUNCTION("KILLINNOCENT", _rect_killinnocent);
	RECT_HIT_FUNCTION("SELCASINO", _rect_selectcasino);
	RECT_HIT_FUNCTION("SELPOOLH", _rect_selectpoolhall);
	RECT_HIT_FUNCTION("SELWAREHSE", _rect_selectwarehouse);
	RECT_HIT_FUNCTION("SELGARAGE", _rect_selectgarage);
	RECT_HIT_FUNCTION("SELMANSION", _rect_selectmansion);
	RECT_HIT_FUNCTION("SELAMMO", _rect_selectammo);
	RECT_HIT_FUNCTION("SELOFFICE", _rect_selectoffice);
	RECT_HIT_FUNCTION("MANBUST", _rect_shotmanbust);
	RECT_HIT_FUNCTION("WOMANBUST", _rect_shotwomanbust);
	RECT_HIT_FUNCTION("BLUEVASE", _rect_shotbluevase);
	RECT_HIT_FUNCTION("CAT", _rect_shotcat);
	RECT_HIT_FUNCTION("INDIAN", _rect_shotindian);
	RECT_HIT_FUNCTION("PLATE", _rect_shotplate);
	RECT_HIT_FUNCTION("BLUEDRESSPIC", _rect_shotbluedresspic);
	RECT_HIT_FUNCTION("MODERNPIC", _rect_shotmodernpic);
	RECT_HIT_FUNCTION("MONALISA", _rect_shotmonalisa);
	RECT_HIT_FUNCTION("GWASHINGTON", _rect_shotgwashington);
	RECT_HIT_FUNCTION("BOYINREDPIC", _rect_shotboyinredpic);
	RECT_HIT_FUNCTION("COATOFARMS", _rect_shotcoatofarms);
	RECT_HIT_FUNCTION("COMBNOA0", _rect_shotcombinationA0);
	RECT_HIT_FUNCTION("COMBNOA1", _rect_shotcombinationA1);
	RECT_HIT_FUNCTION("COMBNOA2", _rect_shotcombinationA2);
	RECT_HIT_FUNCTION("COMBNOA3", _rect_shotcombinationA3);
	RECT_HIT_FUNCTION("COMBNOA4", _rect_shotcombinationA4);
	RECT_HIT_FUNCTION("COMBNOA5", _rect_shotcombinationA5);
	RECT_HIT_FUNCTION("COMBNOB0", _rect_shotcombinationB0);
	RECT_HIT_FUNCTION("COMBNOB1", _rect_shotcombinationB1);
	RECT_HIT_FUNCTION("COMBNOB2", _rect_shotcombinationB2);
	RECT_HIT_FUNCTION("COMBNOB3", _rect_shotcombinationB3);
	RECT_HIT_FUNCTION("COMBNOB4", _rect_shotcombinationB4);
	RECT_HIT_FUNCTION("COMBNOB5", _rect_shotcombinationB5);
	RECT_HIT_FUNCTION("LUCKNO0", _rect_shotluckynum0);
	RECT_HIT_FUNCTION("LUCKNO1", _rect_shotluckynum1);
	RECT_HIT_FUNCTION("LUCKNO2", _rect_shotluckynum2);
	RECT_HIT_FUNCTION("LUCKNO3", _rect_shotluckynum3);
	RECT_HIT_FUNCTION("LUCKNO4", _rect_shotluckynum4);
	RECT_HIT_FUNCTION("LUCKNO5", _rect_shotluckynum5);
#undef RECT_HIT_FUNCTION

#define PRE_OPS_FUNCTION(name, func) _scenePreOps[name] = new JRScriptFunctionScene(this, &GameJohnnyRock::func);
	PRE_OPS_FUNCTION("DRAWRCT", _scene_pso_drawrct);
	PRE_OPS_FUNCTION("PAUSE", _scene_pso_pause);
	PRE_OPS_FUNCTION("FADEIN", _scene_pso_fadein);
	PRE_OPS_FUNCTION("PAUSFI", _scene_pso_pause_fadein);
	PRE_OPS_FUNCTION("PREREAD", _scene_pso_preread);
	PRE_OPS_FUNCTION("PAUSPR", _scene_pso_pause_preread);
	PRE_OPS_FUNCTION("DEFAULT", _scene_pso_drawrct);
	PRE_OPS_FUNCTION("DRAWRCTFDI", _scene_pso_drawrct_fadein);
#undef PRE_OPS_FUNCTION

#define INS_OPS_FUNCTION(name, func) _sceneInsOps[name] = new JRScriptFunctionScene(this, &GameJohnnyRock::func);
	INS_OPS_FUNCTION("DEFAULT", _scene_iso_donothing);
	INS_OPS_FUNCTION("PAUSE", _scene_iso_pause);
	INS_OPS_FUNCTION("SPAUSE", _scene_iso_spause);
	INS_OPS_FUNCTION("STARTGAME", _scene_iso_startgame);
	INS_OPS_FUNCTION("SHOOTPAST", _scene_iso_shootpast);
	INS_OPS_FUNCTION("GOTTOCASINO", _scene_iso_gotocasino);
	INS_OPS_FUNCTION("GOTTOPOOLH", _scene_iso_gotopoolh);
	INS_OPS_FUNCTION("GOTTOWAREHSE", _scene_iso_gotowarehse);
	INS_OPS_FUNCTION("INWAREHSE2", _scene_iso_inwarehse2);
	INS_OPS_FUNCTION("INWAREHSE3", _scene_iso_inwarehse3);
	INS_OPS_FUNCTION("GOTOGARAGE", _scene_iso_gotogarage);
	INS_OPS_FUNCTION("GOTOMANSION", _scene_iso_gotomansion);
	INS_OPS_FUNCTION("INMANSION1", _scene_iso_inmansion1);
#undef INS_OPS_FUNCTION

#define NXT_SCN_FUNCTION(name, func) _sceneNxtScn[name] = new JRScriptFunctionScene(this, &GameJohnnyRock::func);
	NXT_SCN_FUNCTION("DEFAULT", _scene_default_nxtscn);
	NXT_SCN_FUNCTION("DRAWGUN", _scene_default_nxtscn);
	NXT_SCN_FUNCTION("DIED", _scene_nxtscn_died);
	NXT_SCN_FUNCTION("BOMBDEAD", _scene_nxtscn_bombdead);
	NXT_SCN_FUNCTION("PIKUNDRTAKR", _scene_nxtscn_pikundrtakr);
	NXT_SCN_FUNCTION("CALLATTRACT", _scene_nxtscn_callattract);
	NXT_SCN_FUNCTION("PICKLUCKNO", _scene_nxtscn_pikluckno);
	NXT_SCN_FUNCTION("PICKMAP", _scene_nxtscn_pickmap);
	NXT_SCN_FUNCTION("PICKCLUE", _scene_nxtscn_pickclue);
	NXT_SCN_FUNCTION("MAPTIMEOUT", _scene_nxtscn_maptimeout);
	NXT_SCN_FUNCTION("ENTCASINO", _scene_nxtscn_entcasino);
	NXT_SCN_FUNCTION("CASINOWHAT?", _scene_nxtscn_casinowhat);
	NXT_SCN_FUNCTION("ENTPOOLH", _scene_nxtscn_entpoolhall);
	NXT_SCN_FUNCTION("POOLHCLUE", _scene_nxtscn_poolhclue);
	NXT_SCN_FUNCTION("ENTWAREHSE", _scene_nxtscn_entwarehse);
	NXT_SCN_FUNCTION("WAREHSECLUE", _scene_nxtscn_warehseclue);
	NXT_SCN_FUNCTION("ENTGARAGE", _scene_nxtscn_entgarage);
	NXT_SCN_FUNCTION("GARAGECLUE", _scene_nxtscn_garageclue);
	NXT_SCN_FUNCTION("ENTMANSION", _scene_nxtscn_entmansion);
	NXT_SCN_FUNCTION("GIVECLUE", _scene_nxtscn_giveclue);
	NXT_SCN_FUNCTION("PICKFLOWERMAN", _scene_nxtscn_pikflwrman);
	NXT_SCN_FUNCTION("RANDOMSCENE", _scene_nxtscn_randomscene);
	NXT_SCN_FUNCTION("ENDRANDSCENE", _scene_nxtscn_endrandscene);
	NXT_SCN_FUNCTION("SCN_KILLINNOCENT", _scene_nxtscn_killinnocent);
#undef NXT_SCN_FUNCTION

	_zonePtrFb["DEFAULT"] = new JRScriptFunctionPoint(this, &GameJohnnyRock::_zone_bullethole);
	_sceneShowMsg["DEFAULT"] = new JRScriptFunctionScene(this, &GameJohnnyRock::_scene_sm_donothing);
	_sceneWepDwn["DEFAULT"] = new JRScriptFunctionScene(this, &GameJohnnyRock::_scene_default_wepdwn);
	_sceneScnScr["DEFAULT"] = new JRScriptFunctionScene(this, &GameJohnnyRock::_scene_default_score);
	_sceneNxtFrm["DEFAULT"] = new JRScriptFunctionScene(this, &GameJohnnyRock::_scene_nxtfrm);
}

void GameJohnnyRock::verifyScriptFunctions() {
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

JRScriptFunctionPoint GameJohnnyRock::getScriptFunctionZonePtrFb(Common::String name) {
	JRScriptFunctionPointMap::iterator it = _zonePtrFb.find(name);
	if (it != _zonePtrFb.end()) {
		return (*(*it)._value);
	} else {
		error("Could not find zonePtrFb function: %s", name.c_str());
	}
}

JRScriptFunctionRect GameJohnnyRock::getScriptFunctionRectHit(Common::String name) {
	JRScriptFunctionRectMap::iterator it = _rectHitFuncs.find(name);
	if (it != _rectHitFuncs.end()) {
		return (*(*it)._value);
	} else {
		error("Could not find rectHit function: %s", name.c_str());
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
		error("Unkown scene script type: %u", type);
		break;
	}
	JRScriptFunctionSceneMap::iterator it;
	it = functionMap->find(name);
	if (it != functionMap->end()) {
		return (*(*it)._value);
	} else {
		error("Could not find scene type %u function: %s", type, name.c_str());
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
	_NewGame();
	_cur_scene = _startscene;
	Common::String oldscene;
	while (!_vm->shouldQuit()) {
		_leftDown = false;
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

bool GameJohnnyRock::__Fired(Common::Point *point) {
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
		if (_thisGameTimer - _mach_gun_timer > 3) {
			_buttonDown = false;
			_mgun_cnt++;
			if (_mgun_cnt > 5) {
				_mgun_cnt = 0;
				_leftDown = false;
			}
		}
		return false;
	}
	_fired = true;
	point->x = _mousePos.x;
	point->y = _mousePos.y;
	_mach_gun_timer = _thisGameTimer;
	_buttonDown = true;
	return true;
}

void GameJohnnyRock::_NewGame() {
	_game_money = 2000;
	_shots = 400;
	_score = 0;
	_holster = false;
	_UpdateStat();
	_ret_scene = "";
	_sub_scene = "";
}

void GameJohnnyRock::_ResetParams() {
	_sub_scene = "";
	_money_scene = "";
	_this_map = _rnd->getRandomNumber(2);
	_clues = 0;
	_got_this_number = 0;
	_got_this_clue = 0;
	_this_clue = 0;
	_office = 0;
	_casino = 0;
	_pool_hall = 0;
	_warehouse = 0;
	_garage = 0;
	_mansion = 0;
	_did_continue = 0;
	_this_difficulty = _difficulty - 1;
	_casino_type = _rnd->getRandomNumber(1);
	_pool_hall_type = _rnd->getRandomNumber(2);
	_warehouse_type = _rnd->getRandomNumber(2);
	_garage_type = _rnd->getRandomNumber(1);
	_map_timeout = 0;
	_ammo_again = 0;
	_combinations[0] = _rnd->getRandomNumber(5);
	_combinations[1] = _rnd->getRandomNumber(5);
	_combinations[2] = _rnd->getRandomNumber(5);
	_combinations[3] = _rnd->getRandomNumber(5);
	_who_did_it = _rnd->getRandomNumber(3);
	_in_warehouse = 0;
	_office_count = 0;
	_had_go_to_mansion = 0;
	_random_place_bits = 0;
	for (uint8 i = 0; i < 5; i++) {
		// this assigns places from _random_places
		uint16 picked = _pick_bits(&_random_place_bits, 6);
		_entrance_index[i] = picked;
	}
	_random_place_bits = 0;
	for (uint8 i = 5; i < 19; i++) {
		// this assigns places from _random_places_mr
		uint16 picked = _pick_bits(&_random_place_bits, 8);
		_entrance_index[i] = picked;
	}
	_max_repeat = _this_difficulty + 4;
	_repeat_random_place = 0;
	_got_to = 0;
	_UpdateStat();
}

void GameJohnnyRock::_OutShots() {
	_shots = 400;
	_score = 0;
	_holster = false;
	_UpdateStat();
}

void GameJohnnyRock::_DoMenu() {
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
		_leftDown = false;
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

void GameJohnnyRock::_UpdateStat() {
	if (_score != _oldScore) {
		_oldScore = _score;
		Common::String buffer = Common::String::format("%05d", _score);
		for (uint8 i = 0; i < 5; i++) {
			uint8 digit = buffer[i] - '0';
			AlgGraphics::drawImage(_screen, &(*_numbers)[digit], (i * 7) + 0x7D, 0xBE);
		}
	}
	if (_game_money != _oldgame_money) {
		_oldgame_money = _game_money;
		Common::String buffer = Common::String::format("%04d", _game_money < 0 ? 0 : _game_money);
		for (uint8 i = 0; i < 4; i++) {
			uint8 digit = buffer[i] - '0';
			AlgGraphics::drawImage(_screen, &(*_numbers)[digit], (i * 7) + 0x43, 0xBE);
		}
	}
	if (_shots != _oldShots) {
		_oldShots = _shots;
		Common::String buffer = Common::String::format("%04d", _shots);
		for (uint8 i = 0; i < 4; i++) {
			uint8 digit = buffer[i] - '0';
			AlgGraphics::drawImage(_screen, &(*_numbers)[digit], (i * 7) + 0x10A, 0xBE);
		}
	}
	AlgGraphics::drawImage(_screen, &(*_difficultyIcon)[_difficulty - 1], 0xBA, 0xBE);
}

void GameJohnnyRock::_DisplayScore() {
	_UpdateStat();
}

void GameJohnnyRock::_ShowDifficulty(uint8 newDifficulty, bool updateCursor) {
	// reset menu screen
	_screen->copyRectToSurface(_background->getBasePtr(_videoPosX, _videoPosY), _background->pitch, _videoPosX, _videoPosY, _videoDecoder->getWidth(), _videoDecoder->getHeight());
	AlgGraphics::drawImageCentered(_screen, &_levelIcon, _diffpos[newDifficulty][0], _diffpos[newDifficulty][1]);
	if (updateCursor) {
		_DoCursor();
	}
}

void GameJohnnyRock::_ChangeDifficulty(uint8 newDifficulty) {
	if (newDifficulty == _oldDifficulty) {
		return;
	}
	_ShowDifficulty(newDifficulty, true);
	Game::_AdjustDifficulty(newDifficulty, _oldDifficulty);
	_oldDifficulty = newDifficulty;
	_difficulty = newDifficulty;
}

void GameJohnnyRock::_DoCursor() {
	_oldWhichGun = _whichGun;
}

void GameJohnnyRock::_UpdateMouse() {
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

void GameJohnnyRock::_MoveMouse() {
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
	_UpdateMouse();
}

bool GameJohnnyRock::_WeaponDown() {
	if (_rightDown && _mousePos.y >= 0xBC && _mousePos.x >= 0x37) {
		return true;
	}
	return false;
}

bool GameJohnnyRock::_SaveState() {
	Common::OutSaveFile *outSaveFile;
	Common::String saveFileName = _vm->getSaveStateName(0);
	if (!(outSaveFile = g_system->getSavefileManager()->openForSaving(saveFileName))) {
		warning("Can't create file '%s', game not saved", saveFileName.c_str());
		return false;
	}
	outSaveFile->writeUint32BE(MKTAG('A', 'L', 'G', 'S')); // header
	outSaveFile->writeByte(0);                             // version, unused for now
	outSaveFile->writeUint16LE(_total_dies);
	outSaveFile->writeSint16LE(_game_money);
	outSaveFile->writeUint16LE(_ammo_again);
	outSaveFile->writeUint16LE(_map_timeout);
	outSaveFile->writeByte(_lucky_number);
	outSaveFile->writeByte(_this_map);
	outSaveFile->writeUint16LE(_clues);
	outSaveFile->writeUint16LE(_place_bits);
	outSaveFile->writeByte(_random_count);
	outSaveFile->writeUint16LE(_doctor_bits);
	outSaveFile->writeUint16LE(_undertaker_bits);
	for (uint8 i = 0; i < 4; i++) {
		outSaveFile->writeByte(_clue_table[i]);
	}
	outSaveFile->writeUint16LE(_this_clue);
	outSaveFile->writeByte(_got_this_number);
	outSaveFile->writeByte(_casino);
	outSaveFile->writeByte(_pool_hall);
	outSaveFile->writeByte(_warehouse);
	outSaveFile->writeByte(_garage);
	outSaveFile->writeByte(_office);
	outSaveFile->writeByte(_casino_type);
	outSaveFile->writeByte(_pool_hall_type);
	outSaveFile->writeByte(_warehouse_type);
	outSaveFile->writeByte(_garage_type);
	outSaveFile->writeByte(_mansion);
	outSaveFile->writeByte(_in_warehouse);
	outSaveFile->writeByte(_in_office);
	outSaveFile->writeUint16LE(_got_to);
	for (uint8 i = 0; i < 20; i++) {
		outSaveFile->writeUint16LE(_entrance_index[i]);
	}
	for (uint8 i = 0; i < 10; i++) {
		outSaveFile->writeUint16LE(_random_scenes_index[i]);
	}
	for (uint8 i = 0; i < 4; i++) {
		outSaveFile->writeByte(_combinations[i]);
	}
	outSaveFile->writeByte(_who_did_it);
	outSaveFile->writeByte(_had_go_to_mansion);
	outSaveFile->writeUint16LE(_office_count);
	outSaveFile->writeUint16LE(_random_place_bits);
	outSaveFile->writeByte(_max_random_count);
	outSaveFile->writeUint16LE(_goto_after_random);
	outSaveFile->writeUint16LE(_repeat_random_place);
	outSaveFile->writeUint16LE(_max_repeat);
	outSaveFile->writeUint16LE(_got_this_clue);
	outSaveFile->writeUint16LE(_did_continue);
	outSaveFile->writeUint16LE(_this_game_time);
	outSaveFile->writeUint16LE(_shots);
	outSaveFile->writeSint32LE(_score);
	outSaveFile->writeByte(_holster);
	outSaveFile->writeByte(_difficulty);
	outSaveFile->writeByte(_this_difficulty);
	outSaveFile->writeString(_money_scene);
	outSaveFile->writeByte(0);
	outSaveFile->writeString(_cur_scene);
	outSaveFile->writeByte(0);
	outSaveFile->writeString(_sub_scene);
	outSaveFile->writeByte(0);
	outSaveFile->writeString(_ret_scene);
	outSaveFile->writeByte(0);
	outSaveFile->writeByte(_random_scenes_savestate_index);
	outSaveFile->finalize();
	delete outSaveFile;
	return true;
}

bool GameJohnnyRock::_LoadState() {
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
	_game_money = inSaveFile->readSint16LE();
	_ammo_again = inSaveFile->readUint16LE();
	_map_timeout = inSaveFile->readUint16LE();
	_lucky_number = inSaveFile->readByte();
	_this_map = inSaveFile->readByte();
	_clues = inSaveFile->readUint16LE();
	_place_bits = inSaveFile->readUint16LE();
	_random_count = inSaveFile->readByte();
	_doctor_bits = inSaveFile->readUint16LE();
	_undertaker_bits = inSaveFile->readUint16LE();
	for (uint8 i = 0; i < 4; i++) {
		_clue_table[i] = inSaveFile->readByte();
	}
	_this_clue = inSaveFile->readUint16LE();
	_got_this_number = inSaveFile->readByte();
	_casino = inSaveFile->readByte();
	_pool_hall = inSaveFile->readByte();
	_warehouse = inSaveFile->readByte();
	_garage = inSaveFile->readByte();
	_office = inSaveFile->readByte();
	_casino_type = inSaveFile->readByte();
	_pool_hall_type = inSaveFile->readByte();
	_warehouse_type = inSaveFile->readByte();
	_garage_type = inSaveFile->readByte();
	_mansion = inSaveFile->readByte();
	_in_warehouse = inSaveFile->readByte();
	_in_office = inSaveFile->readByte();
	_got_to = inSaveFile->readUint16LE();
	for (uint8 i = 0; i < 20; i++) {
		_entrance_index[i] = inSaveFile->readUint16LE();
	}
	for (uint8 i = 0; i < 10; i++) {
		_random_scenes_index[i] = inSaveFile->readUint16LE();
	}
	for (uint8 i = 0; i < 4; i++) {
		_combinations[i] = inSaveFile->readByte();
	}
	_who_did_it = inSaveFile->readByte();
	_had_go_to_mansion = inSaveFile->readByte();
	_office_count = inSaveFile->readUint16LE();
	_random_place_bits = inSaveFile->readUint16LE();
	_max_random_count = inSaveFile->readByte();
	_goto_after_random = inSaveFile->readUint16LE();
	_repeat_random_place = inSaveFile->readUint16LE();
	_max_repeat = inSaveFile->readUint16LE();
	_got_this_clue = inSaveFile->readUint16LE();
	_did_continue = inSaveFile->readUint16LE();
	_this_game_time = inSaveFile->readUint16LE();
	_shots = inSaveFile->readUint16LE();
	_score = inSaveFile->readSint32LE();
	_holster = inSaveFile->readByte();
	_difficulty = inSaveFile->readByte();
	_this_difficulty = inSaveFile->readByte();
	_money_scene = inSaveFile->readString();
	_cur_scene = inSaveFile->readString();
	_sub_scene = inSaveFile->readString();
	_ret_scene = inSaveFile->readString();
	_random_scenes_savestate_index = inSaveFile->readByte();
	delete inSaveFile;
	// find out where _random_scenes should point
	uint16 placeIndex = _entrance_index[_random_scenes_savestate_index];
	if (_random_scenes_savestate_index < 5) {
		_random_scenes = _random_places[placeIndex];
	} else {
		_random_scenes = _random_places_mr[placeIndex];
	}
	_ChangeDifficulty(_difficulty);
	debug("lucky number: %d", (_lucky_number + 1));
	return true;
}

void GameJohnnyRock::_DoMoneySound() {
	_PlaySound(_skullSound);
}

// Misc game functions
Common::String GameJohnnyRock::_NumtoScene(int n) {
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

uint16 GameJohnnyRock::_ScenetoNum(Common::String sceneName) {
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

void GameJohnnyRock::_default_bullethole(Common::Point *point) {
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

uint16 GameJohnnyRock::_pick_bits(uint16 *bits, uint8 max) {
	// reset bits if full
	if (*bits == (0xFFFF >> (16 - max))) {
		*bits = 0;
	}
	uint8 random = _rnd->getRandomNumber(max - 1);
	// find first unused bit position
	while (*bits & (1 << random)) {
		random++;
		if (random >= max) {
			random = 0;
		}
	}
	*bits |= (1 << random);
	return random;
}

uint16 GameJohnnyRock::_pick_random_place(uint8 place) {
	_random_scenes_savestate_index = place;
	uint16 placeIndex = _entrance_index[place];
	if (place < 5) {
		_random_scenes = _random_places[placeIndex];
	} else {
		_random_scenes = _random_places_mr[placeIndex];
	}
	_place_bits = 0;
	_random_count = 0;
	memset(&_random_scenes_index, 0, (10 * sizeof(uint16)));
	for (uint8 i = 0; i < (_random_scenes[0] + 4); i++) {
		_random_scenes_index[i] = _random_scenes[i];
	}
	if (_random_scenes[1] < 0) {
		_max_random_count = (_this_difficulty * 2) - _random_scenes[1];
	} else {
		_max_random_count = _random_scenes[1];
	}
	uint16 index = _pick_bits(&_place_bits, _random_scenes[0]);
	return _random_scenes[index + 4];
}

void GameJohnnyRock::_show_combination() {
	uint16 offset = (_got_this_clue * 6) + _combinations[_got_this_clue];
	_got_this_clue++;
	if (_got_this_clue == 4) {
		_mansion = 3;
	}
	_cur_scene = _NumtoScene(offset + 0xDB);
}

// Script functions: Zone
void GameJohnnyRock::_zone_bullethole(Common::Point *point) {
	_default_bullethole(point);
}

void GameJohnnyRock::_rect_shotmenu(Rect *rect) {
	_DoMenu();
}

void GameJohnnyRock::_rect_save(Rect *rect) {
	if(_SaveState()) {
		_DoSaveSound();
	}
}

void GameJohnnyRock::_rect_load(Rect *rect) {
	if(_LoadState()) {
		_DoLoadSound();
	}
}

void GameJohnnyRock::_rect_continue(Rect *rect) {
	_inMenu = 0;
	_fired = 0;
	if (_game_money < 0) {
		_NewGame();
		_ret_scene = "";
		_sub_scene = "";
		if (_in_office) {
			_cur_scene = _NumtoScene(_in_office);
		} else {
			_cur_scene = _NumtoScene(_this_map + 174);
		}
		_did_continue++;
	}
	if (_shots <= 0) {
		_OutShots();
		_did_continue++;
	}
}

void GameJohnnyRock::_rect_start(Rect *rect) {
	_inMenu = 0;
	_fired = 0;
	_this_difficulty = 0;
	Scene *scene = _sceneInfo->findScene(_startscene);
	if (scene->nxtscn == "DRAWGUN") {
		callScriptFunctionScene(NXTSCN, "DRAWGUN", scene);
	}
	_cur_scene = _startscene;
	_ResetParams();
	_NewGame();
	_UpdateStat();
}

void GameJohnnyRock::_rect_killinnocent(Rect *rect) {
	_in_office = _ScenetoNum(_cur_scene);
	if (_in_office >= 0x13) {
		_in_office = 0;
	}
	if (!_debug_godMode) {
		_game_money -= 400;
	}
	if (_game_money < 0) {
		_sub_scene = "scene358";
		_ret_scene = "";
		_cur_scene = "scene151";
	} else {
		switch (_rnd->getRandomNumber(2)) {
		case 0:
			_cur_scene = "scene151";
			break;
		case 1:
			_cur_scene = "scene152";
			break;
		case 2:
			_cur_scene = "scene153";
			break;
		}
	}
}

void GameJohnnyRock::_rect_selectcasino(Rect *rect) {
	_ret_scene = "";
	if (_mansion == 5 && _who_did_it == 1) {
		_repeat_random_place = _max_repeat;
		_goto_after_random = 0xF5;
		_cur_scene = _NumtoScene(_pick_random_place(_max_repeat + 5));
	} else if (_casino == 0) {
		if (_got_to & 1) {
			_cur_scene = "scene19";
		} else {
			_goto_after_random = 0x13;
			_cur_scene = _NumtoScene(_pick_random_place(0));
		}
	} else if (_casino == 1 || _casino == 3) {
		_cur_scene = "scene44";
	} else {
		_casino = 3;
		if (_rnd->getRandomBit()) { // original: (_this_game_time & 1) == 0
			_cur_scene = "scene64";
		} else {
			_cur_scene = "scene65";
		}
	}
}

void GameJohnnyRock::_rect_selectpoolhall(Rect *rect) {
	_ret_scene = "";
	if (_mansion == 5 && _who_did_it == 0) {
		_repeat_random_place = _max_repeat;
		_goto_after_random = 0xF7;
		_cur_scene = _NumtoScene(_pick_random_place(_max_repeat + 5));
	} else if (_pool_hall == 0) {
		if (_got_to & 2) {
			_cur_scene = "scene66";
		} else {
			_goto_after_random = 0x42;
			_cur_scene = _NumtoScene(_pick_random_place(1));
		}
	} else if (_pool_hall == 1 || _pool_hall == 3) {
		_cur_scene = "scene82";
	} else {
		_pool_hall = 3;
		_cur_scene = "scene89";
	}
}

void GameJohnnyRock::_rect_selectwarehouse(Rect *rect) {
	_ret_scene = "";
	if (_mansion == 5 && _who_did_it == 2) {
		_repeat_random_place = _max_repeat;
		_goto_after_random = 0xF9;
		_cur_scene = _NumtoScene(_pick_random_place(_max_repeat + 5));
	} else if (_warehouse == 0) {
		if (_in_warehouse < 2) {
			if (_got_to & 4) {
				_cur_scene = "scene90";
			} else {
				_in_warehouse = 1;
				_goto_after_random = 0x5A;
				_cur_scene = _NumtoScene(_pick_random_place(2));
			}
		} else if (_in_warehouse == 2) {
			_cur_scene = "scene93";
		} else if (_in_warehouse == 3) {
			_cur_scene = "scene119";
		}
	} else if (_warehouse == 1 || _warehouse == 3) {
		_cur_scene = "scene122";
	} else {
		_warehouse = 3;
		_cur_scene = "scene121";
	}
}

void GameJohnnyRock::_rect_selectgarage(Rect *rect) {
	_ret_scene = "";
	if (_mansion == 5 && _who_did_it == 3) {
		_repeat_random_place = _max_repeat;
		_goto_after_random = 0xFB;
		_cur_scene = _NumtoScene(_pick_random_place(_max_repeat + 5));
	} else if (_garage == 0) {
		if (_got_to & 8) {
			_cur_scene = "scene123";
		} else {
			_goto_after_random = 0x7B;
			_cur_scene = _NumtoScene(_pick_random_place(3));
		}
	} else if (_garage == 1 || _garage == 3) {
		_cur_scene = "scene138";
	} else {
		_garage = 3;
		_cur_scene = "scene139";
	}
}

void GameJohnnyRock::_rect_selectmansion(Rect *rect) {
	_place_bits = 0;
	_random_count = 1;
	_ret_scene = "";
	if (_mansion == 1) {
		uint16 picked = _pick_bits(&_place_bits, 5);
		_cur_scene = _NumtoScene(0xB8 + (picked * 2));
	} else if (_mansion == 2) {
		_cur_scene = "scene194";
	} else if (_mansion == 3) {
		_cur_scene = "scene207";
	} else if (_mansion == 4) {
		_got_this_number = 0;
		_cur_scene = "scene212";
	} else if (_mansion == 5) {
		_cur_scene = "scene243";
	} else {
		if (_garage == 0 || _casino == 0 || _pool_hall == 0 || _warehouse == 0) {
			_cur_scene = "scene243";
		} else if (_got_to & 0x10) {
			_cur_scene = "scene180";
		} else {
			_goto_after_random = 0xB4;
			_cur_scene = _NumtoScene(_pick_random_place(4));
		}
	}
}

void GameJohnnyRock::_rect_selectammo(Rect *rect) {
	_ret_scene = "";
	if (_game_money >= 100) {
		if (!_debug_godMode) {
			_game_money -= 100;
		}
		_shots += 200;
		_ammo_again = 0;
		_DoMoneySound();
		_cur_scene = "scene178";
	} else {
		_ammo_again++;
		if (_ammo_again >= 2) {
			_cur_scene = "scene243";
		} else {
			_cur_scene = "scene179";
		}
	}
}

void GameJohnnyRock::_rect_selectoffice(Rect *rect) {
	_ret_scene = "";
	if (!_office) {
		_office = 1;
		_cur_scene = "scene168";
	} else {
		if (_rnd->getRandomBit()) { // original: _this_game_time & 1
			_cur_scene = "scene243";
		} else {
			_cur_scene = "scene262";
		}
	}
}

void GameJohnnyRock::_shotclue(uint8 clue) {
	if (_clue_table[_got_this_clue] == clue) {
		_show_combination();
	} else {
		_got_this_clue = 0;
		_cur_scene = "scene374";
	}
}

void GameJohnnyRock::_rect_shotmanbust(Rect *rect) {
	_shotclue(0);
}

void GameJohnnyRock::_rect_shotwomanbust(Rect *rect) {
	_shotclue(1);
}

void GameJohnnyRock::_rect_shotbluevase(Rect *rect) {
	_shotclue(2);
}

void GameJohnnyRock::_rect_shotcat(Rect *rect) {
	_shotclue(3);
}

void GameJohnnyRock::_rect_shotindian(Rect *rect) {
	_shotclue(4);
}

void GameJohnnyRock::_rect_shotplate(Rect *rect) {
	_shotclue(5);
}

void GameJohnnyRock::_rect_shotbluedresspic(Rect *rect) {
	_shotclue(6);
}

void GameJohnnyRock::_rect_shotmodernpic(Rect *rect) {
	_shotclue(7);
}

void GameJohnnyRock::_rect_shotmonalisa(Rect *rect) {
	_shotclue(8);
}

void GameJohnnyRock::_rect_shotgwashington(Rect *rect) {
	_shotclue(9);
}

void GameJohnnyRock::_rect_shotboyinredpic(Rect *rect) {
	_shotclue(10);
}

void GameJohnnyRock::_rect_shotcoatofarms(Rect *rect) {
	_shotclue(11);
}

void GameJohnnyRock::_shotcombination(uint8 combination, bool combinationB) {
	if (_combinations[_got_this_number] == combination) {
		_got_this_number++;
		if (_got_this_number >= 4) {
			_mansion = 5;
			_cur_scene = _NumtoScene(_who_did_it + 0xD7);
		} else {
			if (combinationB) {
				_cur_scene = "scene213";
			} else {
				_cur_scene = "scene214";
			}
		}
	} else {
		_got_this_number = 0;
		_cur_scene = "scene376";
	}
}

void GameJohnnyRock::_rect_shotcombinationA0(Rect *rect) {
	_shotcombination(0, false);
}

void GameJohnnyRock::_rect_shotcombinationA1(Rect *rect) {
	_shotcombination(1, false);
}

void GameJohnnyRock::_rect_shotcombinationA2(Rect *rect) {
	_shotcombination(2, false);
}

void GameJohnnyRock::_rect_shotcombinationA3(Rect *rect) {
	_shotcombination(3, false);
}

void GameJohnnyRock::_rect_shotcombinationA4(Rect *rect) {
	_shotcombination(4, false);
}

void GameJohnnyRock::_rect_shotcombinationA5(Rect *rect) {
	_shotcombination(5, false);
}

void GameJohnnyRock::_rect_shotcombinationB0(Rect *rect) {
	_shotcombination(0, true);
}

void GameJohnnyRock::_rect_shotcombinationB1(Rect *rect) {
	_shotcombination(1, true);
}

void GameJohnnyRock::_rect_shotcombinationB2(Rect *rect) {
	_shotcombination(2, true);
}

void GameJohnnyRock::_rect_shotcombinationB3(Rect *rect) {
	_shotcombination(3, true);
}

void GameJohnnyRock::_rect_shotcombinationB4(Rect *rect) {
	_shotcombination(4, true);
}

void GameJohnnyRock::_rect_shotcombinationB5(Rect *rect) {
	_shotcombination(5, true);
}

void GameJohnnyRock::_shotluckynumber(uint8 number) {
	if (_lucky_number != number || _cur_scene == _money_scene) {
		return;
	}
	_DoMoneySound();
	_game_money += 100;
	_score += 500;
	_money_scene = _cur_scene;
}

void GameJohnnyRock::_rect_shotluckynum0(Rect *rect) {
	_shotluckynumber(0);
}

void GameJohnnyRock::_rect_shotluckynum1(Rect *rect) {
	_shotluckynumber(1);
}

void GameJohnnyRock::_rect_shotluckynum2(Rect *rect) {
	_shotluckynumber(2);
}

void GameJohnnyRock::_rect_shotluckynum3(Rect *rect) {
	_shotluckynumber(3);
}

void GameJohnnyRock::_rect_shotluckynum4(Rect *rect) {
	_shotluckynumber(4);
}

void GameJohnnyRock::_rect_shotluckynum5(Rect *rect) {
	_shotluckynumber(5);
}

// Script functions: Scene PreOps

// Script functions: Scene Scene InsOps
void GameJohnnyRock::_scene_iso_shootpast(Scene *scene) {
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

void GameJohnnyRock::_scene_iso_spause(Scene *scene) {
	_scene_iso_shootpast(scene);
	_scene_iso_pause(scene);
}

void GameJohnnyRock::_scene_iso_gotocasino(Scene *scene) {
	_got_to |= 1;
	_scene_iso_shootpast(scene);
}

void GameJohnnyRock::_scene_iso_gotopoolh(Scene *scene) {
	_got_to |= 2;
	_scene_iso_shootpast(scene);
}

void GameJohnnyRock::_scene_iso_gotowarehse(Scene *scene) {
	_got_to |= 4;
}

void GameJohnnyRock::_scene_iso_inwarehse2(Scene *scene) {
	_in_warehouse = 2;
	_scene_iso_shootpast(scene);
}

void GameJohnnyRock::_scene_iso_inwarehse3(Scene *scene) {
	_in_warehouse = 3;
	_scene_iso_shootpast(scene);
}

void GameJohnnyRock::_scene_iso_gotogarage(Scene *scene) {
	_got_to |= 8;
	_scene_iso_shootpast(scene);
}

void GameJohnnyRock::_scene_iso_gotomansion(Scene *scene) {
	_got_to |= 0x10;
}

void GameJohnnyRock::_scene_iso_inmansion1(Scene *scene) {
	_mansion = 1;
}

// Script functions: Scene NxtScn
void GameJohnnyRock::_scene_nxtscn_died(Scene *scene) {
	uint16 sceneNum = _ScenetoNum(_cur_scene);
	_in_office = sceneNum;
	if (sceneNum >= 0x13) {
		_in_office = 0;
	}
	_total_dies++;
	if (!_debug_godMode) {
		_game_money -= 400;
	}
	if (_game_money < 0) {
		switch (_rnd->getRandomNumber(2)) {
		case 0:
			_cur_scene = "scene148";
			break;
		case 1:
			_cur_scene = "scene149";
			break;
		case 2:
			_cur_scene = "scene150";
			break;
		}
	} else {
		switch (_pick_bits(&_doctor_bits, 8)) {
		case 0:
			_cur_scene = "scene140";
			break;
		case 1:
			_cur_scene = "scene141";
			break;
		case 2:
			_cur_scene = "scene142";
			break;
		case 3:
			_cur_scene = "scene143";
			break;
		case 4:
			_cur_scene = "scene144";
			break;
		case 5:
			_cur_scene = "scene145";
			break;
		case 6:
			_cur_scene = "scene146";
			break;
		case 7:
			_cur_scene = "scene147";
			break;
		}
	}
}

void GameJohnnyRock::_scene_nxtscn_bombdead(Scene *scene) {
	uint16 sceneNum = _ScenetoNum(_cur_scene);
	_in_office = sceneNum;
	if (sceneNum >= 0x13) {
		_in_office = 0;
	}
	_total_dies++;
	if (!_debug_godMode) {
		_game_money -= 400;
	}
	if (_game_money < 0) {
		switch (_rnd->getRandomNumber(2)) {
		case 0:
			_cur_scene = "scene148";
			break;
		case 1:
			_cur_scene = "scene149";
			break;
		case 2:
			_cur_scene = "scene150";
			break;
		}
	} else {
		_cur_scene = "scene142";
	}
}

void GameJohnnyRock::_scene_nxtscn_pikundrtakr(Scene *scene) {
	switch (_pick_bits(&_undertaker_bits, 3)) {
	case 0:
		_cur_scene = "scene154";
		break;
	case 1:
		_cur_scene = "scene155";
		break;
	case 2:
		_cur_scene = "scene156";
		break;
	}
}

void GameJohnnyRock::_scene_nxtscn_callattract(Scene *scene) {
	_ResetParams();
	_NewGame();
	_cur_scene = "scn354aa";
}

void GameJohnnyRock::_scene_nxtscn_pikluckno(Scene *scene) {
	_lucky_number = _rnd->getRandomNumber(5);
	debug("lucky number: %d", (_lucky_number + 1));
	_cur_scene = Common::String::format("scene%d", _lucky_number + 3);
}

void GameJohnnyRock::_scene_nxtscn_pickmap(Scene *scene) {
	Common::String nextScene;
	if (_game_money < 0) {
		switch (_rnd->getRandomNumber(2)) {
		case 0:
			_cur_scene = "scene148";
			break;
		case 1:
			_cur_scene = "scene149";
			break;
		case 2:
			_cur_scene = "scene150";
			break;
		}
		return;
	}
	uint16 sceneNum = _ScenetoNum(_cur_scene);
	if (sceneNum == 18) {
		_in_office = 0;
	}
	if (_in_office) {
		nextScene = _NumtoScene(_in_office);
	} else if (_office_count < 10) {
		if ((_office_count % 3) == 0) {
			uint16 sceneNume = _office_table[_office_count / 3];
			nextScene = _NumtoScene(sceneNume);
		}
		_office_count++;
	} else if (_this_clue == 2 && !_had_go_to_mansion) {
		if (!_did_continue) {
			_this_difficulty++;
			if (_this_difficulty > 7)
				_this_difficulty = 7;
		}
		_had_go_to_mansion = 1;
		if (_pool_hall) {
			_pool_hall = 2;
			nextScene = "scene162";
		} else if (_casino) {
			_casino = 2;
			nextScene = "scene163";
		} else if (_warehouse) {
			_warehouse = 2;
			nextScene = "scene165";
		}
	} else if (_this_clue == 3 && _had_go_to_mansion == 1) {
		if (_total_dies < 4) {
			_this_difficulty++;
			if (_this_difficulty > 7)
				_this_difficulty = 7;
		}
		_had_go_to_mansion = 2;
		if (_pool_hall == 1) {
			_pool_hall = 2;
			nextScene = "scene162";
		} else if (_casino == 1) {
			_casino = 2;
			nextScene = "scene163";
		} else if (_warehouse == 1) {
			_warehouse = 2;
			nextScene = "scene165";
		}
	} else if (_had_go_to_mansion == 2 && _garage && _casino && _pool_hall && _warehouse) {
		if (_total_dies < 5 || _did_continue <= 1) {
			_this_difficulty++;
			if (_this_difficulty > 7)
				_this_difficulty = 7;
		}
		_had_go_to_mansion = 3;
		nextScene = "scene166";
	}
	if(nextScene.size() > 0) {
		_cur_scene = nextScene;
	} else {
		_cur_scene = _NumtoScene(_this_map + 174);
	}
}

void GameJohnnyRock::_scene_nxtscn_pickclue(Scene *scene) {
	uint16 picked = _pick_bits(&_clues, 12);
	_clue_table[_this_clue] = picked;
	_this_clue++;
	_cur_scene = _NumtoScene(picked + 0xC3);
}

void GameJohnnyRock::_scene_nxtscn_maptimeout(Scene *scene) {
	_map_timeout++;
	if (_map_timeout < 3) {
		_cur_scene = "scene360";
	} else {
		_cur_scene = "scene262";
	}
}

void GameJohnnyRock::_scene_nxtscn_entcasino(Scene *scene) {
	_place_bits = 0;
	_random_count = 0;
	uint16 sceneNum;
	if (_casino_type != 0) {
		sceneNum = (_pick_bits(&_place_bits, 12) * 2) + 0x14;
	} else {
		sceneNum = (_pick_bits(&_place_bits, 8) * 2) + 0x2D;
	}
	_cur_scene = _NumtoScene(sceneNum);
}

void GameJohnnyRock::_scene_nxtscn_casinowhat(Scene *scene) {
	_random_count++;
	uint16 maxRandom = ((_this_difficulty * 3) + 6);
	if (_random_count > maxRandom) {
		_casino = 1;
		_cur_scene = "scene63a";
	} else {
		uint16 sceneNum;
		if (_casino_type != 0) {
			sceneNum = (_pick_bits(&_place_bits, 12) * 2) + 0x14;
		} else {
			sceneNum = (_pick_bits(&_place_bits, 8) * 2) + 0x2D;
		}
		_cur_scene = _NumtoScene(sceneNum);
	}
}

void GameJohnnyRock::_scene_nxtscn_entpoolhall(Scene *scene) {
	if (_pool_hall_type == 0) {
		_cur_scene = "scene67";
	} else if (_pool_hall_type == 1) {
		_cur_scene = "scene73";
	} else {
		_cur_scene = "scene78";
	}
}

void GameJohnnyRock::_scene_nxtscn_poolhclue(Scene *scene) {
	_pool_hall = 1;
	uint16 clue = _pick_bits(&_clues, 12);
	_clue_table[_this_clue] = clue;
	_this_clue++;
	_cur_scene = _NumtoScene(clue + 0xC3);
}

void GameJohnnyRock::_scene_nxtscn_entwarehse(Scene *scene) {
	if (_warehouse_type == 0) {
		_cur_scene = "scene94";
	} else if (_warehouse_type == 1) {
		_cur_scene = "scene102";
	} else {
		_cur_scene = "scene110";
	}
}

void GameJohnnyRock::_scene_nxtscn_warehseclue(Scene *scene) {
	_warehouse = 1;
	uint16 clue = _pick_bits(&_clues, 12);
	_clue_table[_this_clue] = clue;
	_this_clue++;
	_cur_scene = _NumtoScene(clue + 0xC3);
}

void GameJohnnyRock::_scene_nxtscn_entgarage(Scene *scene) {
	if (_garage_type != 0) {
		_cur_scene = "scene124";
	} else {
		_cur_scene = "scene131";
	}
}

void GameJohnnyRock::_scene_nxtscn_garageclue(Scene *scene) {
	_garage = 1;
	uint16 clue = _pick_bits(&_clues, 12);
	_clue_table[_this_clue] = clue;
	_this_clue++;
	_cur_scene = _NumtoScene(clue + 0xC3);
}

void GameJohnnyRock::_scene_nxtscn_entmansion(Scene *scene) {
	_mansion = 1;
	_random_count++;
	uint16 maxRandom = ((_this_difficulty * 2) + 7);
	if (_random_count <= maxRandom) {
		uint16 picked = _pick_bits(&_place_bits, 5);
		_cur_scene = _NumtoScene((picked * 2) + 0xB8);
	} else {
		_mansion = 2;
		_cur_scene = "scene194";
	}
}

void GameJohnnyRock::_scene_nxtscn_giveclue(Scene *scene) {
	_score += 1000;
	_game_money += 50;
	_scene_nxtscn_pickmap(scene);
}

void GameJohnnyRock::_scene_nxtscn_pikflwrman(Scene *scene) {
	if (_rnd->getRandomBit()) {
		_cur_scene = "scene10a";
	} else {
		_cur_scene = "scene12a";
	}
}

void GameJohnnyRock::_scene_nxtscn_randomscene(Scene *scene) {
	_random_count++;
	if (_random_count <= _max_random_count) {
		_place_bits = _pick_bits(&_place_bits, _random_scenes[0]);
		_cur_scene = _NumtoScene(_random_scenes[_place_bits + 4]);
	} else {
		if (_random_scenes[2] != 0) {
			_cur_scene = _NumtoScene(_random_scenes[2]);
		} else if (_repeat_random_place > 0) {
			_repeat_random_place--;
			_max_repeat--;
			uint16 picked = _pick_random_place(_repeat_random_place + 5);
			_cur_scene = _NumtoScene(picked);
		} else {
			_cur_scene = _NumtoScene(_goto_after_random);
			_goto_after_random = 0;
			_place_bits = 0;
			_random_count = 1;
		}
	}
}

void GameJohnnyRock::_scene_nxtscn_endrandscene(Scene *scene) {
	if (_repeat_random_place > 0) {
		_repeat_random_place--;
		_max_repeat--;
		uint16 picked = _pick_random_place(_repeat_random_place + 5);
		_cur_scene = _NumtoScene(picked);
	} else {
		_cur_scene = _NumtoScene(_goto_after_random);
		_goto_after_random = 0;
		_place_bits = 0;
		_random_count = 1;
	}
}

void GameJohnnyRock::_scene_nxtscn_killinnocent(Scene *scene) {
	if (!_debug_godMode) {
		_game_money -= 400;
	}
	if (_game_money < 0) {
		_ret_scene = "scene358";
		_cur_scene = "scene151";
	} else {
		switch (_rnd->getRandomNumber(2)) {
		case 0:
			_cur_scene = "scene151";
			break;
		case 1:
			_cur_scene = "scene152";
			break;
		case 2:
			_cur_scene = "scene153";
			break;
		}
	}
}

// Script functions: WepDwn
void GameJohnnyRock::_scene_default_wepdwn(Scene *scene) {
	_inHolster = 9;
	_whichGun = 7;
	_UpdateMouse();
}

// Debug methods
void GameJohnnyRock::debug_warpTo(int val) {
	// TODO implement
}

// Debugger methods
DebuggerJohnnyRock::DebuggerJohnnyRock(GameJohnnyRock *game) : GUI::Debugger() {
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
		_game->debug_warpTo(val);
		return false;
	}
}

bool DebuggerJohnnyRock::cmdDumpLib(int argc, const char **argv) {
	return _game->debug_dumpLibFile();
}

} // End of namespace Alg
