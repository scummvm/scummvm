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

#include "common/language.h"
#include "common/rect.h"
#include "common/scummsys.h"

namespace Common {

class U32String;

} // End of namespace Common

namespace Mohawk {

struct MohawkGameDescription;

/** Immutable release identity and release-specific conventions derived from the detector. */
class ZmbReleaseInfo {
public:
	/** Broad implementation lineages; localized variants retain their parent family. */
	enum Family {
		/**
		 * v1.0BR, v1.0FR, v1.0GE, v1.0BR Demo,
		 * v1.1BR, v1.1FR, v1.1GE, v1.1FR_2002,
		 * v1.0ES, v1.0PT, v1.0IT, and v1.0JP.
		 */
		kEuropeV1,
		/** v1.1US, v1.11KR, and v1.1US Demo. */
		kUsV1,
		/** v2.0US, v2.0PL, and v2.0US Demo. */
		kTlcV2
	};

	/** Detector variants, without assigning a separate identity to every language or demo. */
	enum Variant {
		/** v1.0BR, v1.0FR, v1.0GE, v1.0IT, and v1.0BR Demo. */
		kEuropeV10,
		/** v1.1BR, v1.1FR, and v1.1GE. */
		kEuropeV11,
		/** v1.1FR_2002. */
		kFranceV11_2002,
		/** v1.0ES, v1.0PT. */
		kEsPtV10,
		/** v1.0JP. */
		kJapanV10,
		/** v1.1US, v1.11KR, and v1.1US Demo. */
		kUsV11,
		/** v2.0US, v2.0PL, and v2.0US Demo. */
		kTlcV20,
	};

	/** Page elements whose fixed rects need release-specific adjustments. */
	enum LayoutElement {
		/** Credit Dialog: text-line drawing rect in @ref Mohawk::ZoombiniDialogCredits. */
		kCreditsTextLayout,
		/** Credit Dialog: text clipping area to scroll in @ref Mohawk::ZoombiniDialogCredits. */
		kCreditsScrollLayout,
		/** MsgBox Dialog: body text rect in @ref Mohawk::ZoombiniDialogMsgBox. */
		kMsgBoxBodyLayout,
		/** Options Dialog: title text rect in @ref Mohawk::ZoombiniDialogOptions. */
		kOptionsTitleLayout,
		/** Save/load Dialog: title text rect in @ref Mohawk::ZoombiniDialogSaveLoad. */
		kSaveLoadTitleLayout,
		/** Save Dialog: editable save-name text/caret rect in @ref Mohawk::ZoombiniDialogSaveLoad. */
		kSaveNameInputLayout,
		/** RODMAP: hit rects for the practice level selection in @ref Mohawk::ZoombiniInteractiveRodMap. */
		kMapLegendClickLayout,
		/** RODMAP: legend title rect in @ref Mohawk::ZoombiniInteractiveRodMap. */
		kMapLegendTitleLayout,
		/** RODMAP: text rect for each four level labels of the legend in @ref Mohawk::ZoombiniInteractiveRodMap. */
		kMapLegendRowLayout,
		/** RODMAP: panel title rect for the save name or practice heading in @ref Mohawk::ZoombiniInteractiveRodMap. */
		kMapStatsTitleLayout,
		/** RODMAP: initial row Y position and height for statistics and practice descriptions in @ref Mohawk::ZoombiniInteractiveRodMap. */
		kMapStatsRowLayout,
		/** RODMAP: label-column width shared by journey statistics and practice descriptions in @ref Mohawk::ZoombiniInteractiveRodMap. */
		kMapStatsLabelLayout,
		/** RODMAP: per-row vertical advance and height growth for practice return instructions in @ref Mohawk::ZoombiniInteractiveRodMap. */
		kMapPracticeRowStepLayout,
		/** RODMAP: text rect for map-icon hover puzzle-name tooltip in @ref Mohawk::ZoombiniInteractiveRodMap. */
		kMapTooltipLayout,
		/** PICKER: text rect for Zoombini name preview in @ref Mohawk::ZoombiniShelterPicker. */
		kPickerNameLayout,
		/** NotiBox: long noti box bounds for its background and redraw area in @ref Mohawk::ZoombiniInteractive. */
		kNotiBoxLongLayout,
		/** NotiBox: text rect inside either noti box size in @ref Mohawk::ZoombiniInteractive. */
		kNotiBoxTextLayout,
		/** Help Dialog: rect adjustment for the dialog title and page heading in @ref Mohawk::ZoombiniDialogHelp. */
		kHelpHeadingLayout,
		/** Help Dialog: instruction body text rect in @ref Mohawk::ZoombiniDialogHelp. */
		kHelpBodyLayout,
		/** Help Dialog: Previous, Next and Okay button text rects in @ref Mohawk::ZoombiniDialogHelp. */
		kHelpButtonTextLayout,
	};

	/** Independent offsets added to the four edges of a page-owned rectangle. */
	struct LayoutAdjustment {
		int16 leftOffset = 0;
		int16 topOffset = 0;
		int16 rightOffset = 0;
		int16 bottomOffset = 0;

		/** Return the rectangle with this release's edge adjustments applied. */
		Common::Rect apply(const Common::Rect &rect) const;
	};

	/** Snapshot detector flags and normalize an unknown language to US English. */
	explicit ZmbReleaseInfo(const MohawkGameDescription &gameDesc);
	/** Match a broad implementation lineage. */
	bool isFamily(Family family) const;
	/** Match a detector variant, including its localized and demo releases. */
	bool isVariant(Variant variant) const;
	/** Match both the detector variant and its language. */
	bool isVariant(Variant variant, Common::Language lang) const;
	/** Match either v1 implementation family. */
	bool isV1() const;
	/** Whether the detector marks the release as a demo. */
	bool isDemo() const { return _demo; }
	/** Match a demo without implicitly restricting its language. */
	bool isDemo(Variant variant) const;
	/** Whether the demo starts at a map with fixed practice levels. */
	bool hasDemoPracticeMap() const;
	/** Whether this release supports localized Zoombini names. */
	bool supportsLocalizedNames() const;
	/** Format a memorial date using the original release's order and punctuation. */
	Common::U32String formatMemorialDate(uint16 year, const Common::U32String &monthText, byte day) const;
	/** Return edge adjustments for a fixed page layout element. */
	LayoutAdjustment getLayoutAdjustment(LayoutElement element) const;
	/** 
	 * Return the adjustment for one of the four zero-based map route labels.
	 * @param routeIdx 0-based index for each four routes.
	 */
	LayoutAdjustment getMapRouteLayoutAdjustment(uint32 routeIdx) const;
	/** Return the adjustment for a zero-based Options navigation or toggle label. */
	LayoutAdjustment getOptionsLabelLayoutAdjustment(uint32 buttonIdx) const;
	/** Language used by the engine, including the unknown-language fallback. */
	Common::Language getLanguage() const { return _lang; }
	/** Compact family label used in diagnostic output. */
	const char *getFamilyName() const;

private:
	/** Detector feature bits retained for release-family and variant queries. */
	const uint32 _features;
	/** Detector language normalized to US English when unknown. */
	const Common::Language _lang;
	/** Whether the detector marks this release as a demo. */
	const bool _demo;
};

class MohawkMetaEngine_Zoombini {
public:
	/** Confirm the currently focused dialog action. */
	static constexpr const char *kActionConfirm = "RETURN";
	/** Toggle dialog visibility and sound effects. */
	static constexpr const char *kActionToggleDialogAndSfx = "TG_SFX";
	/** Toggle background music. */
	static constexpr const char *kActionToggleMusic = "TG_BGM";
	/** Toggle sticky-mouse input. */
	static constexpr const char *kActionToggleStickyMouse = "TG_STKM";
	/** Toggle automatic sticky-mouse input. */
	static constexpr const char *kActionToggleAutoStickyMouse = "TG_AT_STKM";
	/** Toggle transition effects. */
	static constexpr const char *kActionToggleTransitions = "TG_TRNST";
	/** Toggle reduced-action mode. */
	static constexpr const char *kActionToggleLessMoreAction = "TG_ACTN";
	/** Toggle in-game cursor visibility. */
	static constexpr const char *kActionToggleCursor = "TG_CURS";
	/** v1.0JP only: Select Japanese or Latin procedural Zoombini names. */
	static constexpr const char *kActionToggleJapaneseNames = "TG_JPN_NAMES";
	/** Replay the puzzle instructions heard before entering the current puzzle. */
	static constexpr const char *kActionReplayPuzzleInstructions = "REPLAY_PUZ_INST";
	/** v2.0US only: Toggle the page narration in the Help dialog. */
	static constexpr const char *kActionToggleHelpDialogNarration = "TG_HLP_DLG_NAR";
	/** v2.0US only: Toggle touch-sense behavior. */
	static constexpr const char *kActionToggleTouchSense = "TG_TSENSE";
	/** Enter practice mode. */
	static constexpr const char *kActionPracticeMode = "PRACTICE";
	/** Start a new game. */
	static constexpr const char *kActionNewGame = "NEWGAME";
	/** Remove the selected save. */
	static constexpr const char *kActionRemoveSave = "DELSAVE";
	/** Request engine quit. */
	static constexpr const char *kActionQuit = "QUIT";

	// Bug fixes.
	/**
	 * Enable fixes for known audio pops.
	 * This is a Zoombini-internal constexpr key to represent Mohawk-inherited string key.
	 */
	constexpr static const char *kOptionFixAudioPops = "fix_audio_pops";
	/** Enable the Fleens tree-descend feet fix. */
	constexpr static const char *kOptionFixFleensTreeDescendFeetBug = "fix_fleens_tree_descend_feet_bug";
	/** Enable the Hotel MIDI halt fix. */
	constexpr static const char *kOptionFixHotelMidiHaltBug = "fix_hotel_midi_halt_bug";
	/** Enable the Caves level-four silent-MIDI fix. */
	constexpr static const char *kOptionFixCavesL4MidiSilentBug = "fix_caves_l4_midi_silent_bug";

	// Gameplay improvements.
	/** Persisted animation tick-rate selection. */
	enum class TickRate : uint32 {
		/**
		 * Accurate 60TPS tickrate for modern displays.
		 */
		kAccurate60FPS = 0,
		/**
		 * Original engine's 17ms tickrate.
		 */
		kOriginal17ms = 1
	};
	/** Select the animation tick timing. */
	static constexpr const char *kOptionTickRate = "tick_rate";
	/** Enable position interpolation for Zoombinis and their selected moving companions. */
	static constexpr const char *kOptionPositionInterpolation = "position_interpolation";
	/** Requested presentation frequency in Hz, independent of animation tick timing. */
	static constexpr const char *kOptionTargetFrameRate = "target_frame_rate";
	/** Persisted presentation frequencies, all integer multiples of the 60 Hz animation clock. */
	enum class TargetFrameRate : uint32 {
		/** Present once per animation tick. */
		k60FPS = 60,
		/** Present twice per animation tick. */
		k120FPS = 120,
		/** Present three times per animation tick. */
		k180FPS = 180,
		/** Present four times per animation tick. */
		k240FPS = 240,
	};
	/** Return a supported presentation frequency, or 60 Hz for an unknown stored value. */
	static TargetFrameRate normalizeTargetFrameRate(int rate);
	/** Enable the enhanced keyboard shortcut set. */
	constexpr static const char *kOptionEnhancedKbdShortcuts = "enhanced_kbd_shortcuts";
	/** Show current remapped shortcuts in the in-game Options dialog. */
	constexpr static const char *kOptionShowRemappedOptionDialogShortcuts = "show_remapped_option_dialog_shortcuts";

	// Gameplay tuning.
	/** Persisted palette-filter selection. */
	enum class PaletteFilter : uint32 {
		/**
		 * Original engine's palette brightening filter.
		 */
		kBrightenPalette = 0,
		/**
		 * Do not apply any filtering.
		 */
		kRawPalette = 1,
	};
	/** Select the palette brightness filter. */
	static constexpr const char *kOptionPaletteFilter = "palette_filter";
	/** Persisted pseudo-random generator selection. */
	enum class PrngAlgorithm : uint32 {
		/**
		 * Original engine's LCG PRNG.
		 */
		kOriginalPrng = 0,
		/**
		 * ScummVM standard xorshift-based PRNG.
		 */
		kStandardPrng = 1,
	};
	/** Select the pseudo-random generator algorithm. */
	static constexpr const char *kOptionPrngAlgorithm = "prng_algorithm";
	/**
	 * MIDI soundtrack selection for v1.x releases.
	 * Both songs are the same songs; the instruments and assumed synthesizers are different.
	 */
	enum class MidiSoundtrack : uint32 {
		/**
		 * Select MIDIMPC.MHK for MIDI soundtrack archive.
		 * Assumes GS synthesizers.
		 */
		kWindowsMPC = 0,
		/**
		 * Select MIDIMAC.MHK for MIDI soundtrack archive.
		 * Assumes SC-55 instrumentation.
		 */
		kMacintosh = 1,
	};
	/**
	 * Select the archive for MIDI music playback.
	 * @remarks Broderbund v1.x only; TLC v2.0 rebuild does not contain any MIDI resources.
	 */
	static constexpr const char *kOptionMidiSoundtrack = "midi_soundtrack";
	/** Enable color-blind presentation adjustments. */
	constexpr static const char *kOptionColorBlindMode = "color_blind_mode";
	/** Always play the Maze completion celebration sound. */
	constexpr static const char *kOptionMazeAlwaysPlayCelebrationSfx = "maze_always_play_celebration_sfx";
	/** Always play the Town memorial sound. */
	constexpr static const char *kOptionTownAlwaysPlayMemorialSfx = "town_always_play_memorial_sfx";
	/** Restore the unused Maze level-4 layout REGS 16607 to the layout pool. */
	constexpr static const char *kOptionMazeRestoreUnusedL4Layout = "maze_restore_unused_l4_layout";
	/** Randomize the next Maze layouts at startup and when layout options change. */
	constexpr static const char *kOptionMazeRandomizeInitialLayout = "maze_randomize_initial_layout";
	/** Restore the dormant Ferry trait-match highlight in every game mode. */
	constexpr static const char *kOptionFerryHighlightTraitMatch = "ferry_restore_unused_highlight_trait_match";

	/** Register Zoombini-specific configuration defaults with ConfMan. */
	static void registerDefaultSettings();
};

} // End of namespace Mohawk

#endif // MOHAWK_ZOOMBINI_METAENGINE_H
