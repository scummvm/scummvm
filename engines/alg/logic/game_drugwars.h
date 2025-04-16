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

#ifndef ALG_GAME_DRUGWARS_H
#define ALG_GAME_DRUGWARS_H

#include "common/hashmap.h"
#include "common/rect.h"

#include "gui/debugger.h"

#include "alg/game.h"
#include "alg/scene.h"

namespace Alg {

typedef Common::Functor1Mem<Scene *, void, GameDrugWars> DWScriptFunctionScene;
typedef Common::Functor1Mem<Rect *, void, GameDrugWars> DWScriptFunctionRect;
typedef Common::HashMap<Common::String, DWScriptFunctionScene *> DWScriptFunctionSceneMap;
typedef Common::HashMap<Common::String, DWScriptFunctionRect *> DWScriptFunctionRectMap;

class GameDrugWars : public Game {

	enum SceneFuncType {
		PREOP = 1,
		SHOWMSG = 2,
		INSOP = 3,
		WEPDWN = 4,
		SCNSCR = 5,
		NXTFRM = 6,
		NXTSCN = 7
	};

public:
	GameDrugWars(AlgEngine *vm, const AlgGameDescription *gd);
	~GameDrugWars() override;
	Common::Error run() override;
	void debugWarpTo(int val);

private:
	void init() override;
	void registerScriptFunctions();
	void verifyScriptFunctions();
	DWScriptFunctionRect getScriptFunctionRectHit(const Common::String &name);
	DWScriptFunctionScene getScriptFunctionScene(SceneFuncType type, const Common::String &name);
	void callScriptFunctionRectHit(const Common::String &name, Rect *rect);
	void callScriptFunctionScene(SceneFuncType type, const Common::String &name, Scene *scene);

	DWScriptFunctionRectMap _rectHitFuncs;
	DWScriptFunctionSceneMap _scenePreOps;
	DWScriptFunctionSceneMap _sceneShowMsg;
	DWScriptFunctionSceneMap _sceneInsOps;
	DWScriptFunctionSceneMap _sceneWepDwn;
	DWScriptFunctionSceneMap _sceneScnScr;
	DWScriptFunctionSceneMap _sceneNxtFrm;
	DWScriptFunctionSceneMap _sceneNxtScn;

	// images
	Graphics::Surface *_shotIcon;
	Graphics::Surface *_emptyIcon;
	Graphics::Surface *_liveIcon;
	Graphics::Surface *_deadIcon;
	Graphics::Surface *_difficultyIcon;
	Graphics::Surface *_bulletholeIcon;

	// constants
	const int16 _randomScenes0[7] = {0x29, 0x2B, 0x2D, 0x2F, 0x31, 0x33, 0};
	const int16 _randomScenes1[6] = {0x37, 0x39, 0x3B, 0x3D, 0x3F, 0};
	const int16 _randomScenes4[8] = {0xA8, 0xAA, 0xAC, 0xAE, 0xB0, 0xB2, 0xB4, 0};
	const int16 _randomScenes8[8] = {0xC0, 0xC2, 0xC4, 0xC6, 0xC8, 0xCA, 0xCC, 0};
	const int16 _randomScenes9[6] = {0xFE, 0x0100, 0x0102, 0x01A3, 0x0105, 0};
	const int16 _randomScenes10[8] = {0x0161, 0x0163, 0x0165, 0x0167, 0x016A, 0x016C, 0x016E, 0};
	const int16 _randomScenes11[9] = {0x010B, 0x010D, 0x010F, 0x0111, 0x0113, 0x0115, 0x0117, 0x0119, 0};
	const int16 _randomScenes12[10] = {0x014C, 0x014E, 0x0150, 0x0152, 0x0154, 0x0156, 0x0158, 0x015A, 0x015C, 0};

	const int16 *_randomScenes[14] = {_randomScenes0, _randomScenes1, nullptr, nullptr, _randomScenes4, nullptr, nullptr, nullptr,
									  _randomScenes8, _randomScenes9, _randomScenes10, _randomScenes11, _randomScenes12, nullptr};
	const uint8 _randomScenesDifficulty[14] = {6, 4, 0, 0, 6, 0, 0, 0, 5, 6, 7, 8, 8, 0};
	const uint16 _randomScenesContinue[14] = {0x51, 0x41, 0, 0, 0x01B5, 0, 0, 0, 0xCE, 0x0107, 0x0170, 0x011B, 0x015E, 0};

	const int16 _diedScenesStage0[4] = {0x52, 0x53, 0x54, 0};
	const int16 _diedScenesStage1[5] = {0x85, 0x86, 0x88, 0x89, 0};
	const int16 _diedScenesStage2[3] = {0xEF, 0xF0, 0};
	const int16 _diedScenesStage3[3] = {0x0135, 0x0136, 0};
	const int16 _diedScenesStage4[3] = {0x0135, 0x0136, 0};

	const int16 *_diedScenesByStage[5] = {_diedScenesStage0, _diedScenesStage1, _diedScenesStage2, _diedScenesStage3, _diedScenesStage4};

	uint16 _deadScenes[5] = {0x56, 0x8A, 0xF2, 0x0134, 0x0134};

	const uint16 _stageStartScenes[5] = {0x51, 0x83, 0xEE, 0x0132, 0x017F};

	const uint16 _scenarioStartScenes[14] = {0x27, 0x36, 0x4A, 0x57, 0x9D, 0x8B, 0x74, 0xD8, 0xBF, 0xB8, 0x0160, 0x010A, 0x0137, 0x017F};

	// gamestate
	uint8 _continues = 0;
	uint16 _gotTo[14] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int8 _gotToIndex = 0;
	int8 _stage = 0;
	int8 _oldStage = -1;
	uint8 _randomSceneCount = 0;
	uint8 _randomMax = 0;
	uint16 _randomMask = 0;
	uint16 _randomPicked = 0;
	uint16 _deathMask = 0;
	int16 _deathPicked = 0;
	uint8 _deathSceneCount = 0;
	uint16 _finalStageScene = _stageStartScenes[4];

	// base functions
	void newGame();
	void resetParams();
	void doMenu();
	void changeDifficulty(uint8 newDifficulty);
	void showDifficulty(uint8 newDifficulty, bool updateCursor);
	void updateCursor();
	void updateMouse();
	void moveMouse();
	void displayLivesLeft();
	void displayScores();
	void displayShotsLeft();
	bool weaponDown();
	bool saveState();
	bool loadState();

	// misc game functions
	void displayShotFiredImage(Common::Point *point);
	void enableVideoFadeIn();
	uint16 sceneToNumber(Scene *scene);
	uint16 pickRandomScene(uint8 index, uint8 max);
	uint16 pickDeathScene();
	void sceneNxtscnGeneric(uint8 index);
	void rectSelectGeneric(uint8 index);

	// Script functions: RectHit
	void rectShotMenu(Rect *rect);
	void rectSave(Rect *rect);
	void rectLoad(Rect *rect);
	void rectContinue(Rect *rect);
	void rectStart(Rect *rect);
	void rectSelectTargetPractice(Rect *rect);
	void rectSelectBar(Rect *rect);
	void rectSelectCarChase(Rect *rect);
	void rectSelectDrugHouse(Rect *rect);
	void rectSelectOffice(Rect *rect);
	void rectSelectCourt(Rect *rect);
	void rectSelectBus(Rect *rect);
	void rectSelectDocks(Rect *rect);
	void rectSelectHouseBoat(Rect *rect);
	void rectSelectParty(Rect *rect);
	void rectSelectAirport(Rect *rect);
	void rectSelectMansion(Rect *rect);
	void rectSelectVillage(Rect *rect);

	// Script functions: Scene PreOps
	void scenePsoGotTo(Scene *scene);

	// Script functions: Scene NxtScn
	void sceneNxtscnGameWon(Scene *scene);
	void sceneNxtscnLoseALife(Scene *scene);
	void sceneNxtscnContinueGame(Scene *scene);
	void sceneNxtscnDidNotContinue(Scene *scene);
	void sceneNxtscnKillInnocentMan(Scene *scene);
	void sceneNxtscnKillInnocentWoman(Scene *scene);
	void sceneNxtscnAfterDie(Scene *scene);
	void sceneNxtscnInitRandom(Scene *scene);
	void sceneNxtscnContinueRandom(Scene *scene);
	void sceneNxtscnSelectScenario(Scene *scene);
	void sceneNxtscnFinishScenario(Scene *scene);

	// Script functions: Scene WepDwn
	void sceneDefaultWepdwn(Scene *scene);
};

class DebuggerDrugWars : public GUI::Debugger {
public:
	DebuggerDrugWars(GameDrugWars *game);
	bool cmdWarpTo(int argc, const char **argv);
	bool cmdDumpLib(int argc, const char **argv);

private:
	GameDrugWars *_game;
};

} // End of namespace Alg

#endif
