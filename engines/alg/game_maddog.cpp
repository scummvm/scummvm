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

#include "alg/game_maddog.h"
#include "alg/graphics.h"
#include "alg/scene.h"

namespace Alg {

GameMaddog::GameMaddog(AlgEngine *vm, const ADGameDescription *desc) : Game(vm) {
	_libFileName = "maddog.lib";
}

GameMaddog::~GameMaddog() {
}

void GameMaddog::init() {
	_videoPosX = 56;
	_videoPosY = 8;

	_SetupCursorTimer();

	loadLibArchive(_libFileName);
	_sceneInfo->loadScnFile("maddog.scn");
	_startscene = _sceneInfo->getStartScene();

	registerScriptFunctions();
	verifyScriptFunctions();

	_menuzone = new Zone();
	_menuzone->name = "MainMenu";
	_menuzone->ptrfb = "GLOBALHIT";
	_menuzone->addRect(0x0C, 0xAC, 0x3D, 0xBF, nullptr, 0, "SHOTMENU", "0");
	_menuzone->addRect(0x00, 0xA6, 0x013F, 0xC7, nullptr, 0, "DEFAULT", "0"); // _mm_bott
	_menuzone->addRect(0x00, 0x00, 0x3B, 0xC7, nullptr, 0, "DEFAULT", "0");   // _mm_left

	_submenzone = new Zone();
	_submenzone->name = "SubMenu";
	_submenzone->ptrfb = "GLOBALHIT";
	_submenzone->addRect(0x8A, 0x3B, 0xC2, 0x48, nullptr, 0, "STARTBOT", "0");
	_submenzone->addRect(0x8A, 0x4E, 0xC2, 0x59, nullptr, 0, "STARTMENU", "0");
	_submenzone->addRect(0x8A, 0x60, 0xC2, 0x6B, nullptr, 0, "CONTMENU", "0");
	_submenzone->addRect(0xE3, 0x3B, 0x011B, 0x48, nullptr, 0, "RECTSAVE", "0");
	_submenzone->addRect(0xE3, 0x4E, 0x011B, 0x59, nullptr, 0, "RECTLOAD", "0");
	_submenzone->addRect(0xE3, 0x60, 0x011B, 0x6B, nullptr, 0, "EXITMENU", "0");
	_submenzone->addRect(0x42, 0x34, 0x5C, 0x4E, nullptr, 0, "RECTEASY", "0");
	_submenzone->addRect(0x42, 0x53, 0x5C, 0x70, nullptr, 0, "RECTAVG", "0");
	_submenzone->addRect(0x42, 0x72, 0x62, 0x8A, nullptr, 0, "RECTHARD", "0");

	_shotSound = _LoadSoundFile("blow.8b");
	_emptySound = _LoadSoundFile("empty.8b");
	_saveSound = _LoadSoundFile("saved.8b");
	_loadSound = _LoadSoundFile("loaded.8b");
	_skullSound = _LoadSoundFile("skull.8b");
	_easySound = _LoadSoundFile("deputy.8b");
	_avgSound = _LoadSoundFile("sheriff.8b");
	_hardSound = _LoadSoundFile("marshall.8b");

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
	Common::Array<Graphics::Surface> *hole = AlgGraphics::loadScreenCoordAniImage("hole.ani", _palette);
	_bulletholeIcon = (*hole)[0];

	_background = AlgGraphics::loadVgaBackground("backgrnd.vga", _palette);
	_screen->copyRectToSurface(_background->getPixels(), _background->pitch, 0, 0, _background->w, _background->h);

	_MoveMouse();
}

void GameMaddog::registerScriptFunctions() {
#define ZONE_PTRFB_FUNCTION(name, func) _zonePtrFb[name] = new MDScriptFunctionPoint(this, &GameMaddog::func);
	ZONE_PTRFB_FUNCTION("DEFAULT", _zone_bullethole);
	ZONE_PTRFB_FUNCTION("GLOBALHIT", _zone_globalhit);
	ZONE_PTRFB_FUNCTION("BULLETHOLE", _zone_bullethole);
	ZONE_PTRFB_FUNCTION("SKULL", _zone_skullhole);
#undef ZONE_PTRFB_FUNCTION

#define RECT_HIT_FUNCTION(name, func) _rectHitFuncs[name] = new MDScriptFunctionRect(this, &GameMaddog::func);
	RECT_HIT_FUNCTION("DEFAULT", _rect_newscene);
	RECT_HIT_FUNCTION("STARTMENU", _rect_start);
	RECT_HIT_FUNCTION("SHOTMENU", _rect_shotmenu);
	RECT_HIT_FUNCTION("EXITMENU", _rect_exit);
	RECT_HIT_FUNCTION("CONTMENU", _rect_continue);
	RECT_HIT_FUNCTION("RECTSAVE", _rect_save);
	RECT_HIT_FUNCTION("RECTLOAD", _rect_load);
	RECT_HIT_FUNCTION("RECTEASY", _rect_easy);
	RECT_HIT_FUNCTION("RECTAVG", _rect_average);
	RECT_HIT_FUNCTION("RECTHARD", _rect_hard);
	RECT_HIT_FUNCTION("STARTBOT", _rect_startbottles);
	RECT_HIT_FUNCTION("HIDEFRONT", _rect_hidefront);
	RECT_HIT_FUNCTION("HIDEREAR", _rect_hiderear);
	RECT_HIT_FUNCTION("NEWSCENE", _rect_newscene);
	RECT_HIT_FUNCTION("MENUSELECT", _rect_menuselect);
	RECT_HIT_FUNCTION("SKULL", _rect_skull);
	RECT_HIT_FUNCTION("KILLMAN", _rect_killman);
	RECT_HIT_FUNCTION("KILLWOMAN", _rect_killwoman);
	RECT_HIT_FUNCTION("PROSPSIGN", _rect_prospsign);
	RECT_HIT_FUNCTION("MINESIGN", _rect_minesign);
	RECT_HIT_FUNCTION("MINEITEM1", _rect_mineitem1);
	RECT_HIT_FUNCTION("MINEITEM2", _rect_mineitem2);
	RECT_HIT_FUNCTION("MINEITEM3", _rect_mineitem3);
	RECT_HIT_FUNCTION("MINELANTERN", _rect_minelantern);
	RECT_HIT_FUNCTION("SHOTHIDEOUT", _rect_shothideout);
	RECT_HIT_FUNCTION("SHOTRIGHT", _rect_shotright);
	RECT_HIT_FUNCTION("SHOTLEFT", _rect_shotleft);
#undef RECT_HIT_FUNCTION

#define PRE_OPS_FUNCTION(name, func) _scenePreOps[name] = new MDScriptFunctionScene(this, &GameMaddog::func);
	PRE_OPS_FUNCTION("DRAWRCT", _scene_pso_drawrct);
	PRE_OPS_FUNCTION("PAUSE", _scene_pso_pause);
	PRE_OPS_FUNCTION("PRESHOOTOUT", _scene_pso_shootout);
	PRE_OPS_FUNCTION("MDSHOOTOUT", _scene_pso_mdshootout);
	PRE_OPS_FUNCTION("FADEIN", _scene_pso_fadein);
	PRE_OPS_FUNCTION("PAUSFI", _scene_pso_pause_fadein);
	PRE_OPS_FUNCTION("PREREAD", _scene_pso_preread);
	PRE_OPS_FUNCTION("PAUSPR", _scene_pso_pause_preread);
	PRE_OPS_FUNCTION("DEFAULT", _scene_pso_drawrct);
#undef PRE_OPS_FUNCTION

#define INS_OPS_FUNCTION(name, func) _sceneInsOps[name] = new MDScriptFunctionScene(this, &GameMaddog::func);
	INS_OPS_FUNCTION("DEFAULT", _scene_iso_donothing);
	INS_OPS_FUNCTION("PAUSE", _scene_iso_pause);
	INS_OPS_FUNCTION("SPAUSE", _scene_iso_spause);
	INS_OPS_FUNCTION("STARTGAME", _scene_iso_startgame);
	INS_OPS_FUNCTION("SHOOTPAST", _scene_iso_shootpast);
	INS_OPS_FUNCTION("SKIPSALOON", _scene_iso_skipsaloon);
	INS_OPS_FUNCTION("SKIPSALOON2", _scene_iso_skipsaloon2);
	INS_OPS_FUNCTION("CHECKSALOON", _scene_iso_checksaloon);
	INS_OPS_FUNCTION("INTOSTABLE", _scene_iso_intostable);
	INS_OPS_FUNCTION("INTOFFICE", _scene_iso_intoffice);
	INS_OPS_FUNCTION("INTOBANK_SP", _scene_iso_intobank);
	INS_OPS_FUNCTION("CHKBARTNDR", _scene_iso_chkbartndr);
	INS_OPS_FUNCTION("DIDHIDEOUT", _scene_iso_didhideout);
	INS_OPS_FUNCTION("DIDSIGNPOST", _scene_iso_didsignpost);
	INS_OPS_FUNCTION("DOSHOOTOUT", _scene_iso_doshootout);
	INS_OPS_FUNCTION("MDSHOOTOUT", _scene_iso_mdshootout);
	INS_OPS_FUNCTION("SHOTINTO24", _scene_iso_donothing);
	INS_OPS_FUNCTION("SHOTINTO116", _scene_iso_shotinto116);
#undef INS_OPS_FUNCTION

#define NXT_SCN_FUNCTION(name, func) _sceneNxtScn[name] = new MDScriptFunctionScene(this, &GameMaddog::func);
	NXT_SCN_FUNCTION("DEFAULT", _scene_default_nxtscn);
	NXT_SCN_FUNCTION("DRAWGUN", _scene_default_nxtscn);
	NXT_SCN_FUNCTION("PICKBOTTLE", _scene_nxtscn_pickbottle);
	NXT_SCN_FUNCTION("DIED", _scene_nxtscn_died);
	NXT_SCN_FUNCTION("AUTOSEL", _scene_nxtscn_autosel);
	NXT_SCN_FUNCTION("FINSALOON", _scene_nxtscn_finsaloon);
	NXT_SCN_FUNCTION("FINOFFICE", _scene_nxtscn_finoffice);
	NXT_SCN_FUNCTION("FINSTABLE", _scene_nxtscn_finstable);
	NXT_SCN_FUNCTION("FINBANK", _scene_nxtscn_finbank);
	NXT_SCN_FUNCTION("PICSALOON", _scene_nxtscn_picsaloon);
	NXT_SCN_FUNCTION("KILLMAN", _scene_nxtscn_killman);
	NXT_SCN_FUNCTION("KILLWOMAN", _scene_nxtscn_killwoman);
	NXT_SCN_FUNCTION("BANK", _scene_nxtscn_bank);
	NXT_SCN_FUNCTION("STABLE", _scene_nxtscn_stable);
	NXT_SCN_FUNCTION("SAVPROSP", _scene_nxtscn_savprosp);
	NXT_SCN_FUNCTION("PICKTOSS", _scene_nxtscn_picktoss);
	NXT_SCN_FUNCTION("HITTOSS", _scene_nxtscn_hittoss);
	NXT_SCN_FUNCTION("MISSTOSS", _scene_nxtscn_misstoss);
	NXT_SCN_FUNCTION("PICKSIGN", _scene_nxtscn_picksign);
	NXT_SCN_FUNCTION("BROCKMAN", _scene_nxtscn_brockman);
	NXT_SCN_FUNCTION("LROCKMAN", _scene_nxtscn_lrockman);
	NXT_SCN_FUNCTION("HOTELMEN", _scene_nxtscn_hotelmen);
#undef NXT_SCN_FUNCTION

	_sceneShowMsg["DEFAULT"] = new MDScriptFunctionScene(this, &GameMaddog::_scene_sm_donothing);
	_sceneWepDwn["DEFAULT"] = new MDScriptFunctionScene(this, &GameMaddog::_scene_default_wepdwn);
	_sceneScnScr["DEFAULT"] = new MDScriptFunctionScene(this, &GameMaddog::_scene_default_score);
	_sceneNxtFrm["DEFAULT"] = new MDScriptFunctionScene(this, &GameMaddog::_scene_nxtfrm);
}

void GameMaddog::verifyScriptFunctions() {
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

MDScriptFunctionPoint GameMaddog::getScriptFunctionZonePtrFb(Common::String name) {
	MDScriptFunctionPointMap::iterator it = _zonePtrFb.find(name);
	if (it != _zonePtrFb.end()) {
		return (*(*it)._value);
	} else {
		error("Could not find zonePtrFb function: %s", name.c_str());
	}
}

MDScriptFunctionRect GameMaddog::getScriptFunctionRectHit(Common::String name) {
	MDScriptFunctionRectMap::iterator it = _rectHitFuncs.find(name);
	if (it != _rectHitFuncs.end()) {
		return (*(*it)._value);
	} else {
		error("Could not find rectHit function: %s", name.c_str());
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
		error("Unkown scene script type: %u", type);
		break;
	}
	MDScriptFunctionSceneMap::iterator it;
	it = functionMap->find(name);
	if (it != functionMap->end()) {
		return (*(*it)._value);
	} else {
		error("Could not find scene type %u function: %s", type, name.c_str());
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
	_NewGame();
	_cur_scene = _startscene;
	Common::String oldscene;
	Rect *hitRect = nullptr;
	while (!_vm->shouldQuit()) {
		oldscene = _cur_scene;
		_SetFrame();
		_fired = false;
		if (_cur_scene == "scene28") {
			_cur_scene = _pick_town();
		}
		Scene *scene = _sceneInfo->findScene(_cur_scene);
		if (!loadScene(scene)) {
			error("Cannot find scene %s in libfile", scene->name.c_str());
		}
		Audio::PacketizedAudioStream *audioStream = _videoDecoder->getAudioStream();
		g_system->getMixer()->stopHandle(_sceneAudioHandle);
		g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &_sceneAudioHandle, audioStream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
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
							hitRect = nullptr;
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

void GameMaddog::_NewGame() {
	_shots = 6;
	_lives = 3;
	_score = 0;
	_holster = false;
	_UpdateStat();
	_sub_scene = "";
}

void GameMaddog::_ResetParams() {
	_been_to = 0;
	_bottles = 0;
	_botmask = 0;
	_got_into = 0;
	_had_skull = false;
	_bad_men = 0;
	_bad_men_bits = 0;
	_people_killed = 0;
	_hide_out_front = false;
	_difficulty = 1;
	_gun_time = 0;
	_pro_clue = 0;
	_got_clue = false;
	_had_lantern = false;
	_map_pos = 0;
	_shoot_out_cnt = 0;
	_max_map_pos = 0;
	_sheriff_cnt = 0;
	_in_shootout = false;
	_ret_scene = "";
	_sub_scene = "";
}

void GameMaddog::_DoMenu() {
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

void GameMaddog::_UpdateStat() {
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

void GameMaddog::_ChangeDifficulty(uint8 newDifficulty) {
	if (newDifficulty == _oldDifficulty) {
		return;
	}
	_ShowDifficulty(newDifficulty, true);
	Game::_AdjustDifficulty(newDifficulty, _oldDifficulty);
	_oldDifficulty = newDifficulty;
	_difficulty = newDifficulty;
}

void GameMaddog::_ShowDifficulty(uint8 newDifficulty, bool updateCursor) {
	// reset menu screen
	_screen->copyRectToSurface(_background->getBasePtr(_videoPosX, _videoPosY), _background->pitch, _videoPosX, _videoPosY, _videoDecoder->getWidth(), _videoDecoder->getHeight());
	AlgGraphics::drawImageCentered(_screen, &_knifeIcon, _diffpos[newDifficulty][0], _diffpos[newDifficulty][1]);
	if (updateCursor) {
		_DoCursor();
	}
}

void GameMaddog::_DoCursor() {
	_UpdateMouse();
}

void GameMaddog::_UpdateMouse() {
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

void GameMaddog::_MoveMouse() {
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
	_UpdateMouse();
}

void GameMaddog::_DisplayScore() {
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
		AlgGraphics::drawImage(_screen, &(*_numbers)[digit], posX, 0xAD);
		posX += 10;
	}
}

bool GameMaddog::_WeaponDown() {
	if (_rightDown && _mousePos.y > 168) {
		return true;
	}
	return false;
}

bool GameMaddog::_SaveState() {
	Common::OutSaveFile *outSaveFile;
	Common::String saveFileName = _vm->getSaveStateName(0);
	if (!(outSaveFile = g_system->getSavefileManager()->openForSaving(saveFileName))) {
		warning("Can't create file '%s', game not saved", saveFileName.c_str());
		return false;
	}
	outSaveFile->writeUint32BE(MKTAG('A', 'L', 'G', 'S')); // header
	outSaveFile->writeByte(0);                             // version, unused for now
	outSaveFile->writeUint16LE(_been_to);
	outSaveFile->writeUint16LE(_got_into);
	outSaveFile->writeByte(_had_skull);
	outSaveFile->writeByte(_bad_men);
	outSaveFile->writeByte(_bad_men_bits);
	outSaveFile->writeByte(_people_killed);
	outSaveFile->writeByte(_hide_out_front);
	outSaveFile->writeByte(_difficulty);
	outSaveFile->writeByte(_pro_clue);
	outSaveFile->writeByte(_got_clue);
	outSaveFile->writeByte(_had_lantern);
	outSaveFile->writeByte(_map_pos);
	outSaveFile->writeByte(_shoot_out_cnt);
	outSaveFile->writeSByte(_map0);
	outSaveFile->writeSByte(_map1);
	outSaveFile->writeSByte(_map2);
	outSaveFile->writeByte(_max_map_pos);
	outSaveFile->writeByte(_bartender_alive);
	outSaveFile->writeByte(_sheriff_cnt);
	outSaveFile->writeByte(_in_shootout);
	outSaveFile->writeString(_cur_scene);
	outSaveFile->writeByte(0);
	outSaveFile->writeString(_ret_scene);
	outSaveFile->writeByte(0);
	outSaveFile->writeString(_sub_scene);
	outSaveFile->writeByte(0);
	outSaveFile->finalize();
	delete outSaveFile;
	return true;
}

bool GameMaddog::_LoadState() {
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
	_been_to = inSaveFile->readUint16LE();
	_got_into = inSaveFile->readUint16LE();
	_had_skull = inSaveFile->readByte();
	_bad_men = inSaveFile->readByte();
	_bad_men_bits = inSaveFile->readByte();
	_people_killed = inSaveFile->readByte();
	_hide_out_front = inSaveFile->readByte();
	_difficulty = inSaveFile->readByte();
	_pro_clue = inSaveFile->readByte();
	_got_clue = inSaveFile->readByte();
	_had_lantern = inSaveFile->readByte();
	_map_pos = inSaveFile->readByte();
	_shoot_out_cnt = inSaveFile->readByte();
	_map0 = inSaveFile->readSByte();
	_map1 = inSaveFile->readSByte();
	_map2 = inSaveFile->readSByte();
	_max_map_pos = inSaveFile->readByte();
	_bartender_alive = inSaveFile->readByte();
	_sheriff_cnt = inSaveFile->readByte();
	_in_shootout = inSaveFile->readByte();
	_cur_scene = inSaveFile->readString();
	_ret_scene = inSaveFile->readString();
	_sub_scene = inSaveFile->readString();
	delete inSaveFile;
	_ChangeDifficulty(_difficulty);
	return true;
}

// misc game functions
void GameMaddog::_default_bullethole(Common::Point *point) {
	if (point->x >= 59 && point->y <= 166) {
		_RestoreCursor();
		uint16 targetX = point->x - _videoPosX;
		uint16 targetY = point->y - _videoPosY;
		AlgGraphics::drawImageCentered(_videoDecoder->getVideoFrame(), &_bulletholeIcon, targetX, targetY);
		_DoCursor();
		_shotFired = true;
		_DoShot();
	}
}

void GameMaddog::_die() {
	Common::String newScene;
	_UpdateStat();
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
	_cur_scene = newScene;
}

uint8 GameMaddog::_pick_rand(uint8 *bits, uint8 max) {
	uint8 random, mask;
	// reset bits if full
	if (*bits == (0xFF >> (8 - max))) {
		*bits = 0;
	}
	do {
		random = _rnd->getRandomNumber(max - 1);
		mask = 1 << random;
	} while (*bits & mask);
	*bits |= mask;
	return random * 2;
}

uint8 GameMaddog::_pick_bad(uint8 max) {
	return _pick_rand(&_bad_men_bits, max);
}

Common::String GameMaddog::_pick_town() {
	_had_skull = false;
	_map_pos = 0;
	_bad_men_bits = 0;
	_shoot_out_cnt++;
	if (_shoot_out_cnt % 5 == 0) {
		if (_shoot_out_cnt > 15 || _shoot_out_cnt == 0) {
			_shoot_out_cnt = 5;
		}
		// surprise showdown!
		int pickedSceneNum = (_shoot_out_cnt / 5) + 106;
		return Common::String::format("scene%d", pickedSceneNum);
	} else if (_been_to == 0) {
		return "scene28";
	} else if (_been_to & 0x100) {
		return "scene250";
	} else if (_been_to & 0x80) {
		return "scene76";
	} else if (_been_to & 0x40) {
		return "scene214";
	} else if (_been_to >= 15) {
		return "scene186";
	} else {
		return Common::String::format("scene%d", _been_to + 29);
	}
}

Common::String GameMaddog::_pick_map() {
	_been_to |= 0x20;
	uint32 random = _rnd->getRandomNumber(5);
	switch (random) {
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

Common::String GameMaddog::_pick_sign() {
	int8 _mapArray[3] = {_map0, _map1, _map2};
	_map_pos++;
	if (_map_pos > _max_map_pos) {
		_max_map_pos = _map_pos;
	}
	if (_map_pos <= 2 && _mapArray[_map_pos] != 0) {
		return Common::String::format("scene%d", _map_pos + 187);
	} else {
		return "scene210";
	}
}

Common::String GameMaddog::_map_right() {
	int8 _mapArray[3] = {_map0, _map1, _map2};
	if (_mapArray[_map_pos] == -1) {
		if (_map_pos >= _max_map_pos) {
			return Common::String::format("scene%d", _fight[_map_pos]);
		} else {
			return _pick_sign();
		}
	} else if (_mapArray[_map_pos] == 0) {
		if (_map_pos >= _max_map_pos) {
			return Common::String::format("scene%d", _fight[_map_pos]);
		} else {
			return _pick_sign();
		}
	} else {
		return Common::String::format("scene%d", _ambush[_map_pos]);
	}
}

Common::String GameMaddog::_map_left() {
	int8 _mapArray[3] = {_map0, _map1, _map2};
	if (_mapArray[_map_pos] == 1) {
		if (_map_pos >= _max_map_pos) {
			return Common::String::format("scene%d", _fight[_map_pos]);
		} else {
			return _pick_sign();
		}
	} else if (_mapArray[_map_pos] == -1) {
		return Common::String::format("scene%d", _ambush[_map_pos]);
	} else {
		if (_map_pos >= _max_map_pos) {
			return Common::String::format("scene%d", _fight[_map_pos]);
		} else {
			return _pick_sign();
		}
	}
}

// Script functions: Zone
void GameMaddog::_zone_bullethole(Common::Point *point) {
	_default_bullethole(point);
}

void GameMaddog::_zone_skullhole(Common::Point *point) {
	if (point->x >= 59 && point->y <= 166) {
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
void GameMaddog::_rect_hidefront(Rect *rect) {
	if (_hide_out_front) {
		_cur_scene = "scene214";
	} else {
		_cur_scene = "scene211";
	}
}

void GameMaddog::_rect_hiderear(Rect *rect) {
	if (!_hide_out_front) {
		_cur_scene = "scene214";
	} else {
		_cur_scene = "scene211";
	}
}

void GameMaddog::_rect_menuselect(Rect *rect) {
	Common::String newScene;
	int cursorX = _mousePos.x;
	int cursorY = _mousePos.y;
	if (cursorX < 184) {
		if (cursorY < 88) {
			if (_been_to & 2)
				return;
			if (_got_into & 2) {
				newScene = "scene130";
			} else {
				newScene = "scene122";
			}
		} else {
			if (_been_to & 8)
				return;
			if (_got_into & 8) {
				if (_been_to & 1) {
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
			if (_been_to & 1)
				return;
			if (_got_into & 1) {
				newScene = "scene69b";
			} else {
				newScene = "scene67";
			}
		} else {
			if (_been_to & 4)
				return;
			if (_got_into & 4) {
				_scene_nxtscn_bank(nullptr);
				return;
			} else {
				newScene = "scene45";
			}
		}
	}

	_cur_scene = newScene;
}

void GameMaddog::_rect_skull(Rect *rect) {
	if (_had_skull) {
		return;
	}
	_had_skull = true;
	if (_been_to < 15) {
		_shots = 9;
	} else {
		_shots = 12;
	}
	_UpdateStat();
}

void GameMaddog::_rect_killman(Rect *rect) {
	_scene_nxtscn_killman(nullptr);
}

void GameMaddog::_rect_killwoman(Rect *rect) {
	_scene_nxtscn_killwoman(nullptr);
}

void GameMaddog::_rect_prospsign(Rect *rect) {
	if (_been_to & 0x10) {
		return;
	}
	_gun_time = 1;
	_cur_scene = rect->scene;
}

void GameMaddog::_rect_minesign(Rect *rect) {
	if (_been_to & 0x20) {
		return;
	}
	_gun_time = 1;
	_cur_scene = rect->scene;
}

void GameMaddog::_rect_mineitem1(Rect *rect) {
	if (_pro_clue != 0) {
		_pauseTime = 0;
		return;
	}

	if (_had_lantern) {
		_cur_scene = _pick_map();
	} else {
		_got_clue = true;
	}
}

void GameMaddog::_rect_mineitem2(Rect *rect) {
	if (_pro_clue != 2) {
		_pauseTime = 0;
		return;
	}

	if (_had_lantern) {
		_cur_scene = _pick_map();
	} else {
		_got_clue = true;
	}
}

void GameMaddog::_rect_mineitem3(Rect *rect) {
	if (_pro_clue != 1) {
		_pauseTime = 0;
		return;
	}

	if (_had_lantern) {
		_cur_scene = _pick_map();
	} else {
		_got_clue = true;
	}
}

void GameMaddog::_rect_minelantern(Rect *rect) {
	_had_lantern = true;

	if (!_got_clue) {
		return;
	}

	_cur_scene = _pick_map();
}

void GameMaddog::_rect_shothideout(Rect *rect) {
	_cur_scene = _pick_sign();
}

void GameMaddog::_rect_shotright(Rect *rect) {
	_cur_scene = _map_right();
}

void GameMaddog::_rect_shotleft(Rect *rect) {
	_cur_scene = _map_left();
}

void GameMaddog::_rect_shotmenu(Rect *rect) {
	_DoMenu();
}

void GameMaddog::_rect_continue(Rect *rect) {
	_inMenu = false;
	_fired = false;
	if (_lives == 0) {
		_NewGame();
		_ret_scene = "";
		_cur_scene = _pick_town();
	}
}

void GameMaddog::_rect_save(Rect *rect) {
	if(_SaveState()) {
		_DoSaveSound();
	}
}

void GameMaddog::_rect_load(Rect *rect) {
	if(_LoadState()) {
		_DoLoadSound();
	}
}

void GameMaddog::_rect_start(Rect *rect) {
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

void GameMaddog::_rect_startbottles(Rect *rect) {
	_inMenu = false;
	_fired = false;
	_cur_scene = "scene7";
	_ResetParams();
	_NewGame();
	_UpdateStat();
}

// Script functions: Scene PreOps
void GameMaddog::_scene_pso_shootout(Scene *scene) {
	sscanf(scene->preopParam.c_str(), "#%ldto%ld", &_minF, &_maxF);
	if (!_debug_unlimitedAmmo) {
		_shots = 0;
	}
	_in_shootout = true;
	_UpdateStat();
	_RestoreCursor();
	AlgGraphics::drawImage(_screen, &_reloadIcon, 0x40, 0xB0);
	_DoCursor();
}

void GameMaddog::_scene_pso_mdshootout(Scene *scene) {
	sscanf(scene->preopParam.c_str(), "#%ldto%ld", &_minF, &_maxF);
	if (!_debug_unlimitedAmmo) {
		_shots = 0;
	}
	_in_shootout = true;
	_UpdateStat();
	_RestoreCursor();
	AlgGraphics::drawImage(_screen, &_reloadIcon, 0x40, 0xB0);
	_DoCursor();
}

// Script functions: Scene Scene InsOps
void GameMaddog::_scene_iso_shootpast(Scene *scene) {
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

void GameMaddog::_scene_iso_spause(Scene *scene) {
	_scene_iso_shootpast(scene);
	_scene_iso_pause(scene);
}

void GameMaddog::_scene_iso_skipsaloon(Scene *scene) {
	if (_got_into & 1) {
		if (_currentFrame < 7561) {
			_cur_scene = scene->insopParam;
			_pauseTime = 0;
			return;
		}
	}
	if (_currentFrame > 7561) {
		_got_into |= 1;
	}
	if (_fired && _currentFrame > 7165 && _currentFrame < 7817) {
		_cur_scene = scene->insopParam;
	}
}

void GameMaddog::_scene_iso_skipsaloon2(Scene *scene) {
	Common::String insopParamTemp = scene->insopParam;
	scene->insopParam = Common::String::format("%u", scene->dataParam2);
	_scene_iso_pause(scene);
	scene->insopParam = insopParamTemp;
	_scene_iso_skipsaloon(scene);
}

void GameMaddog::_scene_iso_checksaloon(Scene *scene) {
	_got_into |= 1;
	if (_currentFrame > 7909) {
		_bartender_alive = false;
	} else {
		_bartender_alive = true;
	}
}

void GameMaddog::_scene_iso_intostable(Scene *scene) {
	_got_into |= 2;
}

void GameMaddog::_scene_iso_intoffice(Scene *scene) {
	_got_into |= 8;
}

void GameMaddog::_scene_iso_intobank(Scene *scene) {
	_got_into |= 4;
	_scene_iso_shootpast(scene);
}

void GameMaddog::_scene_iso_chkbartndr(Scene *scene) {
	if (!_bartender_alive) {
		if (scene->dataParam1 <= (int32)_currentFrame) {
			_cur_scene = scene->insopParam;
		}
	}
	if (_fired) {
		if (scene->dataParam2 < (int32)_currentFrame) {
			callScriptFunctionScene(NXTSCN, scene->nxtscn, scene);
		}
	}
}

void GameMaddog::_scene_iso_didhideout(Scene *scene) {
	_been_to |= 0x80;
}

void GameMaddog::_scene_iso_didsignpost(Scene *scene) {
	_been_to |= 0x40;
}

void GameMaddog::_scene_iso_doshootout(Scene *scene) {
	if (_currentFrame > (uint32)_minF) {
		if (_in_shootout) {
			_RestoreCursor();
			AlgGraphics::drawImage(_screen, &_drawIcon, 0x40, 0xB0);
			_DoCursor();
		}
		_in_shootout = false;
		if (_shots > 0) {
			if (_currentFrame < (uint32)_maxF) {
				callScriptFunctionScene(NXTSCN, scene->nxtscn, scene);
			}
		}
	}
}

void GameMaddog::_scene_iso_mdshootout(Scene *scene) {
	_been_to |= 0x100;
	_scene_iso_doshootout(scene);
}

void GameMaddog::_scene_iso_shotinto116(Scene *scene) {
	uint32 targetFrame = atoi(scene->insopParam.c_str());
	if (_fired) {
		if (_currentFrame > targetFrame) {
			callScriptFunctionScene(NXTSCN, scene->nxtscn, scene);
		}
	}
}

// Script functions: Scene NxtScn
void GameMaddog::_scene_default_nxtscn(Scene *scene) {
	// wipe background drawing from shootout
	_screen->copyRectToSurface(_background->getBasePtr(0x40, 0xB0), _background->pitch, 0x40, 0xB0, _reloadIcon.w, _reloadIcon.h);
	_DoCursor();
	Game::_scene_default_nxtscn(scene);
}

void GameMaddog::_scene_nxtscn_pickbottle(Scene *scene) {
	_bottles++;
	if (_bottles < 4) {
		int rand = _pick_rand(&_botmask, 6);
		_cur_scene = Common::String::format("scene%d", rand + 11);
	} else {
		_cur_scene = "scene253";
	}
}

void GameMaddog::_scene_nxtscn_died(Scene *scene) {
	_had_skull = false;
	_bad_men_bits = 0;
	_bad_men = 0;
	_got_clue = false;
	_had_lantern = false;
	if (!_debug_godMode) {
		_lives--;
	}
	if (_lives <= 0) {
		_lives = 0;
		_sub_scene = "scene255";
		_die();
		return;
	}
	_ret_scene = scene->next;
	_die();
}

void GameMaddog::_scene_nxtscn_autosel(Scene *scene) {
	Common::String newScene;
	if (!(_been_to & 2)) {
		newScene = "scene122";
	} else if (!(_been_to & 8)) {
		newScene = "scene114";
	} else if (!(_been_to & 1)) {
		if (_got_into & 1) {
			newScene = "scene69";
		} else {
			newScene = "scene67";
		}
	} else if (!(_been_to & 4)) {
		newScene = "scene45";
	} else {
		newScene = "scene186";
	}
	_cur_scene = newScene;
}

void GameMaddog::_scene_nxtscn_finsaloon(Scene *scene) {
	_been_to |= 1;
	_cur_scene = _pick_town();
}

void GameMaddog::_scene_nxtscn_finoffice(Scene *scene) {
	_been_to |= 8;
	_cur_scene = _pick_town();
}

void GameMaddog::_scene_nxtscn_finstable(Scene *scene) {
	_been_to |= 2;
	_cur_scene = _pick_town();
}

void GameMaddog::_scene_nxtscn_finbank(Scene *scene) {
	_been_to |= 4;
	_cur_scene = _pick_town();
}

void GameMaddog::_scene_nxtscn_picsaloon(Scene *scene) {
	if (_been_to & 1) {
		_cur_scene = "scene118";
	} else {
		_cur_scene = "scene119";
	}
}

void GameMaddog::_scene_nxtscn_killman(Scene *scene) {
	if (!_debug_godMode) {
		_lives--;
	}
	if (_lives <= 0) {
		_lives = 0;
		_sub_scene = "scene212";
	} else {
		_sub_scene = _pick_town();
	}
	_UpdateStat();
	_bad_men_bits = 0;
	_bad_men = 0;
	_people_killed++;
	if (_people_killed == 1) {
		_cur_scene = "scene155";
	} else {
		_cur_scene = Common::String::format("scene%d", 156 + (_people_killed & 1));
	}
}

void GameMaddog::_scene_nxtscn_killwoman(Scene *scene) {
	if (!_debug_godMode) {
		_lives--;
	}
	if (_lives <= 0) {
		_lives = 0;
		_sub_scene = "scene212";
	} else {
		_sub_scene = _pick_town();
	}
	_UpdateStat();
	_bad_men_bits = 0;
	_bad_men = 0;
	_people_killed++;
	if (_people_killed == 1) {
		_cur_scene = "scene154";
	} else {
		_cur_scene = Common::String::format("scene%d", 156 + (_people_killed & 1));
	}
}

void GameMaddog::_scene_nxtscn_bank(Scene *scene) {
	Common::String newScene;
	uint8 totalBadmen = (_difficulty * 2) + 6;
	_bad_men++;
	if (_bad_men > totalBadmen) {
		if (_bad_men > totalBadmen + 2) {
			_been_to |= 4;
			_bad_men_bits = 0;
			_bad_men = 0;
			if (_rnd->getRandomBit()) {
				_hide_out_front = true;
				newScene = "scene49";
			} else {
				_hide_out_front = false;
				newScene = "scene48";
			}
		} else {
			newScene = "scene65";
		}
	} else {
		int nextSceneNum = _pick_bad(6) + 51;
		newScene = Common::String::format("scene%d", nextSceneNum);
	}
	_cur_scene = newScene;
}

void GameMaddog::_scene_nxtscn_stable(Scene *scene) {
	Common::String newScene;
	uint16 totalBadMen = (_difficulty * 2) + 6;
	totalBadMen -= (_been_to & 8) ? 2 : 0;
	_bad_men++;
	if (_bad_men > totalBadMen) {
		_bad_men_bits = 0;
		_bad_men = 0;
		newScene = "scene143";
	} else {
		int nextSceneNum = _pick_bad(6) + 131;
		newScene = Common::String::format("scene%d", nextSceneNum);
	}
	_cur_scene = newScene;
}

void GameMaddog::_scene_nxtscn_savprosp(Scene *scene) {
	_gun_time = 1;
	_oldScore = -1;
	_pro_clue = _rnd->getRandomNumber(2);
	_been_to |= 0x10;
	_cur_scene = Common::String::format("scene%d", _pro_clue + 160);
}

void GameMaddog::_scene_nxtscn_picktoss(Scene *scene) {
	int index = _pick_bad(7);
	_cur_scene = Common::String::format("scene%d", _bottle_toss[index]);
}

void GameMaddog::_scene_nxtscn_hittoss(Scene *scene) {
	if (_lives > 0) {
		_score += 100;
	}
	_scene_nxtscn_misstoss(scene);
}

void GameMaddog::_scene_nxtscn_misstoss(Scene *scene) {
	_bad_men++;
	if (_bad_men <= 2) {
		_cur_scene = scene->next;
	} else {
		_bad_men_bits = 0;
		_bad_men = 0;
		_been_to |= 0x200;
		_cur_scene = "scene185";
	}
}

void GameMaddog::_scene_nxtscn_picksign(Scene *scene) {
	_cur_scene = _pick_sign();
}

void GameMaddog::_scene_nxtscn_brockman(Scene *scene) {
	long totalBadMen = (_difficulty * 2) + 9;
	_bad_men++;
	if (_bad_men > totalBadMen) {
		_bad_men_bits = 0;
		_bad_men = 0;
		_cur_scene = _pick_sign();
	} else {
		int nextBad = _pick_bad(7);
		_cur_scene = Common::String::format("scene%d", nextBad + 229);
	}
}

void GameMaddog::_scene_nxtscn_lrockman(Scene *scene) {
	long totalBadMen = (_difficulty * 2) + 4;
	_bad_men++;
	if (_bad_men > totalBadMen) {
		_bad_men_bits = 0;
		_bad_men = 0;
		_cur_scene = _pick_sign();
	} else {
		int nextBad = _pick_bad(3);
		_cur_scene = Common::String::format("scene%d", nextBad + 244);
	}
}

void GameMaddog::_scene_nxtscn_hotelmen(Scene *scene) {
	long totalBadMen = (_difficulty * 2) + 9;
	if (_bad_men >= totalBadMen) {
		_bad_men_bits = 0;
		_bad_men = 0;
		_been_to |= 0x100;
		_cur_scene = "scene250";
	} else {
		_bad_men++;
		uint32 index = _pick_bad(5);
		_cur_scene = Common::String::format("scene%d", _hotel_scenes[index]);
	}
}

// Script functions: WepDwn
void GameMaddog::_scene_default_wepdwn(Scene *scene) {
	_inHolster = 9;
	_whichGun = 7;
	_UpdateMouse();
	if (_in_shootout == 0) {
		if (_been_to >= 15) {
			if (_shots < 12) {
				_shots = 12;
			}
		} else {
			if (_shots < 6) {
				_shots = 6;
			}
		}
		_UpdateStat();
	}
}

// Debug methods
void GameMaddog::debug_warpTo(int val) {
	switch (val) {
	case 0:
		_been_to = 0;
		_cur_scene = "scene28";
		break;
	case 1:
		_been_to = 1;
		_cur_scene = _pick_town();
		break;
	case 2:
		_been_to = 15;
		_cur_scene = _pick_town();
		break;
	case 3:
		_been_to = 575;
		// always go right
		_map0 = -1;
		_map1 = -1;
		_map2 = -1;
		_cur_scene = _pick_town();
		break;
	case 4:
		_been_to = 575;
		_hide_out_front = true; // go to front
		_cur_scene = "scene210";
		break;
	case 5:
		_been_to = 639;
		_cur_scene = "scene227";
		break;
	case 6:
		_been_to = 1023;
		_cur_scene = "scene250";
		break;
	default:
		break;
	}
}

// Debugger methods
DebuggerMaddog::DebuggerMaddog(GameMaddog *game) : GUI::Debugger() {
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
		_game->debug_warpTo(val);
		return false;
	}
}

bool DebuggerMaddog::cmdDumpLib(int argc, const char **argv) {
	return _game->debug_dumpLibFile();
}

} // End of namespace Alg
