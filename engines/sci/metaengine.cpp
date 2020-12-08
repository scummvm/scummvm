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

#include "engines/advancedDetector.h"
#include "base/plugins.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/ptr.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"
#include "graphics/thumbnail.h"
#include "graphics/surface.h"

#include "sci/sci.h"
#include "sci/engine/kernel.h"
#include "sci/engine/savegame.h"
#include "sci/engine/script.h"
#include "sci/engine/seg_manager.h"
#include "sci/engine/state.h"

namespace Sci {

struct GameIdStrToEnum {
	const char *gameidStr;
	SciGameId gameidEnum;
	bool isSci32;
};

static const GameIdStrToEnum s_gameIdStrToEnum[] = {
	{ "astrochicken",    GID_ASTROCHICKEN,     false },
	{ "camelot",         GID_CAMELOT,          false },
	{ "castlebrain",     GID_CASTLEBRAIN,      false },
	{ "chest",           GID_CHEST,            true },
	{ "christmas1988",   GID_CHRISTMAS1988,    false },
	{ "christmas1990",   GID_CHRISTMAS1990,    false },
	{ "christmas1992",   GID_CHRISTMAS1992,    false },
	{ "cnick-kq",        GID_CNICK_KQ,         false },
	{ "cnick-laurabow",  GID_CNICK_LAURABOW,   false },
	{ "cnick-longbow",   GID_CNICK_LONGBOW,    false },
	{ "cnick-lsl",       GID_CNICK_LSL,        false },
	{ "cnick-sq",        GID_CNICK_SQ,         false },
	{ "ecoquest",        GID_ECOQUEST,         false },
	{ "ecoquest2",       GID_ECOQUEST2,        false },
	{ "fairytales",      GID_FAIRYTALES,       false },
	{ "freddypharkas",   GID_FREDDYPHARKAS,    false },
	{ "funseeker",       GID_FUNSEEKER,        false },
	{ "gk1demo",         GID_GK1DEMO,          false },
	{ "gk1",             GID_GK1,              true },
	{ "gk2",             GID_GK2,              true },
	{ "hoyle1",          GID_HOYLE1,           false },
	{ "hoyle2",          GID_HOYLE2,           false },
	{ "hoyle3",          GID_HOYLE3,           false },
	{ "hoyle4",          GID_HOYLE4,           false },
	{ "hoyle5",          GID_HOYLE5,           true },
	{ "hoyle5bridge",    GID_HOYLE5,           true },
	{ "hoyle5children",  GID_HOYLE5,           true },
	{ "hoyle5solitaire", GID_HOYLE5,           true },
	{ "iceman",          GID_ICEMAN,           false },
	{ "inndemo",         GID_INNDEMO,          false },
	{ "islandbrain",     GID_ISLANDBRAIN,      false },
	{ "jones",           GID_JONES,            false },
	{ "kq1sci",          GID_KQ1,              false },
	{ "kq4sci",          GID_KQ4,              false },
	{ "kq5",             GID_KQ5,              false },
	{ "kq6",             GID_KQ6,              false },
	{ "kq7",             GID_KQ7,              true },
	{ "kquestions",      GID_KQUESTIONS,       true },
	{ "laurabow",        GID_LAURABOW,         false },
	{ "laurabow2",       GID_LAURABOW2,        false },
	{ "lighthouse",      GID_LIGHTHOUSE,       true },
	{ "longbow",         GID_LONGBOW,          false },
	{ "lsl1sci",         GID_LSL1,             false },
	{ "lsl2",            GID_LSL2,             false },
	{ "lsl3",            GID_LSL3,             false },
	{ "lsl5",            GID_LSL5,             false },
	{ "lsl6",            GID_LSL6,             false },
	{ "lsl6hires",       GID_LSL6HIRES,        true },
	{ "lsl7",            GID_LSL7,             true },
	{ "mothergoose",     GID_MOTHERGOOSE,      false },
	{ "mothergoose256",  GID_MOTHERGOOSE256,   false },
	{ "mothergoosehires",GID_MOTHERGOOSEHIRES, true },
	{ "msastrochicken",  GID_MSASTROCHICKEN,   false },
	{ "pepper",          GID_PEPPER,           false },
	{ "phantasmagoria",  GID_PHANTASMAGORIA,   true },
	{ "phantasmagoria2", GID_PHANTASMAGORIA2,  true },
	{ "pq1sci",          GID_PQ1,              false },
	{ "pq2",             GID_PQ2,              false },
	{ "pq3",             GID_PQ3,              false },
	{ "pq4",             GID_PQ4,              true },
	{ "pq4demo",         GID_PQ4DEMO,          false },
	{ "pqswat",          GID_PQSWAT,           true },
	{ "qfg1",            GID_QFG1,             false },
	{ "qfg1vga",         GID_QFG1VGA,          false },
	{ "qfg2",            GID_QFG2,             false },
	{ "qfg3",            GID_QFG3,             false },
	{ "qfg4",            GID_QFG4,             true },
	{ "qfg4demo",        GID_QFG4DEMO,         false },
	{ "rama",            GID_RAMA,             true },
	{ "sci-fanmade",     GID_FANMADE,          false },
	{ "shivers",         GID_SHIVERS,          true },
	//{ "shivers2",        GID_SHIVERS2,       true },	// Not SCI
	{ "slater",          GID_SLATER,           false },
	{ "sq1sci",          GID_SQ1,              false },
	{ "sq3",             GID_SQ3,              false },
	{ "sq4",             GID_SQ4,              false },
	{ "sq5",             GID_SQ5,              false },
	{ "sq6",             GID_SQ6,              true },
	{ "torin",           GID_TORIN,            true },
	{ NULL,              (SciGameId)-1,        false }
};

struct OldNewIdTableEntry {
	const char *oldId;
	const char *newId;
	SciVersion version;
};

static const OldNewIdTableEntry s_oldNewTable[] = {
	{ "archive",    "chest",            SCI_VERSION_NONE       },
	{ "arthur",		"camelot",			SCI_VERSION_NONE       },
	{ "brain",      "castlebrain",      SCI_VERSION_1_MIDDLE   },	// Amiga
	{ "brain",      "castlebrain",      SCI_VERSION_1_LATE     },
	{ "demo",		"christmas1988",	SCI_VERSION_NONE       },
	{ "card",       "christmas1990",    SCI_VERSION_1_EARLY,   },
	{ "card",       "christmas1992",    SCI_VERSION_1_1        },
	{ "RH Budget",	"cnick-longbow",	SCI_VERSION_NONE       },
	// iceman is the same
	{ "icedemo",	"iceman",			SCI_VERSION_NONE       },
	// longbow is the same
	{ "eco",		"ecoquest",			SCI_VERSION_NONE       },
	{ "eco2",		"ecoquest2",		SCI_VERSION_NONE       },	// EcoQuest 2 demo
	{ "rain",		"ecoquest2",		SCI_VERSION_NONE       },	// EcoQuest 2 full
	{ "tales",		"fairytales",		SCI_VERSION_NONE       },
	{ "fp",			"freddypharkas",	SCI_VERSION_NONE       },
	{ "emc",		"funseeker",		SCI_VERSION_NONE       },
	{ "gk",			"gk1",				SCI_VERSION_NONE       },
	// gk2 is the same
	{ "gk2demo",	"gk2",				SCI_VERSION_NONE       },
	{ "hoyledemo",	"hoyle1",			SCI_VERSION_NONE       },
	{ "cardgames",	"hoyle1",			SCI_VERSION_NONE       },
	{ "solitare",	"hoyle2",			SCI_VERSION_NONE       },
	{ "hoyle3",	    "hoyle3",			SCI_VERSION_NONE       },
	{ "hoyle4",	    "hoyle4",			SCI_VERSION_1_1        },
	{ "hoyle4",	    "hoyle5",			SCI_VERSION_2_1_MIDDLE },
	{ "brain",      "islandbrain",      SCI_VERSION_1_1        },
	{ "demo000",	"kq1sci",			SCI_VERSION_NONE       },
	{ "kq1",		"kq1sci",			SCI_VERSION_NONE       },
	{ "kq4",		"kq4sci",			SCI_VERSION_NONE       },
	// kq5 is the same
	// kq6 is the same
	{ "kq7cd",		"kq7",				SCI_VERSION_NONE       },
	{ "quizgame-demo", "kquestions",    SCI_VERSION_NONE       },
	{ "mm1",		"laurabow",			SCI_VERSION_NONE       },
	{ "cb1",		"laurabow",			SCI_VERSION_NONE       },
	{ "lb2",		"laurabow2",		SCI_VERSION_NONE       },
	{ "rh",			"longbow",			SCI_VERSION_NONE       },
	{ "ll1",		"lsl1sci",			SCI_VERSION_NONE       },
	{ "lsl1",		"lsl1sci",			SCI_VERSION_NONE       },
	// lsl2 is the same
	{ "lsl3",		"lsl3",				SCI_VERSION_NONE       },
	{ "ll5",		"lsl5",				SCI_VERSION_NONE       },
	// lsl5 is the same
	// lsl6 is the same
	{ "mg",			"mothergoose",		SCI_VERSION_NONE       },
	{ "twisty",		"pepper",			SCI_VERSION_NONE       },
	{ "scary",      "phantasmagoria",   SCI_VERSION_NONE       },
	// TODO: distinguish the full version of Phantasmagoria from the demo
	{ "pq1",		"pq1sci",			SCI_VERSION_NONE       },
	{ "pq",			"pq2",				SCI_VERSION_NONE       },
	// pq3 is the same
	// pq4 is the same
	{ "hq",			"qfg1",				SCI_VERSION_NONE       },	// QFG1 SCI0/EGA
	{ "glory",      "qfg1",             SCI_VERSION_0_LATE     },	// QFG1 SCI0/EGA
	{ "trial",		"qfg2",				SCI_VERSION_NONE       },
	{ "hq2demo",	"qfg2",				SCI_VERSION_NONE       },
	// rama is the same
	// TODO: distinguish the full version of rama from the demo
	{ "thegame",	"slater",			SCI_VERSION_NONE       },
	{ "sq1demo",	"sq1sci",			SCI_VERSION_NONE       },
	{ "sq1",		"sq1sci",			SCI_VERSION_NONE       },
	// sq3 is the same
	// sq4 is the same
	// sq5 is the same
	// sq6 is the same
	// TODO: distinguish the full version of SQ6 from the demo
	// torin is the same
	{ "l7",			"lsl7",				SCI_VERSION_NONE       },
	{ "p2",			"phantasmagoria2",	SCI_VERSION_NONE       },
	{ "lite",		"lighthouse",		SCI_VERSION_NONE       },

	{ "", "", SCI_VERSION_NONE }
};

/**
 * Converts the builtin Sierra game IDs to the ones we use in ScummVM
 * @param[in] gameId		The internal game ID
 * @param[in] gameFlags     The game's flags, which are adjusted accordingly for demos
 * @return					The equivalent ScummVM game id
 */
static Common::String convertSierraGameId(Common::String sierraId, uint32 *gameFlags, ResourceManager &resMan) {
	// Convert the id to lower case, so that we match all upper/lower case variants.
	sierraId.toLowercase();

	// If the game has less than the expected scripts, it's a demo
	uint32 demoThreshold = 100;
	// ...but there are some exceptions
	if (sierraId == "brain" || sierraId == "lsl1" ||
		sierraId == "mg" || sierraId == "pq" ||
		sierraId == "jones" ||
		sierraId == "cardgames" || sierraId == "solitare" ||
		sierraId == "catdate" ||
		sierraId == "hoyle4")
		demoThreshold = 40;
	if (sierraId == "hoyle3")
		demoThreshold = 45;	// cnick-kq has 42 scripts. The actual hoyle 3 demo has 27.
	if (sierraId == "fp" || sierraId == "gk" || sierraId == "pq4")
		demoThreshold = 150;

	Common::List<ResourceId> resources = resMan.listResources(kResourceTypeScript, -1);
	if (resources.size() < demoThreshold) {
		*gameFlags |= ADGF_DEMO;

		// Crazy Nick's Picks
		if (sierraId == "lsl1" && resources.size() == 34)
			return "cnick-lsl";
		if (sierraId == "sq4" && resources.size() == 34)
			return "cnick-sq";
		if (sierraId == "hoyle3" && resources.size() == 42)
			return "cnick-kq";
		if (sierraId == "rh budget" && resources.size() == 39)
			return "cnick-longbow";
		// TODO: cnick-laurabow (the name of the game object contains junk)

		// Handle Astrochicken 1 (SQ3) and 2 (SQ4)
		if (sierraId == "sq3" && resources.size() == 20)
			return "astrochicken";
		if (sierraId == "sq4")
			return "msastrochicken";
	}

	if (sierraId == "torin" && resources.size() == 226)	// Torin's Passage demo
		*gameFlags |= ADGF_DEMO;

	for (const OldNewIdTableEntry *cur = s_oldNewTable; cur->oldId[0]; ++cur) {
		if (sierraId == cur->oldId) {
			// Distinguish same IDs via the SCI version
			if (cur->version != SCI_VERSION_NONE && cur->version != getSciVersion())
				continue;

			return cur->newId;
		}
	}

	if (sierraId == "glory") {
		// This could either be qfg1 VGA, qfg3 or qfg4 demo (all SCI1.1),
		// or qfg4 full (SCI2)
		// qfg1 VGA doesn't have view 1
		if (!resMan.testResource(ResourceId(kResourceTypeView, 1)))
			return "qfg1vga";

		// qfg4 full is SCI2
		if (getSciVersion() == SCI_VERSION_2)
			return "qfg4";

		// qfg4 demo has less than 50 scripts
		if (resources.size() < 50)
			return "qfg4demo";

		// Otherwise it's qfg3
		return "qfg3";
	}

	return sierraId;
}

} // End of namespace Sci

namespace Sci {

class SciMetaEngine : public AdvancedMetaEngine {
public:
    const char *getName() const override {
		return "sci";
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const override;
	bool hasFeature(MetaEngineFeature f) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	void removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;

	// A fallback detection method. This is not ideal as all detection lives in MetaEngine, but
	// here fb detection has many engine dependencies.
	virtual ADDetectedGame fallbackDetectExtern(uint md5Bytes, const FileMap &allFiles, const Common::FSList &fslist) const override;
};

Common::Error SciMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const GameIdStrToEnum *g = s_gameIdStrToEnum;
	for (; g->gameidStr; ++g) {
		if (0 == strcmp(desc->gameId, g->gameidStr)) {
#ifndef ENABLE_SCI32
			if (g->isSci32) {
				return Common::Error(Common::kUnsupportedGameidError, _s("SCI32 support not compiled in"));
			}
#endif

			*engine = new SciEngine(syst, desc, g->gameidEnum);
			return Common::kNoError;
		}
	}

	return Common::kUnsupportedGameidError;
}

bool SciMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime);
}

bool SciEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime); // ||
		//(f == kSupportsSavingDuringRuntime);
		// We can't allow saving through ScummVM menu, because
		//  a) lots of games don't like saving everywhere (e.g. castle of dr. brain)
		//  b) some games even dont allow saving in certain rooms (e.g. lsl6)
		//  c) somehow some games even get mad when doing this (execstackbase was 1 all of a sudden in lsl3)
		//  d) for sci0/sci01 games we should at least wait till status bar got drawn, although this may not be enough
		// we can't make sure that the scripts are fine with us saving at a specific location, doing so may work sometimes
		//  and some other times it won't work.
}

SaveStateList SciMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += ".###";

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	bool hasAutosave = false;
	int slotNr = 0;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		slotNr = atoi(file->c_str() + file->size() - 3);

		if (slotNr >= 0 && slotNr <= 99) {
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				SavegameMetadata meta;
				if (!get_savegame_metadata(in, meta)) {
					// invalid
					delete in;
					continue;
				}
				SaveStateDescriptor descriptor(slotNr, meta.name);

				if (slotNr == 0) {
					// ScummVM auto-save slot
					descriptor.setWriteProtectedFlag(true);
					hasAutosave = true;
				} else {
					descriptor.setWriteProtectedFlag(false);
				}

				saveList.push_back(descriptor);
				delete in;
			}
		}
	}

	if (!hasAutosave) {
		SaveStateDescriptor descriptor(0, _("(Autosave)"));
		descriptor.setLocked(true);
		saveList.push_back(descriptor);
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

SaveStateDescriptor SciMetaEngine::querySaveMetaInfos(const char *target, int slotNr) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slotNr);
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(fileName);
	SaveStateDescriptor descriptor(slotNr, "");

	if (slotNr == 0) {
		// ScummVM auto-save slot
		descriptor.setWriteProtectedFlag(true);
		descriptor.setDeletableFlag(false);
	} else {
		descriptor.setWriteProtectedFlag(false);
		descriptor.setDeletableFlag(true);
	}

	if (in) {
		SavegameMetadata meta;

		if (!get_savegame_metadata(in, meta)) {
			// invalid
			delete in;

			descriptor.setDescription("*Invalid*");
			return descriptor;
		}

		descriptor.setDescription(meta.name);

		Graphics::Surface *thumbnail;
		if (!Graphics::loadThumbnail(*in, thumbnail)) {
			// invalid
			delete in;

			descriptor.setDescription("*Invalid*");
			return descriptor;
		}
		descriptor.setThumbnail(thumbnail);

		int day = (meta.saveDate >> 24) & 0xFF;
		int month = (meta.saveDate >> 16) & 0xFF;
		int year = meta.saveDate & 0xFFFF;

		descriptor.setSaveDate(year, month, day);

		int hour = (meta.saveTime >> 16) & 0xFF;
		int minutes = (meta.saveTime >> 8) & 0xFF;

		descriptor.setSaveTime(hour, minutes);

		if (meta.version >= 34) {
			descriptor.setPlayTime(meta.playTime * 1000 / 60);
		} else {
			descriptor.setPlayTime(meta.playTime * 1000);
		}

		delete in;

		return descriptor;
	}
	// Return empty descriptor
	return descriptor;
}

int SciMetaEngine::getMaximumSaveSlot() const { return 99; }

void SciMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(fileName);
}

Common::Error SciEngine::loadGameState(int slot) {
	_gamestate->_delayedRestoreGameId = slot;
	return Common::kNoError;
}

Common::Error SciEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	const char *version = "";
	if (gamestate_save(_gamestate, slot, desc, version)) {
		return Common::kNoError;
	}
	return Common::kWritingFailed;
}

bool SciEngine::canLoadGameStateCurrently() {
#ifdef ENABLE_SCI32
	const Common::String &guiOptions = ConfMan.get("guioptions");
	if (getSciVersion() >= SCI_VERSION_2) {
		if (ConfMan.getBool("originalsaveload") ||
			Common::checkGameGUIOption(GUIO_NOLAUNCHLOAD, guiOptions)) {

			return false;
		}
	}
#endif

	return !_gamestate->executionStackBase;
}

bool SciEngine::canSaveGameStateCurrently() {
	// see comment about kSupportsSavingDuringRuntime in SciEngine::hasFeature
	return false;
}

} // End of namespace Sci

/**
 * External fallback detection-related code.
 */
namespace Sci {

Common::Language charToScummVMLanguage(const char c) {
	switch (c) {
	case 'F':
		return Common::FR_FRA;
	case 'S':
		return Common::ES_ESP;
	case 'I':
		return Common::IT_ITA;
	case 'G':
		return Common::DE_DEU;
	case 'J':
	case 'j':
		return Common::JA_JPN;
	case 'P':
		return Common::PT_BRA;
	default:
		return Common::UNK_LANG;
	}
}

static char s_fallbackGameIdBuf[256];

/**
 * The fallback game descriptor used by the SCI engine's fallbackDetector.
 * Contents of this struct are overwritten by the fallbackDetector.
 */
static ADGameDescription s_fallbackDesc = {
	"",
	"",
	AD_ENTRY1(0, 0), // This should always be AD_ENTRY1(0, 0) in the fallback descriptor
	Common::UNK_LANG,
	Common::kPlatformDOS,
	ADGF_NO_FLAGS,
	GUIO3(GAMEOPTION_PREFER_DIGITAL_SFX, GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_MIDI_MODE)
};

ADDetectedGame SciMetaEngine::fallbackDetectExtern(uint md5Bytes, const FileMap &allFiles, const Common::FSList &fslist) const {
	bool foundResMap = false;
	bool foundRes000 = false;

	// Set some defaults
	s_fallbackDesc.extra = "";
	s_fallbackDesc.language = Common::EN_ANY;
	s_fallbackDesc.flags = ADGF_NO_FLAGS;
	s_fallbackDesc.platform = Common::kPlatformDOS;	// default to PC platform
	s_fallbackDesc.gameId = "sci";
	s_fallbackDesc.guiOptions = GUIO3(GAMEOPTION_PREFER_DIGITAL_SFX, GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_MIDI_MODE);

	if (allFiles.contains("resource.map") || allFiles.contains("Data1")
	    || allFiles.contains("resmap.000") || allFiles.contains("resmap.001")) {
		foundResMap = true;
	}

	// Determine if we got a CD version and set the CD flag accordingly, by checking for
	// resource.aud for SCI1.1 CD games, or audio001.002 for SCI1 CD games. We assume that
	// the file should be over 10MB, as it contains all the game speech and is usually
	// around 450MB+. The size check is for some floppy game versions like KQ6 floppy, which
	// also have a small resource.aud file
	if (allFiles.contains("resource.aud") || allFiles.contains("resaud.001") || allFiles.contains("audio001.002")) {
		Common::FSNode file = allFiles.contains("resource.aud") ? allFiles["resource.aud"] : (allFiles.contains("resaud.001") ? allFiles["resaud.001"] : allFiles["audio001.002"]);
		Common::SeekableReadStream *tmpStream = file.createReadStream();
		if (tmpStream->size() > 10 * 1024 * 1024) {
			// We got a CD version, so set the CD flag accordingly
			s_fallbackDesc.flags |= ADGF_CD;
		}
		delete tmpStream;
	}

	if (allFiles.contains("resource.000") || allFiles.contains("resource.001")
		|| allFiles.contains("ressci.000") || allFiles.contains("ressci.001"))
		foundRes000 = true;

	// Data1 contains both map and volume for SCI1.1+ Mac games
	if (allFiles.contains("Data1")) {
		foundResMap = foundRes000 = true;
		 s_fallbackDesc.platform = Common::kPlatformMacintosh;
	}

	// Determine the game platform
	// The existence of any of these files indicates an Amiga game
	if (allFiles.contains("9.pat") || allFiles.contains("spal") ||
		allFiles.contains("patch.005") || allFiles.contains("bank.001"))
			s_fallbackDesc.platform = Common::kPlatformAmiga;

	// The existence of 7.pat or patch.200 indicates a Mac game
	if (allFiles.contains("7.pat") || allFiles.contains("patch.200"))
		s_fallbackDesc.platform = Common::kPlatformMacintosh;

	// The data files for Atari ST versions are the same as their DOS counterparts


	// If these files aren't found, it can't be SCI
	if (!foundResMap && !foundRes000)
		return ADDetectedGame();

	ResourceManager resMan(true);
	resMan.addAppropriateSourcesForDetection(fslist);
	resMan.init();
	// TODO: Add error handling.

#ifndef ENABLE_SCI32
	// Is SCI32 compiled in? If not, and this is a SCI32 game,
	// stop here
	if (getSciVersionForDetection() >= SCI_VERSION_2)
		return ADDetectedGame();
#endif

	ViewType gameViews = resMan.getViewType();

	// Have we identified the game views? If not, stop here
	// Can't be SCI (or unsupported SCI views). Pinball Creep by Sierra also uses resource.map/resource.000 files
	// but doesn't share SCI format at all
	if (gameViews == kViewUnknown)
		return ADDetectedGame();

	// Set the platform to Amiga if the game is using Amiga views
	if (gameViews == kViewAmiga)
		s_fallbackDesc.platform = Common::kPlatformAmiga;

	// Determine the game id
	Common::String sierraGameId = resMan.findSierraGameId(s_fallbackDesc.platform == Common::kPlatformMacintosh);

	// If we don't have a game id, the game is not SCI
	if (sierraGameId.empty())
		return ADDetectedGame();

	Common::String gameId = convertSierraGameId(sierraGameId, &s_fallbackDesc.flags, resMan);
	Common::strlcpy(s_fallbackGameIdBuf, gameId.c_str(), sizeof(s_fallbackGameIdBuf));
	s_fallbackDesc.gameId = s_fallbackGameIdBuf;

	// Try to determine the game language
	// Load up text 0 and start looking for "#" characters
	// Non-English versions contain strings like XXXX#YZZZZ
	// Where XXXX is the English string, #Y a separator indicating the language
	// (e.g. #G for German) and ZZZZ is the translated text
	// NOTE: This doesn't work for games which use message instead of text resources
	// (like, for example, Eco Quest 1 and all SCI1.1 games and newer, e.g. Freddy Pharkas).
	// As far as we know, these games store the messages of each language in separate
	// resources, and it's not possible to detect that easily
	// Also look for "%J" which is used in japanese games
	Resource *text = resMan.findResource(ResourceId(kResourceTypeText, 0), false);
	uint seeker = 0;
	if (text) {
		while (seeker < text->size()) {
			if (text->getUint8At(seeker) == '#')  {
				if (seeker + 1 < text->size())
					s_fallbackDesc.language = charToScummVMLanguage(text->getUint8At(seeker + 1));
				break;
			}
			if (text->getUint8At(seeker) == '%') {
				if ((seeker + 1 < text->size()) && (text->getUint8At(seeker + 1) == 'J')) {
					s_fallbackDesc.language = charToScummVMLanguage(text->getUint8At(seeker + 1));
					break;
				}
			}
			seeker++;
		}
	}


	// Fill in "extra" field

	// Is this an EGA version that might have a VGA pendant? Then we want
	// to mark it as such in the "extra" field.
	const bool markAsEGA = (gameViews == kViewEga && s_fallbackDesc.platform != Common::kPlatformAmiga
			&& getSciVersion() > SCI_VERSION_1_EGA_ONLY);

	const bool isDemo = (s_fallbackDesc.flags & ADGF_DEMO);
	const bool isCD = (s_fallbackDesc.flags & ADGF_CD);

	if (!isCD)
		s_fallbackDesc.guiOptions = GUIO4(GUIO_NOSPEECH, GAMEOPTION_PREFER_DIGITAL_SFX, GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_MIDI_MODE);

	if (gameId.hasSuffix("sci")) {
		s_fallbackDesc.extra = "SCI";

		// Differentiate EGA versions from the VGA ones, where needed
		if (markAsEGA)
			s_fallbackDesc.extra = "SCI/EGA";

		// Mark as demo.
		// Note: This overwrites the 'EGA' info, if it was previously set.
		if (isDemo)
			s_fallbackDesc.extra = "SCI/Demo";
	} else {
		if (markAsEGA)
			s_fallbackDesc.extra = "EGA";

		// Set "CD" and "Demo" as appropriate.
		// Note: This overwrites the 'EGA' info, if it was previously set.
		if (isDemo && isCD)
			s_fallbackDesc.extra = "CD Demo";
		else if (isDemo)
			s_fallbackDesc.extra = "Demo";
		else if (isCD)
			s_fallbackDesc.extra = "CD";
	}

	return ADDetectedGame(&s_fallbackDesc);
}

} // End of namespace Sci

#if PLUGIN_ENABLED_DYNAMIC(SCI)
	REGISTER_PLUGIN_DYNAMIC(SCI, PLUGIN_TYPE_ENGINE, Sci::SciMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(SCI, PLUGIN_TYPE_ENGINE, Sci::SciMetaEngine);
#endif
