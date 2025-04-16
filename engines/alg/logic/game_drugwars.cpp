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
#include "alg/logic/game_drugwars.h"
#include "alg/scene.h"

namespace Alg {

GameDrugWars::GameDrugWars(AlgEngine *vm, const AlgGameDescription *gd) : Game(vm) {
}

GameDrugWars::~GameDrugWars() {
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

void GameDrugWars::init() {
	Game::init();

	_videoPosX = 11;
	_videoPosY = 2;

	if (_vm->isDemo()) {
		loadLibArchive("dwdemo.lib");
	} else if(_vm->useSingleSpeedVideos()) {
		loadLibArchive("dwss.lib");
	} else {
		loadLibArchive("dwds.lib");
	}

	_sceneInfo->loadScnFile("dw.scn");
	_startScene = _sceneInfo->getStartScene();

	registerScriptFunctions();
	verifyScriptFunctions();

	_menuzone = new Zone("MainMenu", "GLOBALHIT");
	_menuzone->addRect(0x0C, 0xAA, 0x38, 0xC7, nullptr, 0, "SHOTMENU", "0");

	_submenzone = new Zone("SubMenu", "GLOBALHIT");
	_submenzone->addRect(0x1C, 0x13, 0x5D, 0x22, nullptr, 0, "STARTMENU", "0");
	_submenzone->addRect(0x1C, 0x33, 0x5D, 0x42, nullptr, 0, "RECTLOAD", "0");
	_submenzone->addRect(0x1C, 0x53, 0x5D, 0x62, nullptr, 0, "RECTSAVE", "0");
	_submenzone->addRect(0x1C, 0x73, 0x5D, 0x82, nullptr, 0, "CONTMENU", "0");
	_submenzone->addRect(0x1C, 0x93, 0x5D, 0xA2, nullptr, 0, "EXITMENU", "0");
	_submenzone->addRect(0xDD, 0x34, 0x10A, 0x43, nullptr, 0, "RECTEASY", "0");
	_submenzone->addRect(0xDD, 0x55, 0x10A, 0x64, nullptr, 0, "RECTAVG", "0");
	_submenzone->addRect(0xDD, 0x75, 0x10A, 0x84, nullptr, 0, "RECTHARD", "0");

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

	_background = AlgGraphics::loadVgaBackground("dw_menu.vga", _palette);
	_screen->copyRectToSurface(_background->getPixels(), _background->pitch, 0, 0, _background->w, _background->h);

	moveMouse();
}

void GameDrugWars::registerScriptFunctions() {
#define RECT_HIT_FUNCTION(name, func) _rectHitFuncs[name] = new DWScriptFunctionRect(this, &GameDrugWars::func);
	RECT_HIT_FUNCTION("DEFAULT", rectNewScene);
	RECT_HIT_FUNCTION("EXITMENU", rectExit);
	RECT_HIT_FUNCTION("CONTMENU", rectContinue);
	RECT_HIT_FUNCTION("STARTMENU", rectStart);
	RECT_HIT_FUNCTION("SHOTMENU", rectShotMenu);
	RECT_HIT_FUNCTION("RECTSAVE", rectSave);
	RECT_HIT_FUNCTION("RECTLOAD", rectLoad);
	RECT_HIT_FUNCTION("RECTEASY", rectEasy);
	RECT_HIT_FUNCTION("RECTAVG", rectAverage);
	RECT_HIT_FUNCTION("RECTHARD", rectHard);
	RECT_HIT_FUNCTION("SELECT_TARGET_PRACTICE", rectSelectTargetPractice);
	RECT_HIT_FUNCTION("SELECT_BAR", rectSelectBar);
	RECT_HIT_FUNCTION("SELECT_CAR_CHASE", rectSelectCarChase);
	RECT_HIT_FUNCTION("SELECT_DRUG_HOUSE", rectSelectDrugHouse);
	RECT_HIT_FUNCTION("SELECT_OFFICE", rectSelectOffice);
	RECT_HIT_FUNCTION("SELECT_COURT", rectSelectCourt);
	RECT_HIT_FUNCTION("SELECT_BUS", rectSelectBus);
	RECT_HIT_FUNCTION("SELECT_DOCKS", rectSelectDocks);
	RECT_HIT_FUNCTION("SELECT_HOUSE_BOAT", rectSelectHouseBoat);
	RECT_HIT_FUNCTION("SELECT_PARTY", rectSelectParty);
	RECT_HIT_FUNCTION("SELECT_AIRPORT", rectSelectAirport);
	RECT_HIT_FUNCTION("SELECT_MANSION", rectSelectMansion);
	RECT_HIT_FUNCTION("SELECT_VILLAGE", rectSelectVillage);
#undef RECT_HIT_FUNCTION

#define PRE_OPS_FUNCTION(name, func) _scenePreOps[name] = new DWScriptFunctionScene(this, &GameDrugWars::func);
	PRE_OPS_FUNCTION("DEFAULT", scenePsoDrawRct);
	PRE_OPS_FUNCTION("FADEIN", scenePsoFadeIn);
	PRE_OPS_FUNCTION("PAUSE", scenePsoPause);
	PRE_OPS_FUNCTION("PAUSE_FADEIN", scenePsoPauseFadeIn);
	PRE_OPS_FUNCTION("GOT_TO", scenePsoGotTo);
#undef PRE_OPS_FUNCTION

#define INS_OPS_FUNCTION(name, func) _sceneInsOps[name] = new DWScriptFunctionScene(this, &GameDrugWars::func);
	INS_OPS_FUNCTION("DEFAULT", sceneIsoDoNothing);
	INS_OPS_FUNCTION("PAUSE", sceneIsoPause);
#undef INS_OPS_FUNCTION

#define NXT_SCN_FUNCTION(name, func) _sceneNxtScn[name] = new DWScriptFunctionScene(this, &GameDrugWars::func);
	NXT_SCN_FUNCTION("DEFAULT", sceneDefaultNxtscn);
	NXT_SCN_FUNCTION("GAME_WON", sceneNxtscnGameWon);
	NXT_SCN_FUNCTION("LOSE_A_LIFE", sceneNxtscnLoseALife);
	NXT_SCN_FUNCTION("CONTINUE_GAME", sceneNxtscnContinueGame);
	NXT_SCN_FUNCTION("DID_NOT_CONTINUE", sceneNxtscnDidNotContinue);
	NXT_SCN_FUNCTION("KILL_INNOCENT_MAN", sceneNxtscnKillInnocentMan);
	NXT_SCN_FUNCTION("KILL_INNOCENT_WOMAN", sceneNxtscnKillInnocentWoman);
	NXT_SCN_FUNCTION("AFTER_DIE", sceneNxtscnAfterDie);
	NXT_SCN_FUNCTION("INIT_RANDOM", sceneNxtscnInitRandom);
	NXT_SCN_FUNCTION("CONTINUE_RANDOM", sceneNxtscnContinueRandom);
	NXT_SCN_FUNCTION("SELECT_SCENARIO", sceneNxtscnSelectScenario);
	NXT_SCN_FUNCTION("FINISH_SCENARIO", sceneNxtscnFinishScenario);
#undef NXT_SCN_FUNCTION

	_sceneShowMsg["DEFAULT"] = new DWScriptFunctionScene(this, &GameDrugWars::sceneSmDonothing);
	_sceneWepDwn["DEFAULT"] = new DWScriptFunctionScene(this, &GameDrugWars::sceneDefaultWepdwn);
	_sceneScnScr["DEFAULT"] = new DWScriptFunctionScene(this, &GameDrugWars::sceneDefaultScore);
	_sceneNxtFrm["DEFAULT"] = new DWScriptFunctionScene(this, &GameDrugWars::sceneNxtfrm);
}

void GameDrugWars::verifyScriptFunctions() {
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

DWScriptFunctionRect GameDrugWars::getScriptFunctionRectHit(const Common::String &name) {
	auto it = _rectHitFuncs.find(name);
	if (it != _rectHitFuncs.end()) {
		return *it->_value;
	} else {
		error("GameDrugWars::getScriptFunctionRectHit(): Could not find rectHit function: %s", name.c_str());
	}
}

DWScriptFunctionScene GameDrugWars::getScriptFunctionScene(SceneFuncType type, const Common::String &name) {
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
		error("GameDrugWars::getScriptFunctionScene(): Unkown scene script type: %u", type);
		break;
	}
	DWScriptFunctionSceneMap::iterator it;
	it = functionMap->find(name);
	if (it != functionMap->end()) {
		return *it->_value;
	} else {
		error("GameDrugWars::getScriptFunctionScene(): Could not find scene type %u function: %s", type, name.c_str());
	}
}

void GameDrugWars::callScriptFunctionRectHit(const Common::String &name, Rect *rect) {
	DWScriptFunctionRect function = getScriptFunctionRectHit(name);
	function(rect);
}

void GameDrugWars::callScriptFunctionScene(SceneFuncType type, const Common::String &name, Scene *scene) {
	DWScriptFunctionScene function = getScriptFunctionScene(type, name);
	function(scene);
}

Common::Error GameDrugWars::run() {
	init();
	newGame();
	_curScene = _startScene;
	Common::String oldscene;
	while (!_vm->shouldQuit()) {
		oldscene = _curScene;
		_fired = false;
		Scene *scene = _sceneInfo->findScene(_curScene);
		if (!loadScene(scene)) {
			error("GameDrugWars::run(): Cannot find scene %s in libfile", scene->_name.c_str());
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

void GameDrugWars::newGame() {
	_shots = 10;
	_lives = 3;
	_holster = false;
}

void GameDrugWars::resetParams() {
	// fill _gotTo with scenario start scenes
	// 0 in _gotTo array means the scenario is finished
	for (int i = 0; i < 14; i++) {
		_gotTo[i] = _scenarioStartScenes[i];
	}
}

void GameDrugWars::doMenu() {
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

void GameDrugWars::changeDifficulty(uint8 newDifficulty) {
	if (newDifficulty == _oldDifficulty) {
		return;
	}
	showDifficulty(newDifficulty, true);
	_oldDifficulty = newDifficulty;
	_difficulty = newDifficulty;
}

void GameDrugWars::showDifficulty(uint8 newDifficulty, bool cursor) {
	// reset menu screen
	_screen->copyRectToSurface(_background->getBasePtr(_videoPosX, _videoPosY), _background->pitch, _videoPosX, _videoPosY, _videoDecoder->getWidth(), _videoDecoder->getHeight());
	uint16 posY = 0x3C + ((newDifficulty - 1) * 0x21);
	AlgGraphics::drawImageCentered(_screen, _difficultyIcon, 0x0115, posY);
	if (cursor) {
		updateCursor();
	}
}

void GameDrugWars::updateCursor() {
	updateMouse();
}

void GameDrugWars::updateMouse() {
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

void GameDrugWars::moveMouse() {
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

void GameDrugWars::displayLivesLeft() {
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

void GameDrugWars::displayScores() {
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

void GameDrugWars::displayShotsLeft() {
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

bool GameDrugWars::weaponDown() {
	if (_rightDown && _mousePos.y >= 0xAA && _mousePos.x >= 0x113) {
		return true;
	}
	return false;
}

bool GameDrugWars::saveState() {
	Common::OutSaveFile *outSaveFile;
	Common::String saveFileName = _vm->getSaveStateName(0);
	if (!(outSaveFile = g_system->getSavefileManager()->openForSaving(saveFileName))) {
		warning("GameDrugWars::saveState(): Can't create file '%s', game not saved", saveFileName.c_str());
		return false;
	}
	outSaveFile->writeUint32BE(MKTAG('A', 'L', 'G', 'S')); // header
	outSaveFile->writeByte(0);                             // version, unused for now
	outSaveFile->writeSByte(_stage);
	outSaveFile->writeByte(_continues);
	outSaveFile->writeSByte(_gotToIndex);
	for (int i = 0; i < 14; i++) {
		outSaveFile->writeUint16LE(_gotTo[i]);
	}
	outSaveFile->writeSByte(_lives);
	outSaveFile->writeUint16LE(_shots);
	outSaveFile->writeSint32LE(_score);
	outSaveFile->writeByte(_difficulty);
	outSaveFile->writeString(_curScene);
	outSaveFile->writeByte(0);
	outSaveFile->finalize();
	delete outSaveFile;
	return true;
}

bool GameDrugWars::loadState() {
	Common::InSaveFile *inSaveFile;
	Common::String saveFileName = _vm->getSaveStateName(0);
	if (!(inSaveFile = g_system->getSavefileManager()->openForLoading(saveFileName))) {
		debug("GameDrugWars::loadState(): Can't load file '%s', game not loaded", saveFileName.c_str());
		return false;
	}
	uint32 header = inSaveFile->readUint32BE();
	if (header != MKTAG('A', 'L', 'G', 'S')) {
		warning("GameDrugWars::loadState(): Unkown save file, header: %s", tag2str(header));
		return false;
	}
	inSaveFile->skip(1); // version, unused for now
	_stage = inSaveFile->readSByte();
	_continues = inSaveFile->readByte();
	_gotToIndex = inSaveFile->readSByte();
	for (int i = 0; i < 14; i++) {
		_gotTo[i] = inSaveFile->readUint16LE();
	}
	_lives = inSaveFile->readSByte();
	_shots = inSaveFile->readUint16LE();
	_score = inSaveFile->readSint32LE();
	_difficulty = inSaveFile->readByte();
	_curScene = inSaveFile->readString();
	delete inSaveFile;
	_gameInProgress = true;
	changeDifficulty(_difficulty);
	return true;
}

// misc game functions
void GameDrugWars::displayShotFiredImage(Common::Point *point) {
	if (point->x >= _videoPosX && point->x <= (_videoPosX + _videoDecoder->getWidth()) && point->y >= _videoPosY && point->y <= (_videoPosY + _videoDecoder->getHeight())) {
		uint16 targetX = point->x - _videoPosX;
		uint16 targetY = point->y - _videoPosY;
		AlgGraphics::drawImageCentered(_videoDecoder->getVideoFrame(), _bulletholeIcon, targetX, targetY);
	}
}

void GameDrugWars::enableVideoFadeIn() {
	// TODO implement
}

uint16 GameDrugWars::sceneToNumber(Scene *scene) {
	return atoi(scene->_name.substr(5).c_str());
}

uint16 GameDrugWars::pickRandomScene(uint8 index, uint8 max) {
	if (_randomScenes[index] == nullptr) {
		error("GameDrugWars::pickRandomScene(): called with illegal index: %d", index);
	}
	if (max != 0) {
		_randomMax = max;
		_randomMask = 0;
		_randomPicked = 0;
		_randomSceneCount = 0;
		while (_randomScenes[index][_randomSceneCount] != 0) {
			_randomSceneCount++;
		}
	}
	uint16 count = _randomMax--;
	if (count > 0) {
		_randomPicked = randomUnusedInt(_randomSceneCount, &_randomMask, _randomPicked);
		return _randomScenes[index][_randomPicked];
	}
	return 0;
}

uint16 GameDrugWars::pickDeathScene() {
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

void GameDrugWars::sceneNxtscnGeneric(uint8 index) {
	uint16 nextSceneId = 0;
	_gotTo[index] = 0;
	if (_gotTo[0] || _gotTo[1] || _gotTo[3] || _gotTo[2]) {
		nextSceneId = 0x26;
	} else if (_gotTo[4] || _gotTo[5] || _gotTo[6]) {
		if (_stage == 1) {
			nextSceneId = 0x52;
		} else {
			_stage = 1;
			nextSceneId = 0x50;
		}
	} else if (_gotTo[7] || _gotTo[8] || _gotTo[9]) {
		if (_stage == 2) {
			nextSceneId = 0x9A;
		} else {
			_stage = 2;
			nextSceneId = 0x81;
		}
	} else if (_gotTo[10] || _gotTo[11] || _gotTo[12]) {
		if (_stage == 3) {
			nextSceneId = 0xDF;
		} else {
			_stage = 3;
			nextSceneId = 0x14B;
		}
	} else if (_gotTo[13]) {
		_stage = 4;
		nextSceneId = 0x18F;
	} else {
		nextSceneId = 0x21;
	}
	_curScene = Common::String::format("scene%d", nextSceneId);
}

void GameDrugWars::rectSelectGeneric(uint8 index) {
	if (_gotTo[index] > 0) {
		_curScene = Common::String::format("scene%d", _gotTo[index]);
		_gotToIndex = index;
	}
}

// Script functions: RectHit
void GameDrugWars::rectShotMenu(Rect *rect) {
	doMenu();
}

void GameDrugWars::rectSave(Rect *rect) {
	if (saveState()) {
		doSaveSound();
	}
}

void GameDrugWars::rectLoad(Rect *rect) {
	if (loadState()) {
		doLoadSound();
	}
}

void GameDrugWars::rectContinue(Rect *rect) {
	_inMenu = false;
	_fired = false;
	if (_lives <= 0) {
		_score = (int32)(_score * 0.7f);
		uint16 returnScene = _stageStartScenes[_stage];
		_curScene = Common::String::format("scene%d", returnScene);
		newGame();
	}
}

void GameDrugWars::rectStart(Rect *rect) {
	_inMenu = false;
	_fired = false;
	_gameInProgress = true;
	if (_vm->isDemo()) {
		_curScene = "scene54";
		_gotToIndex = 1;
		_gotTo[_gotToIndex] = 54;
	} else {
		_curScene = "scene53";
	}
	resetParams();
	newGame();
}

void GameDrugWars::rectSelectTargetPractice(Rect *rect) {
	rectSelectGeneric(0);
	_gotTo[0] = 0;
}

void GameDrugWars::rectSelectBar(Rect *rect) {
	_gotTo[0] = 0;
	rectSelectGeneric(1);
}

void GameDrugWars::rectSelectCarChase(Rect *rect) {
	_gotTo[0] = 0;
	rectSelectGeneric(2);
}

void GameDrugWars::rectSelectDrugHouse(Rect *rect) {
	_gotTo[0] = 0;
	rectSelectGeneric(3);
}

void GameDrugWars::rectSelectOffice(Rect *rect) {
	rectSelectGeneric(4);
}

void GameDrugWars::rectSelectCourt(Rect *rect) {
	rectSelectGeneric(5);
}

void GameDrugWars::rectSelectBus(Rect *rect) {
	rectSelectGeneric(6);
}

void GameDrugWars::rectSelectDocks(Rect *rect) {
	rectSelectGeneric(7);
}

void GameDrugWars::rectSelectHouseBoat(Rect *rect) {
	rectSelectGeneric(9);
}

void GameDrugWars::rectSelectParty(Rect *rect) {
	rectSelectGeneric(8);
}

void GameDrugWars::rectSelectAirport(Rect *rect) {
	rectSelectGeneric(10);
}

void GameDrugWars::rectSelectMansion(Rect *rect) {
	rectSelectGeneric(11);
}

void GameDrugWars::rectSelectVillage(Rect *rect) {
	rectSelectGeneric(12);
}

// Script functions: Scene PreOps
void GameDrugWars::scenePsoGotTo(Scene *scene) {
	uint16 sceneId = sceneToNumber(scene);
	_gotTo[_gotToIndex] = sceneId;
	if (_gotToIndex == 13) {
		_finalStageScene = sceneToNumber(scene);
	}
	enableVideoFadeIn();
}

// Script functions: Scene NxtScn
void GameDrugWars::sceneNxtscnGameWon(Scene *scene) {
	_gameInProgress = false;
	_curScene = _startScene;
}

void GameDrugWars::sceneNxtscnDidNotContinue(Scene *scene) {
	_gameInProgress = false;
	_curScene = _startScene;
}

void GameDrugWars::sceneNxtscnLoseALife(Scene *scene) {
	uint16 picked = 0;
	if (!_debug_godMode) {
		_lives--;
	}
	if (_vm->isDemo()) {
		_curScene = "scene83";
		return;
	} else if (_lives > 0) {
		displayLivesLeft();
		picked = pickDeathScene();
	} else {
		picked = _deadScenes[_stage];
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameDrugWars::sceneNxtscnContinueGame(Scene *scene) {
	if (_continues < 2) {
		_curScene = "scene438";
	} else {
		sceneNxtscnDidNotContinue(scene);
	}
}

void GameDrugWars::sceneNxtscnKillInnocentMan(Scene *scene) {
	uint16 picked = 0;
	if (!_debug_godMode) {
		_lives--;
	}
	if (_vm->isDemo()) {
		sceneNxtscnAfterDie(scene);
		return;
	} else if (_lives > 0) {
		picked = _stageStartScenes[_stage];
	} else {
		picked = _deadScenes[_stage];
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameDrugWars::sceneNxtscnKillInnocentWoman(Scene *scene) {
	sceneNxtscnKillInnocentMan(scene);
}

void GameDrugWars::sceneNxtscnAfterDie(Scene *scene) {
	if (_vm->isDemo()) {
		if (_gotTo[_gotToIndex] > 54) {
			_curScene = "scene67";
		} else {
			_curScene = "scene54";
		}
	} else {
		uint16 picked = _stageStartScenes[_stage];
		_curScene = Common::String::format("scene%d", picked);
	}
}

void GameDrugWars::sceneNxtscnInitRandom(Scene *scene) {
	int totalRandom = (_difficulty * 2) + _randomScenesDifficulty[_gotToIndex] + 2;
	uint16 picked = pickRandomScene(_gotToIndex, totalRandom);
	_curScene = Common::String::format("scene%d", picked);
}

void GameDrugWars::sceneNxtscnContinueRandom(Scene *scene) {
	uint16 picked = pickRandomScene(_gotToIndex, 0);
	if (picked == 0) {
		picked = _randomScenesContinue[_gotToIndex];
		if (picked == 0) {
			error("GameDrugWars::sceneNxtscnContinueRandom(): called with illegal _got_to_index: %d", _gotToIndex);
		}
	}
	_curScene = Common::String::format("scene%d", picked);
}

void GameDrugWars::sceneNxtscnSelectScenario(Scene *scene) {
	uint16 picked = 0;
	switch (_stage) {
	case 0:
		if (_gotTo[0] > 0) {
			rectSelectGeneric(0);
			_gotTo[0] = 0;
		} else if (_gotTo[1] > 0) {
			rectSelectGeneric(1);
		} else if (_gotTo[2] > 0) {
			rectSelectGeneric(2);
		} else if (_gotTo[3] > 0) {
			rectSelectGeneric(3);
		} else {
			picked = 0x83;
			_stage = 1;
		}
		break;
	case 1:
		if (_gotTo[4] > 0) {
			rectSelectGeneric(4);
		} else if (_gotTo[5] > 0) {
			rectSelectGeneric(5);
		} else if (_gotTo[6] > 0) {
			rectSelectGeneric(6);
		} else {
			picked = 0xEE;
			_stage = 2;
		}
		break;
	case 2:
		if (_gotTo[7] > 0) {
			rectSelectGeneric(7);
		} else if (_gotTo[8] > 0) {
			rectSelectGeneric(8);
		} else if (_gotTo[9] > 0) {
			rectSelectGeneric(9);
		} else {
			picked = 0x0132;
			_stage = 3;
		}
		break;
	case 3:
		if (_gotTo[10] > 0) {
			rectSelectGeneric(10);
		} else if (_gotTo[11] > 0) {
			rectSelectGeneric(11);
		} else if (_gotTo[12] > 0) {
			rectSelectGeneric(12);
		} else {
			picked = _finalStageScene;
			_gotToIndex = 13;
			_stage = 4;
		}
		break;
	}
	if (picked != 0) {
		_curScene = Common::String::format("scene%d", picked);
	}
}

void GameDrugWars::sceneNxtscnFinishScenario(Scene *scene) {
	uint16 picked = 0;
	_gotTo[_gotToIndex] = 0;
	if (_vm->isDemo()) {
		_curScene = _startScene;
		return;
	}
	if (_gotTo[0] || _gotTo[1] || _gotTo[3] || _gotTo[2]) {
		picked = 0x51;
	} else if (_gotTo[4] || _gotTo[5] || _gotTo[6]) {
		if (_stage == 1) {
			picked = 0x83;
		} else {
			_stage = 1;
			picked = 0x6B;
		}
	} else if (_gotTo[7] || _gotTo[8] || _gotTo[9]) {
		if (_stage == 2) {
			picked = 0xEE;
		} else {
			_stage = 2;
			picked = 0xB6;
		}
	} else if (_gotTo[10] || _gotTo[11] || _gotTo[12]) {
		if (_stage == 3) {
			picked = 0x0132;
		} else {
			_stage = 3;
			picked = 0x0109;
		}
	} else if (_gotTo[13] != 0) {
		_stage = 13;
		_stage = 4;
		picked = 0x017F;
	} else {
		picked = 0x21;
	}
	_curScene = Common::String::format("scene%d", picked);
}

// Script functions: WepDwn
void GameDrugWars::sceneDefaultWepdwn(Scene *scene) {
	_shots = 10;
}

// Debug methods
void GameDrugWars::debugWarpTo(int val) {
	// TODO implement
}

// Debugger methods
DebuggerDrugWars::DebuggerDrugWars(GameDrugWars *game) {
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
		_game->debugWarpTo(val);
		return false;
	}
}

bool DebuggerDrugWars::cmdDumpLib(int argc, const char **argv) {
	return _game->debug_dumpLibFile();
}

} // End of namespace Alg
