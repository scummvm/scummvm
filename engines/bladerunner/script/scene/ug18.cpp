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

void SceneScriptUG18::InitializeScene() {
	Setup_Scene_Information(-684.71f, 0.0f, 171.59f, 0);
	Game_Flag_Reset(kFlagUG13toUG18);

	Scene_Exit_Add_2D_Exit(0, 0, 158, 100, 340, 3);

	Ambient_Sounds_Add_Looping_Sound(105, 71, 0, 1);
	Ambient_Sounds_Add_Looping_Sound( 95, 45, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(332, 76, 0, 1);
	Ambient_Sounds_Add_Sound(291, 2,  20, 25, 33, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(292, 2,  20, 25, 33, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(293, 2,  20, 25, 33, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(294, 2,  20, 25, 33, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(402, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(368, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(369, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(397, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(398, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(295, 2,  20, 25, 25, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(303, 5,  50, 47, 57, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(304, 5,  50, 47, 57, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(305, 5,  50, 47, 57, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(  1, 5,  50, 47, 57, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound( 57, 5,  50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound( 58, 5,  50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(306, 5,  50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(307, 5,  50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(308, 5,  50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(196, 5,  50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(197, 5,  50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(198, 5,  50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(199, 5,  50, 27, 37, -100, 100, -101, -101, 0, 0);

	Scene_Loop_Set_Default(4);

	if ( Game_Flag_Query(670)
	 && !Game_Flag_Query(671)
	 &&  Global_Variable_Query(kVariableChapter) == 4
	) {
		Actor_Set_Goal_Number(kActorGuzza, 300);
		Actor_Set_Goal_Number(kActorClovis, 300);
		Actor_Set_Goal_Number(kActorSadik, 300);
	}
}

void SceneScriptUG18::SceneLoaded() {
	Obstacle_Object("MACHINE_01", true);
	Unobstacle_Object("PLATFM_RAIL 01", true);
	Unobstacle_Object("PLATFM_RAIL 02", true);
	Unobstacle_Object("OBSTACLE1", true);
	Clickable_Object("MACHINE_01");
	Unclickable_Object("MACHINE_01");

	if (Game_Flag_Query(671)) {
		Actor_Put_In_Set(kActorGuzza, kSetFreeSlotI);
		Actor_Set_At_Waypoint(kActorGuzza, 41, 0);
		if (Actor_Query_Which_Set_In(kActorSadik) == kSetUG18) {
			Actor_Put_In_Set(kActorSadik, kSetFreeSlotA);
			Actor_Set_At_Waypoint(kActorSadik, 33, 0);
		}
	}
	if ( Game_Flag_Query(670)
	 && !Game_Flag_Query(671)
	 &&  Global_Variable_Query(kVariableChapter) == 4
	) {
		Item_Add_To_World(91, 987, 89, -55.21f, 0.0f, -302.17f, 0, 12, 12, false, true, false, true);
	}
}

bool SceneScriptUG18::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptUG18::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptUG18::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptUG18::ClickedOnItem(int itemId, bool combatMode) {
	if (itemId == 91) {
		if (combatMode) {
			Item_Remove_From_World(91);
		} else if (!Loop_Actor_Walk_To_Item(kActorMcCoy, 91, 12, true, false)) {
			Item_Pickup_Spin_Effect(987, 368, 243);
			Item_Remove_From_World(itemId);
			Game_Flag_Set(703);
			Actor_Clue_Acquire(kActorMcCoy, kClueBriefcase, true, kActorGuzza);
		}
	}
	return false;
}

bool SceneScriptUG18::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -684.71f, 0.0f, 171.59f, 0, true, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagUG18toUG13);
			Set_Enter(kSetUG13, kSceneUG13);
		}
		return true;
	}
	return false;
}

bool SceneScriptUG18::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptUG18::SceneFrameAdvanced(int frame) {
}

void SceneScriptUG18::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
	if (actorId == kActorGuzza) {
		if (newGoal == 303) {
			Game_Flag_Set(607);
			ADQ_Flush();
			Actor_Modify_Friendliness_To_Other(kActorClovis, kActorMcCoy, 7);
			Actor_Modify_Friendliness_To_Other(kActorSadik, kActorMcCoy, 10);
			Player_Loses_Control();
			Actor_Face_Actor(kActorGuzza, kActorMcCoy, true);
			ADQ_Add(kActorGuzza, 1220, 58);
			Scene_Exits_Enable();
			Actor_Set_Goal_Number(kActorGuzza, 305);
		} else if (newGoal == 304) {
			ADQ_Flush();
			Actor_Modify_Friendliness_To_Other(kActorClovis, kActorMcCoy, 7);
			Actor_Modify_Friendliness_To_Other(kActorSadik, kActorMcCoy, 10);
			Player_Loses_Control();
			Actor_Face_Actor(kActorGuzza, kActorMcCoy, true);
			ADQ_Add(kActorGuzza, 1220, 58);
			Scene_Exits_Enable();
			Actor_Set_Goal_Number(kActorGuzza, 306);
		}
		return;
	}

	if (actorId == kActorSadik) {
		if (newGoal == 302) {
			if (Actor_Query_Friendliness_To_Other(kActorClovis, kActorMcCoy) > 55
			 && Game_Flag_Query(607)
			) {
				sub_403588();
			} else {
				Actor_Set_Goal_Number(kActorSadik, 307);
				Actor_Set_Goal_Number(kActorClovis, 310);
			}
		} else if (newGoal == 304) {
			Actor_Modify_Friendliness_To_Other(kActorClovis, kActorMcCoy, -3);
			ADQ_Add(kActorSadik, 380, -1);
			Actor_Set_Goal_Number(kActorSadik, 306);
		} else if (newGoal == 305) {
			Actor_Change_Animation_Mode(kActorSadik, 6);
			Sound_Play(12, 100, 0, 0, 50);
			Actor_Force_Stop_Walking(kActorMcCoy);
			Actor_Change_Animation_Mode(kActorMcCoy, 48);
			Player_Loses_Control();
			Actor_Retired_Here(kActorMcCoy, 6, 6, true, kActorSadik);
		}
	}
}

void SceneScriptUG18::PlayerWalkedIn() {
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, -488.71f, 0.0f, 123.59f, 0, false, false, 0);

	if ( Game_Flag_Query(670)
	 && !Game_Flag_Query(671)
	 &&  Actor_Query_Is_In_Current_Set(kActorGuzza)
	) {
		Scene_Exits_Disable();
		sub_402734();
		sub_403278();
		Game_Flag_Set(671);
	}
}

void SceneScriptUG18::PlayerWalkedOut() {
}

void SceneScriptUG18::DialogueQueueFlushed(int a1) {
	int v0 = Actor_Query_Goal_Number(kActorGuzza);
	if (v0 == 301) {
		Actor_Set_Goal_Number(kActorGuzza, 302);
		Actor_Change_Animation_Mode(kActorSadik, 6);
		Sound_Play(14, 100, 0, 0, 50);
		Actor_Change_Animation_Mode(kActorGuzza, 22);
		ADQ_Add(kActorClovis, 630, 13);
		Actor_Set_Goal_Number(kActorClovis, 301);
	} else if (v0 == 305) {
		Actor_Change_Animation_Mode(kActorMcCoy, 6);
		Sound_Play(13, 100, 0, 0, 50);
		Actor_Change_Animation_Mode(kActorGuzza, 22);
		Delay(900);
		Actor_Change_Animation_Mode(kActorMcCoy, 6);
		Sound_Play(14, 100, 0, 0, 50);
		Actor_Change_Animation_Mode(kActorGuzza, 22);
		Delay(1100);
		Actor_Change_Animation_Mode(kActorMcCoy, 6);
		Sound_Play(12, 100, 0, 0, 50);
		Actor_Change_Animation_Mode(kActorGuzza, 22);
		Delay(900);
		Actor_Change_Animation_Mode(kActorMcCoy, 6);
		Sound_Play(14, 100, 0, 0, 50);
		Actor_Change_Animation_Mode(kActorGuzza, 61);
		Overlay_Play("UG18over", 1, 0, 1, 0);
		Actor_Set_Goal_Number(kActorGuzza, 307);
		Player_Gains_Control();
		ADQ_Add_Pause(2000);
		ADQ_Add(kActorSadik, 360, -1);
		ADQ_Add_Pause(2000);
		ADQ_Add(kActorClovis, 650, 14);
		ADQ_Add(kActorSadik, 370, 14);
		ADQ_Add(kActorClovis, 1320, 14);
		Actor_Set_Goal_Number(kActorClovis, 303);
	} else if (v0 == 306) {
		Actor_Change_Animation_Mode(kActorGuzza, 6);
		Sound_Play(13, 100, 0, 0, 50);
		Actor_Force_Stop_Walking(kActorMcCoy);
		Actor_Change_Animation_Mode(kActorMcCoy, 48);
		Player_Loses_Control();
		Actor_Retired_Here(kActorMcCoy, 6, 6, 1, kActorGuzza);
		Actor_Set_Goal_Number(kActorGuzza, 307);
	}

	int v1 = Actor_Query_Goal_Number(kActorClovis);
	if (v1 == 301) {
		Actor_Change_Animation_Mode(kActorSadik, 6);
		Sound_Play(14, 100, 0, 0, 50);
		Actor_Change_Animation_Mode(kActorGuzza, 22);
		ADQ_Add(kActorClovis, 640, 13);
		ADQ_Add(kActorGuzza, 1210, 13);
		Actor_Set_Goal_Number(kActorClovis, 302);
	} else if (v1 == 302) {
		Actor_Change_Animation_Mode(kActorSadik, 6);
		Sound_Play(14, 100, 0, 0, 50);
		Actor_Change_Animation_Mode(kActorGuzza, 61);
		ADQ_Add_Pause(2000);
		ADQ_Add(kActorClovis, 650, 14);
		ADQ_Add(kActorSadik, 370, 14);
		ADQ_Add(kActorClovis, 1320, 14);
		Actor_Set_Goal_Number(kActorGuzza, 390);
		Actor_Retired_Here(kActorGuzza, 72, 32, 1, kActorSadik);
		Actor_Set_Goal_Number(kActorClovis, 303);
		Scene_Exits_Enable();
	} else if (v1 == 303) {
		Actor_Set_Goal_Number(kActorSadik, 301);
	}
	if (Actor_Query_Goal_Number(kActorSadik) == 306) {
		Actor_Change_Animation_Mode(kActorSadik, 48);
		Actor_Set_Goal_Number(kActorSadik, 307);
		Actor_Set_Goal_Number(kActorClovis, 310);
	}
}

void SceneScriptUG18::sub_402734() {
	Actor_Face_Actor(kActorMcCoy, kActorGuzza, true);
	Actor_Says(kActorMcCoy, 5860, 9);
	Delay(500);
	Actor_Face_Actor(kActorGuzza, kActorMcCoy, true);
	Delay(500);
	Actor_Says(kActorGuzza, 790, 3);
	Actor_Says(kActorMcCoy, 5865, 12);
	Actor_Says(kActorGuzza, 800, 3);
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, -357.13f, 0.0f, -44.47f, 0, false, false, 0);
	Actor_Face_Actor(kActorMcCoy, kActorGuzza, true);
	Actor_Says(kActorMcCoy, 5870, 14);
	Actor_Face_Actor(kActorGuzza, kActorMcCoy, true);
	Actor_Start_Speech_Sample(kActorGuzza, 810);
	Loop_Actor_Walk_To_XYZ(kActorGuzza, -57.21f, 0.0f, -334.17f, 0, false, false, 0);
	Actor_Says(kActorMcCoy, 5875, 13);
	Actor_Says(kActorGuzza, 830, 3);
	Actor_Says(kActorGuzza, 840, 12);
	Actor_Says(kActorGuzza, 850, 14);
	Actor_Says(kActorGuzza, 860, 13);
	Actor_Says(kActorMcCoy, 5880, 15);
	Actor_Says(kActorMcCoy, 5885, 9);
	Actor_Says(kActorMcCoy, 5890, 13);
	Actor_Says(kActorGuzza, 870, 15);
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, -205.13f, 0.0f, -184.47f, 0, false, false, 0);
	Actor_Face_Actor(kActorMcCoy, kActorGuzza, true);
	Actor_Says(kActorMcCoy, 5900, 15);
	Actor_Says(kActorGuzza, 880, 13);
	Actor_Says(kActorMcCoy, 5905, 9);
	Actor_Says(kActorMcCoy, 5910, 12);
	Actor_Says(kActorMcCoy, 5915, 13);
	Actor_Says(kActorGuzza, 890, 16);
	Actor_Says(kActorMcCoy, 5920, 14);
	Loop_Actor_Walk_To_XYZ(kActorGuzza, -57.21f, 0.0f, -334.17f, 0, false, false, 0);
	Actor_Face_Actor(kActorGuzza, kActorMcCoy, true);
	Actor_Says(kActorGuzza, 900, 15);
	Actor_Says(kActorGuzza, 910, 12);
	Actor_Says(kActorGuzza, 920, 16);
	Actor_Says(kActorMcCoy, 5925, 14);
	Actor_Says(kActorGuzza, 940, 14);
	Actor_Says(kActorMcCoy, 5930, 18);
	Actor_Says(kActorGuzza, 950, 14);
	Actor_Says(kActorGuzza, 960, 13);
	Actor_Says(kActorGuzza, 970, 3);
	if (Game_Flag_Query(607)) {
		Actor_Modify_Friendliness_To_Other(kActorClovis, kActorMcCoy, 3);
		Actor_Modify_Friendliness_To_Other(kActorSadik, kActorMcCoy, 5);
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -117.13f, 0.0f, -284.47f, 0, false, false, 0);
		Actor_Face_Actor(kActorMcCoy, kActorGuzza, true);
		Actor_Says(kActorMcCoy, 5960, 9);
		Actor_Says(kActorMcCoy, 5965, 14);
		Actor_Says(kActorGuzza, 980, 15);
		Actor_Says(kActorGuzza, 990, 13);
		Actor_Says(kActorMcCoy, 5970, 14);
		Actor_Says(kActorGuzza, 1000, 3);
		Actor_Says(kActorMcCoy, 5975, 15);
	} else {
		sub_402DE8();
	}
}

void SceneScriptUG18::sub_402DE8() {
	if (Player_Query_Agenda() != kPlayerAgendaPolite) {
		if (Global_Variable_Query(kVariableAffectionTowards) > 1
		 || Player_Query_Agenda() == kPlayerAgendaSurly
		) {
			sub_403114();
		} else {
			sub_402F8C();
		}
	} else {
		Actor_Modify_Friendliness_To_Other(kActorClovis, kActorMcCoy, -1);
		Actor_Modify_Friendliness_To_Other(kActorSadik, kActorMcCoy, -1);
		Actor_Says(kActorMcCoy, 5935, 14);
		Actor_Says(kActorMcCoy, 5940, 18);
		Actor_Says(kActorGuzza, 1020, 13);
		Actor_Says(kActorGuzza, 1030, 14);
	}
}

void SceneScriptUG18::sub_402F8C() {
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, -117.13f, 0.0f, -284.47f, 0, false, false, 0);
	Actor_Face_Actor(kActorMcCoy, kActorGuzza, true);
	Actor_Says(kActorMcCoy, 5945, 12);
	Actor_Says(kActorGuzza, 1040, 15);
	Actor_Says(kActorMcCoy, 5980, 15);
	Actor_Says(kActorGuzza, 1050, 12);
	Actor_Says(kActorGuzza, 1060, 13);
	Actor_Says(kActorGuzza, 1070, 14);
	Actor_Says(kActorMcCoy, 5985, 18);
	Actor_Says(kActorGuzza, 1080, 3);
	Actor_Says(kActorGuzza, 1090, 14);
	Actor_Says(kActorGuzza, 1100, 13);
}

void SceneScriptUG18::sub_403114() {
	Actor_Modify_Friendliness_To_Other(kActorClovis, kActorMcCoy, 20);
	Actor_Modify_Friendliness_To_Other(kActorSadik, kActorMcCoy, 10);
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, -117.13f, 0.0f, -284.47f, 0, false, false, 0);
	Actor_Face_Actor(kActorMcCoy, kActorGuzza, true);
	Actor_Says(kActorMcCoy, 5950, 16);
	Actor_Says(kActorMcCoy, 5955, 14);
	Actor_Says(kActorGuzza, 1110, 13);
	Actor_Says(kActorGuzza, 1120, 15);
	Actor_Says(kActorMcCoy, 5990, 3);
	Actor_Says(kActorGuzza, 1130, 15);
	Actor_Says(kActorGuzza, 1140, 16);
}

void SceneScriptUG18::sub_403278() {
	ADQ_Flush();
	Actor_Start_Speech_Sample(kActorClovis, 590);
	Delay(500);
	Loop_Actor_Walk_To_XYZ(kActorGuzza, 126.79f, 0.0f, -362.17f, 0, false, false, 0);
	Actor_Face_Heading(kActorGuzza, 729, false);
	Actor_Set_Goal_Number(kActorGuzza, 301);
	ADQ_Add(kActorSadik, 350, 13);
	ADQ_Add_Pause(1500);
	ADQ_Add(kActorGuzza, 1150, 58);
	ADQ_Add(kActorClovis, 600, 13);
	ADQ_Add_Pause(1000);
	ADQ_Add(kActorGuzza, 1160, 60);
	ADQ_Add_Pause(500);
	ADQ_Add(kActorGuzza, 1170, 59);
	ADQ_Add(kActorGuzza, 1180, 58);
	ADQ_Add(kActorClovis, 610, 13);
	ADQ_Add(kActorGuzza, 1190, 60);
	ADQ_Add(kActorClovis, 620, 13);
	ADQ_Add(kActorGuzza, 1200, 59);
}

void SceneScriptUG18::sub_403588() {
	Actor_Says(kActorClovis, 660, 13);
	Actor_Says(kActorMcCoy, 5995, 13);
	Actor_Says(kActorClovis, 670, 13);
	Actor_Says(kActorMcCoy, 6000, 13);
	Actor_Says_With_Pause(kActorClovis, 680, 2.0f, 13);
	Actor_Says(kActorClovis, 690, 13);
	Actor_Says(kActorClovis, 700, 13);
	Actor_Set_Goal_Number(kActorSadik, 310);
	Actor_Set_Goal_Number(kActorClovis, 310);
}

} // End of namespace BladeRunner
