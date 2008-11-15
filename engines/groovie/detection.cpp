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

#include "common/savefile.h"

#include "groovie/groovie.h"

namespace Groovie {

//#define GROOVIE_EXPERIMENTAL

static const PlainGameDescriptor groovieGames[] = {
	// Games
	{"t7g", "The 7th Guest"},

#ifdef GROOVIE_EXPERIMENTAL
	{"11h", "The 11th Hour: The sequel to The 7th Guest"},
	{"clandestiny", "Clandestiny"},
	{"unclehenry", "Uncle Henry's Playhouse"},
	{"tlc", "Tender Loving Care"},

	// Extras
	{"making11h", "The Making of The 11th Hour"},
	{"clantrailer", "Clandestiny Trailer"},
#endif

	// Unknown
	{"groovie", "Groovie engine game"},
	{0, 0}
};

static const GroovieGameDescription gameDescriptions[] = {

	// The 7th Guest DOS English
	{ 
		{
			"t7g", "",
			AD_ENTRY1s("script.grv", "d1b8033b40aa67c076039881eccce90d", 16659),
			Common::EN_ANY, Common::kPlatformPC, Common::ADGF_NO_FLAGS
		},
		kGroovieT7G, 0
	},

	// The 7th Guest Mac English
	{
		{
			"t7g", "",
			AD_ENTRY1s("script.grv", "6e30b54b1f3bc2262cdcf7961db2ae67", 17191),
			Common::EN_ANY, Common::kPlatformMacintosh, Common::ADGF_NO_FLAGS
		},
		kGroovieT7G, 0
	},

#ifdef GROOVIE_EXPERIMENTAL
	// The 11th Hour DOS English
	{ 
		{
			"11h", "",
			AD_ENTRY1s("disk.1", "5c0428cd3659fc7bbcd0aa16485ed5da", 227),
			Common::EN_ANY, Common::kPlatformPC, Common::ADGF_NO_FLAGS
		},
		kGroovieV2, 1
	},

	// The Making of The 11th Hour DOS English
	{ 
		{
			"making11h", "",
			AD_ENTRY1s("disk.1", "5c0428cd3659fc7bbcd0aa16485ed5da", 227),
			Common::EN_ANY, Common::kPlatformPC, Common::ADGF_NO_FLAGS
		},
		kGroovieV2, 2
	},

	// Clandestiny Trailer DOS English
	{ 
		{
			"clantrailer", "",
			AD_ENTRY1s("disk.1", "5c0428cd3659fc7bbcd0aa16485ed5da", 227),
			Common::EN_ANY, Common::kPlatformPC, Common::ADGF_NO_FLAGS
		},
		kGroovieV2, 3
	},

	// Clandestiny DOS English
	{ 
		{
			"clandestiny", "",
			AD_ENTRY1s("disk.1", "f79fc1515174540fef6a34132efc4c53", 76),
			Common::EN_ANY, Common::kPlatformPC, Common::ADGF_NO_FLAGS
		},
		kGroovieV2, 1
	},

	// Uncle Henry's Playhouse PC English
	{ 
		{
			"unclehenry", "",
			AD_ENTRY1s("disk.1", "0e1b1d3cecc4fc7efa62a968844d1f7a", 72),
			Common::EN_ANY, Common::kPlatformPC, Common::ADGF_NO_FLAGS
		},
		kGroovieV2, 1
	},

	// Tender Loving Care PC English
	{ 
		{
			"tlc", "",
			AD_ENTRY1s("disk.1", "32a1afa68478f1f9d2b25eeea427f2e3", 84),
			Common::EN_ANY, Common::kPlatformPC, Common::ADGF_NO_FLAGS
		},
		kGroovieV2, 1
	},
#endif

	{AD_TABLE_END_MARKER, kGroovieT7G, 0}
};

static const Common::ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)gameDescriptions,
	// Size of that superset structure
	sizeof(GroovieGameDescription),
	// Number of bytes to compute MD5 sum for
	5000,
	// List of all engine targets
	groovieGames,
	// Structure for autoupgrading obsolete targets
	0,
	// Name of single gameid (optional)
	"groovie",
	// List of files for file-based fallback detection (optional)
	0,
	// Flags
	0
};


class GroovieMetaEngine : public Common::AdvancedMetaEngine {
public:
	GroovieMetaEngine() : Common::AdvancedMetaEngine(detectionParams) {}

	const char *getName() const {
		return "Groovie Engine";
	}

	const char *getCopyright() const {
		return "Groovie Engine (C) 1990-1996 Trilobyte";
	}

	bool createInstance(OSystem *syst, Engine **engine, const Common::ADGameDescription *gd) const;

	bool hasFeature(MetaEngineFeature f) const;
	SaveStateList listSaves(const char *target) const;
	int getMaximumSaveSlot() const;
	void removeSaveState(const char *target, int slot) const;
};

bool GroovieMetaEngine::createInstance(OSystem *syst, Engine **engine, const Common::ADGameDescription *gd) const {
	if (gd) {
		*engine = new GroovieEngine(syst, (GroovieGameDescription *)gd);
	}
	return gd != 0;
}

bool GroovieMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave);
		//(f == kSavesSupportCreationDate)
}

SaveStateList GroovieMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *sfm = g_system->getSavefileManager();
	SaveStateList list;

	// Get the list of savefiles
	Common::String pattern = Common::String(target) + ".00?";
	Common::StringList savefiles = sfm->listSavefiles(pattern.c_str());

	// Sort the list of filenames
	sort(savefiles.begin(), savefiles.end());

	// Fill the information for the existing savegames
	Common::StringList::iterator it = savefiles.begin();
	while (it != savefiles.end()) {
		int slot = it->lastChar() - '0';
		if (slot >= 0 && slot <= 9) {
			Common::InSaveFile *file = sfm->openForLoading(it->c_str());

			// Read the savegame description
			Common::String description;
			unsigned char c = 1;
			for (int i = 0; (c != 0) && (i < 15); i++) {
				c = file->readByte();
				switch (c) {
				case 0:
					break;
				case 16: // @
					c = ' ';
					break;
				case 244: // $
					c = 0;
					break;
				default:
					c += 0x30;
				}
				if (c != 0) {
					description += c;
				}
			}
			delete file;

			list.push_back(SaveStateDescriptor(slot, description));
		}
		it++;
	}

	return list;
}

int GroovieMetaEngine::getMaximumSaveSlot() const {
	return 9;
}

void GroovieMetaEngine::removeSaveState(const char *target, int slot) const {
	if (slot < 0 || slot > 9) {
		// Invalid slot, do nothing
		return;
	}

	char extension[6];
	snprintf(extension, sizeof(extension), ".00%01d", slot);

	Common::String filename = target;
	filename += extension;

	g_system->getSavefileManager()->removeSavefile(filename.c_str());
}

} // End of namespace Groovie

#if PLUGIN_ENABLED_DYNAMIC(GROOVIE)
	REGISTER_PLUGIN_DYNAMIC(GROOVIE, PLUGIN_TYPE_ENGINE, Groovie::GroovieMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(GROOVIE, PLUGIN_TYPE_ENGINE, Groovie::GroovieMetaEngine);
#endif
