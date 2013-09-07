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

enum {
	GF_TRUECOLOR  = (1 << 1),
	GF_WIN95      = (1 << 2),
	GF_COMPRESSED = (1 << 3)
};

bool BuriedEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL);
}

bool BuriedEngine::isDemo() const {
	return (_gameDescription->desc.flags & ADGF_DEMO) != 0;
}

bool BuriedEngine::isTrueColor() const {
	return (_gameDescription->desc.flags & GF_TRUECOLOR) != 0;
}

bool BuriedEngine::isWin95() const {
	return (_gameDescription->desc.flags & GF_WIN95) != 0;
}

bool BuriedEngine::isCompressed() const {
	return (_gameDescription->desc.flags & GF_COMPRESSED) != 0;
}

Common::String BuriedEngine::getEXEName() const {
	return _gameDescription->desc.filesDescriptions[0].fileName;
}

Common::String BuriedEngine::getLibraryName() const {
	return _gameDescription->desc.filesDescriptions[1].fileName;
}

} // End of namespace Buried

static const PlainGameDescriptor buriedGames[] = {
	{"buried", "The Journeyman Project 2: Buried in Time"},
	{0, 0}
};


namespace Buried {

static const BuriedGameDescription gameDescriptions[] = {
	// Windows 3.11 8BPP
	// Installed
	{
		{
			"buried",
			"8BPP",
			{
				{ "BIT816.EXE",  0, "57a14461c77d9c77534bd418043db1ec", 1163776 },
				{ "BIT8LIB.DLL", 0, "31bcd9e5cc32df00b09ce626e6d9106e", 2420480 },
				{ 0, 0, 0, 0 },
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
	},

	// Windows 3.11 24BPP
	// Installed
	{
		{
			"buried",
			"24BPP",
			{
				{ "BIT2416.EXE",  0, "dcbfb3f2916ad902043942fc00d2017f", 1159680 },
				{ "BIT24LIB.DLL", 0, "74ac9dae92f415fea8cdbd220ba8795c", 5211648 },
				{ 0, 0, 0, 0 },
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			GF_TRUECOLOR,
			GUIO0()
		},
	},

	// Windows 3.11 8BPP
	// Not Installed
	{
		{
			"buried",
			"8BPP",
			{
				{ "BIT816.EX_",  0, "166b44e53350c19bb25ef93d2c2b8f79", 364490 },
				{ "BIT8LIB.DL_", 0, "8a345993f60f6bed7c17fa9e7f2bc37d", 908854 },
				{ 0, 0, 0, 0 },
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			GF_COMPRESSED,
			GUIO0()
		},
	},

	// Windows 3.11 24BPP
	// Not Installed
	{
		{
			"buried",
			"24BPP",
			{
				{ "BIT2416.EX_",  0, "a9ac76610ba614b59235a7d5e00e4a62", 361816 },
				{ "BIT24LIB.DL_", 0, "00e6eedbcef824988fbb01a87ca8f7fd", 2269314 },
				{ 0, 0, 0, 0 },
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			GF_COMPRESSED | GF_TRUECOLOR,
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
			ADGF_DEMO,
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
			ADGF_DEMO | GF_TRUECOLOR,
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
		_flags = kADFlagUseExtraAsHint;
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

