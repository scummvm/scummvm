/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "bladerunner/script/scene_script.h"

namespace BladeRunner {

static int kPoliceMazePS11TargetCount = 20;

void SceneScriptPS11::InitializeScene() {
	if (Game_Flag_Query(kFlagPS10toPS11)) {
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, 0, false);
		Scene_Loop_Set_Default(1);
		Game_Flag_Reset(kFlagPS10toPS11);
		Setup_Scene_Information(World_Waypoint_Query_X(6), World_Waypoint_Query_Y(6), World_Waypoint_Query_Z(6), 840);
	} else {
		Scene_Loop_Set_Default(1);
		Setup_Scene_Information(World_Waypoint_Query_X(7), World_Waypoint_Query_Y(7), World_Waypoint_Query_Z(7), 132);
	}
	Scene_Exit_Add_2D_Exit(0, 0, 460, 639, 479, 2);
	Scene_Exit_Add_2D_Exit(1, 0,   0,  20, 479, 3);
}

static int track_data_9[] = {-26, 11, 20, -18, 11, 20, -9, 9, -5, 50, -1, 0, -7, 5000, 5000, -8, 9, -10, 31, 33, -15, 9, 1, -22, 9, -2, 7, -3, 1000, -24, 27, 33, -10, 34, 33, -2, 0, -3, 500, -9, 9, -13, 15, 10, -12, 9, -4};
static int track_data_10[] = {-26, 11, 20, -18, 11, 20, -9, 10, -9, 11, -15, 10, 1, -15, 11, 1, -5, 860, -1, 0, -7, 3000, 6000, -23, 10, -8, 10, -10, 33, 33, -2, 14, -3, 500, -11, 11, -9, 10, -8, 11, -12, 10, -1, 0, -4};
static int track_data_11[] = {-5, 860, -1, 0, -23, 11, -8, 11, -2, 25, -3, 500, -22, 11, -10, 32, 33, -6, 644, 80, -3, 0, -6, 388, 80, -3, 1000, -24, 12, 33, -2, 79, -9, 11, -13, 15, 9, -12, 11, -1, 0, -5, 860, -4};
static int track_data_12[] = {-26, 11, 20, -18, 11, 20, -9, 12, -5, 725, -1, 0, -3, 2000, -15, 12, 1, -23, 12, -8, 12, -2, 82, -25, -3, 1000, -6, 570, 80, -3, 0, -6, 462, 80, -3, 0, -6, 213, 80, -3, 1000, -2, 0, -6, 725, 80, -2, 99, -9, 12, -11, 27, -12, 12, -1, 0, -4};
static int track_data_13[] = {-26, 11, 20, -18, 11, 20, -9, 13, -5, 340, -1, 0, -7, 4000, 8000, -8, 13, -10, 33, 33, -15, 13, 1, -22, 13, -2, 4, -3, 0, -6, 435, 80, -3, 0, -6, 530, 80, -3, 100, -6, 435, 80, -3, 0, -6, 340, 80, -3, 0, -6, 260, 80, -3, 0, -6, 180, 80, -3, 100, -6, 260, 80, -3, 0, -6, 340, 80, -3, 200, -24, 27, 33, -10, 34, 33, -2, 0, -9, 13, -13, 14, 18, -12, 13, -4};
static int track_data_14[] = {-26, 11, 20, -18, 11, 20, -9, 14, -15, 14, 1, -23, 14, -5, 900, -1, 0, -7, 3000, 6000, -8, 14, -10, 33, 33, -2, 5, -3, 500, -22, 14, -6, 644, 80, -3, 0, -6, 388, 80, -3, 1000, -24, 27, 33, -10, 34, 33, -2, 0, -9, 14, -13, 18, 13, -12, 14, -4};
static int track_data_15[] = {-26, 11, 20, -18, 11, 20, -9, 15, -9, 16, -15, 15, 1, -15, 16, 1, -5, 860, -1, 0, -7, 3000, 7000, -8, 15, -10, 29, 33, -23, 15, -2, 14, -25, -3, 1000, -11, 16, -9, 15, -8, 16, -12, 15, -1, 0, -4};
static int track_data_16[] = {-5, 860, -1, 0, -8, 16, -23, 16, -2, 25, -3, 500, -10, 32, 33, -6, 644, 100, -3, 0, -6, 388, 200, -3, 500, -2, 79, -25, -9, 16, -13, 10, 9, -12, 16, -1, 0, -5, 860, -4};
static int track_data_17[] = {-26, 11, 20, -18, 11, 20, -5, 310, -1, 0, -8, 17, -15, 17, 1, -22, 17, -7, 4000, 8000, -10, 32, 33, -2, 10, -3, 0, -24, 27, 33, -2, 0, -15, 17, 1, -22, 17, -2, 24, -15, 17, 1, -22, 17, -2, 10, -3, 0, -24, 27, 33, -2, 24, -3, 1000, -9, 17, -13, 23, 22, -12, 17, -1, 0, -4};
static int track_data_18[] = {-26, 11, 20, -18, 11, 20, -9, 18, -9, 19, -5, 900, -1, 0, -15, 18, 1, -15, 19, 1, -23, 18, -7, 4000, 6000, -8, 18, -2, 5, -10, 19, 33, -10, 3, 33, -3, 1000, -25, -6, 700, 80, -22, 18, -3, 0, -6, 512, 200, -3, 1000, -24, 12, 33, -11, 19, -9, 18, -8, 19, -12, 18, -1, 0, -4};
static int track_data_19[] = {-5, 512, -1, 0, -22, 19, -2, 8, -3, 4000, -15, 19, 1, -22, 19, -2, 2, -10, 32, 33, -3, 1000, -24, 12, 33, -2, 19, -3, 500, -9, 19, -13, 13, 14, -12, 19, -1, 0, -4};
static int track_data_20[] = {-26, 11, 20, -18, 11, 20, -9, 20, -5, 280, -1, 0, -7, 5000, 7000, -15, 20, 1, -22, 20, -8, 20, -2, 9, -10, 32, 33, -3, 1000, -24, 27, 33, -2, 0, -9, 20, -13, 21, 12, -12, 20, -1, 0, -4};
static int track_data_21[] = {-26, 11, 20, -18, 11, 20, -9, 21, -5, 280, -1, 0, -7, 5000, 8000, -15, 21, 1, -23, 21, -8, 21, -2, 5, -25, -3, 1000, -2, 0, -9, 21, -13, 20, 12, -12, 21, -1, 0, -4};
static int track_data_22[] = {-26, 11, 20, -18, 11, 20, -9, 22, -5, 255, -1, 0, -15, 22, 1, -22, 22, -7, 5000, 5000, -8, 22, -2, 7, -10, 32, 33, -3, 1000, -24, 12, 33, -2, 0, -9, 22, -13, 23, 17, -12, 22, -1, 0, -4};
static int track_data_23[] = {-26, 11, 20, -18, 11, 20, -9, 23, -5, 310, -1, 0, -7, 3000, 6000, -15, 23, 1, -23, 23, -8, 23, -3, 1000, -2, 24, -3, 1000, -2, 0, -25, -9, 23, -13, 22, 17, -12, 23, -1, 0, -4};
static int track_data_27[] = {-26, 11, 20, -18, 11, 20, -9, 27, -5, 346, -1, 0, -3, 0, -15, 27, 1, -22, 27, -8, 27, -2, 14, -3, 1000, -24, 12, 33, -2, 0, -9, 27, -13, 21, 20, -12, 27, -1, 0, -4};

void SceneScriptPS11::SceneLoaded() {
	Obstacle_Object("PARKMETR01", true);
	Obstacle_Object("PARKMETR02", true);
	Obstacle_Object("PARKMETR03", true);
	Obstacle_Object("PARKMETR07", true);
	Obstacle_Object("PARKMETR08", true);
	Obstacle_Object("PARKMETR10", true);
	Obstacle_Object("PARKMETR11", true);
	Obstacle_Object("PARKMETR15", true);
	Obstacle_Object("PARKMETR16", true);
	Unclickable_Object("PARKMETR01");
	Unclickable_Object("PARKMETR02");
	Unclickable_Object("PARKMETR03");
	Unclickable_Object("PARKMETR07");
	Unclickable_Object("PARKMETR08");
	Unclickable_Object("PARKMETR10");
	Unclickable_Object("PARKMETR11");
	Unclickable_Object("PARKMETR15");
	Unclickable_Object("PARKMETR16");
	if (!Query_System_Currently_Loading_Game()) {
		Item_Add_To_World(kItemPS11Target1,  449, kSetPS10_PS11_PS12_PS13,  -450.0f,  -7.5f, 335.0f,  50, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS11Target2,  449, kSetPS10_PS11_PS12_PS13,  -740.0f,  27.0f, -30.0f, 860, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS11Target3,  449, kSetPS10_PS11_PS12_PS13,  -740.0f,  99.0f, -30.0f, 860, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS11Target4,  441, kSetPS10_PS11_PS12_PS13,  -400.0f, -9.23f, -75.0f, 725, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS11Target5,  443, kSetPS10_PS11_PS12_PS13, -803.72f, -72.7f, 60.22f, 340, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS11Target6,  443, kSetPS10_PS11_PS12_PS13,  -853.0f, -70.0f, 195.0f, 900, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS11Target7,  447, kSetPS10_PS11_PS12_PS13,  -740.0f,  27.0f, -30.0f, 860, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS11Target8,  447, kSetPS10_PS11_PS12_PS13,  -740.0f,  99.0f, -30.0f, 860, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS11Target9,  445, kSetPS10_PS11_PS12_PS13,  -888.0f, 155.0f, 100.0f, 310, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS11Target10, 443, kSetPS10_PS11_PS12_PS13,  -430.0f, 164.0f,  11.0f, 900, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS11Target11, 443, kSetPS10_PS11_PS12_PS13,  -430.0f, -0.86f,  11.0f, 512, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS11Target12, 443, kSetPS10_PS11_PS12_PS13,  -891.0f,   3.1f,  90.0f, 280, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS11Target13, 447, kSetPS10_PS11_PS12_PS13,  -891.0f,   3.1f,  90.0f, 280, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS11Target14, 445, kSetPS10_PS11_PS12_PS13,  -891.0f, 171.0f, 190.0f, 255, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS11Target15, 441, kSetPS10_PS11_PS12_PS13,  -888.0f, 155.0f,  30.0f, 310, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS11Target16, 445, kSetPS10_PS11_PS12_PS13,  -800.0f, -9.23f, -75.0f, 346, 72, 36, true, false, false, true);
	}

	Police_Maze_Target_Track_Add(kItemPS11Target1,   -450.0f,  -7.5f, 335.0f,  -450.0f,  -7.5f, 295.0f,   8, track_data_9,   true);
	Police_Maze_Target_Track_Add(kItemPS11Target2,   -740.0f,  27.0f, -30.0f,  -740.0f,  99.0f, -30.0f,  15, track_data_10, false);
	Police_Maze_Target_Track_Add(kItemPS11Target3,   -740.0f,  99.0f, -30.0f,  -200.0f,  99.0f, -30.0f,  80, track_data_11, false);
	Police_Maze_Target_Track_Add(kItemPS11Target4,   -400.0f, -9.23f, -75.0f,  -800.0f, -9.23f, -75.0f, 100, track_data_12, false);
	Police_Maze_Target_Track_Add(kItemPS11Target5,  -803.72f, -72.7f, 60.22f, -803.72f,  -0.7f, 60.22f,   6, track_data_13,  true);
	Police_Maze_Target_Track_Add(kItemPS11Target6,   -853.0f, -70.0f, 195.0f,  -853.0f,   2.0f, 195.0f,   6, track_data_14, false);
	Police_Maze_Target_Track_Add(kItemPS11Target7,   -740.0f,  27.0f, -30.0f,  -740.0f,  99.0f, -30.0f,  15, track_data_15, false);
	Police_Maze_Target_Track_Add(kItemPS11Target8,   -740.0f,  99.0f, -30.0f,  -200.0f,  99.0f, -30.0f,  80, track_data_16, false);
	Police_Maze_Target_Track_Add(kItemPS11Target9,   -888.0f, 155.0f, 100.0f,  -888.0f, 155.0f,  30.0f,  25, track_data_17, false);
	Police_Maze_Target_Track_Add(kItemPS11Target10,  -430.0f, 164.0f,  11.0f,  -430.0f, -0.86f,  11.0f,   6, track_data_18, false);
	Police_Maze_Target_Track_Add(kItemPS11Target11,  -430.0f, -0.86f,  11.0f,  -300.0f, -0.86f, -80.0f,  20, track_data_19, false);
	Police_Maze_Target_Track_Add(kItemPS11Target12,  -891.0f,   3.1f,  90.0f,  -891.0f,   3.1f, 105.0f,  10, track_data_20,  true);
	Police_Maze_Target_Track_Add(kItemPS11Target13,  -891.0f,   3.1f,  90.0f,  -891.0f,   3.1f, 105.0f,   6, track_data_21, false);
	Police_Maze_Target_Track_Add(kItemPS11Target14,  -891.0f, 171.0f, 190.0f,  -891.0f, 171.0f, 147.0f,   8, track_data_22, false);
	Police_Maze_Target_Track_Add(kItemPS11Target15,  -888.0f, 155.0f,  30.0f,  -888.0f, 155.0f, 100.0f,  25, track_data_23,  true);
	Police_Maze_Target_Track_Add(kItemPS11Target16,  -800.0f, -9.23f, -75.0f,  -740.0f, -9.23f, -75.0f,  15, track_data_27, false);
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(0);
	Ambient_Sounds_Add_Looping_Sound(387, 50, 1, 1);
	Ambient_Sounds_Add_Looping_Sound( 54, 50, 1, 1);
	Ambient_Sounds_Add_Sound(  1, 10,  50, 16, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(389,  5,  50, 16, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(390,  6,  50, 16, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(443,  2, 100, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(444,  2, 100, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(445,  2, 100, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(446,  2, 100, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(303,  5, 100, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(304,  5, 100, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(305,  5, 100, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(306,  5, 100, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(307,  5, 100, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(308,  5, 100, 17, 27, -100, 100, -101, -101, 0, 0);
}

bool SceneScriptPS11::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptPS11::ClickedOn3DObject(const char *objectName, bool combatMode) {
	return false;
}

bool SceneScriptPS11::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptPS11::ClickedOnItem(int itemId, bool combatMode) {
	if (Player_Query_Combat_Mode()) {
		switch (itemId) {
		case kItemPS11Target4:
			Sound_Play(555, 50, 0, 0, 50);
			break;
		case kItemPS11Target7:
			Sound_Play(4, 50, 0, 0, 50);
			break;
		case kItemPS11Target8:
			Sound_Play(4, 50, 0, 0, 50);
			break;
		case kItemPS11Target13:
			Sound_Play(4, 50, 0, 0, 50);
			break;
		case kItemPS11Target15:
			Sound_Play(555, 50, 0, 0, 50);
			break;
		default:
			Sound_Play(2, 12, 0, 0, 50);
			break;
		}
		Item_Spin_In_World(itemId);
		if (itemId == kItemPS11Target1) {
			Item_Flag_As_Non_Target(kItemPS11Target1);
		}
		if (itemId == kItemPS11Target2) {
			Item_Flag_As_Non_Target(kItemPS11Target2);
			Item_Flag_As_Non_Target(kItemPS11Target3);
		}
		if (itemId == kItemPS11Target3) {
			Item_Flag_As_Non_Target(kItemPS11Target2);
			Item_Flag_As_Non_Target(kItemPS11Target3);
		}
		if (itemId == kItemPS11Target4) {
			Item_Flag_As_Non_Target(kItemPS11Target4);
		}
		if (itemId == kItemPS11Target5) {
			Item_Flag_As_Non_Target(kItemPS11Target5);
		}
		if (itemId == kItemPS11Target6) {
			Item_Flag_As_Non_Target(kItemPS11Target6);
		}
		if (itemId == kItemPS11Target7) {
			Item_Flag_As_Non_Target(kItemPS11Target7);
			Item_Flag_As_Non_Target(kItemPS11Target8);
		}
		if (itemId == kItemPS11Target8) {
			Item_Flag_As_Non_Target(kItemPS11Target7);
			Item_Flag_As_Non_Target(kItemPS11Target8);
		}
		if (itemId == kItemPS11Target9) {
			Item_Flag_As_Non_Target(kItemPS11Target9);
		}
		if (itemId == kItemPS11Target10) {
			Item_Flag_As_Non_Target(kItemPS11Target10);
			Item_Flag_As_Non_Target(kItemPS11Target11);
		}
		if (itemId == kItemPS11Target11) {
			Item_Flag_As_Non_Target(kItemPS11Target10);
			Item_Flag_As_Non_Target(kItemPS11Target11);
		}
		if (itemId == kItemPS11Target12) {
			Item_Flag_As_Non_Target(kItemPS11Target12);
		}
		if (itemId == kItemPS11Target13) {
			Item_Flag_As_Non_Target(kItemPS11Target13);
		}
		if (itemId == kItemPS11Target14) {
			Item_Flag_As_Non_Target(kItemPS11Target14);
		}
		if (itemId == kItemPS11Target15) {
			Item_Flag_As_Non_Target(kItemPS11Target15);
		}
		if (itemId == kItemPS11Target16) {
			Item_Flag_As_Non_Target(kItemPS11Target16);
		}
		return true;
	}
	return false;
}

bool SceneScriptPS11::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_Waypoint(kActorMcCoy, 6, 12, true, false)) {
			Game_Flag_Set(kFlagPS11toPS10);
			removeTargets();
			Set_Enter(kSetPS10_PS11_PS12_PS13, kScenePS10);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_Waypoint(kActorMcCoy, 8, 12, true, false)) {
			Game_Flag_Set(kFlagPS11toPS12);
			removeTargets();
			Global_Variable_Decrement(kVariablePoliceMazeScore, kPoliceMazePS11TargetCount - Global_Variable_Query(kVariablePoliceMazePS11TargetCounter));
			Global_Variable_Set(kVariablePoliceMazePS11TargetCounter, kPoliceMazePS11TargetCount);
			Set_Enter(kSetPS10_PS11_PS12_PS13, kScenePS12);
		}
		return true;
	}
	return false;
}

bool SceneScriptPS11::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptPS11::SceneFrameAdvanced(int frame) {
}

void SceneScriptPS11::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptPS11::PlayerWalkedIn() {
	Police_Maze_Set_Pause_State(false);
}

void SceneScriptPS11::PlayerWalkedOut() {
}

void SceneScriptPS11::DialogueQueueFlushed(int a1) {
}

void SceneScriptPS11::removeTargets() {
	Item_Remove_From_World(kItemPS11Target1);
	Item_Remove_From_World(kItemPS11Target2);
	Item_Remove_From_World(kItemPS11Target3);
	Item_Remove_From_World(kItemPS11Target4);
	Item_Remove_From_World(kItemPS11Target5);
	Item_Remove_From_World(kItemPS11Target6);
	Item_Remove_From_World(kItemPS11Target7);
	Item_Remove_From_World(kItemPS11Target8);
	Item_Remove_From_World(kItemPS11Target9);
	Item_Remove_From_World(kItemPS11Target10);
	Item_Remove_From_World(kItemPS11Target11);
	Item_Remove_From_World(kItemPS11Target12);
	Item_Remove_From_World(kItemPS11Target13);
	Item_Remove_From_World(kItemPS11Target14);
	Item_Remove_From_World(kItemPS11Target15);
	Item_Remove_From_World(kItemPS11Target16);
}

} // End of namespace BladeRunner
