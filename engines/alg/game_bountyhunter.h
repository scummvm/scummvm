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
	GameBountyHunter(AlgEngine *vm, const ADGameDescription *desc);
	~GameBountyHunter();
	Common::Error run();
	void debug_warpTo(int val);

private:
	void init();
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
	Graphics::Surface _shotIcon;
	Graphics::Surface _emptyIcon;
	Graphics::Surface _liveIcon;
	Graphics::Surface _deadIcon;
	Graphics::Surface _diffIcon;
	Graphics::Surface _bulletholeIcon;
	Graphics::Surface _playersIcon1;
	Graphics::Surface _playersIcon2;
	Graphics::Surface _textScoreIcon;
	Graphics::Surface _textMenuIcon;
	Graphics::Surface _textBlackBarIcon;
	Common::Array<Graphics::Surface> *_bagsIcons;
	Common::Array<Graphics::Surface> *_shotgun;

	// sounds
	Audio::SeekableAudioStream *_shotgunSound = nullptr;

	// constants
	const uint16 _randomHarry1[7] = {0x01B9, 0x01B7, 0x01B5, 0x01B3, 0x01AF, 0x01AD, 0};
	const uint16 _randomHarry2[6] = {0x0194, 0x0190, 0x018E, 0x018C, 0};
	const uint16 _randomDan1[5] = {0x0173, 0x0171, 0x016F, 0x016D, 0};
	const uint16 _randomDan1TwoPlayer[6] = {0x0173, 0x0171, 0x016F, 0x016D, 0x016B, 0};
	const uint16 _randomDan2[7] = {0x0165, 0x0163, 0x0161, 0x015F, 0x015D, 0x015B, 0};
	const uint16 _randomLoco1[4] = {0xF7, 0xF5, 0xF3, 0};
	const uint16 _randomLoco2[3] = {0xED, 0xEB, 0};
	const uint16 _randomKid1[4] = {0xBA, 0xB7, 0xB5, 0};
	const uint16 _randomKid2[4] = {0xB1, 0xAE, 0xAC, 0};

	const uint16 *_randomScenes[8] = {_randomHarry1, _randomHarry2, _randomDan1, _randomDan2, _randomLoco1, _randomLoco2, _randomKid1, _randomKid2};
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

	const uint16 _firstSceneInScenario[4] = {4, 0x36, 0x36, 0x66};
	const uint16 _moneyScenes[4] = {0x017D, 0x013C, 0xC3, 0x69};
	const uint16 _gunfightScenarios[18] = {0x0116, 0x0118, 0x011B, 0x011D, 0x011F, 0x0121, 0x0123, 0x0125, 0x0127,
										  0x0129, 0x012B, 0x012D, 0x012F, 0x0131, 0x0133, 0x0135, 0x0137, 0x0139};
	const uint16 _innocentScenarios[5] = {0x0110, 0x010F, 0x010C, 0x010B, 0};
	const uint16 _deathScenarios[9] = {0x0100, 0x0101, 0x0102, 0x0103, 0x0104, 0x0105, 0x0106, 0x0107, 0};
	const uint16 _onePlayerOfTwoDead[2] = {0x0109, 0x010A};
	const uint16 _allPlayersDead = 0x108;

	bool _isDemo = 0;

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

	uint16 usedScenes = 0;
	int16 lastPick = -1;
	int16 initted = 0;
	int16 sceneCount = 0;

	uint16 *_random_scene_list;
	uint8 _random_max = 0;
	uint16 _random_mask = 0;
	int16 _random_picked = 0;
	uint8 _random_scene_count = 0;
	bool _gunfight_initialized = false;
	uint16 _gunfight_mask = 0;
	int16 _gunfight_picked = 0;
	uint8 _gunfight_scene_count = 0;
	bool _innocent_initialized = false;
	uint16 _innocent_mask = 0;
	int16 _innocent_picked = 0;
	uint8 _innocent_scene_count = 0;
	bool _death_initialized = false;
	uint16 _death_mask = 0;
	int16 _death_picked = 0;
	uint8 _death_scene_count = 0;

	uint8 _continuesUsed = 0;
	bool _wounded = false;
	uint16 _mainWounds = 0;
	int8 _gunfightCount = 0;
	bool _given = false;
	uint32 _firstDrawFrame = 0;
	uint8 _count = 0;

	uint8 _unk_2ADA6 = 0;

	// base functions
	void _NewGame();
	void _DoMenu();
	void _DoCursor();
	void _UpdateMouse();
	void _MoveMouse();
	void _DisplayLivesLeft(uint8 player);
	void _DisplayScores(uint8 player);
	void _DisplayShotsLeft(uint8 player);
	bool _WeaponDown();
	bool _SaveState();
	bool _LoadState();

	// misc game functions
	void _SetNextScene(uint16 sceneId);
	void _DisplayShotFiredImage(Common::Point *point);
	void _EnableVideoFadeIn();
	void _IconShotgun();
	void _IconReset();
	uint16 _BeginLevel(uint8 levelNumber);
	uint16 _RandomUnusedInt(uint8 max, uint16 *mask, uint16 exclude);
	uint16 _PickRandomScene(const uint16 *sceneList, uint8 max);
	uint16 _PickGunfightScene();
	uint16 _PickInnocentScene();
	uint16 _PickDeathScene();
	uint16 _TimeForGunfight();
	void _WaitingForShootout(uint32 drawFrame);
	void _DoShotgunSound();

	// Script functions: RectHit
	void _rect_shotmenu(Rect *rect);
	void _rect_save(Rect *rect);
	void _rect_load(Rect *rect);
	void _rect_continue(Rect *rect);
	void _rect_start(Rect *rect);
	void _rect_toggle_players(Rect *rect);
	void _rect_hit_icon_jug(Rect *rect);
	void _rect_hit_icon_lantern(Rect *rect);
	void _rect_hit_icon_skull(Rect *rect);
	void _rect_hit_icon_wheel(Rect *rect);
	void _rect_hit_select_harry(Rect *rect);
	void _rect_hit_select_dan(Rect *rect);
	void _rect_hit_select_loco(Rect *rect);
	void _rect_hit_select_kid(Rect *rect);
	void _rect_hit_kill_man(Rect *rect);
	void _rect_hit_give_shotgun(Rect *rect);
	void _rect_hit_kill3(Rect *rect);
	void _rect_hit_check_shotgun(Rect *rect);
	void _rect_hit_cheater(Rect *rect);

	// Script functions: Scene PreOps
	void _scene_pso_shootout(Scene *scene);
	void _scene_pso_wounded_main(Scene *scene);
	void _scene_pso_gunfight_setup(Scene *scene);
	void _scene_pso_lose_a_life(Scene *scene);
	void _scene_pso_setup_ndrandom1(Scene *scene);
	void _scene_pso_set_current_scene(Scene *scene);

	// Script functions: Scene InsOps
	void _scene_iso_shootout(Scene *scene);
	void _scene_iso_givemoney(Scene *scene);

	// Script functions: Scene NxtScn
	void _scene_nxtscn_lose_a_life(Scene *scene);
	void _scene_nxtscn_continue_game(Scene *scene);
	void _scene_nxtscn_did_not_continue(Scene *scene);
	void _scene_nxtscn_kill_innocent_man(Scene *scene);
	void _scene_nxtscn_kill_innocent_woman(Scene *scene);
	void _scene_nxtscn_after_die(Scene *scene);
	void _scene_nxtscn_goto_level_select(Scene *scene);
	void _scene_nxtscn_continue_random(Scene *scene);
	void _scene_nxtscn_init_random_harry1(Scene *scene);
	void _scene_nxtscn_init_random_harry2(Scene *scene);
	void _scene_nxtscn_init_random_dan1(Scene *scene);
	void _scene_nxtscn_init_random_dan2(Scene *scene);
	void _scene_nxtscn_init_random_loco1(Scene *scene);
	void _scene_nxtscn_init_random_loco2(Scene *scene);
	void _scene_nxtscn_init_random_kid1(Scene *scene);
	void _scene_nxtscn_init_random_kid2(Scene *scene);
	void _scene_nxtscn_next_sub_level(Scene *scene);
	void _scene_nxtscn_goto_bad_guy(Scene *scene);
	void _scene_nxtscn_auto_select_level(Scene *scene);
	void _scene_nxtscn_select_scenario(Scene *scene);
	void _scene_nxtscn_finish_scenario(Scene *scene);
	void _scene_nxtscn_game_won(Scene *scene);
	void _scene_nxtscn_killed_main(Scene *scene);
	void _scene_nxtscn_wounded_main(Scene *scene);
	void _scene_nxtscn_end_level(Scene *scene);
	void _scene_nxtscn_end_game(Scene *scene);
	void _scene_nxtscn_do_breakout_mains(Scene *scene);
	void _scene_nxtscn_died_refed(Scene *scene);
	void _scene_nxtscn_give_shotgun(Scene *scene);
	void _scene_nxtscn_check_2players(Scene *scene);

	// Script functions: Scene WepDwn
	void _scene_default_wepdwn(Scene *scene);
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
