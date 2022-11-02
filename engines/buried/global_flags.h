/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
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

#ifndef BURIED_GLOBAL_FLAGS_H
#define BURIED_GLOBAL_FLAGS_H

#include "common/scummsys.h"

namespace Buried {

// This is the struct that holds all the global variables for the game
// Originally, it was just a 1024 byte block of data with offsets into it

struct GlobalFlags {
	byte cgWallExploded;                // 0
	byte cgHookPresent;                 // 1
	byte cgArrowPresent;                // 2
	byte cgHammerPresent;               // 3
	byte cgSmithyStatus;                // 4
	byte cgSmithyGuard;                 // 5
	byte cgBaileyOneWayGuard;           // 6
	byte cgBaileyTwoWayGuards;          // 7
	byte cgTapestryFlag;                // 8
	byte cgBurnedLetterPresent;         // 9
	byte cgGoldCoinsPresent;            // 10
	byte cgStorageRoomVisit;            // 11
	byte bcTranslateEnabled;            // 12
	byte bcCloakingEnabled;             // 13
	byte bcLocateEnabled;               // 14
	byte myPickedUpCeramicBowl;         // 15
	byte myTPCodeWheelStatus;           // 16
	byte myTPCodeWheelLeftIndex;        // 17
	byte myTPCodeWheelRightIndex;       // 18
	byte myMCPickedUpSkull;             // 19
	byte myMCDeathGodOfferings;         // 20
	byte myWGPlacedRope;                // 21
	byte myWGRetrievedJadeBlock;        // 22
	byte myWTRetrievedLimestoneBlock;   // 23
	byte myWTCurrentBridgeStatus;       // 24
	byte myAGRetrievedEntrySkull;       // 25
	byte myAGRetrievedSpearSkull;       // 26
	byte myAGRetrievedCopperMedal;      // 27
	byte myAGRetrievedObsidianBlock;    // 28
	byte myAGHeadAStatus;               // 29
	byte myAGHeadBStatus;               // 30
	byte myAGHeadCStatus;               // 31
	byte myAGHeadDStatus;               // 32
	byte myAGHeadAStatusSkullID;        // 33
	byte myAGHeadBStatusSkullID;        // 34
	byte myAGHeadCStatusSkullID;        // 35
	byte myAGHeadDStatusSkullID;        // 36
	byte myAGTimerHeadID;               // 37
	uint32 myAGTimerStartTime;          // 38-41
	byte myDGOfferedHeart;              // 42
	byte takenEnvironCart;              // 43
	byte alRDTakenLiveCore;             // 44
	byte alRDTakenDeadCore;             // 45
	byte alNMWrongAlienPrefixCode;      // 46
	byte faKIOvenStatus;                // 47
	byte faKIPostBoxSlotA;              // 48
	byte faKIPostBoxSlotB;              // 49
	byte faKIPostBoxSlotC;              // 50
	byte faERCurrentCartridge;          // 51
	byte faERTakenRemoteControl;        // 52
	byte myMCStingerID;                 // 53
	byte myMCStingerChannelID;          // 54
	byte faStingerID;                   // 55
	byte faStingerChannelID;            // 56
	uint16 curItem;                     // ScummVM enhancement, originally unused bytes 57-58
	byte unused0;                       // 59
	uint32 cgMWCatapultData;            // 60-63
	uint32 cgMWCatapultOffset;          // 64-67
	byte cgTSTriedDoor;                 // 68
	byte cgMBCrossedMoat;               // 69
	byte cgKSSmithyEntryRead;           // 70
	byte cgKSSmithyEntryTranslated;     // 71
	byte cgBSFoundMold;                 // 72
	byte readBurnedLetter;              // 73
	byte evcapNumCaptured;              // 74
	byte evcapBaseID[12];               // 75-86
	byte unused1[3];                    // 87-89
	byte faMNEnvironDoor;               // 90
	byte faMNClockClicked;              // 91
	byte faMNBooksClicked;              // 92
	byte faMNTazClicked;                // 93
	byte faMNPongClicked;               // 94
	byte faKIBirdsBobbed;               // 95
	byte faKICoffeeSpilled;             // 96
	byte cgViewedKeepPlans;             // 97
	byte cgFoundChestPanel;             // 98
	byte cgTRFoundSword;                // 99
	byte faHeardAgentFigure;            // 100
	byte jumpBCNoInfoMessageCycle;      // 101
	byte myTPCalendarTopTranslated;     // 102
	byte myTPCalendarListTranslated;    // 103
	byte myTPTextTranslated;            // 104
	byte myMCTransDoor;                 // 105
	byte myMCTransAGOffering;           // 106
	byte myMCTransWGOffering;           // 107
	byte myMCTransWTOffering;           // 108
	byte myMCTransDGOffering;           // 109
	byte myMCTransMadeAnOffering;       // 110
	byte myWGTransDoorTop;              // 111
	byte myWGSeenLowerPassage;          // 112
	byte myWGCrossedRopeBridge;         // 113
	byte myMCViewedDeathGodDoor;        // 114
	byte myTPTransBreathOfItzamna;      // 115
	uint32 myAGHeadAOpenedTime;         // 116-119
	uint32 myAGHeadBOpenedTime;         // 120-123
	uint32 myAGHeadCOpenedTime;         // 124-127
	uint32 myAGHeadDOpenedTime;         // 128-131
	byte myAGHeadATouched;              // 132
	byte myAGHeadBTouched;              // 133
	byte myAGHeadCTouched;              // 134
	byte myAGHeadDTouched;              // 135
	byte lensFilterActivated;           // 136
	byte dsPTElevatorPresent;           // 137
	byte dsPTElevatorLeverA;            // 138
	byte dsPTElevatorLeverB;            // 139
	byte dsPTDoorLocked;                // 140
	byte dsWSPickedUpWheelAssembly;     // 141
	byte dsWSPickedUpGearAssembly;      // 142
	byte dsWSPickedUpPegs;              // 143
	byte dsWSSiegeCycleStatus;          // 144
	byte dsWSGrabbedSiegeCycle;         // 145
	byte dsPTUseElevatorControls;       // 146
	byte dsPTTransElevatorControls;     // 147
	byte dsGDTakenCoilOfRope;           // 148
	byte dsCTUnlockedDoor;              // 149
	byte dsCTViewedAgent3;              // 150
	byte dsPTViewedAgent3;              // 151
	byte dsCTRetrievedLens;             // 152
	byte dsCTTakenHeart;                // 153
	byte dsCYFiredCannon;               // 154
	byte dsCYBallistaStatus;            // 155
	byte dsCYPlacedSiegeCycle;          // 156
	byte dsCYBallistaXPos;              // 157
	byte dsCYBallistaYPos;              // 158
	byte aiHWStingerID;                 // 159
	byte aiHWStingerChannelID;          // 160
	byte aiCRStingerID;                 // 161
	byte aiCRStingerChannelID;          // 162
	byte aiDBStingerID;                 // 163
	byte aiDBStingerChannelID;          // 164
	byte aiCRGrabbedMetalBar;           // 165
	byte aiICGrabbedWaterCanister;      // 166
	byte aiOxygenTimer;                 // 167
	byte aiCRPressurized;               // 168
	byte aiCRPressurizedAttempted;      // 169
	byte aiMRPressurized;               // 170
	byte aiIceMined;                    // 171
	byte aiOxygenReserves;              // 172
	byte aiSCHeardInitialSpeech;        // 173
	byte aiSCInitialAudioChannel;       // 174
	byte aiSCDBDoorWarning;             // 175
	byte aiSCMoveCenterWarning;         // 176
	byte aiSCConversationStatus;        // 177
	byte aiHWIceDoorUnlocked;           // 178
	byte aiICWaterInFillHandle;         // 179
	byte aiICTakenWaterCanister;        // 180
	byte aiSWStingerID;                 // 181
	byte aiSWStingerChannelID;          // 182
	byte aiMRCorrectFreqSet;            // 183
	byte aiSCHeardNexusDoorComment;     // 184
	byte aiSCHeardNexusDoorCode;        // 185
	byte asInitialGuardsPass;           // 186
	byte asRBPodAStatus;                // 187
	byte asRBPodBStatus;                // 188
	byte asRBPodCStatus;                // 189
	byte asRBPodDStatus;                // 190
	byte asRBPodEStatus;                // 191
	byte asRBPodFStatus;                // 192
	byte asRBPodATakenEnvironCart;      // 193
	byte asRBPodBTakenPuzzleBox;        // 194
	byte asRBPodCTakenCodex;            // 195
	byte asRBPodDTakenSculpture;        // 196
	byte asRBPodETakenSword;            // 197
	byte asTakenEvidenceThisTrip;       // 198
	byte asDangerDoorASealed;           // 199
	byte asDoorBGuardsSeen;             // 200
	byte asAmbassadorEncounter;         // 201
	byte dsCTTriedLockedDoor;           // 202
	byte dsCTCodexTranslateAttempted;   // 203
	byte dsCTCodexFormulaeFound;        // 204
	byte dsCTCodexAtlanticusPage2;      // 205
	byte dsCTTriedElevatorControls;     // 206
	byte aiDBPlayedMomComment;          // 207
	byte aiDBPlayedFirstArthur;         // 208
	byte aiDBPlayedSecondArthur;        // 209
	byte aiDBPlayedThirdArthur;         // 210
	byte aiDBPlayedFourthArthur;        // 211
	byte aiSCPlayedNoStinger;           // 212
	byte faKITakenPostboxItem;          // 213
	byte cgMBVisited;                   // 214
	byte cgKCVisited;                   // 215
	byte cgTRVisited;                   // 216
	byte cgKSReadJournal;               // 217
	byte cgSRClickedOnLockedChest;      // 218
	byte cgSROpenedChest;               // 219
	byte dsVisitedCodexTower;           // 220
	byte dsPTRaisedPlatform;            // 221
	byte dsPTWalkedDownElevator;        // 222
	byte dsPTBeenOnBalcony;             // 223
	byte dsGDClickedOnCodexDoor;        // 224
	byte dsWSSeenCycleSketch;           // 225
	byte dsWSSeenBallistaSketch;        // 226
	byte genHadSiegeCycle;              // 227
	byte genHadDriveAssembly;           // 228
	byte genHadWheelAssembly;           // 229
	byte dsCYNeverConnectedHook;        // 230
	byte dsCYNeverShotBallista;         // 231
	byte dsCYNeverUsedCrank;            // 232
	byte dsCYNeverOpenedBalconyDoor;    // 233
	byte dsCYTranslatedCodex;           // 234
	byte dsCYTriedOpeningDoor;          // 235
	byte dsCYTriedElevator;             // 236
	byte dsCYFoundCodexes;              // 237
	byte myVisitedMainCavern;           // 238
	byte myVisitedArrowGod;             // 239
	byte myVisitedWaterGod;             // 240
	byte myVisitedWealthGod;            // 241
	byte myVisitedDeathGod;             // 242
	byte myVisitedSpecRooms;            // 243
	byte myWTSteppedOnSwings;           // 244
	byte myWTSteppedOnFarLedge;         // 245
	byte myDGOpenedPuzzleBox;           // 246
	byte myAGVisitedAltar;              // 247
	byte dsCTPlayedBallistaFalling;     // 248
	byte cgTSTriedDoorA;                // 249
	byte cgTSTriedDoorB;                // 250
	byte aiHWLastCommentPlayed;         // 251
	byte aiNXPlayedBrainComment;        // 252
	byte asRBLastStingerID;             // 253
	byte asRBStingerID;                 // 254
	byte aiICProcessedOxygen;           // 255
	byte dsCYWeebleClicked;             // 256
	byte aiICUsedMiningControls;        // 257
	byte aiSWAttemptedPresMR;           // 258
	byte aiICRefilledOxygen;            // 259
	byte aiMRUsedHarmonicsInterface;    // 260
	byte alRestoreSkipAgent3Initial;    // 261
	byte unused2[38];                   // 262-299
	byte scoreGotTranslateBioChip;      // 300
	byte scoreEnteredSpaceStation;      // 301
	byte scoreDownloadedArthur;         // 302
	byte scoreFoundSculptureDiagram;    // 303
	byte scoreEnteredKeep;              // 304
	byte scoreGotKeyFromSmithy;         // 305
	byte scoreEnteredTreasureRoom;      // 306
	byte scoreFoundSwordDiamond;        // 307
	byte scoreMadeSiegeCycle;           // 308
	byte scoreEnteredCodexTower;        // 309
	byte scoreLoggedCodexEvidence;      // 310
	byte scoreEnteredMainCavern;        // 311
	byte scoreGotWealthGodPiece;        // 312
	byte scoreGotRainGodPiece;          // 313
	byte scoreGotWarGodPiece;           // 314
	byte scoreCompletedDeathGod;        // 315
	byte scoreEliminatedAgent3;         // 316
	byte scoreTransportToKrynn;         // 317
	byte scoreGotKrynnArtifacts;        // 318
	byte scoreDefeatedIcarus;           // 319

	// This data was modified between 1.01 and 1.03, without changing
	// the saved game version field just to add the Louvre research
	// boolean. The 1.01 offsets are in parentheses.
	byte scoreResearchINNLouvreReport;  // 320 (---)
	byte scoreResearchINNHighBidder;    // 321 (320)
	byte scoreResearchINNAppeal;        // 322 (321)
	byte scoreResearchINNUpdate;        // 323 (322)
	byte scoreResearchINNJumpsuit;      // 324 (323)
	byte scoreResearchBCJumpsuit;       // 325 (324)
	byte scoreResearchMichelle;         // 326 (325)
	byte scoreResearchMichelleBkg;      // 327 (326)
	byte scoreResearchLensFilter;       // 328 (327)
	byte scoreResearchCastleFootprint;  // 329 (328)
	byte scoreResearchDaVinciFootprint; // 330 (329)
	byte scoreResearchMorphSculpture;   // 331 (330)
	byte scoreResearchEnvironCart;      // 332 (331)
	byte scoreResearchAgent3Note;       // 333 (332)
	byte scoreResearchAgent3DaVinci;    // 334 (333)
	uint16 scoreHintsTotal;             // 335-336 (334-335)
	byte unused3[53];                   // 337-389 (336-389)

	byte genJumpCastleBriefing;         // 390
	byte genJumpMayanBriefing;          // 391
	byte genJumpDaVinciBriefing;        // 392
	byte genJumpStationBriefing;        // 393
	byte unused4[106];                  // 394-499
	byte generalWalkthroughMode;        // 500
	byte unused5[11];                   // 501-511
	byte aiData[512];                   // 512-1023
};

enum {
	kAIHWStartingValue = 100,
	kAIICStartingValue = 100,
	kAIOTWalkDecrement = 2,
	kAIOTTurnDecrement = 1,
	kAIOTWaitDecrement = 1,
	kAIOTWaitTimePeriod = 10000
};

} // End of namespace Buried

#endif
