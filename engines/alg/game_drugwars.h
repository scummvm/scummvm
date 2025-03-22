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

#ifndef ALG_GAME_DRUGWARS_H
#define ALG_GAME_DRUGWARS_H

#include "common/hashmap.h"
#include "common/rect.h"

#include "gui/debugger.h"

#include "alg/game.h"
#include "alg/scene.h"

namespace Alg {

typedef Common::Functor1Mem<Scene *, void, GameDrugWars> DWScriptFunctionScene;
typedef Common::Functor1Mem<Rect *, void, GameDrugWars> DWScriptFunctionRect;
typedef Common::HashMap<Common::String, DWScriptFunctionScene *> DWScriptFunctionSceneMap;
typedef Common::HashMap<Common::String, DWScriptFunctionRect *> DWScriptFunctionRectMap;

class GameDrugWars : public Game {

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
	GameDrugWars(AlgEngine *vm, const ADGameDescription *desc);
	~GameDrugWars();
	Common::Error run();
	void debug_warpTo(int val);

private:
	void init();
	void registerScriptFunctions();
	void verifyScriptFunctions();
	DWScriptFunctionRect getScriptFunctionRectHit(Common::String name);
	DWScriptFunctionScene getScriptFunctionScene(SceneFuncType type, Common::String name);
	void callScriptFunctionRectHit(Common::String name, Rect *rect);
	void callScriptFunctionScene(SceneFuncType type, Common::String name, Scene *scene);

	DWScriptFunctionRectMap _rectHitFuncs;
	DWScriptFunctionSceneMap _scenePreOps;
	DWScriptFunctionSceneMap _sceneShowMsg;
	DWScriptFunctionSceneMap _sceneInsOps;
	DWScriptFunctionSceneMap _sceneWepDwn;
	DWScriptFunctionSceneMap _sceneScnScr;
	DWScriptFunctionSceneMap _sceneNxtFrm;
	DWScriptFunctionSceneMap _sceneNxtScn;

	// images
	Graphics::Surface _shotIcon;
	Graphics::Surface _emptyIcon;
	Graphics::Surface _liveIcon;
	Graphics::Surface _deadIcon;
	Graphics::Surface _difficultyIcon;
	Graphics::Surface _bulletholeIcon;

	// constants
	const int16 _random_scenes0[7] = {0x29, 0x2B, 0x2D, 0x2F, 0x31, 0x33, 0};
	const int16 _random_scenes1[6] = {0x37, 0x39, 0x3B, 0x3D, 0x3F, 0};
	const int16 _random_scenes4[8] = {0xA8, 0xAA, 0xAC, 0xAE, 0xB0, 0xB2, 0xB4, 0};
	const int16 _random_scenes8[8] = {0xC0, 0xC2, 0xC4, 0xC6, 0xC8, 0xCA, 0xCC, 0};
	const int16 _random_scenes9[6] = {0xFE, 0x0100, 0x0102, 0x01A3, 0x0105, 0};
	const int16 _random_scenes10[8] = {0x0161, 0x0163, 0x0165, 0x0167, 0x016A, 0x016C, 0x016E, 0};
	const int16 _random_scenes11[9] = {0x010B, 0x010D, 0x010F, 0x0111, 0x0113, 0x0115, 0x0117, 0x0119, 0};
	const int16 _random_scenes12[10] = {0x014C, 0x014E, 0x0150, 0x0152, 0x0154, 0x0156, 0x0158, 0x015A, 0x015C, 0};

	const int16 *_random_scenes[14] = {_random_scenes0, _random_scenes1, nullptr, nullptr, _random_scenes4, nullptr, nullptr, nullptr,
									   _random_scenes8, _random_scenes9, _random_scenes10, _random_scenes11, _random_scenes12, nullptr};
	const uint8 _random_scenes_difficulty[14] = {6, 4, 0, 0, 6, 0, 0, 0, 5, 6, 7, 8, 8, 0};
	const uint16 _random_scenes_continue[14] = {0x51, 0x41, 0, 0, 0x01B5, 0, 0, 0, 0xCE, 0x0107, 0x0170, 0x011B, 0x015E, 0};

	const int16 _died_scenes_stage0[4] = {0x52, 0x53, 0x54, 0};
	const int16 _died_scenes_stage1[5] = {0x85, 0x86, 0x88, 0x89, 0};
	const int16 _died_scenes_stage2[3] = {0xEF, 0xF0, 0};
	const int16 _died_scenes_stage3[3] = {0x0135, 0x0136, 0};
	const int16 _died_scenes_stage4[3] = {0x0135, 0x0136, 0};

	const int16 *_died_scenes_by_stage[5] = {_died_scenes_stage0, _died_scenes_stage1, _died_scenes_stage2, _died_scenes_stage3, _died_scenes_stage4};

	uint16 _dead_scenes[5] = {0x56, 0x8A, 0xF2, 0x0134, 0x0134};

	const uint16 _stage_start_scenes[5] = {0x51, 0x83, 0xEE, 0x0132, 0x017F};

	const uint16 _scenario_start_scenes[14] = {0x27, 0x36, 0x4A, 0x57, 0x9D, 0x8B, 0x74, 0xD8, 0xBF, 0xB8, 0x0160, 0x010A, 0x0137, 0x017F};

	bool _isDemo = 0;

	// gamestate
	uint8 _continues = 0;
	uint16 _got_to[14] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int8 _got_to_index = 0;
	int8 _stage = 0;
	int8 _old_stage = -1;
	uint8 _random_scene_count = 0;
	uint8 _random_max = 0;
	uint16 _random_mask = 0;
	uint16 _random_picked = 0;
	uint16 _death_mask = 0;
	int16 _death_picked = 0;
	uint8 _death_scene_count = 0;
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

	// Script functions: RectHit
	void _rect_shotmenu(Rect *rect);
	void _rect_save(Rect *rect);
	void _rect_load(Rect *rect);
	void _rect_continue(Rect *rect);
	void _rect_start(Rect *rect);
	void _rect_select_target_practice(Rect *rect);
	void _rect_select_bar(Rect *rect);
	void _rect_select_car_chase(Rect *rect);
	void _rect_select_drug_house(Rect *rect);
	void _rect_select_office(Rect *rect);
	void _rect_select_court(Rect *rect);
	void _rect_select_bus(Rect *rect);
	void _rect_select_docks(Rect *rect);
	void _rect_select_house_boat(Rect *rect);
	void _rect_select_party(Rect *rect);
	void _rect_select_airport(Rect *rect);
	void _rect_select_mansion(Rect *rect);
	void _rect_select_village(Rect *rect);

	// Script functions: Scene PreOps
	void _scene_pso_got_to(Scene *scene);

	// Script functions: Scene NxtScn
	void _scene_nxtscn_game_won(Scene *scene);
	void _scene_nxtscn_lose_a_life(Scene *scene);
	void _scene_nxtscn_continue_game(Scene *scene);
	void _scene_nxtscn_did_not_continue(Scene *scene);
	void _scene_nxtscn_kill_innocent_man(Scene *scene);
	void _scene_nxtscn_kill_innocent_woman(Scene *scene);
	void _scene_nxtscn_after_die(Scene *scene);
	void _scene_nxtscn_init_random(Scene *scene);
	void _scene_nxtscn_continue_random(Scene *scene);
	void _scene_nxtscn_select_scenario(Scene *scene);
	void _scene_nxtscn_finish_scenario(Scene *scene);

	// Script functions: Scene WepDwn
	void _scene_default_wepdwn(Scene *scene);
};

class DebuggerDrugWars : public GUI::Debugger {
public:
	DebuggerDrugWars(GameDrugWars *game);
	bool cmdWarpTo(int argc, const char **argv);
	bool cmdDumpLib(int argc, const char **argv);

private:
	GameDrugWars *_game;
};

} // End of namespace Alg

#endif
