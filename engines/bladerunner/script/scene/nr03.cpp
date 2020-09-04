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

enum kNR03Loops {
	kNR03LoopMainLoop    = 0,
	kNR03LoopTableSwivel = 2
};

void SceneScriptNR03::InitializeScene() {
	if (Game_Flag_Query(kFlagNR05toNR03)) {
		Setup_Scene_Information(-301.98f, -70.19f, -348.58f,   0);
	} else if (Game_Flag_Query(kFlagNR04toNR03)) {
		Setup_Scene_Information( -161.0f, -70.19f, -1139.0f, 500);
		Game_Flag_Reset(kFlagNR04toNR03);
	} else {
		Setup_Scene_Information(  410.0f, -70.19f,  -715.0f, 690);
	}

	Scene_Exit_Add_2D_Exit(0, 561,   0, 639, 216, 1);
	Scene_Exit_Add_2D_Exit(1, 210,  85, 240, 145, 0);
	Scene_Exit_Add_2D_Exit(2,   0, 135,  85, 295, 3);

	Scene_2D_Region_Add(0, 331, 73, 375, 114);

	Ambient_Sounds_Add_Looping_Sound(kSfxBARAMB1, 50, 38, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX1,  3, 60, 25, 25,    0, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX3,  3, 60, 25, 25,    0, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX4,  3, 60, 25, 25,    0, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX5,  3, 60, 25, 25,    0, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX6,  3, 60, 25, 25,    0, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBARSFX7,  3, 60, 25, 25,    0, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxCLINK1,   3, 60, 20, 20, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxCLINK2,   3, 60, 20, 20, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxCLINK3,   3, 60, 20, 20, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxCLINK4,   3, 60, 20, 20, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0480R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0540R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0560R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0870R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0900R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_0940R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_1070R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_1080R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfx67_1160R, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);

	if (Game_Flag_Query(kFlagNR03Entered)) {
		if (Game_Flag_Query(kFlagNR05toNR03)) {
			Scene_Loop_Start_Special(kSceneLoopModeLoseControl, kNR03LoopTableSwivel, false);
			Scene_Loop_Set_Default(kNR03LoopMainLoop);
			Game_Flag_Reset(kFlagNR05toNR03);
		} else {
			Scene_Loop_Set_Default(kNR03LoopMainLoop);
		}
	} else {
		Actor_Set_Goal_Number(kActorGuzza, kGoalGuzzaSitAtNR03);
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, kNR03LoopTableSwivel, false);
		Scene_Loop_Set_Default(kNR03LoopMainLoop);
	}

	// Guarding in NR03
	if (Actor_Query_Goal_Number(kActorHanoi) > 209
	 && Actor_Query_Goal_Number(kActorHanoi) < 215
	) {
		Actor_Set_Goal_Number(kActorHanoi, kGoalHanoiNR03StartGuarding);
	}
}

void SceneScriptNR03::SceneLoaded() {
	Obstacle_Object("PG3", true);
	Obstacle_Object("X2BACKBARBOTTOM01", true);
	Obstacle_Object("X2BACKSTAGETOP", true);
	Unclickable_Object("PG3");
	Unobstacle_Object("X2BACKBARBOTTOM02", true);
	Unobstacle_Object("NM2", true);
	Unobstacle_Object("MAN5", true);
	Unobstacle_Object("MAN7", true);
	Unobstacle_Object("X2BACKSTAGETOP", true);
}

bool SceneScriptNR03::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptNR03::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("PG3", objectName)) {
		Actor_Face_Object(kActorMcCoy, "PG3", true);
		Actor_Voice_Over(3770, kActorVoiceOver);
		return true;
	}
	return false;
}

bool SceneScriptNR03::ClickedOnActor(int actorId) {
	if (actorId == kActorHanoi) {
		if (!Loop_Actor_Walk_To_Actor(kActorMcCoy, kActorHanoi, 48, true, false)) {
			AI_Movement_Track_Pause(kActorHanoi);
			Actor_Face_Actor(kActorMcCoy, kActorHanoi, true);
			if (!Game_Flag_Query(kFlagNR03HanoiTalk)) {
				Game_Flag_Set(kFlagNR03HanoiTalk);
				Actor_Says(kActorMcCoy, 3340, kAnimationModeTalk);
				Actor_Face_Actor(kActorHanoi, kActorMcCoy, true);
				Actor_Says(kActorHanoi, 30, 13);
				Actor_Says(kActorMcCoy, 3345, 14);
				Actor_Says(kActorHanoi, 40, 14);
			} else {
				Actor_Says(kActorMcCoy, 3350, 16);
				Actor_Says(kActorHanoi, 50, 17);
			}
			AI_Movement_Track_Unpause(kActorHanoi);
			return true;
		}
	}
	return false;
}

bool SceneScriptNR03::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptNR03::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 410.0f, -70.19f, -715.0f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagNR03toNR01);
			Set_Enter(kSetNR01, kSceneNR01);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -161.0f, -70.19f, -1139.0f, 0, true, false, false)) {
			if (Actor_Query_Which_Set_In(kActorHanoi) == kSetNR03) {
				AI_Movement_Track_Pause(kActorHanoi);
				Actor_Face_Actor(kActorHanoi, kActorMcCoy, true);
				Actor_Face_Actor(kActorMcCoy, kActorHanoi, true);
				switch (Global_Variable_Query(kVariableHanoiNR04Warnings)) {
				case 0:
					Actor_Says(kActorHanoi, 50, 13);
					AI_Movement_Track_Unpause(kActorHanoi);
					break;
				case 1:
					Actor_Says(kActorHanoi, 210, 15);
					AI_Movement_Track_Unpause(kActorHanoi);
					break;
				case 2:
					// fall through
				default:
#if !BLADERUNNER_ORIGINAL_BUGS
					// Needed delay, otherwise McCoy's animation of being thrown out won't play
					Delay(150);
#endif
					Actor_Set_Goal_Number(kActorHanoi, kGoalHanoiThrowOutMcCoy);
					break;
				}
				Global_Variable_Increment(kVariableHanoiNR04Warnings, 1);
#if !BLADERUNNER_ORIGINAL_BUGS
				if (Global_Variable_Query(kVariableHanoiNR04Warnings) > 2) {
					Global_Variable_Set(kVariableHanoiNR04Warnings, 1);
				}
#endif
			} else {
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Game_Flag_Set(kFlagNR03toNR04);
				Set_Enter(kSetNR04, kSceneNR04);
			}
		}

		return true;
	}

	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -151.0f, -70.19f, -476.0f, 12, true, false, false)) {
			if (Actor_Query_Which_Set_In(kActorHanoi) == kSetNR03
			 && Actor_Query_Goal_Number(kActorHanoi) != kGoalHanoiNR03GoToDancer
			) {
				Actor_Face_Heading(kActorMcCoy, 680, false);
				Actor_Change_Animation_Mode(kActorMcCoy, 12);
				Delay(150);
				Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeIdle);
				AI_Movement_Track_Pause(kActorHanoi);
				Actor_Face_Actor(kActorHanoi, kActorMcCoy, true);
				switch (Global_Variable_Query(kVariableHanoiNR05Warnings)) {
				case 0:
					Actor_Says(kActorHanoi, 0, 15);
					Actor_Face_Actor(kActorMcCoy, kActorHanoi, true);
					Actor_Says(kActorMcCoy, 3335, 13);
					Actor_Says(kActorHanoi, 10, 16);
					AI_Movement_Track_Unpause(kActorHanoi);
					break;
				case 1:
					Actor_Face_Actor(kActorMcCoy, kActorHanoi, true);
					Actor_Says(kActorHanoi, 210, 12);
					AI_Movement_Track_Unpause(kActorHanoi);
					break;
				case 2:
					// fall through
				default:
					Actor_Set_Goal_Number(kActorHanoi, kGoalHanoiThrowOutMcCoy);
					break;
				}
				Global_Variable_Increment(kVariableHanoiNR05Warnings, 1);
#if !BLADERUNNER_ORIGINAL_BUGS
				if (Global_Variable_Query(kVariableHanoiNR05Warnings) > 2) {
					Global_Variable_Set(kVariableHanoiNR05Warnings, 1);
				}
#endif
			} else {
				Player_Loses_Control();
				Player_Set_Combat_Mode(false);
				Loop_Actor_Walk_To_XYZ(kActorMcCoy, -229.0f, -70.19f, -469.0f, 0, false, false, true);
				Actor_Face_Heading(kActorMcCoy, 656, false);
				Actor_Change_Animation_Mode(kActorMcCoy, 53);
				Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
				Ambient_Sounds_Remove_All_Looping_Sounds(1);
				Game_Flag_Set(kFlagNR03toNR05);
				Set_Enter(kSetNR05_NR08, kSceneNR05);
				Scene_Loop_Start_Special(kSceneLoopModeChangeSet, kNR03LoopTableSwivel, false);
			}
		}
		return true;
	}
	return false;
}

bool SceneScriptNR03::ClickedOn2DRegion(int region) {
	if (region == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 79.2f, -70.19f, -984.0f, 12, true, false, false)) {
			Actor_Face_Actor(kActorMcCoy, kActorHysteriaPatron1, true);
			switch (Random_Query(0, 4)) {
			case 0:
				Actor_Says(kActorMcCoy, 1055, kAnimationModeTalk);
				break;
			case 1:
				Actor_Says(kActorMcCoy, 8590, kAnimationModeTalk);
				break;
			case 2:
				Actor_Says(kActorMcCoy, 8930, kAnimationModeTalk);
				break;
			case 3:
				Actor_Says(kActorMcCoy, 7465, kAnimationModeTalk);
				break;
#if BLADERUNNER_ORIGINAL_BUGS
			default:
				break;
#else
			case 4:
				if (_vm->_cutContent) {
					switch (Random_Query(1, 10)) {
					case 1:
						// fall through
					case 2:
						// make this rare
						Actor_Says(kActorMcCoy, 8518, kAnimationModeTalk); // Hey, can I lick...
						break;
					default:
						Actor_Says(kActorMcCoy, 8700, kAnimationModeTalk); // Never seen anything like that before.
						break;
					}
				} else {
					// just say the same as in case 0
					Actor_Says(kActorMcCoy, 1055, kAnimationModeTalk);
				}
			default:
				break;
#endif // BLADERUNNER_ORIGINAL_BUGS
			}
		}
		return true;
	}
	return false;
}

void SceneScriptNR03::SceneFrameAdvanced(int frame) {
	if (!Music_Is_Playing()) {
		playNextMusic();
	}

	if (frame == 72) {
		Sound_Play(kSfxDORSLID1, 83, -70, -70, 50);
	}

	if (frame == 76) {
		Sound_Play(kSfxMAGMOVE2, 62, -70, -70, 50);
	}

	if (frame > 70 && frame < 110) {
		rotateActorOnTable(frame);
	} else if (frame == 110) {
		if (Actor_Query_Goal_Number(kActorGuzza) == kGoalGuzzaSitAtNR03) {
			Actor_Set_Goal_Number(kActorGuzza, kGoalGuzzaWasAtNR03);
		} else if (!Game_Flag_Query(kFlagNR03toNR05)) {
			Actor_Set_Goal_Number(kActorMcCoy, kGoalMcCoyNRxxSitAtTable);
			Player_Gains_Control();
		}
	}
	//return true;
	return;
}

void SceneScriptNR03::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptNR03::PlayerWalkedIn() {
	Player_Set_Combat_Mode(false);
	if (!Game_Flag_Query(kFlagNR03Entered)) {
		Game_Flag_Set(kFlagNR03Entered);
		Async_Actor_Walk_To_XYZ(kActorMcCoy, 206.0f, -70.19f, -643.0f, 0, false);
		Game_Flag_Reset(kFlagNR01toNR03);
		Actor_Voice_Over(1490, kActorVoiceOver);
		Actor_Voice_Over(1510, kActorVoiceOver);
		Actor_Voice_Over(1520, kActorVoiceOver);
	} else if (Game_Flag_Query(kFlagNR01toNR03) ) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 302.0f, -70.19f, -715.0f, 0, false, false, false);
		Game_Flag_Reset(kFlagNR01toNR03);
	}

	// This seems redundant since we set the combat mode to false at start of WalkedIn() method
	// Probably a last minute original behavior fix to be more user friendly (not being tossed off the bar immediately upon entering)
	if (Player_Query_Combat_Mode()) {
		Actor_Set_Goal_Number(kActorHanoi, kGoalHanoiThrowOutMcCoy);
	}
	//return false;
}

void SceneScriptNR03::PlayerWalkedOut() {
	if (!Game_Flag_Query(kFlagNR03toNR04)) {
		Music_Stop(2);
	}
	if (Game_Flag_Query(kFlagNR03toNR05)) {
		Player_Gains_Control();
	}
}

void SceneScriptNR03::DialogueQueueFlushed(int a1) {
}

void SceneScriptNR03::rotateActorOnTable(int frame) {
	int facing;
	float angle, invertedAngle;

	angle = cos((frame - 70) * (M_PI / 40.0f)) * M_PI_2;
	invertedAngle = M_PI - angle;
	if (!Game_Flag_Query(kFlagNR03toNR05)
	 &&  Actor_Query_Goal_Number(kActorGuzza) != kGoalGuzzaSitAtNR03
	) {
		angle = angle + M_PI;
		invertedAngle = invertedAngle + M_PI;
	}

	float c = cos(invertedAngle);
	float s = sin(invertedAngle);
	float x = 36.49f * s - -60.21f * c + -265.49f;
	float z = -60.21f * s + 36.49f * c + -408.79f;

	if (Actor_Query_Goal_Number(kActorGuzza) == kGoalGuzzaSitAtNR03) {
		facing = angle * (512.0f / M_PI);
		facing = facing + 144;
		if (facing < 0) {
			facing = facing + 1168;
		}
		if (facing > 1023) {
			facing -= 1024;
		}
		Actor_Set_At_XYZ(kActorGuzza, x, -70.19f, z, facing);
	} else {
		facing = angle * (512.0f / M_PI);
		facing = facing + 400;
		if (facing < 0) {
			facing = facing + 1424;
		}
		if (facing > 1023) {
			facing -= 1024;
		}

		Actor_Set_At_XYZ(kActorMcCoy, x, -70.19f, z, facing);
	}
}

void SceneScriptNR03::playNextMusic() {
	if (Music_Is_Playing()) {
		Music_Adjust(51, 0, 2);
	} else {
		int track = Global_Variable_Query(kVariableEarlyQFrontMusic);
		if (track == 0) {
			Music_Play(kMusicGothic2, 51, 0, 2, -1, 0, 0);
		} else if (track == 1) {
			Music_Play(kMusicGothic1, 51, 0, 2, -1, 0, 0);
		} else if (track == 2) {
			Music_Play(kMusicGothic3, 51, 0, 2, -1, 0, 0);
		}
		++track;
		if (track > 2) {
			track = 0;
		}
		Global_Variable_Set(kVariableEarlyQFrontMusic, track);
	}
}

} // End of namespace BladeRunner
