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

#ifndef ALG_GAME_MADDOG_H
#define ALG_GAME_MADDOG_H

#include "common/hashmap.h"
#include "common/rect.h"

#include "gui/debugger.h"

#include "alg/game.h"
#include "alg/scene.h"

namespace Alg {

typedef Common::Functor1Mem<Scene *, void, GameMaddog> MDScriptFunctionScene;
typedef Common::Functor1Mem<Rect *, void, GameMaddog> MDScriptFunctionRect;
typedef Common::Functor1Mem<Common::Point *, void, GameMaddog> MDScriptFunctionPoint;
typedef Common::HashMap<Common::String, MDScriptFunctionScene *> MDScriptFunctionSceneMap;
typedef Common::HashMap<Common::String, MDScriptFunctionRect *> MDScriptFunctionRectMap;
typedef Common::HashMap<Common::String, MDScriptFunctionPoint *> MDScriptFunctionPointMap;

class GameMaddog : public Game {

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
	GameMaddog(AlgEngine *vm, const AlgGameDescription *gd);
	~GameMaddog() override;
	Common::Error run() override;
	void debugWarpTo(int val);

private:
	void init() override;
	void registerScriptFunctions();
	void verifyScriptFunctions();
	MDScriptFunctionPoint getScriptFunctionZonePtrFb(Common::String name);
	MDScriptFunctionRect getScriptFunctionRectHit(Common::String name);
	MDScriptFunctionScene getScriptFunctionScene(SceneFuncType type, Common::String name);
	void callScriptFunctionZonePtrFb(Common::String name, Common::Point *point);
	void callScriptFunctionRectHit(Common::String name, Rect *rect);
	void callScriptFunctionScene(SceneFuncType type, Common::String name, Scene *scene);

	MDScriptFunctionPointMap _zonePtrFb;
	MDScriptFunctionRectMap _rectHitFuncs;
	MDScriptFunctionSceneMap _scenePreOps;
	MDScriptFunctionSceneMap _sceneShowMsg;
	MDScriptFunctionSceneMap _sceneInsOps;
	MDScriptFunctionSceneMap _sceneWepDwn;
	MDScriptFunctionSceneMap _sceneScnScr;
	MDScriptFunctionSceneMap _sceneNxtFrm;
	MDScriptFunctionSceneMap _sceneNxtScn;

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
	const uint16 _fight[3] = {208, 228, 243};
	const uint16 _ambush[3] = {192, 193, 192};
	const uint16 _hotelScenes[10] = {77, 77, 87, 87, 89, 89, 97, 97, 105, 105};
	const uint16 _bottleToss[14] = {171, 171, 174, 174, 175, 175, 178, 178, 179, 179, 182, 182, 183, 183};

	const uint16 _diffPos[4][2] = {{0, 0}, {0x4D, 0x43}, {0x4E, 0x66}, {0x4F, 0x80}};
	const uint16 _livePos[3][2] = {{0x03, 0x5E}, {0x03, 0x76}, {0x03, 0x8E}};
	const uint16 _shotPos[12][2] = {{0x3, 0x5}, {0x0D, 0x5}, {0x17, 0x5}, {0x21, 0x5}, {0x3, 0x21}, {0x0D, 0x21}, {0x17, 0x21}, {0x21, 0x21}, {0x3, 0x3D}, {0x0D, 0x3D}, {0x17, 0x3D}, {0x21, 0x3D}};

	// gamestate
	uint8 _badMen = 0;
	uint8 _badMenBits = 0;
	bool _bartenderAlive = false;
	uint16 _beenTo = 0;
	uint8 _bottles = 0;
	uint8 _bottlesMask = 0;
	bool _gotClue = false;
	uint16 _gotInto = 0;
	uint8 _gunTime = 0;
	bool _hadSkull = false;
	bool _hadLantern = false;
	bool _hideOutFront = false;
	bool _inShootout = false;
	int8 _map0 = 0;
	int8 _map1 = 0;
	int8 _map2 = 0;
	uint8 _mapPos = 0;
	uint8 _maxMapPos = 0;
	uint8 _peopleKilled = 0;
	uint8 _proClue = 0;
	uint8 _sheriffCnt = 0; // unused
	uint8 _shootOutCnt = 0;

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
	void defaultBullethole(Common::Point *point);
	void die();
	uint8 pickRand(uint8 *bits, uint8 max);
	uint8 pickBad(uint8 max);
	Common::String pickTown();
	Common::String pickMap();
	Common::String pickSign();
	Common::String mapRight();
	Common::String mapLeft();

	// Script functions: Zone
	void zoneBullethole(Common::Point *point);
	void zoneSkullhole(Common::Point *point);

	// Script functions: RectHit
	void rectShotMenu(Rect *rect);
	void rectContinue(Rect *rect);
	void rectSave(Rect *rect);
	void rectLoad(Rect *rect);
	void rectStart(Rect *rect);
	void rectStartBottles(Rect *rect);
	void rectHideFront(Rect *rect);
	void rectHideRear(Rect *rect);
	void rectMenuSelect(Rect *rect);
	void rectSkull(Rect *rect);
	void rectKillMan(Rect *rect);
	void rectKillWoman(Rect *rect);
	void rectProspSign(Rect *rect);
	void rectMineSign(Rect *rect);
	void rectMineItem1(Rect *rect);
	void rectMineItem2(Rect *rect);
	void rectMineItem3(Rect *rect);
	void rectMineLantern(Rect *rect);
	void rectShotHideout(Rect *rect);
	void rectShotRight(Rect *rect);
	void rectShotLeft(Rect *rect);

	// Script functions: Scene PreOps
	void scenePsoShootout(Scene *scene);
	void scenePsoMDShootout(Scene *scene);

	// Script functions: Scene InsOps
	void sceneIsoShootPast(Scene *scene);
	void sceneIsoShootPastPause(Scene *scene);
	void sceneIsoSkipSaloon(Scene *scene);
	void sceneIsoSkipSaloon2(Scene *scene);
	void sceneIsoCheckSaloon(Scene *scene);
	void sceneIsoIntoStable(Scene *scene);
	void sceneIsoIntoOffice(Scene *scene);
	void sceneIsoIntoBank(Scene *scene);
	void sceneIsoCheckBartender(Scene *scene);
	void sceneIsoDidHideout(Scene *scene);
	void sceneIsoDidSignPost(Scene *scene);
	void sceneIsoDoShootout(Scene *scene);
	void sceneIsoMDShootout(Scene *scene);
	void sceneIsoShotInto116(Scene *scene);

	// Script functions: Scene NxtScn
	void sceneDefaultNxtscn(Scene *scene);
	void sceneNxtscnPickBottle(Scene *scene);
	void sceneNxtscnDied(Scene *scene);
	void sceneNxtscnAutoSelect(Scene *scene);
	void sceneNxtscnFinishSaloon(Scene *scene);
	void sceneNxtscnFinishOffice(Scene *scene);
	void sceneNxtscnFinishStable(Scene *scene);
	void sceneNxtscnFinishBank(Scene *scene);
	void sceneNxtscnPicSaloon(Scene *scene);
	void sceneNxtscnKillMan(Scene *scene);
	void sceneNxtscnKillWoman(Scene *scene);
	void sceneNxtscnBank(Scene *scene);
	void sceneNxtscnStable(Scene *scene);
	void sceneNxtscnSavProsp(Scene *scene);
	void sceneNxtscnPickToss(Scene *scene);
	void sceneNxtscnHitToss(Scene *scene);
	void sceneNxtscnMissToss(Scene *scene);
	void sceneNxtscnPickSign(Scene *scene);
	void sceneNxtscnBRockMan(Scene *scene);
	void sceneNxtscnLRockMan(Scene *scene);
	void sceneNxtscnHotelMen(Scene *scene);

	// Script functions: Scene WepDwn
	void sceneDefaultWepdwn(Scene *scene);
};

class DebuggerMaddog : public GUI::Debugger {
public:
	DebuggerMaddog(GameMaddog *game);
	bool cmdWarpTo(int argc, const char **argv);
	bool cmdDumpLib(int argc, const char **argv);

private:
	GameMaddog *_game;
};

} // End of namespace Alg

#endif
