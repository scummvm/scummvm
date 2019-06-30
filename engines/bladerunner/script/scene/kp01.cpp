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

void SceneScriptKP01::InitializeScene() {
	if (Game_Flag_Query(kFlagKP04toKP01)) {
		Setup_Scene_Information(-125.0f, -12.2f,  -61.0f, 400);
	} else if (Game_Flag_Query(kFlagKP03toKP01)) {
		Setup_Scene_Information(-284.0f, -12.2f, -789.0f, 445);
	} else {
		Setup_Scene_Information( 239.0f, -12.2f, -146.0f, 820);
		Game_Flag_Reset(kFlagKP02toKP01);
		if (!Game_Flag_Query(kFlagKP01Entered)
		 && !Game_Flag_Query(kFlagMcCoyIsHelpingReplicants)
		) {
			Game_Flag_Set(kFlagKP01Entered);
			Actor_Set_Goal_Number(kActorSteele, kGoalSteeleKP01Wait);
		}
	}

	Scene_Exit_Add_2D_Exit(0,   0, 0,  30, 479, 3);
	Scene_Exit_Add_2D_Exit(1, 150, 0, 200, 276, 0);
	Scene_Exit_Add_2D_Exit(2, 589, 0, 639, 479, 1);

	Ambient_Sounds_Add_Looping_Sound(kSfxKPAMB1,   34, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxSKINBED1, 27, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxRUMLOOP1, 90, 1, 1);
	Ambient_Sounds_Add_Sound(kSfxSCARY1,  2, 100, 25,  33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY2,  2, 100, 25,  33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY3,  2, 100, 25,  33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER2, 5, 180, 50, 100, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER4, 5, 180, 50, 100, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY4,  2, 100, 25,  33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY5,  2, 100, 25,  33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY6,  2, 100, 25,  33, -100, 100, -101, -101, 0, 0);
}

void SceneScriptKP01::SceneLoaded() {
	Unobstacle_Object("TRAINCAR-1", true);
	Unobstacle_Object("FORE-JUNK-02", true);
	Obstacle_Object("OBSTACLE1", true);
	Obstacle_Object("TUBE1", true);
	Unclickable_Object("OBSTACLE1");
}

bool SceneScriptKP01::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptKP01::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptKP01::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptKP01::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptKP01::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -125.0f, -12.2f, -61.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagKP01toKP04);
			Set_Enter(kSetKP04, kSceneKP04);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -284.0f, -12.2f, -789.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagKP01toKP03);
			Set_Enter(kSetKP03, kSceneKP03);
		}
		return true;
	}

	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 239.0f, 12.2f, -146.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagKP01toKP02);
			Set_Enter(kSetKP02, kSceneKP02);
		}
		return true;
	}
	return false;
}


bool SceneScriptKP01::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptKP01::SceneFrameAdvanced(int frame) {
}

void SceneScriptKP01::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
	if (actorId == kActorSteele) {
		if (newGoal == kGoalSteeleKP01Leave) {
			if (Game_Flag_Query(kFlagDirectorsCut)) {
				Delay(500);
				Actor_Change_Animation_Mode(kActorMcCoy, 75);
				Delay(4500);
				Actor_Face_Current_Camera(kActorMcCoy, true);
				Actor_Says(kActorMcCoy, 510, kAnimationModeTalk);
			} else {
				Delay(3000);
			}
			Async_Actor_Walk_To_XYZ(kActorMcCoy, 76.56f, -12.2f, -405.48f, 0, false);
			//return true;
		} else if (newGoal == kGoalSteeleKP01Left) {
			Player_Gains_Control();
			Actor_Force_Stop_Walking(kActorMcCoy);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagKP01toKP03);
			Set_Enter(kSetKP03, kSceneKP03);
			//return true;
		}
	}
	//return false;
}

void SceneScriptKP01::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagKP04toKP01)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -93.0f, -12.2f, -61.0f, 0, false, false, false);
		Game_Flag_Reset(kFlagKP04toKP01);
		return;
	}

	if (Game_Flag_Query(kFlagKP03toKP01)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -240.0f, -12.2f, -789.0f, 0, false, false, false);
		Game_Flag_Reset(kFlagKP03toKP01);
		return;
	}

	Loop_Actor_Walk_To_XYZ(kActorMcCoy, 211.0f, -12.2f, -146.0f, 0, false, false, false);
	if (!Game_Flag_Query(kFlagMcCoyIsHelpingReplicants)
	 && !Game_Flag_Query(kFlagMcCoyAttackedReplicants)
	 &&  Actor_Query_Goal_Number(kActorSteele) == kGoalSteeleKP01Wait
	 &&  Actor_Query_Goal_Number(kActorSteele) != kGoalSteeleGone
	) {
		Player_Loses_Control();
		Actor_Set_Goal_Number(kActorSteele, kGoalSteeleKP01TalkToMcCoy);
	}
}

void SceneScriptKP01::PlayerWalkedOut() {
}

void SceneScriptKP01::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
