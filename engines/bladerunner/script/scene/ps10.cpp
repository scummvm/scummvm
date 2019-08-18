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
int SceneScriptPS10::getPoliceMazePS10TargetCount() {
	return kPoliceMazePS10TargetCount;
}

static const int *getPoliceMazePS10TrackData1() {   // Enemy linked series (kItemPS10Target1, kItemPS10Target2) - Rotating reveal
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS10TargetCounter, kPoliceMazePS10TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS10TargetCounter, kPoliceMazePS10TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS10Target1, 0,  // remove target-able here
		kPMTITargetSet,       kItemPS10Target2, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS10Target1,
		kPMTIObstacleReset,   kItemPS10Target2,
		kPMTIFacing,          989,
		kPMTIPosition,        0,
		kPMTITargetSet,       kItemPS10Target1, 1,
		kPMTITargetSet,       kItemPS10Target2, 1,
		kPMTIEnemyReset,      kItemPS10Target1,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTIEnemyReset,      kItemPS10Target2,     // both targets should clear their enemy flag here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIWaitRandom,      3000, 5000,
		kPMTIObstacleSet,     kItemPS10Target1,
		kPMTIPlaySound,       kSfxUPTARG3, 100,
		kPMTIMove,            14,
		kPMTIWait,            1000,
		kPMTIRotate,          740, 80,
		kPMTIEnemySet,        kItemPS10Target1,     // rotate - reveal
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTIEnemySet,        kItemPS10Target2,     // both targets should set their enemy flag here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIWait,            0,
		kPMTIRotate,          488, 80,
		kPMTIWait,            1000,
		kPMTIShoot,           kSfxSMCAL3, 33,
		kPMTIWait,            0,
		kPMTIRotate,          740, 80,
		kPMTIPausedReset,     kItemPS10Target2,     // kItemPS10Target2 continues the route of this item
		kPMTIObstacleReset,   kItemPS10Target1,	    // kItemPS10Target1 becomes invisible
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS10Target1, 0,  // remove target-able here - only for Target1 item
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleSet,     kItemPS10Target2,	    // kItemPS10Target2 becomes visible in kItemPS10Target1's place
		kPMTIPausedSet,       kItemPS10Target1,
		kPMTIPosition,        0,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS10TrackData2() {   // Enemy linked series (kItemPS10Target1, kItemPS10Target2)
	static int trackData[] = {
		kPMTIFacing,          740,
		kPMTIPosition,        0,
		kPMTIEnemySet,        kItemPS10Target2,     // [redundant after bug fix]
		kPMTIMove,            69,
		kPMTIWait,            500,
		kPMTIObstacleReset,   kItemPS10Target2,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS10Target2, 0,  // remove target-able here - only for Target2 item
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset,     kItemPS10Target5,
		kPMTIPausedSet,       kItemPS10Target2,
		kPMTIPosition,        0,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS10TrackData3() {   // Enemy (kItemPS10Target3) - Starts activated - Rotating reveal
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS10TargetCounter, kPoliceMazePS10TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS10TargetCounter, kPoliceMazePS10TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS10Target3, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS10Target3,
		kPMTIFacing,          993,
		kPMTIPosition,        0,
		kPMTIWaitRandom,      3000, 5000,
		kPMTIObstacleSet,     kItemPS10Target3,
		kPMTIPlaySound,       kSfxUPTARG3, 100,
		kPMTITargetSet,       kItemPS10Target3, 1,
		kPMTIEnemyReset,      kItemPS10Target3,
		kPMTIMove,            5,
		kPMTIWait,            1000,
		kPMTIEnemySet,        kItemPS10Target3,     // rotate - reveal
		kPMTIRotate,          233, 80,
		kPMTIWait,            0,
		kPMTIRotate,          491, 80,
		kPMTIWait,            500,
		kPMTIShoot,           kSfxSMCAL3, 33,
		kPMTIWait,            500,
		kPMTIRotate,          233, 80,
		kPMTIWait,            0,
		kPMTIRotate,          993, 80,
		kPMTIPlaySound,       kSfxTARGUP6, 33,
		kPMTIMove,            0,
		kPMTIObstacleReset,   kItemPS10Target3,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS10Target3, 0,  // remove target-able here
//		kPMTIPausedSet,       kItemPS10Target3,     // intended: Original kItemPS10Target3 does not get paused - it loops on its own
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS10TrackData4() {   // Innocent (kItemPS10Target4)
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS10TargetCounter, kPoliceMazePS10TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS10TargetCounter, kPoliceMazePS10TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS10Target4, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS10Target4,
		kPMTIFacing,          993,
		kPMTIPosition,        0,
		kPMTIWaitRandom,      3000, 6000,
		kPMTIObstacleSet,     kItemPS10Target4,
		kPMTIPlaySound,       kSfxUPTARG3, 100,
		kPMTITargetSet,       kItemPS10Target4, 1,
		kPMTIEnemyReset,      kItemPS10Target4,
		kPMTIMove,            34,
		kPMTIWait,            500,
		kPMTIRotate,          491, 80,
		kPMTIMove,            0,
		kPMTILeave,
		kPMTIObstacleReset,   kItemPS10Target4,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS10Target4, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset,     kItemPS10Target8,
		kPMTIPausedSet,       kItemPS10Target4,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS10TrackData5() {   // Innocent (kItemPS10Target5) - Starts activated
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS10TargetCounter, kPoliceMazePS10TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS10TargetCounter, kPoliceMazePS10TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS10Target5, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS10Target5,
		kPMTIFacing,          0,
		kPMTIPosition,        0,
		kPMTIWaitRandom,      4000, 6000,
		kPMTIObstacleSet,     kItemPS10Target5,
		kPMTIPlaySound,       kSfxUPTARG3, 100,
		kPMTITargetSet,       kItemPS10Target5, 1,
		kPMTIEnemyReset,      kItemPS10Target5,
		kPMTIMove,            5,
		kPMTIWait,            1000,
		kPMTIRotate,          512, 100,
		kPMTIWait,            2000,
		kPMTIRotate,          0, -100,
		kPMTIPlaySound,       kSfxTARGUP6, 33,
		kPMTIMove,            0,
		kPMTILeave,
		kPMTIObstacleReset,   kItemPS10Target5,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS10Target5, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset,     kItemPS10Target1,
		kPMTIPausedSet,       kItemPS10Target5,
		kPMTIRestart
	};
	return trackData;
}

// NOTE Track 6 is used only once as is; it's activated when entering the room
static const int *getPoliceMazePS10TrackData6() {   // Enemy (kItemPS10Target6) - Starts activated - Rotating reveal
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS10TargetCounter, kPoliceMazePS10TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS10TargetCounter, kPoliceMazePS10TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS10Target6, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS10Target6,
		kPMTIFacing,          999,
		kPMTIPosition,        0,
		kPMTIWaitRandom,      4000, 6000,
		kPMTIObstacleSet,     kItemPS10Target6,
		kPMTIPlaySound,       kSfxUPTARG3, 100,
		kPMTITargetSet,       kItemPS10Target6, 1,
		kPMTIEnemyReset,      kItemPS10Target6,
		kPMTIMove,            7,
		kPMTIWait,            500,
		kPMTIEnemySet,        kItemPS10Target6,     // rotate - reveal
		kPMTIRotate,          750, 80,
		kPMTIWait,            0,
		kPMTIRotate,          500, 80,
		kPMTIWait,            1000,
		kPMTIShoot,           kSfxSMCAL3, 33,
		kPMTIWait,            0,
		kPMTIRotate,          750, 80,
		kPMTIWait,            0,
		kPMTIRotate,          999, 80,
		kPMTIPlaySound,       kSfxTARGUP6, 33,
		kPMTIMove,            0,
		kPMTIObstacleReset,   kItemPS10Target6,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS10Target6, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset,     kItemPS10Target7,
		kPMTIPausedReset,     kItemPS10Target9,
		kPMTIPausedSet,       kItemPS10Target6,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS10TrackData7() {   // Innocent (kItemPS10Target7)
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS10TargetCounter, kPoliceMazePS10TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS10TargetCounter, kPoliceMazePS10TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS10Target7, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS10Target7,
		kPMTIFacing,          264,
		kPMTIPosition,        0,
		kPMTIWaitRandom,      3000, 6000,
		kPMTITargetSet,       kItemPS10Target7, 1,
		kPMTIEnemyReset,      kItemPS10Target7,
		kPMTIObstacleSet,     kItemPS10Target7,
		kPMTIMove,            89,
		kPMTIWaitRandom,      4000, 8000,
		kPMTIFacing,          776,
		kPMTIMove,            0,
		kPMTILeave,
		kPMTIObstacleReset,   kItemPS10Target7,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS10Target7, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedSet,       kItemPS10Target7,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS10TrackData8() {   // Enemy (kItemPS10Target8) - Starts activated - Rotating reveal
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS10TargetCounter, kPoliceMazePS10TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS10TargetCounter, kPoliceMazePS10TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS10Target8, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS10Target8,
		kPMTIFacing,          993,
		kPMTIPosition,        0,
		kPMTIWaitRandom,      4000, 6000,
		kPMTIObstacleSet,     kItemPS10Target8,
		kPMTIPlaySound,       kSfxUPTARG3, 100,
		kPMTITargetSet,       kItemPS10Target8, 1,
		kPMTIEnemyReset,      kItemPS10Target8,
		kPMTIMove,            34,
		kPMTIWait,            500,
		kPMTIEnemySet,        kItemPS10Target8,     // rotate - reveal
		kPMTIRotate,          491, 80,
		kPMTIMove,            20,
		kPMTIWait,            0,                    // this results in shooting too fast - TODO maybe introduce a small wait here (50 or 150)
		kPMTIShoot,           kSfxSMCAL3, 33,
		kPMTIMove,            0,
		kPMTIObstacleReset,   kItemPS10Target8,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS10Target8, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset,     kItemPS10Target4,
		kPMTIPausedSet,       kItemPS10Target8,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS10TrackData9() {   // Special (kItemPS10Target9) - Enemy x2
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS10TargetCounter, kPoliceMazePS10TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS10TargetCounter, kPoliceMazePS10TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS10Target9, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS10Target9,
		kPMTIFacing,          738,
		kPMTIPosition,        0,
		kPMTIWaitRandom,      2000, 5000,
		kPMTITargetSet,       kItemPS10Target9, 1,
		kPMTIEnemySet,        kItemPS10Target9,
		kPMTIObstacleSet,     kItemPS10Target9,
		kPMTIPlaySound,       kSfxCROSLOCK, 33,
		kPMTIMove,            23,
		kPMTIPlaySound,       kSfxCROSLOCK, 33,
		kPMTIWait,            200,
		kPMTIPlaySound,       kSfxTARGUP4, 33,
		kPMTIRotate,          498, 100,
		kPMTIPlaySound,       kSfxCROSLOCK, 33,
		kPMTIWait,            100,
		kPMTIShoot,           kSfxSMCAL3, 33,
		kPMTIPlaySound,       kSfxTARGUP4, 33,
		kPMTIMove,            35,
		kPMTIPlaySound,       kSfxTARGUP4, 33,
		kPMTIWait,            100,
		kPMTIShoot,           kSfxSMCAL3, 33,
		kPMTIPlaySound,       kSfxCROSLOCK, 33,
		kPMTIMove,            23,
		kPMTIPlaySound,       kSfxTARGUP4, 33,
		kPMTIWait,            100,
		kPMTIShoot,           kSfxSMCAL3, 33,
		kPMTIPlaySound,       kSfxTARGUP4, 33,
		kPMTIRotate,          758, 100,
		kPMTIPlaySound,       kSfxTARGUP4, 33,
		kPMTIMove,            89,
		kPMTIPlaySound,       kSfxCROSLOCK, 33,
		kPMTIWaitRandom,      4000, 6000,
		kPMTITargetSet,       kItemPS10Target9, 1,  // intended: special: "second" enemy (re-using the target of the track)
		kPMTIEnemySet,        kItemPS10Target9,     // intended: special: "second" enemy (re-using the target of the track)
		kPMTIFacing,          216,
		kPMTIPlaySound,       kSfxTARGUP4, 33,
		kPMTIMove,            69,
		kPMTIWait,            100,
		kPMTIPlaySound,       kSfxTARGUP4, 33,
		kPMTIRotate,          498, 100,
		kPMTIWait,            100,
		kPMTIShoot,           kSfxSMCAL3, 33,
		kPMTIPlaySound,       kSfxCROSLOCK, 33,
		kPMTIRotate,          216, 100,
		kPMTIPlaySound,       kSfxTARGUP4, 33,
		kPMTIMove,            0,
		kPMTIObstacleReset,   kItemPS10Target9,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS10Target9, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedSet,       kItemPS10Target9,
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

	Ambient_Sounds_Remove_All_Non_Looping_Sounds(false);
	Ambient_Sounds_Add_Looping_Sound(kSfxESPLOOP2, 50, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxCTRAIN1,  50, 1, 1);
	Ambient_Sounds_Add_Sound(kSfxSTEAM1,  10,  50, 16, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSTEAMY1,  5,  50, 16, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSTEAMY2,  6,  50, 16, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY4,   2, 100, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY5,   2, 100, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY6,   2, 100, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY7,   2, 100, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN1,   5, 100, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN2,   5, 100, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN3,   5, 100, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE1,  5, 100, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE2,  5, 100, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE3,  5, 100, 17, 27, -100, 100, -101, -101, 0, 0);
#if BLADERUNNER_ORIGINAL_BUGS
#else
	if (!Game_Flag_Query(kFlagPS11toPS10)) {
		// Moved here from PS15
		Sound_Play(kSfxLABBUZZ1, 90, 0, 0, 50);
	}
#endif // BLADERUNNER_ORIGINAL_BUGS
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
#if BLADERUNNER_ORIGINAL_BUGS
		bool targetStateMZ = true;
#else
// every maze target begins as NON-targetable
		bool targetStateMZ = false;
#endif // BLADERUNNER_ORIGINAL_BUGS
		Item_Add_To_World(kItemPS10Target1, kModelAnimationMaleTargetWithGunActive,     kSetPS10_PS11_PS12_PS13,  -240.0f, -80.74f, 145.0f, 989, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS10Target2, kModelAnimationMaleTargetWithGunActive,     kSetPS10_PS11_PS12_PS13,  -240.0f,  -8.74f, 145.0f, 740, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS10Target3, kModelAnimationMaleTargetWithShotgunActive, kSetPS10_PS11_PS12_PS13,  -165.0f, 111.53f, -10.0f, 993, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS10Target4, kModelAnimationFemaleTargetWithBabyActive,  kSetPS10_PS11_PS12_PS13,  -125.0f,  160.0f, -10.0f, 993, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS10Target5, kModelAnimationMaleTargetEmptyHandsActive,  kSetPS10_PS11_PS12_PS13, -246.71f, 205.51f, -20.0f,   0, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS10Target6, kModelAnimationMaleTargetWithShotgunActive, kSetPS10_PS11_PS12_PS13,  -27.69f, -86.92f, 434.0f, 999, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS10Target7, kModelAnimationMaleTargetEmptyHandsActive,  kSetPS10_PS11_PS12_PS13, -347.15f,   7.68f, -20.0f, 264, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS10Target8, kModelAnimationFemaleTargetWithGunActive,   kSetPS10_PS11_PS12_PS13,   -51.0f,  160.0f, -10.0f, 993, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS10Target9, kModelAnimationMaleTargetWithShotgunActive, kSetPS10_PS11_PS12_PS13,    39.0f,   9.16f, -20.0f, 738, 72, 36, targetStateMZ, false, false, true);

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
	Preload(kModelAnimationMaleTargetEmptyHandsActive);
	Preload(kModelAnimationMaleTargetEmptyHandsDead);
	Preload(kModelAnimationMaleTargetWithGunActive);
	Preload(kModelAnimationMaleTargetWithGunDead);
	Preload(kModelAnimationMaleTargetWithShotgunActive);
	Preload(kModelAnimationMaleTargetWithShotgunDead);
	Preload(kModelAnimationFemaleTargetWithBabyActive);
	Preload(kModelAnimationFemaleTargetWithBabyDead);
	Preload(kModelAnimationFemaleTargetWithGunActive);
	Preload(kModelAnimationFemaleTargetWithGunDead);
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
			Sound_Play(kSfxFEMHURT2, 50, 0, 0, 50);
			break;
		case kItemPS10Target5:              // fall through
		case kItemPS10Target7:
			Sound_Play(kSfxMALEHURT, 50, 0, 0, 50);
			break;
		default:
			Sound_Play(kSfxSPINNY1,  12, 0, 0, 50);
			break;
		}

#if BLADERUNNER_ORIGINAL_BUGS
		Item_Spin_In_World(itemId);
#endif // BLADERUNNER_ORIGINAL_BUGS
		switch (itemId) {
		case kItemPS10Target1:              // fall through // treated the same as kItemPS10Target2 (In the original code they are the same target in different tracks)
		case kItemPS10Target2:
#if BLADERUNNER_ORIGINAL_BUGS
#else
			if (Item_Query_Visible(kItemPS10Target1)) { // without this check, target2 seems to get the spinning while the visible target1 stays put
				Item_Spin_In_World(kItemPS10Target1);
			} else {
				Item_Spin_In_World(kItemPS10Target2);
			}
#endif // BLADERUNNER_ORIGINAL_BUGS
			Item_Flag_As_Non_Target(kItemPS10Target1);
			Item_Flag_As_Non_Target(kItemPS10Target2);
			break;
		case kItemPS10Target3:              // fall through
		case kItemPS10Target4:              // fall through
		case kItemPS10Target5:              // fall through
		case kItemPS10Target6:              // fall through
		case kItemPS10Target7:              // fall through
		case kItemPS10Target8:              // fall through
		case kItemPS10Target9:              // fall through
		default:
#if BLADERUNNER_ORIGINAL_BUGS
#else
			Item_Spin_In_World(itemId);
#endif // BLADERUNNER_ORIGINAL_BUGS
			Item_Flag_As_Non_Target(itemId);
			break;
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
//			Global_Variable_Decrement(kVariablePoliceMazeScore, kPoliceMazePS10TargetCount - Global_Variable_Query(kVariablePoliceMazePS10TargetCounter));
			Police_Maze_Decrement_Score(kPoliceMazePS10TargetCount - Global_Variable_Query(kVariablePoliceMazePS10TargetCounter));
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
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -352.09f, -9.23f, 267.95f, 0, false, true, false);
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
