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

enum kRC02Exits {
	kRC02ExitRC01 = 0,
	kRC02ExitRC51 = 1
};

void SceneScriptRC02::InitializeScene() {
	if (Game_Flag_Query(kFlagRC01toRC02)) {
		Setup_Scene_Information(-103.0f, -1238.89f, 108603.04f, 1007);
	} else {
		Setup_Scene_Information(-20.2f, -1238.89f, 108100.73f, 539);
	}
	Scene_Exit_Add_2D_Exit(kRC02ExitRC01, 0, 460, 639, 479, 2);
	if (Game_Flag_Query(kFlagRC02LucyDeskAvailable)) {
		Scene_Exit_Add_2D_Exit(kRC02ExitRC51, 265, 58, 346, 154, 0);
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

void SceneScriptRC02::SceneLoaded() {
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
	if (Actor_Clue_Query(kActorMcCoy, kClueRuncitersVideo) || Global_Variable_Query(kVariableChapter) > 1) {
		Unclickable_Object("SCRTY CA03");
	}
	if (!Game_Flag_Query(kFlagShellCasingsTaken)) {
		Item_Add_To_World(kItemShellCasingA, 966, 16, -52.88f, -1238.89f, 108467.74f, 256, 6, 6, false, true, false, true);
		Item_Add_To_World(kItemShellCasingB, 966, 16, -37.16f, -1238.89f, 108456.59f, 512, 6, 6, false, true, false, true);
		Item_Add_To_World(kItemShellCasingC, 966, 16, -62.86f, -1238.89f, 108437.52f, 625, 6, 6, false, true, false, true);
	}
}

bool SceneScriptRC02::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptRC02::ClickedOn3DObject(const char *objectName, bool a2) {
	if ( Object_Query_Click("SCRTY CA03", objectName)
	 && !Actor_Clue_Query(kActorMcCoy, kClueRuncitersVideo)
	) {
		if (Actor_Clue_Query(kActorMcCoy, kClueRunciterInterviewA)
		 && Actor_Query_Is_In_Current_Set(kActorRunciter)
		) {
			AI_Movement_Track_Pause(kActorRunciter);
			Actor_Face_Actor(kActorMcCoy, kActorRunciter, true);
			Actor_Says(kActorMcCoy, 4545, 14);
			Actor_Face_Actor(kActorRunciter, kActorMcCoy, true);
			Actor_Says(kActorRunciter, 0, 14);
			Actor_Says(kActorRunciter, 10, 16);
			Actor_Says(kActorMcCoy, 4550, 13);
			Actor_Says(kActorRunciter, 20, 13);
			Loop_Actor_Walk_To_Waypoint(kActorRunciter, 89, 0, 0, false);
			Actor_Face_Actor(kActorMcCoy, kActorRunciter, true);
			Loop_Actor_Walk_To_Waypoint(kActorRunciter, 102, 0, 0, false);
			Actor_Face_Actor(kActorMcCoy, kActorRunciter, true);
			Actor_Face_Heading(kActorRunciter, 539, false);
			Delay(2000);
			Loop_Actor_Walk_To_Waypoint(kActorRunciter, 89, 0, 0, false);
			Loop_Actor_Walk_To_Actor(kActorRunciter, kActorMcCoy, 24, 0, false);
			Item_Pickup_Spin_Effect(975, 357, 228);
			Actor_Face_Actor(kActorMcCoy, kActorRunciter, true);
			Actor_Face_Actor(kActorRunciter, kActorMcCoy, true);
			Actor_Says(kActorRunciter, 30, 23);
			Actor_Says(kActorMcCoy, 4555, 18);
			Actor_Clue_Acquire(kActorMcCoy, kClueRuncitersVideo, 1, kActorRunciter);
			Unclickable_Object("SCRTY CA03");
			AI_Movement_Track_Unpause(kActorRunciter);
			return true;
		} else {
			Actor_Face_Object(kActorMcCoy, "SCRTY CA03", true);
			Actor_Voice_Over(2000, kActorVoiceOver);
			return true;
		}
	}
	return false;
}

void SceneScriptRC02::dialogueWithRunciter() {
	Dialogue_Menu_Clear_List();
	DM_Add_To_List_Never_Repeat_Once_Selected(0, 5, 6, 2);
	DM_Add_To_List_Never_Repeat_Once_Selected(10, 5, 4, 8);
	if (Actor_Clue_Query(kActorMcCoy, kClueRunciterInterviewB1)
	 || Actor_Clue_Query(kActorMcCoy, kClueRunciterInterviewB2)
	) {
		DM_Add_To_List_Never_Repeat_Once_Selected(20, 6, 4, 5);
	}
	Dialogue_Menu_Add_DONE_To_List(30);
	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();
	switch (answer) {
	case 0:
		Actor_Says(kActorMcCoy, 4580, 13);
		Actor_Face_Actor(kActorRunciter, kActorMcCoy, true);
		Actor_Says(kActorRunciter, 110, 18);
		Actor_Says(kActorRunciter, 120, 17);
		Actor_Says(kActorRunciter, 130, 19);
		Actor_Says(kActorMcCoy, 4605, 13);
		Actor_Says(kActorRunciter, 140, 16);
		Game_Flag_Set(kFlagRC02TalkedToRunciter);
		break;
	case 10:
		Actor_Says(kActorMcCoy, 4585, 13);
		Actor_Face_Actor(kActorRunciter, kActorMcCoy, true);
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			Actor_Says(kActorRunciter, 250, 13);
			Actor_Says(kActorRunciter, 270, 13);
			Actor_Clue_Acquire(kActorMcCoy, kClueRunciterInterviewB1, 1, kActorRunciter);
		} else {
			Actor_Says(kActorRunciter, 260, 14);
			Actor_Says(kActorRunciter, 270, 13);
			Actor_Clue_Acquire(kActorMcCoy, kClueRunciterInterviewB2, 1, kActorRunciter);
		}
		Actor_Says(kActorMcCoy, 4645, 13);
		Actor_Says(kActorRunciter, 280, 13);
		Actor_Says(kActorRunciter, 290, 13);
		Actor_Says(kActorMcCoy, 4650, 18);
		Actor_Says(kActorRunciter, 320, 13);
		Actor_Says(kActorMcCoy, 4665, 13);
		Actor_Face_Object(kActorRunciter, "CURTAIN", true);
		Actor_Says(kActorRunciter, 350, 13);
		Actor_Face_Actor(kActorRunciter, kActorMcCoy, true);
		Scene_Exit_Add_2D_Exit(kRC02ExitRC51, 265, 58, 346, 154, 0);
		Game_Flag_Set(kFlagRC02LucyDeskAvailable);
		break;
	case 20:
		Actor_Says(kActorMcCoy, 4590, 19);
		Actor_Face_Actor(kActorRunciter, kActorMcCoy, true);
		Actor_Says(kActorRunciter, 360, 13);
		Loop_Actor_Walk_To_Waypoint(kActorRunciter, 89, 0, 0, false);
		Loop_Actor_Walk_To_Waypoint(kActorRunciter, 102, 0, 0, false);
		Actor_Face_Actor(kActorMcCoy, kActorRunciter, true);
		Actor_Face_Heading(kActorRunciter, 539, false);
		Delay(2000);
		Loop_Actor_Walk_To_Waypoint(kActorRunciter, 89, 0, 0, false);
		Actor_Face_Actor(kActorMcCoy, kActorRunciter, true);
		Loop_Actor_Walk_To_Actor(kActorRunciter, kActorMcCoy, 24, 0, false);
		Actor_Face_Actor(kActorRunciter, kActorMcCoy, true);
		Actor_Face_Actor(kActorMcCoy, kActorRunciter, true);
		Item_Pickup_Spin_Effect(964, 357, 228);
		Actor_Says(kActorRunciter, 1700, 13);
		Actor_Clue_Acquire(kActorMcCoy, kClueReferenceLetter, 1, kActorRunciter);
		break;
	case 30:
		Actor_Says(kActorMcCoy, 4595, 14);
		break;
	}
}

bool SceneScriptRC02::ClickedOnActor(int actorId) {
	if (actorId != kActorRunciter) {
		return false;
	}

	if (Global_Variable_Query(kVariableChapter) == 4) {
		Actor_Face_Actor(kActorMcCoy, kActorRunciter, true);
		if (Actor_Query_Goal_Number(kActorRunciter) == 599) {
			if (Random_Query(1, 2) == 1) {
				Actor_Says(kActorMcCoy, 8715, 17);
			} else {
				Actor_Says(kActorMcCoy, 8720, 17);
			}
		} else if (Game_Flag_Query(705) || Game_Flag_Query(706)) {
			Actor_Says(kActorMcCoy, 4805, 11);
			Actor_Face_Actor(kActorRunciter, kActorMcCoy, true);
			if (Game_Flag_Query(706)) {
				Actor_Says(kActorRunciter, 720, 15);
			} else {
				Actor_Says(kActorRunciter, 730, 14);
			}
			Actor_Face_Heading(kActorRunciter, 1007, false);
		} else {
			Actor_Says(kActorMcCoy, 4690, 11);
			Actor_Says(kActorMcCoy, 4695, 13);
			Actor_Face_Actor(kActorRunciter, kActorMcCoy, true);
			Actor_Says(kActorRunciter, 1610, 14);
			if (Actor_Clue_Query(kActorMcCoy, kClueEnvelope)) {
				Actor_Says(kActorMcCoy, 4700, 12);
				Actor_Says(kActorMcCoy, 4705, 13);
				Actor_Says(kActorRunciter, 1620, 12);
				Actor_Says(kActorMcCoy, 4710, 15);
				Actor_Says(kActorMcCoy, 4715, 11);
				Delay(2000);
				Actor_Says(kActorMcCoy, 4720, 16);
				Actor_Says(kActorMcCoy, 4725, 17);
				Actor_Says(kActorRunciter, 430, 16);
				Actor_Face_Heading(kActorRunciter, 1007, false);
			}
			Game_Flag_Set(706);
		}
		return true;
	}
	AI_Movement_Track_Pause(kActorRunciter);
	Loop_Actor_Walk_To_Actor(kActorMcCoy, kActorRunciter, 48, 1, false);
	Actor_Face_Actor(kActorMcCoy, kActorRunciter, true);
	if (!Game_Flag_Query(kFlagRunciterInterviewA)) {
		Actor_Says(kActorMcCoy, 4560, 13);
		Actor_Face_Actor(kActorRunciter, kActorMcCoy, true);
		Actor_Says(kActorRunciter, 40, 16);
		Actor_Says(kActorRunciter, 50, 15);
		Actor_Says(kActorMcCoy, 4565, 13);
		Actor_Says(kActorRunciter, 60, 14);
		Actor_Says(kActorMcCoy, 4570, 18);
		Actor_Says(kActorRunciter, 70, 13);
		Game_Flag_Set(kFlagRunciterInterviewA);
		Actor_Clue_Acquire(kActorMcCoy, kClueRunciterInterviewA, 1, kActorRunciter);
		AI_Movement_Track_Unpause(kActorRunciter);
		return true;
	}
	if (Game_Flag_Query(kFlagRC02TalkedToRunciter)) {
		if (Player_Query_Agenda() == kPlayerAgendaPolite) {
			Game_Flag_Reset(kFlagNotUsed0);
			dialogueWithRunciter();
			AI_Movement_Track_Unpause(kActorRunciter);
			return true;
		}

		Actor_Says(kActorMcCoy, 4610, 19);
		Actor_Face_Actor(kActorRunciter, kActorMcCoy, true);
		Actor_Says(kActorRunciter, 150, 15);
		Actor_Says(kActorMcCoy, 4615, 13);
		Actor_Says(kActorRunciter, 160, 14);
		Actor_Says(kActorRunciter, 170, 15);
		Actor_Says(kActorRunciter, 180, 13);

		if (Player_Query_Agenda() == kPlayerAgendaSurly) {
			Actor_Says(kActorMcCoy, 4620, 19);
			Actor_Says(kActorRunciter, 190, 14);
			Actor_Says(kActorMcCoy, 4625, 13);
			Actor_Says(kActorRunciter, 210, 13);
			Actor_Says(kActorMcCoy, 4630, 18);
			Actor_Says(kActorRunciter, 220, 14);
			Actor_Says(kActorRunciter, 230, 13);
			Actor_Says(kActorMcCoy, 4635, 19);
			Actor_Says(kActorRunciter, 240, 16);
			Actor_Says(kActorMcCoy, 4640, 17);
		}
		Game_Flag_Reset(kFlagRC02TalkedToRunciter);
		AI_Movement_Track_Unpause(kActorRunciter);
		return true;
	}
	dialogueWithRunciter();
	AI_Movement_Track_Unpause(kActorRunciter);
	return true;
}

bool SceneScriptRC02::ClickedOnItem(int itemId, bool a2) {
	if (itemId == kItemShellCasingA || itemId == kItemShellCasingB || itemId == kItemShellCasingC) {
		if (!Loop_Actor_Walk_To_Item(kActorMcCoy, kItemShellCasingA, 24, 1, false)) {
			Actor_Face_Item(kActorMcCoy, kItemShellCasingA, true);
			Actor_Clue_Acquire(kActorMcCoy, kClueShellCasings, 1, -1);
			Game_Flag_Set(kFlagShellCasingsTaken);
			Item_Remove_From_World(kItemShellCasingA);
			Item_Remove_From_World(kItemShellCasingB);
			Item_Remove_From_World(kItemShellCasingC);
			Item_Pickup_Spin_Effect(966, 395, 352);
			Actor_Voice_Over(1960, kActorVoiceOver);
		}
		return true;
	}
	return false;
}

bool SceneScriptRC02::ClickedOnExit(int exitId) {
	if (exitId == kRC02ExitRC01) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -71.51f, -1238.89f, 108587.15f, 0, true, false, 0)) {
			Game_Flag_Set(kFlagRC02toRC01);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_Looping_Sound(71, true);
			Ambient_Sounds_Remove_Looping_Sound(75, true);
			Ambient_Sounds_Adjust_Looping_Sound(85, 100, -101, 1);
			Actor_Set_Goal_Number(kActorRunciter, 0);
			Set_Enter(kSetRC01, kSceneRC01);
		}
		return true;
	}
	if (exitId == kRC02ExitRC51) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -20.2f, -1238.73f, 108152.73f, 0, true, false, 0)) {
			Async_Actor_Walk_To_XYZ(kActorMcCoy, -8.87f, -1238.89f, 108076.27f, 0, false);
			Set_Enter(kSetRC02_RC51, kSceneRC51);
		}
		return true;
	}
	return false;
}

bool SceneScriptRC02::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptRC02::SceneFrameAdvanced(int frame) {
}

void SceneScriptRC02::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptRC02::PlayerWalkedIn() {
	Player_Set_Combat_Mode(false);
	if (Game_Flag_Query(kFlagRC01toRC02)) {
		Player_Loses_Control();
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -72.2f, -1238.89f, 108496.73f, 0, false, false, 0);
		Player_Gains_Control();
		Game_Flag_Reset(kFlagRC01toRC02);
		if (!Game_Flag_Query(kFlagRC02Discovered)) {
			Actor_Voice_Over(1970, kActorVoiceOver);
			Actor_Voice_Over(1980, kActorVoiceOver);
			Actor_Voice_Over(1990, kActorVoiceOver);
			Actor_Clue_Acquire(kActorMcCoy, kClueLimpingFootprints, 1, -1);
			Actor_Clue_Acquire(kActorMcCoy, kClueGracefulFootprints, 1, -1);
			Game_Flag_Set(kFlagRC02Discovered);
		}
		if (Actor_Query_Which_Set_In(kActorRunciter) == kSetRC02_RC51 && Actor_Query_Goal_Number(kActorRunciter) < 300) {
			Actor_Set_Goal_Number(kActorRunciter, 1);
		}
		if (Actor_Query_Goal_Number(kActorRunciter) == 300 && !Game_Flag_Query(704)) {
			Actor_Face_Actor(kActorRunciter, kActorMcCoy, true);
			Actor_Says(kActorRunciter, 370, 12);
			Actor_Says(kActorRunciter, 380, 14);
			Actor_Face_Actor(kActorMcCoy, kActorRunciter, true);
			Actor_Says(kActorMcCoy, 4670, 15);
			Actor_Says(kActorRunciter, 390, 13);
			Actor_Says(kActorMcCoy, 4675, 14);
			Actor_Face_Heading(kActorRunciter, 1007, false);
			Actor_Says(kActorRunciter, 400, 13);
			Actor_Says(kActorRunciter, 410, 12);
			Game_Flag_Set(704);
		}
	} else {
		Player_Loses_Control();
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -20.2f, -1238.89f, 108152.73f, 0, 0, false, 0);
		Player_Gains_Control();
	}
}

void SceneScriptRC02::PlayerWalkedOut() {
}

void SceneScriptRC02::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
