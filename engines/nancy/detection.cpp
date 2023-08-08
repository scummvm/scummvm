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
	nullptr
};

static const DebugChannelDef debugFlagList[] = {
	{ Nancy::kDebugEngine, "Engine", "Engine general debug" },
	{ Nancy::kDebugActionRecord, "ActionRecord", "Action Record debug" },
	{ Nancy::kDebugScene, "Scene", "Scene debug" },
	{ Nancy::kDebugSound, "Sound", "Sound debug" },
	DEBUG_CHANNEL_END
};

static const PlainGameDescriptor nancyGames[] = {
	// Games
	{ "vampirediaries", "The Vampire Diaries" },
	{ "nancy1", "Nancy Drew: Secrets Can Kill" },
	{ "nancy2", "Nancy Drew: Stay Tuned for Danger" },
	{ "nancy3", "Nancy Drew: Message in a Haunted Mansion" },
	{ "nancy4", "Nancy Drew: Treasure in the Royal Tower" },
	{ "nancy5", "Nancy Drew: The Final Scene" },
	{ "nancy6", "Nancy Drew: Secret of the Scarlet Hand" },
	{ nullptr, nullptr }
};

static const Nancy::NancyGameDescription gameDescriptions[] = {

	{ // MD5 by fracturehill
		{
			"vampirediaries", nullptr,
			AD_ENTRY1s("boot.iff", "66d3b6fe9a90d35de7a28950870719ec", 20340),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_TESTING | ADGF_DROPLANGUAGE | ADGF_DROPPLATFORM,
			GUIO1(GUIO_NOLANG)
		},
		Nancy::kGameTypeVampire
	},
	{ // MD5 by waltervn
		{
			"nancy1", nullptr,
			AD_ENTRY1s("ciftree.dat", "9f89e0b53717515ae0eb82d14ffe0e88", 4317962),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_TESTING | ADGF_DROPPLATFORM,
			GUIO0()
		},
		Nancy::kGameTypeNancy1
	},
	{ // MD5 by fracturehill
		{
			"nancy1", nullptr,
			AD_ENTRY1s("ciftree.dat", "e1cd21841ab1b83a0ea0755ce0254cbc", 4480956),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_TESTING | ADGF_DROPPLATFORM,
			GUIO0()
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
				AD_LISTEND
			},
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_TESTING | ADGF_DROPPLATFORM | Nancy::GF_COMPRESSED,
			GUIO0()
		},
		Nancy::kGameTypeNancy1
	},
	{ // MD5 by waltervn
		{
			"nancy2", nullptr,
			AD_ENTRY1s("ciftree.dat", "fa4293d728a1b31407961cd82e86a015", 7784516),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO0()
		},
		Nancy::kGameTypeNancy2
	},
	{ // MD5 by fracturehill
		{
			"nancy2", nullptr,
			AD_ENTRY1s("ciftree.dat", "c19f4a1193b58939ab1a7e314e9a550e", 7756789),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM,
			GUIO0()
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
			},
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM | Nancy::GF_COMPRESSED,
			GUIO0()
		},
		Nancy::kGameTypeNancy2
	},
	{ // MD5 by waltervn
		{
			"nancy3", nullptr,
			AD_ENTRY1s("ciftree.dat", "ee5f8832226567c3610556497c451b09", 16256355),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM,
			GUIO0()
		},
		Nancy::kGameTypeNancy3
	},
	{ // Steam version
		{
			"nancy3", nullptr,
			AD_ENTRY1s("ciftree.dat", "6b379f9d8edfb2d439062122e08f785c", 16161115),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM,
			GUIO0()
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
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM | Nancy::GF_COMPRESSED,
			GUIO0()
		},
		Nancy::kGameTypeNancy3
	},
	{ // MD5 by fracturehill
		{
			"nancy3", nullptr,
			AD_ENTRY1s("ciftree.dat", "6b379f9d8edfb2d439062122e08f785c", 16161148),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM,
			GUIO0()
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
				AD_LISTEND
			},
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM | Nancy::GF_COMPRESSED,
			GUIO0()
		},
		Nancy::kGameTypeNancy3
	},
	{ // MD5 by waltervn
		{
			"nancy4", nullptr,
			AD_ENTRY1s("ciftree.dat", "e9d45f7db453b0d8f37d202fc979537c", 8742289),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM,
			GUIO0()
		},
		Nancy::kGameTypeNancy4
	},
	{ // MD5 by fracturehill
		{
			"nancy4", nullptr,
			AD_ENTRY1s("ciftree.dat", "8645fad8c3fb8c0ee13e7a0a75902782", 9714463),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM,
			GUIO0()
		},
		Nancy::kGameTypeNancy4
	},
	{ // Steam version
		{
			"nancy4", nullptr,
			AD_ENTRY1s("ciftree.dat", "3ad55cd8f9a3b010a19de44ff4ce7edf", 8786300),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM,
			GUIO0()
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
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM | Nancy::GF_COMPRESSED,
			GUIO0()
		},
		Nancy::kGameTypeNancy4
	},
	{ // MD5 by fracturehill
		{
			"nancy4", nullptr,
			{
  				{ "data1.hdr", 0, "229ab8e318a0fd0f0db366d854be2a20", 277512 },
				{ "data1.cab", 0, "156a26646f48e73c578373694cfd632d", 3466411 },
  				{ "data2.cab", 0, "3db8fcd5414be1b704bef52d300a7fc1", 460324864 },
  				{ "data3.cab", 0, "8e40909e6946cd45aa949e3db1b970ac", 127118355 },
				AD_LISTEND
			},
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM | Nancy::GF_COMPRESSED,
			GUIO0()
		},
		Nancy::kGameTypeNancy4
	},
	{ // MD5 by waltervn
		{
			"nancy5", nullptr,
			AD_ENTRY1s("ciftree.dat", "21fa81f322595c3100d8d58d100852d5", 8187692),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM,
			GUIO0()
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
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM | Nancy::GF_COMPRESSED,
			GUIO0()
		},
		Nancy::kGameTypeNancy5
	},
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
			ADGF_UNSTABLE | ADGF_DROPPLATFORM | Nancy::GF_COMPRESSED,
			GUIO0()
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
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM | Nancy::GF_COMPRESSED,
			GUIO0()
		},
		Nancy::kGameTypeNancy5
	},
	{ // MD5 by Strangerke
		{
			"nancy6", nullptr,
			AD_ENTRY1s("ciftree.dat", "a97b848651fdcf38f5cad7092d98e4a1", 28888006),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM,
			GUIO0()
		},
		Nancy::kGameTypeNancy6
	},
	{ AD_TABLE_END_MARKER, Nancy::kGameTypeNone }
};

class NancyMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	NancyMetaEngineDetection() : AdvancedMetaEngineDetection(gameDescriptions, sizeof(Nancy::NancyGameDescription), nancyGames) {
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
		_guiOptions = GUIO4(GUIO_NOMIDI, GUIO_NOASPECT, GUIO_GAMEOPTIONS1, GUIO_GAMEOPTIONS2);
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
