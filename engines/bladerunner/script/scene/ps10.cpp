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

#include "bladerunner/script/scene.h"

namespace BladeRunner {

void SceneScriptPS10::InitializeScene() {
	Police_Maze_Set_Pause_State(1);
	if (Game_Flag_Query(15)) {
		float x = World_Waypoint_Query_X(4);
		float y = World_Waypoint_Query_Y(4);
		float z = World_Waypoint_Query_Z(4);
		Setup_Scene_Information(x, y, z, 280);
	} else {
		Setup_Scene_Information(-87.08f, -9.23f, 941.9f, 0);
	}
	Scene_Exit_Add_2D_Exit(1, 0, 0, 20, 479, 3);
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(0);
	Ambient_Sounds_Add_Looping_Sound(387, 50, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(54, 50, 1, 1);
	Ambient_Sounds_Add_Sound(1, 10, 50, 16, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(389, 5, 50, 16, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(390, 6, 50, 16, 25, -100, 100, -101, -101, 0, 0);
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

static int track_data_0[] = {-26, 10, 20, -18, 10, 20, -9, 0, -9, 1,-5, 989, -1, 0, -15, 0, 1, -15, 1, 1,-23, 0, -7, 3000, 5000, -8, 0, -10, 159, 100,-2, 14, -3, 1000, -6, 740, 80, -22, 0, -3,0, -6, 488, 80, -3, 1000, -24, 27, 33, -3, 0, -6, 740, 80, -11, 1, -9, 0, -8, 1, -12, 0, -1, 0, -4};
static int track_data_1[] = {-5, 740, -1, 0, -22, 1, -2, 69, -3, 500, -9, 1, -11, 4, -12, 1, -1, 0, -4};
static int track_data_2[] = {-26, 10, 20, -18, 10, 20, -9, 2, -5, 993, -1, 0, -7, 3000, 5000, -8, 2, -10, 159, 100, -15, 2, 1, -23, 2, -2, 5, -3, 1000, -22, 2, -6, 233, 80, -3, 0, -6, 491, 80, -3, 500, -24, 27, 33, -3, 500, -6, 233, 80, -3, 0, -6, 993, 80, -10, 34, 33, -2, 0, -9, 2, -4};
static int track_data_3[] = {-26, 10, 20, -18, 10, 20, -9, 3, -5, 993, -1, 0, -7, 3000, 6000, -8, 3, -10, 159, 100, -15, 3, 1, -23, 3, -2, 34, -3, 500, -6, 491, 80, -2, 0, -25, -9, 3, -11, 7, -12, 3, -4};
static int track_data_4[] = {-26, 10, 20, -18, 10, 20, -9, 4, -5, 0, -1, 0, -7, 4000, 6000, -8, 4, -10, 159, 100, -15, 4, 1, -23, 4, -2, 5, -3, 1000, -6, 512, 100, -3, 2000, -6, 0, -100, -10, 34, 33, -2, 0, -25, -9, 4, -11, 0, -12, 4, -4};
static int track_data_5[] = {-26, 10, 20, -18, 10, 20, -9, 5, -5, 999, -1, 0, -7, 4000, 6000, -8, 5, -10, 159, 100, -15, 5, 1, -23, 5, -2, 7, -3, 500, -22, 5, -6, 750, 80, -3, 0, -6, 500, 80, -3, 1000, -24, 27, 33, -3, 0, -6, 750, 80, -3, 0, -6, 999, 80, -10, 34, 33, -2, 0, -9, 5, -11, 6, -11, 8, -12, 5, -4};
static int track_data_6[] = {-26, 10, 20, -18, 10, 20, -9, 6, -5, 264, -1, 0, -7, 3000, 6000, -15, 6, 1, -23, 6, -8, 6, -2, 89, -7, 4000, 8000, -5, 776, -2, 0, -25, -9, 6, -12, 6, -4};
static int track_data_7[] = {-26, 10, 20, -18, 10, 20, -9, 7, -5, 993, -1, 0, -7, 4000, 6000, -8, 7, -10, 159, 100, -15, 7, 1, -23, 7, -2, 34, -3, 500, -22, 7, -6, 491, 80, -2, 20, -3, 0, -24, 27, 33, -2, 0, -9, 7, -11, 3, -12, 7, -4};
static int track_data_8[] = {-26, 10, 20, -18, 10, 20, -9, 8, -5, 738, -1, 0, -7, 2000, 5000, -15, 8, 1, -22, 8, -8, 8, -10, 0, 33, -2, 23, -10, 0, 33, -3, 200, -10, 32, 33, -6, 498, 100, -10, 0, 33, -3, 100, -24, 27, 33, -10, 32, 33, -2, 35, -10, 32, 33, -3, 100, -24, 27, 33, -10, 0, 33, -2, 23, -10, 32, 33, -3, 100, -24, 27, 33, -10, 32, 33, -6, 758, 100, -10, 32, 33, -2, 89, -10, 0, 33, -7, 4000, 6000, -15, 8, 1, -22, 8, -5, 216, -10, 32, 33, -2, 69, -3, 100, -10, 32, 33, -6, 498, 100, -3, 100, -24, 27, 33, -10, 0, 33, -6, 216, 100, -10, 32, 33, -2, 0, -9, 8, -12, 8, -4};

void SceneScriptPS10::SceneLoaded() {
	Obstacle_Object("PARKMETR01", true);
	Obstacle_Object("PARKMETR02", true);
	Obstacle_Object("PARKMETR03", true);
	Obstacle_Object("PARKMETR07", true);
	Obstacle_Object("PARKMETR08", true);
	Obstacle_Object("PARKMETR10", true);
	Obstacle_Object("PARKMETR11", true);
	Obstacle_Object("PARKMETR15", true);
	Obstacle_Object("PARKMETR16", true);
	Obstacle_Object("TUBE14", true);
	Unclickable_Object("PARKMETR01");
	Unclickable_Object("PARKMETR02");
	Unclickable_Object("PARKMETR03");
	Unclickable_Object("PARKMETR07");
	Unclickable_Object("PARKMETR08");
	Unclickable_Object("PARKMETR10");
	Unclickable_Object("PARKMETR11");
	Unclickable_Object("PARKMETR15");
	Unclickable_Object("PARKMETR16");
	Unobstacle_Object("E.SM.WIRE01", true);
	if (!Query_System_Currently_Loading_Game()) {
		Item_Add_To_World(0, 443, 14, -240.0f, -80.74f, 145.0f, 989, 72, 36, true, false, false, true);
		Item_Add_To_World(1, 443, 14, -240.0f, -8.74f, 145.0f, 740, 72, 36, true, false, false, true);
		Item_Add_To_World(2, 445, 14, -165.0f, 111.53f, -10.0f, 993, 72, 36, true, false, false, true);
		Item_Add_To_World(3, 447, 14, -125.0f, 160.0f, -10.0f, 993, 72, 36, true, false, false, true);
		Item_Add_To_World(4, 441, 14, -246.71f, 205.51f, -20.0f, 0, 72, 36, true, false, false, true);
		Item_Add_To_World(5, 445, 14, -27.69f, -86.92f, 434.0f, 999, 72, 36, true, false, false, true);
		Item_Add_To_World(6, 441, 14, -347.15f, 7.68f, -20.0f, 264, 72, 36, true, false, false, true);
		Item_Add_To_World(7, 449, 14, -51.0f, 160.0f, -10.0f, 993, 72, 36, true, false, false, true);
		Item_Add_To_World(8, 445, 14, 39.0f, 9.16f, -20.0f, 738, 72, 36, true, false, false, true);
	}

	Police_Maze_Target_Track_Add(0, -240.0f, -80.74f, 145.0f, -240.0f, -8.74f, 145.0f, 15, track_data_0, false);
	Police_Maze_Target_Track_Add(1, -240.0f, -8.74f, 145.0f, -450.0f, -8.74f, 145.0f, 70, track_data_1, false);
	Police_Maze_Target_Track_Add(2, -165.0f, 111.53f, -10.0f, -165.0f, 167.53f, -10.0f, 6, track_data_2, true);
	Police_Maze_Target_Track_Add(3, -125.0f, 160.0f, -10.0f, -51.0f, 160.0f, -10.0f, 35, track_data_3, false);
	Police_Maze_Target_Track_Add(4, -246.71f, 205.51f, -20.0f, -246.71f, 241.51f, -20.0f, 6, track_data_4, true);
	Police_Maze_Target_Track_Add(5, -27.69f, -86.92f, 434.0f, -27.69f, -18.92f, 434.0f, 8, track_data_5, true);
	Police_Maze_Target_Track_Add(6, -347.15f, 7.68f, -20.0f, 39.0f, 9.16f, -20.0f, 90, track_data_6, false);
	Police_Maze_Target_Track_Add(7, -51.0f, 160.0f, -10.0f, -125.0f, 160.0f, -10.0f, 35, track_data_7, true);
	Police_Maze_Target_Track_Add(8, 39.0f, 9.16f, -20.0f, -347.15f, 7.68f, -20.0f, 90, track_data_8, false);
	Preload(441);
	Preload(442);
	Preload(443);
	Preload(444);
	Preload(445);
	Preload(446);
	Preload(447);
	Preload(448);
	Preload(449);
	Preload(450);
}

bool SceneScriptPS10::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptPS10::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptPS10::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptPS10::ClickedOnItem(int itemId, bool a2) {
	if (Player_Query_Combat_Mode()) {
		switch (itemId) {
		case 3:
			Sound_Play(4, 50, 0, 0, 50);
			break;
		case 4:
			Sound_Play(555, 50, 0, 0, 50);
			break;
		case 6:
			Sound_Play(555, 50, 0, 0, 50);
			break;
		default:
			Sound_Play(2, 12, 0, 0, 50);
			break;
		}
		Item_Spin_In_World(itemId);
		if (itemId == 0) {
			Item_Flag_As_Non_Target(0);
			Item_Flag_As_Non_Target(1);
		}
		if (itemId == 1) {
			Item_Flag_As_Non_Target(0);
			Item_Flag_As_Non_Target(1);
		}
		if (itemId == 2) {
			Item_Flag_As_Non_Target(2);
		}
		if (itemId == 3) {
			Item_Flag_As_Non_Target(3);
		}
		if (itemId == 4) {
			Item_Flag_As_Non_Target(4);
		}
		if (itemId == 5) {
			Item_Flag_As_Non_Target(5);
		}
		if (itemId == 6) {
			Item_Flag_As_Non_Target(6);
		}
		if (itemId == 7) {
			Item_Flag_As_Non_Target(7);
		}
		if (itemId == 8) {
			Item_Flag_As_Non_Target(8);
		} else {
			Item_Flag_As_Non_Target(itemId);
		}
		return true;
	}

	return false;
}

bool SceneScriptPS10::ClickedOnExit(int exitId) {
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_Waypoint(kActorMcCoy, 6, 12, 1, false)) {
			Game_Flag_Set(14);
			sub_402238();
			Global_Variable_Decrement(9, 20 - Global_Variable_Query(10));
			Global_Variable_Set(10, 20);
			Set_Enter(14, 74);
		}
		return true;
	}

	return false;
}

bool SceneScriptPS10::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptPS10::SceneFrameAdvanced(int frame) {
}

void SceneScriptPS10::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptPS10::PlayerWalkedIn() {
	if (Game_Flag_Query(15)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -352.09f, -9.23f, 267.95f, 0, 0, true, 0);
		Police_Maze_Set_Pause_State(0);
		Game_Flag_Reset(15);
		//return true;
		return;
	} else {
		Player_Set_Combat_Mode(true);
		Loop_Actor_Walk_To_Waypoint(kActorMcCoy, 5, 0, 0, true);
		Actor_Says(kActorAnsweringMachine, 280, 3);
		Actor_Says(kActorAnsweringMachine, 290, 3);
		Actor_Says(kActorAnsweringMachine, 300, 3);
		Police_Maze_Set_Pause_State(0);
		//return true;
		return;
	}
}

void SceneScriptPS10::PlayerWalkedOut() {
}

void SceneScriptPS10::DialogueQueueFlushed(int a1) {
}

void SceneScriptPS10::sub_402238() {
	Item_Remove_From_World(0);
	Item_Remove_From_World(1);
	Item_Remove_From_World(2);
	Item_Remove_From_World(3);
	Item_Remove_From_World(4);
	Item_Remove_From_World(5);
	Item_Remove_From_World(6);
	Item_Remove_From_World(7);
	Item_Remove_From_World(8);
}

} // End of namespace BladeRunner
