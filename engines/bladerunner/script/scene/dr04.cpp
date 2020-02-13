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

enum kDR04Loops {
	kDR04LoopPanFromDR01PostExplosion = 0, //   0 -  27
	kDR04LoopMainPostExplosion        = 1, //  28 -  88
	kDR04LoopPanFromDR01PreExplosion  = 3, //  89 - 116
	kDR04LoopMainPreExplosion         = 4, // 117 - 177
	kDR04LoopDR04Explosion            = 6  // 178 - 237
};

void SceneScriptDR04::InitializeScene() {
	if (Game_Flag_Query(kFlagDR05ViewExplosion)) {
		Setup_Scene_Information(    0.0f,    0.0f,    0.0f,   0);
	} else if (Game_Flag_Query(kFlagDR01toDR04)) {
		Setup_Scene_Information( -711.0f,  -0.04f,   70.0f, 472);
	} else if (Game_Flag_Query(kFlagDR05toDR04)) {
		Setup_Scene_Information(-1067.0f,   7.18f,  421.0f, 125);
	} else if (Game_Flag_Query(kFlagDR06toDR04)) {
		Setup_Scene_Information(-897.75f, 134.45f, 569.75f, 512);
	} else {
		Setup_Scene_Information( -810.0f,  -0.04f,  242.0f, 125);
	}

	Scene_Exit_Add_2D_Exit(0, 589,   0, 639, 479, 1);
	Scene_Exit_Add_2D_Exit(1, 443, 264, 488, 353, 0);
	Scene_Exit_Add_2D_Exit(2, 222, 110, 269, 207, 0);
	if (_vm->_cutContent) {
		Scene_Exit_Add_2D_Exit(3, 0, 440, 589, 479, 2);
	}

	Ambient_Sounds_Remove_All_Non_Looping_Sounds(false);
	Ambient_Sounds_Add_Looping_Sound(kSfxCTRAIN1, 50,    1,   1);
	Ambient_Sounds_Add_Looping_Sound(kSfxFIREBD1, 55, -100,   1);
	Ambient_Sounds_Add_Looping_Sound(kSfxHUMMER1, 28, -100, 100);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy,  0, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy, 20, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy, 40, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(kActorBlimpGuy, 50, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Sound(kSfxSPIN2A, 40, 180,  16,  25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPIN1A, 40, 180,  16,  25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDR1,  5,  80,  50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDR2,  5,  80,  50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDR3,  5,  80,  50, 100, 0, 0, -101, -101, 0, 0);

	if (Game_Flag_Query(kFlagDR05BombExploded)) {
		Scene_Loop_Set_Default(kDR04LoopMainPostExplosion);
	} else {
		Scene_Loop_Set_Default(kDR04LoopMainPreExplosion);
	}
	if (Game_Flag_Query(kFlagDR01toDR04)) {
		if (Game_Flag_Query(kFlagDR05BombExploded)) {
			Scene_Loop_Start_Special(kSceneLoopModeLoseControl, kDR04LoopPanFromDR01PostExplosion, false);
		} else {
			Scene_Loop_Start_Special(kSceneLoopModeLoseControl, kDR04LoopPanFromDR01PreExplosion, false);
		}
	}
}

void SceneScriptDR04::SceneLoaded() {
	Obstacle_Object("TRASH CAN WITH FIRE", true);
	Obstacle_Object("V2PYLON02", true);
	Obstacle_Object("V2PYLON04", true);
	Obstacle_Object("U2 CHEWDOOR", true);
	Obstacle_Object("MMTRASHCAN", true);
	Obstacle_Object("PARKMETR02", true);
	Obstacle_Object("TRANSFORMER 01", true);
	Obstacle_Object("TRANSFORMER 02", true);
	Obstacle_Object("PARKMETR01", true);
	Obstacle_Object("Z2ENTRYDR", true);
	Obstacle_Object("Z2DR2", true);
	Unclickable_Object("PARKMETR01");
	Unclickable_Object("Z2ENTRYDR");
	Unclickable_Object("Z2DR2");
}

bool SceneScriptDR04::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptDR04::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptDR04::ClickedOnActor(int actorId) {
	if ( actorId == kActorMoraji
	 && !Player_Query_Combat_Mode()
	) {
		if (Actor_Query_Goal_Number(kActorMoraji) == kGoalMorajiLayDown) {
			if (!Loop_Actor_Walk_To_Waypoint(kActorMcCoy, 109, 0, true, true)) {
				Actor_Face_Actor(kActorMcCoy, kActorMoraji, true);
				Actor_Says(kActorMcCoy, 945, 13);
				Actor_Says(kActorMoraji, 0, kAnimationModeTalk);
				Actor_Says(kActorMoraji, 10, kAnimationModeTalk);
				Actor_Says(kActorMcCoy, 950, 13);
				Actor_Says(kActorMoraji, 20, kAnimationModeTalk);
				Actor_Says(kActorMoraji, 30, kAnimationModeTalk);
				Actor_Says(kActorMcCoy, 955, 13);
				Actor_Says_With_Pause(kActorMoraji, 40, 0.0f, kAnimationModeTalk);
				Actor_Says(kActorMoraji, 50, kAnimationModeTalk);
				Actor_Clue_Acquire(kActorMcCoy, kClueMorajiInterview, true, kActorMoraji);
				Actor_Set_Goal_Number(kActorMoraji, kGoalMorajiDie);
				Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordArrivesToDR04); // Grayford arrives at scene of Moraji corpse
				return true;
			}
		}

		if (Actor_Query_Goal_Number(kActorMoraji) == kGoalMorajiDead) {
			if (!Loop_Actor_Walk_To_Actor(kActorMcCoy, kActorMoraji, 36, true, false)) {
#if BLADERUNNER_ORIGINAL_BUGS
				Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordStopPatrolToTalkToMcCoyAtDR04);
#else
				// bugfix: original code would result in this conversation repeating multiple times if:
				// Officer Grayford is at 103 (kGoalOfficerGrayfordTalkToMcCoyAndReportAtDR04) goal (asking "What do you know about this?"...
				// and the player skips the conversation fast.
				// So ask about a sheet (goal 106 (kGoalOfficerGrayfordStopPatrolToTalkToMcCoyAtDR04)) for Moraji only when Grayford starts patrolling (104, 105 goals)
				if (Actor_Query_Goal_Number(kActorOfficerGrayford) == kGoalOfficerGrayfordPatrolsAtDR04a
				 || Actor_Query_Goal_Number(kActorOfficerGrayford) == kGoalOfficerGrayfordPatrolsAtDR04b
				) {
					Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordStopPatrolToTalkToMcCoyAtDR04); // This goal reverts to the previous goal after finishing up
				}
#endif // BLADERUNNER_ORIGINAL_BUGS
				return true;
			}
		}
	}
	return false;
}

bool SceneScriptDR04::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptDR04::ClickedOnExit(int exitId) {
	if (Actor_Query_Goal_Number(kActorMoraji) == kGoalMorajiLayDown) {
		Actor_Force_Stop_Walking(kActorMcCoy);
		Actor_Set_Goal_Number(kActorMoraji, kGoalMorajiDie);
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordArrivesToDR04);
		return true;
	}

	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -761.0f, -0.04f, 97.0f, 0, true, false, false)) {
			Async_Actor_Walk_To_XYZ(kActorMcCoy, -683.0f, -0.04f, 43.0f, 0, false);
			Game_Flag_Set(kFlagDR04toDR01);
			Set_Enter(kSetDR01_DR02_DR04, kSceneDR01);
		}
		return true;
	}

	if (_vm->_cutContent) {
		if (exitId == 3) {
			if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -716.17f, 0.12f, 132.48f, 0, true, false, false)) {
				Async_Actor_Walk_To_XYZ(kActorMcCoy, -509.21f, 0.16f, 44.97f, 0, false);
				Game_Flag_Set(kFlagDR04toDR01);
				Set_Enter(kSetDR01_DR02_DR04, kSceneDR01);
			}
			return true;
		}
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -1067.0f, 7.18f, 421.0f, 0, true, false, false)) {
			Game_Flag_Set(kFlagNotUsed232);
			Game_Flag_Set(kFlagDR04toDR05);
			Set_Enter(kSetDR05, kSceneDR05);
		}
		return true;
	}

	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -851.0f, 6.98f, 560.0f, 0, true, false, false)) {
			Footstep_Sound_Override_On(3);
			Actor_Set_Immunity_To_Obstacles(kActorMcCoy, true);
			Actor_Face_Heading(kActorMcCoy, 512, false);
			Loop_Actor_Travel_Stairs(kActorMcCoy, 7, true, kAnimationModeIdle);
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -899.0f, 71.64f, 647.0f, 0, false, false, false);
			Actor_Face_Heading(kActorMcCoy, 0, false);
			Loop_Actor_Travel_Stairs(kActorMcCoy, 7, true, kAnimationModeIdle);
			Actor_Set_Immunity_To_Obstacles(kActorMcCoy, false);
			Footstep_Sound_Override_Off();
			Game_Flag_Set(kFlagDR04toDR06);
			Set_Enter(kSetDR06, kSceneDR06);
		}
		return true;
	}
	return true; //bug?
}

bool SceneScriptDR04::ClickedOn2DRegion(int region) {
	return false;
}

bool SceneScriptDR04::farEnoughFromExplosion() {
	float x, y, z;
	Actor_Query_XYZ(kActorMcCoy, &x, &y, &z);
	float blastRadius = 360.0f; // Original blast radius
	if (_vm->_cutContent && Query_Difficulty_Level() == kGameDifficultyEasy)
		blastRadius = 290.0f; // Allow the player to survive the bomb closer to the Dermo Design entrance
	return (x + 1089.94f) * (x + 1089.94f) + (z - 443.49f) * (z - 443.49f) >= (blastRadius * blastRadius);
}

void SceneScriptDR04::SceneFrameAdvanced(int frame) {
	if (Game_Flag_Query(kFlagDR05ViewExplosion)) {
		Game_Flag_Reset(kFlagDR05ViewExplosion);
		Game_Flag_Reset(kFlagDR05BombWillExplode);
		Scene_Loop_Set_Default(kDR04LoopMainPostExplosion);
		Scene_Loop_Start_Special(kSceneLoopModeOnce, kDR04LoopDR04Explosion, true);
		Music_Stop(4);
		Actor_Set_Goal_Number(kActorMoraji, kGoalMorajiPerished);
	} else {
		if (Game_Flag_Query(kFlagDR05BombWillExplode)) {
			Game_Flag_Reset(kFlagDR05BombWillExplode);
			Game_Flag_Set(kFlagDR05BombExploded);
			Scene_Loop_Set_Default(kDR04LoopMainPostExplosion);
			Scene_Loop_Start_Special(kSceneLoopModeOnce, kDR04LoopDR04Explosion, true);
			Item_Remove_From_World(kItemBomb);
		}

		switch (frame) {
		case 193:
			Sound_Play(kSfxINDXPLOD, 100, 0, 100, 50);
			Actor_Set_Goal_Number(kActorMoraji, kGoalMorajiChooseFate);
			Player_Loses_Control();
			Actor_Force_Stop_Walking(kActorMcCoy);
			if (farEnoughFromExplosion()) {
				if (Player_Query_Combat_Mode()) {
					Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeCombatHit);
				} else {
					Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeHit);
				}
			} else {
				Sound_Play_Speech_Line(kActorMcCoy, 9905, 100, 0, 99);
				Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeDie);
				Actor_Retired_Here(kActorMcCoy, 6, 6, true, -1);
			}
			Player_Gains_Control();
			break;

		case 235:
			if (Actor_Query_Goal_Number(kActorMoraji) != kGoalMorajiJump
			 && Actor_Query_Goal_Number(kActorMoraji) != kGoalMorajiLayDown
			 && Actor_Query_Goal_Number(kActorMoraji) != kGoalMorajiPerished
			) {
				Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordArrivesToDR04);
			}
			Scene_Exits_Enable();
			break;

		case 237:
			Overlay_Play("DR04OVER", 0, true, true, 0);
			break;
		}
	}
}

void SceneScriptDR04::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptDR04::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagDR05ViewExplosion)) {
		Player_Loses_Control();
		Delay(4000);
		Actor_Retired_Here(kActorMcCoy, 6, 6, 1, -1);
	} else {
		if ( Game_Flag_Query(kFlagDR05BombActivated)
		 && !Game_Flag_Query(kFlagDR05BombExploded)
		) {
			Scene_Exits_Disable();
		}

		if (Game_Flag_Query(kFlagDR06toDR04)) {
			Footstep_Sound_Override_On(3);
			Actor_Set_Immunity_To_Obstacles(kActorMcCoy, true);
			Actor_Face_Heading(kActorMcCoy, 512, false);
			Loop_Actor_Travel_Stairs(kActorMcCoy, 7, false, kAnimationModeIdle);
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -851.0f, 71.64f, 647.0f, 0, false, false, false);
			Actor_Face_Heading(kActorMcCoy, 0, false);
			Loop_Actor_Travel_Stairs(kActorMcCoy, 7, false, kAnimationModeIdle);
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -774.85f, 7.18f, 386.67f, 0, false, false, false);
			Actor_Set_Immunity_To_Obstacles(kActorMcCoy, false);
			Footstep_Sound_Override_Off();
		}
	}
	Game_Flag_Reset(kFlagDR01toDR04);
	Game_Flag_Reset(kFlagDR05toDR04);
	Game_Flag_Reset(kFlagDR06toDR04);
}

void SceneScriptDR04::PlayerWalkedOut() {
	Music_Stop(2);
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void SceneScriptDR04::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
