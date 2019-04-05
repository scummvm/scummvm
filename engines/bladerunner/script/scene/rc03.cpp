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

void SceneScriptRC03::InitializeScene() {
	if (Game_Flag_Query(kFlagRC01toRC03)) {
		Setup_Scene_Information(298.0f, -4.0f, 405.0f, 800);
		Game_Flag_Reset(kFlagRC01toRC03);
	} else if (Game_Flag_Query(kFlagAR02toRC03)) {
		Setup_Scene_Information(-469.0f, -4.0f, 279.0f, 250);
	} else if (Game_Flag_Query(kFlagUG01toRC03)) {
		Setup_Scene_Information(147.51f, -4.0f, 166.48f, 500);
		if (!Game_Flag_Query(kFlagRC03UnlockedToUG01)) {
			Game_Flag_Set(kFlagRC03UnlockedToUG01);
		}
	} else if (Game_Flag_Query(kFlagHC04toRC03)) {
		Setup_Scene_Information(-487.0f, 1.0f, 116.0f, 400);
	} else if (Game_Flag_Query(kFlagRC04toRC03)) {
		Setup_Scene_Information(-22.0f, 1.0f, -63.0f, 400);
	} else {
		Setup_Scene_Information(0.0f, 0.0f, 0.0f, 0);
	}

	Scene_Exit_Add_2D_Exit(0, 610, 0, 639, 479, 1);
	Scene_Exit_Add_2D_Exit(1,   0, 0,  30, 479, 3);
	if (Game_Flag_Query(kFlagRC03UnlockedToUG01)) {
		Scene_Exit_Add_2D_Exit(2, 524, 350, 573, 359, 2);
	}
	Scene_Exit_Add_2D_Exit(3,  85, 255, 112, 315, 0);
	Scene_Exit_Add_2D_Exit(4, 428, 260, 453, 324, 0);

	Ambient_Sounds_Add_Looping_Sound(54, 50, 0, 1);
	Ambient_Sounds_Add_Sound(82, 5, 30, 40, 70, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(83, 5, 30, 40, 75, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(84, 5, 30, 40, 70, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy,  0, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy, 20, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy, 40, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy, 50, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Sound(68,  60, 180, 16,  25,    0,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(69,  60, 180, 16,  25,    0,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(375, 60, 180, 50, 100,    0,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(376, 50, 180, 50, 100,    0,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(377, 50, 180, 50, 100,    0,   0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(181,  5,  70, 12,  12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(182,  5,  70, 12,  12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(183,  5,  70, 12,  12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(184,  5,  70, 12,  12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(185,  5,  70, 12,  12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(186,  5,  70, 12,  12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(188,  5,  70, 12,  12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(189,  5,  70, 12,  12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(190,  5,  70, 12,  12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(191,  5,  70, 12,  12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(192,  5,  70, 12,  12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(193,  5,  70, 12,  12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(194,  5,  70, 12,  12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(195,  5,  70, 12,  12, -100, 100, -101, -101, 0, 0);

	if (Game_Flag_Query(kFlagHC04toRC03)
	 && Actor_Query_Goal_Number(kActorIzo) != kGoalIzoWaitingAtRC03
	) {
		if (Random_Query(1, 3) == 1) { // enhancement: don't always play this scene when exiting Hawker's Circle
			Scene_Loop_Start_Special(kSceneLoopModeLoseControl, 0, false);
		}
	}
	Scene_Loop_Set_Default(1);
}

void SceneScriptRC03::SceneLoaded() {
	Obstacle_Object("Box-Streetlight01", true);
	Obstacle_Object("Box-Streetlight02", true);
	Obstacle_Object("Parking Meter 01", true);
	Obstacle_Object("Parking Meter 02", true);
	Obstacle_Object("Parking Meter 03", true);
	Obstacle_Object("Trash can with fire", true);
	Obstacle_Object("Baricade01", true);
	Obstacle_Object("Foreground Junk01", true);
	Obstacle_Object("Steam01", true);
	Obstacle_Object("Steam02", true);
	Obstacle_Object("Box-BBcolumn01", true);
	Obstacle_Object("Box-BBcolumn02", true);
	Obstacle_Object("Box-BBcolumn03", true);
	Obstacle_Object("Box-BBcolumn04", true);
	Obstacle_Object("Box-BBbuilding01", true);
	Obstacle_Object("Box-BBbuilding02", true);
	Obstacle_Object("Box-BBbuilding03", true);
	Obstacle_Object("Box-BBbuilding04", true);
	Unclickable_Object("BOX-BBBUILDING01");
	Unclickable_Object("BOX-BBBUILDING02");
	Unclickable_Object("BOX-BBBUILDING03");
	Unclickable_Object("BOX-BBBUILDING04");
	Unclickable_Object("BOX-STREETLIGHT01");
	Unclickable_Object("BOX-STREETLIGHT02");
	Unclickable_Object("BOX-BBCOLUMN01");
	Unclickable_Object("BOX-BBCOLUMN02");
	Unclickable_Object("BOX-BBCOLUMN03");
	Unclickable_Object("BOX-BBCOLUMN04");
	Unclickable_Object("PARKING METER 02");
	Unclickable_Object("PARKING METER 03");
	Unclickable_Object("TRASH CAN WITH FIRE");
	Unclickable_Object("BARICADE01");
	Unclickable_Object("FOREGROUND JUNK01");
}

bool SceneScriptRC03::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptRC03::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptRC03::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptRC03::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptRC03::ClickedOnExit(int exitId) {
	if (exitId == 0) { // To Runciter's shop
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 298.0f, -4.0f, 405.0f, 0, true, false, 0)) {
			if (Game_Flag_Query(kFlagRC04McCoyShotBob)) {
				Game_Flag_Set(kFlagBulletBobDead);
			}
			Game_Flag_Set(kFlagRC03toRC01);
			Set_Enter(kSetRC01, kSceneRC01);
#if BLADERUNNER_ORIGINAL_BUGS
			Actor_Set_Goal_Number(kActorDektora, kGoalDektoraStartWalkingAround);
#else
			// Restrict Dektora's "walking around" goal only in Chapter 2
			// this is a bug fix for the case where Dektora's goal gets reset from kGoalDektoraGone in Chapter 4/5
			if (Global_Variable_Query(kVariableChapter) == 2 ) {
				Actor_Set_Goal_Number(kActorDektora, kGoalDektoraStartWalkingAround);
			}
#endif // BLADERUNNER_ORIGINAL_BUGS
		}
		return true;
	}
	if (exitId == 1) { // to Animoid Row (Scorpion/Insect Lady)
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -469.0f, -4.0f, 279.0f, 0, true, false, 0)) {
			if (Game_Flag_Query(kFlagRC04McCoyShotBob)) {
				Game_Flag_Set(kFlagBulletBobDead);
			}
			Game_Flag_Set(kFlagRC03toAR02);
			Game_Flag_Reset(kFlagMcCoyInRunciters);
			Game_Flag_Set(kFlagMcCoyInAnimoidRow);
			Set_Enter(kSetAR01_AR02, kSceneAR02);
		}
		return true;
	}
	if (exitId == 2) { // to sewers
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 147.51f, -4.0f, 166.48f, 0, true, false, 0)) {
			Game_Flag_Set(kFlagRC03toUG01);
			Game_Flag_Reset(kFlagMcCoyInRunciters);
			Game_Flag_Set(kFlagMcCoyInUnderground);
			if (Game_Flag_Query(kFlagRC04McCoyShotBob)) {
				Game_Flag_Set(kFlagBulletBobDead);
			}
			Set_Enter(kSetUG01, kSceneUG01);
#if BLADERUNNER_ORIGINAL_BUGS
			Actor_Set_Goal_Number(kActorDektora, kGoalDektoraStartWalkingAround);
#else
			// Restrict Dektora's "walking around" goal only in Chapter 2
			// this is a bug fix for the case where Dektora's goal gets reset from kGoalDektoraGone in Chapter 4/5
			if (Global_Variable_Query(kVariableChapter) == 2 ) {
				Actor_Set_Goal_Number(kActorDektora, kGoalDektoraStartWalkingAround);
			}
#endif // BLADERUNNER_ORIGINAL_BUGS
		}
		return true;
	}
	if (exitId == 3) { // to Hawker's Circle (Mama Izabella's Kingston Kitchen)
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -487.0f, 1.0f, 116.0f, 0, true, false, 0)) {
			Game_Flag_Set(kFlagRC03toHC04);
			Game_Flag_Reset(kFlagMcCoyInRunciters);
			Game_Flag_Set(kFlagMcCoyInHawkersCircle);
			if (Game_Flag_Query(kFlagRC04McCoyShotBob)) {
				Game_Flag_Set(kFlagBulletBobDead);
			}
			Set_Enter(kSetHC01_HC02_HC03_HC04, kSceneHC04);
#if BLADERUNNER_ORIGINAL_BUGS
			Actor_Set_Goal_Number(kActorDektora, kGoalDektoraStartWalkingAround);
#else
			// Restrict Dektora's "walking around" goal only in Chapter 2
			// this is a bug fix for the case where Dektora's goal gets reset from kGoalDektoraGone in Chapter 4/5
			if (Global_Variable_Query(kVariableChapter) == 2 ) {
				Actor_Set_Goal_Number(kActorDektora, kGoalDektoraStartWalkingAround);
			}
#endif // BLADERUNNER_ORIGINAL_BUGS
		}
		return true;
	}
	if (exitId == 4) { // To Bullet Bob's
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -22.0f, 1.0f, -63.0f, 0, true, false, 0)) {
			if (Global_Variable_Query(kVariableChapter) == 3
			 || Global_Variable_Query(kVariableChapter) == 5
			 || Game_Flag_Query(kFlagBulletBobDead)
			) {
				Actor_Says(kActorMcCoy, 8522, 14);
			} else {
				Game_Flag_Set(kFlagRC03toRC04);
				Set_Enter(kSetRC04, kSceneRC04);
			}
		}
		return true;
	}
	return false;
}

bool SceneScriptRC03::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptRC03::SceneFrameAdvanced(int frame) {
	if (frame == 1) {
		Sound_Play(kSfxTRUCKBY1, Random_Query(33, 33), 100, -100, 50);
	}
	if (frame == 15) {
		Sound_Play(kSfxCHEVBY1,  Random_Query(50, 50), -100, 100, 50);
	}
}

void SceneScriptRC03::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptRC03::talkWithSteele() {
	Actor_Face_Actor(kActorSteele, kActorMcCoy, true);
	Actor_Says(kActorSteele, 1820, 3);
	Actor_Face_Actor(kActorMcCoy, kActorSteele, true);
	Actor_Says(kActorMcCoy, 4815, 14);
	Actor_Says(kActorSteele, 1830, 3);
	Actor_Says(kActorSteele, 1840, 3);
	Actor_Says(kActorMcCoy, 4820, 12);
	Actor_Says(kActorSteele, 1850, 3);
	Actor_Says(kActorSteele, 1950, 3);
	Actor_Says(kActorMcCoy, 4835, 14);
	Actor_Says(kActorSteele, 1960, 3);
	Actor_Says(kActorSteele, 1980, 3);
	Actor_Says(kActorMcCoy, 4840, 15);
	Actor_Says(kActorSteele, 1990, 3);
	Actor_Says(kActorSteele, 2000, 3);
}

void SceneScriptRC03::PlayerWalkedIn() {
	if (Actor_Query_Goal_Number(kActorIzo) == kGoalIzoWaitingAtRC03) {
		Scene_Exits_Disable();
		if (Game_Flag_Query(kFlagUG01toRC03)) {
			Player_Set_Combat_Mode(false);
			Player_Loses_Control();
			Actor_Set_At_XYZ(kActorMcCoy, 147.51f, -4.0f, 166.48f, 500);
			Actor_Put_In_Set(kActorIzo, kSetRC03);
			Actor_Set_At_XYZ(kActorIzo, 196.0f, -4.0f, 184.0f, 775);
			Actor_Face_Actor(kActorIzo, kActorMcCoy, true);
			Actor_Face_Actor(kActorMcCoy, kActorIzo, true);
			Actor_Change_Animation_Mode(kActorIzo, kAnimationModeCombatIdle);
			Actor_Says_With_Pause(kActorIzo, 630, 0, -1);
			Actor_Says_With_Pause(kActorIzo, 640, 0, -1);
			Actor_Says_With_Pause(kActorIzo, 650, 0, -1);
			if (Game_Flag_Query(kFlagIzoIsReplicant) ) {
				Actor_Set_Goal_Number(kActorSteele, kGoalSteeleApprehendIzo);
			}
			Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeDodge);
			Loop_Actor_Walk_To_XYZ(kActorIzo, 180.0f, -4.0f, 184.0f, 0, false, false, 0);
			Actor_Change_Animation_Mode(kActorIzo, 6);
			if (!Game_Flag_Query(kFlagIzoIsReplicant)) {
				Actor_Set_Goal_Number(kActorSteele, kGoalSteeleApprehendIzo);
			}
			Player_Gains_Control();
		} else {
			Actor_Put_In_Set(kActorIzo, kSetRC03);
			Actor_Set_At_XYZ(kActorIzo, -226.0f, 1.72f, 86.0f, 0);
			Actor_Set_Targetable(kActorIzo, true);
			Actor_Set_Goal_Number(kActorIzo, kGoalIzoRC03Walk);
		}
	}

	if (Actor_Query_Goal_Number(kActorIzo) == kGoalIzoEscape) {
		Player_Loses_Control();
		Actor_Set_Goal_Number(kActorSteele, 200);
		Actor_Put_In_Set(kActorSteele, kSetRC03);
		if (Game_Flag_Query(kFlagUG01toRC03)
		 || Game_Flag_Query(kFlagRC04toRC03)
		) {
			Actor_Set_At_Waypoint(kActorSteele, 175, 0);
		} else {
			Actor_Set_At_Waypoint(kActorSteele, 203, 0);
		}
		talkWithSteele();
		Async_Actor_Walk_To_Waypoint(kActorSteele, 174, 0, 0);
		Actor_Set_Goal_Number(kActorIzo, 200);
		Player_Gains_Control();
	}
	Game_Flag_Reset(kFlagUG01toRC03);
	Game_Flag_Reset(kFlagAR02toRC03);
	Game_Flag_Reset(kFlagHC04toRC03);
	Game_Flag_Reset(kFlagRC04toRC03);

	if (Global_Variable_Query(kVariableChapter) == 1
	 || Global_Variable_Query(kVariableChapter) == 2
	) {
		Actor_Set_Goal_Number(kActorDektora, kGoalDektoraStopWalkingAround);
	}
}

void SceneScriptRC03::PlayerWalkedOut() {
	if (Actor_Query_Goal_Number(kActorIzo) == kGoalIzoDie) {
		Actor_Set_Goal_Number(kActorIzo, kGoalIzoDieHidden);
	}
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void SceneScriptRC03::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
