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

static int kPoliceMazePS11TargetCount = 20;
int SceneScriptPS11::getPoliceMazePS11TargetCount() {
	return kPoliceMazePS11TargetCount;
}


void SceneScriptPS11::InitializeScene() {
	if (Game_Flag_Query(kFlagPS10toPS11)) {
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, 0, false);
		Scene_Loop_Set_Default(1);
		Game_Flag_Reset(kFlagPS10toPS11);
		Setup_Scene_Information(World_Waypoint_Query_X(6), World_Waypoint_Query_Y(6), World_Waypoint_Query_Z(6), 840);
	} else {
		Scene_Loop_Set_Default(1);
		Setup_Scene_Information(World_Waypoint_Query_X(7), World_Waypoint_Query_Y(7), World_Waypoint_Query_Z(7), 132);
	}
	Scene_Exit_Add_2D_Exit(0, 0, 460, 639, 479, 2);
	Scene_Exit_Add_2D_Exit(1, 0,   0,  20, 479, 3);
}

static const int *getPoliceMazePS11TrackData9() {   // Enemy (kItemPS11Target1) - Starts activated
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS11TargetCounter, kPoliceMazePS11TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS11TargetCounter, kPoliceMazePS11TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS11Target1, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS11Target1,
		kPMTIFacing,          50,
		kPMTIPosition,        0,
#if BLADERUNNER_ORIGINAL_BUGS
		kPMTIWaitRandom,      5000, 5000,
#else
		kPMTIWaitRandom,      5000, 6000,
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleSet,     kItemPS11Target1,
		kPMTIPlaySound,       kSfxTARGUP3, 33,
		kPMTITargetSet,       kItemPS11Target1, 1,
		kPMTIEnemySet,        kItemPS11Target1,
		kPMTIMove,            7,
		kPMTIWait,            1000,
		kPMTIShoot,           kSfxSMCAL3, 33,
		kPMTIPlaySound,       kSfxTARGUP6, 33,
		kPMTIMove,            0,
		kPMTIWait,            500,
		kPMTIObstacleReset,   kItemPS11Target1,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS11Target1, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of2, kItemPS11Target7, kItemPS11Target2,
		kPMTIPausedSet,       kItemPS11Target1,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS11TrackData10() {  // Enemy (kItemPS11Target2, kItemPS11Target3) - Rotating reveal (as kItemPS11Target3)
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS11TargetCounter, kPoliceMazePS11TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS11TargetCounter, kPoliceMazePS11TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS11Target2, 0,  // remove target-able here
		kPMTITargetSet,       kItemPS11Target3, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS11Target2,
		kPMTIObstacleReset,   kItemPS11Target3,
		kPMTITargetSet,       kItemPS11Target2, 1,
		kPMTITargetSet,       kItemPS11Target3, 1,
		kPMTIFacing,          860,
		kPMTIPosition,        0,
		kPMTIWaitRandom,      3000, 6000,
		kPMTIEnemyReset,      kItemPS11Target2,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTIEnemyReset,      kItemPS11Target3,     // both targets should clear their enemy flag here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleSet,     kItemPS11Target2,
		kPMTIPlaySound,       kSfxTARGUP5, 33,
		kPMTIMove,            14,
		kPMTIWait,            500,
		kPMTIPausedReset,     kItemPS11Target3,
		kPMTIObstacleReset,   kItemPS11Target2,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS11Target2, 0,  // remove target-able here - only for kItemPS11Target2 item
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleSet,     kItemPS11Target3,
		kPMTIPausedSet,       kItemPS11Target2,
		kPMTIPosition,        0,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS11TrackData11() {  // Enemy (kItemPS11Target2, kItemPS11Target3)
	static int trackData[] = {
		kPMTIFacing,          860,
		kPMTIPosition,        0,
		kPMTIEnemyReset,      kItemPS11Target3,		// [redundant after bug fix] target 2-3 still is not revealed as enemy
		kPMTIMove,            25,
		kPMTIWait,            500,
		kPMTIEnemySet,        kItemPS11Target3,		// rotate - reveal -- no need to set target 2 as enemy too, since it's gone
		kPMTIPlaySound,       kSfxTARGUP4, 33,
		kPMTIRotate,          644, 80,
		kPMTIWait,            0,
		kPMTIRotate,          388, 80,
		kPMTIWait,            1000,
		kPMTIShoot,           kSfxLGCAL1, 33,
		kPMTIMove,            79,
		kPMTIObstacleReset,   kItemPS11Target3,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS11Target3, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of2, kItemPS11Target7, kItemPS11Target1,
		kPMTIPausedSet,       kItemPS11Target3,
		kPMTIPosition,        0,
		kPMTIFacing,          860,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS11TrackData12() {  // Innocent (kItemPS11Target4)
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS11TargetCounter, kPoliceMazePS11TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS11TargetCounter, kPoliceMazePS11TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS11Target4, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS11Target4,
		kPMTIFacing,          725,
		kPMTIPosition,        0,
		kPMTIWait,            2000,
		kPMTITargetSet,       kItemPS11Target4, 1,
		kPMTIEnemyReset,      kItemPS11Target4,
		kPMTIObstacleSet,     kItemPS11Target4,
		kPMTIMove,            82,
#if BLADERUNNER_ORIGINAL_BUGS
		kPMTILeave,
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIWait,            1000,
		kPMTIRotate,          570, 80,
		kPMTIWait,            0,
		kPMTIRotate,          462, 80,
		kPMTIWait,            0,
		kPMTIRotate,          213, 80,
		kPMTIWait,            1000,
		kPMTIMove,            0,
		kPMTIRotate,          725, 80,
		kPMTIMove,            99,
		kPMTIObstacleReset,   kItemPS11Target4,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTILeave,                                 // Do the leave instruction here, before becoming un-target-able
		kPMTITargetSet,       kItemPS11Target4, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset,     kItemPS11Target16,
		kPMTIPausedSet,       kItemPS11Target4,
		kPMTIPosition,        0,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS11TrackData13() {  // Enemy (kItemPS11Target5)  - Starts activated
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS11TargetCounter, kPoliceMazePS11TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS11TargetCounter, kPoliceMazePS11TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS11Target5, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS11Target5,
		kPMTIFacing,          340,
		kPMTIPosition,        0,
		kPMTIWaitRandom,      4000, 8000,
		kPMTIObstacleSet,     kItemPS11Target5,
		kPMTIPlaySound,       kSfxTARGUP5, 33,
		kPMTITargetSet,       kItemPS11Target5, 1,
		kPMTIEnemySet,        kItemPS11Target5,
		kPMTIMove,            4,
		kPMTIWait,            0,
		kPMTIRotate,          435, 80,
		kPMTIWait,            0,
		kPMTIRotate,          530, 80,
		kPMTIWait,            100,
		kPMTIRotate,          435, 80,
		kPMTIWait,            0,
		kPMTIRotate,          340, 80,
		kPMTIWait,            0,
		kPMTIRotate,          260, 80,
		kPMTIWait,            0,
		kPMTIRotate,          180, 80,
		kPMTIWait,            100,
		kPMTIRotate,          260, 80,
		kPMTIWait,            0,
		kPMTIRotate,          340, 80,
		kPMTIWait,            200,
		kPMTIShoot,           kSfxSMCAL3, 33,
		kPMTIPlaySound,       kSfxTARGUP6, 33,
		kPMTIMove,            0,
		kPMTIObstacleReset,   kItemPS11Target5,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS11Target5, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of2, kItemPS11Target6, kItemPS11Target10,
		kPMTIPausedSet,       kItemPS11Target5,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS11TrackData14() {  // Enemy (kItemPS11Target6) - Rotating reveal
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS11TargetCounter, kPoliceMazePS11TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS11TargetCounter, kPoliceMazePS11TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS11Target6, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS11Target6,
		kPMTITargetSet,       kItemPS11Target6, 1,
		kPMTIEnemyReset,      kItemPS11Target6,
#if BLADERUNNER_ORIGINAL_BUGS
		kPMTIFacing,          900,                  // orientation is wrong here - should conceal the gun entirely
#else
		kPMTIFacing,          750,                  // corrected orientation
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPosition,        0,
		kPMTIWaitRandom,      3000, 6000,
		kPMTIObstacleSet,     kItemPS11Target6,
		kPMTIPlaySound,       kSfxTARGUP5, 33,
		kPMTIMove,            5,
		kPMTIWait,            500,
		kPMTIEnemySet,        kItemPS11Target6,     // rotate - reveal
		kPMTIRotate,          644, 80,
		kPMTIWait,            0,
#if BLADERUNNER_ORIGINAL_BUGS
		kPMTIRotate,          388, 80,              // orientation is wrong here
#else
		kPMTIRotate,          260, 80,              // corrected orientation - face towards McCoy
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIWait,            1000,
		kPMTIShoot,           kSfxSMCAL3, 33,
		kPMTIPlaySound,       kSfxTARGUP6, 33,
		kPMTIMove,            0,
		kPMTIObstacleReset,   kItemPS11Target6,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS11Target6, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of2, kItemPS11Target10, kItemPS11Target5,
		kPMTIPausedSet,       kItemPS11Target6,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS11TrackData15() {  // Special (kItemPS11Target7, kItemPS11Target8) - Innocent x2
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS11TargetCounter, kPoliceMazePS11TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS11TargetCounter, kPoliceMazePS11TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS11Target7, 0,  // remove target-able here - redundant, but consistent with the original kPMTIObstacleReset (redundant) instructions that follow
		kPMTITargetSet,       kItemPS11Target8, 0,  // remove target-able here - redundant, but consistent with the original kPMTIObstacleReset (redundant) instructions that follow
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS11Target7,
		kPMTIObstacleReset,   kItemPS11Target8,
		kPMTITargetSet,       kItemPS11Target7, 1,
		kPMTITargetSet,       kItemPS11Target8, 1,
		kPMTIFacing,          860,
		kPMTIPosition,        0,
		kPMTIWaitRandom,      3000, 7000,
		kPMTIObstacleSet,     kItemPS11Target7,
		kPMTIPlaySound,       kSfxTARGUP1, 33,
		kPMTIEnemyReset,      kItemPS11Target7,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTIEnemyReset,      kItemPS11Target8,     // both targets should clear their enemy flag here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIMove,            14,
		kPMTILeave,                                 // intended: special: credit for "first" innocent escaping
		kPMTIWait,            1000,
		kPMTIPausedReset,     kItemPS11Target8,
		kPMTIObstacleReset,   kItemPS11Target7,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS11Target7, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleSet,     kItemPS11Target8,
		kPMTIPausedSet,       kItemPS11Target7,
		kPMTIPosition,        0,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS11TrackData16() {  // Innocent (kItemPS11Target7, kItemPS11Target8)
	static int trackData[] = {
		kPMTIFacing,          860,
		kPMTIPosition,        0,
		kPMTIObstacleSet,     kItemPS11Target8,
		kPMTIEnemyReset,      kItemPS11Target8,     // [redundant after bug fix]
		kPMTIMove,            25,
		kPMTIWait,            500,
		kPMTIPlaySound,       kSfxTARGUP4, 33,
		kPMTIRotate,          644, 100,
		kPMTIWait,            0,
		kPMTIRotate,          388, 200,
		kPMTIWait,            500,
		kPMTIMove,            79,
		kPMTILeave,                                 // credit for "second" / final innocent
		kPMTIObstacleReset,   kItemPS11Target8,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS11Target8, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of2, kItemPS11Target2, kItemPS11Target1,
		kPMTIPausedSet,       kItemPS11Target8,
		kPMTIPosition,        0,
		kPMTIFacing,          860,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS11TrackData17() {  // Special (kItemPS11Target9) - Enemy x3
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS11TargetCounter, kPoliceMazePS11TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS11TargetCounter, kPoliceMazePS11TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS11Target9, 0,  // remove target-able here
		kPMTIObstacleReset,   kItemPS11Target9,
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIFacing,          310,
		kPMTIPosition,        0,
#if BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleSet,     kItemPS11Target9,
		kPMTITargetSet,       kItemPS11Target9, 1,
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIEnemySet,        kItemPS11Target9,
		kPMTIWaitRandom,      4000, 8000,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTIObstacleSet,     kItemPS11Target9,     // make visible after the wait period, not before
		kPMTITargetSet,       kItemPS11Target9, 1,  // set Target-able after the wait period, not before
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPlaySound,       kSfxTARGUP4, 33,
		kPMTIMove,            10,
		kPMTIWait,            0,                    // this results in shooting too fast - TODO maybe introduce a small wait here (50 or 150)
		kPMTIShoot,           kSfxSMCAL3, 33,
		kPMTIMove,            0,
		kPMTITargetSet,       kItemPS11Target9, 1,  // intended: special: "second" enemy (re-using the target of the track)
		kPMTIEnemySet,        kItemPS11Target9,     // intended: special: "second" enemy (re-using the target of the track)
		kPMTIMove,            24,
		kPMTITargetSet,       kItemPS11Target9, 1,  // intended: special: "third" enemy (re-using the target of the track)
		kPMTIEnemySet,        kItemPS11Target9,     // intended: special: "third" enemy (re-using the target of the track)
		kPMTIMove,            10,
		kPMTIWait,            0,                    // this results in shooting too fast - TODO maybe introduce a small wait here (50 or 150)
		kPMTIShoot,           kSfxSMCAL3, 33,
		kPMTIMove,            24,
		kPMTIWait,            1000,
		kPMTIObstacleReset,   kItemPS11Target9,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS11Target9, 0,  // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of2, kItemPS11Target15, kItemPS11Target14,
		kPMTIPausedSet,       kItemPS11Target9,
		kPMTIPosition,        0,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS11TrackData18() {  // Special (kItemPS11Target10, kItemPS11Target11) - Rotating reveal and Enemy x2
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS11TargetCounter, kPoliceMazePS11TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS11TargetCounter, kPoliceMazePS11TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS11Target10, 0, // remove target-able here
		kPMTITargetSet,       kItemPS11Target11, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS11Target10,
		kPMTIObstacleReset,   kItemPS11Target11,
		kPMTIFacing,          900,
		kPMTIPosition,        0,
		kPMTITargetSet,       kItemPS11Target10, 1,
		kPMTITargetSet,       kItemPS11Target11, 1,
		kPMTIEnemyReset,      kItemPS11Target10,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTIEnemyReset,      kItemPS11Target11,    // both targets should clear their enemy flag here
#endif
		kPMTIWaitRandom,      4000, 6000,
		kPMTIObstacleSet,     kItemPS11Target10,
		kPMTIMove,            5,
		kPMTIPlaySound,       kSfxRICO3, 33,
#if BLADERUNNER_ORIGINAL_BUGS
		kPMTIPlaySound,       kSfxFEMHURT1, 33,     // FEMHURT1 - Wrong sound
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIWait,            1000,
#if BLADERUNNER_ORIGINAL_BUGS
		kPMTILeave,                                 // bug: Leave does not fit here since the target does not actually "escape" (nor is innocent)
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIRotate,          700, 80,
		kPMTIEnemySet,        kItemPS11Target10,    // Now the target is an enemy. (special) Rotate reveal
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTIEnemySet,        kItemPS11Target11,    // both targets should set their enemy flag here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIWait,            0,
		kPMTIRotate,          512, 200,
		kPMTIWait,            1000,
		kPMTIShoot,           kSfxLGCAL1, 33,
		kPMTIPausedReset,     kItemPS11Target11,
		kPMTIObstacleReset,   kItemPS11Target10,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS11Target10, 0, // remove target-able here only for kItemPS11Target10
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleSet,     kItemPS11Target11,
		kPMTIPausedSet,       kItemPS11Target10,
		kPMTIPosition,        0,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS11TrackData19() {  // Special (kItemPS11Target10, kItemPS11Target11) - becomes an Enemy again
	static int trackData[] = {
		kPMTIFacing,          512,
		kPMTIPosition,        0,
		kPMTIEnemySet,        kItemPS11Target11,    // [redundant after bug fix]
		kPMTIMove,            8,
		kPMTIWait,            4000,
		kPMTITargetSet,       kItemPS11Target11, 1, // intended: special: "second" enemy (re-using the target of the track)
		kPMTIEnemySet,        kItemPS11Target11,    // intended: special: "second" enemy (re-using the target of the track)
		kPMTIMove,            2,
		kPMTIPlaySound,       kSfxTARGUP4, 33,
		kPMTIWait,            1000,
		kPMTIShoot,           kSfxLGCAL1, 33,
		kPMTIMove,            kItemPS11Target11,
		kPMTIWait,            500,
		kPMTIObstacleReset,   kItemPS11Target11,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS11Target11, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of2, kItemPS11Target5, kItemPS11Target6,
		kPMTIPausedSet,       kItemPS11Target11,
		kPMTIPosition,        0,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS11TrackData20() {  // Enemy (kItemPS11Target12) - Starts activated
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS11TargetCounter, kPoliceMazePS11TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS11TargetCounter, kPoliceMazePS11TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS11Target12, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS11Target12,
		kPMTIFacing,          280,
		kPMTIPosition,        0,
		kPMTIWaitRandom,      5000, 7000,
		kPMTITargetSet,       kItemPS11Target12, 1,
		kPMTIEnemySet,        kItemPS11Target12,
		kPMTIObstacleSet,     kItemPS11Target12,
		kPMTIMove,            9,
		kPMTIPlaySound,       kSfxTARGUP4, 33,
		kPMTIWait,            1000,
		kPMTIShoot,           kSfxSMCAL3, 33,
		kPMTIMove,            0,
		kPMTIObstacleReset,   kItemPS11Target12,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS11Target12, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of2, kItemPS11Target13, kItemPS11Target4,
		kPMTIPausedSet,       kItemPS11Target12,
		kPMTIPosition,        0,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS11TrackData21() {  // Innocent (kItemPS11Target13)
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS11TargetCounter, kPoliceMazePS11TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS11TargetCounter, kPoliceMazePS11TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS11Target13, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS11Target13,
		kPMTIFacing,          280,
		kPMTIPosition,        0,
		kPMTIWaitRandom,      5000, 8000,
		kPMTITargetSet,       kItemPS11Target13, 1,
		kPMTIEnemyReset,      kItemPS11Target13,
		kPMTIObstacleSet,     kItemPS11Target13,
		kPMTIMove,            5,
#if BLADERUNNER_ORIGINAL_BUGS
		kPMTILeave,
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIWait,            1000,
		kPMTIMove,            0,
		kPMTIObstacleReset,   kItemPS11Target13,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTILeave,                                 // Do the leave instruction here instead
		kPMTITargetSet,       kItemPS11Target13, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of2, kItemPS11Target12, kItemPS11Target4,
		kPMTIPausedSet,       kItemPS11Target13,
		kPMTIPosition,        0,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS11TrackData22() {  // Enemy (kItemPS11Target14)
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS11TargetCounter, kPoliceMazePS11TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS11TargetCounter, kPoliceMazePS11TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS11Target14, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS11Target14,
		kPMTIFacing,          255,
		kPMTIPosition,        0,
		kPMTITargetSet,       kItemPS11Target14, 1,
		kPMTIEnemySet,        kItemPS11Target14,
#if BLADERUNNER_ORIGINAL_BUGS
		kPMTIWaitRandom,      5000, 5000,
#else
		kPMTIWaitRandom,      5000, 6000,
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleSet,     kItemPS11Target14,
		kPMTIMove,            7,
		kPMTIPlaySound,       kSfxTARGUP4, 33,
		kPMTIWait,            1000,
		kPMTIShoot,           kSfxLGCAL1, 33,
		kPMTIMove,            0,
		kPMTIObstacleReset,   kItemPS11Target14,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS11Target14, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of2, kItemPS11Target15, kItemPS11Target9,
		kPMTIPausedSet,       kItemPS11Target14,
		kPMTIPosition,        0,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS11TrackData23() {  // Innocent (kItemPS11Target15) - Starts activated
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS11TargetCounter, kPoliceMazePS11TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS11TargetCounter, kPoliceMazePS11TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS11Target15, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS11Target15,
		kPMTIFacing,          310,
		kPMTIPosition,        0,
		kPMTIWaitRandom,      3000, 6000,
		kPMTITargetSet,       kItemPS11Target15, 1,
		kPMTIEnemyReset,      kItemPS11Target15,
		kPMTIObstacleSet,     kItemPS11Target15,
		kPMTIWait,            1000,
		kPMTIMove,            24,
		kPMTIWait,            1000,
		kPMTIMove,            0,
		kPMTILeave,
		kPMTIObstacleReset,   kItemPS11Target15,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS11Target15, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of2, kItemPS11Target14, kItemPS11Target9,
		kPMTIPausedSet,       kItemPS11Target15,
		kPMTIPosition,        0,
		kPMTIRestart
	};
	return trackData;
}

static const int *getPoliceMazePS11TrackData27() {  // Enemy (kItemPS11Target16)
	static int trackData[] = {
		kPMTIActivate,        kVariablePoliceMazePS11TargetCounter, kPoliceMazePS11TargetCount,
		kPMTIVariableInc,     kVariablePoliceMazePS11TargetCounter, kPoliceMazePS11TargetCount,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS11Target16, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIObstacleReset,   kItemPS11Target16,
		kPMTIFacing,          346,
		kPMTIPosition,        0,
		kPMTIWait,            0,
		kPMTITargetSet,       kItemPS11Target16, 1,
		kPMTIEnemySet,        kItemPS11Target16,
		kPMTIObstacleSet,     kItemPS11Target16,
		kPMTIMove,            14,
		kPMTIWait,            1000,
		kPMTIShoot,           kSfxLGCAL1, 33,
		kPMTIMove,            0,
		kPMTIObstacleReset,   kItemPS11Target16,
#if BLADERUNNER_ORIGINAL_BUGS
#else
		kPMTITargetSet,       kItemPS11Target16, 0, // remove target-able here
#endif // BLADERUNNER_ORIGINAL_BUGS
		kPMTIPausedReset1of2, kItemPS11Target13, kItemPS11Target12,
		kPMTIPausedSet,       kItemPS11Target16,
		kPMTIPosition,        0,
		kPMTIRestart
	};
	return trackData;
}

void SceneScriptPS11::SceneLoaded() {
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
		Item_Add_To_World(kItemPS11Target1,  kModelAnimationFemaleTargetWithGunActive,   kSetPS10_PS11_PS12_PS13,  -450.0f,  -7.5f, 335.0f,  50, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS11Target2,  kModelAnimationFemaleTargetWithGunActive,   kSetPS10_PS11_PS12_PS13,  -740.0f,  27.0f, -30.0f, 860, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS11Target3,  kModelAnimationFemaleTargetWithGunActive,   kSetPS10_PS11_PS12_PS13,  -740.0f,  99.0f, -30.0f, 860, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS11Target4,  kModelAnimationMaleTargetEmptyHandsActive,  kSetPS10_PS11_PS12_PS13,  -400.0f, -9.23f, -75.0f, 725, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS11Target5,  kModelAnimationMaleTargetWithGunActive,     kSetPS10_PS11_PS12_PS13, -803.72f, -72.7f, 60.22f, 340, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS11Target6,  kModelAnimationMaleTargetWithGunActive,     kSetPS10_PS11_PS12_PS13,  -853.0f, -70.0f, 195.0f, 900, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS11Target7,  kModelAnimationFemaleTargetWithBabyActive,  kSetPS10_PS11_PS12_PS13,  -740.0f,  27.0f, -30.0f, 860, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS11Target8,  kModelAnimationFemaleTargetWithBabyActive,  kSetPS10_PS11_PS12_PS13,  -740.0f,  99.0f, -30.0f, 860, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS11Target9,  kModelAnimationMaleTargetWithShotgunActive, kSetPS10_PS11_PS12_PS13,  -888.0f, 155.0f, 100.0f, 310, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS11Target10, kModelAnimationMaleTargetWithGunActive,     kSetPS10_PS11_PS12_PS13,  -430.0f, 164.0f,  11.0f, 900, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS11Target11, kModelAnimationMaleTargetWithGunActive,     kSetPS10_PS11_PS12_PS13,  -430.0f, -0.86f,  11.0f, 512, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS11Target12, kModelAnimationMaleTargetWithGunActive,     kSetPS10_PS11_PS12_PS13,  -891.0f,   3.1f,  90.0f, 280, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS11Target13, kModelAnimationFemaleTargetWithBabyActive,  kSetPS10_PS11_PS12_PS13,  -891.0f,   3.1f,  90.0f, 280, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS11Target14, kModelAnimationMaleTargetWithShotgunActive, kSetPS10_PS11_PS12_PS13,  -891.0f, 171.0f, 190.0f, 255, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS11Target15, kModelAnimationMaleTargetEmptyHandsActive,  kSetPS10_PS11_PS12_PS13,  -888.0f, 155.0f,  30.0f, 310, 72, 36, targetStateMZ, false, false, true);
		Item_Add_To_World(kItemPS11Target16, kModelAnimationMaleTargetWithShotgunActive, kSetPS10_PS11_PS12_PS13,  -800.0f, -9.23f, -75.0f, 346, 72, 36, targetStateMZ, false, false, true);
	}

	Police_Maze_Target_Track_Add(kItemPS11Target1,   -450.0f,  -7.5f, 335.0f,  -450.0f,  -7.5f, 295.0f,   8, getPoliceMazePS11TrackData9(),   true);
	Police_Maze_Target_Track_Add(kItemPS11Target2,   -740.0f,  27.0f, -30.0f,  -740.0f,  99.0f, -30.0f,  15, getPoliceMazePS11TrackData10(), false);
	Police_Maze_Target_Track_Add(kItemPS11Target3,   -740.0f,  99.0f, -30.0f,  -200.0f,  99.0f, -30.0f,  80, getPoliceMazePS11TrackData11(), false);
	Police_Maze_Target_Track_Add(kItemPS11Target4,   -400.0f, -9.23f, -75.0f,  -800.0f, -9.23f, -75.0f, 100, getPoliceMazePS11TrackData12(), false);
	Police_Maze_Target_Track_Add(kItemPS11Target5,  -803.72f, -72.7f, 60.22f, -803.72f,  -0.7f, 60.22f,   6, getPoliceMazePS11TrackData13(),  true);
	Police_Maze_Target_Track_Add(kItemPS11Target6,   -853.0f, -70.0f, 195.0f,  -853.0f,   2.0f, 195.0f,   6, getPoliceMazePS11TrackData14(), false);
	Police_Maze_Target_Track_Add(kItemPS11Target7,   -740.0f,  27.0f, -30.0f,  -740.0f,  99.0f, -30.0f,  15, getPoliceMazePS11TrackData15(), false);
	Police_Maze_Target_Track_Add(kItemPS11Target8,   -740.0f,  99.0f, -30.0f,  -200.0f,  99.0f, -30.0f,  80, getPoliceMazePS11TrackData16(), false);
	Police_Maze_Target_Track_Add(kItemPS11Target9,   -888.0f, 155.0f, 100.0f,  -888.0f, 155.0f,  30.0f,  25, getPoliceMazePS11TrackData17(), false);
	Police_Maze_Target_Track_Add(kItemPS11Target10,  -430.0f, 164.0f,  11.0f,  -430.0f, -0.86f,  11.0f,   6, getPoliceMazePS11TrackData18(), false);
	Police_Maze_Target_Track_Add(kItemPS11Target11,  -430.0f, -0.86f,  11.0f,  -300.0f, -0.86f, -80.0f,  20, getPoliceMazePS11TrackData19(), false);
	Police_Maze_Target_Track_Add(kItemPS11Target12,  -891.0f,   3.1f,  90.0f,  -891.0f,   3.1f, 105.0f,  10, getPoliceMazePS11TrackData20(),  true);
	Police_Maze_Target_Track_Add(kItemPS11Target13,  -891.0f,   3.1f,  90.0f,  -891.0f,   3.1f, 105.0f,   6, getPoliceMazePS11TrackData21(), false);
	Police_Maze_Target_Track_Add(kItemPS11Target14,  -891.0f, 171.0f, 190.0f,  -891.0f, 171.0f, 147.0f,   8, getPoliceMazePS11TrackData22(), false);
	Police_Maze_Target_Track_Add(kItemPS11Target15,  -888.0f, 155.0f,  30.0f,  -888.0f, 155.0f, 100.0f,  25, getPoliceMazePS11TrackData23(),  true);
	Police_Maze_Target_Track_Add(kItemPS11Target16,  -800.0f, -9.23f, -75.0f,  -740.0f, -9.23f, -75.0f,  15, getPoliceMazePS11TrackData27(), false);

	Ambient_Sounds_Remove_All_Non_Looping_Sounds(false);
	Ambient_Sounds_Add_Looping_Sound(kSfxESPLOOP2, 50, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(kSfxCTRAIN1,  50, 1, 1);
	Ambient_Sounds_Add_Sound(kSfxSTEAM1, 10,  50, 16, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSTEAMY1, 5,  50, 16, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSTEAMY2, 6,  50, 16, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY4,  2, 100, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY5,  2, 100, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY6,  2, 100, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxSCARY7,  2, 100, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN1,  5, 100, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN2,  5, 100, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBGRN3,  5, 100, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE1, 5, 100, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE2, 5, 100, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxBBMOVE3, 5, 100, 17, 27, -100, 100, -101, -101, 0, 0);
}

bool SceneScriptPS11::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptPS11::ClickedOn3DObject(const char *objectName, bool combatMode) {
	return false;
}

bool SceneScriptPS11::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptPS11::ClickedOnItem(int itemId, bool combatMode) {
	if (Player_Query_Combat_Mode()) {
		switch (itemId) {
		case kItemPS11Target4:              // fall through
		case kItemPS11Target15:
			Sound_Play(kSfxMALEHURT, 50, 0, 0, 50);
			break;
		case kItemPS11Target7:              // fall through
		case kItemPS11Target8:              // fall through
		case kItemPS11Target13:
			Sound_Play(kSfxFEMHURT2, 50, 0, 0, 50);
			break;
		default:
			Sound_Play(kSfxSPINNY1,  12, 0, 0, 50);
			break;
		}

#if BLADERUNNER_ORIGINAL_BUGS
		Item_Spin_In_World(itemId);
#endif // BLADERUNNER_ORIGINAL_BUGS
		switch (itemId) {
		case kItemPS11Target2:  // fall through
		case kItemPS11Target3:
#if BLADERUNNER_ORIGINAL_BUGS
#else
			if (Item_Query_Visible(kItemPS11Target2)) { // without this check, the wrong target might get the spinning while the visible stays put
				Item_Spin_In_World(kItemPS11Target2);
			} else {
				Item_Spin_In_World(kItemPS11Target3);
			}
#endif // BLADERUNNER_ORIGINAL_BUGS
			Item_Flag_As_Non_Target(kItemPS11Target2);
			Item_Flag_As_Non_Target(kItemPS11Target3);
			break;
		case kItemPS11Target7:  // fall through
		case kItemPS11Target8:
#if BLADERUNNER_ORIGINAL_BUGS
#else
			if (Item_Query_Visible(kItemPS11Target7)) { // without this check, the wrong target might get the spinning while the visible stays put
				Item_Spin_In_World(kItemPS11Target7);
			} else {
				Item_Spin_In_World(kItemPS11Target8);
			}
#endif // BLADERUNNER_ORIGINAL_BUGS
			Item_Flag_As_Non_Target(kItemPS11Target7);
			Item_Flag_As_Non_Target(kItemPS11Target8);
			break;
		case kItemPS11Target10: // fall through
		case kItemPS11Target11:
#if BLADERUNNER_ORIGINAL_BUGS
#else
			if (Item_Query_Visible(kItemPS11Target10)) { // without this check, the wrong target might get the spinning while the visible stays put
				Item_Spin_In_World(kItemPS11Target10);
			} else {
				Item_Spin_In_World(kItemPS11Target11);
			}
#endif // BLADERUNNER_ORIGINAL_BUGS
			Item_Flag_As_Non_Target(kItemPS11Target10);
			Item_Flag_As_Non_Target(kItemPS11Target11);
			break;
		case kItemPS11Target1:  // fall through
		case kItemPS11Target4:  // fall through
		case kItemPS11Target5:  // fall through
		case kItemPS11Target6:  // fall through
		case kItemPS11Target9:  // fall through
		case kItemPS11Target12: // fall through
		case kItemPS11Target13: // fall through
		case kItemPS11Target14: // fall through
		case kItemPS11Target15: // fall through
		case kItemPS11Target16: // fall through
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

bool SceneScriptPS11::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_Waypoint(kActorMcCoy, 6, 12, true, false)) {
			Game_Flag_Set(kFlagPS11toPS10);
			removeTargets();
			Set_Enter(kSetPS10_PS11_PS12_PS13, kScenePS10);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_Waypoint(kActorMcCoy, 8, 12, true, false)) {
			Game_Flag_Set(kFlagPS11toPS12);
			removeTargets();
//			Global_Variable_Decrement(kVariablePoliceMazeScore, kPoliceMazePS11TargetCount - Global_Variable_Query(kVariablePoliceMazePS11TargetCounter));
			Police_Maze_Decrement_Score(kPoliceMazePS11TargetCount - Global_Variable_Query(kVariablePoliceMazePS11TargetCounter));
			Global_Variable_Set(kVariablePoliceMazePS11TargetCounter, kPoliceMazePS11TargetCount);
			Set_Enter(kSetPS10_PS11_PS12_PS13, kScenePS12);
		}
		return true;
	}
	return false;
}

bool SceneScriptPS11::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptPS11::SceneFrameAdvanced(int frame) {
}

void SceneScriptPS11::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptPS11::PlayerWalkedIn() {
	Police_Maze_Set_Pause_State(false);
}

void SceneScriptPS11::PlayerWalkedOut() {
}

void SceneScriptPS11::DialogueQueueFlushed(int a1) {
}

void SceneScriptPS11::removeTargets() {
	Item_Remove_From_World(kItemPS11Target1);
	Item_Remove_From_World(kItemPS11Target2);
	Item_Remove_From_World(kItemPS11Target3);
	Item_Remove_From_World(kItemPS11Target4);
	Item_Remove_From_World(kItemPS11Target5);
	Item_Remove_From_World(kItemPS11Target6);
	Item_Remove_From_World(kItemPS11Target7);
	Item_Remove_From_World(kItemPS11Target8);
	Item_Remove_From_World(kItemPS11Target9);
	Item_Remove_From_World(kItemPS11Target10);
	Item_Remove_From_World(kItemPS11Target11);
	Item_Remove_From_World(kItemPS11Target12);
	Item_Remove_From_World(kItemPS11Target13);
	Item_Remove_From_World(kItemPS11Target14);
	Item_Remove_From_World(kItemPS11Target15);
	Item_Remove_From_World(kItemPS11Target16);
}

} // End of namespace BladeRunner
