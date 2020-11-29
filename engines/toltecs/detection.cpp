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

#include "common/translation.h"
#include "common/savefile.h"
#include "common/str-array.h"
#include "common/system.h"

#include "toltecs/toltecs.h"
#include "toltecs/detection.h"


static const PlainGameDescriptor toltecsGames[] = {
	{"toltecs", "3 Skulls of the Toltecs"},
	{0, 0}
};


namespace Toltecs {

static const ToltecsGameDescription gameDescriptions[] = {

	{
		// 3 Skulls of the Toltecs English version
		{
			"toltecs",
			0,
			AD_ENTRY1s("WESTERN", "05472037e9cfde146e953c434e74f0f4", 337643527),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
	},

	{
		// 3 Skulls of the Toltecs English version (alternate)
		// From bug #3614933
		{
			"toltecs",
			0,
			AD_ENTRY1s("WESTERN", "a9c9cfef9d05b8f7a5573b626fa4ea87", 337643527),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
	},

	{
		// 3 Skulls of the Toltecs PIRATE CD-RIP version (no audio)
		// == DO NOT RE-ADD ==
		{
			"toltecs",
			0,
			AD_ENTRY1s("WESTERN", "56d0da91ec3db8ac869594357584e851", 104804435),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_PIRATED,
			GUIO1(GUIO_NONE)
		},
	},

	{
		// 3 Skulls of the Toltecs Russian version
		{
			"toltecs",
			0,
			AD_ENTRY1s("WESTERN", "ba1742d3193b68ceb9434e2ab7a09a9b", 391462783),
			Common::RU_RUS,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
	},

	{
		// 3 Skulls of the Toltecs German version
		{
			"toltecs",
			0,
			AD_ENTRY1s("WESTERN", "1a3292bad8e0bb5701800c73531dd75e", 345176617),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
	},

	{
		// 3 Skulls of the Toltecs Polish version
		// Reported by cachaito in Trac#11134
		{
			"toltecs",
			0,
			AD_ENTRY1s("WESTERN", "8ec48dd4e52a822d314418f1d3284e64", 337646148),
			Common::PL_POL,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
	},

	{
		// 3 Skulls of the Toltecs French version
		{
			"toltecs",
			0,
			AD_ENTRY1s("WESTERN", "4fb845635cbdac732453fe23be350df9", 327269545),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
	},

	{
		// 3 Skulls of the Toltecs Spanish version
		{
			"toltecs",
			0,
			AD_ENTRY1s("WESTERN", "479f468beccc1b0ce5873ec523d1380e", 308391018),
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
	},

	{
		// 3 Skulls of the Toltecs Hungarian version
		// From bug #3440641
		{
			"toltecs",
			0,
			AD_ENTRY1s("WESTERN", "69a5572e75409d8c6230b787faa353af", 337647960),
			Common::HU_HUN,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
	},

	{
		// 3 Skulls of the Toltecs Czech version
		// Reported by AfBu in Trac#11263
		{
			"toltecs",
			0,
			AD_ENTRY1s("WESTERN", "57503131c0217c76b07d0b5c14805631", 337644552),
			Common::CZ_CZE,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
	},

	{
		// 3 Skulls of the Toltecs English Demo version
		{
			"toltecs",
			0,
			AD_ENTRY1s("WESTERN", "53a0abd1c0bc5cad8ba18f0e56877705", 46241833),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO,
			GUIO1(GUIO_NONE)
		},
	},

	{
		// 3 Skulls of the Toltecs German Demo version
		{
			"toltecs",
			0,
			AD_ENTRY1s("WESTERN", "1c85e82712d24f1d5c1ea2a66ddd75c2", 47730038),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_DEMO,
			GUIO1(GUIO_NONE)
		},
	},

	{ AD_TABLE_END_MARKER }
};

} // End of namespace Toltecs

static const ExtraGuiOption toltecsExtraGuiOption = {
	_s("Use original save/load screens"),
	_s("Use the original save/load screens instead of the ScummVM ones"),
	"originalsaveload",
	false
};

class ToltecsMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	ToltecsMetaEngineDetection() : AdvancedMetaEngineDetection(Toltecs::gameDescriptions, sizeof(Toltecs::ToltecsGameDescription), toltecsGames) {
	}

	const char *getEngineId() const override {
		return "toltecs";
	}

	const char *getName() const override {
		return "3 Skulls of the Toltecs";
	}

	const char *getOriginalCopyright() const override {
		return "3 Skulls of the Toltecs (C) Revistronic 1996";
	}

	const ExtraGuiOptions getExtraGuiOptions(const Common::String &target) const override;
};

const ExtraGuiOptions ToltecsMetaEngineDetection::getExtraGuiOptions(const Common::String &target) const {
	ExtraGuiOptions options;
	options.push_back(toltecsExtraGuiOption);
	return options;
}

REGISTER_PLUGIN_STATIC(TOLTECS_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, ToltecsMetaEngineDetection);
