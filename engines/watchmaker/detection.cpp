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

#include "watchmaker/watchmaker.h"

static const PlainGameDescriptor watchmakerGames[] = {
	{"watchmaker", "The Watchmaker"},
	{nullptr, nullptr}
};

static const DebugChannelDef debugFlagList[] = {
	{Watchmaker::kDebugGeneral, "general", "General debug level"},
	DEBUG_CHANNEL_END
};

namespace Watchmaker {

static const ADGameDescription gameDescriptions[] = {
	// The Watchmaker English Retail
	{
		"watchmaker",
		nullptr,
		AD_ENTRY1s("Data.wm", "a1675e8fd32c2c1eaed88e292a0d153c", 6017156),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Watchmaker

class WatchmakerMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	WatchmakerMetaEngineDetection() : AdvancedMetaEngineDetection(Watchmaker::gameDescriptions, sizeof(ADGameDescription), watchmakerGames) {
	}

	const char *getName() const override {
		return "watchmaker";
	}

	const char *getEngineName() const override {
		return "The Watchmaker";
	}

	const char *getOriginalCopyright() const override {
		return "The Watchmaker (C) 2002 Trecision SpA.";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

REGISTER_PLUGIN_STATIC(WATCHMAKER_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, WatchmakerMetaEngineDetection);
