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

#include "alg/game_drugwars.h"
#include "alg/graphics.h"
#include "alg/scene.h"

namespace Alg {

GameDrugWars::GameDrugWars(AlgEngine *vm, const ADGameDescription *desc) : Game(vm) {
	if (scumm_stricmp(desc->gameId, "dwarss") == 0) {
		_libFileName = "dwss.lib";
	} else if (scumm_stricmp(desc->gameId, "dwarsd") == 0) {
		_libFileName = "dwds.lib";
	} else if (scumm_stricmp(desc->gameId, "dwarsdemo") == 0) {
		_libFileName = "dwdemo.lib";
		_isDemo = true;
	}
}

GameDrugWars::~GameDrugWars() {
}

void GameDrugWars::init() {
	_videoPosX = 11;
	_videoPosY = 2;

	loadLibArchive(_libFileName);
	_sceneInfo->loadScnFile("dw.scn");
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

	_submenzone->addRect(0x1C, 0x13, 0x5D, 0x22, nullptr, 0, "STARTMENU", "0");
	_submenzone->addRect(0x1C, 0x33, 0x5D, 0x42, nullptr, 0, "RECTLOAD", "0");
	_submenzone->addRect(0x1C, 0x53, 0x5D, 0x62, nullptr, 0, "RECTSAVE", "0");
	_submenzone->addRect(0x1C, 0x73, 0x5D, 0x82, nullptr, 0, "CONTMENU", "0");
	_submenzone->addRect(0x1C, 0x93, 0x5D, 0xA2, nullptr, 0, "EXITMENU", "0");
	_submenzone->addRect(0xDD, 0x34, 0x10A, 0x43, nullptr, 0, "RECTEASY", "0");
	_submenzone->addRect(0xDD, 0x55, 0x10A, 0x64, nullptr, 0, "RECTAVG", "0");
	_submenzone->addRect(0xDD, 0x75, 0x10A, 0x84, nullptr, 0, "RECTHARD", "0");

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

	_background = AlgGraphics::loadVgaBackground("dw_menu.vga", _palette);
	_screen->copyRectToSurface(_background->getPixels(), _background->pitch, 0, 0, _background->w, _background->h);

	_MoveMouse();
}

void GameDrugWars::registerScriptFunctions() {
#define RECT_HIT_FUNCTION(name, func) _rectHitFuncs[name] = new DWScriptFunctionRect(this, &GameDrugWars::func);
	RECT_HIT_FUNCTION("DEFAULT", _rect_newscene);
	RECT_HIT_FUNCTION("EXITMENU", _rect_exit);
	RECT_HIT_FUNCTION("CONTMENU", _rect_continue);
	RECT_HIT_FUNCTION("STARTMENU", _rect_start);
	RECT_HIT_FUNCTION("SHOTMENU", _rect_shotmenu);
	RECT_HIT_FUNCTION("RECTSAVE", _rect_save);
	RECT_HIT_FUNCTION("RECTLOAD", _rect_load);
	RECT_HIT_FUNCTION("RECTEASY", _rect_easy);
	RECT_HIT_FUNCTION("RECTAVG", _rect_average);
	RECT_HIT_FUNCTION("RECTHARD", _rect_hard);
	RECT_HIT_FUNCTION("SELECT_TARGET_PRACTICE", _rect_select_target_practice);
	RECT_HIT_FUNCTION("SELECT_BAR", _rect_select_bar);
	RECT_HIT_FUNCTION("SELECT_CAR_CHASE", _rect_select_car_chase);
	RECT_HIT_FUNCTION("SELECT_DRUG_HOUSE", _rect_select_drug_house);
	RECT_HIT_FUNCTION("SELECT_OFFICE", _rect_select_office);
	RECT_HIT_FUNCTION("SELECT_COURT", _rect_select_court);
	RECT_HIT_FUNCTION("SELECT_BUS", _rect_select_bus);
	RECT_HIT_FUNCTION("SELECT_DOCKS", _rect_select_docks);
	RECT_HIT_FUNCTION("SELECT_HOUSE_BOAT", _rect_select_house_boat);
	RECT_HIT_FUNCTION("SELECT_PARTY", _rect_select_party);
	RECT_HIT_FUNCTION("SELECT_AIRPORT", _rect_select_airport);
	RECT_HIT_FUNCTION("SELECT_MANSION", _rect_select_mansion);
	RECT_HIT_FUNCTION("SELECT_VILLAGE", _rect_select_village);
#undef RECT_HIT_FUNCTION

#define PRE_OPS_FUNCTION(name, func) _scenePreOps[name] = new DWScriptFunctionScene(this, &GameDrugWars::func);
	PRE_OPS_FUNCTION("DEFAULT", _scene_pso_drawrct);
	PRE_OPS_FUNCTION("FADEIN", _scene_pso_fadein);
	PRE_OPS_FUNCTION("PAUSE", _scene_pso_pause);
	PRE_OPS_FUNCTION("PAUSE_FADEIN", _scene_pso_pause_fadein);
	PRE_OPS_FUNCTION("GOT_TO", _scene_pso_got_to);
#undef PRE_OPS_FUNCTION

#define INS_OPS_FUNCTION(name, func) _sceneInsOps[name] = new DWScriptFunctionScene(this, &GameDrugWars::func);
	INS_OPS_FUNCTION("DEFAULT", _scene_iso_donothing);
	INS_OPS_FUNCTION("PAUSE", _scene_iso_pause);
#undef INS_OPS_FUNCTION

#define NXT_SCN_FUNCTION(name, func) _sceneNxtScn[name] = new DWScriptFunctionScene(this, &GameDrugWars::func);
	NXT_SCN_FUNCTION("DEFAULT", _scene_default_nxtscn);
	NXT_SCN_FUNCTION("GAME_WON", _scene_nxtscn_game_won);
	NXT_SCN_FUNCTION("LOSE_A_LIFE", _scene_nxtscn_lose_a_life);
	NXT_SCN_FUNCTION("CONTINUE_GAME", _scene_nxtscn_continue_game);
	NXT_SCN_FUNCTION("DID_NOT_CONTINUE", _scene_nxtscn_did_not_continue);
	NXT_SCN_FUNCTION("KILL_INNOCENT_MAN", _scene_nxtscn_kill_innocent_man);
	NXT_SCN_FUNCTION("KILL_INNOCENT_WOMAN", _scene_nxtscn_kill_innocent_woman);
	NXT_SCN_FUNCTION("AFTER_DIE", _scene_nxtscn_after_die);
	NXT_SCN_FUNCTION("INIT_RANDOM", _scene_nxtscn_init_random);
	NXT_SCN_FUNCTION("CONTINUE_RANDOM", _scene_nxtscn_continue_random);
	NXT_SCN_FUNCTION("SELECT_SCENARIO", _scene_nxtscn_select_scenario);
	NXT_SCN_FUNCTION("FINISH_SCENARIO", _scene_nxtscn_finish_scenario);
#undef NXT_SCN_FUNCTION

	_sceneShowMsg["DEFAULT"] = new DWScriptFunctionScene(this, &GameDrugWars::_scene_sm_donothing);
	_sceneWepDwn["DEFAULT"] = new DWScriptFunctionScene(this, &GameDrugWars::_scene_default_wepdwn);
	_sceneScnScr["DEFAULT"] = new DWScriptFunctionScene(this, &GameDrugWars::_scene_default_score);
	_sceneNxtFrm["DEFAULT"] = new DWScriptFunctionScene(this, &GameDrugWars::_scene_nxtfrm);
}

void GameDrugWars::verifyScriptFunctions() {
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

DWScriptFunctionRect GameDrugWars::getScriptFunctionRectHit(Common::String name) {
	DWScriptFunctionRectMap::iterator it = _rectHitFuncs.find(name);
	if (it != _rectHitFuncs.end()) {
		return (*(*it)._value);
	} else {
		error("Could not find rectHit function: %s", name.c_str());
	}
}

DWScriptFunctionScene GameDrugWars::getScriptFunctionScene(SceneFuncType type, Common::String name) {
	DWScriptFunctionSceneMap *functionMap;
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
	DWScriptFunctionSceneMap::iterator it;
	it = functionMap->find(name);
	if (it != functionMap->end()) {
		return (*(*it)._value);
	} else {
		error("Could not find scene type %u function: %s", type, name.c_str());
	}
}

void GameDrugWars::callScriptFunctionRectHit(Common::String name, Rect *rect) {
	DWScriptFunctionRect function = getScriptFunctionRectHit(name);
	function(rect);
}

void GameDrugWars::callScriptFunctionScene(SceneFuncType type, Common::String name, Scene *scene) {
	DWScriptFunctionScene function = getScriptFunctionScene(type, name);
	function(scene);
}

Common::Error GameDrugWars::run() {
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

void GameDrugWars::_NewGame() {
	_shots = 10;
	_lives = 3;
	_holster = false;
}

void GameDrugWars::_ResetParams() {
	// fill _got_to with scenario start scenes
	// 0 in _got_to array means the scenario is finished
	for (uint8 i = 0; i < 14; i++) {
		_got_to[i] = _scenario_start_scenes[i];
	}
}

void GameDrugWars::_DoMenu() {
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

void GameDrugWars::_ChangeDifficulty(uint8 newDifficulty) {
	if (newDifficulty == _oldDifficulty) {
		return;
	}
	_ShowDifficulty(newDifficulty, true);
	_oldDifficulty = newDifficulty;
	_difficulty = newDifficulty;
}

void GameDrugWars::_ShowDifficulty(uint8 newDifficulty, bool updateCursor) {
	// reset menu screen
	_screen->copyRectToSurface(_background->getBasePtr(_videoPosX, _videoPosY), _background->pitch, _videoPosX, _videoPosY, _videoDecoder->getWidth(), _videoDecoder->getHeight());
	uint16 posY = 0x3C + ((newDifficulty - 1) * 0x21);
	AlgGraphics::drawImageCentered(_screen, &_difficultyIcon, 0x0115, posY);
	if (updateCursor) {
		_DoCursor();
	}
}

void GameDrugWars::_DoCursor() {
	_UpdateMouse();
}

void GameDrugWars::_UpdateMouse() {
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

void GameDrugWars::_MoveMouse() {
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

void GameDrugWars::_DisplayLivesLeft() {
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

void GameDrugWars::_DisplayScores() {
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

void GameDrugWars::_DisplayShotsLeft() {
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

bool GameDrugWars::_WeaponDown() {
	if (_rightDown && _mousePos.y >= 0xAA && _mousePos.x >= 0x113) {
		return true;
	}
	return false;
}

bool GameDrugWars::_SaveState() {
	Common::OutSaveFile *outSaveFile;
	Common::String saveFileName = _vm->getSaveStateName(0);
	if (!(outSaveFile = g_system->getSavefileManager()->openForSaving(saveFileName))) {
		warning("Can't create file '%s', game not saved", saveFileName.c_str());
		return false;
	}
	outSaveFile->writeUint32BE(MKTAG('A', 'L', 'G', 'S')); // header
	outSaveFile->writeByte(0);                             // version, unused for now
	outSaveFile->writeSByte(_stage);
	outSaveFile->writeByte(_continues);
	outSaveFile->writeSByte(_got_to_index);
	for (uint8 i = 0; i < 14; i++) {
		outSaveFile->writeUint16LE(_got_to[i]);
	}
	outSaveFile->writeSByte(_lives);
	outSaveFile->writeUint16LE(_shots);
	outSaveFile->writeSint32LE(_score);
	outSaveFile->writeByte(_difficulty);
	outSaveFile->writeString(_cur_scene);
	outSaveFile->writeByte(0);
	outSaveFile->finalize();
	delete outSaveFile;
	return true;
}

bool GameDrugWars::_LoadState() {
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
	_continues = inSaveFile->readByte();
	_got_to_index = inSaveFile->readSByte();
	for (uint8 i = 0; i < 14; i++) {
		_got_to[i] = inSaveFile->readUint16LE();
	}
	_lives = inSaveFile->readSByte();
	_shots = inSaveFile->readUint16LE();
	_score = inSaveFile->readSint32LE();
	_difficulty = inSaveFile->readByte();
	_cur_scene = inSaveFile->readString();
	delete inSaveFile;
	_gameInProgress = true;
	_ChangeDifficulty(_difficulty);
	return true;
}

// misc game functions
void GameDrugWars::_DisplayShotFiredImage(Common::Point *point) {
	if (point->x >= _videoPosX && point->x <= (_videoPosX + _videoDecoder->getWidth()) && point->y >= _videoPosY && point->y <= (_videoPosY + _videoDecoder->getHeight())) {
		uint16 targetX = point->x - _videoPosX;
		uint16 targetY = point->y - _videoPosY;
		AlgGraphics::drawImageCentered(_videoDecoder->getVideoFrame(), &_bulletholeIcon, targetX, targetY);
	}
}

void GameDrugWars::_EnableVideoFadeIn() {
	// TODO implement
}

uint16 GameDrugWars::_SceneToNumber(Scene *scene) {
	return atoi(scene->name.substr(5).c_str());
}

uint16 GameDrugWars::_RandomUnusedInt(uint8 max, uint16 *mask, uint16 exclude) {
	if (max == 1) {
		return 0;
	}
	// reset mask if full
	uint16 fullMask = 0xFFFF >> (16 - max);
	if (*mask == fullMask) {
		*mask = 0;
	}
	uint16 random;
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

uint16 GameDrugWars::_PickRandomScene(uint8 index, uint8 max) {
	if (_random_scenes[index] == nullptr) {
		error("_PickRandomScene called with illegal index: %d", index);
	}
	if (max != 0) {
		_random_max = max;
		_random_mask = 0;
		_random_picked = -1;
		_random_scene_count = 0;
		while (_random_scenes[index][_random_scene_count] != 0) {
			_random_scene_count++;
		}
	}
	unsigned short count = _random_max--;
	if (count > 0) {
		_random_picked = _RandomUnusedInt(_random_scene_count, &_random_mask, _random_picked);
		return _random_scenes[index][_random_picked];
	}
	return 0;
}

uint16 GameDrugWars::_PickDeathScene() {
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

void GameDrugWars::_scene_nxtscn_generic(uint8 index) {
	uint16 nextSceneId = 0;
	_got_to[index] = 0;
	if (_got_to[0] || _got_to[1] || _got_to[3] || _got_to[2]) {
		nextSceneId = 0x26;
	} else if (_got_to[4] || _got_to[5] || _got_to[6]) {
		if (_stage == 1) {
			nextSceneId = 0x52;
		} else {
			_stage = 1;
			nextSceneId = 0x50;
		}
	} else if (_got_to[7] || _got_to[8] || _got_to[9]) {
		if (_stage == 2) {
			nextSceneId = 0x9A;
		} else {
			_stage = 2;
			nextSceneId = 0x81;
		}
	} else if (_got_to[10] || _got_to[11] || _got_to[12]) {
		if (_stage == 3) {
			nextSceneId = 0xDF;
		} else {
			_stage = 3;
			nextSceneId = 0x14B;
		}
	} else if (_got_to[13]) {
		_stage = 4;
		nextSceneId = 0x18F;
	} else {
		nextSceneId = 0x21;
	}
	_cur_scene = Common::String::format("scene%d", nextSceneId);
}

void GameDrugWars::_rect_select_generic(uint8 index) {
	if (_got_to[index] > 0) {
		_cur_scene = Common::String::format("scene%d", _got_to[index]);
		_got_to_index = index;
	}
}

// Script functions: RectHit
void GameDrugWars::_rect_shotmenu(Rect *rect) {
	_DoMenu();
}

void GameDrugWars::_rect_save(Rect *rect) {
	if(_SaveState()) {
		_DoSaveSound();
	}
}

void GameDrugWars::_rect_load(Rect *rect) {
	if(_LoadState()) {
		_DoLoadSound();
	}
}

void GameDrugWars::_rect_continue(Rect *rect) {
	_inMenu = false;
	_fired = false;
	if (_lives <= 0) {
		_score = (int32)(_score * 0.7f);
		uint16 returnScene = _stage_start_scenes[_stage];
		_cur_scene = Common::String::format("scene%d", returnScene);
		_NewGame();
	}
}

void GameDrugWars::_rect_start(Rect *rect) {
	_inMenu = false;
	_fired = false;
	_gameInProgress = true;
	if (_isDemo) {
		_cur_scene = "scene54";
		_got_to_index = 1;
		_got_to[_got_to_index] = 54;
	} else {
		_cur_scene = "scene53";
	}
	_ResetParams();
	_NewGame();
}

void GameDrugWars::_rect_select_target_practice(Rect *rect) {
	_rect_select_generic(0);
	_got_to[0] = 0;
}

void GameDrugWars::_rect_select_bar(Rect *rect) {
	_got_to[0] = 0;
	_rect_select_generic(1);
}

void GameDrugWars::_rect_select_car_chase(Rect *rect) {
	_got_to[0] = 0;
	_rect_select_generic(2);
}

void GameDrugWars::_rect_select_drug_house(Rect *rect) {
	_got_to[0] = 0;
	_rect_select_generic(3);
}

void GameDrugWars::_rect_select_office(Rect *rect) {
	_rect_select_generic(4);
}

void GameDrugWars::_rect_select_court(Rect *rect) {
	_rect_select_generic(5);
}

void GameDrugWars::_rect_select_bus(Rect *rect) {
	_rect_select_generic(6);
}

void GameDrugWars::_rect_select_docks(Rect *rect) {
	_rect_select_generic(7);
}

void GameDrugWars::_rect_select_house_boat(Rect *rect) {
	_rect_select_generic(9);
}

void GameDrugWars::_rect_select_party(Rect *rect) {
	_rect_select_generic(8);
}

void GameDrugWars::_rect_select_airport(Rect *rect) {
	_rect_select_generic(10);
}

void GameDrugWars::_rect_select_mansion(Rect *rect) {
	_rect_select_generic(11);
}

void GameDrugWars::_rect_select_village(Rect *rect) {
	_rect_select_generic(12);
}

// Script functions: Scene PreOps
void GameDrugWars::_scene_pso_got_to(Scene *scene) {
	uint16 sceneId = _SceneToNumber(scene);
	_got_to[_got_to_index] = sceneId;
	if (_got_to_index == 13) {
		_final_stage_scene = _SceneToNumber(scene);
	}
	_EnableVideoFadeIn();
}

// Script functions: Scene NxtScn
void GameDrugWars::_scene_nxtscn_game_won(Scene *scene) {
	_gameInProgress = false;
	_cur_scene = _startscene;
}

void GameDrugWars::_scene_nxtscn_did_not_continue(Scene *scene) {
	_gameInProgress = false;
	_cur_scene = _startscene;
}

void GameDrugWars::_scene_nxtscn_lose_a_life(Scene *scene) {
	uint16 picked = 0;
	if (!_debug_godMode) {
		_lives--;
	}
	if (_isDemo) {
		_cur_scene = "scene83";
		return;
	} else if (_lives > 0) {
		_DisplayLivesLeft();
		picked = _PickDeathScene();
	} else {
		picked = _dead_scenes[_stage];
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameDrugWars::_scene_nxtscn_continue_game(Scene *scene) {
	if (_continues < 2) {
		_cur_scene = "scene438";
	} else {
		_scene_nxtscn_did_not_continue(scene);
	}
}

void GameDrugWars::_scene_nxtscn_kill_innocent_man(Scene *scene) {
	uint16 picked = 0;
	if (!_debug_godMode) {
		_lives--;
	}
	if (_isDemo) {
		_scene_nxtscn_after_die(scene);
		return;
	} else if (_lives > 0) {
		picked = _stage_start_scenes[_stage];
	} else {
		picked = _dead_scenes[_stage];
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameDrugWars::_scene_nxtscn_kill_innocent_woman(Scene *scene) {
	_scene_nxtscn_kill_innocent_man(scene);
}

void GameDrugWars::_scene_nxtscn_after_die(Scene *scene) {
	if (_isDemo) {
		if (_got_to[_got_to_index] > 54) {
			_cur_scene = "scene67";
		} else {
			_cur_scene = "scene54";
		}
	} else {
		uint16 picked = _stage_start_scenes[_stage];
		_cur_scene = Common::String::format("scene%d", picked);
	}
}

void GameDrugWars::_scene_nxtscn_init_random(Scene *scene) {
	int totalRandom = (_difficulty * 2) + _random_scenes_difficulty[_got_to_index] + 2;
	uint16 picked = _PickRandomScene(_got_to_index, totalRandom);
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameDrugWars::_scene_nxtscn_continue_random(Scene *scene) {
	uint16 picked = _PickRandomScene(_got_to_index, 0);
	if (picked == 0) {
		picked = _random_scenes_continue[_got_to_index];
		if (picked == 0) {
			error("_scene_nxtscn_continue_random called with illegal _got_to_index: %d", _got_to_index);
		}
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

void GameDrugWars::_scene_nxtscn_select_scenario(Scene *scene) {
	uint16 picked = 0;
	switch (_stage) {
	case 0:
		if (_got_to[0] > 0) {
			_rect_select_generic(0);
			_got_to[0] = 0;
		} else if (_got_to[1] > 0) {
			_rect_select_generic(1);
		} else if (_got_to[2] > 0) {
			_rect_select_generic(2);
		} else if (_got_to[3] > 0) {
			_rect_select_generic(3);
		} else {
			picked = 0x83;
			_stage = 1;
		}
		break;
	case 1:
		if (_got_to[4] > 0) {
			_rect_select_generic(4);
		} else if (_got_to[5] > 0) {
			_rect_select_generic(5);
		} else if (_got_to[6] > 0) {
			_rect_select_generic(6);
		} else {
			picked = 0xEE;
			_stage = 2;
		}
		break;
	case 2:
		if (_got_to[7] > 0) {
			_rect_select_generic(7);
		} else if (_got_to[8] > 0) {
			_rect_select_generic(8);
		} else if (_got_to[9] > 0) {
			_rect_select_generic(9);
		} else {
			picked = 0x0132;
			_stage = 3;
		}
		break;
	case 3:
		if (_got_to[10] > 0) {
			_rect_select_generic(10);
		} else if (_got_to[11] > 0) {
			_rect_select_generic(11);
		} else if (_got_to[12] > 0) {
			_rect_select_generic(12);
		} else {
			picked = _final_stage_scene;
			_got_to_index = 13;
			_stage = 4;
		}
		break;
	}
	if (picked != 0) {
		_cur_scene = Common::String::format("scene%d", picked);
	}
}

void GameDrugWars::_scene_nxtscn_finish_scenario(Scene *scene) {
	uint16 picked = 0;
	_got_to[_got_to_index] = 0;
	if (_isDemo) {
		_cur_scene = _startscene;
		return;
	}
	if (_got_to[0] || _got_to[1] || _got_to[3] || _got_to[2]) {
		picked = 0x51;
	} else if (_got_to[4] || _got_to[5] || _got_to[6]) {
		if (_stage == 1) {
			picked = 0x83;
		} else {
			_stage = 1;
			picked = 0x6B;
		}
	} else if (_got_to[7] || _got_to[8] || _got_to[9]) {
		if (_stage == 2) {
			picked = 0xEE;
		} else {
			_stage = 2;
			picked = 0xB6;
		}
	} else if (_got_to[10] || _got_to[11] || _got_to[12]) {
		if (_stage == 3) {
			picked = 0x0132;
		} else {
			_stage = 3;
			picked = 0x0109;
		}
	} else if (_got_to[13] != 0) {
		_stage = 13;
		_stage = 4;
		picked = 0x017F;
	} else {
		picked = 0x21;
	}
	_cur_scene = Common::String::format("scene%d", picked);
}

// Script functions: WepDwn
void GameDrugWars::_scene_default_wepdwn(Scene *scene) {
	_shots = 10;
}

// Debug methods
void GameDrugWars::debug_warpTo(int val) {
	// TODO implement
}

// Debugger methods
DebuggerDrugWars::DebuggerDrugWars(GameDrugWars *game) : GUI::Debugger() {
	_game = game;
	registerVar("drawRects", &game->_debug_drawRects);
	registerVar("godMode", &game->_debug_godMode);
	registerVar("unlimitedAmmo", &game->_debug_unlimitedAmmo);
	registerCmd("warpTo", WRAP_METHOD(DebuggerDrugWars, cmdWarpTo));
	registerCmd("dumpLib", WRAP_METHOD(DebuggerDrugWars, cmdDumpLib));
}

bool DebuggerDrugWars::cmdWarpTo(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: warp <int>");
		return true;
	} else {
		int val = atoi(argv[1]);
		_game->debug_warpTo(val);
		return false;
	}
}

bool DebuggerDrugWars::cmdDumpLib(int argc, const char **argv) {
	return _game->debug_dumpLibFile();
}

} // End of namespace Alg
