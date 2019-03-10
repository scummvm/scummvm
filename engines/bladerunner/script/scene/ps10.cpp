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

static int kPoliceMazePS10TargetCount = 20;

static const int *getPoliceMazePS10TrackData1() {
	static int trackData[] = {
		kPMTIActivate,		kVariablePoliceMazePS10TargetCounter, kPoliceMazePS10TargetCount,
		kPMTIVariableInc,	kVariablePoliceMazePS10TargetCounter, kPoliceMazePS10TargetCount,
		kPMTIObstacleReset,	kItemPS10Target1,
		kPMTIObstacleReset,	kItemPS10Target2,
		kPMTIFacing,		989,
		kPMTIPosition,		0,
		kPMTITargetSet,		kItemPS10Target1, 1,
		kPMTITargetSet,		kItemPS10Target2, 1,
		kPMTIEnemyReset,	kItemPS10Target1,
		kPMTIWaitRandom,	3000, 5000,
		kPMTIObstacleSet,	kItemPS10Target1,
		kPMTIPlaySound,		159, 100,
		kPMTIMove,			14,
		kPMTIWait,			1000,
		kPMTIRotate,		740, 80,
		kPMTIEnemySet,		kItemPS10Target1,
		kPMTIWait,			0,
		kPMTIRotate,		488, 80,
		kPMTIWait,			1000,
		kPMTIShoot,			27, 33,
		kPMTIWait,			0,
		kPMTIRotate,		740, 80,
		kPMTIPausedReset,	kItemPS10Target2,
		kPMTIObstacleReset,	kItemPS10Target1,
		kPMTIObstacleSet,	kItemPS10Target2,
		kPMTIPausedSet,		kItemPS10Target1,
		kPMTIPosition,		0,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS10TrackData2() {
	static int trackData[] = {
		kPMTIFacing,		740,
		kPMTIPosition,		0,
		kPMTIEnemySet,		kItemPS10Target2,
		kPMTIMove,			69,
		kPMTIWait,			500,
		kPMTIObstacleReset,	kItemPS10Target2,
		kPMTIPausedReset,	kItemPS10Target5,
		kPMTIPausedSet,		kItemPS10Target2,
		kPMTIPosition,		0,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS10TrackData3() {
	static int trackData[] = {
		kPMTIActivate,		kVariablePoliceMazePS10TargetCounter, kPoliceMazePS10TargetCount,
		kPMTIVariableInc,	kVariablePoliceMazePS10TargetCounter, kPoliceMazePS10TargetCount,
		kPMTIObstacleReset,	kItemPS10Target3,
		kPMTIFacing,		993,
		kPMTIPosition,		0,
		kPMTIWaitRandom,	3000, 5000,
		kPMTIObstacleSet,	kItemPS10Target3,
		kPMTIPlaySound,		159, 100,
		kPMTITargetSet,		kItemPS10Target3, 1,
		kPMTIEnemyReset,	kItemPS10Target3,
		kPMTIMove,			5,
		kPMTIWait,			1000,
		kPMTIEnemySet,		kItemPS10Target3,
		kPMTIRotate,		233, 80,
		kPMTIWait,			0,
		kPMTIRotate,		491, 80,
		kPMTIWait,			500,
		kPMTIShoot,			27, 33,
		kPMTIWait,			500,
		kPMTIRotate,		233, 80,
		kPMTIWait,			0,
		kPMTIRotate,		993, 80,
		kPMTIPlaySound,		34, 33,
		kPMTIMove,			0,
		kPMTIObstacleReset,	kItemPS10Target3,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS10TrackData4() {
	static int trackData[] = {
		kPMTIActivate,		kVariablePoliceMazePS10TargetCounter, kPoliceMazePS10TargetCount,
		kPMTIVariableInc,	kVariablePoliceMazePS10TargetCounter, kPoliceMazePS10TargetCount,
		kPMTIObstacleReset,	kItemPS10Target4,
		kPMTIFacing,		993,
		kPMTIPosition,		0,
		kPMTIWaitRandom,	3000, 6000,
		kPMTIObstacleSet,	kItemPS10Target4,
		kPMTIPlaySound,		159, 100,
		kPMTITargetSet,		kItemPS10Target4, 1,
		kPMTIEnemyReset,	kItemPS10Target4,
		kPMTIMove,			34,
		kPMTIWait,			500,
		kPMTIRotate,		491, 80,
		kPMTIMove,			0,
		kPMTILeave,
		kPMTIObstacleReset,	kItemPS10Target4,
		kPMTIPausedReset,	kItemPS10Target8,
		kPMTIPausedSet,		kItemPS10Target4,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS10TrackData5() {
	static int trackData[] = {
		kPMTIActivate,		kVariablePoliceMazePS10TargetCounter, kPoliceMazePS10TargetCount,
		kPMTIVariableInc,	kVariablePoliceMazePS10TargetCounter, kPoliceMazePS10TargetCount,
		kPMTIObstacleReset,	kItemPS10Target5,
		kPMTIFacing,		0,
		kPMTIPosition,		0,
		kPMTIWaitRandom,	4000, 6000,
		kPMTIObstacleSet,	kItemPS10Target5,
		kPMTIPlaySound,		159, 100,
		kPMTITargetSet,		kItemPS10Target5, 1,
		kPMTIEnemyReset,	kItemPS10Target5,
		kPMTIMove,			5,
		kPMTIWait,			1000,
		kPMTIRotate,		512, 100,
		kPMTIWait,			2000,
		kPMTIRotate,		0, -100,
		kPMTIPlaySound,		34, 33,
		kPMTIMove,			0,
		kPMTILeave,
		kPMTIObstacleReset,	kItemPS10Target5,
		kPMTIPausedReset,	kItemPS10Target1,
		kPMTIPausedSet,		kItemPS10Target5,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS10TrackData6() {
	static int trackData[] = {
		kPMTIActivate,		kVariablePoliceMazePS10TargetCounter, kPoliceMazePS10TargetCount,
		kPMTIVariableInc,	kVariablePoliceMazePS10TargetCounter, kPoliceMazePS10TargetCount,
		kPMTIObstacleReset,	kItemPS10Target6,
		kPMTIFacing,		999,
		kPMTIPosition,		0,
		kPMTIWaitRandom,	4000, 6000,
		kPMTIObstacleSet,	kItemPS10Target6,
		kPMTIPlaySound,		159, 100,
		kPMTITargetSet,		kItemPS10Target6, 1,
		kPMTIEnemyReset,	kItemPS10Target6,
		kPMTIMove,			7,
		kPMTIWait,			500,
		kPMTIEnemySet,		kItemPS10Target6,
		kPMTIRotate,		750, 80,
		kPMTIWait,			0,
		kPMTIRotate,		500, 80,
		kPMTIWait,			1000,
		kPMTIShoot,			27, 33,
		kPMTIWait,			0,
		kPMTIRotate,		750, 80,
		kPMTIWait,			0,
		kPMTIRotate,		999, 80,
		kPMTIPlaySound,		34, 33,
		kPMTIMove,			0,
		kPMTIObstacleReset,	kItemPS10Target6,
		kPMTIPausedReset,	kItemPS10Target7,
		kPMTIPausedReset,	kItemPS10Target9,
		kPMTIPausedSet,		kItemPS10Target6,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS10TrackData7() {
	static int trackData[] = {
		kPMTIActivate,		kVariablePoliceMazePS10TargetCounter, kPoliceMazePS10TargetCount,
		kPMTIVariableInc,	kVariablePoliceMazePS10TargetCounter, kPoliceMazePS10TargetCount,
		kPMTIObstacleReset,	kItemPS10Target7,
		kPMTIFacing,		264,
		kPMTIPosition,		0,
		kPMTIWaitRandom,	3000, 6000,
		kPMTITargetSet,		kItemPS10Target7, 1,
		kPMTIEnemyReset,	kItemPS10Target7,
		kPMTIObstacleSet,	kItemPS10Target7,
		kPMTIMove,			89,
		kPMTIWaitRandom,	4000, 8000,
		kPMTIFacing,		776,
		kPMTIMove,			0,
		kPMTILeave,
		kPMTIObstacleReset,	kItemPS10Target7,
		kPMTIPausedSet,		kItemPS10Target7,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS10TrackData8() {
	static int trackData[] = {
		kPMTIActivate,		kVariablePoliceMazePS10TargetCounter, kPoliceMazePS10TargetCount,
		kPMTIVariableInc,	kVariablePoliceMazePS10TargetCounter, kPoliceMazePS10TargetCount,
		kPMTIObstacleReset,	kItemPS10Target8,
		kPMTIFacing,		993,
		kPMTIPosition,		0,
		kPMTIWaitRandom,	4000, 6000,
		kPMTIObstacleSet,	kItemPS10Target8,
		kPMTIPlaySound,		159, 100,
		kPMTITargetSet,		kItemPS10Target8, 1,
		kPMTIEnemyReset,	kItemPS10Target8,
		kPMTIMove,			34,
		kPMTIWait,			500,
		kPMTIEnemySet,		kItemPS10Target8,
		kPMTIRotate,		491, 80,
		kPMTIMove,			20,
		kPMTIWait,			0,
		kPMTIShoot,			27, 33,
		kPMTIMove,			0,
		kPMTIObstacleReset,	kItemPS10Target8,
		kPMTIPausedReset,	kItemPS10Target4,
		kPMTIPausedSet,		kItemPS10Target8,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS10TrackData9() {
	static int trackData[] = {
		kPMTIActivate,		kVariablePoliceMazePS10TargetCounter, kPoliceMazePS10TargetCount,
		kPMTIVariableInc,	kVariablePoliceMazePS10TargetCounter, kPoliceMazePS10TargetCount,
		kPMTIObstacleReset,	kItemPS10Target9,
		kPMTIFacing,		738,
		kPMTIPosition,		0,
		kPMTIWaitRandom,	2000, 5000,
		kPMTITargetSet,		kItemPS10Target9, 1,
		kPMTIEnemySet,		kItemPS10Target9,
		kPMTIObstacleSet,	kItemPS10Target9,
		kPMTIPlaySound,		0, 33,
		kPMTIMove,			23,
		kPMTIPlaySound,		0, 33,
		kPMTIWait,			200,
		kPMTIPlaySound,		32, 33,
		kPMTIRotate,		498, 100,
		kPMTIPlaySound,		0, 33,
		kPMTIWait,			100,
		kPMTIShoot,			27, 33,
		kPMTIPlaySound,		32, 33,
		kPMTIMove,			35,
		kPMTIPlaySound,		32, 33,
		kPMTIWait,			100,
		kPMTIShoot,			27, 33,
		kPMTIPlaySound,		0, 33,
		kPMTIMove,			23,
		kPMTIPlaySound,		32, 33,
		kPMTIWait,			100,
		kPMTIShoot,			27, 33,
		kPMTIPlaySound,		32, 33,
		kPMTIRotate,		758, 100,
		kPMTIPlaySound,		32, 33,
		kPMTIMove,			89,
		kPMTIPlaySound,		0, 33,
		kPMTIWaitRandom,	4000, 6000,
		kPMTITargetSet,		kItemPS10Target9, 1,
		kPMTIEnemySet,		kItemPS10Target9,
		kPMTIFacing,		216,
		kPMTIPlaySound,		32, 33,
		kPMTIMove,			69,
		kPMTIWait,			100,
		kPMTIPlaySound,		32, 33,
		kPMTIRotate,		498, 100,
		kPMTIWait,			100,
		kPMTIShoot,			27, 33,
		kPMTIPlaySound,		0, 33,
		kPMTIRotate,		216, 100,
		kPMTIPlaySound,		32, 33,
		kPMTIMove,			0,
		kPMTIObstacleReset,	kItemPS10Target9,
		kPMTIPausedSet,		kItemPS10Target9,
		kPMTIRestart
	};
	return trackData;
}

void SceneScriptPS10::InitializeScene() {
	Police_Maze_Set_Pause_State(true);

	if (Game_Flag_Query(kFlagPS11toPS10)) {
		float x = World_Waypoint_Query_X(4);
		float y = World_Waypoint_Query_Y(4);
		float z = World_Waypoint_Query_Z(4);
		Setup_Scene_Information(x, y, z, 280);
	} else {
		Setup_Scene_Information(-87.08f, -9.23f, 941.9f, 0);
	}

	Scene_Exit_Add_2D_Exit(1, 0, 0, 20, 479, 3);

	Ambient_Sounds_Remove_All_Non_Looping_Sounds(0);
	Ambient_Sounds_Add_Looping_Sound(387, 50, 1, 1);
	Ambient_Sounds_Add_Looping_Sound( 54, 50, 1, 1);
	Ambient_Sounds_Add_Sound(  1, 10,  50, 16, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(389,  5,  50, 16, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(390,  6,  50, 16, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(443,  2, 100, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(444,  2, 100, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(445,  2, 100, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(446,  2, 100, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(303,  5, 100, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(304,  5, 100, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(305,  5, 100, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(306,  5, 100, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(307,  5, 100, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(308,  5, 100, 17, 27, -100, 100, -101, -101, 0, 0);
}

void SceneScriptPS10::SceneLoaded() {
	Obstacle_Object("PARKMETR01", true);
	Obstacle_Object("PARKMETR02", true);
	Obstacle_Object("PARKMETR03", true);
	Obstacle_Object("PARKMETR07", true);
	Obstacle_Object("PARKMETR08", true);
	Obstacle_Object("PARKMETR10", true);
	Obstacle_Object("PARKMETR11", true);
	Obstacle_Object("PARKMETR15", true);
	Obstacle_Object("PARKMETR16", true);
	Obstacle_Object("TUBE14", true);
	Unclickable_Object("PARKMETR01");
	Unclickable_Object("PARKMETR02");
	Unclickable_Object("PARKMETR03");
	Unclickable_Object("PARKMETR07");
	Unclickable_Object("PARKMETR08");
	Unclickable_Object("PARKMETR10");
	Unclickable_Object("PARKMETR11");
	Unclickable_Object("PARKMETR15");
	Unclickable_Object("PARKMETR16");
	Unobstacle_Object("E.SM.WIRE01", true);

	if (!Query_System_Currently_Loading_Game()) {
		Item_Add_To_World(kItemPS10Target1, 443, kSetPS10_PS11_PS12_PS13,  -240.0f, -80.74f, 145.0f, 989, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS10Target2, 443, kSetPS10_PS11_PS12_PS13,  -240.0f,  -8.74f, 145.0f, 740, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS10Target3, 445, kSetPS10_PS11_PS12_PS13,  -165.0f, 111.53f, -10.0f, 993, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS10Target4, 447, kSetPS10_PS11_PS12_PS13,  -125.0f,  160.0f, -10.0f, 993, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS10Target5, 441, kSetPS10_PS11_PS12_PS13, -246.71f, 205.51f, -20.0f,   0, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS10Target6, 445, kSetPS10_PS11_PS12_PS13,  -27.69f, -86.92f, 434.0f, 999, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS10Target7, 441, kSetPS10_PS11_PS12_PS13, -347.15f,   7.68f, -20.0f, 264, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS10Target8, 449, kSetPS10_PS11_PS12_PS13,   -51.0f,  160.0f, -10.0f, 993, 72, 36, true, false, false, true);
		Item_Add_To_World(kItemPS10Target9, 445, kSetPS10_PS11_PS12_PS13,    39.0f,   9.16f, -20.0f, 738, 72, 36, true, false, false, true);
	}

	Police_Maze_Target_Track_Add(kItemPS10Target1,  -240.0f, -80.74f, 145.0f,  -240.0f,  -8.74f, 145.0f, 15, getPoliceMazePS10TrackData1(), false);
	Police_Maze_Target_Track_Add(kItemPS10Target2,  -240.0f,  -8.74f, 145.0f,  -450.0f,  -8.74f, 145.0f, 70, getPoliceMazePS10TrackData2(), false);
	Police_Maze_Target_Track_Add(kItemPS10Target3,  -165.0f, 111.53f, -10.0f,  -165.0f, 167.53f, -10.0f,  6, getPoliceMazePS10TrackData3(),  true);
	Police_Maze_Target_Track_Add(kItemPS10Target4,  -125.0f,  160.0f, -10.0f,   -51.0f,  160.0f, -10.0f, 35, getPoliceMazePS10TrackData4(), false);
	Police_Maze_Target_Track_Add(kItemPS10Target5, -246.71f, 205.51f, -20.0f, -246.71f, 241.51f, -20.0f,  6, getPoliceMazePS10TrackData5(),  true);
	Police_Maze_Target_Track_Add(kItemPS10Target6,  -27.69f, -86.92f, 434.0f,  -27.69f, -18.92f, 434.0f,  8, getPoliceMazePS10TrackData6(),  true);
	Police_Maze_Target_Track_Add(kItemPS10Target7, -347.15f,   7.68f, -20.0f,    39.0f,   9.16f, -20.0f, 90, getPoliceMazePS10TrackData7(), false);
	Police_Maze_Target_Track_Add(kItemPS10Target8,   -51.0f,  160.0f, -10.0f,  -125.0f,  160.0f, -10.0f, 35, getPoliceMazePS10TrackData8(),  true);
	Police_Maze_Target_Track_Add(kItemPS10Target9,    39.0f,   9.16f, -20.0f, -347.15f,   7.68f, -20.0f, 90, getPoliceMazePS10TrackData9(), false);
	Preload(441);
	Preload(442);
	Preload(443);
	Preload(444);
	Preload(445);
	Preload(446);
	Preload(447);
	Preload(448);
	Preload(449);
	Preload(450);
}

bool SceneScriptPS10::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptPS10::ClickedOn3DObject(const char *objectName, bool combatMode) {
	return false;
}

bool SceneScriptPS10::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptPS10::ClickedOnItem(int itemId, bool combatMode) {
	if (Player_Query_Combat_Mode()) {
		switch (itemId) {
		case kItemPS10Target4:
			Sound_Play(4, 50, 0, 0, 50);
			break;
		case kItemPS10Target5:
			Sound_Play(555, 50, 0, 0, 50);
			break;
		case kItemPS10Target7:
			Sound_Play(555, 50, 0, 0, 50);
			break;
		default:
			Sound_Play(2, 12, 0, 0, 50);
			break;
		}
		Item_Spin_In_World(itemId);
		if (itemId == kItemPS10Target1) {
			Item_Flag_As_Non_Target(kItemPS10Target1);
			Item_Flag_As_Non_Target(kItemPS10Target2);
		}
		if (itemId == kItemPS10Target2) {
			Item_Flag_As_Non_Target(kItemPS10Target1);
			Item_Flag_As_Non_Target(kItemPS10Target2);
		}
		if (itemId == kItemPS10Target3) {
			Item_Flag_As_Non_Target(kItemPS10Target3);
		}
		if (itemId == kItemPS10Target4) {
			Item_Flag_As_Non_Target(kItemPS10Target4);
		}
		if (itemId == kItemPS10Target5) {
			Item_Flag_As_Non_Target(kItemPS10Target5);
		}
		if (itemId == kItemPS10Target6) {
			Item_Flag_As_Non_Target(kItemPS10Target6);
		}
		if (itemId == kItemPS10Target7) {
			Item_Flag_As_Non_Target(kItemPS10Target7);
		}
		if (itemId == kItemPS10Target8) {
			Item_Flag_As_Non_Target(kItemPS10Target8);
		}
		if (itemId == kItemPS10Target9) {
			Item_Flag_As_Non_Target(kItemPS10Target9);
		} else {
			Item_Flag_As_Non_Target(itemId);
		}
		return true;
	}

	return false;
}

bool SceneScriptPS10::ClickedOnExit(int exitId) {
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_Waypoint(kActorMcCoy, 6, 12, true, false)) {
			Game_Flag_Set(kFlagPS10toPS11);
			removeTargets();
			Global_Variable_Decrement(kVariablePoliceMazeScore, kPoliceMazePS10TargetCount - Global_Variable_Query(kVariablePoliceMazePS10TargetCounter));
			Global_Variable_Set(kVariablePoliceMazePS10TargetCounter, kPoliceMazePS10TargetCount);
			Set_Enter(kSetPS10_PS11_PS12_PS13, kScenePS11);
		}
		return true;
	}

	return false;
}

bool SceneScriptPS10::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptPS10::SceneFrameAdvanced(int frame) {
}

void SceneScriptPS10::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptPS10::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagPS11toPS10)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -352.09f, -9.23f, 267.95f, 0, false, true, 0);
		Police_Maze_Set_Pause_State(false);
		Game_Flag_Reset(kFlagPS11toPS10);
		//return true;
		return;
	} else {
		Player_Set_Combat_Mode(true);
		Loop_Actor_Walk_To_Waypoint(kActorMcCoy, 5, 0, false, true);
		Actor_Says(kActorAnsweringMachine, 280, kAnimationModeTalk);
		Actor_Says(kActorAnsweringMachine, 290, kAnimationModeTalk);
		Actor_Says(kActorAnsweringMachine, 300, kAnimationModeTalk);
		Police_Maze_Set_Pause_State(false);
		//return true;
		return;
	}
}

void SceneScriptPS10::PlayerWalkedOut() {
}

void SceneScriptPS10::DialogueQueueFlushed(int a1) {
}

void SceneScriptPS10::removeTargets() {
	Item_Remove_From_World(kItemPS10Target1);
	Item_Remove_From_World(kItemPS10Target2);
	Item_Remove_From_World(kItemPS10Target3);
	Item_Remove_From_World(kItemPS10Target4);
	Item_Remove_From_World(kItemPS10Target5);
	Item_Remove_From_World(kItemPS10Target6);
	Item_Remove_From_World(kItemPS10Target7);
	Item_Remove_From_World(kItemPS10Target8);
	Item_Remove_From_World(kItemPS10Target9);
}

} // End of namespace BladeRunner
