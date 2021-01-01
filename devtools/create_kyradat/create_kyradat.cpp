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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

// HACK to allow building with the SDL backend on MinGW
// see bug #1800764 "TOOLS: MinGW tools building broken"
#ifdef main
#undef main
#endif // main

#include "create_kyradat.h"
#include "resources.h"
#include "types.h"

#include "pak.h"

#include "md5.h"
#include "common/language.h"
#include "common/platform.h"

#include <vector>
#include <string>
#include <algorithm>


enum {
	kKyraDatVersion = 108
};

const ExtractFilename extractFilenames[] = {
	// GENERIC ID MAP
	{ kIdMap, -1, true },

	// INTRO / OUTRO sequences
	{ k1ForestSeq, kRawData, false },
	{ k1KallakWritingSeq, kRawData, false },
	{ k1KyrandiaLogoSeq, kRawData, false },
	{ k1KallakMalcolmSeq, kRawData, false },
	{ k1MalcolmTreeSeq, kRawData, false },
	{ k1WestwoodLogoSeq, kRawData, false },
	{ k1Demo1Seq, kRawData, false },
	{ k1Demo2Seq, kRawData, false },
	{ k1Demo3Seq, kRawData, false },
	{ k1Demo4Seq, kRawData, false },
	{ k1OutroReunionSeq, kRawData, false },

	// INTRO / OUTRO strings
	{ k1IntroCPSStrings, kStringList, false },
	{ k1IntroCOLStrings, kStringList, false },
	{ k1IntroWSAStrings, kStringList, false },
	{ k1IntroStrings, kStringList, true },
	{ k1OutroHomeString, kStringList, true },

	// INGAME strings
	{ k1ItemNames, kStringList, true },
	{ k1TakenStrings, kStringList, true },
	{ k1PlacedStrings, kStringList, true },
	{ k1DroppedStrings, kStringList, true },
	{ k1NoDropStrings, kStringList, true },
	{ k1PutDownString, kStringList, true },
	{ k1WaitAmuletString, kStringList, true },
	{ k1BlackJewelString, kStringList, true },
	{ k1PoisonGoneString, kStringList, true },
	{ k1HealingTipString, kStringList, true },
	{ k1ThePoisonStrings, kStringList, true },
	{ k1FluteStrings, kStringList, true },
	{ k1WispJewelStrings, kStringList, true },
	{ k1MagicJewelStrings, kStringList, true },
	{ k1FlaskFullString, kStringList, true },
	{ k1FullFlaskString, kStringList, true },
	{ k1VeryCleverString, kStringList, true },
	{ k1NewGameString, kStringList, true },

	// GUI strings table
	{ k1GUIStrings, kStringList, true },
	{ k1ConfigStrings, kStringList, true },

	// ROOM table/filenames
	{ k1RoomList, kRoomList, false },
	{ k1RoomFilenames, kStringList, false },

	// SHAPE tables
	{ k1DefaultShapes, kShapeList, false },
	{ k1Healing1Shapes, kShapeList, false },
	{ k1Healing2Shapes, kShapeList, false },
	{ k1PoisonDeathShapes, kShapeList, false },
	{ k1FluteShapes, kShapeList, false },
	{ k1Winter1Shapes, kShapeList, false },
	{ k1Winter2Shapes, kShapeList, false },
	{ k1Winter3Shapes, kShapeList, false },
	{ k1DrinkShapes, kShapeList, false },
	{ k1WispShapes, kShapeList, false },
	{ k1MagicAnimShapes, kShapeList, false },
	{ k1BranStoneShapes, kShapeList, false },

	// IMAGE filename table
	{ k1CharacterImageFilenames, kStringList, false },

	// AUDIO filename table
	{ k1AudioTracks, kStringList, false },
	{ k1AudioTracksIntro, kStringList, false },

	// AMULET anim
	{ k1AmuleteAnimSeq, kRawData, false },

	// PALETTE table
	{ k1SpecialPalette1, kRawData, false },
	{ k1SpecialPalette2, kRawData, false },
	{ k1SpecialPalette3, kRawData, false },
	{ k1SpecialPalette4, kRawData, false },
	{ k1SpecialPalette5, kRawData, false },
	{ k1SpecialPalette6, kRawData, false },
	{ k1SpecialPalette7, kRawData, false },
	{ k1SpecialPalette8, kRawData, false },
	{ k1SpecialPalette9, kRawData, false },
	{ k1SpecialPalette10, kRawData, false },
	{ k1SpecialPalette11, kRawData, false },
	{ k1SpecialPalette12, kRawData, false },
	{ k1SpecialPalette13, kRawData, false },
	{ k1SpecialPalette14, kRawData, false },
	{ k1SpecialPalette15, kRawData, false },
	{ k1SpecialPalette16, kRawData, false },
	{ k1SpecialPalette17, kRawData, false },
	{ k1SpecialPalette18, kRawData, false },
	{ k1SpecialPalette19, kRawData, false },
	{ k1SpecialPalette20, kRawData, false },
	{ k1SpecialPalette21, kRawData, false },
	{ k1SpecialPalette22, kRawData, false },
	{ k1SpecialPalette23, kRawData, false },
	{ k1SpecialPalette24, kRawData, false },
	{ k1SpecialPalette25, kRawData, false },
	{ k1SpecialPalette26, kRawData, false },
	{ k1SpecialPalette27, kRawData, false },
	{ k1SpecialPalette28, kRawData, false },
	{ k1SpecialPalette29, kRawData, false },
	{ k1SpecialPalette30, kRawData, false },
	{ k1SpecialPalette31, kRawData, false },
	{ k1SpecialPalette32, kRawData, false },
	{ k1SpecialPalette33, kRawData, false },

	// CREDITS (used in FM-TOWNS and AMIGA)
	{ k1CreditsStrings, kRawData, true },

	// FM-TOWNS specific
	{ k1TownsMusicFadeTable, kRawData, false },
	{ k1TownsSFXwdTable, kRawData, false },
	{ k1TownsSFXbtTable, kRawData, false },
	{ k1TownsCDATable, kRawData, false },

	// PC98 specific
	{ k1PC98StoryStrings, kStringList, true },
	{ k1PC98IntroSfx, kRawData, false },

	// AMIGA specific
	{ k1AmigaIntroSFXTable, kAmigaSfxTable, false },
	{ k1AmigaGameSFXTable, kAmigaSfxTable, false },

	// HAND OF FATE

	// Sequence Player
	{ k2SeqplayPakFiles, kStringList, false },
	{ k2SeqplayCredits, kRawData, false },
	{ k2SeqplayCreditsSpecial, kStringList, false },
	{ k2SeqplayStrings, kStringList, true },
	{ k2SeqplaySfxFiles, kStringList, false },
	{ k2SeqplayTlkFiles, kStringList, true },
	{ k2SeqplaySeqData, k2SeqData, false },
	{ k2SeqplayIntroTracks, kStringList, false },
	{ k2SeqplayFinaleTracks, kStringList, false },
	{ k2SeqplayIntroCDA, kRawData, false },
	{ k2SeqplayFinaleCDA, kRawData, false },
	{ k2SeqplayShapeAnimData, k2SeqItemAnimData, false },

	// Ingame
	{ k2IngamePakFiles, kStringList, false },
	{ k2IngameSfxFiles, kStringList, false },
	{ k2IngameSfxIndex, kRawData, false },
	{ k2IngameTracks, kStringList, false },
	{ k2IngameCDA, kRawData, false },
	{ k2IngameTalkObjIndex, kRawData, false },
	{ k2IngameTimJpStrings, kStringList, false },
	{ k2IngameShapeAnimData, k2ItemAnimDefinition, false },
	{ k2IngameTlkDemoStrings, kStringList, true },


	// MALCOLM'S REVENGE
	{ k3MainMenuStrings, kStringList, false },
	{ k3MusicFiles, kStringList, false },
	{ k3ScoreTable, kRawData, false },
	{ k3SfxFiles, kStringList, false },
	{ k3SfxMap, kRawData, false },
	{ k3ItemAnimData, k2ItemAnimDefinition, false },
	{ k3ItemMagicTable, kRawData, false },
	{ k3ItemStringMap, kRawData, false },

	// EYE OF THE BEHOLDER COMMON
	{ kEoBBaseChargenStrings1, kStringList, true },
	{ kEoBBaseChargenStrings2, kStringList, true },
	{ kEoBBaseChargenStartLevels, kRawData, false },
	{ kEoBBaseChargenStatStrings, kStringList, true},
	{ kEoBBaseChargenRaceSexStrings, kStringList, true },
	{ kEoBBaseChargenClassStrings, kStringList, true },
	{ kEoBBaseChargenAlignmentStrings, kStringList, true },
	{ kEoBBaseChargenEnterGameStrings, kStringList, true },
	{ kEoBBaseChargenClassMinStats, kRawData, false },
	{ kEoBBaseChargenRaceMinStats, kRawData, false },
	{ kEoBBaseChargenRaceMaxStats, kRawDataBe16, false },

	{ kEoBBaseSaveThrowTable1, kRawData, false },
	{ kEoBBaseSaveThrowTable2, kRawData, false },
	{ kEoBBaseSaveThrowTable3, kRawData, false },
	{ kEoBBaseSaveThrowTable4, kRawData, false },
	{ kEoBBaseSaveThrwLvlIndex, kRawData, false },
	{ kEoBBaseSaveThrwModDiv, kRawData, false },
	{ kEoBBaseSaveThrwModExt, kRawData, false },

	{ kEoBBasePryDoorStrings, kStringList, true },
	{ kEoBBaseWarningStrings, kStringList, true },

	{ kEoBBaseItemSuffixStringsRings, kStringList, true },
	{ kEoBBaseItemSuffixStringsPotions, kStringList, true },
	{ kEoBBaseItemSuffixStringsWands, kStringList, true },

	{ kEoBBaseRipItemStrings, kStringList, true },
	{ kEoBBaseCursedString, kStringList, true },
	{ kEoBBaseEnchantedString, kStringList, false },
	{ kEoBBaseMagicObjectStrings, kStringList, true },
	{ kEoBBaseMagicObjectString5, kStringList, true },
	{ kEoBBasePatternSuffix, kStringList, true },
	{ kEoBBasePatternGrFix1, kStringList, true },
	{ kEoBBasePatternGrFix2, kStringList, true },
	{ kEoBBaseValidateArmorString, kStringList, true },
	{ kEoBBaseValidateCursedString, kStringList, true },
	{ kEoBBaseValidateNoDropString, kStringList, true },
	{ kEoBBasePotionStrings, kStringList, true },
	{ kEoBBaseWandStrings, kStringList, true },
	{ kEoBBaseItemMisuseStrings, kStringList, true },

	{ kEoBBaseTakenStrings, kStringList, true },
	{ kEoBBasePotionEffectStrings, kStringList, true },

	{ kEoBBaseYesNoStrings, kStringList, true },
	{ kRpgCommonMoreStrings, kStringList, true },
	{ kEoBBaseNpcMaxStrings, kStringList, true },
	{ kEoBBaseOkStrings, kStringList, true },
	{ kEoBBaseNpcJoinStrings, kStringList, true },
	{ kEoBBaseCancelStrings, kStringList, true },
	{ kEoBBaseAbortStrings, kStringList, true },

	{ kEoBBaseMenuStringsMain, kStringList, true },
	{ kEoBBaseMenuStringsSaveLoad, kStringList, true },
	{ kEoBBaseMenuStringsOnOff, kStringList, true },
	{ kEoBBaseMenuStringsSpells, kStringList, true },
	{ kEoBBaseMenuStringsRest, kStringList, true },
	{ kEoBBaseMenuStringsDrop, kStringList, true },
	{ kEoBBaseMenuStringsExit, kStringList, true },
	{ kEoBBaseMenuStringsStarve, kStringList, true },
	{ kEoBBaseMenuStringsScribe, kStringList, true },
	{ kEoBBaseMenuStringsDrop2, kStringList, true },
	{ kEoBBaseMenuStringsHead, kStringList, true },
	{ kEoBBaseMenuStringsPoison, kStringList, true },
	{ kEoBBaseMenuStringsMgc, kStringList, true },
	{ kEoBBaseMenuStringsPrefs, kStringList, true },
	{ kEoBBaseMenuStringsRest2, kStringList, true },
	{ kEoBBaseMenuStringsRest3, kStringList, true },
	{ kEoBBaseMenuStringsRest4, kStringList, true },
	{ kEoBBaseMenuStringsDefeat, kStringList, true },
	{ kEoBBaseMenuStringsTransfer, kStringList, true },
	{ kEoBBaseMenuStringsSpec, kStringList, true },
	{ kEoBBaseMenuStringsSpellNo, kStringList, false },
	{ kEoBBaseMenuYesNoStrings, kStringList, true },

	{ kEoBBaseSpellLevelsMage, kRawData, false },
	{ kEoBBaseSpellLevelsCleric, kRawData, false },
	{ kEoBBaseNumSpellsCleric, kRawData, false },
	{ kEoBBaseNumSpellsWisAdj, kRawData, false },
	{ kEoBBaseNumSpellsPal, kRawData, false },
	{ kEoBBaseNumSpellsMage, kRawData, false },

	{ kEoBBaseCharGuiStringsHp, kStringList, true },
	{ kEoBBaseCharGuiStringsWp1, kStringList, true },
	{ kEoBBaseCharGuiStringsWp2, kStringList, true },
	{ kEoBBaseCharGuiStringsWr, kStringList, true },
	{ kEoBBaseCharGuiStringsSt1, kStringList, true },
	{ kEoBBaseCharGuiStringsSt2, kStringList, true },
	{ kEoBBaseCharGuiStringsIn, kStringList, true },

	{ kEoBBaseCharStatusStrings7, kStringList, true },
	{ kEoBBaseCharStatusStrings81, kStringList, true },
	{ kEoBBaseCharStatusStrings82, kStringList, true },
	{ kEoBBaseCharStatusStrings9, kStringList, true },
	{ kEoBBaseCharStatusStrings12, kStringList, true },
	{ kEoBBaseCharStatusStrings131, kStringList, true },
	{ kEoBBaseCharStatusStrings132, kStringList, true },

	{ kEoBBaseLevelGainStrings, kStringList, true },
	{ kEoBBaseExperienceTable0, kRawDataBe32, false },
	{ kEoBBaseExperienceTable1, kRawDataBe32, false },
	{ kEoBBaseExperienceTable2, kRawDataBe32, false },
	{ kEoBBaseExperienceTable3, kRawDataBe32, false },
	{ kEoBBaseExperienceTable4, kRawDataBe32, false },

	{ kEoBBaseWllFlagPreset, kRawData, false },
	{ kEoBBaseDscShapeCoords, kRawDataBe16, false },
	{ kRpgCommonDscDoorScaleOffs, kRawData, false },
	{ kEoBBaseDscDoorScaleMult1, kRawData, false },
	{ kEoBBaseDscDoorScaleMult2, kRawData, false },
	{ kEoBBaseDscDoorScaleMult3, kRawData, false },
	{ kEoBBaseDscDoorScaleMult4, kRawData, false },
	{ kEoBBaseDscDoorScaleMult5, kRawData, false },
	{ kEoBBaseDscDoorScaleMult6, kRawData, false },
	{ kEoBBaseDscDoorType5Offs, kRawData, false },
	{ kEoBBaseDscDoorXE, kRawData, false },
	{ kEoBBaseDscDoorY1, kRawData, false },
	{ kEoBBaseDscDoorY3, kRawData, false },
	{ kEoBBaseDscDoorY4, kRawData, false },
	{ kEoBBaseDscDoorY5, kRawData, false },
	{ kEoBBaseDscDoorY6, kRawData, false },
	{ kEoBBaseDscDoorY7, kRawData, false },
	{ kEoBBaseDscDoorCoordsExt, kRawDataBe16, false },

	{ kEoBBaseDscItemPosIndex, kRawData, false },
	{ kEoBBaseDscItemShpX, kRawDataBe16, false },
	{ kEoBBaseDscItemScaleIndex, kRawData, false },
	{ kEoBBaseDscItemTileIndex, kRawData, false },
	{ kEoBBaseDscItemShapeMap, kRawData, false },
	{ kEoBBaseDscTelptrShpCoords, kRawData, false },

	{ kEoBBasePortalSeqData, kRawData, false },
	{ kEoBBaseManDef, kRawData, true },
	{ kEoBBaseManWord, kStringList, true },
	{ kEoBBaseManPrompt, kStringList, true },

	{ kEoBBaseDscMonsterFrmOffsTbl1, kRawData, false },
	{ kEoBBaseDscMonsterFrmOffsTbl2, kRawData, false },

	{ kEoBBaseInvSlotX, kRawDataBe16, false },
	{ kEoBBaseInvSlotY, kRawData, false },
	{ kEoBBaseSlotValidationFlags, kRawDataBe16, false },

	{ kEoBBaseProjectileWeaponTypes, kRawData, false },
	{ kEoBBaseWandTypes, kRawData, false },

	{ kEoBBaseDrawObjPosIndex, kRawData, false },
	{ kEoBBaseFlightObjFlipIndex, kRawData, false },
	{ kEoBBaseFlightObjShpMap, kRawData, false },
	{ kEoBBaseFlightObjSclIndex, kRawData, false },

	{ kEoBBaseBookNumbers, kStringList, true },
	{ kEoBBaseMageSpellsList, kStringList, true },
	{ kEoBBaseClericSpellsList, kStringList, true },
	{ kEoBBaseMageSpellsList2, kStringList, true },
	{ kEoBBaseClericSpellsList2, kStringList, true },
	{ kEoBBaseSpellNames, kStringList, true },

	{ kEoBBaseMagicStrings1, kStringList, true },
	{ kEoBBaseMagicStrings2, kStringList, true },
	{ kEoBBaseMagicStrings3, kStringList, true },
	{ kEoBBaseMagicStrings4, kStringList, true },
	{ kEoBBaseMagicStrings6, kStringList, true },
	{ kEoBBaseMagicStrings7, kStringList, true },
	{ kEoBBaseMagicStrings8, kStringList, true },
	{ kEoBBaseMagicStrings9, kStringList, true },

	{ kEoBBaseExpObjectTlMode, kRawData, false },
	{ kEoBBaseExpObjectTblIndex, kRawData, false },
	{ kEoBBaseExpObjectShpStart, kRawData, false },
	{ kEoBBaseExpObjectTbl1, kRawData, false },
	{ kEoBBaseExpObjectTbl2, kRawData, false },
	{ kEoBBaseExpObjectTbl3, kRawData, false },
	{ kEoBBaseExpObjectY, kRawData, false },

	{ kEoBBaseSparkDefSteps, kRawData, false },
	{ kEoBBaseSparkDefSubSteps, kRawData, false },
	{ kEoBBaseSparkDefShift, kRawData, false },
	{ kEoBBaseSparkDefAdd, kRawData, false },
	{ kEoBBaseSparkDefX, kRawData, false },
	{ kEoBBaseSparkDefY, kRawData, false },
	{ kEoBBaseSparkOfFlags1, kRawDataBe32, false },
	{ kEoBBaseSparkOfFlags2, kRawDataBe32, false },
	{ kEoBBaseSparkOfShift, kRawData, false },
	{ kEoBBaseSparkOfX, kRawData, false },
	{ kEoBBaseSparkOfY, kRawData, false },
	{ kEoBBaseSpellProperties, kRawData, false },
	{ kEoBBaseMagicFlightProps, kRawData, false },
	{ kEoBBaseTurnUndeadEffect, kRawData, false },
	{ kEoBBaseBurningHandsDest, kRawData, false },
	{ kEoBBaseConeOfColdDest1, kRawData, false },
	{ kEoBBaseConeOfColdDest2, kRawData, false },
	{ kEoBBaseConeOfColdDest3, kRawData, false },
	{ kEoBBaseConeOfColdDest4, kRawData, false },
	{ kEoBBaseConeOfColdGfxTbl, kRawData, false },

	{ kEoBBaseSoundMap, kStringList, false },
	{ kEoBBaseSoundFilesIntro, kStringList, false },
	{ kEoBBaseSoundFilesIngame, kStringList, false },
	{ kEoBBaseSoundFilesFinale, kStringList, false },
	{ kEoBBaseLevelSoundFiles1, kStringList, false },
	{ kEoBBaseLevelSoundFiles2, kStringList, false },

	// EYE OF THE BEHOLDER I
	{ kEoB1DefaultPartyStats, kRawData, false },
	{ kEoB1DefaultPartyNames, kStringList, true },
	{ kEoB1MainMenuStrings, kStringList, true },
	{ kEoB1BonusStrings, kStringList, true },

	{ kEoB1IntroFilesOpening, kStringList, false },
	{ kEoB1IntroFilesTower, kStringList, false },
	{ kEoB1IntroFilesOrb, kStringList, false },
	{ kEoB1IntroFilesWdEntry, kStringList, false },
	{ kEoB1IntroFilesKing, kStringList, false },
	{ kEoB1IntroFilesHands, kStringList, false },
	{ kEoB1IntroFilesWdExit, kStringList, false },
	{ kEoB1IntroFilesTunnel, kStringList, false },

	{ kEoB1IntroStringsTower, kStringList, true },
	{ kEoB1IntroStringsOrb, kStringList, true },
	{ kEoB1IntroStringsWdEntry, kStringList, true },
	{ kEoB1IntroStringsKing, kStringList, true },
	{ kEoB1IntroStringsHands, kStringList, true },
	{ kEoB1IntroStringsWdExit, kStringList, true },
	{ kEoB1IntroStringsTunnel, kStringList, true },

	{ kEoB1FinaleStrings, kStringList, true },

	{ kEoB1IntroOpeningFrmDelay, kRawData, false },
	{ kEoB1IntroWdEncodeX, kRawData, false },
	{ kEoB1IntroWdEncodeY, kRawData, false },
	{ kEoB1IntroWdEncodeWH, kRawData, false },
	{ kEoB1IntroWdDsX, kRawDataBe16, false },
	{ kEoB1IntroWdDsY, kRawData, false },
	{ kEoB1IntroTvlX1, kRawData, false },
	{ kEoB1IntroTvlY1, kRawData, false },
	{ kEoB1IntroTvlX2, kRawData, false },
	{ kEoB1IntroTvlY2, kRawData, false },
	{ kEoB1IntroTvlW, kRawData, false },
	{ kEoB1IntroTvlH, kRawData, false },
	{ kEoB1IntroOrbFadePal, kRawData, false },
	{ kEoB1FinaleCouncilAnim1, kRawData, false },
	{ kEoB1FinaleCouncilAnim2, kRawData, false },
	{ kEoB1FinaleCouncilAnim3, kRawData, false },
	{ kEoB1FinaleCouncilAnim4, kRawData, false },
	{ kEoB1FinaleEyesAnim, kRawData, false },
	{ kEoB1FinaleHandsAnim, kRawDataBe16, false },
	{ kEoB1FinaleHandsAnim2, kRawDataBe16, false },
	{ kEoB1FinaleHandsAnim3, kRawData, false },
	{ kEoB1FinaleTextDuration, kRawData, false },
	{ kEoB1CreditsStrings, kRawData, true },
	{ kEoB1CreditsCharWdth, kRawData, false },
	{ kEoB1CreditsStrings2, kStringList, true },
	{ kEoB1CreditsTileGrid, kRawData, false },
	{ kEoB1DoorShapeDefs, kRawData, false },
	{ kEoB1DoorSwitchShapeDefs, kRawData, false },
	{ kEoB1DoorSwitchCoords, kRawData, false },
	{ kEoB1MonsterProperties, kRawData, false },
	{ kEoB1EnemyMageSpellList, kRawData, false },
	{ kEoB1EnemyMageSfx, kRawData, false },
	{ kEoB1BeholderSpellList, kRawData, false },
	{ kEoB1BeholderSfx, kRawData, false },
	{ kEoB1TurnUndeadString, kStringList, true },

	{ kEoB1CgaMappingDefault, kRawData, false },
	{ kEoB1CgaMappingAlt, kRawData, false },
	{ kEoB1CgaMappingInv, kRawData, false },
	{ kEoB1CgaMappingItemsL, kRawData, false },
	{ kEoB1CgaMappingItemsS, kRawData, false },
	{ kEoB1CgaMappingThrown, kRawData, false },
	{ kEoB1CgaMappingIcons, kRawData, false },
	{ kEoB1CgaMappingDeco, kRawData, false },
	{ kEoB1CgaLevelMappingIndex, kRawData, false },
	{ kEoB1CgaMappingLevel0, kRawData, false },
	{ kEoB1CgaMappingLevel1, kRawData, false },
	{ kEoB1CgaMappingLevel2, kRawData, false },
	{ kEoB1CgaMappingLevel3, kRawData, false },
	{ kEoB1CgaMappingLevel4, kRawData, false },

	{ kEoB1Palettes16c, kRawData, false },
	{ kEoB1PalCycleData, kRawData, false },
	{ kEoB1PalCycleStyle1, kRawDataBe16, false },
	{ kEoB1PalCycleStyle2, kRawDataBe16, false },
	{ kEoB1PalettesSega, kRawDataBe16, false },
	{ kEoB1PatternTable0, kRawDataBe16, false },
	{ kEoB1PatternTable1, kRawDataBe16, false },
	{ kEoB1PatternTable2, kRawDataBe16, false },
	{ kEoB1PatternTable3, kRawDataBe16, false },
	{ kEoB1PatternTable4, kRawDataBe16, false },
	{ kEoB1PatternTable5, kRawDataBe16, false },
	{ kEoB1PatternAddTable1, kRawDataBe16, false },
	{ kEoB1PatternAddTable2, kRawDataBe16, false },

	{ kEoB1NpcShpData, kRawData, false },
	{ kEoB1NpcSubShpIndex1, kRawData, false },
	{ kEoB1NpcSubShpIndex2, kRawData, false },
	{ kEoB1NpcSubShpY, kRawData, false },
	{ kEoB1Npc0Strings, kStringList, true },
	{ kEoB1Npc11Strings, kStringList, true },
	{ kEoB1Npc12Strings, kStringList, true },
	{ kEoB1Npc21Strings, kStringList, true },
	{ kEoB1Npc22Strings, kStringList, true },
	{ kEoB1Npc31Strings, kStringList, true },
	{ kEoB1Npc32Strings, kStringList, true },
	{ kEoB1Npc4Strings, kStringList, true },
	{ kEoB1Npc5Strings, kStringList, true },
	{ kEoB1Npc6Strings, kStringList, true },
	{ kEoB1Npc7Strings, kStringList, true },

	{ kEoB1ParchmentStrings, kStringList, true },
	{ kEoB1ItemNames, kStringList, true },
	{ kEoB1SpeechAnimData, kRawData, false },
	{ kEoB1WdAnimSprites, kRawData, false },
	{ kEoB1SequenceTrackMap, kRawData, false },

	{ kEoB1MapStrings1, kStringList, true },
	{ kEoB1MapStrings2, kStringList, true },
	{ kEoB1MapStrings3, kStringList, true },
	{ kEoB1MapLevelData, kRawData, false },

	{ kEoB1Ascii2SjisTable1, kRawDataBe16, false },
	{ kEoB1Ascii2SjisTable2, kRawDataBe16, false },
	{ kEoB1FontLookupTable, kRawData, false },
	{ kEoB1CharWidthTable1, kRawData, false },
	{ kEoB1CharWidthTable2, kRawData, false },
	{ kEoB1CharWidthTable3, kRawData, false },
	{ kEoB1CharTilesTable, kRawData, true },

	// EYE OF THE BEHOLDER II
	{ kEoB2MainMenuStrings, kStringList, true },
	{ kEoB2MainMenuUtilStrings, kStringList, true },

	{ kEoB2TransferPortraitFrames, kRawDataBe16, false },
	{ kEoB2TransferConvertTable, kRawData, false },
	{ kEoB2TransferItemTable, kRawData, false },
	{ kEoB2TransferExpTable, kRawDataBe32, false },
	{ kEoB2TransferStrings1, kStringList, true },
	{ kEoB2TransferStrings2, kStringList, true },
	{ kEoB2TransferLabels, kStringList, true },

	{ kEoB2IntroStrings, kStringList, true },
	{ kEoB2IntroCPSFiles, kStringList, true },
	{ kEoB2IntroAnimData00, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData01, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData02, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData03, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData04, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData05, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData06, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData07, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData08, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData09, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData10, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData11, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData12, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData13, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData14, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData15, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData16, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData17, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData18, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData19, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData20, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData21, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData22, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData23, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData24, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData25, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData26, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData27, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData28, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData29, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData30, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData31, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData32, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData33, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData34, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData35, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData36, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData37, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData38, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData39, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData40, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData41, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData42, kEoB2SequenceData, false },
	{ kEoB2IntroAnimData43, kEoB2SequenceData, false },
	{ kEoB2IntroShapes00, kEoB2ShapeData, false },
	{ kEoB2IntroShapes01, kEoB2ShapeData, false },
	{ kEoB2IntroShapes04, kEoB2ShapeData, false },
	{ kEoB2IntroShapes07, kEoB2ShapeData, false },
	{ kEoB2IntroShapes13, kEoB2ShapeData, false },
	{ kEoB2IntroShapes14, kEoB2ShapeData, false },
	{ kEoB2IntroShapes15, kEoB2ShapeData, false },
	{ kEoB2IntroCpsDataStreet1, kRawData, false },
	{ kEoB2IntroCpsDataStreet2, kRawData, false },
	{ kEoB2IntroCpsDataDoorway1, kRawData, false },
	{ kEoB2IntroCpsDataDoorway2, kRawData, false },
	{ kEoB2IntroCpsDataWestwood, kRawData, false },
	{ kEoB2IntroCpsDataWinding, kRawData, false },
	{ kEoB2IntroCpsDataKhelban2, kRawData, false },
	{ kEoB2IntroCpsDataKhelban1, kRawData, false },
	{ kEoB2IntroCpsDataKhelban3, kRawData, false },
	{ kEoB2IntroCpsDataKhelban4, kRawData, false },
	{ kEoB2IntroCpsDataCoin, kRawData, false },
	{ kEoB2IntroCpsDataKhelban5, kRawData, false },
	{ kEoB2IntroCpsDataKhelban6, kRawData, false },

	{ kEoB2FinaleStrings, kStringList, true },
	{ kEoB2CreditsData, kRawData, true },
	{ kEoB2FinaleCPSFiles, kStringList, true },
	{ kEoB2FinaleAnimData00, kEoB2SequenceData, false },
	{ kEoB2FinaleAnimData01, kEoB2SequenceData, false },
	{ kEoB2FinaleAnimData02, kEoB2SequenceData, false },
	{ kEoB2FinaleAnimData03, kEoB2SequenceData, false },
	{ kEoB2FinaleAnimData04, kEoB2SequenceData, false },
	{ kEoB2FinaleAnimData05, kEoB2SequenceData, false },
	{ kEoB2FinaleAnimData06, kEoB2SequenceData, false },
	{ kEoB2FinaleAnimData07, kEoB2SequenceData, false },
	{ kEoB2FinaleAnimData08, kEoB2SequenceData, false },
	{ kEoB2FinaleAnimData09, kEoB2SequenceData, false },
	{ kEoB2FinaleAnimData10, kEoB2SequenceData, false },
	{ kEoB2FinaleAnimData11, kEoB2SequenceData, false },
	{ kEoB2FinaleAnimData12, kEoB2SequenceData, false },
	{ kEoB2FinaleAnimData13, kEoB2SequenceData, false },
	{ kEoB2FinaleAnimData14, kEoB2SequenceData, false },
	{ kEoB2FinaleAnimData15, kEoB2SequenceData, false },
	{ kEoB2FinaleAnimData16, kEoB2SequenceData, false },
	{ kEoB2FinaleAnimData17, kEoB2SequenceData, false },
	{ kEoB2FinaleAnimData18, kEoB2SequenceData, false },
	{ kEoB2FinaleAnimData19, kEoB2SequenceData, false },
	{ kEoB2FinaleAnimData20, kEoB2SequenceData, false },
	{ kEoB2FinaleShapes00, kEoB2ShapeData, false },
	{ kEoB2FinaleShapes03, kEoB2ShapeData, false },
	{ kEoB2FinaleShapes07, kEoB2ShapeData, false },
	{ kEoB2FinaleShapes09, kEoB2ShapeData, false },
	{ kEoB2FinaleShapes10, kEoB2ShapeData, false },
	{ kEoB2FinaleCpsDataDragon1, kRawData, false },
	{ kEoB2FinaleCpsDataDragon2, kRawData, false },
	{ kEoB2FinaleCpsDataHurry1, kRawData, false },
	{ kEoB2FinaleCpsDataHurry2, kRawData, false },
	{ kEoB2FinaleCpsDataDestroy0, kRawData, false },
	{ kEoB2FinaleCpsDataDestroy1, kRawData, false },
	{ kEoB2FinaleCpsDataDestroy2, kRawData, false },
	{ kEoB2FinaleCpsDataMagic, kRawData, false },
	{ kEoB2FinaleCpsDataDestroy3, kRawData, false },
	{ kEoB2FinaleCpsDataCredits2, kRawData, false },
	{ kEoB2FinaleCpsDataCredits3, kRawData, false },
	{ kEoB2FinaleCpsDataHeroes, kRawData, false },
	{ kEoB2FinaleCpsDataThanks, kRawData, false },

	{ kEoB2NpcShapeData, kRawData, false },
	{ kEoBBaseClassModifierFlags, kRawData, false },
	{ kEoBBaseMonsterStepTable01, kRawData, false },
	{ kEoBBaseMonsterStepTable02, kRawData, false },
	{ kEoBBaseMonsterStepTable1, kRawData, false },
	{ kEoBBaseMonsterStepTable2, kRawData, false },
	{ kEoBBaseMonsterStepTable3, kRawData, false },
	{ kEoBBaseMonsterCloseAttPosTable1, kRawData, false },
	{ kEoBBaseMonsterCloseAttPosTable21, kRawData, false },
	{ kEoBBaseMonsterCloseAttPosTable22, kRawData, false },
	{ kEoBBaseMonsterCloseAttUnkTable, kRawData, false },
	{ kEoBBaseMonsterCloseAttChkTable1, kRawData, false },
	{ kEoBBaseMonsterCloseAttChkTable2, kRawData, false },
	{ kEoBBaseMonsterCloseAttDstTable1, kRawData, false },
	{ kEoBBaseMonsterCloseAttDstTable2, kRawData, false },
	{ kEoBBaseMonsterProximityTable, kRawData, false },
	{ kEoBBaseFindBlockMonstersTable, kRawData, false },
	{ kEoBBaseMonsterDirChangeTable, kRawData, false },
	{ kEoBBaseMonsterDistAttStrings, kStringList, true },
	{ kEoBBaseEncodeMonsterDefs, kRawDataBe16, false },
	{ kEoBBaseEncodeMonsterDefs00, kRawData, false },
	{ kEoBBaseEncodeMonsterDefs01, kRawData, false },
	{ kEoBBaseEncodeMonsterDefs02, kRawData, false },
	{ kEoBBaseEncodeMonsterDefs03, kRawData, false },
	{ kEoBBaseEncodeMonsterDefs04, kRawData, false },
	{ kEoBBaseEncodeMonsterDefs05, kRawData, false },
	{ kEoBBaseEncodeMonsterDefs06, kRawData, false },
	{ kEoBBaseEncodeMonsterDefs07, kRawData, false },
	{ kEoBBaseEncodeMonsterDefs08, kRawData, false },
	{ kEoBBaseEncodeMonsterDefs09, kRawData, false },
	{ kEoBBaseEncodeMonsterDefs10, kRawData, false },
	{ kEoBBaseEncodeMonsterDefs11, kRawData, false },
	{ kEoBBaseEncodeMonsterDefs12, kRawData, false },
	{ kEoBBaseEncodeMonsterDefs13, kRawData, false },
	{ kEoBBaseEncodeMonsterDefs14, kRawData, false },
	{ kEoBBaseEncodeMonsterDefs15, kRawData, false },
	{ kEoBBaseEncodeMonsterDefs16, kRawData, false },
	{ kEoBBaseEncodeMonsterDefs17, kRawData, false },
	{ kEoBBaseEncodeMonsterDefs18, kRawData, false },
	{ kEoBBaseEncodeMonsterDefs19, kRawData, false },
	{ kEoBBaseEncodeMonsterDefs20, kRawData, false },
	{ kEoBBaseEncodeMonsterDefs21, kRawData, false },

	{ kEoB1MonsterAnimFrames00, kRawData, false },
	{ kEoB1MonsterAnimFrames01, kRawData, false },
	{ kEoB1MonsterAnimFrames02, kRawData, false },
	{ kEoB1MonsterAnimFrames03, kRawData, false },
	{ kEoB1MonsterAnimFrames04, kRawData, false },
	{ kEoB1MonsterAnimFrames05, kRawData, false },
	{ kEoB1MonsterAnimFrames06, kRawData, false },
	{ kEoB1MonsterAnimFrames07, kRawData, false },
	{ kEoB1MonsterAnimFrames08, kRawData, false },
	{ kEoB1MonsterAnimFrames09, kRawData, false },
	{ kEoB1MonsterAnimFrames10, kRawData, false },
	{ kEoB1MonsterAnimFrames11, kRawData, false },
	{ kEoB1MonsterAnimFrames12, kRawData, false },
	{ kEoB1MonsterAnimFrames13, kRawData, false },
	{ kEoB1MonsterAnimFrames14, kRawData, false },
	{ kEoB1MonsterAnimFrames15, kRawData, false },
	{ kEoB1MonsterAnimFrames16, kRawData, false },
	{ kEoB1MonsterAnimFrames17, kRawData, false },
	{ kEoB1MonsterAnimFrames18, kRawData, false },
	{ kEoB1MonsterAnimFrames19, kRawData, false },
	{ kEoB1MonsterAnimFrames20, kRawData, false },
	{ kEoB1MonsterAnimFrames21, kRawData, false },
	{ kEoB1MonsterAnimFrames22, kRawData, false },
	{ kEoB1MonsterAnimFrames23, kRawData, false },
	{ kEoB1MonsterAnimFrames24, kRawData, false },
	{ kEoB1MonsterAnimFrames25, kRawData, false },
	{ kEoB1MonsterAnimFrames26, kRawData, false },
	{ kEoB1MonsterAnimFrames27, kRawData, false },
	{ kEoB1MonsterAnimFrames28, kRawData, false },
	{ kEoB1MonsterAnimFrames29, kRawData, false },
	{ kEoB1MonsterAnimFrames30, kRawData, false },
	{ kEoB1MonsterAnimFrames31, kRawData, false },
	{ kEoB1MonsterAnimFrames32, kRawData, false },
	{ kEoB1MonsterAnimFrames33, kRawData, false },
	{ kEoB1MonsterAnimFrames34, kRawData, false },
	{ kEoB1MonsterAnimFrames35, kRawData, false },
	{ kEoB1MonsterAnimFrames36, kRawData, false },
	{ kEoB1MonsterAnimFrames37, kRawData, false },
	{ kEoB1MonsterAnimFrames38, kRawData, false },
	{ kEoB1MonsterAnimFrames39, kRawData, false },
	{ kEoB1MonsterAnimFrames40, kRawData, false },
	{ kEoB1MonsterAnimFrames41, kRawData, false },
	{ kEoB1MonsterAnimFrames42, kRawData, false },
	{ kEoB1MonsterAnimFrames43, kRawData, false },
	{ kEoB1MonsterAnimFrames44, kRawData, false },
	{ kEoB1MonsterAnimFrames45, kRawData, false },
	{ kEoB1MonsterAnimFrames46, kRawData, false },
	{ kEoB1MonsterAnimFrames47, kRawData, false },
	{ kEoB1MonsterAnimFrames48, kRawData, false },
	{ kEoB1MonsterAnimFrames49, kRawData, false },
	{ kEoB1MonsterAnimFrames50, kRawData, false },
	{ kEoB1MonsterAnimFrames51, kRawData, false },
	{ kEoB1MonsterAnimFrames52, kRawData, false },
	{ kEoB1MonsterAnimFrames53, kRawData, false },
	{ kEoB1MonsterAnimFrames54, kRawData, false },
	{ kEoB1MonsterAnimFrames55, kRawData, false },
	{ kEoB1MonsterAnimFrames56, kRawData, false },
	{ kEoB1MonsterAnimFrames57, kRawData, false },
	{ kEoB1MonsterAnimFrames58, kRawData, false },
	{ kEoB1MonsterAnimFrames59, kRawData, false },
	{ kEoB1MonsterAnimFrames60, kRawData, false },
	{ kEoB1MonsterAnimFrames61, kRawData, false },
	{ kEoB1MonsterAnimFrames62, kRawData, false },
	{ kEoB1MonsterAnimFrames63, kRawData, false },
	{ kEoB1MonsterAnimFrames64, kRawData, false },
	{ kEoB1MonsterAnimFrames65, kRawData, false },
	{ kEoB1MonsterAnimFrames66, kRawData, false },
	{ kEoB1MonsterAnimFrames67, kRawData, false },
	{ kEoB1MonsterAnimFrames68, kRawData, false },
	{ kEoB1MonsterAnimFrames69, kRawData, false },
	{ kEoB1MonsterAnimFrames70, kRawData, false },
	{ kEoB1MonsterAnimFrames71, kRawData, false },
	{ kEoB1MonsterAnimFrames72, kRawData, false },
	{ kEoB1MonsterAnimFrames73, kRawData, false },
	{ kEoB1MonsterAnimFrames74, kRawData, false },
	{ kEoB1MonsterAnimFrames75, kRawData, false },
	{ kEoB1MonsterAnimFrames76, kRawData, false },
	{ kEoB1MonsterAnimFrames77, kRawData, false },
	{ kEoB1MonsterAnimFrames78, kRawData, false },
	{ kEoB1MonsterAnimFrames79, kRawData, false },
	{ kEoB1MonsterAnimFrames80, kRawData, false },
	{ kEoB1MonsterAnimFrames81, kRawData, false },
	{ kEoB1MonsterAnimFrames82, kRawData, false },
	{ kEoB1MonsterAnimFrames83, kRawData, false },
	{ kEoB1MonsterAnimFrames84, kRawData, false },
	{ kEoB1MonsterAnimFrames85, kRawData, false },
	{ kEoB1MonsterAnimFrames86, kRawData, false },
	{ kEoB1MonsterAnimFrames87, kRawData, false },
	{ kEoB1MonsterAnimFrames88, kRawData, false },
	{ kEoB1MonsterAnimFrames89, kRawData, false },
	{ kEoB1MonsterAnimFrames90, kRawData, false },
	{ kEoB1MonsterAnimFrames91, kRawData, false },
	{ kEoB1MonsterAnimFrames92, kRawData, false },
	{ kEoB1MonsterAnimFrames93, kRawData, false },
	{ kEoB1MonsterAnimFrames94, kRawData, false },
	{ kEoB1MonsterAnimFrames95, kRawData, false },
	{ kEoB1MonsterAnimFrames96, kRawData, false },
	{ kEoB1MonsterAnimFrames97, kRawData, false },
	{ kEoB1MonsterAnimFrames98, kRawData, false },
	{ kEoB1MonsterAnimFrames99, kRawData, false },
	{ kEoB1MonsterAnimFrames100, kRawData, false },
	{ kEoB1MonsterAnimFrames101, kRawData, false },
	{ kEoB1MonsterAnimFrames102, kRawData, false },
	{ kEoB1MonsterAnimFrames103, kRawData, false },
	{ kEoB1MonsterAnimFrames104, kRawData, false },
	{ kEoB1MonsterAnimFrames105, kRawData, false },
	{ kEoB1MonsterAnimFrames106, kRawData, false },
	{ kEoB1MonsterAnimFrames107, kRawData, false },
	{ kEoB1MonsterAnimFrames108, kRawData, false },
	{ kEoB1MonsterAnimFrames109, kRawData, false },

	{ kEoBBaseNpcPresets, kEoBNpcData, false },
	{ kEoBBaseNpcPresetsNames, kStringList, true },
	{ kEoB2Npc1Strings, kStringList, true },
	{ kEoB2Npc2Strings, kStringList, true },
	{ kEoB2MonsterDustStrings, kStringList, true },
	{ kEoB2DreamSteps, kRawData, false },
	{ kEoB2KheldranStrings, kStringList, true },
	{ kEoB2HornStrings, kStringList, true },
	{ kEoB2HornSounds, kRawData, false },
	{ kEoB2WallOfForceDsX, kRawDataBe16, false },
	{ kEoB2WallOfForceDsY, kRawData, false },
	{ kEoB2WallOfForceNumW, kRawData, false },
	{ kEoB2WallOfForceNumH, kRawData, false },
	{ kEoB2WallOfForceShpId, kRawData, false },

	{ kEoB2ItemIconShapeData00, kRawData, false },
	{ kEoB2ItemIconShapeData01, kRawData, false },
	{ kEoB2ItemIconShapeData02, kRawData, false },
	{ kEoB2ItemIconShapeData03, kRawData, false },
	{ kEoB2ItemIconShapeData04, kRawData, false },
	{ kEoB2ItemIconShapeData05, kRawData, false },
	{ kEoB2ItemIconShapeData06, kRawData, false },
	{ kEoB2ItemIconShapeData07, kRawData, false },
	{ kEoB2ItemIconShapeData08, kRawData, false },
	{ kEoB2ItemIconShapeData09, kRawData, false },
	{ kEoB2ItemIconShapeData10, kRawData, false },
	{ kEoB2ItemIconShapeData11, kRawData, false },
	{ kEoB2ItemIconShapeData12, kRawData, false },
	{ kEoB2ItemIconShapeData13, kRawData, false },
	{ kEoB2ItemIconShapeData14, kRawData, false },
	{ kEoB2ItemIconShapeData15, kRawData, false },
	{ kEoB2ItemIconShapeData16, kRawData, false },
	{ kEoB2ItemIconShapeData17, kRawData, false },
	{ kEoB2ItemIconShapeData18, kRawData, false },
	{ kEoB2ItemIconShapeData19, kRawData, false },
	{ kEoB2ItemIconShapeData20, kRawData, false },
	{ kEoB2ItemIconShapeData21, kRawData, false },
	{ kEoB2ItemIconShapeData22, kRawData, false },
	{ kEoB2ItemIconShapeData23, kRawData, false },
	{ kEoB2ItemIconShapeData24, kRawData, false },
	{ kEoB2ItemIconShapeData25, kRawData, false },
	{ kEoB2ItemIconShapeData26, kRawData, false },
	{ kEoB2ItemIconShapeData27, kRawData, false },
	{ kEoB2ItemIconShapeData28, kRawData, false },
	{ kEoB2ItemIconShapeData29, kRawData, false },
	{ kEoB2ItemIconShapeData30, kRawData, false },
	{ kEoB2ItemIconShapeData31, kRawData, false },
	{ kEoB2ItemIconShapeData32, kRawData, false },
	{ kEoB2ItemIconShapeData33, kRawData, false },
	{ kEoB2ItemIconShapeData34, kRawData, false },
	{ kEoB2ItemIconShapeData35, kRawData, false },
	{ kEoB2ItemIconShapeData36, kRawData, false },
	{ kEoB2ItemIconShapeData37, kRawData, false },
	{ kEoB2ItemIconShapeData38, kRawData, false },
	{ kEoB2ItemIconShapeData39, kRawData, false },
	{ kEoB2ItemIconShapeData40, kRawData, false },
	{ kEoB2ItemIconShapeData41, kRawData, false },
	{ kEoB2ItemIconShapeData42, kRawData, false },
	{ kEoB2ItemIconShapeData43, kRawData, false },
	{ kEoB2ItemIconShapeData44, kRawData, false },
	{ kEoB2ItemIconShapeData45, kRawData, false },
	{ kEoB2ItemIconShapeData46, kRawData, false },
	{ kEoB2ItemIconShapeData47, kRawData, false },
	{ kEoB2ItemIconShapeData48, kRawData, false },
	{ kEoB2ItemIconShapeData49, kRawData, false },
	{ kEoB2ItemIconShapeData50, kRawData, false },
	{ kEoB2ItemIconShapeData51, kRawData, false },
	{ kEoB2ItemIconShapeData52, kRawData, false },
	{ kEoB2ItemIconShapeData53, kRawData, false },
	{ kEoB2ItemIconShapeData54, kRawData, false },
	{ kEoB2ItemIconShapeData55, kRawData, false },
	{ kEoB2ItemIconShapeData56, kRawData, false },
	{ kEoB2ItemIconShapeData57, kRawData, false },
	{ kEoB2ItemIconShapeData58, kRawData, false },
	{ kEoB2ItemIconShapeData59, kRawData, false },
	{ kEoB2ItemIconShapeData60, kRawData, false },
	{ kEoB2ItemIconShapeData61, kRawData, false },
	{ kEoB2ItemIconShapeData62, kRawData, false },
	{ kEoB2ItemIconShapeData63, kRawData, false },
	{ kEoB2ItemIconShapeData64, kRawData, false },
	{ kEoB2ItemIconShapeData65, kRawData, false },
	{ kEoB2ItemIconShapeData66, kRawData, false },
	{ kEoB2ItemIconShapeData67, kRawData, false },
	{ kEoB2ItemIconShapeData68, kRawData, false },
	{ kEoB2ItemIconShapeData69, kRawData, false },
	{ kEoB2ItemIconShapeData70, kRawData, false },
	{ kEoB2ItemIconShapeData71, kRawData, false },
	{ kEoB2ItemIconShapeData72, kRawData, false },
	{ kEoB2ItemIconShapeData73, kRawData, false },
	{ kEoB2ItemIconShapeData74, kRawData, false },
	{ kEoB2ItemIconShapeData75, kRawData, false },
	{ kEoB2ItemIconShapeData76, kRawData, false },
	{ kEoB2ItemIconShapeData77, kRawData, false },
	{ kEoB2ItemIconShapeData78, kRawData, false },
	{ kEoB2ItemIconShapeData79, kRawData, false },
	{ kEoB2ItemIconShapeData80, kRawData, false },
	{ kEoB2ItemIconShapeData81, kRawData, false },
	{ kEoB2ItemIconShapeData82, kRawData, false },
	{ kEoB2ItemIconShapeData83, kRawData, false },
	{ kEoB2ItemIconShapeData84, kRawData, false },
	{ kEoB2ItemIconShapeData85, kRawData, false },
	{ kEoB2ItemIconShapeData86, kRawData, false },
	{ kEoB2ItemIconShapeData87, kRawData, false },
	{ kEoB2ItemIconShapeData88, kRawData, false },
	{ kEoB2ItemIconShapeData89, kRawData, false },
	{ kEoB2ItemIconShapeData90, kRawData, false },
	{ kEoB2ItemIconShapeData91, kRawData, false },
	{ kEoB2ItemIconShapeData92, kRawData, false },
	{ kEoB2ItemIconShapeData93, kRawData, false },
	{ kEoB2ItemIconShapeData94, kRawData, false },
	{ kEoB2ItemIconShapeData95, kRawData, false },
	{ kEoB2ItemIconShapeData96, kRawData, false },
	{ kEoB2ItemIconShapeData97, kRawData, false },
	{ kEoB2ItemIconShapeData98, kRawData, false },
	{ kEoB2ItemIconShapeData99, kRawData, false },
	{ kEoB2ItemIconShapeData100, kRawData, false },
	{ kEoB2ItemIconShapeData101, kRawData, false },
	{ kEoB2ItemIconShapeData102, kRawData, false },
	{ kEoB2ItemIconShapeData103, kRawData, false },
	{ kEoB2ItemIconShapeData104, kRawData, false },
	{ kEoB2ItemIconShapeData105, kRawData, false },
	{ kEoB2ItemIconShapeData106, kRawData, false },
	{ kEoB2ItemIconShapeData107, kRawData, false },
	{ kEoB2ItemIconShapeData108, kRawData, false },
	{ kEoB2ItemIconShapeData109, kRawData, false },
	{ kEoB2ItemIconShapeData110, kRawData, false },
	{ kEoB2ItemIconShapeData111, kRawData, false },

	{ kEoB2LargeItemsShapeData00, kRawData, false },
	{ kEoB2LargeItemsShapeData01, kRawData, false },
	{ kEoB2LargeItemsShapeData02, kRawData, false },
	{ kEoB2LargeItemsShapeData03, kRawData, false },
	{ kEoB2LargeItemsShapeData04, kRawData, false },
	{ kEoB2LargeItemsShapeData05, kRawData, false },
	{ kEoB2LargeItemsShapeData06, kRawData, false },
	{ kEoB2LargeItemsShapeData07, kRawData, false },
	{ kEoB2LargeItemsShapeData08, kRawData, false },
	{ kEoB2LargeItemsShapeData09, kRawData, false },
	{ kEoB2LargeItemsShapeData10, kRawData, false },

	{ kEoB2SmallItemsShapeData00, kRawData, false },
	{ kEoB2SmallItemsShapeData01, kRawData, false },
	{ kEoB2SmallItemsShapeData02, kRawData, false },
	{ kEoB2SmallItemsShapeData03, kRawData, false },
	{ kEoB2SmallItemsShapeData04, kRawData, false },
	{ kEoB2SmallItemsShapeData05, kRawData, false },
	{ kEoB2SmallItemsShapeData06, kRawData, false },
	{ kEoB2SmallItemsShapeData07, kRawData, false },
	{ kEoB2SmallItemsShapeData08, kRawData, false },
	{ kEoB2SmallItemsShapeData09, kRawData, false },
	{ kEoB2SmallItemsShapeData10, kRawData, false },
	{ kEoB2SmallItemsShapeData11, kRawData, false },
	{ kEoB2SmallItemsShapeData12, kRawData, false },
	{ kEoB2SmallItemsShapeData13, kRawData, false },
	{ kEoB2SmallItemsShapeData14, kRawData, false },
	{ kEoB2SmallItemsShapeData15, kRawData, false },
	{ kEoB2SmallItemsShapeData16, kRawData, false },
	{ kEoB2SmallItemsShapeData17, kRawData, false },
	{ kEoB2SmallItemsShapeData18, kRawData, false },
	{ kEoB2SmallItemsShapeData19, kRawData, false },
	{ kEoB2SmallItemsShapeData20, kRawData, false },
	{ kEoB2SmallItemsShapeData21, kRawData, false },
	{ kEoB2SmallItemsShapeData22, kRawData, false },
	{ kEoB2SmallItemsShapeData23, kRawData, false },
	{ kEoB2SmallItemsShapeData24, kRawData, false },
	{ kEoB2SmallItemsShapeData25, kRawData, false },

	{ kEoB2ThrownShapeData00, kRawData, false },
	{ kEoB2ThrownShapeData01, kRawData, false },
	{ kEoB2ThrownShapeData02, kRawData, false },
	{ kEoB2ThrownShapeData03, kRawData, false },
	{ kEoB2ThrownShapeData04, kRawData, false },
	{ kEoB2ThrownShapeData05, kRawData, false },
	{ kEoB2ThrownShapeData06, kRawData, false },
	{ kEoB2ThrownShapeData07, kRawData, false },
	{ kEoB2ThrownShapeData08, kRawData, false },

	{ kEoB2SpellShapeData00, kRawData, false },
	{ kEoB2SpellShapeData01, kRawData, false },
	{ kEoB2SpellShapeData02, kRawData, false },
	{ kEoB2SpellShapeData03, kRawData, false },

	{ kEoB2TeleporterShapeData00, kRawData, false },
	{ kEoB2TeleporterShapeData01, kRawData, false },
	{ kEoB2TeleporterShapeData02, kRawData, false },
	{ kEoB2TeleporterShapeData03, kRawData, false },
	{ kEoB2TeleporterShapeData04, kRawData, false },
	{ kEoB2TeleporterShapeData05, kRawData, false },

	{ kEoB2LightningColumnShapeData, kRawData, false },
	{ kEoB2DeadCharShapeData, kRawData, false },
	{ kEoB2DisabledCharGridShapeData, kRawData, false },
	{ kEoB2WeaponSlotGridShapeData, kRawData, false },
	{ kEoB2SmallGridShapeData, kRawData, false },
	{ kEoB2WideGridShapeData, kRawData, false },
	{ kEoB2RedSplatShapeData, kRawData, false },
	{ kEoB2GreenSplatShapeData, kRawData, false },

	{ kEoB2FirebeamShapeData00, kRawData, false },
	{ kEoB2FirebeamShapeData01, kRawData, false },
	{ kEoB2FirebeamShapeData02, kRawData, false },

	{ kEoB2SparkShapeData00, kRawData, false },
	{ kEoB2SparkShapeData01, kRawData, false },
	{ kEoB2SparkShapeData02, kRawData, false },

	{ kEoB2CompassShapeData00, kRawData, false },
	{ kEoB2CompassShapeData01, kRawData, false },
	{ kEoB2CompassShapeData02, kRawData, false },
	{ kEoB2CompassShapeData03, kRawData, false },
	{ kEoB2CompassShapeData04, kRawData, false },
	{ kEoB2CompassShapeData05, kRawData, false },
	{ kEoB2CompassShapeData06, kRawData, false },
	{ kEoB2CompassShapeData07, kRawData, false },
	{ kEoB2CompassShapeData08, kRawData, false },
	{ kEoB2CompassShapeData09, kRawData, false },
	{ kEoB2CompassShapeData10, kRawData, false },
	{ kEoB2CompassShapeData11, kRawData, false },

	{ kEoB2WallOfForceShapeData00, kRawData, false },
	{ kEoB2WallOfForceShapeData01, kRawData, false },
	{ kEoB2WallOfForceShapeData02, kRawData, false },
	{ kEoB2WallOfForceShapeData03, kRawData, false },
	{ kEoB2WallOfForceShapeData04, kRawData, false },
	{ kEoB2WallOfForceShapeData05, kRawData, false },

	{ kEoB2UtilMenuStrings, kStringList, true },
	{ kEoB2Config2431Strings, kStringList, true },
	{ kEoBBaseTextInputCharacterLines, kStringList, true },
	{ kEoBBaseTextInputSelectStrings, kStringList, true },
	{ kEoB2FontDmpSearchTbl, kRawDataBe16, false },
	{ kEoB2Ascii2SjisTables, kStringList, false },
	{ kEoB2Ascii2SjisTables2, kStringList, false },
	{ kEoBBaseSaveNamePatterns, kStringList, true },

	{ kEoB2PcmSoundEffectsIngame, kRawData, false },
	{ kEoB2PcmSoundEffectsIntro, kRawData, false },
	{ kEoB2PcmSoundEffectsFinale, kRawData, false },

	{ kEoB2SoundMapExtra, kStringList, false },
	{ kEoB2SoundIndex1, kRawData, false },
	{ kEoB2SoundIndex2, kRawData, false },
	{ kEoB2SoundFilesIngame2, kStringList, false },
	{ kEoB2MonsterSoundPatchData, kRawData, false },

	// LANDS OF LORE

	// Ingame
	{ kLoLIngamePakFiles, kStringList, false },

	{ kLoLCharacterDefs, kLoLCharData, true },
	{ kLoLIngameSfxFiles, kStringList, false },
	{ kLoLIngameSfxIndex, kRawDataBe16, false },
	{ kLoLMusicTrackMap, kRawData, false },
	{ kLoLIngameGMSfxIndex, kRawData, false },
	{ kLoLIngameMT32SfxIndex, kRawData, false },
	{ kLoLIngamePcSpkSfxIndex, kRawData, false },
	{ kLoLSpellProperties, kLoLSpellData, false },
	{ kLoLGameShapeMap, kRawData, false },
	{ kLoLSceneItemOffs, kRawData, false },
	{ kLoLCharInvIndex, kRawData, false },
	{ kLoLCharInvDefs, kRawData, false },
	{ kLoLCharDefsMan, kRawDataBe16, false },
	{ kLoLCharDefsWoman, kRawDataBe16, false },
	{ kLoLCharDefsKieran, kRawDataBe16, false },
	{ kLoLCharDefsAkshel, kRawDataBe16, false },
	{ kLoLExpRequirements, kRawDataBe32, false },
	{ kLoLMonsterModifiers1, kRawDataBe16, false },
	{ kLoLMonsterModifiers2, kRawDataBe16, false },
	{ kLoLMonsterModifiers3, kRawDataBe16, false },
	{ kLoLMonsterModifiers4, kRawDataBe16, false },
	{ kLoLMonsterShiftOffsets, kRawData, false },
	{ kLoLMonsterDirFlags, kRawData, false },
	{ kLoLMonsterScaleY, kRawData, false },
	{ kLoLMonsterScaleX, kRawData, false },
	{ kLoLMonsterScaleWH, kRawDataBe16, false },
	{ kLoLFlyingObjectShp, kLoLFlightShpData, false },
	{ kLoLInventoryDesc, kRawDataBe16, false },
	{ kLoLLevelShpList, kStringList, false },
	{ kLoLLevelDatList, kStringList, false },
	{ kLoLCompassDefs, kLoLCompassData, false },
	{ kLoLItemPrices, kRawDataBe16, false },
	{ kLoLStashSetup, kRawData, false },

	{ kLoLDscWalls, kRawData, false },
	{ kRpgCommonDscShapeIndex, kRawData, false },
	{ kLoLDscOvlMap, kRawData, false },
	{ kLoLDscScaleWidthData, kRawDataBe16, false },
	{ kLoLDscScaleHeightData, kRawDataBe16, false },
	{ kRpgCommonDscX, kRawDataBe16, false },
	{ kLoLBaseDscY, kRawData, false },
	{ kRpgCommonDscTileIndex, kRawData, false },
	{ kRpgCommonDscDoorShapeIndex, kRawData, false },
	{ kRpgCommonDscDimData1, kRawData, false },
	{ kRpgCommonDscDimData2, kRawData, false },
	{ kRpgCommonDscBlockMap, kRawData, false },
	{ kRpgCommonDscDimMap, kRawData, false },
	{ kLoLDscDoorScale, kRawDataBe16, false },
	{ kLoLDscOvlIndex, kRawData, false },
	{ kRpgCommonDscBlockIndex, kRawData, false },
	{ kLoLDscDoor4, kRawDataBe16, false },
	{ kRpgCommonDscDoorY2, kRawData, false },
	{ kRpgCommonDscDoorFrameY1, kRawData, false },
	{ kRpgCommonDscDoorFrameY2, kRawData, false },
	{ kRpgCommonDscDoorFrameIndex1, kRawData, false },
	{ kRpgCommonDscDoorFrameIndex2, kRawData, false },
	{ kLoLDscDoorX, kRawDataBe16, false },
	{ kLoLDscDoorY, kRawDataBe16, false },

	{ kLoLScrollXTop, kRawData, false },
	{ kLoLScrollYTop, kRawData, false },
	{ kLoLScrollXBottom, kRawData, false },
	{ kLoLScrollYBottom, kRawData, false },

	{ kLoLButtonDefs, kLoLButtonData, false },
	{ kLoLButtonList1, kRawData, false },
	{ kLoLButtonList2, kRawData, false },
	{ kLoLButtonList3, kRawData, false },
	{ kLoLButtonList4, kRawData, false },
	{ kLoLButtonList5, kRawData, false },
	{ kLoLButtonList6, kRawData, false },
	{ kLoLButtonList7, kRawData, false },
	{ kLoLButtonList8, kRawData, false },

	{ kLoLLegendData, kRawData, false },
	{ kLoLMapCursorOvl, kRawData, false },
	{ kLoLMapStringId, kRawDataBe16, false },

	{ kLoLSpellbookAnim, kRawData, false },
	{ kLoLSpellbookCoords, kRawData, false },
	{ kLoLHealShapeFrames, kRawData, false },
	{ kLoLLightningDefs, kRawData, false },
	{ kLoLFireballCoords, kRawDataBe16, false },

	{ kLoLCredits, kRawData, false },

	{ kLoLHistory, kRawData, false },

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
	{ kEoB1, 3 },
	{ kEoB2, 4 },
	{ kLoL, 5 },
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
	{ HE_ISR, 8 },
	{ -1, -1 }
};

byte getLanguageID(int lang) {
	return std::find(languageTable, ARRAYEND(languageTable) - 1, lang)->value;
}

const TypeTable platformTable[] = {
	{ kPlatformDOS, 0 },
	{ kPlatformAmiga, 1 },
	{ kPlatformFMTowns, 2 },
	{ kPlatformPC98, 3 },
	{ kPlatformSegaCD, 4 },
	{ kPlatformMacintosh, 5 },
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

uint32 getFilename(int game, int plat, int spec, int lang, const ExtractFilename *fDesc) {
	// GAME, PLATFORM, SPECIAL, ID, LANG
	return ((getGameID(game) & 0xF) << 24) |
	       ((getPlatformID(plat) & 0xF) << 20) |
	       ((getSpecialID(spec) & 0xF) << 16) |
	       ((fDesc->id & 0xFFF) << 4) |
	       ((getLanguageID(fDesc->langSpecific ? lang : UNK_LANG) & 0xF) << 0);
}

uint32 getFilename(const Game *g, const int id) {
	const ExtractFilename *fDesc = getFilenameDesc(id);

	if (!fDesc)
		return 0;

	return getFilename(g->game, g->platform, g->special, g->lang, fDesc);
}

uint32 getFilename(const ResourceProvider *provider, const ExtractFilename *fDesc) {
	return getFilename(provider->game, provider->platform, provider->special, provider->language, fDesc);
}

bool getFilename(char *dstFilename, const Game *g, const int id) {
	sprintf(dstFilename, "%08X", getFilename(g, id));
	return true;
}

// index generation

typedef uint16 GameDef;

GameDef createGameDef(const Game *g) {
	return ((getGameID(g->game) & 0xF) << 12) |
	       ((getPlatformID(g->platform) & 0xF) << 8) |
	       ((getSpecialID(g->special) & 0xF) << 4) |
	       ((getLanguageID(g->lang) & 0xF) << 0);
	return 0;
}

// main processing

void printHelp(const char *f) {
	printf("Usage:\n");
	printf("%s output\n", f);
}

void outputAllResources(PAKFile &out);
void outputAllGames(PAKFile &out, std::vector<GameDef> &games);

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printHelp(argv[0]);
		return -1;
	}

	PAKFile out;

	// First step: Write out all resources.
	outputAllResources(out);

	// Second step: Write all game version information
	std::vector<GameDef> games;
	outputAllGames(out, games);

	// Third step: Write index file
	byte *const indexBuffer = new byte[8 + 2 * games.size()];
	byte *dst = indexBuffer;
	WRITE_BE_UINT32(dst, kKyraDatVersion); dst += 4;
	WRITE_BE_UINT32(dst, games.size()); dst += 4;
	for (std::vector<GameDef>::const_iterator i = games.begin(), end = games.end(); i != end; ++i) {
		WRITE_BE_UINT16(dst, *i); dst += 2;
	}
	if (!out.addFile("INDEX", indexBuffer, 8 + 2 * games.size())) {
		error("couldn't write INDEX file");
	}

	if (!out.saveFile(argv[1])) {
		error("couldn't save changes to '%s'", argv[1]);
	}

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

void outputAllResources(PAKFile &out) {
	char filename[128];
	for (const ResourceProvider *resource = obtainResourceProviders();
	     resource->id != kMaxResIDs; ++resource) {
		const ExtractFilename *desc = getFilenameDesc(resource->id);

		snprintf(filename, sizeof(filename), "%08X", getFilename(resource, desc));

		writeResource(out, filename, (ResTypes)desc->type, resource->provider);
	}
}

bool createIDMap(PAKFile &out, const Game *g, const int *needList);

void outputAllGames(PAKFile &out, std::vector<GameDef> &games) {
	for (const Game *const *descs = gameDescs;
	     *descs; ++descs) {
		for (const Game *game = *descs;
		     game->game != -1; ++game) {
			const GameDef gameDef = createGameDef(game);
			if (!createIDMap(out, game, getNeedList(game))) {
				error("Could not create ID map for game %04X", gameDef);
			}
			games.push_back(gameDef);
		}
	}
}

bool createIDMap(PAKFile &out, const Game *g, const int *needList) {
	int dataEntries = 0;
	// Count entries in the need list and check whether the resources are
	// present
	for (const int *n = needList; *n != -1; ++n) {
		char filename[12];
		if (!getFilename(filename, g, *n) || !out.getFileList()->findEntry(filename)) {
			fprintf(stderr, "ERROR: Could not find need %d for game %04X", *n, createGameDef(g));
			return false;
		}
		++dataEntries;
	}

	const int mapSize = 2 + dataEntries * (2 + 1 + 4);
	uint8 *map = new uint8[mapSize];
	uint8 *dst = map;

	WRITE_BE_UINT16(dst, dataEntries); dst += 2;
	for (const int *id = needList; *id != -1; ++id) {
		WRITE_BE_UINT16(dst, *id); dst += 2;
		const ExtractFilename *fDesc = getFilenameDesc(*id);
		if (!fDesc) {
			delete[] map;
			return false;
		}
		*dst++ = fDesc->type;
		WRITE_BE_UINT32(dst, getFilename(g, *id)); dst += 4;
	}

	char filename[12];
	if (!getFilename(filename, g, 0)) {
		fprintf(stderr, "ERROR: Could not create ID map for game\n");
		delete[] map;
		return false;
	}

	if (!out.addFile(filename, map, mapSize)) {
		fprintf(stderr, "ERROR: Could not add ID map \"%s\" to kyra.dat\n", filename);
		delete[] map;
		return false;
	}

	return true;
}
