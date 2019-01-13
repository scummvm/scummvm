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

void SceneScriptRC04::InitializeScene() {
	Setup_Scene_Information(45.0f, 0.15f, 68.0f, 1018);
	Game_Flag_Reset(kFlagRC03toRC04);
	Scene_Exit_Add_2D_Exit(0, 225, 47, 359, 248, 0);
	if (!Game_Flag_Query(289)) {
		Actor_Put_In_Set(kActorBulletBob, 71);
		Actor_Set_At_XYZ(kActorBulletBob, -60.0f, -11.0f, 62.0f, 12);
	}
	if (Game_Flag_Query(289)) {
		Actor_Change_Animation_Mode(kActorBulletBob, 88);
	}
	Ambient_Sounds_Add_Looping_Sound(381, 100, 1, 1);
	Ambient_Sounds_Add_Sound(82, 5, 30, 10, 20, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(83, 5, 30, 10, 20, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(84, 5, 30, 10, 20, -100, 100, -101, -101, 0, 0);
}

void SceneScriptRC04::SceneLoaded() {
	Obstacle_Object("Door New 01", true);
	Obstacle_Object("GRNDNEON05", true);
	Obstacle_Object("GRNDNEON06", true);
	Obstacle_Object("GRNDNEON07", true);
	Unobstacle_Object("DisplayTrim", true);
	Unobstacle_Object("Display01", true);
	Actor_Set_Goal_Number(kActorGenwalkerA, 200);
}

bool SceneScriptRC04::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptRC04::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

void SceneScriptRC04::sub_401DF4() {
	Dialogue_Menu_Clear_List();
	if (Actor_Clue_Query(kActorMcCoy, kClueLabShellCasings) && !Actor_Clue_Query(kActorMcCoy, kClueRadiationGoggles)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(580, -1, 4, 9);
	}
	if (Actor_Clue_Query(kActorMcCoy, kClueShellCasings) && !Actor_Clue_Query(kActorMcCoy, kClueLabShellCasings) && !Actor_Clue_Query(kActorMcCoy, kClueRadiationGoggles)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(590, 6, 5, 3);
	}
	if (Actor_Clue_Query(kActorMcCoy, kClueHasanInterview)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(600, -1, 3, 7);
		DM_Add_To_List_Never_Repeat_Once_Selected(1310, -1, 2, 8);
	}
	if (Actor_Clue_Query(kActorMcCoy, kClueRadiationGoggles) && !Actor_Clue_Query(kActorMcCoy, kClueGogglesReplicantIssue)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(610, 4, 5, 6);
	}
	if (!Game_Flag_Query(305)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1280, 1, 2, 3);
	}
	if (Actor_Clue_Query(kActorMcCoy, kClueHoldensBadge) && !Actor_Clue_Query(kActorBulletBob, kClueHoldensBadge)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(620, 1, -1, -1);
	}
	Dialogue_Menu_Add_DONE_To_List(630);
	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();

	switch (answer) {
	case 580:
		Actor_Says(kActorMcCoy, 4955, 30);
		Actor_Says(kActorBulletBob, 210, 37);
		Actor_Says(kActorBulletBob, 220, 37);
		Actor_Says(kActorBulletBob, 230, 37);
		Actor_Says(kActorBulletBob, 240, 37);
		Actor_Says(kActorMcCoy, 4990, 16);
		Actor_Says(kActorMcCoy, 4995, 11);
		Actor_Says(kActorBulletBob, 270, 31);
		Actor_Says(kActorMcCoy, 5005, 16);
		Actor_Says(kActorBulletBob, 280, 32);
		Actor_Says(kActorBulletBob, 290, 30);
		Actor_Says(kActorBulletBob, 300, 33);
		Actor_Says(kActorBulletBob, 310, 31);
		Actor_Says(kActorMcCoy, 5010, 11);
		Actor_Says(kActorBulletBob, 320, 30);
		Actor_Says(kActorBulletBob, 330, 33);
		Actor_Says(kActorBulletBob, 340, 37);
		Actor_Says(kActorMcCoy, 5015, 11);
		if (Game_Flag_Query(kFlagIzoIsReplicant)) {
			Actor_Says(kActorBulletBob, 350, 32);
			Actor_Says(kActorBulletBob, 360, 33);
			Actor_Says(kActorBulletBob, 370, 30);
			Actor_Says(kActorMcCoy, 5020, 16);
			Actor_Says(kActorBulletBob, 380, 37);
			Actor_Says(kActorBulletBob, 390, 11);
			Actor_Says(kActorBulletBob, 400, 37);
			Actor_Clue_Acquire(kActorMcCoy, kClueBobInterview1, 1, kActorMcCoy);
		} else {
			Actor_Says(kActorBulletBob, 410, 11);
			Actor_Says(kActorBulletBob, 420, 37);
			Actor_Says(kActorMcCoy, 5025, 16);
			Actor_Says(kActorBulletBob, 430, 30);
			Actor_Says(kActorBulletBob, 440, 31);
			Actor_Says(kActorBulletBob, 450, 32);
			Actor_Says(kActorMcCoy, 5030, 16);
			Actor_Says(kActorBulletBob, 460, 37);
			Actor_Clue_Acquire(kActorMcCoy, kClueBobInterview2, 1, kActorMcCoy);
		}
		return;
	case 590:
		Actor_Says(kActorMcCoy, 4960, 13);
		Actor_Says(kActorBulletBob, 250, 30);
		Actor_Says(kActorBulletBob, 260, 33);
		Actor_Says(kActorMcCoy, 4995, 15);
		Actor_Says(kActorBulletBob, 270, 32);
		Actor_Says(kActorMcCoy, 5005, 11);
		Actor_Says(kActorBulletBob, 280, 33);
		Actor_Says(kActorBulletBob, 290, 30);
		Actor_Says(kActorBulletBob, 300, 32);
		Actor_Says(kActorBulletBob, 310, 37);
		Actor_Says(kActorMcCoy, 5010, 13);
		Actor_Says(kActorBulletBob, 320, 37);
		Actor_Says(kActorBulletBob, 330, 33);
		Actor_Says(kActorBulletBob, 340, 11);
		Actor_Says(kActorMcCoy, 5015, 16);
		Actor_Modify_Friendliness_To_Other(kActorBulletBob, kActorMcCoy, 3);
		if (Game_Flag_Query(kFlagIzoIsReplicant)) {
			Actor_Says(kActorBulletBob, 350, 32);
			Actor_Says(kActorBulletBob, 360, 30);
			Actor_Says(kActorBulletBob, 370, 33);
			Actor_Says(kActorMcCoy, 5020, 15);
			Actor_Says(kActorBulletBob, 380, 33);
			Actor_Says(kActorBulletBob, 390, 37);
			Actor_Says(kActorBulletBob, 400, 32);
			Actor_Clue_Acquire(kActorMcCoy, kClueBobInterview1, 1, kActorMcCoy);
		} else {
			Actor_Says(kActorBulletBob, 410, 32);
			Actor_Says(kActorBulletBob, 420, 30);
			Actor_Says(kActorMcCoy, 5025, 13);
			Actor_Says(kActorBulletBob, 430, 33);
			Actor_Says(kActorBulletBob, 440, 32);
			Actor_Says(kActorBulletBob, 450, 37);
			Actor_Says(kActorMcCoy, 5030, 16);
			Actor_Says(kActorBulletBob, 460, 30);
			Actor_Clue_Acquire(kActorMcCoy, kClueBobInterview2, 1, kActorMcCoy);
		}
		break;
	case 600:
		Actor_Says(kActorMcCoy, 4965, 11);
		Actor_Says(kActorBulletBob, 470, 11);
		Actor_Says(kActorMcCoy, 5035, 15);
		Actor_Says(kActorBulletBob, 480, 30);
		Actor_Says(kActorBulletBob, 490, 31);
		Actor_Says(kActorBulletBob, 500, 32);
		Actor_Says(kActorBulletBob, 510, 33);
		Actor_Says(kActorBulletBob, 520, 34);
		Actor_Says(kActorBulletBob, 530, 35);
		Actor_Says(kActorBulletBob, 540, 36);
		Actor_Says(kActorMcCoy, 5040, 16);
		Actor_Says(kActorBulletBob, 550, 11);
		Actor_Modify_Friendliness_To_Other(kActorBulletBob, kActorMcCoy, -6);
		break;
	case 610:
		Actor_Says(kActorMcCoy, 4970, 16);
		if (Actor_Query_Friendliness_To_Other(kActorBulletBob, kActorMcCoy) < 50) {
			Actor_Says(kActorBulletBob, 700, 11);
			Actor_Says(kActorMcCoy, 5070, 11);
			Actor_Says(kActorBulletBob, 710, 11);
			Actor_Says(kActorMcCoy, 5075, 15);
			Actor_Says(kActorBulletBob, 720, 30);
			Actor_Says(kActorMcCoy, 5080, 11);
			Actor_Says(kActorBulletBob, 730, 37);
			Actor_Clue_Acquire(kActorMcCoy, kClueBobInterview1, 1, kActorMcCoy);
		} else {
			Actor_Says(kActorBulletBob, 560, 37);
			Actor_Says(kActorMcCoy, 5070, 13);
			Actor_Says(kActorBulletBob, 570, 36);
			Actor_Says(kActorBulletBob, 580, 37);
			Actor_Says(kActorBulletBob, 590, 31);
			Actor_Says(kActorBulletBob, 600, 32);
			Actor_Says(kActorBulletBob, 610, 30);
			Actor_Says(kActorMcCoy, 5050, 16);
			Actor_Says(kActorBulletBob, 620, 35);
			Actor_Says(kActorBulletBob, 630, 35);
			Actor_Says(kActorMcCoy, 5055, 11);
			Actor_Says(kActorBulletBob, 640, 36);
			Actor_Says(kActorBulletBob, 650, 35);
			Actor_Says(kActorBulletBob, 660, 30);
			Actor_Says(kActorMcCoy, 5060, 13);
			Actor_Clue_Acquire(kActorMcCoy, kClueGogglesReplicantIssue, 1, kActorMcCoy);
		}
		break;
	case 1280:
		Actor_Says(kActorMcCoy, 9040, 16);
		if (!Game_Flag_Query(305)) {
			Actor_Says(kActorBulletBob, 2080, 30);
			Actor_Says(kActorBulletBob, 2090, 37);
			Actor_Says(kActorMcCoy, 9045, 14);
			Actor_Says(kActorBulletBob, 2100, 32);
			Actor_Says(kActorBulletBob, 2110, 37);
			Game_Flag_Set(305);
		}
		Actor_Says(kActorBulletBob, 2120, 31);
		if (Global_Variable_Query(2) > 40 || Query_Difficulty_Level() == 0) {
			Actor_Says(kActorMcCoy, 4940, 13);
			if (Query_Difficulty_Level() != 0) {
				Global_Variable_Decrement(2, 40);
			}
			Item_Pickup_Spin_Effect(995, 405, 192);
			Give_McCoy_Ammo(1, 24);
		} else {
			Actor_Says(kActorMcCoy, 125, 13);
			Actor_Modify_Friendliness_To_Other(kActorBulletBob, kActorMcCoy, -2);
		}
		break;
	case 1310:
		Actor_Says(kActorMcCoy, 4980, 11);
		if (Actor_Query_Friendliness_To_Other(kActorBulletBob, kActorMcCoy) > 49) {
			Actor_Says(kActorBulletBob, 740, 37);
			Actor_Says(kActorMcCoy, 5085, 16);
			Actor_Says(kActorBulletBob, 750, 37);
			Actor_Says(kActorBulletBob, 760, 37);
			Voight_Kampff_Activate(kActorBulletBob, 50);
			Actor_Modify_Friendliness_To_Other(kActorBulletBob, kActorMcCoy, 3);
			Actor_Says(kActorBulletBob, 810, 37);
			Actor_Says(kActorMcCoy, 5025, 13);
			Actor_Says(kActorBulletBob, 820, 32);
			Actor_Says(kActorMcCoy, 5100, 11);
			Actor_Says(kActorBulletBob, 830, 31);
			Actor_Says(kActorBulletBob, 840, 35);
		} else {
			Actor_Says(kActorBulletBob, 770, 36);
			Actor_Says(kActorBulletBob, 780, 36);
			Actor_Says(kActorMcCoy, 5090, 16);
			Actor_Says(kActorBulletBob, 790, 36);
			Actor_Says(kActorBulletBob, 800, 35);
			Voight_Kampff_Activate(kActorBulletBob, 50);
			Actor_Says(kActorBulletBob, 810, 30);
			Actor_Says(kActorMcCoy, 5025, 13);
			Actor_Says(kActorBulletBob, 820, 31);
			Actor_Says(kActorMcCoy, 5100, 15);
			Actor_Says(kActorBulletBob, 830, 34);
			Actor_Says(kActorBulletBob, 840, 34);
		}
		break;
	case 620:
		Actor_Says(kActorMcCoy, 4985, 11);
		Actor_Says(kActorBulletBob, 850, 35);
		Actor_Says(kActorMcCoy, 5105, 13);
		Actor_Says(kActorMcCoy, 5110, 11);
		Actor_Says(kActorBulletBob, 860, 30);
		Actor_Says(kActorMcCoy, 5115, 16);
		Actor_Says(kActorBulletBob, 870, 31);
		Actor_Says(kActorMcCoy, 5120, 15);
		Actor_Says(kActorBulletBob, 880, 34);
		Actor_Clue_Acquire(kActorBulletBob, kClueHoldensBadge, 1, kActorMcCoy);
		Actor_Modify_Friendliness_To_Other(kActorBulletBob, kActorMcCoy, 8);
		if (Query_Difficulty_Level() != 0) {
			Global_Variable_Increment(2, 60);
		}
		break;
	case 630:
		Actor_Says(kActorMcCoy, 1315, 12);
		break;
	}
}

bool SceneScriptRC04::ClickedOnActor(int actorId) {
	if (Player_Query_Combat_Mode()) {
		return false;
	}
	if (actorId == 14 && Global_Variable_Query(kVariableChapter) == 2 && !Game_Flag_Query(289)) {
		Loop_Actor_Walk_To_Waypoint(kActorMcCoy, 104, 0, 0, false);
		Actor_Face_Actor(kActorMcCoy, kActorBulletBob, true);
		if (Game_Flag_Query(287) && !Game_Flag_Query(292) && Actor_Query_Friendliness_To_Other(kActorBulletBob, kActorMcCoy) > 45) {
			Actor_Says(kActorBulletBob, 30, 30);
			Actor_Says(kActorMcCoy, 4875, 13);
			Actor_Says(kActorBulletBob, 80, 31);
			Actor_Says(kActorMcCoy, 4900, 15);
			Actor_Says(kActorBulletBob, 90, 33);
			Actor_Says(kActorBulletBob, 100, 34);
			Actor_Says(kActorMcCoy, 4905, 15);
			Game_Flag_Set(292);
		} else if (Game_Flag_Query(287) && !Game_Flag_Query(290) && Actor_Query_Friendliness_To_Other(kActorBulletBob, kActorMcCoy) < 45) {
			Actor_Says(kActorBulletBob, 40, 30);
			Actor_Says(kActorMcCoy, 4880, 13);
			Actor_Says(kActorBulletBob, 50, 35);
			Actor_Says(kActorMcCoy, 4875, 16);
			Actor_Says(kActorBulletBob, 60, 36);
			Actor_Says(kActorMcCoy, 4890, 13);
			Actor_Says(kActorBulletBob, 70, 33);
			Actor_Says(kActorMcCoy, 4895, 16);
			Actor_Modify_Friendliness_To_Other(kActorBulletBob, kActorMcCoy, -5);
			Game_Flag_Set(290);
		} else if (Actor_Query_Friendliness_To_Other(kActorBulletBob, kActorMcCoy) > 51 && !Game_Flag_Query(717)) {
			Actor_Says(kActorBulletBob, 1870, 30);
			Actor_Says(kActorBulletBob, 1880, 30);
			Actor_Says(kActorMcCoy, 8960, 13);
			Actor_Says(kActorBulletBob, 1890, 36);
			Actor_Says(kActorBulletBob, 1900, 35);
			Actor_Says(kActorMcCoy, 8965, 16);
			Actor_Says(kActorBulletBob, 1920, 36);
			Actor_Says(kActorBulletBob, 1930, 33);
			Actor_Says(kActorBulletBob, 1940, 36);
			Actor_Says(kActorBulletBob, 1950, 30);
			Actor_Says(kActorMcCoy, 8970, 13);
			Actor_Says(kActorBulletBob, 1960, 33);
			Actor_Says(kActorBulletBob, 1970, 30);
			Actor_Says(kActorBulletBob, 1980, 36);
			Delay(1000);
			Actor_Says(kActorBulletBob, 2010, 35);
			if (Global_Variable_Query(2) > 50 || Query_Difficulty_Level() == 0) {
				Actor_Says(kActorMcCoy, 8975, 16);
				if (Query_Difficulty_Level() != 0) {
					Global_Variable_Decrement(2, 50);
				}
				Delay(3000);
				Item_Pickup_Spin_Effect(941, 405, 192);
				Actor_Says(kActorBulletBob, 2030, 30);
				Game_Flag_Set(kFlagKIAPrivacyAddon);
			} else {
				Actor_Says(kActorMcCoy, 8980, 16);
				Actor_Says(kActorBulletBob, 2040, 30);
				Actor_Says(kActorMcCoy, 8985, 15);
				Actor_Says(kActorBulletBob, 2050, 33);
			}
			Game_Flag_Set(717);
		} else if (Actor_Clue_Query(kActorMcCoy, kClueLabShellCasings) || Actor_Clue_Query(kActorMcCoy, kClueShellCasings) || Actor_Clue_Query(kActorMcCoy, kClueRadiationGoggles) || Actor_Clue_Query(kActorMcCoy, kClueHoldensBadge) || Actor_Clue_Query(kActorMcCoy, kClueHasanInterview) || !Game_Flag_Query(305)) {
			sub_401DF4();
		} else {
			Actor_Says(kActorBulletBob, 1820, 30);
		}
		return true;
	}
	if (actorId == 14 && Game_Flag_Query(289)) {
		Actor_Face_Actor(kActorMcCoy, kActorBulletBob, true);
		if (Actor_Clue_Query(kActorMcCoy, kClueVKBobGorskyReplicant)) {
			Actor_Says(kActorMcCoy, 8590, -1);
		} else {
			Actor_Voice_Over(2100, kActorVoiceOver);
			Actor_Voice_Over(2110, kActorVoiceOver);
		}
		return true;
	}
	return false;
}

bool SceneScriptRC04::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptRC04::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 45.0f, 0.15f, 68.0f, 0, 1, false, 0)) {
			Game_Flag_Set(kFlagRC04toRC03);
			Set_Enter(kSetRC03, kSceneRC03);
		}
		return true;
	}
	return false;
}

bool SceneScriptRC04::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptRC04::SceneFrameAdvanced(int frame) {
}

void SceneScriptRC04::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptRC04::PlayerWalkedIn() {
	Loop_Actor_Walk_To_Waypoint(kActorMcCoy, 103, 0, 0, false);
	if (Global_Variable_Query(kVariableChapter) != 2 || Game_Flag_Query(287) || Player_Query_Combat_Mode()) {
		if (Global_Variable_Query(kVariableChapter) == 4 && !Game_Flag_Query(289) && !Game_Flag_Query(306)) {
			Actor_Says(kActorDispatcher, 40, 3);
			Actor_Says(kActorBulletBob, 890, 37);
			Actor_Set_Goal_Number(kActorBulletBob, 2);
		}
		Game_Flag_Set(287);
	} else {
		Actor_Says(kActorBulletBob, 0, 31);
		Loop_Actor_Walk_To_Waypoint(kActorMcCoy, 104, 0, 0, false);
		Actor_Face_Actor(kActorMcCoy, kActorBulletBob, true);
		Actor_Says(kActorMcCoy, 4865, 13);
		Actor_Says(kActorBulletBob, 10, 32);
		Actor_Says(kActorMcCoy, 4870, 16);
		Actor_Says(kActorBulletBob, 20, 31);
		Game_Flag_Set(287);
	}
}

void SceneScriptRC04::PlayerWalkedOut() {
	Game_Flag_Reset(303);
}

void SceneScriptRC04::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
