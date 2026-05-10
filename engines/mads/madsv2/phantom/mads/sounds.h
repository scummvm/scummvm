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

#ifndef MADS_PHANTOM_MADS_SOUNDS_H
#define MADS_PHANTOM_MADS_SOUNDS_H

#include "common/scummsys.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {

enum {
	/* Shared */
	N_AllFade = 1,
	N_MusicOff = 2,
	N_MusicFade = 3,
	N_NoiseFade = 5,
	N_IsAnySoundOn = 8,
	N_BackgroundMus = 16,
	N_DoorOpens = 24,
	N_DoorCloses = 25,
	N_TakeObjectSnd = 26,
	N_PlayerDies = 27,

	/* Section 1 */
	N_JacquesDeadMus = 32,
	N_1881Music = 33,
	N_AngelMus001 = 34,
	N_Christine1881_001 = 35,
	N_Christine1993 = 36,
	N_PhantomAppears001 = 37,
	N_BackMus1stTime = 38,
	N_ChaseMusic001 = 39,
	N_TrapDoor001 = 64,
	N_SqueakyDoor = 66,
	N_PlayerFalls = 67,
	N_EchoSteps = 68,
	N_SandbagThud = 70,
	N_DoorHandle = 73,

	/* Section 2 */
	N_IsabelWedding = 34,
	N_WomanScream002 = 65,
	N_Applause002 = 69,
	N_KeyTurnSnd = 71,
	N_DoorHandle002 = 72,

	/* Section 3 */
	N_FightMusic304 = 33,
	N_LakeMusic = 36,
	N_Crash003 = 66,
	N_WomanScream003 = 74,

	/* Section 4 */
	N_PoisonGas = 61,
	N_LeverSnap = 65,
	N_LeverBing = 66,
	N_DoorGong = 67,
	N_DoorHandle004 = 70,
	N_DoorHandle501 = 74,

	/* Section 5 */
	N_FightMusic504 = 33,
	N_Bach_dm = 34,
	N_Gigue = 35,
	N_Canon = 36,
	N_Bach_gm = 37,
	N_AngelMus505 = 39,
	N_PanelClank = 65,
	N_HeatHiss = 67,
	N_LassoThrow = 69,
	N_SecretDoor = 73,
	N_DoorHandle005 = 74,
	N_CoffinUnlocks = 76,
	N_PushSkull = 77
};

} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
