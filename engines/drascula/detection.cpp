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

#include "drascula/drascula.h"


namespace Drascula {

struct DrasculaGameDescription {
	Common::ADGameDescription desc;

	int gameID;
	int gameType;
	uint32 features;
	uint16 version;
};

uint32 DrasculaEngine::getGameID() const {
	return _gameDescription->gameID;
}

uint32 DrasculaEngine::getFeatures() const {
	return _gameDescription->features;
}

Common::Platform DrasculaEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

uint16 DrasculaEngine::getVersion() const {
	return _gameDescription->version;
}

}

static const PlainGameDescriptor drasculaGames[] = {
	{"drascula", "Drascula: The Vampire Strikes Back"},
	{0, 0}
};


namespace Drascula {

static const DrasculaGameDescription gameDescriptions[] = {

	{
		// Drascula English version
		{
			"drascula",
			0,
			AD_ENTRY1("14.ald", "09b2735953edcd43af115c65ae00b10e"),
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		0,
		0,
		0,
		0,
	},
/*
	{
		// Drascula Spanish version
		{
			"drascula",
			0,
			AD_ENTRY1("14.ald", "0746ed1a5cc8d9728f790c29813f4b43"),
			Common::ES_ESP,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		0,
		0,
		0,
		0,
	},*/

	{ AD_TABLE_END_MARKER, 0, 0, 0, 0 }
};

/**
 * The fallback game descriptor used by the Drascula engine's fallbackDetector.
 * Contents of this struct are to be overwritten by the fallbackDetector.
 */
static DrasculaGameDescription g_fallbackDesc = {
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

} // End of namespace Drascula

static const Common::ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)Drascula::gameDescriptions,
	// Size of that superset structure
	sizeof(Drascula::DrasculaGameDescription),
	// Number of bytes to compute MD5 sum for
	5000,
	// List of all engine targets
	drasculaGames,
	// Structure for autoupgrading obsolete targets
	0,
	// Name of single gameid (optional)
	"drascula",
	// List of files for file-based fallback detection (optional)
	0,
	// Flags
	Common::kADFlagAugmentPreferredTarget
};

class DrasculaMetaEngine : public Common::AdvancedMetaEngine {
public:
	DrasculaMetaEngine() : Common::AdvancedMetaEngine(detectionParams) {}

	virtual const char *getName() const {
		return "Drascula Engine";
	}

	virtual const char *getCopyright() const {
		return "Drascula Engine (C) 2000 Alcachofa Soft, 1996 (C) Digital Dreams Multimedia, 1994 (C) Emilio de Paz";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const Common::ADGameDescription *desc) const;

	const Common::ADGameDescription *fallbackDetect(const FSList *fslist) const;

};

bool DrasculaMetaEngine::createInstance(OSystem *syst, Engine **engine, const Common::ADGameDescription *desc) const {
	const Drascula::DrasculaGameDescription *gd = (const Drascula::DrasculaGameDescription *)desc;
	if (gd) {
		*engine = new Drascula::DrasculaEngine(syst, gd);
	}
	return gd != 0;
}

const Common::ADGameDescription *DrasculaMetaEngine::fallbackDetect(const FSList *fslist) const {
	// Set the default values for the fallback descriptor's ADGameDescription part.
	Drascula::g_fallbackDesc.desc.language = Common::UNK_LANG;
	Drascula::g_fallbackDesc.desc.platform = Common::kPlatformPC;
	Drascula::g_fallbackDesc.desc.flags = Common::ADGF_NO_FLAGS;

	// Set default values for the fallback descriptor's DrasculaGameDescription part.
	Drascula::g_fallbackDesc.gameID = 0;
	Drascula::g_fallbackDesc.features = 0;
	Drascula::g_fallbackDesc.version = 0;

	return (const Common::ADGameDescription *)&Drascula::g_fallbackDesc;
}

REGISTER_PLUGIN(DRASCULA, PLUGIN_TYPE_ENGINE, DrasculaMetaEngine);
