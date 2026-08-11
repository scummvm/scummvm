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
#include "engines/advancedDetector.h"
#include "chamber/detection.h"

namespace Chamber {
static const PlainGameDescriptor ChamberGames[] = {
	{"chamber", "Chamber of the Sci-Mutant Priestess"},
	{ 0, 0 }
};


static const ADGameDescription gameDescriptions[] = {
	{
		"chamber",
		"",
		AD_ENTRY2s("ere.pxi",  "5aa5bd2d79eefde70675b0b6734944f2", 134358,
		           "PRES.BIN", "b8376a81d9ef6d9c5f783c64f718a782",   8005),
		Common::UNK_LANG, // EN/FR/DE
		Common::kPlatformDOS,
		ADGF_TESTING | GF_SPLASH_PRES | GF_SPLASH2_DRAP,
		GUIO3(GUIO_RENDERHERCGREEN, GUIO_RENDERHERCAMBER, GUIO_RENDERCGA)
	},

	{
		"chamber",
		"",
		AD_ENTRY2s("kult1.pxi",   "fc0bd31a3c380338f76ff53e421e47b6", 140537,
		           "PRESCGA.BIN", "346be1f34fa5278e78bec85f3d349dc3",   5105),
		Common::EN_USA,
		Common::kPlatformDOS,
		ADGF_TESTING | GF_SPLASH_PRESCGA,
		GUIO3(GUIO_RENDERHERCGREEN, GUIO_RENDERHERCAMBER, GUIO_RENDERCGA)
	},

	{
		"chamber",
		"EGA",
		AD_ENTRY2s("Kult2.pxi",   "36a9a0a14c1badfff7643ac3fed24b43", 142658,
		           "PRESEGA.EGA", "c3fb9e55ec789cd6dc67259a48025c4c",  32000),
		Common::EN_USA,
		Common::kPlatformDOS,
		ADGF_TESTING | GF_SPLASH_PRESEGA,
		GUIO4(GUIO_RENDERHERCGREEN, GUIO_RENDERHERCAMBER, GUIO_RENDERCGA, GUIO_RENDEREGA)
	},

	// Amiga EU "Kult" build.
	{
		"chamber",
		"Amiga",
		AD_ENTRY2s("FOND.BIN", "a838b23a9fa5d25f88a02dee95307996", 32000,
		           "6.BIN",    "t:926c11afce55dc08e1bb15b1253fd677", 10702),
		Common::UNK_LANG, // EU EN/FR/DE
		Common::kPlatformAmiga,
		ADGF_TESTING,
		GUIO1(GUIO_RENDERAMIGA)
	},

	// Amiga US "Chamber" (Draconian) build.
	{
		"chamber",
		"Amiga",
		AD_ENTRY2s("FOND.BIN", "a838b23a9fa5d25f88a02dee95307996", 32000,
		           "6.BIN",    "t:3eb1d3f844593949697540de3e74f74a", 16046),
		Common::EN_USA,
		Common::kPlatformAmiga,
		ADGF_TESTING,
		GUIO1(GUIO_RENDERAMIGA)
	},

	AD_TABLE_END_MARKER
};
} // End of namespace Chamber

class ChamberMetaEngineDetection : public AdvancedMetaEngineDetection<ADGameDescription> {
public:
	ChamberMetaEngineDetection() : AdvancedMetaEngineDetection(Chamber::gameDescriptions, Chamber::ChamberGames) {
		// Use kADFlagUseExtraAsHint to distinguish between the CGA and EGA versions
		// when both are present in the same directory (e.g. the Steam release)
		_flags = kADFlagUseExtraAsHint;
	}

	const char *getName() const override {
		return "chamber";
	}

	const char *getEngineName() const override {
		return "chamber";
	}

	const char *getOriginalCopyright() const override {
		return "Chamber (C) 1989 ERE Informatique";
	}
};

REGISTER_PLUGIN_STATIC(CHAMBER_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, ChamberMetaEngineDetection);
