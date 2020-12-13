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

void SceneScriptBB11::InitializeScene() {
	Setup_Scene_Information(43.39f, -10.27f, -20.52f, 200);
	if (!Game_Flag_Query(kFlagBB11SadikFight)) {
		Scene_Exit_Add_2D_Exit(0, 280, 154, 388, 247, 2);
		if (_vm->_cutContent && !Game_Flag_Query(kFlagMcCoyCommentsOnFans)) {
			Scene_2D_Region_Add(0, 454, 1, 639, 228);// right fans
			Scene_2D_Region_Add(1, 1, 1, 240, 375);  // left fans
		}
	}

	Ambient_Sounds_Add_Looping_Sound(kSfxROOFRAN1, 90, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxROOFAIR1, 45, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxROOFRMB1, 76, 0, 1);
	Ambient_Sounds_Add_Sound(kSfxSPIN2B,  5, 180, 16,  25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSPIN3A,  5, 180, 16,  25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER2, 5, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER3, 5, 180, 50, 100, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxTHNDER4, 5, 180, 50, 100, 0, 0, -101, -101, 0, 0);

	if (Game_Flag_Query(kFlagBB11SadikFight)) {
		Preload(kModelAnimationMcCoyIdle);
		Preload(kModelAnimationClovisWalking);
		Preload(kModelAnimationClovisIdle);
		Preload(kModelAnimationSadikIdle);
		Preload(kModelAnimationSadikKicksSomeoneWhoIsDown);
		Preload(kModelAnimationSadikHoldsSomeoneAndPunches);
		Preload(kModelAnimationMcCoyGotHitRight);
		Preload(kModelAnimationMcCoyRunning);
		Preload(kModelAnimationSadikRunning);
		Preload(kModelAnimationSadikWalking);
		Preload(kModelAnimationMcCoyFallsOnHisBack);
		Preload(kModelAnimationSadikPicksUpAndThrowsMcCoy);
	}
}

void SceneScriptBB11::SceneLoaded() {
	Obstacle_Object("X2AIRCON01", true);
	Unclickable_Object("X2AIRCON01");
	if (Game_Flag_Query(kFlagBB11SadikFight)) {
		Unobstacle_Object("X2PIPES01", true);
		Unobstacle_Object("X2PIPES02", true);
		Unobstacle_Object("X2PIPES03", true);
		Unobstacle_Object("X2_VENTS05", true);
		Unobstacle_Object("X2_VENTSCYL05", true);
	}
}

bool SceneScriptBB11::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptBB11::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptBB11::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptBB11::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptBB11::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 43.39f, -10.27f, -68.52f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagBB11toBB10);
			Set_Enter(kSetBB10, kSceneBB10);
		}
		return true;
	}
	return false;
}

bool SceneScriptBB11::ClickedOn2DRegion(int region) {
	if (_vm->_cutContent) {
		if (!Game_Flag_Query(kFlagMcCoyCommentsOnFans) && (region == 0 || region == 1) ) {
			Game_Flag_Set(kFlagMcCoyCommentsOnFans);
			Actor_Force_Stop_Walking(kActorMcCoy);
			Actor_Face_Heading(kActorMcCoy, 550, false);
			Actor_Voice_Over(3740, kActorVoiceOver);
			Actor_Voice_Over(3750, kActorVoiceOver);
			Scene_2D_Region_Remove(0);
			Scene_2D_Region_Remove(1);
			return true;
		}
	}
	return false;
}

void SceneScriptBB11::SceneFrameAdvanced(int frame) {
	if ( Actor_Query_Goal_Number(kActorSadik) == kGoalSadikBB11KnockOutMcCoy
	 && !Game_Flag_Query(kFlagBB11SadikPunchedMcCoy)
	) {
		Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeDie);
		Game_Flag_Set(kFlagBB11SadikPunchedMcCoy);
	} else {
		if (frame == 1) {
			Sound_Play(kSfxSWEEP4, 10, -100, 100, 50);
		}
	}
}

void SceneScriptBB11::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptBB11::PlayerWalkedIn() {
	if (Actor_Query_Goal_Number(kActorSadik) == kGoalSadikBB11Wait
	 && Global_Variable_Query(kVariableChapter) == 2
	) {
		Actor_Set_Invisible(kActorMcCoy, true);
		Actor_Set_Goal_Number(kActorSadik, kGoalSadikBB11ThrowMcCoy);
		Music_Play(kMusicBeating1, 61, 0, 1, -1, 0, 0);
		Player_Loses_Control();
	}
}

void SceneScriptBB11::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void SceneScriptBB11::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
