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

void SceneScriptDR06::InitializeScene() {
	if (Game_Flag_Query(kFlagDR04toDR06)) {
		Setup_Scene_Information(-733.57f, 136.6f,  -968.64f,   0);
	} else {
		Setup_Scene_Information(-707.57f, 136.6f, -1132.64f, 472);
	}

	Scene_Exit_Add_2D_Exit(0, 601, 11, 639, 479, 1);
	if (Global_Variable_Query(kVariableChapter) > 3
	 && Game_Flag_Query(kFlagDR06UnlockedToUG16)
	) {
		Scene_Exit_Add_2D_Exit(1, 0, 272, 46, 477, 2);
	}
	if (_vm->_cutContent && !Game_Flag_Query(kFlagMcCoyCommentsOnFans)) {
		Scene_2D_Region_Add(0, 300,  80, 360, 190); // statue 1
		Scene_2D_Region_Add(1,  60, 120, 155, 260); // statue 2
		Scene_2D_Region_Add(2, 378, 270, 420, 320); // statue 3
	}

	Ambient_Sounds_Add_Looping_Sound(kSfxSKINBED1, 25, 0, 1);
	Ambient_Sounds_Add_Sound(kSfxSWEEP3, 5, 60, 20, 20, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSWEEP4, 5, 60, 20, 20, -100, 100, -101, -101, 0, 0);

	if (Game_Flag_Query(kFlagDR06JesterActive)) {
		Overlay_Play("DR06over", 1, true, false, 0);
		Ambient_Sounds_Add_Looping_Sound(kSfxJESTMOV1, 47, -75, 0);
	} else {
		Overlay_Play("DR06over", 0, true, false, 0);
	}
	if (Game_Flag_Query(kFlagDR06MannequinHeadOpen)) {
		Overlay_Play("DR06ovr2", 0, true, false, 0);
	}
}

void SceneScriptDR06::SceneLoaded() {
	Obstacle_Object("X2_ASHTRAY", true);
	Clickable_Object("X2_ASHTRAY");
	Clickable_Object("X2KEYBRD02");
	Clickable_Object("X2_MON01D01");
	Clickable_Object("X2_MON01A04");
	Clickable_Object("X2_TORSO04HIRES");
	Clickable_Object("BOX16");
	if (Actor_Clue_Query(kActorMcCoy, kClueEnvelope)) {
		Unclickable_Object("X2_TORSO04HIRES");
	}
}

bool SceneScriptDR06::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptDR06::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("BOX16", objectName)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -743.0f, 136.6f, -1091.0f, 0, true, false, false);
		Actor_Face_Object(kActorMcCoy, "BOX16", true);
		if (!Game_Flag_Query(kFlagDR06JesterActive)) {
			Overlay_Play("DR06over", 1, true, true, 0);
			Ambient_Sounds_Add_Looping_Sound(kSfxJESTMOV1, 47, -75, 0);
			Game_Flag_Set(kFlagDR06JesterActive);
		} else {
			Overlay_Play("DR06over", 0, true, true, 0);
			Ambient_Sounds_Remove_Looping_Sound(kSfxJESTMOV1, 0);
			Game_Flag_Reset(kFlagDR06JesterActive);
		}
		return true;
	}

	if (Object_Query_Click("X2_MON01A04", objectName)) {
		if (Actor_Clue_Query(kActorMcCoy, kClueAnsweringMachineMessage)) {
			Actor_Face_Object(kActorMcCoy, "X2_MON01A04", true);
			Actor_Says(kActorMcCoy, 8570, 13);
		} else {
			if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -684.94f, 136.6f, -1136.12f, 0, true, false, false)) {
				Actor_Face_Object(kActorMcCoy, "X2_MON01A04", true);
				Actor_Says(kActorAnsweringMachine, 10, 3);
				Actor_Says(kActorAnsweringMachine, 20, 3);
				Actor_Says(kActorAnsweringMachine, 30, 3);
				Actor_Says(kActorMcCoy, 1025, 13);
				Actor_Says(kActorSebastian, 0, 3);
				Actor_Says(kActorSebastian, 10, 3);
				Actor_Says(kActorSebastian, 20, 3);
				Actor_Says(kActorSebastian, 30, 3);
				Actor_Says(kActorSebastian, 40, 3);
				Actor_Says(kActorSebastian, 50, 3);
				Actor_Says(kActorAnsweringMachine, 40, 3);
				Actor_Says(kActorMcCoy, 1030, 13);
				Actor_Says(kActorAnsweringMachine, 50, 3);
				Actor_Clue_Acquire(kActorMcCoy, kClueAnsweringMachineMessage, true, kActorAnsweringMachine);
			}
		}
		return true;
	}

	if (Object_Query_Click("X2_MON01D01", objectName)) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -645.34f, 136.6f, -1047.37f, 0, true, false, false)) {
			Actor_Face_Heading(kActorMcCoy, 329, false);
			if ( Actor_Clue_Query(kActorMcCoy, kClueFolder)
			 &&  Actor_Clue_Query(kActorMcCoy, kClueGuzzaFramedMcCoy)
			 && !Game_Flag_Query(kFlagCallWithGuzza)
			) {
				Actor_Set_Goal_Number(kActorMcCoy, kGoalMcCoyCallWithGuzza);
				Game_Flag_Set(kFlagCallWithGuzza);
			} else if (!Game_Flag_Query(kFlagDR06VidphoneChecked)) {
				Actor_Voice_Over(770, kActorVoiceOver);
				Actor_Voice_Over(780, kActorVoiceOver);
				Actor_Voice_Over(790, kActorVoiceOver);
				Actor_Voice_Over(800, kActorVoiceOver);
				Game_Flag_Set(kFlagDR06VidphoneChecked);
			} else {
				Actor_Says(kActorMcCoy, 8570, 13);
			}
		}
		return true;
	}
#if BLADERUNNER_ORIGINAL_BUGS
	if ( Object_Query_Click("X2_KEYBRD02", objectName) // a bug? there is no X2_KEYBRD02 only X2KEYBRD02
	 && !Game_Flag_Query(kFlagDR06KeyboardChecked)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -655.57f, 136.6f, -1092.64f, 0, true, false, false);
		Actor_Face_Object(kActorMcCoy, "X2_KEYBRD02", true);
		Actor_Voice_Over(830, kActorVoiceOver);
		Actor_Voice_Over(840, kActorVoiceOver);
		Game_Flag_Set(kFlagDR06KeyboardChecked);
		return true;
	}
#else
	if ( Object_Query_Click("X2KEYBRD02", objectName) // Use X2KEYBRD02 in place of X2_KEYBRD02
	 && !Game_Flag_Query(kFlagDR06KeyboardChecked)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -655.57f, 136.6f, -1092.64f, 0, true, false, false);
		Actor_Face_Object(kActorMcCoy, "X2KEYBRD02", true);
		Actor_Voice_Over(830, kActorVoiceOver);
		Actor_Voice_Over(840, kActorVoiceOver);
		Game_Flag_Set(kFlagDR06KeyboardChecked);
		return true;
	}
#endif // BLADERUNNER_ORIGINAL_BUGS

	if (Object_Query_Click("X2_TORSO04HIRES", objectName)) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -700.0f, 136.6f, -1133.0f, 4, true, false, false)) {
			Actor_Face_Object(kActorMcCoy, "x2_TORSO04HIRES", true);
			if (Global_Variable_Query(kVariableDR06MannequinHeadOpened) > 12) {
				return true;
			}

			if (!Game_Flag_Query(kFlagDR06MannequinHeadOpen)) {
				Overlay_Play("DR06ovr2", 0, true, false, 0);
				Game_Flag_Set(kFlagDR06MannequinHeadOpen);
				Sound_Play(kSfxCEMENTL1, 100, 0, 0, 50);
				if (!Actor_Clue_Query(kActorMcCoy, kClueEnvelope)) {
					Actor_Voice_Over(850, kActorVoiceOver);
					Item_Pickup_Spin_Effect(kModelAnimationEnvelope, 171, 280);
					Actor_Voice_Over(860, kActorVoiceOver);
					Actor_Voice_Over(870, kActorVoiceOver);
					Actor_Voice_Over(880, kActorVoiceOver);
					Actor_Clue_Acquire(kActorMcCoy, kClueEnvelope, true, kActorLance);
					if (Query_Difficulty_Level() != kGameDifficultyEasy) {
						Global_Variable_Increment(kVariableChinyen, 200);
					}
				}
			} else {
				Overlay_Remove("DR06ovr2");
				Game_Flag_Reset(kFlagDR06MannequinHeadOpen);
				Sound_Play(kSfxCEMENTL2, 100, 0, 0, 50);
			}

			Global_Variable_Increment(kVariableDR06MannequinHeadOpened, 1);
			if (Global_Variable_Query(kVariableDR06MannequinHeadOpened) > 12) {
				Sound_Play(kSfxBIGPOT3, 100, 0, 0, 50);
				Unclickable_Object("X2_TORSO04HIRES");
			}
		}
		return true;
	}

	Actor_Face_Object(kActorMcCoy, "X2_MON01D01", true); //a bug? or why?
	Actor_Says(kActorMcCoy, 8525, 13);
	return true;
}

bool SceneScriptDR06::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptDR06::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptDR06::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -733.0f, 136.6f, -980.0f, 0, true, false, false)) {
			Game_Flag_Set(kFlagDR06toDR04);
			Set_Enter(kSetDR01_DR02_DR04, kSceneDR04);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -707.57f, 136.6f, -1132.64f, 0, true, false, false)) {
			Game_Flag_Set(kFlagDR06toUG16);
			Set_Enter(kSetUG16, kSceneUG16);
		}
		return true;
	}
	return false;
}

bool SceneScriptDR06::ClickedOn2DRegion(int region) {
if (_vm->_cutContent) {
		if (!Game_Flag_Query(kFlagMcCoyCommentsOnStatues) && (region == 0 || region == 1 || region == 2) ) {
			Game_Flag_Set(kFlagMcCoyCommentsOnStatues);
			Actor_Face_Heading(kActorMcCoy, 88, true);
			Actor_Voice_Over(810, kActorVoiceOver);
			Actor_Voice_Over(820, kActorVoiceOver);
			Scene_2D_Region_Remove(0);
			Scene_2D_Region_Remove(1);
			Scene_2D_Region_Remove(2);
			return true;
		}
	}
	return false;
}

void SceneScriptDR06::SceneFrameAdvanced(int frame) {
}

void SceneScriptDR06::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptDR06::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagDR04toDR06)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -729.57f, 136.6f, -1016.0f, 0, false, false, false);
	}
	Game_Flag_Reset(kFlagDR04toDR06);
	Game_Flag_Reset(kFlagUG16toDR06);
}

void SceneScriptDR06::PlayerWalkedOut() {
	Overlay_Remove("DR06over");
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(false);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void SceneScriptDR06::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
