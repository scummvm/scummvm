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

static const PlainGameDescriptor trecisionGames[] = {
	{"aot", "Ark of Time"},
	{"nl", "Nightlong: Union City Conspiracy"},
	{0, 0}
};

namespace Trecision {

#define AD_NL_ENTRY(md5, size) \
	{ \
		{"data.nl", 0, md5, size}, \
		{"nlanim.cd1", 0, nullptr, -1}, \
		{"nlanim.cd2", 0, nullptr, -1}, \
		{"nlanim.cd3", 0, nullptr, -1}, \
		AD_LISTEND \
	}

#define AD_NL_DEMO_ENTRY(md5, size) \
	{ \
		{"data.nl", 0, md5, size}, \
		{"nlanim.cd1", 0, nullptr, -1}, \
		AD_LISTEND \
	}

static const ADGameDescription gameDescriptions[] = {
	{
		"nl",
		0,
		AD_NL_ENTRY("7665db13ad2a1ceb576531be3e1efb30", 436228),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	{
		"nl",
		0,
		AD_NL_ENTRY("7665db13ad2a1ceb576531be3e1efb30", 436598),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	{
		"nl",
		0,
		AD_NL_ENTRY("7665db13ad2a1ceb576531be3e1efb30", 457299),
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	{
		"nl",
		0,
		AD_NL_ENTRY("7665db13ad2a1ceb576531be3e1efb30", 436697),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	{
		"nl",
		0,
		AD_NL_ENTRY("7665db13ad2a1ceb576531be3e1efb30", 456209),
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	{
		"nl",
		0,
		AD_NL_ENTRY("7665db13ad2a1ceb576531be3e1efb30", 446634),
		Common::IT_ITA,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	{
		"nl",
		0,
		AD_NL_ENTRY("7665db13ad2a1ceb576531be3e1efb30", 432900),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	// bug #12619
	{
		"nl",
		0,
		AD_NL_ENTRY("7665db13ad2a1ceb576531be3e1efb30", 429370),
		Common::HU_HUN,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	// bug #12619
	{
		"nl",
		0,
		AD_NL_ENTRY("7665db13ad2a1ceb576531be3e1efb30", 429731),
		Common::HU_HUN,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	{
		"nl",
		"Demo",
		AD_NL_DEMO_ENTRY("7665db13ad2a1ceb576531be3e1efb30", 392950),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DEMO,
		GUIO0()
	},
	{
		"nl",
		"Demo",
		AD_NL_DEMO_ENTRY("7665db13ad2a1ceb576531be3e1efb30", 413651),
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_DEMO,
		GUIO0()
	},
	{
		"nl",
		0,
		AD_NL_ENTRY("2bfc3f5cc1ee5c7e80058db853296416", 436807),
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO0()
	},
	{
		"nl",
		0,
		AD_NL_ENTRY("2bfc3f5cc1ee5c7e80058db853296416", 457508),
		Common::DE_DEU,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO0()
	},
	{
		"nl",
		0,
		AD_NL_ENTRY("2bfc3f5cc1ee5c7e80058db853296416", 436842),
		Common::ES_ESP,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO0()
	},
	{
		"nl",
		0,
		AD_NL_ENTRY("2bfc3f5cc1ee5c7e80058db853296416", 456354),
		Common::FR_FRA,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO0()
	},
	{
		"nl",
		0,
		AD_NL_ENTRY("2bfc3f5cc1ee5c7e80058db853296416", 446779),
		Common::IT_ITA,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO0()
	},
	{
		"aot",
		_s("Missing game code"),
		{
			{"dialogue.dat", 0, "afc71fe29b1be3a9b145b8d61dfa4539", 166155130},
			{"sentence.dat", 0, "f38afcd22e7de14f9a2343e911eaa126", 75668232},
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSUPPORTED,
		GUIO0()
	},
	AD_TABLE_END_MARKER
};

#define GAMEOPTION_ORIGINAL_SAVELOAD GUIO_GAMEOPTIONS1

static const ADExtraGuiOptionsMap optionsList[] = {

	{
		GAMEOPTION_ORIGINAL_SAVELOAD,
		{
			_s("Use original save/load screens"),
			_s("Use the original save/load screens instead of the ScummVM ones"),
			"originalsaveload",
			false,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

} // End of namespace Trecision

static const char *directoryGlobs[] = {
	"autorun",
	"data",
	0
};

class TrecisionMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	TrecisionMetaEngineDetection() : AdvancedMetaEngineDetection(Trecision::gameDescriptions, sizeof(ADGameDescription), trecisionGames, Trecision::optionsList) {
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
		_guiOptions = GUIO2(GUIO_NOMIDI, GAMEOPTION_ORIGINAL_SAVELOAD);
	}

	const char *getName() const override {
		return "trecision";
	}

	const char *getEngineName() const override {
		return "Trecision Adventure Module";
	}

	const char *getOriginalCopyright() const override {
		return "(C) 1993-98 Trecision S.p.A.";
	}
};

REGISTER_PLUGIN_STATIC(TRECISION_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, TrecisionMetaEngineDetection);
