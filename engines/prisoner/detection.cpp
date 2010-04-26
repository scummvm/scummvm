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

#include "engines/advancedDetector.h"
#include "common/file.h"

#include "prisoner/prisoner.h"


namespace Prisoner {

struct PrisonerGameDescription {
	ADGameDescription desc;

	int gameID;
	int gameType;
	uint32 features;
	uint16 version;
};

uint32 PrisonerEngine::getGameID() const {
	return _gameDescription->gameID;
}

uint32 PrisonerEngine::getFeatures() const {
	return _gameDescription->features;
}

Common::Platform PrisonerEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

uint16 PrisonerEngine::getVersion() const {
	return _gameDescription->version;
}

}

static const PlainGameDescriptor prisonerGames[] = {
	{"prisoner", "Prisoner of Ice"},
	{0, 0}
};


namespace Prisoner {

using Common::GUIO_NONE;
using Common::GUIO_NOSPEECH;

static const PrisonerGameDescription gameDescriptions[] = {

	{
		// Prisoner English version
		{
			"prisoner",
			0,
			AD_ENTRY1s("e_klang.bin", "898cc3ba5b382cf9eed2918bea56ac64", 561),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		0,
		0,
		0,
		0,
	},

	{ AD_TABLE_END_MARKER, 0, 0, 0, 0 }
};

/**
 * The fallback game descriptor used by the Prisoner engine's fallbackDetector.
 * Contents of this struct are to be overwritten by the fallbackDetector.
 */
static PrisonerGameDescription g_fallbackDesc = {
	{
		"",
		"",
		AD_ENTRY1(0, 0), // This should always be AD_ENTRY1(0, 0) in the fallback descriptor
		Common::UNK_LANG,
		Common::kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO_NONE
	},
	0,
	0,
	0,
	0,
};

} // End of namespace Prisoner

static const ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)Prisoner::gameDescriptions,
	// Size of that superset structure
	sizeof(Prisoner::PrisonerGameDescription),
	// Number of bytes to compute MD5 sum for
	5000,
	// List of all engine targets
	prisonerGames,
	// Structure for autoupgrading obsolete targets
	0,
	// Name of single gameid (optional)
	"prisoner",
	// List of files for file-based fallback detection (optional)
	0,
	// Flags
	0,
	// Additional GUI options (for every game}
	Common::GUIO_NONE
};

class PrisonerMetaEngine : public AdvancedMetaEngine {
public:
	PrisonerMetaEngine() : AdvancedMetaEngine(detectionParams) {}

	virtual const char *getName() const {
		return "Prisoner Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Prisoner Engine (C) Infogrames";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;

	const ADGameDescription *fallbackDetect(const Common::FSList &fslist) const;

};

bool PrisonerMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		false;
}

bool Prisoner::PrisonerEngine::hasFeature(EngineFeature f) const {
	return
		false;
}

bool PrisonerMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Prisoner::PrisonerGameDescription *gd = (const Prisoner::PrisonerGameDescription *)desc;
	if (gd) {
		*engine = new Prisoner::PrisonerEngine(syst, gd);
	}
	return gd != 0;
}

const ADGameDescription *PrisonerMetaEngine::fallbackDetect(const Common::FSList &fslist) const {
	// Set the default values for the fallback descriptor's ADGameDescription part.
	Prisoner::g_fallbackDesc.desc.language = Common::UNK_LANG;
	Prisoner::g_fallbackDesc.desc.platform = Common::kPlatformPC;
	Prisoner::g_fallbackDesc.desc.flags = ADGF_NO_FLAGS;

	// Set default values for the fallback descriptor's PrisonerGameDescription part.
	Prisoner::g_fallbackDesc.gameID = 0;
	Prisoner::g_fallbackDesc.features = 0;
	Prisoner::g_fallbackDesc.version = 3;

	return NULL;
}

#if PLUGIN_ENABLED_DYNAMIC(PRISONER)
	REGISTER_PLUGIN_DYNAMIC(PRISONER, PLUGIN_TYPE_ENGINE, PrisonerMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(PRISONER, PLUGIN_TYPE_ENGINE, PrisonerMetaEngine);
#endif
