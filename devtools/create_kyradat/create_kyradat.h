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

#ifndef CREATE_KYRADAT_CREATE_KYRADAT_H
#define CREATE_KYRADAT_CREATE_KYRADAT_H

#include "util.h"

// This list has to match orderwise (and thus value wise) the static data list of "engines/kyra/resource.h"!
enum kExtractID {
	kIdMap = 0,

	k1ForestSeq = 1,
	k1KallakWritingSeq,
	k1KyrandiaLogoSeq,
	k1KallakMalcolmSeq,
	k1MalcolmTreeSeq,
	k1WestwoodLogoSeq,

	k1Demo1Seq,
	k1Demo2Seq,
	k1Demo3Seq,
	k1Demo4Seq,

	k1AmuleteAnimSeq,

	k1OutroReunionSeq,

	k1IntroCPSStrings,
	k1IntroCOLStrings,
	k1IntroWSAStrings,
	k1IntroStrings,

	k1OutroHomeString,

	k1RoomFilenames,
	k1RoomList,

	k1CharacterImageFilenames,

	k1ItemNames,
	k1TakenStrings,
	k1PlacedStrings,
	k1DroppedStrings,
	k1NoDropStrings,

	k1PutDownString,
	k1WaitAmuletString,
	k1BlackJewelString,
	k1PoisonGoneString,
	k1HealingTipString,
	k1WispJewelStrings,
	k1MagicJewelStrings,

	k1ThePoisonStrings,
	k1FluteStrings,

	k1FlaskFullString,
	k1FullFlaskString,

	k1VeryCleverString,
	k1NewGameString,

	k1DefaultShapes,
	k1Healing1Shapes,
	k1Healing2Shapes,
	k1PoisonDeathShapes,
	k1FluteShapes,
	k1Winter1Shapes,
	k1Winter2Shapes,
	k1Winter3Shapes,
	k1DrinkShapes,
	k1WispShapes,
	k1MagicAnimShapes,
	k1BranStoneShapes,

	k1SpecialPalette1,
	k1SpecialPalette2,
	k1SpecialPalette3,
	k1SpecialPalette4,
	k1SpecialPalette5,
	k1SpecialPalette6,
	k1SpecialPalette7,
	k1SpecialPalette8,
	k1SpecialPalette9,
	k1SpecialPalette10,
	k1SpecialPalette11,
	k1SpecialPalette12,
	k1SpecialPalette13,
	k1SpecialPalette14,
	k1SpecialPalette15,
	k1SpecialPalette16,
	k1SpecialPalette17,
	k1SpecialPalette18,
	k1SpecialPalette19,
	k1SpecialPalette20,
	k1SpecialPalette21,
	k1SpecialPalette22,
	k1SpecialPalette23,
	k1SpecialPalette24,
	k1SpecialPalette25,
	k1SpecialPalette26,
	k1SpecialPalette27,
	k1SpecialPalette28,
	k1SpecialPalette29,
	k1SpecialPalette30,
	k1SpecialPalette31,
	k1SpecialPalette32,
	k1SpecialPalette33,

	k1GUIStrings,
	k1ConfigStrings,

	k1AudioTracks,
	k1AudioTracks2,
	k1AudioTracksIntro,

	k1CreditsStrings,

	k1TownsMusicFadeTable,
	k1TownsSFXwdTable,
	k1TownsSFXbtTable,
	k1TownsCDATable,

	k1PC98StoryStrings,
	k1PC98IntroSfx,

	k1AmigaIntroSFXTable,
	k1AmigaGameSFXTable,

	k2SeqplayPakFiles,
	k2SeqplayCredits,
	k2SeqplayCreditsSpecial,
	k2SeqplayStrings,
	k2SeqplaySfxFiles,
	k2SeqplayTlkFiles,
	k2SeqplaySeqData,
	k2SeqplayIntroTracks,
	k2SeqplayFinaleTracks,
	k2SeqplayIntroCDA,
	k2SeqplayFinaleCDA,
	k2SeqplayShapeAnimData,

	k2IngamePakFiles,
	k2IngameSfxFiles,
	k2IngameSfxIndex,
	k2IngameTracks,
	k2IngameCDA,
	k2IngameTalkObjIndex,
	k2IngameTimJpStrings,
	k2IngameShapeAnimData,
	k2IngameTlkDemoStrings,

	k3MainMenuStrings,
	k3MusicFiles,
	k3ScoreTable,
	k3SfxFiles,
	k3SfxMap,
	k3ItemAnimData,
	k3ItemMagicTable,
	k3ItemStringMap,

	kLolEobCommonMoreStrings,
	kLolEobCommonDscShapeIndex,
	kLolEobCommonDscX,
	kLolEobCommonDscTileIndex,
	kLolEobCommonDscUnk2,
	kLolEobCommonDscDoorShapeIndex,
	kLolEobCommonDscDimData1,
	kLolEobCommonDscDimData2,
	kLolEobCommonDscBlockMap,
	kLolEobCommonDscDimMap,
	kLolEobCommonDscDoorY2,
	kLolEobCommonDscBlockIndex,

	kEobBaseChargenStrings1,
	kEobBaseChargenStrings2,
	kEobBaseChargenStartLevels,
	kEobBaseChargenStatStrings,
	kEobBaseChargenRaceSexStrings,
	kEobBaseChargenClassStrings,
	kEobBaseChargenAlignmentStrings,
	kEobBaseChargenEnterGameStrings,
	kEobBaseChargenClassMinStats,
	kEobBaseChargenRaceMinStats,
	kEobBaseChargenRaceMaxStats,

	kEobBaseConstModTable1,
	kEobBaseConstModTable2,
	kEobBaseConstModTable3,
	kEobBaseConstModTable4,
	kEobBaseConstModLvlIndex,
	kEobBaseConstModDiv,
	kEobBaseConstModExt,

	kEobBasePryDoorStrings,
	kEobBaseWarningStrings,
	kEobBaseItemSuffixStrings,
	kEobBaseItemExtraStrings,
	kEobBaseTakenStrings,
	kEobBasePotionEffectStrings,

	kEobBaseYesNoStrings,
	kEobBaseNpcMaxStrings,
	kEobBaseOkStrings,
	kEobBaseNpcJoinStrings,
	kEobBaseCancelStrings,
	kEobBaseAbortStrings,

	kEobBaseMenuStringsMain,
	kEobBaseMenuStringsSaveLoad,
	kEobBaseMenuStringsOnOff,
	kEobBaseMenuStringsSpells,
	kEobBaseMenuStringsRest,
	kEobBaseMenuStringsDrop,
	kEobBaseMenuStringsExit,
	kEobBaseMenuStringsStarve,
	kEobBaseMenuStringsScribe,
	kEobBaseMenuStringsDrop2,
	kEobBaseMenuStringsHead,
	kEobBaseMenuStringsPoison,
	kEobBaseMenuStringsMgc,
	kEobBaseMenuStringsPrefs,
	kEobBaseMenuStringsRest2,
	kEobBaseMenuStringsRest3,
	kEobBaseMenuStringsRest4,
	kEobBaseMenuStringsDefeat,
	kEobBaseMenuStringsTransfer,
	kEobBaseMenuStringsSpec,
	kEobBaseMenuStringsSpellNo,

	kEobBaseSpellLevelsMage,
	kEobBaseSpellLevelsCleric,
	kEobBaseSpellMps1,
	kEobBaseSpellMps2,
	kEobBaseSpellMps3,
	kEobBaseSpellMps4,

	kEobBaseCharGuiStringsHp,
	kEobBaseCharGuiStringsWp1,
	kEobBaseCharGuiStringsWp2,
	kEobBaseCharGuiStringsWr,
	kEobBaseCharGuiStringsSt1,
	kEobBaseCharGuiStringsSt2,
	kEobBaseCharGuiStringsIn,

	kEobBaseCharStatusStrings7,
	kEobBaseCharStatusStrings81,
	kEobBaseCharStatusStrings82,
	kEobBaseCharStatusStrings9,
	kEobBaseCharStatusStrings12,
	kEobBaseCharStatusStrings131,
	kEobBaseCharStatusStrings132,

	kEobBaseLevelGainStrings,
	kEobBaseExperienceTable0,
	kEobBaseExperienceTable1,
	kEobBaseExperienceTable2,
	kEobBaseExperienceTable3,
	kEobBaseExperienceTable4,

	kEobBaseClassModifierFlags,

	kEobBaseMonsterStepTable01,
	kEobBaseMonsterStepTable02,
	kEobBaseMonsterStepTable1,
	kEobBaseMonsterStepTable2,
	kEobBaseMonsterStepTable3,
	kEobBaseMonsterCloseAttPosTable1,
	kEobBaseMonsterCloseAttPosTable21,
	kEobBaseMonsterCloseAttPosTable22,
	kEobBaseMonsterCloseAttUnkTable,
	kEobBaseMonsterCloseAttChkTable1,
	kEobBaseMonsterCloseAttChkTable2,
	kEobBaseMonsterCloseAttDstTable1,
	kEobBaseMonsterCloseAttDstTable2,

	kEobBaseMonsterProximityTable,
	kEobBaseFindBlockMonstersTable,
	kEobBaseMonsterDirChangeTable,
	kEobBaseMonsterDistAttStrings,

	kEobBaseEncodeMonsterDefs,
	kEobBaseNpcPresets,

	kEobBaseWllFlagPreset,
	kEobBaseDscShapeCoords,

	kEobBaseDscDoorScaleOffs,
	kEobBaseDscDoorScaleMult1,
	kEobBaseDscDoorScaleMult2,
	kEobBaseDscDoorScaleMult3,
	kEobBaseDscDoorScaleMult4,
	kEobBaseDscDoorScaleMult5,
	kEobBaseDscDoorScaleMult6,
	kEobBaseDscDoorType5Offs,
	kEobBaseDscDoorY1,
	kEobBaseDscDoorY3,
	kEobBaseDscDoorY4,
	kEobBaseDscDoorY5,
	kEobBaseDscDoorY6,
	kEobBaseDscDoorCoordsExt,

	kEobBaseDscItemPosIndex,
	kEobBaseDscItemShpX,
	kEobBaseDscItemPosUnk,
	kEobBaseDscItemTileIndex,
	kEobBaseDscItemShapeMap,

	kEobBaseDscMonsterFrmOffsTbl1,
	kEobBaseDscMonsterFrmOffsTbl2,

	kEobBaseInvSlotX,
	kEobBaseInvSlotY,
	kEobBaseSlotValidationFlags,
	kEobBaseDrawObjPosIndex,
	kEobBaseFlightObjFlipIndex,
	kEobBaseFlightObjShpMap,
	kEobBaseFlightObjSclIndex,

	kEobBaseDscTelptrShpCoords,

	kEobBaseBookNumbers,
	kEobBaseMageSpellsList,
	kEobBaseClericSpellsList,
	kEobBaseSpellNames,
	kEobBaseMagicStrings1,
	kEobBaseMagicStrings2,
	kEobBaseMagicStrings3,
	kEobBaseMagicStrings4,
	kEobBaseMagicStrings6,
	kEobBaseMagicStrings7,
	kEobBaseMagicStrings8,

	kEobBaseExpObjectTlMode,
	kEobBaseExpObjectTblIndex,
	kEobBaseExpObjectShpStart,
	kEobBaseExpObjectTbl1,
	kEobBaseExpObjectTbl2,
	kEobBaseExpObjectTbl3,
	kEobBaseExpObjectY,

	kEobBaseSparkDefSteps,
	kEobBaseSparkDefSubSteps,
	kEobBaseSparkDefShift,
	kEobBaseSparkDefAdd,
	kEobBaseSparkDefX,
	kEobBaseSparkDefY,
	kEobBaseSparkOfFlags1,
	kEobBaseSparkOfFlags2,
	kEobBaseSparkOfShift,
	kEobBaseSparkOfX,
	kEobBaseSparkOfY,

	kEobBaseSpellProperties,
	kEobBaseMagicFlightProps,

	kEob1MainMenuStrings,
	kEob1DoorShapeDefs,
	kEob1DoorSwitchShapeDefs,
	kEob1DoorSwitchCoords,
	kEob1MonsterProperties,

	kEob1MonsterDistAttType10,
	kEob1MonsterDistAttSfx10,
	kEob1MonsterDistAttType17,
	kEob1MonsterDistAttSfx17,


	kEob2MainMenuStrings,
	kEob2IntroStrings,
	kEob2IntroCPSFiles,
	kEob2IntroSeqData00,
 	kEob2IntroSeqData01,
 	kEob2IntroSeqData02,
 	kEob2IntroSeqData03,
 	kEob2IntroSeqData04,
 	kEob2IntroSeqData05,
 	kEob2IntroSeqData06,
 	kEob2IntroSeqData07,
 	kEob2IntroSeqData08,
 	kEob2IntroSeqData09,
 	kEob2IntroSeqData10,
 	kEob2IntroSeqData11,
 	kEob2IntroSeqData12,
 	kEob2IntroSeqData13,
 	kEob2IntroSeqData14,
 	kEob2IntroSeqData15,
 	kEob2IntroSeqData16,
 	kEob2IntroSeqData17,
 	kEob2IntroSeqData18,
 	kEob2IntroSeqData19,
 	kEob2IntroSeqData20,
 	kEob2IntroSeqData21,
 	kEob2IntroSeqData22,
 	kEob2IntroSeqData23,
 	kEob2IntroSeqData24,
 	kEob2IntroSeqData25,
 	kEob2IntroSeqData26,
 	kEob2IntroSeqData27,
 	kEob2IntroSeqData28,
 	kEob2IntroSeqData29,
 	kEob2IntroSeqData30,
 	kEob2IntroSeqData31,
 	kEob2IntroSeqData32,
 	kEob2IntroSeqData33,
 	kEob2IntroSeqData34,
 	kEob2IntroSeqData35,
 	kEob2IntroSeqData36,
 	kEob2IntroSeqData37,
 	kEob2IntroSeqData38,
 	kEob2IntroSeqData39,
 	kEob2IntroSeqData40,
 	kEob2IntroSeqData41,
 	kEob2IntroSeqData42,
 	kEob2IntroSeqData43,
	kEob2IntroShapes00,
 	kEob2IntroShapes01,
 	kEob2IntroShapes04,
 	kEob2IntroShapes07,

	kEob2FinaleStrings,
	kEob2CreditsData,
	kEob2FinaleCPSFiles,
	kEob2FinaleSeqData00,
 	kEob2FinaleSeqData01,
 	kEob2FinaleSeqData02,
 	kEob2FinaleSeqData03,
 	kEob2FinaleSeqData04,
 	kEob2FinaleSeqData05,
 	kEob2FinaleSeqData06,
 	kEob2FinaleSeqData07,
 	kEob2FinaleSeqData08,
 	kEob2FinaleSeqData09,
 	kEob2FinaleSeqData10,
 	kEob2FinaleSeqData11,
 	kEob2FinaleSeqData12,
 	kEob2FinaleSeqData13,
 	kEob2FinaleSeqData14,
 	kEob2FinaleSeqData15,
 	kEob2FinaleSeqData16,
 	kEob2FinaleSeqData17,
 	kEob2FinaleSeqData18,
 	kEob2FinaleSeqData19,
 	kEob2FinaleSeqData20,
	kEob2FinaleShapes00,
 	kEob2FinaleShapes03,
 	kEob2FinaleShapes07,
 	kEob2FinaleShapes09,
 	kEob2FinaleShapes10,

	kEob2NpcShapeData,
	kEob2Npc1Strings,
	kEob2Npc2Strings,
	kEob2MonsterDustStrings,

	kLolIngamePakFiles,
	kLolCharacterDefs,
	kLolIngameSfxFiles,
	kLolIngameSfxIndex,
	kLolMusicTrackMap,
	kLolIngameGMSfxIndex,
	kLolIngameMT32SfxIndex,
	kLolIngamePcSpkSfxIndex,
	kLolSpellProperties,
	kLolGameShapeMap,
	kLolSceneItemOffs,
	kLolCharInvIndex,
	kLolCharInvDefs,
	kLolCharDefsMan,
	kLolCharDefsWoman,
	kLolCharDefsKieran,
	kLolCharDefsAkshel,
	kLolExpRequirements,
	kLolMonsterModifiers,
	kLolMonsterShiftOffsets,
	kLolMonsterDirFlags,
	kLolMonsterScaleY,
	kLolMonsterScaleX,
	kLolMonsterScaleWH,
	kLolFlyingObjectShp,
	kLolInventoryDesc,

	kLolLevelShpList,
	kLolLevelDatList,
	kLolCompassDefs,
	kLolItemPrices,
	kLolStashSetup,

	kLolDscWalls,
	kLolDscOvlMap,
	kLolDscScaleWidthData,
	kLolDscScaleHeightData,
	kLolDscY,

	kLolDscDoorScale,
	kLolDscDoor4,
	kLolDscDoorX,
	kLolDscDoorY,
	kLolDscOvlIndex,	

	kLolScrollXTop,
	kLolScrollYTop,
	kLolScrollXBottom,
	kLolScrollYBottom,

	kLolButtonDefs,
	kLolButtonList1,
	kLolButtonList2,
	kLolButtonList3,
	kLolButtonList4,
	kLolButtonList5,
	kLolButtonList6,
	kLolButtonList7,
	kLolButtonList8,

	kLolLegendData,
	kLolMapCursorOvl,
	kLolMapStringId,

	kLolSpellbookAnim,
	kLolSpellbookCoords,
	kLolHealShapeFrames,
	kLolLightningDefs,
	kLolFireballCoords,

	kLolCredits,

	kLolHistory,

	kMaxResIDs
};

struct ExtractFilename {
	int id;
	int type;
	bool langSpecific;
};

enum kSpecial {
	kNoSpecial = 0,
	kTalkieVersion,
	kDemoVersion,
	kTalkieDemoVersion,
	kOldFloppy
};

enum kGame {
	kKyra1 = 0,
	kKyra2,
	kKyra3,
	kLol,
	kEob1,
	kEob2,
};

struct Game {
	int game;
	int lang[3];
	int platform;
	int special;

	const char *md5[2];
};

#define GAME_DUMMY_ENTRY { -1, { -1, -1, -1 }, -1, -1, { 0, 0 } }

extern const Game * const gameDescs[];

const int *getNeedList(const Game *g);

struct TypeTable {
	int type;
	int value;

	bool operator==(int t) const {
		return (type == t);
	}
};

#endif
