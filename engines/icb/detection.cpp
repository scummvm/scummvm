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

#include "engines/icb/detection.h"

namespace ICB {

static const PlainGameDescriptor icbGames[] = {
	{ "icb", "In Cold Blood" },
	{ "eldorado", "The Road to El Dorado" },
	{ nullptr, nullptr }
};

static const IcbGameDescription gameDescriptions[] = {
	{
		{
			// In Cold Blood
			// English GOG Version
			"icb",
			"GOG",
			AD_ENTRY2s("g/speech.clu", "ced60009bdffa9a1055863bf10d2e79e", 7683040, "g/g", "ebe9e5377ee9d231e7a7e33666eb517b", 63892),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		GType_ICB
	},

	{
		{
			// In Cold Blood
			// English Demo
			"icb",
			"Demo",
			AD_ENTRY2s("g/speech.clu", "ced60009bdffa9a1055863bf10d2e79e", 7683040, "g/g", "f8e1a762cc25554846f6e45d59f55159", 54560),
			Common::EN_GRB,
			Common::kPlatformWindows,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		GType_ICB
	},

	{
		{
			// In Cold Blood
			// English Demo
			"icb",
			"Demo",
			AD_ENTRY2s("g/speech.clu", "ced60009bdffa9a1055863bf10d2e79e", 7683040, "g/g", "2b9a8ca8d7a1bd91701708535b171aa8", 62080),
			Common::EN_USA,
			Common::kPlatformWindows,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		GType_ICB
	},

	{
		{
			// The Road to El Dorado
			// English
			"eldorado",
			"",
			AD_ENTRY2s("g/speech.clu", "65acb862dcdef2a7a69b961760bd565e", 1927072, "g/g", "ed36c6fb807a3ececba1bbda94172842", 17704),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		GType_ELDORADO
	},

	{
		{
			// The Road to El Dorado
			// Polish
			"eldorado",
			"",
			AD_ENTRY2s("g/speech.clu", "b063eee3ae6e327c37149a8342700004", 2009640, "g/g", "1427ecf118fc37da4ebb2f29d16d3d9b", 18520),
			Common::PL_POL,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		GType_ELDORADO
	},

	{ AD_TABLE_END_MARKER, GType_ICB }
};

class IcbMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	IcbMetaEngineDetection() : AdvancedMetaEngineDetection(gameDescriptions, sizeof(IcbGameDescription), icbGames) {
		_guiOptions = GUIO_NOMIDI;
		_flags = kADFlagMatchFullPaths;
	}

	const char *getEngineName() const override { return "In Cold Blood Engine"; }

	const char *getName() const override {
		return "icb";
	}

	const char *getOriginalCopyright() const override { return "(C) 2000 Revolution Software Ltd"; }
};

} // End of namespace ICB

REGISTER_PLUGIN_STATIC(ICB_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, ICB::IcbMetaEngineDetection);
