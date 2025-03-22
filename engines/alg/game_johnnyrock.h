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
	GameJohnnyRock(AlgEngine *vm, const ADGameDescription *desc);
	~GameJohnnyRock();
	Common::Error run();
	void debug_warpTo(int val);

private:
	void init();
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
	Common::Array<Graphics::Surface> *_difficultyIcon;
	Graphics::Surface _levelIcon;
	Graphics::Surface _bulletholeIcon;

	// constants
	const int16 _random_rooftop[6] = {2, -4, 0x104, 0x1E, 0x100, 0x102};
	const int16 _random_theater[9] = {5, -5, 0x111, 0x1E, 0x107, 0x109, 0x10B, 0x10D, 0x10F};
	const int16 _random_alley[10] = {6, -4, 0, 0x1E, 0x113, 0x115, 0x117, 0x119, 0x11B, 0x11D};
	const int16 _random_funeral[10] = {6, -5, 0, 0x1E, 0x11F, 0x121, 0x123, 0x125, 0x127, 0x129};
	const int16 _random_funeral_mr[10] = {6, -5, 0x12B, 0x1E, 0x11F, 0x121, 0x123, 0x125, 0x127, 0x129};
	const int16 _random_book[7] = {3, 5, 0, 0x1E, 0x12E, 0x130, 0x132};
	const int16 _random_book_mr[8] = {4, 0x5, 0, 0x1E, 0x12E, 0x130, 0x132, 0x134};
	const int16 _random_stairway[8] = {4, -3, 0, 0x1E, 0x139, 0x13B, 0x13D, 0x13F};
	const int16 _random_hall[8] = {4, -5, 0, 0x1E, 0x141, 0x143, 0x145, 0x146};
	const int16 _random_windows[10] = {6, -3, 0, 0x1E, 0x154, 0x156, 0x158, 0x15A, 0x15C, 0x15E};
	const int16 _random_car[5] = {1, 1, 0, 0, 0x0FE};
	const int16 _random_hall1[5] = {1, 1, 0, 0x1E, 0x148};
	const int16 _random_elevator[5] = {1, 1, 0, 0, 0x14C};
	const int16 _random_elevator_mr[5] = {1, 1, 0, 0, 0x151};
	const int16 _random_baby[5] = {1, 1, 0, 0, 0x160};
	const int16 *_random_places[6] = {_random_windows, _random_stairway, _random_car, _random_hall1, _random_elevator, _random_baby};
	const int16 *_random_places_mr[8] = {_random_book_mr, _random_funeral_mr, _random_alley, _random_theater, _random_hall, _random_windows, _random_hall1, _random_rooftop};

	const int16 _office_table[5] = {0xA7, 0x9F, 0x9E, 0x0A0, 0x0AD};

	const uint16 _diffpos[4][2] = {{0, 0}, {0xCD, 0x35}, {0xD2, 0x53}, {0xD2, 0x6E}};

	// gamestate
	uint16 _total_dies = 0;
	int16 _game_money = 0;
	int16 _oldgame_money = 0;
	uint16 _ammo_again = 0;
	uint16 _map_timeout = 0;
	uint8 _lucky_number = 0;
	uint8 _this_map = 0;
	uint16 _clues = 0;
	uint16 _place_bits = 0;
	uint8 _random_count = 0;
	uint16 _doctor_bits = 0;
	uint16 _undertaker_bits = 0;
	uint16 _this_clue = 0;
	uint8 _got_this_number = 0;
	uint8 _casino = 0;
	uint8 _pool_hall = 0;
	uint8 _warehouse = 0;
	uint8 _garage = 0;
	uint8 _office = 0;
	uint8 _casino_type = 0;
	uint8 _pool_hall_type = 0;
	uint8 _warehouse_type = 0;
	uint8 _garage_type = 0;
	uint8 _mansion = 0;
	uint8 _in_warehouse = 0;
	uint8 _in_office = 0;
	uint16 _got_to = 0;
	uint8 _who_did_it = 0;
	uint8 _had_go_to_mansion = 0;
	uint16 _office_count = 0;
	uint16 _random_place_bits = 0;
	uint8 _max_random_count = 0;
	uint16 _goto_after_random = 0;
	uint16 _repeat_random_place = 0;
	uint16 _max_repeat = 0;
	uint16 _got_this_clue = 0;
	uint16 _did_continue = 0;
	uint16 _this_game_time = 0;
	uint8 _this_difficulty = 0;
	uint8 _clue_table[4];
	uint8 _combinations[4];
	uint16 _entrance_index[20];
	const int16 *_random_scenes = nullptr;
	uint8 _random_scenes_savestate_index = 0;
	uint16 _random_scenes_index[10];
	Common::String _money_scene = "";
	uint8 _mgun_cnt = 0;
	uint32 _mach_gun_timer = 0;

	// base functions
	bool __Fired(Common::Point *point);
	void _NewGame();
	void _ResetParams();
	void _OutShots();
	void _DoMenu();
	void _UpdateStat();
	void _DisplayScore();
	void _ShowDifficulty(uint8 newDifficulty, bool updateCursor);
	void _ChangeDifficulty(uint8 newDifficulty);
	void _DoCursor();
	void _UpdateMouse();
	void _MoveMouse();
	bool _WeaponDown();
	bool _SaveState();
	bool _LoadState();
	void _DoMoneySound();

	// misc game functions
	Common::String _NumtoScene(int n);
	uint16 _ScenetoNum(Common::String sceneName);
	void _default_bullethole(Common::Point *point);
	uint16 _pick_bits(uint16 *bits, uint8 max);
	uint16 _pick_random_place(uint8 place);
	void _show_combination();
	void _shotclue(uint8 clue);
	void _shotcombination(uint8 combination, bool combinationB);
	void _shotluckynumber(uint8 number);

	// Script functions: Zone
	void _zone_bullethole(Common::Point *point);

	// Script functions: RectHit
	void _rect_shotmenu(Rect *rect);
	void _rect_save(Rect *rect);
	void _rect_load(Rect *rect);
	void _rect_continue(Rect *rect);
	void _rect_start(Rect *rect);
	void _rect_killinnocent(Rect *rect);
	void _rect_selectcasino(Rect *rect);
	void _rect_selectpoolhall(Rect *rect);
	void _rect_selectwarehouse(Rect *rect);
	void _rect_selectgarage(Rect *rect);
	void _rect_selectmansion(Rect *rect);
	void _rect_selectammo(Rect *rect);
	void _rect_selectoffice(Rect *rect);
	void _rect_shotmanbust(Rect *rect);
	void _rect_shotwomanbust(Rect *rect);
	void _rect_shotbluevase(Rect *rect);
	void _rect_shotcat(Rect *rect);
	void _rect_shotindian(Rect *rect);
	void _rect_shotplate(Rect *rect);
	void _rect_shotbluedresspic(Rect *rect);
	void _rect_shotmodernpic(Rect *rect);
	void _rect_shotmonalisa(Rect *rect);
	void _rect_shotgwashington(Rect *rect);
	void _rect_shotboyinredpic(Rect *rect);
	void _rect_shotcoatofarms(Rect *rect);
	void _rect_shotcombinationA0(Rect *rect);
	void _rect_shotcombinationA1(Rect *rect);
	void _rect_shotcombinationA2(Rect *rect);
	void _rect_shotcombinationA3(Rect *rect);
	void _rect_shotcombinationA4(Rect *rect);
	void _rect_shotcombinationA5(Rect *rect);
	void _rect_shotcombinationB0(Rect *rect);
	void _rect_shotcombinationB1(Rect *rect);
	void _rect_shotcombinationB2(Rect *rect);
	void _rect_shotcombinationB3(Rect *rect);
	void _rect_shotcombinationB4(Rect *rect);
	void _rect_shotcombinationB5(Rect *rect);
	void _rect_shotluckynum0(Rect *rect);
	void _rect_shotluckynum1(Rect *rect);
	void _rect_shotluckynum2(Rect *rect);
	void _rect_shotluckynum3(Rect *rect);
	void _rect_shotluckynum4(Rect *rect);
	void _rect_shotluckynum5(Rect *rect);

	// Script functions: Scene InsOps
	void _scene_iso_shootpast(Scene *scene);
	void _scene_iso_spause(Scene *scene);
	void _scene_iso_gotocasino(Scene *scene);
	void _scene_iso_gotopoolh(Scene *scene);
	void _scene_iso_gotowarehse(Scene *scene);
	void _scene_iso_inwarehse2(Scene *scene);
	void _scene_iso_inwarehse3(Scene *scene);
	void _scene_iso_gotogarage(Scene *scene);
	void _scene_iso_gotomansion(Scene *scene);
	void _scene_iso_inmansion1(Scene *scene);

	// Script functions: Scene NxtScn
	void _scene_nxtscn_died(Scene *scene);
	void _scene_nxtscn_bombdead(Scene *scene);
	void _scene_nxtscn_pikundrtakr(Scene *scene);
	void _scene_nxtscn_callattract(Scene *scene);
	void _scene_nxtscn_pikluckno(Scene *scene);
	void _scene_nxtscn_pickmap(Scene *scene);
	void _scene_nxtscn_pickclue(Scene *scene);
	void _scene_nxtscn_maptimeout(Scene *scene);
	void _scene_nxtscn_entcasino(Scene *scene);
	void _scene_nxtscn_casinowhat(Scene *scene);
	void _scene_nxtscn_entpoolhall(Scene *scene);
	void _scene_nxtscn_poolhclue(Scene *scene);
	void _scene_nxtscn_entwarehse(Scene *scene);
	void _scene_nxtscn_warehseclue(Scene *scene);
	void _scene_nxtscn_entgarage(Scene *scene);
	void _scene_nxtscn_garageclue(Scene *scene);
	void _scene_nxtscn_entmansion(Scene *scene);
	void _scene_nxtscn_giveclue(Scene *scene);
	void _scene_nxtscn_pikflwrman(Scene *scene);
	void _scene_nxtscn_randomscene(Scene *scene);
	void _scene_nxtscn_endrandscene(Scene *scene);
	void _scene_nxtscn_killinnocent(Scene *scene);

	// Script functions: Scene WepDwn
	void _scene_default_wepdwn(Scene *scene);
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
