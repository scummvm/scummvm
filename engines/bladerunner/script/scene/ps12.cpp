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

static int kPoliceMazePS12TargetCount = 20;
int SceneScriptPS12::getPoliceMazePS12TargetCount() {
	return kPoliceMazePS12TargetCount;
}


void SceneScriptPS12::InitializeScene() {
	Police_Maze_Set_Pause_State(true);
	if (Game_Flag_Query(kFlagPS11toPS12)) {
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, 0, false);
		Scene_Loop_Set_Default(1);
		Setup_Scene_Information(World_Waypoint_Query_X(8), World_Waypoint_Query_Y(8), World_Waypoint_Query_Z(8), 512);
	} else {
		Scene_Loop_Set_Default(1);
		Setup_Scene_Information(World_Waypoint_Query_X(9), World_Waypoint_Query_Y(9), World_Waypoint_Query_Z(9), 0);
	}
	Scene_Exit_Add_2D_Exit(0, 0, 460, 639, 479, 2);
	Scene_Exit_Add_2D_Exit(1, 0,   5, 110, 196, 3);
}

static const int *getPoliceMazePS12TrackData29() { // Enemy (kItemPS12Target1) - Starts activated
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS12TargetCounter, kPoliceMazePS12TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS12TargetCounter, kPoliceMazePS12TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS12Target1, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS12Target1,
		kPMTIFacing,          200,
		kPMTIPosition,        0,
		kPMTIWaitRandom,      4000, 10000,
		kPMTIObstacleSet,     kItemPS12Target1,
		kPMTIPlaySound,       kSfxTARGUP5, 33,
		kPMTITargetSet,       kItemPS12Target1, 1,
		kPMTIEnemySet,        kItemPS12Target1,
		kPMTIMove,            5,
		kPMTIWait,            500,
		kPMTIShoot,           kSfxSMCAL3, 33,
		kPMTIPlaySound,       kSfxTARGUP6, 33,
		kPMTIMove,            0,
		kPMTIObstacleReset,   kItemPS12Target1,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS12Target1, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of2, kItemPS12Target16, kItemPS12Target14,
		kPMTIPausedSet,       kItemPS12Target1,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS12TrackData30() { // Enemy (kItemPS12Target2) - Starts activated
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS12TargetCounter, kPoliceMazePS12TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS12TargetCounter, kPoliceMazePS12TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS12Target2, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS12Target2,
		kPMTIFacing,          67,
		kPMTIPosition,        0,
		kPMTIWaitRandom,      3000, 9000,
		kPMTIObstacleSet,     kItemPS12Target2,
		kPMTIPlaySound,       kSfxTARGUP5, 33,
		kPMTITargetSet,       kItemPS12Target2, 1,
		kPMTIEnemySet,        kItemPS12Target2,
		kPMTIMove,            5,
		kPMTIWait,            500,
		kPMTIShoot,           kSfxSMCAL3, 33,
		kPMTIPlaySound,       kSfxTARGUP6, 33,
		kPMTIMove,            0,
		kPMTIObstacleReset,   kItemPS12Target2,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS12Target2, 0, // remove target-able after making target invisible
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of2, kItemPS12Target13, kItemPS12Target11,
		kPMTIPausedSet,       kItemPS12Target2,
#if BLADERUNNER_ORIGINAL_BUGS
		kPMTITargetSet,       kItemPS12Target2, 0, // removes target-able here (original code)
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS12TrackData31() { // Innocent (kItemPS12Target3)
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS12TargetCounter, kPoliceMazePS12TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS12TargetCounter, kPoliceMazePS12TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
		kPMTITargetSet,       kItemPS12Target3, 1,
		kPMTIEnemyReset,      kItemPS12Target3,
#else
		kPMTITargetSet,       kItemPS12Target3, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS12Target3,
		kPMTIFacing,          480,
		kPMTIPosition,        0,
		kPMTIWaitRandom,      3000, 10000,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS12Target3, 1,
		kPMTIEnemyReset,      kItemPS12Target3,
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleSet,     kItemPS12Target3,
		kPMTIMove,            5,
		kPMTIPlaySound,       kSfxTARGUP1, 33,
		kPMTIWait,            1000,
		kPMTIRotate,          968, 100,
		kPMTIPlaySound,       kSfxTARGUP1, 33,
		kPMTIWait,            500,
		kPMTIMove,            0,
		kPMTIPlaySound,       kSfxTARGUP1, 33,
		kPMTILeave,
		kPMTIObstacleReset,   kItemPS12Target3,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS12Target3, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of3, kItemPS12Target12, kItemPS12Target4, kItemPS12Target5,
		kPMTIPausedSet,       kItemPS12Target3,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS12TrackData32() { // Enemy (kItemPS12Target4)
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS12TargetCounter, kPoliceMazePS12TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS12TargetCounter, kPoliceMazePS12TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS12Target4, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS12Target4,
		kPMTIFacing,          1010,
		kPMTIPosition,        0,
		kPMTIWaitRandom,      3000, 10000,
		kPMTIObstacleSet,     kItemPS12Target4,
		kPMTIPlaySound,       kSfxTARGUP5, 33,
		kPMTITargetSet,       kItemPS12Target4, 1,
		kPMTIEnemySet,        kItemPS12Target4,
		kPMTIMove,            5,
		kPMTIWait,            500,
		kPMTIShoot,           kSfxSMCAL3, 33,
		kPMTIRotate,          498, 80,
		kPMTIPlaySound,       kSfxTARGUP1, 33,
		kPMTIWait,            500,
		kPMTIMove,            0,
		kPMTIObstacleReset,   kItemPS12Target4,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS12Target4, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of3, kItemPS12Target5, kItemPS12Target3, kItemPS12Target12,
		kPMTIPausedSet,       kItemPS12Target4,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS12TrackData33() { // Innocent (kItemPS12Target5)
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS12TargetCounter, kPoliceMazePS12TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS12TargetCounter, kPoliceMazePS12TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS12Target5, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS12Target5,
		kPMTIFacing,          540,
		kPMTIPosition,        0,
		kPMTIWaitRandom,      4000, 10000,
		kPMTIObstacleSet,     kItemPS12Target5,
		kPMTITargetSet,       kItemPS12Target5, 1,
		kPMTIEnemyReset,      kItemPS12Target5,
		kPMTIMove,            5,
		kPMTIPlaySound,       kSfxTARGUP1, 33,
		kPMTIWait,            1000,
		kPMTIRotate,          284, 80,
		kPMTIWait,            0,
		kPMTIRotate,          28, 80,
		kPMTIPlaySound,       kSfxTARGUP1, 33,
		kPMTIWait,            1000,
		kPMTIMove,            0,
		kPMTILeave,
		kPMTIObstacleReset,   kItemPS12Target5,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS12Target5, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of3, kItemPS12Target12, kItemPS12Target3, kItemPS12Target4,
		kPMTIPausedSet,       kItemPS12Target5,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS12TrackData34() {  // Special (kItemPS12Target6, kItemPS12Target7, kItemPS12Target8) - Enemy x2
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS12TargetCounter, kPoliceMazePS12TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS12TargetCounter, kPoliceMazePS12TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS12Target6, 0,  // remove target-able here
		kPMTITargetSet,       kItemPS12Target7, 0,  // remove target-able here
		kPMTITargetSet,       kItemPS12Target8, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS12Target6,
		kPMTIObstacleReset,   kItemPS12Target7,
		kPMTIObstacleReset,   kItemPS12Target8,
		kPMTIFacing,          469,
		kPMTIPosition,        0,
		kPMTITargetSet,       kItemPS12Target6, 1,
		kPMTITargetSet,       kItemPS12Target7, 1,
		kPMTITargetSet,       kItemPS12Target8, 1,
		kPMTIWaitRandom,      3000, 10000,
		kPMTIObstacleSet,     kItemPS12Target6,
		kPMTIEnemySet,        kItemPS12Target6,
		kPMTIEnemySet,        kItemPS12Target7,
		kPMTIEnemySet,        kItemPS12Target8,
		kPMTIMove,            5,
		kPMTIPlaySound,       kSfxTARGUP1, 33,
		kPMTIWait,            1000,
		kPMTIRotate,          376, 80,
		kPMTIWait,            0,
		kPMTIRotate,          168, 80,
		kPMTIPlaySound,       kSfxTARGUP1, 33,
		kPMTIMove,            9,
		kPMTIRotate,          33, 80,
		kPMTIWait,            0,
		kPMTIRotate,          15, 80,
		kPMTIPlaySound,       kSfxTARGUP1, 33,
		kPMTIWait,            500,
		kPMTIShoot,           kSfxSMCAL3, 33,
		kPMTIMove,            5,
		kPMTIPlaySound,       kSfxTARGUP1, 33,
		kPMTIRotate,          168, 80,
		kPMTIWait,            0,
		kPMTIRotate,          376, 80,
		kPMTIWait,            0,
		kPMTIRotate,          469, 80,
		kPMTIPlaySound,       kSfxTARGUP1, 33,
		kPMTIWait,            500,
		kPMTITargetSet,       kItemPS12Target6, 1,  // intended: special: "second" enemy (re-using the targets of the track)
		kPMTITargetSet,       kItemPS12Target7, 1,  // intended: special: "second" enemy (re-using the targets of the track)
		kPMTITargetSet,       kItemPS12Target8, 1,  // intended: special: "second" enemy (re-using the targets of the track)
		kPMTIEnemySet,        kItemPS12Target6,     // intended: special: "second" enemy (re-using the targets of the track)
		kPMTIEnemySet,        kItemPS12Target7,     // intended: special: "second" enemy (re-using the targets of the track)
		kPMTIEnemySet,        kItemPS12Target8,     // intended: special: "second" enemy (re-using the targets of the track)
		kPMTIRotate,          376, 80,
		kPMTIWait,            0,
		kPMTIRotate,          168, 80,
		kPMTIMove,            9,
		kPMTIRotate,          33, 80,
		kPMTIWait,            0,
		kPMTIRotate,          15, 80,
		kPMTIPlaySound,       kSfxTARGUP1, 33,
		kPMTIWait,            500,
		kPMTIShoot,           kSfxSMCAL3, 33,
		kPMTIMove,            5,
		kPMTIPlaySound,       kSfxTARGUP1, 33,
		kPMTIWait,            0,
		kPMTIRotate,          469, 80,
		kPMTIWait,            500,
		kPMTIRotate,          198, 80,
		kPMTIPlaySound,       kSfxTARGUP1, 33,
		kPMTIWait,            0,
		kPMTIMove,            19,
		kPMTIPlaySound,       kSfxTARGUP1, 33,
		kPMTIWait,            0,
#if BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS12Target6,
		kPMTIPausedReset,     kItemPS12Target7,
		kPMTIPausedSet,       kItemPS12Target6,
#else
		// Prevent blinking at hand-off
		kPMTIPausedReset,     kItemPS12Target7,
		kPMTIObstacleReset,   kItemPS12Target6,
		kPMTITargetSet,       kItemPS12Target6, 0,  // remove target-able here - only for this item
		kPMTIObstacleSet,     kItemPS12Target7,
		kPMTIPausedSet,       kItemPS12Target6,
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPosition,        0,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS12TrackData35() {  // Enemy (kItemPS12Target6, kItemPS12Target7, kItemPS12Target8)
	static int trackData[] = {
		kPMTIObstacleSet,     kItemPS12Target7,  // [redundant after bug fix]
		kPMTIFacing,          198,
		kPMTIPosition,        0,
		kPMTIRotate,          469, 80,
		kPMTIMove,            9,
		kPMTIWait,            0,
#if BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS12Target7,
		kPMTIPausedReset,     kItemPS12Target8,
		kPMTIPausedSet,       kItemPS12Target7,
		kPMTIFacing,          198,
		kPMTIObstacleSet,     kItemPS12Target7,    // this is a bad idea, since kItemPS12Target7 role has ended
#else
		// prevent target from blinking midway (at the hand-off of kItemPS12Target7 to kItemPS12Target8
		// adopt the better instruction sequence logic from PS10 similar case (kItemPS10Target1 - kItemPS10Target2)
		kPMTIPausedReset,     kItemPS12Target8,    // kItemPS12Target8 continues the route of this item
		kPMTIObstacleReset,   kItemPS12Target7,	   // kItemPS12Target7 becomes invisible
		kPMTITargetSet,       kItemPS12Target7, 0, // remove target-able here - only for this item
		kPMTIObstacleSet,     kItemPS12Target8,	   // kItemPS12Target8 becomes visible in kItemPS12Target7's place
		kPMTIPausedSet,       kItemPS12Target7,
		kPMTIFacing,          198,
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPosition,        0,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS12TrackData36() {  // Enemy (kItemPS12Target6, kItemPS12Target7, kItemPS12Target8)
	static int trackData[] = {
		kPMTIObstacleSet,     kItemPS12Target8,     // [redundant after bug fix]
		kPMTIFacing,          469,
		kPMTIPosition,        0,
		kPMTIObstacleSet,     kItemPS12Target8,     // redundant (original)
		kPMTIMove,            9,
		kPMTIPlaySound,       kSfxTARGUP1, 33,
		kPMTIWait,            1000,
		kPMTIObstacleReset,   kItemPS12Target6,
		kPMTIObstacleReset,   kItemPS12Target7,
		kPMTIObstacleReset,   kItemPS12Target8,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS12Target8, 0,  // remove target-able here - only for this item
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset,     kItemPS12Target9,
		kPMTIPausedSet,       kItemPS12Target8,
		kPMTIFacing,          469,
#if BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleSet,     kItemPS12Target8,
#endif
		kPMTIPosition,        0,
		kPMTIRestart
	};
	return trackData;
}

// kItemPS12Target9 and kItemPS12Target10 go together (innocent and enemy who uses innocent as a human shield)
// They count as one activated target, but they are credited separately (+1 for not shooting the innocent, +1 for shooting the enemy)
static const int *getPoliceMazePS12TrackData37() {  // Special: Innocent (kItemPS12Target9) - Starts activated, goes together with enemy kItemPS12Target10 (not same target)
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS12TargetCounter, kPoliceMazePS12TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS12TargetCounter, kPoliceMazePS12TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS12Target9, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS12Target9,
		kPMTIWaitRandom,      3000, 6000,
		kPMTIPausedReset,     kItemPS12Target10,    // kItemPS12Target10 is unpaused early since he is the enemy using kItemPS12Target9 as human shield
		kPMTIFacing,          1010,
		kPMTIPosition,        0,
		kPMTIWait,            2000,
		kPMTIObstacleSet,     kItemPS12Target9,
		kPMTIPlaySound,       kSfxTARGUP5, 33,
		kPMTITargetSet,       kItemPS12Target9, 1,
		kPMTIEnemyReset,      kItemPS12Target9,
		kPMTIMove,            9,
		kPMTIWait,            3000,
		kPMTIPlaySound,       kSfxTARGUP6, 33,
		kPMTIMove,            0,
		kPMTILeave,
		kPMTIObstacleReset,   kItemPS12Target9,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS12Target9, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedSet,       kItemPS12Target9,
		kPMTIPosition,        0,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS12TrackData38() {  // Enemy (kItemPS12Target10) - Special (bonus point)
	static int trackData[] = {
#if BLADERUNNER_ORIGINAL_BUGS
#else
//		kPMTIActivate,        kVariablePoliceMazePS12TargetCounter, kPoliceMazePS12TargetCount, // intended: special: Original missing activate instruction
//		kPMTIVariableInc,     kVariablePoliceMazePS12TargetCounter, kPoliceMazePS12TargetCount, // intended: special: Original missing increase counter instruction
		kPMTITargetSet,       kItemPS12Target10, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS12Target10,
		kPMTIFacing,          990,
		kPMTIPosition,        0,
		kPMTIWait,            3000,
		kPMTIObstacleSet,     kItemPS12Target10,
		kPMTIPlaySound,       kSfxTARGUP5, 33,
		kPMTITargetSet,       kItemPS12Target10, 1,
		kPMTIEnemySet,        kItemPS12Target10,
		kPMTIMove,            9,
		kPMTIWait,            1000,
		kPMTIShoot,           kSfxLGCAL1, 33,
		kPMTIPlaySound,       kSfxTARGUP6, 33,
		kPMTIMove,            0,
		kPMTIObstacleReset,   kItemPS12Target10,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS12Target10, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset,     kItemPS12Target6,
		kPMTIPausedSet,       kItemPS12Target10,
		kPMTIPosition,        0,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS12TrackData39() {  // Enemy (kItemPS12Target11) - Rotating reveal
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS12TargetCounter, kPoliceMazePS12TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS12TargetCounter, kPoliceMazePS12TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTIEnemyReset,      kItemPS12Target11,
		kPMTITargetSet,       kItemPS12Target11, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS12Target11,
		kPMTIFacing,          513,
		kPMTIPosition,        0,
#if BLADERUNNER_ORIGINAL_BUGS
		kPMTIWaitRandom,      5000, 5000,
#else
		kPMTIWaitRandom,      5000, 6000,
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleSet,     kItemPS12Target11,
		kPMTIPlaySound,       kSfxTARGUP5, 33,
		kPMTITargetSet,       kItemPS12Target11, 1,
		kPMTIMove,            5,
		kPMTIWait,            1000,
		kPMTIEnemySet,        kItemPS12Target11,    // rotate - reveal
		kPMTIRotate,          1010, 80,
		kPMTIPlaySound,       kSfxTARGUP1, 33,
		kPMTIWait,            500,
		kPMTIShoot,           kSfxSMCAL3, 33,
		kPMTIPlaySound,       kSfxTARGUP6, 33,
		kPMTIMove,            0,
		kPMTIObstacleReset,   kItemPS12Target11,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS12Target11, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of2, kItemPS12Target13, kItemPS12Target2,
		kPMTIPausedSet,       kItemPS12Target11,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS12TrackData40() {  // Enemy (kItemPS12Target12) - Starts activated - Rotating reveal
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS12TargetCounter, kPoliceMazePS12TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS12TargetCounter, kPoliceMazePS12TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTIEnemyReset,      kItemPS12Target12,
		kPMTITargetSet,       kItemPS12Target12, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS12Target12,
		kPMTIFacing,          480,
		kPMTIPosition,        0,
		kPMTIWaitRandom,      4000, 8000,
		kPMTIObstacleSet,     kItemPS12Target12,
		kPMTIPlaySound,       kSfxTARGUP1, 33,
		kPMTITargetSet,       kItemPS12Target12, 1,
		kPMTIMove,            5,
		kPMTIPlaySound,       kSfxTARGUP1, 33,
		kPMTIWait,            500,
		kPMTIEnemySet,        kItemPS12Target12,    // rotate - reveal
		kPMTIRotate,          968, 80,
		kPMTIPlaySound,       kSfxTARGUP1, 33,
		kPMTIWait,            1000,
		kPMTIShoot,           kSfxSMCAL3, 33,
		kPMTIMove,            0,
		kPMTIObstacleReset,   kItemPS12Target12,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS12Target12, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of3, kItemPS12Target3, kItemPS12Target4, kItemPS12Target5,
		kPMTIPausedSet,       kItemPS12Target12,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS12TrackData41() {  // Innocent (kItemPS12Target13)
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS12TargetCounter, kPoliceMazePS12TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS12TargetCounter, kPoliceMazePS12TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS12Target13, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS12Target13,
		kPMTIFacing,          513,
		kPMTIPosition,        0,
		kPMTIWaitRandom,      4000, 6000,
		kPMTIObstacleSet,     kItemPS12Target13,
		kPMTIPlaySound,       kSfxTARGUP5, 33,
		kPMTITargetSet,       kItemPS12Target13, 1,
		kPMTIEnemyReset,      kItemPS12Target13,
		kPMTIMove,            5,
		kPMTIWait,            500,
		kPMTIRotate,          1010, 80,
		kPMTIPlaySound,       kSfxTARGUP1, 33,
		kPMTIWait,            1000,
		kPMTIMove,            0,
		kPMTIPlaySound,       kSfxTARGUP6, 33,
		kPMTILeave,
		kPMTIObstacleReset,   kItemPS12Target13,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS12Target13, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of2, kItemPS12Target11, kItemPS12Target2,
		kPMTIPausedSet,       kItemPS12Target13,
		kPMTIRestart
	};
	return trackData;
}

// kItemPS12Target14 does not count as an Active target in the original code
// Supposedly he is "Special" in the sense that he acts as a potential bonus point since he appears from a place where an innocent (kItemPS12Target16) can appear
static const int *getPoliceMazePS12TrackData42() {  // Enemy (kItemPS12Target14) - Special (bonus point)
	static int trackData[] = {
#if BLADERUNNER_ORIGINAL_BUGS
#else
//		kPMTIActivate,        kVariablePoliceMazePS12TargetCounter, kPoliceMazePS12TargetCount, // intended: special: Original missing activate instruction
//		kPMTIVariableInc,     kVariablePoliceMazePS12TargetCounter, kPoliceMazePS12TargetCount, // intended: special: Original missing increase counter instruction
		kPMTITargetSet,       kItemPS12Target14, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS12Target14,
		kPMTIFacing,          109,
		kPMTIPosition,        0,
		kPMTIWaitRandom,      2000, 5000,
		kPMTIObstacleSet,     kItemPS12Target14,
		kPMTIPlaySound,       kSfxTARGUP1, 33,
		kPMTITargetSet,       kItemPS12Target14, 1,
		kPMTIEnemySet,        kItemPS12Target14,
		kPMTIMove,            5,
		kPMTIPlaySound,       kSfxTARGUP1, 33,
		kPMTIWait,            1000,
		kPMTIShoot,           kSfxSMCAL3, 33,
		kPMTIMove,            0,
		kPMTIObstacleReset,   kItemPS12Target14,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS12Target14, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of2, kItemPS12Target16, kItemPS12Target1,
		kPMTIPausedSet,       kItemPS12Target14,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS12TrackData43() {  // Innocent (kItemPS12Target15) - Starts activated
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS12TargetCounter, kPoliceMazePS12TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS12TargetCounter, kPoliceMazePS12TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS12Target15, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS12Target15,
		kPMTIFacing,          540,
		kPMTIPosition,        0,
		kPMTIWaitRandom,      5000, 7000,
		kPMTIObstacleSet,     kItemPS12Target15,
		kPMTIPlaySound,       kSfxTARGUP5, 33,
		kPMTITargetSet,       kItemPS12Target15, 1,
		kPMTIEnemyReset,      kItemPS12Target15,
		kPMTIMove,            9,
		kPMTIWait,            2000,
		kPMTIRotate,          284, 80,
		kPMTIWait,            0,
		kPMTIRotate,          28, 80,
		kPMTIPlaySound,       kSfxTARGUP1, 33,
		kPMTIWait,            2000,
		kPMTIPlaySound,       kSfxTARGUP6, 33,
		kPMTIMove,            0,
		kPMTILeave,
		kPMTIObstacleReset,   kItemPS12Target15,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS12Target15, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset,     kItemPS12Target17,
		kPMTIPausedSet,       kItemPS12Target15,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS12TrackData44() {  // Innocent (kItemPS12Target16)
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS12TargetCounter, kPoliceMazePS12TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS12TargetCounter, kPoliceMazePS12TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS12Target16, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS12Target16,
		kPMTIFacing,          109,
		kPMTIPosition,        0,
#if BLADERUNNER_ORIGINAL_BUGS
		kPMTIWaitRandom,      5000, 5000,
#else
		kPMTIWaitRandom,      5000, 6000,
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleSet,     kItemPS12Target16,
		kPMTIPlaySound,       kSfxTARGUP1, 33,
		kPMTITargetSet,       kItemPS12Target16, 1,
		kPMTIEnemyReset,      kItemPS12Target16,
		kPMTIMove,            5,
#if BLADERUNNER_ORIGINAL_BUGS
		kPMTIWaitRandom,      2000, 2000,
#else
		kPMTIWaitRandom,      2000, 3000,
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPlaySound,       kSfxTARGUP1, 33,
		kPMTIMove,            0,
		kPMTILeave,
		kPMTIObstacleReset,   kItemPS12Target16,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS12Target16, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of2, kItemPS12Target14, kItemPS12Target1,
		kPMTIPausedSet,       kItemPS12Target16,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS12TrackData45() {  // Enemy (kItemPS12Target17) - Rotating reveal
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS12TargetCounter, kPoliceMazePS12TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS12TargetCounter, kPoliceMazePS12TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTIEnemyReset,      kItemPS12Target17,
		kPMTITargetSet,       kItemPS12Target17, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS12Target17,
		kPMTIFacing,          540,
		kPMTIPosition,        0,
		kPMTIWaitRandom,      3000, 10000,
		kPMTIObstacleSet,     kItemPS12Target17,
		kPMTIPlaySound,       kSfxTARGUP5, 33,
		kPMTITargetSet,       kItemPS12Target17, 1,
		kPMTIMove,            9,
		kPMTIWait,            1000,
		kPMTIEnemySet,        kItemPS12Target17,    // rotate - reveal
		kPMTIRotate,          284, 80,
		kPMTIWait,            0,
		kPMTIRotate,          28, 80,
		kPMTIWait,            1000,
		kPMTIShoot,           kSfxSMCAL3, 33,
		kPMTIPlaySound,       kSfxTARGUP6, 33,
		kPMTIMove,            0,
		kPMTIObstacleReset,   kItemPS12Target17,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS12Target17, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset,     kItemPS12Target15,
		kPMTIPausedSet,       kItemPS12Target17,
		kPMTIRestart
	};
	return trackData;
}

void SceneScriptPS12::SceneLoaded() {
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
		Item_Add_To_World(kItemPS12Target1,  kModelAnimationFemaleTargetWithGunActive,   kSetPS10_PS11_PS12_PS13,  -691.8f, -9.06f, 587.67f,  200, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS12Target2,  kModelAnimationMaleTargetWithShotgunActive, kSetPS10_PS11_PS12_PS13,  -679.6f, -45.4f, 721.05f,   67, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS12Target3,  kModelAnimationFemaleTargetWithBabyActive,  kSetPS10_PS11_PS12_PS13, -414.04f, -8.98f, 711.91f,  480, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS12Target4,  kModelAnimationMaleTargetWithGunActive,     kSetPS10_PS11_PS12_PS13,  -440.0f, -8.97f, 1137.0f, 1010, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS12Target5,  kModelAnimationMaleTargetEmptyHandsActive,  kSetPS10_PS11_PS12_PS13, -764.92f, -0.84f, 950.22f,  540, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS12Target6,  kModelAnimationFemaleTargetWithGunActive,   kSetPS10_PS11_PS12_PS13,  -696.0f,  -5.7f, 1185.0f,  469, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS12Target7,  kModelAnimationFemaleTargetWithGunActive,   kSetPS10_PS11_PS12_PS13,  -635.0f,  -5.7f, 1165.0f,  198, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS12Target8,  kModelAnimationFemaleTargetWithGunActive,   kSetPS10_PS11_PS12_PS13,  -620.0f, -8.63f, 1366.0f,  469, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS12Target9,  kModelAnimationFemaleTargetWithBabyActive,  kSetPS10_PS11_PS12_PS13,  -584.0f, -79.4f,  775.0f, 1010, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS12Target10, kModelAnimationMaleTargetWithShotgunActive, kSetPS10_PS11_PS12_PS13,  -578.0f, -79.4f,  810.0f,  990, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS12Target11, kModelAnimationMaleTargetWithGunActive,     kSetPS10_PS11_PS12_PS13,  -400.0f, -12.0f, 1110.0f,  513, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS12Target12, kModelAnimationFemaleTargetWithGunActive,   kSetPS10_PS11_PS12_PS13, -414.04f, -8.98f, 711.91f,  480, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS12Target13, kModelAnimationFemaleTargetWithBabyActive,  kSetPS10_PS11_PS12_PS13,  -400.0f, -12.0f, 1110.0f,  513, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS12Target14, kModelAnimationFemaleTargetWithGunActive,   kSetPS10_PS11_PS12_PS13,  -731.0f, 93.66f,  788.0f,  109, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS12Target15, kModelAnimationMaleTargetEmptyHandsActive,  kSetPS10_PS11_PS12_PS13,  -580.0f, -80.0f,  925.0f,  540, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS12Target16, kModelAnimationMaleTargetEmptyHandsActive,  kSetPS10_PS11_PS12_PS13,  -731.0f, 93.66f,  788.0f,  109, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS12Target17, kModelAnimationMaleTargetWithGunActive,     kSetPS10_PS11_PS12_PS13,  -580.0f, -80.0f,  925.0f,  540, 72, 36, targetStateMZ, false, false, true);
	}
	Police_Maze_Target_Track_Add(kItemPS12Target1,   -691.8f, -9.06f, 587.67f, -649.11f, -9.06f, 587.71f,  6, getPoliceMazePS12TrackData29(),  true);
	Police_Maze_Target_Track_Add(kItemPS12Target2,   -679.6f, -45.4f, 721.05f,  -679.6f,  -1.4f, 721.05f,  6, getPoliceMazePS12TrackData30(),  true);
	Police_Maze_Target_Track_Add(kItemPS12Target3,  -414.04f, -8.98f, 711.91f, -459.54f, -8.99f, 707.81f,  6, getPoliceMazePS12TrackData31(), false);
	Police_Maze_Target_Track_Add(kItemPS12Target4,   -440.0f, -8.97f, 1137.0f,  -430.0f, -8.97f,  921.0f,  6, getPoliceMazePS12TrackData32(), false);
	Police_Maze_Target_Track_Add(kItemPS12Target5,  -764.92f, -0.84f, 950.22f, -722.92f, -0.84f, 950.22f,  6, getPoliceMazePS12TrackData33(), false);
	Police_Maze_Target_Track_Add(kItemPS12Target6,   -696.0f,  -5.7f, 1185.0f,  -635.0f,  -5.7f, 1185.0f, 20, getPoliceMazePS12TrackData34(), false);
	Police_Maze_Target_Track_Add(kItemPS12Target7,   -635.0f,  -5.7f, 1165.0f,  -620.0f, -8.63f, 1366.0f, 10, getPoliceMazePS12TrackData35(), false);
	Police_Maze_Target_Track_Add(kItemPS12Target8,   -620.0f, -8.63f, 1366.0f,  -595.0f, -8.63f, 1366.0f, 10, getPoliceMazePS12TrackData36(), false);
	Police_Maze_Target_Track_Add(kItemPS12Target9,   -584.0f, -79.4f,  775.0f,  -584.0f, -27.4f,  775.0f, 10, getPoliceMazePS12TrackData37(),  true);
	Police_Maze_Target_Track_Add(kItemPS12Target10,  -578.0f, -79.4f,  810.0f,  -578.0f, -27.4f,  810.0f, 10, getPoliceMazePS12TrackData38(), false);
	Police_Maze_Target_Track_Add(kItemPS12Target11,  -400.0f, -12.0f, 1110.0f,  -400.0f,  60.0f, 1110.0f,  6, getPoliceMazePS12TrackData39(), false);
	Police_Maze_Target_Track_Add(kItemPS12Target12, -414.04f, -8.98f, 711.91f, -459.54f, -8.99f, 707.81f,  6, getPoliceMazePS12TrackData40(),  true);
	Police_Maze_Target_Track_Add(kItemPS12Target13,  -400.0f, -12.0f, 1110.0f,  -400.0f,  60.0f, 1110.0f,  6, getPoliceMazePS12TrackData41(), false);
	Police_Maze_Target_Track_Add(kItemPS12Target14,  -731.0f, 93.66f,  788.0f,  -702.0f, 93.66f,  788.0f,  6, getPoliceMazePS12TrackData42(), false);
	Police_Maze_Target_Track_Add(kItemPS12Target15,  -580.0f, -80.0f,  925.0f,  -580.0f,  -8.0f,  925.0f, 10, getPoliceMazePS12TrackData43(),  true);
	Police_Maze_Target_Track_Add(kItemPS12Target16,  -731.0f, 93.66f,  788.0f,  -702.0f, 93.66f,  788.0f,  6, getPoliceMazePS12TrackData44(), false);
	Police_Maze_Target_Track_Add(kItemPS12Target17,  -580.0f, -80.0f,  925.0f,  -580.0f,  -8.0f,  925.0f, 10, getPoliceMazePS12TrackData45(), false);

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

bool SceneScriptPS12::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptPS12::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptPS12::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptPS12::ClickedOnItem(int itemId, bool a2) {
	if (Player_Query_Combat_Mode()) {
		switch (itemId) {
		case kItemPS12Target3:              // fall through
		case kItemPS12Target9:              // fall through
		case kItemPS12Target13:             // fall through
			Sound_Play(kSfxFEMHURT2, 50, 0, 0, 50);
			break;
		case kItemPS12Target5:              // fall through
		case kItemPS12Target15:             // fall through
		case kItemPS12Target16:             // fall through
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
		case kItemPS12Target6:              // fall through
		case kItemPS12Target7:              // fall through
		case kItemPS12Target8:
#if BLADERUNNER_ORIGINAL_BUGS
#else
			if (Item_Query_Visible(kItemPS12Target6)) { // without this check, the wrong target might get the spinning while the visible stays put
				Item_Spin_In_World(kItemPS12Target6);
			} else if (Item_Query_Visible(kItemPS12Target7)) {
				Item_Spin_In_World(kItemPS12Target7);
			} else {
				Item_Spin_In_World(kItemPS12Target8);
			}
#endif // BLADERUNNER_ORIGINAL_BUGS
			Item_Flag_As_Non_Target(kItemPS12Target6);
			Item_Flag_As_Non_Target(kItemPS12Target7);
			Item_Flag_As_Non_Target(kItemPS12Target8);
			break;
		case kItemPS12Target1:              // fall through
		case kItemPS12Target2:              // fall through
		case kItemPS12Target3:              // fall through
		case kItemPS12Target4:              // fall through
		case kItemPS12Target5:              // fall through
		case kItemPS12Target9:              // fall through
		case kItemPS12Target10:             // fall through
		case kItemPS12Target11:             // fall through
		case kItemPS12Target12:             // fall through
		case kItemPS12Target13:             // fall through
		case kItemPS12Target14:             // fall through
		case kItemPS12Target15:             // fall through
		case kItemPS12Target16:             // fall through
		case kItemPS12Target17:             // fall through
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

bool SceneScriptPS12::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_Waypoint(kActorMcCoy, 8, 12, true, false)) {
			Game_Flag_Set(kFlagPS12toPS11);
			removeTargets();
			Set_Enter(kSetPS10_PS11_PS12_PS13, kScenePS11);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_Waypoint(kActorMcCoy, 9, 12, true, false)) {
			Player_Loses_Control();
			Loop_Actor_Walk_To_Waypoint(kActorMcCoy, 10, 12, false, false);
			Player_Gains_Control();
			Game_Flag_Set(kFlagPS12toPS13);
			removeTargets();
//			Global_Variable_Decrement(kVariablePoliceMazeScore, kPoliceMazePS12TargetCount - Global_Variable_Query(kVariablePoliceMazePS12TargetCounter));
			Police_Maze_Decrement_Score(kPoliceMazePS12TargetCount - Global_Variable_Query(kVariablePoliceMazePS12TargetCounter));
			Global_Variable_Set(kVariablePoliceMazePS12TargetCounter, kPoliceMazePS12TargetCount);
			Set_Enter(kSetPS10_PS11_PS12_PS13, kScenePS13);
		}
		return true;
	}
	return false;
}

bool SceneScriptPS12::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptPS12::SceneFrameAdvanced(int frame) {
}

void SceneScriptPS12::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptPS12::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagPS11toPS12)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -546.0f, -9.06f, 570.0f, 0, true, false, false);
		Game_Flag_Reset(kFlagPS11toPS12);
	}
	Police_Maze_Set_Pause_State(false);
}

void SceneScriptPS12::PlayerWalkedOut() {
}

void SceneScriptPS12::DialogueQueueFlushed(int a1) {
}

void SceneScriptPS12::removeTargets() {
	Item_Remove_From_World(kItemPS12Target1);
	Item_Remove_From_World(kItemPS12Target2);
	Item_Remove_From_World(kItemPS12Target3);
	Item_Remove_From_World(kItemPS12Target4);
	Item_Remove_From_World(kItemPS12Target5);
	Item_Remove_From_World(kItemPS12Target6);
	Item_Remove_From_World(kItemPS12Target7);
	Item_Remove_From_World(kItemPS12Target8);
	Item_Remove_From_World(kItemPS12Target9);
	Item_Remove_From_World(kItemPS12Target10);
	Item_Remove_From_World(kItemPS12Target11);
	Item_Remove_From_World(kItemPS12Target12);
	Item_Remove_From_World(kItemPS12Target13);
	Item_Remove_From_World(kItemPS12Target14);
	Item_Remove_From_World(kItemPS12Target15);
	Item_Remove_From_World(kItemPS12Target16);
	Item_Remove_From_World(kItemPS12Target17);
}

} // End of namespace BladeRunner
