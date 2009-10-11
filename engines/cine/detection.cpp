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
#include "common/system.h"

#include "cine/cine.h"
#include "cine/various.h"

namespace Cine {

struct CINEGameDescription {
	ADGameDescription desc;

	int gameType;
	uint32 features;
};

int CineEngine::getGameType() const { return _gameDescription->gameType; }
uint32 CineEngine::getFeatures() const { return _gameDescription->features; }
Common::Language CineEngine::getLanguage() const { return _gameDescription->desc.language; }
Common::Platform CineEngine::getPlatform() const { return _gameDescription->desc.platform; }

} // End of namespace Cine

static const PlainGameDescriptor cineGames[] = {
	{"cine", "Cinematique evo.1 engine game"},
	{"fw", "Future Wars"},
	{"os", "Operation Stealth"},
	{0, 0}
};

static const ADObsoleteGameID obsoleteGameIDsTable[] = {
	{"fw", "cine", Common::kPlatformUnknown},
	{"os", "cine", Common::kPlatformUnknown},
	{0, 0, Common::kPlatformUnknown}
};

namespace Cine {

using Common::GUIO_NONE;

static const CINEGameDescription gameDescriptions[] = {
	{
		{
			"fw",
			"",
			AD_ENTRY1("part01", "61d003202d301c29dd399acfb1354310"),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GType_FW,
		0,
	},

	// This is a CD version of Future Wars published by Sony.
	// This version has a crypted AUTO00.PRC.
	{
		{
			"fw",
			"Sony CD version",
			{
				{ "AUTO00.PRC", 0, "4fe1e7930b38e3c63f0f2474d471bf8f", -1},
				{ "PART01", 0, "61d003202d301c29dd399acfb1354310", -1},
				{ NULL, 0, NULL, 0}
			},
			Common::EN_USA,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO_NONE
		},
		GType_FW,
		GF_CD | GF_CRYPTED_BOOT_PRC,
	},

	{
		// This is the version included in the UK "Classic Collection"
		{
			"fw",
			"",
			AD_ENTRY1("part01", "91d7271155520eae6915a9dd2dac120c"),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GType_FW,
		0,
	},

	{
		{
			"fw",
			"",
			AD_ENTRY1("part01", "f5e98fcca3fb5e7afa284c81c39d8b14"),
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GType_FW,
		GF_ALT_FONT,
	},

	{
		{
			"fw",
			"",
			AD_ENTRY1("part01", "570109f965c7f53984b98c83d86eb206"),
			Common::ES_ESP,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GType_FW,
		GF_ALT_FONT,
	},

	{
		{
			"fw",
			"",
			AD_ENTRY1("part01", "5d1acb97abe9591f9008e00d07add95a"),
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GType_FW,
		0,
	},

	{
		{
			"fw",
			"",
			AD_ENTRY1("part01", "57afd280b598b4180fda6689fbedc4b8"),
			Common::EN_ANY,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GType_FW,
		0,
	},

	{ // Amiga "Interplay" labeled version
		{
			"fw",
			"",
			AD_ENTRY1("part01", "a17a5eb15200c63276d486a88263ccd0"),
			Common::EN_USA,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GType_FW,
		0,
	},

	{
		{
			"fw",
			"",
			AD_ENTRY1("part01", "3a87a913e0e33963a48a7f822ca0eb0e"),
			Common::DE_DEU,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GType_FW,
		GF_ALT_FONT,
	},

	{
		{
			"fw",
			"",
			AD_ENTRY1("part01", "5ad0007ccd5f7b3dd6b15ea7f281f9e1"),
			Common::ES_ESP,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GType_FW,
		0,
	},

	{
		{
			"fw",
			"",
			AD_ENTRY1("part01", "460f2da8793bc581a2d4b6fc19ccb5ae"),
			Common::FR_FRA,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GType_FW,
		0,
	},

	{
		{
			"fw",
			"",
			AD_ENTRY1("part01", "1c8e5207743172134409ac58860021af"),
			Common::IT_ITA,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GType_FW,
		0,
	},

	{
		{
			"fw",
			"Demo",
			{
				{ "demo",		0,	"0f50767cd964e302d3af0ba2528df8c4", -1},
				{ "demo.prc",	0,	"d2ac3a743d288359c63644ea7071edae", -1},
				{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformAmiga,
			ADGF_DEMO,
			GUIO_NONE
		},
		GType_FW,
		0,
	},

	{
		{
			"fw",
			"",
			AD_ENTRY1("part01", "36050db13af57e462ca1adc4df99de4e"),
			Common::EN_ANY,
			Common::kPlatformAtariST,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GType_FW,
		0,
	},

	{
		{
			"fw",
			"",
			AD_ENTRY1("part01", "ef245573b7dab0d4825ceb98e37cef4d"),
			Common::FR_FRA,
			Common::kPlatformAtariST,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GType_FW,
		0,
	},

#if 0
	{
		{
			"os",
			"256 colors",
			AD_ENTRY1("procs00",	"d6752e7d25924cb866b61eb7cb0c8b56"),
			Common::EN_GRB,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GType_OS,
		0,
	},

	{
		// This is a 16 color PC version (It came on three 720kB 3.5" disks).
		// The protagonist is named John Glames in this version.
		{
			"os",
			"",
			AD_ENTRY1("procs1", "9629129b86979fa592c1787385bf3695"),
			Common::EN_GRB,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GType_OS,
		0,
	},

	{
		{
			"os",
			"",
			AD_ENTRY1("procs1", "d8c3a9d05a63e4cfa801826a7063a126"),
			Common::EN_USA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GType_OS,
		0,
	},

	{
		{
			"os",
			"256 colors",
			AD_ENTRY1("procs00", "862a75d76fb7fffec30e52be9ad1c474"),
			Common::EN_USA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GType_OS,
		GF_CD,
	},

	{
		{
			"os",
			"",
			AD_ENTRY1("procs1", "39b91ae35d1297ce0a76a1a803ca1593"),
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GType_OS,
		0,
	},

	{
		{
			"os",
			"",
			AD_ENTRY1("procs1", "74c2dabd9d212525fca8875a5f6d8994"),
			Common::ES_ESP,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GType_OS,
		0,
	},

	{
		{
			"os",
			"256 colors",
			{
				{ "procs1",		0,	"74c2dabd9d212525fca8875a5f6d8994", -1},
				{ "sds1",		0,	"75443ba39cdc95667e07d7118e5c151c", -1},
				{ NULL, 0, NULL, 0}
			},
			Common::ES_ESP,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GType_OS,
		GF_CD,
	},

	{
		{
			"os",
			"256 colors",
			AD_ENTRY1("procs00", "f143567f08cfd1a9b1c9a41c89eadfef"),
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GType_OS,
		0,
	},

	{
		{
			"os",
			"",
			AD_ENTRY1("procs1", "da066e6b8dd93f2502c2a3755f08dc12"),
			Common::IT_ITA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GType_OS,
		0,
	},

	{
		{
			"os",
			"",
			AD_ENTRY1("procs0", "a9da5531ead0ebf9ad387fa588c0cbb0"),
			Common::EN_GRB,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GType_OS,
		0,
	},

	{
		{
			"os",
			"alt",
			AD_ENTRY1("procs0", "8a429ced2f4acff8a15ae125174042e8"),
			Common::EN_GRB,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GType_OS,
		0,
	},

	{
		{
			"os",
			"",
			AD_ENTRY1("procs0", "d5f27e33fc29c879f36f15b86ccfa58c"),
			Common::EN_USA,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GType_OS,
		0,
	},

	{
		{
			"os",
			"",
			AD_ENTRY1("procs0", "8b7dce249821d3a62b314399c4334347"),
			Common::DE_DEU,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GType_OS,
		0,
	},

	{
		{
			"os",
			"",
			AD_ENTRY1("procs0", "35fc295ddd0af9da932d256ba799a4b0"),
			Common::ES_ESP,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GType_OS,
		0,
	},

	{
		{
			"os",
			"",
			AD_ENTRY1("procs0", "d4ea4a97e01fa67ea066f9e785050ed2"),
			Common::FR_FRA,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GType_OS,
		0,
	},

	{
		{
			"os",
			"Demo",
			AD_ENTRY1("demo", "8d3a750d1c840b1b1071e42f9e6f6aa2"),
			Common::EN_GRB,
			Common::kPlatformAmiga,
			ADGF_DEMO,
			GUIO_NONE
		},
		GType_OS,
		GF_DEMO,
	},

	{
		{
			"os",
			"",
			AD_ENTRY1("procs0", "1501d5ae364b2814a33ed19347c3fcae"),
			Common::EN_GRB,
			Common::kPlatformAtariST,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GType_OS,
		0,
	},

	{
		{
			"os",
			"",
			AD_ENTRY1("procs0", "2148d25de3219dd4a36580ca735d0afa"),
			Common::FR_FRA,
			Common::kPlatformAtariST,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GType_OS,
		0,
	},
#endif

	{ AD_TABLE_END_MARKER, 0, 0 }
};

} // End of namespace Cine

static const ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)Cine::gameDescriptions,
	// Size of that superset structure
	sizeof(Cine::CINEGameDescription),
	// Number of bytes to compute MD5 sum for
	5000,
	// List of all engine targets
	cineGames,
	// Structure for autoupgrading obsolete targets
	obsoleteGameIDsTable,
	// Name of single gameid (optional)
	"cine",
	// List of files for file-based fallback detection (optional)
	0,
	// Flags
	0,
	// Additional GUI options (for every game}
	Common::GUIO_NOSPEECH | Common::GUIO_NOMIDI
};

class CineMetaEngine : public AdvancedMetaEngine {
public:
	CineMetaEngine() : AdvancedMetaEngine(detectionParams) {}

	virtual const char *getName() const {
		return "Cinematique evo 1 engine";
	}

	virtual const char *getOriginalCopyright() const {
#if 0
		return "Future Wars & Operation Stealth (C) Delphine Software";
#else
		return "Future Wars (C) Delphine Software";
#endif
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	virtual void removeSaveState(const char *target, int slot) const;
};

bool CineMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave);
}

bool Cine::CineEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

bool CineMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Cine::CINEGameDescription *gd = (const Cine::CINEGameDescription *)desc;
	if (gd) {
		*engine = new Cine::CineEngine(syst, gd);
	}
	return gd != 0;
}

SaveStateList CineMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	SaveStateList saveList;

	Common::String pattern = target;
	pattern += ".?";
	Common::StringList filenames = saveFileMan->listSavefiles(pattern);
	sort(filenames.begin(), filenames.end());
	Common::StringList::const_iterator file;

	Common::String filename = target;
	filename += ".dir";
	Common::InSaveFile *in = saveFileMan->openForLoading(filename);
	if (in) {
		typedef char CommandeType[20];
		CommandeType saveNames[10];

		// Initialize all savegames' descriptions to empty strings
		// so that if the savegames' descriptions can only be partially read from file
		// then the missing ones are correctly set to empty strings.
		memset(saveNames, 0, sizeof(saveNames));

		in->read(saveNames, 10 * 20);
		CommandeType saveDesc;

		for (file = filenames.begin(); file != filenames.end(); ++file) {
			// Jump over savegame files that don't end with a digit (e.g. "fw.3" is ok, "fw.a" is not).
			if (!isdigit(file->lastChar()))
				continue;

			// Obtain the last digit of the filename, since they correspond to the save slot
			int slotNum = atoi(file->c_str() + file->size() - 1);

			// Copy the savegame description making sure it ends with a trailing zero
			strncpy(saveDesc, saveNames[slotNum], 20);
			saveDesc[sizeof(CommandeType) - 1] = 0;

			saveList.push_back(SaveStateDescriptor(slotNum, saveDesc));
		}
	}

	delete in;

	return saveList;
}

int CineMetaEngine::getMaximumSaveSlot() const { return 9; }

void CineMetaEngine::removeSaveState(const char *target, int slot) const {
	// Load savegame descriptions from index file
	typedef char CommandeType[20];
	CommandeType saveNames[10];

	// Initialize all savegames' descriptions to empty strings
	// so that if the savegames' descriptions can only be partially read from file
	// then the missing ones are correctly set to empty strings.
	memset(saveNames, 0, sizeof(saveNames));

	Common::InSaveFile *in;
	char tmp[80];

	snprintf(tmp, 80, "%s.dir", target);
	in = g_system->getSavefileManager()->openForLoading(tmp);

	if (!in)
		return;

	in->read(saveNames, 10 * 20);
	delete in;

	// Set description for selected slot
	char slotName[20];
	slotName[0] = 0;
	strncpy(saveNames[slot], slotName, 20);

	// Update savegame descriptions
	char indexFile[80];
	snprintf(indexFile, 80, "%s.dir", target);

	Common::OutSaveFile *out = g_system->getSavefileManager()->openForSaving(indexFile);
	if (!out) {
		warning("Unable to open file %s for saving", indexFile);
		return;
	}

	out->write(saveNames, 10 * 20);
	delete out;

	// Delete save file
	char saveFileName[256];
	sprintf(saveFileName, "%s.%1d", target, slot);

	g_system->getSavefileManager()->removeSavefile(saveFileName);
}

#if PLUGIN_ENABLED_DYNAMIC(CINE)
	REGISTER_PLUGIN_DYNAMIC(CINE, PLUGIN_TYPE_ENGINE, CineMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(CINE, PLUGIN_TYPE_ENGINE, CineMetaEngine);
#endif

namespace Cine {

Common::Error CineEngine::loadGameState(int slot) {
	char saveNameBuffer[256];
	sprintf(saveNameBuffer, "%s.%1d", _targetName.c_str(), slot);
	bool gameLoaded = makeLoad(saveNameBuffer);

	return gameLoaded ? Common::kNoError : Common::kUnknownError;
}

Common::Error CineEngine::saveGameState(int slot, const char *desc) {
	// Load savegame descriptions from index file
	loadSaveDirectory();

	// Set description for selected slot making sure it ends with a trailing zero
	strncpy(currentSaveName[slot], desc, 20);
	currentSaveName[slot][sizeof(CommandeType) - 1] = 0;

	// Update savegame descriptions
	char indexFile[80];
	snprintf(indexFile, 80, "%s.dir", _targetName.c_str());

	Common::OutSaveFile *fHandle = g_saveFileMan->openForSaving(indexFile);
	if (!fHandle) {
		warning("Unable to open file %s for saving", indexFile);
		return Common::kUnknownError;
	}

	fHandle->write(currentSaveName, 10 * 20);
	delete fHandle;

	// Save game
	char saveFileName[256];
	sprintf(saveFileName, "%s.%1d", _targetName.c_str(), slot);
	makeSave(saveFileName);

	checkDataDisk(-1);

	return Common::kNoError;
}

bool CineEngine::canLoadGameStateCurrently() {
	return (!disableSystemMenu && !inMenu);
}

bool CineEngine::canSaveGameStateCurrently() {
	return (allowPlayerInput && !disableSystemMenu && !inMenu);
}

} // End of namespace Cine
