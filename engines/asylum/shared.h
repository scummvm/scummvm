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
	kGameFlag186                  = 186,
	kGameFlagCommentLeavingCell   = 214,
	kGameFlag219                  = 219,
	kGameFlagSolveVCRBlowUpPuzzle = 220,
	kGameFlag235                  = 235,
	kGameFlag238                  = 238,
	kGameFlag239                  = 239,
	kGameFlag243                  = 243,
	kGameFlag244                  = 244,
	kGameFlag245                  = 245,
	kGameFlag246                  = 246,
	kGameFlag247                  = 247,
	kGameFlag248                  = 248,
	kGameFlag263                  = 263,
	kGameFlag262                  = 262,
	kGameFlag264                  = 264,
	kGameFlag265                  = 265,
	kGameFlag266                  = 266,
	kGameFlag267                  = 267,
	kGameFlag268                  = 268,
	kGameFlag269                  = 269,
	kGameFlag270                  = 270,
	kGameFlag271                  = 271,
	kGameFlag272                  = 272,
	kGameFlag273                  = 273,
	kGameFlag274                  = 274,
	kGameFlag275                  = 275,
	kGameFlag276                  = 276,
	kGameFlag279                  = 279,
	kGameFlag353                  = 353,
	kGameFlag354                  = 354,
	kGameFlag355                  = 355,
	kGameFlag358                  = 358,
	kGameFlag368                  = 368,
	kGameFlag369                  = 369,
	kGameFlag370                  = 370,
	kGameFlag371                  = 371,
	kGameFlag387                  = 387,
	kGameFlag405                  = 405,
	kGameFlag423                  = 423,
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
	kGameFlag470                  = 470,
	kGameFlag528                  = 528,
	kGameFlag543                  = 543,
	kGameFlag544                  = 544,
	kGameFlag545                  = 545,
	kGameFlag553                  = 553,
	kGameFlag556                  = 556,
	kGameFlag560                  = 560,
	kGameFlag570                  = 570,
	kGameFlag776                  = 776,
	kGameFlag815                  = 815,
	kGameFlag816                  = 816,
	kGameFlag817                  = 817,
	kGameFlag818                  = 818,
	kGameFlag819                  = 819,
	kGameFlag820                  = 820,
	kGameFlag821                  = 821,
	kGameFlag822                  = 822,
	kGameFlag823                  = 823,
	kGameFlag824                  = 824,
	kGameFlag825                  = 825,
	kGameFlag826                  = 826,
	kGameFlag827                  = 827,
	kGameFlag828                  = 828,
	kGameFlag829                  = 829,
	kGameFlag830                  = 830,
	kGameFlag866                  = 866,
	kGameFlag875                  = 875,
	kGameFlag876                  = 876,
	kGameFlag877                  = 877,
	kGameFlag878                  = 878,
	kGameFlag879                  = 879,
	kGameFlag880                  = 880,
	kGameFlag881                  = 881,
	kGameFlag897                  = 897,
	kGameFlagFinishGame           = 901,
	kGameFlag937                  = 937,
	kGameFlag1009                 = 1009,
	kGameFlag1021                 = 1021,
	kGameFlag1022                 = 1022,
	kGameFlag1023                 = 1023,
	kGameFlag1062                 = 1062,
	kGameFlag1063                 = 1063,
	kGameFlag1064                 = 1064,
	kGameFlag1065                 = 1065,
	kGameFlag1066                 = 1066,
	kGameFlag1067                 = 1067,
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
	kResourcePackText                  = 0,
	kResourcePackShared                = 1,
	kResourcePackMusic                 = 2,    // Special case: will load from mus* resources
	kResourcePackSpeech                = 3,
	kResourcePackSharedSound           = 4,
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
	kResourcePack304                   = 304,

	kResourcePackInvalid               = 0xFFF
};

#define MAKE_RESOURCE(pack, index) (ResourceId)((((pack) << 16) + 0x80000000) + (uint32)(index))

typedef unsigned int ResourceId;

enum ResourcesId {
	kResourceNone         = 0
};

enum MusicIndex {
	kMusicStopped         = 0xFFFFFD66 // -666
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

enum ActorDirection {
	kDirectionInvalid = -1,
	kDirectionN = 0,
	kDirectionNO = 1,
	kDirectionO = 2,
	kDirectionSO = 3,
	kDirectionS = 4,
	kDirectionSE = 5,
	kDirectionE = 6,
	kDirectionNE = 7,
	kDirection8 = 8
};

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
	kObjectTicaTac01               = 391,
	kObjectGuyFalls                = 434,
	kObjectGuyWobbles              = 405,
	kObjectRocker                  = 441,
	kObjectOpeningGate             = 485,
	kObjectTableRecordRoom         = 659,
	kObject703                     = 703,
	kObjectTree8                   = 726,
	kObjectSlab                    = 743,
	kObjectPreacherBobAssistant    = 802,
	kObjectPreacherBob             = 803,
	kObjectNPC024Church            = 816,
	kObjectNPC024Fountain          = 825,
	kObjectNPC027Sit               = 838,
	kObjectNPC028Sit               = 839,
	kObjectNPC029Sit               = 840,
	kObjectNPC027Dancing           = 844,
	kObjectNPC028Dancing           = 845,
	kObjectNPC026OutOfWay          = 861,
	kObjectNPC026Talking           = 862,
	kObjectOrangeRecord            = 920,
	kObjectJessieStatusQuo         = 984,
	kObjectMarty02                 = 991,
	kObjectEileenOnBench           = 993,
	kObject994                     = 994,
	kObjectFishingBoy              = 1001,
	kObjectDennisStatusQuo         = 1011,
	kObjectSailorBoy               = 1013,
	kObjectEleenOnGround           = 1019,
	kObjectSailorStatusQuo         = 1021,
	kObjectNPC026TalkStatusQuo     = 1038,
	kObjectSuckerSittingStatusQuo  = 1084,
	kObjectBubbles                 = 1185,
	kObjectGlow                    = 1186,
	kObjectDome                    = 1187,
	kObjectRedLight                = 1188,
	kObjectRing                    = 1189,
	kObjectBallMovesUpright        = 1190,
	kObjectGearsLightUp            = 1191,
	kObjectMariaPointsLeft         = 1200,
	kObjectMariaPointsRight        = 1201,
	kObjectNPC032Sleeping          = 1250,
	kObjectNPC032StatusQuoOutside  = 1254,
	kObjectNPC032StatusQuoBigTop   = 1256,
	kObjectGlobe                   = 1261,
	kObjectDrawers3                = 1276,
	kObjectChalice                 = 1286,
	kObjectFreezerHallInterior     = 1337,
	kObjectNPC033StartEnc          = 1338,
	kObjectNPC033GetBook           = 1343,
	kObjectNPC033Reading           = 1344,
	kObjectOldMan3                 = 1377,
	kObjectTattooManStatusQuo      = 1391,   // NPC 34
	kObjectStrongmanStatusQuo      = 1402,   // NPC 35
	kObjectStrongmanStatusQuo2     = 1405,   // NPC 35
	kObjectInfernoStatusQuo        = 1408,   // NPC 36
	kObjectJugglerWithPin          = 1423,   // NPC 37
	kObjectJuggler                 = 1424,   // NPC 37
	kObjectClownStatusQuo          = 1428,   // NPC 38
	kObjectTrixieStatusQuo         = 1432,   // NPC 39
	kObjectSimonStatusQuo          = 1434,   // NPC 40
	kObjectBigTopBarrel            = 1436,
	kObjectFunTixStatusQuo         = 1437,   // NPC 44
	kObjectStandBehindJuggler      = 1438,
	kObjectFreakTixStatusQuoUp     = 1439,   // NPC 45
	kObjectFreakTixStatusQuoDown   = 1440,   // NPC 45
	kObjectFortTellerStatusQuo     = 1444,   // NPC 45
	kObjectStrongmanLeft           = 1449,   // NPC 35
	kObjectStrongmanRight          = 1450,   // NPC 35
	kObjectRingTossStatusQuo       = 1451,   // NPC 47
	kObjectKnockDownStatusQuo      = 1455,   // NPC 47
	kObjectPigShootStatusQuo       = 1456,   // NPC 47
	kObjectPretZoolStatusQuo       = 1461,   // NPC 48
	kObjectTimberStatusQuo         = 1462,   // NPC 49
	kObjectTwinsStatusQuo          = 1465,   // NPC 50
	kObjectSeanStatusQuo           = 1468,   // NPC 51
	kObjectMomAndPopStatusQuo      = 1470,   // NPCs 52-53
	kObjectCrow1FlysAway           = 1485,
	kObjectCrow1Pecks              = 1486,
	kObjectCrow2AmbientPecks       = 1495,
	kObjectCrow2FlysAway           = 1500,
	kObjectCrow3FlysAway           = 1502,
	kObjectCrow3Pecks              = 1503,
	kObjectCrow4FlysAway           = 1506,
	kObjectCrow4Pecks              = 1507,
	kObjectPumpkin1Loop            = 1523,
	kObjectPumpkin2Loop            = 1524,
	kObjectPumpkin3Loop            = 1525,
	kObjectPumpkin4Loop            = 1526,
	kObjectPumpkin5Loop            = 1527,
	kObjectPumpkin6Loop            = 1528,
	kObjectPumpkin7Loop            = 1529,
	kObjectPumpkin1Dies            = 1530,
	kObjectPumpkin2Dies            = 1531,
	kObjectPumpkin3Dies            = 1532,
	kObjectPumpkin4Dies            = 1533,
	kObjectPumpkin5Dies            = 1534,
	kObjectPumpkin6Dies            = 1535,
	kObjectPumpkin7Dies            = 1536,
	kObjectDennisStatus2           = 1580,
	kObjectPreAlphaNut             = 1582,
	kObjectPreAlphaNut2            = 1584,
	kObjectDeadMomFromOutside      = 1589,
	kObjectHeadOnTable             = 1595,
	kObjectWitchDoctor             = 1778,
	kObjectStoneWifeStatuQuo       = 1782,
	kObjectFishermanWidowStatusQuo = 1786,
	kObjectWheel1                  = 2113,
	kObjectWheel2                  = 2114,
	kObjectWheel3                  = 2115,
	kObjectWheel4                  = 2116,
	kObjectWheel5                  = 2117,
	kObjectWheel6                  = 2118,
	kObjectWheel7                  = 2119,
	kObjectStoneMasonStatusHitt    = 2123,
	kObjectFlamingHeadLeftSide     = 2129,
	kObjectHook1B                  = 2131,
	kObjectHook2B                  = 2132,
	kObjectHook3B                  = 2133,
	kObjectHook4B                  = 2134,
	kObjectHook5B                  = 2135,
	kObjectHook6B                  = 2136,
	kObjectHook0Down               = 2152,
	kObjectHook2Down               = 2154,
	kObjectHook3Down               = 2155,
	kObjectHook4Down               = 2156,
	kObjectHook5Down               = 2157,
	kObjectHook6Down               = 2158,
	kObject2230                    = 2230,
	kObjectGravinStatusQuoCyber    = 2324,
	kObjectGravinWorkMachine       = 2337,
	kObjectNPC062GritzaStatusQuo   = 2399,
	kObjectNPC063GrundleStatusQuo  = 2410,
	kObjectNPC064GrellaStatusQuo   = 2418,
	kObjectLavaBridge              = 2445,
	kObjectNPC065StatusQuo         = 2469,
	kObjectGlow4                   = 2478,
	kObjectBugOnTable              = 2480,
	kObjectWadeThroughLava         = 2500,
	kObjectNPC066StatusQuo         = 2504,
	kObject2507                    = 2507,
	kObjectBrokenPipe              = 2508,
	kObjectBodySlides1             = 2514,
	kObjectBodySlides2             = 2515,
	kObjectBodySlides3             = 2516,
	kObjectBodySlides4             = 2517,
	kObjectBodySlides5             = 2518,
	kObjectBodySlides6             = 2519,
	kObjectEmberPopsOut            = 2526,
	kObjectBugCarriesEmber         = 2527,
	kObjectGirlStatusQuo           = 2541,
	kObjectWitchWifeStirring       = 2543,
	kObjectFurnaceHole             = 2554,
	kObjectInsidePipeCyberPod      = 2555,
	kObjectTopOfFurnace            = 2566,
	kObjectElderBugLimb            = 2572,
	kObjectQuetzlcaotlStatusQuo    = 2578,
	kObjectGhost1                  = 2579,
	kObjectGhost2                  = 2580,
	kObjectGhost3                  = 2581,
	kObjectGhost4                  = 2582,
	kObjectGhost5                  = 2583,
	kObjectGhost6                  = 2584,
	kObjectDiscardedBugPincer      = 2587,
	kObjectGong1                   = 2590,
	kObjectGong2                   = 2591,
	kObjectGong3                   = 2592,
	kObjectGong4                   = 2593,
	kObjectGong5                   = 2594,
	kObjectGong6                   = 2596,
	kObjectGhost2b                 = 2634,
	kObjectRitualLoop              = 2674,
	kObjectBlanklPixelForWarrior   = 2884,
	kObjectPixelForHutWarrior      = 2885,
	kObjectPixelForWaterfallGhost  = 2886,
	kObjectLitLimbScanner          = 3061,
	kObjectOfficeWallNew           = 3062,
	kObjectCyberTable              = 3065,

	kObjectEnd
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
#define getSharedData()  _vm->getData()
#define getEncounter()   _vm->encounter()
#define getCursor()      _vm->cursor()
#define getResource()    _vm->resource()
#define getSound()       _vm->sound()
#define getScene()       _vm->scene()
#define getScreen()      _vm->screen()
#define getScript()      _vm->scene()->actions()
#define getSpeech()      _vm->scene()->speech()
#define getText()        _vm->text()
#define getVideo()       _vm->video()
#define getWorld()       _vm->scene()->worldstats()

//////////////////////////////////////////////////////////////////////////
// Sub-integer partial access macros
//////////////////////////////////////////////////////////////////////////
#define LOBYTE(d)   (*((char*)&(d)))
#define BYTE1(d)    (*((char*)&(d)+1))   // Same as HIBYTE()

//////////////////////////////////////////////////////////////////////////
// Resource ID macros
//////////////////////////////////////////////////////////////////////////
#define RESOURCE_PACK(id) (ResourcePackId)((((uint32)id) >> 16) & 0x7FFF)
#define RESOURCE_INDEX(id) (uint16)(id)

} // End of namespace Asylum

#endif // ASYLUM_SHARED_H
