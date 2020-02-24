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

void SceneScriptNR02::InitializeScene() {
	playNextMusic();
	Setup_Scene_Information(-283.0f, -24.0f, 326.0f, 200);
	Game_Flag_Reset(kFlagNR01toNR02);

	Scene_Exit_Add_2D_Exit(0, 0, 105, 75, 291, 3);

	Ambient_Sounds_Add_Looping_Sound(kSfxBARAMB1, 50, 38, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX1,  3, 60, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX3,  3, 60, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX4,  3, 60, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX5,  3, 60, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX6,  3, 60, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX7,  3, 60, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxCLINK1,   3, 60, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxCLINK2,   3, 60, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxCLINK3,   3, 60, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxCLINK4,   3, 60, 25, 25, -100, 100, -101, -101, 0, 0);
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

void SceneScriptNR02::SceneLoaded() {
	Obstacle_Object("VID PHONE 01", true);
	Unobstacle_Object("VICTORIAN CHAIR", true);
	Unobstacle_Object("WALL CANDLES", true);
	Unobstacle_Object("STAIRS", true);
	Unobstacle_Object("BOX30", true);
	Unobstacle_Object("VID CAM COVER", true);
	Unobstacle_Object("VID CAM COVER01", true);
	Unobstacle_Object("VID OUTER GLASS", true);
	Unobstacle_Object("VID OUTER GLASS01", true);
	Unobstacle_Object("VID MAIN MONITOR", true);
	Unobstacle_Object("VID MAIN MONITOR01", true);
	Clickable_Object("VID PHONE 01");
	Clickable_Object("VID PHONE 02");
}

bool SceneScriptNR02::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptNR02::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("VID PHONE 01", objectName)
	 || Object_Query_Click("VID PHONE 02", objectName)
	) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -191.9f, -24.0f, 62.15f, 0, true, false, false)) {
			Actor_Face_Heading(kActorMcCoy, 13, false);
			if ( Actor_Clue_Query(kActorMcCoy, kClueFolder)
			 &&  Actor_Clue_Query(kActorMcCoy, kClueGuzzaFramedMcCoy)
			 && !Game_Flag_Query(kFlagCallWithGuzza)
			) {
				Actor_Set_Goal_Number(kActorMcCoy, kGoalMcCoyCallWithGuzza);
				Game_Flag_Set(kFlagCallWithGuzza);
			} else {
				Sound_Play(kSfxSPNBEEP9, 50, 0, 0, 50);
				Delay(1000);
				Sound_Play(kSfxVIDFONE1, 30, 0, 0, 50);
				Delay(1500);
				Sound_Play(kSfxVIDFONE1, 30, 0, 0, 50);
				Delay(1500);
				Sound_Play(kSfxVIDFONE1, 30, 0, 0, 50);
				Delay(1500);
				Sound_Play(kSfxSPNBEEP9, 50, 0, 0, 50);
				Delay(1000);
				Actor_Says(kActorMcCoy, 170, 14);
			}
		}
	}
	return false;
}

bool SceneScriptNR02::ClickedOnActor(int actorId) {
	if (actorId == kActorGordo
	 && Actor_Query_Goal_Number(kActorGordo) == kGoalGordoNR02WaitAtBar
	) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 67.37f, -24.0f, 389.32f, 0, true, false, false)) {
			Actor_Set_Goal_Number(kActorGordo, kGoalGordoNR02TalkToMcCoy);
		}
	}
	return false;
}

bool SceneScriptNR02::ClickedOnItem(int itemId, bool a2) {
	if (itemId == kItemGordosLighterReplicant
	 || itemId == kItemGordosLighterHuman
	) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 109.38f, -24.0f, 420.5f, 0, true, false, false)) {
			Actor_Face_Heading(kActorMcCoy, 423, false);
			if (itemId == kItemGordosLighterReplicant) {
				Item_Remove_From_World(kItemGordosLighterReplicant);
				Item_Pickup_Spin_Effect(kModelAnimationGordosLighterReplicant, 214, 380);
				Actor_Clue_Acquire(kActorMcCoy, kClueGordosLighterReplicant, true, -1);
			}

			if (itemId == kItemGordosLighterHuman) {
				Item_Remove_From_World(kItemGordosLighterHuman);
				Item_Pickup_Spin_Effect(kModelAnimationGordosLighterHuman, 214, 380);
				Actor_Clue_Acquire(kActorMcCoy, kClueGordosLighterHuman, true, -1);
			}
		}
	}
	return false;
}

bool SceneScriptNR02::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -283.0f, -24.0f, 326.0f, 0, true, false, false)) {
			if (Actor_Query_Goal_Number(kActorGordo) < kGoalGordoNR01WaitAndAttack
			 || Actor_Query_Goal_Number(kActorGordo) > kGoalGordoNR01WaitAndTakeHostage
			) {
				Music_Stop(2);
			}
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagNR02toNR01);
			Set_Enter(kSetNR01, kSceneNR01);
		}
		return true;
	}
	return false;
}

bool SceneScriptNR02::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptNR02::SceneFrameAdvanced(int frame) {
	// Gordo not running away
	if (!Music_Is_Playing()
	 && (Actor_Query_Goal_Number(kActorGordo) < kGoalGordoNR02TalkAboutMcCoy
	  || Actor_Query_Goal_Number(kActorGordo) > kGoalGordoNR02RunAway3
	 )
	) {
		playNextMusic();
	}
	//return false;
}

void SceneScriptNR02::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptNR02::PlayerWalkedIn() {
	if (Actor_Query_Goal_Number(kActorGordo) == kGoalGordoNR02WaitAtPodium) {
		Actor_Set_Goal_Number(kActorGordo, kGoalGordoNR02RunAway1);
	}

	if (Actor_Query_Goal_Number(kActorGordo) == kGoalGordoNR02WaitForMcCoy) {
		Actor_Set_Goal_Number(kActorGordo, kGoalGordoNR02NextAct);
	}

	if (Actor_Query_Goal_Number(kActorGordo) == kGoalGordoNR02TellJoke1) {
		Actor_Set_Goal_Number(kActorGordo, kGoalGordoNR02NextAct);
	}

	Loop_Actor_Walk_To_XYZ(kActorMcCoy, -203.0f, -24.0f, 334.0f, 0, false, false, false);
	//return false;
}

void SceneScriptNR02::PlayerWalkedOut() {
	Music_Stop(2);

	// Gordo on the podium
	if (Actor_Query_Goal_Number(kActorGordo) < kGoalGordoNR02TalkAboutMcCoy
	 && Actor_Query_Goal_Number(kActorGordo) >= kGoalGordoNR02NextAct
	) {
		Actor_Set_Goal_Number(kActorGordo, kGoalGordoNR02WaitForMcCoy);
	}
}

void SceneScriptNR02::DialogueQueueFlushed(int a1) {
	if (Player_Query_Current_Scene() == kSceneNR02
	 && Actor_Query_Goal_Number(kActorGordo) == kGoalGordoNR02TellJoke1
	) {
		Sound_Play(kSfxRIMSHOT1, 50, 0, 0, 50);
		Sound_Play(kSfxAUDLAFF3, 50, 0, 0, 50);
	}

	if (Player_Query_Current_Scene() == kSceneNR02
	 && Actor_Query_Goal_Number(kActorGordo) == kGoalGordoNR02TellJoke2
	) {
		Sound_Play(kSfxRIMSHOT2, 50, 0, 0, 50);
		Sound_Play(kSfxAUDLAFF5, 50, 0, 0, 50);
	}

	if (Player_Query_Current_Scene() == kSceneNR02
	 && Actor_Query_Goal_Number(kActorGordo) == kGoalGordoNR02TellJoke3
	) {
		Sound_Play(kSfxRIMSHOT5, 50, 0, 0, 50);
		Sound_Play(kSfxAUDLAFF6, 50, 0, 0, 50);
	}

	// Gordo on the podium
	if (Player_Query_Current_Scene() == kSceneNR02
	 && Actor_Query_Goal_Number(kActorGordo) > kGoalGordoNR02NextAct
	 && Actor_Query_Goal_Number(kActorGordo) < kGoalGordoNR02TalkAboutMcCoy
	) {
		Actor_Set_Goal_Number(kActorGordo, kGoalGordoNR02NextAct);
		return; // true;
	}

	// Gordo on the podium, but McCoy is outside
	if (Actor_Query_Goal_Number(kActorGordo) > kGoalGordoNR02NextAct
	 && Actor_Query_Goal_Number(kActorGordo) < kGoalGordoNR02TalkAboutMcCoy
	) {
		Actor_Set_Goal_Number(kActorGordo, kGoalGordoNR02WaitForMcCoy);
		return; // true;
	}
	//return false;
}

void SceneScriptNR02::playNextMusic() {
	int track = Global_Variable_Query(kVariableTaffyLewisMusic);
	if (track == 0) {
		Music_Play(kMusicTaffy2, 41, 0, 2, -1, 0, 0);
	} else if (track == 1) {
		Music_Play(kMusicTaffy3, 41, 0, 2, -1, 0, 0);
	} else if (track == 2) {
		Music_Play(kMusicTaffy4, 41, 0, 2, -1, 0, 0);
	}
	++track;
	if (track > 2) {
		track = 0;
	}
	Global_Variable_Set(kVariableTaffyLewisMusic, track);
}

} // End of namespace BladeRunner
