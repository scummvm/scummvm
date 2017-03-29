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

void SceneScriptAR02::InitializeScene() {
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

void SceneScriptAR02::SceneLoaded() {
	Obstacle_Object("DF_BOOTH", true);
	if (!Game_Flag_Query(374)) {
		Item_Add_To_World(106, 976, 0, -442.84f, 36.77f, -1144.51f, 360, 36, 36, false, true, false, true);
	}
	if (Global_Variable_Query(1) == 4 && !Game_Flag_Query(374)) {
		Game_Flag_Set(0);
		Item_Remove_From_World(106);
	}
}

bool SceneScriptAR02::MouseClick(int x, int y) {
	return Region_Check(250, 215, 325, 260);
}

bool SceneScriptAR02::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptAR02::ClickedOnActor(int actorId) {
	if (actorId == 16) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -386.96f, 0.0f, -1078.45f, 12, 1, false, 0)) {
			Actor_Face_Actor(kActorMcCoy, kActorInsectDealer, true);
			Actor_Face_Actor(kActorInsectDealer, kActorMcCoy, true);
			if (Global_Variable_Query(1) == 2) {
				if (Game_Flag_Query(329) && !Game_Flag_Query(366)) {
					Game_Flag_Set(366);
					Player_Loses_Control();
					ADQ_Flush();
					ADQ_Add(kActorInsectDealer, 210, 14);
					Loop_Actor_Walk_To_XYZ(kActorMcCoy, -350.66f, 0.0f, -1117.19f, 0, 0, false, 0);
					Actor_Face_Actor(kActorMcCoy, kActorInsectDealer, true);
					Actor_Says(kActorMcCoy, 110, 18);
					Actor_Says(kActorInsectDealer, 230, 14);
					Actor_Says(kActorMcCoy, 115, 18);
					Actor_Says(kActorInsectDealer, 240, 14);
					Item_Pickup_Spin_Effect(956, 288, 257);
					Actor_Says(kActorInsectDealer, 250, 14);
					Player_Gains_Control();
					sub_402AE0();
				} else if (Game_Flag_Query(329)) {
					Actor_Says(kActorMcCoy, 75, 18);
					Actor_Says(kActorInsectDealer, 60, 12);
					Actor_Says(kActorInsectDealer, 70, 14);
				} else {
					sub_402694();
				}
			} else if (Global_Variable_Query(1) > 2) {
				if (Actor_Clue_Query(kActorMcCoy, kCluePeruvianLadyInterview) && !Actor_Clue_Query(kActorMcCoy, kClueDragonflyCollection)) {
					Actor_Says(kActorMcCoy, 205, 16);
					Actor_Says(kActorInsectDealer, 290, 12);
					Actor_Says(kActorInsectDealer, 300, 13);
					Actor_Says(kActorMcCoy, 210, 15);
					Actor_Says(kActorInsectDealer, 310, 12);
					Actor_Says(kActorMcCoy, 215, 13);
					if (Game_Flag_Query(374)) {
						Actor_Says(kActorMcCoy, 220, 14);
						Actor_Says(kActorInsectDealer, 320, 12);
						Actor_Says(kActorMcCoy, 225, 13);
						Actor_Says(kActorInsectDealer, 330, 14);
						Actor_Says(kActorMcCoy, 230, 19);
						Actor_Says(kActorInsectDealer, 340, 13);
						Actor_Says(kActorInsectDealer, 350, 12);
						Actor_Says(kActorMcCoy, 235, 16);
						Actor_Clue_Acquire(kActorMcCoy, kCluePurchasedScorpions, 0, kActorInsectDealer);
					}
					Actor_Clue_Acquire(kActorMcCoy, kClueDragonflyCollection, 0, kActorInsectDealer);
				} else {
					Actor_Says(kActorMcCoy, 240, 17);
					Actor_Says(kActorInsectDealer, 360, 13);
					Actor_Says(kActorInsectDealer, 370, 14);
					Actor_Says(kActorMcCoy, 245, 13);
				}
			}
			return true;
		}
	}
	if (actorId == 20 && Global_Variable_Query(1) == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -240.79f, 0.0f, -1328.89f, 12, 1, false, 0)) {
			Actor_Face_Actor(kActorMcCoy, kActorHasan, true);
			Actor_Face_Actor(kActorHasan, kActorMcCoy, true);
			if (Game_Flag_Query(330)) {
				sub_402CE4();
				return false;
			}
			Actor_Says(kActorHasan, 0, 14);
			Actor_Says(kActorMcCoy, 140, 18);
			Game_Flag_Set(330);
			return true;
		}
	}
	return false;
}

bool SceneScriptAR02::ClickedOnItem(int itemId, bool a2) {
	if (itemId == 106) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -386.96f, 0.0f, -1078.45f, 12, 1, false, 0)) {
			Actor_Face_Actor(kActorMcCoy, kActorInsectDealer, true);
			if (!Game_Flag_Query(331)) {
				Actor_Says(kActorInsectDealer, 0, 14);
				Actor_Says(kActorMcCoy, 55, 18);
				Actor_Says(kActorInsectDealer, 10, 14);
				Actor_Says(kActorMcCoy, 60, 18);
				Actor_Says(kActorInsectDealer, 20, 14);
				Game_Flag_Set(331);
			} else if (Game_Flag_Query(331) && !Game_Flag_Query(367)) {
				Actor_Says(kActorMcCoy, 65, 21);
				Actor_Says(kActorInsectDealer, 30, 14);
				Actor_Says(kActorInsectDealer, 40, 14);
				Actor_Says(kActorMcCoy, 70, 18);
				Actor_Says(kActorInsectDealer, 50, 14);
				Game_Flag_Set(367);
			} else {
				Actor_Says(kActorMcCoy, 8527, 14);
			}
			return true;
		}
	}
	return false;
}

bool SceneScriptAR02::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -182.0f, 0.0f, -551.0f, 0, 1, false, 0)) {
			Game_Flag_Set(321);
			Async_Actor_Walk_To_XYZ(kActorMcCoy, -182.0f, 0.0f, -407.0f, 0, false);
			Set_Enter(0, 0);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -465.0f, 0.0f, -799.0f, 0, 1, false, 0)) {
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -560.0f, 0.0f, -799.0f, 0, 0, false, 0);
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

bool SceneScriptAR02::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptAR02::SceneFrameAdvanced(int frame) {
}

void SceneScriptAR02::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptAR02::PlayerWalkedIn() {
	if (Game_Flag_Query(116) == 1) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -465.0f, 0.0f, -799.0f, 0, 0, false, 0);
		Game_Flag_Reset(116);
	}
	Game_Flag_Set(726);
}

void SceneScriptAR02::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void SceneScriptAR02::DialogueQueueFlushed(int a1) {
}

void SceneScriptAR02::sub_402694() {
	Dialogue_Menu_Clear_List();
	if (Actor_Clue_Query(kActorMcCoy, kClueDragonflyEarring) || Actor_Clue_Query(kActorMcCoy, kClueBombingSuspect)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(490, 3, 5, 5);
	}
	if (Actor_Clue_Query(kActorMcCoy, kClueDragonflyAnklet) && !Actor_Clue_Query(kActorMcCoy, kClueDragonflyEarring) && !Actor_Clue_Query(kActorMcCoy, kClueBombingSuspect)) {
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
			Actor_Says(kActorMcCoy, 145, 15);
		} else {
			Actor_Says(kActorMcCoy, 150, 15);
		}
		Actor_Says(kActorInsectDealer, 80, 14);
		Actor_Says(kActorMcCoy, 80, 16);
		Actor_Says(kActorInsectDealer, 90, 12);
		Actor_Says(kActorMcCoy, 85, 17);
		Actor_Says(kActorInsectDealer, 100, 14);
		Actor_Says(kActorInsectDealer, 110, 12);
		Actor_Says(kActorInsectDealer, 120, 12);
		Actor_Says(kActorMcCoy, 90, 13);
		Actor_Says(kActorInsectDealer, 130, 12);
		Actor_Says(kActorInsectDealer, 140, 14);
		Actor_Says(kActorMcCoy, 95, 15);
		Actor_Says(kActorInsectDealer, 150, 12);
		Actor_Says(kActorInsectDealer, 160, 13);
		Actor_Says(kActorInsectDealer, 170, 14);
		Actor_Says(kActorMcCoy, 100, 16);
		Actor_Says(kActorInsectDealer, 180, 13);
		Game_Flag_Set(329);
		Actor_Clue_Acquire(kActorMcCoy, kCluePeruvianLadyInterview, 1, kActorInsectDealer);
		break;
	case 510:
		Actor_Says(kActorMcCoy, 8475, 12);
		Actor_Says(kActorInsectDealer, 190, 12);
		Actor_Says(kActorMcCoy, 105, 15);
		Actor_Says(kActorInsectDealer, 200, 14);
		break;
	case 520:
		Actor_Says(kActorMcCoy, 215, 16);
		break;
	}
}

void SceneScriptAR02::sub_402AE0() {
	Dialogue_Menu_Clear_List();
	if (Global_Variable_Query(2) >= 15 || Query_Difficulty_Level() == 0) {
		DM_Add_To_List_Never_Repeat_Once_Selected(530, 7, 5, 3);
	}
	DM_Add_To_List_Never_Repeat_Once_Selected(540, 3, 5, 7);
	Dialogue_Menu_Appear(320, 240);
	int answerValue = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();
	if (answerValue == 530) {
		Actor_Says(kActorMcCoy, 120, 12);
		if (Query_Difficulty_Level() != 0) {
			Global_Variable_Decrement(2, 15);
		}
		Actor_Clue_Acquire(kActorMcCoy, kClueMaggieBracelet, 1, kActorInsectDealer);
		Actor_Modify_Friendliness_To_Other(kActorInsectDealer, kActorMcCoy, 5);
	} else if (answerValue == 540) {
		Actor_Says(kActorMcCoy, 125, 13);
		Actor_Says(kActorInsectDealer, 260, 3);
		Actor_Says(kActorMcCoy, 130, 15);
		Actor_Says(kActorInsectDealer, 270, 3);
		Actor_Says(kActorInsectDealer, 280, 3);
		Actor_Says(kActorMcCoy, 135, 11);
		Actor_Modify_Friendliness_To_Other(kActorInsectDealer, kActorMcCoy, -5);
	}
}

void SceneScriptAR02::sub_402CE4() {
	Dialogue_Menu_Clear_List();
	if (Actor_Clue_Query(kActorMcCoy, kClueStrangeScale1)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(550, 8, 5, 2);
	}
	if (Actor_Clue_Query(kActorMcCoy, kClueDragonflyEarring)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(560, 6, 5, 7);
	}
	Dialogue_Menu_Add_DONE_To_List(570);
	Dialogue_Menu_Appear(320, 240);
	int answerValue = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();
	switch (answerValue) {
	case 550:
		Actor_Says(kActorMcCoy, 145, 11);
		Actor_Says(kActorHasan, 30, 11);
		Actor_Says(kActorMcCoy, 160, 11);
		Actor_Says(kActorHasan, 40, 11);
		Actor_Says(kActorMcCoy, 165, 11);
		Actor_Says(kActorHasan, 50, 11);
		Actor_Says(kActorMcCoy, 170, 11);
		Actor_Says(kActorHasan, 60, 11);
		Actor_Says(kActorMcCoy, 175, 11);
		Game_Flag_Set(370);
		break;
	case 560:
		Actor_Says(kActorMcCoy, 150, 11);
		Actor_Says(kActorHasan, 140, 11);
		Actor_Says(kActorMcCoy, 185, 11);
		Actor_Says(kActorHasan, 150, 11);
		Actor_Says(kActorHasan, 160, 11);
		Actor_Says(kActorMcCoy, 190, 11);
		Actor_Says(kActorHasan, 170, 11);
		Actor_Says(kActorMcCoy, 195, 11);
		Actor_Says(kActorHasan, 180, 11);
		Actor_Says(kActorHasan, 190, 11);
		Actor_Says(kActorHasan, 200, 11);
		Actor_Says(kActorMcCoy, 200, 11);
		Actor_Says(kActorHasan, 210, 11);
		Actor_Says(kActorHasan, 220, 11);
		Actor_Says(kActorHasan, 230, 11);
		Game_Flag_Set(370);
		break;
	case 570:
		if (Actor_Clue_Query(kActorMcCoy, kClueHasanInterview)) {
			Actor_Says(kActorMcCoy, 1315, 11);
		} else {
			Actor_Says(kActorMcCoy, 940, 13);
			Actor_Says(kActorHasan, 70, 12);
			Actor_Says(kActorHasan, 90, 12);
			Actor_Says(kActorMcCoy, 180, 15);
			Actor_Says(kActorHasan, 100, 14);
			Actor_Says(kActorHasan, 110, 12);
			Actor_Says(kActorHasan, 120, 13);
			Actor_Modify_Friendliness_To_Other(kActorHasan, kActorMcCoy, -1);
			Actor_Clue_Acquire(kActorMcCoy, kClueHasanInterview, 0, kActorHasan);
		}
		break;
	}
}

} // End of namespace BladeRunner
