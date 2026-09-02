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

#ifndef MOHAWK_ZOOMBINI_METAENGINE_H
#define MOHAWK_ZOOMBINI_METAENGINE_H

#include "common/config-manager.h"

namespace Mohawk {

class MohawkMetaEngine_Zoombini {
public:
	/** Confirm the currently focused dialog action. */
	static const char *const kActionConfirm;
	/** Toggle dialog visibility and sound effects. */
	static const char *const kActionToggleDialogAndSfx;
	/** Toggle background music. */
	static const char *const kActionToggleMusic;
	/** Toggle sticky-mouse input. */
	static const char *const kActionToggleStickyMouse;
	/** Toggle automatic sticky-mouse input. */
	static const char *const kActionToggleAutoStickyMouse;
	/** Toggle transition effects. */
	static const char *const kActionToggleTransitions;
	/** Toggle reduced-action mode. */
	static const char *const kActionToggleLessMoreAction;
	/** Toggle in-game cursor visibility. */
	static const char *const kActionToggleCursor;
	/** Replay the puzzle instructions heard before entering the current puzzle. */
	static const char *const kActionReplayPuzzleInstructions;
	/** v2.0US only: Toggle the page narration in the Help dialog. */
	static const char *const kActionToggleHelpDialogNarration;
	/** Toggle touch-sense behavior. */
	static const char *const kActionToggleTouchSense;
	/** Enter practice mode. */
	static const char *const kActionPracticeMode;
	/** Start a new game. */
	static const char *const kActionNewGame;
	/** Remove the selected save. */
	static const char *const kActionRemoveSave;
	/** Request engine quit. */
	static const char *const kActionQuit;

	// Bug fixes.
	/** Enable fixes for known audio pops. */
	constexpr static const char *kOptionFixAudioPops = "fix_audio_pops";
	/** Enable the Fleens tree-descend feet fix. */
	constexpr static const char *kOptionFixFleensTreeDescendFeetBug = "fix_fleens_tree_descend_feet_bug";
	/** Enable the Hotel MIDI halt fix. */
	constexpr static const char *kOptionFixHotelMidiHaltBug = "fix_hotel_midi_halt_bug";
	/** Enable the Caves level-four silent-MIDI fix. */
	constexpr static const char *kOptionFixCavesL4MidiSilentBug = "fix_caves_l4_midi_silent_bug";

	// Gameplay improvements.
	/** Use a stable 60 FPS timing model for the engine. */
	constexpr static const char *kOptionUseAccurate60FPS = "use_accurate_60fps";
	/** Enable the enhanced keyboard shortcut set. */
	constexpr static const char *kOptionEnhancedKbdShortcuts = "enhanced_kbd_shortcuts";
	/** Show current remapped shortcuts in the in-game Options dialog. */
	constexpr static const char *kOptionShowRemappedOptionDialogShortcuts = "show_remapped_option_dialog_shortcuts";

	// Gameplay tuning.
	/** Enable the brighter palette presentation. */
	constexpr static const char *kOptionBrightenPalette = "brighten_palette";
	/** Use the original pseudo-random generator behavior. */
	constexpr static const char *kOptionOriginalPRNG = "original_prng";
	/** Enable color-blind presentation adjustments. */
	constexpr static const char *kOptionColorBlindMode = "color_blind_mode";
	/** Always play the Maze completion celebration sound. */
	constexpr static const char *kOptionMazeAlwaysPlayCelebrationSfx = "maze_always_play_celebration_sfx";
	/** Always play the Town memorial sound. */
	constexpr static const char *kOptionTownAlwaysPlayMemorialSfx = "town_always_play_memorial_sfx";
	/** Restore the unused Maze level-4 layout REGS 16607 to the layout pool. */
	constexpr static const char *kOptionMazeRestoreUnusedL4Layout = "maze_restore_unused_l4_layout";
	/** Randomize the first Maze layout selection from non-gameplay system time. */
	constexpr static const char *kOptionMazeRandomizeInitialLayout = "maze_randomize_initial_layout";
	/** Restore the dormant Ferry trait-match highlight in every game mode. */
	constexpr static const char *kOptionFerryHighlightTraitMatch = "ferry_restore_unused_highlight_trait_match";

	/**
	 * When enabled, v1.x pages load MIDIMAC.MHK instead of MIDIMPC.MHK.
	 * This plays the Macintosh MIDI soundtrack instead of the Windows soundtrack.
	 * Mac MIDI assumes SC-55 instrumentation, while Windows MIDI assumes GS.
	 * @remarks Broderbund v1.x only; TLC v2.0 rebuild, does not contain any MIDI resources.
	 */
	constexpr static const char *kOptionUseMacMidi = "use_mac_midi";

	/** Register Zoombini-specific configuration defaults with ConfMan. */
	static void registerDefaultSettings();
};

} // End of namespace Mohawk

#endif // MOHAWK_ZOOMBINI_METAENGINE_H
