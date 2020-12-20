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

enum kDR05Loops {
	kDR05LoopMainLoop      = 0, //  0 -  60
	kDR05LoopMainDestroyed = 2  // 61 - 121
};

void SceneScriptDR05::InitializeScene() {
	Setup_Scene_Information(-22.0f, 0.3f, 221.0f, 0);

	Game_Flag_Reset(kFlagDR04toDR05);

	Scene_Exit_Add_2D_Exit(0, 0, 38, 80, 467, 3);

	Ambient_Sounds_Add_Looping_Sound(kSfxSKINBED1, 25, 0, 1);

	if (!Game_Flag_Query(kFlagDR05BombExploded)) {
		Overlay_Play("DR05OVER", 0, true, false, 0);
	}
	if (Game_Flag_Query(kFlagDR05BombExploded)) {
		Scene_Loop_Set_Default(kDR05LoopMainDestroyed);
	} else {
		Scene_Loop_Set_Default(kDR05LoopMainLoop);
	}
}

void SceneScriptDR05::SceneLoaded() {
	Obstacle_Object("MAN PROPHI", true);
	Unobstacle_Object("BOX06", true);
	Unobstacle_Object("BOX183", true);
	Clickable_Object("T2 DOORWAY");

	if (!Game_Flag_Query(kFlagDR05BombExploded)) {
		Item_Add_To_World(kItemBomb, kModelAnimationBomb, kSetDR05, -1.57f, 31.33f, 75.21f, 540, 16, 16, true, true, false, true);
		if (Actor_Query_Goal_Number(kActorMoraji) == kGoalMorajiDefault) {
			Item_Add_To_World(kItemChain, kModelAnimationBadge, kSetDR05, 37.35f, 1.59f, 46.72f, 0, 20, 20, true, true, false, true); // TODO a bug? reusing still animation of kModelAnimationBadge
		}
	}
}

bool SceneScriptDR05::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptDR05::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("T2 DOORWAY", objectName)) {
		if (!Game_Flag_Query(kFlagNotUsed276)
		 &&  Actor_Query_Goal_Number(kActorMoraji) == kGoalMorajiDefault
		) {
			Actor_Face_Object(kActorMcCoy, "T2 DOORWAY", true);
			Actor_Says(kActorMcCoy, 1020, 14);
			Actor_Says(kActorMoraji, 90, 13);
		} else {
			if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 57.61f, 0.3f, 69.27f, 0, true, false, false)) {
				Actor_Face_Object(kActorMcCoy, "T2 DOORWAY", true);
				Actor_Says(kActorMcCoy, 8522, 13);
				Actor_Says(kActorMcCoy, 8521, 14);
			}
		}

		return true;
	}
	return false;
}

bool SceneScriptDR05::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptDR05::ClickedOnItem(int itemId, bool a2) {
	if (itemId == kItemBomb) {
		if (Player_Query_Combat_Mode()) {
			Game_Flag_Set(kFlagDR05BombWillExplode);
			Actor_Set_Goal_Number(kActorMoraji, kGoalMorajiChooseFate);
		} else if (!Game_Flag_Query(kFlagDR05BombExploded)
		        && !Loop_Actor_Walk_To_Item(kActorMcCoy, kItemBomb, 24, true, true)
		        &&  Actor_Query_Goal_Number(kActorMoraji) != kGoalMorajiRunOut
		) {
			if (!Actor_Query_Goal_Number(kActorMoraji)) {
				Actor_Says_With_Pause(kActorMcCoy, 1015, 0.1f, 12);
#if BLADERUNNER_ORIGINAL_BUGS
				Actor_Says(kActorMoraji, 70, 13);
#else
				// Moraji speaks and the place blows up immediately after
				Actor_Says_With_Pause(kActorMoraji, 70, 0.0f, 13);
#endif // BLADERUNNER_ORIGINAL_BUGS
			}
			Actor_Set_Goal_Number(kActorMoraji, kGoalMorajiChooseFate);
		}
		//return true; //bug?
	}

	if (itemId == kItemChain
	 && Player_Query_Combat_Mode()
	 && Actor_Query_Goal_Number(kActorMoraji) == kGoalMorajiDefault
	) {
		Overlay_Play("DR05OVER", 1, false, true, 0);
		Item_Remove_From_World(kItemChain);
		Game_Flag_Set(kFlagDR05ChainShot);
		Actor_Set_Goal_Number(kActorMoraji, kGoalMorajiFreed);
		Music_Play(kMusicMoraji, 71, 0, 0, -1, 0, 2);
		return true;
	}
	return false;
}

bool SceneScriptDR05::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -22.0f, 0.3f, 221.0f, 0, true, false, false)) {
			Game_Flag_Reset(kFlagNotUsed232);
			Game_Flag_Set(kFlagDR05toDR04);
			Set_Enter(kSetDR01_DR02_DR04, kSceneDR04);
		}
		return true;
	}
	return false;
}

bool SceneScriptDR05::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptDR05::SceneFrameAdvanced(int frame) {
	if (frame == 49) {
		Sound_Play(kSfxLABMISC3, Random_Query(50, 50), 80, 80, 50);
	}

	if (Game_Flag_Query(kFlagDR05BombWillExplode)) {
		Item_Remove_From_World(kItemBomb);
		Game_Flag_Reset(kFlagDR05BombWillExplode);
		Game_Flag_Set(kFlagDR05BombExploded);
		Actor_Set_Goal_Number(kActorMoraji, kGoalMorajiChooseFate);
	}
}

void SceneScriptDR05::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptDR05::PlayerWalkedIn() {
	if (!Game_Flag_Query(kFlagDR05ExplodedEntered)
	 && !Game_Flag_Query(kFlagDR05ChainShot)
	 &&  Game_Flag_Query(kFlagDR05BombExploded)
	) {
		Item_Remove_From_World(kItemChain);
	}

	if (Game_Flag_Query(kFlagDR05BombExploded)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -10.0f, 0.3f, 133.0f, 0, false, false, false);
		if (!Game_Flag_Query(kFlagDR05ExplodedEntered)) {
			Game_Flag_Set(kFlagDR05ExplodedEntered);
			if (Game_Flag_Query(kFlagSadikIsReplicant)) {
				Actor_Voice_Over(730, kActorVoiceOver);
				Actor_Voice_Over(740, kActorVoiceOver);
				Actor_Voice_Over(750, kActorVoiceOver);
				Actor_Voice_Over(760, kActorVoiceOver);
				Actor_Clue_Acquire(kActorMcCoy, kClueExpertBomber, true, -1);
			} else {
				Actor_Voice_Over(670, kActorVoiceOver);
				Actor_Voice_Over(680, kActorVoiceOver);
				Actor_Voice_Over(700, kActorVoiceOver);
				Actor_Voice_Over(710, kActorVoiceOver);
				Actor_Voice_Over(720, kActorVoiceOver);
				Actor_Clue_Acquire(kActorMcCoy, kClueAmateurBomber, true, -1);
			}
		}
	} else {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -10.0f, 0.3f, 133.0f, 0, false, true, false);
	}

	if (!Game_Flag_Query(kFlagDR05MorajiTalk)
	 &&  Actor_Query_Goal_Number(kActorMoraji) == kGoalMorajiDefault
	) {
		Actor_Face_Actor(kActorMcCoy, kActorMoraji, true);
		Actor_Says(kActorMcCoy, 1010, 13);
		Actor_Face_Item(kActorMcCoy, kItemBomb, true);
		Player_Set_Combat_Mode(true);
		Actor_Says(kActorMoraji, 60, 12);
		Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeIdle);
		Game_Flag_Set(kFlagDR05MorajiTalk);
		//return true;
	}
	//return false;
}

void SceneScriptDR05::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);

	if (Actor_Query_Goal_Number(kActorMoraji) == kGoalMorajiFreed
	 || Actor_Query_Goal_Number(kActorMoraji) == kGoalMorajiGetUp
	 || Actor_Query_Goal_Number(kActorMoraji) == kGoalMorajiScream
	) {
		Actor_Set_Goal_Number(kActorMoraji, kGoalMorajiRunOut);
		//return true;
	}
	//return false;
}

void SceneScriptDR05::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
