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

void SceneScriptHF03::InitializeScene() {
	if (Game_Flag_Query(kFlagHF02toHF03)) {
		Setup_Scene_Information( 479.0f, 47.76f,  -496.0f, 600);
	} else {
		Setup_Scene_Information(185.62f, 47.76f, -867.42f, 300);
	}

	Scene_Exit_Add_2D_Exit(0,   0,   0,  30, 479, 3);
	Scene_Exit_Add_2D_Exit(1, 589,   0, 639, 479, 1);
	Scene_Exit_Add_2D_Exit(2, 323, 110, 380, 166, 0);

	Ambient_Sounds_Add_Looping_Sound(kSfxARCBED1, 50, 0, 1);
	Ambient_Sounds_Add_Sound(kSfx67_0480R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0540R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0560R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0870R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0900R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0940R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_1070R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_1080R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_1160R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
}

void SceneScriptHF03::SceneLoaded() {
	Obstacle_Object("MAIN", true);
	Unclickable_Object("MAIN");
}

bool SceneScriptHF03::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptHF03::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click(objectName, "MAIN")) {
		Actor_Says(kActorMcCoy, Random_Query(0, 3) + 8525, 18);
	}
	return false;
}

void SceneScriptHF03::dialogueWithLucy() {
	Dialogue_Menu_Clear_List();
	DM_Add_To_List_Never_Repeat_Once_Selected(840, -1, 3, 8); // VOIGT-KAMPFF
	DM_Add_To_List_Never_Repeat_Once_Selected(850, 6, 5, 2); // FATHER
	DM_Add_To_List_Never_Repeat_Once_Selected(860, 8, -1, -1); // CRYSTAL
	DM_Add_To_List_Never_Repeat_Once_Selected(870, 2, 8, 6); // RUNCITER
	Dialogue_Menu_Add_DONE_To_List(880); // DONE

	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();

	switch (answer) {
	case 840: // VOIGT-KAMPFF
		Actor_Says(kActorMcCoy, 1630, 15);
		if (Global_Variable_Query(kVariableHollowayArrest) == 3) {
			Actor_Set_Goal_Number(kActorLucy, kGoalLucyHF03RunAwayWithHelp1);
		} else if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			Actor_Set_Goal_Number(kActorLucy, kGoalLucyHF03RunToHF041);
		} else {
			Actor_Set_Goal_Number(kActorLucy, kGoalLucyHF03RunOutPhase1);
			Game_Flag_Set(kFlagLucyRanAway);
		}
		break;

	case 850: // FATHER
		Actor_Says(kActorMcCoy, 1635, 15);
		Actor_Says(kActorLucy, 200, 13);
		Actor_Modify_Friendliness_To_Other(kActorLucy, kActorMcCoy, 3);
		break;

	case 860: // CRYSTAL
		Actor_Says(kActorMcCoy, 1640, 12);
		if (Global_Variable_Query(kVariableHollowayArrest) == 3) {
			Actor_Set_Goal_Number(kActorLucy, kGoalLucyHF03RunAwayWithHelp1);
		} else {
			Actor_Says(kActorLucy, 210, 13);
			Actor_Says(kActorMcCoy, 1655, 15);
			Actor_Modify_Friendliness_To_Other(kActorLucy, kActorMcCoy, Random_Query(9, 10));
			if (Actor_Query_Friendliness_To_Other(kActorLucy, kActorMcCoy) > 59
			 && Global_Variable_Query(kVariableAffectionTowards) == kAffectionTowardsNone
			) {
				Global_Variable_Set(kVariableAffectionTowards, kAffectionTowardsLucy);
				Actor_Says(kActorLucy, 940, 14);
				Actor_Says(kActorMcCoy, 6780, 11);
				Actor_Says(kActorLucy, 950, 12);
				Actor_Says(kActorLucy, 960, 13);
				Actor_Says(kActorMcCoy, 6785, 15);
				Actor_Says(kActorLucy, 970, 16);
				Actor_Says(kActorLucy, 980, 17);
				if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
					Actor_Says(kActorLucy, 990, 17);
				}
				Actor_Says(kActorMcCoy, 6790, 15);
				Actor_Says(kActorLucy, 1000, 13);
				Actor_Says(kActorLucy, 1010, 17);
				Actor_Says(kActorLucy, 1020, 18);
				Actor_Says(kActorMcCoy, 6795, 14);
				Actor_Says(kActorLucy, 1030, 17);
				Actor_Says(kActorMcCoy, 6800, 14);
			}
			Actor_Says(kActorLucy, 220, 13);
			Actor_Says(kActorMcCoy, 1660, 15);
			Actor_Says(kActorLucy, 230, 14);
			Actor_Clue_Acquire(kActorLucy, kClueMcCoyHelpedLucy, true, kActorMcCoy);

			if (Game_Flag_Query(kFlagLucyIsReplicant)) {
				Actor_Set_Goal_Number(kActorLucy, kGoalLucyHF03RunToHF041);
			} else {
				Actor_Set_Goal_Number(kActorLucy, kGoalLucyHF03RunOutPhase1);
			}
		}
		break;

	case 870: // RUNCITER
		Actor_Says(kActorMcCoy, 1645, 18);
		Actor_Says(kActorLucy, 240, 14);
		Actor_Says(kActorLucy, 250, 12);
		Actor_Says(kActorLucy, 260, 13);
		Actor_Says(kActorLucy, 270, 19);
		Actor_Says(kActorMcCoy, 1665, 18);
		Actor_Says(kActorLucy, 280, 13);
		Actor_Says(kActorMcCoy, 1670, 12);
		Actor_Says(kActorLucy, 290, 14);
		Actor_Says(kActorLucy, 300, 16);
		Actor_Says(kActorMcCoy, 1675, 12);
		Actor_Says(kActorLucy, 310, 13);
		Actor_Clue_Acquire(kActorMcCoy, kClueLucyInterview, false, kActorLucy);
		break;

	case 880: // DONE
		Actor_Says(kActorMcCoy, 1650, 14);
		break;
	}
}

bool SceneScriptHF03::ClickedOnActor(int actorId) {
	if (actorId == kActorLucy
	 && Actor_Query_Goal_Number(kActorLucy) == kGoalLucyGoToHF03
	 ) {
		if (Game_Flag_Query(kFlagLucyIsReplicant)
		 ? !Loop_Actor_Walk_To_Waypoint(kActorMcCoy, 377, 0, true, false)
		 : !Loop_Actor_Walk_To_Waypoint(kActorMcCoy, 378, 0, true, false)
		) {
			Actor_Face_Actor(kActorMcCoy, kActorLucy, true);
			if (!Game_Flag_Query(kFlagHF03LucyTalk)) {
				Game_Flag_Set(kFlagHF03LucyTalk);
				if (Game_Flag_Query(kFlagLucyIsReplicant)) {
					Actor_Says(kActorMcCoy, 1605, 15);
					Actor_Says(kActorLucy, 100, 12);
					Actor_Says(kActorMcCoy, 1610, 14);
				} else {
					Actor_Says(kActorMcCoy, 1615, 16);
					Actor_Says(kActorLucy, 110, 13);
				}
				Actor_Says(kActorLucy, 120, 13);
				Actor_Says(kActorMcCoy, 1620, 14);
				Actor_Says(kActorLucy, 130, 17);
				Actor_Says(kActorMcCoy, 1625, 15);
				if (Game_Flag_Query(kFlagLucyIsReplicant)) {
					Actor_Says(kActorLucy, 140, 12);
					Actor_Says(kActorLucy, 150, 13);
					Actor_Says(kActorLucy, 160, 15);
				} else {
					Actor_Says(kActorLucy, 170, 12);
					Actor_Says(kActorLucy, 180, 13);
					Actor_Says(kActorLucy, 190, 15);
				}
			}

			dialogueWithLucy();
		}
	}
	return false;
}

bool SceneScriptHF03::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptHF03::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 179.0f, 46.76f, -824.0f, 0, true, false, false)) {
			Game_Flag_Set(kFlagHF03toHF01);
			Set_Enter(kSetHF01, kSceneHF01);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 479.0f, 47.76f, -524.0f, 0, true, false, false)) {
			Game_Flag_Set(kFlagHF03toHF02);
			Set_Enter(kSetHF02, kSceneHF02);
		}
		return true;
	}

	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 942.0f, 47.76f, -847.0f, 0, true, false, false)) {
			Game_Flag_Set(kFlagHF03toHF04);
			Set_Enter(kSetHF04, kSceneHF04);
		}
		return true;
	}
	return false;
}

bool SceneScriptHF03::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptHF03::SceneFrameAdvanced(int frame) {
}

void SceneScriptHF03::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptHF03::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagHF02toHF03)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 479.0f, 47.76f, -524.0f, 0, false, false, false);
		Game_Flag_Reset(kFlagHF02toHF03);
	} else {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 179.0f, 47.76f, -824.0f, 0, false, false, false);
		Game_Flag_Reset(kFlagHF01toHF03);
	}

	if (Actor_Query_Goal_Number(kActorLucy) == kGoalLucyReturnToHF03) {
		Actor_Set_Goal_Number(kActorLucy, kGoalLucyHF03RunToHF041);
		Actor_Says(kActorSteele, 210, 13);
		Actor_Face_Actor(kActorMcCoy, kActorSteele, true);
		Actor_Says(kActorMcCoy, 1680, 15);
		Actor_Says(kActorSteele, 220, 14);
		Actor_Says(kActorMcCoy, 1685, 13);
		Actor_Says(kActorSteele, 230, 16);
		Actor_Says(kActorMcCoy, 1690, 12);
		Actor_Says(kActorSteele, 240, 13);
		Actor_Set_Goal_Number(kActorSteele, kGoalSteeleHF03McCoyChasingLucy);
	}
}

void SceneScriptHF03::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void SceneScriptHF03::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
