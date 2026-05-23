/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MADS_DRAGONSPHERE_MADS_SOUNDS_H
#define MADS_DRAGONSPHERE_MADS_SOUNDS_H

#include "common/scummsys.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {

enum {
	// Section 1
	N_DoorOpens          =  24,
	N_DoorCloses         =  25,
	N_TakeObjectSnd      =  26,
	N_PlayerDies         =  27,
	N_InvokeCrystalBall  =  28,
	N_DungeonMus         =  32,
	N_DungeonIntro       =  33,
	N_OminousMus         =  34,
	N_SealMus            =  35,
	N_UnderGroundMus     =  36,
	N_WayStationMus      =  37,
	N_Bk112Mus           =  38,
	N_Bk115Mus           =  39,
	N_Bk116Mus           =  40,
	N_Bk119Mus           =  41,
	N_Angels             =  42,
	N_SwordFightMus      =  44,
	N_QueenMother        =  45,
	N_WalkingMus         =  46,
	N_RoyalDecreeMus     =  47,
	N_TurnDiaryPage      =  65,
	N_WallGrinds         =  67,
	N_RushingWater       =  71,
	N_DragonInWater      =  72,
	N_OldMachinery       =  74,
	N_DogWhimper         =  79,
	N_BeastSnd           =  80,
	N_GrabKing           =  82,
	N_WaterBubbles       =  86,
	N_MagicDoorOpens     =  88,
	N_MagicDoorUnlocked  =  89,
	N_JumpDownWell       =  93,
	N_BooksRumble        =  94,
	N_McMornTipsTable    = 100,

	// Section 2
	N_Bk203Music         =  32,
	N_SlathanMus         =  34,
	N_DreamMusic         =  35,
	N_GuardCollapses     =  65,
	N_TentaclesSplash    =  66,
	N_LavaPlops          =  68,
	N_002HealMeSnd       =  70,
	N_MouthRockTalks     =  71,
	N_PoolMonsterEatsPid =  72,

	// Section 3
	N_MazeMusic          =  32,
	N_ToadRing           =  33,
	N_ToadEatsPlayer     =  70,
	N_RalphIsRed         =  71,
	N_EveryoneScatter    =  72,
	N_CrystalPing        =  73,

	// Section 4
	N_WindWhistles       =  29,
	N_DeathFountain      =  32,
	N_Bk404Music         =  33,
	N_Bk406Music         =  35,
	N_BellyDanceMusic    =  36,
	N_Bk409Music         =  37,
	N_Bk410Music         =  38,
	N_RocAndSoulEgg      =  39,
	N_Bk408Music         =  40,
	N_GamePieceSnd       =  66,
	N_FlameBurst         =  67,
	N_SnakeHiss          =  68,
	N_BigBirdCall        =  69,
	N_JumpThwang         =  70,
	N_StepOnFloatingDisk =  77,
	N_004CryOfDismay     =  78,

	// Section 5
	N_ShakMus            =  32,
	N_Battle             =  33,
	N_Hermit             =  34,
	N_KissMusic          =  35,
	N_TheKiss            =  36,
	N_TenseMusic         =  38,
	N_005Waterfall       =  42,
	N_FarEchos           =  67,
	N_RockClatter        =  68,
	N_CryOfDismay        =  72,
	N_005FlyingInsect    =  78,

	// Section 6
	N_Bk603Music         =  32,
	N_Bk604Music         =  33,
	N_Bk605Music         =  34,
	N_Bk605WithRope      =  35,
	N_Bk606Music         =  36,
	N_Bk609Music         =  37,
	N_Bk612Music         =  38,
	N_WaterFlows         =  39,
	N_Bk614Music         =  40,
	N_006Waterfall       =  64,
	N_TurnPetcock        =  65,
	N_CastleDoorMorphs   =  69,
	N_CatchStone         =  70,
	N_EyeZap             =  73,
	N_MudInTheEye        =  74,
	N_AcidDrips          =  75,
	N_ElevatorButton     =  78,
	N_ElevatorOn         =  79,
	N_ElevatorOff        =  80,
	N_AcidBurnsFloor     =  85,
	N_AppearInHotRoom    =  89,
	N_MeltInHotRoom      =  90,
	N_MachineClank       =  93,
	N_MachineZap         =  94,
	N_MachineMist        =  95,
	N_EerieSounds        =  96,
	N_MeltInGlowingFloor =  97,
	N_CreakyCastleDoor   =  98,

	// Section 9/General
	N_AllFade            =   1,
	N_MusicFade          =   3,
	N_NoiseOff           =   4,
	N_NoiseFade          =   5,
	N_BackgroundMus      =  16,
	N_MusicBoxOn         =  17,
	N_MusicBoxOff        =  18,
	N_BlowBirdCall       =  30,
	N_ListenToFlies      =  31,
	N_WeddingMus         =  62
};

} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS

#endif
