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

#include "sci/sci.h"
#include "sci/engine/kernel.h"
#include "sci/engine/seg_manager.h"
#include "sci/engine/vm.h"		// for convertSierraGameId

namespace Sci {

#define GF_FOR_SCI0_BEFORE_629 GF_SCI0_OLDGETTIME

// Titles of the games
static const PlainGameDescriptor SciGameTitles[] = {
	{"sci",             "Sierra SCI Game"},
	{"sci-fanmade",     "Fanmade SCI Game"},
	{"astrochicken",    "Astro Chicken"},
	{"christmas1988",   "Christmas Card 1988"},
	{"christmas1990",   "Christmas Card 1990: The Seasoned Professional"},
	{"christmas1992",   "Christmas Card 1992"},
	{"castlebrain",     "Castle of Dr. Brain"},
	{"iceman",          "Codename: Iceman"},
	{"camelot",         "Conquests of Camelot: King Arthur, Quest for the Grail"},
	{"longbow",         "Conquests of the Longbow: The Adventures of Robin Hood"},
	{"cnick-lsl",       "Crazy Nick's Software Picks: Leisure Suit Larry's Casino"},
	{"cnick-kq",        "Crazy Nick's Software Picks: King Graham's Board Game Challenge"},
	{"cnick-laurabow",  "Crazy Nick's Software Picks: Parlor Games with Laura Bow"},
	{"cnick-longbow",   "Crazy Nick's Software Picks: Robin Hood's Game of Skill and Chance"},
	{"cnick-sq",        "Crazy Nick's Software Picks: Roger Wilco's Spaced Out Game Pack"},
	{"ecoquest",        "EcoQuest: The Search for Cetus"},
	{"ecoquest2",       "EcoQuest II: Lost Secret of the Rainforest"},
	{"freddypharkas",   "Freddy Pharkas: Frontier Pharmacist"},
	{"funseeker",       "Fun Seeker's Guide"},
	{"hoyle1",          "Hoyle Official Book of Games: Volume 1"},
	{"hoyle2",          "Hoyle Official Book of Games: Volume 2"},
	{"hoyle3",          "Hoyle Official Book of Games: Volume 3"},
	{"hoyle4",          "Hoyle Classic Card Games"},
	{"jones",           "Jones in the Fast Lane"},
	{"kq1sci",          "King's Quest I: Quest for the Crown, SCI Remake"},
	{"kq4sci",          "King's Quest IV: The Perils of Rosella, SCI Remake"},
	{"kq5",             "King's Quest V: Absence Makes the Heart Go Yonder"},
	{"kq6",             "King's Quest VI: Heir Today, Gone Tomorrow"},
	{"laurabow",        "Laura Bow: The Colonel's Bequest"},
	{"laurabow2",       "Laura Bow 2: The Dagger of Amon Ra"},
	{"lsl1vga",         "Leisure Suit Larry in the Land of the Lounge Lizards, VGA Remake"},
	{"lsl2",            "Leisure Suit Larry 2: Goes Looking for Love (in Several Wrong Places)"},
	{"lsl3",            "Leisure Suit Larry 3: Passionate Patti in Pursuit of the Pulsating Pectorals"},
	{"lsl5",            "Leisure Suit Larry 5: Passionate Patti Does a Little Undercover Work"},
	{"lsl6",            "Leisure Suit Larry 6: Shape Up or Slip Out!"},
	{"fairytales",      "Mixed-up Fairy Tales"},
	{"mothergoose",     "Mixed-Up Mother Goose"},
	{"msastrochicken",  "Ms. Astro Chicken"},
	{"pepper",          "Pepper's Adventure in Time"},
	{"pq1vga",          "Police Quest: In Pursuit of the Death Angel, VGA Remake"},
	{"pq2",             "Police Quest II: The Vengeance"},
	{"pq3",             "Police Quest III: The Kindred"},
	{"qfg1",            "Quest for Glory I: So You Want to Be a Hero"},
	{"qfg1vga",         "Quest for Glory I: So You Want to Be a Hero, VGA Remake"},
	{"qfg2",            "Quest for Glory II: Trial by Fire"},
	{"qfg3",            "Quest for Glory III: Wages of War"},
	{"slater",          "Slater & Charlie Go Camping"},
	{"sq1vga",          "Space Quest I: The Sarien Encounter, VGA Remake"},
	{"sq3",             "Space Quest III: The Pirates of Pestulon"},
	{"sq4",             "Space Quest IV: Roger Wilco and the Time Rippers"},
	{"sq5",             "Space Quest V: The Next Mutation"},
	{"islandbrain",     "The Island of Dr. Brain"},
	// SCI32 games
	{"gk1",             "Gabriel Knight: Sins of the Fathers"},	// demo is SCI11, full version SCI32
	{"gk2",             "The Beast Within: A Gabriel Knight Mystery"},
	{"kq7",             "King's Quest VII: The Princeless Bride"},
	{"lsl7",            "Leisure Suit Larry 7: Love for Sail!"},
	{"lighthouse",      "Lighthouse: The Dark Being"},
	{"phantasmagoria",  "Phantasmagoria"},
	{"phantasmagoria2", "Phantasmagoria II: A Puzzle of Flesh"},
	{"pq4",             "Police Quest IV: Open Season"},
	{"pqswat",          "Police Quest: SWAT"},
	{"qfg4",            "Quest for Glory IV: Shadows of Darkness"},
	{"rama",            "RAMA"},
	{"shivers",         "Shivers"},
	{"shivers2",        "Shivers II: Harvest of Souls"},
	{"sq6",             "Space Quest 6: The Spinal Frontier"},
	{"torin",           "Torin's Passage"},
	{0, 0}
};

#include "sci/detection_tables.h"

/**
 * The fallback game descriptor used by the SCI engine's fallbackDetector.
 * Contents of this struct are to be overwritten by the fallbackDetector.
 */
static SciGameDescription s_fallbackDesc = {
	{
		"",
		"",
		AD_ENTRY1(0, 0), // This should always be AD_ENTRY1(0, 0) in the fallback descriptor
		Common::UNK_LANG,
		Common::kPlatformPC,
		ADGF_NO_FLAGS,
		Common::GUIO_NONE
	},
	0
};


static const ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)Sci::SciGameDescriptions,
	// Size of that superset structure
	sizeof(SciGameDescription),
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

const ADGameDescription *SciMetaEngine::fallbackDetect(const Common::FSList &fslist) const {
	bool foundResMap = false;
	bool foundRes000 = false;
	// This flag is used to determine if the size of resource.000 is less than 1MB, to distinguish
	// between full and demo versions
	bool smallResource000Size = false;

	// Set some defaults
	s_fallbackDesc.desc.extra = "";
	s_fallbackDesc.desc.language = Common::EN_ANY;
	s_fallbackDesc.desc.flags = ADGF_NO_FLAGS;
	s_fallbackDesc.desc.platform = Common::kPlatformPC;	// default to PC platform
	s_fallbackDesc.desc.gameid = "sci";

	// First grab all filenames
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory())
			continue;

		Common::String filename = file->getName();
		filename.toLowercase();

		if (filename.contains("resource.map") || filename.contains("resmap.000")) {
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
				s_fallbackDesc.desc.flags |= ADGF_CD;
				s_fallbackDesc.desc.extra = "CD";
			}
			delete tmpStream;
		}

		if (filename.contains("resource.000")) {
			Common::SeekableReadStream *tmpStream = file->createReadStream();
			if (tmpStream->size() < 1 * 1024 * 1024)
				smallResource000Size = true;
			delete tmpStream;
		}

		if (filename.contains("resource.000") || filename.contains("resource.001")
			|| filename.contains("ressci.000") || filename.contains("ressci.001"))
			foundRes000 = true;

		// Determine the game platform
		// The existence of any of these files indicates an Amiga game
		if (filename.contains("9.pat") || filename.contains("spal") ||
			filename.contains("patch.005") || filename.contains("bank.001"))
				s_fallbackDesc.desc.platform = Common::kPlatformAmiga;

		// The existence of 7.pat indicates a Mac game
		if (filename.contains("7.pat"))
			s_fallbackDesc.desc.platform = Common::kPlatformMacintosh;
	
		// The data files for Atari ST versions are the same as their DOS counterparts
	}

	// If these files aren't found, it can't be SCI
	if (!foundResMap && !foundRes000) {
		SearchMan.remove("SCI_detection");
		return 0;
	}

	ResourceManager *resourceManager = new ResourceManager(fslist);
	ViewType gameViews = resourceManager->getViewType();

	// Have we identified the game views? If not, stop here
	if (gameViews == kViewUnknown) {
		SearchMan.remove("SCI_detection");
		delete resourceManager;
		return (const ADGameDescription *)&s_fallbackDesc;
	}

#ifndef ENABLE_SCI32
	// Is SCI32 compiled in? If not, and this is a SCI32 game,
	// stop here
	if (resourceManager->sciVersion() >= SCI_VERSION_2) {
		SearchMan.remove("SCI_detection");
		delete resourceManager;
		return (const ADGameDescription *)&s_fallbackDesc;
	}
#endif

	// EGA views
	if (gameViews == kViewEga && s_fallbackDesc.desc.platform != Common::kPlatformAmiga)
		s_fallbackDesc.desc.extra = "EGA";

	// Set the platform to Amiga if the game is using Amiga views
	if (gameViews == kViewAmiga)
		s_fallbackDesc.desc.platform = Common::kPlatformAmiga;

	// Determine the game id
	SegManager *segManager = new SegManager(resourceManager);
	if (!script_instantiate(resourceManager, segManager, 0)) {
		warning("fallbackDetect(): Could not instantiate script 0");
		SearchMan.remove("SCI_detection");
		delete segManager;
		delete resourceManager;
		return 0;
	}
	reg_t game_obj = script_lookup_export(segManager, 0, 0);
	const char *gameName = obj_get_name(segManager, game_obj);
	debug(2, "Detected ID: \"%s\" at %04x:%04x", gameName, PRINT_REG(game_obj));
	s_fallbackDesc.desc.gameid = convertSierraGameId(gameName, &s_fallbackDesc.desc.flags);
	delete segManager;

	// Try to determine the game language
	// Load up text 0 and start looking for "#" characters
	// Non-English versions contain strings like XXXX#YZZZZ
	// Where XXXX is the English string, #Y a separator indicating the language
	// (e.g. #G for German) and ZZZZ is the translated text
	// NOTE: This doesn't work for games which use message instead of text resources
	// (like, for example, Eco Quest 1 and all SCI1.1 games and newer, e.g. Freddy Pharkas). 
	// As far as we know, these games store the messages of each language in separate
	// resources, and it's not possible to detect that easily
	Resource *text = resourceManager->findResource(ResourceId(kResourceTypeText, 0), 0);
	uint seeker = 0;
	if (text) {
		while (seeker < text->size) {
			if (text->data[seeker] == '#') {
				s_fallbackDesc.desc.language = charToScummVMLanguage(text->data[seeker + 1]);
				break;
			}
			seeker++;
		}
	}

	delete resourceManager;

	// Distinguish demos from full versions
	if (!strcmp(s_fallbackDesc.desc.gameid, "castlebrain") && !Common::File::exists("resource.002")) {
		// The Spanish full version doesn't have resource.002, but we can distinguish it from the
		// demo from the size of resource.000
		if (smallResource000Size)
			s_fallbackDesc.desc.flags |= ADGF_DEMO;
	}

	if (!strcmp(s_fallbackDesc.desc.gameid, "islandbrain") && smallResource000Size)
		s_fallbackDesc.desc.flags |= ADGF_DEMO;

	if (!strcmp(s_fallbackDesc.desc.gameid, "kq6") && smallResource000Size)
		s_fallbackDesc.desc.flags |= ADGF_DEMO;

	SearchMan.remove("SCI_detection");

	return (const ADGameDescription *)&s_fallbackDesc;
}

bool SciMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const {
	const SciGameDescription *desc = (const SciGameDescription *)gd;

	*engine = new SciEngine(syst, desc);

	return true;
}

} // End of namespace Sci

#if PLUGIN_ENABLED_DYNAMIC(SCI)
	REGISTER_PLUGIN_DYNAMIC(SCI, PLUGIN_TYPE_ENGINE, Sci::SciMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(SCI, PLUGIN_TYPE_ENGINE, Sci::SciMetaEngine);
#endif
