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

void SceneScriptPS13::InitializeScene() {
	Police_Maze_Set_Pause_State(1);
	if (Game_Flag_Query(18)) {
		Scene_Loop_Start_Special(0, 0, 0);
		Scene_Loop_Set_Default(1);
		Game_Flag_Reset(18);
		Setup_Scene_Information(World_Waypoint_Query_X(10), World_Waypoint_Query_Y(10), World_Waypoint_Query_Z(10), 200);
	} else {
		Scene_Loop_Set_Default(1);
		Setup_Scene_Information(World_Waypoint_Query_X(11), World_Waypoint_Query_Y(11), World_Waypoint_Query_Z(11), 840);
	}
	Scene_Exit_Add_2D_Exit(0, 0, 460, 639, 479, 2);
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
		Item_Add_To_World(46, 443, 14, -372.0f, -9.0f, 1509.0f, 960, 72, 36, true, false, false, true);
		Item_Add_To_World(47, 443, 14, 291.61f, -0.66f, 1610.3f, 823, 72, 36, true, false, false, true);
		Item_Add_To_World(48, 447, 14, -25.0f, 102.0f, 1625.0f, 823, 72, 36, true, false, false, true);
		Item_Add_To_World(49, 449, 14, -45.51f, -8.8f, 1676.0f, 922, 72, 36, true, false, false, true);
		Item_Add_To_World(50, 447, 14, 291.61f, -0.66f, 1610.3f, 823, 72, 36, true, false, false, true);
		Item_Add_To_World(51, 443, 14, -24.0f, 102.0f, 1625.0f, 823, 72, 36, true, false, false, true);
		Item_Add_To_World(52, 449, 14, 180.0f, -72.7f, 1605.0f, 305, 72, 36, true, false, false, true);
		Item_Add_To_World(53, 443, 14, 127.79f, 14.56f, 1703.03f, 356, 72, 36, true, false, false, true);
		Item_Add_To_World(54, 443, 14, 136.37f, -6.84f, 1425.4301f, 512, 72, 36, true, false, false, true);
		Item_Add_To_World(55, 441, 14, 77.83f, -79.8f, 1520.5f, 327, 72, 36, true, false, false, true);
		Item_Add_To_World(56, 441, 14, 77.83f, -7.8f, 1520.5f, 327, 72, 36, true, false, false, true);
		Item_Add_To_World(57, 443, 14, -88.0f, -8.8f, 1520.5f, 327, 72, 36, true, false, false, true);
		Item_Add_To_World(58, 447, 14, -45.51f, -8.8f, 1676.0f, 922, 72, 36, true, false, false, true);
		Item_Add_To_World(62, 445, 14, -300.0f, -79.75f, 1543.0f, 465, 72, 36, true, false, false, true);
		Item_Add_To_World(63, 449, 14, -325.0f, -7.75f, 1543.0f, 465, 72, 36, true, false, false, true);
	}
	Police_Maze_Target_Track_Add(46, -372.0f, -9.0f, 1509.0f, -345.0f, -9.0f, 1509.0f, 6, track_data_46, true);
	Police_Maze_Target_Track_Add(47, 291.61f, -0.66f, 1610.3f, 238.83f, 1.03f, 1557.03f, 10, track_data_47, true);
	Police_Maze_Target_Track_Add(48, -25.0f, 102.0f, 1625.0f, -25.0f, 138.0f, 1625.0f, 10, track_data_48, true);
	Police_Maze_Target_Track_Add(49, -45.51f, -8.8f, 1676.0f, 15.51f, -8.8f, 1679.0f, 10, track_data_49, false);
	Police_Maze_Target_Track_Add(50, 291.61f, -0.66f, 1610.3f, 238.83f, 1.03f, 1557.03f, 10, track_data_50, false);
	Police_Maze_Target_Track_Add(51, -24.0f, 102.0f, 1625.0f, -24.0f, 138.0f, 1625.0f, 10, track_data_51, false);
	Police_Maze_Target_Track_Add(52, 180.0f, -72.7f, 1605.0f, 180.0f, -0.7f, 1605.0f, 10, track_data_52, false);
	Police_Maze_Target_Track_Add(53, 127.79f, 14.56f, 1703.03f, -56.07f, 1.89f, 1589.04f, 6, track_data_53, false);
	Police_Maze_Target_Track_Add(54, 136.37f, -6.84f, 1425.4301f, 117.55f, -6.84f, 1442.09f, 4, track_data_54, false);
	Police_Maze_Target_Track_Add(55, 77.83f, -79.8f, 1520.5f, 77.83f, -7.8f, 1520.5f, 15, track_data_55, false);
	Police_Maze_Target_Track_Add(56, 77.83f, -7.8f, 1520.5f, -88.0f, -8.8f, 1520.5f, 15, track_data_56, false);
	Police_Maze_Target_Track_Add(57, -88.0f, -8.8f, 1520.5f, -88.0f, -80.8f, 1520.5f, 15, track_data_57, false);
	Police_Maze_Target_Track_Add(58, -45.51f, -8.8f, 1676.0f, 15.51f, -8.8f, 1679.0f, 10, track_data_58, true);
	Police_Maze_Target_Track_Add(62, -300.0f, -79.75f, 1543.0f, -300.0f, -14.75f, 1543.0f, 15, track_data_62, false);
	Police_Maze_Target_Track_Add(63, -325.0f, -7.75f, 1543.0f, -300.0f, -7.75f, 1543.0f, 10, track_data_63, false);
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
		case 48:
			Sound_Play(4, 50, 0, 0, 50);
			break;
		case 50:
			Sound_Play(4, 50, 0, 0, 50);
			break;
		case 55:
			Sound_Play(555, 50, 0, 0, 50);
			break;
		case 56:
			Sound_Play(555, 50, 0, 0, 50);
			break;
		default:
			Sound_Play(2, 12, 0, 0, 50);
			break;
		}
		Item_Spin_In_World(itemId);
		Item_Flag_As_Non_Target(itemId);
		if (itemId == 46) {
			Item_Flag_As_Non_Target(46);
		}
		if (itemId == 47) {
			Item_Flag_As_Non_Target(47);
		}
		if (itemId == 48) {
			Item_Flag_As_Non_Target(48);
		}
		if (itemId == 49) {
			Item_Flag_As_Non_Target(49);
		}
		if (itemId == 50) {
			Item_Flag_As_Non_Target(50);
		}
		if (itemId == 51) {
			Item_Flag_As_Non_Target(51);
		}
		if (itemId == 52) {
			Item_Flag_As_Non_Target(52);
		}
		if (itemId == 53) {
			Item_Flag_As_Non_Target(53);
		}
		if (itemId == 54) {
			Item_Flag_As_Non_Target(54);
		}
		if (itemId == 55) {
			Item_Flag_As_Non_Target(55);
			Item_Flag_As_Non_Target(56);
			Item_Flag_As_Non_Target(57);
		}
		if (itemId == 56) {
			Item_Flag_As_Non_Target(55);
			Item_Flag_As_Non_Target(56);
			Item_Flag_As_Non_Target(57);
		}
		if (itemId == 57) {
			Item_Flag_As_Non_Target(55);
			Item_Flag_As_Non_Target(56);
			Item_Flag_As_Non_Target(57);
		}
		if (itemId == 58) {
			Item_Flag_As_Non_Target(58);
		}
		if (itemId == 62) {
			Item_Flag_As_Non_Target(62);
		}
		if (itemId == 63) {
			Item_Flag_As_Non_Target(63);
		}
		return true;
	}
	return false;

}

bool SceneScriptPS13::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_Waypoint(kActorMcCoy, 10, 12, 1, false)) {
			Game_Flag_Set(19);
			sub_40267C();
			Set_Enter(14, 75);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_Waypoint(kActorMcCoy, 11, 12, 1, false)) {
			Game_Flag_Set(21);
			Player_Set_Combat_Mode(false);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			sub_40267C();
			Global_Variable_Decrement(9, 20 - Global_Variable_Query(13));
			Set_Score(0, Global_Variable_Query(9));
			Global_Variable_Reset(10);
			Global_Variable_Reset(11);
			Global_Variable_Reset(12);
			Global_Variable_Reset(13);
			Global_Variable_Reset(9);
			Set_Enter(15, 69);
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
	Police_Maze_Set_Pause_State(0);
}

void SceneScriptPS13::PlayerWalkedOut() {
}

void SceneScriptPS13::DialogueQueueFlushed(int a1) {
}

void SceneScriptPS13::sub_40267C() {
	Item_Remove_From_World(46);
	Item_Remove_From_World(47);
	Item_Remove_From_World(48);
	Item_Remove_From_World(49);
	Item_Remove_From_World(50);
	Item_Remove_From_World(51);
	Item_Remove_From_World(52);
	Item_Remove_From_World(53);
	Item_Remove_From_World(54);
	Item_Remove_From_World(55);
	Item_Remove_From_World(56);
	Item_Remove_From_World(57);
	Item_Remove_From_World(58);
	Item_Remove_From_World(62);
	Item_Remove_From_World(63);
}

} // End of namespace BladeRunner
