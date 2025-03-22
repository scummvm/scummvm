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
	GameSpacePirates(AlgEngine *vm, const ADGameDescription *desc);
	~GameSpacePirates();
	Common::Error run();
	void debug_warpTo(int val);

private:
	void init();
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
	Graphics::Surface _shotIcon;
	Graphics::Surface _emptyIcon;
	Graphics::Surface _deadIcon;
	Graphics::Surface _liveIcon1;
	Graphics::Surface _liveIcon2;
	Graphics::Surface _liveIcon3;
	Graphics::Surface _difficultyIcon;
	Graphics::Surface _bulletholeIcon;

	// constants

	bool _isDemo = 0;

	// gamestate
	bool _game_loaded = false;
	int8 _lives_loaded = 0;
	uint16 _shots_loaded = 0;
	int32 _score_loaded = 0;
	uint8 _difficulty_loaded = 0;
	bool _next_scene_found = false;
	bool _player_died = false;
	int16 _random_picked = -1;
	uint32 _last_extra_life_score = 0;
	uint16 _random_scenes_values[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	uint8 _random_scenes_used[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	uint8 _max_random = 0;
	uint8 _random_count = 0;
	uint8 _random_count_asteroids = 0;
	uint8 _random_count_midship = 0;
	uint8 _misc_rooms_count = 0;
	uint16 _picked_misc_rooms = 0;
	uint16 _got_to = 0;
	int8 _current_world = 0;
	uint16 _world_got_to[4] = {0, 0, 0, 0};
	bool _world_done[4] = {false, false, false, false};
	bool _selected_a_world = false;
	uint16 _selected_world_start = 0;
	uint16 _scene_before_flying_skulls = 0;
	uint8 _shot_grin_reaper_count = 0;
	uint16 _clue = 0;
	uint8 _shot_color = 0;
	uint8 _shot_direction = 0;
	uint8 _crystals_shot = 0;
	uint8 _crystal_state = 0;
	uint16 _picked_start_splitter = 0;
	bool _target_practice_reset = false;

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
	void _PlayErrorSound();
	void _DisplayShotFiredImage();
	void _DisplayShotFiredImage(Common::Point *point);
	void _DisplayShotLine(uint16 startX, uint16 startY, uint16 endX, uint16 endY);
	void _DisplayMultipleShotLines();
	void _EnableVideoFadeIn();
	uint16 _SceneToNumber(Scene *scene);
	uint16 _RandomUnusedScene(uint8 max);
	uint16 _RandomNumberInRange(uint16 min, uint16 max);
	uint16 _PickCrystalScene(uint16 scene1, uint16 scene2, uint16 scene3);

	// Script functions: RectHit
	void _rect_shotmenu(Rect *rect);
	void _rect_save(Rect *rect);
	void _rect_load(Rect *rect);
	void _rect_continue(Rect *rect);
	void _rect_start(Rect *rect);
	void _rect_easy(Rect *rect);
	void _rect_average(Rect *rect);
	void _rect_hard(Rect *rect);
	void _rect_default(Rect *rect);
	void _rect_kill_innocent_person(Rect *rect);
	void _rect_continue_junk_rings(Rect *rect);
	void _rect_shot_grin_reaper(Rect *rect);
	void _rect_show_mad_dog(Rect *rect);
	void _rect_pott_world_show_crystal(Rect *rect);
	void _rect_shot_left(Rect *rect);
	void _rect_shot_right(Rect *rect);
	void _rect_shot_gold(Rect *rect);
	void _rect_shot_silver(Rect *rect);
	void _rect_selected_dune_world(Rect *rect);
	void _rect_selected_junk_world(Rect *rect);
	void _rect_selected_dragons_teeth_world(Rect *rect);
	void _rect_selected_volcano_world(Rect *rect);
	void _rect_shot_red_death_grip(Rect *rect);
	void _rect_shot_blue_death_grip(Rect *rect);
	void _rect_shot_green_death_grip(Rect *rect);
	void _rect_shot_yellow(Rect *rect);
	void _rect_shot_blue(Rect *rect);
	void _rect_shot_red_crystal(Rect *rect);
	void _rect_shot_blue_crystal(Rect *rect);
	void _rect_shot_green_crystal(Rect *rect);
	void _rect_shot_black_dragon_1(Rect *rect);
	void _rect_shot_black_dragon_2(Rect *rect);
	void _rect_shot_black_dragon_3(Rect *rect);
	void _rect_do_flying_skull(Rect *rect);
	void _rect_skip_scene(Rect *rect);
	void _rect_hit_pirate_ship(Rect *rect);

	// Script functions: Scene PreOps
	void _scene_pso_fadein_video(Scene *scene);
	void _scene_pso_set_got_to(Scene *scene);
	void _scene_pso_set_got_to_no_fadein(Scene *scene);
	void _scene_pso_set_world_got_to(Scene *scene);

	// Script functions: Scene InsOps
	void _scene_iso_pick_a_world(Scene *scene);
	void _scene_iso_set_world_got_to(Scene *scene);

	// Script functions: Scene NxtScn
	void _scene_nxtscn_got_chewed_out(Scene *scene);
	void _scene_nxtscn_restart_from_last(Scene *scene);
	void _scene_nxtscn_player_died(Scene *scene);
	void _scene_nxtscn_misc_rooms_1(Scene *scene);
	void _scene_nxtscn_pick_dungeon_clue(Scene *scene);
	void _scene_nxtscn_continue_dungeon_clue(Scene *scene);
	void _scene_nxtscn_start_midship_random_scene(Scene *scene);
	void _scene_nxtscn_continue_midship_random_scene(Scene *scene);
	void _scene_nxtscn_show_death_grip_beam_color(Scene *scene);
	void _scene_nxtscn_select_asteroids(Scene *scene);
	void _scene_nxtscn_asteroids_done(Scene *scene);
	void _scene_nxtscn_do_flying_skulls(Scene *scene);
	void _scene_nxtscn_did_flying_skulls(Scene *scene);
	void _scene_nxtscn_show_which_start_splitter(Scene *scene);
	void _scene_nxtscn_goto_selected_world(Scene *scene);
	void _scene_nxtscn_start_volcano_popup(Scene *scene);
	void _scene_nxtscn_continue_volcano_popup(Scene *scene);
	void _scene_nxtscn_give_falina_clue(Scene *scene);
	void _scene_nxtscn_check_falina_clues(Scene *scene);
	void _scene_nxtscn_setup_falina_target_practice(Scene *scene);
	void _scene_nxtscn_continue_falina_target_practice(Scene *scene);
	void _scene_nxtscn_start_dune_popup(Scene *scene);
	void _scene_nxtscn_continue_dune_popup(Scene *scene);
	void _scene_nxtscn_pott_or_pan_shoots(Scene *scene);
	void _scene_nxtscn_setup_pott_target_practice(Scene *scene);
	void _scene_nxtscn_continue_pott_target_practice(Scene *scene);
	void _scene_nxtscn_start_dragons_teeth_popup(Scene *scene);
	void _scene_nxtscn_continue_dragons_teeth_popup(Scene *scene);
	void _scene_nxtscn_grin_reaper_clue(Scene *scene);
	void _scene_nxtscn_start_grin_reaper(Scene *scene);
	void _scene_nxtscn_continue_grin_reaper(Scene *scene);
	void _scene_nxtscn_grin_target_practice(Scene *scene);
	void _scene_nxtscn_continue_grin_target_practice(Scene *scene);
	void _scene_nxtscn_start_junk_world(Scene *scene);
	void _scene_nxtscn_continue_junk_world(Scene *scene);
	void _scene_nxtscn_start_junk_rings(Scene *scene);
	void _scene_nxtscn_show_junk_world_crystal(Scene *scene);
	void _scene_nxtscn_start_junk_world_target_practice(Scene *scene);
	void _scene_nxtscn_continue_junk_world_target_practice(Scene *scene);
	void _scene_nxtscn_are_all_worlds_done(Scene *scene);
	void _scene_nxtscn_start_practice_pirate_ship(Scene *scene);
	void _scene_nxtscn_more_practice_pirate_ship(Scene *scene);
	void _scene_nxtscn_player_won(Scene *scene);

	// Script functions: MissedRect
	void _scene_missedrects_default(Scene *scene);
	void _scene_missedrects_missed_pirate_ship(Scene *scene);

	// Script functions: Scene WepDwn
	void _scene_default_wepdwn(Scene *scene);

	// Script functions: Scene ScnScr
	void _scene_default_scnscr(Scene *scene);
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
