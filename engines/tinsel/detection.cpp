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

#include "tinsel/tinsel.h"
#include "tinsel/savescn.h"	// needed by TinselMetaEngine::listSaves


namespace Tinsel {

struct TinselGameDescription {
	Common::ADGameDescription desc;

	int gameID;
	int gameType;
	uint32 features;
	uint16 version;
};

uint32 TinselEngine::getGameID() const {
	return _gameDescription->gameID;
}

uint32 TinselEngine::getFeatures() const {
	return _gameDescription->features;
}

Common::Language TinselEngine::getLanguage() const { 
	return _gameDescription->desc.language; 
}

Common::Platform TinselEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

uint16 TinselEngine::getVersion() const {
	return _gameDescription->version;
}

}

static const PlainGameDescriptor tinselGames[] = {
	{"tinsel", "Tinsel engine game"},
	{"dw", "Discworld"},
	{"dw2", "Discworld 2: Mortality Bytes!"},
	{0, 0}
};


namespace Tinsel {

static const TinselGameDescription gameDescriptions[] = {

	// The DW1 demo was based on an older revision of the Tinsel engine
	// than the one used in the released game. We call it Tinsel v0 as 
	// opposed to v1 which was used in the full retail version of DW.

	{	// Demo from http://www.adventure-treff.de/specials/dl_demos.php
		{
			"dw",
			"Demo",
			AD_ENTRY1s("dw.gra", "ce1b57761ba705221bcf70955b827b97", 441192),
			//AD_ENTRY1s("dw.scn", "ccd72f02183d0e96b6e7d8df9492cda8", 23308),
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_DEMO
		},
		GID_DW1,
		0,
		GF_DEMO,
		TINSEL_V0,
	},

	{
		{	// This version has *.gra files
			"dw",
			"Floppy",
			AD_ENTRY1s("dw.gra", "c8808ccd988d603dd35dff42013ae7fd", 781656),
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GID_DW1,
		0,
		GF_FLOPPY,
		TINSEL_V1,
	},

	{	// Multilingual floppy with *.gra files.
		// Note: It contains no english subtitles.
		// Reported on our forums.
		{
			"dw",
			"Floppy",
			{
				{"dw.gra", 0, "c8808ccd988d603dd35dff42013ae7fd", 781656},
				{"french.txt", 0, NULL, -1},
				{"german.txt", 0, NULL, -1},
				{"italian.txt", 0, NULL, -1},
				{"spanish.txt", 0, NULL, -1},
				{NULL, 0, NULL, 0}
			},
			Common::FR_FRA,
			Common::kPlatformPC,
			Common::ADGF_DROPLANGUAGE
		},
		GID_DW1,
		0,
		GF_FLOPPY | GF_USE_4FLAGS,
		TINSEL_V1,
	},

	{	// English CD. This version has *.gra files
		{
			"dw",
			"CD",
			{
				{"dw.gra", 0, "c8808ccd988d603dd35dff42013ae7fd", 781656},
				{"english.smp", 0, NULL, -1},
				{NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GID_DW1,
		0,
		GF_CD,
		TINSEL_V1,
	},

	{	// Multilingual CD with english speech and *.gra files.
		// Note: It contains no english subtitles.
		{
			"dw",
			"CD",
			{
				{"dw.gra", 0, "c8808ccd988d603dd35dff42013ae7fd", 781656},
				{"english.smp", 0, NULL, -1},
				{"french.txt", 0, NULL, -1},
				{"german.txt", 0, NULL, -1},
				{"italian.txt", 0, NULL, -1},
				{"spanish.txt", 0, NULL, -1},
				{NULL, 0, NULL, 0}
			},
			Common::FR_FRA,
			Common::kPlatformPC,
			Common::ADGF_DROPLANGUAGE
		},
		GID_DW1,
		0,
		GF_CD | GF_USE_4FLAGS,
		TINSEL_V1,
	},
	{
		{
			"dw",
			"CD",
			{
				{"dw.gra", 0, "c8808ccd988d603dd35dff42013ae7fd", 781656},
				{"english.smp", 0, NULL, -1},
				{"french.txt", 0, NULL, -1},
				{"german.txt", 0, NULL, -1},
				{"italian.txt", 0, NULL, -1},
				{"spanish.txt", 0, NULL, -1},
				{NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			Common::ADGF_DROPLANGUAGE
		},
		GID_DW1,
		0,
		GF_CD | GF_USE_4FLAGS,
		TINSEL_V1,
	},
	{
		{
			"dw",
			"CD",
			{
				{"dw.gra", 0, "c8808ccd988d603dd35dff42013ae7fd", 781656},
				{"english.smp", 0, NULL, -1},
				{"french.txt", 0, NULL, -1},
				{"german.txt", 0, NULL, -1},
				{"italian.txt", 0, NULL, -1},
				{"spanish.txt", 0, NULL, -1},
				{NULL, 0, NULL, 0}
			},
			Common::IT_ITA,
			Common::kPlatformPC,
			Common::ADGF_DROPLANGUAGE
		},
		GID_DW1,
		0,
		GF_CD | GF_USE_4FLAGS,
		TINSEL_V1,
	},
	{
		{
			"dw",
			"CD",
			{
				{"dw.gra", 0, "c8808ccd988d603dd35dff42013ae7fd", 781656},
				{"english.smp", 0, NULL, -1},
				{"french.txt", 0, NULL, -1},
				{"german.txt", 0, NULL, -1},
				{"italian.txt", 0, NULL, -1},
				{"spanish.txt", 0, NULL, -1},
				{NULL, 0, NULL, 0}
			},
			Common::ES_ESP,
			Common::kPlatformPC,
			Common::ADGF_DROPLANGUAGE
		},
		GID_DW1,
		0,
		GF_CD | GF_USE_4FLAGS,
		TINSEL_V1,
	},

	{	// English CD with SCN files
		{
			"dw",
			"CD",
			{
				{"dw.scn", 0, "70955425870c7720d6eebed903b2ef41", 776188},
				{"english.smp", 0, NULL, -1},
				{NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GID_DW1,
		0,
		GF_CD | GF_SCNFILES,
		TINSEL_V1,
	},

#if 0
	{	// English Saturn CD. Not (yet?) supported
		{
			"dw",
			"CD",
			{
				{"dw.scn", 0, "6803f293c88758057cc685b9437f7637", 382248},
				{"english.smp", 0, NULL, -1},
				{NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GID_DW1,
		0,
		GF_CD,
		TINSEL_V1,
	},
#endif

	{	// German CD re-release "Neon Edition"
		// Note: This release has ENGLISH.TXT (with german content) instead of GERMAN.TXT
		{
			"dw",
			"CD",
			AD_ENTRY1s("dw.scn", "6182c7986eaec893c62fb6ea13a9f225", 774556),
			Common::DE_DEU,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GID_DW1,
		0,
		GF_CD | GF_SCNFILES,
		TINSEL_V1,
	},

	{ AD_TABLE_END_MARKER, 0, 0, 0, 0 }
};

} // End of namespace Tinsel

static const Common::ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)Tinsel::gameDescriptions,
	// Size of that superset structure
	sizeof(Tinsel::TinselGameDescription),
	// Number of bytes to compute MD5 sum for
	5000,
	// List of all engine targets
	tinselGames,
	// Structure for autoupgrading obsolete targets
	0,
	// Name of single gameid (optional)
	"tinsel",
	// List of files for file-based fallback detection (optional)
	0,
	// Flags
	0
};

class TinselMetaEngine : public Common::AdvancedMetaEngine {
public:
	TinselMetaEngine() : Common::AdvancedMetaEngine(detectionParams) {}

	virtual const char *getName() const {
		return "Tinsel Engine";
	}

	virtual const char *getCopyright() const {
		// FIXME: Bad copyright string.
		// Should be something like "Tinsel (C) Psygnosis" or so... ???
		return "Tinsel Engine";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const Common::ADGameDescription *desc) const;

	virtual bool hasFeature(MetaEngineFeature f) const;	
	virtual SaveStateList listSaves(const char *target) const;
};

bool TinselMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves);
}

namespace Tinsel {
extern int getList(Common::SaveFileManager *saveFileMan, const Common::String &target);
}

SaveStateList TinselMetaEngine::listSaves(const char *target) const {
	int numStates = Tinsel::getList(g_system->getSavefileManager(), target);

	SaveStateList saveList;
	for (int i = 0; i < numStates; i++) {
		SaveStateDescriptor sd(i,
				Tinsel::ListEntry(i, Tinsel::LE_DESC),
				Tinsel::ListEntry(i, Tinsel::LE_NAME));
		// TODO: Also add savedFiles[i].dateTime to the SaveStateDescriptor
		saveList.push_back(sd);
	}

	return saveList;
}


bool TinselMetaEngine::createInstance(OSystem *syst, Engine **engine, const Common::ADGameDescription *desc) const {
	const Tinsel::TinselGameDescription *gd = (const Tinsel::TinselGameDescription *)desc;
	if (gd) {
		*engine = new Tinsel::TinselEngine(syst, gd);
	}
	return gd != 0;
}

#if PLUGIN_ENABLED_DYNAMIC(TINSEL)
	REGISTER_PLUGIN_DYNAMIC(TINSEL, PLUGIN_TYPE_ENGINE, TinselMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(TINSEL, PLUGIN_TYPE_ENGINE, TinselMetaEngine);
#endif
