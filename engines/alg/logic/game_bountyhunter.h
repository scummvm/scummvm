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

#ifndef ALG_GAME_BOUNTYHUNTER_H
#define ALG_GAME_BOUNTYHUNTER_H

#include "common/hashmap.h"
#include "common/rect.h"

#include "gui/debugger.h"

#include "alg/game.h"
#include "alg/scene.h"

namespace Alg {

typedef Common::Functor1Mem<Scene *, void, GameBountyHunter> BHScriptFunctionScene;
typedef Common::Functor1Mem<Rect *, void, GameBountyHunter> BHScriptFunctionRect;
typedef Common::Functor1Mem<Common::Point *, void, GameJohnnyRock> BHScriptFunctionPoint;
typedef Common::HashMap<Common::String, BHScriptFunctionScene *> BHScriptFunctionSceneMap;
typedef Common::HashMap<Common::String, BHScriptFunctionRect *> BHScriptFunctionRectMap;
typedef Common::HashMap<Common::String, BHScriptFunctionPoint *> BHScriptFunctionPointMap;

class GameBountyHunter : public Game {

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
	GameBountyHunter(AlgEngine *vm, const AlgGameDescription *gd);
	~GameBountyHunter() override;
	Common::Error run() override;
	void debugWarpTo(int val);

private:
	void init() override;
	void registerScriptFunctions();
	void verifyScriptFunctions();
	BHScriptFunctionRect getScriptFunctionRectHit(Common::String name);
	BHScriptFunctionScene getScriptFunctionScene(SceneFuncType type, Common::String name);
	void callScriptFunctionRectHit(Common::String name, Rect *rect);
	void callScriptFunctionScene(SceneFuncType type, Common::String name, Scene *scene);

	BHScriptFunctionRectMap _rectHitFuncs;
	BHScriptFunctionSceneMap _scenePreOps;
	BHScriptFunctionSceneMap _sceneShowMsg;
	BHScriptFunctionSceneMap _sceneInsOps;
	BHScriptFunctionSceneMap _sceneWepDwn;
	BHScriptFunctionSceneMap _sceneScnScr;
	BHScriptFunctionSceneMap _sceneNxtFrm;
	BHScriptFunctionSceneMap _sceneNxtScn;

	// images
	Graphics::Surface *_shotIcon;
	Graphics::Surface *_emptyIcon;
	Graphics::Surface *_liveIcon;
	Graphics::Surface *_deadIcon;
	Graphics::Surface *_diffIcon;
	Graphics::Surface *_bulletholeIcon;
	Graphics::Surface *_playersIcon1;
	Graphics::Surface *_playersIcon2;
	Graphics::Surface *_textScoreIcon;
	Graphics::Surface *_textMenuIcon;
	Graphics::Surface *_textBlackBarIcon;
	Common::Array<Graphics::Surface *> *_bagsIcons;
	Common::Array<Graphics::Surface *> *_shotgun;

	// sounds
	Audio::SeekableAudioStream *_shotgunSound = nullptr;

	// constants
	uint16 _randomHarry1[7] = {0x01B9, 0x01B7, 0x01B5, 0x01B3, 0x01AF, 0x01AD, 0};
	uint16 _randomHarry2[6] = {0x0194, 0x0190, 0x018E, 0x018C, 0};
	uint16 _randomDan1[5] = {0x0173, 0x0171, 0x016F, 0x016D, 0};
	uint16 _randomDan1TwoPlayer[6] = {0x0173, 0x0171, 0x016F, 0x016D, 0x016B, 0};
	uint16 _randomDan2[7] = {0x0165, 0x0163, 0x0161, 0x015F, 0x015D, 0x015B, 0};
	uint16 _randomLoco1[4] = {0xF7, 0xF5, 0xF3, 0};
	uint16 _randomLoco2[3] = {0xED, 0xEB, 0};
	uint16 _randomKid1[4] = {0xBA, 0xB7, 0xB5, 0};
	uint16 _randomKid2[4] = {0xB1, 0xAE, 0xAC, 0};

	uint16 *_randomScenes[8] = {_randomHarry1, _randomHarry2, _randomDan1, _randomDan2, _randomLoco1, _randomLoco2, _randomKid1, _randomKid2};
	const uint8 _randomScenesPicks[8] = {6, 6, 4, 7, 3, 2, 5, 5};

	const uint8 _subLevelOrder[96] = {0, 1, 2, 4, 0, 0, 0, 1, 3, 4, 0, 0, 0, 2, 3, 4, 0, 0, 0, 1, 2, 3, 4, 0,
									  0, 1, 2, 4, 0, 0, 0, 1, 3, 4, 0, 0, 0, 2, 3, 4, 0, 0, 0, 1, 2, 3, 4, 0,
									  0, 1, 3, 4, 0, 0, 0, 2, 3, 4, 0, 0, 1, 2, 3, 4, 0, 0, 0, 1, 2, 3, 4, 0,
									  0, 1, 2, 3, 4, 0, 0, 1, 2, 3, 4, 0, 0, 1, 2, 3, 4, 0, 0, 1, 2, 3, 4, 0};
	const uint16 _subLevelSceneIds[20] = {0x01BE, 0x017A, 0x01A2, 0x0198, 0x0183, 0x0178, 0x0167, 0x0159, 0x014B, 0x0147,
										  0xF1, 0xE1, 0xFF, 0xD8, 0xD0, 0x9B, 0xA8, 0x86, 0xBF, 0x74};

	const uint16 _clueLevels[4] = {0x017A, 0x013B, 0xC2, 0x68};
	const uint8 _mainLevelMasks[5] = {2, 4, 8, 0x10, 0x80};
	const uint8 _gunfightCountDown[15] = {5, 4, 3, 3, 3, 4, 3, 3, 2, 1, 3, 2, 2, 2, 1};

	// const uint16 _firstSceneInScenario[4] = {4, 0x36, 0x36, 0x66};
	const uint16 _moneyScenes[4] = {0x017D, 0x013C, 0xC3, 0x69};
	const uint16 _gunfightScenarios[18] = {0x0116, 0x0118, 0x011B, 0x011D, 0x011F, 0x0121, 0x0123, 0x0125, 0x0127,
										   0x0129, 0x012B, 0x012D, 0x012F, 0x0131, 0x0133, 0x0135, 0x0137, 0x0139};
	const uint16 _innocentScenarios[5] = {0x0110, 0x010F, 0x010C, 0x010B, 0};
	const uint16 _deathScenarios[9] = {0x0100, 0x0101, 0x0102, 0x0103, 0x0104, 0x0105, 0x0106, 0x0107, 0};
	const uint16 _onePlayerOfTwoDead[2] = {0x0109, 0x010A};
	const uint16 _allPlayersDead = 0x108;

	// gamestate
	uint16 _restartScene = 0;
	uint8 _numPlayers = 1;
	uint8 _player = 0;
	uint8 _playerLives[2] = {0, 0};
	uint8 _playerGun[2] = {1, 1};
	uint8 _playerShots[2] = {0, 0};
	uint32 _playerScore[2] = {0, 0};

	uint8 _currentLevel = 0;
	uint16 _currentSubLevelSceneId = 0;
	uint8 _numLevelsDone = 0;
	uint8 _levelDoneMask = 0;
	uint8 _numSubLevelsDone = 0;

	// uint16 _usedScenes = 0;
	// int16 _lastPick = -1;
	// int16 _initted = 0;
	// int16 _sceneCount = 0;

	uint16 *_randomSceneList;
	uint8 _randomMax = 0;
	uint16 _randomMask = 0;
	int16 _randomPicked = 0;
	uint8 _randomSceneCount = 0;
	bool _gunfightInitialized = false;
	uint16 _gunfightMask = 0;
	int16 _gunfightPicked = 0;
	uint8 _gunfightSceneCount = 0;
	bool _innocentInitialized = false;
	uint16 _innocentMask = 0;
	int16 _innocentPicked = 0;
	uint8 _innocentSceneCount = 0;
	bool _deathInitialized = false;
	uint16 _deathMask = 0;
	int16 _deathPicked = 0;
	uint8 _deathSceneCount = 0;

	uint8 _continuesUsed = 0;
	bool _wounded = false;
	uint16 _mainWounds = 0;
	int8 _gunfightCount = 0;
	bool _given = false;
	uint32 _firstDrawFrame = 0;
	uint8 _count = 0;

	uint8 _unk_2ADA6 = 0;

	// base functions
	void newGame();
	void doMenu();
	void updateCursor();
	void updateMouse();
	void moveMouse();
	void displayLivesLeft(uint8 player);
	void displayScores(uint8 player);
	void displayShotsLeft(uint8 player);
	bool weaponDown();
	bool saveState();
	bool loadState();

	// misc game functions
	void setNextScene(uint16 sceneId);
	void displayShotFiredImage(Common::Point *point);
	void enableVideoFadeIn();
	void iconShotgun();
	void iconReset();
	uint16 beginLevel(uint8 levelNumber);
	uint16 pickRandomScene(uint16 *sceneList, uint8 max);
	uint16 pickGunfightScene();
	uint16 pickInnocentScene();
	uint16 pickDeathScene();
	uint16 timeForGunfight();
	void waitingForShootout(uint32 drawFrame);
	void doShotgunSound();

	// Script functions: RectHit
	void rectShotMenu(Rect *rect);
	void rectSave(Rect *rect);
	void rectLoad(Rect *rect);
	void rectContinue(Rect *rect);
	void rectStart(Rect *rect);
	void rectTogglePlayers(Rect *rect);
	void rectHitIconJug(Rect *rect);
	void rectHitIconLantern(Rect *rect);
	void rectHitIconSkull(Rect *rect);
	void rectHitIconWheel(Rect *rect);
	void rectHitSelectHarry(Rect *rect);
	void rectHitSelectDan(Rect *rect);
	void rectHitSelectLoco(Rect *rect);
	void rectHitSelectKid(Rect *rect);
	void rectHitKillMan(Rect *rect);
	void rectHitGiveShotgun(Rect *rect);
	void rectHitKill3(Rect *rect);
	void rectHitCheckShotgun(Rect *rect);
	void rectHitCheater(Rect *rect);

	// Script functions: Scene PreOps
	void scenePsoShootout(Scene *scene);
	void scenePsoWoundedMain(Scene *scene);
	void scenePsoGunfightSetup(Scene *scene);
	void scenePsoLoseALife(Scene *scene);
	void scenePsoSetupNdRandom1(Scene *scene);
	void scenePsoSetCurrentScene(Scene *scene);

	// Script functions: Scene InsOps
	void sceneIsoShootout(Scene *scene);
	void sceneIsoGivemoney(Scene *scene);

	// Script functions: Scene NxtScn
	void sceneNxtscnLoseALife(Scene *scene);
	void sceneNxtscnContinueGame(Scene *scene);
	void sceneNxtscnDidNotContinue(Scene *scene);
	void sceneNxtscnKillInnocentMan(Scene *scene);
	void sceneNxtscnKillInnocentWoman(Scene *scene);
	void sceneNxtscnAfterDie(Scene *scene);
	void sceneNxtscnGotoLevelSelect(Scene *scene);
	void sceneNxtscnContinueRandom(Scene *scene);
	void sceneNxtscnInitRandomHarry1(Scene *scene);
	void sceneNxtscnInitRandomHarry2(Scene *scene);
	void sceneNxtscnInitRandomDan1(Scene *scene);
	void sceneNxtscnInitRandomDan2(Scene *scene);
	void sceneNxtscnInitRandomLoco1(Scene *scene);
	void sceneNxtscnInitRandomLoco2(Scene *scene);
	void sceneNxtscnInitRandomKid1(Scene *scene);
	void sceneNxtscnInitRandomKid2(Scene *scene);
	void sceneNxtscnNextSubLevel(Scene *scene);
	void sceneNxtscnGotoBadGuy(Scene *scene);
	void sceneNxtscnAutoSelectLevel(Scene *scene);
	void sceneNxtscnSelectScenario(Scene *scene);
	void sceneNxtscnFinishScenario(Scene *scene);
	void sceneNxtscnGameWon(Scene *scene);
	void sceneNxtscnKilledMain(Scene *scene);
	void sceneNxtscnWoundedMain(Scene *scene);
	void sceneNxtscnEndLevel(Scene *scene);
	void sceneNxtscnEndGame(Scene *scene);
	void sceneNxtscnDoBreakoutMains(Scene *scene);
	void sceneNxtscnDiedRefed(Scene *scene);
	void sceneNxtscnGiveShotgun(Scene *scene);
	void sceneNxtscnCheck2Players(Scene *scene);

	// Script functions: Scene WepDwn
	void sceneDefaultWepdwn(Scene *scene);
};

class DebuggerBountyHunter : public GUI::Debugger {
public:
	DebuggerBountyHunter(GameBountyHunter *game);
	bool cmdWarpTo(int argc, const char **argv);
	bool cmdDumpLib(int argc, const char **argv);

private:
	GameBountyHunter *_game;
};

} // End of namespace Alg

#endif
