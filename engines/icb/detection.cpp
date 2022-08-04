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

	{
		{
			// The Road to El Dorado
			// Brazilian Portuguese
			"eldorado",
			"",
			AD_ENTRY2s("g/speech.clu", "f69efab57478cd9b1792880298eada8d", 1927072, "g/g", "ed4e6fed9c3c5b2cbccec30484eef434", 19280),
			Common::PT_BRA,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		GType_ELDORADO
	},

	{
		{
			// The Road to El Dorado
			// English demo from CyberMycha 02/2001
			"eldorado",
			"Demo",
			AD_ENTRY2s("g/speech.clu", "93905d81f6e11f8c1b9c6a1dcd303e68", 1970016, "g/g", "7e8aabc6fe481317de92fd18bdf328df", 17040),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		GType_ELDORADO
	},

	{
		{
			// The Road to El Dorado
			// English US PS1
			"eldorado",
			0,
			AD_ENTRY2s("g/speech.clu", "641adcc7b812dcd99a09e78bce22f2ce", 349480, "g/havenota.clu", "0a51dd4f3f49e8d0b209ea410ed42853", 17064),
			Common::EN_USA,
			Common::kPlatformPSX,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		GType_ELDORADO
	},

	{
		{
			// The Road to El Dorado
			// English EU PS1
			"eldorado",
			0,
			AD_ENTRY2s("g/speech.clu", "641adcc7b812dcd99a09e78bce22f2ce", 349480, "g/havenota.clu", "7276ce48dcb2ff87181292f9c6889f19", 17064),
			Common::EN_GRB,
			Common::kPlatformPSX,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		GType_ELDORADO
	},

	{
		{
			// The Road to El Dorado
			// German PS1
			"eldorado",
			0,
			AD_ENTRY2s("g/speech.clu", "4314272e910bfc72aa9224c179ea90ba", 328600, "g/havenota.clu", "0fb92797c62e1f5fd35a08292c681cd1", 18176),
			Common::DE_DEU,
			Common::kPlatformPSX,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		GType_ELDORADO
	},

	{
		{
			// The Road to El Dorado
			// French PS1
			"eldorado",
			0,
			AD_ENTRY2s("g/speech.clu", "2c0095ac493d847a60a9c395282bfe9b", 287032, "g/havenota.clu", "195c71302b79b3be2b60e11a2b49bae0", 18032),
			Common::FR_FRA,
			Common::kPlatformPSX,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		GType_ELDORADO
	},

	{
		{
			// The Road to El Dorado
			// Spanish PS1
			"eldorado",
			0,
			AD_ENTRY2s("g/speech.clu", "3862579927ef45bd603a6e19ee1bbaff", 353656, "g/havenota.clu", "c053da010d3aeb3d242f7f7d6f222669", 18112),
			Common::ES_ESP,
			Common::kPlatformPSX,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		GType_ELDORADO
	},

	{
		{
			// The Road to El Dorado
			// Italian PS1
			"eldorado",
			0,
			AD_ENTRY2s("g/speech.clu", "c28ccf1def3fbf6de597f900b9c8d4e4", 288984, "g/havenota.clu", "fec13ff938c9f7ed054124b0bba10528", 17944),
			Common::IT_ITA,
			Common::kPlatformPSX,
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
