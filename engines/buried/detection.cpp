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

Common::Language BuriedEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

} // End of namespace Buried

static const PlainGameDescriptor buriedGames[] = {
	{"buried", "The Journeyman Project 2: Buried in Time"},
	{0, 0}
};


namespace Buried {

static const BuriedGameDescription gameDescriptions[] = {
	// English Windows 3.11 8BPP
	// Installed
	// v1.01
	{
		{
			"buried",
			"v1.01 8BPP",
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

	// English Windows 3.11 24BPP
	// Installed
	// v1.01
	{
		{
			"buried",
			"v1.01 24BPP",
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

	// Japanese Windows 3.11 8BPP
	// Installed
	// v1.051
	{
		{
			"buried",
			"v1.051 8BPP",
			{
				{ "BIT816.EXE",  0, "decbf9a7d91803525137ffd980d16708", 1163264 },
				{ "BIT8LIB.DLL", 0, "f5ccde0efccb95afe902627a35262568", 2418816 },
				{ 0, 0, 0, 0 },
			},
			Common::JA_JPN,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
	},

	// Japanese Windows 3.11 24BPP
	// Installed
	// v1.051
	{
		{
			"buried",
			"v1.051 24BPP",
			{
				{ "BIT2416.EXE",  0, "9435b9a40e3ac83e6fa1e83caaf57792", 1157632 },
				{ "BIT24LIB.DLL", 0, "4d55802259d9648b9aa396461bfd53a3", 6576896 },
				{ 0, 0, 0, 0 },
			},
			Common::JA_JPN,
			Common::kPlatformWindows,
			GF_TRUECOLOR,
			GUIO0()
		},
	},

	// English Windows 3.11 8BPP
	// Not Installed
	// v1.01
	{
		{
			"buried",
			"v1.01 8BPP",
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

	// English Windows 3.11 24BPP
	// Not Installed
	// v1.01
	{
		{
			"buried",
			"v1.01 24BPP",
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

	// English Windows 95 8BPP
	// v1.1
	{
		{
			"buried",
			"v1.1 8BPP",
			{
				{ "BIT832.EXE",  0, "f4f8007f49197ba40ea633eb113c0b6d", 1262592 },
				{ "BIT8L32.DLL", 0, "addfef0420e1f41a7766ecc6baa58553", 2424832 },
				{ 0, 0, 0, 0 },
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			GF_WIN95,
			GUIO0()
		},
	},

	// English Windows 95 24BPP
	// v1.1
	{
		{
			"buried",
			"v1.1 24BPP",
			{
				{ "BIT2432.EXE",  0, "4086a8200938eac3e72d238a84f65618", 1257472 },
				{ "BIT24L32.DLL", 0, "198bfd476d5228c4a7a63c029cffadfc", 5216256 },
				{ 0, 0, 0, 0 },
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			GF_TRUECOLOR | GF_WIN95,
			GUIO0()
		},
	},

	// English Windows Demo 8BPP
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

	// English Windows Demo 24BPP
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

static const char *directoryGlobs[] = {
	"win31",
	"manual",
	0
};

} // End of namespace Buried


class BuriedMetaEngine : public AdvancedMetaEngine {
public:
	BuriedMetaEngine() : AdvancedMetaEngine(Buried::gameDescriptions, sizeof(Buried::BuriedGameDescription), buriedGames) {
		_singleid = "buried";
		_flags = kADFlagUseExtraAsHint;
		_maxScanDepth = 3;
		_directoryGlobs = Buried::directoryGlobs;
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

