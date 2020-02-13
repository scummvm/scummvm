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

void SceneScriptKP07::InitializeScene() {
	Setup_Scene_Information(-12.0f, -41.58f, 72.0f, 0);

	Game_Flag_Reset(kFlagKP06toKP07);

	Scene_Exit_Add_2D_Exit(0, 315, 185, 381, 285, 0);

	if (Game_Flag_Query(kFlagMcCoyIsHelpingReplicants)) {
		if (Game_Flag_Query(kFlagDektoraIsReplicant)
		 && Actor_Query_Goal_Number(kActorDektora) < kGoalDektoraGone
		) {
#if BLADERUNNER_ORIGINAL_BUGS
#else
			if (Game_Flag_Query(kFlagNR11DektoraBurning)) {
				Game_Flag_Reset(kFlagNR11DektoraBurning); // resolves a bug of an original game, where Dektora would default to burning state
				Actor_Change_Animation_Mode(kActorDektora, kAnimationModeTalk); // dummy animation change to ensure that the next will trigger the mode change case
				Actor_Change_Animation_Mode(kActorDektora, kAnimationModeIdle);
			}
			AI_Movement_Track_Flush(kActorDektora);
			Actor_Set_Goal_Number(kActorDektora, kGoalDektoraKP07Wait); // new clear goal
#endif // BLADERUNNER_ORIGINAL_BUGS
			Actor_Set_Targetable(kActorDektora, true);
			Global_Variable_Increment(kVariableReplicantsSurvivorsAtMoonbus, 1);
			Actor_Put_In_Set(kActorDektora, kSetKP07);
			Actor_Set_At_XYZ(kActorDektora, -52.0f, -41.52f, -5.0f, 289);
		}

		if (Actor_Query_Goal_Number(kActorZuben) < kGoalZubenGone) {
#if BLADERUNNER_ORIGINAL_BUGS
#else
			AI_Movement_Track_Flush(kActorZuben);
			Actor_Set_Goal_Number(kActorZuben, kGoalZubenKP07Wait); // new clear goal
#endif // BLADERUNNER_ORIGINAL_BUGS
			Global_Variable_Increment(kVariableReplicantsSurvivorsAtMoonbus, 1);
			Actor_Set_Targetable(kActorZuben, true);
			Actor_Put_In_Set(kActorZuben, kSetKP07);
			Actor_Set_At_XYZ(kActorZuben, -26.0f, -41.52f, -135.0f, 0);
		}

		if (Game_Flag_Query(kFlagIzoIsReplicant)
		 && Actor_Query_Goal_Number(kActorIzo) < 599
		) {
#if BLADERUNNER_ORIGINAL_BUGS
#else
			AI_Movement_Track_Flush(kActorIzo);
			Actor_Set_Goal_Number(kActorIzo, kGoalIzoKP07Wait); // new clear goal
#endif // BLADERUNNER_ORIGINAL_BUGS
			Global_Variable_Increment(kVariableReplicantsSurvivorsAtMoonbus, 1);
			Actor_Set_Targetable(kActorIzo, true);
			Actor_Put_In_Set(kActorIzo, kSetKP07);
			Actor_Set_At_XYZ(kActorIzo, -38.0f, -41.52f, -175.0f, 500);
		}

		if (Game_Flag_Query(kFlagGordoIsReplicant)
		 && Actor_Query_Goal_Number(kActorGordo) < kGoalGordoGone
		) {
#if BLADERUNNER_ORIGINAL_BUGS
#else
			AI_Movement_Track_Flush(kActorGordo);
			Actor_Set_Goal_Number(kActorGordo, kGoalGordoKP07Wait); // new clear goal
#endif // BLADERUNNER_ORIGINAL_BUGS
			Global_Variable_Increment(kVariableReplicantsSurvivorsAtMoonbus, 1);
			Actor_Set_Targetable(kActorGordo, true);
			Actor_Put_In_Set(kActorGordo, kSetKP07);
			Actor_Set_At_XYZ(kActorGordo, 61.0f, -41.52f, -3.0f, 921);
		}

		if (Game_Flag_Query(kFlagLucyIsReplicant)
		 && Actor_Query_Goal_Number(kActorLucy) < kGoalLucyGone
		) {
#if BLADERUNNER_ORIGINAL_BUGS
#else
			AI_Movement_Track_Flush(kActorLucy);
			Actor_Set_Goal_Number(kActorLucy, kGoalLucyKP07Wait); // new clear goal
#endif // BLADERUNNER_ORIGINAL_BUGS
			Global_Variable_Increment(kVariableReplicantsSurvivorsAtMoonbus, 1);
			Actor_Put_In_Set(kActorLucy, kSetKP07);
			Actor_Set_At_XYZ(kActorLucy, 78.0f, -41.52f, -119.0f, 659);
		}

		if (Actor_Query_Goal_Number(kActorLuther) < kGoalLutherGone) {
#if BLADERUNNER_ORIGINAL_BUGS
#else
			AI_Movement_Track_Flush(kActorLuther);
			Actor_Set_Goal_Number(kActorLuther, kGoalLutherKP07Wait); // new goal to avoid resuming his walking around routine
#endif // BLADERUNNER_ORIGINAL_BUGS
			Global_Variable_Increment(kVariableReplicantsSurvivorsAtMoonbus, 1);
			Actor_Put_In_Set(kActorLuther, kSetKP07);
			Actor_Set_At_XYZ(kActorLuther, -47.0f, 0.0f, 151.0f, 531);
		}
	}
#if BLADERUNNER_ORIGINAL_BUGS
#else
	// Additional fix for saves with bad state (goal 513) for Clovis
	// which resulted in him standing, clipping through his moonbus bed
	// when McCoy is not helping the Replicants
	else {
		// McCoy is not helping the Replicants
		if (Actor_Query_Goal_Number(kActorClovis) == kGoalClovisKP07Wait
			&& !Game_Flag_Query(kFlagClovisLyingDown)) {
			// this goal set is only for the purpose of switch Clovis goal out of kGoalClovisKP07Wait
			Actor_Set_Goal_Number(kActorClovis, kGoalClovisStartChapter5);
			// And explicitly switching back to kGoalClovisKP07Wait in order
			// to trigger the bug-fixed GoalChanged() case in his AI
			Actor_Set_Goal_Number(kActorClovis, kGoalClovisKP07Wait);
		}
	}
#endif // BLADERUNNER_ORIGINAL_BUGS

	Ambient_Sounds_Add_Looping_Sound(kSfxCOMPBED1,  7, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxMOONBED2, 52, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxBRBED3,   38, 1, 1);

	if (Game_Flag_Query(kFlagKP07BusActive)) {
		Scene_Loop_Set_Default(2);
	} else {
		Scene_Loop_Set_Default(0);
	}
}

void SceneScriptKP07::SceneLoaded() {
	if (!Game_Flag_Query(kFlagMcCoyIsHelpingReplicants)) {
		Music_Play(kMusicClovDie1, 25, 0, 0, -1, 1, 0);
	}
	Obstacle_Object("BUNK_TRAY01", true);
	Unobstacle_Object("BUNK_TRAY01", true);
	if (Game_Flag_Query(kFlagMcCoyIsHelpingReplicants)) {
		Player_Set_Combat_Mode(false);
		Scene_Exits_Disable();
	}
}

bool SceneScriptKP07::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptKP07::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptKP07::ClickedOnActor(int actorId) {
	if (actorId == kActorClovis) {
#if BLADERUNNER_ORIGINAL_BUGS
#else
		Actor_Face_Actor(kActorMcCoy, kActorClovis, true);
#endif // BLADERUNNER_ORIGINAL_BUGS
		if (!Game_Flag_Query(kFlagKP07McCoyPulledGun)
		 &&  Actor_Query_Goal_Number(kActorClovis) != kGoalClovisGone
		 &&  Actor_Query_Goal_Number(kActorClovis) != kGoalClovisKP07SayFinalWords
		) {
			if (Game_Flag_Query(kFlagMcCoyIsHelpingReplicants)) {
				Actor_Set_Goal_Number(kActorClovis, kGoalClovisKP07FlyAway);
			} else {
				Music_Play(kMusicClovDies, 31, 0, 0, -1, 1, 0);
				Actor_Set_Goal_Number(kActorClovis, kGoalClovisKP07TalkToMcCoy);
			}
			return true;
		}
	} else {
#if BLADERUNNER_ORIGINAL_BUGS
		Actor_Face_Actor(kActorMcCoy, actorId, true);
		Actor_Says(kActorMcCoy, 8590, 14);
		return true;
#else
		if (!Player_Query_Combat_Mode()) {
			Actor_Face_Actor(kActorMcCoy, actorId, true);
			Actor_Says(kActorMcCoy, 8590, 14);
			return true;
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
	}

	return false;
}

bool SceneScriptKP07::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptKP07::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -12.0f, -41.58f, 72.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagKP07toKP06);
			Set_Enter(kSetKP05_KP06, kSceneKP06);
		}
		return true;
	}
	return false;
}

bool SceneScriptKP07::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptKP07::SceneFrameAdvanced(int frame) {
}

void SceneScriptKP07::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptKP07::PlayerWalkedIn() {
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, 9.0f, -41.88f, -81.0f, 0, false, false, false);
	if (!Game_Flag_Query(kFlagKP07Entered)) {
		if (Game_Flag_Query(kFlagMcCoyIsHelpingReplicants)) {
			Actor_Face_Actor(kActorMcCoy, kActorClovis, true);
			Actor_Says(kActorClovis, 1240, 3);
			Actor_Says(kActorMcCoy, 8500, 3);
			Actor_Says(kActorClovis, 1250, 3);
			if (Actor_Query_Goal_Number(kActorSadik) == kGoalSadikKP06NeedsReactorCoreFromMcCoy) {
				Actor_Put_In_Set(kActorSadik, kSetKP07);
				Global_Variable_Increment(kVariableReplicantsSurvivorsAtMoonbus, 1);
				Actor_Set_At_XYZ(kActorSadik, -12.0f, -41.58f, 72.0f, 0);
				Actor_Face_Actor(kActorSadik, kActorClovis, true);
			}
		} else {
			Actor_Face_Actor(kActorMcCoy, kActorClovis, true);
			Actor_Says(kActorClovis, 160, 3);
			Actor_Retired_Here(kActorClovis, 72, 60, 0, -1);
		}
		Game_Flag_Set(kFlagKP07Entered);
	}
}

void SceneScriptKP07::PlayerWalkedOut() {
	Music_Stop(3);
}

void SceneScriptKP07::DialogueQueueFlushed(int a1) {
	if (Actor_Query_Goal_Number(kActorClovis) == kGoalClovisKP07SayFinalWords) {
		Actor_Set_Targetable(kActorClovis, false);
		Actor_Change_Animation_Mode(kActorClovis, kAnimationModeHit);
		Actor_Retired_Here(kActorClovis, 12, 48, true, -1);
		Actor_Set_Goal_Number(kActorClovis, kGoalClovisGone);
	}
}

} // End of namespace BladeRunner
