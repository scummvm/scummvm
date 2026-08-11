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

#ifndef SCI_ENGINE_FEATURES_H
#define SCI_ENGINE_FEATURES_H

#include "sci/resource/resource.h"
#include "sci/engine/seg_manager.h"

namespace Sci {

enum MoveCountType {
	kMoveCountUninitialized,
	kIgnoreMoveCount,
	kIncrementMoveCount
};

enum PseudoMouseAbilityType {
	kPseudoMouseAbilityUninitialized,
	kPseudoMouseAbilityFalse,
	kPseudoMouseAbilityTrue
};

enum MessageTypeSyncStrategy {
	kMessageTypeSyncStrategyNone,
	kMessageTypeSyncStrategyDefault
#ifdef ENABLE_SCI32
	,
	kMessageTypeSyncStrategyLSL6Hires,
	kMessageTypeSyncStrategyShivers
#endif
};

enum {
	kSpeedThrottleDefaultDelay = 30 // kGameIsRestarting default max delay in ms
};

class GameFeatures {
public:
	GameFeatures(SegManager *segMan, Kernel *kernel);
	~GameFeatures() {}

	/**
	 * Autodetects the DoSound type
	 * @return DoSound type, SCI_VERSION_0_EARLY / SCI_VERSION_0_LATE /
	 *                       SCI_VERSION_1_EARLY / SCI_VERSION_1_LATE
	 */
	SciVersion detectDoSoundType();

	/**
	 * Autodetects the SetCursor type
	 * @return SetCursor type, SCI_VERSION_0_EARLY / SCI_VERSION_1_1
	 */
	SciVersion detectSetCursorType();

	/**
	 * Autodetects the Lofs type
	 * @return Lofs type, SCI_VERSION_0_EARLY / SCI_VERSION_1_MIDDLE / SCI_VERSION_1_1 / SCI_VERSION_3
	 */
	SciVersion detectLofsType();

	/**
	 * Autodetects the graphics functions used
	 * @return Graphics functions type, SCI_VERSION_0_EARLY / SCI_VERSION_0_LATE
	 */
	SciVersion detectGfxFunctionsType();

	/**
	 * Autodetects the message function used
	 * @return Message function type, SCI_VERSION_1_LATE / SCI_VERSION_1_1
	 */
	SciVersion detectMessageFunctionType();

#ifdef ENABLE_SCI32
	/**
	 * Autodetects the kernel functions used in SCI2.1
	 * @return Graphics functions type, SCI_VERSION_2 / SCI_VERSION_2_1
	 */
	SciVersion detectSci21KernelType();

	inline bool usesModifiedAudioAttenuation() const {
		switch (g_sci->getGameId()) {
		case GID_PQ4:
		case GID_QFG4:
			return g_sci->isCD();
		case GID_MOTHERGOOSEHIRES:
		case GID_SQ6:
			// SQ6 1.0 uses modified attenuation, 1.11 does not.
			// The interpreters are different even though they both have the
			// same date and version string. ("May 24 1995", "2.100.002")
			return true;
		case GID_KQ7:
			// KQ7 1.51 (SCI2.1early) uses the non-standard attenuation, but
			// 2.00b (SCI2.1mid) does not
			return getSciVersion() == SCI_VERSION_2_1_EARLY;
		default:
			return false;
		}
	}

	inline bool gameScriptsControlMasterVolume() const {
		switch (g_sci->getGameId()) {
		case GID_LSL7:
		case GID_PHANTASMAGORIA2:
		case GID_TORIN:
			return true;
		default:
			return false;
		}
	}

	inline bool hasSci3Audio() const {
		return getSciVersion() == SCI_VERSION_3 || g_sci->getGameId() == GID_GK2;
	}

	inline bool hasTransparentPicturePlanes() const {
		const SciGameId &gid = g_sci->getGameId();

		// MGDX is assumed to not have transparent picture planes since it
		// was released before SQ6, but this has not been verified since it
		// cannot be disassembled at the moment (Phar Lap Windows-only release)
		return getSciVersion() >= SCI_VERSION_2_1_MIDDLE &&
			gid != GID_SQ6 &&
			gid != GID_MOTHERGOOSEHIRES;
	}

	inline bool hasMidPaletteCode() const {
		return getSciVersion() >= SCI_VERSION_2_1_MIDDLE || g_sci->getGameId() == GID_KQ7;
	}

	inline bool hasLatePaletteCode() const {
		return getSciVersion() > SCI_VERSION_2_1_MIDDLE ||
			g_sci->getGameId() == GID_GK2 ||
			g_sci->getGameId() == GID_PQSWAT ||
			// Guessing that Shivers has the late palette code because it has a
			// brightness slider
			g_sci->getGameId() == GID_SHIVERS ||
			g_sci->getGameId() == GID_TORIN;
	}

	inline bool VMDOpenStopsAudio() const {
		// Of the games that use VMDs:
		// Yes: Phant1, Shivers, Torin
		// No: SQ6
		// TODO: Optional extra flag to kPlayVMD which defaults to Yes: PQ:SWAT
		// TODO: SCI3, GK2 (GK2's VMD code is closer to SCI3 than SCI21)
		return getSciVersion() == SCI_VERSION_2_1_MIDDLE &&
			g_sci->getGameId() != GID_SQ6 &&
			g_sci->getGameId() != GID_GK2;
	}

	inline bool useDoSoundMac32() const {
		// Several SCI 2.1 Middle Mac games use a modified kDoSound with
		//  different subop numbers.
		return g_sci->getPlatform() == Common::kPlatformMacintosh &&
			(g_sci->getGameId() == GID_HOYLE5 ||
			 g_sci->getGameId() == GID_PHANTASMAGORIA ||
			 g_sci->getGameId() == GID_PQSWAT ||
			 g_sci->getGameId() == GID_SHIVERS ||
			 g_sci->getGameId() == GID_SQ6);
	}

	inline bool useMacGammaLevel() const {
		// SCI32 Mac interpreters were hard-coded to use gamma level 2 until
		//  Torin's Passage, PQSWAT, and the 2.1 Late games. The colors in
		//  the game resources are significantly darker than their PC versions.
		//  Confirmed in disassembly of all Mac interpreters.
		return g_sci->getPlatform() == Common::kPlatformMacintosh &&
			getSciVersion() >= SCI_VERSION_2 &&
			getSciVersion() < SCI_VERSION_2_1_LATE &&
			g_sci->getGameId() != GID_PQSWAT &&
			g_sci->getGameId() != GID_TORIN;
	}

	inline bool usesAlternateSelectors() const {
		return g_sci->getGameId() == GID_PHANTASMAGORIA2;
	}
#endif

	/**
	 * If true, the current game supports simultaneous speech & subtitles.
	 */
	bool supportsSpeechWithSubtitles() const;

	/**
	 * If true, the game supports changing text speed.
	 */
	bool supportsTextSpeed() const {
		switch (g_sci->getGameId()) {
		case GID_GK1:
		case GID_SQ6:
			return true;
		default:
			return false;
		}
	}

	/**
	 * If true, audio volume sync between the game and ScummVM is done by
	 * monitoring and setting game global variables.
	 */
	bool audioVolumeSyncUsesGlobals() const;

	/**
	 * The strategy that should be used when synchronising the message type
	 * (text/speech/text+speech) between the game and ScummVM.
	 */
	MessageTypeSyncStrategy getMessageTypeSyncStrategy() const;

	/**
	 * Applies to all versions before 0.000.502
	 * Old SCI versions used to interpret the third DrawPic() parameter inversely,
	 * with the opposite default value (obviously).
	 * Also, they used 15 priority zones from 42 to 200 instead of 14 priority
	 * zones from 42 to 190.
	 */
	bool usesOldGfxFunctions() { return detectGfxFunctionsType() == SCI_VERSION_0_EARLY; }

	/**
	 * Autodetects the Bresenham routine used in the actor movement functions
	 * @return Move count type, kIncrementMoveCnt / kIgnoreMoveCnt
	 */
	MoveCountType detectMoveCountType();

	int detectPlaneIdBase();

	bool handleMoveCount() { return detectMoveCountType() == kIncrementMoveCount; }

	bool usesCdTrack() { return _usesCdTrack; }

	/**
	 * Checks if the alternative Windows GM MIDI soundtrack should be used. Such
	 * soundtracks are available for the Windows CD versions of EcoQuest, Jones,
	 * KQ5 and SQ4.
	 */
	bool useAltWinGMSound();

	/**
	 * Checks if the game only supports General MIDI for music playback.
	 */
	bool generalMidiOnly();

	/**
	 * Forces DOS soundtracks in Windows CD versions when the user hasn't
	 * selected a MIDI output device
	 */
	void forceDOSTracks() { _forceDOSTracks = true; }

	bool useWindowsCursors() { return _useWindowsCursors; }

	/**
	 * Autodetects, if Pseudo Mouse ability is enabled (different behavior in keyboard driver)
	 * @return kPseudoMouseAbilityTrue or kPseudoMouseAbilityFalse
	 */
	PseudoMouseAbilityType detectPseudoMouseAbility();

	bool useAudioPopfix() const { return _useAudioPopfix; }

	bool useEarlyGetLongestTextCalculations() const;

	/**
	 * Several SCI1.1 Macintosh games have empty strings for almost all of the
	 * object names in the script resources.
	 *
	 * @return true if the game's object names aren't empty strings.
	 */
	bool hasScriptObjectNames() const;

	/**
	 * Returns if the game can be saved via the GMM.
	 * Saving via the GMM doesn't work as expected in
	 * games which don't follow the normal saving scheme.
	*/
	bool canSaveFromGMM() const;
	
	/**
	 * Returns the global variable index to the start of the game's
	 * global flags array. This is used by the console debugger.
	 *
	 * @return Non-zero index if successful, otherwise zero.
	 */
	uint16 getGameFlagsGlobal() const;

	/**
	 * Returns the bit order in which game flags are stored.
	 *
	 * @return true if bit order is normal or false if reversed.
	 */
	bool isGameFlagBitOrderNormal() const;

private:
	reg_t getDetectionAddr(const Common::String &objName, Selector slc, int methodNum = -1);

	bool autoDetectLofsType(const Common::String& gameSuperClassName, int methodNum);
	bool autoDetectGfxFunctionsType(int methodNum = -1);
	bool autoDetectSoundType();
	bool autoDetectMoveCountType();
#ifdef ENABLE_SCI32
	bool autoDetectSci21KernelType();
#endif

	SciVersion _doSoundType, _setCursorType, _lofsType, _gfxFunctionsType, _messageFunctionType;
#ifdef ENABLE_SCI32
	SciVersion _sci21KernelType;
#endif

	MoveCountType _moveCountType;
	bool _usesCdTrack;
	bool _forceDOSTracks;
	bool _useWindowsCursors;

	PseudoMouseAbilityType _pseudoMouseAbility;

	bool _useAudioPopfix;

	SegManager *_segMan;
	Kernel *_kernel;
};

} // End of namespace Sci

#endif // SCI_ENGINE_FEATURES_H
