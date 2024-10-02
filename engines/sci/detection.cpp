/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/advancedDetector.h"
#include "base/plugins.h"
#include "common/file.h"
#include "common/hashmap.h"
#include "common/ptr.h"

#include "gui/ThemeEval.h"
#include "gui/widget.h"
#include "gui/widgets/popup.h"

#include "sci/detection.h"
#include "sci/detection_internal.h"
#include "sci/dialogs.h"
#include "sci/graphics/helpers_detection_enums.h"
#include "sci/sci.h"

static const DebugChannelDef debugFlagList[] = {
	{Sci::kDebugLevelError, "Error", "Script error debugging"},
	{Sci::kDebugLevelNodes, "Lists", "Lists and nodes debugging"},
	{Sci::kDebugLevelGraphics, "Graphics", "Graphics debugging"},
	{Sci::kDebugLevelStrings, "Strings", "Strings debugging"},
	{Sci::kDebugLevelMemory, "Memory", "Memory debugging"},
	{Sci::kDebugLevelFuncCheck, "Func", "Function parameter debugging"},
	{Sci::kDebugLevelBresen, "Bresenham", "Bresenham algorithms debugging"},
	{Sci::kDebugLevelSound, "Sound", "Sound debugging"},
	{Sci::kDebugLevelBaseSetter, "Base", "Base Setter debugging"},
	{Sci::kDebugLevelParser, "Parser", "Parser debugging"},
	{Sci::kDebugLevelSaid, "Said", "Said specs debugging"},
	{Sci::kDebugLevelFile, "File", "File I/O debugging"},
	{Sci::kDebugLevelTime, "Time", "Time debugging"},
	{Sci::kDebugLevelRoom, "Room", "Room number debugging"},
	{Sci::kDebugLevelAvoidPath, "Pathfinding", "Pathfinding debugging"},
	{Sci::kDebugLevelDclInflate, "DCL", "DCL inflate debugging"},
	{Sci::kDebugLevelVM, "VM", "VM debugging"},
	{Sci::kDebugLevelScripts, "Scripts", "Notifies when scripts are unloaded"},
	{Sci::kDebugLevelPatcher, "Patcher", "Notifies when scripts or resources are patched"},
	{Sci::kDebugLevelWorkarounds, "Workarounds", "Notifies when workarounds are triggered"},
	{Sci::kDebugLevelVideo, "Video", "Video (SEQ, VMD, RBT) debugging"},
	{Sci::kDebugLevelGame, "Game", "Debug calls from game scripts"},
	{Sci::kDebugLevelGC, "GC", "Garbage Collector debugging"},
	{Sci::kDebugLevelResMan, "ResMan", "Resource manager debugging"},
	{Sci::kDebugLevelOnStartup, "OnStartup", "Enter debugger at start of game"},
	{Sci::kDebugLevelDebugMode, "DebugMode", "Enable game debug mode at start of game"},
	DEBUG_CHANNEL_END
};

namespace Sci {

// Titles of the games
static const PlainGameDescriptor s_sciGameTitles[] = {
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
	{"kq4sci",          "King's Quest IV: The Perils of Rosella"},	// Note: There was also an AGI version of this
	{"laurabow",        "Laura Bow I: The Colonel's Bequest"},
	{"lsl2",            "Leisure Suit Larry 2: Goes Looking for Love (in Several Wrong Places)"},
	{"lsl3",            "Leisure Suit Larry 3: Passionate Patti in Pursuit of the Pulsating Pectorals"},
	{"mothergoose",     "Mixed-Up Mother Goose"},
	{"pq2",             "Police Quest II: The Vengeance"},
	{"qfg1",            "Hero's Quest: So You Want to Be a Hero"},	// Note: There was also a SCI11 VGA remake of this (further down) called Quest for Glory I: So You Want to Be a Hero
	{"sq3",             "Space Quest III: The Pirates of Pestulon"},
	// === SCI01 games ========================================================
	{"qfg2",            "Quest for Glory II: Trial by Fire"},
	{"kq1sci",          "King's Quest I: Quest for the Crown"},	// Note: There was also an AGI version of this called King's Quest: Quest for the Crown
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
	{"lsl1sci",         "Leisure Suit Larry in the Land of the Lounge Lizards"},	// Note: There was also an AGI version of this
	{"lsl5",            "Leisure Suit Larry 5: Passionate Patti Does a Little Undercover Work"},
	{"mothergoose256",  "Mixed-Up Mother Goose"},
	{"msastrochicken",  "Ms. Astro Chicken"},
	{"pq1sci",          "Police Quest: In Pursuit of the Death Angel"},	// Note: There was also an AGI version of this
	{"pq3",             "Police Quest III: The Kindred"},
	{"sq1sci",          "Space Quest I: Roger Wilco in the Sarien Encounter"},	// Note: There was also an AGI version of this called Space Quest: Chapter I - The Sarien Encounter
	{"sq4",             "Space Quest IV: Roger Wilco and the Time Rippers"},	// floppy is SCI1, CD SCI1.1
	// === SCI1.1 games =======================================================
	{"christmas1992",   "Christmas Card 1992"},
	{"ecoquest2",       "EcoQuest II: Lost Secret of the Rainforest"},
	{"freddypharkas",   "Freddy Pharkas: Frontier Pharmacist"},
	{"hoyle4",          "Hoyle Classic Card Games"},
	{"inndemo",         "ImagiNation Network (INN) Demo"},
	{"kq6",             "King's Quest VI: Heir Today, Gone Tomorrow"},
	{"laurabow2",       "Laura Bow II: The Dagger of Amon Ra"},
	{"qfg1vga",         "Quest for Glory I: So You Want to Be a Hero"},	// Note: There was also a SCI0 version of this (further up) called Hero's Quest: So You Want to Be a Hero
	{"qfg3",            "Quest for Glory III: Wages of War"},
	{"sq5",             "Space Quest V: The Next Mutation"},
	{"islandbrain",     "The Island of Dr. Brain"},
	{"lsl6",            "Leisure Suit Larry 6: Shape Up or Slip Out!"},
	{"pepper",          "Pepper's Adventures in Time"},
	{"slater",          "Slater & Charlie Go Camping"},
	{"gk1demo",         "Gabriel Knight: Sins of the Fathers"},
	{"qfg4demo",        "Quest for Glory IV: Shadows of Darkness"},
	{"pq4demo",         "Police Quest IV: Open Season"},
	// === SCI2 games =========================================================
	{"gk1",             "Gabriel Knight: Sins of the Fathers"},
	{"pq4",             "Police Quest IV: Open Season"}, // floppy is SCI2, CD SCI2.1
	{"qfg4",            "Quest for Glory IV: Shadows of Darkness"},	// floppy is SCI2, CD SCI2.1
	// === SCI2.1 games ========================================================
	{"hoyle5",          "Hoyle Classic Games"},
	{"hoyle5bridge",    "Hoyle Bridge"},
	{"hoyle5children",  "Hoyle Children's Collection"},
	{"hoyle5school",    "Hoyle School House Math"},
	{"hoyle5solitaire", "Hoyle Solitaire"},
	{"chest",           "Inside the Chest"},	// aka Behind the Developer's Shield
	{"gk2",             "The Beast Within: A Gabriel Knight Mystery"},
	{"kq7",             "King's Quest VII: The Princeless Bride"},
	{"kquestions",      "King's Questions"},
	{"lsl6hires",       "Leisure Suit Larry 6: Shape Up or Slip Out!"},
	{"mothergoosehires","Mixed-Up Mother Goose Deluxe"},
	{"phantasmagoria",  "Phantasmagoria"},
	{"pqswat",          "Police Quest: SWAT"},
	{"realm",           "The Realm"},
	{"shivers",         "Shivers"},
	{"sq6",             "Space Quest 6: The Spinal Frontier"},
	{"torin",           "Torin's Passage"},
	// === SCI3 games =========================================================
	{"lsl7",            "Leisure Suit Larry 7: Love for Sail!"},
	{"lighthouse",      "Lighthouse: The Dark Being"},
	{"phantasmagoria2", "Phantasmagoria 2: A Puzzle of Flesh"},
	{"shivers2",        "Shivers II: Harvest of Souls"},	// Not SCI
	{"rama",            "RAMA"},
	{nullptr, nullptr}
};

} // End of namespace Sci

#include "sci/detection_tables.h"

namespace Sci {

static const char *const directoryGlobs[] = {
	"avi",
	"english",
	"french",
	"german",
	"italian",
	"msg",
	"spanish",
	"patches",
	nullptr
};

class SciMetaEngineDetection : public AdvancedMetaEngineDetection<ADGameDescription> {
public:
	SciMetaEngineDetection() : AdvancedMetaEngineDetection(Sci::SciGameDescriptions, s_sciGameTitles) {
		_maxScanDepth = 3;
		_directoryGlobs = directoryGlobs;
		// Use SCI fallback detection results instead of the partial matches found by
		// advanced detector. SCI fallback detection is excellent because games have
		// predictable file names and contain a unique game string.
		// Advanced detector's partial matches aren't very useful in SCI because of
		// those similar file names; most games are partial matches of each other.
		_flags = kADFlagMatchFullPaths | kADFlagPreferFallbackDetection;
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}

	const char *getName() const override {
		return "sci";
	}

	const char *getEngineName() const override {
		return "SCI ["
#ifdef ENABLE_SCI32
			"all games"
#else
			"SCI0, SCI01, SCI10, SCI11"
#endif
			"]";
	}

	const char *getOriginalCopyright() const override {
		return "Sierra's Creative Interpreter (C) Sierra Online";
	}

	DetectedGames detectGames(const Common::FSList &fslist, uint32 skipADFlags, bool skipIncomplete) override;

	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist, ADDetectedGameExtraInfo **extra) const override;

private:
	void addFileToDetectedGame(const Common::Path &name, const FileMap &allFiles, MD5Properties md5Prop, ADDetectedGame &game) const;
};

DetectedGames SciMetaEngineDetection::detectGames(const Common::FSList &fslist, uint32 skipADFlags, bool skipIncomplete) {
	DetectedGames games = AdvancedMetaEngineDetection::detectGames(fslist, skipADFlags, skipIncomplete);

	for (DetectedGame &game : games) {
		const GameIdStrToEnum *g = gameIdStrToEnum;
		for (; g->gameidStr; ++g) {
			if (game.gameId.equals(g->gameidStr))
				break;
		}
		game.setGUIOptions(customizeGuiOptions(fslist.begin()->getParent().getPath(), parseGameGUIOptions(game.getGUIOptions()), game.platform, g->gameidStr, g->version));
		game.appendGUIOptions(getGameGUIOptionsDescriptionLanguage(game.language));
	}

	return games;
}

ADDetectedGame SciMetaEngineDetection::fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist, ADDetectedGameExtraInfo **extra) const {
	/**
	 * Fallback detection for Sci heavily depends on engine resources, so it's not possible
	 * to use them without the engine present in a clean way.
	 */

	if (ConfMan.hasKey("always_run_fallback_detection_extern")) {
		if (ConfMan.getBool("always_run_fallback_detection_extern") == false) {
			warning("SCI: Fallback detection is disabled");
			return ADDetectedGame();
		}
	}

	const Plugin *enginePlugin = PluginMan.findEnginePlugin(getName());
	if (!enginePlugin) {
		static bool warn = true;
		if (warn) {
			warning("Engine plugin for SCI not present. Fallback detection is disabled");
			warn = false;
		}
		return ADDetectedGame();
	}

	ADDetectedGame game = enginePlugin->get<AdvancedMetaEngineBase>().fallbackDetectExtern(_md5Bytes, allFiles, fslist);
	if (!game.desc) {
		return game;
	}

	// detect all the matched files here in SciMetaEngineDetection, instead of
	// external fallback detection, so that we can use AdvancedMetaEngineDetection
	// methods instead of duplicating code. fallback detection has identified the
	// game, platform, and language. now we want a full list of all resource map
	// and volume files in the directory. this code attempts to add all possible
	// files, even if there are gaps, because we want to be able to identify
	// incomplete directories when users submit unknown-game reports.
	MD5Properties md5Prop = kMD5Head;
	if (allFiles.contains("resource.map")) {
		// add the map and volumes
		addFileToDetectedGame("resource.map", allFiles, md5Prop, game);
		for (int i = 0; i <= 11; i++) {
			Common::String volume = Common::String::format("resource.%03d", i);
			addFileToDetectedGame(Common::Path(volume, '/'), allFiles, md5Prop, game);
		}

		// add message and audio volumes.
		// sometimes we need these to differentiate between localized versions.
		addFileToDetectedGame("resource.aud", allFiles, md5Prop, game);
		addFileToDetectedGame("resource.msg", allFiles, md5Prop, game);
	} else if (allFiles.contains("resmap.000") || allFiles.contains("resmap.001")) {
		// add maps and volumes
		for (int i = 0; i <= 7; i++) {
			Common::String map = Common::String::format("resmap.%03d", i);
			Common::String volume = Common::String::format("ressci.%03d", i);
			addFileToDetectedGame(Common::Path(map), allFiles, md5Prop, game);
			addFileToDetectedGame(Common::Path(volume), allFiles, md5Prop, game);
		}
	} else if (allFiles.contains("Data1")) {
		// add Mac volumes
		md5Prop = (MD5Properties)(md5Prop | kMD5MacResOrDataFork);
		for (int i = 1; i <= 13; i++) {
			Common::String volume = Common::String::format("Data%d", i);
			addFileToDetectedGame(Common::Path(volume), allFiles, md5Prop, game);
		}
	}

	return game;
}

void SciMetaEngineDetection::addFileToDetectedGame(const Common::Path &name, const FileMap &allFiles, MD5Properties md5Prop, ADDetectedGame &game) const {
	FileProperties fileProperties;
	if (getFileProperties(allFiles, md5Prop, name, fileProperties)) {
		game.hasUnknownFiles = true;
		game.matchedFiles[name] = fileProperties;
	}
}

} // End of namespace Sci

REGISTER_PLUGIN_STATIC(SCI_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, Sci::SciMetaEngineDetection);
