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

#include "chewy/detection.h"

#include "common/translation.h"

static const PlainGameDescriptor chewyGames[] = {
	{"chewy", "Chewy: Esc from F5"},
	{nullptr, nullptr}
};

namespace Chewy {

#define GAMEOPTION_ORIGINAL_SAVELOAD GUIO_GAMEOPTIONS1

static const ChewyGameDescription gameDescriptions[] = {

	{
		{
			"chewy",
			nullptr,
			AD_ENTRY1s("txt/atds.tap", "e6050c144dd4f23d79ea4f89a8ef306e", 218857),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOMIDI, GAMEOPTION_ORIGINAL_SAVELOAD)
		},
	},

	{
		// Chewy - ESC from F5 - Spanish fanmade translation
		{
			"chewy",
			nullptr,
			AD_ENTRY1s("txt/atds.tap", "b1210066a524fe0f88862f44671ed97d", 226988),
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOMIDI, GAMEOPTION_ORIGINAL_SAVELOAD)
		},
	},

	{
		{
			"chewy",
			nullptr,
			AD_ENTRY1s("txt/atds.tap", "c117e884cc5b4bbe50ae1217d13916c4", 231071),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOMIDI, GAMEOPTION_ORIGINAL_SAVELOAD)
		},
	},

	{
		{
			"chewy",
			nullptr,
			AD_ENTRY1s("txt/atds.tap", "e22f97761c0e7772ec99660f2277b1a4", 231001),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOMIDI, GAMEOPTION_ORIGINAL_SAVELOAD)
		},
	},

	{
		// Chewy - ESC von F5 - German demo
		// Demo version 1.1 (CHEWY.EXE - offset 0x8A520)
		// From Blue Byte News Version III/97
		// The source CD-ROM has the Matrix code SONOPRESS T-2037 A
		// CD-ROM has the label "DEMO"
		{
			"chewy",
			nullptr,
			AD_ENTRY1s("txt/atds.tap", "c3be5641e90dd01274309b778cf8146d", 230686),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_DEMO,
			GUIO2(GUIO_NOMIDI, GAMEOPTION_ORIGINAL_SAVELOAD)
		},
	},

	{ AD_TABLE_END_MARKER }
};

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ORIGINAL_SAVELOAD,
		{
			_s("Use original save/load screens"),
			_s("Use the original save/load screens instead of the ScummVM ones"),
			"original_menus",
			false
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR};
} // namespace Chewy

class ChewyMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	ChewyMetaEngineDetection() : AdvancedMetaEngineDetection(Chewy::gameDescriptions, sizeof(Chewy::ChewyGameDescription), chewyGames, Chewy::optionsList) {
		_maxScanDepth = 2;
		_flags = kADFlagMatchFullPaths;
	}

	const char *getEngineId() const override {
		return "chewy";
	}

	const char *getName() const override {
		return "Chewy: Esc from F5";
	}

	const char *getOriginalCopyright() const override {
		return "Chewy: Esc from F5 (C) 1995 New Generation Software";
	}
};

REGISTER_PLUGIN_STATIC(CHEWY_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, ChewyMetaEngineDetection);
