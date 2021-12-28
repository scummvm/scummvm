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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "base/plugins.h"

#include "engines/advancedDetector.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"

#include "buried/buried.h"

static const PlainGameDescriptor buriedGames[] = {
	{"buried", "The Journeyman Project 2: Buried in Time"},
	{nullptr, nullptr}
};

#include "buried/detection_tables.h"

namespace Buried {

static const char *directoryGlobs[] = {
	"win31",
	"manual",
	nullptr
};

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ALLOW_SKIP,
		{
			// I18N: This option allows the user to skip cutscenes.
			_s("Skip support"),
			_s("Allow cutscenes to be skipped"),
			"skip_support",
			true
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};
} // End of namespace Buried


class BuriedMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	BuriedMetaEngineDetection() : AdvancedMetaEngineDetection(
		Buried::gameDescriptions,
		sizeof(ADGameDescription),
		buriedGames,
		Buried::optionsList) {
		_flags = kADFlagUseExtraAsHint;
		_maxScanDepth = 3;
		_directoryGlobs = Buried::directoryGlobs;
	}

	const char *getEngineId() const override {
		return "buried";
	}

	const char *getName() const override {
		return "The Journeyman Project 2: Buried in Time";
	}

	const char *getOriginalCopyright() const override {
		return "The Journeyman Project 2: Buried in Time (C) Presto Studios";
	}
};

REGISTER_PLUGIN_STATIC(BURIED_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, BuriedMetaEngineDetection);
