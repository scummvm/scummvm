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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ASYLUM_SHARED_H
#define ASYLUM_SHARED_H

namespace Asylum {

//////////////////////////////////////////////////////////////////////////
// Global
//////////////////////////////////////////////////////////////////////////
enum GameFlag {
	kGameFlag0                    = 0,

	kGameFlag4                    = 4,
	kGameFlag12                   = 12,
	kGameFlag52                   = 52,
	kGameFlag86                   = 86,
	kGameFlag87                   = 87,
	kGameFlag88                   = 88,
	kGameFlagBrokenPipeSpraying   = 96,
	kGameFlagSmFtnOverflows       = 97,
	kGameFlagFountainFilling      = 98,
	kGameFlagSewerExplodes        = 99,
	kGameFlag114                  = 114,
	kGameFlag115                  = 115,
	kGameFlag128                  = 128,
	kGameFlag169                  = 169,
	kGameFlagScriptProcessing     = 183,
	kGameFlag186                  = 186,
	kGameFlagCommentLeavingCell   = 214,
	kGameFlag215                  = 215,
	kGameFlag219                  = 219,
	kGameFlagSolveVCRPuzzle       = 220,
	kGameFlag235                  = 235,
	kGameFlag238                  = 238,
	kGameFlag239                  = 239,
	kGameFlag243                  = 243,
	kGameFlag244                  = 244,
	kGameFlag245                  = 245,
	kGameFlag246                  = 246,
	kGameFlag247                  = 247,
	kGameFlag248                  = 248,
	kGameFlag249                  = 249,
	kGameFlag253                  = 253,
	kGameFlag254                  = 254,
	kGameFlag255                  = 255,
	kGameFlag256                  = 256,
	kGameFlag257                  = 257,
	kGameFlag258                  = 258,
	kGameFlag259                  = 259,
	kGameFlag260                  = 260,
	kGameFlag261                  = 261,
	kGameFlag262                  = 262,
	kGameFlag263                  = 263,
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
	kGameFlag281                  = 281,
	kGameFlag282                  = 282,
	kGameFlag283                  = 283,
	kGameFlag284                  = 284,
	kGameFlag289                  = 289,
	kGameFlag319                  = 319,
	kGameFlag320                  = 320,
	kGameFlag321                  = 321,
	kGameFlag322                  = 322,
	kGameFlag323                  = 323,
	kGameFlag353                  = 353,
	kGameFlag354                  = 354,
	kGameFlag355                  = 355,
	kGameFlag358                  = 358,
	kGameFlag368                  = 368,
	kGameFlag369                  = 369,
	kGameFlag370                  = 370,
	kGameFlag371                  = 371,
	kGameFlag384                  = 384,
	kGameFlag387                  = 387,
	kGameFlag391                  = 391,
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
	kGameFlag481                  = 481,
	kGameFlag511                  = 511,
	kGameFlag528                  = 528,
	kGameFlag543                  = 543,
	kGameFlag544                  = 544,
	kGameFlag545                  = 545,
	kGameFlag553                  = 553,
	kGameFlag556                  = 556,
	kGameFlag557                  = 557,
	kGameFlag558                  = 558,
	kGameFlag560                  = 560,
	kGameFlag561                  = 561,
	kGameFlag562                  = 562,
	kGameFlag563                  = 563,
	kGameFlag564                  = 564,
	kGameFlag565                  = 565,
	kGameFlag566                  = 566,
	kGameFlag567                  = 567,
	kGameFlag568                  = 568,
	kGameFlag569                  = 569,
	kGameFlag570                  = 570,
	kGameFlag572                  = 572,
	kGameFlag582                  = 582,
	kGameFlag583                  = 583,
	kGameFlag596                  = 596,
	kGameFlag597                  = 597,
	kGameFlag598                  = 598,
	kGameFlag599                  = 599,
	kGameFlag600                  = 600,
	kGameFlag608                  = 608,
	kGameFlag619                  = 619,
	kGameFlag635                  = 635,
	kGameFlag636                  = 636,
	kGameFlag637                  = 637,
	kGameFlagSolveHiveControl     = 640,
	kGameFlag684                  = 684,
	kGameFlag685                  = 685,
	kGameFlag686                  = 686,
	kGameFlag687                  = 687,
	kGameFlag688                  = 688,
	kGameFlag689                  = 689,
	kGameFlag690                  = 690,
	kGameFlag691                  = 691,
	kGameFlag692                  = 692,
	kGameFlag693                  = 693,
	kGameFlag694                  = 694,
	kGameFlag695                  = 695,
	kGameFlag696                  = 696,
	kGameFlag697                  = 697,
	kGameFlag698                  = 698,
	kGameFlag699                  = 699,
	kGameFlag700                  = 700,
	kGameFlag701                  = 701,
	kGameFlag702                  = 702,
	kGameFlag703                  = 703,
	kGameFlag704                  = 704,
	kGameFlag705                  = 705,
	kGameFlag706                  = 706,
	kGameFlag707                  = 707,
	kGameFlag708                  = 708,
	kGameFlag722                  = 722,
	kGameFlag723                  = 723,
	kGameFlag724                  = 724,
	kGameFlag725                  = 725,
	kGameFlag726                  = 726,
	kGameFlag727                  = 727,
	kGameFlag728                  = 728,
	kGameFlag729                  = 729,
	kGameFlag730                  = 730,
	kGameFlag759                  = 759,
	kGameFlag760                  = 760,
	kGameFlag761                  = 761,
	kGameFlag776                  = 776,
	kGameFlag790                  = 790,
	kGameFlag791                  = 791,
	kGameFlag792                  = 792,
	kGameFlag794                  = 794,
	kGameFlag795                  = 795,
	kGameFlag796                  = 796,
	kGameFlag797                  = 797,
	kGameFlag801                  = 801,
	kGameFlag802                  = 802,
	kGameFlag803                  = 803,
	kGameFlag804                  = 804,
	kGameFlag805                  = 805,
	kGameFlag806                  = 806,
	kGameFlag809                  = 809,
	kGameFlag810                  = 810,
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
	kGameFlagSolveHiveMachine     = 834,
	kGameFlag866                  = 866,
	kGameFlag875                  = 875,
	kGameFlag876                  = 876,
	kGameFlag877                  = 877,
	kGameFlag878                  = 878,
	kGameFlag879                  = 879,
	kGameFlag880                  = 880,
	kGameFlag881                  = 881,
	kGameFlag897                  = 897,
	kGameFlag899                  = 899,
	kGameFlagFinishGame           = 901,
	kGameFlag925                  = 925,
	kGameFlag937                  = 937,
	kGameFlag1009                 = 1009,
	kGameFlag1021                 = 1021,
	kGameFlag1022                 = 1022,
	kGameFlag1023                 = 1023,
	kGameFlag1054                 = 1054,
	kGameFlag1055                 = 1055,
	kGameFlag1056                 = 1056,
	kGameFlag1057                 = 1057,
	kGameFlag1058                 = 1058,
	kGameFlag1059                 = 1059,
	kGameFlag1060                 = 1060,
	kGameFlag1061                 = 1061,
	kGameFlag1062                 = 1062,
	kGameFlag1063                 = 1063,
	kGameFlag1064                 = 1064,
	kGameFlag1065                 = 1065,
	kGameFlag1066                 = 1066,
	kGameFlag1067                 = 1067,
	kGameFlag1099                 = 1099,
	kGameFlag1108                 = 1108,
	kGameFlag1121                 = 1121,
	kGameFlag1122                 = 1122,
	kGameFlag1131                 = 1131,
	kGameFlag1137                 = 1137,
	kGameFlag1144                 = 1144,
	kGameFlag3189                 = 3189,
	kGameFlag3351                 = 3351,
	kGameFlag3386                 = 3386,
	kGameFlag3387                 = 3387,
	kGameFlag3388                 = 3388,
	kGameFlag3389                 = 3389,
	kGameFlag3754                 = 3754,
	kGameFlag3755                 = 3755,
	kGameFlag3810                 = 3810,
	kGameFlag3823                 = 3823,
	kGameFlag3842                 = 3842,
	kGameFlag3843                 = 3843,
	kGameFlag3931                 = 3931
};

enum ChapterIndex {
	kChapterInvalid = -1,
	kChapterNone    = 0,
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
	kResourcePackLaboratory            = 9,
	kResourcePackHive                  = 10,
	kResourcePackMorgueAndCemetery     = 11,
	kResourcePackLostVillage           = 12,
	kResourcePackGauntlet              = 13,
	kResourcePackMansion               = 14,
	kResourcePackCave                  = 15,
	kResourcePackMaze                  = 16,
	kResourcePackMorgansLastGame       = 17,
	kResourcePackSound                 = 18,
	kResourcePack104                   = 104,
	kResourcePack204                   = 204,
	kResourcePack304                   = 304,

	kResourcePackInvalid               = 0xFFF
};

#define MAKE_RESOURCE(pack, index) (ResourceId)((((pack) << 16) + 0x80000000) + (unsigned) (int32)(index))

typedef int ResourceId;

enum ResourcesId {
	kResourceInvalid = -1,
	kResourceNone    = 0
};

#define kMusicStopped -666

//////////////////////////////////////////////////////////////////////////
// Drawing
//////////////////////////////////////////////////////////////////////////
enum DrawFlags {
	kDrawFlagNone            = 0,
	kDrawFlagMirrorLeftRight = 2
};

//////////////////////////////////////////////////////////////////////////
// Actions
//////////////////////////////////////////////////////////////////////////
enum ActionType {
	kActionTypeNone = 0,
	kActionTypeFind = 1,
	kActionTypeTalk = 2,
	kActionTypeGrab = 4,
	kActionType8    = 8,
	kActionType16   = 16
};

//////////////////////////////////////////////////////////////////////////
// Script
//////////////////////////////////////////////////////////////////////////
enum OpcodeType {
	kOpcodeReturn = 0,
	kOpcodeSetGameFlag,
	kOpcodeClearGameFlag,
	kOpcodeToggleGameFlag,
	kOpcodeJumpIfGameFlag,
	kOpcodeHideCursor,                          // 5
	kOpcodeShowCursor,
	kOpcodePlayAnimation,
	kOpcodeMoveScenePosition,
	kOpcodeHideActor,
	kOpcodeShowActor,                           // 10
	kOpcodeSetActorPosition,
	kOpcodeSetSceneMotionStatus,
	kOpcodeDisableActor,
	kOpcodeEnableActor,
	kOpcodeEnableObjects,                       // 15
	kOpcodeReturn1,
	kOpcodeRemoveObject,
	kOpcodeJumpActorSpeech,
	kOpcodeJumpAndSetDirection,
	kOpcodeJumpIfActorCoordinates,              // 20
	kOpcodeNop,
	kOpcodeResetAnimation,
	kOpcodeDisableObject,
	kOpcodeJumpIfSoundPlayingAndPlaySound,
	kOpcodeJumpIfActionFind,                    // 25
	kOpcodeSetActionFind,
	kOpcodeClearActionFind,
	kOpcodeJumpIfActionGrab,
	kOpcodeSetActionGrab,
	kOpcodeClearActionGrab,                     // 30
	kOpcodeJumpIfActionTalk,
	kOpcodeSetActionTalk,
	kOpcodeClearActionTalk,
	kOpcodeAddToInventory,
	kOpcodeRemoveFromInventory,                 // 35
	kOpcodeJumpIfInventoryOmits,
	kOpcodeRunEncounter,
	kOpcodeJumpIfAction16,
	kOpcodeSetAction16,
	kOpcodeClearAction16,                       // 40
	kOpcodeSelectInventoryItem,
	kOpcodeJumpIfInventoryItemNotSelected,
	kOpcodeChangeScene,
	kOpcodeInteract,
	kOpcodePlayMovie,                           // 45
	kOpcodeStopAllObjectsSounds,
	kOpcodeStopProcessing,
	kOpcodeResumeProcessing,
	kOpcodeResetSceneRect,
	kOpcodeChangeMusicById,                     // 50
	kOpcodeStopMusic,
	kOpcodeIncrementParam1,
	kOpcodeSetVolume,
	kOpcodeJump,
	kOpcodeRunPuzzle,                           // 55
	kOpcodeJumpIfAction8,
	kOpcodeSetAction8,
	kOpcodeClearAction8,
	kOpcodeCreatePalette,
	kOpcodeIncrementParam2,                     // 60
	kOpcodeWaitUntilFramePlayed,
	kOpcodeUpdateWideScreen,
	kOpcodeJumpIfActor,
	kOpcodePlaySpeechScene,
	kOpcodePlaySpeech,                          // 65
	kOpcodePlaySpeechScene2,
	kOpcodeMoveScenePositionFromActor,
	kOpcodePaletteFade,
	kOpcodeQueuePaletteFade,
	kOpcodePlaySoundUpdateObject,               // 70
	kOpcodeActorFaceTarget,
	kOpcodeHidMatteBars,
	kOpcodeShowMatteBars,
	kOpcodeJumpIfSoundPlaying,
	kOpcodeChangePlayer,                         // 75
	kOpcodeChangeActorStatus,
	kOpcodeStopSound,
	kOpcodeJumpRandom,
	kOpcodeClearScreen,
	kOpcodeQuit,                                // 80
	kOpcodeJumpObjectFrame,
	kOpcodeDeleteGraphics,
	kOpcodeSetPlayerField944,
	kOpcodeSetScriptField1BB0,
	kOpcodeOnScriptField1BB0,                   // 85
	kOpcodeWalkToActor,
	kOpcodeSetResourcePalette,
	kOpcodeSetObjectFrameIndexAndFlags,
	kOpcodeSetObjectFlags,
	kOpcodeSetActorActionIndex2,                // 90
	kOpcodeUpdateTransparency,
	kOpcodeQueueScript,
	kOpcodeProcessActor,
	kOpcodeClearActorFields,
	kOpcodeSetObjectLastFrameIndex,             // 95
	kOpcodeSetActionAreaFlags,
	kOpcodeMorphActor,
	kOpcodeShowMenu,
	kOpcodeUpdateGlobalFlags
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
	kActorStatusNone    = 0,
	kActorStatusWalking = 1,      // Walking
	kActorStatusWalkingTo,        // Auto-walking to target
	kActorStatusInteracting,
	kActorStatusEnabled,          // Standing Still
	kActorStatusDisabled,         // 5
	kActorStatusShowingInventory,
	kActorStatusStoppedInteracting,
	kActorStatus8,
	kActorStatusFidget,
	kActorStatus10,
	kActorStatus11,
	kActorStatusWalking2,
	kActorStatusWalkingTo2,
	kActorStatusEnabled2,
	kActorStatusAttacking,        // 15
	kActorStatusGettingHurt,
	kActorStatusRestarting,
	kActorStatus18,
	kActorStatusHittingPumpkin,
	kActorStatusStoppedHitting,
	kActorStatusMorphingInto
};

enum ActorDirection {
	kDirectionInvalid = -1,
	kDirectionN       = 0,
	kDirectionNW      = 1,
	kDirectionW       = 2,
	kDirectionSW      = 3,
	kDirectionS       = 4,
	kDirectionSE      = 5,
	kDirectionE       = 6,
	kDirectionNE      = 7,
	kDirection8       = 8
};

enum ActorFlags {
	kActorFlagVisible = 1,
	kActorFlagMasked  = 2
};

enum DirectionFrom {
	kDirectionFromObject     = 0,
	kDirectionFromPolygons   = 1,
	kDirectionFromActor      = 2,
	kDirectionFromParameters = 3
};

//////////////////////////////////////////////////////////////////////////
// Object
//////////////////////////////////////////////////////////////////////////
enum ObjectId {
	kObjectInvalid                 =   -1,
	kObjectNone                    =    0,
	kObjectHeadBanger              =  101,
	kObjectAngelFlares             =  112,
	kObjectTicaTac01               =  391,
	kObjectGuyWobbles              =  405,
	kObjectGuyFalls                =  434,
	kObjectRocker                  =  441,
	kObjectOpeningGate             =  485,
	kObjectTableRecordRoom         =  659,
	kObject703                     =  703,
	kObjectTree8                   =  726,
	kObjectSlab                    =  743,
	kObjectDrMorgan                =  801,
	kObjectPreacherBobAssistant    =  802,
	kObjectPreacherBob             =  803,
	kObjectNPC024Church            =  816,
	kObjectNpc024TalkChurch        =  817,
	kObjectNpc024DanceAway         =  820,
	kObjectNpc024TalkFount         =  824,
	kObjectNPC024Fountain          =  825,
	kObjectNPC027Sit               =  838,
	kObjectNPC028Sit               =  839,
	kObjectNPC029Sit               =  840,
	kObjectNPC027Dancing           =  844,
	kObjectNPC028Dancing           =  845,
	kObjectNpc029Dancing           =  846,
	kObjectNPC026OutOfWay          =  861,
	kObjectNPC026Talking           =  862,
	kObjectNpc025Talking           =  863,
	kObjectNpc000Talking           =  873,
	kObjectOrangeRecord            =  920,
	kObjectNpc029Talk              =  922,
	kObjectNpc027Talk              =  925,
	kObjectNpc028Talk              =  927,
	kObjectJessieStatusQuo         =  984,
	kObjectMarty02                 =  991,
	kObjectEileenOnBench           =  993,
	kObject994                     =  994,
	kObjectFishingBoy              = 1001,
	kObjectDennisStatusQuo         = 1011,
	kObjectDennisTalk              = 1012,
	kObjectSailorBoy               = 1013,
	kObjectEleenOnGround           = 1019,
	kObjectSailorStatusQuo         = 1021,
	kObjectNPC026TalkStatusQuo     = 1038,
	kObjectDennisTalkHide          = 1051,
	kObjectDennisFoundSt           = 1052,
	kObjectBillyTalks              = 1061,
	kObjectJessieTalks             = 1072,
	kObjectSuckerTalks             = 1082,
	kObjectSuckerSittingStatusQuo  = 1084,
	kObjectBillyStatusUp           = 1090,
	kObjectJessieStatusWhileUp     = 1091,
	kObjectTalkToBallBoy           = 1105,
	kObjectIleanStatusFrame        = 1125,
	kObjectBubbles                 = 1185,
	kObjectGlow                    = 1186,
	kObjectDome                    = 1187,
	kObjectRedLight                = 1188,
	kObjectRing                    = 1189,
	kObjectBallMovesUpright        = 1190,
	kObjectGearsLightUp            = 1191,
	kObjectMarisStatusQuo          = 1199,
	kObjectMariaPointsLeft         = 1200,
	kObjectMariaPointsRight        = 1201,
	kObjectNPC032Sleeping          = 1250,
	kObjectNPC032StatusQuoOutside  = 1254,
	kObjectNpc032TalkOutside       = 1255,
	kObjectNPC032StatusQuoBigTop   = 1256,
	kObjectRingmasterTalkTent      = 1257,
	kObjectGlobe                   = 1261,
	kObjectDrawers3                = 1276,
	kObjectChalice                 = 1286,
	kObjectFreezerHallInterior     = 1337,
	kObjectNPC033StartEnc          = 1338,
	kObjectNpc033Talking           = 1339,
	kObjectNpc033SqNoBook          = 1341,
	kObjectNPC033GetBook           = 1343,
	kObjectNPC033Reading           = 1344,
	kObjectBodyTalks               = 1371,
	kObjectBodyStat                = 1373,
	kObjectOldMan3                 = 1377,
	kObjectTattooingStrongMan      = 1390,
	kObjectTattooManStatusQuo      = 1391,   // NPC 34
	kObjectTattooGuy034Talk        = 1392,
	kObjectStrongmanStatusQuo      = 1402,   // NPC 35
	kObjectStrongman035Talk        = 1403,
	kObjectStrongmanStatusQuo2     = 1405,   // NPC 35
	kObjectInfernoStatusQuo        = 1408,   // NPC 36
	kObjectInfernoTalk036          = 1409,
	kObjectJugglerWithPin          = 1423,   // NPC 37
	kObjectJuggler                 = 1424,   // NPC 37
	kObjectClown038Talk            = 1427,
	kObjectClownStatusQuo          = 1428,   // NPC 38
	kObjectTrixieStatusQuo         = 1432,   // NPC 39
	kObjectTrixie039Talk           = 1433,
	kObjectSimonStatusQuo          = 1434,   // NPC 40
	kObjectSSimon040Talk           = 1435,
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
	kObjectPretzool048Talk         = 1460,
	kObjectPretZoolStatusQuo       = 1461,   // NPC 48
	kObjectTimberStatusQuo         = 1462,   // NPC 49
	kObjectTimber049Talk           = 1463,
	kObjectTwinsStatusQuo          = 1465,   // NPC 50
	kObjectTwins050Talk            = 1466,
	kObjectSean051Talk             = 1467,
	kObjectSeanStatusQuo           = 1468,   // NPC 51
	kObjectMom052Talk              = 1469,
	kObjectMomAndPopStatusQuo      = 1470,   // NPCs 52-53
	kObjectPop053Talk              = 1471,
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
	kObjectMotherTalking           = 1587,
	kObjectDeadMomFromOutside      = 1589,
	kObjectHeadOnTable             = 1595,
	kObjectNpc049Sq2               = 1658,
	kObjectTreeTalks               = 1671,
	kObjectCave                    = 1763,
	kObjectWitchDoctor             = 1778,
	kObjectWitchTalks              = 1779,
	kObjectStoneMaison             = 1780,
	kObjectStoneMaisonTalk         = 1781,
	kObjectStoneWifeStatuQuo       = 1782,
	kObjectStoneWifeTalks          = 1783,
	kObjectFishermanWidowStatusQuo = 1786,
	kObjectFishermansWTalks        = 1787,
	kObjectOracleTalks             = 1853,
	kObjectOracleTalkStatus        = 1854,
	kObjectABarrier                = 1899,
	kObjectMonsterUp               = 1956,
	kObjectMonsterStatus           = 1957,
	kObjectMonsterDeath            = 1958,
	kObjectMonsterAttack           = 1959,
	kObjectPuke1                   = 1960,
	kObjectPuke2                   = 1961,
	kObjectPuke3                   = 1962,
	kObjectMonsterHurt             = 1997,
	kObjectBlinks                  = 2064,
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
	kObjectMotherTalkingSq         = 2280,
	kObjectGravinStatusQuoCyber    = 2324,
	kObjectGravinTalkCyber         = 2325,
	kObjectGravinSqGravins         = 2327,
	kObjectGravinTalkGravins       = 2328,
	kObjectGravinSqDoor            = 2329,
	kObjectGravinTalkDoor          = 2330,
	kObjectGravinSqGromnas         = 2332,
	kObjectGravinTalkGromnas       = 2333,
	kObjectGravinWorkMachine       = 2337,
	kObjectNPC062GritzaStatusQuo   = 2399,
	kObjectNpc062GritzaTalk        = 2400,
	kObjectNPC063GrundleStatusQuo  = 2410,
	kObjectNpc063GrundleTalk       = 2411,
	kObjectNPC064GrellaStatusQuo   = 2418,
	kObjectNpc064GrellaTalk        = 2420,
	kObjectSparkPuzzleLeft         = 2427,
	kObjectSparkPuzzleMiddle       = 2429,
	kObjectLavaBridge              = 2445,
	kObjectSparkPuzzleRight        = 2451,
	kObjectNpc065Talk              = 2467,
	kObjectNPC065StatusQuo         = 2469,
	kObjectFloorTrap1              = 2473,
	kObjectFloorTrap2              = 2474,
	kObjectFloorTrap3              = 2475,
	kObjectFloorTrap4              = 2476,
	kObjectFloorTrap5              = 2477,
	kObjectGlow4                   = 2478,
	kObjectBugOnTable              = 2480,
	kObjectFloorTrap6              = 2481,
	kObjectFloorTrap7              = 2482,
	kObjectFloorTrap8              = 2483,
	kObjectFloorTrap9              = 2484,
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
	kObjectLittleGirlTalk          = 2540,
	kObjectGirlStatusQuo           = 2541,
	kObjectWitchWifeStirring       = 2543,
	kObjectWitchWifeTalking        = 2546,
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
	kObjectDeadShamanStill         = 2604,
	kObjectGhost2b                 = 2634,
	kObjectJumpDown                = 2648,
	kObjectRitualLoop              = 2674,
	kObjectLine                    = 2859,
	kObjectZapPattern1             = 2876,
	kObjectZapPattern2             = 2877,
	kObjectZapPattern3             = 2878,
	kObjectBlanklPixelForWarrior   = 2884,
	kObjectPixelForHutWarrior      = 2885,
	kObjectPixelForWaterfallGhost  = 2886,
	kObjectPixelForQueztza         = 2891,
	kObjectZapPattern4             = 2892,
	kObjectZapPattern5             = 2893,
	kObjectZapPattern6             = 2894,
	kObjectZapPattern7             = 2895,
	kObjectZapPattern8             = 2896,
	kObjectZapPattern9             = 2897,
	kObjectObituary                = 2990,
	kObjectCarolsDiary             = 2992,
	kObjectTrixieSq2               = 3002,
	kObjectSimonSq2                = 3003,
	kObjectEileenTalkingWithShove  = 3011,
	kObjectStqEileenTalkWithShov   = 3016,
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
	kObjectFlagBF        = 0xBF,
	kObjectFlag1000      = 0x1000,
	kObjectFlagC000      = 0xC000,
	kObjectFlag10000     = 0x10000,
	kObjectFlag10E38     = 0x10E38,
	kObjectFlag20000     = 0x20000,
	kObjectFlag40000     = 0x40000
};

//////////////////////////////////////////////////////////////////////////
// Puzzles
//////////////////////////////////////////////////////////////////////////
enum PuzzleId {
	kPuzzleVCR = 0,
	kPuzzlePipes,
	kPuzzleTicTacToe,
	kPuzzleLock,
	kPuzzle4,
	kPuzzleWheel,                 // 5
	kPuzzleBoardSalvation,
	kPuzzleBoardYouth,
	kPuzzleBoardKeyHidesTo,
	kPuzzleWritings,
	kPuzzle11,                    // 10
	kPuzzleMorgueDoor,
	kPuzzleClock,
	kPuzzleTimeMachine,
	kPuzzleFisherman,
	kPuzzleHiveMachine,           // 15
	kPuzzleHiveControl
};

//////////////////////////////////////////////////////////////////////////
// Custom engine actions
//////////////////////////////////////////////////////////////////////////
enum AsylumAction {
	kAsylumActionShowVersion,
	kAsylumActionQuickLoad,
	kAsylumActionQuickSave,
	kAsylumActionSwitchToSarah,
	kAsylumActionSwitchToGrimwall,
	kAsylumActionSwitchToOlmec,
	kAsylumActionOpenInventory
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
#define getEncounter()   _vm->encounter()
#define getCursor()      _vm->cursor()
#define getMenu()        _vm->menu()
#define getPuzzles()     _vm->puzzles()
#define getReaction()    _vm->reaction()
#define getResource()    _vm->resource()
#define getSound()       _vm->sound()
#define getSaveLoad()    _vm->savegame()
#define getScene()       _vm->scene()
#define getScreen()      _vm->screen()
#define getScript()      _vm->script()
#define getSharedData()  _vm->data()
#define getSpecial()     _vm->special()
#define getSpeech()      _vm->speech()
#define getText()        _vm->text()
#define getVideo()       _vm->video()
#define getWorld()       _vm->scene()->worldstats()

//////////////////////////////////////////////////////////////////////////
// Sub-integer partial access macros
//////////////////////////////////////////////////////////////////////////
#if   defined SCUMM_LITTLE_ENDIAN
#define LOBYTE(d)   (*((char *)&(d)))
#define BYTE1(d)    (*((char *)&(d) + 1))   // Same as HIBYTE()
#elif defined SCUMM_BIG_ENDIAN
#define LOBYTE(d)   (*((char *)&(d) + sizeof(d) - 1))
#define BYTE1(d)    (*((char *)&(d) + sizeof(d) - 2))
#endif

//////////////////////////////////////////////////////////////////////////
// Resource ID macros
//////////////////////////////////////////////////////////////////////////
#define RESOURCE_PACK(id) (ResourcePackId)((((uint32)id) >> 16) & 0x7FFF)
#define RESOURCE_INDEX(id) (uint16)(id)

} // End of namespace Asylum

#endif // ASYLUM_SHARED_H
