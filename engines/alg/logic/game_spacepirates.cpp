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
#include "alg/logic/game_spacepirates.h"
#include "alg/scene.h"

namespace Alg {

GameSpacePirates::GameSpacePirates(AlgEngine *vm, const AlgGameDescription *gd) : Game(vm) {
}

GameSpacePirates::~GameSpacePirates() {
	if (_shotIcon) {
		_shotIcon->free();
		delete _shotIcon;
	}
	if (_emptyIcon) {
		_emptyIcon->free();
		delete _emptyIcon;
	}
	if (_deadIcon) {
		_deadIcon->free();
		delete _deadIcon;
	}
	if (_liveIcon1) {
		_liveIcon1->free();
		delete _liveIcon1;
	}
	if (_liveIcon2) {
		_liveIcon2->free();
		delete _liveIcon2;
	}
	if (_liveIcon3) {
		_liveIcon3->free();
		delete _liveIcon3;
	}
	if (_bulletholeIcon) {
		_bulletholeIcon->free();
		delete _bulletholeIcon;
	}
}

void GameSpacePirates::init() {
	Game::init();

	_videoPosX = 11;
	_videoPosY = 2;

	_difficulty = 0;
	_lives = 3;
	_shots = 10;

	if (_vm->isDemo()) {
		loadLibArchive("sp.lib");
	} else if(_vm->useSingleSpeedVideos()) {
		loadLibArchive("spss.lib");
	} else {
		loadLibArchive("spds.lib");
	}

	_sceneInfo->loadScnFile(_vm->isDemo() ? "spacepir.scn" : "sp.scn");
	_startScene = _sceneInfo->getStartScene();

	registerScriptFunctions();
	verifyScriptFunctions();

	_menuzone = new Zone("MainMenu", "GLOBALHIT");
	_menuzone->addRect(0x0C, 0xAA, 0x38, 0xC7, nullptr, 0, "SHOTMENU", "0");

	_submenzone = new Zone("SubMenu", "GLOBALHIT");
	_submenzone->addRect(0x24, 0x16, 0x64, 0x26, nullptr, 0, "STARTMENU", "0");
	_submenzone->addRect(0x24, 0x36, 0x64, 0x46, nullptr, 0, "RECTLOAD", "0");
	_submenzone->addRect(0x24, 0x56, 0x64, 0x66, nullptr, 0, "RECTSAVE", "0");
	_submenzone->addRect(0x24, 0x76, 0x64, 0x86, nullptr, 0, "CONTMENU", "0");
	_submenzone->addRect(0x24, 0x96, 0x64, 0xA6, nullptr, 0, "EXITMENU", "0");
	_submenzone->addRect(0xD5, 0x3B, 0x0115, 0x4B, nullptr, 0, "RECTEASY", "0");
	_submenzone->addRect(0xD5, 0x63, 0x0115, 0x73, nullptr, 0, "RECTAVG", "0");
	_submenzone->addRect(0xD5, 0x90, 0x0115, 0xA0, nullptr, 0, "RECTHARD", "0");

	_shotSound = loadSoundFile("phaser.8b");
	_emptySound = loadSoundFile("emptygun.8b");
	_saveSound = loadSoundFile("saved.8b");
	_loadSound = loadSoundFile("loaded.8b");
	_skullSound = loadSoundFile("error.8b");
	_easySound = loadSoundFile("difflev.8b");
	_avgSound = loadSoundFile("difflev.8b");
	_hardSound = loadSoundFile("difflev.8b");

	_gun = AlgGraphics::loadScreenCoordAniImage("gun.ani", _palette);
	_difficultyIcon = (*_gun)[1];
	_numbers = AlgGraphics::loadAniImage("numbers.ani", _palette);
	auto bullets = AlgGraphics::loadAniImage("bullets.ani", _palette);
	_shotIcon = (*bullets)[0];
	_emptyIcon = (*bullets)[1];
	auto lives = AlgGraphics::loadAniImage("lives.ani", _palette);
	_liveIcon1 = (*lives)[0];
	_liveIcon2 = (*lives)[1];
	_liveIcon3 = (*lives)[2];
	_deadIcon = (*lives)[3];
	auto hole = AlgGraphics::loadScreenCoordAniImage("hole.ani", _palette);
	_bulletholeIcon = (*hole)[0];

	_background = AlgGraphics::loadVgaBackground("backgrnd.vga", _palette);
	_screen->copyRectToSurface(_background->getPixels(), _background->pitch, 0, 0, _background->w, _background->h);

	moveMouse();
}

void GameSpacePirates::registerScriptFunctions() {
#define RECT_HIT_FUNCTION(name, func) _rectHitFuncs[name] = new SPScriptFunctionRect(this, &GameSpacePirates::func);
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
	RECT_HIT_FUNCTION("KILL_INNOCENT_PERSON", rectKillInnocentPerson);
	RECT_HIT_FUNCTION("CONTINUE_JUNK_RINGS", rectContinueJunkRings);
	RECT_HIT_FUNCTION("SHOT_GRIN_REAPER", rectShotGrinReaper);
	RECT_HIT_FUNCTION("SHOW_MAD_DOG", rectShowMadDog);
	RECT_HIT_FUNCTION("POTT_WORLD_SHOW_CRYSTAL", rectPottWorldShowCrystal);
	RECT_HIT_FUNCTION("SHOT_LEFT", rectShotLeft);
	RECT_HIT_FUNCTION("SHOT_RIGHT", rectShotRight);
	RECT_HIT_FUNCTION("SHOT_GOLD", rectShotGold);
	RECT_HIT_FUNCTION("SHOT_SILVER", rectShotSilver);
	RECT_HIT_FUNCTION("SELECTED_DUNE_WORLD", rectSelectedDuneWorld);
	RECT_HIT_FUNCTION("SELECTED_JUNK_WORLD", rectSelectedJunkWorld);
	RECT_HIT_FUNCTION("SELECTED_DRAGONS_TEETH_WORLD", rectSelectedDragonsTeethWorld);
	RECT_HIT_FUNCTION("SELECTED_VOLCANO_WORLD", rectSelectedVolcanoWorld);
	RECT_HIT_FUNCTION("SHOT_RED_DEATH_GRIP", rectShotRedDeathGrip);
	RECT_HIT_FUNCTION("SHOT_BLUE_DEATH_GRIP", rectShotBlueDeathGrip);
	RECT_HIT_FUNCTION("SHOT_GREEN_DEATH_GRIP", rectShotGreenDeathGrip);
	RECT_HIT_FUNCTION("SHOT_YELLOW", rectShotYellow);
	RECT_HIT_FUNCTION("SHOT_BLUE", rectShotBlue);
	RECT_HIT_FUNCTION("SHOT_RED_CRYSTAL", rectShotRedCrystal);
	RECT_HIT_FUNCTION("SHOT_BLUE_CRYSTAL", rectShotBlueCrystal);
	RECT_HIT_FUNCTION("SHOT_GREEN_CRYSTAL", rectShotGreenCrystal);
	RECT_HIT_FUNCTION("SHOT_BLACK_DRAGON_1", rectShotBlackDragon1);
	RECT_HIT_FUNCTION("SHOT_BLACK_DRAGON_2", rectShotBlackDragon2);
	RECT_HIT_FUNCTION("SHOT_BLACK_DRAGON_3", rectShotBlackDragon3);
	RECT_HIT_FUNCTION("DO_FLYING_SKULL", rectDoFlyingSkull);
	RECT_HIT_FUNCTION("SKIP_SCENE", rectSkipScene);
	RECT_HIT_FUNCTION("HIT_PIRATE_SHIP", rectHitPirateShip);
#undef RECT_HIT_FUNCTION

#define PRE_OPS_FUNCTION(name, func) _scenePreOps[name] = new SPScriptFunctionScene(this, &GameSpacePirates::func);
	PRE_OPS_FUNCTION("DEFAULT", scenePsoDrawRct);
	PRE_OPS_FUNCTION("PAUSE", scenePsoPause);
	PRE_OPS_FUNCTION("PAUSE_THEN_FADEIN", scenePsoPauseFadeIn);
	PRE_OPS_FUNCTION("FADEIN_VIDEO", scenePsoFadeInVideo);
	PRE_OPS_FUNCTION("FADEIN_SCREEN", scenePsoFadeIn);
	PRE_OPS_FUNCTION("SET_GOT_TO", scenePsoSetGotTo);
	PRE_OPS_FUNCTION("SET_GOT_TO_NO_FADEIN", scenePsoSetGotToNoFadeIn);
	PRE_OPS_FUNCTION("SET_WORLD_GOT_TO", scenePsoSetWorldGotTo);

#undef PRE_OPS_FUNCTION

#define INS_OPS_FUNCTION(name, func) _sceneInsOps[name] = new SPScriptFunctionScene(this, &GameSpacePirates::func);
	INS_OPS_FUNCTION("DEFAULT", sceneIsoDoNothing);
	INS_OPS_FUNCTION("PAUSE", sceneIsoPause);
	INS_OPS_FUNCTION("PICK_A_WORLD", sceneIsoPickAWorld);
	INS_OPS_FUNCTION("SET_WORLD_GOT_TO", sceneIsoSetWorldGotTo);
#undef INS_OPS_FUNCTION

#define NXT_SCN_FUNCTION(name, func) _sceneNxtScn[name] = new SPScriptFunctionScene(this, &GameSpacePirates::func);
	NXT_SCN_FUNCTION("DEFAULT", sceneDefaultNxtscn);
	NXT_SCN_FUNCTION("GET_CHEWED_OUT", sceneNxtscnGotChewedOut);
	NXT_SCN_FUNCTION("RESTART_FROM_LAST", sceneNxtscnRestartFromLast);
	NXT_SCN_FUNCTION("PLAYER_DIED", sceneNxtscnPlayerDied);
	NXT_SCN_FUNCTION("MISC_ROOMS_1", sceneNxtscnMiscRooms1);
	NXT_SCN_FUNCTION("PICK_DUNGEON_CLUE", sceneNxtscnPickDungeonClue);
	NXT_SCN_FUNCTION("CONTINUE_DUNGEON_CLUE", sceneNxtscnContinueDungeonClue);
	NXT_SCN_FUNCTION("START_MIDSHIP_RANDOM_SCENE", sceneNxtscnStartMidshipRandomScene);
	NXT_SCN_FUNCTION("CONTINUE_MIDSHIP_RANDOM_SCENE", sceneNxtscnContinueMidshipRandomScene);
	NXT_SCN_FUNCTION("SHOW_DEATH_GRIP_BEAM_COLOR", sceneNxtscnShowDeathGripBeamColor);
	NXT_SCN_FUNCTION("SELECT_ASTEROIDS", sceneNxtscnSelectAsteroids);
	NXT_SCN_FUNCTION("ASTEROIDS_DONE", sceneNxtscnAsteroidsDone);
	NXT_SCN_FUNCTION("DO_FLYING_SKULLS", sceneNxtscnDoFlyingSkulls);
	NXT_SCN_FUNCTION("DID_FLYING_SKULLS", sceneNxtscnDidFlyingSkulls);
	NXT_SCN_FUNCTION("SHOW_WHICH_STAR_SPLITTER", sceneNxtscnShowWhichStartSplitter);
	NXT_SCN_FUNCTION("GOTO_SELECTED_WORLD", sceneNxtscnGotoSelectedWorld);
	NXT_SCN_FUNCTION("START_VOLCANO_POPUP", sceneNxtscnStartVolcanoPopup);
	NXT_SCN_FUNCTION("CONTINUE_VOLCANO_POPUP", sceneNxtscnContinueVolcanoPopup);
	NXT_SCN_FUNCTION("GIVE_FALINA_CLUE", sceneNxtscnGiveFalinaClue);
	NXT_SCN_FUNCTION("CHECK_FALINAS_CLUES", sceneNxtscnCheckFalinaClues);
	NXT_SCN_FUNCTION("SETUP_FALINA_TARGET_PRACTICE", sceneNxtscnSetupFalinaTargetPractice);
	NXT_SCN_FUNCTION("CONTINUE_FALINA_TARGET_PRACTICE", sceneNxtscnContinueFalinaTargetPractice);
	NXT_SCN_FUNCTION("START_DUNE_POPUP", sceneNxtscnStartDunePopup);
	NXT_SCN_FUNCTION("CONTINUE_DUNE_POPUP", sceneNxtscnContinueDunePopup);
	NXT_SCN_FUNCTION("POTT_OR_PAN_SHOOTS", sceneNxtscnPottOrPanShoots);
	NXT_SCN_FUNCTION("SETUP_POTT_TARGET_PRACTICE", sceneNxtscnSetupPottTargetPractice);
	NXT_SCN_FUNCTION("CONTINUE_POTT_TARGET_PRACTICE", sceneNxtscnContinuePottTargetPractice);
	NXT_SCN_FUNCTION("START_DRAGONS_TEETH_POPUP", sceneNxtscnStartDragonsTeethPopup);
	NXT_SCN_FUNCTION("CONTINUE_DRAGONS_TEETH_POPUP", sceneNxtscnContinueDragonsTeethPopup);
	NXT_SCN_FUNCTION("GRIN_REAPER_CLUE", sceneNxtscnGrinReaperClue);
	NXT_SCN_FUNCTION("START_GRIN_REAPER", sceneNxtscnStartGrinReaper);
	NXT_SCN_FUNCTION("CONTINUE_GRIN_REAPER", sceneNxtscnContinueGrinReaper);
	NXT_SCN_FUNCTION("GRIN_TARGET_PRACTICE", sceneNxtscnGrinTargetPractice);
	NXT_SCN_FUNCTION("CONTINUE_GRIN_TARGET_PRACTICE", sceneNxtscnContinueGrinTargetPractice);
	NXT_SCN_FUNCTION("START_JUNK_WORLD", sceneNxtscnStartJunkWorld);
	NXT_SCN_FUNCTION("CONTINUE_JUNK_WORLD", sceneNxtscnContinueJunkWorld);
	NXT_SCN_FUNCTION("START_JUNK_RINGS", sceneNxtscnStartJunkRings);
	NXT_SCN_FUNCTION("SHOW_JUNK_WORLD_CRYSTAL", sceneNxtscnShowJunkWorldCrystal);
	NXT_SCN_FUNCTION("START_JUNK_WORLD_TARGET_PRACTICE", sceneNxtscnStartJunkWorldTargetPractice);
	NXT_SCN_FUNCTION("CONTINUE_JUNK_WORLD_TARGET_PRACTICE", sceneNxtscnContinueJunkWorldTargetPractice);
	NXT_SCN_FUNCTION("ARE_ALL_WORLDS_DONE", sceneNxtscnAreAllWorldsDone);
	NXT_SCN_FUNCTION("START_PRACTICE_PIRATE_SHIP", sceneNxtscnStartPracticePirateShip);
	NXT_SCN_FUNCTION("MORE_PRACTICE_PIRATE_SHIP", sceneNxtscnMorePracticePirateShip);
	NXT_SCN_FUNCTION("PLAYER_WON", sceneNxtscnPlayerWon);
#undef NXT_SCN_FUNCTION

#define MISSEDRECTS_FUNCTION(name, func) _sceneMissedRects[name] = new SPScriptFunctionScene(this, &GameSpacePirates::func);
	MISSEDRECTS_FUNCTION("DEFAULT", sceneMissedRectsDefault);
	MISSEDRECTS_FUNCTION("MISSED_PIRATE_SHIP", sceneMissedRectsMissedPirateShip);
#undef MISSEDRECTS_FUNCTION

	_sceneShowMsg["DEFAULT"] = new SPScriptFunctionScene(this, &GameSpacePirates::sceneSmDonothing);
	_sceneWepDwn["DEFAULT"] = new SPScriptFunctionScene(this, &GameSpacePirates::sceneDefaultWepdwn);
	_sceneScnScr["DEFAULT"] = new SPScriptFunctionScene(this, &GameSpacePirates::sceneDefaultScore);
	_sceneNxtFrm["DEFAULT"] = new SPScriptFunctionScene(this, &GameSpacePirates::sceneNxtfrm);
}

void GameSpacePirates::verifyScriptFunctions() {
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

SPScriptFunctionRect GameSpacePirates::getScriptFunctionRectHit(Common::String name) {
	auto it = _rectHitFuncs.find(name);
	if (it != _rectHitFuncs.end()) {
		return *it->_value;
	} else {
		error("GameSpacePirates::getScriptFunctionRectHit(): Could not find rectHit function: %s", name.c_str());
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
		error("GameSpacePirates::getScriptFunctionScene(): Unkown scene script type: %u", type);
		break;
	}
	SPScriptFunctionSceneMap::iterator it;
	it = functionMap->find(name);
	if (it != functionMap->end()) {
		return *it->_value;
	} else {
		error("GameSpacePirates::getScriptFunctionScene(): Could not find scene type %u function: %s", type, name.c_str());
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
	newGame();
	_curScene = _startScene;
	Common::String oldscene;
	while (!_vm->shouldQuit()) {
		oldscene = _curScene;
		_fired = false;
		Scene *scene = _sceneInfo->findScene(_curScene);
		if (!loadScene(scene)) {
			error("GameSpacePirates::run(): Cannot find scene %s in libfile", scene->_name.c_str());
		}
		_nextSceneFound = false;
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
							callScriptFunctionScene(MISSEDRECTS, scene->_missedRects, scene);
						}
					} else {
						playSound(_emptySound);
					}
				}
			}
			if (_nextSceneFound && _curScene == oldscene) {
				callScriptFunctionScene(NXTSCN, scene->_nxtscn, scene);
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

void GameSpacePirates::newGame() {
	_holster = false;
	_shots = 10;
}

void GameSpacePirates::resetParams() {
	if (_gameLoaded) {
		_oldDifficulty = _difficulty;
		_lives = _livesLoaded;
		_shots = _shotsLoaded;
		_score = _scoreLoaded;
		_difficulty = _difficultyLoaded;
		changeDifficulty(_oldDifficulty);
	} else {
		changeDifficulty(0);
		_randomCountAsteroids = 0;
		_sceneBeforeFlyingSkulls = 0;
		_miscRoomsCount = 0;
		_playerDied = false;
		_gotTo = 0;
		_selectedAWorld = false;
		_selectedWorldStart = 0;
		_currentWorld = -1;
		_worldGotTo[0] = 0x91;
		_worldGotTo[1] = 0x0103;
		_worldGotTo[2] = 0xCE;
		_worldGotTo[3] = 0x72;
		_worldDone[0] = false;
		_worldDone[1] = false;
		_worldDone[2] = false;
		_worldDone[3] = false;
		_crystalState = 0;
		_crystalsShot = 1;
		_lastExtraLifeScore = 0;
	}
}

void GameSpacePirates::doMenu() {
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

void GameSpacePirates::changeDifficulty(uint8 newDifficulty) {
	if (newDifficulty == _oldDifficulty) {
		return;
	}
	showDifficulty(newDifficulty, true);
	_oldDifficulty = newDifficulty;
	_difficulty = newDifficulty;
}

void GameSpacePirates::showDifficulty(uint8 newDifficulty, bool cursor) {
	// reset menu screen
	_screen->copyRectToSurface(_background->getBasePtr(_videoPosX, _videoPosY), _background->pitch, _videoPosX, _videoPosY, _videoDecoder->getWidth(), _videoDecoder->getHeight());
	uint16 posY = 0x31;
	if (newDifficulty == 1) {
		posY = 0x5B;
	} else if (newDifficulty == 2) {
		posY = 0x86;
	}
	AlgGraphics::drawImageCentered(_screen, _difficultyIcon, 0x0111, posY);
	if (cursor) {
		updateCursor();
	}
}

void GameSpacePirates::updateCursor() {
	updateMouse();
}

void GameSpacePirates::updateMouse() {
	if (_oldWhichGun != _whichGun) {
		Graphics::Surface *cursor = (*_gun)[_whichGun];
		uint16 hotspotX = (cursor->w / 2) + 8;
		uint16 hotspotY = (cursor->h / 2) + 10;
		if (_whichGun == 2) {
			hotspotY += 14;
		}
		if (debugChannelSet(1, Alg::kAlgDebugGraphics)) {
			cursor->drawLine(0, hotspotY, cursor->w, hotspotY, 1);
			cursor->drawLine(hotspotX, 0, hotspotX, cursor->h, 1);
		}
		CursorMan.replaceCursor(cursor->getPixels(), cursor->w, cursor->h, hotspotX, hotspotY, 0);
		CursorMan.showMouse(true);
		_oldWhichGun = _whichGun;
	}
}

void GameSpacePirates::moveMouse() {
	if (_inMenu) {
		_whichGun = 2; // in menu cursor
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

void GameSpacePirates::displayLivesLeft() {
	if (_lives == _oldLives) {
		return;
	}

	int16 posY = 0x73;
	int16 posX = 0x0130;
	int16 margin = 14;
	if (_vm->isDemo()) {
		posY = 0x68;
		posX = 0x012F;
		margin = 13;
	}
	for (int i = 0; i < 3; i++) {
		AlgGraphics::drawImage(_screen, _deadIcon, posX, posY + (i * margin));
	}
	if (_lives > 2) {
		AlgGraphics::drawImage(_screen, _liveIcon3, posX, posY + (margin * 2));
	}
	if (_lives > 1) {
		AlgGraphics::drawImage(_screen, _liveIcon2, posX, posY + margin);
	}
	if (_lives > 0) {
		AlgGraphics::drawImage(_screen, _liveIcon1, posX, posY);
	}
	_oldLives = _lives;
}

void GameSpacePirates::displayScores() {
	if (_score == _oldScore) {
		return;
	}
	Common::String scoreString = Common::String::format("%05d", _score);
	int posX = 0x71;
	for (int i = 0; i < 5; i++) {
		uint8 digit = scoreString[i] - '0';
		AlgGraphics::drawImage(_screen, (*_numbers)[digit], posX, 0xBF);
		posX += 7;
	}
	_oldScore = _score;
}

void GameSpacePirates::displayShotsLeft() {
	if (_shots == _oldShots) {
		return;
	}
	uint16 posX = 0xAA;
	for (int i = 0; i < 10; i++) {
		AlgGraphics::drawImage(_screen, _emptyIcon, posX, 0xBF);
		posX += 8;
	}
	posX = 0xAA;
	for (int i = 0; i < _shots; i++) {
		AlgGraphics::drawImage(_screen, _shotIcon, posX, 0xBF);
		posX += 8;
	}
	_oldShots = _shots;
}

bool GameSpacePirates::weaponDown() {
	if (_rightDown && _mousePos.y >= 0xAA && _mousePos.x >= 0x113) {
		return true;
	}
	return false;
}

bool GameSpacePirates::saveState() {
	Scene *scene = _sceneInfo->findScene(_curScene);
	uint16 sceneNum = sceneToNumber(scene);
	if ((sceneNum < 0xAC || sceneNum > 0xB9) && sceneNum != 0x6F) {
		Common::OutSaveFile *outSaveFile;
		Common::String saveFileName = _vm->getSaveStateName(0);
		if (!(outSaveFile = g_system->getSavefileManager()->openForSaving(saveFileName))) {
			warning("GameSpacePirates::saveState(): Can't create file '%s', game not saved", saveFileName.c_str());
			return false;
		}
		outSaveFile->writeUint32BE(MKTAG('A', 'L', 'G', 'S')); // header
		outSaveFile->writeByte(0);                             // version, unused for now
		outSaveFile->writeSByte(_lives);
		outSaveFile->writeUint16LE(_shots);
		outSaveFile->writeUint32LE(_score);
		outSaveFile->writeByte(_difficulty);
		outSaveFile->writeUint16LE(sceneNum);
		for (int i = 0; i < 9; i++) {
			outSaveFile->writeUint16LE(_randomScenesValues[i]);
			outSaveFile->writeByte(_randomScenesUsed[i]);
		}
		for (int i = 0; i < 4; i++) {
			outSaveFile->writeByte(_worldDone[i]);
			outSaveFile->writeUint16LE(_worldGotTo[i]);
		}
		outSaveFile->writeByte(_maxRandom);
		outSaveFile->writeByte(_randomCountAsteroids);
		outSaveFile->writeUint16LE(_sceneBeforeFlyingSkulls);
		outSaveFile->writeByte(_miscRoomsCount);
		outSaveFile->writeByte(_randomCountMidship);
		outSaveFile->writeByte(_randomCount);
		outSaveFile->writeUint16LE(_gotTo);
		outSaveFile->writeByte(_playerDied);
		outSaveFile->writeUint16LE(_clue);
		outSaveFile->writeByte(_shotColor);
		outSaveFile->writeByte(_shotDirection);
		outSaveFile->writeUint16LE(_pickedStartSplitter);
		outSaveFile->writeSByte(_currentWorld);
		outSaveFile->writeByte(_selectedAWorld);
		outSaveFile->writeUint16LE(_selectedWorldStart);
		outSaveFile->writeByte(_crystalState);
		outSaveFile->writeByte(_shotGrinReaperCount);
		outSaveFile->writeByte(_crystalsShot);
		outSaveFile->writeUint32LE(_lastExtraLifeScore);
		outSaveFile->finalize();
		delete outSaveFile;
		return true;
	}
	return false;
}

bool GameSpacePirates::loadState() {
	Common::InSaveFile *inSaveFile;
	Common::String saveFileName = _vm->getSaveStateName(0);
	if (!(inSaveFile = g_system->getSavefileManager()->openForLoading(saveFileName))) {
		debug("GameSpacePirates::loadState(): Can't load file '%s', game not loaded", saveFileName.c_str());
		return false;
	}
	uint32 header = inSaveFile->readUint32BE();
	if (header != MKTAG('A', 'L', 'G', 'S')) {
		warning("GameSpacePirates::loadState(): Unkown save file, header: %s", tag2str(header));
		return false;
	}
	inSaveFile->skip(1); // version, unused for now
	_lives = inSaveFile->readSByte();
	_shots = inSaveFile->readUint16LE();
	_score = inSaveFile->readUint32LE();
	_difficulty = inSaveFile->readByte();
	uint16 sceneNum = inSaveFile->readUint16LE();
	for (int i = 0; i < 9; i++) {
		_randomScenesValues[i] = inSaveFile->readUint16LE();
		_randomScenesUsed[i] = inSaveFile->readByte();
	}
	for (int i = 0; i < 4; i++) {
		_worldDone[i] = inSaveFile->readByte();
		_worldGotTo[i] = inSaveFile->readUint16LE();
	}
	_maxRandom = inSaveFile->readByte();
	_randomCountAsteroids = inSaveFile->readByte();
	_sceneBeforeFlyingSkulls = inSaveFile->readUint16LE();
	_miscRoomsCount = inSaveFile->readByte();
	_randomCountMidship = inSaveFile->readByte();
	_randomCount = inSaveFile->readByte();
	_gotTo = inSaveFile->readUint16LE();
	_playerDied = inSaveFile->readByte();
	_clue = inSaveFile->readUint16LE();
	_shotColor = inSaveFile->readByte();
	_shotDirection = inSaveFile->readByte();
	_pickedStartSplitter = inSaveFile->readUint16LE();
	_currentWorld = inSaveFile->readSByte();
	_selectedAWorld = inSaveFile->readByte();
	_selectedWorldStart = inSaveFile->readUint16LE();
	_crystalState = inSaveFile->readByte();
	_shotGrinReaperCount = inSaveFile->readByte();
	_crystalsShot = inSaveFile->readByte();
	_lastExtraLifeScore = inSaveFile->readUint32LE();
	delete inSaveFile;
	_gameLoaded = true;
	_gameInProgress = true;
	_livesLoaded = _lives;
	_shotsLoaded = _shots;
	_scoreLoaded = _score;
	_difficultyLoaded = _difficulty;
	_curScene = Common::String::format("scene%d", sceneNum);
	changeDifficulty(_difficulty);
	if (!(_worldDone[0] && _worldDone[1] && _worldDone[2] && _worldDone[3]) && _currentWorld != -1) {
		_curScene = "scene110";
	}
	return true;
}

// misc game functions
void GameSpacePirates::playErrorSound() {
	playSound(_skullSound);
}

void GameSpacePirates::displayShotFiredImage() {
	displayShotFiredImage(&_mousePos);
}

void GameSpacePirates::displayShotFiredImage(Common::Point *point) {
	if (point->x >= _videoPosX && point->x <= (_videoPosX + _videoDecoder->getWidth()) && point->y >= _videoPosY && point->y <= (_videoPosY + _videoDecoder->getHeight())) {
		uint16 targetX = point->x - _videoPosX - 4;
		uint16 targetY = point->y - _videoPosY - 4;
		AlgGraphics::drawImageCentered(_videoDecoder->getVideoFrame(), _bulletholeIcon, targetX, targetY);
	}
}

void GameSpacePirates::displayShotLine(uint16 startX, uint16 startY, uint16 endX, uint16 endY) {
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
			displayShotFiredImage(&position);
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

void GameSpacePirates::displayMultipleShotLines() {
	displayShotLine(0x73, 0x37, _mousePos.x, _mousePos.y);
	displayShotLine(0x20, 0x9B, _mousePos.x, _mousePos.y);
	displayShotLine(0x73, 0xFB, _mousePos.x, _mousePos.y);
}

void GameSpacePirates::enableVideoFadeIn() {
	// TODO implement
}

uint16 GameSpacePirates::sceneToNumber(Scene *scene) {
	return atoi(scene->_name.substr(5).c_str());
}

uint16 GameSpacePirates::randomUnusedScene(uint8 max) {
	bool found = false;
	uint8 randomNum = 0;
	for (int i = 0; i < max && !found; i++) {
		randomNum = _rnd->getRandomNumber(max - 1);
		if (_randomScenesUsed[randomNum] == 0) {
			found = true;
		}
	}
	if (!found) {
		for (int i = 0; i < max && !found; i++) {
			if (_randomScenesUsed[i] == 0) {
				found = true;
				randomNum = i;
				break;
			}
		}
	}
	_randomScenesUsed[randomNum] = 1;
	return _randomScenesValues[randomNum];
}

uint16 GameSpacePirates::randomNumberInRange(uint16 min, uint16 max) {
	uint16 randomNum = 0;
	do {
		randomNum = _rnd->getRandomNumberRng(min, max);
	} while (randomNum == _randomPicked);
	_randomPicked = randomNum;
	return randomNum;
}

uint16 GameSpacePirates::pickCrystalScene(uint16 scene1, uint16 scene2, uint16 scene3) {
	uint16 picked = 0;
	if (_crystalState == 7) {
		picked = 110;
	} else {
		if ((_crystalState & 1) == 0) {
			picked = scene1;
			_crystalState |= 1;
		} else if ((_crystalState & 2) == 0) {
			picked = scene2;
			_crystalState |= 2;
		} else if ((_crystalState & 4) == 0) {
			picked = scene3;
			_crystalState |= 4;
		}
	}
	return picked;
}

// Script functions: RectHit
void GameSpacePirates::rectShotMenu(Rect *rect) {
	doMenu();
}

void GameSpacePirates::rectSave(Rect *rect) {
	if (saveState()) {
		doSaveSound();
	}
}

void GameSpacePirates::rectLoad(Rect *rect) {
	if (loadState()) {
		doLoadSound();
	}
}

void GameSpacePirates::rectContinue(Rect *rect) {
	_inMenu = false;
	_fired = false;
}

void GameSpacePirates::rectStart(Rect *rect) {
	changeDifficulty(0);
	_gameLoaded = false;
	_inMenu = false;
	_fired = false;
	_gameInProgress = true;
	_curScene = _vm->isDemo() ? "scene62" : "scene187";
	resetParams();
	newGame();
}

void GameSpacePirates::rectEasy(Rect *rect) {
	doDiffSound(1);
	_difficulty = 0;
}

void GameSpacePirates::rectAverage(Rect *rect) {
	doDiffSound(2);
	_difficulty = 1;
}

void GameSpacePirates::rectHard(Rect *rect) {
	doDiffSound(3);
	_difficulty = 2;
}

void GameSpacePirates::rectDefault(Rect *rect) {
	displayShotFiredImage();
	doShot();
	_score += rect->_score;
	if (_score - _lastExtraLifeScore >= 1500 && _lives < 3) {
		_lives++;
		_lastExtraLifeScore = _score;
	}
	if (!rect->_scene.empty()) {
		_nextSceneFound = true;
		_curScene = rect->_scene;
	}
}

void GameSpacePirates::rectKillInnocentPerson(Rect *rect) {
	displayShotFiredImage();
	doShot();
	if (!_debug_godMode) {
		_lives--;
	}
	_nextSceneFound = true;
	_playerDied = true;
	if (_vm->isDemo()) {
		_curScene = "scene185";
		return;
	}
	Scene *scene = _sceneInfo->findScene(rect->_scene);
	uint16 picked = sceneToNumber(scene);
	if (picked == 0) {
		picked = randomNumberInRange(0xB7, 0xB9);
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::rectContinueJunkRings(Rect *rect) {
	displayShotFiredImage();
	doShot();
	_randomCount++;
	uint16 picked = 0;
	if (_randomCount >= 10) {
		if (_crystalState == 7) {
			picked = 0x124;
		} else {
			picked = 0x119;
		}
	} else {
		picked = randomNumberInRange(0x011E, 0x0122);
	}
	_nextSceneFound = true;
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::rectShotGrinReaper(Rect *rect) {
	displayShotFiredImage();
	doShot();
	_shotGrinReaperCount++;
	uint16 picked = 0;
	if (_clue - 223 <= _shotGrinReaperCount) {
		_nextSceneFound = true;
		if (_crystalState == 7) {
			picked = 0xF9;
		} else {
			picked = pickCrystalScene(0xF6, 0xF7, 0xF8);
		}
	} else {
		picked = 0;
	}
	if (picked != 0) {
		_curScene = Common::String::format("scene%d", picked);
	}
}

void GameSpacePirates::rectShowMadDog(Rect *rect) {
	displayShotFiredImage();
	doShot();
	_nextSceneFound = true;
	_curScene = "scene354";
}

void GameSpacePirates::rectPottWorldShowCrystal(Rect *rect) {
	displayShotFiredImage();
	doShot();
	uint16 picked = 0;
	if (_crystalState == 7) {
		picked = 0xA6;
	} else {
		picked = pickCrystalScene(0xA3, 0xA4, 0xA5);
	}
	_nextSceneFound = true;
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::rectShotLeft(Rect *rect) {
	displayShotFiredImage();
	doShot();
	_shotDirection = 1;
	_nextSceneFound = true;
}

void GameSpacePirates::rectShotRight(Rect *rect) {
	displayShotFiredImage();
	doShot();
	_shotDirection = 2;
	_nextSceneFound = true;
}

void GameSpacePirates::rectShotGold(Rect *rect) {
	displayShotFiredImage();
	doShot();
	_shotColor = 3;
	_nextSceneFound = true;
	_curScene = rect->_scene;
}

void GameSpacePirates::rectShotSilver(Rect *rect) {
	displayShotFiredImage();
	doShot();
	_shotColor = 4;
	_nextSceneFound = true;
	_curScene = rect->_scene;
}

void GameSpacePirates::rectSelectedDuneWorld(Rect *rect) {
	displayShotFiredImage();
	doShot();
	if (!_worldDone[0]) {
		_selectedAWorld = true;
		_currentWorld = 0;
		_selectedWorldStart = _worldGotTo[0];
		_curScene = "scene111";
	} else {
		playErrorSound();
	}
	_shots++;
}

void GameSpacePirates::rectSelectedJunkWorld(Rect *rect) {
	displayShotFiredImage();
	doShot();
	if (!_worldDone[1]) {
		_selectedAWorld = true;
		_currentWorld = 1;
		_selectedWorldStart = _worldGotTo[1];
		_curScene = "scene111";
	} else {
		playErrorSound();
	}
	_shots++;
}

void GameSpacePirates::rectSelectedDragonsTeethWorld(Rect *rect) {
	displayShotFiredImage();
	doShot();
	if (!_worldDone[2]) {
		_selectedAWorld = true;
		_currentWorld = 2;
		_selectedWorldStart = _worldGotTo[2];
		_curScene = "scene111";
	} else {
		playErrorSound();
	}
	_shots++;
}

void GameSpacePirates::rectSelectedVolcanoWorld(Rect *rect) {
	displayShotFiredImage();
	doShot();
	if (!_worldDone[3]) {
		_selectedAWorld = true;
		_currentWorld = 3;
		_selectedWorldStart = _worldGotTo[3];
		_curScene = "scene111";
	} else {
		playErrorSound();
	}
	_shots++;
}

void GameSpacePirates::rectShotRedDeathGrip(Rect *rect) {
	uint16 picked = 0;
	_nextSceneFound = true;
	if (_clue == 0x36) {
		displayShotFiredImage();
		doShot();
		picked = 0x5A;
	} else {
		rectKillInnocentPerson(rect);
		picked = randomNumberInRange(0xB7, 0xB9);
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::rectShotBlueDeathGrip(Rect *rect) {
	uint16 picked = 0;
	_nextSceneFound = true;
	if (_clue == 0x38) {
		displayShotFiredImage();
		doShot();
		picked = 0x5C;
	} else {
		rectKillInnocentPerson(rect);
		picked = randomNumberInRange(0xB7, 0xB9);
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::rectShotGreenDeathGrip(Rect *rect) {
	uint16 picked = 0;
	_nextSceneFound = true;
	if (_clue == 0x37) {
		displayShotFiredImage();
		doShot();
		picked = 0x5B;
	} else {
		rectKillInnocentPerson(rect);
		picked = randomNumberInRange(0xB7, 0xB9);
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::rectShotYellow(Rect *rect) {
	displayShotFiredImage();
	doShot();
	_shotColor = 0x0F;
	_nextSceneFound = true;
}

void GameSpacePirates::rectShotBlue(Rect *rect) {
	displayShotFiredImage();
	doShot();
	_shotColor = 0x0E;
	_nextSceneFound = true;
}

void GameSpacePirates::rectShotRedCrystal(Rect *rect) {
	uint16 picked = 0;
	displayShotFiredImage();
	doShot();
	Scene *scene = _sceneInfo->findScene(_curScene);
	if (_crystalsShot == 1) {
		if (_pickedStartSplitter == 0x6A) {
			picked = 0xC6;
			_crystalsShot++;
		} else {
			_curScene = scene->_next;
			return;
		}
	} else if (_crystalsShot == 2) {
		if (_pickedStartSplitter == 0x6C) {
			picked = 0xC5;
			_crystalsShot++;
		} else {
			_curScene = scene->_next;
			return;
		}
	} else if (_crystalsShot == 3) {
		if (_pickedStartSplitter == 0x6B) {
			picked = 0x14E;
		} else {
			_curScene = scene->_next;
			return;
		}
	}
	_nextSceneFound = true;
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::rectShotBlueCrystal(Rect *rect) {
	uint16 picked = 0;
	displayShotFiredImage();
	doShot();
	Scene *scene = _sceneInfo->findScene(_curScene);
	if (_crystalsShot == 1) {
		if (_pickedStartSplitter == 0x6C) {
			picked = 0xC4;
			_crystalsShot++;
		} else {
			_curScene = scene->_next;
			return;
		}
	} else if (_crystalsShot == 2) {
		if (_pickedStartSplitter == 0x6B) {
			picked = 0xC2;
			_crystalsShot++;
		} else {
			_curScene = scene->_next;
			return;
		}
	} else if (_crystalsShot == 3) {
		if (_pickedStartSplitter == 0x6A) {
			picked = 0x14E;
		} else {
			_curScene = scene->_next;
			return;
		}
	}
	_nextSceneFound = true;
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::rectShotGreenCrystal(Rect *rect) {
	uint16 picked = 0;
	displayShotFiredImage();
	doShot();
	Scene *scene = _sceneInfo->findScene(_curScene);
	if (_crystalsShot == 1) {
		if (_pickedStartSplitter == 0x6B) {
			picked = 0xC1;
			_crystalsShot++;
		} else {
			_curScene = scene->_next;
			return;
		}
	} else if (_crystalsShot == 2) {
		if (_pickedStartSplitter == 0x6A) {
			picked = 0xC7;
			_crystalsShot++;
		} else {
			_curScene = scene->_next;
			return;
		}
	} else if (_crystalsShot == 3) {
		if (_pickedStartSplitter == 0x6C) {
			picked = 0x14E;
		} else {
			_curScene = scene->_next;
			return;
		}
	}
	_nextSceneFound = true;
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::rectShotBlackDragon1(Rect *rect) {
	displayShotFiredImage();
	doShot();
	_score += rect->_score;
	_nextSceneFound = true;
	_curScene = "scene203";
}

void GameSpacePirates::rectShotBlackDragon2(Rect *rect) {
	displayShotFiredImage();
	doShot();
	_score += rect->_score;
	_nextSceneFound = true;
	_curScene = "scene204";
}

void GameSpacePirates::rectShotBlackDragon3(Rect *rect) {
	displayShotFiredImage();
	doShot();
	_score += rect->_score;
	_nextSceneFound = true;
	_curScene = "scene335";
}

void GameSpacePirates::rectDoFlyingSkull(Rect *rect) {
	displayShotFiredImage();
	doShot();
	_nextSceneFound = true;
	Scene *scene = _sceneInfo->findScene(rect->_scene);
	_sceneBeforeFlyingSkulls = sceneToNumber(scene);
	uint16 picked = randomNumberInRange(0x014A, 0x014D);
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::rectSkipScene(Rect *rect) {
	displayShotFiredImage();
	doShot();
	_nextSceneFound = true;
	Scene *scene = _sceneInfo->findScene(_curScene);
	_curScene = scene->_next;
}

void GameSpacePirates::rectHitPirateShip(Rect *rect) {
	displayShotFiredImage();
	doShot();
	displayMultipleShotLines();
	_score += rect->_score;
	_nextSceneFound = true;
}

// Script functions: Scene PreOps
void GameSpacePirates::scenePsoFadeInVideo(Scene *scene) {
	// TODO implement
}

void GameSpacePirates::scenePsoSetGotTo(Scene *scene) {
	enableVideoFadeIn();
	_gotTo = sceneToNumber(scene);
}

void GameSpacePirates::scenePsoSetGotToNoFadeIn(Scene *scene) {
	_gotTo = sceneToNumber(scene);
}

void GameSpacePirates::scenePsoSetWorldGotTo(Scene *scene) {
	enableVideoFadeIn();
	uint16 sceneNum = sceneToNumber(scene);
	_worldGotTo[_currentWorld] = sceneNum;
}

// Script functions: Scene InsOps
void GameSpacePirates::sceneIsoPickAWorld(Scene *scene) {
	Zone *zone = scene->_zones[0];
	uint8 world = 3;
	for (auto &rect : zone->_rects) {
		if (_worldDone[world]) {
			uint16 centerX = rect->left + (rect->width() / 2);
			uint16 centerY = rect->top + (rect->height() / 2);
			AlgGraphics::drawImageCentered(_videoDecoder->getVideoFrame(), (*_gun)[2], centerX - 16, centerY - 24);
		}
		world--;
	}
	if (_worldDone[0] && _worldDone[1] && _worldDone[2] && _worldDone[3]) {
		_currentWorld = -1;
		_nextSceneFound = true;
	} else if (_selectedAWorld) {
		if (_worldDone[_currentWorld]) {
			_currentWorld = -1;
		}
	} else {
		_currentWorld = -1;
	}
}

void GameSpacePirates::sceneIsoSetWorldGotTo(Scene *scene) {
	uint16 sceneNum = sceneToNumber(scene);
	_worldGotTo[_currentWorld] = sceneNum;
}

// Script functions: Scene NxtScn
void GameSpacePirates::sceneNxtscnGotChewedOut(Scene *scene) {
	uint16 picked = randomNumberInRange(0xB7, 0xB9);
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::sceneNxtscnRestartFromLast(Scene *scene) {
	uint16 picked = 0;
	if (!_playerDied) {
		_curScene = scene->_next;
	} else {
		if (_lives > 0) {
			picked = _gotTo;
			_playerDied = false;
		} else {
			_nextSceneFound = true;
			_gameInProgress = false;
			_shots = 0;
			picked = 368;
		}
		_curScene = Common::String::format("scene%d", picked);
	}
}

void GameSpacePirates::sceneNxtscnPlayerDied(Scene *scene) {
	uint16 picked = 0;
	_playerDied = true;
	if (!_debug_godMode) {
		_lives--;
	}
	if (_lives > 0) {
		if (_vm->isDemo()) {
			picked = 178;
		} else {
			picked = randomNumberInRange(0xB2, 0xB4);
		}
	} else {
		if (_vm->isDemo()) {
			picked = 172;
		} else {
			uint8 randomNum = _rnd->getRandomNumber(9);
			if (randomNum < 5) {
				picked = randomNumberInRange(0xB5, 0xB6);
			} else {
				picked = randomNumberInRange(0xAC, 0xB1);
			}
		}
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::sceneNxtscnMiscRooms1(Scene *scene) {
	uint16 picked = 0;
	if (_miscRoomsCount == 0) {
		_pickedMiscRooms = 0;
		while (true) {
			uint8 randomNum = _rnd->getRandomNumber(9);
			if (randomNum <= 5) {
				picked = 0x1F;
			} else {
				picked = 0x21;
			}
			if (_pickedMiscRooms != picked) {
				_pickedMiscRooms = picked;
				break;
			}
		}
	} else {
		if (_miscRoomsCount < 2) {
			if (_pickedMiscRooms == 0x1F) {
				picked = 0x21;
			} else if (_pickedMiscRooms == 0x21) {
				picked = 0x1F;
			}
		} else {
			_curScene = scene->_next;
			return;
		}
	}
	_miscRoomsCount++;
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::sceneNxtscnPickDungeonClue(Scene *scene) {
	_clue = randomNumberInRange(14, 15);
	_shotColor = 0;
	_curScene = Common::String::format("scene%d", _clue);
}

void GameSpacePirates::sceneNxtscnContinueDungeonClue(Scene *scene) {
	uint16 picked = 0;
	if (_clue == _shotColor) {
		if (_shotColor == 14) {
			picked = 0x15;
		} else {
			picked = 0x16;
		}
	} else {
		sceneNxtscnPlayerDied(scene);
		picked = 0xAE;
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::sceneNxtscnStartMidshipRandomScene(Scene *scene) {
	_maxRandom = 5;
	_randomCountMidship = 0;
	_randomScenesValues[0] = 0x25;
	_randomScenesValues[1] = 0x27;
	_randomScenesValues[2] = 0x29;
	_randomScenesValues[3] = 0x2B;
	_randomScenesValues[4] = 0x2D;
	memset(&_randomScenesUsed, 0, (9 * sizeof(uint8)));
	uint16 picked = randomUnusedScene(5);
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::sceneNxtscnContinueMidshipRandomScene(Scene *scene) {
	uint16 picked = 0;
	_randomCountMidship++;
	if (((_difficulty * 5) + 5) > _randomCountMidship) {
		memset(&_randomScenesUsed, 0, (9 * sizeof(uint8)));
		picked = randomUnusedScene(_maxRandom);
		_curScene = Common::String::format("scene%d", picked);
	} else {
		_curScene = scene->_next;
	}
}

void GameSpacePirates::sceneNxtscnShowDeathGripBeamColor(Scene *scene) {
	_clue = randomNumberInRange(0x36, 0x38);
	_shotColor = 0;
	_curScene = Common::String::format("scene%d", _clue);
}

void GameSpacePirates::sceneNxtscnSelectAsteroids(Scene *scene) {
	uint16 picked = randomNumberInRange(0x013E, 0x0142);
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::sceneNxtscnAsteroidsDone(Scene *scene) {
	uint16 picked = 0;
	if (_randomCountAsteroids < 4) {
		_randomCountAsteroids++;
		picked = randomNumberInRange(0x013E, 0x0142);
	} else {
		sceneNxtscnDoFlyingSkulls(scene);
		return;
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::sceneNxtscnDoFlyingSkulls(Scene *scene) {
	_sceneBeforeFlyingSkulls = sceneToNumber(scene);
	uint16 picked = randomNumberInRange(0x014A, 0x014D);
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::sceneNxtscnDidFlyingSkulls(Scene *scene) {
	uint16 picked = 0;
	switch (_sceneBeforeFlyingSkulls) {
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
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::sceneNxtscnShowWhichStartSplitter(Scene *scene) {
	uint16 picked = randomNumberInRange(0x6A, 0x6C);
	_pickedStartSplitter = picked;
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::sceneNxtscnGotoSelectedWorld(Scene *scene) {
	_selectedAWorld = false;
	_curScene = Common::String::format("scene%d", _selectedWorldStart);
}

void GameSpacePirates::sceneNxtscnStartVolcanoPopup(Scene *scene) {
	_maxRandom = 3;
	_randomCount = 0;
	_randomScenesValues[0] = 0x73;
	_randomScenesValues[1] = 0x75;
	_randomScenesValues[2] = 0x77;
	memset(&_randomScenesUsed, 0, (9 * sizeof(uint8)));
	uint16 picked = randomUnusedScene(3);
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::sceneNxtscnContinueVolcanoPopup(Scene *scene) {
	_randomCount++;
	if (_randomCount < 3) {
		uint16 picked = randomUnusedScene(_maxRandom);
		_curScene = Common::String::format("scene%d", picked);
	} else {
		_curScene = scene->_next;
	}
}

void GameSpacePirates::sceneNxtscnGiveFalinaClue(Scene *scene) {
	_clue = randomNumberInRange(0x7F, 0x82);
	_shotColor = 0;
	_shotDirection = 0;
	_curScene = Common::String::format("scene%d", _clue);
}

void GameSpacePirates::sceneNxtscnCheckFalinaClues(Scene *scene) {
	uint16 picked = 0;
	if (_clue == 0x7F) {
		if (_shotColor == 3 && _shotDirection == 1) {
			if (_crystalState == 7) {
				picked = 0x89;
			} else {
				picked = pickCrystalScene(0x86, 0x87, 0x88);
			}
		} else {
			_curScene = scene->_next;
			return;
		}
	} else if (_clue == 0x80) {
		if (_shotColor == 3 && _shotDirection == 2) {
			if (_crystalState == 7) {
				picked = 0x89;
			} else {
				picked = pickCrystalScene(0x86, 0x87, 0x88);
			}
		} else {
			_curScene = scene->_next;
			return;
		}
	} else if (_clue == 0x81) {
		if (_shotColor == 4 && _shotDirection == 2) {
			if (_crystalState == 7) {
				picked = 0x89;
			} else {
				picked = pickCrystalScene(0x86, 0x87, 0x88);
			}
		} else {
			_curScene = scene->_next;
			return;
		}
	} else if (_clue == 0x82) {
		if (_shotColor == 4 && _shotDirection == 1) {
			if (_crystalState == 7) {
				picked = 0x89;
			} else {
				picked = pickCrystalScene(0x86, 0x87, 0x88);
			}
		} else {
			_curScene = scene->_next;
			return;
		}
	} else {
		_curScene = scene->_next;
		return;
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::sceneNxtscnSetupFalinaTargetPractice(Scene *scene) {
	_randomCount = 0;
	uint16 picked = randomNumberInRange(0x8A, 0x90);
	if (picked == 0x8D) {
		picked++;
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::sceneNxtscnContinueFalinaTargetPractice(Scene *scene) {
	uint16 picked = 0;
	_randomCount++;
	if (_randomCount < 6) {
		picked = randomNumberInRange(0x8A, 0x90);
		if (picked == 0x8D) {
			picked++;
		}
		_curScene = Common::String::format("scene%d", picked);
	} else {
		sceneNxtscnDoFlyingSkulls(scene);
		_worldDone[3] = true;
	}
}

void GameSpacePirates::sceneNxtscnStartDunePopup(Scene *scene) {
	_maxRandom = 4;
	_randomCount = 0;
	_randomScenesValues[0] = 0x93;
	_randomScenesValues[1] = 0x95;
	_randomScenesValues[2] = 0x97;
	_randomScenesValues[3] = 0x99;
	memset(&_randomScenesUsed, 0, (9 * sizeof(uint8)));
	uint16 picked = randomUnusedScene(4);
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::sceneNxtscnContinueDunePopup(Scene *scene) {
	_randomCount++;
	if (((_difficulty * 4) + 4) > _randomCount) {
		memset(&_randomScenesUsed, 0, (9 * sizeof(uint8)));
		uint16 picked = randomUnusedScene(_maxRandom);
		_curScene = Common::String::format("scene%d", picked);
		return;
	}
	_curScene = scene->_next;
}

void GameSpacePirates::sceneNxtscnPottOrPanShoots(Scene *scene) {
	uint8 randomNum = _rnd->getRandomNumber(9);
	uint16 picked = (randomNum > 5) ? 0xA2 : 0xA1;
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::sceneNxtscnSetupPottTargetPractice(Scene *scene) {
	uint16 picked = 0;
	_randomCount = 0;
	uint8 randomNum = _rnd->getRandomNumber(9);
	if (randomNum < 5) {
		picked = 0xA7;
	} else {
		picked = 0xAA;
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::sceneNxtscnContinuePottTargetPractice(Scene *scene) {
	uint16 picked = 0;
	_randomCount++;
	if (_randomCount < 6) {
		uint8 randomNum = _rnd->getRandomNumber(9);
		if (randomNum <= 5) {
			picked = 0xA7;
		} else {
			picked = 0xAA;
		}
	} else {
		sceneNxtscnDoFlyingSkulls(scene);
		_worldDone[0] = true;
		return;
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::sceneNxtscnStartDragonsTeethPopup(Scene *scene) {
	_maxRandom = 9;
	_randomCount = 0;
	_randomScenesValues[0] = 0xCF;
	_randomScenesValues[1] = 0xD2;
	_randomScenesValues[2] = 0xD4;
	_randomScenesValues[3] = 0xD6;
	_randomScenesValues[4] = 0xD8;
	_randomScenesValues[5] = 0xDA;
	_randomScenesValues[6] = 0xDC;
	_randomScenesValues[7] = 0xDE;
	_randomScenesValues[8] = 0xE0;
	memset(&_randomScenesUsed, 0, (9 * sizeof(uint8)));
	uint16 picked = randomUnusedScene(9);
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::sceneNxtscnContinueDragonsTeethPopup(Scene *scene) {
	if (sceneToNumber(scene) != 0x14F) {
		_randomCount++;
	}
	if (((_difficulty * 9) + 9) > _randomCount) {
		memset(&_randomScenesUsed, 0, (9 * sizeof(uint8)));
		uint16 picked = randomUnusedScene(_maxRandom);
		_curScene = Common::String::format("scene%d", picked);
		return;
	}
	_curScene = scene->_next;
}

void GameSpacePirates::sceneNxtscnGrinReaperClue(Scene *scene) {
	_shotGrinReaperCount = 0;
	_clue = randomNumberInRange(0xE2, 0xE7);
	_shotColor = 0;
	_curScene = Common::String::format("scene%d", _clue);
}

void GameSpacePirates::sceneNxtscnStartGrinReaper(Scene *scene) {
	_maxRandom = 8;
	_randomCount = 0;
	_randomScenesValues[0] = 0xEC;
	_randomScenesValues[1] = 0xED;
	_randomScenesValues[2] = 0xEE;
	_randomScenesValues[3] = 0xEF;
	_randomScenesValues[4] = 0xF0;
	_randomScenesValues[5] = 0xF1;
	_randomScenesValues[6] = 0xF2;
	_randomScenesValues[7] = 0xF3;
	memset(&_randomScenesUsed, 0, (9 * sizeof(uint8)));
	_randomScenesUsed[0] = 1;
	_curScene = Common::String::format("scene%d", _randomScenesValues[0]);
}

void GameSpacePirates::sceneNxtscnContinueGrinReaper(Scene *scene) {
	_randomCount++;
	uint16 picked = 0;
	if (_randomCount >= 8 || (_clue - 223) <= _randomCount) {
		_curScene = scene->_next;
	} else {
		picked = randomUnusedScene(_maxRandom);
		_curScene = Common::String::format("scene%d", picked);
	}
}

void GameSpacePirates::sceneNxtscnGrinTargetPractice(Scene *scene) {
	uint16 picked = 0;
	_randomCount = 0;
	picked = randomNumberInRange(0xFB, 0x0101);
	if (picked == 0xFE) {
		picked++;
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::sceneNxtscnContinueGrinTargetPractice(Scene *scene) {
	uint16 picked = 0;
	_randomCount++;
	if (_randomCount < 5) {
		picked = randomNumberInRange(0xFB, 0x0101);
		if (picked == 0xFE) {
			picked++;
		}
	} else {
		sceneNxtscnDoFlyingSkulls(scene);
		_worldDone[2] = true;
		return;
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::sceneNxtscnStartJunkWorld(Scene *scene) {
	_maxRandom = 6;
	_randomCount = 0;
	_randomScenesValues[0] = 0x0105;
	_randomScenesValues[1] = 0x0107;
	_randomScenesValues[2] = 0x0109;
	_randomScenesValues[3] = 0x010B;
	_randomScenesValues[4] = 0x010D;
	_randomScenesValues[5] = 0x010F;
	memset(&_randomScenesUsed, 0, (9 * sizeof(uint8)));
	uint16 picked = randomUnusedScene(6);
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::sceneNxtscnContinueJunkWorld(Scene *scene) {
	uint16 picked = 0;
	_randomCount++;
	if (((_difficulty * 6) + 6) > _randomCount) {
		memset(&_randomScenesUsed, 0, (9 * sizeof(uint8)));
		picked = randomUnusedScene(_maxRandom);
	} else {
		_curScene = scene->_next;
		return;
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::sceneNxtscnStartJunkRings(Scene *scene) {
	_randomCount = 0;
	uint16 picked = randomNumberInRange(0x011E, 0x0122);
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::sceneNxtscnShowJunkWorldCrystal(Scene *scene) {
	uint16 picked = pickCrystalScene(0x11D, 0x11C, 0x11B);
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::sceneNxtscnStartJunkWorldTargetPractice(Scene *scene) {
	_maxRandom = 6;
	_randomCount = 0;
	_randomScenesValues[0] = 0x125;
	_randomScenesValues[1] = 0x127;
	_randomScenesValues[2] = 0x129;
	_randomScenesValues[3] = 0x12C;
	_randomScenesValues[4] = 0x12E;
	_randomScenesValues[5] = 0x130;
	memset(&_randomScenesUsed, 0, (9 * sizeof(uint8)));
	uint16 picked = randomUnusedScene(6);
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::sceneNxtscnContinueJunkWorldTargetPractice(Scene *scene) {
	_randomCount++;
	if (_randomCount == 1) {
		_targetPracticeReset = false;
	}
	if (_randomCount >= 0xC) {
		sceneNxtscnDoFlyingSkulls(scene);
		_worldDone[1] = true;
		return;
	}
	if (_randomCount >= 6 && !_targetPracticeReset) {
		memset(&_randomScenesUsed, 0, (9 * sizeof(uint8)));
		_targetPracticeReset = true;
	}
	uint16 picked = randomUnusedScene(_maxRandom);
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::sceneNxtscnAreAllWorldsDone(Scene *scene) {
	if (_worldDone[0] && _worldDone[1] && _worldDone[2] && _worldDone[3]) {
		_curScene = "scene192";
	} else {
		_curScene = scene->_next;
	}
}

void GameSpacePirates::sceneNxtscnStartPracticePirateShip(Scene *scene) {
	_randomCount = 0;
	uint16 picked = randomNumberInRange(0xBE, 0xBF);
	_curScene = Common::String::format("scene%d", picked);
}

void GameSpacePirates::sceneNxtscnMorePracticePirateShip(Scene *scene) {
	_randomCount++;
	if (_randomCount < 5) {
		uint16 picked = randomNumberInRange(0xBE, 0xBF);
		_curScene = Common::String::format("scene%d", picked);
	} else {
		_curScene = scene->_next;
	}
}

void GameSpacePirates::sceneNxtscnPlayerWon(Scene *scene) {
	_gameInProgress = false;
	_curScene = scene->_next;
}

// Script functions: MissedRect
void GameSpacePirates::sceneMissedRectsDefault(Scene *scene) {
	// do nothing
}

void GameSpacePirates::sceneMissedRectsMissedPirateShip(Scene *scene) {
	displayMultipleShotLines();
}

// Script functions: WepDwn
void GameSpacePirates::sceneDefaultWepdwn(Scene *scene) {
	_shots = 10;
}

// Debug methods
void GameSpacePirates::debugWarpTo(int val) {
	// TODO implement
}

// Debugger methods
DebuggerSpacePirates::DebuggerSpacePirates(GameSpacePirates *game) {
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
		_game->debugWarpTo(val);
		return false;
	}
}

bool DebuggerSpacePirates::cmdDumpLib(int argc, const char **argv) {
	return _game->debug_dumpLibFile();
}

} // End of namespace Alg
