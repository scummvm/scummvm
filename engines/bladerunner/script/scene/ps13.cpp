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

static int kPoliceMazePS13TargetCount = 20;

void SceneScriptPS13::InitializeScene() {
	Police_Maze_Set_Pause_State(true);
	if (Game_Flag_Query(kFlagPS12toPS13)) {
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, 0, false);
		Scene_Loop_Set_Default(1);
		Game_Flag_Reset(kFlagPS12toPS13);
		Setup_Scene_Information(World_Waypoint_Query_X(10), World_Waypoint_Query_Y(10), World_Waypoint_Query_Z(10), 200);
	} else {
		Scene_Loop_Set_Default(1);
		Setup_Scene_Information(World_Waypoint_Query_X(11), World_Waypoint_Query_Y(11), World_Waypoint_Query_Z(11), 840);
	}
	Scene_Exit_Add_2D_Exit(0, 0, 460, 639, 479, 2);
	Scene_Exit_Add_2D_Exit(1, 0,   0,  20, 479, 3);
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(0);
	Ambient_Sounds_Add_Looping_Sound(387, 50, 1, 1);
	Ambient_Sounds_Add_Looping_Sound( 54, 50, 1, 1);
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

static int track_data_46[] = {-26, 13, 20, -18, 13, 20, -9, 46, -5, 960, -1, 0, -3, 2000, -8, 46, -10, 33, 33, -15, 46, 1, -22, 46, -3, 1000, -2, 5, -3, 500, -24, 27, 33, -3, 500, -2, 0, -9, 46, -13, 62, 63, -12, 46, -4};
static int track_data_47[] = {-26, 13, 20, -18, 13, 20, -9, 47, -5, 823, -1, 0, -7, 5000, 5000, -8, 47, -10, 33, 33, -15, 47, 1, -22, 47, -2, 9, -3, 2000, -24, 27, 33, -2, 0, -9, 47, -13, 50, 52, -12, 47, -4};
static int track_data_48[] = {-26, 13, 20, -18, 13, 20, -9, 48, -5, 823, -1, 0, -7, 2000, 2000, -8, 48, -10, 33, 33, -15, 48, 1, -23, 48, -2, 9, -3, 2000, -25, -2, 0, -9, 48, -13, 53, 51, -12, 48, -4};
static int track_data_49[] = {-26, 13, 20, -18, 13, 20, -9, 49, -5, 922, -1, 0, -7, 3000, 3000, -8, 49, -10, 33, 33, -15, 49, 1, -22, 49, -2, 9, -3, 1000, -24, 27, 33, -2, 0, -9, 49, -13, 54, 55, -12, 49, -1, 0, -4};
static int track_data_50[] = {-26, 13, 20, -18, 13, 20, -9, 50, -5, 823, -1, 0, -7, 3000, 5000, -8, 50, -10, 33, 33, -15, 50, 1, -23, 50, -2, 9, -3, 2000, -2, 0, -25, -9, 50, -13, 47, 52, -12, 50, -4};
static int track_data_51[] = {-26, 13, 20, -18, 13, 20, -9, 51, -5, 823, -1, 0, -7, 2000, 2000, -8, 51, -10, 33, 33, -15, 51, 1, -22, 51, -2, 9, -3, 1000, -24, 27, 33, -3, 500, -2, 0, -9, 51, -13, 53, 48, -12, 51, -4};
static int track_data_52[] = {-26, 13, 20, -18, 13, 20, -9, 52, -5, 305, -1, 0, -7, 5000, 10000, -8, 52, -10, 33, 33, -15, 52, 1, -23, 52, -2, 9, -3, 500, -22, 52, -6, 555, 80, -3, 0, -6, 833, 80, -3, 1000, -24, 27, 33, -2, 0, -9, 52, -13, 47, 50, -12, 52, -4};
static int track_data_53[] = {-26, 13, 20, -18, 13, 20, -9, 53, -5, 356, -1, 0, -7, 3000, 3000, -8, 53, -10, 33, 33, -15, 53, 1, -23, 53, -2, 5, -22, 53, -6, 868, 200, -3, 1000, -24, 27, 33, -6, 356, 60, -3, 1000, -2, 0, -9, 53, -13, 48, 51, -12, 53, -4};
static int track_data_54[] = {-26, 13, 20, -18, 13, 20, -9, 54, -5, 512, -1, 0, -7, 10000, 20000, -8, 54, -10, 33, 33, -15, 54, 1, -23, 54, -2, 3, -3, 500, -22, 54, -6, 768, 80, -3, 1000, -24, 27, 33, -2, 0, -9, 54, -13, 58, 55, -12, 54, -4};
static int track_data_55[] = {-26, 13, 20, -18, 13, 20, -9, 55, -9, 56, -9, 57, -15, 55, 1, -15, 56, 1, -15, 57, 1, -5, 327, -1, 0, -7, 1000, 1000, -10, 33, 33, -23, 55, -8, 55, -2, 14, -3, 1000, -11, 56, -9, 55, -8, 56, -12, 55, -1, 0, -4};
static int track_data_56[] = {-5, 327, -1, 0, -8, 56, -23, 56, -2, 14, -3, 1000, -11, 57, -9, 56, -8, 57, -12, 56, -1, 0, -4};
static int track_data_57[] = {-22, 57, -5, 327, -1, 0, -8, 57, -6, 516, 80, -3, 0, -6, 843, 80, -3, 1000, -24, 27, 33, -3, 500, -2, 14, -9, 57, -13, 58, 54, -12, 57, -1, 0, -4};
static int track_data_58[] = {-26, 13, 20, -18, 13, 20, -9, 58, -5, 922, -1, 0, -7, 3000, 3000, -8, 58, -10, 33, 33, -15, 58, 1, -23, 58, -2, 9, -3, 200, -2, 0, -25, -15, 58, 1, -23, 58, -3, 200, -2, 9, -3, 200, -2, 0, -25, -9, 58, -11, 49, -12, 58, -1, 0, -4};
static int track_data_62[] = {-26, 13, 20, -18, 13, 20, -9, 62, -5, 465, -1, 0, -7, 3000, 3000, -8, 62, -10, 33, 33, -15, 62, 1, -23, 62, -2, 14, -3, 1000, -22, 62, -6, 650, 80, -3, 0, -6, 937, 80, -3, 1000, -24, 27, 33, -3, 500, -6, 650, 80, -3, 0, -6, 465, 80, -2, 0, -9, 62, -13, 46, 63, -12, 62, -4};
static int track_data_63[] = {-26, 13, 20, -18, 13, 20, -9, 63, -5, 465, -1, 0, -3, 3000, -8, 63, -10, 33, 33, -15, 63, 1, -23, 63, -2, 9, -3, 1000, -22, 63, -6, 710, 80, -3, 0, -6, 960, 80, -3, 1000, -24, 27, 33, -3, 500, -6, 710, 80, -3, 0, -6, 460, 80, -2, 0, -9, 63, -13, 46, 62, -12, 63, -4};

void SceneScriptPS13::SceneLoaded() {
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
		Item_Add_To_World(kItemPS13Target1,  443, kSetPS10_PS11_PS12_PS13, -372.0f,   -9.0f,  1509.0f, 960, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS13Target2,  443, kSetPS10_PS11_PS12_PS13, 291.61f,  -0.66f,  1610.3f, 823, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS13Target3,  447, kSetPS10_PS11_PS12_PS13,  -25.0f,  102.0f,  1625.0f, 823, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS13Target4,  449, kSetPS10_PS11_PS12_PS13, -45.51f,   -8.8f,  1676.0f, 922, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS13Target5,  447, kSetPS10_PS11_PS12_PS13, 291.61f,  -0.66f,  1610.3f, 823, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS13Target6,  443, kSetPS10_PS11_PS12_PS13,  -24.0f,  102.0f,  1625.0f, 823, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS13Target7,  449, kSetPS10_PS11_PS12_PS13,  180.0f,  -72.7f,  1605.0f, 305, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS13Target8,  443, kSetPS10_PS11_PS12_PS13, 127.79f,  14.56f, 1703.03f, 356, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS13Target9,  443, kSetPS10_PS11_PS12_PS13, 136.37f,  -6.84f, 1425.43f, 512, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS13Target10, 441, kSetPS10_PS11_PS12_PS13,  77.83f,  -79.8f,  1520.5f, 327, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS13Target11, 441, kSetPS10_PS11_PS12_PS13,  77.83f,   -7.8f,  1520.5f, 327, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS13Target12, 443, kSetPS10_PS11_PS12_PS13,  -88.0f,   -8.8f,  1520.5f, 327, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS13Target13, 447, kSetPS10_PS11_PS12_PS13, -45.51f,   -8.8f,  1676.0f, 922, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS13Target14, 445, kSetPS10_PS11_PS12_PS13, -300.0f, -79.75f,  1543.0f, 465, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS13Target15, 449, kSetPS10_PS11_PS12_PS13, -325.0f,  -7.75f,  1543.0f, 465, 72, 36, true, false, false, true);
	}
	Police_Maze_Target_Track_Add(kItemPS13Target1,  -372.0f,   -9.0f,  1509.0f, -345.0f,   -9.0f,  1509.0f,  6, track_data_46,  true);
	Police_Maze_Target_Track_Add(kItemPS13Target2,  291.61f,  -0.66f,  1610.3f, 238.83f,   1.03f, 1557.03f, 10, track_data_47,  true);
	Police_Maze_Target_Track_Add(kItemPS13Target3,   -25.0f,  102.0f,  1625.0f,  -25.0f,  138.0f,  1625.0f, 10, track_data_48,  true);
	Police_Maze_Target_Track_Add(kItemPS13Target4,  -45.51f,   -8.8f,  1676.0f,  15.51f,   -8.8f,  1679.0f, 10, track_data_49, false);
	Police_Maze_Target_Track_Add(kItemPS13Target5,  291.61f,  -0.66f,  1610.3f, 238.83f,   1.03f, 1557.03f, 10, track_data_50, false);
	Police_Maze_Target_Track_Add(kItemPS13Target6,   -24.0f,  102.0f,  1625.0f,  -24.0f,  138.0f,  1625.0f, 10, track_data_51, false);
	Police_Maze_Target_Track_Add(kItemPS13Target7,   180.0f,  -72.7f,  1605.0f,  180.0f,   -0.7f,  1605.0f, 10, track_data_52, false);
	Police_Maze_Target_Track_Add(kItemPS13Target8,  127.79f,  14.56f, 1703.03f, -56.07f,   1.89f, 1589.04f,  6, track_data_53, false);
	Police_Maze_Target_Track_Add(kItemPS13Target9,  136.37f,  -6.84f, 1425.43f, 117.55f,  -6.84f, 1442.09f,  4, track_data_54, false);
	Police_Maze_Target_Track_Add(kItemPS13Target10,  77.83f,  -79.8f,  1520.5f,  77.83f,   -7.8f,  1520.5f, 15, track_data_55, false);
	Police_Maze_Target_Track_Add(kItemPS13Target11,  77.83f,   -7.8f,  1520.5f,  -88.0f,   -8.8f,  1520.5f, 15, track_data_56, false);
	Police_Maze_Target_Track_Add(kItemPS13Target12,  -88.0f,   -8.8f,  1520.5f,  -88.0f,  -80.8f,  1520.5f, 15, track_data_57, false);
	Police_Maze_Target_Track_Add(kItemPS13Target13, -45.51f,   -8.8f,  1676.0f,  15.51f,   -8.8f,  1679.0f, 10, track_data_58,  true);
	Police_Maze_Target_Track_Add(kItemPS13Target14, -300.0f, -79.75f,  1543.0f, -300.0f, -14.75f,  1543.0f, 15, track_data_62, false);
	Police_Maze_Target_Track_Add(kItemPS13Target15, -325.0f,  -7.75f,  1543.0f, -300.0f,  -7.75f,  1543.0f, 10, track_data_63, false);
}

bool SceneScriptPS13::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptPS13::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptPS13::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptPS13::ClickedOnItem(int itemId, bool a2) {
	if (Player_Query_Combat_Mode()) {
		switch (itemId) {
		case kItemPS13Target3:
			Sound_Play(4, 50, 0, 0, 50);
			break;
		case kItemPS13Target5:
			Sound_Play(4, 50, 0, 0, 50);
			break;
		case kItemPS13Target10:
			Sound_Play(555, 50, 0, 0, 50);
			break;
		case kItemPS13Target11:
			Sound_Play(555, 50, 0, 0, 50);
			break;
		default:
			Sound_Play(2, 12, 0, 0, 50);
			break;
		}
		Item_Spin_In_World(itemId);
		Item_Flag_As_Non_Target(itemId);
		if (itemId == kItemPS13Target1) {
			Item_Flag_As_Non_Target(kItemPS13Target1);
		}
		if (itemId == kItemPS13Target2) {
			Item_Flag_As_Non_Target(kItemPS13Target2);
		}
		if (itemId == kItemPS13Target3) {
			Item_Flag_As_Non_Target(kItemPS13Target3);
		}
		if (itemId == kItemPS13Target4) {
			Item_Flag_As_Non_Target(kItemPS13Target4);
		}
		if (itemId == kItemPS13Target5) {
			Item_Flag_As_Non_Target(kItemPS13Target5);
		}
		if (itemId == kItemPS13Target6) {
			Item_Flag_As_Non_Target(kItemPS13Target6);
		}
		if (itemId == kItemPS13Target7) {
			Item_Flag_As_Non_Target(kItemPS13Target7);
		}
		if (itemId == kItemPS13Target8) {
			Item_Flag_As_Non_Target(kItemPS13Target8);
		}
		if (itemId == kItemPS13Target9) {
			Item_Flag_As_Non_Target(kItemPS13Target9);
		}
		if (itemId == kItemPS13Target10) {
			Item_Flag_As_Non_Target(kItemPS13Target10);
			Item_Flag_As_Non_Target(kItemPS13Target11);
			Item_Flag_As_Non_Target(kItemPS13Target12);
		}
		if (itemId == kItemPS13Target11) {
			Item_Flag_As_Non_Target(kItemPS13Target10);
			Item_Flag_As_Non_Target(kItemPS13Target11);
			Item_Flag_As_Non_Target(kItemPS13Target12);
		}
		if (itemId == kItemPS13Target12) {
			Item_Flag_As_Non_Target(kItemPS13Target10);
			Item_Flag_As_Non_Target(kItemPS13Target11);
			Item_Flag_As_Non_Target(kItemPS13Target12);
		}
		if (itemId == kItemPS13Target13) {
			Item_Flag_As_Non_Target(kItemPS13Target13);
		}
		if (itemId == kItemPS13Target14) {
			Item_Flag_As_Non_Target(kItemPS13Target14);
		}
		if (itemId == kItemPS13Target15) {
			Item_Flag_As_Non_Target(kItemPS13Target15);
		}
		return true;
	}
	return false;

}

bool SceneScriptPS13::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_Waypoint(kActorMcCoy, 10, 12, true, false)) {
			Game_Flag_Set(kFlagPS13toPS12);
			removeTargets();
			Set_Enter(kSetPS10_PS11_PS12_PS13, kScenePS12);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_Waypoint(kActorMcCoy, 11, 12, true, false)) {
			Game_Flag_Set(kFlagPS13toPS05);
			Player_Set_Combat_Mode(false);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			removeTargets();
			Global_Variable_Decrement(kVariablePoliceMazeScore, kPoliceMazePS13TargetCount - Global_Variable_Query(kVariablePoliceMazePS13TargetCounter));
			Set_Score(kActorMcCoy, Global_Variable_Query(kVariablePoliceMazeScore));
			Global_Variable_Reset(kVariablePoliceMazePS10TargetCounter);
			Global_Variable_Reset(kVariablePoliceMazePS11TargetCounter);
			Global_Variable_Reset(kVariablePoliceMazePS12TargetCounter);
			Global_Variable_Reset(kVariablePoliceMazePS13TargetCounter);
			Global_Variable_Reset(kVariablePoliceMazeScore);
			Set_Enter(kSetPS05, kScenePS05);
		}
		return true;
	}
	return false;
}

bool SceneScriptPS13::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptPS13::SceneFrameAdvanced(int frame) {
}

void SceneScriptPS13::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptPS13::PlayerWalkedIn() {
	Police_Maze_Set_Pause_State(false);
}

void SceneScriptPS13::PlayerWalkedOut() {
}

void SceneScriptPS13::DialogueQueueFlushed(int a1) {
}

void SceneScriptPS13::removeTargets() {
	Item_Remove_From_World(kItemPS13Target1);
	Item_Remove_From_World(kItemPS13Target2);
	Item_Remove_From_World(kItemPS13Target3);
	Item_Remove_From_World(kItemPS13Target4);
	Item_Remove_From_World(kItemPS13Target5);
	Item_Remove_From_World(kItemPS13Target6);
	Item_Remove_From_World(kItemPS13Target7);
	Item_Remove_From_World(kItemPS13Target8);
	Item_Remove_From_World(kItemPS13Target9);
	Item_Remove_From_World(kItemPS13Target10);
	Item_Remove_From_World(kItemPS13Target11);
	Item_Remove_From_World(kItemPS13Target12);
	Item_Remove_From_World(kItemPS13Target13);
	Item_Remove_From_World(kItemPS13Target14);
	Item_Remove_From_World(kItemPS13Target15);
}

} // End of namespace BladeRunner
