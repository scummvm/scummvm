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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_SHARED_H
#define ASYLUM_SHARED_H

namespace Asylum {

//////////////////////////////////////////////////////////////////////////
// Global
//////////////////////////////////////////////////////////////////////////
enum GameFlag {
	kGameFlag4 = 4,
	kGameFlag12 = 12,
	kGameFlagScriptProcessing     = 183,
	kGameFlagCommentLeavingCell   = 214,
	kGameFlag219                  = 219,
	kGameFlagSolveVCRBlowUpPuzzle = 220,
	kGameFlag238                  = 238,
	kGameFlag239                  = 239,
	kGameFlag279                  = 279,
	kGameFlag368                  = 368,
	kGameFlag556                  = 556,
	kGameFlagFinishGame           = 901,
	kGameFlag1131                 = 1131
};

//////////////////////////////////////////////////////////////////////////
// Resources
//////////////////////////////////////////////////////////////////////////

typedef int ResourceId;

enum ResourceIdEnum {
	kResourceNone            = 0,
	kResourceSpeech_8000050A = 0x8000050A,
	kResourceMusic_80020000  = 0x80020000,
	kResourceMusic_80020001  = 0x80020001,
	kResourceSound_80030203  = 0x80030203,
	kResourceSound_80120001  = 0x80120001,
	kResourceSound_80120006  = 0x80120006,
	kResourceSoundIntro      = 0x80120007,
	kResourceMusic_FFFFFD66  = 0xFFFFFD66
};

//////////////////////////////////////////////////////////////////////////
// Actions
//////////////////////////////////////////////////////////////////////////

enum ActionType {
	kActionTypeNone = 0,
	kActionTypeFind = 1,
	kActionTypeTalk = 2,
	kActionTypeGrab = 4,
	kActionType8 = 8,
	kActionType16 = 16
};

//////////////////////////////////////////////////////////////////////////
// Actor
//////////////////////////////////////////////////////////////////////////

typedef int ActorIndex;

enum ActorIndexes {
	kActorPlayer = -1,
	// TODO: check if the names match the actor type
	kActorMax     = 0,
	kActorSarah   = 1,
	kActorCyclops = 2,
	kActorAztec   = 3
};

enum ActorStatus {
	kActorStatus1 = 1,
	kActorStatus2,
	kActorStatus3,
	kActorStatusEnabled,
	kActorStatusDisabled,
	kActorStatus6,
	kActorStatus7,
	kActorStatus8,
	kActorStatus9,
	kActorStatus10,
	kActorStatus11,
	kActorStatus12,
	kActorStatus13,
	kActorStatus14,
	kActorStatus15,
	kActorStatus16,
	kActorStatus17,
	kActorStatus18,
	kActorStatus19,
	kActorStatus20,
	kActorStatus21
};

typedef int ActorDirection;

enum ActorFlags {
	kActorFlagVisible = 1
};

enum DirectionFrom {
	kDirectionFromBarrier = 0,
	kDirectionFromPolygons = 1,
	kDirectionFromActor = 2,
	kDirectionFromParameters = 3
};

// TODO investigate other actor resources (from other
// scenes) to see if the unused blocks in the actor
// definition are in fact used elsewhere
enum ActorResources {
	kSound1 = 0,
	kSound2 = 1,
	kSound3 = 2,
	kSound4 = 3,
	//kUnused = 4,	// 0
	kFlags  = 5,

	kWalkN  = 6,
	kWalkNW = 7,
	kWalkW  = 8,
	kWalkSW = 9,
	kWalkS  = 10,

	kFaceN  = 11,
	kFaceNW = 12,
	kFaceW  = 13,
	kFaceSW = 14,
	kFaceS  = 15,

	kFaceAndShakeN  = 16,
	kFaceAndShakeNW = 17,
	kFaceAndShakeW  = 18,
	kFaceAndShakeSW = 19,
	kFaceAndShakeS  = 20,

	kStretchN  = 21,
	kStretchNW = 22,
	kStretchW  = 23,
	kStretchSW = 24,
	kStretchS  = 25,

	kFidgetN  = 26,
	kFidgetNW = 27,
	kFidgetW  = 28,
	kFidgetSW = 29,
	kFidgetS  = 30,

	// These are 0
	/*
	kUnknown1 = 31,
	kUnknown2 = 32,
	kUnknown3 = 33,
	kUnknown4 = 34,
	kUnknown5 = 35,
	*/

	kPickupN  = 36,
	kPickupNW = 37,
	kPickupW  = 38,
	kPickupSW = 39,
	kPickupS  = 40,

	kOperateN  = 41,
	kOperateNW = 42,
	kOperateW  = 43,
	kOperateSW = 44,
	kOperateS  = 45,

	kOperate2N  = 46,
	kOperate2NW = 47,
	kOperate2W  = 48,
	kOperate2SW = 49,
	kOperate2S  = 50,

	// These are 0
	/*
	kUnknown6  = 51,
	kUnknown7  = 52,
	kUnknown8  = 53,
	kUnknown9  = 54,
	kUnknown10 = 55,
	kUnknown11 = 56,
	kUnknown12 = 57,
	kUnknown13 = 58,
	kUnknown14 = 59
	*/

	// Horizontally flipped animations (100 + regular anim Id)
	kWalkNE = 107,
	kWalkE  = 108,
	kWalkSE = 109,

	kFaceNE = 112,
	kFaceE  = 113,
	kFaceSE = 114
	// TODO: finish these
};

//////////////////////////////////////////////////////////////////////////
// Barrier
//////////////////////////////////////////////////////////////////////////
enum BarrierFlag {
	kBarrierFlagDestroyed = -2,
	kBarrierFlag4         = 0x4,
	kBarrierFlag8         = 0x8,
	kBarrierFlag20        = 0x20,
	kBarrierFlagC000      = 0xC000,
	kBarrierFlag10000     = 0x10000,
	kBarrierFlag10E38     = 0x10E38,
	kBarrierFlag20000     = 0x20000,
	kBarrierFlag40000     = 0x40000
};

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////
#define CLEAR_ARRAY(type, name) { \
	for (Common::Array<type *>::iterator it = name.begin(); it != name.end(); it++) {\
		delete *it; \
		*it = NULL; \
	} \
	name.clear(); \
}

//////////////////////////////////////////////////////////////////////////
// Macros
//////////////////////////////////////////////////////////////////////////

// Misc
#define rnd(val) _vm->getRandom(val)

// Engine subclasses
#define getSound()  _vm->sound()
#define getScene()  _vm->scene()
#define getScreen() _vm->screen()
#define getWorld()  _vm->scene()->worldstats()




} // End of namespace Asylum

#endif // ASYLUM_SHARED_H
