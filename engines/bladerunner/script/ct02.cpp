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

void ScriptCT02::InitializeScene() {
	if (Game_Flag_Query(70)) {
		Game_Flag_Reset(70);
		Setup_Scene_Information(-154.83f, -145.11f, 9.39f, 516);
	} else if (Game_Flag_Query(720)) {
		Setup_Scene_Information(-213.82f, -145.11f, 214.43f, 82);
	} else {
		Setup_Scene_Information(-119.02f, -145.11f, 240.99f, 768);
	}
	Scene_Exit_Add_2D_Exit(0, 590, 0, 639, 479, 1);
	if (Actor_Clue_Query(0, 18)) {
		Scene_Exit_Add_2D_Exit(1, 332, 163, 404, 297, 0);
	} else {
		Overlay_Play("ct02over", 0, 1, 0, 0);
	}
	Ambient_Sounds_Add_Looping_Sound(96, 25, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(56, 38, 100, 1);
	Ambient_Sounds_Add_Looping_Sound(95, 32, 0, 1);
	Ambient_Sounds_Add_Sound(61, 10, 30, 8, 8, 100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(62, 10, 30, 7, 7, 100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(63, 10, 30, 8, 8, 100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(64, 10, 30, 7, 7, 100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Speech_Sound(60, 0, 10, 260, 17, 19, 100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 20, 10, 260, 17, 19, 100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 40, 10, 260, 17, 19, 100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 50, 10, 260, 17, 19, 100, 100, -101, -101, 1, 1);
	if (Game_Flag_Query(293)) {
		Scene_Loop_Set_Default(3);
	} else {
		Scene_Loop_Set_Default(0);
	}
}

void ScriptCT02::SceneLoaded() {
	Obstacle_Object("STOVE-1", true);
	Unobstacle_Object("BACK-DOOR", true);
	Unclickable_Object("STOVE-1");
	Unclickable_Object("STOVE-2");
	Unclickable_Object("STOVE-3");
	Unclickable_Object("STOVE-4");
	Unclickable_Object("BIGPOT");
	Unclickable_Object("SOUP-BOWL");
	Unclickable_Object("HOWCOUNTRM");
	Unclickable_Object("LFTSTOVE-1");
	Unclickable_Object("FRIDGE-1");
	Unclickable_Object("LEFTWALL");
	Unclickable_Object("RIGHTWALL");
	Unclickable_Object("BACKWALL");
	Unclickable_Object("TABLE-1");
	Unclickable_Object("COUNTER-2");
	Unclickable_Object("COFFEJUG IN FOREGRO");
	Unclickable_Object("BACK-DOOR");
	if (!Game_Flag_Query(293)) {
		Preload(0);
		Preload(3);
		Preload(3);
		Preload(28);
		Preload(400);
		Preload(419);
		Preload(420);
	}
	if (Game_Flag_Query(720)) {
		Game_Flag_Reset(720);
		Actor_Change_Animation_Mode(0, 0);
		Player_Set_Combat_Mode(true);
		Player_Gains_Control();
	}
}

bool ScriptCT02::MouseClick(int x, int y) {
	if (Actor_Query_Goal_Number(19) == 8) {
		Actor_Set_Goal_Number(0, 1);
		return true;
	}
	return false;
}

bool ScriptCT02::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

void ScriptCT02::sub_401ACC() {
	Dialogue_Menu_Clear_List();
	if (Actor_Clue_Query(0, 13)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(270, 8, 5, 3);
	}
	if (Actor_Clue_Query(0, 22) && !Actor_Clue_Query(0, 13)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(280, 8, 5, 3);
	}
	int v0 = 0;
	if (Actor_Clue_Query(0, 2)) {
		v0 = 1;
	}
	if (Actor_Clue_Query(0, 10)) {
		++v0;
	}
	if (Actor_Clue_Query(0, 3)) {
		++v0;
	}
	if (Actor_Clue_Query(0, 16)) {
		++v0;
	}
	if (Actor_Clue_Query(0, 25)) {
		++v0;
	}
	if (v0 > 3) {
		DM_Add_To_List_Never_Repeat_Once_Selected(290, -1, 4, 8);
	}
	Dialogue_Menu_Add_DONE_To_List(300);
	Dialogue_Menu_Appear(320, 240);
	int answerValue = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();
	switch (answerValue) {
	case 270:
		Actor_Says(0, 380, 11);
		Actor_Says(19, 30, 17);
		Actor_Says(19, 40, 15);
		Actor_Says(0, 410, 9);
		Actor_Says(19, 50, 18);
		Actor_Says(0, 415, 10);
		Actor_Clue_Acquire(0, 19, 0, -1);
		Actor_Modify_Friendliness_To_Other(19, 0, -5);
		if (Actor_Query_Friendliness_To_Other(19, 0) < 44) {
			Scene_Exits_Disable();
			Actor_Clue_Acquire(0, 18, 1, -1);
			Actor_Set_Goal_Number(19, 8);
			Game_Flag_Set(293);
			Scene_Loop_Set_Default(3);
			Scene_Loop_Start_Special(2, 2, 1);
		}
		break;
	case 280:
		Actor_Says(0, 385, 9);
		Actor_Says(19, 40, 19);
		Actor_Modify_Friendliness_To_Other(19, 0, -2);
		if (Actor_Query_Friendliness_To_Other(19, 0) < 44) {
			Scene_Exits_Disable();
			Actor_Clue_Acquire(0, 18, 1, -1);
			Actor_Set_Goal_Number(19, 8);
			Game_Flag_Set(293);
			Scene_Loop_Set_Default(3);
			Scene_Loop_Start_Special(2, 2, 1);
		}
		break;
	case 290:
		Actor_Says(0, 395, 9);
		Actor_Says(0, 400, 9);
		Actor_Says(19, 70, 17);
		Actor_Says(0, 420, 10);
		Actor_Says(19, 80, 14);
		Actor_Modify_Friendliness_To_Other(19, 0, -10);
		if (Actor_Query_Friendliness_To_Other(19, 0) < 44) {
			Scene_Exits_Disable();
			Actor_Clue_Acquire(0, 18, 1, -1);
			Actor_Set_Goal_Number(19, 8);
			Game_Flag_Set(293);
			Scene_Loop_Set_Default(3);
			Scene_Loop_Start_Special(2, 2, 1);
		}
		break;
	case 300:
		Actor_Says(0, 405, 11);
		if (Actor_Query_Friendliness_To_Other(19, 0) < 44) {
			Scene_Exits_Disable();
			Actor_Clue_Acquire(0, 18, 1, -1);
			Actor_Set_Goal_Number(19, 8);
			Game_Flag_Set(293);
			Scene_Loop_Set_Default(3);
			Scene_Loop_Start_Special(2, 2, 1);
		}
		break;
	}
}

bool ScriptCT02::ClickedOnActor(int actorId) {
	if (actorId == 19 && Actor_Query_Goal_Number(19) == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, -255.02f, -145.11f, 212.42f, 0, 1, false, 0)) {
			Actor_Face_Actor(0, 19, true);
			Actor_Face_Actor(19, 0, true);
			if (!Game_Flag_Query(59)) {
				Actor_Says(0, 370, 10);
				Actor_Says(19, 20, 19);
				Actor_Says(0, 375, 9);
				Game_Flag_Set(59);
			}
			sub_401ACC();
			return true;
		}
	}
	return false;
}

bool ScriptCT02::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptCT02::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, -111.2f, -145.11f, 243.28f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(234);
			Game_Flag_Set(68);
			Set_Enter(4, 13);
		}
		return true;
	}
	if (exitId == 1) {
		bool v1;
		if (Player_Query_Combat_Mode()) {
			v1 = Loop_Actor_Walk_To_XYZ(0, -154.83f, -145.11f, -82.61f, 0, 1, true, 0);
		} else {
			v1 = Loop_Actor_Walk_To_XYZ(0, -154.83f, -145.11f, -82.61f, 0, 1, false, 0);
		}
		if (!v1) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(69);
			Set_Enter(5, 15);
		}
		return true;
	}
	return false;
}

bool ScriptCT02::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptCT02::SceneFrameAdvanced(int frame) {
	if (frame == 6 || frame == 12 || frame == 19 || frame == 25 || frame == 46 || frame == 59) {
		Sound_Play(97, Random_Query(25, 33), -70, -70, 50);
	}
	if (frame == 72) {
		Sound_Play(200, 50, 0, 0, 50);
	}
	if (frame == 71) {
		Sound_Play(204, 40, 0, 0, 50);
	}
	if (frame == 72) {
		Sound_Play(203, 60, -20, 40, 50);
	}
	if (frame == 61) {
		Music_Play(1, 50, 0, 2, -1, 0, 0);
	}
	if (frame == 81) {
		Scene_Exit_Add_2D_Exit(1, 332, 163, 404, 297, 0);
		Scene_Exits_Enable();
	}
}

void ScriptCT02::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptCT02::PlayerWalkedIn() {
}

void ScriptCT02::PlayerWalkedOut() {
	if (Actor_Clue_Query(0, 18)) {
		return;
	}
	Overlay_Remove("ct02over");
}

void ScriptCT02::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
