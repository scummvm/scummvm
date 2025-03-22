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
	GameMaddog2(AlgEngine *vm, const ADGameDescription *desc);
	~GameMaddog2();
	Common::Error run();
	void debug_warpTo(int val);

private:
	void init();
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
	Graphics::Surface _shotIcon;
	Graphics::Surface _emptyIcon;
	Graphics::Surface _liveIcon;
	Graphics::Surface _deadIcon;
	Graphics::Surface _reloadIcon;
	Graphics::Surface _drawIcon;
	Graphics::Surface _knifeIcon;
	Graphics::Surface _bulletholeIcon;

	// constants
	const int16 _sb_clue[3] = {0x67, 0x68, 0x69};
	const int16 _bb_clue[3] = {0x47, 0x49, 0x48};
	const int16 _tp_clue[3] = {0x84, 0x85, 0x86};
	const int16 _sb_scenes[14] = {0x4A, 0x50, 0xB8, 0x55, 0x57, 0x66, 0xBE, 0x94, 0x87, 0x93, 0xCD, 0x95, 0xE3, -1};
	const int16 _bb_scenes[14] = {0x33, 0x39, 0xB4, 0x41, 0x43, 0x46, 0xA2, 0x8D, 0x87, 0x8F, 0xCD, 0x8E, 0xE3, -1};
	const int16 _tp_scenes[14] = {0x6A, 0xC3, 0x76, 0x81, 0xAD, 0x83, 0xDC, 0x92, 0x87, 0x90, 0xCD, 0x91, 0xE3, -1};
	const int16 _village_scenes[6] = {0x58, 0x5A, 0x5C, 0x5E, 0x60, 0x62};
	const int16 _cowboy_scenes[7] = {0xCD, 0xCF, 0xD2, 0xD4, 0xD6, 0xD8, 0xDA};
	const int16 _farmyard_scenes[4] = {0xDC, 0xDE, 0x129, 0xE1};
	const int16 _cave_scenes[5] = {0xE4, 0xE6, 0xE8, 0xEA, 0xEC};
	const int16 _ez_shoot_outs[5] = {0xAB, 0xBC, 0xC5, 0xC7, 0xCB};
	const int16 _shoot_outs[6] = {0xAB, 0xBC, 0xC5, 0xC7, 0xC9, 0xCB};
	const int16 _gg_scenes[3] = {0x87, 0x88, 0x89};
	const int16 _die_scenes[6] = {0x96, 0x97, 0x98, 0x9B, 0x9F, 0xA1};

	const uint16 _diffpos[3][2] = {{0x0FA, 0x6E}, {0xFA, 0x8E}, {0xFA, 0xAF}};
	const uint16 _livepos[3][2] = {{0x81, 0xBE}, {0x70, 0xBE}, {0x5F, 0xBE}};
	const uint16 _shotpos[12][2] = {{0x96, 0xBD}, {0x9A, 0xBD}, {0x9E, 0xBD}, {0x0A2, 0xBD}, {0x0A6, 0xBD}, {0x0AA, 0xBD}, {0x0AE, 0xBD}, {0x0B2, 0xBD}, {0x0B6, 0xBD}, {0x0BA, 0xBD}, {0x0BE, 0xBD}, {0x0C2, 0xBD}};

	// gamestate
	uint16 _been_to = 0;
	uint16 _die_bits = 0;
	uint16 _got_to = 0;
	uint16 _sb_got_to = 0;
	uint16 _bb_got_to = 0;
	uint16 _tp_got_to = 0;
	bool _had_skull = false;
	int16 _this_guide = 0;
	uint16 _done_guide = 0;
	uint16 _shoot_out_bits = 0;
	int16 _last_shoot_out = 0;
	uint16 _start_lives = 3;
	bool _was_a_shootout = false;
	bool _shootout_from_die = false;
	uint16 _which_padre = 0;
	uint16 _which_gatlin_gun = 0;
	bool _in_shootout = false;
	uint16 _pick_mask = 0;
	uint16 _last_pick = 0;
	uint16 _place_bits = 0;
	uint16 _random_count = 0;
	uint16 _shoot_out_cnt = 0;
	uint16 _total_dies = 0;

	// base functions
	void _NewGame();
	void _ResetParams();
	void _DoMenu();
	void _UpdateStat();
	void _ChangeDifficulty(uint8 newDifficulty);
	void _ShowDifficulty(uint8 newDifficulty, bool updateCursor);
	void _DoCursor();
	void _UpdateMouse();
	void _MoveMouse();
	void _DisplayScore();
	bool _WeaponDown();
	bool _SaveState();
	bool _LoadState();

	// misc game functions
	Common::String _NumtoScene(int n);
	uint16 _ScenetoNum(Common::String sceneName);
	void _default_bullethole(Common::Point *point);
	uint16 _die();
	uint16 _pick_bits(uint16 *bits, uint8 max);
	uint16 _pick_shootout();
	void _next_sb();
	void _next_bb();
	void _next_tp();
	void _gg_pick_man();
	void _generic_next();
	void _player_won();

	// Script functions: Zone
	void _zone_bullethole(Common::Point *point);
	void _zone_skullhole(Common::Point *point);

	// Script functions: RectHit
	void _rect_shotmenu(Rect *rect);
	void _rect_save(Rect *rect);
	void _rect_load(Rect *rect);
	void _rect_continue(Rect *rect);
	void _rect_start(Rect *rect);
	void _rect_skull(Rect *rect);
	void _rect_killinnocentman(Rect *rect);
	void _rect_killinnocentwoman(Rect *rect);
	void _rect_selectbeaver(Rect *rect);
	void _rect_selectbonnie(Rect *rect);
	void _rect_selectprofessor(Rect *rect);
	void _rect_shotammo(Rect *rect);
	void _rect_shotgin(Rect *rect);
	void _rect_shotlantern(Rect *rect);
	void _rect_shootskull(Rect *rect);

	// Script functions: Scene PreOps
	void _scene_pso_shootout(Scene *scene);
	void _scene_pso_mdshootout(Scene *scene);

	// Script functions: Scene InsOps
	void _scene_iso_shootpast(Scene *scene);
	void _scene_iso_spause(Scene *scene);
	void _scene_iso_stagecoach(Scene *scene);
	void _scene_iso_differentpadres(Scene *scene);
	void _scene_iso_differentpadrespas(Scene *scene);
	void _scene_iso_dontpopnext(Scene *scene);
	void _scene_iso_getintorock(Scene *scene);
	void _scene_iso_benatcave(Scene *scene);
	void _scene_iso_skullatcave(Scene *scene);
	void _scene_iso_startoftrain(Scene *scene);
	void _scene_iso_mission(Scene *scene);
	void _scene_iso_mdshootout(Scene *scene);
	void _scene_iso_startofboardinghouse(Scene *scene);
	void _scene_iso_dontcontinue(Scene *scene);
	void _scene_iso_doshootout(Scene *scene);

	// Script functions: Scene NxtScn
	void _scene_default_nxtscn(Scene *scene);
	void _scene_nxtscn_died(Scene *scene);
	void _scene_nxtscn_killinnocentman(Scene *scene);
	void _scene_nxtscn_killinnocentwoman(Scene *scene);
	void _scene_nxtscn_killguide(Scene *scene);
	void _scene_nxtscn_shootskull(Scene *scene);
	void _scene_nxtscn_callattract(Scene *scene);
	void _scene_nxtscn_pickundertaker(Scene *scene);
	void _scene_nxtscn_choosepadre(Scene *scene);
	void _scene_nxtscn_selectguide(Scene *scene);
	void _scene_nxtscn_savebonnie(Scene *scene);
	void _scene_nxtscn_finishbonnie(Scene *scene);
	void _scene_nxtscn_showggclue(Scene *scene);
	void _scene_nxtscn_bbafterclue(Scene *scene);
	void _scene_nxtscn_asfarshegoes(Scene *scene);
	void _scene_nxtscn_savebeaver(Scene *scene);
	void _scene_nxtscn_finishbeaver(Scene *scene);
	void _scene_nxtscn_togatlingunsbclue(Scene *scene);
	void _scene_nxtscn_toguideafterclue(Scene *scene);
	void _scene_nxtscn_toguidecave(Scene *scene);
	void _scene_nxtscn_initrandomvillage(Scene *scene);
	void _scene_nxtscn_pickvillagescenes(Scene *scene);
	void _scene_nxtscn_saveprofessor(Scene *scene);
	void _scene_nxtscn_finishprofessor(Scene *scene);
	void _scene_nxtscn_togatlinguntpclue(Scene *scene);
	void _scene_nxtscn_tpafterclue(Scene *scene);
	void _scene_nxtscn_finishgatlingun1(Scene *scene);
	void _scene_nxtscn_finishguyatgg(Scene *scene);
	void _scene_nxtscn_finishgatlingun2(Scene *scene);
	void _scene_nxtscn_howwedid(Scene *scene);
	void _scene_nxtscn_playerwon(Scene *scene);
	void _scene_nxtscn_backtonxtguide(Scene *scene);
	void _scene_nxtscn_finishgenericscene(Scene *scene);
	void _scene_nxtscn_initrandomcowboys(Scene *scene);
	void _scene_nxtscn_tocowboyscenes(Scene *scene);
	void _scene_nxtscn_initrandomfarmyard(Scene *scene);
	void _scene_nxtscn_tofarmyardscenes(Scene *scene);
	void _scene_nxtscn_initrandomcave(Scene *scene);
	void _scene_nxtscn_tocavescenes(Scene *scene);
	void _scene_nxtscn_pickskullatcave(Scene *scene);
	void _scene_nxtscn_drawgun(Scene *scene);

	// Script functions: Scene WepDwn
	void _scene_default_wepdwn(Scene *scene);
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
