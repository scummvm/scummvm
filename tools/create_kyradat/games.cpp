/* ScummVM Tools
 * Copyright (C) 2009 The ScummVM project
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

#include "create_kyradat.h"

// Game tables

namespace {

const Game kyra1Games[] = {
	// Demos
	{ kKyra1, EN_ANY, kPlatformPC, kDemoVersion, "7b7504c8560ffc914d34c44c71b3094c" },
	{ kKyra1, EN_ANY, kPlatformPC, kDemoCDVersion, "226fdba99cb11ef1047131d9a50e6292" },

	// Amiga
	{ kKyra1, EN_ANY, kPlatformAmiga, -1, "b620564b6b7e0787b053ca9e35bd9f52" },

	// Floppy
	{ kKyra1, EN_ANY, kPlatformPC, -1, "76a4fc84e173cadb6369785787e1546e" },
	{ kKyra1, DE_DEU, kPlatformPC, -1, "9442d6f7db6a41f3dd4aa4de5d36e107" },
	{ kKyra1, FR_FRA, kPlatformPC, -1, "aa9d6d78d8b199deaf48efeca6d19af2" },
	{ kKyra1, IT_ITA, kPlatformPC, -1, "5d7550306b369a3492f9f3402702477c" },
	{ kKyra1, ES_ESP, kPlatformPC, -1, "9ff130d2558bcd674d4074849d93c362" },

	// Talkie
	{ kKyra1, EN_ANY, kPlatformPC, kTalkieVersion, "1ebc18f3e7fbb72474a55cb0fa089ed4" },
	{ kKyra1, DE_DEU, kPlatformPC, kTalkieVersion, "c65d381184f98ac26d9efd2d45baef51" },
	{ kKyra1, FR_FRA, kPlatformPC, kTalkieVersion, "307c5d4a554d9068ac3d326e350ae4a6" },
	{ kKyra1, IT_ITA, kPlatformPC, kTalkieVersion, "d0f1752098236083d81b9497bd2b6989" }, // Italian fan translation

	// FM-TOWNS
	{ kKyra1, EN_ANY, kPlatformFMTowns, kFMTownsVersionE, "5a3ad60ccd0f2e29463e0368cd14a60d" },
	{ kKyra1, JA_JPN, kPlatformFMTowns, kFMTownsVersionJ, "5a3ad60ccd0f2e29463e0368cd14a60d" },

	GAME_DUMMY_ENTRY
};

const Game kyra2Games[] = {
	// demos
	{ kKyra2, EN_ANY, kPlatformPC, k2DemoVersion, "a620a37579dd44ab0403482285e3897f" },
	{ kKyra2, EN_ANY, kPlatformPC, k2CDDemoE, "fa54d8abfe05f9186c05f7de7eaf1480" },
	{ kKyra2, FR_FRA, kPlatformPC, k2CDDemoF, "fa54d8abfe05f9186c05f7de7eaf1480" },
	{ kKyra2, DE_DEU, kPlatformPC, k2CDDemoG, "fa54d8abfe05f9186c05f7de7eaf1480" },

	// floppy games
	{ kKyra2, EN_ANY, kPlatformPC, k2FloppyFile1, "9b0f5e57b5a2ed88b5b989cbb402b6c7" },
	{ kKyra2, FR_FRA, kPlatformPC, k2FloppyFile1, "df31cc9e37e1cf68df2fdc75ddf2d87b" },
	{ kKyra2, DE_DEU, kPlatformPC, k2FloppyFile1, "0ca4f9a1438264a4c63c3218e064ed3b" },
	{ kKyra2, IT_ITA, kPlatformPC, k2FloppyFile1, "178d3ab913f61bfba21d2fb196405e8c" },
	{ kKyra2, EN_ANY, kPlatformPC, k2FloppyFile2, "7c3eadbe5122722cf2e5e1611e19dfb9" },
	{ kKyra2, FR_FRA, kPlatformPC, k2FloppyFile2, "fc2c6782778e6c6d5a553d1cb73c98ad" },
	{ kKyra2, DE_DEU, kPlatformPC, k2FloppyFile2, "0d9b0eb7b0ad889ec942d74d80dde1bf" },
	{ kKyra2, IT_ITA, kPlatformPC, k2FloppyFile2, "3a61ed6b7c00ddae383a0361799e2ba6" },

	// talkie games
	{ kKyra2, EN_ANY, kPlatformPC, k2CDFile1E, "85bbc1cc6c4cef6ad31fc6ee79518efb" },
	{ kKyra2, FR_FRA, kPlatformPC, k2CDFile1F, "85bbc1cc6c4cef6ad31fc6ee79518efb" },
	{ kKyra2, DE_DEU, kPlatformPC, k2CDFile1G, "85bbc1cc6c4cef6ad31fc6ee79518efb" },
	{ kKyra2, EN_ANY, kPlatformPC, k2CDFile2E, "e20d0d2e500f01e399ec588247a7e213" },
	{ kKyra2, FR_FRA, kPlatformPC, k2CDFile2F, "e20d0d2e500f01e399ec588247a7e213" },
	{ kKyra2, DE_DEU, kPlatformPC, k2CDFile2G, "e20d0d2e500f01e399ec588247a7e213" },
	{ kKyra2, IT_ITA, kPlatformPC, k2CDFile1I, "130795aa8f2333250c895dae9028b9bb" }, // Italian Fan Translation (using same offsets as English)

	// FM-TOWNS games
	{ kKyra2, EN_ANY, kPlatformFMTowns, k2TownsFile1E, "74f50d79c919cc8e7196c24942ce43d7" },
	{ kKyra2, JA_JPN, kPlatformFMTowns, k2TownsFile1J, "74f50d79c919cc8e7196c24942ce43d7" },
	{ kKyra2, EN_ANY, kPlatformFMTowns, k2TownsFile2E, "a9a7fd4f05d00090e9e8bda073e6d431" },
	{ kKyra2, JA_JPN, kPlatformFMTowns, k2TownsFile2J, "a9a7fd4f05d00090e9e8bda073e6d431" },

	GAME_DUMMY_ENTRY
};

const Game kyra3Games[] = {
	{ kKyra3, EN_ANY, kPlatformPC, -1, "bf68701eb591d0b72219f314c0d32688" },
	GAME_DUMMY_ENTRY
};

const Game lolGames[] = {
	// DOS demo
	{ kLol, EN_ANY, kPlatformPC, k2DemoLol, "30bb5af87d38adb47d3e6ce06b1cb042" },

	// DOS floppy
	{ kLol, EN_ANY, kPlatformPC, -1, "6b843869772c1b779e1386be868c15dd" },

	// DOS CD
	{ kLol, EN_ANY, kPlatformPC, kLolCD1, "9d1778314de80598c0b0d032e2a1a1cf" },
	{ kLol, EN_ANY, kPlatformPC, kLolCD2, "263998ec600afca1cc7b935c473df670" },

	GAME_DUMMY_ENTRY
};

} // end of anonymous namespace

const Game *gameDescs[] = {
	kyra1Games,
	kyra2Games,
	kyra3Games,
	lolGames,
	0
};

// Need tables

namespace {

const int kyra1FloppyNeed[] = {
	kKallakWritingSeq,
	kMalcolmTreeSeq,
	kWestwoodLogoSeq,
	kKyrandiaLogoSeq,
	kKallakMalcolmSeq,
	kForestSeq,
	kIntroCPSStrings,
	kIntroCOLStrings,
	kIntroWSAStrings,
	kIntroStrings,
	kRoomList,
	kRoomFilenames,
	kCharacterImageFilenames,
	kDefaultShapes,
	kItemNames,
	kTakenStrings,
	kPlacedStrings,
	kDroppedStrings,
	kNoDropStrings,
	kAmuleteAnimSeq,
	kPaletteList1,
	kPaletteList2,
	kPaletteList3,
	kPaletteList4,
	kPaletteList5,
	kPaletteList6,
	kPaletteList7,
	kPaletteList8,
	kPaletteList9,
	kPaletteList10,
	kPaletteList11,
	kPaletteList12,
	kPaletteList13,
	kPaletteList14,
	kPaletteList15,
	kPaletteList16,
	kPaletteList17,
	kPaletteList18,
	kPaletteList19,
	kPaletteList20,
	kPaletteList21,
	kPaletteList22,
	kPaletteList23,
	kPaletteList24,
	kPaletteList25,
	kPaletteList26,
	kPaletteList27,
	kPaletteList28,
	kPaletteList29,
	kPaletteList30,
	kPaletteList31,
	kPaletteList32,
	kPaletteList33,
	kPutDownString,
	kWaitAmuletString,
	kBlackJewelString,
	kHealingTipString,
	kPoisonGoneString,
	kHealing1Shapes,
	kHealing2Shapes,
	kThePoisonStrings,
	kFluteStrings,
	kPoisonDeathShapes,
	kFluteShapes,
	kWinter1Shapes,
	kWinter2Shapes,
	kWinter3Shapes,
	kDrinkShapes,
	kWispShapes,
	kMagicAnimShapes,
	kBranStoneShapes,
	kWispJewelStrings,
	kMagicJewelStrings,
	kFlaskFullString,
	kFullFlaskString,
	kOutroReunionSeq,
	kOutroHomeString,
	kVeryCleverString,
	kGUIStrings,
	kNewGameString,
	kConfigStrings,
	kAudioTracks,
	kAudioTracksIntro,
	-1
};

const int kyra1CDNeed[] = {
	kKallakWritingSeq,
	kMalcolmTreeSeq,
	kWestwoodLogoSeq,
	kKyrandiaLogoSeq,
	kKallakMalcolmSeq,
	kForestSeq,
	kIntroCPSStrings,
	kIntroCOLStrings,
	kIntroWSAStrings,
	kIntroStrings,
	kRoomList,
	kRoomFilenames,
	kCharacterImageFilenames,
	kDefaultShapes,
	kItemNames,
	kTakenStrings,
	kPlacedStrings,
	kDroppedStrings,
	kNoDropStrings,
	kAmuleteAnimSeq,
	kPaletteList1,
	kPaletteList2,
	kPaletteList3,
	kPaletteList4,
	kPaletteList5,
	kPaletteList6,
	kPaletteList7,
	kPaletteList8,
	kPaletteList9,
	kPaletteList10,
	kPaletteList11,
	kPaletteList12,
	kPaletteList13,
	kPaletteList14,
	kPaletteList15,
	kPaletteList16,
	kPaletteList17,
	kPaletteList18,
	kPaletteList19,
	kPaletteList20,
	kPaletteList21,
	kPaletteList22,
	kPaletteList23,
	kPaletteList24,
	kPaletteList25,
	kPaletteList26,
	kPaletteList27,
	kPaletteList28,
	kPaletteList29,
	kPaletteList30,
	kPaletteList31,
	kPaletteList32,
	kPaletteList33,
	kPutDownString,
	kWaitAmuletString,
	kBlackJewelString,
	kHealingTipString,
	kPoisonGoneString,
	kHealing1Shapes,
	kHealing2Shapes,
	kThePoisonStrings,
	kFluteStrings,
	kPoisonDeathShapes,
	kFluteShapes,
	kWinter1Shapes,
	kWinter2Shapes,
	kWinter3Shapes,
	kDrinkShapes,
	kWispShapes,
	kMagicAnimShapes,
	kBranStoneShapes,
	kWispJewelStrings,
	kMagicJewelStrings,
	kFlaskFullString,
	kFullFlaskString,
	kOutroReunionSeq,
	kOutroHomeString,
	kVeryCleverString,
	kGUIStrings,
	kNewGameString,
	kConfigStrings,
	kAudioTracks,
	kAudioTracksIntro,
	-1
};

const int kyra1DemoNeed[] = {
	kWestwoodLogoSeq,
	kKyrandiaLogoSeq,
	kIntroCPSStrings,
	kIntroCOLStrings,
	kIntroWSAStrings,
	kIntroStrings,
	kDemo1Seq,
	kDemo2Seq,
	kDemo3Seq,
	kDemo4Seq,
	kAudioTracksIntro,
	-1
};

const int kyra1DemoCDNeed[] = {
	kKallakWritingSeq,
	kMalcolmTreeSeq,
	kWestwoodLogoSeq,
	kKyrandiaLogoSeq,
	kKallakMalcolmSeq,
	kForestSeq,
	kIntroCPSStrings,
	kIntroCOLStrings,
	kIntroWSAStrings,
	kIntroStrings,
	kAudioTracksIntro,
	-1
};

const int kyra1TownsNeed[] = {
	kKallakWritingSeq,
	kMalcolmTreeSeq,
	kWestwoodLogoSeq,
	kKyrandiaLogoSeq,
	kKallakMalcolmSeq,
	kForestSeq,
	kIntroCPSStrings,
	kIntroCOLStrings,
	kIntroWSAStrings,
	kIntroStrings,
	kRoomList,
	kRoomFilenames,
	kCharacterImageFilenames,
	kDefaultShapes,
	kItemNames,
	kTakenStrings,
	kPlacedStrings,
	kDroppedStrings,
	kNoDropStrings,
	kAmuleteAnimSeq,
	kPaletteList1,
	kPaletteList2,
	kPaletteList3,
	kPaletteList4,
	kPaletteList5,
	kPaletteList6,
	kPaletteList7,
	kPaletteList8,
	kPaletteList9,
	kPaletteList10,
	kPaletteList11,
	kPaletteList12,
	kPaletteList13,
	kPaletteList14,
	kPaletteList15,
	kPaletteList16,
	kPaletteList17,
	kPaletteList18,
	kPaletteList19,
	kPaletteList20,
	kPaletteList21,
	kPaletteList22,
	kPaletteList23,
	kPaletteList24,
	kPaletteList25,
	kPaletteList26,
	kPaletteList27,
	kPaletteList28,
	kPaletteList29,
	kPaletteList30,
	kPaletteList31,
	kPaletteList32,
	kPaletteList33,
	kPutDownString,
	kWaitAmuletString,
	kBlackJewelString,
	kHealingTipString,
	kPoisonGoneString,
	kHealing1Shapes,
	kHealing2Shapes,
	kThePoisonStrings,
	kFluteStrings,
	kPoisonDeathShapes,
	kFluteShapes,
	kWinter1Shapes,
	kWinter2Shapes,
	kWinter3Shapes,
	kDrinkShapes,
	kWispShapes,
	kMagicAnimShapes,
	kBranStoneShapes,
	kWispJewelStrings,
	kMagicJewelStrings,
	kFlaskFullString,
	kFullFlaskString,
	kOutroReunionSeq,
	kOutroHomeString,
	kVeryCleverString,
	kGUIStrings,
	kNewGameString,
	kConfigStrings,

	kKyra1TownsSFXwdTable,
	kKyra1TownsSFXbtTable,
	kKyra1TownsCDATable,
	kAudioTracks,
	kCreditsStrings,
	-1
};

const int kyra1AmigaNeed[] = {
	kKallakWritingSeq,
	kMalcolmTreeSeq,
	kWestwoodLogoSeq,
	kKyrandiaLogoSeq,
	kKallakMalcolmSeq,
	kForestSeq,
	kIntroCPSStrings,
	kIntroWSAStrings,
	kIntroCOLStrings,
	kIntroStrings,
	kRoomList,
	kRoomFilenames,
	kCharacterImageFilenames,
	kDefaultShapes,
	kItemNames,
	kTakenStrings,
	kPlacedStrings,
	kDroppedStrings,
	kNoDropStrings,
	kAmuleteAnimSeq,
	kPutDownString,
	kWaitAmuletString,
	kBlackJewelString,
	kHealingTipString,
	kPoisonGoneString,
	kHealing1Shapes,
	kHealing2Shapes,
	kThePoisonStrings,
	kFluteStrings,
	kPoisonDeathShapes,
	kFluteShapes,
	kWinter1Shapes,
	kWinter2Shapes,
	kWinter3Shapes,
	kDrinkShapes,
	kWispShapes,
	kMagicAnimShapes,
	kBranStoneShapes,
	kWispJewelStrings,
	kMagicJewelStrings,
	kFlaskFullString,
	kFullFlaskString,
	kOutroReunionSeq,
	kOutroHomeString,
	kVeryCleverString,
	kGUIStrings,
	kNewGameString,
	kConfigStrings,
	kCreditsStrings,
	kAmigaIntroSFXTable,
	kAmigaGameSFXTable,
	-1
};

const int kyra2CDFile1Need[] = {
	k2SeqplayPakFiles,
	k2SeqplayCredits,
	k2SeqplayCreditsSpecial,
	k2SeqplayStrings,
	k2SeqplaySfxFiles,
	k2SeqplayTlkFiles,
	k2SeqplaySeqData,
	k2SeqplayIntroTracks,
	k2SeqplayFinaleTracks,
	-1
};

const int kyra2CDFile2Need[] = {
	k2IngameSfxFiles,
	k2IngameSfxIndex,
	k2IngameTracks,
	k2IngameTalkObjIndex,
	k2IngameItemAnimData,
	-1
};

const int kyra2CDDemoNeed[] = {
	k2IngameSfxFiles,
	k2IngameSfxIndex,
	k2IngameTracks,
	k2IngameTalkObjIndex,
	k2IngameItemAnimData,
	k2IngameTlkDemoStrings,
	-1
};

const int kyra2FloppyFile1Need[] = {
	k2SeqplayPakFiles,
	k2SeqplayStrings,
	k2SeqplaySfxFiles,
	k2SeqplayIntroTracks,
	k2SeqplayFinaleTracks,
	k2SeqplaySeqData,
	-1
};

const int kyra2FloppyFile2Need[] = {
	k2IngamePakFiles,
	k2IngameSfxFiles,
	k2IngameSfxIndex,
	k2IngameTracks,
	k2IngameTalkObjIndex,
	k2IngameItemAnimData,
	-1
};

const int kyra2TownsFile1Need[] = {
	k2SeqplayPakFiles,
	k2SeqplayStrings,
	k2SeqplaySfxFiles,
	k2SeqplaySeqData,
	k2SeqplayIntroCDA,
	k2SeqplayFinaleCDA,
	-1
};

const int kyra2TownsFile2Need[] = {
	k2IngamePakFiles,
	k2IngameSfxFilesTns,
	k2IngameSfxIndex,
	k2IngameCDA,
	k2IngameTalkObjIndex,
	k2IngameTimJpStrings,
	k2IngameItemAnimData,
	-1
};

const int kyra2DemoNeed[] = {
	k2SeqplayPakFiles,
	k2SeqplaySeqData,
	k2SeqplaySfxFiles,
	k2SeqplayIntroTracks,
	k2SeqplayShapeAnimData,
	-1
};

const int kyra2TlkDemoNeed[] = {
	k2IngameTlkDemoStrings,
	-1
};

const int kyra3Need[] = {
	k3MainMenuStrings,
	k3MusicFiles,
	k3ScoreTable,
	k3SfxFiles,
	k3SfxMap,
	k3ItemAnimData,
	k3ItemMagicTable,
	k3ItemStringMap,
	-1
};

const int lolFloppyNeed[] = {
	kLolIngamePakFiles,

	kLolCharacterDefs,
	kLolIngameSfxFiles,
	kLolIngameSfxIndex,
	kLolMusicTrackMap,
	kLolGMSfxIndex,
	kLolMT32SfxIndex,
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
	kLolMonsterLevelOffsets,
	kLolMonsterDirFlags,
	kLolMonsterScaleY,
	kLolMonsterScaleX,
	kLolMonsterScaleWH,
	kLolFlyingItemShp,
	kLolInventoryDesc,

	kLolLevelShpList,
	kLolLevelDatList,
	kLolCompassDefs,
	kLolStashSetup,
	kLolDscUnk1,
	kLolDscShapeIndex1,
	kLolDscShapeIndex2,
	kLolDscScaleWidthData,
	kLolDscScaleHeightData,
	kLolDscX,
	kLolDscY,
	kLolDscTileIndex,
	kLolDscUnk2,
	kLolDscDoorShapeIndex,
	kLolDscDimData1,
	kLolDscDimData2,
	kLolDscBlockMap,
	kLolDscDimMap,
	kLolDscShapeOvlIndex,
	kLolDscBlockIndex,
	kLolDscDoor1,
	kLolDscDoorScale,
	kLolDscDoor4,
	kLolDscDoorX,
	kLolDscDoorY,

	kLolScrollXTop,
	kLolScrollYTop,
	kLolScrollXBottom,
	kLolScrollYBottom,

	kLolButtonDefs,
	kLolButtonList1,
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

	-1
};

const int lolCDFile1Need[] = {
	kLolHistory,
	-1
};

const int lolCDFile2Need[] = {
	kLolCharacterDefs,
	kLolIngameSfxFiles,
	kLolIngameSfxIndex,
	kLolMusicTrackMap,
	kLolGMSfxIndex,
	kLolMT32SfxIndex,
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
	kLolMonsterLevelOffsets,
	kLolMonsterDirFlags,
	kLolMonsterScaleY,
	kLolMonsterScaleX,
	kLolMonsterScaleWH,
	kLolFlyingItemShp,
	kLolInventoryDesc,

	kLolLevelShpList,
	kLolLevelDatList,
	kLolCompassDefs,
	kLolItemPrices,
	kLolStashSetup,
	kLolDscUnk1,
	kLolDscShapeIndex1,
	kLolDscShapeIndex2,
	kLolDscScaleWidthData,
	kLolDscScaleHeightData,
	kLolDscX,
	kLolDscY,
	kLolDscTileIndex,
	kLolDscUnk2,
	kLolDscDoorShapeIndex,
	kLolDscDimData1,
	kLolDscDimData2,
	kLolDscBlockMap,
	kLolDscDimMap,
	kLolDscShapeOvlIndex,
	kLolDscBlockIndex,
	kLolDscDoor1,
	kLolDscDoorScale,
	kLolDscDoor4,
	kLolDscDoorX,
	kLolDscDoorY,

	kLolScrollXTop,
	kLolScrollYTop,
	kLolScrollXBottom,
	kLolScrollYBottom,

	kLolButtonDefs,
	kLolButtonList1,
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

	-1
};

const int lolDemoNeed[] = {
	k2SeqplayPakFiles,
	k2SeqplayStrings,
	k2SeqplaySeqData,
	k2SeqplaySfxFiles,
	kLolSeqplayIntroTracks,
	-1
};

struct GameNeed {
	int game;
	int platform;
	int special;

	const int *entries;
};

const GameNeed gameNeedTable[] = {
	{ kKyra1, kPlatformPC, -1, kyra1FloppyNeed },
	{ kKyra1, kPlatformAmiga, -1, kyra1AmigaNeed },

	{ kKyra1, kPlatformPC, kTalkieVersion, kyra1CDNeed },

	{ kKyra1, kPlatformFMTowns, kFMTownsVersionE , kyra1TownsNeed },
	{ kKyra1, kPlatformFMTowns, kFMTownsVersionJ, kyra1TownsNeed },

	{ kKyra1, kPlatformPC, kDemoVersion, kyra1DemoNeed },

	{ kKyra1, kPlatformPC, kDemoCDVersion, kyra1DemoCDNeed },

	{ kKyra2, kPlatformPC, k2FloppyFile1, kyra2FloppyFile1Need },
	{ kKyra2, kPlatformPC, k2FloppyFile2, kyra2FloppyFile2Need },

	{ kKyra2, kPlatformPC, k2CDFile1E, kyra2CDFile1Need },
	{ kKyra2, kPlatformPC, k2CDFile2E, kyra2CDFile2Need },

	{ kKyra2, kPlatformPC, k2CDFile1F, kyra2CDFile1Need },
	{ kKyra2, kPlatformPC, k2CDFile2F, kyra2CDFile2Need },

	{ kKyra2, kPlatformPC, k2CDFile1G, kyra2CDFile1Need },
	{ kKyra2, kPlatformPC, k2CDFile2G, kyra2CDFile2Need },

	{ kKyra2, kPlatformPC, k2CDFile1I, kyra2CDFile1Need }, // Italian fan translation

	{ kKyra2, kPlatformPC, k2CDDemoE, kyra2CDDemoNeed },
	{ kKyra2, kPlatformPC, k2CDDemoF, kyra2CDDemoNeed },
	{ kKyra2, kPlatformPC, k2CDDemoG, kyra2CDDemoNeed },

	{ kKyra2, kPlatformFMTowns, k2TownsFile1E , kyra2TownsFile1Need },
	{ kKyra2, kPlatformFMTowns, k2TownsFile2E , kyra2TownsFile2Need },

	{ kKyra2, kPlatformFMTowns, k2TownsFile1J, kyra2TownsFile1Need },
	{ kKyra2, kPlatformFMTowns, k2TownsFile2J, kyra2TownsFile2Need },

	{ kKyra2, kPlatformPC, k2DemoVersion, kyra2DemoNeed },

	{ kKyra2, kPlatformPC, k2DemoVersionTlkE, kyra2TlkDemoNeed },
	{ kKyra2, kPlatformPC, k2DemoVersionTlkF, kyra2TlkDemoNeed },
	{ kKyra2, kPlatformPC, k2DemoVersionTlkG, kyra2TlkDemoNeed },

	{ kLol, kPlatformPC, k2DemoLol, lolDemoNeed },

	{ kKyra3, kPlatformPC, -1, kyra3Need },

	{ kLol, kPlatformPC, -1, lolFloppyNeed },

	{ kLol, kPlatformPC, kLolCD1, lolCDFile1Need },
	{ kLol, kPlatformPC, kLolCD2, lolCDFile2Need },

	{ -1, -1, 0 }
};

} // end of anonymous namespace

const int *getNeedList(const Game *g) {
	for (const GameNeed *need = gameNeedTable; need->game != -1; ++need) {
		if (need->game == g->game && g->platform == need->platform && need->special == g->special)
			return need->entries;
	}

	return 0;
}

