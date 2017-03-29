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

#include "bladerunner/script/scene.h"

namespace BladeRunner {

void SceneScriptDR06::InitializeScene() {
	if (Game_Flag_Query(230)) {
		Setup_Scene_Information(-733.57001f, 136.60001f, -968.64001f, 0);
	} else {
		Setup_Scene_Information(-707.57001f, 136.60001f, -1132.64f, 472);
	}
	Scene_Exit_Add_2D_Exit(0, 601, 11, 639, 479, 1);
	if (Global_Variable_Query(1) > 3 && Game_Flag_Query(715)) {
		Scene_Exit_Add_2D_Exit(1, 0, 272, 46, 477, 2);
	}
	Ambient_Sounds_Add_Looping_Sound(383, 25, 0, 1);
	Ambient_Sounds_Add_Sound(73, 5, 60, 20, 20, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(74, 5, 60, 20, 20, -100, 100, -101, -101, 0, 0);
	if (Game_Flag_Query(268)) {
		Overlay_Play("DR06over", 1, 1, 0, 0);
		Ambient_Sounds_Add_Looping_Sound(300, 47, -75, 0);
	} else {
		Overlay_Play("DR06over", 0, 1, 0, 0);
	}
	if (Game_Flag_Query(548)) {
		Overlay_Play("DR06ovr2", 0, 1, 0, 0);
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
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -743.0f, 136.6f, -1091.0f, 0, 1, false, 0);
		Actor_Face_Object(kActorMcCoy, "BOX16", true);
		if (!Game_Flag_Query(268)) {
			Overlay_Play("DR06over", 1, 1, 1, 0);
			Ambient_Sounds_Add_Looping_Sound(300, 47, -75, 0);
			Game_Flag_Set(268);
			return true;
		}
		Overlay_Play("DR06over", 0, 1, 1, 0);
		Ambient_Sounds_Remove_Looping_Sound(300, false);
		Game_Flag_Reset(268);
		return true;
	}
	if (Object_Query_Click("X2_MON01A04", objectName)) {
		if (Actor_Clue_Query(kActorMcCoy, kClueAnsweringMachineMessage)) {
			Actor_Face_Object(kActorMcCoy, "X2_MON01A04", true);
			Actor_Says(kActorMcCoy, 8570, 13);
		} else if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -684.94f, 136.6f, -1136.12f, 0, 1, false, 0)) {
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
			Actor_Clue_Acquire(kActorMcCoy, kClueAnsweringMachineMessage, 1, kActorAnsweringMachine);
		}
		return true;
	}
	if (Object_Query_Click("X2_MON01D01", objectName)) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -645.34f, 136.6f, -1047.37f, 0, 1, false, 0)) {
			Actor_Face_Heading(kActorMcCoy, 329, false);
			if (Actor_Clue_Query(kActorMcCoy, kClueFolder) && Actor_Clue_Query(kActorMcCoy, kClueGuzzaFramedMcCoy) && !Game_Flag_Query(670)) {
				Actor_Set_Goal_Number(kActorMcCoy, 350);
				Game_Flag_Set(670);
			} else if (Game_Flag_Query(280)) {
				Actor_Says(kActorMcCoy, 8570, 13);
			} else {
				Actor_Voice_Over(770, kActorVoiceOver);
				Actor_Voice_Over(780, kActorVoiceOver);
				Actor_Voice_Over(790, kActorVoiceOver);
				Actor_Voice_Over(800, kActorVoiceOver);
				Game_Flag_Set(280);
			}
		}
		return true;
	}
	if (Object_Query_Click("X2_KEYBRD02", objectName) && !Game_Flag_Query(278)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -655.57f, 136.6f, -1092.64f, 0, 1, false, 0);
		Actor_Face_Object(kActorMcCoy, "X2_KEYBRD02", true);
		Actor_Voice_Over(830, kActorVoiceOver);
		Actor_Voice_Over(840, kActorVoiceOver);
		Game_Flag_Set(278);
		return true;
	}
	if (Object_Query_Click("X2_TORSO04HIRES", objectName)) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -700.0f, 136.6f, -1133.0f, 4, 1, false, 0)) {
			Actor_Face_Object(kActorMcCoy, "x2_TORSO04HIRES", true);
			if (Global_Variable_Query(39) > 12) {
				return true;
			}
			if (Game_Flag_Query(548)) {
				Overlay_Remove("DR06ovr2");
				Game_Flag_Reset(548);
				Sound_Play(161, 100, 0, 0, 50);
			} else {
				Overlay_Play("DR06ovr2", 0, 1, 0, 0);
				Game_Flag_Set(548);
				Sound_Play(160, 100, 0, 0, 50);
				if (!Actor_Clue_Query(kActorMcCoy, kClueEnvelope)) {
					Actor_Voice_Over(850, kActorVoiceOver);
					Item_Pickup_Spin_Effect(944, 171, 280);
					Actor_Voice_Over(860, kActorVoiceOver);
					Actor_Voice_Over(870, kActorVoiceOver);
					Actor_Voice_Over(880, kActorVoiceOver);
					Actor_Clue_Acquire(kActorMcCoy, kClueEnvelope, 1, kActorLance);
					if (Query_Difficulty_Level() != 0) {
						Global_Variable_Increment(2, 200);
					}
				}
			}
			Global_Variable_Increment(39, 1);
			if (Global_Variable_Query(39) > 12) {
				Sound_Play(204, 100, 0, 0, 50);
				Unclickable_Object("X2_TORSO04HIRES");
			}
		}
		return true;
	}
	Actor_Face_Object(kActorMcCoy, "X2_MON01D01", true);
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
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -733.0f, 136.6f, -980.0f, 0, 1, false, 0)) {
			Game_Flag_Set(231);
			Set_Enter(7, 28);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -707.57f, 136.6f, -1132.64f, 0, 1, false, 0)) {
			Game_Flag_Set(552);
			Set_Enter(19, 100);
		}
		return true;
	}
	return false;
}

bool SceneScriptDR06::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptDR06::SceneFrameAdvanced(int frame) {
}

void SceneScriptDR06::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptDR06::PlayerWalkedIn() {
	if (Game_Flag_Query(230)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -729.57f, 136.6f, -1016.0f, 0, 0, false, 0);
	}
	Game_Flag_Reset(230);
	Game_Flag_Reset(551);
}

void SceneScriptDR06::PlayerWalkedOut() {
	Overlay_Remove("DR06over");
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(0);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void SceneScriptDR06::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
