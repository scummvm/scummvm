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

#include "engines/advancedDetector.h"
#include "base/plugins.h"

#include "bbvs/detection.h"

static const PlainGameDescriptor bbvsGames[] = {
	{ "bbvs", "Beavis and Butt-Head in Virtual Stupidity" },
	{ nullptr, nullptr }
};

namespace Bbvs {

static const ADGameDescription gameDescriptions[] = {
	{
		"bbvs",
		nullptr,
		AD_ENTRY1s("vspr0001.vnm", "7ffe9b9e7ca322db1d48e86f5130578e", 1166628),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)
	},

	{
		"bbvs",
		"Demo",
		AD_ENTRY1s("vspr0007.vnm", "5db44940fa93fdd5becb5c2a5ded7478", 242376),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DEMO | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)
	},

	{
		"bbvs",
		"Loogie Demo",
		AD_ENTRY1s("BBLOOGIE.000", "607d3bf55ec6458dce484473b1eecb4d", 324416),
		Common::EN_ANY,
		Common::kPlatformWindows,
		GF_LOOGIE_DEMO | ADGF_DEMO | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)
	},

	{
		"bbvs",
		"Loogie Demo",
		AD_ENTRY1s("BBLOOGIE.000", "83921c65bd93be7e35056c31bd37310b", 324764),
		Common::EN_ANY,
		Common::kPlatformWindows,
		GF_LOOGIE_ALT_DEMO | ADGF_DEMO | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)
	},

	{
		"bbvs",
		nullptr,
		AD_ENTRY1s("vspr0001.vnm", "91c76b1048f93208cd7b1a05ebccb408", 1176976),
		Common::RU_RUS,
		Common::kPlatformWindows,
		GF_GUILANGSWITCH | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Bbvs

static const char * const directoryGlobs[] = {
	"vnm",
	nullptr
};

class BbvsMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	BbvsMetaEngineDetection() : AdvancedMetaEngineDetection(Bbvs::gameDescriptions, sizeof(ADGameDescription), bbvsGames) {
		_maxScanDepth = 3;
		_directoryGlobs = directoryGlobs;
	}

	const char *getName() const override {
		return "bbvs";
	}

	const char *getEngineName() const override {
		return "MTV's Beavis and Butt-Head in Virtual Stupidity";
	}

	const char *getOriginalCopyright() const override {
		return "(C) 1995 Viacom New Media";
	}
};

REGISTER_PLUGIN_STATIC(BBVS_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, BbvsMetaEngineDetection);
