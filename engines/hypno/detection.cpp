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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "base/plugins.h"
#include "common/translation.h"
#include "engines/advancedDetector.h"
#include "hypno/hypno.h"

static const DebugChannelDef debugFlagList[] = {
	{Hypno::kHypnoDebugMedia, "media", "Media debug channel"},
	{Hypno::kHypnoDebugParser, "parser", "Parser debug channel"},
	{Hypno::kHypnoDebugScene, "scene", "Scene debug channel"},
	{Hypno::kHypnoDebugArcade, "arcade", "Arcade debug channel"},
	DEBUG_CHANNEL_END};

namespace Hypno {
static const PlainGameDescriptor hypnoGames[] = {
	{"sinistersix", "Marvel Comics Spider-Man: The Sinister Six"},
	{"wetlands", "Wetlands"},
	{"soldierboyz", "Soldier Boyz"},
	{"teacher", "Bruce Coville's My Teacher Is an Alien"},
	{nullptr, nullptr}};

static const ADGameDescription gameDescriptions[] = {
	{
		"sinistersix", // Demo from the US release
		"Demo",
		AD_ENTRY2s("DATA.Z",  "2a9c7cf8920ec794482f0a5873102da5", 1285960,
				"DCINE1.SMK", "1ff3db09d148e8dd8b56d2e87e7296b8", 493752),
		Common::EN_USA,
		Common::kPlatformDOS,
		ADGF_TESTING | ADGF_DEMO,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"sinistersix", // US release
		nullptr,
		AD_ENTRY2s("DATA.Z", "a1f71005a45e6ee454bb0bf3868dff54", 8766307,
				"MISSIONS.LIB", "585704e26094cbaf14fbee90798e8d5d", 119945),
		Common::EN_USA,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"sinistersix", // ES release
		nullptr,
		AD_ENTRY2s("SPIDER.EXE", "dbd912d6f6724c6d44775fc19cfa8ca0", 483871,
				"MISSIONS.LIB", "585704e26094cbaf14fbee90798e8d5d", 119945),
		Common::ES_ESP,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"sinistersix", // DE release
		nullptr,
		AD_ENTRY2s("Setup1.Sax", "86b6ae45f45a8273ef3116be6bac01f5", 9591164,
				"MISSIONS.LIB", "585704e26094cbaf14fbee90798e8d5d", 119945),
		Common::DE_DEU,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"sinistersix", // IT release
		nullptr,
		AD_ENTRY2s("DATA.Z", "8e1aa1ab39e38c4f1bf67c0b330b3991", 8740866,
				"MISSIONS.LIB", "585704e26094cbaf14fbee90798e8d5d", 119945),
		Common::IT_ITA,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"sinistersix", // HE release
		nullptr,
		AD_ENTRY2s("SPIDER.EXE", "dbd912d6f6724c6d44775fc19cfa8ca0", 483359,
				"MISSIONS.LIB", "585704e26094cbaf14fbee90798e8d5d", 119945),
		Common::HE_ISR,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"wetlands", // Wetlands Demo Disc (November 1995)
		"Demo",
		AD_ENTRY3s("wetlands.exe", "15a6b1b3819ef002438df340509b5373", 642231,
				"wetdemo.exe", "15a6b1b3819ef002438df340509b5373", 458319,
				"demo.exe", "15a6b1b3819ef002438df340509b5373", 533221),
		Common::EN_USA,
		Common::kPlatformDOS,
		ADGF_TESTING | ADGF_DEMO,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"wetlands", // Wetlands Demo from a Hebrew magazine
		"DemoHebrew",
		AD_ENTRY3s("wetlands.exe", "15a6b1b3819ef002438df340509b5373", 629503,
				"wetdemo.exe", "15a6b1b3819ef002438df340509b5373", 458319,
				"demo.exe", "15a6b1b3819ef002438df340509b5373", 533221),
		Common::EN_USA,
		Common::kPlatformDOS,
		ADGF_TESTING | ADGF_DEMO,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"wetlands", // Personal Computer World (UK) (May 1996) - Chapter 11 demo
		"PCWDemo",
		AD_ENTRY2s("wetlands.exe", "15a6b1b3819ef002438df340509b5373", 553355,
				"missions.lib", "6ffa658f22a00b6e17d7f920fcc13578", 12469),
		Common::EN_USA,
		Common::kPlatformDOS,
		ADGF_TESTING | ADGF_DEMO,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"wetlands", // PC Gamer Disc 12 (November 1995) - Chapter 31 demo
		"PCGDemo",
		AD_ENTRY2s("wetlands.exe", "15a6b1b3819ef002438df340509b5373", 553355,
				"missions.lib", "34b922fac8f64546c0690aa83f09e98e", 40891),
		Common::EN_USA,
		Common::kPlatformDOS,
		ADGF_TESTING | ADGF_DEMO,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"wetlands", // Génération 4 (FR) - Number 81 (October 1995) - Chapters 31/52 demo
		"Gen4",
		AD_ENTRY2s("wetlands.exe", "15a6b1b3819ef002438df340509b5373", 629503,
				"missions.lib", "34b922fac8f64546c0690aa83f09e98e", 40891),
		Common::EN_USA,
		Common::kPlatformDOS,
		ADGF_TESTING | ADGF_DEMO,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"wetlands", // Non Interactive: PC Review 49 (November 1995)
		"NonInteractive",
		AD_ENTRY2s("playsmks.exe", "edc5b0c0caf3d5b01d344cb555d9a085", 422607,
				"wetmusic.81m", "0d99c63ce19633d09569b1fdcdff1505", 2833439),
		Common::EN_USA,
		Common::kPlatformDOS,
		ADGF_TESTING | ADGF_DEMO,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"wetlands", // Non Interactive: Joystick HS 7 (September 1995)
		"NonInteractiveJoystick",
		AD_ENTRY2s("playsmks.exe", "edc5b0c0caf3d5b01d344cb555d9a085", 422607,
				"c44_22k.raw", "4b2279af59ce3049cc5177b0047e8447", 5247618),
		Common::EN_USA,
		Common::kPlatformDOS,
		ADGF_TESTING | ADGF_DEMO,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"wetlands", // Wetlands (US)
		nullptr,
		AD_ENTRY2s("wetlands.exe", "15a6b1b3819ef002438df340509b5373", 647447,
				"missions.lib", "aeaaa8b26ab17e37f060334a311a3ff6", 309793),
		Common::EN_USA,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"wetlands", // Wetlands (FR)
		nullptr,
		AD_ENTRY2s("wetlands.exe", "edc5b0c0caf3d5b01d344cb555d9a085", 629575,
			    "missions.lib", "aeaaa8b26ab17e37f060334a311a3ff6", 309793),
		Common::FR_FRA,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"wetlands", // Wetlands (ES)
		nullptr,
		AD_ENTRY2s("wetlands.exe", "8d0f3630523da827bb25e665b7d3f879", 644055,
				"missions.lib", "aeaaa8b26ab17e37f060334a311a3ff6", 309793),
		Common::ES_ESP,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"wetlands", // Wetlands (KO)
		nullptr,
		AD_ENTRY2s("wetlands.exe", "edc5b0c0caf3d5b01d344cb555d9a085", 360151,
				"missions.lib", "aeaaa8b26ab17e37f060334a311a3ff6", 309793),
		Common::KO_KOR,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"soldierboyz", // Solidier Boyz (US)
		nullptr,
		AD_ENTRY2s("boyz.exe", "bac1d734f2606dbdd0816dfa7a5cf518", 263347,
					"setup.exe", "bac1d734f2606dbdd0816dfa7a5cf518", 160740),
		Common::EN_USA,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"teacher", // Bruce Coville's My Teacher Is an Alien Demo - PC Collector 10 (July 1997)
		"Demo",
		AD_ENTRY2s("teacher.exe", "7650ab104a21e2ca33a1d0d54a51e9d1", 258560,
				"demomenu.smk", "abb06755ff1d345b11b0f2c2d42e5dc7", 2424),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DEMO,
		GUIO1(GUIO_NOMIDI)
	},
	AD_TABLE_END_MARKER
};
} // End of namespace Hypno

static const char *const directoryGlobs[] = {
	"boyz",
	"spider",
	"wetlands",
	"sixdemo",
	"demo",
	"factory",
	"movie",
	"c_misc",
	"data",
	"demo",
	nullptr
};

static const ExtraGuiOption hypnoExtraGuiOptionOriginalCheats = {
	_s("Enable original cheats"),
	_s("Allow cheats using the C key."),
	"cheats",
	true,
	0,
	0
};

static const ExtraGuiOption hypnoExtraGuiOptionInfiniteHealthCheat = {
	_s("Enable infinite health cheat"),
	_s("Player health will never decrease (except for game over scenes)."),
	"infiniteHealth",
	false,
	0,
	0
};

static const ExtraGuiOption hypnoExtraGuiOptionInfiniteAmmoCheat = {
	_s("Enable infinite ammo cheat"),
	_s("Player ammo will never decrease."),
	"infiniteAmmo",
	false,
	0,
	0
};

static const ExtraGuiOption hypnoExtraGuiOptionUnlockAllLevels = {
	_s("Unlock all levels"),
	_s("All levels will be available to play."),
	"unlockAllLevels",
	false,
	0,
	0
};

static const ExtraGuiOption hypnoExtraGuiOptionRestoredContent = {
	_s("Enable restored content"),
	_s("Add additional content that is not enabled the original implementation."),
	"restored",
	true,
	0,
	0
};

class HypnoMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	HypnoMetaEngineDetection() : AdvancedMetaEngineDetection(Hypno::gameDescriptions, sizeof(ADGameDescription), Hypno::hypnoGames) {
		_guiOptions = GUIO1(GUIO_NOMIDI);
		_maxScanDepth = 3;
		_directoryGlobs = directoryGlobs;
	}

	const char *getEngineId() const override {
		return "hypno";
	}

	const char *getName() const override {
		return "Hypno";
	}

	const char *getOriginalCopyright() const override {
		return	"Marvel Comics Spider-Man: The Sinister Six (C) Brooklyn Multimedia\n"
				"Wetlands (C) Hypnotix, Inc.\n"
				"Soldier Boyz (C) Hypnotix, Inc., Motion Picture Corporation of America Interactive";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}

	const ExtraGuiOptions getExtraGuiOptions(const Common::String &target) const override;
};

const ExtraGuiOptions HypnoMetaEngineDetection::getExtraGuiOptions(const Common::String &target) const {
	ExtraGuiOptions options;
	options.push_back(hypnoExtraGuiOptionOriginalCheats);
	options.push_back(hypnoExtraGuiOptionInfiniteHealthCheat);
	options.push_back(hypnoExtraGuiOptionInfiniteAmmoCheat);
	options.push_back(hypnoExtraGuiOptionRestoredContent);
	options.push_back(hypnoExtraGuiOptionUnlockAllLevels);
	return options;
}

REGISTER_PLUGIN_STATIC(HYPNO_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, HypnoMetaEngineDetection);

