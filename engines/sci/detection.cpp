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
#include "sci/exereader.h"
#include "sci/engine/seg_manager.h"

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
	{"ecoquest",        "EcoQuest: The Search for Cetus"},
	{"ecoquest2",       "EcoQuest II: Lost Secret of the Rainforest"},
	{"freddypharkas",   "Freddy Pharkas: Frontier Pharmacist"},
	{"funseeker",       "Fun Seeker's Guide"},
	{"hoyle1",          "Hoyle Official Book of Games: Volume 1"},
	{"hoyle2",          "Hoyle Official Book of Games: Volume 2"},
	{"hoyle3",          "Hoyle Official Book of Games: Volume 3"},
	{"hoyle4",          "Hoyle Classic Card Games"},
	{"jones",           "Jones in the Fast Lane"},
	{"kq1sci",          "King's Quest I: Quest for the Crown"},
	{"kq4sci",          "King's Quest IV: The Perils of Rosella"},
	{"kq5",             "King's Quest V: Absence Makes the Heart Go Yonder"},
	{"kq6",             "King's Quest VI: Heir Today, Gone Tomorrow"},
	{"laurabow",        "Laura Bow: The Colonel's Bequest"},
	{"laurabow2",       "Laura Bow 2: The Dagger of Amon Ra"},
	{"lsl1sci",         "Leisure Suit Larry in the Land of the Lounge Lizards"},
	{"lsl2",            "Leisure Suit Larry 2: Goes Looking for Love (in Several Wrong Places)"},
	{"lsl3",            "Leisure Suit Larry 3: Passionate Patti in Pursuit of the Pulsating Pectorals"},
	{"lsl5",            "Leisure Suit Larry 5: Passionate Patti Does a Little Undercover Work"},
	{"lsl6",            "Leisure Suit Larry 6: Shape Up or Slip Out!"},
	{"lslcasino",       "Crazy Nick's Software Picks: Leisure Suit Larry's Casino"},
	{"fairytales",      "Mixed-up Fairy Tales"},
	{"mothergoose",     "Mixed-Up Mother Goose"},
	{"msastrochicken",  "Ms. Astro Chicken"},
	{"pepper",          "Pepper's Adventure in Time"},
	{"pq1sci",          "Police Quest: In Pursuit of the Death Angel"},
	{"pq2",             "Police Quest II: The Vengeance"},
	{"pq3",             "Police Quest III: The Kindred"},
	{"qfg1",            "Quest for Glory I: So You Want to Be a Hero"},
	{"qfg2",            "Quest for Glory II: Trial by Fire"},
	{"qfg3",            "Quest for Glory III: Wages of War"},
	{"slater",          "Slater & Charlie Go Camping"},
	{"sq1sci",          "Space Quest I: The Sarien Encounter"},
	{"sq3",             "Space Quest III: The Pirates of Pestulon"},
	{"sq4",             "Space Quest IV: Roger Wilco and the Time Rippers"},
	{"sq5",             "Space Quest V: The Next Mutation"},
	{"islandbrain",     "The Island of Dr. Brain"},
#ifdef ENABLE_SCI32
	{"gk1",             "Gabriel Knight: Sins of the Fathers"},
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
#endif // ENABLE_SCI32
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

Common::String convertSierraGameId(Common::String sierraId) {
	// TODO: SCI32 IDs

	// TODO: The internal id of christmas1988 is "demo"
	if (sierraId == "card") {
		// This could either be christmas1990 or christmas1992
		// christmas1990 has a "resource.001" file, whereas 
		// christmas1992 has a "resource.000" file
		return (Common::File::exists("resource.001")) ? "christmas1990" : "christmas1992";
	}
	if (sierraId == "arthur")
		return "camelot";
	if (sierraId == "brain") {
		// This could either be The Castle of Dr. Brain, or The Island of Dr. Brain
		// castlebrain has resource.001, whereas islandbrain doesn't
		return (Common::File::exists("resource.001")) ? "castlebrain" : "islandbrain";
	}
	// iceman is the same
	// longbow is the same
	if (sierraId == "eco")
		return "ecoquest";
	if (sierraId == "rain")
		return "ecoquest2";
	if (sierraId == "fp")
		return "freddypharkas";
	if (sierraId == "emc")
		return "funseeker";
	if (sierraId == "cardgames")
		return "hoyle1";
	if (sierraId == "solitare")
		return "hoyle2";
	// hoyle3 is the same
	// hoyle4 is the same
	if (sierraId == "kq1")
		return "kq1sci";
	if (sierraId == "kq4")
		return "kq4sci";
	if (sierraId == "lsl1")
		return "lsl1sci";
	// lsl2 is the same
	// lsl3 is the same
	// lsl5 is the same
	// lsl6 is the same
	// TODO: lslcasino
	if (sierraId == "tales")
		return "fairytales";
	if (sierraId == "mg")
		return "mothergoose";
	if (sierraId == "cb1")
		return "laurabow";
	if (sierraId == "lb2")
		return "laurabow2";
	// TODO: lb2 floppy (its resources can't be read)
	if (sierraId == "twisty")
		return "pepper";
	// TODO: pq1sci (its resources can't be read)
	if (sierraId == "pq")
		return "pq2";
	// pq3 is the same
	if (sierraId == "glory")
		return "qfg1";
	// TODO: qfg1 VGA (its resources can't be read)
	if (sierraId == "trial")
		return "qfg2";
	if (sierraId == "qfg1")
		return "qfg3";
	if (sierraId == "thegame")
		return "slater";
	if (sierraId == "sq1")
		return "sq1sci";
	if (sierraId == "sq3") {
		// Both SQ3 and the separately released subgame, Astro Chicken,
		// have internal ID "sq3", but Astro Chicken only has "resource.map"
		// and "resource.001". Detect if it's SQ3 by the existence of
		// "resource.002"
		return (Common::File::exists("resource.002")) ? "sq3" : "astrochicken";
	}
	if (sierraId == "sq4") {
		// Both SQ4 and the separately released subgame, Ms. Astro Chicken,
		// have internal ID "sq4", but Astro Chicken only has "resource.map"
		// and "resource.001". Detect if it's SQ4 by the existence of
		// "resource.000" (which exists in both SQ4 floppy and CD, but not in
		// the subgame)
		return (Common::File::exists("resource.000")) ? "sq4" : "msastrochicken";
	}
	// sq4 is the same
	// sq5 is the same

	return sierraId;
}

const ADGameDescription *SciMetaEngine::fallbackDetect(const Common::FSList &fslist) const {
	bool foundResMap = false;
	bool foundRes000 = false;
	Common::Platform exePlatform = Common::kPlatformUnknown;
	Common::String exeVersionString;

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

		if (filename.contains("resource.000") || filename.contains("resource.001")
			|| filename.contains("ressci.000") || filename.contains("ressci.001"))
			foundRes000 = true;

		// Check if it's a known executable name
		// Note: "sier" matches "sier.exe", "sierra.exe", "sierw.exe" and "sierw5.exe"
		if (filename.contains("scidhuv") || filename.contains("sciduv") ||
			filename.contains("sciv") || filename.contains("sciw") ||
			filename.contains("prog") || filename.contains("sier")) {

			// We already found a valid exe, no need to check this one.
			if (!exeVersionString.empty())
				continue;

			// Is it really an executable file?
			Common::SeekableReadStream *fileStream = file->createReadStream();
			exePlatform = getGameExePlatform(fileStream);

			// It's a valid exe, read the interpreter version string
			if (exePlatform != Common::kPlatformUnknown)
				exeVersionString = readSciVersionFromExe(fileStream, exePlatform);

			delete fileStream;
		}
	}

	// If these files aren't found, it can't be SCI
	if (!foundResMap && !foundRes000) {
		SearchMan.remove("SCI_detection");
		return 0;
	}

	// Set some defaults
	s_fallbackDesc.desc.extra = "";
	s_fallbackDesc.desc.language = Common::UNK_LANG;
	s_fallbackDesc.desc.platform = exePlatform;
	s_fallbackDesc.desc.flags = ADGF_NO_FLAGS;

	// Determine the game id
	ResourceManager *resMgr = new ResourceManager(fslist);
	SciVersion version = resMgr->sciVersion();
	SegManager *segManager = new SegManager(resMgr, version);
	if (!script_instantiate(resMgr, segManager, version, 0)) {
		warning("fallbackDetect(): Could not instantiate script 0");
		SearchMan.remove("SCI_detection");
		return 0;
	}
	reg_t game_obj = script_lookup_export(segManager, 0, 0);
	Common::String gameName = obj_get_name(segManager, version, game_obj);
	debug(2, "Detected ID: \"%s\" at %04x:%04x", gameName.c_str(), PRINT_REG(game_obj));
	gameName.toLowercase();
	s_fallbackDesc.desc.gameid = strdup(convertSierraGameId(gameName).c_str());
	delete segManager;
	delete resMgr;

	printf("If this is *NOT* a fan-modified version (in particular, not a fan-made\n");
	printf("translation), please, report the data above, including the following\n");
	printf("version number, from the game's executable:\n");
	printf("Version: %s\n\n", exeVersionString.empty() ? "not found" : exeVersionString.c_str());

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
