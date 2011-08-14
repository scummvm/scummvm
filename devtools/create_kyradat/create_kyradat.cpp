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
 */

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

// HACK to allow building with the SDL backend on MinGW
// see bug #1800764 "TOOLS: MinGW tools building broken"
#ifdef main
#undef main
#endif // main

#include "create_kyradat.h"

#include "search.h"
#include "pak.h"

#include "tables.h"
#include "extract.h"

#include "md5.h"

#include <string>
#include <map>
#include <algorithm>
#include <map>

enum {
	kKyraDatVersion = 79
};

const ExtractFilename extractFilenames[] = {
	// GENERIC ID MAP
	{ kIdMap, -1, true },

	// INTRO / OUTRO sequences
	{ k1ForestSeq, kTypeForestSeqData, false },
	{ k1KallakWritingSeq, kTypeRawData, false },
	{ k1KyrandiaLogoSeq, kTypeRawData, false },
	{ k1KallakMalcolmSeq, kTypeRawData, false },
	{ k1MalcolmTreeSeq, kTypeRawData, false },
	{ k1WestwoodLogoSeq, kTypeRawData, false },
	{ k1Demo1Seq, kTypeRawData, false },
	{ k1Demo2Seq, kTypeRawData, false },
	{ k1Demo3Seq, kTypeRawData, false },
	{ k1Demo4Seq, kTypeRawData, false },
	{ k1OutroReunionSeq, kTypeRawData, false },

	// INTRO / OUTRO strings
	{ k1IntroCPSStrings, kTypeStringList, false },
	{ k1IntroCOLStrings, kTypeStringList, false },
	{ k1IntroWSAStrings, kTypeStringList, false },
	{ k1IntroStrings, kTypeStringList, true },
	{ k1OutroHomeString, kTypeStringList, true },

	// INGAME strings
	{ k1ItemNames, kTypeStringList, true },
	{ k1TakenStrings, kTypeStringList, true },
	{ k1PlacedStrings, kTypeStringList, true },
	{ k1DroppedStrings, kTypeStringList, true },
	{ k1NoDropStrings, kTypeStringList, true },
	{ k1PutDownString, kTypeStringList, true },
	{ k1WaitAmuletString, kTypeStringList, true },
	{ k1BlackJewelString, kTypeStringList, true },
	{ k1PoisonGoneString, kTypeStringList, true },
	{ k1HealingTipString, kTypeStringList, true },
	{ k1ThePoisonStrings, kTypeStringList, true },
	{ k1FluteStrings, kTypeStringList, true },
	{ k1WispJewelStrings, kTypeStringList, true },
	{ k1MagicJewelStrings, kTypeStringList, true },
	{ k1FlaskFullString, kTypeStringList, true },
	{ k1FullFlaskString, kTypeStringList, true },
	{ k1VeryCleverString, kTypeStringList, true },
	{ k1NewGameString, kTypeStringList, true },

	// GUI strings table
	{ k1GUIStrings, kTypeStringList, true },
	{ k1ConfigStrings, kTypeStringList, true },

	// ROOM table/filenames
	{ k1RoomList, kTypeRoomList, false },
	{ k1RoomFilenames, kTypeStringList, false },

	// SHAPE tables
	{ k1DefaultShapes, kTypeShapeList, false },
	{ k1Healing1Shapes, kTypeShapeList, false },
	{ k1Healing2Shapes, kTypeShapeList, false },
	{ k1PoisonDeathShapes, kTypeShapeList, false },
	{ k1FluteShapes, kTypeShapeList, false },
	{ k1Winter1Shapes, kTypeShapeList, false },
	{ k1Winter2Shapes, kTypeShapeList, false },
	{ k1Winter3Shapes, kTypeShapeList, false },
	{ k1DrinkShapes, kTypeShapeList, false },
	{ k1WispShapes, kTypeShapeList, false },
	{ k1MagicAnimShapes, kTypeShapeList, false },
	{ k1BranStoneShapes, kTypeShapeList, false },

	// IMAGE filename table
	{ k1CharacterImageFilenames, kTypeStringList, false },

	// AUDIO filename table
	{ k1AudioTracks, kTypeStringList, false },
	{ k1AudioTracks2, kTypeStringList, false },
	{ k1AudioTracksIntro, kTypeStringList, false },

	// AMULET anim
	{ k1AmuleteAnimSeq, kTypeRawData, false },

	// PALETTE table
	{ k1SpecialPalette1, kTypeRawData, false },
	{ k1SpecialPalette2, kTypeRawData, false },
	{ k1SpecialPalette3, kTypeRawData, false },
	{ k1SpecialPalette4, kTypeRawData, false },
	{ k1SpecialPalette5, kTypeRawData, false },
	{ k1SpecialPalette6, kTypeRawData, false },
	{ k1SpecialPalette7, kTypeRawData, false },
	{ k1SpecialPalette8, kTypeRawData, false },
	{ k1SpecialPalette9, kTypeRawData, false },
	{ k1SpecialPalette10, kTypeRawData, false },
	{ k1SpecialPalette11, kTypeRawData, false },
	{ k1SpecialPalette12, kTypeRawData, false },
	{ k1SpecialPalette13, kTypeRawData, false },
	{ k1SpecialPalette14, kTypeRawData, false },
	{ k1SpecialPalette15, kTypeRawData, false },
	{ k1SpecialPalette16, kTypeRawData, false },
	{ k1SpecialPalette17, kTypeRawData, false },
	{ k1SpecialPalette18, kTypeRawData, false },
	{ k1SpecialPalette19, kTypeRawData, false },
	{ k1SpecialPalette20, kTypeRawData, false },
	{ k1SpecialPalette21, kTypeRawData, false },
	{ k1SpecialPalette22, kTypeRawData, false },
	{ k1SpecialPalette23, kTypeRawData, false },
	{ k1SpecialPalette24, kTypeRawData, false },
	{ k1SpecialPalette25, kTypeRawData, false },
	{ k1SpecialPalette26, kTypeRawData, false },
	{ k1SpecialPalette27, kTypeRawData, false },
	{ k1SpecialPalette28, kTypeRawData, false },
	{ k1SpecialPalette29, kTypeRawData, false },
	{ k1SpecialPalette30, kTypeRawData, false },
	{ k1SpecialPalette31, kTypeRawData, false },
	{ k1SpecialPalette32, kTypeRawData, false },
	{ k1SpecialPalette33, kTypeRawData, false },

	// CREDITS (used in FM-TOWNS and AMIGA)
	{ k1CreditsStrings, kTypeRawData, true },

	// FM-TOWNS specific
	{ k1TownsMusicFadeTable, k3TypeRaw16to8, false },
	{ k1TownsSFXwdTable, kTypeTownsWDSfxTable, false },
	{ k1TownsSFXbtTable, kTypeRawData, false },
	{ k1TownsCDATable, kTypeRawData, false },

	// PC98 specific
	{ k1PC98StoryStrings, kTypeStringList, true },
	{ k1PC98IntroSfx, kTypeRawData, false },

	// AMIGA specific
	{ k1AmigaIntroSFXTable, kTypeAmigaSfxTable, false },
	{ k1AmigaGameSFXTable, kTypeAmigaSfxTable, false },

	// HAND OF FATE

	// Sequence Player
	{ k2SeqplayPakFiles, kTypeStringList, false },
	{ k2SeqplayCredits, kTypeRawData, false },
	{ k2SeqplayCreditsSpecial, kTypeStringList, false },
	{ k2SeqplayStrings, kTypeStringList, true },
	{ k2SeqplaySfxFiles, k2TypeSoundList, false },
	{ k2SeqplayTlkFiles, k2TypeLangSoundList, true },
	{ k2SeqplaySeqData, k2TypeSeqData, false },
	{ k2SeqplayIntroTracks, kTypeStringList, false },
	{ k2SeqplayFinaleTracks, kTypeStringList, false },
	{ k2SeqplayIntroCDA, kTypeRawData, false },
	{ k2SeqplayFinaleCDA, kTypeRawData, false },
	{ k2SeqplayShapeAnimData, k2TypeShpDataV1, false },

	// Ingame
	{ k2IngamePakFiles, kTypeStringList, false },
	{ k2IngameSfxFiles, k2TypeSize10StringList, false },
	{ k2IngameSfxIndex, kTypeRawData, false },
	{ k2IngameTracks, kTypeStringList, false },
	{ k2IngameCDA, kTypeRawData, false },
	{ k2IngameTalkObjIndex, kTypeRawData, false },
	{ k2IngameTimJpStrings, kTypeStringList, false },
	{ k2IngameShapeAnimData, k2TypeShpDataV2, false },
	{ k2IngameTlkDemoStrings, kTypeStringList, true },


	// MALCOLM'S REVENGE
	{ k3MainMenuStrings, kTypeStringList, false },
	{ k3MusicFiles, k2TypeSoundList, false },
	{ k3ScoreTable, kTypeRawData, false },
	{ k3SfxFiles, k2TypeSfxList, false },
	{ k3SfxMap, k3TypeRaw16to8, false },
	{ k3ItemAnimData, k3TypeShpData, false },
	{ k3ItemMagicTable, k3TypeRaw16to8, false },
	{ k3ItemStringMap, kTypeRawData, false },

	// EYE OF THE BEHOLDER COMMON
	{ kEobBaseChargenStrings1, kTypeStringList, true },
	{ kEobBaseChargenStrings2, kTypeStringList, true },
	{ kEobBaseChargenStartLevels, kTypeRawData, false },
	{ kEobBaseChargenStatStrings, kTypeStringList, true},
	{ kEobBaseChargenRaceSexStrings, kTypeStringList, true },
	{ kEobBaseChargenClassStrings, kTypeStringList, true },
	{ kEobBaseChargenAlignmentStrings, kTypeStringList, true },
	{ kEobBaseChargenEnterGameStrings, kTypeStringList, true },
	{ kEobBaseChargenClassMinStats, k3TypeRaw16to8, false },
	{ kEobBaseChargenRaceMinStats, k3TypeRaw16to8, false },
	{ kEobBaseChargenRaceMaxStats, kLolTypeRaw16, false },

	{ kEobBaseConstModTable1, kTypeRawData, false },
	{ kEobBaseConstModTable2, kTypeRawData, false },
	{ kEobBaseConstModTable3, kTypeRawData, false },
	{ kEobBaseConstModTable4, kTypeRawData, false },
	{ kEobBaseConstModLvlIndex, kTypeRawData, false },
	{ kEobBaseConstModDiv, kTypeRawData, false },
	{ kEobBaseConstModExt, kTypeRawData, false },

	{ kEobBasePryDoorStrings, kTypeStringList, true },
	{ kEobBaseWarningStrings, kTypeStringList, true },

	{ kEobBaseItemSuffixStringsRings, kTypeStringList, true },
	{ kEobBaseItemSuffixStringsPotions, kTypeStringList, true },
	{ kEobBaseItemSuffixStringsWands, kTypeStringList, true },

	{ kEobBaseRipItemStrings, kTypeStringList, true },
	{ kEobBaseCursedString, kTypeStringList, true },
	{ kEobBaseEnchantedString, kTypeStringList, false },
	{ kEobBaseMagicObjectStrings, kTypeStringList, true },
	{ kEobBaseMagicObject5String, kTypeStringList, true },
	{ kEobBasePatternSuffix, kTypeStringList, true },
	{ kEobBasePatternGrFix1, kTypeStringList, true },
	{ kEobBasePatternGrFix2, kTypeStringList, true },
	{ kEobBaseValidateArmorString, kTypeStringList, true },
	{ kEobBaseValidateCursedString, kTypeStringList, true },
	{ kEobBaseValidateNoDropString, kTypeStringList, true },
	{ kEobBasePotionStrings, kTypeStringList, true },
	{ kEobBaseWandString, kTypeStringList, true },
	{ kEobBaseItemMisuseStrings, kTypeStringList, true },

	{ kEobBaseTakenStrings, kTypeStringList, true },
	{ kEobBasePotionEffectStrings, kTypeStringList, true },

	{ kEobBaseYesNoStrings, kTypeStringList, true },
	{ kLolEobCommonMoreStrings, kTypeStringList, true },
	{ kEobBaseNpcMaxStrings, kTypeStringList, true },
	{ kEobBaseOkStrings, kTypeStringList, true },
	{ kEobBaseNpcJoinStrings, kTypeStringList, true },
	{ kEobBaseCancelStrings, kTypeStringList, true },
	{ kEobBaseAbortStrings, kTypeStringList, true },

	{ kEobBaseMenuStringsMain, kTypeStringList, true },
	{ kEobBaseMenuStringsSaveLoad, kTypeStringList, true },
	{ kEobBaseMenuStringsOnOff, kTypeStringList, true },
	{ kEobBaseMenuStringsSpells, kTypeStringList, true },
	{ kEobBaseMenuStringsRest, kTypeStringList, true },
	{ kEobBaseMenuStringsDrop, kTypeStringList, true },
	{ kEobBaseMenuStringsExit, kTypeStringList, true },
	{ kEobBaseMenuStringsStarve, kTypeStringList, true },
	{ kEobBaseMenuStringsScribe, kTypeStringList, true },
	{ kEobBaseMenuStringsDrop2, kTypeStringList, true },
	{ kEobBaseMenuStringsHead, kTypeStringList, true },
	{ kEobBaseMenuStringsPoison, kTypeStringList, true },
	{ kEobBaseMenuStringsMgc, kTypeStringList, true },
	{ kEobBaseMenuStringsPrefs, kTypeStringList, true },
	{ kEobBaseMenuStringsRest2, kTypeStringList, true },
	{ kEobBaseMenuStringsRest3, kTypeStringList, true },
	{ kEobBaseMenuStringsRest4, kTypeStringList, true },
	{ kEobBaseMenuStringsDefeat, kTypeStringList, true },
	{ kEobBaseMenuStringsTransfer, kTypeStringList, true },
	{ kEobBaseMenuStringsSpec, kTypeStringList, true },
	{ kEobBaseMenuStringsSpellNo, kTypeStringList, false },
	{ kEobBaseMenuYesNoStrings, kTypeStringList, true },

	{ kEobBaseSpellLevelsMage, kTypeRawData, false },
	{ kEobBaseSpellLevelsCleric, kTypeRawData, false },
	{ kEobBaseNumSpellsCleric, kTypeRawData, false },
	{ kEobBaseNumSpellsWisAdj, kTypeRawData, false },
	{ kEobBaseNumSpellsPal, kTypeRawData, false },
	{ kEobBaseNumSpellsMage, kTypeRawData, false },
	
	{ kEobBaseCharGuiStringsHp, kTypeStringList, true },
	{ kEobBaseCharGuiStringsWp1, kTypeStringList, true },
	{ kEobBaseCharGuiStringsWp2, kTypeStringList, true },
	{ kEobBaseCharGuiStringsWr, kTypeStringList, true },
	{ kEobBaseCharGuiStringsSt1, kTypeStringList, true },
	{ kEobBaseCharGuiStringsSt2, kTypeStringList, true },
	{ kEobBaseCharGuiStringsIn, kTypeStringList, true },

	{ kEobBaseCharStatusStrings7, kTypeStringList, true },
	{ kEobBaseCharStatusStrings81, kTypeStringList, true },
	{ kEobBaseCharStatusStrings82, kTypeStringList, true },
	{ kEobBaseCharStatusStrings9, kTypeStringList, true },
	{ kEobBaseCharStatusStrings12, kTypeStringList, true },
	{ kEobBaseCharStatusStrings131, kTypeStringList, true },
	{ kEobBaseCharStatusStrings132, kTypeStringList, true },

	{ kEobBaseLevelGainStrings, kTypeStringList, true },
	{ kEobBaseExperienceTable0, kLolTypeRaw32, false },
	{ kEobBaseExperienceTable1, kLolTypeRaw32, false },
	{ kEobBaseExperienceTable2, kLolTypeRaw32, false },
	{ kEobBaseExperienceTable3, kLolTypeRaw32, false },
	{ kEobBaseExperienceTable4, kLolTypeRaw32, false },

	{ kEobBaseWllFlagPreset, kTypeRawData, false },
	{ kEobBaseDscShapeCoords, kLolTypeRaw16, false },
	{ kEobBaseDscDoorScaleOffs, kTypeRawData, false },
	{ kEobBaseDscDoorScaleMult1, kTypeRawData, false },
	{ kEobBaseDscDoorScaleMult2, kTypeRawData, false },
	{ kEobBaseDscDoorScaleMult3, kTypeRawData, false },
	{ kEobBaseDscDoorScaleMult4, kTypeRawData, false },
	{ kEobBaseDscDoorScaleMult5, kTypeRawData, false },
	{ kEobBaseDscDoorScaleMult6, kTypeRawData, false },
	{ kEobBaseDscDoorType5Offs, kTypeRawData, false },
	{ kEobBaseDscDoorXE, kTypeRawData, false },
	{ kEobBaseDscDoorY1, kTypeRawData, false },
	{ kEobBaseDscDoorY3, kTypeRawData, false },
	{ kEobBaseDscDoorY4, kTypeRawData, false },
	{ kEobBaseDscDoorY5, kTypeRawData, false },
	{ kEobBaseDscDoorY6, kTypeRawData, false },
	{ kEobBaseDscDoorY7, kTypeRawData, false },
	{ kEobBaseDscDoorCoordsExt, kLolTypeRaw16, false },

	{ kEobBaseDscItemPosIndex, kTypeRawData, false },
	{ kEobBaseDscItemShpX, kLolTypeRaw16, false },
	{ kEobBaseDscItemPosUnk, kTypeRawData, false },
	{ kEobBaseDscItemTileIndex, kTypeRawData, false },
	{ kEobBaseDscItemShapeMap, kTypeRawData, false },
	{ kEobBaseDscTelptrShpCoords, kTypeRawData, false },

	{ kEobBasePortalSeqData, kTypeRawData, false },
	{ kEobBaseManDef, kTypeRawData, true },
	{ kEobBaseManWord, kTypeStringList, true },
	{ kEobBaseManPrompt, kTypeStringList, true },

	{ kEobBaseDscMonsterFrmOffsTbl1, kTypeRawData, false },
	{ kEobBaseDscMonsterFrmOffsTbl2, kTypeRawData, false },

	{ kEobBaseInvSlotX, kLolTypeRaw16, false },
	{ kEobBaseInvSlotY, kTypeRawData, false },
	{ kEobBaseSlotValidationFlags, kLolTypeRaw16, false },

	{ kEobBaseProjectileWeaponTypes, kTypeRawData, false },
	{ kEobBaseWandTypes, kTypeRawData, false },

	{ kEobBaseDrawObjPosIndex, kTypeRawData, false },
	{ kEobBaseFlightObjFlipIndex, kTypeRawData, false },
	{ kEobBaseFlightObjShpMap, kTypeRawData, false },
	{ kEobBaseFlightObjSclIndex, kTypeRawData, false },

	{ kEobBaseBookNumbers, kTypeStringList, true },
	{ kEobBaseMageSpellsList, kTypeStringList, true },
	{ kEobBaseClericSpellsList, kTypeStringList, true },
	{ kEobBaseSpellNames, kTypeStringList, true },

	{ kEobBaseMagicStrings1, kTypeStringList, true },
	{ kEobBaseMagicStrings2, kTypeStringList, true },
	{ kEobBaseMagicStrings3, kTypeStringList, true },
	{ kEobBaseMagicStrings4, kTypeStringList, true },
	{ kEobBaseMagicStrings6, kTypeStringList, true },
	{ kEobBaseMagicStrings7, kTypeStringList, true },
	{ kEobBaseMagicStrings8, kTypeStringList, true },
	
	{ kEobBaseExpObjectTlMode, kTypeRawData, false },
	{ kEobBaseExpObjectTblIndex, kTypeRawData, false },
	{ kEobBaseExpObjectShpStart, kTypeRawData, false },
	{ kEobBaseExpObjectTbl1, kTypeRawData, false },
	{ kEobBaseExpObjectTbl2, kTypeRawData, false },
	{ kEobBaseExpObjectTbl3, kTypeRawData, false },
	{ kEobBaseExpObjectY, k3TypeRaw16to8, false },

	{ kEobBaseSparkDefSteps, kTypeRawData, false },
	{ kEobBaseSparkDefSubSteps, kTypeRawData, false },
	{ kEobBaseSparkDefShift, kTypeRawData, false },
	{ kEobBaseSparkDefAdd, kTypeRawData, false },
	{ kEobBaseSparkDefX, k3TypeRaw16to8, false },
	{ kEobBaseSparkDefY, kTypeRawData, false },
	{ kEobBaseSparkOfFlags1, kLolTypeRaw32, false },
	{ kEobBaseSparkOfFlags2, kLolTypeRaw32, false },
	{ kEobBaseSparkOfShift, kTypeRawData, false },
	{ kEobBaseSparkOfX, kTypeRawData, false },
	{ kEobBaseSparkOfY, kTypeRawData, false },
	{ kEobBaseSpellProperties, kTypeRawData, false },
	{ kEobBaseMagicFlightProps, kTypeRawData, false },
	{ kEobBaseTurnUndeadEffect, kTypeRawData, false },
	{ kEobBaseBurningHandsDest, kTypeRawData, false },
	{ kEobBaseConeOfColdDest1, kTypeRawData, false },
	{ kEobBaseConeOfColdDest2, kTypeRawData, false },
	{ kEobBaseConeOfColdDest3, kTypeRawData, false },
	{ kEobBaseConeOfColdDest4, kTypeRawData, false },
	{ kEobBaseConeOfColdGfxTbl, k3TypeRaw16to8, false },

	// EYE OF THE BEHOLDER I
	{ kEob1MainMenuStrings, kTypeStringList, true },
	{ kEob1BonusStrings, kTypeStringList, true },

	{ kEob1IntroFilesOpening, kTypeStringList, false },
	{ kEob1IntroFilesTower, kTypeStringList, false },
	{ kEob1IntroFilesOrb, kTypeStringList, false },
	{ kEob1IntroFilesWdEntry, kTypeStringList, false },
	{ kEob1IntroFilesKing, kTypeStringList, false },
	{ kEob1IntroFilesHands, kTypeStringList, false },
	{ kEob1IntroFilesWdExit, kTypeStringList, false },
	{ kEob1IntroFilesTunnel, kTypeStringList, false },
	{ kEob1IntroOpeningFrmDelay, k3TypeRaw16to8, false },
	{ kEob1IntroWdEncodeX, kTypeRawData, false },
	{ kEob1IntroWdEncodeY, kTypeRawData, false },
	{ kEob1IntroWdEncodeWH, kTypeRawData, false },
	{ kEob1IntroWdDsX, kLolTypeRaw16, false },
	{ kEob1IntroWdDsY, kTypeRawData, false },
	{ kEob1IntroTvlX1, kTypeRawData, false },
	{ kEob1IntroTvlY1, kTypeRawData, false },
	{ kEob1IntroTvlX2, kTypeRawData, false },
	{ kEob1IntroTvlY2, kTypeRawData, false },
	{ kEob1IntroTvlW, kTypeRawData, false },
	{ kEob1IntroTvlH, kTypeRawData, false },

	{ kEob1DoorShapeDefs, kTypeRawData, false },
	{ kEob1DoorSwitchShapeDefs, kTypeRawData, false },
	{ kEob1DoorSwitchCoords, kTypeRawData, false },
	{ kEob1MonsterProperties, kTypeRawData, false },
	{ kEob1EnemyMageSpellList, kTypeRawData, false },
	{ kEob1EnemyMageSfx, kTypeRawData, false },
	{ kEob1BeholderSpellList, kTypeRawData, false },
	{ kEob1BeholderSfx, kTypeRawData, false },
	{ kEob1TurnUndeadString, kTypeStringList, true },

	{ kEob1NpcShpData, kTypeRawData, false },
	{ kEob1NpcSubShpIndex1, kTypeRawData, false },
	{ kEob1NpcSubShpIndex2, kTypeRawData, false },
	{ kEob1NpcSubShpY, kTypeRawData, false },
	{ kEob1Npc0Strings, kTypeStringList, true },
	{ kEob1Npc11Strings, kTypeStringList, true },
	{ kEob1Npc12Strings, kTypeStringList, true },
	{ kEob1Npc21Strings, kTypeStringList, true },
	{ kEob1Npc22Strings, kTypeStringList, true },
	{ kEob1Npc31Strings, kTypeStringList, true },
	{ kEob1Npc32Strings, kTypeStringList, true },
	{ kEob1Npc4Strings, kTypeStringList, true },
	{ kEob1Npc5Strings, kTypeStringList, true },
	{ kEob1Npc6Strings, kTypeStringList, true },
	{ kEob1Npc7Strings, kTypeStringList, true },

	// EYE OF THE BEHOLDER II
	{ kEob2MainMenuStrings, kTypeStringList, true },

	{ kEob2IntroStrings, k2TypeSfxList, true },
	{ kEob2IntroCPSFiles, kTypeStringList, true },
	{ kEob2IntroSeqData00, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData01, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData02, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData03, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData04, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData05, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData06, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData07, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData08, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData09, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData10, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData11, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData12, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData13, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData14, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData15, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData16, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData17, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData18, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData19, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData20, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData21, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData22, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData23, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData24, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData25, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData26, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData27, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData28, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData29, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData30, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData31, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData32, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData33, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData34, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData35, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData36, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData37, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData38, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData39, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData40, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData41, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData42, kEob2TypeSeqData, false },
 	{ kEob2IntroSeqData43, kEob2TypeSeqData, false },
	{ kEob2IntroShapes00, kEob2TypeShapeData, false },
 	{ kEob2IntroShapes01, kEob2TypeShapeData, false },
 	{ kEob2IntroShapes04, kEob2TypeShapeData, false },
 	{ kEob2IntroShapes07, kEob2TypeShapeData, false },

	{ kEob2FinaleStrings, k2TypeSfxList, true },
	{ kEob2CreditsData, kTypeRawData, true },
	{ kEob2FinaleCPSFiles, kTypeStringList, true },
	{ kEob2FinaleSeqData00, kEob2TypeSeqData, false },
 	{ kEob2FinaleSeqData01, kEob2TypeSeqData, false },
 	{ kEob2FinaleSeqData02, kEob2TypeSeqData, false },
 	{ kEob2FinaleSeqData03, kEob2TypeSeqData, false },
 	{ kEob2FinaleSeqData04, kEob2TypeSeqData, false },
 	{ kEob2FinaleSeqData05, kEob2TypeSeqData, false },
 	{ kEob2FinaleSeqData06, kEob2TypeSeqData, false },
 	{ kEob2FinaleSeqData07, kEob2TypeSeqData, false },
 	{ kEob2FinaleSeqData08, kEob2TypeSeqData, false },
 	{ kEob2FinaleSeqData09, kEob2TypeSeqData, false },
 	{ kEob2FinaleSeqData10, kEob2TypeSeqData, false },
 	{ kEob2FinaleSeqData11, kEob2TypeSeqData, false },
 	{ kEob2FinaleSeqData12, kEob2TypeSeqData, false },
 	{ kEob2FinaleSeqData13, kEob2TypeSeqData, false },
 	{ kEob2FinaleSeqData14, kEob2TypeSeqData, false },
 	{ kEob2FinaleSeqData15, kEob2TypeSeqData, false },
 	{ kEob2FinaleSeqData16, kEob2TypeSeqData, false },
 	{ kEob2FinaleSeqData17, kEob2TypeSeqData, false },
 	{ kEob2FinaleSeqData18, kEob2TypeSeqData, false },
 	{ kEob2FinaleSeqData19, kEob2TypeSeqData, false },
 	{ kEob2FinaleSeqData20, kEob2TypeSeqData, false },
	{ kEob2FinaleShapes00, kEob2TypeShapeData, false },
 	{ kEob2FinaleShapes03, kEob2TypeShapeData, false },
 	{ kEob2FinaleShapes07, kEob2TypeShapeData, false },
 	{ kEob2FinaleShapes09, kEob2TypeShapeData, false },
 	{ kEob2FinaleShapes10, kEob2TypeShapeData, false },
	{ kEob2NpcShapeData, kTypeRawData, false },
	{ kEobBaseClassModifierFlags, kTypeRawData, false },
	{ kEobBaseMonsterStepTable01, kTypeRawData, false },
	{ kEobBaseMonsterStepTable02, kTypeRawData, false },
	{ kEobBaseMonsterStepTable1, kTypeRawData, false },
	{ kEobBaseMonsterStepTable2, k3TypeRaw16to8, false },
	{ kEobBaseMonsterStepTable3, k3TypeRaw16to8, false },
	{ kEobBaseMonsterCloseAttPosTable1, kTypeRawData, false },
	{ kEobBaseMonsterCloseAttPosTable21, kTypeRawData, false },
	{ kEobBaseMonsterCloseAttPosTable22, kTypeRawData, false },
	{ kEobBaseMonsterCloseAttUnkTable, kTypeRawData, false },
	{ kEobBaseMonsterCloseAttChkTable1, kTypeRawData, false },
	{ kEobBaseMonsterCloseAttChkTable2, kTypeRawData, false },
	{ kEobBaseMonsterCloseAttDstTable1, kTypeRawData, false },
	{ kEobBaseMonsterCloseAttDstTable2, kTypeRawData, false },
	{ kEobBaseMonsterProximityTable, kTypeRawData, false },
	{ kEobBaseFindBlockMonstersTable, kTypeRawData, false },
	{ kEobBaseMonsterDirChangeTable, kTypeRawData, false },
	{ kEobBaseMonsterDistAttStrings, kTypeStringList, true },
	{ kEobBaseEncodeMonsterDefs, kLolTypeRaw16, false },
	{ kEobBaseNpcPresets, kEobTypeNpcData, false },
	{ kEob2Npc1Strings, kTypeStringList, true },
	{ kEob2Npc2Strings, kTypeStringList, true },
	{ kEob2MonsterDustStrings, kTypeStringList, true },
	{ kEob2DranFoolsStrings, kTypeStringList, true },
	{ kEob2HornStrings, kTypeStringList, true },
	{ kEob2HornSounds, kTypeRawData, false },
	
	// LANDS OF LORE

	// Ingame
	{ kLolIngamePakFiles, kTypeStringList, false },

	{ kLolCharacterDefs, kLolTypeCharData, true },
	{ kLolIngameSfxFiles, k2TypeSfxList, false },
	{ kLolIngameSfxIndex, kTypeRawData, false },
	{ kLolMusicTrackMap, kTypeRawData, false },
	{ kLolIngameGMSfxIndex, kTypeRawData, false },
	{ kLolIngameMT32SfxIndex, kTypeRawData, false },
	{ kLolIngamePcSpkSfxIndex, kTypeRawData, false },
	{ kLolSpellProperties, kLolTypeSpellData, false },
	{ kLolGameShapeMap, kTypeRawData, false },
	{ kLolSceneItemOffs, kTypeRawData, false },
	{ kLolCharInvIndex, k3TypeRaw16to8, false },
	{ kLolCharInvDefs, kTypeRawData, false },
	{ kLolCharDefsMan, kLolTypeRaw16, false },
	{ kLolCharDefsWoman, kLolTypeRaw16, false },
	{ kLolCharDefsKieran, kLolTypeRaw16, false },
	{ kLolCharDefsAkshel, kLolTypeRaw16, false },
	{ kLolExpRequirements, kLolTypeRaw32, false },
	{ kLolMonsterModifiers, kLolTypeRaw16, false },
	{ kLolMonsterShiftOffsets, kTypeRawData, false },
	{ kLolMonsterDirFlags, kTypeRawData, false },
	{ kLolMonsterScaleY, kTypeRawData, false },
	{ kLolMonsterScaleX, kTypeRawData, false },
	{ kLolMonsterScaleWH, kLolTypeRaw16, false },
	{ kLolFlyingObjectShp, kLolTypeFlightShpData, false },
	{ kLolInventoryDesc, kLolTypeRaw16, false },
	{ kLolLevelShpList, kTypeStringList, false },
	{ kLolLevelDatList, kTypeStringList, false },
	{ kLolCompassDefs, kLolTypeCompassData, false },
	{ kLolItemPrices, kLolTypeRaw16, false },
	{ kLolStashSetup, kTypeRawData, false },

	{ kLolDscWalls, kTypeRawData, false },
	{ kLolEobCommonDscShapeIndex, kTypeRawData, false },
	{ kLolDscOvlMap, kTypeRawData, false },
	{ kLolDscScaleWidthData, kLolTypeRaw16, false },
	{ kLolDscScaleHeightData, kLolTypeRaw16, false },
	{ kLolEobCommonDscX, kLolTypeRaw16, false },
	{ kLolDscY, kTypeRawData, false },
	{ kLolEobCommonDscTileIndex, kTypeRawData, false },
	{ kLolEobCommonDscUnk2, kTypeRawData, false },
	{ kLolEobCommonDscDoorShapeIndex, kTypeRawData, false },
	{ kLolEobCommonDscDimData1, kTypeRawData, false },
	{ kLolEobCommonDscDimData2, kTypeRawData, false },
	{ kLolEobCommonDscBlockMap, kTypeRawData, false },
	{ kLolEobCommonDscDimMap, kTypeRawData, false },
	{ kLolDscDoorScale, kLolTypeRaw16, false },
	{ kLolDscOvlIndex, k3TypeRaw16to8, false },
	{ kLolEobCommonDscBlockIndex, kTypeRawData, false },
	{ kLolDscDoor4, kLolTypeRaw16, false },
	{ kLolEobCommonDscDoorY2, kTypeRawData, false },
	{ kLolDscDoorX, kLolTypeRaw16, false },
	{ kLolDscDoorY, kLolTypeRaw16, false },

	{ kLolScrollXTop, k3TypeRaw16to8, false },
	{ kLolScrollYTop, k3TypeRaw16to8, false },
	{ kLolScrollXBottom, k3TypeRaw16to8, false },
	{ kLolScrollYBottom, k3TypeRaw16to8, false },

	{ kLolButtonDefs, kLolTypeButtonDef, false },
	{ kLolButtonList1, kLolTypeRaw16, false },
	{ kLolButtonList2, kLolTypeRaw16, false },
	{ kLolButtonList3, kLolTypeRaw16, false },
	{ kLolButtonList4, kLolTypeRaw16, false },
	{ kLolButtonList5, kLolTypeRaw16, false },
	{ kLolButtonList6, kLolTypeRaw16, false },
	{ kLolButtonList7, kLolTypeRaw16, false },
	{ kLolButtonList8, kLolTypeRaw16, false },

	{ kLolLegendData, kTypeRawData, false },
	{ kLolMapCursorOvl, kTypeRawData, false },
	{ kLolMapStringId, kLolTypeRaw16, false },

	{ kLolSpellbookAnim, k3TypeRaw16to8, false },
	{ kLolSpellbookCoords, k3TypeRaw16to8, false },
	{ kLolHealShapeFrames, kTypeRawData, false },
	{ kLolLightningDefs, kTypeRawData, false },
	{ kLolFireballCoords, kLolTypeRaw16, false },

	{ kLolCredits, kTypeRawData, false },

	{ kLolHistory, kTypeRawData, false },

	{ -1, 0, 0 }
};

const ExtractFilename *getFilenameDesc(const int id) {
	for (const ExtractFilename *i = extractFilenames; i->id != -1; ++i) {
		if (i->id == id)
			return i;
	}
	return 0;
}

bool isLangSpecific(const int id) {
	const ExtractFilename *desc = getFilenameDesc(id);
	if (!desc)
		return false;
	return desc->langSpecific;
}

// misc tables

const TypeTable gameTable[] = {
	{ kKyra1, 0 },
	{ kKyra2, 1 },
	{ kKyra3, 2 },
	{ kEob1, 3 },
	{ kEob2, 4 },
	{ kLol, 5 },
	{ -1, -1 }
};

byte getGameID(int game) {
	return std::find(gameTable, ARRAYEND(gameTable) - 1, game)->value;
}

const TypeTable languageTable[] = {
	{ UNK_LANG, 0 },
	{ EN_ANY, 1 },
	{ FR_FRA, 2 },
	{ DE_DEU, 3 },
	{ ES_ESP, 4 },
	{ IT_ITA, 5 },
	{ JA_JPN, 6 },
	{ RU_RUS, 7 },
	{ -1, -1 }
};

byte getLanguageID(int lang) {
	return std::find(languageTable, ARRAYEND(languageTable) - 1, lang)->value;
}

const TypeTable platformTable[] = {
	{ kPlatformPC, 0 },
	{ kPlatformAmiga, 1 },
	{ kPlatformFMTowns, 2 },
	{ kPlatformPC98, 3 },
	{ kPlatformMacintosh, 4 },
	{ -1, -1 }
};

byte getPlatformID(int platform) {
	return std::find(platformTable, ARRAYEND(platformTable) - 1, platform)->value;
}

const TypeTable specialTable[] = {
	{ kNoSpecial, 0 },
	{ kTalkieVersion, 1 },
	{ kDemoVersion, 2 },
	{ kTalkieDemoVersion, 3 },
	{ kOldFloppy, 4 },
	{ -1, -1 }
};

byte getSpecialID(int special) {
	return std::find(specialTable, ARRAYEND(specialTable) - 1, special)->value;
}

// filename processing

uint32 getFilename(const ExtractInformation *info, const int id) {
	const ExtractFilename *fDesc = getFilenameDesc(id);

	if (!fDesc)
		return 0;

	// GAME, PLATFORM, SPECIAL, ID, LANG
	return ((getGameID(info->game) & 0xF) << 24) |
	       ((getPlatformID(info->platform) & 0xF) << 20) |
	       ((getSpecialID(info->special) & 0xF) << 16) |
	       ((id & 0xFFF) << 4) |
	       ((getLanguageID(fDesc->langSpecific ? info->lang : UNK_LANG) & 0xF) << 0);
}

// TODO: Get rid of this
bool getFilename(char *dstFilename, const ExtractInformation *info, const int id) {
	sprintf(dstFilename, "%08X", getFilename(info, id));
	return true;
}

// index generation

typedef uint16 GameDef;

GameDef createGameDef(const ExtractInformation *eI) {
	return ((getGameID(eI->game) & 0xF) << 12) |
	       ((getPlatformID(eI->platform) & 0xF) << 8) |
	       ((getSpecialID(eI->special) & 0xF) << 4) |
	       ((getLanguageID(eI->lang) & 0xF) << 0);
}

struct Index {
	Index() : version(0), includedGames(0), gameList() {}

	uint32 version;
	uint32 includedGames;

	typedef std::list<GameDef> GameList;
	GameList gameList;
};

Index parseIndex(const uint8 *data, uint32 size) {
	Index result;

	if (size < 8)
		return result;

	result.version = READ_BE_UINT32(data); data += 4;
	result.includedGames = READ_BE_UINT32(data); data += 4;

	if (result.includedGames * 2 + 8 != size) {
		result.version = result.includedGames = 0;
		return result;
	}

	for (uint32 i = 0; i < result.includedGames; ++i) {
		GameDef game = READ_BE_UINT16(data); data += 2;
		result.gameList.push_back(game);
	}

	return result;
}

bool updateIndex(PAKFile &out, const ExtractInformation *eI) {
	uint32 size = 0;
	const uint8 *data = out.getFileData("INDEX", &size);

	Index index;
	if (data)
		index = parseIndex(data, size);

	GameDef gameDef = createGameDef(eI);
	if (index.version == kKyraDatVersion) {
		if (std::find(index.gameList.begin(), index.gameList.end(), gameDef) == index.gameList.end()) {
			++index.includedGames;
			index.gameList.push_back(gameDef);
		} else {
			// Already included in the game list, thus we do not need any further processing here.
			return true;
		}
	} else {
		index.version = kKyraDatVersion;
		index.includedGames = 1;
		index.gameList.push_back(gameDef);
	}

	const uint32 indexBufferSize = 8 + index.includedGames * 2;
	uint8 *indexBuffer = new uint8[indexBufferSize];
	assert(indexBuffer);
	uint8 *dst = indexBuffer;
	WRITE_BE_UINT32(dst, index.version); dst += 4;
	WRITE_BE_UINT32(dst, index.includedGames); dst += 4;
	for (Index::GameList::const_iterator i = index.gameList.begin(); i != index.gameList.end(); ++i) {
		WRITE_BE_UINT16(dst, *i); dst += 2;
	}

	out.removeFile("INDEX");
	if (!out.addFile("INDEX", indexBuffer, indexBufferSize)) {
		fprintf(stderr, "ERROR: couldn't update kyra.dat INDEX\n");
		delete[] indexBuffer;
		return false;
	}

	return true;
}

bool checkIndex(PAKFile &file) {
	uint32 size = 0;
	const uint8 *data = file.getFileData("INDEX", &size);
	if (!data)
		return false;

	Index index = parseIndex(data, size);

	if (index.version != kKyraDatVersion)
		return false;
	if (index.includedGames * 2 + 8 != size)
		return false;

	return true;
}

// main processing

void printHelp(const char *f) {
	printf("Usage:\n");
	printf("%s output inputfiles ...\n", f);
}

bool process(PAKFile &out, const Game *g, const byte *data, const uint32 size);
const Game *findGame(const byte *buffer, const uint32 size);

typedef std::map<std::string, std::string> MD5Map;
MD5Map createMD5Sums(int files, const char * const *filenames);

struct File {
	File() : data(0), size(0) {}
	File(uint8 *d, uint32 s) : data(d), size(s) {}

	uint8 *data;
	uint32 size;
};
typedef std::map<const Game *, File> GameMap;
GameMap createGameMap(const MD5Map &map);

int main(int argc, char *argv[]) {
	if (argc < 3) {
		printHelp(argv[0]);
		return -1;
	}

	// Special case for developer mode of this tool:
	// With "--create filename offset size" the tool will output
	// a search entry for the specifed data in the specified file.
	if (!strcmp(argv[1], "--create")) {
		if (argc < 5) {
			printf("Developer usage: %s --create input_file hex_offset hex_size\n", argv[0]);
			return -1;
		}

		uint32 offset, size;
		sscanf(argv[3], "%x", &offset);
		sscanf(argv[4], "%x", &size);

		FILE *input = fopen(argv[2], "rb");
		if (!input)
			error("Couldn't open file '%s'", argv[2]);

		byte *buffer = new byte[size];
		fseek(input, offset, SEEK_SET);
		if (fread(buffer, 1, size, input) != size) {
			delete[] buffer;
			error("Couldn't read from file '%s'", argv[2]);
		}

		fclose(input);

		SearchData d = SearchCreator::create(buffer, size);
		delete[] buffer;

		printf("{ 0x%.08X, 0x%.08X, { {", d.size, d.byteSum);
		for (int j = 0; j < 16; ++j) {
			printf(" 0x%.2X", d.hash.digest[j]);
			if (j != 15)
				printf(",");
			else
				printf(" } } }\n");
		}

		return 0;
	}

	PAKFile out;
	out.loadFile(argv[1], false);

	// When the output file is no valid kyra.dat file, we will delete
	// all the output.
	if (!checkIndex(out))
		out.clearFile();

	MD5Map inputFiles = createMD5Sums(argc - 2, &argv[2]);

	GameMap games = createGameMap(inputFiles);

	// Check for unused input files
	MD5Map unusedFiles = inputFiles;
	for (GameMap::const_iterator i = games.begin(); i != games.end(); ++i) {
		unusedFiles.erase(i->first->md5[0]);
		if (i->first->md5[1])
			unusedFiles.erase(i->first->md5[1]);
	}

	for (MD5Map::const_iterator i = unusedFiles.begin(); i != unusedFiles.end(); ++i)
		printf("Input file '%s' with md5 sum '%s' is not known.\n", i->second.c_str(), i->first.c_str());

	unusedFiles.clear();

	// Short circuit, in case no games are found.
	if (games.empty()) {
		printf("No games found. Exiting prematurely\n");
		return -1;
	}

	// Process all games found
	for (GameMap::const_iterator i = games.begin(); i != games.end(); ++i) {
		MD5Map::const_iterator f1 = inputFiles.find(i->first->md5[0]);
		MD5Map::const_iterator f2 = inputFiles.end();
		if (i->first->md5[1])
			f2 = inputFiles.find(i->first->md5[1]);

		if (f2 != inputFiles.end())
			printf("Processing files '%s' and '%s'...\n", f1->second.c_str(), f2->second.c_str());
		else
			printf("Processing file '%s'...\n", f1->second.c_str());

		if (!process(out, i->first, i->second.data, i->second.size))
			printf("FAILED\n");
		else
			printf("OK\n");
	}

	// Free up memory
	for (GameMap::iterator i = games.begin(); i != games.end(); ++i)
		delete[] i->second.data;
	games.clear();
	inputFiles.clear();

	if (!out.saveFile(argv[1]))
	error("couldn't save changes to '%s'", argv[1]);

	uint8 digest[16];
	if (!md5_file(argv[1], digest, 0))
		error("couldn't calc. md5 for file '%s'", argv[1]);
	FILE *f = fopen(argv[1], "ab");
	if (!f)
		error("couldn't open file '%s'", argv[1]);
	if (fwrite(digest, 1, 16, f) != 16)
		error("couldn't write md5sum to file '%s'", argv[1]);
	fclose(f);

	return 0;
}

MD5Map createMD5Sums(int files, const char * const *filenames) {
	MD5Map result;

	while (files--) {
		const char *inputFile = *filenames++;
		FILE *input = fopen(inputFile, "rb");

		uint32 size = fileSize(input);
		fseek(input, 0, SEEK_SET);

		byte *buffer = new uint8[size];
		assert(buffer);

		if (fread(buffer, 1, size, input) != size) {
			warning("couldn't read from file '%s', skipping it", inputFile);
			delete[] buffer;
			fclose(input);
			continue;
		}
		fclose(input);

		md5_context ctx;
		uint8 digest[16];
		char md5Str[33];

		md5_starts(&ctx);
		md5_update(&ctx, buffer, size);
		md5_finish(&ctx, digest);

		for (int j = 0; j < 16; ++j)
			sprintf(md5Str + j*2, "%02x", (int)digest[j]);

		delete[] buffer;

		result[md5Str] = inputFile;
	}

	return result;
}

GameMap createGameMap(const MD5Map &map) {
	GameMap result;

	for (const Game * const *g = gameDescs; *g != 0; ++g) {
		for (const Game *sub = *g; sub->game != -1; ++sub) {
			MD5Map::const_iterator file1 = map.find(sub->md5[0]);
			if (file1 == map.end())
				continue;

			MD5Map::const_iterator file2 = map.end();
			if (sub->md5[1] != 0) {
				file2 = map.find(sub->md5[1]);
				if (file2 == map.end())
					continue;
			}

			FILE *f1 = fopen(file1->second.c_str(), "rb");
			FILE *f2 = 0;

			if (file2 != map.end())
				f2 = fopen(file2->second.c_str(), "rb");

			uint32 file1Size = fileSize(f1);
			uint32 file2Size = 0;
			if (f2)
				file2Size = fileSize(f2);

			uint8 *buffer = new uint8[file1Size + file2Size];
			assert(buffer);

			fread(buffer, 1, file1Size, f1);
			if (f2)
				fread(buffer + file1Size, 1, file2Size, f2);

			fclose(f1);
			if (f2)
				fclose(f2);

			result[sub] = File(buffer, file1Size + file2Size);
		}
	}

	return result;
}

const char *getIdString(const int id) {
	switch (id) {
	case k1ForestSeq:
		return "k1ForestSeq";
	case k1KallakWritingSeq:
		return "k1KallakWritingSeq";
	case k1KyrandiaLogoSeq:
		return "k1KyrandiaLogoSeq";
	case k1KallakMalcolmSeq:
		return "k1KallakMalcolmSeq";
	case k1MalcolmTreeSeq:
		return "k1MalcolmTreeSeq";
	case k1WestwoodLogoSeq:
		return "k1WestwoodLogoSeq";
	case k1Demo1Seq:
		return "k1Demo1Seq";
	case k1Demo2Seq:
		return "k1Demo2Seq";
	case k1Demo3Seq:
		return "k1Demo3Seq";
	case k1Demo4Seq:
		return "k1Demo4Seq";
	case k1AmuleteAnimSeq:
		return "k1AmuleteAnimSeq";
	case k1OutroReunionSeq:
		return "k1OutroReunionSeq";
	case k1IntroCPSStrings:
		return "k1IntroCPSStrings";
	case k1IntroCOLStrings:
		return "k1IntroCOLStrings";
	case k1IntroWSAStrings:
		return "k1IntroWSAStrings";
	case k1IntroStrings:
		return "k1IntroStrings";
	case k1OutroHomeString:
		return "k1OutroHomeString";
	case k1RoomFilenames:
		return "k1RoomFilenames";
	case k1RoomList:
		return "k1RoomList";
	case k1CharacterImageFilenames:
		return "k1CharacterImageFilenames";
	case k1AudioTracks:
		return "k1AudioTracks";
	case k1AudioTracks2:
		return "k1AudioTracks2";
	case k1AudioTracksIntro:
		return "k1AudioTracksIntro";
	case k1ItemNames:
		return "k1ItemNames";
	case k1TakenStrings:
		return "k1TakenStrings";
	case k1PlacedStrings:
		return "k1PlacedStrings";
	case k1DroppedStrings:
		return "k1DroppedStrings";
	case k1NoDropStrings:
		return "k1NoDropStrings";
	case k1PutDownString:
		return "k1PutDownString";
	case k1WaitAmuletString:
		return "k1WaitAmuletString";
	case k1BlackJewelString:
		return "k1BlackJewelString";
	case k1PoisonGoneString:
		return "k1PoisonGoneString";
	case k1HealingTipString:
		return "k1HealingTipString";
	case k1WispJewelStrings:
		return "k1WispJewelStrings";
	case k1MagicJewelStrings:
		return "k1MagicJewelStrings";
	case k1ThePoisonStrings:
		return "k1ThePoisonStrings";
	case k1FluteStrings:
		return "k1FluteStrings";
	case k1FlaskFullString:
		return "k1FlaskFullString";
	case k1FullFlaskString:
		return "k1FullFlaskString";
	case k1VeryCleverString:
		return "k1VeryCleverString";
	case k1NewGameString:
		return "k1NewGameString";
	case k1DefaultShapes:
		return "k1DefaultShapes";
	case k1Healing1Shapes:
		return "k1Healing1Shapes";
	case k1Healing2Shapes:
		return "k1Healing2Shapes";
	case k1PoisonDeathShapes:
		return "k1PoisonDeathShapes";
	case k1FluteShapes:
		return "k1FluteShapes";
	case k1Winter1Shapes:
		return "k1Winter1Shapes";
	case k1Winter2Shapes:
		return "k1Winter2Shapes";
	case k1Winter3Shapes:
		return "k1Winter3Shapes";
	case k1DrinkShapes:
		return "k1DrinkShapes";
	case k1WispShapes:
		return "k1WispShapes";
	case k1MagicAnimShapes:
		return "k1MagicAnimShapes";
	case k1BranStoneShapes:
		return "k1BranStoneShapes";
	case k1SpecialPalette1:
		return "k1SpecialPalette1";
	case k1SpecialPalette2:
		return "k1SpecialPalette2";
	case k1SpecialPalette3:
		return "k1SpecialPalette3";
	case k1SpecialPalette4:
		return "k1SpecialPalette4";
	case k1SpecialPalette5:
		return "k1SpecialPalette5";
	case k1SpecialPalette6:
		return "k1SpecialPalette6";
	case k1SpecialPalette7:
		return "k1SpecialPalette7";
	case k1SpecialPalette8:
		return "k1SpecialPalette8";
	case k1SpecialPalette9:
		return "k1SpecialPalette9";
	case k1SpecialPalette10:
		return "k1SpecialPalette10";
	case k1SpecialPalette11:
		return "k1SpecialPalette11";
	case k1SpecialPalette12:
		return "k1SpecialPalette12";
	case k1SpecialPalette13:
		return "k1SpecialPalette13";
	case k1SpecialPalette14:
		return "k1SpecialPalette14";
	case k1SpecialPalette15:
		return "k1SpecialPalette15";
	case k1SpecialPalette16:
		return "k1SpecialPalette16";
	case k1SpecialPalette17:
		return "k1SpecialPalette17";
	case k1SpecialPalette18:
		return "k1SpecialPalette18";
	case k1SpecialPalette19:
		return "k1SpecialPalette19";
	case k1SpecialPalette20:
		return "k1SpecialPalette20";
	case k1SpecialPalette21:
		return "k1SpecialPalette21";
	case k1SpecialPalette22:
		return "k1SpecialPalette22";
	case k1SpecialPalette23:
		return "k1SpecialPalette23";
	case k1SpecialPalette24:
		return "k1SpecialPalette24";
	case k1SpecialPalette25:
		return "k1SpecialPalette25";
	case k1SpecialPalette26:
		return "k1SpecialPalette26";
	case k1SpecialPalette27:
		return "k1SpecialPalette27";
	case k1SpecialPalette28:
		return "k1SpecialPalette28";
	case k1SpecialPalette29:
		return "k1SpecialPalette29";
	case k1SpecialPalette30:
		return "k1SpecialPalette30";
	case k1SpecialPalette31:
		return "k1SpecialPalette31";
	case k1SpecialPalette32:
		return "k1SpecialPalette32";
	case k1SpecialPalette33:
		return "k1SpecialPalette33";
	case k1GUIStrings:
		return "k1GUIStrings";
	case k1ConfigStrings:
		return "k1ConfigStrings";
	case k1TownsMusicFadeTable:
		return "k1TownsMusicFadeTable";
	case k1TownsSFXwdTable:
		return "k1TownsSFXwdTable";
	case k1TownsSFXbtTable:
		return "k1TownsSFXbtTable";
	case k1TownsCDATable:
		return "k1TownsCDATable";
	case k1PC98StoryStrings:
		return "k1PC98StoryStrings";
	case k1PC98IntroSfx:
		return "k1PC98IntroSfx";
	case k1CreditsStrings:
		return "k1CreditsStrings";
	case k1AmigaIntroSFXTable:
		return "k1AmigaIntroSFXTable";
	case k1AmigaGameSFXTable:
		return "k1AmigaGameSFXTable";
	case k2SeqplayPakFiles:
		return "k2SeqplayPakFiles";
	case k2SeqplayStrings:
		return "k2SeqplayStrings";
	case k2SeqplaySfxFiles:
		return "k2SeqplaySfxFiles";
	case k2SeqplayTlkFiles:
		return "k2SeqplayTlkFiles";
	case k2SeqplaySeqData:
		return "k2SeqplaySeqData";
	case k2SeqplayCredits:
		return "k2SeqplayCredits";
	case k2SeqplayCreditsSpecial:
		return "k2SeqplayCreditsSpecial";
	case k2SeqplayIntroTracks:
		return "k2SeqplayIntroTracks";
	case k2SeqplayFinaleTracks:
		return "k2SeqplayFinaleTracks";
	case k2SeqplayIntroCDA:
		return "k2SeqplayIntroCDA";
	case k2SeqplayFinaleCDA:
		return "k2SeqplayFinaleCDA";
	case k2SeqplayShapeAnimData:
		return "k2SeqplayShapeAnimData";
	case k2IngamePakFiles:
		return "k2IngamePakFiles";
	case k2IngameSfxFiles:
		return "k2IngameSfxFiles";
	case k2IngameSfxIndex:
		return "k2IngameSfxIndex";
	case k2IngameTracks:
		return "k2IngameTracks";
	case k2IngameCDA:
		return "k2IngameCDA";
	case k2IngameTalkObjIndex:
		return "k2IngameTalkObjIndex";
	case k2IngameTimJpStrings:
		return "k2IngameTimJpStrings";
	case k2IngameShapeAnimData:
		return "k2IngameShapeAnimData";
	case k2IngameTlkDemoStrings:
		return "k2IngameTlkDemoStrings";
	case k3MainMenuStrings:
		return "k3MainMenuStrings";
	case k3MusicFiles:
		return "k3MusicFiles";
	case k3ScoreTable:
		return "k3ScoreTable";
	case k3SfxFiles:
		return "k3SfxFiles";
	case k3SfxMap:
		return "k3SfxMap";
	case k3ItemAnimData:
		return "k3ItemAnimData";
	case k3ItemMagicTable:
		return "k3ItemMagicTable";
	case k3ItemStringMap:
		return "k3ItemStringMap";
	case kEobBaseChargenStrings1:
		return "kEobBaseChargenStrings1";
	case kEobBaseChargenStrings2:
		return "kEobBaseChargenStrings2";
	case kEobBaseChargenStartLevels:
		return "kEobBaseChargenStartLevels";
	case kEobBaseChargenStatStrings:
		return "kEobBaseChargenStatStrings";
	case kEobBaseChargenRaceSexStrings:
		return "kEobBaseChargenRaceSexStrings";
	case kEobBaseChargenClassStrings:
		return "kEobBaseChargenClassStrings";
	case kEobBaseChargenAlignmentStrings:
		return "kEobBaseChargenAlignmentStrings";
	case kEobBaseChargenEnterGameStrings:
		return "kEobBaseChargenEnterGameStrings";
	case kEobBaseChargenClassMinStats:
		return "kEobBaseChargenClassMinStats";
	case kEobBaseChargenRaceMinStats:
		return "kEobBaseChargenRaceMinStats";
	case kEobBaseChargenRaceMaxStats:
		return "kEobBaseChargenRaceMaxStats";
	case kEobBaseConstModTable1:
		return "kEobBaseConstModTable1";
	case kEobBaseConstModTable2:
		return "kEobBaseConstModTable2";
	case kEobBaseConstModTable3:
		return "kEobBaseConstModTable3";
	case kEobBaseConstModTable4:
		return "kEobBaseConstModTable4";
	case kEobBaseConstModLvlIndex:
		return "kEobBaseConstModLvlIndex";
	case kEobBaseConstModDiv:
		return "kEobBaseConstModDiv";
	case kEobBaseConstModExt:
		return "kEobBaseConstModExt";
	case kEobBasePryDoorStrings:
		return "kEobBasePryDoorStrings";
	case kEobBaseWarningStrings:
		return "kEobBaseWarningStrings";
	case kEobBaseItemSuffixStringsRings:
		return "kEobBaseItemSuffixStringsRings";
	case kEobBaseItemSuffixStringsPotions:
		return "kEobBaseItemSuffixStringsPotions";
	case kEobBaseItemSuffixStringsWands:
		return "kEobBaseItemSuffixStringsWands";
	case kEobBaseRipItemStrings:
		return "kEobBaseRipItemStrings";
	case kEobBaseCursedString:
		return "kEobBaseCursedString";
	case kEobBaseEnchantedString:
		return "kEobBaseEnchantedString";
	case kEobBaseMagicObjectStrings:
		return "kEobBaseMagicObjectStrings";
	case kEobBaseMagicObject5String:
		return "kEobBaseMagicObject5String";
	case kEobBasePatternSuffix:
		return "kEobBasePatternSuffix";
	case kEobBasePatternGrFix1:
		return "kEobBasePatternGrFix1";
	case kEobBasePatternGrFix2:
		return "kEobBasePatternGrFix2";
	case kEobBaseValidateArmorString:
		return "kEobBaseValidateArmorString";
	case kEobBaseValidateCursedString:
		return "kEobBaseValidateCursedString";
	case kEobBaseValidateNoDropString:
		return "kEobBaseValidateNoDropString";
	case kEobBasePotionStrings:
		return "kEobBasePotionStrings";
	case kEobBaseWandString:
		return "kEobBaseWandString";
	case kEobBaseItemMisuseStrings:
		return "kEobBaseItemMisuseStrings";
	case kEobBaseTakenStrings:
		return "kEobBaseTakenStrings";
	case kEobBasePotionEffectStrings:
		return "kEobBasePotionEffectStrings";
	case kEobBaseYesNoStrings:
		return "kEobBaseYesNoStrings";
	case kLolEobCommonMoreStrings:
		return "kLolEobCommonMoreStrings";
	case kEobBaseNpcMaxStrings:
		return "kEobBaseNpcMaxStrings";
	case kEobBaseOkStrings:
		return "kEobBaseOkStrings";
	case kEobBaseNpcJoinStrings:
		return "kEobBaseNpcJoinStrings";		
	case kEobBaseCancelStrings:
		return "kEobBaseCancelStrings";
	case kEobBaseAbortStrings:
		return "kEobBaseAbortStrings";
	case kEobBaseMenuStringsMain:
		return "kEobBaseMenuStringsMain";
	case kEobBaseMenuStringsSaveLoad:
		return "kEobBaseMenuStringsSaveLoad";
	case kEobBaseMenuStringsOnOff:
		return "kEobBaseMenuStringsOnOff";
	case kEobBaseMenuStringsSpells:
		return "kEobBaseMenuStringsSpells";
	case kEobBaseMenuStringsRest:
		return "kEobBaseMenuStringsRest";
	case kEobBaseMenuStringsDrop:
		return "kEobBaseMenuStringsDrop";
	case kEobBaseMenuStringsExit:
		return "kEobBaseMenuStringsExit";
	case kEobBaseMenuStringsStarve:
		return "kEobBaseMenuStringsStarve";
	case kEobBaseMenuStringsScribe:
		return "kEobBaseMenuStringsScribe";
	case kEobBaseMenuStringsDrop2:
		return "kEobBaseMenuStringsDrop2";
	case kEobBaseMenuStringsHead:
		return "kEobBaseMenuStringsHead";
	case kEobBaseMenuStringsPoison:
		return "kEobBaseMenuStringsPoison";
	case kEobBaseMenuStringsMgc:
		return "kEobBaseMenuStringsMgc";
	case kEobBaseMenuStringsPrefs:
		return "kEobBaseMenuStringsPrefs";
	case kEobBaseMenuStringsRest2:
		return "kEobBaseMenuStringsRest2";
	case kEobBaseMenuStringsRest3:
		return "kEobBaseMenuStringsRest3";
	case kEobBaseMenuStringsRest4:
		return "kEobBaseMenuStringsRest4";
	case kEobBaseMenuStringsDefeat:
		return "kEobBaseMenuStringsDefeat";
	case kEobBaseMenuStringsTransfer:
		return "kEobBaseMenuStringsTransfer";
	case kEobBaseMenuStringsSpec:
		return "kEobBaseMenuStringsSpec";
	case kEobBaseMenuStringsSpellNo:
		return "kEobBaseMenuStringsSpellNo";
	case kEobBaseMenuYesNoStrings:
		return "kEobBaseMenuYesNoStrings";
	case kEobBaseSpellLevelsMage:
		return "kEobBaseSpellLevelsMage";
	case kEobBaseSpellLevelsCleric:
		return "kEobBaseSpellLevelsCleric";
	case kEobBaseNumSpellsCleric:
		return "kEobBaseNumSpellsCleric";
	case kEobBaseNumSpellsWisAdj:
		return "kEobBaseNumSpellsWisAdj";
	case kEobBaseNumSpellsPal:
		return "kEobBaseNumSpellsPal";
	case kEobBaseNumSpellsMage:
		return "kEobBaseNumSpellsMage";
	case kEobBaseCharGuiStringsHp:
		return "kEobBaseCharGuiStringsHp";
	case kEobBaseCharGuiStringsWp1:
		return "kEobBaseCharGuiStringsWp1";
	case kEobBaseCharGuiStringsWp2:
		return "kEobBaseCharGuiStringsWp2";
	case kEobBaseCharGuiStringsWr:
		return "kEobBaseCharGuiStringsWr";
	case kEobBaseCharGuiStringsSt1:
		return "kEobBaseCharGuiStringsSt1";
	case kEobBaseCharGuiStringsSt2:
		return "kEobBaseCharGuiStringsSt2";
	case kEobBaseCharGuiStringsIn:
		return "kEobBaseCharGuiStringsIn";
	case kEobBaseCharStatusStrings7:
		return "kEobBaseCharStatusStrings7";
	case kEobBaseCharStatusStrings81:
		return "kEobBaseCharStatusStrings81";
	case kEobBaseCharStatusStrings82:
		return "kEobBaseCharStatusStrings82";
	case kEobBaseCharStatusStrings9:
		return "kEobBaseCharStatusStrings9";
	case kEobBaseCharStatusStrings12:
		return "kEobBaseCharStatusStrings12";
	case kEobBaseCharStatusStrings131:
		return "kEobBaseCharStatusStrings131";
	case kEobBaseCharStatusStrings132:
		return "kEobBaseCharStatusStrings132";
	case kEobBaseLevelGainStrings:
		return "kEobBaseLevelGainStrings";
	case kEobBaseExperienceTable0:
		return "kEobBaseExperienceTable0";
	case kEobBaseExperienceTable1:
		return "kEobBaseExperienceTable1";
	case kEobBaseExperienceTable2:
		return "kEobBaseExperienceTable2";
	case kEobBaseExperienceTable3:
		return "kEobBaseExperienceTable3";
	case kEobBaseExperienceTable4:
		return "kEobBaseExperienceTable4";
	case kEobBaseWllFlagPreset:
		return "kEobBaseWllFlagPreset";
	case kEobBaseDscShapeCoords:
		return "kEobBaseDscShapeCoords";
	case kEobBaseDscDoorScaleOffs:
		return "kEobBaseDscDoorScaleOffs";
	case kEobBaseDscDoorScaleMult1:
		return "kEobBaseDscDoorScaleMult1";
	case kEobBaseDscDoorScaleMult2:
		return "kEobBaseDscDoorScaleMult2";
	case kEobBaseDscDoorScaleMult3:
		return "kEobBaseDscDoorScaleMult3";
	case kEobBaseDscDoorScaleMult4:
		return "kEobBaseDscDoorScaleMult4";
	case kEobBaseDscDoorScaleMult5:
		return "kEobBaseDscDoorScaleMult5";
	case kEobBaseDscDoorScaleMult6:
		return "kEobBaseDscDoorScaleMult6";
	case kEobBaseDscDoorType5Offs:
		return "kEobBaseDscDoorType5Offs";
	case kEobBaseDscDoorXE:
		return "kEobBaseDscDoorXE";
	case kEobBaseDscDoorY1:
		return "kEobBaseDscDoorY1";
	case kEobBaseDscDoorY3:
		return "kEobBaseDscDoorY3";
	case kEobBaseDscDoorY4:
		return "kEobBaseDscDoorY4";
	case kEobBaseDscDoorY5:
		return "kEobBaseDscDoorY5";
	case kEobBaseDscDoorY6:
		return "kEobBaseDscDoorY6";
	case kEobBaseDscDoorY7:
		return "kEobBaseDscDoorY7";
	case kEobBaseDscDoorCoordsExt:
		return "kEobBaseDscDoorCoordsExt";
	case kEobBaseDscItemPosIndex:
		return "kEobBaseDscItemPosIndex";
	case kEobBaseDscItemShpX:
		return "kEobBaseDscItemShpX";
	case kEobBaseDscItemPosUnk:
		return "kEobBaseDscItemPosUnk";
	case kEobBaseDscItemTileIndex:
		return "kEobBaseDscItemTileIndex";
	case kEobBaseDscItemShapeMap:
		return "kEobBaseDscItemShapeMap";
	case kEobBaseDscMonsterFrmOffsTbl1:
		return "kEobBaseDscMonsterFrmOffsTbl1";
	case kEobBaseDscMonsterFrmOffsTbl2:
		return "kEobBaseDscMonsterFrmOffsTbl2";
	case kEobBaseInvSlotX:
		return "kEobBaseInvSlotX";
	case kEobBaseInvSlotY:
		return "kEobBaseInvSlotY";
	case kEobBaseSlotValidationFlags:
		return "kEobBaseSlotValidationFlags";
	case kEobBaseProjectileWeaponTypes:
		return "kEobBaseProjectileWeaponTypes";
	case kEobBaseWandTypes:
		return "kEobBaseWandTypes";
	case kEobBaseDrawObjPosIndex:
		return "kEobBaseDrawObjPosIndex";
	case kEobBaseFlightObjFlipIndex:
		return "kEobBaseFlightObjFlipIndex";
	case kEobBaseFlightObjShpMap:
		return "kEobBaseFlightObjShpMap";
	case kEobBaseFlightObjSclIndex:
		return "kEobBaseFlightObjSclIndex";
	case kEobBaseDscTelptrShpCoords:
		return "kEobBaseDscTelptrShpCoords";
	case kEobBasePortalSeqData:
		return "kEobBasePortalSeqData";
	case kEobBaseManDef:
		return "kEobBaseManDef";
	case kEobBaseManWord:
		return "kEobBaseManWord";
	case kEobBaseManPrompt:
		return "kEobBaseManPrompt";
	case kEobBaseBookNumbers:
		return "kEobBaseBookNumbers";
	case kEobBaseMageSpellsList:
		return "kEobBaseMageSpellsList";
	case kEobBaseClericSpellsList:
		return "kEobBaseClericSpellsList";
	case kEobBaseSpellNames:
		return "kEobBaseSpellNames";

	case kEobBaseMagicStrings1:
		return "kEobBaseMagicStrings1";
	case kEobBaseMagicStrings2:
		return "kEobBaseMagicStrings2";
	case kEobBaseMagicStrings3:
		return "kEobBaseMagicStrings3";
	case kEobBaseMagicStrings4:
		return "kEobBaseMagicStrings4";
	case kEobBaseMagicStrings6:
		return "kEobBaseMagicStrings6";
	case kEobBaseMagicStrings7:
		return "kEobBaseMagicStrings7";
	case kEobBaseMagicStrings8:
		return "kEobBaseMagicStrings8";
	case kEobBaseExpObjectTlMode:
		return "kEobBaseExpObjectTlMode";
	case kEobBaseExpObjectTblIndex:
		return "kEobBaseExpObjectTblIndex";
	case kEobBaseExpObjectShpStart:
		return "kEobBaseExpObjectShpStart";
	case kEobBaseExpObjectTbl1:
		return "kEobBaseExpObjectTbl1";
	case kEobBaseExpObjectTbl2:
		return "kEobBaseExpObjectTbl2";
	case kEobBaseExpObjectTbl3:
		return "kEobBaseExpObjectTbl3";
	case kEobBaseExpObjectY:
		return "kEobBaseExpObjectY";
	case kEobBaseSparkDefSteps:
		return "kEobBaseSparkDefSteps";
	case kEobBaseSparkDefSubSteps:
		return "kEobBaseSparkDefSubSteps";
	case kEobBaseSparkDefShift:
		return "kEobBaseSparkDefShift";
	case kEobBaseSparkDefAdd:
		return "kEobBaseSparkDefAdd";
	case kEobBaseSparkDefX:
		return "kEobBaseSparkDefX";
	case kEobBaseSparkDefY:
		return "kEobBaseSparkDefY";
	case kEobBaseSparkOfFlags1:
		return "kEobBaseSparkOfFlags1";
	case kEobBaseSparkOfFlags2:
		return "kEobBaseSparkOfFlags2";
	case kEobBaseSparkOfShift:
		return "kEobBaseSparkOfShift";
	case kEobBaseSparkOfX:
		return "kEobBaseSparkOfX";
	case kEobBaseSparkOfY:
		return "kEobBaseSparkOfY";
	case kEobBaseSpellProperties:
		return "kEobBaseSpellProperties";
	case kEobBaseMagicFlightProps:
		return "kEobBaseMagicFlightProps";
	case kEobBaseTurnUndeadEffect:
		return "kEobBaseTurnUndeadEffect";
	case kEobBaseBurningHandsDest:
		return "kEobBaseBurningHandsDest";
	case kEobBaseConeOfColdDest1:
		return "kEobBaseConeOfColdDest1";
	case kEobBaseConeOfColdDest2:
		return "kEobBaseConeOfColdDest2";
	case kEobBaseConeOfColdDest3:
		return "kEobBaseConeOfColdDest3";
	case kEobBaseConeOfColdDest4:
		return "kEobBaseConeOfColdDest4";
	case kEobBaseConeOfColdGfxTbl:
		return "kEobBaseConeOfColdGfxTbl";
	case kEob1MainMenuStrings:
		return "kEob1MainMenuStrings";
	case kEob1BonusStrings:
		return "kEob1BonusStrings";
	case kEob1IntroFilesOpening:
		return "kEob1IntroFilesOpening";
	case kEob1IntroFilesTower:
		return "kEob1IntroFilesTower";
	case kEob1IntroFilesOrb:
		return "kEob1IntroFilesOrb";
	case kEob1IntroFilesWdEntry:
		return "kEob1IntroFilesWdEntry";
	case kEob1IntroFilesKing:
		return "kEob1IntroFilesKing";
	case kEob1IntroFilesHands:
		return "kEob1IntroFilesHands";
	case kEob1IntroFilesWdExit:
		return "kEob1IntroFilesWdExit";
	case kEob1IntroFilesTunnel:
		return "kEob1IntroFilesTunnel";
	case kEob1IntroOpeningFrmDelay:
		return "kEob1IntroOpeningFrmDelay";
	case kEob1IntroWdEncodeX:
		return "kEob1IntroWdEncodeX";
	case kEob1IntroWdEncodeY:
		return "kEob1IntroWdEncodeY";
	case kEob1IntroWdEncodeWH:
		return "kEob1IntroWdEncodeWH";
	case kEob1IntroWdDsX:
		return "kEob1IntroWdDsX";
	case kEob1IntroWdDsY:
		return "kEob1IntroWdDsY";
	case kEob1IntroTvlX1:
		return "kEob1IntroTvlX1";
	case kEob1IntroTvlY1:
		return "kEob1IntroTvlY1";
	case kEob1IntroTvlX2:
		return "kEob1IntroTvlX2";
	case kEob1IntroTvlY2:
		return "kEob1IntroTvlY2";
	case kEob1IntroTvlW:
		return "kEob1IntroTvlW";
	case kEob1IntroTvlH:
		return "kEob1IntroTvlH";
	case kEob1DoorShapeDefs:
		return "kEob1DoorShapeDefs";
	case kEob1DoorSwitchCoords:
		return "kEob1DoorSwitchCoords";
	case kEob1MonsterProperties:
		return "kEob1MonsterProperties";
	case kEob1EnemyMageSpellList:
		return "kEob1EnemyMageSpellList";
	case kEob1EnemyMageSfx:
		return "kEob1EnemyMageSfx";
	case kEob1BeholderSpellList:
		return "kEob1BeholderSpellList";
	case kEob1BeholderSfx:
		return "kEob1BeholderSfx";
	case kEob1TurnUndeadString:
		return "kEob1TurnUndeadString";
	case kEob1NpcShpData:
		return "kEob1NpcShpData";
	case kEob1NpcSubShpIndex1:
		return "kEob1NpcSubShpIndex1";
	case kEob1NpcSubShpIndex2:
		return "kEob1NpcSubShpIndex2";
	case kEob1NpcSubShpY:
		return "kEob1NpcSubShpY";
	case kEob1Npc0Strings:
		return "kEob1Npc0Strings";
	case kEob1Npc11Strings:
		return "kEob1Npc11Strings";
	case kEob1Npc12Strings:
		return "kEob1Npc12Strings";
	case kEob1Npc21Strings:
		return "kEob1Npc21Strings";
	case kEob1Npc22Strings:
		return "kEob1Npc22Strings";
	case kEob1Npc31Strings:
		return "kEob1Npc31Strings";
	case kEob1Npc32Strings:
		return "kEob1Npc32Strings";
	case kEob1Npc4Strings:
		return "kEob1Npc4Strings";
	case kEob1Npc5Strings:
		return "kEob1Npc5Strings";
	case kEob1Npc6Strings:
		return "kEob1Npc6Strings";
	case kEob1Npc7Strings:
		return "kEob1Npc7Strings";
	case kEob2MainMenuStrings:
		return "kEob2MainMenuStrings";
	case kEob2IntroStrings:
		return "kEob2IntroStrings";
	case kEob2IntroCPSFiles:
		return "kEob2IntroCPSFiles";
	case kEob2IntroSeqData00:
		return "kEob2IntroSeqData00";
 	case kEob2IntroSeqData01:
		return "kEob2IntroSeqData01";
 	case kEob2IntroSeqData02:
		return "kEob2IntroSeqData02";
 	case kEob2IntroSeqData03:
		return "kEob2IntroSeqData03";
 	case kEob2IntroSeqData04:
		return "kEob2IntroSeqData04";
 	case kEob2IntroSeqData05:
		return "kEob2IntroSeqData05";
 	case kEob2IntroSeqData06:
		return "kEob2IntroSeqData06";
 	case kEob2IntroSeqData07:
		return "kEob2IntroSeqData07";
 	case kEob2IntroSeqData08:
		return "kEob2IntroSeqData08";
 	case kEob2IntroSeqData09:
		return "kEob2IntroSeqData09";
 	case kEob2IntroSeqData10:
		return "kEob2IntroSeqData10";
 	case kEob2IntroSeqData11:
		return "kEob2IntroSeqData11";
 	case kEob2IntroSeqData12:
		return "kEob2IntroSeqData12";
 	case kEob2IntroSeqData13:
		return "kEob2IntroSeqData13";
 	case kEob2IntroSeqData14:
		return "kEob2IntroSeqData14";
 	case kEob2IntroSeqData15:
		return "kEob2IntroSeqData15";
 	case kEob2IntroSeqData16:
		return "kEob2IntroSeqData16";
 	case kEob2IntroSeqData17:
		return "kEob2IntroSeqData17";
 	case kEob2IntroSeqData18:
		return "kEob2IntroSeqData18";
 	case kEob2IntroSeqData19:
		return "kEob2IntroSeqData19";
 	case kEob2IntroSeqData20:
		return "kEob2IntroSeqData20";
 	case kEob2IntroSeqData21:
		return "kEob2IntroSeqData21";
 	case kEob2IntroSeqData22:
		return "kEob2IntroSeqData22";
 	case kEob2IntroSeqData23:
		return "kEob2IntroSeqData23";
 	case kEob2IntroSeqData24:
		return "kEob2IntroSeqData24";
 	case kEob2IntroSeqData25:
		return "kEob2IntroSeqData25";
 	case kEob2IntroSeqData26:
		return "kEob2IntroSeqData26";
 	case kEob2IntroSeqData27:
		return "kEob2IntroSeqData27";
 	case kEob2IntroSeqData28:
		return "kEob2IntroSeqData28";
 	case kEob2IntroSeqData29:
		return "kEob2IntroSeqData29";
 	case kEob2IntroSeqData30:
		return "kEob2IntroSeqData30";
 	case kEob2IntroSeqData31:
		return "kEob2IntroSeqData31";
 	case kEob2IntroSeqData32:
		return "kEob2IntroSeqData32";
 	case kEob2IntroSeqData33:
		return "kEob2IntroSeqData33";
 	case kEob2IntroSeqData34:
		return "kEob2IntroSeqData34";
 	case kEob2IntroSeqData35:
		return "kEob2IntroSeqData35";
 	case kEob2IntroSeqData36:
		return "kEob2IntroSeqData36";
 	case kEob2IntroSeqData37:
		return "kEob2IntroSeqData37";
 	case kEob2IntroSeqData38:
		return "kEob2IntroSeqData38";
 	case kEob2IntroSeqData39:
		return "kEob2IntroSeqData39";
 	case kEob2IntroSeqData40:
		return "kEob2IntroSeqData40";
 	case kEob2IntroSeqData41:
		return "kEob2IntroSeqData41";
 	case kEob2IntroSeqData42:
		return "kEob2IntroSeqData42";
 	case kEob2IntroSeqData43:
		return "kEob2IntroSeqData43";
 	case kEob2IntroShapes00:
		return "kEob2IntroShapes00";
 	case kEob2IntroShapes01:
		return "kEob2IntroShapes01";
 	case kEob2IntroShapes04:
		return "kEob2IntroShapes04";
 	case kEob2IntroShapes07:
		return "kEob2IntroShapes07";
	case kEob2FinaleStrings:
		return "kEob2FinaleStrings";
	case kEob2CreditsData:
		return "kEob2CreditsData";
	case kEob2FinaleCPSFiles:
		return "kEob2FinaleCPSFiles";		
 	case kEob2FinaleSeqData00:
		return "kEob2FinaleSeqData00";
 	case kEob2FinaleSeqData01:
		return "kEob2FinaleSeqData01";
 	case kEob2FinaleSeqData02:
		return "kEob2FinaleSeqData02";
 	case kEob2FinaleSeqData03:
		return "kEob2FinaleSeqData03";
 	case kEob2FinaleSeqData04:
		return "kEob2FinaleSeqData04";
 	case kEob2FinaleSeqData05:
		return "kEob2FinaleSeqData05";
 	case kEob2FinaleSeqData06:
		return "kEob2FinaleSeqData06";
 	case kEob2FinaleSeqData07:
		return "kEob2FinaleSeqData07";
 	case kEob2FinaleSeqData08:
		return "kEob2FinaleSeqData08";
 	case kEob2FinaleSeqData09:
		return "kEob2FinaleSeqData09";
 	case kEob2FinaleSeqData10:
		return "kEob2FinaleSeqData10";
 	case kEob2FinaleSeqData11:
		return "kEob2FinaleSeqData11";
 	case kEob2FinaleSeqData12:
		return "kEob2FinaleSeqData12";
 	case kEob2FinaleSeqData13:
		return "kEob2FinaleSeqData13";
 	case kEob2FinaleSeqData14:
		return "kEob2FinaleSeqData14";
 	case kEob2FinaleSeqData15:
		return "kEob2FinaleSeqData15";
 	case kEob2FinaleSeqData16:
		return "kEob2FinaleSeqData16";
 	case kEob2FinaleSeqData17:
		return "kEob2FinaleSeqData17";
 	case kEob2FinaleSeqData18:
		return "kEob2FinaleSeqData18";
 	case kEob2FinaleSeqData19:
		return "kEob2FinaleSeqData19";
 	case kEob2FinaleSeqData20:
		return "kEob2FinaleSeqData20";	
 	case kEob2FinaleShapes00:
		return "kEob2FinaleShapes00";
 	case kEob2FinaleShapes03:
		return "kEob2FinaleShapes03";
 	case kEob2FinaleShapes07:
		return "kEob2FinaleShapes07";
 	case kEob2FinaleShapes09:
		return "kEob2FinaleShapes09";
 	case kEob2FinaleShapes10:
		return "kEob2FinaleShapes10";
	case kEob2NpcShapeData:
		return "kEob2NpcShapeData";
	case kEobBaseClassModifierFlags:
		return "kEobBaseClassModifierFlags";
	case kEobBaseMonsterStepTable01:
		return "kEobBaseMonsterStepTable01";
	case kEobBaseMonsterStepTable02:
		return "kEobBaseMonsterStepTable02";
	case kEobBaseMonsterStepTable1:
		return "kEobBaseMonsterStepTable1";
	case kEobBaseMonsterStepTable2:
		return "kEobBaseMonsterStepTable2";
	case kEobBaseMonsterStepTable3:
		return "kEobBaseMonsterStepTable3";
	case kEobBaseMonsterCloseAttPosTable1:
		return "kEobBaseMonsterCloseAttPosTable1";
	case kEobBaseMonsterCloseAttPosTable21:
		return "kEobBaseMonsterCloseAttPosTable21";
	case kEobBaseMonsterCloseAttPosTable22:
		return "kEobBaseMonsterCloseAttPosTable22";
	case kEobBaseMonsterCloseAttUnkTable:
		return "kEobBaseMonsterCloseAttUnkTable";
	case kEobBaseMonsterCloseAttChkTable1:
		return "kEobBaseMonsterCloseAttChkTable1";
	case kEobBaseMonsterCloseAttChkTable2:
		return "kEobBaseMonsterCloseAttChkTable2";
	case kEobBaseMonsterCloseAttDstTable1:
		return "kEobBaseMonsterCloseAttDstTable1";
	case kEobBaseMonsterCloseAttDstTable2:
		return "kEobBaseMonsterCloseAttDstTable2";
	case kEobBaseMonsterProximityTable:
		return "kEobBaseMonsterProximityTable";
	case kEobBaseFindBlockMonstersTable:
		return "kEobBaseFindBlockMonstersTable";
	case kEobBaseMonsterDirChangeTable:
		return "kEobBaseMonsterDirChangeTable";
	case kEobBaseMonsterDistAttStrings:
		return "kEobBaseMonsterDistAttStrings";
	case kEobBaseEncodeMonsterDefs:
		return "kEobBaseEncodeMonsterDefs";
	case kEobBaseNpcPresets:
		return "kEobBaseNpcPresets";
	case kEob2Npc1Strings:
		return "kEob2Npc1Strings";
	case kEob2Npc2Strings:
		return "kEob2Npc2Strings";
	case kEob2MonsterDustStrings:
		return "kEob2MonsterDustStrings";
	case kEob2DranFoolsStrings:
		return "kEob2DranFoolsStrings";
	case kEob2HornStrings:
		return "kEob2HornStrings";
	case kEob2HornSounds:
		return "kEob2HornSounds";
	case kLolIngamePakFiles:
		return "kLolIngamePakFiles";
	case kLolCharacterDefs:
		return "kLolCharacterDefs";
	case kLolIngameSfxFiles:
		return "kLolIngameSfxFiles";
	case kLolIngameSfxIndex:
		return "kLolIngameSfxIndex";
	case kLolMusicTrackMap:
		return "kLolMusicTrackMap";
	case kLolIngameGMSfxIndex:
		return "kLolIngameGMSfxIndex";
	case kLolIngameMT32SfxIndex:
		return "kLolIngameMT32SfxIndex";
	case kLolIngamePcSpkSfxIndex:
		return "kLolIngamePcSpkSfxIndex";
	case kLolSpellProperties:
		return "kLolSpellProperties";
	case kLolGameShapeMap:
		return "kLolGameShapeMap";
	case kLolSceneItemOffs:
		return "kLolSceneItemOffs";
	case kLolCharInvIndex:
		return "kLolCharInvIndex";
	case kLolCharInvDefs:
		return "kLolCharInvDefs";
	case kLolCharDefsMan:
		return "kLolCharDefsMan";
	case kLolCharDefsWoman:
		return "kLolCharDefsWoman";
	case kLolCharDefsKieran:
		return "kLolCharDefsKieran";
	case kLolCharDefsAkshel:
		return "kLolCharDefsAkshel";
	case kLolExpRequirements:
		return "kLolExpRequirements";
	case kLolMonsterModifiers:
		return "kLolMonsterModifiers";
	case kLolMonsterShiftOffsets:
		return "kLolMonsterShiftOffsets";
	case kLolMonsterDirFlags:
		return "kLolMonsterDirFlags";
	case kLolMonsterScaleY:
		return "kLolMonsterScaleY";
	case kLolMonsterScaleX:
		return "kLolMonsterScaleX";
	case kLolMonsterScaleWH:
		return "kLolMonsterScaleWH";
	case kLolFlyingObjectShp:
		return "kLolFlyingObjectShp";
	case kLolInventoryDesc:
		return "kLolInventoryDesc";
	case kLolLevelShpList:
		return "kLolLevelShpList";
	case kLolLevelDatList:
		return "kLolLevelDatList";
	case kLolCompassDefs:
		return "kLolCompassDefs";
	case kLolItemPrices:
		return "kLolItemPrices";
	case kLolStashSetup:
		return "kLolStashSetup";
	case kLolDscWalls:
		return "kLolDscWalls";
	case kLolEobCommonDscShapeIndex:
		return "kLolEobCommonDscShapeIndex";
	case kLolDscOvlMap:
		return "kLolDscOvlMap";
	case kLolDscScaleWidthData:
		return "kLolDscScaleWidthData";
	case kLolDscScaleHeightData:
		return "kLolDscScaleHeightData";
	case kLolEobCommonDscX:
		return "kLolEobCommonDscX";
	case kLolDscY:
		return "kLolDscY";
	case kLolEobCommonDscTileIndex:
		return "kLolEobCommonDscTileIndex";
	case kLolEobCommonDscUnk2:
		return "kLolEobCommonDscUnk2";
	case kLolEobCommonDscDoorShapeIndex:
		return "kLolEobCommonDscDoorShapeIndex";
	case kLolEobCommonDscDimData1:
		return "kLolEobCommonDscDimData1";
	case kLolEobCommonDscDimData2:
		return "kLolEobCommonDscDimData2";
	case kLolEobCommonDscBlockMap:
		return "kLolEobCommonDscBlockMap";
	case kLolEobCommonDscDimMap:
		return "kLolEobCommonDscDimMap";
	case kLolDscOvlIndex:
		return "kLolDscOvlIndex";
	case kLolEobCommonDscBlockIndex:
		return "kLolEobCommonDscBlockIndex";
	case kLolEobCommonDscDoorY2:
		return "kLolEobCommonDscDoorY2";
	case kLolDscDoorScale:
		return "kLolDscDoorScale";
	case kLolDscDoor4:
		return "kLolDscDoor4";
	case kLolDscDoorX:
		return "kLolDscDoorX";
	case kLolDscDoorY:
		return "kLolDscDoorY";
	case kLolScrollXTop:
		return "kLolScrollXTop";
	case kLolScrollYTop:
		return "kLolScrollYTop";
	case kLolScrollXBottom:
		return "kLolScrollXBottom";
	case kLolScrollYBottom:
		return "kLolScrollYBottom";
	case kLolButtonDefs:
		return "kLolButtonDefs";
	case kLolButtonList1:
		return "kLolButtonList1";
	case kLolButtonList2:
		return "kLolButtonList2";
	case kLolButtonList3:
		return "kLolButtonList3";
	case kLolButtonList4:
		return "kLolButtonList4";
	case kLolButtonList5:
		return "kLolButtonList5";
	case kLolButtonList6:
		return "kLolButtonList6";
	case kLolButtonList7:
		return "kLolButtonList7";
	case kLolButtonList8:
		return "kLolButtonList8";
	case kLolLegendData:
		return "kLolLegendData";
	case kLolMapCursorOvl:
		return "kLolMapCursorOvl";
	case kLolMapStringId:
		return "kLolMapStringId";
	case kLolSpellbookAnim:
		return "kLolSpellbookAnim";
	case kLolSpellbookCoords:
		return "kLolSpellbookCoords";
	case kLolHealShapeFrames:
		return "kLolHealShapeFrames";
	case kLolLightningDefs:
		return "kLolLightningDefs";
	case kLolFireballCoords:
		return "kLolFireballCoords";
	case kLolHistory:
		return "kLolHistory";
	default:
		return "Unknown";
	}
}

struct ExtractData {
	ExtractData() : desc(), offset() {}
	ExtractData(ExtractEntrySearchData d, uint32 o) : desc(d), offset(o) {}

	ExtractEntrySearchData desc;
	uint32 offset;
};

typedef std::pair<int, ExtractEntrySearchData> SearchMapEntry;
typedef std::multimap<int, ExtractEntrySearchData> SearchMap;

typedef std::pair<int, ExtractData> ExtractMapEntry;
typedef std::multimap<int, ExtractData> ExtractMap;

bool getExtractionData(const Game *g, Search &search, ExtractMap &map);

bool createIDMap(PAKFile &out, const ExtractInformation *eI, const int *needList);

bool process(PAKFile &out, const Game *g, const byte *data, const uint32 size) {
	char filename[128];

	Search search(data, size);
	ExtractMap ids;

	if (!getExtractionData(g, search, ids))
		return false;

	const int *needList = getNeedList(g);
	if (!needList) {
		fprintf(stderr, "ERROR: No entry need list available\n");
		return false;
	}

	ExtractInformation extractInfo;
	extractInfo.game = g->game;
	extractInfo.platform = g->platform;
	extractInfo.special = g->special;

	for (ExtractMap::const_iterator i = ids.begin(); i != ids.end(); ++i) {
		const int id = i->first;
		extractInfo.lang = i->second.desc.lang;

		const ExtractFilename *fDesc = getFilenameDesc(id);

		if (!fDesc) {
			fprintf(stderr, "ERROR: couldn't find file description for id %d/%s\n", id, getIdString(id));
			return false;
		}

		filename[0] = 0;
		if (!getFilename(filename, &extractInfo, id)) {
			fprintf(stderr, "ERROR: couldn't get filename for id %d/%s\n", id, getIdString(id));
			return false;
		}

		const ExtractType *tDesc = findExtractType(fDesc->type);

		if (!tDesc) {
			fprintf(stderr, "ERROR: couldn't find type description for id %d/%s (%d)\n", id, getIdString(id), fDesc->type);
			return false;
		}

		PAKFile::cFileList *list = out.getFileList();
		if (list && list->findEntry(filename) != 0)
			continue;

		if (!tDesc->extract(out, &extractInfo, data + i->second.offset, i->second.desc.hint.size, filename, id)) {
			fprintf(stderr, "ERROR: couldn't extract id %d/%s\n", id, getIdString(id));
			return false;
		}
	}

	for (int i = 0; i < 3; ++i) {
		if (g->lang[i] == -1)
			continue;

		extractInfo.lang = g->lang[i];
		if (!createIDMap(out, &extractInfo, needList))
			return false;

		if (!updateIndex(out, &extractInfo)) {
			error("couldn't update INDEX file, stop processing of all files");
			return false;
		}
	}

	return true;
}

bool createIDMap(PAKFile &out, const ExtractInformation *eI, const int *needList) {
	int dataEntries = 0;
	// Count entries in the need list
	for (const int *n = needList; *n != -1; ++n)
		++dataEntries;

	const int mapSize = 2 + dataEntries * (2 + 1 + 4);
	uint8 *map = new uint8[mapSize];
	uint8 *dst = map;

	WRITE_BE_UINT16(dst, dataEntries); dst += 2;
	for (const int *id = needList; *id != -1; ++id) {
		WRITE_BE_UINT16(dst, *id); dst += 2;
		const ExtractFilename *fDesc = getFilenameDesc(*id);
		if (!fDesc)
			return false;
		*dst++ = getTypeID(fDesc->type);
		WRITE_BE_UINT32(dst, getFilename(eI, *id)); dst += 4;
	}

	char filename[12];
	if (!getFilename(filename, eI, 0)) {
		fprintf(stderr, "ERROR: Could not create ID map for game\n");
		return false;
	}

	out.removeFile(filename);
	if (!out.addFile(filename, map, mapSize)) {
		fprintf(stderr, "ERROR: Could not add ID map \"%s\" to kyra.dat\n", filename);
		return false;
	}

	return true;
}

// Uncomment this to get various debug information about the detection table entries.
//#define DEBUG_EXTRACTION_TABLES

bool setupSearch(const Game *g, const int *needList, Search &search, SearchMap &searchData) {
	for (const int *entry = needList; *entry != -1; ++entry) {
		ExtractEntryList providers = getProvidersForId(*entry);

		if (providers.empty()) {
			fprintf(stderr, "ERROR: No provider for id %d/%s\n", *entry, getIdString(*entry));
			return false;
		} else {
			for (ExtractEntryList::const_iterator i = providers.begin(); i != providers.end(); ++i) {
				// Only add generic or partly matching providers here.
#ifndef DEBUG_EXTRACTION_TABLES
				if ((i->lang == UNK_LANG || i->lang == g->lang[0] || i->lang == g->lang[1] || i->lang == g->lang[2]) &&
				    (i->platform == kPlatformUnknown || (i->platform == g->platform))) {
#endif
					search.addData(i->hint);
					searchData.insert(SearchMapEntry(*entry, *i));
#ifndef DEBUG_EXTRACTION_TABLES
				}
#endif
			}
		}
	}

	return true;
}

typedef std::list<ExtractMap::const_iterator> MatchList;
MatchList filterPlatformMatches(const Game *g, std::pair<ExtractMap::const_iterator, ExtractMap::const_iterator> range) {
	bool hasPlatformMatch = false;
	for (ExtractMap::const_iterator i = range.first; i != range.second; ++i) {
		if (i->second.desc.platform == g->platform) {
			hasPlatformMatch = true;
			break;
		}
	}

	MatchList result;
	if (hasPlatformMatch) {
		for (ExtractMap::const_iterator i = range.first; i != range.second; ++i) {
			if (i->second.desc.platform == g->platform)
				result.push_back(i);
		}
	} else {
		for (ExtractMap::const_iterator i = range.first; i != range.second; ++i)
			result.push_back(i);
	}

	return result;
}

MatchList filterLanguageMatches(const int lang, const MatchList &input) {
	std::list<ExtractMap::const_iterator> result;

	for (MatchList::const_iterator i = input.begin(); i != input.end(); ++i) {
		if ((*i)->second.desc.lang == lang)
			result.push_back(*i);
	}

	return result;
}

MatchList::const_iterator filterOutBestMatch(const MatchList &input) {
	MatchList::const_iterator result = input.begin();

	if (input.size() > 1)
		warning("Multiple entries found for id %d/%s", (*result)->first, getIdString((*result)->first));

	for (MatchList::const_iterator i = input.begin(); i != input.end(); ++i) {
		// Reduce all entries to one single entry.
		//
		// We use the following rules for this (in this order):
		// - Prefer the entry with the higest size
		// - Prefer the entry, which starts at the smallest offest
		//
		// TODO: These rules might not be safe for all games, but hopefully
		// they will work fine. If there are any problems it should be rather
		// easy to identify them, since we print out a warning for multiple
		// entries found.
		if ((*result)->second.desc.hint.size <= (*i)->second.desc.hint.size) {
			if ((*result)->second.offset >= (*i)->second.offset)
				result = i;
		}
	}

	return result;
}

bool getExtractionData(const Game *g, Search &search, ExtractMap &map) {
	SearchMap searchMap;

	const int *needList = getNeedList(g);
	if (!needList) {
		fprintf(stderr, "ERROR: No entry need list available\n");
		return false;
	}

	if (!setupSearch(g, needList, search, searchMap))
		return false;

	// Process the data search
	Search::ResultList results;
	search.search(results);

	if (results.empty()) {
		fprintf(stderr, "ERROR: Couldn't find any required data\n");
		return false;
	}

	ExtractMap temporaryExtractMap;
	for (const int *entry = needList; *entry != -1; ++entry) {
		typedef std::pair<SearchMap::const_iterator, SearchMap::const_iterator> KeyRange;
		KeyRange idRange = searchMap.equal_range(*entry);

		for (Search::ResultList::const_iterator i = results.begin(); i != results.end(); ++i) {
			for (SearchMap::const_iterator j = idRange.first; j != idRange.second; ++j) {
				if (j->second.hint == i->data)
					temporaryExtractMap.insert(ExtractMapEntry(*entry, ExtractData(j->second, i->offset)));
			}
		}
	}

	// Free up some memory
	results.clear();
	searchMap.clear();

	bool result = true;

	for (const int *entry = needList; *entry != -1; ++entry) {
		MatchList possibleMatches = filterPlatformMatches(g, temporaryExtractMap.equal_range(*entry));

		if (possibleMatches.empty()) {
			fprintf(stderr, "ERROR: No entry found for id %d/%s\n", *entry, getIdString(*entry));
			result = false;
			continue;
		}

		if (isLangSpecific(*entry)) {
			for (int i = 0; i < 3; ++i) {
				if (g->lang[i] == -1)
					continue;

				MatchList langMatches = filterLanguageMatches(g->lang[i], possibleMatches);
				MatchList::const_iterator bestMatch = filterOutBestMatch(langMatches);

				if (bestMatch == langMatches.end()) {
					// TODO: Add nice language name to output message.
					fprintf(stderr, "ERROR: No entry found for id %d/%s for language %d\n", *entry, getIdString(*entry), g->lang[i]);
					result = false;
					continue;
				}

#ifdef DEBUG_EXTRACTION_TABLES
				if (((*bestMatch)->second.desc.platform != kPlatformUnknown && (*bestMatch)->second.desc.platform != g->platform))
					printf("%s: %.8X %.8X %d %d\n", getIdString(*entry), (*bestMatch)->second.desc.hint.size, (*bestMatch)->second.desc.hint.byteSum, (*bestMatch)->second.desc.lang, (*bestMatch)->second.desc.platform);
#endif

				map.insert(**bestMatch);
			}
		} else {
			MatchList::const_iterator bestMatch = filterOutBestMatch(possibleMatches);

			if (bestMatch == possibleMatches.end()) {
				fprintf(stderr, "ERROR: No entry found for id %d/%s\n", *entry, getIdString(*entry));
				result = false;
				continue;
			}

#ifdef DEBUG_EXTRACTION_TABLES
			if (((*bestMatch)->second.desc.platform != kPlatformUnknown && (*bestMatch)->second.desc.platform != g->platform))
				printf("%s: %.8X %.8X %d %d\n", getIdString(*entry), (*bestMatch)->second.desc.hint.size, (*bestMatch)->second.desc.hint.byteSum, (*bestMatch)->second.desc.lang, (*bestMatch)->second.desc.platform);
#endif

			map.insert(**bestMatch);
		}
	}

	return result;
}
