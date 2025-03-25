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

#ifndef ALG_GAME_CRIMEPATROL_H
#define ALG_GAME_CRIMEPATROL_H

#include "common/hashmap.h"
#include "common/rect.h"

#include "gui/debugger.h"

#include "alg/game.h"
#include "alg/scene.h"

namespace Alg {

typedef Common::Functor1Mem<Scene *, void, GameCrimePatrol> CPScriptFunctionScene;
typedef Common::Functor1Mem<Rect *, void, GameCrimePatrol> CPScriptFunctionRect;
typedef Common::HashMap<Common::String, CPScriptFunctionScene *> CPScriptFunctionSceneMap;
typedef Common::HashMap<Common::String, CPScriptFunctionRect *> CPScriptFunctionRectMap;

class GameCrimePatrol : public Game {

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
	GameCrimePatrol(AlgEngine *vm, const AlgGameDescription *gd);
	~GameCrimePatrol() override;
	void init() override;
	void debugWarpTo(int val);

private:
	Common::Error run() override;
	void registerScriptFunctions();
	void verifyScriptFunctions();
	CPScriptFunctionRect getScriptFunctionRectHit(Common::String name);
	CPScriptFunctionScene getScriptFunctionScene(SceneFuncType type, Common::String name);
	void callScriptFunctionRectHit(Common::String name, Rect *rect);
	void callScriptFunctionScene(SceneFuncType type, Common::String name, Scene *scene);

	CPScriptFunctionRectMap _rectHitFuncs;
	CPScriptFunctionSceneMap _scenePreOps;
	CPScriptFunctionSceneMap _sceneShowMsg;
	CPScriptFunctionSceneMap _sceneInsOps;
	CPScriptFunctionSceneMap _sceneWepDwn;
	CPScriptFunctionSceneMap _sceneScnScr;
	CPScriptFunctionSceneMap _sceneNxtFrm;
	CPScriptFunctionSceneMap _sceneNxtScn;

	// images
	Graphics::Surface *_shotIcon;
	Graphics::Surface *_emptyIcon;
	Graphics::Surface *_liveIcon;
	Graphics::Surface *_deadIcon;
	Graphics::Surface *_difficultyIcon;
	Graphics::Surface *_bulletholeIcon;

	// constants
	const int16 _scenesLevel0[2] = {0x0191, 0};
	const int16 _scenesLevel1[3] = {0x27, 0x01B7, 0};
	const int16 _scenesLevel2[4] = {0x33, 0x01C7, 0x01B8, 0};
	const int16 _scenesLevel3[3] = {0x48, 0x01C8, 0};
	const int16 _scenesLevel4[2] = {0x53, 0};
	const int16 _scenesLevel5[2] = {0x60, 0};
	const int16 _scenesLevel6[2] = {0x82, 0};
	const int16 _scenesLevel7[2] = {0x9B, 0};
	const int16 _scenesLevel8[2] = {0xC7, 0};
	const int16 _scenesLevel9[2] = {0xB6, 0};
	const int16 _scenesLevel10[6] = {0xE0, 0x01BF, 0x01C0, 0x01C1, 0x01C2, 0};
	const int16 _scenesLevel11[3] = {0x0136, 0x01C3, 0};
	const int16 _scenesLevel12[4] = {0x0119, 0x01C5, 0x0131, 0};
	const int16 _scenesLevel13[2] = {0x014C, 0};
	const int16 _scenesLevel14[13] = {0x01B1, 0x01B2, 0x01B3, 0x01B4, 0x01B5, 0x01B6, 0};
	const int16 _scenesLevel15[5] = {0x3B, 0x3C, 0x3F, 0x41, 0};
	const int16 _scenesLevel16[3] = {0x61, 0x65, 0};
	const int16 _scenesLevel17[7] = {0x68, 0x6A, 0x6C, 0x6E, 0x70, 0x72, 0};
	const int16 _scenesLevel18[8] = {0x83, 0x85, 0x87, 0x8A, 0x8C, 0x8F, 0x90, 0};
	const int16 _scenesLevel19[7] = {0x9C, 0x9E, 0xA0, 0xA2, 0xA4, 0xA6, 0};
	const int16 _scenesLevel20[5] = {0xC8, 0xCA, 0xCC, 0xCE, 0};
	const int16 _scenesLevel21[3] = {0xE8, 0xE9, 0};
	const int16 _scenesLevel22[11] = {0xF4, 0xF6, 0xF8, 0xFA, 0xFC, 0xFE, 0x0100, 0x0102, 0x0104, 0x0106, 0};
	const int16 _scenesLevel23[3] = {0x010E, 0x0113, 0};
	const int16 _scenesLevel24[8] = {0x011D, 0x011F, 0x0121, 0x0123, 0x0125, 0x0127, 0x0129, 0};
	const int16 _scenesLevel25[6] = {0x013D, 0x013F, 0x0141, 0x0143, 0x0145, 0};
	const int16 _scenesLevel26[10] = {0x0157, 0x0159, 0x015B, 0x015D, 0x015F, 0x0161, 0x0163, 0x0165, 0x0167, 0};

	const int16 *_levelScenes[27] = {_scenesLevel0, _scenesLevel1, _scenesLevel2, _scenesLevel3, _scenesLevel4, _scenesLevel5, _scenesLevel6,
									 _scenesLevel7, _scenesLevel8, _scenesLevel9, _scenesLevel10, _scenesLevel11, _scenesLevel12, _scenesLevel13,
									 _scenesLevel14, _scenesLevel15, _scenesLevel16, _scenesLevel17, _scenesLevel18, _scenesLevel19, _scenesLevel20,
									 _scenesLevel21, _scenesLevel22, _scenesLevel23, _scenesLevel24, _scenesLevel25, _scenesLevel26};

	const int16 _diedScenesStage0[5] = {0x30, 0x31, 0x47, 0x51, 0};
	const int16 _diedScenesStage1[7] = {0x5E, 0x5F, 0x7E, 0x7F, 0x98, 0x99, 0};
	const int16 _diedScenesStage2[4] = {0xB2, 0xC5, 0xDB, 0};
	const int16 _diedScenesStage3[7] = {0x0115, 0x0116, 0x0135, 0x014A, 0x0175, 0x0190, 0};
	const int16 _diedScenesStage4[7] = {0x0115, 0x0116, 0x0135, 0x014A, 0x0175, 0x0190, 0};

	const int16 *_diedScenesByStage[5] = {_diedScenesStage0, _diedScenesStage1, _diedScenesStage2, _diedScenesStage3, _diedScenesStage4};

	const uint16 _deadScenes[5] = {0x32, 0x80, 0xDC, 0x018D, 0x018D};

	const uint16 _stageStartScenes[5] = {0x26, 0x52, 0x9A, 0xDF, 0x014C};

	const int16 _practiceTargetLeft[5] = {0xE1, 0x45, 0xA8, 0x73, 0xE1};
	const int16 _practiceTargetTop[5] = {0x0A, 0x3E, 0x41, 0x6E, 0x6E};
	const int16 _practiceTargetRight[5] = {0x0104, 0x6D, 0xCB, 0x95, 0x0104};
	const int16 _practiceTargetBottom[5] = {0x3D, 0x79, 0x7B, 0xA1, 0xA1};

	// gamestate
	uint16 _gotTo[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int8 _stage = 0;
	int8 _oldStage = -1;
	uint8 _randomSceneCount = 0;
	uint8 _randomMax = 0;
	uint16 _randomMask = 0;
	uint16 _randomPicked = 0;
	uint16 _deathMask = 0;
	int16 _deathPicked = 0;
	uint8 _deathSceneCount = 0;
	uint8 _practiceMask = 0;
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
	void sceneIsoGotToGeneric(uint8 index, uint16 sceneId);

	// Script functions: RectHit
	void rectShotMenu(Rect *rect);
	void rectSave(Rect *rect);
	void rectLoad(Rect *rect);
	void rectContinue(Rect *rect);
	void rectStart(Rect *rect);
	void rectTargetPractice(Rect *rect);
	void rectSelectTargetPractice(Rect *rect);
	void rectSelectGangFight(Rect *rect);
	void rectSelectWarehouse(Rect *rect);
	void rectSelectWestcoastSound(Rect *rect);
	void rectSelectDrugDeal(Rect *rect);
	void rectSelectCarRing(Rect *rect);
	void rectSelectBar(Rect *rect);
	void rectSelectBank(Rect *rect);
	void rectSelectCrackHouse(Rect *rect);
	void rectSelectMethLab(Rect *rect);
	void rectSelectAirplane(Rect *rect);
	void rectSelectNukeTransport(Rect *rect);
	void rectSelectAirport(Rect *rect);
	void rectKillInnocentMan(Rect *rect);

	// Script functions: Scene PreOps
	void scenePsoWarehouseGotTo(Scene *scene);
	void scenePsoGangFightGotTo(Scene *scene);
	void scenePsoWestcoastSoundGotTo(Scene *scene);
	void scenePsoDrugDealGotTo(Scene *scene);
	void scenePsoCarRingGotTo(Scene *scene);
	void scenePsoBankGotTo(Scene *scene);
	void scenePsoCrackHouseGotTo(Scene *scene);
	void scenePsoMethLabGotTo(Scene *scene);
	void scenePsoAirplaneGotTo(Scene *scene);
	void scenePsoAirportGotTo(Scene *scene);
	void scenePsoNukeTransportGotTo(Scene *scene);
	void scenePsoPowerPlantGotTo(Scene *scene);

	// Script functions: Scene NxtScn
	void sceneNxtscnGameWon(Scene *scene);
	void sceneNxtscnLoseALife(Scene *scene);
	void sceneNxtscnDidNotContinue(Scene *scene);
	void sceneNxtscnKillInnocentMan(Scene *scene);
	void sceneNxtscnKillInnocentWoman(Scene *scene);
	void sceneNxtscnAfterDie(Scene *scene);
	void sceneNxtscnSelectLanguage1(Scene *scene);
	void sceneNxtscnSelectLanguage2(Scene *scene);
	void sceneNxtscnSelectRookieScenario(Scene *scene);
	void sceneNxtscnSelectUndercoverScenario(Scene *scene);
	void sceneNxtscnSelectSwatScenario(Scene *scene);
	void sceneNxtscnSelectDeltaScenario(Scene *scene);
	void sceneNxtscnInitRandomTargetPractice(Scene *scene);
	void sceneNxtscnContinueTargetPractice(Scene *scene);
	void sceneNxtscnFinishGangFight(Scene *scene);
	void sceneNxtscnFinishWestcoastSound(Scene *scene);
	void sceneNxtscnFinishWarehouse(Scene *scene);
	void sceneNxtscnInitRandomWarehouse(Scene *scene);
	void sceneNxtscnContinueWarehouse(Scene *scene);
	void sceneNxtscnFinishDrugDeal(Scene *scene);
	void sceneNxtscnInitRandomCarRingLeader(Scene *scene);
	void sceneNxtscnContinueCarRingLeader1(Scene *scene);
	void sceneNxtscnContinueCarRingLeader2(Scene *scene);
	void sceneNxtscnInitRandomCarRing(Scene *scene);
	void sceneNxtscnContinueCarRing(Scene *scene);
	void sceneNxtscnFinishCarRing(Scene *scene);
	void sceneNxtscnFinishBar(Scene *scene);
	void sceneNxtscnFinishBank(Scene *scene);
	void sceneNxtscnFinishCrackHouse(Scene *scene);
	void sceneNxtscnFinishMethLab(Scene *scene);
	void sceneNxtscnFinishAirplane(Scene *scene);
	void sceneNxtscnFinishAirport(Scene *scene);
	void sceneNxtscnFinishNukeTransport(Scene *scene);
	void sceneNxtscnInitRandomBar(Scene *scene);
	void sceneNxtscnContinueBar(Scene *scene);
	void sceneNxtscnInitRandomBank(Scene *scene);
	void sceneNxtscnContinueBank(Scene *scene);
	void sceneNxtscnInitRandomMethLab(Scene *scene);
	void sceneNxtscnContinueMethLab(Scene *scene);
	void sceneNxtscnPickRandomRapeller(Scene *scene);
	void sceneNxtscnInitRandomAirplane(Scene *scene);
	void sceneNxtscnContinueAirplane(Scene *scene);
	void sceneNxtscnPickRandomAirplaneFront(Scene *scene);
	void sceneNxtscnInitRandomAirport(Scene *scene);
	void sceneNxtscnContinueAirport(Scene *scene);
	void sceneNxtscnInitRandomNukeTransport(Scene *scene);
	void sceneNxtscnContinueNukeTransport(Scene *scene);
	void sceneNxtscnInitRandomPowerplant(Scene *scene);
	void sceneNxtscnContinuePowerplant(Scene *scene);

	// Script functions: Scene WepDwn
	void sceneDefaultWepdwn(Scene *scene);
	void debugDrawPracticeRects();
};

class DebuggerCrimePatrol : public GUI::Debugger {
public:
	DebuggerCrimePatrol(GameCrimePatrol *game);
	bool cmdWarpTo(int argc, const char **argv);
	bool cmdDumpLib(int argc, const char **argv);

private:
	GameCrimePatrol *_game;
};

} // End of namespace Alg

#endif
