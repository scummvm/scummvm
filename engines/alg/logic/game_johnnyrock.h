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

#ifndef ALG_GAME_JOHNNYROCK_H
#define ALG_GAME_JOHNNYROCK_H

#include "common/hashmap.h"
#include "common/rect.h"

#include "gui/debugger.h"

#include "alg/game.h"
#include "alg/scene.h"

namespace Alg {

typedef Common::Functor1Mem<Scene *, void, GameJohnnyRock> JRScriptFunctionScene;
typedef Common::Functor1Mem<Rect *, void, GameJohnnyRock> JRScriptFunctionRect;
typedef Common::Functor1Mem<Common::Point *, void, GameJohnnyRock> JRScriptFunctionPoint;
typedef Common::HashMap<Common::String, JRScriptFunctionScene *> JRScriptFunctionSceneMap;
typedef Common::HashMap<Common::String, JRScriptFunctionRect *> JRScriptFunctionRectMap;
typedef Common::HashMap<Common::String, JRScriptFunctionPoint *> JRScriptFunctionPointMap;

class GameJohnnyRock : public Game {

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
	GameJohnnyRock(AlgEngine *vm, const AlgGameDescription *gd);
	~GameJohnnyRock() override;
	Common::Error run() override;
	void debugWarpTo(int val);

private:
	void init() override;
	void registerScriptFunctions();
	void verifyScriptFunctions();
	JRScriptFunctionPoint getScriptFunctionZonePtrFb(Common::String name);
	JRScriptFunctionRect getScriptFunctionRectHit(Common::String name);
	JRScriptFunctionScene getScriptFunctionScene(SceneFuncType type, Common::String name);
	void callScriptFunctionZonePtrFb(Common::String name, Common::Point *point);
	void callScriptFunctionRectHit(Common::String name, Rect *rect);
	void callScriptFunctionScene(SceneFuncType type, Common::String name, Scene *scene);

	JRScriptFunctionPointMap _zonePtrFb;
	JRScriptFunctionRectMap _rectHitFuncs;
	JRScriptFunctionSceneMap _scenePreOps;
	JRScriptFunctionSceneMap _sceneShowMsg;
	JRScriptFunctionSceneMap _sceneInsOps;
	JRScriptFunctionSceneMap _sceneWepDwn;
	JRScriptFunctionSceneMap _sceneScnScr;
	JRScriptFunctionSceneMap _sceneNxtFrm;
	JRScriptFunctionSceneMap _sceneNxtScn;

	// images
	Common::Array<Graphics::Surface *> *_difficultyIcon;
	Graphics::Surface *_levelIcon;
	Graphics::Surface *_bulletholeIcon;

	// constants
	const int16 _randomRooftop[6] = {2, -4, 0x104, 0x1E, 0x100, 0x102};
	const int16 _randomTheater[9] = {5, -5, 0x111, 0x1E, 0x107, 0x109, 0x10B, 0x10D, 0x10F};
	const int16 _randomAlley[10] = {6, -4, 0, 0x1E, 0x113, 0x115, 0x117, 0x119, 0x11B, 0x11D};
	const int16 _randomFuneralMR[10] = {6, -5, 0x12B, 0x1E, 0x11F, 0x121, 0x123, 0x125, 0x127, 0x129};
	const int16 _randomBookMR[8] = {4, 5, 0, 0x1E, 0x12E, 0x130, 0x132, 0x134};
	const int16 _randomStairway[8] = {4, -3, 0, 0x1E, 0x139, 0x13B, 0x13D, 0x13F};
	const int16 _randomHall[8] = {4, -5, 0, 0x1E, 0x141, 0x143, 0x145, 0x146};
	const int16 _randomWindows[10] = {6, -3, 0, 0x1E, 0x154, 0x156, 0x158, 0x15A, 0x15C, 0x15E};
	const int16 _randomCar[5] = {1, 1, 0, 0, 0x0FE};
	const int16 _randomHall1[5] = {1, 1, 0, 0x1E, 0x148};
	const int16 _randomElevator[5] = {1, 1, 0, 0, 0x14C};
	const int16 _randomBaby[5] = {1, 1, 0, 0, 0x160};
	const int16 *_randomPlaces[6] = {_randomWindows, _randomStairway, _randomCar, _randomHall1, _randomElevator, _randomBaby};
	const int16 *_randomPlacesMR[8] = {_randomBookMR, _randomFuneralMR, _randomAlley, _randomTheater, _randomHall, _randomWindows, _randomHall1, _randomRooftop};

	const int16 _officeTable[5] = {0xA7, 0x9F, 0x9E, 0x0A0, 0x0AD};

	const uint16 _diffPos[4][2] = {{0, 0}, {0xCD, 0x35}, {0xD2, 0x53}, {0xD2, 0x6E}};

	// gamestate
	uint16 _totalDies = 0;
	int16 _gameMoney = 0;
	int16 _oldGameMoney = 0;
	uint16 _ammoAgain = 0;
	uint16 _mapTimeout = 0;
	uint8 _luckyNumber = 0;
	uint8 _thisMap = 0;
	uint16 _clues = 0;
	uint16 _placeBits = 0;
	uint8 _randomCount = 0;
	uint16 _doctorBits = 0;
	uint16 _undertakerBits = 0;
	uint16 _thisClue = 0;
	uint8 _gotThisNumber = 0;
	uint8 _casino = 0;
	uint8 _poolHall = 0;
	uint8 _warehouse = 0;
	uint8 _garage = 0;
	uint8 _office = 0;
	uint8 _casinoType = 0;
	uint8 _poolHallType = 0;
	uint8 _warehouseType = 0;
	uint8 _garageType = 0;
	uint8 _mansion = 0;
	uint8 _inWarehouse = 0;
	uint8 _inOffice = 0;
	uint16 _gotTo = 0;
	uint8 _whoDidIt = 0;
	uint8 _hadGoToMansion = 0;
	uint16 _officeCount = 0;
	uint16 _randomPlaceBits = 0;
	uint8 _maxRandomCount = 0;
	uint16 _gotoAfterRandom = 0;
	uint16 _repeatRandomPlace = 0;
	uint16 _maxRepeat = 0;
	uint16 _gotThisClue = 0;
	uint16 _didContinue = 0;
	uint16 _thisGameTime = 0;
	uint8 _thisDifficulty = 0;
	uint8 _clueTable[4];
	uint8 _combinations[4];
	uint16 _entranceIndex[20];
	const int16 *_randomScenes = nullptr;
	uint8 _randomScenesSavestateIndex = 0;
	uint16 _randomScenesIndex[10];
	Common::String _moneyScene = "";
	uint8 _mgunCnt = 0;
	uint32 _machGunTimer = 0;

	// base functions
	bool fired(Common::Point *point);
	void newGame();
	void resetParams();
	void outShots();
	void doMenu();
	void updateStat();
	void displayScore();
	void showDifficulty(uint8 newDifficulty, bool updateCursor);
	void changeDifficulty(uint8 newDifficulty);
	void updateCursor();
	void updateMouse();
	void moveMouse();
	bool weaponDown();
	bool saveState();
	bool loadState();
	void doMoneySound();

	// misc game functions
	Common::String numToScene(int n);
	uint16 sceneToNum(Common::String sceneName);
	void defaultBullethole(Common::Point *point);
	uint16 pickBits(uint16 *bits, uint8 max);
	uint16 pickRandomPlace(uint8 place);
	void showCombination();
	void shotClue(uint8 clue);
	void shotCombination(uint8 combination, bool combinationB);
	void shotLuckyNumber(uint8 number);

	// Script functions: Zone
	void zoneBullethole(Common::Point *point);

	// Script functions: RectHit
	void rectShotMenu(Rect *rect);
	void rectSave(Rect *rect);
	void rectLoad(Rect *rect);
	void rectContinue(Rect *rect);
	void rectStart(Rect *rect);
	void rectKillInnocent(Rect *rect);
	void rectSelectCasino(Rect *rect);
	void rectSelectPoolhall(Rect *rect);
	void rectSelectWarehouse(Rect *rect);
	void rectSelectGarage(Rect *rect);
	void rectSelectMansion(Rect *rect);
	void rectSelectAmmo(Rect *rect);
	void rectSelectOffice(Rect *rect);
	void rectShotManBust(Rect *rect);
	void rectShotWomanBust(Rect *rect);
	void rectShotBlueVase(Rect *rect);
	void rectShotCat(Rect *rect);
	void rectShotIndian(Rect *rect);
	void rectShotPlate(Rect *rect);
	void rectShotBlueDressPic(Rect *rect);
	void rectShotModernPic(Rect *rect);
	void rectShotMonaLisa(Rect *rect);
	void rectShotGWashington(Rect *rect);
	void rectShotBoyInRedPic(Rect *rect);
	void rectShotCoatOfArms(Rect *rect);
	void rectShotCombinationA0(Rect *rect);
	void rectShotCombinationA1(Rect *rect);
	void rectShotCombinationA2(Rect *rect);
	void rectShotCombinationA3(Rect *rect);
	void rectShotCombinationA4(Rect *rect);
	void rectShotCombinationA5(Rect *rect);
	void rectShotCombinationB0(Rect *rect);
	void rectShotCombinationB1(Rect *rect);
	void rectShotCombinationB2(Rect *rect);
	void rectShotCombinationB3(Rect *rect);
	void rectShotCombinationB4(Rect *rect);
	void rectShotCombinationB5(Rect *rect);
	void rectShotLuckyNumber0(Rect *rect);
	void rectShotLuckyNumber1(Rect *rect);
	void rectShotLuckyNumber2(Rect *rect);
	void rectShotLuckyNumber3(Rect *rect);
	void rectShotLuckyNumber4(Rect *rect);
	void rectShotLuckyNumber5(Rect *rect);

	// Script functions: Scene InsOps
	void sceneIsoShootpast(Scene *scene);
	void sceneIsoShootpastPause(Scene *scene);
	void sceneIsoGotoCasino(Scene *scene);
	void sceneIsoGotoPoolhall(Scene *scene);
	void sceneIsoGotoWarehouse(Scene *scene);
	void sceneIsoInWarehouse2(Scene *scene);
	void sceneIsoInwarehouse3(Scene *scene);
	void sceneIsoGotoGarage(Scene *scene);
	void sceneIsoGotoMansion(Scene *scene);
	void sceneIsoInMansion1(Scene *scene);

	// Script functions: Scene NxtScn
	void sceneNxtscnDied(Scene *scene);
	void sceneNxtscnBombDead(Scene *scene);
	void sceneNxtscnPickUndertaker(Scene *scene);
	void sceneNxtscnCallAttract(Scene *scene);
	void sceneNxtscnPickLuckyNumber(Scene *scene);
	void sceneNxtscnPickMap(Scene *scene);
	void sceneNxtscnPickClue(Scene *scene);
	void sceneNxtscnMapTimeout(Scene *scene);
	void sceneNxtscnEnterCasino(Scene *scene);
	void sceneNxtscnCasinoWhat(Scene *scene);
	void sceneNxtscnEnterPoolhall(Scene *scene);
	void sceneNxtscnPoolhallClue(Scene *scene);
	void sceneNxtscnEnterWarehouse(Scene *scene);
	void sceneNxtscnWarehouseClue(Scene *scene);
	void sceneNxtscnEnterGarage(Scene *scene);
	void sceneNxtscnGarageClue(Scene *scene);
	void sceneNxtscnEnterMansion(Scene *scene);
	void sceneNxtscnGiveClue(Scene *scene);
	void sceneNxtscnPickFlowerMan(Scene *scene);
	void sceneNxtscnRandomScene(Scene *scene);
	void sceneNxtscnEndRandScene(Scene *scene);
	void sceneNxtscnKillInnocent(Scene *scene);

	// Script functions: Scene WepDwn
	void sceneDefaultWepdwn(Scene *scene);
};

class DebuggerJohnnyRock : public GUI::Debugger {
public:
	DebuggerJohnnyRock(GameJohnnyRock *game);
	bool cmdWarpTo(int argc, const char **argv);
	bool cmdDumpLib(int argc, const char **argv);

private:
	GameJohnnyRock *_game;
};

} // End of namespace Alg

#endif
