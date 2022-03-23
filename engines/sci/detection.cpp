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
#include "common/translation.h"

#include "gui/ThemeEval.h"
#include "gui/widget.h"
#include "gui/widgets/popup.h"

#include "sci/detection.h"
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
	{"laurabow",        "Laura Bow: The Colonel's Bequest"},
	{"lsl2",            "Leisure Suit Larry 2: Goes Looking for Love (in Several Wrong Places)"},
	{"lsl3",            "Leisure Suit Larry 3: Passionate Patti in Pursuit of the Pulsating Pectorals"},
	{"mothergoose",     "Mixed-Up Mother Goose"},
	{"pq2",             "Police Quest II: The Vengeance"},
	{"qfg1",            "Quest for Glory I: So You Want to Be a Hero"},	// Note: There was also a SCI11 VGA remake of this (further down)
	{"sq3",             "Space Quest III: The Pirates of Pestulon"},
	// === SCI01 games ========================================================
	{"qfg2",            "Quest for Glory II: Trial by Fire"},
	{"kq1sci",          "King's Quest I: Quest for the Crown"},	// Note: There was also an AGI version of this
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
	{"sq1sci",          "Space Quest I: The Sarien Encounter"},	// Note: There was also an AGI version of this
	{"sq4",             "Space Quest IV: Roger Wilco and the Time Rippers"},	// floppy is SCI1, CD SCI1.1
	// === SCI1.1 games =======================================================
	{"christmas1992",   "Christmas Card 1992"},
	{"ecoquest2",       "EcoQuest II: Lost Secret of the Rainforest"},
	{"freddypharkas",   "Freddy Pharkas: Frontier Pharmacist"},
	{"hoyle4",          "Hoyle Classic Card Games"},
	{"inndemo",         "ImagiNation Network (INN) Demo"},
	{"kq6",             "King's Quest VI: Heir Today, Gone Tomorrow"},
	{"laurabow2",       "Laura Bow 2: The Dagger of Amon Ra"},
	{"qfg1vga",         "Quest for Glory I: So You Want to Be a Hero"},	// Note: There was also a SCI0 version of this (further up)
	{"qfg3",            "Quest for Glory III: Wages of War"},
	{"sq5",             "Space Quest V: The Next Mutation"},
	{"islandbrain",     "The Island of Dr. Brain"},
	{"lsl6",            "Leisure Suit Larry 6: Shape Up or Slip Out!"},
	{"pepper",          "Pepper's Adventures in Time"},
	{"slater",          "Slater & Charlie Go Camping"},
	{"gk1demo",         "Gabriel Knight: Sins of the Fathers"},
	{"qfg4demo",        "Quest for Glory IV: Shadows of Darkness"},
	{"pq4demo",         "Police Quest IV: Open Season"},
	// === SCI1.1+ games ======================================================
	{"catdate",         "The Dating Pool"},
	// === SCI2 games =========================================================
	{"gk1",             "Gabriel Knight: Sins of the Fathers"},
	{"pq4",             "Police Quest IV: Open Season"}, // floppy is SCI2, CD SCI2.1
	{"qfg4",            "Quest for Glory IV: Shadows of Darkness"},	// floppy is SCI2, CD SCI2.1
	// === SCI2.1 games ========================================================
	{"hoyle5",          "Hoyle Classic Games"},
	{"hoyle5bridge",    "Hoyle Bridge"},
	{"hoyle5children",  "Hoyle Children's Collection"},
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
	//{"shivers2",        "Shivers II: Harvest of Souls"},	// Not SCI
	{"rama",            "RAMA"},
	{nullptr, nullptr}
};

} // End of namespace Sci

#include "sci/detection_tables.h"

namespace Sci {

static const char *directoryGlobs[] = {
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

class SciMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	SciMetaEngineDetection() : AdvancedMetaEngineDetection(Sci::SciGameDescriptions, sizeof(ADGameDescription), s_sciGameTitles) {
		_maxScanDepth = 3;
		_directoryGlobs = directoryGlobs;
		_flags = kADFlagMatchFullPaths;
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}

	const char *getEngineId() const override {
		return "sci";
	}

	const char *getName() const override {
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

	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist, ADDetectedGameExtraInfo **extra) const override;
};

ADDetectedGame SciMetaEngineDetection::fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist, ADDetectedGameExtraInfo **extra) const {
	/**
	 * Fallback detection for Sci heavily depends on engine resources, so it's not possible
	 * to use them without the engine present in a clean way.
	 */

	if (ConfMan.hasKey("always_run_fallback_detection_extern")) {
		if (ConfMan.getBool("always_run_fallback_detection_extern") == false) {
			warning("SCI: Fallback detection is disabled.");
			return ADDetectedGame();
		}
	}

	const Plugin *metaEnginePlugin = EngineMan.findPlugin(getEngineId());

	if (metaEnginePlugin) {
		const Plugin *enginePlugin = PluginMan.getEngineFromMetaEngine(metaEnginePlugin);
		if (enginePlugin) {
			return enginePlugin->get<AdvancedMetaEngine>().fallbackDetectExtern(_md5Bytes, allFiles, fslist);
		} else {
			static bool warn = true;
			if (warn) {
				warning("Engine plugin for SCI not present. Fallback detection is disabled.");
				warn = false;
			}
		}
	}

	return ADDetectedGame();
}

} // End of namespace Sci

REGISTER_PLUGIN_STATIC(SCI_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, Sci::SciMetaEngineDetection);
