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

void SceneScriptPS01::InitializeScene() {
	Setup_Scene_Information(1872.0f, 16592.0f, -2975.0f, 200);
	Scene_Exit_Add_2D_Exit(0, 36, 194, 138, 326, 0);
	if (Game_Flag_Query(251)) {
		Scene_Exit_Add_2D_Exit(1, 344, 288, 584, 384, 2);
	}
	Ambient_Sounds_Add_Looping_Sound(381, 100, 1, 1);
	Ambient_Sounds_Add_Sound(68, 60, 180, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(69, 60, 180, 16, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(375, 60, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(376, 50, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(377, 50, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	if (Game_Flag_Query(130)) {
		if (Game_Flag_Query(251)) {
			Scene_Loop_Set_Default(1);
		} else {
			Scene_Loop_Set_Default(5);
		}
	} else {
		Actor_Set_Invisible(kActorMcCoy, true);
		Game_Flag_Set(273);
		Scene_Loop_Start_Special(0, 0, 0);
		Scene_Loop_Set_Default(1);
		Player_Loses_Control();
	}
}

void SceneScriptPS01::SceneLoaded() {
	Obstacle_Object("TUBE81", true);
	if (Game_Flag_Query(251)) {
		Unobstacle_Object("Barrier Obstacle", true);
	}
	Unobstacle_Object("BOX38", true);
	Unobstacle_Object("TUBE81", true);
}

bool SceneScriptPS01::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptPS01::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptPS01::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptPS01::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptPS01::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 1920.0f, 16581.0f, -2653.0f, 12, 1, false, 0)) {
			Game_Flag_Set(718);
			Set_Enter(62, 66);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 1877.9f, 16592.0f, -2975.0f, 0, 1, false, 0)) {
			Actor_Set_At_XYZ(kActorMcCoy, 1872.0f, 16592.0f, -2975.0f, 870);
			Game_Flag_Reset(176);
			Game_Flag_Reset(182);
			Game_Flag_Reset(179);
			Game_Flag_Reset(178);
			Game_Flag_Reset(180);
			Game_Flag_Reset(261);
			Game_Flag_Reset(177);
			Game_Flag_Reset(258);
			int spinnerDest = Spinner_Interface_Choose_Dest(3, 1);
			switch (spinnerDest) {
			case 2:
				Game_Flag_Set(182);
				Game_Flag_Reset(251);
				Game_Flag_Set(249);
				Set_Enter(69, 78);
				Scene_Loop_Start_Special(1, 4, 1);
				break;
			case 1:
				Game_Flag_Set(179);
				Game_Flag_Reset(251);
				Game_Flag_Set(250);
				Set_Enter(49, 48);
				Scene_Loop_Start_Special(1, 4, 1);
				break;
			case 3:
				Game_Flag_Set(176);
				Game_Flag_Reset(251);
				Game_Flag_Set(248);
				Set_Enter(4, 13);
				Scene_Loop_Start_Special(1, 4, 1);
				break;
			case 5:
				Game_Flag_Set(261);
				Game_Flag_Reset(251);
				Game_Flag_Set(307);
				Set_Enter(17, 82);
				Scene_Loop_Start_Special(1, 4, 1);
				break;
			case 4:
				Game_Flag_Set(180);
				Game_Flag_Reset(251);
				Game_Flag_Set(252);
				Set_Enter(0, 0);
				Scene_Loop_Start_Special(1, 4, 1);
				break;
			case 6:
				Game_Flag_Set(177);
				Game_Flag_Reset(251);
				Game_Flag_Set(253);
				Set_Enter(7, 25);
				Scene_Loop_Start_Special(1, 4, 1);
				break;
			case 7:
				Game_Flag_Set(258);
				Game_Flag_Reset(251);
				Game_Flag_Set(254);
				Set_Enter(20, 2);
				Scene_Loop_Start_Special(1, 4, 1);
				break;
			case 8:
				Game_Flag_Set(181);
				Game_Flag_Reset(251);
				Game_Flag_Set(255);
				Set_Enter(54, 54);
				Scene_Loop_Start_Special(1, 4, 1);
				break;
			case 9:
				Game_Flag_Set(257);
				Game_Flag_Reset(251);
				Game_Flag_Set(256);
				Set_Enter(37, 34);
				Scene_Loop_Start_Special(1, 4, 1);
				break;
			default:
				Actor_Face_Heading(kActorMcCoy, 870, false);
				Game_Flag_Set(178);
				Game_Flag_Set(273);
				Player_Loses_Control();
				Scene_Loop_Start_Special(2, 3, 1);
				break;
			}
		}
		return true;
	}
	return false;
}

bool SceneScriptPS01::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptPS01::SceneFrameAdvanced(int frame) {
	if (frame == 71 || frame == 188) {
		Sound_Play(116, 100, 40, 0, 50);
	}
	if (frame == 108 || frame == 228) {
		Sound_Play(119, 100, 40, 0, 50);
	}
	if (frame == 1) {
		Sound_Play(118, 40, 0, 0, 50);
	}
	if (frame == 76) {
		Sound_Play(121, 50, 0, 0, 50);
	}
	if (frame == 192) {
		Sound_Play(120, 50, 0, 0, 50);
	}
	if (frame == 59) {
		Sound_Play(122, 15, 0, 0, 50);
	}
	if (frame == 275) {
		Sound_Play(117, 40, 0, 0, 50);
	}
	if (!Game_Flag_Query(273)) {
		switch (frame) {
		case 196:
			Actor_Face_Heading(kActorMcCoy, 870, false);
			Actor_Set_Frame_Rate_FPS(kActorMcCoy, -1);
			Actor_Change_Animation_Mode(kActorMcCoy, 41);
			break;
		case 220:
			Actor_Set_Frame_Rate_FPS(kActorMcCoy, 0);
			break;
		case 240:
			Actor_Set_Frame_Rate_FPS(kActorMcCoy, -2);
			break;
		}
		//return true;
		return;
	}
	if (frame == 75) {
		Actor_Face_Heading(kActorMcCoy, 870, false);
		Actor_Change_Animation_Mode(kActorMcCoy, 42);
		//return true;
		return;
	}
	if (frame == 119) {
		Game_Flag_Reset(273);
		Player_Gains_Control();
		//return true;
		return;
	}
	if (frame > 195) {
		if (frame == 239) {
			Game_Flag_Reset(273);
			Player_Gains_Control();
		}
		//return true;
		return;
	}
	if (frame == 181) {
		Actor_Face_Heading(kActorMcCoy, 870, false);
		Actor_Change_Animation_Mode(kActorMcCoy, 42);
	} else if (frame == 182) {
		Actor_Set_Frame_Rate_FPS(kActorMcCoy, 0);
	} else if (frame == 195) {
		Actor_Set_Frame_Rate_FPS(kActorMcCoy, -2);
	}
	//return true;
	return;
}

void SceneScriptPS01::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptPS01::PlayerWalkedIn() {
	if (Game_Flag_Query(130)) {
		Actor_Set_At_XYZ(kActorMcCoy, 1920.0f, 16581.0f, -2653.0f, 150);
		Game_Flag_Reset(130);
	}
	//return false;
}

void SceneScriptPS01::PlayerWalkedOut() {
	Actor_Set_Invisible(kActorMcCoy, false);
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
	if (!Game_Flag_Query(718) && Global_Variable_Query(1) == 1) {
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		Outtake_Play(38, 1, -1);
	}
}

void SceneScriptPS01::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
