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

void SceneScriptUG16::InitializeScene() {
	if (Game_Flag_Query(kFlagDR06toUG16)) {
		Setup_Scene_Information(-270.76f, -34.88f, -504.02f, 404);
		Game_Flag_Reset(kFlagDR06toUG16);
	} else if (Game_Flag_Query(kFlagUG15toUG16a)) {
		Setup_Scene_Information(-322.0f,   -34.0f,  -404.0f, 345);
		Game_Flag_Reset(kFlagUG15toUG16a);
	} else {
		Setup_Scene_Information(-318.0f,   -34.0f,  -216.0f, 340);
		Game_Flag_Reset(kFlagUG15toUG16b);
	}

	Scene_Exit_Add_2D_Exit(0, 242, 169, 282, 262, 3);
	Scene_Exit_Add_2D_Exit(1, 375, 166, 407, 251, 3);
	Scene_Exit_Add_2D_Exit(2, 461, 148, 523, 248, 0);

	Ambient_Sounds_Add_Looping_Sound(kSfxELECLAB1, 33, 81, 0);
	Ambient_Sounds_Add_Looping_Sound(kSfxUGBED1,   40,  0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxUGBED2,   40,  0, 1);

	if (Game_Flag_Query(kFlagUG16ComputerOff)) {
		Scene_Loop_Set_Default(5);
	} else {
		Scene_Loop_Set_Default(0);
	}
}

void SceneScriptUG16::SceneLoaded() {
	Obstacle_Object("BED", true);
	Obstacle_Object("QUADPATCH07", true);
	Obstacle_Object("QUADPATCH05", true);
	Obstacle_Object("SCREEN 01", true);
	Obstacle_Object("BOX49", true);
	Obstacle_Object("CYLINDER07", true);
	Unobstacle_Object("SEAT 1", true);
	Unobstacle_Object("SEAT 2", true);
	Unclickable_Object("BED");
	Unclickable_Object("QUADPATCH07");
	Clickable_Object("QUADPATCH05");
	Clickable_Object("SCREEN 01");
	Unclickable_Object("BOX49");
	Unclickable_Object("CYLINDER07");
	Unobstacle_Object("BOX67", true);
	Footstep_Sounds_Set(0, 3);
	Footstep_Sounds_Set(1, 2);
	Footstep_Sounds_Set(2, 3);
	Footstep_Sounds_Set(6, 3);
}

bool SceneScriptUG16::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptUG16::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("QUADPATCH05", objectName)) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 194.0f, -35.0f, 160.8f, 0, true, false, false)) {
			Actor_Face_Heading(kActorMcCoy, 870, false);
			if (!Game_Flag_Query(kFlagUG16FolderFound)
			 &&  Game_Flag_Query(kFlagLutherLanceAreDead)
			) {
				Game_Flag_Set(kFlagUG16FolderFound);
				Delay(1000);
				Actor_Voice_Over(3480, kActorVoiceOver);
				Actor_Change_Animation_Mode(kActorMcCoy, 38);
				Sound_Play(kSfxDRAWER1, 100, 0, 0, 50);
				Delay(1000);
				Item_Pickup_Spin_Effect(kModelAnimationFolder, 460, 287);
				Actor_Voice_Over(2740, kActorVoiceOver);
				Actor_Voice_Over(2750, kActorVoiceOver);
				Actor_Voice_Over(2760, kActorVoiceOver);
				Actor_Voice_Over(2770, kActorVoiceOver);
				Actor_Clue_Acquire(kActorMcCoy, kClueFolder, true, -1);
			} else {
				Actor_Says(kActorMcCoy, 8523, 12);
				Actor_Says(kActorMcCoy, 8635, 12);
			}
			return true;
		}
	}

	if (Object_Query_Click("SCREEN 01", objectName)) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 194.0f, -35.0f, 160.8f, 0, true, false, false)) {

			Actor_Face_Heading(kActorMcCoy, 870, false);

			if (( Game_Flag_Query(kFlagLutherLanceAreDead)
			  || !Actor_Query_Is_In_Current_Set(kActorLuther)
			 )
			 && !Actor_Clue_Query(kActorMcCoy, kClueDNALutherLance)
			 && !Game_Flag_Query(kFlagUG16ComputerOff)
			) {
				Delay(2000);
				Actor_Face_Heading(kActorMcCoy, 1016, false);
				Delay(2000);
				Actor_Says(kActorMcCoy, 5725, 14);
				Delay(1000);
				Item_Pickup_Spin_Effect(kModelAnimationDNADataDisc, 418, 305);
				Actor_Clue_Acquire(kActorMcCoy, kClueDNALutherLance, true, -1);
				return true;
			}

			Actor_Says(kActorMcCoy, 8525, 12);
			Actor_Says(kActorMcCoy, 8526, 12);
		}
	}
	return false;
}

bool SceneScriptUG16::ClickedOnActor(int actorId) {
	if (Actor_Query_Goal_Number(kActorLuther) < 490) { // Luther & Lance are alive
		dialogueWithLuther();
		return true;
	}
	return false;
}

bool SceneScriptUG16::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptUG16::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -322.0f, -34.0f, -216.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagUG16toUG15b);
			Set_Enter(kSetUG15, kSceneUG15);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -322.0f, -34.0f, -404.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagUG16toUG15a);
			Set_Enter(kSetUG15, kSceneUG15);
		}
		return true;
	}

	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -316.78f, -34.88f, -533.27f, 0, true, false, false)) {
			Actor_Face_Heading(kActorMcCoy, 0, false);
			Loop_Actor_Travel_Stairs(kActorMcCoy, 13, true, kAnimationModeIdle);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagUG16toDR06);
			Set_Enter(kSetDR06, kSceneDR06);
		}
		return true;
	}
	return false;
}

bool SceneScriptUG16::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptUG16::SceneFrameAdvanced(int frame) {
	if (frame == 132) {
		Ambient_Sounds_Remove_Looping_Sound(kSfxELECLAB1, 1);
	}
}

void SceneScriptUG16::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptUG16::PlayerWalkedIn() {
	Game_Flag_Set(kFlagDR06UnlockedToUG16);

	if (!Game_Flag_Query(kFlagLutherLanceAreDead)) {
		Actor_Set_Goal_Number(kActorLuther, kGoalLutherStop);
	}

	if (!Game_Flag_Query(kFlagUG16LutherLanceTalk1)
	 &&  Actor_Query_Is_In_Current_Set(kActorLuther)
	) {
		Player_Loses_Control();
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 120.29f, -35.67f, 214.8f, 310, false, false, false);
		Actor_Face_Actor(kActorMcCoy, kActorLuther, true);
		Actor_Says(kActorLuther, 0, 6);
		Actor_Says(kActorLuther, 30, 13);
		Actor_Change_Animation_Mode(kActorLuther, 17);
		Actor_Says(kActorLance, 0, 17);
		Actor_Says(kActorMcCoy, 5710, 14);
		Actor_Says(kActorLuther, 40, 13);
		Actor_Says(kActorLuther, 50, 15);
		Actor_Says(kActorLance, 20, 12);
		Actor_Says(kActorLuther, 60, 23);
		Actor_Says(kActorMcCoy, 5715, 14);
		Actor_Says(kActorLance, 30, 16);
		Actor_Says(kActorLuther, 70, 6);
		Player_Gains_Control();
		Game_Flag_Set(kFlagUG16LutherLanceTalk1);
	}
}

void SceneScriptUG16::PlayerWalkedOut() {
	if (!Game_Flag_Query(kFlagLutherLanceAreDead)) {
		Actor_Set_Goal_Number(kActorLuther, kGoalLutherMoveAround);
		//return true;
	}
	//return false;
}

void SceneScriptUG16::DialogueQueueFlushed(int a1) {
}

void SceneScriptUG16::dialogueWithLuther() {
	Dialogue_Menu_Clear_List();
	DM_Add_To_List_Never_Repeat_Once_Selected(1400, 5, 6, 2); // REPLICANTS
	DM_Add_To_List_Never_Repeat_Once_Selected(1410, 5, 4, 8); // WORK
	if (Game_Flag_Query(kFlagUG16LutherLanceTalkReplicants)
	 || Game_Flag_Query(kFlagUG16LutherLanceTalkHumans)
	) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1420, 6, 4, 5); // LIFESPAN
		DM_Add_To_List_Never_Repeat_Once_Selected(1430, 6, 4, 5); // CLOVIS
		DM_Add_To_List_Never_Repeat_Once_Selected(1440, 6, 4, 5); // VOIGT-KAMPFF
	}
	if ( Global_Variable_Query(kVariableCorruptedGuzzaEvidence) > 1
	 && !Actor_Clue_Query(kActorMcCoy, kClueFolder)
	 ) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1450, 6, 4, 5); // GUZZA
	}
	if (Actor_Clue_Query(kActorMcCoy, kClueEnvelope)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1460, 6, 4, 5); // RUNCITER
	}
	if ( Actor_Clue_Query(kActorMcCoy, kClueDNATyrell)
	 && !Actor_Clue_Query(kActorMcCoy, kClueFolder)
	 &&  Game_Flag_Query(kFlagUG15LanceLuthorTrade)
	) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1470, 6, 4, 5); // TRADE
	}
	Dialogue_Menu_Add_DONE_To_List(1480); // DONE

	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();

	switch (answer) {
	case 1400: // REPLICANTS
		Actor_Says(kActorMcCoy, 5730, 13);
		Actor_Face_Actor(kActorMcCoy, kActorLuther, true);
		Actor_Says(kActorLuther, 100, 18);
		Actor_Says(kActorMcCoy, 5775, 13);
		Actor_Says(kActorLance, 70, 17);
		Actor_Says(kActorLuther, 110, 16);
		Actor_Says(kActorLance, 80, 6);
		Actor_Says(kActorMcCoy, 5780, 13);
		Actor_Says(kActorLuther, 120, 16);
		Actor_Says(kActorLance, 120, 13);
		Actor_Says(kActorMcCoy, 5785, 13);
		Actor_Says(kActorLuther, 130, 6);
		Actor_Says(kActorMcCoy, 5825, 13);
		Actor_Modify_Friendliness_To_Other(kActorLuther, kActorMcCoy, -5);
		if (Game_Flag_Query(kFlagLutherLanceIsReplicant)) {
			Actor_Says(kActorLuther, 140, 13);
			Actor_Says(kActorLuther, 150, 14);
			Actor_Says(kActorLuther, 160, 13);
			Actor_Says(kActorLance, 140, 16);
			Actor_Says(kActorMcCoy, 5790, 13);
			Actor_Says(kActorLuther, 170, 14);
			Game_Flag_Set(kFlagUG16LutherLanceTalkReplicants);
			Actor_Modify_Friendliness_To_Other(kActorLuther, kActorMcCoy, 5);
		} else {
			Actor_Says(kActorLuther, 180, 14);
			Actor_Says(kActorMcCoy, 5795, 13);
			Actor_Says(kActorLance, 150, 17);
			Actor_Says(kActorMcCoy, 5800, 13);
			Actor_Says(kActorLuther, 190, 15);
			Game_Flag_Set(kFlagUG16LutherLanceTalkHumans);
			Actor_Modify_Friendliness_To_Other(kActorLuther, kActorMcCoy, -10);
		}
		break;

	case 1410: // WORK
		Actor_Says(kActorMcCoy, 5735, 13);
		Actor_Face_Actor(kActorMcCoy, kActorLuther, true);
		Actor_Says(kActorLance, 160, 17);
		Actor_Says(kActorLuther, 200, 14);
		break;

	case 1420: // LIFESPAN
		Actor_Says(kActorMcCoy, 5740, 13);
		Actor_Face_Actor(kActorMcCoy, kActorLuther, true);
		Actor_Says(kActorLance, 180, 15);
		Actor_Says(kActorLuther, 220, 13);
		Actor_Says(kActorLance, 190, 17);
		Actor_Says(kActorMcCoy, 5805, 13);
		Actor_Says(kActorLuther, 230, 14);
		Actor_Says(kActorLuther, 240, 13);
		Actor_Says(kActorLance, 200, 17);
		Actor_Says(kActorLuther, 260, 13);
		Actor_Says(kActorLuther, 270, 15);
		Actor_Says(kActorLance, 210, 14);
		Actor_Says(kActorMcCoy, 5810, 13);
		Actor_Says(kActorLance, 220, 14);
		Actor_Says(kActorLance, 230, 17);
		Actor_Clue_Acquire(kActorMcCoy, kClueLutherLanceInterview, true, kActorLuther);
		break;

	case 1430: // CLOVIS
		Actor_Says(kActorMcCoy, 5745, 13);
		Actor_Face_Actor(kActorMcCoy, kActorLuther, true);
		Actor_Says(kActorLance, 240, 15);
		Actor_Says(kActorMcCoy, 5815, 13);
		Actor_Says(kActorLance, 250, 16);
		Actor_Says(kActorLuther, 290, 15);
		Actor_Says(kActorLance, 260, 15);
		break;

	case 1440: // VOIGT-KAMPFF
		Actor_Says(kActorMcCoy, 5750, 13);
		Actor_Face_Actor(kActorMcCoy, kActorLuther, true);
		Actor_Says(kActorLance, 280, 6);
		Actor_Says(kActorLuther, 300, 14);
		Actor_Says(kActorLuther, 310, 15);
		Actor_Modify_Friendliness_To_Other(kActorLuther, kActorMcCoy, -5);
		break;

	case 1450: // GUZZA
		Actor_Says(kActorMcCoy, 5755, 13);
		Actor_Face_Actor(kActorMcCoy, kActorLuther, true);
		Actor_Says(kActorLance, 290, 17);
		Actor_Says(kActorLuther, 320, 16);
		Actor_Says(kActorMcCoy, 5820, 13);
		Actor_Says(kActorLance, 300, 17);
		Actor_Says(kActorLuther, 330, 14);
		Actor_Says(kActorMcCoy, 5825, 13);
		Actor_Says(kActorLuther, 340, 13);
		Actor_Says(kActorLance, 310, 13);
		Actor_Says(kActorLuther, 350, 13);
		Actor_Says(kActorLuther, 360, 15);
		Actor_Says(kActorMcCoy, 5830, 13);
		Actor_Says(kActorLance, 320, 16);
		Actor_Says(kActorLance, 330, 15);
		Game_Flag_Set(kFlagUG15LanceLuthorTrade);
		break;

	case 1460: // RUNCITER
		Actor_Says(kActorMcCoy, 5760, 13);
		Actor_Face_Actor(kActorMcCoy, kActorLuther, true);
		Actor_Says(kActorLuther, 370, 15);
		Actor_Says(kActorLance, 340, 14);
		Actor_Says(kActorMcCoy, 5835, 13);
		Actor_Says(kActorLuther, 380, 15);
		Actor_Says(kActorLance, 370, 6);
		Actor_Says(kActorMcCoy, 5840, 13);
		Actor_Says(kActorLance, 380, 13);
		break;

	case 1470: // TRADE
		Actor_Says(kActorMcCoy, 5765, 13);
		Actor_Face_Actor(kActorMcCoy, kActorLuther, true);
		Actor_Says(kActorLance, 400, 15);
		Actor_Says(kActorMcCoy, 5845, 13);
		Actor_Says(kActorLuther, 390, 23);
		Actor_Says(kActorLance, 410, 14);
		Actor_Says(kActorLance, 420, 17);
#if BLADERUNNER_ORIGINAL_BUGS
		// This quote is repeated (also used in RUNCITER question
		// "That probably had something to do with you guys getting fired."
		// It makes little sense to be here.
		Actor_Says(kActorMcCoy, 5835, 13);
#endif // BLADERUNNER_ORIGINAL_BUGS
		Delay(1000);
		Item_Pickup_Spin_Effect(kModelAnimationFolder, 239, 454);
		Actor_Voice_Over(2740, kActorVoiceOver);
		Actor_Voice_Over(2750, kActorVoiceOver);
		Actor_Voice_Over(2760, kActorVoiceOver);
		Actor_Voice_Over(2770, kActorVoiceOver);
		Actor_Says(kActorMcCoy, 5850, 13);
		Actor_Says(kActorLuther, 400, 15);
		Actor_Says(kActorLance, 430, 6);
		Actor_Says(kActorMcCoy, 5855, 13);
		Actor_Says(kActorLuther, 410, 14);
		Game_Flag_Set(kFlagUG16FolderFound);
		Actor_Clue_Acquire(kActorMcCoy, kClueFolder, true, kActorLuther);
		break;

	case 1480: // DONE
		Actor_Says(kActorMcCoy, 4595, 14);
		break;
	}
}

} // End of namespace BladeRunner
