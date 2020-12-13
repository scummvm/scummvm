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

enum kNR05Loops {
	kNR05LoopPanFromNR08 = 0,
	kNR05LoopMainLoop    = 1,
	kNR05LoopTableSwivel = 3
};


void SceneScriptNR05::InitializeScene() {
	if (Game_Flag_Query(kFlagNR08toNR05)) {
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, kNR05LoopPanFromNR08, false);
		Scene_Loop_Set_Default(kNR05LoopMainLoop);
#if BLADERUNNER_ORIGINAL_BUGS
		Setup_Scene_Information(-777.56f,  0.0f, -166.86f,   0);
#else
		Setup_Scene_Information(-752.09f, 0.0f, -193.58f, 258);
#endif // BLADERUNNER_ORIGINAL_BUGS
	} else if (Game_Flag_Query(kFlagNR03toNR05)) {
		Setup_Scene_Information( -456.0f,  0.0f,  -611.0f,   0);
	} else {
		Setup_Scene_Information( -527.0f, 1.57f,  -406.0f, 649);
	}

	Scene_Exit_Add_2D_Exit(0, 459, 147, 639, 290, 1);
	if (Game_Flag_Query(kFlagNR08Available)) {
		Scene_Exit_Add_2D_Exit(1, 0, 0, 30, 479, 3);
	}

	Ambient_Sounds_Add_Looping_Sound(kSfxBARAMB1, 50, 38, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX1,  3, 60, 20, 20,  -30,  30, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX3,  3, 60, 20, 20,  -30,  30, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX4,  3, 60, 20, 20,  -30,  30, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX5,  3, 60, 20, 20,  -30,  30, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX6,  3, 60, 20, 20,  -30,  30, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX7,  3, 60, 20, 20,  -30,  30, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxCLINK1,   3, 60, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxCLINK2,   3, 60, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxCLINK3,   3, 60, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxCLINK4,   3, 60, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxFEMORG1,  5, 70, 11, 11, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxFEMORG2,  5, 70, 11, 11, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxFEMORG3,  5, 70, 11, 11, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxFEMORG4,  5, 70, 11, 11, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0480R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0540R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0560R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0870R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0900R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0940R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_1070R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_1080R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_1160R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);

	if (Game_Flag_Query(kFlagNR08toNR05)) {
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, kNR05LoopPanFromNR08, false);
		Scene_Loop_Set_Default(kNR05LoopMainLoop);
	} else if (Game_Flag_Query(kFlagNR03toNR05)) {
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, kNR05LoopTableSwivel, false);
		Scene_Loop_Set_Default(kNR05LoopMainLoop);
		Game_Flag_Reset(kFlagNR03toNR05);
	} else {
		Scene_Loop_Set_Default(kNR05LoopMainLoop);
	}
}

void SceneScriptNR05::SceneLoaded() {
	Obstacle_Object("NM1-1+", true);
#if BLADERUNNER_ORIGINAL_BUGS
#else
	Obstacle_Object("CUSTLFTBLOCK", true);
#endif // BLADERUNNER_ORIGINAL_BUGS
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
		if (!Loop_Actor_Walk_To_Actor(kActorMcCoy, kActorEarlyQBartender, 120, true, false)) {
			talkToBartender();
		}
		return true;
	}

	if (actorId == kActorEarlyQ) {
		Actor_Set_Goal_Number(kActorEarlyQ, kGoalEarlyQNR05TalkingToMcCoy);
		if (!Loop_Actor_Walk_To_Actor(kActorMcCoy, kActorEarlyQ, 36, true, false)) {
			talkToEarlyQ();
		}
		Actor_Set_Goal_Number(kActorEarlyQ, kGoalEarlyQNR05WillLeave);
		return true;
	}

	return false;
}

bool SceneScriptNR05::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptNR05::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -444.0f, 0.0f, -451.0f, 0, true, false, false)) {
			Player_Loses_Control();
			Music_Stop(2);
			Player_Set_Combat_Mode(false);
			Actor_Face_Heading(kActorMcCoy, 1021, false);
			Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeSit);
			Game_Flag_Set(kFlagNR05toNR03);
			Set_Enter(kSetNR03, kSceneNR03);
			Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kNR05LoopTableSwivel, false);
		}
		return true;
	}

	if (exitId == 1) {
#if BLADERUNNER_ORIGINAL_BUGS
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -777.56f, 0.0f, -166.86f, 0, true, false, false)) {
			Game_Flag_Set(kFlagNR05toNR08);
			Set_Enter(kSetNR05_NR08, kSceneNR08);
		}
#else
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -752.09f, 0.0f, -193.58f, 0, true, false, false)) {
			Game_Flag_Set(kFlagNR05toNR08);
			Set_Enter(kSetNR05_NR08, kSceneNR08);
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
		return true;
	}
	return false;
}

bool SceneScriptNR05::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptNR05::SceneFrameAdvanced(int frame) {
	if (!Music_Is_Playing()) {
		playNextMusic();
	}

	if (frame == 78) {
		Sound_Play(kSfxDORSLID1, 83, 70, 70, 50);
	}

	if (frame == 86) {
		Sound_Play(kSfxMAGMOVE2, 62, 70, 70, 50);
	}

	rotateActorOnGround(kActorHysteriaPatron2);
	if (_vm->_cutContent) {
		rotateActorOnGround(kActorHysteriaPatron3);
	}
	rotateActorOnGround(kActorMcCoy);

	if (Actor_Query_Goal_Number(kActorEarlyQ) == kGoalEarlyQNR05UnlockNR08) {
		Actor_Set_Goal_Number(kActorEarlyQ, kGoalEarlyQNR05UnlockedNR08);
		if (Player_Query_Current_Scene() == kSceneNR05) {
			Scene_Exit_Add_2D_Exit(1, 0, 0, 30, 479, 3);
		}
	}

	if (frame > 77 && frame <= 134) {
		rotateActorOnTable(frame - 13);
		if ( frame == 134 && !Game_Flag_Query(kFlagNR05toNR03)) {
			Actor_Set_Goal_Number(kActorMcCoy, kGoalMcCoyNRxxSitAtTable);
		}
		//return true;
	}
	//return false;
}

void SceneScriptNR05::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptNR05::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagNR08toNR05)) {
		Music_Stop(2);
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -697.56f, 0.0f, -174.86f, 0, true, false, false);
		Game_Flag_Reset(kFlagNR08toNR05);
	}
	//	return false;
}

void SceneScriptNR05::PlayerWalkedOut() {
	if (Game_Flag_Query(kFlagNR05toNR03)) {
		Music_Stop(2);
	}
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void SceneScriptNR05::DialogueQueueFlushed(int a1) {
}

void SceneScriptNR05::rotateActorOnTable(int frame) {
	float angle = cos((frame - 65) * (M_PI / 57.0f)) * M_PI_2;
	float invertedAngle = M_PI - angle;
	if (!Game_Flag_Query(kFlagNR05toNR03)) {
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

void SceneScriptNR05::talkToBartender() {
	Actor_Face_Actor(kActorMcCoy, kActorEarlyQBartender, true);
	Actor_Face_Actor(kActorEarlyQBartender, kActorMcCoy, true);
	if (!Game_Flag_Query(kFlagNR05BartenderTalk1)) {
		Actor_Says(kActorEarlyQBartender, 0, 13);
		Actor_Says(kActorMcCoy, 3470, kAnimationModeTalk);
		Actor_Says(kActorEarlyQBartender, 10, 23);
		Game_Flag_Set(kFlagNR05BartenderTalk1);
		Actor_Change_Animation_Mode(kActorMcCoy, 75);
		Global_Variable_Increment(kVariableMcCoyDrinks, 1);
	} else if (!Game_Flag_Query(kFlagNR05BartenderTalk2)) {
		Actor_Says(kActorMcCoy, 3475, 17);
		Actor_Says(kActorEarlyQBartender, 20, 23);
		Game_Flag_Set(kFlagNR05BartenderTalk2);
		Actor_Change_Animation_Mode(kActorMcCoy, 75);
		Global_Variable_Increment(kVariableMcCoyDrinks, 1);
	} else {
		Actor_Says(kActorMcCoy, 3480, 19);
		Actor_Says(kActorEarlyQBartender, 30, 12);
		Actor_Says(kActorMcCoy, 3485, kAnimationModeTalk);
		Actor_Says(kActorEarlyQBartender, 40, 13);
		Actor_Change_Animation_Mode(kActorEarlyQBartender, 23);
		Actor_Change_Animation_Mode(kActorMcCoy, 75);
		Global_Variable_Increment(kVariableMcCoyDrinks, 1);
	}
}

void SceneScriptNR05::talkToEarlyQ() {
	if (Actor_Query_Goal_Number(kActorEarlyQ) == kGoalEarlyQNR05Wait) {
		Actor_Set_Goal_Number(kActorEarlyQ, kGoalEarlyQNR05WillLeave);
	}

	Actor_Face_Actor(kActorMcCoy, kActorEarlyQ, true);
	Actor_Face_Actor(kActorEarlyQ, kActorMcCoy, true);

	if (!Game_Flag_Query(kFlagNR05EarlyQTalk)) {
		Actor_Says(kActorMcCoy, 8513, kAnimationModeTalk);
		Actor_Says(kActorEarlyQ, 360, kAnimationModeTalk);
		Actor_Says(kActorMcCoy, 3495, 11);
		Actor_Says(kActorEarlyQ, 370, 15);
		Actor_Says(kActorMcCoy, 3500, 17);
		Actor_Says(kActorEarlyQ, 380, 13);
		Game_Flag_Set(kFlagNR05EarlyQTalk);
		return;
	}

	Dialogue_Menu_Clear_List();
	if (Actor_Query_Friendliness_To_Other(kActorEarlyQ, kActorMcCoy) >= 48) {
		if (Actor_Clue_Query(kActorMcCoy, kClueDragonflyCollection)
		 || Actor_Clue_Query(kActorMcCoy, kClueCollectionReceipt)
		) {
			DM_Add_To_List_Never_Repeat_Once_Selected(890, -1, 4, 8); // JEWELRY
		}
		if (Actor_Clue_Query(kActorMcCoy, kClueLucy)) {
			DM_Add_To_List_Never_Repeat_Once_Selected(900, 5, 6, 5); // LUCY
		}
		if (Actor_Clue_Query(kActorMcCoy, kClueDektorasDressingRoom)) {
			DM_Add_To_List_Never_Repeat_Once_Selected(910, 5, 5, 5); // BLOND WOMAN
		}
	}

	if (!Dialogue_Menu_Query_List_Size()) {
		Actor_Says(kActorMcCoy, 3520, kAnimationModeTalk);
		Actor_Says(kActorEarlyQ, 730, kAnimationModeTalk);
		Actor_Face_Heading(kActorEarlyQ, 849, false);
		return;
	}

	Dialogue_Menu_Add_DONE_To_List(100); // DONE

	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();

	switch (answer) {
	case 890: // JEWELRY
		Actor_Says(kActorMcCoy, 3505, kAnimationModeTalk);
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
		break;

	case 900: // LUCY
		Actor_Says(kActorMcCoy, 3510, 15);
		Actor_Modify_Friendliness_To_Other(kActorEarlyQ, kActorMcCoy, -1);
		Actor_Says_With_Pause(kActorEarlyQ, 530, 1.2f, kAnimationModeTalk);
		Actor_Says(kActorEarlyQ, 540, 15);
		Actor_Says(kActorMcCoy, 3550, 13);
		Actor_Says(kActorEarlyQ, 560, 14);
		Actor_Says(kActorEarlyQ, 570, 13);
		Actor_Says(kActorMcCoy, 3555, 12);
		Actor_Face_Heading(kActorEarlyQ, 849, false);
		break;

	case 910: // BLOND WOMAN
		Actor_Says(kActorMcCoy, 3515, 14);
		Actor_Modify_Friendliness_To_Other(kActorEarlyQ, kActorMcCoy, -1);
		if (Actor_Clue_Query(kActorMcCoy, kClueGrigoriansNote)) { // cut content? this clue is unobtanium
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
		break;
	}
}

void SceneScriptNR05::rotateActorOnGround(int actorId) {
	int animationMode = Actor_Query_Animation_Mode(actorId);
	if (animationMode == kAnimationModeWalk
	 || animationMode == kAnimationModeRun
	 || animationMode == kAnimationModeCombatWalk
	 || animationMode == kAnimationModeCombatRun
	) {
		return;
	}

	float x, y, z;
	Actor_Query_XYZ(actorId, &x, &y, &z);
	x -= -542.0f;
	z -= -195.0f;
	if (x * x + z * z < 92.0f * 92.0f) {
		float s = sin(M_PI / 128.0f);
		float c = cos(M_PI / 128.0f);
		float newX = x * c - z * s + -542.0f;
		float newZ = x * s + z * c + -195.0f;
		int newFacing = (Actor_Query_Facing_1024(actorId) + 4) % 1024;
		Actor_Set_At_XYZ(actorId, newX, y, newZ, newFacing);
	}
}

void SceneScriptNR05::playNextMusic() {
	if (Music_Is_Playing()) {
		Music_Adjust(51, 0, 2);
	} else {
		int track = Global_Variable_Query(kVariableEarlyQBackMusic);
		if (track == 0) {
			Music_Play(kMusicDkoDnce1, 61, -80, 2, -1, 0, 0);
		} else if (track == 1) {
			Music_Play(kMusicStrip1, 41, -80, 2, -1, 0, 0);
		} else if (track == 2) {
			Music_Play(kMusicArkDnce1, 41, -80, 2, -1, 0, 0);
		}
		++track;
		if (track > 2) {
			track = 0;
		}
		Global_Variable_Set(kVariableEarlyQBackMusic, track);
	}
}

} // End of namespace BladeRunner
