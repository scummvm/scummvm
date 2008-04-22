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
	{"rtz", "Return to Zork"},
	{0, 0}
};


namespace Made {

static const MadeGameDescription gameDescriptions[] = {

	{
		// Return to Zork - English CD version
		{
			"rtz",
			"CD",
			AD_ENTRY1("rtzcd.dat", "a1db8c97a78dae10f91d356f16ad07b8"),
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		0,
		0,
		0,
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
		0,
		0,
		GF_DEMO,
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
		return "MADE Engine";
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

	return (const Common::ADGameDescription *)&Made::g_fallbackDesc;
}

REGISTER_PLUGIN(MADE, PLUGIN_TYPE_ENGINE, MadeMetaEngine);
