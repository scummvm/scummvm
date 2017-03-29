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

void SceneScriptTB07::InitializeScene() {
	Setup_Scene_Information(68.0f, 12.0f, 288.0f, 0);
	Scene_Exit_Add_2D_Exit(0, 383, 445, 639, 479, 2);
	Ambient_Sounds_Add_Looping_Sound(109, 20, 0, 1);
	Ambient_Sounds_Add_Sound(363, 2, 55, 14, 14, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(364, 2, 55, 14, 14, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(365, 2, 55, 14, 14, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(366, 2, 55, 14, 14, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(212, 1, 15, 20, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(213, 1, 15, 20, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(214, 1, 20, 20, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(215, 1, 15, 20, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(216, 1, 15, 20, 25, -100, 100, -101, -101, 0, 0);
	if (Global_Variable_Query(1) == 4 && !Actor_Clue_Query(kActorMcCoy, kClueDNATyrell)) {
		Item_Add_To_World(83, 941, 18, 9.7f, 48.7f, -174.22f, 0, 12, 12, false, true, false, true);
	}
	if (Game_Flag_Query(661)) {
		Scene_Loop_Set_Default(3);
	} else {
		Scene_Loop_Set_Default(0);
	}
}

void SceneScriptTB07::SceneLoaded() {
	Obstacle_Object("EAGLE01", true);
	Clickable_Object("EAGLE01");
}

bool SceneScriptTB07::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptTB07::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptTB07::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptTB07::ClickedOnItem(int itemId, bool a2) {
	if (!Loop_Actor_Walk_To_Item(kActorMcCoy, itemId, 36, 1, false)) {
		Actor_Face_Item(kActorMcCoy, itemId, true);
		if (itemId == 83) {
			Item_Pickup_Spin_Effect(941, 331, 296);
			Actor_Clue_Acquire(kActorMcCoy, kClueDNATyrell, 0, -1);
		}
		Item_Remove_From_World(itemId);
	}
	return false;
}

bool SceneScriptTB07::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 68.0f, 12.0f, 288.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			if (Global_Variable_Query(1) == 4) {
				Game_Flag_Set(608);
				Set_Enter(17, 82);
			} else {
				Game_Flag_Reset(176);
				Game_Flag_Reset(182);
				Game_Flag_Reset(179);
				Game_Flag_Reset(178);
				Game_Flag_Reset(258);
				Game_Flag_Reset(257);
				Game_Flag_Reset(261);
				Game_Flag_Reset(450);
				switch (Spinner_Interface_Choose_Dest(-1, 0)) {
				case 9:
					Game_Flag_Set(257);
					Game_Flag_Reset(307);
					Game_Flag_Set(256);
					Set_Enter(37, 34);
					break;
				case 8:
					Game_Flag_Set(181);
					Game_Flag_Reset(307);
					Game_Flag_Set(255);
					Set_Enter(54, 54);
					break;
				case 7:
					Game_Flag_Set(258);
					Game_Flag_Reset(307);
					Game_Flag_Set(254);
					Set_Enter(20, 2);
					break;
				case 6:
					Game_Flag_Set(177);
					Game_Flag_Reset(307);
					Game_Flag_Set(253);
					Set_Enter(7, 25);
					break;
				case 4:
					Game_Flag_Set(180);
					Game_Flag_Reset(307);
					Game_Flag_Set(252);
					Set_Enter(0, 0);
					break;
				case 3:
					Game_Flag_Set(176);
					Game_Flag_Reset(307);
					Game_Flag_Set(248);
					Set_Enter(4, 13);
					break;
				case 2:
					Game_Flag_Set(182);
					Game_Flag_Reset(307);
					Game_Flag_Set(249);
					Set_Enter(69, 78);
					break;
				case 1:
					Game_Flag_Set(179);
					Game_Flag_Reset(307);
					Game_Flag_Set(250);
					Set_Enter(49, 48);
					break;
				case 0:
					Game_Flag_Set(178);
					Game_Flag_Reset(307);
					Game_Flag_Set(251);
					Set_Enter(61, 65);
					break;
				default:
					Game_Flag_Set(261);
					Loop_Actor_Walk_To_XYZ(kActorMcCoy, 44.0f, 12.0f, 176.0f, 0, 0, false, 0);
					break;
				}
			}
		}
		return true;
	}
	return false;
}

bool SceneScriptTB07::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptTB07::SceneFrameAdvanced(int frame) {
	if (frame == 66) {
		Ambient_Sounds_Play_Sound(591, 20, 99, 0, 0);
	}
	//return false;
}

void SceneScriptTB07::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptTB07::PlayerWalkedIn() {
	int v0 = Global_Variable_Query(1);
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, 44.0f, 12.0f, 176.0f, 0, 0, false, 0);
	if ((v0 == 2 || v0 == 3) && !Game_Flag_Query(612)) {
		Player_Set_Combat_Mode(false);
		sub_401B0C();
	}
}

void SceneScriptTB07::PlayerWalkedOut() {
}

void SceneScriptTB07::DialogueQueueFlushed(int a1) {
}

void SceneScriptTB07::sub_401B0C() {
	Game_Flag_Set(612);
	Delay(1500);
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, 44.98f, 12.0f, 49.79f, 0, 0, false, 0);
	Actor_Face_Heading(kActorMcCoy, 178, true);
	Delay(3000);
	Actor_Put_In_Set(kActorRachael, 18);
	Actor_Set_At_XYZ(kActorRachael, -260.15f, 12.0f, -19.16f, 256);
	Actor_Change_Animation_Mode(kActorRachael, 0);
	Outtake_Play(39, 1, -1);
	Loop_Actor_Walk_To_XYZ(kActorRachael, -146.15f, 12.0f, -5.84f, 0, 0, false, 0);
	Actor_Face_Actor(kActorRachael, kActorMcCoy, true);
	Actor_Says(kActorRachael, 480, 14);
	Actor_Face_Actor(kActorMcCoy, kActorRachael, true);
	Actor_Says(kActorMcCoy, 5315, 9);
	Actor_Says(kActorRachael, 490, 3);
	Actor_Face_Heading(kActorMcCoy, 178, true);
	Actor_Says(kActorMcCoy, 5320, 15);
	Actor_Says_With_Pause(kActorMcCoy, 5325, 1.0f, 19);
	Actor_Start_Speech_Sample(kActorRachael, 500);
	Loop_Actor_Walk_To_XYZ(kActorRachael, -60.15f, 12.0f, 60.84f, 0, 0, false, 0);
	Actor_Face_Actor(kActorRachael, kActorMcCoy, true);
	Actor_Face_Actor(kActorMcCoy, kActorRachael, true);
	Actor_Says(kActorMcCoy, 5330, 14);
	Actor_Says(kActorRachael, 510, 12);
	Actor_Says(kActorMcCoy, 5335, 16);
	Actor_Says(kActorRachael, 520, 17);
	Actor_Says(kActorMcCoy, 5340, 3);
	Actor_Start_Speech_Sample(kActorRachael, 530);
	Loop_Actor_Walk_To_XYZ(kActorRachael, -4.15f, 12.0f, 54.73f, 0, 0, false, 0);
	Actor_Says(kActorRachael, 540, 16);
	Actor_Says(kActorMcCoy, 5345, 18);
	Actor_Says(kActorRachael, 550, 13);
	Actor_Says(kActorRachael, 570, 18);
	Actor_Says_With_Pause(kActorMcCoy, 5350, 0.0f, 18);
	Actor_Says(kActorRachael, 580, 16);
	Actor_Says(kActorMcCoy, 5355, 16);
	Actor_Says(kActorRachael, 590, 17);
	Actor_Says(kActorMcCoy, 5360, 17);
	Actor_Says(kActorMcCoy, 5365, 13);
	Actor_Says_With_Pause(kActorRachael, 600, 1.0f, 12);
	Actor_Says(kActorMcCoy, 5370, 3);
	Loop_Actor_Walk_To_XYZ(kActorRachael, -24.15f, 12.0f, -10.84f, 0, 0, false, 0);
	Actor_Says(kActorRachael, 610, 13);
	Actor_Face_Actor(kActorMcCoy, kActorRachael, true);
	Actor_Says(kActorMcCoy, 5375, 18);
	Actor_Says(kActorMcCoy, 5380, 19);
	Actor_Face_Actor(kActorRachael, kActorMcCoy, true);
	Actor_Says(kActorRachael, 620, 18);
	Actor_Says_With_Pause(kActorMcCoy, 5385, 2.0f, 12);
	Actor_Says_With_Pause(kActorMcCoy, 5390, 2.0f, 14);
	Actor_Says(kActorMcCoy, 5395, 15);
	Actor_Says_With_Pause(kActorRachael, 630, 0.0f, 14);
	Actor_Says(kActorMcCoy, 5400, 18);
	Actor_Says(kActorMcCoy, 5405, 3);
	Actor_Says(kActorRachael, 640, 12);
	Actor_Says(kActorMcCoy, 5410, 16);
	Actor_Says(kActorRachael, 650, 15);
	Actor_Says_With_Pause(kActorMcCoy, 5415, 1.0f, 17);
	Actor_Says(kActorMcCoy, 5420, 14);
	Actor_Says(kActorRachael, 660, 15);
	Actor_Put_In_Set(kActorTyrell, 18);
	Actor_Set_At_XYZ(kActorTyrell, 68.0f, 12.0f, 288.0f, 0);
	Actor_Change_Animation_Mode(kActorTyrell, 0);
	Scene_Loop_Set_Default(3);
	Scene_Loop_Start_Special(2, 2, 0);
	Actor_Start_Speech_Sample(kActorTyrell, 0);
	Loop_Actor_Walk_To_XYZ(kActorTyrell, 44.0f, 12.0f, 176.0f, 0, 0, false, 0);
	Actor_Face_Actor(kActorTyrell, kActorMcCoy, true);
	Actor_Face_Actor(kActorMcCoy, kActorTyrell, true);
	Actor_Face_Actor(kActorRachael, kActorTyrell, true);
	Actor_Says(kActorTyrell, 10, 12);
	Actor_Says(kActorTyrell, 20, 3);
	Actor_Says(kActorTyrell, 30, 12);
	Actor_Says(kActorMcCoy, 5425, 18);
	Actor_Set_Goal_Number(kActorRachael, 200);
	Actor_Says(kActorTyrell, 40, 15);
	Actor_Start_Speech_Sample(kActorTyrell, 50);
	Loop_Actor_Walk_To_XYZ(kActorTyrell, -10.0f, 12.0f, 100.0f, 0, 0, false, 0);
	Actor_Face_Actor(kActorTyrell, kActorMcCoy, true);
	Actor_Face_Actor(kActorMcCoy, kActorTyrell, true);
	Actor_Says(kActorMcCoy, 5430, 17);
	Actor_Says(kActorMcCoy, 5435, 16);
	Actor_Says(kActorTyrell, 60, 14);
	Actor_Face_Actor(kActorMcCoy, kActorTyrell, true);
	Actor_Says(kActorMcCoy, 5440, 14);
	Actor_Says(kActorTyrell, 70, 13);
	Actor_Says(kActorMcCoy, 5445, 15);
	Actor_Says_With_Pause(kActorTyrell, 80, 1.0f, 12);
	Actor_Says(kActorTyrell, 90, 15);
	Actor_Says_With_Pause(kActorMcCoy, 5450, 1.0f, 15);
	Actor_Says(kActorMcCoy, 5455, 12);
	Actor_Says(kActorTyrell, 100, 14);
	Actor_Clue_Acquire(kActorMcCoy, kClueRachaelInterview, 0, kActorRachael);
	Actor_Clue_Acquire(kActorMcCoy, kClueTyrellInterview, 0, kActorTyrell);
	Loop_Actor_Walk_To_XYZ(kActorTyrell, -260.15f, 12.0f, -19.16f, 0, 0, false, 0);
}

} // End of namespace BladeRunner
