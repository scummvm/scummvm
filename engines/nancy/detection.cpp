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

#include "common/config-manager.h"

#include "engines/nancy/detection.h"

const char *const directoryGlobs[] = {
	"game",
	"iff",
	"cifTree",
	"datafiles",
	"cdvideo", // used to test for unpacked variants
	"hdvideo", // used to test for unpacked variants
	nullptr
};

static const DebugChannelDef debugFlagList[] = {
	{ Nancy::kDebugEngine, "Engine", "Engine general debug" },
	{ Nancy::kDebugActionRecord, "ActionRecord", "Action Record debug" },
	{ Nancy::kDebugScene, "Scene", "Scene debug" },
	{ Nancy::kDebugSound, "Sound", "Sound debug" },
	{ Nancy::kDebugHypertext, "Hypertext", "Hypertext rendering debug" },
	DEBUG_CHANNEL_END
};

static const PlainGameDescriptor nancyGames[] = {
	// Games
	{ "vampirediaries", "The Vampire Diaries" },
	{ "nancy1",  "Nancy Drew: Secrets Can Kill" },
	{ "nancy2",  "Nancy Drew: Stay Tuned for Danger" },
	{ "nancy3",  "Nancy Drew: Message in a Haunted Mansion" },
	{ "nancy4",  "Nancy Drew: Treasure in the Royal Tower" },
	{ "nancy5",  "Nancy Drew: The Final Scene" },
	{ "nancy6",  "Nancy Drew: Secret of the Scarlet Hand" },
	{ "nancy7",  "Nancy Drew: Ghost Dogs of Moon Lake" },
	{ "nancy8",  "Nancy Drew: The Haunted Carousel" },
	{ "nancy9",  "Nancy Drew: Danger on Deception Island" },
	{ "nancy10", "Nancy Drew: The Secret of Shadow Ranch" },
	{ "nancy11", "Nancy Drew: Curse of Blackmoor Manor" },
	{ nullptr, nullptr }
};

#define NANCY2_GUIOPTIONS GUIO4(GAMEOPTION_PLAYER_SPEECH, GAMEOPTION_CHARACTER_SPEECH, GAMEOPTION_FIX_SOFTLOCKS, GAMEOPTION_NANCY2_TIMER)
#define NANCY5_GUIOPTIONS GUIO3(GAMEOPTION_PLAYER_SPEECH, GAMEOPTION_CHARACTER_SPEECH, GAMEOPTION_FIX_SOFTLOCKS)
#define NANCY6_7_GUIOPTIONS GUIO2(GAMEOPTION_AUTO_MOVE, GAMEOPTION_FIX_SOFTLOCKS)

static const Nancy::NancyGameDescription gameDescriptions[] = {

	{ // MD5 by fracturehill
		{
			"vampirediaries", nullptr,
			AD_ENTRY1s("boot.iff", "66d3b6fe9a90d35de7a28950870719ec", 20340),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DROPLANGUAGE | ADGF_DROPPLATFORM,
			GUIO3(GUIO_NOLANG, GAMEOPTION_PLAYER_SPEECH, GAMEOPTION_CHARACTER_SPEECH)
		},
		Nancy::kGameTypeVampire
	},
	{ // MD5 by waltervn
		{
			"nancy1", nullptr,
			AD_ENTRY1s("ciftree.dat", "9f89e0b53717515ae0eb82d14ffe0e88", 4317962),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO2(GAMEOPTION_PLAYER_SPEECH, GAMEOPTION_CHARACTER_SPEECH)
		},
		Nancy::kGameTypeNancy1
	},
	{ // MD5 by fracturehill
		{
			"nancy1", nullptr,
			AD_ENTRY1s("ciftree.dat", "e1cd21841ab1b83a0ea0755ce0254cbc", 4480956),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO2(GAMEOPTION_PLAYER_SPEECH, GAMEOPTION_CHARACTER_SPEECH)
		},
		Nancy::kGameTypeNancy1
	},
	{ // MD5 by fracturehill
		{
			"nancy1", nullptr,
			{
				{ "data1.hdr", 0, "39b33ad649d3e7261508d3c6907f237f", 139814 },
				{ "data1.cab", 0, "f900861c47b0cb88191f5c6189db6cb1", 1916153 },
				{ "data2.cab", 0, "9c652edb9846a721839cb7e1dcc94a3e", 462008320 },
				{ "data3.cab", 0, "3053edb46a2574e118c4d4347a25a949", 626903743 },
				{ "is:data1.cab:ciftree.dat", 0, "A:e1cd21841ab1b83a0ea0755ce0254cbc", 4480956 },
				AD_LISTEND
			},
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM | Nancy::GF_COMPRESSED,
			GUIO2(GAMEOPTION_PLAYER_SPEECH, GAMEOPTION_CHARACTER_SPEECH)
		},
		Nancy::kGameTypeNancy1
	},
	{ // MD5 by waltervn
		{
			"nancy2", nullptr,
			AD_ENTRY1s("ciftree.dat", "fa4293d728a1b31407961cd82e86a015", 7784516),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			NANCY2_GUIOPTIONS
		},
		Nancy::kGameTypeNancy2
	},
	{ // MD5 by fracturehill
		{
			"nancy2", nullptr,
			AD_ENTRY1s("ciftree.dat", "c19f4a1193b58939ab1a7e314e9a550e", 7756789),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			NANCY2_GUIOPTIONS
		},
		Nancy::kGameTypeNancy2
	},
	{ // MD5 by fracturehill
		{
			"nancy2", nullptr,
			{
				{ "data1.hdr", 0, "af916152b6cbf1810076517897585f62", 227999 },
				{ "data1.cab", 0, "8139908ecba23f6cf58711ebd59c5b8b", 3854339 },
				{ "data2.cab", 0, "99926a30ced5af845220a96d3b657498", 459982848 },
				{ "data3.cab", 0, "39d396865ab10f908d55eb2ec733cb45", 60604580 },
				{ "is:data1.cab:ciftree.dat", 0, "A:c19f4a1193b58939ab1a7e314e9a550e", 7756789 },
			},
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM | Nancy::GF_COMPRESSED,
			NANCY2_GUIOPTIONS
		},
		Nancy::kGameTypeNancy2
	},
	{ // MD5 by waltervn
		{
			"nancy3", nullptr,
			AD_ENTRY1s("ciftree.dat", "ee5f8832226567c3610556497c451b09", 16256355),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO2(GAMEOPTION_PLAYER_SPEECH, GAMEOPTION_CHARACTER_SPEECH)
		},
		Nancy::kGameTypeNancy3
	},
	{ // Steam version
		{
			"nancy3", nullptr,
			AD_ENTRY1s("ciftree.dat", "6b379f9d8edfb2d439062122e08f785c", 16161115),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO2(GAMEOPTION_PLAYER_SPEECH, GAMEOPTION_CHARACTER_SPEECH)
		},
		Nancy::kGameTypeNancy3
	},
	{ // MD5 by fracturehill
		{
			"nancy3", nullptr,
			{
				{ "data1.hdr", 0, "f05167f20c11972f17e6abf7db0034b3", 207514 },
				{ "data1.cab", 0, "cb5d147affbd1f2f70b436c166d35e5b", 1595638 },
				{ "data2.cab", 0, "1ea22dabe4a5cff022dac123577188e7", 137304516 },
				{ "is:data1.cab:ciftree.dat", 0, "A:6b379f9d8edfb2d439062122e08f785c", 16161115 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM | Nancy::GF_COMPRESSED,
			GUIO2(GAMEOPTION_PLAYER_SPEECH, GAMEOPTION_CHARACTER_SPEECH)
		},
		Nancy::kGameTypeNancy3
	},
	{ // MD5 by waltervn
		{
			"nancy3", nullptr,
			{
				{ "data1.hdr", 0, "44906f3d2242f73f16feb8eb6a5161cb", 207327 },
				{ "data1.cab", 0, "e258cc871e5de5ae004d03c4e31431c7", 1555916 },
				{ "data2.cab", 0, "364dfd25677026da505f1fa6edd5571f", 137373135 },
				{ "is:data1.cab:ciftree.dat", 0, "A:ee5f8832226567c3610556497c451b09", 16256355 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM | Nancy::GF_COMPRESSED,
			GUIO2(GAMEOPTION_PLAYER_SPEECH, GAMEOPTION_CHARACTER_SPEECH)
		},
		Nancy::kGameTypeNancy3
	},
	{ // MD5 by fracturehill
		{
			"nancy3", nullptr,
			AD_ENTRY1s("ciftree.dat", "6b379f9d8edfb2d439062122e08f785c", 16161148),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO2(GAMEOPTION_PLAYER_SPEECH, GAMEOPTION_CHARACTER_SPEECH)
		},
		Nancy::kGameTypeNancy3
	},
	{ // MD5 by fracturehill
		{
			"nancy3", nullptr,
			{
				{ "data1.hdr", 0, "9da72fec24e1ca4f8f6b563bbdab3276", 237686 },
				{ "data1.cab", 0, "a7a259e45ae643aed63fa958531cc318", 3473219 },
				{ "data2.cab", 0, "cb709fba73605814f9dda823b1cfaf85", 433625036 },
				{ "is:data1.cab:ciftree.dat", 0, "A:6b379f9d8edfb2d439062122e08f785c", 16161148 },
				AD_LISTEND
			},
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM | Nancy::GF_COMPRESSED,
			GUIO2(GAMEOPTION_PLAYER_SPEECH, GAMEOPTION_CHARACTER_SPEECH)
		},
		Nancy::kGameTypeNancy3
	},
	{ // MD5 by waltervn
		{
			"nancy4", nullptr,
			AD_ENTRY1s("ciftree.dat", "e9d45f7db453b0d8f37d202fc979537c", 8742289),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO2(GAMEOPTION_PLAYER_SPEECH, GAMEOPTION_CHARACTER_SPEECH)
		},
		Nancy::kGameTypeNancy4
	},
	{ // Steam version
		{
			"nancy4", nullptr,
			AD_ENTRY1s("ciftree.dat", "3ad55cd8f9a3b010a19de44ff4ce7edf", 8786300),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO2(GAMEOPTION_PLAYER_SPEECH, GAMEOPTION_CHARACTER_SPEECH)
		},
		Nancy::kGameTypeNancy4
	},
	{ // MD5 by fracturehill
		{
			"nancy4", nullptr,
			{
				{ "data1.hdr", 0, "1cfe79efba356d1b5545f9b0cca99f31", 256336 },
				{ "data1.cab", 0, "31ad655a6de1f16c8990b94f4f094cc2", 1598815 },
				{ "data2.cab", 0, "9e134ed2dd0ce262b1c93bc91ce67d95", 59466427 },
				{ "is:data1.cab:ciftree.dat", 0, "A:3ad55cd8f9a3b010a19de44ff4ce7edf", 8786300 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM | Nancy::GF_COMPRESSED,
			GUIO2(GAMEOPTION_PLAYER_SPEECH, GAMEOPTION_CHARACTER_SPEECH)
		},
		Nancy::kGameTypeNancy4
	},
	{ // MD5 by waltervn
		{
			"nancy4", nullptr,
			{
				{ "data1.hdr", 0, "fa4e7a1c411053557169a7731f287012", 263443 },
				{ "data1.cab", 0, "8f689f92fcca443d6a03faa5de7e2f1c", 1568756 },
				{ "data2.cab", 0, "5525aa428041f3f1421a6fb5d1b8dba1", 140518758 },
				{ "is:data1.cab:ciftree.dat", 0, "A:e9d45f7db453b0d8f37d202fc979537c", 8742289 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM | Nancy::GF_COMPRESSED,
			GUIO2(GAMEOPTION_PLAYER_SPEECH, GAMEOPTION_CHARACTER_SPEECH)
		},
		Nancy::kGameTypeNancy4
	},
	{ // MD5 by eriktorbjorn
		{
			"nancy4", nullptr,
			{
				{ "data1.hdr", 0, "9febd79adc61148088b464140a124172", 263445 },
				{ "data1.cab", 0, "af5e06e381473fdc3ff6f7cac5619e32", 1569396 },
				{ "data2.cab", 0, "ed0ca7ba3cc315f705e48fe604d53523", 140562769 },
				{ "is:data1.cab:ciftree.dat", 0, "A:3ad55cd8f9a3b010a19de44ff4ce7edf", 8786300 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM | Nancy::GF_COMPRESSED,
			GUIO2(GAMEOPTION_PLAYER_SPEECH, GAMEOPTION_CHARACTER_SPEECH)
		},
		Nancy::kGameTypeNancy4
	},
	{ // MD5 by fracturehill
		{
			"nancy4", nullptr,
			AD_ENTRY1s("ciftree.dat", "a1090497f5fefce17494804bd1d624e1", 9991442),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO2(GAMEOPTION_PLAYER_SPEECH, GAMEOPTION_CHARACTER_SPEECH)
		},
		Nancy::kGameTypeNancy4
	},
	{ // MD5 by fracturehill
		{
			"nancy4", nullptr,
			{
				{ "data1.hdr", 0, "694d11d7ebdf2c96cd395f4baf938c10", 256535 },
				{ "data1.cab", 0, "9c9c9c60c5344e877e033e54564c6e6e", 4892300 },
				{ "data2.cab", 0, "05ef8fac76a227f829479719f40522b5", 458917888 },
				{ "data3.cab", 0, "9b266564726664befe1770754150f5d8", 65795720 },
				{ "is:data1.cab:ciftree.dat", 0, "A:a1090497f5fefce17494804bd1d624e1", 9991442 },
				AD_LISTEND
			},
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM | Nancy::GF_COMPRESSED,
			GUIO2(GAMEOPTION_PLAYER_SPEECH, GAMEOPTION_CHARACTER_SPEECH)
		},
		Nancy::kGameTypeNancy4
	},
	{ // MD5 by waltervn
		{
			"nancy5", nullptr,
			AD_ENTRY1s("ciftree.dat", "21fa81f322595c3100d8d58d100852d5", 8187692),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			NANCY5_GUIOPTIONS
		},
		Nancy::kGameTypeNancy5
	},
	{ // MD5 by waltervn
		{
			"nancy5", nullptr,
			{
				{ "data1.hdr", 0, "261105fba2a1226eedb090c2ce79fd35", 284091 },
				{ "data1.cab", 0, "7d27bb947ef7305831f1faaf1512a598", 1446301 },
				{ "data2.cab", 0, "00719c86cab733c1094b27079ce030f3", 145857935 },
				{ "is:data1.cab:ciftree.dat", 0, "A:21fa81f322595c3100d8d58d100852d5", 8187692 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM | Nancy::GF_COMPRESSED,
			NANCY5_GUIOPTIONS
		},
		Nancy::kGameTypeNancy5
	},
	{ // MD5 by fracturehill
		{
			"nancy5", nullptr,
			{
				{ "data1.hdr", 0, "0db3fb5bc002eb875eebb872969a22ca", 278505 },
				{ "data1.cab", 0, "b5d2d218ded5683b5ca2eafcdc1ed76e", 1720654 },
				{ "data2.cab", 0, "d379a879fb23b3013f78537927ac6cfe", 548761463 },
				{ "is:data1.cab:ciftree.dat", 0, "A:21fa81f322595c3100d8d58d100852d5", 8187692 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM | Nancy::GF_COMPRESSED,
			NANCY5_GUIOPTIONS
		},
		Nancy::kGameTypeNancy5
	},
	/* Temporarily disabled; needs ciftree.dat
	{ // MD5 by clone2727
		{
			"nancy5", nullptr,
			{
				{ "data1.hdr", 0, "258e27792fa7cc7a7125fd74d89f8487", 284091 },
				{ "data1.cab", 0, "70433b30b6114031d54d0c991ad44577", 1446301 },
				{ "data2.cab", 0, "66f47e4f5e6d431f815aa5250eb044bc", 145857937 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM | Nancy::GF_COMPRESSED,
			NANCY5_GUIOPTIONS
		},
		Nancy::kGameTypeNancy5
	},*/
	{ // MD5 by fracturehill
		{
			"nancy5", nullptr,
			AD_ENTRY1s("ciftree.dat", "8645fad8c3fb8c0ee13e7a0a75902782", 9714463),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			NANCY5_GUIOPTIONS
		},
		Nancy::kGameTypeNancy5
	},
	{ // MD5 by fracturehill
		{
			"nancy5", nullptr,
			{
				{ "data1.hdr", 0, "229ab8e318a0fd0f0db366d854be2a20", 277512 },
				{ "data1.cab", 0, "156a26646f48e73c578373694cfd632d", 3466411 },
				{ "data2.cab", 0, "3db8fcd5414be1b704bef52d300a7fc1", 460324864 },
				{ "data3.cab", 0, "8e40909e6946cd45aa949e3db1b970ac", 127118355 },
				{ "is:data1.cab:ciftree.dat", 0, "A:8645fad8c3fb8c0ee13e7a0a75902782", 9714463 },
				AD_LISTEND
			},
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM | Nancy::GF_COMPRESSED,
			NANCY5_GUIOPTIONS
		},
		Nancy::kGameTypeNancy5
	},
	{ // MD5 by Strangerke
		{
			"nancy6", nullptr,
			{
				{ "ciftree.dat", 0, "a97b848651fdcf38f5cad7092d98e4a1", 28888006 },
				{ "amfid.avf", 0, "5f39a351a9fdd13fc24efbcb841cb059", 207695 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM,
			NANCY6_7_GUIOPTIONS
		},
		Nancy::kGameTypeNancy6
	},
	{ // MD5 by eriktorbjorn
		{
			"nancy6", nullptr,
			{
				{ "data1.hdr", 0, "6a45cdf632301ae1b55fd7474bec3650", 298551 },
				{ "data1.cab", 0, "399b83b188bfc05077d81cc1eadd456c", 4470086 },
				{ "data2.cab", 0, "30ca94b596f6b5971a22c111b24d6764", 206382511 },
				{ "is:data1.cab:ciftree.dat", 0, "A:a97b848651fdcf38f5cad7092d98e4a1", 28888006 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM | Nancy::GF_COMPRESSED,
			NANCY6_7_GUIOPTIONS
		},
		Nancy::kGameTypeNancy6
	},
	{ // MD5 by fracturehill
		{
			"nancy6", nullptr,
			{
				{ "data1.hdr", 0, "fe9ccf3ac298dfdba50c27971eb17758", 275738 },
				{ "data1.cab", 0, "44379edf93cc7132e69800558c84c1a5", 4470232 },
				{ "data2.cab", 0, "e0552258607ac7d8ed89890768b705ae", 404327022 },
				{ "ciftree.dat", 0, "a97b848651fdcf38f5cad7092d98e4a1", 28888006 }, // ciftree is outside of cabfiles
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM | Nancy::GF_COMPRESSED,
			NANCY6_7_GUIOPTIONS
		},
		Nancy::kGameTypeNancy6
	},
	{ // MD5 by fracturehill
		{
			"nancy6", nullptr,
			AD_ENTRY1s("ciftree.dat", "e1a4db8cbac3de50d8e808f06d673b36", 29643931),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM,
			NANCY6_7_GUIOPTIONS
		},
		Nancy::kGameTypeNancy6
	},
	{ // MD5 by fracturehill
		{
			"nancy6", nullptr,
			{
				{ "data1.hdr", 0, "4a95296bcc54c1376661496d17f2eeb1", 276630 },
				{ "data1.cab", 0, "67f1f05f7ddfae3f63fc2f208e7b6e5d", 6634156 },
				{ "data2.cab", 0, "299c8165f5f7dacfcb6e625ffeac47b4", 457154560 },
				{ "is:data1.cab:ciftree.dat", 0, "A:e1a4db8cbac3de50d8e808f06d673b36", 29643931 },
				AD_LISTEND
			},
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM | Nancy::GF_COMPRESSED,
			NANCY6_7_GUIOPTIONS
		},
		Nancy::kGameTypeNancy6
	},
	{ // MD5 by fracturehill
		{
			"nancy7", nullptr,
			AD_ENTRY1s("ciftree.dat", "e49e6f56a47c363e2651bd19a70ff557", 55835579),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM,
			NANCY6_7_GUIOPTIONS
		},
		Nancy::kGameTypeNancy7
	},
	{ // MD5 by eriktorbjorn
		{
			"nancy7", nullptr,
			{
				{ "data1.hdr", 0, "f58175e4647e635e96d73dde49deb871", 335485 },
				{ "data1.cab", 0, "6b48a626a8c6c12c9d7f68ee6c80212a", 4693602 },
				{ "data2.cab", 0, "0aec46d4a59ea0279228ab25bfb3fcd5", 144613827 },
				{ "is:data1.cab:ciftree.dat", 0, "A:e49e6f56a47c363e2651bd19a70ff557", 55835579 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM | Nancy::GF_COMPRESSED,
			NANCY6_7_GUIOPTIONS
		},
		Nancy::kGameTypeNancy7
	},
	{ // MD5 by fracturehill
		{
			"nancy7", nullptr,
			AD_ENTRY1s("ciftree.dat", "a2001796b82a88d36693d087b15526e1", 56580014),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM,
			NANCY6_7_GUIOPTIONS
		},
		Nancy::kGameTypeNancy7
	},
	{ // MD5 by fracturehill
		{
			"nancy7", nullptr,
			{
				{ "data1.hdr", 0, "32708e36d0250cfd031a82b7534aebb9", 327394 },
				{ "data1.cab", 0, "41681a27e35123a5edfcf45a58101295", 4805355 },
				{ "data2.cab", 0, "4e5644ac2e0d523f5aaf343d115889a7", 460975235 },
				{ "is:data1.cab:ciftree.dat", 0, "A:a2001796b82a88d36693d087b15526e1", 56580014 },
				AD_LISTEND
			},
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM | Nancy::GF_COMPRESSED,
			NANCY6_7_GUIOPTIONS
		},
		Nancy::kGameTypeNancy7
	},
	{ // MD5 by fracturehill
		{
			"nancy8", nullptr,
			AD_ENTRY1s("ciftree.dat", "d85192a942a207017ebf0a19207ac698", 19498032),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM,
			GUIO1(GAMEOPTION_AUTO_MOVE)
		},
		Nancy::kGameTypeNancy8
	},
	{ // MD5 by fracturehill
		{
			"nancy8", nullptr,
			{
				{ "data1.hdr", 0, "463f68bd7b09bafcfed40c7875b775cc", 318378 },
				{ "data1.cab", 0, "6dd3805a1e2f5dd4c81726acf77e3869", 5451563 },
				{ "data2.cab", 0, "14dde9855a10ed9ee6622aea53477ba6", 159239822 },
				{ "is:data1.cab:ciftree.dat", 0, "A:d85192a942a207017ebf0a19207ac698", 19498032 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM | Nancy::GF_COMPRESSED,
			GUIO1(GAMEOPTION_AUTO_MOVE)
		},
		Nancy::kGameTypeNancy8
	},
	{ // MD5 by fracturehill
		{
			"nancy8", nullptr,
			AD_ENTRY1s("ciftree.dat", "9ed40f0bcc1d9e3e38e127bfbc6797d7", 20358011),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM,
			GUIO1(GAMEOPTION_AUTO_MOVE)
		},
		Nancy::kGameTypeNancy8
	},
	{ // MD5 by fracturehill
		{
			"nancy8", nullptr,
			{
				{ "data1.cab", 0, "d9cc1694249e2084ccf5afa2a2398f80", 3297523 },
				{ "data1.hdr", 0, "e2440f9e2ce02bc1aace6a376c9a9939", 303244 },
				{ "data2.cab", 0, "448974e37758bd91f81f92f508cbffd5", 379253931 },
				{ "is:data1.cab:ciftree.dat", 0, "A:9ed40f0bcc1d9e3e38e127bfbc6797d7", 20358011 },
				AD_LISTEND
			},
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM | Nancy::GF_COMPRESSED,
			GUIO1(GAMEOPTION_AUTO_MOVE)
		},
		Nancy::kGameTypeNancy8
	},
	{ // MD5 by fracturehill
		{
			"nancy9", nullptr,
			{
				{ "ciftree.dat", 0, "3a756e09631f4a2c8f48bf316e77b5d5", 26610718 },
				{ "andyfid.avf", 0, "4e11e5237e6776e342df6262fc930142", 1171821 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM,
			GUIO1(GAMEOPTION_AUTO_MOVE)
		},
		Nancy::kGameTypeNancy9
	},
	{ // MD5 by fracturehill
		{
			"nancy9", nullptr,
			{
				{ "data1.hdr", 0, "6dcc79a737b5275d431087b36fb81c88", 403996 },
				{ "data1.cab", 0, "1b1c1067b46ead0771485948bcfd8320", 6874331 },
				{ "data2.cab", 0, "d0c4a054d38de8dc85e9fde9667fff31", 115877879 },
				{ "ciftree.dat", 0, "3a756e09631f4a2c8f48bf316e77b5d5", 26610718 }, // ciftree is outside of cabfiles
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM | Nancy::GF_COMPRESSED,
			GUIO1(GAMEOPTION_AUTO_MOVE)
		},
		Nancy::kGameTypeNancy9
	},
	{
		{
			"nancy10", nullptr,
			{
				{ "ciftree.dat", 0, "270133c5e89b8538caef08041598d049", 29413095 },
				{ "den_cal_back16.avf", 0, "ee6c779d1bc61a6483f9166aa20e4545", 324644 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM,
			GUIO1(GAMEOPTION_AUTO_MOVE)
		},
		Nancy::kGameTypeNancy10
	},
	{
		{
			"nancy10", nullptr,
			{
				{ "data1.hdr", 0, "fb73756ee951627e97f859ff27182e6f", 394467 },
				{ "data1.cab", 0, "d72b503642cb2533bcc6ce2b0ca463ac", 7510894 },
				{ "data2.cab", 0, "63b9d6400a241fc0777ec8ad3e3c4076", 150997862 },
				{ "ciftree.dat", 0, "270133c5e89b8538caef08041598d049", 29413095 }, // ciftree is outside of cabfiles
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM | Nancy::GF_COMPRESSED,
			GUIO1(GAMEOPTION_AUTO_MOVE)
		},
		Nancy::kGameTypeNancy10
	},
	{
		{
			"nancy11", nullptr,
			{
				{ "ciftree.dat", 0, "3c406d4f391b6536982c6081f2dd1f4e", 55960641 },
				{ "gre_oil_cinematic.bik", 0, "28a2b6f939f1c7795e47a99337d7343a", 21514180 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM,
			GUIO1(GAMEOPTION_AUTO_MOVE)
		},
		Nancy::kGameTypeNancy11
	},
	{
		{
			"nancy11", nullptr,
			{
				{ "data1.hdr", 0, "ac8c62e5db77edff6265d80238c0f2c5", 375104 },
				{ "data1.cab", 0, "17aed043f3f1247a97f32ce950ec1cab", 8055234 },
				{ "data2.cab", 0, "42756dd5c89779d03bfcf4636912ef73", 149200126 },
				{ "ciftree.dat", 0, "3c406d4f391b6536982c6081f2dd1f4e", 55960641 }, // ciftree is outside of cabfiles
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM | Nancy::GF_COMPRESSED,
			GUIO1(GAMEOPTION_AUTO_MOVE)
		},
		Nancy::kGameTypeNancy11
	},

	// Do not delete
	{ AD_TABLE_END_MARKER, Nancy::kGameTypeNone }
};

class NancyMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	NancyMetaEngineDetection() : AdvancedMetaEngineDetection(gameDescriptions, sizeof(Nancy::NancyGameDescription), nancyGames) {
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
		_guiOptions = GUIO2(GUIO_NOMIDI, GUIO_NOASPECT);
	}

	const char *getName() const override {
		return "nancy";
	}

	const char *getEngineName() const override {
		return "Nancy Drew";
	}

	const char *getOriginalCopyright() const override {
		return "Nancy Drew Engine copyright Her Interactive, 1995-2012";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

REGISTER_PLUGIN_STATIC(NANCY_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, NancyMetaEngineDetection);
