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
	N_SealMus            =  35,
	N_UnderGroundMus     =  36,
	N_WayStationMus      =  37,
	N_Angels             =  42,
	N_QueenMother        =  45,
	N_TurnDiaryPage      =  65,
	N_WallGrinds         =  67,
	N_RushingWater       =  71,
	N_DragonInWater      =  72,
	N_OldMachinery       =  74,
	N_DogWhimper         =  79,
	N_BeastSnd           =  80,
	N_GrabKing           =  82,
	N_MagicDoorOpens     =  88,
	N_MagicDoorUnlocked  =  89,
	N_JumpDownWell       =  93,
	N_BooksRumble        =  94,
	N_McMornTipsTable    = 100,

	// Section 2
	N_BackgroundMus      =  16,
	N_GuardCollapses     =  65,
	N_TentaclesSplash    =  66,
	N_LavaPlops          =  68,
	N_002HealMeSnd       =  70,
	N_MouthRockTalks     =  71,
	N_PoolMonsterEatsPid =  72,

	// Section 3
	N_ToadEatsPlayer     =  70,
	N_RalphIsRed         =  71,
	N_EveryoneScatter    =  72,
	N_CrystalPing        =  73,

	// Section 4
	N_WindWhistles       =  29,
	N_Bk404Music         =  33,
	N_Bk406Music         =  35,
	N_BellyDanceMusic    =  36
};

} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS

#endif
