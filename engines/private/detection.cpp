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
#include "engines/advancedDetector.h"
#include "private/private.h"

static const DebugChannelDef debugFlagList[] = {
	{Private::kPrivateDebugFunction, "functions", "Function execution debug channel"},
	{Private::kPrivateDebugCode, "code", "Code execution debug channel"},
	{Private::kPrivateDebugScript, "script", "Script execution debug channel"},
	DEBUG_CHANNEL_END
};

namespace Private {
static const PlainGameDescriptor privateGames[] = {
	{ "private-eye", "Private Eye" },
	{ 0, 0 }
};

static const ADGameDescription gameDescriptions[] = {
	{
		"private-eye",  // US release
		0,
		AD_ENTRY2s("pvteye.z", "b682118cda6a42fa89833cae2b8824bd", 271895,
		   "bklynlgo.bmp", "1dfb703349a46f8ec183de107992b7f5", 33118),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_TESTING,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"private-eye", // Demo from the US release
		"Demo",
		AD_ENTRY2s("pvteye.z", "af383c813157810e89d8d6d595895ff7", 263893,
		   "bklynlgo.bmp", "1dfb703349a46f8ec183de107992b7f5", 33118),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_TESTING | ADGF_DEMO,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"private-eye",  // EU release
		0,
		AD_ENTRY2s("pvteye.z", "d9ce391395701615e8b5d04bc4bf7ec3", 284699,
		   "bklynlgo.bmp", "1dfb703349a46f8ec183de107992b7f5", 33118),
		Common::EN_GRB,
		Common::kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"private-eye", // Demo from the EU release
		"Demo",
		AD_ENTRY2s("pvteye.z", "01ca8641970189cb2ca3a96526026091", 284129,
		   "bklynlgo.bmp", "1dfb703349a46f8ec183de107992b7f5", 33118),
		Common::EN_GRB,
		Common::kPlatformWindows,
		ADGF_DEMO | ADGF_UNSUPPORTED,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"private-eye", // Demo from PCGamer Disc 2.6 JULY 1996
		"Demo",
		AD_ENTRY2s("pvteye.z", "8ef908e212bb9c1e10f5e3c81f56682c", 263893,
		   "bklynlgo.bmp", "1dfb703349a46f8ec183de107992b7f5", 33118),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_TESTING | ADGF_DEMO,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"private-eye", // Another demo
		"Demo",
		AD_ENTRY2s("pvteye.z", "af383c813157810e89d8d6d595895ff7", 271214,
		   "bklynlgo.bmp", "1dfb703349a46f8ec183de107992b7f5", 33118),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_TESTING | ADGF_DEMO,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"private-eye", // EU release (ES)
		"It uses different file format for the assest",
		AD_ENTRY2s("pvteye.ex_", "f41770550ab717086b2d0c805fef4b8f", 498176,
		   "bklynlgo.bmp", "1dfb703349a46f8ec183de107992b7f5", 33118),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"private-eye", // Demo from the  EU release (ES)
		"It uses different file format for the assest",
		AD_ENTRY2s("pvtdemo.ex_", "048f751acd7a0f1a87b20d6dc5229210", 497152,
		   "bklynlgo.bmp", "1dfb703349a46f8ec183de107992b7f5", 33118),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_DEMO | ADGF_UNSUPPORTED,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"private-eye", // EU release (FR)
		"It uses different file format for the assest",
		AD_ENTRY2s("pvteye.ex_", "ae0dec43b2f54d45c8a1c93e97092141", 600576,
		   "bklynlgo.bmp", "1dfb703349a46f8ec183de107992b7f5", 33118),
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"private-eye", // EU release (DE)
		"It uses different file format for the assest",
		AD_ENTRY2s("pvteye.ex_", "5ca171c4e8d804c7277887277d049f03", 600576,
		   "bklynlgo.bmp", "1dfb703349a46f8ec183de107992b7f5", 33118),
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"private-eye", // Demo from the EU release
		"Demo",
		AD_ENTRY2s("Private Eye Demo", "", 284129,
		   "bklynlgo.bmp", "1dfb703349a46f8ec183de107992b7f5", 33118),
		Common::EN_GRB,
		Common::kPlatformWindows,
		ADGF_DEMO | ADGF_UNSUPPORTED,
		GUIO1(GUIO_NOMIDI)
	},
	/*
	{
		"private-eye", // Demo from the EU release
		"Demo",
		AD_ENTRY1s("PVTEYE.Z", "", 0),
		Common::EN_GRB,
		Common::kPlatformWindows,
		ADGF_DEMO | ADGF_UNSUPPORTED,
		GUIO1(GUIO_NOMIDI)
	},
	*/
	AD_TABLE_END_MARKER
};
} // End of namespace Private

static const char *const directoryGlobs[] = {
	"support",
	"intro",
	0
};

class PrivateMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	PrivateMetaEngineDetection() : AdvancedMetaEngineDetection(Private::gameDescriptions, sizeof(ADGameDescription), Private::privateGames) {
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	const char *getEngineId() const override {
		return "private";
	}

	const char *getName() const override {
		return "Private Eye";
	}

	const char *getOriginalCopyright() const override {
		return "Copyright (C) Brooklyn Multimedia";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

REGISTER_PLUGIN_STATIC(PRIVATE_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, PrivateMetaEngineDetection);
