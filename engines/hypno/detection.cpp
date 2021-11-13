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
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"sinistersix", // ES release
		nullptr,
		AD_ENTRY2s("SPIDER.EXE", "dbd912d6f6724c6d44775fc19cfa8ca0", 483871,
				"MISSIONS.LIB", "585704e26094cbaf14fbee90798e8d5d", 119945),
		Common::ES_ESP,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
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
		"wetlands", // Personal Computer World (UK) (May 1996) - Chapter 11 demo
		"Demo",
		AD_ENTRY2s("wetlands.exe", "15a6b1b3819ef002438df340509b5373", 553355,
				"missions.lib", "6ffa658f22a00b6e17d7f920fcc13578", 12469),
		Common::EN_GRB,
		Common::kPlatformDOS,
		ADGF_TESTING | ADGF_DEMO,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"wetlands", // PC Gamer Disc 12 (November 1995) - Chapter 31 demo
		"Demo",
		AD_ENTRY2s("wetlands.exe", "15a6b1b3819ef002438df340509b5373", 553355,
				"missions.lib", "34b922fac8f64546c0690aa83f09e98e", 40891),
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
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"wetlands", // Wetlands (FR)
		nullptr,
		AD_ENTRY2s("wetlands.exe", "edc5b0c0caf3d5b01d344cb555d9a085", 629575,
			    "missions.lib", "aeaaa8b26ab17e37f060334a311a3ff6", 309793),
		Common::FR_FRA,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"wetlands", // Wetlands (ES)
		nullptr,
		AD_ENTRY2s("wetlands.exe", "8d0f3630523da827bb25e665b7d3f879", 644055,
				"missions.lib", "aeaaa8b26ab17e37f060334a311a3ff6", 309793),
		Common::ES_ESP,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOMIDI)},
	{
		"soldierboyz", // Solidier Boyz (US)
		_s("Missing game code"),
		AD_ENTRY2s("boyz.exe", "bac1d734f2606dbdd0816dfa7a5cf518", 263347,
					"setup.exe", "bac1d734f2606dbdd0816dfa7a5cf518", 160740),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSUPPORTED,
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
	nullptr
};

class HypnoMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	HypnoMetaEngineDetection() : AdvancedMetaEngineDetection(Hypno::gameDescriptions, sizeof(ADGameDescription), Hypno::hypnoGames) {
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
				"Wetlands (C) Hypnotix, Inc.";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

REGISTER_PLUGIN_STATIC(HYPNO_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, HypnoMetaEngineDetection);

