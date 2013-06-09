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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "base/plugins.h"

#include "engines/advancedDetector.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/savefile.h"

#include "buried/buried.h"

namespace Buried {

struct BuriedGameDescription {
	ADGameDescription desc;
};

bool BuriedEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL);
}

bool BuriedEngine::isDemo() const {
	return (_gameDescription->desc.flags & ADGF_DEMO) != 0;
}

} // End of namespace Buried

static const PlainGameDescriptor buriedGames[] = {
	{"buried", "The Journeyman Project 2: Buried in Time"},
	{0, 0}
};


namespace Buried {

static const BuriedGameDescription gameDescriptions[] = {
	// Windows 3.11 8BPP
	{
		{
			"buried",
			"8BPP",
			AD_ENTRY1("BIT816.EX_", "166b44e53350c19bb25ef93d2c2b8f79"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
	},

	// Windows 3.11 24BPP
	{
		{
			"buried",
			"24BPP",
			AD_ENTRY1("BIT2416.EX_", "a9ac76610ba614b59235a7d5e00e4a62"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
	},

	// Demo 8BPP
	{
		{
			"buried",
			"Demo 8BPP",
			AD_ENTRY1("BIT816.EXE", "a5bca831dac0903a304c29c320f881c5"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
	},

	// Demo 24BPP
	{
		{
			"buried",
			"Demo 24BPP",
			AD_ENTRY1("BIT2416.EXE", "9857e2d2b7a63b1304058dabc5098249"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
	},

	{ AD_TABLE_END_MARKER }
};

} // End of namespace Buried


class BuriedMetaEngine : public AdvancedMetaEngine {
public:
	BuriedMetaEngine() : AdvancedMetaEngine(Buried::gameDescriptions, sizeof(Buried::BuriedGameDescription), buriedGames) {
		_singleid = "buried";
	}

	virtual const char *getName() const {
		return "The Journeyman Project 2: Buried in Time";
	}

	virtual const char *getOriginalCopyright() const {
		return "The Journeyman Project 2: Buried in Time (C) Presto Studios";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool BuriedMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Buried::BuriedGameDescription *gd = (const Buried::BuriedGameDescription *)desc;

	if (gd)
		*engine = new Buried::BuriedEngine(syst, gd);

	return (gd != 0);
}

#if PLUGIN_ENABLED_DYNAMIC(BURIED)
	REGISTER_PLUGIN_DYNAMIC(BURIED, PLUGIN_TYPE_ENGINE, BuriedMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(BURIED, PLUGIN_TYPE_ENGINE, BuriedMetaEngine);
#endif

