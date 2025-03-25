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

#ifndef ALG_GAME_SPACEPIRATES_H
#define ALG_GAME_SPACEPIRATES_H

#include "common/hashmap.h"
#include "common/rect.h"

#include "gui/debugger.h"

#include "alg/game.h"
#include "alg/scene.h"

namespace Alg {

typedef Common::Functor1Mem<Scene *, void, GameSpacePirates> SPScriptFunctionScene;
typedef Common::Functor1Mem<Rect *, void, GameSpacePirates> SPScriptFunctionRect;
typedef Common::HashMap<Common::String, SPScriptFunctionScene *> SPScriptFunctionSceneMap;
typedef Common::HashMap<Common::String, SPScriptFunctionRect *> SPScriptFunctionRectMap;

class GameSpacePirates : public Game {

	enum SceneFuncType {
		PREOP = 1,
		SHOWMSG = 2,
		INSOP = 3,
		WEPDWN = 4,
		SCNSCR = 5,
		NXTFRM = 6,
		NXTSCN = 7,
		MISSEDRECTS = 8
	};

public:
	GameSpacePirates(AlgEngine *vm, const AlgGameDescription *gd);
	~GameSpacePirates() override;
	Common::Error run() override;
	void debugWarpTo(int val);

private:
	void init() override;
	void registerScriptFunctions();
	void verifyScriptFunctions();
	SPScriptFunctionRect getScriptFunctionRectHit(Common::String name);
	SPScriptFunctionScene getScriptFunctionScene(SceneFuncType type, Common::String name);
	void callScriptFunctionRectHit(Common::String name, Rect *rect);
	void callScriptFunctionScene(SceneFuncType type, Common::String name, Scene *scene);

	SPScriptFunctionRectMap _rectHitFuncs;
	SPScriptFunctionSceneMap _scenePreOps;
	SPScriptFunctionSceneMap _sceneShowMsg;
	SPScriptFunctionSceneMap _sceneInsOps;
	SPScriptFunctionSceneMap _sceneWepDwn;
	SPScriptFunctionSceneMap _sceneScnScr;
	SPScriptFunctionSceneMap _sceneNxtFrm;
	SPScriptFunctionSceneMap _sceneNxtScn;
	SPScriptFunctionSceneMap _sceneMissedRects;

	// images
	Graphics::Surface *_shotIcon;
	Graphics::Surface *_emptyIcon;
	Graphics::Surface *_deadIcon;
	Graphics::Surface *_liveIcon1;
	Graphics::Surface *_liveIcon2;
	Graphics::Surface *_liveIcon3;
	Graphics::Surface *_difficultyIcon;
	Graphics::Surface *_bulletholeIcon;

	// constants

	// gamestate
	bool _gameLoaded = false;
	int8 _livesLoaded = 0;
	uint16 _shotsLoaded = 0;
	int32 _scoreLoaded = 0;
	uint8 _difficultyLoaded = 0;
	bool _nextSceneFound = false;
	bool _playerDied = false;
	int16 _randomPicked = -1;
	uint32 _lastExtraLifeScore = 0;
	uint16 _randomScenesValues[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	uint8 _randomScenesUsed[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	uint8 _maxRandom = 0;
	uint8 _randomCount = 0;
	uint8 _randomCountAsteroids = 0;
	uint8 _randomCountMidship = 0;
	uint8 _miscRoomsCount = 0;
	uint16 _pickedMiscRooms = 0;
	uint16 _gotTo = 0;
	int8 _currentWorld = 0;
	uint16 _worldGotTo[4] = {0, 0, 0, 0};
	bool _worldDone[4] = {false, false, false, false};
	bool _selectedAWorld = false;
	uint16 _selectedWorldStart = 0;
	uint16 _sceneBeforeFlyingSkulls = 0;
	uint8 _shotGrinReaperCount = 0;
	uint16 _clue = 0;
	uint8 _shotColor = 0;
	uint8 _shotDirection = 0;
	uint8 _crystalsShot = 0;
	uint8 _crystalState = 0;
	uint16 _pickedStartSplitter = 0;
	bool _targetPracticeReset = false;

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
	void playErrorSound();
	void displayShotFiredImage();
	void displayShotFiredImage(Common::Point *point);
	void displayShotLine(uint16 startX, uint16 startY, uint16 endX, uint16 endY);
	void displayMultipleShotLines();
	void enableVideoFadeIn();
	uint16 sceneToNumber(Scene *scene);
	uint16 randomUnusedScene(uint8 max);
	uint16 randomNumberInRange(uint16 min, uint16 max);
	uint16 pickCrystalScene(uint16 scene1, uint16 scene2, uint16 scene3);

	// Script functions: RectHit
	void rectShotMenu(Rect *rect);
	void rectSave(Rect *rect);
	void rectLoad(Rect *rect);
	void rectContinue(Rect *rect);
	void rectStart(Rect *rect);
	void rectEasy(Rect *rect);
	void rectAverage(Rect *rect);
	void rectHard(Rect *rect);
	void rectDefault(Rect *rect);
	void rectKillInnocentPerson(Rect *rect);
	void rectContinueJunkRings(Rect *rect);
	void rectShotGrinReaper(Rect *rect);
	void rectShowMadDog(Rect *rect);
	void rectPottWorldShowCrystal(Rect *rect);
	void rectShotLeft(Rect *rect);
	void rectShotRight(Rect *rect);
	void rectShotGold(Rect *rect);
	void rectShotSilver(Rect *rect);
	void rectSelectedDuneWorld(Rect *rect);
	void rectSelectedJunkWorld(Rect *rect);
	void rectSelectedDragonsTeethWorld(Rect *rect);
	void rectSelectedVolcanoWorld(Rect *rect);
	void rectShotRedDeathGrip(Rect *rect);
	void rectShotBlueDeathGrip(Rect *rect);
	void rectShotGreenDeathGrip(Rect *rect);
	void rectShotYellow(Rect *rect);
	void rectShotBlue(Rect *rect);
	void rectShotRedCrystal(Rect *rect);
	void rectShotBlueCrystal(Rect *rect);
	void rectShotGreenCrystal(Rect *rect);
	void rectShotBlackDragon1(Rect *rect);
	void rectShotBlackDragon2(Rect *rect);
	void rectShotBlackDragon3(Rect *rect);
	void rectDoFlyingSkull(Rect *rect);
	void rectSkipScene(Rect *rect);
	void rectHitPirateShip(Rect *rect);

	// Script functions: Scene PreOps
	void scenePsoFadeInVideo(Scene *scene);
	void scenePsoSetGotTo(Scene *scene);
	void scenePsoSetGotToNoFadeIn(Scene *scene);
	void scenePsoSetWorldGotTo(Scene *scene);

	// Script functions: Scene InsOps
	void sceneIsoPickAWorld(Scene *scene);
	void sceneIsoSetWorldGotTo(Scene *scene);

	// Script functions: Scene NxtScn
	void sceneNxtscnGotChewedOut(Scene *scene);
	void sceneNxtscnRestartFromLast(Scene *scene);
	void sceneNxtscnPlayerDied(Scene *scene);
	void sceneNxtscnMiscRooms1(Scene *scene);
	void sceneNxtscnPickDungeonClue(Scene *scene);
	void sceneNxtscnContinueDungeonClue(Scene *scene);
	void sceneNxtscnStartMidshipRandomScene(Scene *scene);
	void sceneNxtscnContinueMidshipRandomScene(Scene *scene);
	void sceneNxtscnShowDeathGripBeamColor(Scene *scene);
	void sceneNxtscnSelectAsteroids(Scene *scene);
	void sceneNxtscnAsteroidsDone(Scene *scene);
	void sceneNxtscnDoFlyingSkulls(Scene *scene);
	void sceneNxtscnDidFlyingSkulls(Scene *scene);
	void sceneNxtscnShowWhichStartSplitter(Scene *scene);
	void sceneNxtscnGotoSelectedWorld(Scene *scene);
	void sceneNxtscnStartVolcanoPopup(Scene *scene);
	void sceneNxtscnContinueVolcanoPopup(Scene *scene);
	void sceneNxtscnGiveFalinaClue(Scene *scene);
	void sceneNxtscnCheckFalinaClues(Scene *scene);
	void sceneNxtscnSetupFalinaTargetPractice(Scene *scene);
	void sceneNxtscnContinueFalinaTargetPractice(Scene *scene);
	void sceneNxtscnStartDunePopup(Scene *scene);
	void sceneNxtscnContinueDunePopup(Scene *scene);
	void sceneNxtscnPottOrPanShoots(Scene *scene);
	void sceneNxtscnSetupPottTargetPractice(Scene *scene);
	void sceneNxtscnContinuePottTargetPractice(Scene *scene);
	void sceneNxtscnStartDragonsTeethPopup(Scene *scene);
	void sceneNxtscnContinueDragonsTeethPopup(Scene *scene);
	void sceneNxtscnGrinReaperClue(Scene *scene);
	void sceneNxtscnStartGrinReaper(Scene *scene);
	void sceneNxtscnContinueGrinReaper(Scene *scene);
	void sceneNxtscnGrinTargetPractice(Scene *scene);
	void sceneNxtscnContinueGrinTargetPractice(Scene *scene);
	void sceneNxtscnStartJunkWorld(Scene *scene);
	void sceneNxtscnContinueJunkWorld(Scene *scene);
	void sceneNxtscnStartJunkRings(Scene *scene);
	void sceneNxtscnShowJunkWorldCrystal(Scene *scene);
	void sceneNxtscnStartJunkWorldTargetPractice(Scene *scene);
	void sceneNxtscnContinueJunkWorldTargetPractice(Scene *scene);
	void sceneNxtscnAreAllWorldsDone(Scene *scene);
	void sceneNxtscnStartPracticePirateShip(Scene *scene);
	void sceneNxtscnMorePracticePirateShip(Scene *scene);
	void sceneNxtscnPlayerWon(Scene *scene);

	// Script functions: MissedRect
	void sceneMissedRectsDefault(Scene *scene);
	void sceneMissedRectsMissedPirateShip(Scene *scene);

	// Script functions: Scene WepDwn
	void sceneDefaultWepdwn(Scene *scene);

	// Script functions: Scene ScnScr
	void sceneDefaultScnscr(Scene *scene);
};

class DebuggerSpacePirates : public GUI::Debugger {
public:
	DebuggerSpacePirates(GameSpacePirates *game);
	bool cmdWarpTo(int argc, const char **argv);
	bool cmdDumpLib(int argc, const char **argv);

private:
	GameSpacePirates *_game;
};

} // End of namespace Alg

#endif
