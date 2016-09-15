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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "base/plugins.h"

#include "engines/advancedDetector.h"

#include "common/config-manager.h"
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
	if (ConfMan.hasKey("start_movie"))
		return ConfMan.get("start_movie");

	return _gameDescription->desc.filesDescriptions[0].fileName;
}

bool DirectorEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL);
}

} // End of Namespace Director

static const PlainGameDescriptor directorGames[] = {
	{ "director",	"Macromedia Director Game" },
	{ "directortest",	"Macromedia Director Test Target" },
	{ "theapartment",	"The Apartment, Interactive demo" },
	{ "gundam0079",	"Gundam 0079: The War for Earth" },
	{ "jewels",		"Jewels of the Oracle" },
	{ "jman",		"The Journeyman Project" },
	{ "majestic",	"Majestic Part I: Alien Encounter" },
	{ "mediaband",	"Meet Mediaband" },
	{ "melements",	"Masters of the Elements" },
	{ "spyclub",	"Spy Club" },
	{ "amber",		"AMBER: Journeys Beyond"},
	{ "vvvampire",	"Victor Vector & Yondo: The Vampire's Coffin"},
	{ "vvdinosaur",	"Victor Vector & Yondo: The Last Dinosaur Egg"},
	{ "warlock", 	"Spaceship Warlock"},
	{ "ernie",		"Ernie"},
	{ 0, 0 }
};

#include "director/detection_tables.h"

static const char *directoryGlobs[] = {
	"install",
	0
};

class DirectorMetaEngine : public AdvancedMetaEngine {
public:
	DirectorMetaEngine() : AdvancedMetaEngine(Director::gameDescriptions, sizeof(Director::DirectorGameDescription), directorGames) {
		_singleId = "director";
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	virtual const char *getEngineId() const {
		return "director";
	}

	virtual const char *getName() const {
		return "Macromedia Director";
	}

	virtual const char *getOriginalCopyright() const {
		return "Macromedia Director (C) Macromedia";
	}

	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const override;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool DirectorMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Director::DirectorGameDescription *gd = (const Director::DirectorGameDescription *)desc;

	if (gd)
		*engine = new Director::DirectorEngine(syst, gd);

	return (gd != 0);
}

static Director::DirectorGameDescription s_fallbackDesc = {
	{
		"director",
		"",
		AD_ENTRY1(0, 0),
		Common::UNK_LANG,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	Director::GID_GENERIC,
	0
};

static char s_fallbackFileNameBuffer[51];

ADDetectedGame DirectorMetaEngine::fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const {
	// TODO: Handle Mac fallback

	// reset fallback description
	Director::DirectorGameDescription *desc = &s_fallbackDesc;
	desc->desc.gameId = "director";
	desc->desc.extra = "";
	desc->desc.language = Common::UNK_LANG;
	desc->desc.flags = ADGF_NO_FLAGS;
	desc->desc.platform = Common::kPlatformWindows;
	desc->desc.guiOptions = GUIO0();
	desc->desc.filesDescriptions[0].fileName = 0;
	desc->version = 0;
	desc->gameID = Director::GID_GENERIC;

	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory())
			continue;

		Common::String fileName = file->getName();
		fileName.toLowercase();
		if (!fileName.hasSuffix(".exe"))
			continue;

		Common::File f;
		if (!f.open(*file))
			continue;

		f.seek(-4, SEEK_END);

		uint32 offset = f.readUint32LE();

		if (f.eos() || offset == 0 || offset >= (uint32)(f.size() - 4))
			continue;

		f.seek(offset);

		uint32 tag = f.readUint32LE();

		switch (tag) {
		case MKTAG('3', '9', 'J', 'P'):
			desc->version = 4;
			break;
		case MKTAG('P', 'J', '9', '5'):
			desc->version = 5;
			break;
		case MKTAG('P', 'J', '0', '0'):
			desc->version = 7;
			break;
		default:
			// Prior to version 4, there was no tag here. So we'll use a bit of a
			// heuristic to detect. The first field is the entry count, of which
			// there should only be one.
			if ((tag & 0xFFFF) != 1)
				continue;

			f.skip(3);

			uint32 mmmSize = f.readUint32LE();

			if (f.eos() || mmmSize == 0)
				continue;

			byte fileNameSize = f.readByte();

			if (f.eos())
				continue;

			f.skip(fileNameSize);
			byte directoryNameSize = f.readByte();

			if (f.eos())
				continue;

			f.skip(directoryNameSize);

			if (f.pos() != f.size() - 4)
				continue;

			// Assume v3 at this point (for now at least)
			desc->version = 3;
		}

		strncpy(s_fallbackFileNameBuffer, fileName.c_str(), 50);
		s_fallbackFileNameBuffer[50] = '\0';
		desc->desc.filesDescriptions[0].fileName = s_fallbackFileNameBuffer;

		warning("Director fallback detection D%d", desc->version);

		return ADDetectedGame(&desc->desc);
	}

	return ADDetectedGame();
}

#if PLUGIN_ENABLED_DYNAMIC(DIRECTOR)
	REGISTER_PLUGIN_DYNAMIC(DIRECTOR, PLUGIN_TYPE_ENGINE, DirectorMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(DIRECTOR, PLUGIN_TYPE_ENGINE, DirectorMetaEngine);
#endif
