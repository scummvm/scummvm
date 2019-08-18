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

static int kPoliceMazePS13TargetCount = 20;

int SceneScriptPS13::getPoliceMazePS13TargetCount() {
	return kPoliceMazePS13TargetCount;
}

void SceneScriptPS13::InitializeScene() {
	Police_Maze_Set_Pause_State(true);
	if (Game_Flag_Query(kFlagPS12toPS13)) {
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, 0, false);
		Scene_Loop_Set_Default(1);
		Game_Flag_Reset(kFlagPS12toPS13);
		Setup_Scene_Information(World_Waypoint_Query_X(10), World_Waypoint_Query_Y(10), World_Waypoint_Query_Z(10), 200);
	} else {
		Scene_Loop_Set_Default(1);
		Setup_Scene_Information(World_Waypoint_Query_X(11), World_Waypoint_Query_Y(11), World_Waypoint_Query_Z(11), 840);
	}

	Scene_Exit_Add_2D_Exit(0, 0, 460, 639, 479, 2);
	Scene_Exit_Add_2D_Exit(1, 0,   0,  20, 479, 3);

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
}

static const int *getPoliceMazePS13TrackData46() {  // Enemy (kItemPS13Target1) - Starts activated
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS13TargetCounter, kPoliceMazePS13TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS13TargetCounter, kPoliceMazePS13TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS13Target1, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS13Target1,
		kPMTIFacing,          960,
		kPMTIPosition,        0,
		kPMTIWait,            2000,
		kPMTIObstacleSet,     kItemPS13Target1,
		kPMTIPlaySound,       kSfxTARGUP5, 33,
		kPMTITargetSet,       kItemPS13Target1, 1,
		kPMTIEnemySet,        kItemPS13Target1,
		kPMTIWait,            1000,
		kPMTIMove,            5,
		kPMTIWait,            500,
		kPMTIShoot,           kSfxSMCAL3, 33,
		kPMTIWait,            500,
		kPMTIMove,            0,
		kPMTIObstacleReset,   kItemPS13Target1,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS13Target1, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of2, kItemPS13Target14, kItemPS13Target15,
		kPMTIPausedSet,       kItemPS13Target1,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS13TrackData47() {  // Enemy (kItemPS13Target2) - Starts activated
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS13TargetCounter, kPoliceMazePS13TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS13TargetCounter, kPoliceMazePS13TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS13Target2, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS13Target2,
		kPMTIFacing,          823,
		kPMTIPosition,        0,
#if BLADERUNNER_ORIGINAL_BUGS
		kPMTIWaitRandom,      5000, 5000,
#else
		kPMTIWaitRandom,      5000, 6000,
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleSet,     kItemPS13Target2,
		kPMTIPlaySound,       kSfxTARGUP5, 33,
		kPMTITargetSet,       kItemPS13Target2, 1,
		kPMTIEnemySet,        kItemPS13Target2,
		kPMTIMove,            9,
		kPMTIWait,            2000,
		kPMTIShoot,           kSfxSMCAL3, 33,
		kPMTIMove,            0,
		kPMTIObstacleReset,   kItemPS13Target2,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS13Target2, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of2, kItemPS13Target5, kItemPS13Target7,
		kPMTIPausedSet,       kItemPS13Target2,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS13TrackData48() {  // Innocent (kItemPS13Target3) - Starts activated
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS13TargetCounter, kPoliceMazePS13TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS13TargetCounter, kPoliceMazePS13TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS13Target3, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS13Target3,
		kPMTIFacing,          823,
		kPMTIPosition,        0,
#if BLADERUNNER_ORIGINAL_BUGS
		kPMTIWaitRandom,      2000, 2000,
#else
		kPMTIWaitRandom,      2000, 3000,
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleSet,     kItemPS13Target3,
		kPMTIPlaySound,       kSfxTARGUP5, 33,
		kPMTITargetSet,       kItemPS13Target3, 1,
		kPMTIEnemyReset,      kItemPS13Target3,
		kPMTIMove,            9,
		kPMTIWait,            2000,
		kPMTILeave,
		kPMTIMove,            0,
		kPMTIObstacleReset,   kItemPS13Target3,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS13Target3, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of2, kItemPS13Target8, kItemPS13Target6,
		kPMTIPausedSet,       kItemPS13Target3,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS13TrackData49() {  // Enemy (kItemPS13Target4)
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS13TargetCounter, kPoliceMazePS13TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS13TargetCounter, kPoliceMazePS13TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS13Target4, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS13Target4,
		kPMTIFacing,          922,
		kPMTIPosition,        0,
		kPMTIWaitRandom,      3000, 3000,
		kPMTIObstacleSet,     kItemPS13Target4,
		kPMTIPlaySound,       kSfxTARGUP5, 33,
		kPMTITargetSet,       kItemPS13Target4, 1,
		kPMTIEnemySet,        kItemPS13Target4,
		kPMTIMove,            9,
		kPMTIWait,            1000,
		kPMTIShoot,           kSfxSMCAL3, 33,
		kPMTIMove,            0,
		kPMTIObstacleReset,   kItemPS13Target4,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS13Target4, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of2, kItemPS13Target9, kItemPS13Target10,
		kPMTIPausedSet,       kItemPS13Target4,
		kPMTIPosition,        0,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS13TrackData50() {  // Innocent (kItemPS13Target5)
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS13TargetCounter, kPoliceMazePS13TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS13TargetCounter, kPoliceMazePS13TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS13Target5, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS13Target5,
		kPMTIFacing,          823,
		kPMTIPosition,        0,
		kPMTIWaitRandom,      3000, 5000,
		kPMTIObstacleSet,     kItemPS13Target5,
		kPMTIPlaySound,       kSfxTARGUP5, 33,
		kPMTITargetSet,       kItemPS13Target5, 1,
		kPMTIEnemyReset,      kItemPS13Target5,
		kPMTIMove,            9,
		kPMTIWait,            2000,
		kPMTIMove,            0,
		kPMTILeave,
		kPMTIObstacleReset,   kItemPS13Target5,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS13Target5, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of2, kItemPS13Target2, kItemPS13Target7,
		kPMTIPausedSet,       kItemPS13Target5,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS13TrackData51() {  // Enemy (kItemPS13Target6)
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS13TargetCounter, kPoliceMazePS13TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS13TargetCounter, kPoliceMazePS13TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS13Target6, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS13Target6,
		kPMTIFacing,          823,
		kPMTIPosition,        0,
#if BLADERUNNER_ORIGINAL_BUGS
		kPMTIWaitRandom,      2000, 2000,
#else
		kPMTIWaitRandom,      2000, 3000,
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleSet,     kItemPS13Target6,
		kPMTIPlaySound,       kSfxTARGUP5, 33,
		kPMTITargetSet,       kItemPS13Target6, 1,
		kPMTIEnemySet,        kItemPS13Target6,
		kPMTIMove,            9,
		kPMTIWait,            1000,
		kPMTIShoot,           kSfxSMCAL3, 33,
		kPMTIWait,            500,
		kPMTIMove,            0,
		kPMTIObstacleReset,   kItemPS13Target6,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS13Target6, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of2, kItemPS13Target8, kItemPS13Target3,
		kPMTIPausedSet,       kItemPS13Target6,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS13TrackData52() {  // Enemy (kItemPS13Target7) - Rotating reveal
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS13TargetCounter, kPoliceMazePS13TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS13TargetCounter, kPoliceMazePS13TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS13Target7, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS13Target7,
		kPMTIFacing,          305,
		kPMTIPosition,        0,
		kPMTIWaitRandom,      5000, 10000,
		kPMTIObstacleSet,     kItemPS13Target7,
		kPMTIPlaySound,       kSfxTARGUP5, 33,
		kPMTITargetSet,       kItemPS13Target7, 1,
		kPMTIEnemyReset,      kItemPS13Target7,
		kPMTIMove,            9,
		kPMTIWait,            500,
		kPMTIEnemySet,        kItemPS13Target7,     // rotate - reveal
		kPMTIRotate,          555, 80,
		kPMTIWait,            0,
		kPMTIRotate,          833, 80,
		kPMTIWait,            1000,
		kPMTIShoot,           kSfxSMCAL3, 33,
		kPMTIMove,            0,
		kPMTIObstacleReset,   kItemPS13Target7,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS13Target7, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of2, kItemPS13Target2, kItemPS13Target5,
		kPMTIPausedSet,       kItemPS13Target7,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS13TrackData53() {  // Enemy (kItemPS13Target8) - Rotating reveal
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS13TargetCounter, kPoliceMazePS13TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS13TargetCounter, kPoliceMazePS13TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS13Target8, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS13Target8,
		kPMTIFacing,          356,
		kPMTIPosition,        0,
#if BLADERUNNER_ORIGINAL_BUGS
		kPMTIWaitRandom,      3000, 3000,
#else
		kPMTIWaitRandom,      3000, 4000,
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleSet,     kItemPS13Target8,
		kPMTIPlaySound,       kSfxTARGUP5, 33,
		kPMTITargetSet,       kItemPS13Target8, 1,
		kPMTIEnemyReset,      kItemPS13Target8,
		kPMTIMove,            5,
		kPMTIEnemySet,        kItemPS13Target8,     // rotate - reveal
		kPMTIRotate,          868, 200,
		kPMTIWait,            1000,
		kPMTIShoot,           kSfxSMCAL3, 33,
		kPMTIRotate,          356, 60,
		kPMTIWait,            1000,
		kPMTIMove,            0,
		kPMTIObstacleReset,   kItemPS13Target8,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS13Target8, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of2, kItemPS13Target3, kItemPS13Target6,
		kPMTIPausedSet,       kItemPS13Target8,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS13TrackData54() {  // Enemy (kItemPS13Target9) - Rotating reveal
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS13TargetCounter, kPoliceMazePS13TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS13TargetCounter, kPoliceMazePS13TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS13Target9, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS13Target9,
#if BLADERUNNER_ORIGINAL_BUGS
		kPMTIFacing,          512,                  // fix starting orientation
#else
		kPMTIFacing,          250,                  // must have concealed weapon since starting as innocent
#endif
		kPMTIPosition,        0,
		kPMTIWaitRandom,      10000, 20000,
		kPMTIObstacleSet,     kItemPS13Target9,
		kPMTIPlaySound,       kSfxTARGUP5, 33,
		kPMTITargetSet,       kItemPS13Target9, 1,
		kPMTIEnemyReset,      kItemPS13Target9,
		kPMTIMove,            3,
		kPMTIWait,            500,
		kPMTIEnemySet,        kItemPS13Target9,     // rotate - reveal
		kPMTIRotate,          768, 80,
		kPMTIWait,            1000,
		kPMTIShoot,           kSfxSMCAL3, 33,
		kPMTIMove,            0,
		kPMTIObstacleReset,   kItemPS13Target9,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS13Target9, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of2, kItemPS13Target13, kItemPS13Target10,
		kPMTIPausedSet,       kItemPS13Target9,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS13TrackData55() {  // Enemy linked series (kItemPS13Target10, kItemPS13Target11, kItemPS13Target12) - Rotating reveal
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS13TargetCounter, kPoliceMazePS13TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS13TargetCounter, kPoliceMazePS13TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS13Target10, 0,  // remove target-able here - redundant here though
		kPMTITargetSet,       kItemPS13Target11, 0,  // remove target-able here - redundant here though
		kPMTITargetSet,       kItemPS13Target12, 0,  // remove target-able here - redundant here though
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS13Target10,
		kPMTIObstacleReset,   kItemPS13Target11,
		kPMTIObstacleReset,   kItemPS13Target12,
		kPMTITargetSet,       kItemPS13Target10, 1,
		kPMTITargetSet,       kItemPS13Target11, 1,
		kPMTITargetSet,       kItemPS13Target12, 1,
		kPMTIFacing,          327,
		kPMTIPosition,        0,
#if BLADERUNNER_ORIGINAL_BUGS
		kPMTIWaitRandom,      1000, 1000,
#else
		kPMTIWaitRandom,      1000, 2000,
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPlaySound,       kSfxTARGUP5, 33,
		kPMTIEnemyReset,      kItemPS13Target10,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTIEnemyReset,      kItemPS13Target11,    // set the others in the series as non-enemies here too
		kPMTIEnemyReset,      kItemPS13Target12,    // set the others in the series as non-enemies here too
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleSet,     kItemPS13Target10,
		kPMTIMove,            14,
		kPMTIWait,            1000,
		kPMTIPausedReset,     kItemPS13Target11,
		kPMTIObstacleReset,   kItemPS13Target10,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS13Target10, 0, // remove target-able here - only for kItemPS13Target10 item
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleSet,     kItemPS13Target11,
		kPMTIPausedSet,       kItemPS13Target10,
		kPMTIPosition,        0,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS13TrackData56() {  // Enemy linked series (kItemPS13Target10, kItemPS13Target11, kItemPS13Target12) - Rotating reveal
	static int trackData[] = {
		kPMTIFacing,          327,
		kPMTIPosition,        0,
		kPMTIObstacleSet,     kItemPS13Target11,    // redundant (original)
		kPMTIEnemyReset,      kItemPS13Target11,    // [redundant after bug fix] target 10-11-12 still is not revealed as enemy
		kPMTIMove,            14,
		kPMTIWait,            1000,
		kPMTIPausedReset,     kItemPS13Target12,
		kPMTIObstacleReset,   kItemPS13Target11,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS13Target11, 0, // remove target-able here - only for kItemPS13Target11 item
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleSet,     kItemPS13Target12,
		kPMTIPausedSet,       kItemPS13Target11,
		kPMTIPosition,        0,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS13TrackData57() {  // Enemy linked series (kItemPS13Target10, kItemPS13Target11, kItemPS13Target12) - Rotating reveal
	static int trackData[] = {
		kPMTIEnemySet,        kItemPS13Target12,    // rotate - reveal
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTIEnemySet,        kItemPS13Target10, // set all other targets in linked series as enemies here too for consistency
		kPMTIEnemySet,        kItemPS13Target11, // set all other targets in linked series as enemies here too for consistency
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIFacing,          327,
		kPMTIPosition,        0,
		kPMTIObstacleSet,     kItemPS13Target12,    // redundant (original)
		kPMTIRotate,          516, 80,
		kPMTIWait,            0,
		kPMTIRotate,          843, 80,
		kPMTIWait,            1000,
		kPMTIShoot,           kSfxSMCAL3, 33,
		kPMTIWait,            500,
		kPMTIMove,            14,
		kPMTIObstacleReset,   kItemPS13Target12,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS13Target12, 0, // remove target-able here - only for kItemPS13Target12 item
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of2, kItemPS13Target13, kItemPS13Target9,
		kPMTIPausedSet,       kItemPS13Target12,
		kPMTIPosition,        0,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS13TrackData58() {  // Special (kItemPS13Target13) - Starts activated - Innocent x2
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS13TargetCounter, kPoliceMazePS13TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS13TargetCounter, kPoliceMazePS13TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS13Target13, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS13Target13,
		kPMTIFacing,          922,
		kPMTIPosition,        0,
#if BLADERUNNER_ORIGINAL_BUGS
		kPMTIWaitRandom,      3000, 3000,
#else
		kPMTIWaitRandom,      3000, 4000,
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleSet,     kItemPS13Target13,
		kPMTIPlaySound,       kSfxTARGUP5, 33,
		kPMTITargetSet,       kItemPS13Target13, 1,
		kPMTIEnemyReset,      kItemPS13Target13,
		kPMTIMove,            9,
		kPMTIWait,            200,
		kPMTIMove,            0,
		kPMTILeave,                                 // intended: special: credit for "first" innocent escaping
		kPMTITargetSet,       kItemPS13Target13, 1, // intended: special: "second" innocent (re-using the target of the track)
		kPMTIEnemyReset,      kItemPS13Target13,    // intended: special: "second" innocent (re-using the target of the track)
		kPMTIWait,            200,
		kPMTIMove,            9,
		kPMTIWait,            200,
		kPMTIMove,            0,
		kPMTILeave,                                 // credit for "second" / final innocent
		kPMTIObstacleReset,   kItemPS13Target13,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS13Target13, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset,     kItemPS13Target4,
		kPMTIPausedSet,       kItemPS13Target13,
		kPMTIPosition,        0,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS13TrackData62() {  // Enemy (kItemPS13Target14) - Rotating reveal
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS13TargetCounter, kPoliceMazePS13TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS13TargetCounter, kPoliceMazePS13TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS13Target14, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS13Target14,
		kPMTIFacing,          465,
		kPMTIPosition,        0,
#if BLADERUNNER_ORIGINAL_BUGS
		kPMTIWaitRandom,      3000, 3000,
#else
		kPMTIWaitRandom,      3000, 4000,
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleSet,     kItemPS13Target14,
		kPMTIPlaySound,       kSfxTARGUP5, 33,
		kPMTITargetSet,       kItemPS13Target14, 1,
		kPMTIEnemyReset,      kItemPS13Target14,
		kPMTIMove,            14,
		kPMTIWait,            1000,
		kPMTIEnemySet,        kItemPS13Target14,    // rotate - reveal
		kPMTIRotate,          650, 80,
		kPMTIWait,            0,
		kPMTIRotate,          937, 80,
		kPMTIWait,            1000,
		kPMTIShoot,           kSfxSMCAL3, 33,
		kPMTIWait,            500,
		kPMTIRotate,          650, 80,
		kPMTIWait,            0,
		kPMTIRotate,          465, 80,
		kPMTIMove,            0,
		kPMTIObstacleReset,   kItemPS13Target14,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS13Target14, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of2, kItemPS13Target1, kItemPS13Target15,
		kPMTIPausedSet,       kItemPS13Target14,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS13TrackData63() {  // Enemy (kItemPS13Target15) - Rotating reveal
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS13TargetCounter, kPoliceMazePS13TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS13TargetCounter, kPoliceMazePS13TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS13Target15, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS13Target15,
		kPMTIFacing,          465,
		kPMTIPosition,        0,
		kPMTIWait,            3000,
		kPMTIObstacleSet,     kItemPS13Target15,
		kPMTIPlaySound,       kSfxTARGUP5, 33,
		kPMTITargetSet,       kItemPS13Target15, 1,
		kPMTIEnemyReset,      kItemPS13Target15,
		kPMTIMove,            9,
		kPMTIWait,            1000,
		kPMTIEnemySet,        kItemPS13Target15,    // rotate - reveal
		kPMTIRotate,          710, 80,
		kPMTIWait,            0,
		kPMTIRotate,          960, 80,
		kPMTIWait,            1000,
		kPMTIShoot,           kSfxSMCAL3, 33,
		kPMTIWait,            500,
		kPMTIRotate,          710, 80,
		kPMTIWait,            0,
		kPMTIRotate,          460, 80,
		kPMTIMove,            0,
		kPMTIObstacleReset,   kItemPS13Target15,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS13Target15, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of2, kItemPS13Target1, kItemPS13Target14,
		kPMTIPausedSet,       kItemPS13Target15,
		kPMTIRestart
	};
	return trackData;
}


void SceneScriptPS13::SceneLoaded() {
	Obstacle_Object("PARKMETR01", true);
	Obstacle_Object("PARKMETR02", true);
	Obstacle_Object("PARKMETR03", true);
	Obstacle_Object("PARKMETR07", true);
	Obstacle_Object("PARKMETR08", true);
	Obstacle_Object("PARKMETR10", true);
	Obstacle_Object("PARKMETR11", true);
	Obstacle_Object("PARKMETR15", true);
	Obstacle_Object("PARKMETR16", true);
	Unclickable_Object("PARKMETR01");
	Unclickable_Object("PARKMETR02");
	Unclickable_Object("PARKMETR03");
	Unclickable_Object("PARKMETR07");
	Unclickable_Object("PARKMETR08");
	Unclickable_Object("PARKMETR10");
	Unclickable_Object("PARKMETR11");
	Unclickable_Object("PARKMETR15");
	Unclickable_Object("PARKMETR16");

	if (!Query_System_Currently_Loading_Game()) {
#if BLADERUNNER_ORIGINAL_BUGS
		bool targetStateMZ = true;
#else
// every maze target begins as NON-targetable
		bool targetStateMZ = false;
#endif // BLADERUNNER_ORIGINAL_BUGS
		Item_Add_To_World(kItemPS13Target1,  kModelAnimationMaleTargetWithGunActive,     kSetPS10_PS11_PS12_PS13, -372.0f,   -9.0f,  1509.0f, 960, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS13Target2,  kModelAnimationMaleTargetWithGunActive,     kSetPS10_PS11_PS12_PS13, 291.61f,  -0.66f,  1610.3f, 823, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS13Target3,  kModelAnimationFemaleTargetWithBabyActive,  kSetPS10_PS11_PS12_PS13,  -25.0f,  102.0f,  1625.0f, 823, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS13Target4,  kModelAnimationFemaleTargetWithGunActive,   kSetPS10_PS11_PS12_PS13, -45.51f,   -8.8f,  1676.0f, 922, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS13Target5,  kModelAnimationFemaleTargetWithBabyActive,  kSetPS10_PS11_PS12_PS13, 291.61f,  -0.66f,  1610.3f, 823, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS13Target6,  kModelAnimationMaleTargetWithGunActive,     kSetPS10_PS11_PS12_PS13,  -24.0f,  102.0f,  1625.0f, 823, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS13Target7,  kModelAnimationFemaleTargetWithGunActive,   kSetPS10_PS11_PS12_PS13,  180.0f,  -72.7f,  1605.0f, 305, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS13Target8,  kModelAnimationMaleTargetWithGunActive,     kSetPS10_PS11_PS12_PS13, 127.79f,  14.56f, 1703.03f, 356, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS13Target9,  kModelAnimationMaleTargetWithGunActive,     kSetPS10_PS11_PS12_PS13, 136.37f,  -6.84f, 1425.43f, 512, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS13Target10, kModelAnimationMaleTargetEmptyHandsActive,  kSetPS10_PS11_PS12_PS13,  77.83f,  -79.8f,  1520.5f, 327, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS13Target11, kModelAnimationMaleTargetEmptyHandsActive,  kSetPS10_PS11_PS12_PS13,  77.83f,   -7.8f,  1520.5f, 327, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS13Target12, kModelAnimationMaleTargetWithGunActive,     kSetPS10_PS11_PS12_PS13,  -88.0f,   -8.8f,  1520.5f, 327, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS13Target13, kModelAnimationFemaleTargetWithBabyActive,  kSetPS10_PS11_PS12_PS13, -45.51f,   -8.8f,  1676.0f, 922, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS13Target14, kModelAnimationMaleTargetWithShotgunActive, kSetPS10_PS11_PS12_PS13, -300.0f, -79.75f,  1543.0f, 465, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS13Target15, kModelAnimationFemaleTargetWithGunActive,   kSetPS10_PS11_PS12_PS13, -325.0f,  -7.75f,  1543.0f, 465, 72, 36, targetStateMZ, false, false, true);
	}

	Police_Maze_Target_Track_Add(kItemPS13Target1,  -372.0f,   -9.0f,  1509.0f, -345.0f,   -9.0f,  1509.0f,  6, getPoliceMazePS13TrackData46(),  true);
	Police_Maze_Target_Track_Add(kItemPS13Target2,  291.61f,  -0.66f,  1610.3f, 238.83f,   1.03f, 1557.03f, 10, getPoliceMazePS13TrackData47(),  true);
	Police_Maze_Target_Track_Add(kItemPS13Target3,   -25.0f,  102.0f,  1625.0f,  -25.0f,  138.0f,  1625.0f, 10, getPoliceMazePS13TrackData48(),  true);
	Police_Maze_Target_Track_Add(kItemPS13Target4,  -45.51f,   -8.8f,  1676.0f,  15.51f,   -8.8f,  1679.0f, 10, getPoliceMazePS13TrackData49(), false);
	Police_Maze_Target_Track_Add(kItemPS13Target5,  291.61f,  -0.66f,  1610.3f, 238.83f,   1.03f, 1557.03f, 10, getPoliceMazePS13TrackData50(), false);
	Police_Maze_Target_Track_Add(kItemPS13Target6,   -24.0f,  102.0f,  1625.0f,  -24.0f,  138.0f,  1625.0f, 10, getPoliceMazePS13TrackData51(), false);
	Police_Maze_Target_Track_Add(kItemPS13Target7,   180.0f,  -72.7f,  1605.0f,  180.0f,   -0.7f,  1605.0f, 10, getPoliceMazePS13TrackData52(), false);
	Police_Maze_Target_Track_Add(kItemPS13Target8,  127.79f,  14.56f, 1703.03f, -56.07f,   1.89f, 1589.04f,  6, getPoliceMazePS13TrackData53(), false);
	Police_Maze_Target_Track_Add(kItemPS13Target9,  136.37f,  -6.84f, 1425.43f, 117.55f,  -6.84f, 1442.09f,  4, getPoliceMazePS13TrackData54(), false);
	Police_Maze_Target_Track_Add(kItemPS13Target10,  77.83f,  -79.8f,  1520.5f,  77.83f,   -7.8f,  1520.5f, 15, getPoliceMazePS13TrackData55(), false);
	Police_Maze_Target_Track_Add(kItemPS13Target11,  77.83f,   -7.8f,  1520.5f,  -88.0f,   -8.8f,  1520.5f, 15, getPoliceMazePS13TrackData56(), false);
	Police_Maze_Target_Track_Add(kItemPS13Target12,  -88.0f,   -8.8f,  1520.5f,  -88.0f,  -80.8f,  1520.5f, 15, getPoliceMazePS13TrackData57(), false);
	Police_Maze_Target_Track_Add(kItemPS13Target13, -45.51f,   -8.8f,  1676.0f,  15.51f,   -8.8f,  1679.0f, 10, getPoliceMazePS13TrackData58(),  true);
	Police_Maze_Target_Track_Add(kItemPS13Target14, -300.0f, -79.75f,  1543.0f, -300.0f, -14.75f,  1543.0f, 15, getPoliceMazePS13TrackData62(), false);
	Police_Maze_Target_Track_Add(kItemPS13Target15, -325.0f,  -7.75f,  1543.0f, -300.0f,  -7.75f,  1543.0f, 10, getPoliceMazePS13TrackData63(), false);
}

bool SceneScriptPS13::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptPS13::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptPS13::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptPS13::ClickedOnItem(int itemId, bool a2) {
	if (Player_Query_Combat_Mode()) {
		switch (itemId) {
		case kItemPS13Target3:             // fall through
		case kItemPS13Target5:
			Sound_Play(kSfxFEMHURT2, 50, 0, 0, 50);
			break;
		case kItemPS13Target10:            // fall through
		case kItemPS13Target11:
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
		case kItemPS13Target10:            // fall through
		case kItemPS13Target11:            // fall through
		case kItemPS13Target12:
#if BLADERUNNER_ORIGINAL_BUGS
#else
			if (Item_Query_Visible(kItemPS13Target10)) { // without this check, the wrong target might get the spinning while the visible stays put
				Item_Spin_In_World(kItemPS13Target10);
			} else if (Item_Query_Visible(kItemPS13Target11)) {
				Item_Spin_In_World(kItemPS13Target11);
			} else {
				Item_Spin_In_World(kItemPS13Target12);
			}
#endif // BLADERUNNER_ORIGINAL_BUGS
			Item_Flag_As_Non_Target(kItemPS13Target10);
			Item_Flag_As_Non_Target(kItemPS13Target11);
			Item_Flag_As_Non_Target(kItemPS13Target12);
			break;
		case kItemPS13Target1:             // fall through
		case kItemPS13Target2:             // fall through
		case kItemPS13Target3:             // fall through
		case kItemPS13Target4:             // fall through
		case kItemPS13Target5:             // fall through
		case kItemPS13Target6:             // fall through
		case kItemPS13Target7:             // fall through
		case kItemPS13Target8:             // fall through
		case kItemPS13Target9:             // fall through
		case kItemPS13Target13:            // fall through
		case kItemPS13Target14:            // fall through
		case kItemPS13Target15:            // fall through
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

bool SceneScriptPS13::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_Waypoint(kActorMcCoy, 10, 12, true, false)) {
			Game_Flag_Set(kFlagPS13toPS12);
			removeTargets();
			Set_Enter(kSetPS10_PS11_PS12_PS13, kScenePS12);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_Waypoint(kActorMcCoy, 11, 12, true, false)) {
			Game_Flag_Set(kFlagPS13toPS05);
			Player_Set_Combat_Mode(false);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			removeTargets();
//			Global_Variable_Decrement(kVariablePoliceMazeScore, kPoliceMazePS13TargetCount - Global_Variable_Query(kVariablePoliceMazePS13TargetCounter));
			Police_Maze_Decrement_Score(kPoliceMazePS13TargetCount - Global_Variable_Query(kVariablePoliceMazePS13TargetCounter));
			Set_Score(kActorMcCoy, Police_Maze_Query_Score());
//			Common::String scoreString = Common::String::format("Final: %03d", Global_Variable_Query(kVariablePoliceMazeScore)); // Display final score as subtitles
//			Set_Subtitle_Text_On_Screen(scoreString); // Display final score as subtitles
			Global_Variable_Reset(kVariablePoliceMazePS10TargetCounter);
			Global_Variable_Reset(kVariablePoliceMazePS11TargetCounter);
			Global_Variable_Reset(kVariablePoliceMazePS12TargetCounter);
			Global_Variable_Reset(kVariablePoliceMazePS13TargetCounter);
//			Global_Variable_Reset(kVariablePoliceMazeScore);
			Police_Maze_Zero_Score();
			Set_Enter(kSetPS05, kScenePS05);
		}
		return true;
	}
	return false;
}

bool SceneScriptPS13::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptPS13::SceneFrameAdvanced(int frame) {
}

void SceneScriptPS13::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptPS13::PlayerWalkedIn() {
	Police_Maze_Set_Pause_State(false);
}

void SceneScriptPS13::PlayerWalkedOut() {
}

void SceneScriptPS13::DialogueQueueFlushed(int a1) {
}

void SceneScriptPS13::removeTargets() {
	Item_Remove_From_World(kItemPS13Target1);
	Item_Remove_From_World(kItemPS13Target2);
	Item_Remove_From_World(kItemPS13Target3);
	Item_Remove_From_World(kItemPS13Target4);
	Item_Remove_From_World(kItemPS13Target5);
	Item_Remove_From_World(kItemPS13Target6);
	Item_Remove_From_World(kItemPS13Target7);
	Item_Remove_From_World(kItemPS13Target8);
	Item_Remove_From_World(kItemPS13Target9);
	Item_Remove_From_World(kItemPS13Target10);
	Item_Remove_From_World(kItemPS13Target11);
	Item_Remove_From_World(kItemPS13Target12);
	Item_Remove_From_World(kItemPS13Target13);
	Item_Remove_From_World(kItemPS13Target14);
	Item_Remove_From_World(kItemPS13Target15);
}

} // End of namespace BladeRunner
