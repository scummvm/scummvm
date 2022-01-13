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

#include "gnap/gnap.h"

#include "common/config-manager.h"
#include "engines/advancedDetector.h"
#include "common/savefile.h"
#include "common/system.h"
#include "base/plugins.h"
#include "graphics/thumbnail.h"

static const PlainGameDescriptor gnapGames[] = {
	{ "gnap", "Gnap" },
	{ nullptr, nullptr }
};

static const DebugChannelDef debugFlagList[] = {
	{Gnap::kDebugBasic, "basic", "Basic debug level"},
	DEBUG_CHANNEL_END
};

namespace Gnap {

static const ADGameDescription gameDescriptions[] = {
	{
		"gnap", "",
		AD_ENTRY1s("stock_n.dat", "46819043d019a2f36b727cc2bdd6980f", 12515823),
		Common::EN_ANY, Common::kPlatformWindows, ADGF_NO_FLAGS, GUIO0()
	},
	{
		"gnap", "",
		AD_ENTRY1s("stock_n.dat", "46819043d019a2f36b727cc2bdd6980f", 13497301),
		Common::EN_ANY, Common::kPlatformWindows, ADGF_NO_FLAGS, GUIO0()
	},
	{	// Bugreport #11404
		"gnap", "",
		AD_ENTRY1s("stock_n.dat", "46819043d019a2f36b727cc2bdd6980f", 13035286),
		Common::EN_ANY, Common::kPlatformWindows, ADGF_NO_FLAGS, GUIO0()
	},
	{
		"gnap", "",
		AD_ENTRY1s("stock_n.dat", "46819043d019a2f36b727cc2bdd6980f", 12995485),
		Common::RU_RUS, Common::kPlatformWindows, ADGF_NO_FLAGS, GUIO0()
	},
	{
		"gnap", "Fargus",
		AD_ENTRY1s("stock_n.dat", "46819043d019a2f36b727cc2bdd6980f", 12847726),
		Common::RU_RUS, Common::kPlatformWindows, ADGF_NO_FLAGS, GUIO0()
	},
	{
		"gnap", "",
		AD_ENTRY1s("stock_n.dat", "9729e1c77a74f2f23c53596fae385bfa", 13428133),
		Common::PL_POL, Common::kPlatformWindows, ADGF_NO_FLAGS, GUIO0()
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Gnap

class GnapMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	GnapMetaEngineDetection() : AdvancedMetaEngineDetection(Gnap::gameDescriptions, sizeof(ADGameDescription), gnapGames) {
		_maxScanDepth = 3;
	}

	const char *getName() const override {
		return "gnap";
	}

	const char *getEngineName() const override {
		return "Gnap";
	}

	const char *getOriginalCopyright() const override {
		return "Gnap (C) Artech Digital Entertainment 1997";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

REGISTER_PLUGIN_STATIC(GNAP_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, GnapMetaEngineDetection);
