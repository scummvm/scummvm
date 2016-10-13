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

#include "bladerunner/script/script.h"

namespace BladeRunner {

void ScriptPS06::InitializeScene() {
	Setup_Scene_Information(11257.26f, 707.3f, -4778.31f, 120);
	Scene_Exit_Add_2D_Exit(0, 610, 0, 639, 479, 1);
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(0);
	Ambient_Sounds_Add_Looping_Sound(388, 50, 1, 1);

}

void ScriptPS06::SceneLoaded() {
	Obstacle_Object("E.SCREEN02", true);
	Clickable_Object("E.SCREEN02");
	Clickable_Object("E.MONITOR1");
	Clickable_Object("E.SCREEN03");
	Clickable_Object("E.MONITOR3");
}

bool ScriptPS06::MouseClick(int x, int y) {
	return false;
}

bool ScriptPS06::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("E.MONITOR1", objectName)) {
		ESPER_Flag_To_Activate();
		return true;
	}
	if (Object_Query_Click("E.SCREEN03", objectName) || Object_Query_Click("E.MONITOR3", objectName)) {
		Actor_Says(39, 330, 3);
		if (!Actor_Clue_Query(0, 111) || Actor_Clue_Query(0, 113) || Actor_Clue_Query(0, 114) || Actor_Clue_Query(0, 115)) {
			Actor_Clues_Transfer_New_To_Mainframe(0);
			Ambient_Sounds_Play_Sound(587, 50, 0, 0, 99);
			Delay(2000);
			Actor_Says(39, 340, 3);
			Actor_Clues_Transfer_New_From_Mainframe(0);
			Ambient_Sounds_Play_Sound(587, 50, 0, 0, 99);
			Delay(2000);
			Ambient_Sounds_Play_Sound(588, 80, 0, 0, 99);
			Actor_Says(39, 350, 3);
			return true;
		} else {
			Delay(2000);
			Actor_Voice_Over(3780, 99);
			Actor_Voice_Over(3790, 99);
			if (Game_Flag_Query(47)) {
				Actor_Voice_Over(3800, 99);
				Actor_Voice_Over(3810, 99);
				Actor_Voice_Over(3820, 99);
				Actor_Voice_Over(3830, 99);
				Actor_Clue_Acquire(0, 113, 1, -1);
			} else if (Game_Flag_Query(45)) {
				Actor_Voice_Over(3840, 99);
				Actor_Voice_Over(3850, 99);
				Actor_Voice_Over(3860, 99);
				Actor_Voice_Over(3870, 99);
				Actor_Clue_Acquire(0, 114, 1, -1);
			} else {
				Actor_Voice_Over(3880, 99);
				Actor_Voice_Over(3890, 99);
				Actor_Voice_Over(3900, 99);
				Actor_Voice_Over(3910, 99);
				Actor_Clue_Acquire(0, 115, 1, -1);
			}
			Actor_Clues_Transfer_New_To_Mainframe(0);
			Actor_Clues_Transfer_New_From_Mainframe(0);
			return true;
		}
	}
	return false;
}

bool ScriptPS06::ClickedOnActor(int actorId) {
	return false;
}

bool ScriptPS06::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptPS06::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		Game_Flag_Set(23);
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		Set_Enter(15, 69);
		return true;
	}
	return false;
}

bool ScriptPS06::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptPS06::SceneFrameAdvanced(int frame) {
}

void ScriptPS06::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptPS06::PlayerWalkedIn() {
	if (Game_Flag_Query(136)) {
		Game_Flag_Reset(136);
	}
}

void ScriptPS06::PlayerWalkedOut() {
}

void ScriptPS06::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
