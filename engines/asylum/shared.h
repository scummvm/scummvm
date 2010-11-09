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
	kGameFlag243                  = 243,
	kGameFlag244                  = 244,
	kGameFlag245                  = 245,
	kGameFlag246                  = 246,
	kGameFlag247                  = 247,
	kGameFlag248                  = 248,
	kGameFlag279                  = 279,
	kGameFlag368                  = 368,
	kGameFlag371                  = 371,
	kGameFlag387                  = 387,
	kGameFlag438                  = 438,
	kGameFlag439                  = 439,
	kGameFlag440                  = 440,
	kGameFlag441                  = 441,
	kGameFlag442                  = 442,
	kGameFlag447                  = 447,
	kGameFlag448                  = 448,
	kGameFlag450                  = 450,
	kGameFlag451                  = 451,
	kGameFlag452                  = 452,
	kGameFlag453                  = 453,
	kGameFlag454                  = 454,
	kGameFlag455                  = 455,
	kGameFlag556                  = 556,
	kGameFlag776                  = 776,
	kGameFlagFinishGame           = 901,
	kGameFlag1009                 = 1009,
	kGameFlag1021                 = 1021,
	kGameFlag1022                 = 1022,
	kGameFlag1023                 = 1023,
	kGameFlag1108                 = 1108,
	kGameFlag1131                 = 1131,
	kGameFlag1137                 = 1137
};

enum ChapterIndex {
	kChapterNone = 0,
	kChapter1,
	kChapter2,
	kChapter3,
	kChapter4,
	kChapter5,
	kChapter6,
	kChapter7,
	kChapter8,
	kChapter9,
	kChapter10,
	kChapter11,
	kChapter12,
	kChapter13
};

//////////////////////////////////////////////////////////////////////////
// Resources
//////////////////////////////////////////////////////////////////////////

enum ResourcePackId {
	kResourcePackInvalid               = -1,
	kResourcePackText                  = 0,
	kResourcePackShared                = 1,
	kResourcePackMusic                 = 2,    // Special case: will load from mus* resources
	kResourcePackSpeech                = 3,
	kResourcePackSpeechOther           = 4,
	kResourcePackTowerCells            = 5,
	kResourcePackInnocentAbandoned     = 6,
	kResourcePackCourtyardAndChapel    = 7,
	kResourcePackCircusOfFools         = 8,
	kResourcePackCave                  = 9,
	kResourcePackMansion               = 10,
	kResourcePackLaboratory            = 11,
	kResourcePackHive                  = 12,
	kResourcePackMorgueAndCementary    = 13,
	kResourcePackLostVillage           = 14,
	kResourcePackMaze                  = 15,
	kResourcePackGauntlet              = 16,
	kResourcePackMorgansLastGame       = 17,
	kResourcePackSound                 = 18,
	kResourcePack104                   = 104,
	kResourcePack204                   = 204,
	kResourcePack304                   = 304
};

#define MAKE_RESOURCE(pack, index) (ResourceId)((0x80000000 + ((pack) << 16)) + (index))

enum ResourceId {
	kResourceNone         = 0,
	kResourceMusicStopped = 0xFFFFFD66 // -666
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
	kActorInvalid = -1,
	// TODO: check if the names match the actor type
	kActorMax     = 0,
	kActorSarah   = 1,
	kActorCyclops = 2,
	kActorAztec   = 3
};

enum ActorStatus {
	kActorStatusNone = 0,
	kActorStatus1 = 1,
	kActorStatus2,
	kActorStatus3,
	kActorStatusEnabled,
	kActorStatusDisabled,    // 5
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
	kActorFlagVisible = 1,
	kActorFlagMasked  = 2
};

enum DirectionFrom {
	kDirectionFromObject = 0,
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
// Object
//////////////////////////////////////////////////////////////////////////
enum ObjectId {
	kObjectInvalid                 = -1,
	kObjectNone                    = 0,
	kObjectHeadBanger              = 101,
	kObjectAngelFlares             = 112,
	kObjectGuyFalls                = 434,
	kObjectOpeningGate             = 485,
	kObjectBubbles                 = 1185,
	kObjectGlow                    = 1186,
	kObjectDome                    = 1187,
	kObjectRedLight                = 1188,
	kObjectRing                    = 1189,
	kObjectBallMovesUpright        = 1190,
	kObjectGearsLightUp            = 1191,
	kObjectGlobe                   = 1261,
	kObjectFreezerHallInterior     = 1337,
	kObjectNPC033StartEnc          = 1338,
	kObjectNPC033GetBook           = 1343,
	kObjectCrow1FlysAway           = 1485,
	kObjectCrow1Pecks              = 1486,
	kObjectCrow2AmbientPecks       = 1495,
	kObjectCrow2FlysAway           = 1500,
	kObjectCrow3FlysAway           = 1502,
	kObjectCrow3Pecks              = 1503,
	kObjectCrow4FlysAway           = 1506,
	kObjectCrow4Pecks              = 1507,
	kObjectBodySlides1             = 2514,
	kObjectBodySlides2             = 2515,
	kObjectBodySlides3             = 2516,
	kObjectBodySlides4             = 2517,
	kObjectBodySlides5             = 2518,
	kObjectBodySlides6             = 2519
};

enum ObjectFlag {
	kObjectFlagEnabled   = 0x1,
	kObjectFlag2         = 0x2,
	kObjectFlag4         = 0x4,
	kObjectFlag6         = 0x6,
	kObjectFlag8         = 0x8,
	kObjectFlag10        = 0x10,
	kObjectFlag20        = 0x20,
	kObjectFlag40        = 0x40,
	kObjectFlag80        = 0x80,
	kObjectFlagC000      = 0xC000,
	kObjectFlag10000     = 0x10000,
	kObjectFlag10E38     = 0x10E38,
	kObjectFlag20000     = 0x20000,
	kObjectFlag40000     = 0x40000
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
#define getEncounter() _vm->encounter()
#define getCursor()    _vm->cursor()
#define getResource()  _vm->resource()
#define getSound()     _vm->sound()
#define getScene()     _vm->scene()
#define getScreen()    _vm->screen()
#define getScript()    _vm->scene()->actions()
#define getSpeech()    _vm->scene()->speech()
#define getText()      _vm->text()
#define getVideo()     _vm->video()
#define getWorld()     _vm->scene()->worldstats()

//////////////////////////////////////////////////////////////////////////
// Sub-integer partial access macros
//////////////////////////////////////////////////////////////////////////
#define LOBYTE(d)   (*((char*)&(d)))
#define BYTE1(d)    (*((char*)&(d)+1))   // Same as HIBYTE()



} // End of namespace Asylum

#endif // ASYLUM_SHARED_H
