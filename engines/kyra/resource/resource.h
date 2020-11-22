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

#ifndef KYRA_RESOURCE_H
#define KYRA_RESOURCE_H


#include "common/scummsys.h"
#include "common/str.h"
#include "common/file.h"
#include "common/list.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/stream.h"
#include "common/ptr.h"
#include "common/archive.h"

#include "kyra/kyra_v1.h"
#include "kyra/engine/darkmoon.h"
#include "kyra/engine/lol.h"
#include "kyra/engine/kyra_hof.h"

namespace Kyra {

class Resource;

class ResArchiveLoader;

class Resource {
public:
	Resource(KyraEngine_v1 *vm);
	~Resource();

	bool reset();

	bool loadPakFile(Common::String filename);
	bool loadPakFile(Common::String name, Common::ArchiveMemberPtr file);

	void unloadPakFile(Common::String filename, bool remFromCache = false);

	bool isInPakList(Common::String filename);

	bool isInCacheList(Common::String name);

	bool loadFileList(const Common::String &filedata);
	bool loadFileList(const char *const *filelist, uint32 numFiles);

	// This unloads *all* pakfiles, even kyra.dat and protected ones.
	// It does not remove files from cache though!
	void unloadAllPakFiles();

	void listFiles(const Common::String &pattern, Common::ArchiveMemberList &list);

	bool exists(const char *file, bool errorOutOnFail=false);
	uint32 getFileSize(const char *file);
	uint8 *fileData(const char *file, uint32 *size);
	Common::SeekableReadStream *createReadStream(const Common::String &file);

	enum Endianness {
		kPlatformEndianness = 0,
		kForceLE,
		kForceBE
	};

	Common::SeekableReadStreamEndian *createEndianAwareReadStream(const Common::String &file, int endianness = kPlatformEndianness);

	bool loadFileToBuf(const char *file, void *buf, uint32 maxSize);
protected:
	typedef Common::HashMap<Common::String, Common::Archive *, Common::CaseSensitiveString_Hash, Common::CaseSensitiveString_EqualTo> ArchiveMap;
	ArchiveMap _archiveCache;

	Common::SearchSet _files;
	Common::SearchSet _archiveFiles;
	Common::SearchSet _protectedFiles;

	Common::Archive *loadArchive(const Common::String &name, Common::ArchiveMemberPtr member);
	Common::Archive *loadInstallerArchive(const Common::String &file, const Common::String &ext, const uint8 offset);

	bool loadProtectedFiles(const char *const * list);

	void initializeLoaders();

	typedef Common::List<Common::SharedPtr<ResArchiveLoader> > LoaderList;
	LoaderList _loaders;

	const bool _bigEndianPlatForm;
	KyraEngine_v1 *_vm;
};

enum KyraResources {
	kLoadAll = -1,

	// This list has to match orderwise (and thus value wise) the static data list of "devtools/create_kyradat/create_kyradat.h"!
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

#if defined(ENABLE_EOB) || defined(ENABLE_LOL)
	kRpgCommonMoreStrings,
	kRpgCommonDscShapeIndex,
	kRpgCommonDscX,
	kRpgCommonDscTileIndex,
	kRpgCommonDscDoorShapeIndex,
	kRpgCommonDscDimData1,
	kRpgCommonDscDimData2,
	kRpgCommonDscBlockMap,
	kRpgCommonDscDimMap,
	kRpgCommonDscDoorY2,
	kRpgCommonDscDoorFrameY1,
	kRpgCommonDscDoorFrameY2,
	kRpgCommonDscDoorFrameIndex1,
	kRpgCommonDscDoorFrameIndex2,
	kRpgCommonDscDoorScaleOffs,
	kRpgCommonDscBlockIndex,

	kEoBBaseChargenStrings1,
	kEoBBaseChargenStrings2,
	kEoBBaseChargenStartLevels,
	kEoBBaseChargenStatStrings,
	kEoBBaseChargenRaceSexStrings,
	kEoBBaseChargenClassStrings,
	kEoBBaseChargenAlignmentStrings,
	kEoBBaseChargenEnterGameStrings,
	kEoBBaseChargenClassMinStats,
	kEoBBaseChargenRaceMinStats,
	kEoBBaseChargenRaceMaxStats,

	kEoBBaseSaveThrowTable1,
	kEoBBaseSaveThrowTable2,
	kEoBBaseSaveThrowTable3,
	kEoBBaseSaveThrowTable4,
	kEoBBaseSaveThrwLvlIndex,
	kEoBBaseSaveThrwModDiv,
	kEoBBaseSaveThrwModExt,

	kEoBBasePryDoorStrings,
	kEoBBaseWarningStrings,

	kEoBBaseItemSuffixStringsRings,
	kEoBBaseItemSuffixStringsPotions,
	kEoBBaseItemSuffixStringsWands,

	kEoBBaseRipItemStrings,
	kEoBBaseCursedString,
	kEoBBaseEnchantedString,
	kEoBBaseMagicObjectStrings,
	kEoBBaseMagicObjectString5,
	kEoBBasePatternSuffix,
	kEoBBasePatternGrFix1,
	kEoBBasePatternGrFix2,
	kEoBBaseValidateArmorString,
	kEoBBaseValidateCursedString,
	kEoBBaseValidateNoDropString,
	kEoBBasePotionStrings,
	kEoBBaseWandStrings,
	kEoBBaseItemMisuseStrings,

	kEoBBaseTakenStrings,
	kEoBBasePotionEffectStrings,

	kEoBBaseYesNoStrings,
	kEoBBaseNpcMaxStrings,
	kEoBBaseOkStrings,
	kEoBBaseNpcJoinStrings,
	kEoBBaseCancelStrings,
	kEoBBaseAbortStrings,

	kEoBBaseMenuStringsMain,
	kEoBBaseMenuStringsSaveLoad,
	kEoBBaseMenuStringsOnOff,
	kEoBBaseMenuStringsSpells,
	kEoBBaseMenuStringsRest,
	kEoBBaseMenuStringsDrop,
	kEoBBaseMenuStringsExit,
	kEoBBaseMenuStringsStarve,
	kEoBBaseMenuStringsScribe,
	kEoBBaseMenuStringsDrop2,
	kEoBBaseMenuStringsHead,
	kEoBBaseMenuStringsPoison,
	kEoBBaseMenuStringsMgc,
	kEoBBaseMenuStringsPrefs,
	kEoBBaseMenuStringsRest2,
	kEoBBaseMenuStringsRest3,
	kEoBBaseMenuStringsRest4,
	kEoBBaseMenuStringsDefeat,
	kEoBBaseMenuStringsTransfer,
	kEoBBaseMenuStringsSpec,
	kEoBBaseMenuStringsSpellNo,
	kEoBBaseMenuYesNoStrings,

	kEoBBaseSpellLevelsMage,
	kEoBBaseSpellLevelsCleric,
	kEoBBaseNumSpellsCleric,
	kEoBBaseNumSpellsWisAdj,
	kEoBBaseNumSpellsPal,
	kEoBBaseNumSpellsMage,

	kEoBBaseCharGuiStringsHp,
	kEoBBaseCharGuiStringsWp1,
	kEoBBaseCharGuiStringsWp2,
	kEoBBaseCharGuiStringsWr,
	kEoBBaseCharGuiStringsSt1,
	kEoBBaseCharGuiStringsSt2,
	kEoBBaseCharGuiStringsIn,

	kEoBBaseCharStatusStrings7,
	kEoBBaseCharStatusStrings81,
	kEoBBaseCharStatusStrings82,
	kEoBBaseCharStatusStrings9,
	kEoBBaseCharStatusStrings12,
	kEoBBaseCharStatusStrings131,
	kEoBBaseCharStatusStrings132,

	kEoBBaseLevelGainStrings,
	kEoBBaseExperienceTable0,
	kEoBBaseExperienceTable1,
	kEoBBaseExperienceTable2,
	kEoBBaseExperienceTable3,
	kEoBBaseExperienceTable4,

	kEoBBaseClassModifierFlags,

	kEoBBaseMonsterStepTable01,
	kEoBBaseMonsterStepTable02,
	kEoBBaseMonsterStepTable1,
	kEoBBaseMonsterStepTable2,
	kEoBBaseMonsterStepTable3,
	kEoBBaseMonsterCloseAttPosTable1,
	kEoBBaseMonsterCloseAttPosTable21,
	kEoBBaseMonsterCloseAttPosTable22,
	kEoBBaseMonsterCloseAttUnkTable,
	kEoBBaseMonsterCloseAttChkTable1,
	kEoBBaseMonsterCloseAttChkTable2,
	kEoBBaseMonsterCloseAttDstTable1,
	kEoBBaseMonsterCloseAttDstTable2,

	kEoBBaseMonsterProximityTable,
	kEoBBaseFindBlockMonstersTable,
	kEoBBaseMonsterDirChangeTable,
	kEoBBaseMonsterDistAttStrings,

	kEoBBaseEncodeMonsterDefs,
	kEoBBaseEncodeMonsterDefs00,
	kEoBBaseEncodeMonsterDefs01,
	kEoBBaseEncodeMonsterDefs02,
	kEoBBaseEncodeMonsterDefs03,
	kEoBBaseEncodeMonsterDefs04,
	kEoBBaseEncodeMonsterDefs05,
	kEoBBaseEncodeMonsterDefs06,
	kEoBBaseEncodeMonsterDefs07,
	kEoBBaseEncodeMonsterDefs08,
	kEoBBaseEncodeMonsterDefs09,
	kEoBBaseEncodeMonsterDefs10,
	kEoBBaseEncodeMonsterDefs11,
	kEoBBaseEncodeMonsterDefs12,
	kEoBBaseEncodeMonsterDefs13,
	kEoBBaseEncodeMonsterDefs14,
	kEoBBaseEncodeMonsterDefs15,
	kEoBBaseEncodeMonsterDefs16,
	kEoBBaseEncodeMonsterDefs17,
	kEoBBaseEncodeMonsterDefs18,
	kEoBBaseEncodeMonsterDefs19,
	kEoBBaseEncodeMonsterDefs20,
	kEoBBaseEncodeMonsterDefs21,
	kEoBBaseNpcPresets,
	kEoBBaseNpcPresetsNames,

	kEoBBaseWllFlagPreset,
	kEoBBaseDscShapeCoords,

	kEoBBaseDscDoorScaleMult1,
	kEoBBaseDscDoorScaleMult2,
	kEoBBaseDscDoorScaleMult3,
	kEoBBaseDscDoorScaleMult4,
	kEoBBaseDscDoorScaleMult5,
	kEoBBaseDscDoorScaleMult6,
	kEoBBaseDscDoorType5Offs,
	kEoBBaseDscDoorXE,
	kEoBBaseDscDoorY1,
	kEoBBaseDscDoorY3,
	kEoBBaseDscDoorY4,
	kEoBBaseDscDoorY5,
	kEoBBaseDscDoorY6,
	kEoBBaseDscDoorY7,
	kEoBBaseDscDoorCoordsExt,

	kEoBBaseDscItemPosIndex,
	kEoBBaseDscItemShpX,
	kEoBBaseDscItemScaleIndex,
	kEoBBaseDscItemTileIndex,
	kEoBBaseDscItemShapeMap,

	kEoBBaseDscMonsterFrmOffsTbl1,
	kEoBBaseDscMonsterFrmOffsTbl2,

	kEoBBaseInvSlotX,
	kEoBBaseInvSlotY,
	kEoBBaseSlotValidationFlags,

	kEoBBaseProjectileWeaponTypes,
	kEoBBaseWandTypes,

	kEoBBaseDrawObjPosIndex,
	kEoBBaseFlightObjFlipIndex,
	kEoBBaseFlightObjShpMap,
	kEoBBaseFlightObjSclIndex,

	kEoBBaseDscTelptrShpCoords,

	kEoBBasePortalSeqData,
	kEoBBaseManDef,
	kEoBBaseManWord,
	kEoBBaseManPrompt,

	kEoBBaseBookNumbers,
	kEoBBaseMageSpellsList,
	kEoBBaseClericSpellsList,
	kEoBBaseMageSpellsList2,
	kEoBBaseClericSpellsList2,
	kEoBBaseSpellNames,
	kEoBBaseMagicStrings1,
	kEoBBaseMagicStrings2,
	kEoBBaseMagicStrings3,
	kEoBBaseMagicStrings4,
	kEoBBaseMagicStrings6,
	kEoBBaseMagicStrings7,
	kEoBBaseMagicStrings8,
	kEoBBaseMagicStrings9,

	kEoBBaseExpObjectTlMode,
	kEoBBaseExpObjectTblIndex,
	kEoBBaseExpObjectShpStart,
	kEoBBaseExpObjectTbl1,
	kEoBBaseExpObjectTbl2,
	kEoBBaseExpObjectTbl3,
	kEoBBaseExpObjectY,

	kEoBBaseSparkDefSteps,
	kEoBBaseSparkDefSubSteps,
	kEoBBaseSparkDefShift,
	kEoBBaseSparkDefAdd,
	kEoBBaseSparkDefX,
	kEoBBaseSparkDefY,
	kEoBBaseSparkOfFlags1,
	kEoBBaseSparkOfFlags2,
	kEoBBaseSparkOfShift,
	kEoBBaseSparkOfX,
	kEoBBaseSparkOfY,

	kEoBBaseSpellProperties,
	kEoBBaseMagicFlightProps,
	kEoBBaseTurnUndeadEffect,
	kEoBBaseBurningHandsDest,
	kEoBBaseConeOfColdDest1,
	kEoBBaseConeOfColdDest2,
	kEoBBaseConeOfColdDest3,
	kEoBBaseConeOfColdDest4,
	kEoBBaseConeOfColdGfxTbl,

	kEoBBaseSoundMap,
	kEoBBaseSoundFilesIntro,
	kEoBBaseSoundFilesIngame,
	kEoBBaseSoundFilesFinale,
	kEoBBaseLevelSounds1,
	kEoBBaseLevelSounds2,

	kEoBBaseTextInputCharacterLines,
	kEoBBaseTextInputSelectStrings,

	kEoBBaseSaveNamePatterns,

	kEoB1DefaultPartyStats,
	kEoB1DefaultPartyNames,
	kEoB1MainMenuStrings,
	kEoB1BonusStrings,

	kEoB1IntroFilesOpening,
	kEoB1IntroFilesTower,
	kEoB1IntroFilesOrb,
	kEoB1IntroFilesWdEntry,
	kEoB1IntroFilesKing,
	kEoB1IntroFilesHands,
	kEoB1IntroFilesWdExit,
	kEoB1IntroFilesTunnel,

	kEoB1IntroStringsTower,
	kEoB1IntroStringsOrb,
	kEoB1IntroStringsWdEntry,
	kEoB1IntroStringsKing,
	kEoB1IntroStringsHands,
	kEoB1IntroStringsWdExit,
	kEoB1IntroStringsTunnel,

	kEoB1FinaleStrings,

	kEoB1IntroOpeningFrmDelay,
	kEoB1IntroWdEncodeX,
	kEoB1IntroWdEncodeY,
	kEoB1IntroWdEncodeWH,
	kEoB1IntroWdDsX,
	kEoB1IntroWdDsY,
	kEoB1IntroTvlX1,
	kEoB1IntroTvlY1,
	kEoB1IntroTvlX2,
	kEoB1IntroTvlY2,
	kEoB1IntroTvlW,
	kEoB1IntroTvlH,
	kEoB1IntroOrbFadePal,

	kEoB1FinaleCouncilAnim1,
	kEoB1FinaleCouncilAnim2,
	kEoB1FinaleCouncilAnim3,
	kEoB1FinaleCouncilAnim4,
	kEoB1FinaleEyesAnim,
	kEoB1FinaleHandsAnim,
	kEoB1FinaleHandsAnim2,
	kEoB1FinaleHandsAnim3,
	kEoB1FinaleTextDuration,
	kEoB1CreditsStrings,
	kEoB1CreditsCharWdth,

	kEoB1CreditsStrings2,
	kEoB1CreditsTileGrid,

	kEoB1DoorShapeDefs,
	kEoB1DoorSwitchShapeDefs,
	kEoB1DoorSwitchCoords,
	kEoB1MonsterProperties,

	kEoB1EnemyMageSpellList,
	kEoB1EnemyMageSfx,
	kEoB1BeholderSpellList,
	kEoB1BeholderSfx,
	kEoB1TurnUndeadString,

	kEoB1CgaMappingDefault,
	kEoB1CgaMappingAlt,
	kEoB1CgaMappingInv,
	kEoB1CgaMappingItemsL,
	kEoB1CgaMappingItemsS,
	kEoB1CgaMappingThrown,
	kEoB1CgaMappingIcons,
	kEoB1CgaMappingDeco,
	kEoB1CgaLevelMappingIndex,
	kEoB1CgaMappingLevel0,
	kEoB1CgaMappingLevel1,
	kEoB1CgaMappingLevel2,
	kEoB1CgaMappingLevel3,
	kEoB1CgaMappingLevel4,

	kEoB1Palettes16c,
	kEoB1PalCycleData,
	kEoB1PalCycleStyle1,
	kEoB1PalCycleStyle2,
	kEoB1PalettesSega,
	kEoB1PatternTable0,
	kEoB1PatternTable1,
	kEoB1PatternTable2,
	kEoB1PatternTable3,
	kEoB1PatternTable4,
	kEoB1PatternTable5,
	kEoB1PatternAddTable1,
	kEoB1PatternAddTable2,

	kEoB1MonsterAnimFrames00,
	kEoB1MonsterAnimFrames01,
	kEoB1MonsterAnimFrames02,
	kEoB1MonsterAnimFrames03,
	kEoB1MonsterAnimFrames04,
	kEoB1MonsterAnimFrames05,
	kEoB1MonsterAnimFrames06,
	kEoB1MonsterAnimFrames07,
	kEoB1MonsterAnimFrames08,
	kEoB1MonsterAnimFrames09,
	kEoB1MonsterAnimFrames10,
	kEoB1MonsterAnimFrames11,
	kEoB1MonsterAnimFrames12,
	kEoB1MonsterAnimFrames13,
	kEoB1MonsterAnimFrames14,
	kEoB1MonsterAnimFrames15,
	kEoB1MonsterAnimFrames16,
	kEoB1MonsterAnimFrames17,
	kEoB1MonsterAnimFrames18,
	kEoB1MonsterAnimFrames19,
	kEoB1MonsterAnimFrames20,
	kEoB1MonsterAnimFrames21,
	kEoB1MonsterAnimFrames22,
	kEoB1MonsterAnimFrames23,
	kEoB1MonsterAnimFrames24,
	kEoB1MonsterAnimFrames25,
	kEoB1MonsterAnimFrames26,
	kEoB1MonsterAnimFrames27,
	kEoB1MonsterAnimFrames28,
	kEoB1MonsterAnimFrames29,
	kEoB1MonsterAnimFrames30,
	kEoB1MonsterAnimFrames31,
	kEoB1MonsterAnimFrames32,
	kEoB1MonsterAnimFrames33,
	kEoB1MonsterAnimFrames34,
	kEoB1MonsterAnimFrames35,
	kEoB1MonsterAnimFrames36,
	kEoB1MonsterAnimFrames37,
	kEoB1MonsterAnimFrames38,
	kEoB1MonsterAnimFrames39,
	kEoB1MonsterAnimFrames40,
	kEoB1MonsterAnimFrames41,
	kEoB1MonsterAnimFrames42,
	kEoB1MonsterAnimFrames43,
	kEoB1MonsterAnimFrames44,
	kEoB1MonsterAnimFrames45,
	kEoB1MonsterAnimFrames46,
	kEoB1MonsterAnimFrames47,
	kEoB1MonsterAnimFrames48,
	kEoB1MonsterAnimFrames49,
	kEoB1MonsterAnimFrames50,
	kEoB1MonsterAnimFrames51,
	kEoB1MonsterAnimFrames52,
	kEoB1MonsterAnimFrames53,
	kEoB1MonsterAnimFrames54,
	kEoB1MonsterAnimFrames55,
	kEoB1MonsterAnimFrames56,
	kEoB1MonsterAnimFrames57,
	kEoB1MonsterAnimFrames58,
	kEoB1MonsterAnimFrames59,
	kEoB1MonsterAnimFrames60,
	kEoB1MonsterAnimFrames61,
	kEoB1MonsterAnimFrames62,
	kEoB1MonsterAnimFrames63,
	kEoB1MonsterAnimFrames64,
	kEoB1MonsterAnimFrames65,
	kEoB1MonsterAnimFrames66,
	kEoB1MonsterAnimFrames67,
	kEoB1MonsterAnimFrames68,
	kEoB1MonsterAnimFrames69,
	kEoB1MonsterAnimFrames70,
	kEoB1MonsterAnimFrames71,
	kEoB1MonsterAnimFrames72,
	kEoB1MonsterAnimFrames73,
	kEoB1MonsterAnimFrames74,
	kEoB1MonsterAnimFrames75,
	kEoB1MonsterAnimFrames76,
	kEoB1MonsterAnimFrames77,
	kEoB1MonsterAnimFrames78,
	kEoB1MonsterAnimFrames79,
	kEoB1MonsterAnimFrames80,
	kEoB1MonsterAnimFrames81,
	kEoB1MonsterAnimFrames82,
	kEoB1MonsterAnimFrames83,
	kEoB1MonsterAnimFrames84,
	kEoB1MonsterAnimFrames85,
	kEoB1MonsterAnimFrames86,
	kEoB1MonsterAnimFrames87,
	kEoB1MonsterAnimFrames88,
	kEoB1MonsterAnimFrames89,
	kEoB1MonsterAnimFrames90,
	kEoB1MonsterAnimFrames91,
	kEoB1MonsterAnimFrames92,
	kEoB1MonsterAnimFrames93,
	kEoB1MonsterAnimFrames94,
	kEoB1MonsterAnimFrames95,
	kEoB1MonsterAnimFrames96,
	kEoB1MonsterAnimFrames97,
	kEoB1MonsterAnimFrames98,
	kEoB1MonsterAnimFrames99,
	kEoB1MonsterAnimFrames100,
	kEoB1MonsterAnimFrames101,
	kEoB1MonsterAnimFrames102,
	kEoB1MonsterAnimFrames103,
	kEoB1MonsterAnimFrames104,
	kEoB1MonsterAnimFrames105,
	kEoB1MonsterAnimFrames106,
	kEoB1MonsterAnimFrames107,
	kEoB1MonsterAnimFrames108,
	kEoB1MonsterAnimFrames109,

	kEoB1NpcShpData,
	kEoB1NpcSubShpIndex1,
	kEoB1NpcSubShpIndex2,
	kEoB1NpcSubShpY,
	kEoB1Npc0Strings,
	kEoB1Npc11Strings,
	kEoB1Npc12Strings,
	kEoB1Npc21Strings,
	kEoB1Npc22Strings,
	kEoB1Npc31Strings,
	kEoB1Npc32Strings,
	kEoB1Npc4Strings,
	kEoB1Npc5Strings,
	kEoB1Npc6Strings,
	kEoB1Npc7Strings,

	kEoB1ParchmentStrings,
	kEoB1ItemNames,
	kEoB1SpeechAnimData,
	kEoB1WdAnimSprites,
	kEoB1SequenceTrackMap,

	kEoB1MapStrings1,
	kEoB1MapStrings2,
	kEoB1MapStrings3,
	kEoB1MapLevelData,

	kEoB1Ascii2SjisTable1,
	kEoB1Ascii2SjisTable2,
	kEoB1FontLookupTable,
	kEoB1CharWidthTable1,
	kEoB1CharWidthTable2,
	kEoB1CharWidthTable3,
	kEoB1CharTilesTable,

	kEoB2MainMenuStrings,
	kEoB2MainMenuUtilStrings,

	kEoB2TransferPortraitFrames,
	kEoB2TransferConvertTable,
	kEoB2TransferItemTable,
	kEoB2TransferExpTable,
	kEoB2TransferStrings1,
	kEoB2TransferStrings2,
	kEoB2TransferLabels,

	kEoB2IntroStrings,
	kEoB2IntroCPSFiles,
	kEoB2IntroAnimData00,
	kEoB2IntroAnimData01,
	kEoB2IntroAnimData02,
	kEoB2IntroAnimData03,
	kEoB2IntroAnimData04,
	kEoB2IntroAnimData05,
	kEoB2IntroAnimData06,
	kEoB2IntroAnimData07,
	kEoB2IntroAnimData08,
	kEoB2IntroAnimData09,
	kEoB2IntroAnimData10,
	kEoB2IntroAnimData11,
	kEoB2IntroAnimData12,
	kEoB2IntroAnimData13,
	kEoB2IntroAnimData14,
	kEoB2IntroAnimData15,
	kEoB2IntroAnimData16,
	kEoB2IntroAnimData17,
	kEoB2IntroAnimData18,
	kEoB2IntroAnimData19,
	kEoB2IntroAnimData20,
	kEoB2IntroAnimData21,
	kEoB2IntroAnimData22,
	kEoB2IntroAnimData23,
	kEoB2IntroAnimData24,
	kEoB2IntroAnimData25,
	kEoB2IntroAnimData26,
	kEoB2IntroAnimData27,
	kEoB2IntroAnimData28,
	kEoB2IntroAnimData29,
	kEoB2IntroAnimData30,
	kEoB2IntroAnimData31,
	kEoB2IntroAnimData32,
	kEoB2IntroAnimData33,
	kEoB2IntroAnimData34,
	kEoB2IntroAnimData35,
	kEoB2IntroAnimData36,
	kEoB2IntroAnimData37,
	kEoB2IntroAnimData38,
	kEoB2IntroAnimData39,
	kEoB2IntroAnimData40,
	kEoB2IntroAnimData41,
	kEoB2IntroAnimData42,
	kEoB2IntroAnimData43,

	kEoB2IntroShapes00,
	kEoB2IntroShapes01,
	kEoB2IntroShapes04,
	kEoB2IntroShapes07,
	kEoB2IntroShapes13,
	kEoB2IntroShapes14,
	kEoB2IntroShapes15,

	kEoB2FinaleStrings,
	kEoB2CreditsData,
	kEoB2FinaleCPSFiles,
	kEoB2FinaleAnimData00,
	kEoB2FinaleAnimData01,
	kEoB2FinaleAnimData02,
	kEoB2FinaleAnimData03,
	kEoB2FinaleAnimData04,
	kEoB2FinaleAnimData05,
	kEoB2FinaleAnimData06,
	kEoB2FinaleAnimData07,
	kEoB2FinaleAnimData08,
	kEoB2FinaleAnimData09,
	kEoB2FinaleAnimData10,
	kEoB2FinaleAnimData11,
	kEoB2FinaleAnimData12,
	kEoB2FinaleAnimData13,
	kEoB2FinaleAnimData14,
	kEoB2FinaleAnimData15,
	kEoB2FinaleAnimData16,
	kEoB2FinaleAnimData17,
	kEoB2FinaleAnimData18,
	kEoB2FinaleAnimData19,
	kEoB2FinaleAnimData20,
	kEoB2FinaleShapes00,
	kEoB2FinaleShapes03,
	kEoB2FinaleShapes07,
	kEoB2FinaleShapes09,
	kEoB2FinaleShapes10,

	kEoB2NpcShapeData,
	kEoB2Npc1Strings,
	kEoB2Npc2Strings,
	kEoB2MonsterDustStrings,

	kEoB2DreamSteps,
	kEoB2KheldranStrings,
	kEoB2HornStrings,
	kEoB2HornSounds,

	kEoB2WallOfForceDsX,
	kEoB2WallOfForceDsY,
	kEoB2WallOfForceNumW,
	kEoB2WallOfForceNumH,
	kEoB2WallOfForceShpId,

	kEoB2IntroCpsDataStreet1,
	kEoB2IntroCpsDataStreet2,
	kEoB2IntroCpsDataDoorway1,
	kEoB2IntroCpsDataDoorway2,
	kEoB2IntroCpsDataWestwood,
	kEoB2IntroCpsDataWinding,
	kEoB2IntroCpsDataKhelban2,
	kEoB2IntroCpsDataKhelban1,
	kEoB2IntroCpsDataKhelban3,
	kEoB2IntroCpsDataKhelban4,
	kEoB2IntroCpsDataCoin,
	kEoB2IntroCpsDataKhelban5,
	kEoB2IntroCpsDataKhelban6,

	kEoB2FinaleCpsDataDragon1,
	kEoB2FinaleCpsDataDragon2,
	kEoB2FinaleCpsDataHurry1,
	kEoB2FinaleCpsDataHurry2,
	kEoB2FinaleCpsDataDestroy0,
	kEoB2FinaleCpsDataDestroy1,
	kEoB2FinaleCpsDataDestroy2,
	kEoB2FinaleCpsDataMagic,
	kEoB2FinaleCpsDataDestroy3,
	kEoB2FinaleCpsDataCredits2,
	kEoB2FinaleCpsDataCredits3,
	kEoB2FinaleCpsDataHeroes,
	kEoB2FinaleCpsDataThanks,

	kEoB2ItemIconShapeData00,
	kEoB2ItemIconShapeData01,
	kEoB2ItemIconShapeData02,
	kEoB2ItemIconShapeData03,
	kEoB2ItemIconShapeData04,
	kEoB2ItemIconShapeData05,
	kEoB2ItemIconShapeData06,
	kEoB2ItemIconShapeData07,
	kEoB2ItemIconShapeData08,
	kEoB2ItemIconShapeData09,
	kEoB2ItemIconShapeData10,
	kEoB2ItemIconShapeData11,
	kEoB2ItemIconShapeData12,
	kEoB2ItemIconShapeData13,
	kEoB2ItemIconShapeData14,
	kEoB2ItemIconShapeData15,
	kEoB2ItemIconShapeData16,
	kEoB2ItemIconShapeData17,
	kEoB2ItemIconShapeData18,
	kEoB2ItemIconShapeData19,
	kEoB2ItemIconShapeData20,
	kEoB2ItemIconShapeData21,
	kEoB2ItemIconShapeData22,
	kEoB2ItemIconShapeData23,
	kEoB2ItemIconShapeData24,
	kEoB2ItemIconShapeData25,
	kEoB2ItemIconShapeData26,
	kEoB2ItemIconShapeData27,
	kEoB2ItemIconShapeData28,
	kEoB2ItemIconShapeData29,
	kEoB2ItemIconShapeData30,
	kEoB2ItemIconShapeData31,
	kEoB2ItemIconShapeData32,
	kEoB2ItemIconShapeData33,
	kEoB2ItemIconShapeData34,
	kEoB2ItemIconShapeData35,
	kEoB2ItemIconShapeData36,
	kEoB2ItemIconShapeData37,
	kEoB2ItemIconShapeData38,
	kEoB2ItemIconShapeData39,
	kEoB2ItemIconShapeData40,
	kEoB2ItemIconShapeData41,
	kEoB2ItemIconShapeData42,
	kEoB2ItemIconShapeData43,
	kEoB2ItemIconShapeData44,
	kEoB2ItemIconShapeData45,
	kEoB2ItemIconShapeData46,
	kEoB2ItemIconShapeData47,
	kEoB2ItemIconShapeData48,
	kEoB2ItemIconShapeData49,
	kEoB2ItemIconShapeData50,
	kEoB2ItemIconShapeData51,
	kEoB2ItemIconShapeData52,
	kEoB2ItemIconShapeData53,
	kEoB2ItemIconShapeData54,
	kEoB2ItemIconShapeData55,
	kEoB2ItemIconShapeData56,
	kEoB2ItemIconShapeData57,
	kEoB2ItemIconShapeData58,
	kEoB2ItemIconShapeData59,
	kEoB2ItemIconShapeData60,
	kEoB2ItemIconShapeData61,
	kEoB2ItemIconShapeData62,
	kEoB2ItemIconShapeData63,
	kEoB2ItemIconShapeData64,
	kEoB2ItemIconShapeData65,
	kEoB2ItemIconShapeData66,
	kEoB2ItemIconShapeData67,
	kEoB2ItemIconShapeData68,
	kEoB2ItemIconShapeData69,
	kEoB2ItemIconShapeData70,
	kEoB2ItemIconShapeData71,
	kEoB2ItemIconShapeData72,
	kEoB2ItemIconShapeData73,
	kEoB2ItemIconShapeData74,
	kEoB2ItemIconShapeData75,
	kEoB2ItemIconShapeData76,
	kEoB2ItemIconShapeData77,
	kEoB2ItemIconShapeData78,
	kEoB2ItemIconShapeData79,
	kEoB2ItemIconShapeData80,
	kEoB2ItemIconShapeData81,
	kEoB2ItemIconShapeData82,
	kEoB2ItemIconShapeData83,
	kEoB2ItemIconShapeData84,
	kEoB2ItemIconShapeData85,
	kEoB2ItemIconShapeData86,
	kEoB2ItemIconShapeData87,
	kEoB2ItemIconShapeData88,
	kEoB2ItemIconShapeData89,
	kEoB2ItemIconShapeData90,
	kEoB2ItemIconShapeData91,
	kEoB2ItemIconShapeData92,
	kEoB2ItemIconShapeData93,
	kEoB2ItemIconShapeData94,
	kEoB2ItemIconShapeData95,
	kEoB2ItemIconShapeData96,
	kEoB2ItemIconShapeData97,
	kEoB2ItemIconShapeData98,
	kEoB2ItemIconShapeData99,
	kEoB2ItemIconShapeData100,
	kEoB2ItemIconShapeData101,
	kEoB2ItemIconShapeData102,
	kEoB2ItemIconShapeData103,
	kEoB2ItemIconShapeData104,
	kEoB2ItemIconShapeData105,
	kEoB2ItemIconShapeData106,
	kEoB2ItemIconShapeData107,
	kEoB2ItemIconShapeData108,
	kEoB2ItemIconShapeData109,
	kEoB2ItemIconShapeData110,
	kEoB2ItemIconShapeData111,

	kEoB2LargeItemsShapeData00,
	kEoB2LargeItemsShapeData01,
	kEoB2LargeItemsShapeData02,
	kEoB2LargeItemsShapeData03,
	kEoB2LargeItemsShapeData04,
	kEoB2LargeItemsShapeData05,
	kEoB2LargeItemsShapeData06,
	kEoB2LargeItemsShapeData07,
	kEoB2LargeItemsShapeData08,
	kEoB2LargeItemsShapeData09,
	kEoB2LargeItemsShapeData10,

	kEoB2SmallItemsShapeData00,
	kEoB2SmallItemsShapeData01,
	kEoB2SmallItemsShapeData02,
	kEoB2SmallItemsShapeData03,
	kEoB2SmallItemsShapeData04,
	kEoB2SmallItemsShapeData05,
	kEoB2SmallItemsShapeData06,
	kEoB2SmallItemsShapeData07,
	kEoB2SmallItemsShapeData08,
	kEoB2SmallItemsShapeData09,
	kEoB2SmallItemsShapeData10,
	kEoB2SmallItemsShapeData11,
	kEoB2SmallItemsShapeData12,
	kEoB2SmallItemsShapeData13,
	kEoB2SmallItemsShapeData14,
	kEoB2SmallItemsShapeData15,
	kEoB2SmallItemsShapeData16,
	kEoB2SmallItemsShapeData17,
	kEoB2SmallItemsShapeData18,
	kEoB2SmallItemsShapeData19,
	kEoB2SmallItemsShapeData20,
	kEoB2SmallItemsShapeData21,
	kEoB2SmallItemsShapeData22,
	kEoB2SmallItemsShapeData23,
	kEoB2SmallItemsShapeData24,
	kEoB2SmallItemsShapeData25,

	kEoB2ThrownShapeData00,
	kEoB2ThrownShapeData01,
	kEoB2ThrownShapeData02,
	kEoB2ThrownShapeData03,
	kEoB2ThrownShapeData04,
	kEoB2ThrownShapeData05,
	kEoB2ThrownShapeData06,
	kEoB2ThrownShapeData07,
	kEoB2ThrownShapeData08,

	kEoB2SpellShapeData00,
	kEoB2SpellShapeData01,
	kEoB2SpellShapeData02,
	kEoB2SpellShapeData03,

	kEoB2TeleporterShapeData00,
	kEoB2TeleporterShapeData01,
	kEoB2TeleporterShapeData02,
	kEoB2TeleporterShapeData03,
	kEoB2TeleporterShapeData04,
	kEoB2TeleporterShapeData05,

	kEoB2LightningColumnShapeData,
	kEoB2DeadCharShapeData,
	kEoB2DisabledCharGridShapeData,
	kEoB2WeaponSlotGridShapeData,
	kEoB2SmallGridShapeData,
	kEoB2WideGridShapeData,
	kEoB2RedSplatShapeData,
	kEoB2GreenSplatShapeData,

	kEoB2FirebeamShapeData00,
	kEoB2FirebeamShapeData01,
	kEoB2FirebeamShapeData02,

	kEoB2SparkShapeData00,
	kEoB2SparkShapeData01,
	kEoB2SparkShapeData02,

	kEoB2CompassShapeData00,
	kEoB2CompassShapeData01,
	kEoB2CompassShapeData02,
	kEoB2CompassShapeData03,
	kEoB2CompassShapeData04,
	kEoB2CompassShapeData05,
	kEoB2CompassShapeData06,
	kEoB2CompassShapeData07,
	kEoB2CompassShapeData08,
	kEoB2CompassShapeData09,
	kEoB2CompassShapeData10,
	kEoB2CompassShapeData11,

	kEoB2WallOfForceShapeData00,
	kEoB2WallOfForceShapeData01,
	kEoB2WallOfForceShapeData02,
	kEoB2WallOfForceShapeData03,
	kEoB2WallOfForceShapeData04,
	kEoB2WallOfForceShapeData05,

	kEoB2UtilMenuStrings,
	kEoB2Config2431Strings,
	kEoB2FontDmpSearchTbl,
	kEoB2Ascii2SjisTables,
	kEoB2Ascii2SjisTables2,
	kEoB2PcmSoundEffectsIngame,
	kEoB2PcmSoundEffectsIntro,
	kEoB2PcmSoundEffectsFinale,

	kEoB2SoundMapExtra,
	kEoB2SoundIndex1,
	kEoB2SoundIndex2,
	kEoB2SoundFilesIngame2,
	kEoB2MonsterSoundPatchData,

	kLoLIngamePakFiles,
	kLoLCharacterDefs,
	kLoLIngameSfxFiles,
	kLoLIngameSfxIndex,
	kLoLMusicTrackMap,
	kLoLIngameGMSfxIndex,
	kLoLIngameMT32SfxIndex,
	kLoLIngamePcSpkSfxIndex,
	kLoLSpellProperties,
	kLoLGameShapeMap,
	kLoLSceneItemOffs,
	kLoLCharInvIndex,
	kLoLCharInvDefs,
	kLoLCharDefsMan,
	kLoLCharDefsWoman,
	kLoLCharDefsKieran,
	kLoLCharDefsAkshel,
	kLoLExpRequirements,
	kLoLMonsterModifiers1,
	kLoLMonsterModifiers2,
	kLoLMonsterModifiers3,
	kLoLMonsterModifiers4,
	kLoLMonsterShiftOffsets,
	kLoLMonsterDirFlags,
	kLoLMonsterScaleY,
	kLoLMonsterScaleX,
	kLoLMonsterScaleWH,
	kLoLFlyingObjectShp,
	kLoLInventoryDesc,

	kLoLLevelShpList,
	kLoLLevelDatList,
	kLoLCompassDefs,
	kLoLItemPrices,
	kLoLStashSetup,

	kLoLDscWalls,
	kLoLDscOvlMap,
	kLoLDscScaleWidthData,
	kLoLDscScaleHeightData,
	kLoLBaseDscY,

	kLoLDscDoorScale,
	kLoLDscDoor4,
	kLoLDscDoorX,
	kLoLDscDoorY,
	kLoLDscOvlIndex,

	kLoLScrollXTop,
	kLoLScrollYTop,
	kLoLScrollXBottom,
	kLoLScrollYBottom,

	kLoLButtonDefs,
	kLoLButtonList1,
	kLoLButtonList2,
	kLoLButtonList3,
	kLoLButtonList4,
	kLoLButtonList5,
	kLoLButtonList6,
	kLoLButtonList7,
	kLoLButtonList8,

	kLoLLegendData,
	kLoLMapCursorOvl,
	kLoLMapStringId,

	kLoLSpellbookAnim,
	kLoLSpellbookCoords,
	kLoLHealShapeFrames,
	kLoLLightningDefs,
	kLoLFireballCoords,

	kLoLCredits,

	kLoLHistory,
#endif // ENABLE_EOB || ENABLE_LOL

	kMaxResIDs
};

struct Shape;
struct Room;
struct AmigaSfxTable;
struct HoFSeqData;
struct HoFSeqItemAnimData;

class StaticResource {
public:
	static const Common::String staticDataFilename() { return "KYRA.DAT"; }

	StaticResource(KyraEngine_v1 *vm) : _vm(vm), _resList(), _fileLoader(0), _dataTable() {}
	~StaticResource() { deinit(); }

	bool loadStaticResourceFile();

	bool init();
	void deinit();

	const char *const *loadStrings(int id, int &strings);
	const uint8 *loadRawData(int id, int &size);
	const Shape *loadShapeTable(int id, int &entries);
	const AmigaSfxTable *loadAmigaSfxTable(int id, int &entries);
	const Room *loadRoomTable(int id, int &entries);
	const HoFSeqData *loadHoFSequenceData(int id, int &entries);
	const HoFSeqItemAnimData *loadHoFSeqItemAnimData(int id, int &entries);
	const ItemAnimDefinition *loadItemAnimDefinition(int id, int &entries);
#if defined(ENABLE_EOB) || defined(ENABLE_LOL)
	const uint16 *loadRawDataBe16(int id, int &entries);
	const uint32 *loadRawDataBe32(int id, int &entries);
#endif // (ENABLE_EOB || ENABLE_LOL)
#ifdef ENABLE_LOL
	const LoLCharacter *loadCharData(int id, int &entries);
	const SpellProperty *loadSpellData(int id, int &entries);
	const CompassDef *loadCompassData(int id, int &entries);
	const FlyingObjectShape *loadFlyingObjectData(int id, int &entries);
	const LoLButtonDef *loadButtonDefs(int id, int &entries);
#endif // ENABLE_LOL
#ifdef ENABLE_EOB
	const DarkMoonAnimCommand *loadEoB2SeqData(int id, int &entries);
	const DarkMoonShapeDef *loadEoB2ShapeData(int id, int &entries);
	const EoBCharacter *loadEoBNpcData(int id, int &entries);
#endif // ENABLE_EOB

	// use '-1' to prefetch/unload all ids
	// prefetchId retruns false if only on of the resources
	// can't be loaded and it breaks then the first res
	// can't be loaded
	bool prefetchId(int id);
	void unloadId(int id);
private:
	bool tryKyraDatLoad();

	KyraEngine_v1 *_vm;

	struct FileType;

	bool checkResList(int id, int &type, const void *&ptr, int &size);
	const FileType *getFiletype(int type);
	const void *getData(int id, int requesttype, int &size);

	bool loadDummy(Common::SeekableReadStream &stream, void *&ptr, int &size);
	bool loadStringTable(Common::SeekableReadStream &stream, void *&ptr, int &size);
	bool loadRawData(Common::SeekableReadStream &stream, void *&ptr, int &size);
	bool loadShapeTable(Common::SeekableReadStream &stream, void *&ptr, int &size);
	bool loadAmigaSfxTable(Common::SeekableReadStream &stream, void *&ptr, int &size);
	bool loadRoomTable(Common::SeekableReadStream &stream, void *&ptr, int &size);
	bool loadHoFSequenceData(Common::SeekableReadStream &stream, void *&ptr, int &size);
	bool loadHoFSeqItemAnimData(Common::SeekableReadStream &stream, void *&ptr, int &size);
	bool loadItemAnimDefinition(Common::SeekableReadStream &stream, void *&ptr, int &size);
#if defined(ENABLE_EOB) || defined(ENABLE_LOL)
	bool loadRawDataBe16(Common::SeekableReadStream &stream, void *&ptr, int &size);
	bool loadRawDataBe32(Common::SeekableReadStream &stream, void *&ptr, int &size);
#endif // (ENABLE_LOL || ENABLE_EOB)
#ifdef ENABLE_LOL
	bool loadCharData(Common::SeekableReadStream &stream, void *&ptr, int &size);
	bool loadSpellData(Common::SeekableReadStream &stream, void *&ptr, int &size);
	bool loadCompassData(Common::SeekableReadStream &stream, void *&ptr, int &size);
	bool loadFlyingObjectData(Common::SeekableReadStream &stream, void *&ptr, int &size);
	bool loadButtonDefs(Common::SeekableReadStream &stream, void *&ptr, int &size);
#endif // ENABLE_LOL
#ifdef ENABLE_EOB
	bool loadEoB2SeqData(Common::SeekableReadStream &stream, void *&ptr, int &size);
	bool loadEoB2ShapeData(Common::SeekableReadStream &stream, void *&ptr, int &size);
	bool loadEoBNpcData(Common::SeekableReadStream &stream, void *&ptr, int &size);
#endif // ENABLE_EOB

	void freeDummy(void *&ptr, int &size);
	void freeRawData(void *&ptr, int &size);
	void freeStringTable(void *&ptr, int &size);
	void freeShapeTable(void *&ptr, int &size);
	void freeAmigaSfxTable(void *&ptr, int &size);
	void freeRoomTable(void *&ptr, int &size);
	void freeHoFSequenceData(void *&ptr, int &size);
	void freeHoFSeqItemAnimData(void *&ptr, int &size);
	void freeItemAnimDefinition(void *&ptr, int &size);
#if defined(ENABLE_EOB) || defined(ENABLE_LOL)
	void freeRawDataBe16(void *&ptr, int &size);
	void freeRawDataBe32(void *&ptr, int &size);
#endif // (ENABLE_EOB || ENABLE_LOL)
#ifdef ENABLE_LOL
	void freeCharData(void *&ptr, int &size);
	void freeSpellData(void *&ptr, int &size);
	void freeCompassData(void *&ptr, int &size);
	void freeFlyingObjectData(void *&ptr, int &size);
	void freeButtonDefs(void *&ptr, int &size);
#endif // ENABLE_LOL
#ifdef ENABLE_EOB
	void freeEoB2SeqData(void *&ptr, int &size);
	void freeEoB2ShapeData(void *&ptr, int &size);
	void freeEoBNpcData(void *&ptr, int &size);
#endif // ENABLE_EOB

	enum ResTypes {
		kStringList = 0,
		kRawData = 1,
		kRoomList = 2,
		kShapeList = 3,
		kAmigaSfxTable = 4,

		k2SeqData = 5,
		k2SeqItemAnimData = 6,
		k2ItemAnimDefinition = 7,

		kLoLCharData = 8,
		kLoLSpellData = 9,
		kLoLCompassData = 10,
		kLoLFlightShpData = 11,
		kLoLButtonData = 12,
		kRawDataBe16 = 13,
		kRawDataBe32 = 14,

		kEoB2SequenceData = 15,
		kEoB2ShapeData = 16,
		kEoBNpcData = 17
	};

	struct FileType {
		int type;
		typedef bool (StaticResource::*LoadFunc)(Common::SeekableReadStream &stream, void *&ptr, int &size);
		typedef void (StaticResource::*FreeFunc)(void *&ptr, int &size);

		LoadFunc load;
		FreeFunc free;
	};

	struct ResData {
		int id;
		int type;
		int size;
		void *data;
	};

	Common::List<ResData> _resList;

	const FileType *_fileLoader;

	struct DataDescriptor {
		DataDescriptor() : filename(0), type(0) {}
		DataDescriptor(uint32 f, uint8 t) : filename(f), type(t) {}

		uint32 filename;
		uint8 type;
	};
	typedef Common::HashMap<uint16, DataDescriptor> DataMap;
	DataMap _dataTable;
};

} // End of namespace Kyra

#endif
