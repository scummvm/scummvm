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
	GameMaddog(AlgEngine *vm, const ADGameDescription *desc);
	~GameMaddog() override;
	Common::Error run() override;
	void debug_warpTo(int val);

private:
	void init();
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
	Graphics::Surface _shotIcon;
	Graphics::Surface _emptyIcon;
	Graphics::Surface _liveIcon;
	Graphics::Surface _deadIcon;
	Graphics::Surface _reloadIcon;
	Graphics::Surface _drawIcon;
	Graphics::Surface _knifeIcon;
	Graphics::Surface _bulletholeIcon;

	// constants
	const uint16 _fight[3] = {208, 228, 243};
	const uint16 _ambush[3] = {192, 193, 192};
	const uint16 _hotel_scenes[10] = {77, 77, 87, 87, 89, 89, 97, 97, 105, 105};
	const uint16 _bottle_toss[14] = {171, 171, 174, 174, 175, 175, 178, 178, 179, 179, 182, 182, 183, 183};

	const uint16 _diffpos[4][2] = {{0, 0}, {0x4D, 0x43}, {0x4E, 0x66}, {0x4F, 0x80}};
	const uint16 _livepos[3][2] = {{0x03, 0x5E}, {0x03, 0x76}, {0x03, 0x8E}};
	const uint16 _shotpos[12][2] = {{0x3, 0x5}, {0x0D, 0x5}, {0x17, 0x5}, {0x21, 0x5}, {0x3, 0x21}, {0x0D, 0x21}, {0x17, 0x21}, {0x21, 0x21}, {0x3, 0x3D}, {0x0D, 0x3D}, {0x17, 0x3D}, {0x21, 0x3D}};

	// gamestate
	uint8 _bad_men = 0;
	uint8 _bad_men_bits = 0;
	bool _bartender_alive = 0;
	uint16 _been_to = 0;
	uint8 _bottles = 0;
	uint8 _botmask = 0;
	bool _got_clue = false;
	uint16 _got_into = 0;
	uint8 _gun_time = 0;
	bool _had_skull = 0;
	bool _had_lantern = 0;
	bool _hide_out_front = 0;
	bool _in_shootout = 0;
	int8 _map0 = 0;
	int8 _map1 = 0;
	int8 _map2 = 0;
	uint8 _map_pos = 0;
	uint8 _max_map_pos = 0;
	uint8 _people_killed = 0;
	uint8 _pro_clue = 0;
	uint8 _sheriff_cnt = 0; // unused
	uint8 _shoot_out_cnt = 0;

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
	void _default_bullethole(Common::Point *point);
	void _die();
	uint8 _pick_rand(uint8 *bits, uint8 max);
	uint8 _pick_bad(uint8 max);
	Common::String _pick_town();
	Common::String _pick_map();
	Common::String _pick_sign();
	Common::String _map_right();
	Common::String _map_left();

	// Script functions: Zone
	void _zone_bullethole(Common::Point *point);
	void _zone_skullhole(Common::Point *point);

	// Script functions: RectHit
	void _rect_shotmenu(Rect *rect);
	void _rect_continue(Rect *rect);
	void _rect_save(Rect *rect);
	void _rect_load(Rect *rect);
	void _rect_start(Rect *rect);
	void _rect_startbottles(Rect *rect);
	void _rect_hidefront(Rect *rect);
	void _rect_hiderear(Rect *rect);
	void _rect_menuselect(Rect *rect);
	void _rect_skull(Rect *rect);
	void _rect_killman(Rect *rect);
	void _rect_killwoman(Rect *rect);
	void _rect_prospsign(Rect *rect);
	void _rect_minesign(Rect *rect);
	void _rect_mineitem1(Rect *rect);
	void _rect_mineitem2(Rect *rect);
	void _rect_mineitem3(Rect *rect);
	void _rect_minelantern(Rect *rect);
	void _rect_shothideout(Rect *rect);
	void _rect_shotright(Rect *rect);
	void _rect_shotleft(Rect *rect);

	// Script functions: Scene PreOps
	void _scene_pso_shootout(Scene *scene);
	void _scene_pso_mdshootout(Scene *scene);

	// Script functions: Scene InsOps
	void _scene_iso_shootpast(Scene *scene);
	void _scene_iso_spause(Scene *scene);
	void _scene_iso_skipsaloon(Scene *scene);
	void _scene_iso_skipsaloon2(Scene *scene);
	void _scene_iso_checksaloon(Scene *scene);
	void _scene_iso_intostable(Scene *scene);
	void _scene_iso_intoffice(Scene *scene);
	void _scene_iso_intobank(Scene *scene);
	void _scene_iso_chkbartndr(Scene *scene);
	void _scene_iso_didhideout(Scene *scene);
	void _scene_iso_didsignpost(Scene *scene);
	void _scene_iso_doshootout(Scene *scene);
	void _scene_iso_mdshootout(Scene *scene);
	void _scene_iso_shotinto116(Scene *scene);

	// Script functions: Scene NxtScn
	void _scene_default_nxtscn(Scene *scene);
	void _scene_nxtscn_pickbottle(Scene *scene);
	void _scene_nxtscn_died(Scene *scene);
	void _scene_nxtscn_autosel(Scene *scene);
	void _scene_nxtscn_finsaloon(Scene *scene);
	void _scene_nxtscn_finoffice(Scene *scene);
	void _scene_nxtscn_finstable(Scene *scene);
	void _scene_nxtscn_finbank(Scene *scene);
	void _scene_nxtscn_picsaloon(Scene *scene);
	void _scene_nxtscn_killman(Scene *scene);
	void _scene_nxtscn_killwoman(Scene *scene);
	void _scene_nxtscn_bank(Scene *scene);
	void _scene_nxtscn_stable(Scene *scene);
	void _scene_nxtscn_savprosp(Scene *scene);
	void _scene_nxtscn_picktoss(Scene *scene);
	void _scene_nxtscn_hittoss(Scene *scene);
	void _scene_nxtscn_misstoss(Scene *scene);
	void _scene_nxtscn_picksign(Scene *scene);
	void _scene_nxtscn_brockman(Scene *scene);
	void _scene_nxtscn_lrockman(Scene *scene);
	void _scene_nxtscn_hotelmen(Scene *scene);

	// Script functions: Scene WepDwn
	void _scene_default_wepdwn(Scene *scene);
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
