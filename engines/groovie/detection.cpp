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
#include "groovie/saveload.h"

namespace Groovie {

static const PlainGameDescriptor groovieGames[] = {
	// Games
	{"t7g", "The 7th Guest"},

#ifdef ENABLE_GROOVIE2
	{"11h", "The 11th Hour: The sequel to The 7th Guest"},
	{"clandestiny", "Clandestiny"},
	{"unclehenry", "Uncle Henry's Playhouse"},
	{"tlc", "Tender Loving Care"},
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
			Common::EN_ANY, Common::kPlatformPC, ADGF_NO_FLAGS, Common::GUIO_NONE
		},
		kGroovieT7G, 0
	},

	// The 7th Guest Mac English
	{
		{
			"t7g", "",
			AD_ENTRY1s("script.grv", "6e30b54b1f3bc2262cdcf7961db2ae67", 17191),
			Common::EN_ANY, Common::kPlatformMacintosh, ADGF_NO_FLAGS, Common::GUIO_NONE
		},
		kGroovieT7G, 0
	},

	// The 7th Guest DOS Russian (Akella)
	{
		{
			"t7g", "",
			{
				{ "script.grv", 0, "d1b8033b40aa67c076039881eccce90d", 16659},
				{ "intro.gjd", 0, NULL, 31711554},
				{ NULL, 0, NULL, 0}
			},
			Common::RU_RUS, Common::kPlatformPC, ADGF_NO_FLAGS, Common::GUIO_NONE
		},
		kGroovieT7G, 0
	},

#ifdef ENABLE_GROOVIE2
	// The 11th Hour DOS English
	{
		{
			"11h", "",
			AD_ENTRY1s("disk.1", "5c0428cd3659fc7bbcd0aa16485ed5da", 227),
			Common::EN_ANY, Common::kPlatformPC, ADGF_NO_FLAGS, Common::GUIO_NONE
		},
		kGroovieV2, 1
	},

	// The 11th Hour DOS Demo English
	{
		{
			"11h", "Demo",
			AD_ENTRY1s("disk.1", "aacb32ce07e0df2894bd83a3dee40c12", 70),
			Common::EN_ANY, Common::kPlatformPC, ADGF_DEMO, Common::GUIO_NOLAUNCHLOAD
		},
		kGroovieV2, 1
	},

	// The Making of The 11th Hour DOS English
	{
		{
			"11h", "Making Of",
			AD_ENTRY1s("disk.1", "5c0428cd3659fc7bbcd0aa16485ed5da", 227),
			Common::EN_ANY, Common::kPlatformPC, ADGF_NO_FLAGS, Common::GUIO_NOMIDI | Common::GUIO_NOLAUNCHLOAD
		},
		kGroovieV2, 2
	},

	// Clandestiny Trailer DOS English
	{
		{
			"clandestiny", "Trailer",
			AD_ENTRY1s("disk.1", "5c0428cd3659fc7bbcd0aa16485ed5da", 227),
			Common::EN_ANY, Common::kPlatformPC, ADGF_NO_FLAGS, Common::GUIO_NOMIDI | Common::GUIO_NOLAUNCHLOAD
		},
		kGroovieV2, 3
	},

	// Clandestiny DOS English
	{
		{
			"clandestiny", "",
			AD_ENTRY1s("disk.1", "f79fc1515174540fef6a34132efc4c53", 76),
			Common::EN_ANY, Common::kPlatformPC, ADGF_NO_FLAGS, Common::GUIO_NOMIDI
		},
		kGroovieV2, 1
	},

	// Uncle Henry's Playhouse PC English
	{
		{
			"unclehenry", "",
			AD_ENTRY1s("disk.1", "0e1b1d3cecc4fc7efa62a968844d1f7a", 72),
			Common::EN_ANY, Common::kPlatformPC, ADGF_NO_FLAGS, Common::GUIO_NOMIDI
		},
		kGroovieV2, 1
	},

	// Tender Loving Care PC English
	{
		{
			"tlc", "",
			AD_ENTRY1s("disk.1", "32a1afa68478f1f9d2b25eeea427f2e3", 84),
			Common::EN_ANY, Common::kPlatformPC, ADGF_NO_FLAGS, Common::GUIO_NOMIDI
		},
		kGroovieV2, 1
	},
#endif

	{AD_TABLE_END_MARKER, kGroovieT7G, 0}
};

static const ADParams detectionParams = {
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
	kADFlagUseExtraAsHint,
	// Additional GUI options (for every game}
	Common::GUIO_NOSUBTITLES | Common::GUIO_NOSFX
};


class GroovieMetaEngine : public AdvancedMetaEngine {
public:
	GroovieMetaEngine() : AdvancedMetaEngine(detectionParams) {}

	const char *getName() const {
		return "Groovie Engine";
	}

	const char *getOriginalCopyright() const {
		return "Groovie Engine (C) 1990-1996 Trilobyte";
	}

	bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const;

	bool hasFeature(MetaEngineFeature f) const;
	SaveStateList listSaves(const char *target) const;
	int getMaximumSaveSlot() const;
	void removeSaveState(const char *target, int slot) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
};

bool GroovieMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const {
	if (gd) {
		*engine = new GroovieEngine(syst, (const GroovieGameDescription *)gd);
	}
	return gd != 0;
}

bool GroovieMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo);
}

SaveStateList GroovieMetaEngine::listSaves(const char *target) const {
	return SaveLoad::listValidSaves(target);
}

int GroovieMetaEngine::getMaximumSaveSlot() const {
	return SaveLoad::getMaximumSlot();
}

void GroovieMetaEngine::removeSaveState(const char *target, int slot) const {
	if (!SaveLoad::isSlotValid(slot)) {
		// Invalid slot, do nothing
		return;
	}

	Common::String filename = SaveLoad::getSlotSaveName(target, slot);
	g_system->getSavefileManager()->removeSavefile(filename);
}

SaveStateDescriptor GroovieMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	SaveStateDescriptor desc;

	Common::InSaveFile *savefile = SaveLoad::openForLoading(target, slot, &desc);
	if (savefile) {
		// Loaded correctly
		delete savefile;
	}

	return desc;
}

} // End of namespace Groovie

#if PLUGIN_ENABLED_DYNAMIC(GROOVIE)
	REGISTER_PLUGIN_DYNAMIC(GROOVIE, PLUGIN_TYPE_ENGINE, Groovie::GroovieMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(GROOVIE, PLUGIN_TYPE_ENGINE, Groovie::GroovieMetaEngine);
#endif
