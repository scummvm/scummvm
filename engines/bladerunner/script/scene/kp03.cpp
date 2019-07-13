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

enum KP03Loops {
	kKP03MainLoopBombInactive  = 0,
	kKP03MainLoopBombActive    = 2,
	kKP03MainLoopBombExploding = 4,
	kKP03MainLoopBombExploded  = 5,
	kKP03MainLoopBombNoWire    = 7
};

void SceneScriptKP03::InitializeScene() {
	if (Game_Flag_Query(kFlagKP05toKP03)) {
		Setup_Scene_Information(   1.0f, -36.55f, 111.0f, 200);
	} else {
		Setup_Scene_Information(-321.0f, -36.55f,  26.0f, 350);
	}
	Scene_Exit_Add_2D_Exit(0,   0,   0,  30, 479, 3);
	Scene_Exit_Add_2D_Exit(1, 287, 104, 367, 255, 0);

	Ambient_Sounds_Add_Looping_Sound(kSfxRAIN10, 100, 1, 1);
	Ambient_Sounds_Add_Sound(kSfxSPIN2B,  60, 180, 16,  25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPIN3A,  60, 180, 16,  25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER2, 60, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER3, 50, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER4, 50, 180, 50, 100, 0, 0, -101, -101, 0, 0);

	if (Game_Flag_Query(kFlagKP03BombExploded)) {
		Scene_Loop_Set_Default(kKP03MainLoopBombExploded);
	} else if (Game_Flag_Query(kFlagKP03BombDisarmed)) {
		Scene_Loop_Set_Default(kKP03MainLoopBombNoWire);
	} else {
		Scene_Loop_Set_Default(kKP03MainLoopBombActive);
		Game_Flag_Set(kFlagKP03BombActive);
	}

	if (( Actor_Query_Goal_Number(kActorSteele) != kGoalSteeleGone
	  && !Game_Flag_Query(kFlagKP03BombExploded)
	  && !Game_Flag_Query(kFlagKP03BombDisarmed)
	 )
	 && ((Game_Flag_Query(kFlagMcCoyIsHelpingReplicants)
	   && Game_Flag_Query(kFlagKP05toKP03)
	  )
	  || (!Game_Flag_Query(kFlagMcCoyIsHelpingReplicants)
	   &&  Game_Flag_Query(kFlagKP01toKP03)
	  )
	 )
	) {
		Actor_Put_In_Set(kActorSteele, kSetKP03);
		Actor_Set_At_XYZ(kActorSteele, -300.0f, -36.55f, 26.0f, 350);
	}
}

void SceneScriptKP03::SceneLoaded() {
	Obstacle_Object("BRACK MID", true);
	Unobstacle_Object("OBSTACLE_REMOVE", true);
	Unobstacle_Object("BOX11", true);
	Unobstacle_Object("OBSTACLE05", true);
	Clickable_Object("BRACK MID");
}

bool SceneScriptKP03::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptKP03::ClickedOn3DObject(const char *objectName, bool combatMode) {
	Actor_Face_Object(kActorSteele, "BRACK MID", true);
	if ( Object_Query_Click("BRACK MID", objectName)
	 && !Game_Flag_Query(kFlagKP03BombExploded)
	) {
		if (combatMode) {
			Scene_Loop_Set_Default(kKP03MainLoopBombExploded);
			Scene_Loop_Start_Special(kSceneLoopModeOnce, kKP03MainLoopBombExploding, true);
			Actor_Change_Animation_Mode(kActorMcCoy, 39);
			Actor_Retired_Here(kActorMcCoy, 72, 18, 1, -1);
			Game_Flag_Set(kFlagKP03BombExploded);
			Game_Flag_Reset(kFlagKP03BombActive);
			return false;
		}

		if (Actor_Query_Goal_Number(kActorSteele) == kGoalSteeleKP03Walk) {
			Scene_Exits_Enable();
			saveSteele();
		} else {
			if (Game_Flag_Query(kFlagKP01toKP03)) {
				Loop_Actor_Walk_To_XYZ(kActorMcCoy, -137.0f, -36.55f, 26.0f, 0, false, true, false);
			} else if (Game_Flag_Query(kFlagKP05toKP03)) {
				Loop_Actor_Walk_To_XYZ(kActorMcCoy, -50.0f, -36.55f, 78.0f, 0, false, true, false);
			}

			if (Game_Flag_Query(kFlagKP03BombActive)) {
				Actor_Face_Object(kActorMcCoy, "BRACK MID", true);
				Game_Flag_Set(kFlagKP03BombDisarmed);
				Game_Flag_Reset(kFlagKP03BombActive);
				Scene_Loop_Set_Default(kKP03MainLoopBombNoWire);
				Scene_Loop_Start_Special(kSceneLoopModeOnce, kKP03MainLoopBombInactive, false);
				if (_vm->_cutContent) {
					Actor_Voice_Over(1100, kActorVoiceOver); // I'd been lucky this time.
				}
				Actor_Voice_Over(1110, kActorVoiceOver);
				Actor_Voice_Over(1120, kActorVoiceOver);
				if (_vm->_cutContent) {
					Actor_Voice_Over(1130, kActorVoiceOver); // One false step and...
				}
			} else {
				Actor_Face_Object(kActorMcCoy, "BRACK MID", true);
				Actor_Says(kActorMcCoy, 8580, 13);
			}
		}
		return true;
	}
	return false;
}

bool SceneScriptKP03::ClickedOnActor(int actorId) {
	if (actorId == kActorSteele
	 && Actor_Query_Goal_Number(kActorSteele) == kGoalSteeleKP03Walk
	) {
		Actor_Face_Object(kActorSteele, "BRACK MID", true);
		saveSteele();
	}
	return false;
}

bool SceneScriptKP03::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptKP03::ClickedOnExit(int exitId) {
	if (Actor_Query_Goal_Number(kActorSteele) == 410) {
		Actor_Set_Goal_Number(kActorSteele, kGoalSteeleKP03ShootMcCoy);
	} else {
		if (exitId == 0) {
			if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 1.0f, -36.55f, 111.0f, 0, true, false, false)) {
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Music_Stop(2);
				Game_Flag_Reset(kFlagKP01toKP03);
				Game_Flag_Reset(kFlagKP05toKP03);
				Game_Flag_Set(kFlagKP03toKP05);
				Set_Enter(kSetKP05_KP06, kSceneKP05);
			}
			return true;
		}

		if (exitId == 1) {
			if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -321.0f, -36.55f, 26.0f, 0, true, false, false)) {
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Music_Stop(2);
				Game_Flag_Reset(kFlagKP01toKP03);
				Game_Flag_Reset(kFlagKP05toKP03);
				Game_Flag_Set(kFlagKP03toKP01);
				Set_Enter(kSetKP01, kSceneKP01);
			}
			return true;
		}
	}
	return false;
}

bool SceneScriptKP03::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptKP03::SceneFrameAdvanced(int frame) {
	if (frame == 123) {
		Ambient_Sounds_Play_Sound(kSfxCRYEXPL1, 99, -60, 100, 99);
	}

	if ( Game_Flag_Query(kFlagKP03BombActive)
	 && !Game_Flag_Query(kFlagKP03BombDisarmed)
	 && !Game_Flag_Query(kFlagKP03BombExploded)
	) {
		float x, y, z;
		int bombTriggeredByActor = -1;

		Actor_Query_XYZ(kActorMcCoy, &x, &y, &z);
		if ((Game_Flag_Query(kFlagKP01toKP03)
			&& -130.0f < x
			)
			|| (Game_Flag_Query(kFlagKP05toKP03)
			&& -130.0f > x
			)
		) {
			bombTriggeredByActor = kActorMcCoy;
		}

		Actor_Query_XYZ(kActorSteele, &x, &y, &z);
		if (Game_Flag_Query(kFlagMcCoyIsHelpingReplicants)
		 && Actor_Query_Which_Set_In(kActorSteele) == kSetKP03
		) {
			if ((Game_Flag_Query(kFlagKP01toKP03)
			  && -130.0f > x
			 )
			 || (Game_Flag_Query(kFlagKP05toKP03)
			  && -130.0f < x
			 )
			) {
				bombTriggeredByActor = kActorSteele;
			}
		} else if ((Game_Flag_Query(kFlagKP01toKP03)
		         && -130.0f < x
		        )
		        || (Game_Flag_Query(kFlagKP05toKP03)
		         && -130.0f > x
		        )
		) {
			bombTriggeredByActor = kActorSteele;
		}

		if (bombTriggeredByActor != -1) {
			Scene_Loop_Set_Default(kKP03MainLoopBombExploded);
			Scene_Loop_Start_Special(kSceneLoopModeOnce, kKP03MainLoopBombExploding, true);
			Game_Flag_Set(kFlagKP03BombExploded);
			Game_Flag_Reset(kFlagKP03BombActive);
			Unclickable_Object("BRACK MID");
			Scene_Exits_Enable();

			if (bombTriggeredByActor == kActorSteele) {
				Actor_Set_Goal_Number(kActorSteele, kGoalSteeleKP03Exploded);
				Music_Play(kMusicCrysDie1, 25, 0, 1, -1, 0, 0);
				if (Actor_Query_Inch_Distance_From_Actor(kActorMcCoy, kActorSteele) <= 120) {
					bombTriggeredByActor = kActorMcCoy;
				}
			}

			if (bombTriggeredByActor == kActorSteele) {
				Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeHit);
			} else {
				Actor_Force_Stop_Walking(kActorMcCoy);
				Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeDie);
				Actor_Retired_Here(kActorMcCoy, 72, 18, true, -1);
			}
		}
	}
}

void SceneScriptKP03::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptKP03::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagKP05toKP03)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 1.0f, -36.55f, 87.0f, 0, false, false, false);
	}

	if (Actor_Query_Is_In_Current_Set(kActorSteele)
	 && Actor_Query_Goal_Number(kActorSteele) != kGoalSteeleKP03Dead
	) {
		if (Game_Flag_Query(kFlagMcCoyIsHelpingReplicants)) {
			if (Game_Flag_Query(kFlagKP05toKP03)) {
				Actor_Set_Goal_Number(kActorSteele, 410);
			}
			return;
		}

		if (!Game_Flag_Query(kFlagKP03BombExploded)
		 && !Game_Flag_Query(kFlagKP03BombDisarmed)
		 &&  Game_Flag_Query(kFlagKP01toKP03)
		) {
			Scene_Exits_Disable();
			Delay(1000);
			Actor_Set_Goal_Number(kActorSteele, kGoalSteeleKP03Walk);
		}
	}
}

void SceneScriptKP03::PlayerWalkedOut() {
}

void SceneScriptKP03::DialogueQueueFlushed(int a1) {
}

void SceneScriptKP03::saveSteele() {
	Player_Loses_Control();
	Actor_Says(kActorMcCoy, 2180, 14);
	Actor_Set_Goal_Number(kActorSteele, kGoalSteeleKP03StopWalking);
	Actor_Says(kActorSteele, 480, 60);
	Actor_Face_Object(kActorMcCoy, "BRACK MID", true);
	Actor_Says(kActorMcCoy, 2185, 14);
	Loop_Actor_Walk_To_XYZ(kActorSteele, -137.0f, -36.55f, 26.0f, 0, false, false, false);
	Actor_Face_Object(kActorSteele, "BRACK MID", true);
	Actor_Says(kActorSteele, 490, 58);
	Actor_Says(kActorMcCoy, 2190, 14);
	Actor_Says(kActorSteele, 500, 58);
	Actor_Says(kActorSteele, 510, 59);
	Actor_Says(kActorSteele, 520, 60);
	Game_Flag_Set(kFlagKP03BombDisarmed);
	Game_Flag_Reset(kFlagKP03BombActive);
	Scene_Loop_Set_Default(kKP03MainLoopBombNoWire);
	Scene_Loop_Start_Special(kSceneLoopModeOnce, kKP03MainLoopBombNoWire, false);
	Actor_Set_Goal_Number(kActorSteele, kGoalSteeleKP03Leave);
	Actor_Says(kActorMcCoy, 2195, 14);
	Ambient_Sounds_Play_Sound(kSfxLABMISC6, 40, -60, -60, 0);
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, 1.0f, -36.55f, 111.0f, 0, false, false, false);
	Actor_Set_Goal_Number(kActorSteele, kGoalSteeleKP05Enter);
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
	Game_Flag_Reset(kFlagKP01toKP03);
	Game_Flag_Reset(kFlagKP05toKP03);
	Game_Flag_Set(kFlagKP03toKP05);
	Set_Enter(kSetKP05_KP06, kSceneKP05);
	Player_Gains_Control();
}

} // End of namespace BladeRunner
