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

static int kPoliceMazePS12TargetCount = 20;

void SceneScriptPS12::InitializeScene() {
	Police_Maze_Set_Pause_State(true);
	if (Game_Flag_Query(kFlagPS11toPS12)) {
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, 0, false);
		Scene_Loop_Set_Default(1);
		Setup_Scene_Information(World_Waypoint_Query_X(8), World_Waypoint_Query_Y(8), World_Waypoint_Query_Z(8), 512);
	} else {
		Scene_Loop_Set_Default(1);
		Setup_Scene_Information(World_Waypoint_Query_X(9), World_Waypoint_Query_Y(9), World_Waypoint_Query_Z(9), 0);
	}
	Scene_Exit_Add_2D_Exit(0, 0, 460, 639, 479, 2);
	Scene_Exit_Add_2D_Exit(1, 0,   5, 110, 196, 3);
}

static int track_data_29[] = {-26, 12, 20, -18, 12, 20, -9, 29, -5, 200, -1, 0, -7, 4000, 10000, -8, 29, -10, 33, 33, -15, 29, 1, -22, 29, -2, 5, -3, 500, -24, 27, 33, -10, 34, 33, -2, 0, -9, 29, -13, 44, 42, -12, 29, -4};
static int track_data_30[] = {-26, 12, 20, -18, 12, 20, -9, 30, -5, 67, -1, 0, -7, 3000, 9000, -8, 30, -10, 33, 33, -15, 30, 1, -22, 30, -2, 5, -3, 500, -24, 27, 33, -10, 34, 33, -2, 0, -9, 30, -13, 41, 39, -12, 30, -15, 30, 0, -4};
static int track_data_31[] = {-26, 12, 20, -18, 12, 20, -15, 31, 1, -23, 31, -9, 31, -5, 480, -1, 0, -7, 3000, 10000, -8, 31, -2, 5, -10, 29, 33, -3, 1000, -6, 968, 100, -10, 29, 33, -3, 500, -2, 0, -10, 29, 33, -25, -9, 31, -14, 40, 32, 33, -12, 31, -4};
static int track_data_32[] = {-26, 12, 20, -18, 12, 20, -9, 32, -5, 1010, -1, 0, -7, 3000, 10000, -8, 32, -10, 33, 33, -15, 32, 1, -22, 32, -2, 5, -3, 500, -24, 27, 33, -6, 498, 80, -10, 29, 33, -3, 500, -2, 0, -9, 32, -14, 33, 31, 40, -12, 32, -4};
static int track_data_33[] = {-26, 12, 20, -18, 12, 20, -9, 33, -5, 540, -1, 0, -7, 4000, 10000, -8, 33, -15, 33, 1, -23, 33, -2, 5, -10, 29, 33, -3, 1000, -6, 284, 80, -3, 0, -6, 28, 80, -10, 29, 33, -3, 1000, -2, 0, -25, -9, 33, -14, 40, 31, 32, -12, 33, -4};
static int track_data_34[] = {-26, 12, 20, -18, 12, 20, -9, 34, -9, 35, -9, 36, -5, 469, -1, 0, -15, 34, 1, -15, 35, 1, -15, 36, 1, -7, 3000, 10000, -8, 34, -22, 34, -22, 35, -22, 36, -2, 5, -10, 29, 33, -3, 1000, -6, 376, 80, -3, 0, -6, 168, 80, -10, 29, 33, -2, 9, -6, 33, 80, -3, 0, -6, 15, 80, -10, 29, 33, -3, 500, -24, 27, 33, -2, 5, -10, 29, 33, -6, 168, 80, -3, 0, -6, 376, 80, -3, 0, -6, 469, 80, -10, 29, 33, -3, 500, -15, 34, 1, -15, 35, 1, -15, 36, 1, -22, 34, -22, 35, -22, 36, -6, 376, 80, -3, 0, -6, 168, 80, -2, 9, -6, 33, 80, -3, 0, -6, 15, 80, -10, 29, 33, -3, 500, -24, 27, 33, -2, 5, -10, 29, 33, -3, 0, -6, 469, 80, -3, 500, -6, 198, 80, -10, 29, 33, -3, 0, -2, 19, -10, 29,33, -3, 0, -9, 34, -11, 35, -12, 34, -1, 0, -4};
static int track_data_35[] = {-8, 35, -5, 198, -1, 0, -6, 469, 80, -2, 9, -3, 0, -9, 35, -11, 36, -12, 35, -5, 198, -8, 35, -1, 0, -4};
static int track_data_36[] = {-8, 36, -5, 469, -1, 0, -8, 36, -2, 9, -10, 29, 33, -3, 1000, -9, 34, -9, 35, -9, 36, -11, 37, -12, 36, -5, 469, -8, 36, -1, 0, -4};
static int track_data_37[] = {-26, 12, 20, -18, 12, 20, -9, 37, -7, 3000, 6000, -11, 38, -5, 1010, -1, 0, -3, 2000, -8, 37, -10, 33, 33, -15, 37, 1, -23, 37, -2, 9, -3, 3000, -10, 34, 33, -2, 0, -25, -9, 37, -12, 37, -1, 0, -4};
static int track_data_38[] = {-9, 38, -5, 990, -1, 0, -3, 3000, -8, 38, -10, 33, 33, -15, 38, 1, -22, 38, -2, 9, -3, 1000, -24, 12, 33, -10, 34, 33, -2, 0, -9, 38, -11, 34, -12, 38, -1, 0, -4};
static int track_data_39[] = {-26, 12, 20, -18, 12, 20, -9, 39, -5, 513, -1, 0, -7, 5000, 5000, -8, 39, -10, 33, 33, -15, 39, 1, -2, 5, -3, 1000, -22, 39, -6, 1010, 80, -10, 29, 33, -3, 500, -24, 27, 33, -10, 34, 33, -2, 0, -9, 39, -13, 41, 30, -12, 39, -4};
static int track_data_40[] = {-26, 12, 20, -18, 12, 20, -9, 40, -5, 480, -1, 0, -7, 4000, 8000, -8, 40, -10, 29, 33, -15, 40, 1, -2, 5, -10, 29, 33, -3, 500, -22, 40, -6, 968, 80, -10, 29, 33, -3, 1000, -24, 27, 33, -2, 0, -9, 40, -14, 31, 32, 33, -12, 40, -4};
static int track_data_41[] = {-26, 12, 20, -18, 12, 20, -9, 41, -5, 513, -1, 0, -7, 4000, 6000, -8, 41, -10, 33, 33, -15, 41, 1, -23, 41, -2, 5, -3, 500, -6, 1010, 80, -10, 29, 33, -3, 1000, -2, 0, -10, 34, 33, -25, -9, 41, -13, 39, 30, -12, 41, -4};
static int track_data_42[] = {-9, 42, -5, 109, -1, 0, -7, 2000, 5000, -8, 42, -10, 29, 33, -15, 42, 1, -22, 42, -2, 5, -10, 29, 33, -3, 1000, -24, 27, 33, -2, 0, -9, 42, -13, 44, 29, -12, 42, -4};
static int track_data_43[] = {-26, 12, 20, -18, 12, 20, -9, 43, -5, 540, -1, 0, -7, 5000, 7000, -8, 43, -10, 33, 33, -15, 43, 1, -23, 43, -2, 9, -3, 2000, -6, 284, 80, -3, 0, -6, 28, 80, -10, 29, 33, -3, 2000, -10, 34, 33, -2, 0, -25, -9, 43, -11, 45, -12, 43, -4};
static int track_data_44[] = {-26, 12, 20, -18, 12, 20, -9, 44, -5, 109, -1, 0, -7, 5000, 5000, -8, 44, -10, 29, 33, -15, 44, 1, -23, 44, -2, 5, -7, 2000, 2000, -10, 29, 33, -2, 0, -25, -9, 44, -13, 42, 29, -12, 44, -4};
static int track_data_45[] = {-26, 12, 20, -18, 12, 20, -9, 45, -5, 540, -1, 0, -7, 3000, 10000, -8, 45, -10, 33, 33, -15, 45, 1, -2, 9, -3, 1000, -22, 45, -6, 284, 80, -3, 0, -6, 28, 80, -3, 1000, -24, 27, 33, -10, 34, 33, -2, 0, -9, 45, -11, 43, -12, 45, -4};

void SceneScriptPS12::SceneLoaded() {
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
		Item_Add_To_World(kItemPS12Target1,  449, kSetPS10_PS11_PS12_PS13,  -691.8f, -9.06f, 587.67f,  200, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS12Target2,  445, kSetPS10_PS11_PS12_PS13,  -679.6f, -45.4f, 721.05f,   67, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS12Target3,  447, kSetPS10_PS11_PS12_PS13, -414.04f, -8.98f, 711.91f,  480, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS12Target4,  443, kSetPS10_PS11_PS12_PS13,  -440.0f, -8.97f, 1137.0f, 1010, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS12Target5,  441, kSetPS10_PS11_PS12_PS13, -764.92f, -0.84f, 950.22f,  540, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS12Target6,  449, kSetPS10_PS11_PS12_PS13,  -696.0f,  -5.7f, 1185.0f,  469, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS12Target7,  449, kSetPS10_PS11_PS12_PS13,  -635.0f,  -5.7f, 1165.0f,  198, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS12Target8,  449, kSetPS10_PS11_PS12_PS13,  -620.0f, -8.63f, 1366.0f,  469, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS12Target9,  447, kSetPS10_PS11_PS12_PS13,  -584.0f, -79.4f,  775.0f, 1010, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS12Target10, 445, kSetPS10_PS11_PS12_PS13,  -578.0f, -79.4f,  810.0f,  990, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS12Target11, 443, kSetPS10_PS11_PS12_PS13,  -400.0f, -12.0f, 1110.0f,  513, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS12Target12, 449, kSetPS10_PS11_PS12_PS13, -414.04f, -8.98f, 711.91f,  480, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS12Target13, 447, kSetPS10_PS11_PS12_PS13,  -400.0f, -12.0f, 1110.0f,  513, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS12Target14, 449, kSetPS10_PS11_PS12_PS13,  -731.0f, 93.66f,  788.0f,  109, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS12Target15, 441, kSetPS10_PS11_PS12_PS13,  -580.0f, -80.0f,  925.0f,  540, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS12Target16, 441, kSetPS10_PS11_PS12_PS13,  -731.0f, 93.66f,  788.0f,  109, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS12Target17, 443, kSetPS10_PS11_PS12_PS13,  -580.0f, -80.0f,  925.0f,  540, 72, 36, true, false, false, true);
	}
	Police_Maze_Target_Track_Add(kItemPS12Target1,   -691.8f, -9.06f, 587.67f, -649.11f, -9.06f, 587.71f,  6, track_data_29,  true);
	Police_Maze_Target_Track_Add(kItemPS12Target2,   -679.6f, -45.4f, 721.05f,  -679.6f,  -1.4f, 721.05f,  6, track_data_30,  true);
	Police_Maze_Target_Track_Add(kItemPS12Target3,  -414.04f, -8.98f, 711.91f, -459.54f, -8.99f, 707.81f,  6, track_data_31, false);
	Police_Maze_Target_Track_Add(kItemPS12Target4,   -440.0f, -8.97f, 1137.0f,  -430.0f, -8.97f,  921.0f,  6, track_data_32, false);
	Police_Maze_Target_Track_Add(kItemPS12Target5,  -764.92f, -0.84f, 950.22f, -722.92f, -0.84f, 950.22f,  6, track_data_33, false);
	Police_Maze_Target_Track_Add(kItemPS12Target6,   -696.0f,  -5.7f, 1185.0f,  -635.0f,  -5.7f, 1185.0f, 20, track_data_34, false);
	Police_Maze_Target_Track_Add(kItemPS12Target7,   -635.0f,  -5.7f, 1165.0f,  -620.0f, -8.63f, 1366.0f, 10, track_data_35, false);
	Police_Maze_Target_Track_Add(kItemPS12Target8,   -620.0f, -8.63f, 1366.0f,  -595.0f, -8.63f, 1366.0f, 10, track_data_36, false);
	Police_Maze_Target_Track_Add(kItemPS12Target9,   -584.0f, -79.4f,  775.0f,  -584.0f, -27.4f,  775.0f, 10, track_data_37,  true);
	Police_Maze_Target_Track_Add(kItemPS12Target10,  -578.0f, -79.4f,  810.0f,  -578.0f, -27.4f,  810.0f, 10, track_data_38, false);
	Police_Maze_Target_Track_Add(kItemPS12Target11,  -400.0f, -12.0f, 1110.0f,  -400.0f,  60.0f, 1110.0f,  6, track_data_39, false);
	Police_Maze_Target_Track_Add(kItemPS12Target12, -414.04f, -8.98f, 711.91f, -459.54f, -8.99f, 707.81f,  6, track_data_40,  true);
	Police_Maze_Target_Track_Add(kItemPS12Target13,  -400.0f, -12.0f, 1110.0f,  -400.0f,  60.0f, 1110.0f,  6, track_data_41, false);
	Police_Maze_Target_Track_Add(kItemPS12Target14,  -731.0f, 93.66f,  788.0f,  -702.0f, 93.66f,  788.0f,  6, track_data_42, false);
	Police_Maze_Target_Track_Add(kItemPS12Target15,  -580.0f, -80.0f,  925.0f,  -580.0f,  -8.0f,  925.0f, 10, track_data_43,  true);
	Police_Maze_Target_Track_Add(kItemPS12Target16,  -731.0f, 93.66f,  788.0f,  -702.0f, 93.66f,  788.0f,  6, track_data_44, false);
	Police_Maze_Target_Track_Add(kItemPS12Target17,  -580.0f, -80.0f,  925.0f,  -580.0f,  -8.0f,  925.0f, 10, track_data_45, false);
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(0);
	Ambient_Sounds_Add_Looping_Sound(387, 50, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(54, 50, 1, 1);
	Ambient_Sounds_Add_Sound(1,  10,  50, 16, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(389, 5,  50, 16, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(390, 6,  50, 16, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(443, 2, 100, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(444, 2, 100, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(445, 2, 100, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(446, 2, 100, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(303, 5, 100, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(304, 5, 100, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(305, 5, 100, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(306, 5, 100, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(307, 5, 100, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(308, 5, 100, 17, 27, -100, 100, -101, -101, 0, 0);
}

bool SceneScriptPS12::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptPS12::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptPS12::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptPS12::ClickedOnItem(int itemId, bool a2) {
	if (Player_Query_Combat_Mode()) {
		switch (itemId) {
		case kItemPS12Target3:
			Sound_Play(4, 50, 0, 0, 50);
			break;
		case kItemPS12Target5:
			Sound_Play(555, 50, 0, 0, 50);
			break;
		case kItemPS12Target9:
			Sound_Play(4, 50, 0, 0, 50);
			break;
		case kItemPS12Target13:
			Sound_Play(4, 50, 0, 0, 50);
			break;
		case kItemPS12Target15:
			Sound_Play(555, 50, 0, 0, 50);
			break;
		case kItemPS12Target16:
			Sound_Play(555, 50, 0, 0, 50);
			break;
		default:
			Sound_Play(2, 12, 0, 0, 50);
			break;
		}
		Item_Spin_In_World(itemId);
		Item_Flag_As_Non_Target(itemId);
		if (itemId == kItemPS12Target1) {
			Item_Flag_As_Non_Target(kItemPS12Target1);
		}
		if (itemId == kItemPS12Target2) {
			Item_Flag_As_Non_Target(kItemPS12Target2);
		}
		if (itemId == kItemPS12Target3) {
			Item_Flag_As_Non_Target(kItemPS12Target3);
		}
		if (itemId == kItemPS12Target4) {
			Item_Flag_As_Non_Target(kItemPS12Target4);
		}
		if (itemId == kItemPS12Target5) {
			Item_Flag_As_Non_Target(kItemPS12Target5);
		}
		if (itemId == kItemPS12Target6) {
			Item_Flag_As_Non_Target(kItemPS12Target6);
			Item_Flag_As_Non_Target(kItemPS12Target7);
			Item_Flag_As_Non_Target(kItemPS12Target8);
		}
		if (itemId == kItemPS12Target7) {
			Item_Flag_As_Non_Target(kItemPS12Target6);
			Item_Flag_As_Non_Target(kItemPS12Target7);
			Item_Flag_As_Non_Target(kItemPS12Target8);
		}
		if (itemId == kItemPS12Target8) {
			Item_Flag_As_Non_Target(kItemPS12Target6);
			Item_Flag_As_Non_Target(kItemPS12Target7);
			Item_Flag_As_Non_Target(kItemPS12Target8);
		}
		if (itemId == kItemPS12Target9) {
			Item_Flag_As_Non_Target(kItemPS12Target9);
		}
		if (itemId == kItemPS12Target10) {
			Item_Flag_As_Non_Target(kItemPS12Target10);
		}
		if (itemId == kItemPS12Target11) {
			Item_Flag_As_Non_Target(kItemPS12Target11);
		}
		if (itemId == kItemPS12Target12) {
			Item_Flag_As_Non_Target(kItemPS12Target12);
		}
		if (itemId == kItemPS12Target13) {
			Item_Flag_As_Non_Target(kItemPS12Target13);
		}
		if (itemId == kItemPS12Target14) {
			Item_Flag_As_Non_Target(kItemPS12Target14);
		}
		if (itemId == kItemPS12Target15) {
			Item_Flag_As_Non_Target(kItemPS12Target15);
		}
		if (itemId == kItemPS12Target16) {
			Item_Flag_As_Non_Target(kItemPS12Target16);
		}
		if (itemId == kItemPS12Target17) {
			Item_Flag_As_Non_Target(kItemPS12Target17);
		}
		return true;
	}
	return false;
}

bool SceneScriptPS12::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_Waypoint(kActorMcCoy, 8, 12, true, false)) {
			Game_Flag_Set(kFlagPS12toPS11);
			removeTargets();
			Set_Enter(kSetPS10_PS11_PS12_PS13, kScenePS11);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_Waypoint(kActorMcCoy, 9, 12, true, false)) {
			Player_Loses_Control();
			Loop_Actor_Walk_To_Waypoint(kActorMcCoy, 10, 12, false, false);
			Player_Gains_Control();
			Game_Flag_Set(kFlagPS12toPS13);
			removeTargets();
			Global_Variable_Decrement(kVariablePoliceMazeScore, kPoliceMazePS12TargetCount - Global_Variable_Query(kVariablePoliceMazePS12TargetCounter));
			Global_Variable_Set(kVariablePoliceMazePS12TargetCounter, kPoliceMazePS12TargetCount);
			Set_Enter(kSetPS10_PS11_PS12_PS13, kScenePS13);
		}
		return true;
	}
	return false;
}

bool SceneScriptPS12::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptPS12::SceneFrameAdvanced(int frame) {
}

void SceneScriptPS12::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptPS12::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagPS11toPS12)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -546.0f, -9.06f, 570.0f, 0, true, false, 0);
		Game_Flag_Reset(kFlagPS11toPS12);
	}
	Police_Maze_Set_Pause_State(false);
}

void SceneScriptPS12::PlayerWalkedOut() {
}

void SceneScriptPS12::DialogueQueueFlushed(int a1) {
}

void SceneScriptPS12::removeTargets() {
	Item_Remove_From_World(kItemPS12Target1);
	Item_Remove_From_World(kItemPS12Target2);
	Item_Remove_From_World(kItemPS12Target3);
	Item_Remove_From_World(kItemPS12Target4);
	Item_Remove_From_World(kItemPS12Target5);
	Item_Remove_From_World(kItemPS12Target6);
	Item_Remove_From_World(kItemPS12Target7);
	Item_Remove_From_World(kItemPS12Target8);
	Item_Remove_From_World(kItemPS12Target9);
	Item_Remove_From_World(kItemPS12Target10);
	Item_Remove_From_World(kItemPS12Target11);
	Item_Remove_From_World(kItemPS12Target12);
	Item_Remove_From_World(kItemPS12Target13);
	Item_Remove_From_World(kItemPS12Target14);
	Item_Remove_From_World(kItemPS12Target15);
	Item_Remove_From_World(kItemPS12Target16);
	Item_Remove_From_World(kItemPS12Target17);
}

} // End of namespace BladeRunner
