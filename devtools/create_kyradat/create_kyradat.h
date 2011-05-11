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
	kLolDscDoor1,
	kLolDscDoorScale,
	kLolDscDoor4,
	kLolDscDoorX,
	kLolDscDoorY,
	kLolDscOvlIndex,
	kLolDscBlockIndex,

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
	kTalkieDemoVersion
};

enum kGame {
	kKyra1 = 0,
	kKyra2,
	kKyra3,
	kLol
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
