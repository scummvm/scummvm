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

void SceneScriptHF06::InitializeScene() {
	Setup_Scene_Information(150.0f, 349.93f, 502.0f, 229);
	Game_Flag_Reset(529);
	Scene_Exit_Add_2D_Exit(0, 195, 197, 271, 237, 2);
	Ambient_Sounds_Add_Looping_Sound(54, 50, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(99, 40, -100, 1);
	Ambient_Sounds_Add_Looping_Sound(100, 40, 100, 1);
	Ambient_Sounds_Add_Sound(68, 10, 100, 25, 50, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(69, 10, 100, 25, 50, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(375, 10, 70, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(376, 10, 70, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(377, 10, 70, 50, 100, 0, 0, -101, -101, 0, 0);
	if (Game_Flag_Query(559)) {
		Scene_Loop_Set_Default(3);
		sub_4023E0();
	} else {
		Scene_Loop_Set_Default(0);
	}
}

void SceneScriptHF06::SceneLoaded() {
	Unobstacle_Object("BOX22", true);
	Unobstacle_Object("BOX34", true);
	Clickable_Object("BOX19");
	Clickable_Object("BOX21");
	Clickable_Object("BOX23");
	Clickable_Object("HOOD BOX");
	Clickable_Object("BOX28");
	Clickable_Object("BOX29");
	Clickable_Object("BOX30");
}

bool SceneScriptHF06::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptHF06::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("BOX28", objectName) || Object_Query_Click("BOX29", objectName) || Object_Query_Click("BOX30", objectName) || Object_Query_Click("HOOD BOX", objectName)) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 14.33f, 367.93f, 399.0f, 0, 1, false, 0)) {
			Actor_Face_Heading(kActorMcCoy, 486, true);
			if (Actor_Query_In_Set(kActorDektora, kSetHF06) && Actor_Query_Goal_Number(kActorDektora) != 599) {
				Actor_Face_Actor(kActorDektora, kActorMcCoy, true);
				Actor_Says(kActorDektora, 210, 12);
				Actor_Says(kActorMcCoy, 2125, 12);
			} else if (Actor_Query_In_Set(kActorLucy, kSetHF06) && Actor_Query_Goal_Number(kActorLucy) != 599) {
				Actor_Face_Actor(kActorLucy, kActorMcCoy, true);
				Actor_Says(kActorLucy, 490, 18);
				Actor_Says(kActorMcCoy, 2125, 12);
			} else {
				Actor_Says(kActorMcCoy, 8635, 12);
			}
		}
		return false;
	}
	if (Object_Query_Click("BOX19", objectName) || Object_Query_Click("BOX21", objectName)) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 290.0f, 367.93f, 318.0f, 0, 1, false, 0)) {
			Actor_Face_Heading(kActorMcCoy, 85, true);
			Actor_Says(kActorMcCoy, 8522, 0);
		}
		return false;
	}
	if (Object_Query_Click("BOX13", objectName)) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 63.0f, 367.93f, 120.0f, 0, 1, false, 0)) {
			Actor_Face_Heading(kActorMcCoy, 568, true);
			Actor_Says(kActorMcCoy, 8522, 0);
		}
		return false;
	}
	return false;
}

bool SceneScriptHF06::ClickedOnActor(int actorId) {
	if (actorId == 6 && Actor_Query_Goal_Number(kActorLucy) != 599) {
		Actor_Face_Actor(kActorLucy, kActorMcCoy, true);
		Actor_Face_Actor(kActorMcCoy, kActorLucy, true);
		if (Game_Flag_Query(559)) {
			Actor_Says(kActorLucy, 390, 18);
			Actor_Says(kActorMcCoy, 2115, 17);
		}
	} else if (actorId == 3 && Actor_Query_Goal_Number(kActorDektora) != 599) {
		Actor_Face_Actor(kActorDektora, kActorMcCoy, true);
		Actor_Face_Actor(kActorMcCoy, kActorDektora, true);
		if (Game_Flag_Query(559)) {
			Actor_Says(kActorDektora, 100, 3);
			Actor_Says(kActorMcCoy, 2115, 17);
		}
	}
	return false;
}

bool SceneScriptHF06::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptHF06::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 170.0f, 367.93f, 497.0f, 0, 1, false, 0)) {
			Actor_Face_Heading(kActorMcCoy, 730, false);
			Loop_Actor_Travel_Stairs(kActorMcCoy, 2, 0, kAnimationModeIdle);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(530);
			Set_Enter(kSetHF05, kSceneHF05);
		}
		return true;
	}
	return false;
}

bool SceneScriptHF06::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptHF06::SceneFrameAdvanced(int frame) {
}

void SceneScriptHF06::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
	if (actorId == kActorSteele && oldGoal != 599 && newGoal == 599) {
		Loop_Actor_Walk_To_Actor(kActorMcCoy, kActorSteele, 24, 0, false);
		Actor_Says(kActorSteele, 250, -1);
		Actor_Says(kActorMcCoy, 2120, 4);
		Actor_Says(kActorSteele, 260, -1);
		Actor_Says(kActorSteele, 270, -1);
		int otherActorId = -1;
		if (Actor_Query_In_Set(kActorDektora, kSetHF06) && Actor_Query_Goal_Number(kActorDektora) == 599) {
			otherActorId = kActorDektora;
		} else if (Actor_Query_In_Set(kActorLucy, kSetHF06) && Actor_Query_Goal_Number(kActorLucy) == 599) {
			otherActorId = kActorLucy;
		}
		if (otherActorId != -1) {
			Music_Play(21, 35, 0, 3, -1, 0, 0);
			Player_Set_Combat_Mode(false);
			Delay(1000);
			Actor_Voice_Over(990, kActorVoiceOver);
			Actor_Voice_Over(1000, kActorVoiceOver);
			Actor_Voice_Over(1010, kActorVoiceOver);
			Loop_Actor_Walk_To_Actor(kActorMcCoy, otherActorId, 24, 0, false);
			Item_Pickup_Spin_Effect(932, 355, 200);
			Actor_Voice_Over(1020, kActorVoiceOver);
			Actor_Voice_Over(1030, kActorVoiceOver);
			Actor_Voice_Over(1040, kActorVoiceOver);
			Actor_Voice_Over(1050, kActorVoiceOver);
			Actor_Clue_Acquire(kActorMcCoy, kClueBomb, true, -1);
		}
		if (Actor_Query_In_Set(kActorDektora, kSetHF06)) {
			Actor_Set_Targetable(kActorDektora, false);
		}
		if (Actor_Query_In_Set(kActorLucy, kSetHF06)) {
			Actor_Set_Targetable(kActorLucy, false);
		}
		Scene_Exits_Enable();
	}
}

void SceneScriptHF06::PlayerWalkedIn() {
	if (Game_Flag_Query(662)) {
		int actorId = -1;
		if (Global_Variable_Query(kVariableAffectionTowards) == 3 && Actor_Query_Goal_Number(kActorLucy) != 599) {
			actorId = kActorLucy;
		} else if (Global_Variable_Query(kVariableAffectionTowards) == 2 && Actor_Query_Goal_Number(kActorDektora) != 599) {
			actorId = kActorDektora;
		} 

		if (actorId != -1) {
			Actor_Put_In_Set(actorId, kSetHF06);
			if (Game_Flag_Query(559)) {
				Actor_Set_At_XYZ(actorId, 173.67f, 367.93f, 446.04f, 229);
				Async_Actor_Walk_To_XYZ(actorId, 173.67f, 367.93f, 394.04f, 0, false);
			} else {
				Actor_Set_At_XYZ(actorId, 97.67f, 367.93f, 534.04f, 725);
				Async_Actor_Walk_To_XYZ(actorId, 24.2f, 367.93f, 537.71f, 0, false);
			}
		}
	}
	Footstep_Sound_Override_On(3);
	Loop_Actor_Travel_Stairs(kActorMcCoy, 2, 1, kAnimationModeIdle);
	Footstep_Sound_Override_Off();
	if (Game_Flag_Query(662) && !Game_Flag_Query(559)) {
		sub_401EF4();
	}
}

void SceneScriptHF06::PlayerWalkedOut() {
	Music_Stop(2);
}

void SceneScriptHF06::DialogueQueueFlushed(int a1) {
}

void SceneScriptHF06::sub_401EF4() {
	int actorId;

	if (Actor_Query_In_Set(kActorDektora, kSetHF06)) {
		actorId = kActorDektora;
	} else if (Actor_Query_In_Set(kActorLucy, kSetHF06)) {
		actorId = kActorLucy;
	} else {
		return;
	}
	Actor_Set_Targetable(actorId, true);
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, 14.33f, 367.93f, 399.0f, 0, 0, true, 0);
	Actor_Face_Heading(kActorMcCoy, 486, true);
	sub_4023E0();
	Actor_Put_In_Set(kActorSteele, kSetHF06);
	Actor_Set_At_XYZ(kActorSteele, 92.0f, 367.93f, 19.0f, 0);
	Actor_Set_Targetable(kActorSteele, true);
	Actor_Face_Actor(actorId, kActorSteele, true);
	if (actorId == 3) {
		Actor_Says(kActorDektora, 90, 13);
	} else if (actorId == 6) {
		Actor_Says(kActorLucy, 380, 13);
	}
	Actor_Says(kActorMcCoy, 6230, 0);
	Actor_Says(kActorSteele, 280, 58);
	Actor_Face_Actor(kActorMcCoy, kActorSteele, true);
	Player_Set_Combat_Mode(true);
	Actor_Change_Animation_Mode(kActorMcCoy, 5);
	Actor_Change_Animation_Mode(kActorSteele, 7);
	Loop_Actor_Walk_To_XYZ(kActorSteele, 92.0f, 367.93f, 107.0f, 0, 0, false, 0);
	Actor_Face_Actor(kActorSteele, kActorMcCoy, true);
	Actor_Change_Animation_Mode(kActorSteele, kAnimationModeCombatIdle);
	Actor_Says(kActorSteele, 290, 58);
	Actor_Says(kActorMcCoy, 2130, -1);
	Actor_Says(kActorSteele, 300, 59);
	Actor_Says(kActorMcCoy, 2135, -1);
	Actor_Says(kActorSteele, 310, 60);
	Actor_Says(kActorMcCoy, 2140, -1);
	Actor_Says(kActorSteele, 320, 59);
	Actor_Says(kActorMcCoy, 2145, -1);
	Actor_Says(kActorSteele, 330, 58);
	Actor_Says(kActorSteele, 340, 58);
	Actor_Says(kActorSteele, 350, 58);
	Actor_Change_Animation_Mode(kActorSteele, kAnimationModeCombatIdle);
	Game_Flag_Set(644);
	Actor_Set_Goal_Number(kActorSteele, 402);
	Actor_Face_Actor(kActorSteele, actorId, true);
	Actor_Change_Animation_Mode(kActorSteele, 6);
	Delay(500);
	Scene_Loop_Set_Default(3);
	Scene_Loop_Start_Special(kSceneLoopModeOnce, 2, true);
	Sound_Play(562, 50, 0, 0, 50);
	Game_Flag_Set(559);
	Scene_Exits_Disable();
	Non_Player_Actor_Combat_Mode_On(kActorSteele, kActorCombatStateUncover, true, actorId, 15, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, 0, 0, 100, 10, 300, false);
}

void SceneScriptHF06::sub_4023E0() {
	Ambient_Sounds_Add_Sound(87, 20, 80, 20, 100, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Speech_Sound(23, 250, 5, 70, 7, 10, -50, 50, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(23, 330, 5, 70, 7, 10, -50, 50, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(23, 340, 5, 90, 7, 10, -50, 50, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(23, 360, 5, 70, 7, 10, -50, 50, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(24, 380, 5, 70, 7, 10, -50, 50, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(24, 510, 5, 70, 7, 10, -50, 50, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(38, 80, 5, 70, 7, 10, -50, 50, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(38, 160, 5, 70, 7, 10, -50, 50, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(38, 280, 5, 70, 7, 10, -50, 50, -101, -101, 1, 1);
}

} // End of namespace BladeRunner
