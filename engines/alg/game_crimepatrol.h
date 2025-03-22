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
	GameCrimePatrol(AlgEngine *vm, const ADGameDescription *desc);
	~GameCrimePatrol();
	Common::Error run();
	void debug_warpTo(int val);

private:
	void init();
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
	Graphics::Surface _shotIcon;
	Graphics::Surface _emptyIcon;
	Graphics::Surface _liveIcon;
	Graphics::Surface _deadIcon;
	Graphics::Surface _difficultyIcon;
	Graphics::Surface _bulletholeIcon;

	// constants
	const int16 _scenes_level0[2] = {0x0191, 0};
	const int16 _scenes_level1[3] = {0x27, 0x01B7, 0};
	const int16 _scenes_level2[4] = {0x33, 0x01C7, 0x01B8, 0};
	const int16 _scenes_level3[3] = {0x48, 0x01C8, 0};
	const int16 _scenes_level4[2] = {0x53, 0};
	const int16 _scenes_level5[2] = {0x60, 0};
	const int16 _scenes_level6[2] = {0x82, 0};
	const int16 _scenes_level7[2] = {0x9B, 0};
	const int16 _scenes_level8[2] = {0xC7, 0};
	const int16 _scenes_level9[2] = {0xB6, 0};
	const int16 _scenes_level10[6] = {0xE0, 0x01BF, 0x01C0, 0x01C1, 0x01C2, 0};
	const int16 _scenes_level11[3] = {0x0136, 0x01C3, 0};
	const int16 _scenes_level12[4] = {0x0119, 0x01C5, 0x0131, 0};
	const int16 _scenes_level13[2] = {0x014C, 0};
	const int16 _scenes_level14[13] = {0x01B1, 0x01B2, 0x01B3, 0x01B4, 0x01B5, 0x01B6, 0};
	const int16 _scenes_level15[5] = {0x3B, 0x3C, 0x3F, 0x41, 0};
	const int16 _scenes_level16[3] = {0x61, 0x65, 0};
	const int16 _scenes_level17[7] = {0x68, 0x6A, 0x6C, 0x6E, 0x70, 0x72, 0};
	const int16 _scenes_level18[8] = {0x83, 0x85, 0x87, 0x8A, 0x8C, 0x8F, 0x90, 0};
	const int16 _scenes_level19[7] = {0x9C, 0x9E, 0xA0, 0xA2, 0xA4, 0xA6, 0};
	const int16 _scenes_level20[5] = {0xC8, 0xCA, 0xCC, 0xCE, 0};
	const int16 _scenes_level21[3] = {0xE8, 0xE9, 0};
	const int16 _scenes_level22[11] = {0xF4, 0xF6, 0xF8, 0xFA, 0xFC, 0xFE, 0x0100, 0x0102, 0x0104, 0x0106, 0};
	const int16 _scenes_level23[3] = {0x010E, 0x0113, 0};
	const int16 _scenes_level24[8] = {0x011D, 0x011F, 0x0121, 0x0123, 0x0125, 0x0127, 0x0129, 0};
	const int16 _scenes_level25[6] = {0x013D, 0x013F, 0x0141, 0x0143, 0x0145, 0};
	const int16 _scenes_level26[10] = {0x0157, 0x0159, 0x015B, 0x015D, 0x015F, 0x0161, 0x0163, 0x0165, 0x0167, 0};

	const int16 *_level_scenes[27] = {_scenes_level0, _scenes_level1, _scenes_level2, _scenes_level3, _scenes_level4, _scenes_level5, _scenes_level6,
									  _scenes_level7, _scenes_level8, _scenes_level9, _scenes_level10, _scenes_level11, _scenes_level12, _scenes_level13,
									  _scenes_level14, _scenes_level15, _scenes_level16, _scenes_level17, _scenes_level18, _scenes_level19, _scenes_level20,
									  _scenes_level21, _scenes_level22, _scenes_level23, _scenes_level24, _scenes_level25, _scenes_level26};

	const int16 _died_scenes_stage0[5] = {0x30, 0x31, 0x47, 0x51, 0};
	const int16 _died_scenes_stage1[7] = {0x5E, 0x5F, 0x7E, 0x7F, 0x98, 0x99, 0};
	const int16 _died_scenes_stage2[4] = {0xB2, 0xC5, 0xDB, 0};
	const int16 _died_scenes_stage3[7] = {0x0115, 0x0116, 0x0135, 0x014A, 0x0175, 0x0190, 0};
	const int16 _died_scenes_stage4[7] = {0x0115, 0x0116, 0x0135, 0x014A, 0x0175, 0x0190, 0};

	const int16 *_died_scenes_by_stage[5] = {_died_scenes_stage0, _died_scenes_stage1, _died_scenes_stage2, _died_scenes_stage3, _died_scenes_stage4};

	const uint16 _dead_scenes[5] = {0x32, 0x80, 0xDC, 0x018D, 0x018D};

	const uint16 _stage_start_scenes[5] = {0x26, 0x52, 0x9A, 0xDF, 0x014C};

	const int16 _practice_target_left[5] = {0xE1, 0x45, 0xA8, 0x73, 0xE1};
	const int16 _practice_target_top[5] = {0x0A, 0x3E, 0x41, 0x6E, 0x6E};
	const int16 _practice_target_right[5] = {0x0104, 0x6D, 0xCB, 0x95, 0x0104};
	const int16 _practice_target_bottom[5] = {0x3D, 0x79, 0x7B, 0xA1, 0xA1};

	bool _isDemo = 0;

	// gamestate
	uint16 _got_to[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int8 _stage = 0;
	int8 _old_stage = -1;
	uint8 _random_scene_count = 0;
	uint8 _random_max = 0;
	uint16 _random_mask = 0;
	uint16 _random_picked = 0;
	uint16 _death_mask = 0;
	int16 _death_picked = 0;
	uint8 _death_scene_count = 0;
	uint8 _practice_mask = 0;
	uint16 _final_stage_scene = _stage_start_scenes[4];

	// base functions
	void _NewGame();
	void _ResetParams();
	void _DoMenu();
	void _ChangeDifficulty(uint8 newDifficulty);
	void _ShowDifficulty(uint8 newDifficulty, bool updateCursor);
	void _DoCursor();
	void _UpdateMouse();
	void _MoveMouse();
	void _DisplayLivesLeft();
	void _DisplayScores();
	void _DisplayShotsLeft();
	bool _WeaponDown();
	bool _SaveState();
	bool _LoadState();

	// misc game functions
	void _DisplayShotFiredImage(Common::Point *point);
	void _EnableVideoFadeIn();
	uint16 _SceneToNumber(Scene *scene);
	uint16 _RandomUnusedInt(uint8 max, uint16 *mask, uint16 exclude);
	uint16 _PickRandomScene(uint8 index, uint8 max);
	uint16 _PickDeathScene();
	void _scene_nxtscn_generic(uint8 index);
	void _rect_select_generic(uint8 index);
	void _scene_iso_got_to_generic(uint8 index, uint16 sceneId);

	// Script functions: RectHit
	void _rect_shotmenu(Rect *rect);
	void _rect_save(Rect *rect);
	void _rect_load(Rect *rect);
	void _rect_continue(Rect *rect);
	void _rect_start(Rect *rect);
	void _rect_target_practice(Rect *rect);
	void _rect_select_target_practice(Rect *rect);
	void _rect_select_gang_fight(Rect *rect);
	void _rect_select_warehouse(Rect *rect);
	void _rect_select_westcoast_sound(Rect *rect);
	void _rect_select_drug_deal(Rect *rect);
	void _rect_select_car_ring(Rect *rect);
	void _rect_select_bar(Rect *rect);
	void _rect_select_bank(Rect *rect);
	void _rect_select_crack_house(Rect *rect);
	void _rect_select_meth_lab(Rect *rect);
	void _rect_select_airplane(Rect *rect);
	void _rect_select_nuke_transport(Rect *rect);
	void _rect_select_airport(Rect *rect);
	void _rect_kill_innocent_man(Rect *rect);

	// Script functions: Scene PreOps
	void _scene_pso_warehouse_got_to(Scene *scene);
	void _scene_pso_gang_fight_got_to(Scene *scene);
	void _scene_pso_westcoast_sound_got_to(Scene *scene);
	void _scene_pso_drug_deal_got_to(Scene *scene);
	void _scene_pso_car_ring_got_to(Scene *scene);
	void _scene_pso_bank_got_to(Scene *scene);
	void _scene_pso_crack_house_got_to(Scene *scene);
	void _scene_pso_meth_lab_got_to(Scene *scene);
	void _scene_pso_airplane_got_to(Scene *scene);
	void _scene_pso_airport_got_to(Scene *scene);
	void _scene_pso_nuke_transport_got_to(Scene *scene);
	void _scene_pso_power_plant_got_to(Scene *scene);

	// Script functions: Scene NxtScn
	void _scene_nxtscn_game_won(Scene *scene);
	void _scene_nxtscn_lose_a_life(Scene *scene);
	void _scene_nxtscn_did_not_continue(Scene *scene);
	void _scene_nxtscn_kill_innocent_man(Scene *scene);
	void _scene_nxtscn_kill_innocent_woman(Scene *scene);
	void _scene_nxtscn_after_die(Scene *scene);
	void _scene_nxtscn_select_language_1(Scene *scene);
	void _scene_nxtscn_select_language_2(Scene *scene);
	void _scene_nxtscn_select_rookie_scenario(Scene *scene);
	void _scene_nxtscn_select_undercover_scenario(Scene *scene);
	void _scene_nxtscn_select_swat_scenario(Scene *scene);
	void _scene_nxtscn_select_delta_scenario(Scene *scene);
	void _scene_nxtscn_init_random_target_practice(Scene *scene);
	void _scene_nxtscn_continue_target_practice(Scene *scene);
	void _scene_nxtscn_finish_gang_fight(Scene *scene);
	void _scene_nxtscn_finish_westcoast_sound(Scene *scene);
	void _scene_nxtscn_finish_warehouse(Scene *scene);
	void _scene_nxtscn_init_random_warehouse(Scene *scene);
	void _scene_nxtscn_continue_warehouse(Scene *scene);
	void _scene_nxtscn_finish_drug_deal(Scene *scene);
	void _scene_nxtscn_init_random_car_ring_leader(Scene *scene);
	void _scene_nxtscn_continue_car_ring_leader_1(Scene *scene);
	void _scene_nxtscn_continue_car_ring_leader_2(Scene *scene);
	void _scene_nxtscn_init_random_car_ring(Scene *scene);
	void _scene_nxtscn_continue_car_ring(Scene *scene);
	void _scene_nxtscn_finish_car_ring(Scene *scene);
	void _scene_nxtscn_finish_bar(Scene *scene);
	void _scene_nxtscn_finish_bank(Scene *scene);
	void _scene_nxtscn_finish_crack_house(Scene *scene);
	void _scene_nxtscn_finish_meth_lab(Scene *scene);
	void _scene_nxtscn_finish_airplane(Scene *scene);
	void _scene_nxtscn_finish_airport(Scene *scene);
	void _scene_nxtscn_finish_nuke_transport(Scene *scene);
	void _scene_nxtscn_init_random_bar(Scene *scene);
	void _scene_nxtscn_continue_bar(Scene *scene);
	void _scene_nxtscn_init_random_bank(Scene *scene);
	void _scene_nxtscn_continue_bank(Scene *scene);
	void _scene_nxtscn_init_random_meth_lab(Scene *scene);
	void _scene_nxtscn_continue_meth_lab(Scene *scene);
	void _scene_nxtscn_pick_random_rapeller(Scene *scene);
	void _scene_nxtscn_init_random_airplane(Scene *scene);
	void _scene_nxtscn_continue_airplane(Scene *scene);
	void _scene_nxtscn_pick_random_airplane_front(Scene *scene);
	void _scene_nxtscn_init_random_airport(Scene *scene);
	void _scene_nxtscn_continue_airport(Scene *scene);
	void _scene_nxtscn_init_random_nuke_transport(Scene *scene);
	void _scene_nxtscn_continue_nuke_transport(Scene *scene);
	void _scene_nxtscn_init_random_powerplant(Scene *scene);
	void _scene_nxtscn_continue_powerplant(Scene *scene);

	// Script functions: Scene WepDwn
	void _scene_default_wepdwn(Scene *scene);
	void debug_drawPracticeRects();
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
