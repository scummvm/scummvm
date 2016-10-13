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

#include "bladerunner/script/script.h"

namespace BladeRunner {

void ScriptRC02::InitializeScene() {
	if (Game_Flag_Query(8)) {
		Setup_Scene_Information(-103.0f, -1238.89f, 108603.04f, 1007);
	} else {
		Setup_Scene_Information(-20.2f, -1238.89f, 108100.73f, 539);
	}
	Scene_Exit_Add_2D_Exit(0, 0, 460, 639, 479, 2);
	if (Game_Flag_Query(141)) {
		Scene_Exit_Add_2D_Exit(1, 265, 58, 346, 154, 0);
	}
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(0);
	Ambient_Sounds_Add_Looping_Sound(71, 50, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(75, 75, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(105, 30, 100, 1);
	Ambient_Sounds_Add_Sound(73, 5, 20, 10, 10, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(74, 5, 20, 10, 10, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(76, 5, 40, 6, 6, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(77, 5, 40, 6, 6, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(78, 5, 40, 6, 6, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(79, 5, 40, 6, 6, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Speech_Sound(23, 250, 10, 60, 5, 5, 100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(23, 330, 10, 60, 5, 5, 100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(24, 380, 10, 60, 5, 5, 100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(24, 510, 10, 60, 5, 5, 100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(38, 80, 10, 60, 5, 5, 100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(38, 160, 10, 60, 5, 5, 100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Sound(87, 20, 80, 10, 20, 100, 100, -101, -101, 0, 0);
}

void ScriptRC02::SceneLoaded() {
	Obstacle_Object("TABLETOP", true);
	Obstacle_Object("DRAPE01", true);
	Obstacle_Object("DRAPE03", true);
	Obstacle_Object("DRAPE04", true);
	Obstacle_Object("DRAPE05", true);
	Obstacle_Object("DRAPE06", true);
	Obstacle_Object("DRAPE07", true);
	Obstacle_Object("OUTR_DESK", true);
	Obstacle_Object("CAGE_BASE", true);
	Obstacle_Object("POLE_ROP01", true);
	Unobstacle_Object("LEGS", true);
	Unobstacle_Object("SLATS01", true);
	Unobstacle_Object("DRAPE07", true);
	Clickable_Object("SCRTY CA03");
	Unclickable_Object("GRL_DSKLEG");
	Unclickable_Object("CURTAIN");
	Unclickable_Object("DRAPE01");
	Unclickable_Object("DRAPE02");
	Unclickable_Object("DRAPE03");
	Unclickable_Object("DRAPE05");
	Unclickable_Object("DRAPE06");
	Unclickable_Object("DRAPE07");
	if (Actor_Clue_Query(0, 12) || Global_Variable_Query(1) > 1) {
		Unclickable_Object("SCRTY CA03");
	}
	if (!Game_Flag_Query(190)) {
		Item_Add_To_World(100, 966, 16, -52.88f, -1238.89f, 108467.74f, 256, 6, 6, false, true, false, true);
		Item_Add_To_World(101, 966, 16, -37.16f, -1238.89f, 108456.59f, 512, 6, 6, false, true, false, true);
		Item_Add_To_World(102, 966, 16, -62.86f, -1238.89f, 108437.52f, 625, 6, 6, false, true, false, true);
	}
}

bool ScriptRC02::MouseClick(int x, int y) {
	return false;
}

bool ScriptRC02::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("SCRTY CA03", objectName) && !Actor_Clue_Query(0, 12)) {
		if (Actor_Clue_Query(0, 22) && Actor_Query_Is_In_Current_Set(15)) {
			AI_Movement_Track_Pause(15);
			Actor_Face_Actor(0, 15, true);
			Actor_Says(0, 4545, 14);
			Actor_Face_Actor(15, 0, true);
			Actor_Says(15, 0, 14);
			Actor_Says(15, 10, 16);
			Actor_Says(0, 4550, 13);
			Actor_Says(15, 20, 13);
			Loop_Actor_Walk_To_Waypoint(15, 89, 0, 0, false);
			Actor_Face_Actor(0, 15, true);
			Loop_Actor_Walk_To_Waypoint(15, 102, 0, 0, false);
			Actor_Face_Actor(0, 15, true);
			Actor_Face_Heading(15, 539, false);
			Delay(2000);
			Loop_Actor_Walk_To_Waypoint(15, 89, 0, 0, false);
			Loop_Actor_Walk_To_Actor(15, 0, 24, 0, false);
			Item_Pickup_Spin_Effect(975, 357, 228);
			Actor_Face_Actor(0, 15, true);
			Actor_Face_Actor(15, 0, true);
			Actor_Says(15, 30, 23);
			Actor_Says(0, 4555, 18);
			Actor_Clue_Acquire(0, 12, 1, 15);
			Unclickable_Object("SCRTY CA03");
			AI_Movement_Track_Unpause(15);
			return true;
		} else {
			Actor_Face_Object(0, "SCRTY CA03", true);
			Actor_Voice_Over(2000, 99);
			return true;
		}
	}
	return false;
}

void ScriptRC02::sub_402A7C() {
	Dialogue_Menu_Clear_List();
	DM_Add_To_List_Never_Repeat_Once_Selected(0, 5, 6, 2);
	DM_Add_To_List_Never_Repeat_Once_Selected(10, 5, 4, 8);
	if (Actor_Clue_Query(0, 23) || (Actor_Clue_Query(0, 24))) {
		DM_Add_To_List_Never_Repeat_Once_Selected(20, 6, 4, 5);
	}
	Dialogue_Menu_Add_DONE_To_List(30);
	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();
	if (answer) {
		switch (answer) {
		case 10:
			Actor_Says(0, 4585, 13);
			Actor_Face_Actor(15, 0, true);
			if (Game_Flag_Query(46)) {
				Actor_Says(15, 250, 13);
				Actor_Says(15, 270, 13);
				Actor_Clue_Acquire(0, 23, 1, 15);
			} else {
				Actor_Says(15, 260, 14);
				Actor_Says(15, 270, 13);
				Actor_Clue_Acquire(0, 24, 1, 15);
			}
			Actor_Says(0, 4645, 13);
			Actor_Says(15, 280, 13);
			Actor_Says(15, 290, 13);
			Actor_Says(0, 4650, 18);
			Actor_Says(15, 320, 13);
			Actor_Says(0, 4665, 13);
			Actor_Face_Object(15, "CURTAIN", true);
			Actor_Says(15, 350, 13);
			Actor_Face_Actor(15, 0, true);
			Scene_Exit_Add_2D_Exit(1, 265, 58, 346, 154, 0);
			Game_Flag_Set(141);
			break;
		case 20:
			Actor_Says(0, 4590, 19);
			Actor_Face_Actor(15, 0, true);
			Actor_Says(15, 360, 13);
			Loop_Actor_Walk_To_Waypoint(15, 89, 0, 0, false);
			Loop_Actor_Walk_To_Waypoint(15, 102, 0, 0, false);
			Actor_Face_Actor(0, 15, true);
			Actor_Face_Heading(15, 539, false);
			Delay(2000);
			Loop_Actor_Walk_To_Waypoint(15, 89, 0, 0, false);
			Actor_Face_Actor(0, 15, true);
			Loop_Actor_Walk_To_Actor(15, 0, 24, 0, false);
			Actor_Face_Actor(15, 0, true);
			Actor_Face_Actor(0, 15, true);
			Item_Pickup_Spin_Effect(964, 357, 228);
			Actor_Says(15, 1700, 13);
			Actor_Clue_Acquire(0, 15, 1, 15);
			break;
		case 30:
			Actor_Says(0, 4595, 14);
			break;
		}
	} else {
		Actor_Says(0, 4580, 13);
		Actor_Face_Actor(15, 0, true);
		Actor_Says(15, 110, 18);
		Actor_Says(15, 120, 17);
		Actor_Says(15, 130, 19);
		Actor_Says(0, 4605, 13);
		Actor_Says(15, 140, 16);
		Game_Flag_Set(187);
	}
}

bool ScriptRC02::ClickedOnActor(int actorId) {
	if (actorId != 15) {
		return false;
	}

	if (Global_Variable_Query(1) == 4) {
		Actor_Face_Actor(0, 15, true);
		if (Actor_Query_Goal_Number(15) == 599) {
			if (Random_Query(1, 2) == 1) {
				Actor_Says(0, 8715, 17);
			} else {
				Actor_Says(0, 8720, 17);
			}
		} else if (Game_Flag_Query(705) || Game_Flag_Query(706)) {
			Actor_Says(0, 4805, 11);
			Actor_Face_Actor(15, 0, true);
			if (Game_Flag_Query(706)) {
				Actor_Says(15, 720, 15);
			} else {
				Actor_Says(15, 730, 14);
			}
			Actor_Face_Heading(15, 1007, false);
		} else {
			Actor_Says(0, 4690, 11);
			Actor_Says(0, 4695, 13);
			Actor_Face_Actor(15, 0, true);
			Actor_Says(15, 1610, 14);
			if (Actor_Clue_Query(0, 76)) {
				Actor_Says(0, 4700, 12);
				Actor_Says(0, 4705, 13);
				Actor_Says(15, 1620, 12);
				Actor_Says(0, 4710, 15);
				Actor_Says(0, 4715, 11);
				Delay(2000);
				Actor_Says(0, 4720, 16);
				Actor_Says(0, 4725, 17);
				Actor_Says(15, 430, 16);
				Actor_Face_Heading(15, 1007, false);
			}
			Game_Flag_Set(706);
		}
		return true;
	}
	AI_Movement_Track_Pause(15);
	Loop_Actor_Walk_To_Actor(0, 15, 48, 1, false);
	Actor_Face_Actor(0, 15, true);
	if (!Game_Flag_Query(6)) {
		Actor_Says(0, 4560, 13);
		Actor_Face_Actor(15, 0, true);
		Actor_Says(15, 40, 16);
		Actor_Says(15, 50, 15);
		Actor_Says(0, 4565, 13);
		Actor_Says(15, 60, 14);
		Actor_Says(0, 4570, 18);
		Actor_Says(15, 70, 13);
		Game_Flag_Set(6);
		Actor_Clue_Acquire(0, 22, 1, 15);
		AI_Movement_Track_Unpause(15);
		return true;
	}
	if (Game_Flag_Query(187)) {
		if (Player_Query_Agenda() == 0) {
			Game_Flag_Reset(0);
			sub_402A7C();
			AI_Movement_Track_Unpause(15);
			return true;
		}

		Actor_Says(0, 4610, 19);
		Actor_Face_Actor(15, 0, true);
		Actor_Says(15, 150, 15);
		Actor_Says(0, 4615, 13);
		Actor_Says(15, 160, 14);
		Actor_Says(15, 170, 15);
		Actor_Says(15, 180, 13);

		if (Player_Query_Agenda() == 2) {
			Actor_Says(0, 4620, 19);
			Actor_Says(15, 190, 14);
			Actor_Says(0, 4625, 13);
			Actor_Says(15, 210, 13);
			Actor_Says(0, 4630, 18);
			Actor_Says(15, 220, 14);
			Actor_Says(15, 230, 13);
			Actor_Says(0, 4635, 19);
			Actor_Says(15, 240, 16);
			Actor_Says(0, 4640, 17);
		}
		Game_Flag_Reset(187);
		AI_Movement_Track_Unpause(15);
		return true;
	}
	sub_402A7C();
	AI_Movement_Track_Unpause(15);
	return true;
}

bool ScriptRC02::ClickedOnItem(int itemId, bool a2) {
	if (itemId == 100 || itemId == 101 || itemId == 102) {
		if (!Loop_Actor_Walk_To_Item(0, 100, 24, 1, false)) {
			Actor_Face_Item(0, 100, true);
			Actor_Clue_Acquire(0, 5, 1, -1);
			Game_Flag_Set(190);
			Item_Remove_From_World(100);
			Item_Remove_From_World(101);
			Item_Remove_From_World(102);
			Item_Pickup_Spin_Effect(966, 395, 352);
			Actor_Voice_Over(1960, 99);
		}
		return true;
	}
	return false;
}

bool ScriptRC02::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, -71.51f, -1238.89f, 108587.15f, 0, 1, false, 0)) {
			Game_Flag_Set(9);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_Looping_Sound(71, true);
			Ambient_Sounds_Remove_Looping_Sound(75, true);
			Ambient_Sounds_Adjust_Looping_Sound(85, 100, -101, 1);
			Actor_Set_Goal_Number(15, 0);
			Set_Enter(69, 78);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, -20.2f, -1238.73f, 108152.73f, 0, 1, false, 0)) {
			Async_Actor_Walk_To_XYZ(0, -8.87f, -1238.89f, 108076.27f, 0, false);
			Set_Enter(16, 107);
		}
		return true;
	}
	return false;
}

bool ScriptRC02::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptRC02::SceneFrameAdvanced(int frame) {
}

void ScriptRC02::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptRC02::PlayerWalkedIn() {
	Player_Set_Combat_Mode(false);
	if (Game_Flag_Query(8)) {
		Player_Loses_Control();
		Loop_Actor_Walk_To_XYZ(0, -72.2f, -1238.89f, 108496.73f, 0, 0, false, 0);
		Player_Gains_Control();
		Game_Flag_Reset(8);
		if (!Game_Flag_Query(1)) {
			Actor_Voice_Over(1970, 99);
			Actor_Voice_Over(1980, 99);
			Actor_Voice_Over(1990, 99);
			Actor_Clue_Acquire(0, 3, 1, -1);
			Actor_Clue_Acquire(0, 4, 1, -1);
			Game_Flag_Set(1);
		}
		if (Actor_Query_Which_Set_In(15) == 16 && Actor_Query_Goal_Number(15) < 300) {
			Actor_Set_Goal_Number(15, 1);
		}
		if (Actor_Query_Goal_Number(15) == 300 && !Game_Flag_Query(704)) {
			Actor_Face_Actor(15, 0, true);
			Actor_Says(15, 370, 12);
			Actor_Says(15, 380, 14);
			Actor_Face_Actor(0, 15, true);
			Actor_Says(0, 4670, 15);
			Actor_Says(15, 390, 13);
			Actor_Says(0, 4675, 14);
			Actor_Face_Heading(15, 1007, false);
			Actor_Says(15, 400, 13);
			Actor_Says(15, 410, 12);
			Game_Flag_Set(704);
		}
	} else {
		Player_Loses_Control();
		Loop_Actor_Walk_To_XYZ(0, -20.2f, -1238.89f, 108152.73f, 0, 0, false, 0);
		Player_Gains_Control();
	}
}

void ScriptRC02::PlayerWalkedOut() {
}

void ScriptRC02::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
