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
 * $URL$
 * $Id$
 *
 */

#include "base/plugins.h"

#include "common/advancedDetector.h"
#include "common/file.h"

#include "made/made.h"


namespace Made {

struct MadeGameDescription {
	Common::ADGameDescription desc;

	int gameID;
	int gameType;
	uint32 features;
	uint16 version;
};

uint32 MadeEngine::getGameID() const {
	return _gameDescription->gameID;
}

uint32 MadeEngine::getFeatures() const {
	return _gameDescription->features;
}

Common::Platform MadeEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

uint16 MadeEngine::getVersion() const {
	return _gameDescription->version;
}

}

static const PlainGameDescriptor madeGames[] = {
	{"made", "MADE engine game"},
	{"manhole", "The Manhole"},
	{"rtz", "Return to Zork"},
	{"lgop2", "Leather Goddesses of Phobos 2"},
	{"rodney", "Rodney's Funscreen"},
	{0, 0}
};


namespace Made {

static const MadeGameDescription gameDescriptions[] = {

	{
		// NOTE: Return to Zork entries with *.dat are used to detect the game via rtzcd.dat,
		// which is packed inside rtzcd.red. Entries with *.red refer to the packed file
		// directly, which is the "official" way.

		// Return to Zork - English CD version 1.0 9/15/93
		// Patch #1953654 submitted by spookypeanut
		{
			"rtz",
			"CD",
			AD_ENTRY1("rtzcd.dat", "e95c38ded389e39cfbf87a8cb250b12e"),
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GID_RTZ,
		0,
		GF_CD,
		0,
	},

	{
		// Return to Zork - English CD version 1.0 9/15/93
		// Patch #1953654 submitted by spookypeanut
		{
			"rtz",
			"CD",
			AD_ENTRY1("rtzcd.red", "cd8b62ece4677c438688c1de3f5379b9"),
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GID_RTZ,
		0,
		GF_CD_COMPRESSED,
		0,
	},

	{
		// Return to Zork - English CD version 1.1 12/7/93
		{
			"rtz",
			"CD",
			AD_ENTRY1("rtzcd.dat", "a1db8c97a78dae10f91d356f16ad07b8"),
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GID_RTZ,
		0,
		GF_CD,
		0,
	},

	{
		// Return to Zork - English CD version 1.1 12/7/93
		{
			"rtz",
			"CD",
			AD_ENTRY1("rtzcd.red", "c4e2430e6b6c6ff1562a80fb4a9df24c"),
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GID_RTZ,
		0,
		GF_CD_COMPRESSED,
		0,
	},

	{
		// Return to Zork - English CD version 1.2 9/29/94
		// Supplied by Dark-Star in the ScummVM forums
		{
			"rtz",
			"CD",
			AD_ENTRY1("rtzcd.dat", "9d740378da2d16e83d0d0efff01bf83a"),
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GID_RTZ,
		0,
		GF_CD,
		0,
	},

	{
		// Return to Zork - English CD version 1.2 9/29/94
		{
			"rtz",
			"CD",
			AD_ENTRY1("rtzcd.red", "946997d8b0aa6cb4e848bad02a1fc3d2"),
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GID_RTZ,
		0,
		GF_CD_COMPRESSED,
		0,
	},

	{
		// Return to Zork - German CD version 1.2 9/29/94
		// (same as the English 1.2 version)
		// Supplied by Dark-Star in the ScummVM forums
		{
			"rtz",
			"CD",
			AD_ENTRY1("rtzcd.dat", "9d740378da2d16e83d0d0efff01bf83a"),
			Common::DE_DEU,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GID_RTZ,
		0,
		GF_CD,
		0,
	},

	{
		// Return to Zork - German CD version 1.2 4/18/95
		// (same as the English 1.2 version)
		// Supplied by Dark-Star in the ScummVM forums
		{
			"rtz",
			"CD",
			AD_ENTRY1("rtzcd.red", "946997d8b0aa6cb4e848bad02a1fc3d2"),
			Common::DE_DEU,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GID_RTZ,
		0,
		GF_CD_COMPRESSED,
		0,
	},

	{
		// Return to Zork - English floppy version
		{
			"rtz",
			"Floppy",
			AD_ENTRY1("rtz.prj", "764d02f52ce1c219f2c0066677fba4ce"),
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GID_RTZ,
		0,
		GF_FLOPPY,
		0,
	},

	{
		// Return to Zork - Demo
		{
			"rtz",
			"Demo",
			AD_ENTRY1("demo.dat", "2a6a1354bd5346fad4aee08e5b56caaa"),
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_DEMO
		},
		GID_RTZ,
		0,
		GF_DEMO,
		0,
	},
	
	{
		// The Manhole: New and Enhanced
		{
			"manhole",
			"",
			AD_ENTRY1("manhole.dat", "cb21e31ed35c963208343bc995225b73"),
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GID_MANHOLE,
		0,
		GF_CD,
		0,
	},

	{
		// Leather Goddesses of Phobos 2
		{
			"lgop2",
			"",
			AD_ENTRY1("lgop2.dat", "8137996db200ff67e8f172ff106f2e48"),
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GID_LGOP2,
		0,
		GF_FLOPPY,
		0,
	},

	{
		// Rodney's Funscreen
		{
			"rodney",
			"",
			AD_ENTRY1("rodneys.dat", "a79887dbaa47689facd7c6f09258ba5a"),
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GID_RODNEY,
		0,
		GF_FLOPPY,
		0,
	},

	{ AD_TABLE_END_MARKER, 0, 0, 0, 0 }
};

/**
 * The fallback game descriptor used by the Made engine's fallbackDetector.
 * Contents of this struct are to be overwritten by the fallbackDetector.
 */
static MadeGameDescription g_fallbackDesc = {
	{
		"",
		"",
		AD_ENTRY1(0, 0), // This should always be AD_ENTRY1(0, 0) in the fallback descriptor
		Common::UNK_LANG,
		Common::kPlatformPC,
		Common::ADGF_NO_FLAGS
	},
	0,
	0,
	0,
	0,
};

} // End of namespace Made

static const Common::ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)Made::gameDescriptions,
	// Size of that superset structure
	sizeof(Made::MadeGameDescription),
	// Number of bytes to compute MD5 sum for
	5000,
	// List of all engine targets
	madeGames,
	// Structure for autoupgrading obsolete targets
	0,
	// Name of single gameid (optional)
	"made",
	// List of files for file-based fallback detection (optional)
	0,
	// Flags
	0
};

class MadeMetaEngine : public Common::AdvancedMetaEngine {
public:
	MadeMetaEngine() : Common::AdvancedMetaEngine(detectionParams) {}

	virtual const char *getName() const {
		return "MADE Engine";
	}

	virtual const char *getCopyright() const {
		return "MADE Engine (C) Activision";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const Common::ADGameDescription *desc) const;

	const Common::ADGameDescription *fallbackDetect(const FSList *fslist) const;

};

bool MadeMetaEngine::createInstance(OSystem *syst, Engine **engine, const Common::ADGameDescription *desc) const {
	const Made::MadeGameDescription *gd = (const Made::MadeGameDescription *)desc;
	if (gd) {
		*engine = new Made::MadeEngine(syst, gd);
	}
	return gd != 0;
}

const Common::ADGameDescription *MadeMetaEngine::fallbackDetect(const FSList *fslist) const {
	// Set the default values for the fallback descriptor's ADGameDescription part.
	Made::g_fallbackDesc.desc.language = Common::UNK_LANG;
	Made::g_fallbackDesc.desc.platform = Common::kPlatformPC;
	Made::g_fallbackDesc.desc.flags = Common::ADGF_NO_FLAGS;

	// Set default values for the fallback descriptor's MadeGameDescription part.
	Made::g_fallbackDesc.gameID = 0;
	Made::g_fallbackDesc.features = 0;
	Made::g_fallbackDesc.version = 0;

	//return (const Common::ADGameDescription *)&Made::g_fallbackDesc;
	return NULL;
}

#if PLUGIN_ENABLED_DYNAMIC(MADE)
	REGISTER_PLUGIN_DYNAMIC(MADE, PLUGIN_TYPE_ENGINE, MadeMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(MADE, PLUGIN_TYPE_ENGINE, MadeMetaEngine);
#endif
