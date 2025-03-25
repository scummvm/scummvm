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

#ifndef ALG_GAME_MADDOG2_H
#define ALG_GAME_MADDOG2_H

#include "common/hashmap.h"
#include "common/rect.h"

#include "gui/debugger.h"

#include "alg/game.h"
#include "alg/scene.h"

namespace Alg {

typedef Common::Functor1Mem<Scene *, void, GameMaddog2> MD2ScriptFunctionScene;
typedef Common::Functor1Mem<Rect *, void, GameMaddog2> MD2ScriptFunctionRect;
typedef Common::Functor1Mem<Common::Point *, void, GameMaddog2> MD2ScriptFunctionPoint;
typedef Common::HashMap<Common::String, MD2ScriptFunctionScene *> MD2ScriptFunctionSceneMap;
typedef Common::HashMap<Common::String, MD2ScriptFunctionRect *> MD2ScriptFunctionRectMap;
typedef Common::HashMap<Common::String, MD2ScriptFunctionPoint *> MD2ScriptFunctionPointMap;

class GameMaddog2 : public Game {

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
	GameMaddog2(AlgEngine *vm, const AlgGameDescription *gd);
	~GameMaddog2() override;
	Common::Error run() override;
	void debugWarpTo(int val);

private:
	void init() override;
	void registerScriptFunctions();
	void verifyScriptFunctions();
	MD2ScriptFunctionPoint getScriptFunctionZonePtrFb(Common::String name);
	MD2ScriptFunctionRect getScriptFunctionRectHit(Common::String name);
	MD2ScriptFunctionScene getScriptFunctionScene(SceneFuncType type, Common::String name);
	void callScriptFunctionZonePtrFb(Common::String name, Common::Point *point);
	void callScriptFunctionRectHit(Common::String name, Rect *rect);
	void callScriptFunctionScene(SceneFuncType type, Common::String name, Scene *scene);

	MD2ScriptFunctionPointMap _zonePtrFb;
	MD2ScriptFunctionRectMap _rectHitFuncs;
	MD2ScriptFunctionSceneMap _scenePreOps;
	MD2ScriptFunctionSceneMap _sceneShowMsg;
	MD2ScriptFunctionSceneMap _sceneInsOps;
	MD2ScriptFunctionSceneMap _sceneWepDwn;
	MD2ScriptFunctionSceneMap _sceneScnScr;
	MD2ScriptFunctionSceneMap _sceneNxtFrm;
	MD2ScriptFunctionSceneMap _sceneNxtScn;

	// images
	Graphics::Surface *_shotIcon;
	Graphics::Surface *_emptyIcon;
	Graphics::Surface *_liveIcon;
	Graphics::Surface *_deadIcon;
	Graphics::Surface *_reloadIcon;
	Graphics::Surface *_drawIcon;
	Graphics::Surface *_knifeIcon;
	Graphics::Surface *_bulletholeIcon;

	// constants
	const int16 _sbClue[3] = {0x67, 0x68, 0x69};
	const int16 _bbClue[3] = {0x47, 0x49, 0x48};
	const int16 _tpClue[3] = {0x84, 0x85, 0x86};
	const int16 _sbScenes[14] = {0x4A, 0x50, 0xB8, 0x55, 0x57, 0x66, 0xBE, 0x94, 0x87, 0x93, 0xCD, 0x95, 0xE3, -1};
	const int16 _bbScenes[14] = {0x33, 0x39, 0xB4, 0x41, 0x43, 0x46, 0xA2, 0x8D, 0x87, 0x8F, 0xCD, 0x8E, 0xE3, -1};
	const int16 _tpScenes[14] = {0x6A, 0xC3, 0x76, 0x81, 0xAD, 0x83, 0xDC, 0x92, 0x87, 0x90, 0xCD, 0x91, 0xE3, -1};
	const int16 _villageScenes[6] = {0x58, 0x5A, 0x5C, 0x5E, 0x60, 0x62};
	const int16 _cowboyScenes[7] = {0xCD, 0xCF, 0xD2, 0xD4, 0xD6, 0xD8, 0xDA};
	const int16 _farmyardScenes[4] = {0xDC, 0xDE, 0x129, 0xE1};
	const int16 _caveScenes[5] = {0xE4, 0xE6, 0xE8, 0xEA, 0xEC};
	const int16 _ezShootOuts[5] = {0xAB, 0xBC, 0xC5, 0xC7, 0xCB};
	const int16 _shootOuts[6] = {0xAB, 0xBC, 0xC5, 0xC7, 0xC9, 0xCB};
	const int16 _ggScenes[3] = {0x87, 0x88, 0x89};
	const int16 _dieScenes[6] = {0x96, 0x97, 0x98, 0x9B, 0x9F, 0xA1};

	const uint16 _diffPos[3][2] = {{0x0FA, 0x6E}, {0xFA, 0x8E}, {0xFA, 0xAF}};
	const uint16 _livePos[3][2] = {{0x81, 0xBE}, {0x70, 0xBE}, {0x5F, 0xBE}};
	const uint16 _shotPos[12][2] = {{0x96, 0xBD}, {0x9A, 0xBD}, {0x9E, 0xBD}, {0x0A2, 0xBD}, {0x0A6, 0xBD}, {0x0AA, 0xBD}, {0x0AE, 0xBD}, {0x0B2, 0xBD}, {0x0B6, 0xBD}, {0x0BA, 0xBD}, {0x0BE, 0xBD}, {0x0C2, 0xBD}};

	// gamestate
	uint16 _dieBits = 0;
	uint16 _gotTo = 0;
	uint16 _sbGotTo = 0;
	uint16 _bbGotTo = 0;
	uint16 _tpGotTo = 0;
	bool _hadSkull = false;
	int16 _thisGuide = 0;
	uint16 _doneGuide = 0;
	uint16 _shootOutBits = 0;
	int16 _lastShootOut = 0;
	uint16 _startLives = 3;
	bool _wasAShootout = false;
	bool _shootoutFromDie = false;
	uint16 _whichPadre = 0;
	uint16 _whichGatlingGun = 0;
	bool _inShootout = false;
	uint16 _pickMask = 0;
	uint16 _lastPick = 0;
	uint16 _placeBits = 0;
	uint16 _randomCount = 0;
	uint16 _shootOutCnt = 0;
	uint16 _totalDies = 0;

	// base functions
	void newGame();
	void resetParams();
	void doMenu();
	void updateStat();
	void changeDifficulty(uint8 newDifficulty);
	void showDifficulty(uint8 newDifficulty, bool updateCursor);
	void updateCursor();
	void updateMouse();
	void moveMouse();
	void displayScore();
	bool weaponDown();
	bool saveState();
	bool loadState();

	// misc game functions
	Common::String numToScene(int n);
	uint16 sceneToNum(Common::String sceneName);
	void defaultBullethole(Common::Point *point);
	uint16 die();
	uint16 pickBits(uint16 *bits, uint8 max);
	uint16 pickShootout();
	void nextSB();
	void nextBB();
	void nextTP();
	void ggPickMan();
	void genericNext();
	void playerWon();

	// Script functions: Zone
	void zoneBullethole(Common::Point *point);
	void zoneSkullhole(Common::Point *point);

	// Script functions: RectHit
	void rectShotmenu(Rect *rect);
	void rectSave(Rect *rect);
	void rectLoad(Rect *rect);
	void rectContinue(Rect *rect);
	void rectStart(Rect *rect);
	void rectSkull(Rect *rect);
	void rectKillInnocentMan(Rect *rect);
	void rectKillInnocentWoman(Rect *rect);
	void rectSelectBeaver(Rect *rect);
	void rectSelectBonnie(Rect *rect);
	void rectSelectProfessor(Rect *rect);
	void rectShotAmmo(Rect *rect);
	void rectShotGin(Rect *rect);
	void rectShotLantern(Rect *rect);
	void rectShootSkull(Rect *rect);

	// Script functions: Scene PreOps
	void scenePsoShootout(Scene *scene);
	void scenePsoMDShootout(Scene *scene);

	// Script functions: Scene InsOps
	void sceneIsoShootpast(Scene *scene);
	void sceneIsoShootpastPause(Scene *scene);
	void sceneIsoStagecoach(Scene *scene);
	void sceneIsoDifferentPadres(Scene *scene);
	void sceneIsoDifferentPadresPause(Scene *scene);
	void sceneIsoDontPopNext(Scene *scene);
	void sceneIsoGetIntoRock(Scene *scene);
	void sceneIsoBenAtCave(Scene *scene);
	void sceneIsoSkullAtCave(Scene *scene);
	void sceneIsoStartOfTrain(Scene *scene);
	void sceneIsoMission(Scene *scene);
	void sceneIsoMDShootout(Scene *scene);
	void sceneIsoStartOfBoardinghouse(Scene *scene);
	void sceneIsoDontContinue(Scene *scene);
	void sceneIsoDoShootout(Scene *scene);

	// Script functions: Scene NxtScn
	void sceneDefaultNxtscn(Scene *scene);
	void sceneNxtscnDied(Scene *scene);
	void sceneNxtscnKillInnocentMan(Scene *scene);
	void sceneNxtscnKillInnocentWoman(Scene *scene);
	void sceneNxtscnKillGuide(Scene *scene);
	void sceneNxtscnShootSkull(Scene *scene);
	void sceneNxtscnCallAttract(Scene *scene);
	void sceneNxtscnPickUndertaker(Scene *scene);
	void sceneNxtscnChoosePadre(Scene *scene);
	void sceneNxtscnSelectGuide(Scene *scene);
	void sceneNxtscnSaveBonnie(Scene *scene);
	void sceneNxtscnFinishBonnie(Scene *scene);
	void sceneNxtscnShowGGClue(Scene *scene);
	void sceneNxtscnBBAfterClue(Scene *scene);
	void sceneNxtscnAsFarSheGoes(Scene *scene);
	void sceneNxtscnSaveBeaver(Scene *scene);
	void sceneNxtscnFinishBeaver(Scene *scene);
	void sceneNxtscnToGatlingGunSBClue(Scene *scene);
	void sceneNxtscnToGuideafterClue(Scene *scene);
	void sceneNxtscnToGuideCave(Scene *scene);
	void sceneNxtscnInitRandomVillage(Scene *scene);
	void sceneNxtscnPickVillageScenes(Scene *scene);
	void sceneNxtscnSaveProfessor(Scene *scene);
	void sceneNxtscnFinishProfessor(Scene *scene);
	void sceneNxtscnToGatlingGunTPClue(Scene *scene);
	void sceneNxtscnTPAfterClue(Scene *scene);
	void sceneNxtscnFinishGatlingGun1(Scene *scene);
	void sceneNxtscnFinishGuyAtGG(Scene *scene);
	void sceneNxtscnFinishGatlingGun2(Scene *scene);
	void sceneNxtscnHowWeDid(Scene *scene);
	void sceneNxtscnPlayerWon(Scene *scene);
	void sceneNxtscnBackToNextGuide(Scene *scene);
	void sceneNxtscnFinishGenericScene(Scene *scene);
	void sceneNxtscnInitRandomCowboys(Scene *scene);
	void sceneNxtscnToCowboyScenes(Scene *scene);
	void sceneNxtscnInitRandomFarmyard(Scene *scene);
	void sceneNxtscnToFarmyardScenes(Scene *scene);
	void sceneNxtscnInitRandomCave(Scene *scene);
	void sceneNxtscnToCaveScenes(Scene *scene);
	void sceneNxtscnPickSkullAtCave(Scene *scene);
	void sceneNxtscnDrawGun(Scene *scene);

	// Script functions: Scene WepDwn
	void sceneDefaultWepdwn(Scene *scene);
};

class DebuggerMaddog2 : public GUI::Debugger {
public:
	DebuggerMaddog2(GameMaddog2 *game);
	bool cmdWarpTo(int argc, const char **argv);
	bool cmdDumpLib(int argc, const char **argv);

private:
	GameMaddog2 *_game;
};

} // End of namespace Alg

#endif
