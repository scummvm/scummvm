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

void ScriptAR02::InitializeScene() {
	Music_Play(0, 22, 0, 2, -1, 1, 2);
	if (Game_Flag_Query(116)) {
		Setup_Scene_Information(-560.0f, 0.0f, -799.0f, 333);
	} else {
		Setup_Scene_Information(-182.0f, 0.0f, -551.0f, 973);
	}
	Scene_Exit_Add_2D_Exit(0, 0, 439, 212, 479, 2);
	Scene_Exit_Add_2D_Exit(1, 81, 202, 215, 406, 3);
	Ambient_Sounds_Add_Looping_Sound(54, 50, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(81, 60, 100, 1);
	Ambient_Sounds_Add_Looping_Sound(241, 50, 1, 1);
	Ambient_Sounds_Add_Sound(182, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(184, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(185, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(186, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(188, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(189, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(191, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(192, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(195, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(242, 3, 30, 16, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(243, 3, 30, 16, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(244, 3, 30, 16, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(245, 3, 30, 16, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(246, 3, 30, 16, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(247, 3, 30, 16, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(248, 3, 30, 16, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(249, 3, 30, 16, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Speech_Sound(60, 0, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 20, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 40, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 50, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Sound(68, 10, 180, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(69, 10, 180, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(375, 10, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(376, 10, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(377, 10, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	if (Game_Flag_Query(252) && Game_Flag_Query(320)) {
		Scene_Loop_Start_Special(0, 1, 0);
		Scene_Loop_Set_Default(2);
		Game_Flag_Reset(320);
	} else if (!Game_Flag_Query(252) && Game_Flag_Query(320)) {
		Scene_Loop_Start_Special(0, 0, 0);
		Scene_Loop_Set_Default(2);
		Game_Flag_Reset(320);
	} else {
		Scene_Loop_Set_Default(2);
	}
}

void ScriptAR02::SceneLoaded() {
	Obstacle_Object("DF_BOOTH", true);
	if (!Game_Flag_Query(374)) {
		Item_Add_To_World(106, 976, 0, -442.84f, 36.77f, -1144.51f, 360, 36, 36, false, true, false, true);
	}
	if (Global_Variable_Query(1) == 4 && !Game_Flag_Query(374)) {
		Game_Flag_Set(0);
		Item_Remove_From_World(106);
	}
}

bool ScriptAR02::MouseClick(int x, int y) {
	return Region_Check(250, 215, 325, 260);
}

bool ScriptAR02::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool ScriptAR02::ClickedOnActor(int actorId) {
	if (actorId == 16) {
		if (!Loop_Actor_Walk_To_XYZ(0, -386.96f, 0.0f, -1078.45f, 12, 1, false, 0)) {
			Actor_Face_Actor(0, 16, true);
			Actor_Face_Actor(16, 0, true);
			if (Global_Variable_Query(1) == 2) {
				if (Game_Flag_Query(329) && !Game_Flag_Query(366)) {
					Game_Flag_Set(366);
					Player_Loses_Control();
					ADQ_Flush();
					ADQ_Add(16, 210, 14);
					Loop_Actor_Walk_To_XYZ(0, -350.66f, 0.0f, -1117.19f, 0, 0, false, 0);
					Actor_Face_Actor(0, 16,true);
					Actor_Says(0, 110, 18);
					Actor_Says(16, 230, 14);
					Actor_Says(0, 115, 18);
					Actor_Says(16, 240, 14);
					Item_Pickup_Spin_Effect(956, 288, 257);
					Actor_Says(16, 250, 14);
					Player_Gains_Control();
					sub_402AE0();
				} else if (Game_Flag_Query(329)) {
					Actor_Says(0, 75, 18);
					Actor_Says(16, 60, 12);
					Actor_Says(16, 70, 14);
				} else {
					sub_402694();
				}
			} else if (Global_Variable_Query(1) > 2) {
				if (Actor_Clue_Query(0, 56) && !Actor_Clue_Query(0, 90)) {
					Actor_Says(0, 205, 16);
					Actor_Says(16, 290, 12);
					Actor_Says(16, 300, 13);
					Actor_Says(0, 210, 15);
					Actor_Says(16, 310, 12);
					Actor_Says(0, 215, 13);
					if (Game_Flag_Query(374)) {
						Actor_Says(0, 220, 14);
						Actor_Says(16, 320, 12);
						Actor_Says(0, 225, 13);
						Actor_Says(16, 330, 14);
						Actor_Says(0, 230, 19);
						Actor_Says(16, 340, 13);
						Actor_Says(16, 350, 12);
						Actor_Says(0, 235, 16);
						Actor_Clue_Acquire(0, 79, 0, 16);
					}
					Actor_Clue_Acquire(0, 90, 0, 16);
				} else {
					Actor_Says(0, 240, 17);
					Actor_Says(16, 360, 13);
					Actor_Says(16, 370, 14);
					Actor_Says(0, 245, 13);
				}
			}
			return true;
		}
	}
	if (actorId == 20 && Global_Variable_Query(1) == 2) {
		if (!Loop_Actor_Walk_To_XYZ(0, -240.79f, 0.0f, -1328.89f, 12, 1, false, 0)) {
			Actor_Face_Actor(0, 20, true);
			Actor_Face_Actor(20, 0, true);
			if (Game_Flag_Query(330)) {
				sub_402CE4();
				return false;
			}
			Actor_Says(20, 0, 14);
			Actor_Says(0, 140, 18);
			Game_Flag_Set(330);
			return true;
		}
	}
	return false;
}

bool ScriptAR02::ClickedOnItem(int itemId, bool a2) {
	if (itemId == 106) {
		if (!Loop_Actor_Walk_To_XYZ(0, -386.96f, 0.0f, -1078.45f, 12, 1, false, 0)) {
			Actor_Face_Actor(0, 16, true);
			if (!Game_Flag_Query(331)) {
				Actor_Says(16, 0, 14);
				Actor_Says(0, 55, 18);
				Actor_Says(16, 10, 14);
				Actor_Says(0, 60, 18);
				Actor_Says(16, 20, 14);
				Game_Flag_Set(331);
			} else if (Game_Flag_Query(331) && !Game_Flag_Query(367)) {
				Actor_Says(0, 65, 21);
				Actor_Says(16, 30, 14);
				Actor_Says(16, 40, 14);
				Actor_Says(0, 70, 18);
				Actor_Says(16, 50, 14);
				Game_Flag_Set(367);
			} else {
				Actor_Says(0, 8527, 14);
			}
			return true;
		}
	}
	return false;
}

bool ScriptAR02::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, -182.0f, 0.0f, -551.0f, 0, 1, false, 0)) {
			Game_Flag_Set(321);
			Async_Actor_Walk_To_XYZ(0, -182.0f, 0.0f, -407.0f, 0, false);
			Set_Enter(0, 0);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, -465.0f, 0.0f, -799.0f, 0, 1, false, 0)) {
			Loop_Actor_Walk_To_XYZ(0, -560.0f, 0.0f, -799.0f, 0, 0, false, 0);
			Game_Flag_Set(117);
			Game_Flag_Reset(180);
			Game_Flag_Set(182);
			Music_Stop(3);
			Set_Enter(70, 80);
		}
		return true;
	}
	return false;
}

bool ScriptAR02::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptAR02::SceneFrameAdvanced(int frame) {
}

void ScriptAR02::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptAR02::PlayerWalkedIn() {
	if (Game_Flag_Query(116) == 1) {
		Loop_Actor_Walk_To_XYZ(0, -465.0f, 0.0f, -799.0f, 0, 0, false, 0);
		Game_Flag_Reset(116);
	}
	Game_Flag_Set(726);
}

void ScriptAR02::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void ScriptAR02::DialogueQueueFlushed(int a1) {
}

void ScriptAR02::sub_402694() {
	Dialogue_Menu_Clear_List();
	if (Actor_Clue_Query(0, 44) || Actor_Clue_Query(0, 47)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(490, 3, 5, 5);
	}
	if (Actor_Clue_Query(0, 14) && !Actor_Clue_Query(0, 44) && !Actor_Clue_Query(0, 47)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(500, 3, 5, 5);
	}
	DM_Add_To_List_Never_Repeat_Once_Selected(510, 8, 3, -1);
	Dialogue_Menu_Add_DONE_To_List(520);
	Dialogue_Menu_Appear(320, 240);
	int answerValue = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();
	switch (answerValue) {
	case 490:
	case 500:
		if (answerValue == 490) {
			Actor_Says(0, 145, 15);
		} else {
			Actor_Says(0, 150, 15);
		}
		Actor_Says(16, 80, 14);
		Actor_Says(0, 80, 16);
		Actor_Says(16, 90, 12);
		Actor_Says(0, 85, 17);
		Actor_Says(16, 100, 14);
		Actor_Says(16, 110, 12);
		Actor_Says(16, 120, 12);
		Actor_Says(0, 90, 13);
		Actor_Says(16, 130, 12);
		Actor_Says(16, 140, 14);
		Actor_Says(0, 95, 15);
		Actor_Says(16, 150, 12);
		Actor_Says(16, 160, 13);
		Actor_Says(16, 170, 14);
		Actor_Says(0, 100, 16);
		Actor_Says(16, 180, 13);
		Game_Flag_Set(329);
		Actor_Clue_Acquire(0, 56, 1, 16);
		break;
	case 510:
		Actor_Says(0, 8475, 12);
		Actor_Says(16, 190, 12);
		Actor_Says(0, 105, 15);
		Actor_Says(16, 200, 14);
		break;
	case 520:
		Actor_Says(0, 215, 16);
		break;
	}
}

void ScriptAR02::sub_402AE0() {
	Dialogue_Menu_Clear_List();
	if (Global_Variable_Query(2) >= 15 || Query_Difficulty_Level() == 0) {
		DM_Add_To_List_Never_Repeat_Once_Selected(530, 7, 5, 3);
	}
	DM_Add_To_List_Never_Repeat_Once_Selected(540, 3, 5, 7);
	Dialogue_Menu_Appear(320, 240);
	int answerValue = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();
	if (answerValue == 530) {
		Actor_Says(0, 120, 12);
		if (Query_Difficulty_Level() != 0) {
			Global_Variable_Decrement(2, 15);
		}
		Actor_Clue_Acquire(0, 75, 1, 16);
		Actor_Modify_Friendliness_To_Other(16, 0, 5);
	} else if (answerValue == 540) {
		Actor_Says(0, 125, 13);
		Actor_Says(16, 260, 3);
		Actor_Says(0, 130, 15);
		Actor_Says(16, 270, 3);
		Actor_Says(16, 280, 3);
		Actor_Says(0, 135, 11);
		Actor_Modify_Friendliness_To_Other(16, 0, -5);
	}
}

void ScriptAR02::sub_402CE4() {
	Dialogue_Menu_Clear_List();
	if (Actor_Clue_Query(0, 93)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(550, 8, 5, 2);
	}
	if (Actor_Clue_Query(0, 44)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(560, 6, 5, 7);
	}
	Dialogue_Menu_Add_DONE_To_List(570);
	Dialogue_Menu_Appear(320, 240);
	int answerValue = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();
	switch (answerValue) {
	case 550:
		Actor_Says(0, 145, 11);
		Actor_Says(20, 30, 11);
		Actor_Says(0, 160, 11);
		Actor_Says(20, 40, 11);
		Actor_Says(0, 165, 11);
		Actor_Says(20, 50, 11);
		Actor_Says(0, 170, 11);
		Actor_Says(20, 60, 11);
		Actor_Says(0, 175, 11);
		Game_Flag_Set(370);
		break;
	case 560:
		Actor_Says(0, 150, 11);
		Actor_Says(20, 140, 11);
		Actor_Says(0, 185, 11);
		Actor_Says(20, 150, 11);
		Actor_Says(20, 160, 11);
		Actor_Says(0, 190, 11);
		Actor_Says(20, 170, 11);
		Actor_Says(0, 195, 11);
		Actor_Says(20, 180, 11);
		Actor_Says(20, 190, 11);
		Actor_Says(20, 200, 11);
		Actor_Says(0, 200, 11);
		Actor_Says(20, 210, 11);
		Actor_Says(20, 220, 11);
		Actor_Says(20, 230, 11);
		Game_Flag_Set(370);
		break;
	case 570:
		if (Actor_Clue_Query(0, 57)) {
			Actor_Says(0, 1315, 11);
		} else {
			Actor_Says(0, 940, 13);
			Actor_Says(20, 70, 12);
			Actor_Says(20, 90, 12);
			Actor_Says(0, 180, 15);
			Actor_Says(20, 100, 14);
			Actor_Says(20, 110, 12);
			Actor_Says(20, 120, 13);
			Actor_Modify_Friendliness_To_Other(20, 0, -1);
			Actor_Clue_Acquire(0, 57, 0, 20);
		}
		break;
	}
}

} // End of namespace BladeRunner
