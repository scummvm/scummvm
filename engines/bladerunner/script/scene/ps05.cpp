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

void SceneScriptPS05::InitializeScene() {
	if (Game_Flag_Query(kFlagPS13toPS05)) {
		Setup_Scene_Information(547.59f, 0.18f, -216.84f, 334);
	} else if (Game_Flag_Query(kFlagPS02toPS05)) {
		Setup_Scene_Information(635.0f, 0.0f, -598.0f, 475);
	} else { // kFlagPS06toPS05 || kFlagPS15toPS05, but for some reason kFlagPS06toPS05 is then set in PlayerWalkedIn
#if BLADERUNNER_ORIGINAL_BUGS
		Setup_Scene_Information(630.72f, 0.38f, -469.26f, 400);
#else
		// exiting from PS15
		Setup_Scene_Information(553.24f, 0.37f, -422.97f, 400);
#endif // BLADERUNNER_ORIGINAL_BUGS
	}
	Scene_Exit_Add_2D_Exit(0, 218, 98, 280, 246, 3);
	Scene_Exit_Add_2D_Exit(1, 330, 90, 436, 198, 0);
	Scene_Exit_Add_2D_Exit(2, 476, 96, 524, 240, 1);
	Scene_2D_Region_Add(0, 519, 107, 537, 122);
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(false);
	Ambient_Sounds_Add_Looping_Sound(kSfxRUMLOOP1, 50, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxLABAMB3,  80, 0, 1);
	Ambient_Sounds_Add_Sound(kSfxPHONE1, 5, 50,  8,  8, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxPAGE1,  5, 20, 30, 30,  -70,  70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxPAGE2,  5, 20, 30, 30,  -70,  70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxPAGE3,  5, 20, 30, 30,  -70,  70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSWEEP3, 5, 20,  5,  9,  -70,  70, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSWEEP4, 5, 20,  5,  9,  -70,  70, -101, -101, 0, 0);
}

void SceneScriptPS05::SceneLoaded() {
	Obstacle_Object("WATER FOUNTAIN", true);
#if BLADERUNNER_ORIGINAL_BUGS
#else
	Obstacle_Object("MAINFBLOCK", true);
#endif // BLADERUNNER_ORIGINAL_BUGS
	Clickable_Object("WATER FOUNTAIN");
	Clickable_Object("ASHTRAY");
	Clickable_Object("FIRE EXTINGISHER");
	Clickable_Object("CUP");
	Clickable_Object("WIRE BASKET");
	Clickable_Object("WANTED POSTERS");
	Unclickable_Object("WATER FOUNTAIN");
	Unclickable_Object("CUP");
#if BLADERUNNER_ORIGINAL_BUGS
#else
	Unclickable_Object("FIRE EXTINGISHER");
#endif // BLADERUNNER_ORIGINAL_BUGS

}

bool SceneScriptPS05::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptPS05::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("WATER FOUNTAIN", objectName) && !Loop_Actor_Walk_To_Scene_Object(kActorMcCoy, "WATER FOUNTAIN", 12, true, false)) {
		Actor_Face_Object(kActorMcCoy, "WATER FOUNTAIN", true);
		Actor_Says(kActorMcCoy, 3490, 18);
	}
	if (Object_Query_Click("ASHTRAY", objectName) && !Loop_Actor_Walk_To_XYZ(kActorMcCoy, 662.0f, 0.37f, -180.0f, 0, true, false, false)) {
		Actor_Face_Object(kActorMcCoy, "ASHTRAY", true);
		Actor_Voice_Over(1770, kActorVoiceOver);
		Actor_Voice_Over(1780, kActorVoiceOver);
		Actor_Voice_Over(1790, kActorVoiceOver);
	}
	if (Object_Query_Click("WIRE BASKET", objectName) && !Loop_Actor_Walk_To_Scene_Object(kActorMcCoy, "WIRE BASKET", 12, true, false)) {
		Actor_Face_Object(kActorMcCoy, "WIRE BASKET", true);
		Actor_Voice_Over(1810, kActorVoiceOver);
		Actor_Voice_Over(1820, kActorVoiceOver);
	}
	if (Object_Query_Click("WANTED POSTERS", objectName) && !Loop_Actor_Walk_To_Scene_Object(kActorMcCoy, "WANTED POSTERS", 12, true, false)) {
		Actor_Face_Object(kActorMcCoy, "WANTED POSTERS", true);
		Actor_Voice_Over(1800, kActorVoiceOver);
	}
	return false;
}

bool SceneScriptPS05::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptPS05::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptPS05::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_Waypoint(kActorMcCoy, 2, 24, true, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Set_Enter(kSetPS15, kScenePS15);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 635.0f, 0.0f, -598.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Set_Enter(kSetPS02, kScenePS02);
		}
		return true;
	}
	if (exitId == 2) {
#if BLADERUNNER_ORIGINAL_BUGS
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 742.52f, 0.37f, -457.69f, 0, true, false, false)) {
			Game_Flag_Set(kFlagPS05toPS06);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Set_Enter(kSetPS06, kScenePS06);
		}
#else
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 715.52f, 0.37f, -457.69f, 0, true, false, false)) {
			Game_Flag_Set(kFlagPS05toPS06);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Set_Enter(kSetPS06, kScenePS06);
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
		return true;
	}
	return false;
}

bool SceneScriptPS05::ClickedOn2DRegion(int region) {
	if (region == 0 && !Loop_Actor_Walk_To_XYZ(kActorMcCoy, 694.78f, 0.37f, -321.05f, 0, true, false, false)) {
		Actor_Face_Heading(kActorMcCoy, 130, false);
		View_Score_Board();
	}
	return false;
}

void SceneScriptPS05::SceneFrameAdvanced(int frame) {
	if (frame == 1 || frame == 16 || frame == 31 || frame == 46) {
		Sound_Play(kSfxLABMISC4, Random_Query(10, 10), 70, 70, 50);
	}
	//return true;
}

void SceneScriptPS05::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptPS05::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagPS06toPS05)) {
#if BLADERUNNER_ORIGINAL_BUGS
		Actor_Set_At_XYZ(kActorMcCoy, 718.72f, 0.37f, -461.26f, 600);
#else
		Actor_Set_At_XYZ(kActorMcCoy, 710.14f, 0.37f, -455.48f, 600);
#endif // BLADERUNNER_ORIGINAL_BUGS
	} else if (Game_Flag_Query(kFlagPS02toPS05)) {
		selectNextTvNews();
		turnOnTV();
	}
	Game_Flag_Reset(kFlagPS02toPS05);
	Game_Flag_Reset(kFlagPS06toPS05);
	Game_Flag_Reset(kFlagPS13toPS05);
	Game_Flag_Reset(kFlagPS15toPS05);
}

void SceneScriptPS05::PlayerWalkedOut() {
}

void SceneScriptPS05::DialogueQueueFlushed(int a1) {
	Overlay_Remove("PS05OVER");
}

void SceneScriptPS05::selectNextTvNews() {
	int i = 0;
	int arr[7];
	if (Global_Variable_Query(kVariableChapter) < 4 && Game_Flag_Query(kFlagGordoIsReplicant)) {
		arr[i++] = 0;
	}
	arr[i++] = 1;
	if (Global_Variable_Query(kVariableChapter) >= 3) {
		arr[i++] = 2;
	}
	if (Global_Variable_Query(kVariableChapter) >= 2 && Global_Variable_Query(kVariableChapter) <= 4) {
		arr[i++] = 3;
	}
	if (Game_Flag_Query(kFlagCT04HomelessBodyFound) && Game_Flag_Query(kFlagCT04HomelessBodyInDumpster)) {
		arr[i++] = 4;
	}
	Global_Variable_Set(kVariableNextTvNews, arr[Random_Query(0, i - 1)]);
}

void SceneScriptPS05::turnOnTV() {
	switch (Global_Variable_Query(kVariableNextTvNews)) {
	case 0:
		if (!Game_Flag_Query(kFlagPS05TV0)) {
			Overlay_Play("PS05OVER", 0, true, false, 0);
			ADQ_Add(kActorNewscaster, 0, kAnimationModeTalk);
			ADQ_Add(kActorNewscaster, 10, kAnimationModeTalk);
			ADQ_Add(kActorNewscaster, 20, kAnimationModeTalk);
			ADQ_Add(kActorNewscaster, 30, kAnimationModeTalk);
			ADQ_Add(kActorTyrell, 430, kAnimationModeTalk);
			ADQ_Add(kActorTyrell, 440, kAnimationModeTalk);
			ADQ_Add(kActorTyrell, 450, kAnimationModeTalk);
			ADQ_Add(kActorTyrell, 460, kAnimationModeTalk);
			Game_Flag_Set(kFlagPS05TV0);
		}
		break;
	case 1:
		if (!Game_Flag_Query(kFlagPS05TV1)) {
			Overlay_Play("PS05OVER", 0, true, false, 0);
			ADQ_Add(kActorNewscaster, 40, kAnimationModeTalk);
			ADQ_Add(kActorNewscaster, 50, kAnimationModeTalk);
			ADQ_Add(kActorNewscaster, 60, kAnimationModeTalk);
			ADQ_Add(kActorNewscaster, 70, kAnimationModeTalk);
			ADQ_Add(kActorNewscaster, 80, kAnimationModeTalk);
			Game_Flag_Set(kFlagPS05TV1);
		}
		break;
	case 2:
		if (!Game_Flag_Query(kFlagPS05TV2)) {
			Overlay_Play("PS05OVER", 0, true, false, 0);
			if (Actor_Query_Friendliness_To_Other(kActorClovis, kActorMcCoy) > Actor_Query_Friendliness_To_Other(kActorSteele, kActorMcCoy)) {
				ADQ_Add(kActorNewscaster, 120, kAnimationModeTalk);
				ADQ_Add(kActorNewscaster, 130, kAnimationModeTalk);
				ADQ_Add(kActorNewscaster, 140, kAnimationModeTalk);
				ADQ_Add(kActorNewscaster, 150, kAnimationModeTalk);
				if (_vm->_cutContent && Random_Query(1, 3) == 1) {
					ADQ_Add(kActorGuzza, 1600, kAnimationModeTalk);
				} else {
					ADQ_Add(kActorGuzza, 1570, kAnimationModeTalk);
					ADQ_Add(kActorGuzza, 1580, kAnimationModeTalk);
					ADQ_Add(kActorGuzza, 1590, kAnimationModeTalk);
				}
			} else {
				ADQ_Add(kActorNewscaster, 90, kAnimationModeTalk);
				ADQ_Add(kActorNewscaster, 100, kAnimationModeTalk);
				ADQ_Add(kActorNewscaster, 110, kAnimationModeTalk);
				ADQ_Add(kActorGuzza, 1540, kAnimationModeTalk);
				ADQ_Add(kActorGuzza, 1550, kAnimationModeTalk);
				ADQ_Add(kActorGuzza, 1560, kAnimationModeTalk);
			}
			Game_Flag_Set(kFlagPS05TV2);
		}
		break;
	case 3:
		if (!Game_Flag_Query(kFlagPS05TV3)) {
			Overlay_Play("PS05OVER", 0, true, false, 0);
			ADQ_Add(kActorNewscaster, 170, kAnimationModeTalk);
			ADQ_Add(kActorNewscaster, 180, kAnimationModeTalk);
			ADQ_Add(kActorNewscaster, 190, kAnimationModeTalk);
			ADQ_Add(kActorNewscaster, 200, kAnimationModeTalk);
			ADQ_Add(kActorNewscaster, 210, kAnimationModeTalk);
			ADQ_Add(kActorNewscaster, 220, kAnimationModeTalk);
			ADQ_Add(kActorGovernorKolvig, 80, kAnimationModeTalk);
			ADQ_Add(kActorGovernorKolvig, 90, kAnimationModeTalk);
			ADQ_Add(kActorGovernorKolvig, 100, kAnimationModeTalk);
			ADQ_Add(kActorGovernorKolvig, 110, kAnimationModeTalk);
			ADQ_Add(kActorGovernorKolvig, 120, kAnimationModeTalk);
			ADQ_Add(kActorGovernorKolvig, 130, kAnimationModeTalk);
			Game_Flag_Set(kFlagPS05TV3);
		}
		break;
	case 4:
		if (!Game_Flag_Query(kFlagPS05TV4)) {
			Overlay_Play("PS05OVER", 0, true, false, 0);
			ADQ_Add(kActorNewscaster, 230, kAnimationModeTalk);
			ADQ_Add(kActorNewscaster, 240, kAnimationModeTalk);
			Game_Flag_Set(kFlagPS05TV4);
		}
		break;
	}
}

} // End of namespace BladeRunner
