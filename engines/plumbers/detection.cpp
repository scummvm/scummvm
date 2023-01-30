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
#include "engines/game.h"

#include "plumbers/plumbers.h"

static const PlainGameDescriptor plumbersGames[] = {
	{"plumbers", "Plumbers Don't Wear Ties!"},
	{nullptr, nullptr}
};

static const DebugChannelDef debugFlagList[] = {
	{Plumbers::kDebugGeneral, "general", "General debug level"},
	DEBUG_CHANNEL_END
};

namespace Plumbers {

static const ADGameDescription gameDescriptions[] = {
	// Plumbers PC version
	{
		"plumbers",
		nullptr,
		AD_ENTRY1s("GAME.BIN", "02c965b11e952ce1ee83019576f5aef5", 41622),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},

	// Plumbers 3DO version
	{
		"plumbers",
		nullptr,
		AD_ENTRY1s("launchme", "d3ab77d1a8a2289422a0f51e7aa91821", 143300),
		Common::EN_ANY,
		Common::kPlatform3DO,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Plumbers

class PlumbersMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	PlumbersMetaEngineDetection() : AdvancedMetaEngineDetection(Plumbers::gameDescriptions, sizeof(ADGameDescription), plumbersGames) {
	}

	const char *getName() const override {
		return "plumbers";
	}

	const char *getEngineName() const override {
		return "Plumbers Don't Wear Ties";
	}

	const char *getOriginalCopyright() const override {
		return "Plumbers Don't Wear Ties (C) 1993-94 Kirin Entertainment";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

REGISTER_PLUGIN_STATIC(PLUMBERS_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, PlumbersMetaEngineDetection);
