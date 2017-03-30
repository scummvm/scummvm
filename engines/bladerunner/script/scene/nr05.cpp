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

void SceneScriptNR05::InitializeScene() {
	if (Game_Flag_Query(547)) {
		Scene_Loop_Start_Special(0, 0, 0);
		Scene_Loop_Set_Default(1);
		Setup_Scene_Information(-777.56f, 0.0f, -166.86f, 0);
	} else if (Game_Flag_Query(536)) {
		Setup_Scene_Information(-456.0f, 0.0f, -611.0f, 0);
	} else {
		Setup_Scene_Information(-527.0f, 1.57f, -406.0f, 649);
	}
	Scene_Exit_Add_2D_Exit(0, 459, 147, 639, 290, 1);
	if (Game_Flag_Query(620)) {
		Scene_Exit_Add_2D_Exit(1, 0, 0, 30, 479, 3);
	}
	Ambient_Sounds_Add_Looping_Sound(280, 50, 38, 0);
	Ambient_Sounds_Add_Sound(252, 3, 60, 20, 20, -30, 30, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(254, 3, 60, 20, 20, -30, 30, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(255, 3, 60, 20, 20, -30, 30, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(256, 3, 60, 20, 20, -30, 30, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(257, 3, 60, 20, 20, -30, 30, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(258, 3, 60, 20, 20, -30, 30, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(259, 3, 60, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(260, 3, 60, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(261, 3, 60, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(262, 3, 60, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(570, 5, 70, 11, 11, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(571, 5, 70, 11, 11, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(572, 5, 70, 11, 11, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(573, 5, 70, 11, 11, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(182, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(184, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(185, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(186, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(188, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(189, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(191, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(192, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(195, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	if (Game_Flag_Query(547)) {
		Scene_Loop_Start_Special(0, 0, 0);
		Scene_Loop_Set_Default(1);
	} else if (Game_Flag_Query(536)) {
		Scene_Loop_Start_Special(0, 3, 0);
		Scene_Loop_Set_Default(1);
		Game_Flag_Reset(536);
	} else {
		Scene_Loop_Set_Default(1);
	}
}

void SceneScriptNR05::SceneLoaded() {
	Obstacle_Object("NM1-1+", true);
	Clickable_Object("NM1-1+");
	Unclickable_Object("NM1-1+");
}

bool SceneScriptNR05::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptNR05::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptNR05::ClickedOnActor(int actorId) {
	if (actorId == kActorEarlyQBartender) {
		if (!Loop_Actor_Walk_To_Actor(kActorMcCoy, kActorEarlyQBartender, 120, 1, false)) {
			sub_4020B4();
		}
		return true;
	}
	if (actorId == kActorEarlyQ) {
		Actor_Set_Goal_Number(kActorEarlyQ, 229);
		if (!Loop_Actor_Walk_To_Actor(kActorMcCoy, kActorEarlyQ, 36, 1, false)) {
			sub_4022DC();
		}
		Actor_Set_Goal_Number(kActorEarlyQ, 221);
		return true;
	}
	return false;
}

bool SceneScriptNR05::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptNR05::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -444.0f, 0.0f, -451.0f, 0, 1, false, 0)) {
			Player_Loses_Control();
			Music_Stop(2);
			Player_Set_Combat_Mode(false);
			Actor_Face_Heading(kActorMcCoy, 1021, false);
			Actor_Change_Animation_Mode(kActorMcCoy, 53);
			Game_Flag_Set(537);
			Set_Enter(55, 56);
			Scene_Loop_Start_Special(1, 3, 0);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -777.56f, 0.0f, -166.86f, 0, 1, false, 0)) {
			Game_Flag_Set(546);
			Set_Enter(13, 61);
		}
		return true;
	}
	return false;
}

bool SceneScriptNR05::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptNR05::SceneFrameAdvanced(int frame) {
	if (!Music_Is_Playing()) {
		sub_402B9C();
	}
	if (frame == 78) {
		Sound_Play(345, 83, 70, 70, 50);
	}
	if (frame == 86) {
		Sound_Play(353, 62, 70, 70, 50);
	}
	sub_402A48(48);
	sub_402A48(0);
	if (Actor_Query_Goal_Number(kActorEarlyQ) == 224) {
		Actor_Set_Goal_Number(kActorEarlyQ, 225);
		if (Player_Query_Current_Scene() == 58) {
			Scene_Exit_Add_2D_Exit(1, 0, 0, 30, 479, 3);
		}
	}
	if (frame > 77 && frame <= 134) {
		sub_401F74(frame - 13);
		if (frame == 134 && !Game_Flag_Query(537)) {
			Actor_Set_Goal_Number(kActorMcCoy, 200);
		}
		//return true;
		return;
	} else {
		//return false;
		return;
	}

}

void SceneScriptNR05::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptNR05::PlayerWalkedIn() {
	if (Game_Flag_Query(547)) {
		Music_Stop(2);
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -697.56f, 0.0f, -174.86f, 0, 1, false, 0);
		Game_Flag_Reset(547);
	}
	//	return false;
}

void SceneScriptNR05::PlayerWalkedOut() {
	if (Game_Flag_Query(537)) {
		Music_Stop(2);
	}
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void SceneScriptNR05::DialogueQueueFlushed(int a1) {
}

void SceneScriptNR05::sub_401F74(int frame) {
	float angle = cos((frame - 65) * (M_PI / 57.0f)) * M_PI_2;
	float invertedAngle = M_PI - angle;
	if (!Game_Flag_Query(537)) {
		angle = angle + M_PI;
		invertedAngle = invertedAngle + M_PI;
	}
	float c = cos(invertedAngle);
	float s = sin(invertedAngle);
	float x = 6.0f * s - 80.0f * c + -450.0f;
	float z = 80.0f * s + 6.0f * c + -531.0f;

	int facing = angle * (512.0f / M_PI);
	facing = facing + 765;
	if (facing < 0) {
		facing = facing + 1789;
	}
	if (facing > 1023) {
		facing -= 1024;
	}
	Actor_Set_At_XYZ(kActorMcCoy, x, 0.0f, z, facing);
}

void SceneScriptNR05::sub_4020B4() {
	Actor_Face_Actor(kActorMcCoy, kActorEarlyQBartender, true);
	Actor_Face_Actor(kActorEarlyQBartender, kActorMcCoy, true);
	if (Game_Flag_Query(588)) {
		if (Game_Flag_Query(589)) {
			Actor_Says(kActorMcCoy, 3480, 19);
			Actor_Says(kActorEarlyQBartender, 30, 12);
			Actor_Says(kActorMcCoy, 3485, 3);
			Actor_Says(kActorEarlyQBartender, 40, 13);
			Actor_Change_Animation_Mode(kActorEarlyQBartender, 23);
			Actor_Change_Animation_Mode(kActorMcCoy, 75);
			Global_Variable_Increment(42, 1);
		} else {
			Actor_Says(kActorMcCoy, 3475, 17);
			Actor_Says(kActorEarlyQBartender, 20, 23);
			Game_Flag_Set(589);
			Actor_Change_Animation_Mode(kActorMcCoy, 75);
			Global_Variable_Increment(42, 1);
		}
	} else {
		Actor_Says(kActorEarlyQBartender, 0, 13);
		Actor_Says(kActorMcCoy, 3470, 3);
		Actor_Says(kActorEarlyQBartender, 10, 23);
		Game_Flag_Set(588);
		Actor_Change_Animation_Mode(kActorMcCoy, 75);
		Global_Variable_Increment(42, 1);
	}
}

void SceneScriptNR05::sub_4022DC() {
	if (Actor_Query_Goal_Number(kActorEarlyQ) == 220) {
		Actor_Set_Goal_Number(kActorEarlyQ, 221);
	}
	Actor_Face_Actor(kActorMcCoy, kActorEarlyQ, true);
	Actor_Face_Actor(kActorEarlyQ, kActorMcCoy, true);
	if (!Game_Flag_Query(590)) {
		Actor_Says(kActorMcCoy, 8513, 3);
		Actor_Says(kActorEarlyQ, 360, 3);
		Actor_Says(kActorMcCoy, 3495, 11);
		Actor_Says(kActorEarlyQ, 370, 15);
		Actor_Says(kActorMcCoy, 3500, 17);
		Actor_Says(kActorEarlyQ, 380, 13);
		Game_Flag_Set(590);
		return;
	}
	Dialogue_Menu_Clear_List();
	if (Actor_Query_Friendliness_To_Other(kActorEarlyQ, kActorMcCoy) >= 48) {
		if (Actor_Clue_Query(kActorMcCoy, kClueDragonflyCollection) || Actor_Clue_Query(kActorMcCoy, kClueCollectionReceipt)) {
			DM_Add_To_List_Never_Repeat_Once_Selected(890, -1, 4, 8);
		}
		if (Actor_Clue_Query(kActorMcCoy, kClueLucy)) {
			DM_Add_To_List_Never_Repeat_Once_Selected(900, 5, 6, 5);
		}
		if (Actor_Clue_Query(kActorMcCoy, kClueDektorasDressingRoom)) {
			DM_Add_To_List_Never_Repeat_Once_Selected(910, 5, 5, 5);
		}
	}
	if (!Dialogue_Menu_Query_List_Size()) {
		Actor_Says(kActorMcCoy, 3520, 3);
		Actor_Says(kActorEarlyQ, 730, 3);
		Actor_Face_Heading(kActorEarlyQ, 849, false);
		return;
	}
	Dialogue_Menu_Add_DONE_To_List(100);
	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();
	if (answer == 890) {
		Actor_Says(kActorMcCoy, 3505, 3);
		Actor_Modify_Friendliness_To_Other(kActorEarlyQ, kActorMcCoy, -1);
		Actor_Says(kActorEarlyQ, 420, 12);
		Actor_Says(kActorEarlyQ, 430, 13);
		Actor_Says(kActorMcCoy, 3530, 15);
		Actor_Says(kActorEarlyQ, 440, 15);
		Actor_Says(kActorMcCoy, 3535, 13);
		Actor_Says(kActorEarlyQ, 460, 16);
		Actor_Says(kActorMcCoy, 3540, 15);
		Actor_Says(kActorEarlyQ, 490, 16);
		Actor_Says(kActorEarlyQ, 500, 13);
		Actor_Says(kActorMcCoy, 3545, 15);
		Actor_Says(kActorEarlyQ, 520, 12);
		Actor_Face_Heading(kActorEarlyQ, 849, false);
	} else if (answer == 900) {
		Actor_Says(kActorMcCoy, 3510, 15);
		Actor_Modify_Friendliness_To_Other(kActorEarlyQ, kActorMcCoy, -1);
		Actor_Says_With_Pause(kActorEarlyQ, 530, 1.2f, 3);
		Actor_Says(kActorEarlyQ, 540, 15);
		Actor_Says(kActorMcCoy, 3550, 13);
		Actor_Says(kActorEarlyQ, 560, 14);
		Actor_Says(kActorEarlyQ, 570, 13);
		Actor_Says(kActorMcCoy, 3555, 12);
		Actor_Face_Heading(kActorEarlyQ, 849, false);
	} else if (answer == 910) {
		Actor_Says(kActorMcCoy, 3515, 14);
		Actor_Modify_Friendliness_To_Other(kActorEarlyQ, kActorMcCoy, -1);
		if (Actor_Clue_Query(kActorMcCoy, kClueGrigoriansNote)) {
			Actor_Says(kActorEarlyQ, 580, 12);
			Actor_Says(kActorMcCoy, 3560, 13);
			Actor_Says(kActorEarlyQ, 590, 16);
			Actor_Says(kActorMcCoy, 3565, 16);
			Actor_Says(kActorEarlyQ, 600, 13);
			Actor_Says(kActorMcCoy, 3570, 14);
			Actor_Says(kActorEarlyQ, 620, 15);
			Actor_Says(kActorMcCoy, 3575, 13);
		} else {
			Actor_Says(kActorEarlyQ, 640, 13);
			Actor_Says(kActorMcCoy, 3580, 15);
			Actor_Says(kActorEarlyQ, 660, 12);
		}
		Actor_Face_Heading(kActorEarlyQ, 849, false);
	}
}

void SceneScriptNR05::sub_402A48(int actorId) {
	int animationMode = Actor_Query_Animation_Mode(actorId);
	if (animationMode == 1 || animationMode == 2 || animationMode == 7 || animationMode == 8) {
		return;
	}
	float x, y, z;
	Actor_Query_XYZ(actorId, &x, &y, &z);
	if ((x - -542.0f) * (x - -542.0f) + (z - -195.0f) * (z - -195.0f) < 8464.0f) {
		float s = sin(M_PI / 128.0f);
		float c = cos(M_PI / 128.0f);
		float newX = x * c - z * s + -542.0f;
		float newZ = x * s + z * c + -195.0f;
		int newFacing = (Actor_Query_Facing_1024(actorId) + 4) % 1024;
		Actor_Set_At_XYZ(actorId, newX, y, newZ, newFacing);
	}
}

void SceneScriptNR05::sub_402B9C() {
	if (Music_Is_Playing()) {
		Music_Adjust(51, 0, 2);
	} else {
		int v0 = Global_Variable_Query(54);
		if (v0 == 0) {
			Music_Play(16, 61, -80, 2, -1, 0, 0);
		} else if (v0 == 1) {
			Music_Play(15, 41, -80, 2, -1, 0, 0);
		} else if (v0 == 2) {
			Music_Play(7, 41, -80, 2, -1, 0, 0);
		}
		v0++;
		if (v0 > 2) {
			v0 = 0;
		}
		Global_Variable_Set(54, v0);
	}
}

} // End of namespace BladeRunner
