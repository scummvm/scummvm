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

#include "mohawk/zoombini_metaengine.h"

const char *const Mohawk::MohawkMetaEngine_Zoombini::kActionConfirm = "RETURN";
const char *const Mohawk::MohawkMetaEngine_Zoombini::kActionToggleDialogAndSfx = "TG_SFX";
const char *const Mohawk::MohawkMetaEngine_Zoombini::kActionToggleMusic = "TG_BGM";
const char *const Mohawk::MohawkMetaEngine_Zoombini::kActionToggleStickyMouse = "TG_STKM";
const char *const Mohawk::MohawkMetaEngine_Zoombini::kActionToggleAutoStickyMouse = "TG_AT_STKM";
const char *const Mohawk::MohawkMetaEngine_Zoombini::kActionToggleTransitions = "TG_TRNST";
const char *const Mohawk::MohawkMetaEngine_Zoombini::kActionToggleLessMoreAction = "TG_ACTN";
const char *const Mohawk::MohawkMetaEngine_Zoombini::kActionToggleCursor = "TG_CURS";
const char *const Mohawk::MohawkMetaEngine_Zoombini::kActionReplayPuzzleInstructions = "REPLAY_PUZ_INST";
const char *const Mohawk::MohawkMetaEngine_Zoombini::kActionToggleHelpDialogNarration = "TG_HLP_DLG_NAR";
/** v2.0US only */
const char *const Mohawk::MohawkMetaEngine_Zoombini::kActionToggleTouchSense = "TG_TSENSE";
const char *const Mohawk::MohawkMetaEngine_Zoombini::kActionPracticeMode = "PRACTICE";
const char *const Mohawk::MohawkMetaEngine_Zoombini::kActionNewGame = "NEWGAME";
const char *const Mohawk::MohawkMetaEngine_Zoombini::kActionRemoveSave = "DELSAVE";
const char *const Mohawk::MohawkMetaEngine_Zoombini::kActionQuit = "QUIT";

void Mohawk::MohawkMetaEngine_Zoombini::registerDefaultSettings() {
	// Bug fixes.
	ConfMan.registerDefault(kOptionFixAudioPops, true);
	ConfMan.registerDefault(kOptionFixFleensTreeDescendFeetBug, true);
	ConfMan.registerDefault(kOptionFixHotelMidiHaltBug, true);
	ConfMan.registerDefault(kOptionFixCavesL4MidiSilentBug, true);
	// Gameplay improvements.
	ConfMan.registerDefault(kOptionUseAccurate60FPS, true);
	ConfMan.registerDefault(kOptionEnhancedKbdShortcuts, true);
	ConfMan.registerDefault(kOptionShowRemappedOptionDialogShortcuts, true);
	// Gameplay tuning.
	ConfMan.registerDefault(kOptionBrightenPalette, true);
	ConfMan.registerDefault(kOptionOriginalPRNG, true);
	ConfMan.registerDefault(kOptionColorBlindMode, false);
	ConfMan.registerDefault(kOptionMazeAlwaysPlayCelebrationSfx, false);
	ConfMan.registerDefault(kOptionTownAlwaysPlayMemorialSfx, true);
	ConfMan.registerDefault(kOptionMazeRestoreUnusedL4Layout, false);
	ConfMan.registerDefault(kOptionMazeRandomizeInitialLayout, false);
	ConfMan.registerDefault(kOptionFerryHighlightTraitMatch, false);
	// Default off: preserve the authentic Windows-engine behavior (MIDIMPC.MHK).
	// The Macintosh soundtrack (MIDIMAC.MHK) is opt-in.
	ConfMan.registerDefault(kOptionUseMacMidi, false);
}
