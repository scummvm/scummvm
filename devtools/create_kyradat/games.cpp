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

#include "create_kyradat.h"

// Game tables

namespace {

const Game kyra1Games[] = {
	// Demos
	{ kKyra1, { EN_ANY, -1, -1 }, kPlatformPC, kDemoVersion, { "7b7504c8560ffc914d34c44c71b3094c", 0 } },
	{ kKyra1, { EN_ANY, -1, -1 }, kPlatformPC, kTalkieDemoVersion, { "226fdba99cb11ef1047131d9a50e6292", 0 } },

	// Amiga
	{ kKyra1, { EN_ANY, -1, -1 }, kPlatformAmiga, kNoSpecial, { "b620564b6b7e0787b053ca9e35bd9f52", 0 } },
	{ kKyra1, { DE_DEU, -1, -1 }, kPlatformAmiga, kNoSpecial, { "ceddb4bd4df51698e3851e75106d117a", 0 } },

	// Floppy
	{ kKyra1, { EN_ANY, -1, -1 }, kPlatformPC, kNoSpecial, { "76a4fc84e173cadb6369785787e1546e", 0 } },
	{ kKyra1, { DE_DEU, -1, -1 }, kPlatformPC, kNoSpecial, { "9442d6f7db6a41f3dd4aa4de5d36e107", 0 } },
	{ kKyra1, { FR_FRA, -1, -1 }, kPlatformPC, kNoSpecial, { "aa9d6d78d8b199deaf48efeca6d19af2", 0 } },
	{ kKyra1, { IT_ITA, -1, -1 }, kPlatformPC, kNoSpecial, { "5d7550306b369a3492f9f3402702477c", 0 } },
	{ kKyra1, { ES_ESP, -1, -1 }, kPlatformPC, kNoSpecial, { "9ff130d2558bcd674d4074849d93c362", 0 } },

	// Talkie
	{ kKyra1, { EN_ANY, -1, -1 }, kPlatformPC, kTalkieVersion, { "1ebc18f3e7fbb72474a55cb0fa089ed4", 0 } },
	{ kKyra1, { DE_DEU, -1, -1 }, kPlatformPC, kTalkieVersion, { "c65d381184f98ac26d9efd2d45baef51", 0 } },
	{ kKyra1, { FR_FRA, -1, -1 }, kPlatformPC, kTalkieVersion, { "307c5d4a554d9068ac3d326e350ae4a6", 0 } },
	{ kKyra1, { IT_ITA, -1, -1 }, kPlatformPC, kTalkieVersion, { "d0f1752098236083d81b9497bd2b6989", 0 } }, // Italian fan translation

	// FM-TOWNS
	{ kKyra1, { EN_ANY, JA_JPN, -1 }, kPlatformFMTowns, kNoSpecial, { "5a3ad60ccd0f2e29463e0368cd14a60d", 0 } },

	// PC-98
	{ kKyra1, { JA_JPN, -1, -1 }, kPlatformPC98, kNoSpecial, { "b9c06ac5177f5bf1f1acc0eea3937f6d", 0 } },

	GAME_DUMMY_ENTRY
};

const Game kyra2Games[] = {
	// demos
	{ kKyra2, { EN_ANY, -1, -1 }, kPlatformPC, kDemoVersion, { "a620a37579dd44ab0403482285e3897f", 0 } },

	{ kKyra2, { EN_ANY, FR_FRA, DE_DEU }, kPlatformPC, kTalkieDemoVersion, { "85bbc1cc6c4cef6ad31fc6ee79518efb", "fa54d8abfe05f9186c05f7de7eaf1480" } },

	// floppy games
	{ kKyra2, { EN_ANY, -1, -1 }, kPlatformPC, kNoSpecial, { "9b0f5e57b5a2ed88b5b989cbb402b6c7", "7c3eadbe5122722cf2e5e1611e19dfb9" } },
	{ kKyra2, { FR_FRA, -1, -1 }, kPlatformPC, kNoSpecial, { "df31cc9e37e1cf68df2fdc75ddf2d87b", "fc2c6782778e6c6d5a553d1cb73c98ad" } },
	{ kKyra2, { DE_DEU, -1, -1 }, kPlatformPC, kNoSpecial, { "0ca4f9a1438264a4c63c3218e064ed3b", "0d9b0eb7b0ad889ec942d74d80dde1bf" } },
	{ kKyra2, { IT_ITA, -1, -1 }, kPlatformPC, kNoSpecial, { "178d3ab913f61bfba21d2fb196405e8c", "3a61ed6b7c00ddae383a0361799e2ba6" } },
	{ kKyra2, { RU_RUS, -1, -1 }, kPlatformPC, kNoSpecial, { "fd6a388c01de9a578e24e3bbeacd8012", "3a61ed6b7c00ddae383a0361799e2ba6" } },

	// talkie games
	{ kKyra2, { EN_ANY, FR_FRA, DE_DEU }, kPlatformPC, kTalkieVersion, { "85bbc1cc6c4cef6ad31fc6ee79518efb", "e20d0d2e500f01e399ec588247a7e213" } },
	{ kKyra2, { IT_ITA, FR_FRA, DE_DEU }, kPlatformPC, kTalkieVersion, { "130795aa8f2333250c895dae9028b9bb", "e20d0d2e500f01e399ec588247a7e213" } }, // Italian Fan Translation

	// FM-TOWNS games
	{ kKyra2, { EN_ANY, JA_JPN, -1 }, kPlatformFMTowns, kNoSpecial, { "74f50d79c919cc8e7196c24942ce43d7", "a9a7fd4f05d00090e9e8bda073e6d431" } },

	// PC-98
	{ kKyra2, { EN_ANY, JA_JPN, -1 }, kPlatformPC98, kNoSpecial, { "9bbf9a69be956db072b485929b416082", "f55fda3e60c4956ce6e72b24d2ae1a07" } },

	GAME_DUMMY_ENTRY
};

const Game kyra3Games[] = {
	// DOS CD (multi language version, with no language specific strings)
	{ kKyra3, { EN_ANY, FR_FRA, DE_DEU }, kPlatformPC, kTalkieVersion, { "bf68701eb591d0b72219f314c0d32688", 0 } },
	{ kKyra3, { EN_ANY, IT_ITA, DE_DEU }, kPlatformPC, kTalkieVersion, { "bf68701eb591d0b72219f314c0d32688", 0 } }, // Fan translation // TODO: Verify md5sum
	{ kKyra3, { ES_ESP, FR_FRA, DE_DEU }, kPlatformPC, kTalkieVersion, { "bf68701eb591d0b72219f314c0d32688", 0 } }, // Fan translation // TODO: Verify md5sum

	GAME_DUMMY_ENTRY
};

const Game lolGames[] = {
	// DOS demo
	{ kLol, { EN_ANY, -1, -1 }, kPlatformPC, kDemoVersion, { "30bb5af87d38adb47d3e6ce06b1cb042", 0 } },

	// DOS floppy (no language specifc strings except character presets)
	{ kLol, { EN_ANY, -1, -1 }, kPlatformPC, kNoSpecial, { "0cc764a204f7ba8cefe1a5f14c479619", 0 } },
	{ kLol, { RU_RUS, -1, -1 }, kPlatformPC, kNoSpecial, { "80a9f9bf243bc6ed36d98584fc6988c4", 0 } },
	{ kLol, { DE_DEU, -1, -1 }, kPlatformPC, kNoSpecial, { "6b843869772c1b779e1386be868c15dd", 0 } },

	// PC98 (no language specifc strings)
	{ kLol, { JA_JPN, -1, -1 }, kPlatformPC98, kNoSpecial, { "6d5bd4a2f5ce433365734ca6b7a8d984", "1b0a457c48ae6908da301b656fe0aab4" } },

	// DOS CD (multi language version, with no language specific strings)
	{ kLol, { EN_ANY, FR_FRA, DE_DEU }, kPlatformPC, kTalkieVersion, { "9d1778314de80598c0b0d032e2a1a1cf", "263998ec600afca1cc7b935c473df670" } },
	{ kLol, { IT_ITA, FR_FRA, DE_DEU }, kPlatformPC, kTalkieVersion, { "9d1778314de80598c0b0d032e2a1a1cf", "f2af366e00f79dbf832fa19701d71ed9" } }, // Italian fan translation
	{ kLol, { EN_ANY, FR_FRA, RU_RUS }, kPlatformPC, kTalkieVersion, { "9d1778314de80598c0b0d032e2a1a1cf", "5b33478718968676343803911dd5e3e4" } }, // Russian fan translation
	GAME_DUMMY_ENTRY
};

} // end of anonymous namespace

const Game * const gameDescs[] = {
	kyra1Games,
	kyra2Games,
	kyra3Games,
	lolGames,
	0
};

// Need tables

namespace {

const int kyra1FloppyNeed[] = {
	k1KallakWritingSeq,
	k1MalcolmTreeSeq,
	k1WestwoodLogoSeq,
	k1KyrandiaLogoSeq,
	k1KallakMalcolmSeq,
	k1ForestSeq,
	k1IntroCPSStrings,
	k1IntroCOLStrings,
	k1IntroWSAStrings,
	k1IntroStrings,
	k1RoomList,
	k1RoomFilenames,
	k1CharacterImageFilenames,
	k1DefaultShapes,
	k1ItemNames,
	k1TakenStrings,
	k1PlacedStrings,
	k1DroppedStrings,
	k1NoDropStrings,
	k1AmuleteAnimSeq,
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
	k1PutDownString,
	k1WaitAmuletString,
	k1BlackJewelString,
	k1HealingTipString,
	k1PoisonGoneString,
	k1Healing1Shapes,
	k1Healing2Shapes,
	k1ThePoisonStrings,
	k1FluteStrings,
	k1PoisonDeathShapes,
	k1FluteShapes,
	k1Winter1Shapes,
	k1Winter2Shapes,
	k1Winter3Shapes,
	k1DrinkShapes,
	k1WispShapes,
	k1MagicAnimShapes,
	k1BranStoneShapes,
	k1WispJewelStrings,
	k1MagicJewelStrings,
	k1FlaskFullString,
	k1FullFlaskString,
	k1OutroReunionSeq,
	k1OutroHomeString,
	k1VeryCleverString,
	k1GUIStrings,
	k1NewGameString,
	k1ConfigStrings,
	k1AudioTracks,
	k1AudioTracksIntro,
	-1
};

const int kyra1CDNeed[] = {
	k1KallakWritingSeq,
	k1MalcolmTreeSeq,
	k1WestwoodLogoSeq,
	k1KyrandiaLogoSeq,
	k1KallakMalcolmSeq,
	k1ForestSeq,
	k1IntroCPSStrings,
	k1IntroCOLStrings,
	k1IntroWSAStrings,
	k1IntroStrings,
	k1RoomList,
	k1RoomFilenames,
	k1CharacterImageFilenames,
	k1DefaultShapes,
	k1ItemNames,
	k1TakenStrings,
	k1PlacedStrings,
	k1DroppedStrings,
	k1NoDropStrings,
	k1AmuleteAnimSeq,
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
	k1PutDownString,
	k1WaitAmuletString,
	k1BlackJewelString,
	k1HealingTipString,
	k1PoisonGoneString,
	k1Healing1Shapes,
	k1Healing2Shapes,
	k1ThePoisonStrings,
	k1FluteStrings,
	k1PoisonDeathShapes,
	k1FluteShapes,
	k1Winter1Shapes,
	k1Winter2Shapes,
	k1Winter3Shapes,
	k1DrinkShapes,
	k1WispShapes,
	k1MagicAnimShapes,
	k1BranStoneShapes,
	k1WispJewelStrings,
	k1MagicJewelStrings,
	k1FlaskFullString,
	k1FullFlaskString,
	k1OutroReunionSeq,
	k1OutroHomeString,
	k1VeryCleverString,
	k1GUIStrings,
	k1NewGameString,
	k1ConfigStrings,
	k1AudioTracks,
	k1AudioTracksIntro,
	-1
};

const int kyra1DemoNeed[] = {
	k1WestwoodLogoSeq,
	k1KyrandiaLogoSeq,
	k1IntroCPSStrings,
	k1IntroCOLStrings,
	k1IntroWSAStrings,
	k1IntroStrings,
	k1Demo1Seq,
	k1Demo2Seq,
	k1Demo3Seq,
	k1Demo4Seq,
	k1AudioTracksIntro,
	-1
};

const int kyra1DemoCDNeed[] = {
	k1KallakWritingSeq,
	k1MalcolmTreeSeq,
	k1WestwoodLogoSeq,
	k1KyrandiaLogoSeq,
	k1KallakMalcolmSeq,
	k1ForestSeq,
	k1IntroCPSStrings,
	k1IntroCOLStrings,
	k1IntroWSAStrings,
	k1IntroStrings,
	k1AudioTracksIntro,
	-1
};

const int kyra1TownsNeed[] = {
	k1KallakWritingSeq,
	k1MalcolmTreeSeq,
	k1WestwoodLogoSeq,
	k1KyrandiaLogoSeq,
	k1KallakMalcolmSeq,
	k1ForestSeq,
	k1IntroCPSStrings,
	k1IntroCOLStrings,
	k1IntroWSAStrings,
	k1IntroStrings,
	k1RoomList,
	k1RoomFilenames,
	k1CharacterImageFilenames,
	k1DefaultShapes,
	k1ItemNames,
	k1TakenStrings,
	k1PlacedStrings,
	k1DroppedStrings,
	k1NoDropStrings,
	k1AmuleteAnimSeq,
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
	k1PutDownString,
	k1WaitAmuletString,
	k1BlackJewelString,
	k1HealingTipString,
	k1PoisonGoneString,
	k1Healing1Shapes,
	k1Healing2Shapes,
	k1ThePoisonStrings,
	k1FluteStrings,
	k1PoisonDeathShapes,
	k1FluteShapes,
	k1Winter1Shapes,
	k1Winter2Shapes,
	k1Winter3Shapes,
	k1DrinkShapes,
	k1WispShapes,
	k1MagicAnimShapes,
	k1BranStoneShapes,
	k1WispJewelStrings,
	k1MagicJewelStrings,
	k1FlaskFullString,
	k1FullFlaskString,
	k1OutroReunionSeq,
	k1OutroHomeString,
	k1VeryCleverString,
	k1GUIStrings,
	k1NewGameString,
	k1ConfigStrings,

	k1TownsMusicFadeTable,
	k1TownsMusicFadeTable,
	k1TownsSFXwdTable,
	k1TownsSFXbtTable,
	k1TownsCDATable,
	k1AudioTracks,
	k1CreditsStrings,
	-1
};

const int kyra1PC98Need[] = {
	k1KallakWritingSeq,
	k1MalcolmTreeSeq,
	k1WestwoodLogoSeq,
	k1KyrandiaLogoSeq,
	k1KallakMalcolmSeq,
	k1ForestSeq,
	k1IntroCPSStrings,
	k1IntroCOLStrings,
	k1IntroWSAStrings,
	k1IntroStrings,
	k1RoomList,
	k1RoomFilenames,
	k1CharacterImageFilenames,
	k1DefaultShapes,
	k1ItemNames,
	k1TakenStrings,
	k1PlacedStrings,
	k1DroppedStrings,
	k1NoDropStrings,
	k1AmuleteAnimSeq,
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
	k1PutDownString,
	k1WaitAmuletString,
	k1BlackJewelString,
	k1HealingTipString,
	k1PoisonGoneString,
	k1Healing1Shapes,
	k1Healing2Shapes,
	k1ThePoisonStrings,
	k1FluteStrings,
	k1PoisonDeathShapes,
	k1FluteShapes,
	k1Winter1Shapes,
	k1Winter2Shapes,
	k1Winter3Shapes,
	k1DrinkShapes,
	k1WispShapes,
	k1MagicAnimShapes,
	k1BranStoneShapes,
	k1WispJewelStrings,
	k1MagicJewelStrings,
	k1FlaskFullString,
	k1FullFlaskString,
	k1OutroReunionSeq,
	k1OutroHomeString,
	k1VeryCleverString,
	k1GUIStrings,
	k1NewGameString,
	k1ConfigStrings,

	k1PC98StoryStrings,
	k1PC98IntroSfx,
	k1CreditsStrings,
	-1
};

const int kyra1AmigaNeed[] = {
	k1KallakWritingSeq,
	k1MalcolmTreeSeq,
	k1WestwoodLogoSeq,
	k1KyrandiaLogoSeq,
	k1KallakMalcolmSeq,
	k1ForestSeq,
	k1IntroCPSStrings,
	k1IntroWSAStrings,
	k1IntroCOLStrings,
	k1IntroStrings,
	k1RoomList,
	k1RoomFilenames,
	k1CharacterImageFilenames,
	k1DefaultShapes,
	k1ItemNames,
	k1TakenStrings,
	k1PlacedStrings,
	k1DroppedStrings,
	k1NoDropStrings,
	k1AmuleteAnimSeq,
	k1PutDownString,
	k1WaitAmuletString,
	k1BlackJewelString,
	k1HealingTipString,
	k1PoisonGoneString,
	k1Healing1Shapes,
	k1Healing2Shapes,
	k1ThePoisonStrings,
	k1FluteStrings,
	k1PoisonDeathShapes,
	k1FluteShapes,
	k1Winter1Shapes,
	k1Winter2Shapes,
	k1Winter3Shapes,
	k1DrinkShapes,
	k1WispShapes,
	k1MagicAnimShapes,
	k1BranStoneShapes,
	k1WispJewelStrings,
	k1MagicJewelStrings,
	k1FlaskFullString,
	k1FullFlaskString,
	k1OutroReunionSeq,
	k1OutroHomeString,
	k1VeryCleverString,
	k1GUIStrings,
	k1NewGameString,
	k1ConfigStrings,
	k1CreditsStrings,
	k1AmigaIntroSFXTable,
	k1AmigaGameSFXTable,
	-1
};

const int kyra2CDNeed[] = {
	k2SeqplayPakFiles,
	k2SeqplayCredits,
	k2SeqplayCreditsSpecial,
	k2SeqplayStrings,
	k2SeqplaySfxFiles,
	k2SeqplayTlkFiles,
	k2SeqplaySeqData,
	k2SeqplayIntroTracks,
	k2SeqplayFinaleTracks,
	k2IngameSfxFiles,
	k2IngameSfxIndex,
	k2IngameTracks,
	k2IngameTalkObjIndex,
	k2IngameShapeAnimData,
	-1
};

const int kyra2CDDemoNeed[] = {
	k2SeqplayPakFiles,
	k2SeqplayCredits,
	k2SeqplayCreditsSpecial,
	k2SeqplayStrings,
	k2SeqplaySfxFiles,
	k2SeqplayTlkFiles,
	k2SeqplaySeqData,
	k2SeqplayIntroTracks,
	k2SeqplayFinaleTracks,
	k2IngameSfxFiles,
	k2IngameSfxIndex,
	k2IngameTracks,
	k2IngameTalkObjIndex,
	k2IngameShapeAnimData,
	k2IngameTlkDemoStrings,
	-1
};

const int kyra2FloppyNeed[] = {
	k2SeqplayPakFiles,
	k2SeqplayStrings,
	k2SeqplaySfxFiles,
	k2SeqplayIntroTracks,
	k2SeqplayFinaleTracks,
	k2SeqplaySeqData,
	k2IngamePakFiles,
	k2IngameSfxFiles,
	k2IngameSfxIndex,
	k2IngameTracks,
	k2IngameTalkObjIndex,
	k2IngameShapeAnimData,
	-1
};

const int kyra2TownsNeed[] = {
	k2SeqplayPakFiles,
	k2SeqplayStrings,
	k2SeqplaySfxFiles,
	k2SeqplaySeqData,
	k2SeqplayIntroCDA,
	k2SeqplayFinaleCDA,
	k2IngamePakFiles,
	k2IngameSfxFiles,
	k2IngameSfxIndex,
	k2IngameCDA,
	k2IngameTalkObjIndex,
	k2IngameTimJpStrings,
	k2IngameShapeAnimData,
	-1
};

const int kyra2PC98Need[] = {
	k2SeqplayPakFiles,
	k2SeqplayStrings,
	k2SeqplaySfxFiles,
	k2SeqplaySeqData,
	k2IngamePakFiles,
	k2IngameSfxFiles,
	k2IngameSfxIndex,
	k2IngameTalkObjIndex,
	k2IngameTimJpStrings,
	k2IngameShapeAnimData,
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
	kLolStashSetup,
	kLolDscUnk1,
	kLolDscShapeIndex,
	kLolDscOvlMap,
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
	kLolDscOvlIndex,
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

const int lolPC98Need[] = {
	kLolIngamePakFiles,

	kLolCharacterDefs,
	kLolIngameSfxFiles,
	kLolIngameSfxIndex,
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
	kLolStashSetup,
	kLolDscUnk1,
	kLolDscShapeIndex,
	kLolDscOvlMap,
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
	kLolDscOvlIndex,
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
	kLolMapStringId,

	kLolSpellbookAnim,
	kLolSpellbookCoords,
	kLolHealShapeFrames,
	kLolLightningDefs,
	kLolFireballCoords,

	kLolCredits,

	-1
};

const int lolCDNeed[] = {
	kLolHistory,
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
	kLolDscUnk1,
	kLolDscShapeIndex,
	kLolDscOvlMap,
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
	kLolDscOvlIndex,
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
	k2SeqplayIntroTracks,
	-1
};

struct GameNeed {
	int game;
	int platform;
	int special;

	const int *entries;
};

const GameNeed gameNeedTable[] = {
	{ kKyra1, kPlatformPC, kNoSpecial, kyra1FloppyNeed },
	{ kKyra1, kPlatformAmiga, kNoSpecial, kyra1AmigaNeed },

	{ kKyra1, kPlatformPC, kTalkieVersion, kyra1CDNeed },

	{ kKyra1, kPlatformFMTowns, kNoSpecial, kyra1TownsNeed },

	{ kKyra1, kPlatformPC98, kNoSpecial, kyra1PC98Need },

	{ kKyra1, kPlatformPC, kDemoVersion, kyra1DemoNeed },

	{ kKyra1, kPlatformPC, kTalkieDemoVersion, kyra1DemoCDNeed },

	{ kKyra2, kPlatformPC, kNoSpecial, kyra2FloppyNeed },

	{ kKyra2, kPlatformPC, kTalkieVersion, kyra2CDNeed },

	{ kKyra2, kPlatformPC, kTalkieDemoVersion, kyra2CDDemoNeed },

	{ kKyra2, kPlatformFMTowns, kNoSpecial, kyra2TownsNeed },

	{ kKyra2, kPlatformPC98, kNoSpecial, kyra2PC98Need },

	{ kKyra2, kPlatformPC, kDemoVersion, kyra2DemoNeed },

	{ kLol, kPlatformPC, kDemoVersion, lolDemoNeed },

	{ kKyra3, kPlatformPC, kTalkieVersion, kyra3Need },

	{ kLol, kPlatformPC, kNoSpecial, lolFloppyNeed },
	{ kLol, kPlatformPC98, kNoSpecial, lolPC98Need },

	{ kLol, kPlatformPC, kTalkieVersion, lolCDNeed },

	{ -1, -1, -1, 0 }
};

} // end of anonymous namespace

const int *getNeedList(const Game *g) {
	for (const GameNeed *need = gameNeedTable; need->game != -1; ++need) {
		if (need->game == g->game && g->platform == need->platform && need->special == g->special)
			return need->entries;
	}

	return 0;
}

