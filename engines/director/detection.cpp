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
#include "common/savefile.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "director/director.h"

namespace Director {

struct DirectorGameDescription {
	ADGameDescription desc;

	DirectorGameID gameID;
	uint16 version;
};

DirectorGameID DirectorEngine::getGameID() const {
	return _gameDescription->gameID;
}

Common::Platform DirectorEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

uint16 DirectorEngine::getVersion() const {
	return _gameDescription->version;
}

Common::Language DirectorEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

Common::String DirectorEngine::getEXEName() const {
	return _gameDescription->desc.filesDescriptions[0].fileName;
}

bool DirectorEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL);
}

} // End of Namespace Director

static const PlainGameDescriptor directorGames[] = {
	{ "director",   "Macromedia Director Game" },
	{ "gundam0079", "Gundam 0079: The War for Earth" },
	{ "jewels",     "Jewels of the Oracle" },
	{ "jman",       "The Journeyman Project" },
	{ "majestic",   "Majestic Part I: Alien Encounter" },
	{ "spyclub",    "Spy Club" },
	{ 0, 0 }
};

#include "director/detection_tables.h"

class DirectorMetaEngine : public AdvancedMetaEngine {
public:
	DirectorMetaEngine() : AdvancedMetaEngine(Director::gameDescriptions, sizeof(Director::DirectorGameDescription), directorGames) {
		_singleid = "director";
	}

	virtual const char *getName() const {
		return "Macromedia Director";
	}

	virtual const char *getOriginalCopyright() const {
		return "Macromedia Director (C) Macromedia";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool DirectorMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Director::DirectorGameDescription *gd = (const Director::DirectorGameDescription *)desc;

	if (gd)
		*engine = new Director::DirectorEngine(syst, gd);

	return (gd != 0);
}

#if PLUGIN_ENABLED_DYNAMIC(DIRECTOR)
	REGISTER_PLUGIN_DYNAMIC(DIRECTOR, PLUGIN_TYPE_ENGINE, DirectorMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(DIRECTOR, PLUGIN_TYPE_ENGINE, DirectorMetaEngine);
#endif
