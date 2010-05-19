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

#include "engines/advancedDetector.h"
#include "base/plugins.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"
#include "graphics/thumbnail.h"

#include "sci/sci.h"
#include "sci/engine/kernel.h"
#include "sci/engine/savegame.h"
#include "sci/engine/script.h"
#include "sci/engine/seg_manager.h"
#include "sci/engine/state.h"
#include "sci/engine/vm.h"		// for convertSierraGameId

namespace Sci {

// Titles of the games
static const PlainGameDescriptor SciGameTitles[] = {
	{"sci",             "Sierra SCI Game"},
	{"sci-fanmade",     "Fanmade SCI Game"},
	// === SCI0 games =========================================================
	{"astrochicken",    "Astro Chicken"},
	{"christmas1988",   "Christmas Card 1988"},
	{"iceman",          "Codename: Iceman"},
	{"camelot",         "Conquests of Camelot: King Arthur, Quest for the Grail"},
	{"funseeker",       "Fun Seeker's Guide"},
	{"hoyle1",          "Hoyle Official Book of Games: Volume 1"},
	{"hoyle2",          "Hoyle Official Book of Games: Volume 2"},
	{"kq4sci",          "King's Quest IV: The Perils of Rosella, SCI Remake"},
	{"laurabow",        "Laura Bow: The Colonel's Bequest"},
	{"lsl2",            "Leisure Suit Larry 2: Goes Looking for Love (in Several Wrong Places)"},
	{"lsl3",            "Leisure Suit Larry 3: Passionate Patti in Pursuit of the Pulsating Pectorals"},
	{"pq2",             "Police Quest II: The Vengeance"},
	{"qfg1",            "Quest for Glory I: So You Want to Be a Hero"},	// EGA is SCI0, VGA SCI1.1
	{"sq3",             "Space Quest III: The Pirates of Pestulon"},
	// === SCI01 games ========================================================
	{"qfg2",            "Quest for Glory II: Trial by Fire"},
	{"kq1sci",          "King's Quest I: Quest for the Crown, SCI Remake"},
	// === SCI1 games =========================================================
	{"castlebrain",     "Castle of Dr. Brain"},
	{"christmas1990",   "Christmas Card 1990: The Seasoned Professional"},
	{"cnick-lsl",       "Crazy Nick's Software Picks: Leisure Suit Larry's Casino"},
	{"cnick-kq",        "Crazy Nick's Software Picks: King Graham's Board Game Challenge"},
	{"cnick-laurabow",  "Crazy Nick's Software Picks: Parlor Games with Laura Bow"},
	{"cnick-longbow",   "Crazy Nick's Software Picks: Robin Hood's Game of Skill and Chance"},
	{"cnick-sq",        "Crazy Nick's Software Picks: Roger Wilco's Spaced Out Game Pack"},
	{"ecoquest",        "EcoQuest: The Search for Cetus"},	// floppy is SCI1, CD SCI1.1
	{"fairytales",      "Mixed-up Fairy Tales"},
	{"hoyle3",          "Hoyle Official Book of Games: Volume 3"},
	{"jones",           "Jones in the Fast Lane"},
	{"kq5",             "King's Quest V: Absence Makes the Heart Go Yonder"},
	{"longbow",         "Conquests of the Longbow: The Adventures of Robin Hood"},
	{"lsl1sci",         "Leisure Suit Larry in the Land of the Lounge Lizards"},
	{"lsl5",            "Leisure Suit Larry 5: Passionate Patti Does a Little Undercover Work"},
	{"msastrochicken",  "Ms. Astro Chicken"},
	{"pq1sci",          "Police Quest: In Pursuit of the Death Angel"},
	{"pq3",             "Police Quest III: The Kindred"},
	{"sq1sci",          "Space Quest I: The Sarien Encounter"},
	{"sq4",             "Space Quest IV: Roger Wilco and the Time Rippers"},	// floppy is SCI1, CD SCI1.1
	// === SCI1.1 games =======================================================
	{"christmas1992",   "Christmas Card 1992"},
	{"ecoquest2",       "EcoQuest II: Lost Secret of the Rainforest"},
	{"freddypharkas",   "Freddy Pharkas: Frontier Pharmacist"},
	{"hoyle4",          "Hoyle Classic Card Games"},
	{"kq6",             "King's Quest VI: Heir Today, Gone Tomorrow"},
	{"laurabow2",       "Laura Bow 2: The Dagger of Amon Ra"},
	{"qfg3",            "Quest for Glory III: Wages of War"},
	{"sq5",             "Space Quest V: The Next Mutation"},
	{"islandbrain",     "The Island of Dr. Brain"},
	{"lsl6",            "Leisure Suit Larry 6: Shape Up or Slip Out!"},
	{"mothergoose",     "Mixed-Up Mother Goose"},	// floppy is SCI1.1, CD SCI2.1
	{"pepper",          "Pepper's Adventure in Time"},
	{"slater",          "Slater & Charlie Go Camping"},
	// === SCI2 games =========================================================
	{"gk1",             "Gabriel Knight: Sins of the Fathers"},	// demo is SCI11, full version SCI32
	{"pq4",             "Police Quest IV: Open Season"}, // floppy is SCI2, CD SCI2.1
	{"qfg4",            "Quest for Glory IV: Shadows of Darkness"},	// floppy is SCI2, CD SCI2.1
	// === SCI2.1 games ========================================================
	{"gk2",             "The Beast Within: A Gabriel Knight Mystery"},
	// TODO: Inside The Chest/Behind the Developer's Shield
	{"kq7",             "King's Quest VII: The Princeless Bride"},
	// TODO: King's Questions
	{"phantasmagoria",  "Phantasmagoria"},
	{"pqswat",          "Police Quest: SWAT"},
	{"shivers",         "Shivers"},
	{"sq6",             "Space Quest 6: The Spinal Frontier"},
	{"torin",           "Torin's Passage"},
	// === SCI3 games =========================================================
	{"lsl7",            "Leisure Suit Larry 7: Love for Sail!"},
	{"lighthouse",      "Lighthouse: The Dark Being"},
	{"phantasmagoria2", "Phantasmagoria II: A Puzzle of Flesh"},
	{"shivers2",        "Shivers II: Harvest of Souls"},
	{"rama",            "RAMA"},
	{0, 0}
};

#include "sci/detection_tables.h"

/**
 * The fallback game descriptor used by the SCI engine's fallbackDetector.
 * Contents of this struct are overwritten by the fallbackDetector.
 */
static ADGameDescription s_fallbackDesc = {
	"",
	"",
	AD_ENTRY1(0, 0), // This should always be AD_ENTRY1(0, 0) in the fallback descriptor
	Common::UNK_LANG,
	Common::kPlatformPC,
	ADGF_NO_FLAGS,
	Common::GUIO_NONE
};

static char s_fallbackGameIdBuf[256];


static const ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)Sci::SciGameDescriptions,
	// Size of that superset structure
	sizeof(ADGameDescription),
	// Number of bytes to compute MD5 sum for
	5000,
	// List of all engine targets
	SciGameTitles,
	// Structure for autoupgrading obsolete targets
	0,
	// Name of single gameid (optional)
	"sci",
	// List of files for file-based fallback detection (optional)
	0,
	// Flags
	0,
	// Additional GUI options (for every game}
	Common::GUIO_NONE
};

class SciMetaEngine : public AdvancedMetaEngine {
public:
	SciMetaEngine() : AdvancedMetaEngine(detectionParams) {}

	virtual const char *getName() const {
		return "SCI Engine [SCI0, SCI01, SCI10, SCI11"
#ifdef ENABLE_SCI32
			", SCI32"
#endif
			"]";
	}

	virtual const char *getOriginalCopyright() const {
		return "Sierra's Creative Interpreter (C) Sierra Online";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const;
	const ADGameDescription *fallbackDetect(const Common::FSList &fslist) const;
	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	virtual void removeSaveState(const char *target, int slot) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
};

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

#define READ_UINT16(buf) (!resMan->isSci11Mac() ? READ_LE_UINT16(buf) : READ_BE_UINT16(buf))

// Finds the internal ID of the current game from script 0
Common::String getSierraGameId(ResourceManager *resMan) {
	Resource *script = resMan->findResource(ResourceId(kResourceTypeScript, 0), false);
	Script *script000 = new Script();
	script000->init(0, resMan);
	script000->mcpyInOut(0, script->data, script->size);
	uint16 curOffset = (getSciVersion() == SCI_VERSION_0_EARLY) ? 2 : 0;
	uint16 objLength = 0;
	int objType = 0;
	int16 exportsOffset = 0;
	Common::String sierraId;

	// Now find the export table of the script
	do {
		objType = READ_UINT16(script000->_buf + curOffset);
		if (!objType)
			break;
	
		objLength = READ_UINT16(script000->_buf + curOffset + 2);
		curOffset += 4;		// skip header

		if (objType == SCI_OBJ_EXPORTS) {
			exportsOffset = READ_UINT16(script000->_buf + curOffset + 2);
			break;
		}
	} while (objType != 0);

	// The game object is the first export. Script 0 is always at segment 1
	reg_t gameObj = make_reg(1, exportsOffset);

	// TODO: stop using the segment manager and read the object name here
	SegManager *segMan = new SegManager(resMan);
	script_instantiate(resMan, segMan, 0);
	sierraId = segMan->getObjectName(gameObj);
	delete segMan;

	delete script000;

	return sierraId;
}

const ADGameDescription *SciMetaEngine::fallbackDetect(const Common::FSList &fslist) const {
	bool foundResMap = false;
	bool foundRes000 = false;

	// Set some defaults
	s_fallbackDesc.extra = "";
	s_fallbackDesc.language = Common::EN_ANY;
	s_fallbackDesc.flags = ADGF_NO_FLAGS;
	s_fallbackDesc.platform = Common::kPlatformPC;	// default to PC platform
	s_fallbackDesc.gameid = "sci";

	// First grab all filenames
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory())
			continue;

		Common::String filename = file->getName();
		filename.toLowercase();

		if (filename.contains("resource.map") || filename.contains("resmap.00") || filename.contains("Data1")) {
			// HACK: resource.map is located in the same directory as the other resource files,
			// therefore add the directory here, so that the game files can be opened later on
			// We now add the parent directory temporary to our SearchMan so the engine code
			// used in the detection can access all files via Common::File without any problems.
			// In all branches returning from this function, we need to have a call to
			// SearchMan.remove to remove it from the default directory pool again.
			//
			// A proper solution to remove this hack would be to have the code, which is needed
			// for detection, to operate on Stream objects, so they can be easily called from
			// the detection code. This might be easily to achieve through refactoring the
			// code needed for detection.
			assert(!SearchMan.hasArchive("SCI_detection"));
			SearchMan.addDirectory("SCI_detection", file->getParent());
			foundResMap = true;
		}

		// Determine if we got a CD version and set the CD flag accordingly, by checking for
		// resource.aud for SCI1.1 CD games, or audio001.002 for SCI1 CD games. We assume that
		// the file should be over 10MB, as it contains all the game speech and is usually
		// around 450MB+. The size check is for some floppy game versions like KQ6 floppy, which
		// also have a small resource.aud file
		if (filename.contains("resource.aud") || filename.contains("audio001.002")) {
			Common::SeekableReadStream *tmpStream = file->createReadStream();
			if (tmpStream->size() > 10 * 1024 * 1024) {
				// We got a CD version, so set the CD flag accordingly
				s_fallbackDesc.flags |= ADGF_CD;
				s_fallbackDesc.extra = "CD";
			}
			delete tmpStream;
		}

		if (filename.contains("resource.000") || filename.contains("resource.001")
			|| filename.contains("ressci.000") || filename.contains("ressci.001"))
			foundRes000 = true;

		// Data1 contains both map and volume for SCI1.1+ Mac games
		if (filename.contains("Data1")) {
			foundResMap = foundRes000 = true;
			 s_fallbackDesc.platform = Common::kPlatformMacintosh;
		}

		// Determine the game platform
		// The existence of any of these files indicates an Amiga game
		if (filename.contains("9.pat") || filename.contains("spal") ||
			filename.contains("patch.005") || filename.contains("bank.001"))
				s_fallbackDesc.platform = Common::kPlatformAmiga;

		// The existence of 7.pat indicates a Mac game
		if (filename.contains("7.pat"))
			s_fallbackDesc.platform = Common::kPlatformMacintosh;

		// The data files for Atari ST versions are the same as their DOS counterparts
	}

	// If these files aren't found, it can't be SCI
	if (!foundResMap && !foundRes000) {
		SearchMan.remove("SCI_detection");
		return 0;
	}

	ResourceManager *resMan = new ResourceManager(fslist);
	ViewType gameViews = resMan->getViewType();

	// Have we identified the game views? If not, stop here
	if (gameViews == kViewUnknown) {
		SearchMan.remove("SCI_detection");
		delete resMan;
		// Can't be SCI (or unsupported SCI views). Pinball Creep by sierra also uses resource.map/resource.000 files
		//  but doesnt share sci format at all, if we dont return 0 here we will detect this game as SCI
		return 0;
	}

#ifndef ENABLE_SCI32
	// Is SCI32 compiled in? If not, and this is a SCI32 game,
	// stop here
	if (getSciVersion() >= SCI_VERSION_2) {
		SearchMan.remove("SCI_detection");
		delete resMan;
		return (const ADGameDescription *)&s_fallbackDesc;
	}
#endif

	// EGA views
	if (gameViews == kViewEga && s_fallbackDesc.platform != Common::kPlatformAmiga)
		s_fallbackDesc.extra = "EGA";

	// Set the platform to Amiga if the game is using Amiga views
	if (gameViews == kViewAmiga)
		s_fallbackDesc.platform = Common::kPlatformAmiga;

	// Determine the game id
	Common::String gameId = convertSierraGameId(getSierraGameId(resMan).c_str(), &s_fallbackDesc.flags, resMan);
	strncpy(s_fallbackGameIdBuf, gameId.c_str(), sizeof(s_fallbackGameIdBuf) - 1);
	s_fallbackGameIdBuf[sizeof(s_fallbackGameIdBuf) - 1] = 0;	// Make sure string is NULL terminated
	s_fallbackDesc.gameid = s_fallbackGameIdBuf;

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
	Resource *text = resMan->findResource(ResourceId(kResourceTypeText, 0), 0);
	uint seeker = 0;
	if (text) {
		while (seeker < text->size) {
			if (text->data[seeker] == '#')  {
				if (seeker + 1 < text->size)
					s_fallbackDesc.language = charToScummVMLanguage(text->data[seeker + 1]);
				break;
			}
			if (text->data[seeker] == '%') {
				if ((seeker + 1 < text->size) && (text->data[seeker + 1] == 'J')) {
					s_fallbackDesc.language = charToScummVMLanguage(text->data[seeker + 1]);
					break;
				}
			}
			seeker++;
		}
	}

	delete resMan;

	// Fill in extras field
	if (!strcmp(s_fallbackDesc.gameid, "lsl1sci") ||
		!strcmp(s_fallbackDesc.gameid, "pq1sci") ||
		!strcmp(s_fallbackDesc.gameid, "sq1sci"))
		s_fallbackDesc.extra = "VGA Remake";

	if (!strcmp(s_fallbackDesc.gameid, "qfg1") && !Common::File::exists("resource.001"))
		s_fallbackDesc.extra = "VGA Remake";

	// Add "demo" to the description for demos
	if (s_fallbackDesc.flags & ADGF_DEMO)
		s_fallbackDesc.extra = "demo";

	SearchMan.remove("SCI_detection");

	return (const ADGameDescription *)&s_fallbackDesc;
}

bool SciMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const {
	const ADGameDescription *desc = (const ADGameDescription *)gd;

	*engine = new SciEngine(syst, desc);

	return true;
}

bool SciMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate);
}

bool SciEngine::hasFeature(EngineFeature f) const {
	return
		//(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime);
		//(f == kSupportsSavingDuringRuntime);
}

SaveStateList SciMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += ".???";

	filenames = saveFileMan->listSavefiles(pattern);
	sort(filenames.begin(), filenames.end());	// Sort (hopefully ensuring we are sorted numerically..)

	SaveStateList saveList;
	int slotNum = 0;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		slotNum = atoi(file->c_str() + file->size() - 3);

		if (slotNum >= 0 && slotNum < 999) {
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				SavegameMetadata meta;
				if (!get_savegame_metadata(in, &meta)) {
					// invalid
					delete in;
					continue;
				}
				saveList.push_back(SaveStateDescriptor(slotNum, meta.savegame_name));
				delete in;
			}
		}
	}

	return saveList;
}

SaveStateDescriptor SciMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String fileName = Common::String::printf("%s.%03d", target, slot);
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(fileName);

	if (in) {
		SavegameMetadata meta;
		if (!get_savegame_metadata(in, &meta)) {
			// invalid
			delete in;

			SaveStateDescriptor desc(slot, "Invalid");
			return desc;
		}

		SaveStateDescriptor desc(slot, meta.savegame_name);

		Graphics::Surface *thumbnail = new Graphics::Surface();
		assert(thumbnail);
		if (!Graphics::loadThumbnail(*in, *thumbnail)) {
			delete thumbnail;
			thumbnail = 0;
		}

		desc.setThumbnail(thumbnail);

		desc.setDeletableFlag(true);
		desc.setWriteProtectedFlag(false);

		int day = (meta.savegame_date >> 24) & 0xFF;
		int month = (meta.savegame_date >> 16) & 0xFF;
		int year = meta.savegame_date & 0xFFFF;

		desc.setSaveDate(year, month, day);

		int hour = (meta.savegame_time >> 16) & 0xFF;
		int minutes = (meta.savegame_time >> 8) & 0xFF;

		desc.setSaveTime(hour, minutes);

		// TODO: played time

		delete in;

		return desc;
	}

	return SaveStateDescriptor();
}

int SciMetaEngine::getMaximumSaveSlot() const { return 999; }

void SciMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::printf("%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(fileName);
}

Common::Error SciEngine::loadGameState(int slot) {
	Common::String fileName = Common::String::printf("%s.%03d", _targetName.c_str(), slot);
	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
	Common::SeekableReadStream *in = saveFileMan->openForLoading(fileName);

	if (in) {
		// found a savegame file
		gamestate_restore(_gamestate, in);
		delete in;
	}

	if (_gamestate->r_acc != make_reg(0, 1)) {
		return Common::kNoError;
	} else {
		warning("Restoring gamestate '%s' failed", fileName.c_str());
		return Common::kUnknownError;
	}
}

Common::Error SciEngine::saveGameState(int slot, const char *desc) {
	Common::String fileName = Common::String::printf("%s.%03d", _targetName.c_str(), slot);
	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
	Common::OutSaveFile *out = saveFileMan->openForSaving(fileName);
	const char *version = "";
	if (!out) {
		warning("Opening savegame \"%s\" for writing failed", fileName.c_str());
		return Common::kWritingFailed;
	}

	if (gamestate_save(_gamestate, out, desc, version)) {
		warning("Saving the game state to '%s' failed", fileName.c_str());
		return Common::kWritingFailed;
	} else {
		out->finalize();
		if (out->err()) {
			warning("Writing the savegame failed.");
			return Common::kWritingFailed;
		}
		delete out;
	}

	return Common::kNoError;
}

bool SciEngine::canLoadGameStateCurrently() {
	return !_gamestate->execution_stack_base;
}

bool SciEngine::canSaveGameStateCurrently() {
	return !_gamestate->execution_stack_base;
}

} // End of namespace Sci

#if PLUGIN_ENABLED_DYNAMIC(SCI)
	REGISTER_PLUGIN_DYNAMIC(SCI, PLUGIN_TYPE_ENGINE, Sci::SciMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(SCI, PLUGIN_TYPE_ENGINE, Sci::SciMetaEngine);
#endif
