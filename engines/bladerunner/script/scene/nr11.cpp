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

enum kNR11Loops {
	kNR11LoopMainLoop         = 0,
	kNR11LoopGunshots         = 2,
	kNR11LoopMainLoopFires    = 3,
	kNR11LoopMainLoopBurnedUp = 5
};

void SceneScriptNR11::InitializeScene() {
	Setup_Scene_Information(100.0f, 1.75f, -4.0f, 0);

	Scene_Exit_Add_2D_Exit(0, 450, 305, 565, 345, 2);

	if (!Game_Flag_Query(kFlagNR10CameraDestroyed)) {
		Ambient_Sounds_Adjust_Looping_Sound(kSfx35MM, 22, 0, 1);
	}

	Ambient_Sounds_Add_Looping_Sound(kSfxCTDRONE1, 22, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxBRBED5,   33, 0, 1);
	Ambient_Sounds_Add_Sound(kSfxBBGRN1,  2, 50, 7, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN2,  2, 50, 7, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN3,  2, 50, 7, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE1, 2, 50, 7, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE2, 2, 50, 7, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE3, 2, 50, 7, 17, -100, 100, -101, -101, 0, 0);

	if (Game_Flag_Query(kFlagNR01DektoraFall)) {
		Scene_Loop_Set_Default(kNR11LoopMainLoopFires);
		Ambient_Sounds_Add_Looping_Sound(kSfxRAIN10, 83, 0, 1);
	} else if (Game_Flag_Query(kFlagNR11BurnedUp)) {
		Scene_Loop_Set_Default(kNR11LoopMainLoopBurnedUp);
		Ambient_Sounds_Add_Looping_Sound(kSfxRAIN10, 83, 0, 1);
	} else {
		Scene_Loop_Set_Default(kNR11LoopMainLoop);
		Overlay_Play("NR11OVER", 0, true, false, 0);
	}
}

void SceneScriptNR11::SceneLoaded() {
	Obstacle_Object("COATRACK", true);
	Unobstacle_Object("BOX13", true);
	Clickable_Object("LOFT04");
	Unclickable_Object("LOFT04");
	if (Actor_Query_Goal_Number(kActorDektora) == kGoalDektoraNR11Hiding) {
		Clickable_Object("CLOTHING02");
		Clickable_Object("BOX27");
		Clickable_Object("BOX39");
#if BLADERUNNER_ORIGINAL_BUGS
		// in our bugfixes this object is removed
		Clickable_Object("BOX44");
#endif // BLADERUNNER_ORIGINAL_BUGS
		Clickable_Object("DRESS");
		Clickable_Object("COATRACK");
		Clickable_Object("COLUMN3 DETS");
		Clickable_Object("COLUMN PIPE01");
		Clickable_Object("RECTANGLE02");
		Clickable_Object("COLUMN04");
		Clickable_Object("COATRACK01");
		Clickable_Object("SHIRT");
		Clickable_Object("SKIRT 02");
		Clickable_Object("CLOTHING B 03");
		Clickable_Object("BUST BUST");
		Combat_Target_Object("CLOTHING02");
		Combat_Target_Object("BOX27");
		Combat_Target_Object("BOX39");
#if BLADERUNNER_ORIGINAL_BUGS
		// in our bugfixes this object is removed
		Combat_Target_Object("BOX44");
#endif // BLADERUNNER_ORIGINAL_BUGS
		Combat_Target_Object("DRESS");
		Combat_Target_Object("COATRACK");
		Combat_Target_Object("COLUMN3 DETS");
		Combat_Target_Object("COLUMN PIPE01");
		Combat_Target_Object("RECTANGLE02");
		Combat_Target_Object("COLUMN04");
		Combat_Target_Object("COATRACK01");
		Combat_Target_Object("SHIRT");
		Combat_Target_Object("SKIRT 02");
		Combat_Target_Object("CLOTHING B 03");
		Combat_Target_Object("BUST BUST");
	} else {
		Unclickable_Object("CLOTHING02");
		Unclickable_Object("BOX27");
		Unclickable_Object("BOX39");
#if BLADERUNNER_ORIGINAL_BUGS
		// in our bugfixes this object is removed
		Unclickable_Object("BOX44");
#endif
		Unclickable_Object("DRESS");
		Unclickable_Object("COATRACK");
		Unclickable_Object("COLUMN3 DETS");
		Unclickable_Object("COLUMN PIPE01");
		Unclickable_Object("RECTANGLE02");
		Unclickable_Object("COLUMN04");
		Unclickable_Object("COATRACK01");
		Unclickable_Object("SHIRT");
		Unclickable_Object("SKIRT 02");
		Unclickable_Object("CLOTHING B 03");
		Unclickable_Object("BUST BUST");
	}
}

bool SceneScriptNR11::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptNR11::ClickedOn3DObject(const char *objectName, bool combatMode) {

	if (Object_Query_Click("CLOTHING02", objectName)
	 || Object_Query_Click("BOX27", objectName)
	 || Object_Query_Click("BOX39", objectName)
#if BLADERUNNER_ORIGINAL_BUGS
	 // in our bugfixes this object is removed
	 || Object_Query_Click("BOX44", objectName)
#endif // BLADERUNNER_ORIGINAL_BUGS
	 || Object_Query_Click("DRESS", objectName)
	 || Object_Query_Click("COATRACK", objectName)
	 || Object_Query_Click("COLUMN3 DETS", objectName)
	 || Object_Query_Click("COLUMN PIPE01", objectName)
	 || Object_Query_Click("RECTANGLE02", objectName)
	 || Object_Query_Click("COLUMN04", objectName)
	 || Object_Query_Click("COATRACK01", objectName)
	 || Object_Query_Click("SHIRT", objectName)
	 || Object_Query_Click("SKIRT 02", objectName)
	 || Object_Query_Click("CLOTHING B 03", objectName)
	 || Object_Query_Click("BUST BUST", objectName)
	) {
		if (combatMode) {
			Actor_Set_Goal_Number(kActorSteele, kGoalSteeleNR11StopWaiting);
			Scene_Exits_Disable();
			untargetEverything();
			Player_Loses_Control();
			if (!Player_Query_Combat_Mode()) {
				Player_Set_Combat_Mode(true);
			}
			Actor_Set_Goal_Number(kActorMcCoy, kGoalMcCoyNR11Shoot);
			Scene_Loop_Set_Default(kNR11LoopMainLoopFires);
			Scene_Loop_Start_Special(kSceneLoopModeOnce, kNR11LoopGunshots, true);
		} else if (Actor_Query_Goal_Number(kActorDektora) == kGoalDektoraNR11Hiding) {
			if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 24.0f, 0.33f, 0.0f, 0, true, false, false)) {
				Actor_Face_XYZ(kActorMcCoy, -180.0f, 0.0f, -170.0f, true);
				untargetEverything();
				Actor_Set_Goal_Number(kActorSteele, kGoalSteeleNR11StopWaiting);
				if (Actor_Query_Friendliness_To_Other(kActorDektora, kActorMcCoy) < 30) {
					Actor_Set_At_XYZ(kActorDektora, 0.5f, 0.33f, -162.0f, 0);
					Loop_Actor_Walk_To_XYZ(kActorDektora, -24.0f, 0.33f, -35.4f, 0, false, true, false);
					Actor_Face_Actor(kActorMcCoy, kActorDektora, true);
					Actor_Change_Animation_Mode(kActorDektora, 71);
					Delay(500);
					Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeDie);
					Delay(2000);
					Actor_Set_Goal_Number(kActorMcCoy, kGoalMcCoyNR10Fall);
				} else {
					Actor_Says(kActorMcCoy, 3840, 18);
					Delay(1000);
					if (Actor_Query_Friendliness_To_Other(kActorDektora, kActorMcCoy) > 59
					 && Global_Variable_Query(kVariableAffectionTowards) == kAffectionTowardsNone
					) {
						Music_Play(kMusicLoveSong, 35, 0, 3, -1, 0, 0);
					}
					Loop_Actor_Walk_To_XYZ(kActorDektora, -135.0f, 0.33f, -267.0f, 0, false, false, false);
					Actor_Face_Actor(kActorDektora, kActorMcCoy, true);
					Actor_Face_Actor(kActorMcCoy, kActorDektora, true);
					Actor_Clue_Acquire(kActorMcCoy, kClueDektoraInterview4, true, kActorDektora);
					Actor_Says(kActorDektora, 990, 13);
					Actor_Says(kActorDektora, 1000, 14);
					Loop_Actor_Walk_To_Actor(kActorDektora, kActorMcCoy, 108, false, false);
					Actor_Says(kActorMcCoy, 3845, 13);
					Actor_Says(kActorMcCoy, 3850, 15);
					Actor_Says(kActorDektora, 1010, 14);
					Actor_Says(kActorMcCoy, 3855, 13);
					Actor_Says(kActorDektora, 1020, 12);
					Actor_Says(kActorMcCoy, 3860, 12);
					Actor_Says_With_Pause(kActorDektora, 1030, 1.0f, 14);
					Actor_Says(kActorDektora, 1040, 13);
					Actor_Says(kActorMcCoy, 3865, 15);
					Actor_Says_With_Pause(kActorDektora, 1050, 0.8f, 14);
					Actor_Says(kActorDektora, 1060, 13);
					Actor_Says(kActorMcCoy, 3870, 3);
					Actor_Says(kActorDektora, 1070, 14);
					Actor_Modify_Friendliness_To_Other(kActorDektora, kActorMcCoy, 5);
					if (Actor_Query_Friendliness_To_Other(kActorDektora, kActorMcCoy) > 55
					 && Global_Variable_Query(kVariableAffectionTowards) == kAffectionTowardsNone
					) {
						Global_Variable_Set(kVariableAffectionTowards, kAffectionTowardsDektora);
						Actor_Says(kActorDektora, 1130, 17);
						Actor_Says(kActorMcCoy, 6365, 12);
						Actor_Says(kActorDektora, 1140, 14);
						Actor_Says(kActorMcCoy, 6370, 14);
						Actor_Says(kActorDektora, 1150, 12);
						Actor_Says(kActorDektora, 1160, 16);
					}
					Actor_Says(kActorDektora, 1080, 13);
					Actor_Says(kActorMcCoy, 3875, 14);
					Actor_Says(kActorDektora, 1090, 17);
					Music_Stop(4);
					Actor_Set_Goal_Number(kActorDektora, kGoalDektoraNR11WalkAway);
					if (Global_Variable_Query(kVariableHollowayArrest) == 1) {
						Actor_Set_Goal_Number(kActorSteele, kGoalSteeleNR10Wait);
					}
					Game_Flag_Set(kFlagDektoraRanAway);
				}
			} else {
				// TODO why put this here? Could be a bug.
				// There's a chance that McCoy will say this while "searching" for hidden Dektora
				if (Random_Query(1, 2) == 1) {
					Actor_Says(kActorMcCoy, 8575, 14); // More useless junk.
				} else {
					Actor_Says(kActorMcCoy, 8580, 14); // Nothing else there
				}
			}
		}
		return true;
	}
	return false;
}

bool SceneScriptNR11::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptNR11::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptNR11::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 100.0f, 1.75f, -8.0f, 0, true, false, false)) {
			Game_Flag_Set(kFlagNR11toNR10);
			Set_Enter(kSetNR10, kSceneNR10);
			return true;
		}
	}
	return false;
}

bool SceneScriptNR11::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptNR11::SceneFrameAdvanced(int frame) {
	if (frame == 62) {
		Ambient_Sounds_Play_Sound(kSfxMCGUN1,  40, 100, 100, 10);
	}

	if (frame == 67) {
		Ambient_Sounds_Play_Sound(kSfxMCGUN1,  30,  90,  90, 10);
	}

	if (frame == 74) {
		Ambient_Sounds_Play_Sound(kSfxMCGUN2,  50,  83,  83, 10);
	}

	if (frame == 80) {
		Ambient_Sounds_Play_Sound(kSfxMCGUN1,  60,  65,  65, 10);
	}

	if (frame == 92) {
		Ambient_Sounds_Play_Sound(kSfxMCGUN2,  30,  50,  50, 10);
	}

	if (frame == 97) {
		Ambient_Sounds_Play_Sound(kSfxMCGUN1,  50, -40, -40, 10);
	}

	if (frame == 103) {
		Ambient_Sounds_Play_Sound(kSfxMCGUN2,  40, -27, -27, 10);
	}

	if (frame == 109) {
		Ambient_Sounds_Play_Sound(kSfxMCGUN1,  60, -20, -20, 10);
	}

	if (frame == 62) {
		Ambient_Sounds_Play_Sound(kSfxMTLHIT1, 80, 100, 100, 15);
	}

	if (Game_Flag_Query(kFlagNR11BreakWindow)) {
		Game_Flag_Reset(kFlagNR11BreakWindow);
		Overlay_Remove("NR11OVER");
		Overlay_Play("NR11OVER", 1, false, true, 0);
	}

	if (Game_Flag_Query(kFlagNR11SteeleShoot)) {
		untargetEverything();
		Player_Loses_Control();
		if (!Player_Query_Combat_Mode()) {
			Player_Set_Combat_Mode(true);
		}
		Actor_Set_Goal_Number(kActorMcCoy, kGoalMcCoyNR11Shoot);
		Scene_Loop_Set_Default(kNR11LoopMainLoopFires);
		Scene_Loop_Start_Special(kSceneLoopModeOnce, kNR11LoopGunshots, true);
		Game_Flag_Reset(kFlagNR11SteeleShoot);
	} else {
		if (frame < 61
		 || frame > 120
		) {
			//return false;
			return;
		}
		actorSweepArea(kActorMcCoy, frame);
		if (Actor_Query_Goal_Number(kActorSteele) == kGoalSteeleNR11Shoot) {
			actorSweepArea(kActorSteele, frame);
		}
		if (frame == 120) {
			Actor_Set_Goal_Number(kActorMcCoy, kGoalMcCoyDefault);
			Player_Gains_Control();
			if (Actor_Query_Goal_Number(kActorSteele) == kGoalSteeleNR11Shoot) {
				Actor_Set_Goal_Number(kActorSteele, kGoalSteeleNR11StopShooting);
			}
			Actor_Set_Goal_Number(kActorDektora, kGoalDektoraNR11PrepareBurning);
		}
	}
	//return true;
}

void SceneScriptNR11::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptNR11::PlayerWalkedIn() {
	if (Actor_Query_Goal_Number(kActorDektora) == kGoalDektoraNR11Hiding) {
		Player_Set_Combat_Mode(true);
		if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
			Actor_Set_Goal_Number(kActorSteele, kGoalSteeleNR11StartWaiting);
		}
	}

	if (Game_Flag_Query(kFlagNR01DektoraFall)) {
		Game_Flag_Reset(kFlagNR01DektoraFall);
		Game_Flag_Set(kFlagNR11BurnedUp);
		Actor_Put_In_Set(kActorDektora, kSetFreeSlotI);
		Actor_Set_At_Waypoint(kActorDektora, 41, 0);
		Actor_Set_Invisible(kActorMcCoy, false);
		Player_Set_Combat_Mode(false);
		Player_Gains_Control();
		if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
			if (Actor_Query_Goal_Number(kActorSteele) == kGoalSteeleNR11StopWaiting) {
				Actor_Set_At_XYZ(kActorMcCoy, -37.41f, 0.33f, -86.0f, 26);
				Delay(500);
				Actor_Face_Current_Camera(kActorMcCoy, true);
				Delay(750);
				Actor_Says(kActorMcCoy, 5290, 12); // Jesus
				Delay(1000);
				Actor_Set_Goal_Number(kActorSteele, kGoalSteeleNR11Enter);
				Actor_Face_Actor(kActorMcCoy, kActorSteele, true);
			} else {
				Actor_Set_At_XYZ(kActorMcCoy, -15.53f, 0.33f, 73.49f, 954);
				Actor_Modify_Friendliness_To_Other(kActorSteele, kActorMcCoy, 3);
				Delay(1500);
				Actor_Says(kActorMcCoy, 3805, 12);
				Actor_Face_Actor(kActorMcCoy, kActorSteele, true);
				Actor_Face_Actor(kActorSteele, kActorMcCoy, true);
				Actor_Says_With_Pause(kActorSteele, 1720, 0.3f, 16);
				Actor_Says(kActorMcCoy, 3810, 16);
				Actor_Says_With_Pause(kActorSteele, 1730, 0.2f, 14);
				Actor_Says(kActorSteele, 1740, 15);
				Actor_Set_Goal_Number(kActorDektora, kGoalDektoraGone);
				Actor_Put_In_Set(kActorDektora, kSetFreeSlotI);
				Actor_Set_At_Waypoint(kActorDektora, 41, 0);
				Actor_Set_Goal_Number(kActorSteele, kGoalSteeleNRxxGoToSpinner);
			}
		} else {
			Actor_Set_Goal_Number(kActorMcCoy, kGoalMcCoyArrested);
		}
	}
	// return true;
}

void SceneScriptNR11::PlayerWalkedOut() {

}

void SceneScriptNR11::DialogueQueueFlushed(int a1) {
}

void SceneScriptNR11::actorSweepArea(int actorId, signed int frame) {
	float x;
	float y;
	float z;
	float coef;

	if (frame < 80) {
		coef = (frame - 60) / 20.0f;
		x = -106.66f * coef + 126.0f;
		y = 57.79f * coef + 0.85f;
		z = 42.0f * coef + -151.0f;
	} else {
		coef = (frame - 80) / 40.0f;
		x = -97.87f * coef + -150.0f;
		y = -10.8f * coef + 52.0f;
		z = 57.0f * coef + -232.0f;
	}
	Actor_Face_XYZ(actorId, x, y, z, true);
}

void SceneScriptNR11::untargetEverything() {
	Un_Combat_Target_Object("CLOTHING02");
	Un_Combat_Target_Object("BOX27");
	Un_Combat_Target_Object("BOX39");
#if BLADERUNNER_ORIGINAL_BUGS
	// in our bugfixes this object is removed
	Un_Combat_Target_Object("BOX44");
#endif // BLADERUNNER_ORIGINAL_BUGS
	Un_Combat_Target_Object("DRESS");
	Un_Combat_Target_Object("COATRACK");
	Un_Combat_Target_Object("COLUMN3 DETS");
	Un_Combat_Target_Object("COLUMN PIPE01");
	Un_Combat_Target_Object("RECTANGLE02");
	Un_Combat_Target_Object("COLUMN04");
	Un_Combat_Target_Object("COATRACK01");
	Un_Combat_Target_Object("SHIRT");
	Un_Combat_Target_Object("SKIRT 02");
	Un_Combat_Target_Object("CLOTHING B 03");
	Un_Combat_Target_Object("BUST BUST");
}

} // End of namespace BladeRunner
