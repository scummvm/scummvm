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
	{"sinister-six", "Marvel Comics Spider-Man: The Sinister Six"},
	{"wetlands", "Wetlands"},
	{"soldier-boyz", "Soldier Boyz"},
	{0, 0}};

static const ADGameDescription gameDescriptions[] = {
	{"sinister-six", // Demo from the US release
	 "Demo",
	 AD_ENTRY2s("DATA.Z",  "2a9c7cf8920ec794482f0a5873102da5", 1285960,
				"DCINE1.SMK", "1ff3db09d148e8dd8b56d2e87e7296b8", 493752),
	 Common::EN_USA,
	 Common::kPlatformDOS,
	 ADGF_TESTING | ADGF_DEMO,
	 GUIO1(GUIO_NOMIDI)},
	{"sinister-six", // US release
	 "Not yet implemented",
	 AD_ENTRY2s("COMBAT.EXE", "bac1d734f2606dbdd0816dfa7a5cf518", 255115,
				"SPIDER.EXE", "bac1d734f2606dbdd0816dfa7a5cf518", 248056),
	 Common::EN_USA,
	 Common::kPlatformDOS,
	 ADGF_UNSUPPORTED | ADGF_TESTING,
	 GUIO1(GUIO_NOMIDI)},
	{"wetlands", // Wetlands Demo Disc (November 1995)
	 "Demo",
	 AD_ENTRY2s("wetlands.exe", "15a6b1b3819ef002438df340509b5373", 642231,
				"wetdemo.exe", "15a6b1b3819ef002438df340509b5373", 458319),
	 Common::EN_USA,
	 Common::kPlatformDOS,
	 ADGF_TESTING | ADGF_DEMO,
	 GUIO1(GUIO_NOMIDI)},
    {"wetlands", // Personal Computer World (UK) (May 1996) - Chapter 11 demo
	 "Demo",
	 AD_ENTRY2s("wetlands.exe", "15a6b1b3819ef002438df340509b5373", 553355,
				"missions.lib", "6ffa658f22a00b6e17d7f920fcc13578", 12469),
	 Common::EN_GRB,
	 Common::kPlatformDOS,
	 ADGF_TESTING | ADGF_DEMO,
	 GUIO1(GUIO_NOMIDI)},
     {"wetlands", // PC Gamer Disc 12 (November 1995) - Chapter 31 demo
	 "Demo",
	 AD_ENTRY2s("wetlands.exe", "15a6b1b3819ef002438df340509b5373", 553355,
				"missions.lib", "34b922fac8f64546c0690aa83f09e98e", 40891),
	 Common::EN_USA,
	 Common::kPlatformDOS,
	 ADGF_TESTING | ADGF_DEMO,
	 GUIO1(GUIO_NOMIDI)},

	{"wetlands", // Wetlands (US)
	 "",
	 AD_ENTRY2s("wetlands.exe", "15a6b1b3819ef002438df340509b5373", 647447,
				"install.exe", "15a6b1b3819ef002438df340509b5373", 477839),
	 Common::EN_USA,
	 Common::kPlatformDOS,
	 ADGF_UNSUPPORTED,
	 GUIO1(GUIO_NOMIDI)},
	{"soldier-boyz", // Solidier Boyz (US)
	 "",
	 AD_ENTRY2s("boyz.exe", "bac1d734f2606dbdd0816dfa7a5cf518", 263347,
				"setup.exe", "bac1d734f2606dbdd0816dfa7a5cf518", 160740),
	 Common::EN_USA,
	 Common::kPlatformWindows,
	 ADGF_UNSUPPORTED,
	 GUIO1(GUIO_NOMIDI)},
	AD_TABLE_END_MARKER};
} // End of namespace Hypno

static const char *const directoryGlobs[] = {
	"boyz",
	"sixdemo",
	"demo",
	"factory",
	"wetlands",
	"movie",
	"c_misc",
	0};

class HypnoMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	HypnoMetaEngineDetection() : AdvancedMetaEngineDetection(Hypno::gameDescriptions, sizeof(ADGameDescription), Hypno::hypnoGames) {
		_maxScanDepth = 10;
		_directoryGlobs = directoryGlobs;
	}

	const char *getEngineId() const override {
		return "hypno";
	}

	const char *getName() const override {
		return "Hypno";
	}

	const char *getOriginalCopyright() const override {
		return "Marvel Comics Spider-Man: The Sinister Six (C) Brooklyn Multimedia\n"
			   "Wetlands (C) Hypnotix, Inc.";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

REGISTER_PLUGIN_STATIC(HYPNO_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, HypnoMetaEngineDetection);

