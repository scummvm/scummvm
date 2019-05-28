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

void SceneScriptBB07::InitializeScene() {
	if (Game_Flag_Query(kFlagBB12toBB07)) {
		Setup_Scene_Information(-655.0f, 252.59f, -1136.0f, 323);
	} else if (Game_Flag_Query(kFlagBB06toBB07)) {
		Setup_Scene_Information(-551.0f, 252.59f, -1004.0f,  29);
		Game_Flag_Reset(kFlagBB06toBB07);
	} else {
		Setup_Scene_Information(-652.0f, 252.59f, -1018.0f, 268);
	}

	Scene_Exit_Add_2D_Exit(0,   0,  16,  51, 426, 3);
	Scene_Exit_Add_2D_Exit(1, 124, 101, 172, 305, 3);
	Scene_Exit_Add_2D_Exit(2, 282, 408, 476, 479, 2);

	Scene_2D_Region_Add(0, 308, 283, 354, 308);

	Ambient_Sounds_Add_Looping_Sound(kSfxUGBED1,   44, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxSTMLOOP7, 24, 0, 1);
	Ambient_Sounds_Add_Sound(kSfxSCARY4,  2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY5,  2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY6,  2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY7,  2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN1,  5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN2,  5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN3,  5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE1, 5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE2, 5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE3, 5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHAUNT1,  5,  50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxHAUNT2,  5,  50, 17, 27, -100, 100, -101, -101, 0, 0);

	Overlay_Play("BB07OVER", 0, true, false, 0);
}

void SceneScriptBB07::SceneLoaded() {
	Obstacle_Object("COUCH", true);
	Unobstacle_Object("X2MAINWALLLEFT01", true);
	Clickable_Object("PRINTER");
}

bool SceneScriptBB07::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptBB07::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("PRINTER", objectName)) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -472.63f, 252.59f, -1086.81f, 0, false, false, false)) {
			Actor_Face_Object(kActorMcCoy, "PRINTER", true);
			if ( Game_Flag_Query(kFlagBB07ElectricityOn)
			 && !Game_Flag_Query(kFlagBB07PrinterChecked)
			) {
				Actor_Voice_Over(130, kActorVoiceOver);
				Item_Pickup_Spin_Effect(kModelAnimationDNADataDisc, 439, 242);
				Actor_Voice_Over(140, kActorVoiceOver);
				Game_Flag_Set(kFlagBB07PrinterChecked);
				Actor_Clue_Acquire(kActorMcCoy, kClueDNASebastian, true, -1);
			} else if (Game_Flag_Query(kFlagBB07ElectricityOn)
			        && Game_Flag_Query(kFlagBB07PrinterChecked)
			) {
				Actor_Face_Object(kActorMcCoy, "PRINTER", true);
				Actor_Says(kActorMcCoy, 8570, 13);
			} else {
				Actor_Face_Object(kActorMcCoy, "PRINTER", true);
				Actor_Says(kActorMcCoy, 8575, 13);
			}
		}
	}
	return false;
}

bool SceneScriptBB07::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptBB07::ClickedOnItem(int itemId, bool a2) {
	if (itemId == kItemDNATyrell) { // this item is never added to this set/scene - a bug?
		if (!Loop_Actor_Walk_To_Item(kActorMcCoy, kItemDNATyrell, 36, true, false)) {
			Actor_Face_Item(kActorMcCoy, kItemDNATyrell, true);
			if (Game_Flag_Query(kFlagBB07ElectricityOn)) {
				Actor_Voice_Over(150, kActorVoiceOver);
				Actor_Voice_Over(160, kActorVoiceOver);
				Actor_Voice_Over(170, kActorVoiceOver);
			}
		}
	}
	return false;
}

bool SceneScriptBB07::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -615.0f, 252.59f, -1018.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Overlay_Remove("BB07OVER");
			Game_Flag_Set(kFlagBB07toBB05);
			Set_Enter(kSetBB05, kSceneBB05);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -619.0f, 252.59f, -1136.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Overlay_Remove("BB07OVER");
			Game_Flag_Set(kFlagBB07toBB12);
			Set_Enter(kSetBB12, kSceneBB12);
		}
		return true;
	}

	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -551.0f, 252.59f, -1004.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Overlay_Remove("BB07OVER");
			Game_Flag_Set(kFlagBB07toBB06);
			Set_Enter(kSetBB06_BB07, kSceneBB06);
		}
		return true;
	}
	return false;
}

bool SceneScriptBB07::ClickedOn2DRegion(int region) {
	if (region == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -568.63f, 252.59f, -1114.81f, 0, true, false, false)) {
			Actor_Face_Heading(kActorMcCoy, 229, false);
			if (!Game_Flag_Query(kFlagBB07ElectricityOn)) {
				Ambient_Sounds_Play_Sound(kSfxCOMPON1, 40, 20, 20, 99);
				Overlay_Play("BB07OVER", 1, false, true, 0);
				Overlay_Play("BB07OVER", 2, true, false, 0);
				Game_Flag_Set(kFlagBB07ElectricityOn);
				if (!Game_Flag_Query(kFlagBB07PrinterChecked)) {
					Actor_Says(kActorAnsweringMachine, 0, kAnimationModeTalk);
				}
			} else {
				Actor_Says(kActorMcCoy, 8585, 15);
			}
		}
	}
	return false;
}

void SceneScriptBB07::SceneFrameAdvanced(int frame) {
}

void SceneScriptBB07::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptBB07::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagBB05toBB07)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -594.0f, 252.59f, -1018.0f, 6, false, false, false);
		Game_Flag_Reset(kFlagBB05toBB07);
	}

	if (Game_Flag_Query(kFlagBB12toBB07)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -602.0f, 252.59f, -1124.0f, 6, false, false, false);
		Game_Flag_Reset(kFlagBB12toBB07);
	}
}

void SceneScriptBB07::PlayerWalkedOut() {
}

void SceneScriptBB07::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
