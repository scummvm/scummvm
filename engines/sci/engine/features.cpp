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

#include "sci/engine/features.h"
#include "sci/engine/kernel.h"
#include "sci/engine/script.h"
#include "sci/engine/selector.h"
#include "sci/engine/vm.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/gui_options.h"

namespace Sci {

GameFeatures::GameFeatures(SegManager *segMan, Kernel *kernel) : _segMan(segMan), _kernel(kernel) {
	_setCursorType = SCI_VERSION_NONE;
	_doSoundType = SCI_VERSION_NONE;
	_lofsType = SCI_VERSION_NONE;
	_gfxFunctionsType = SCI_VERSION_NONE;
	_messageFunctionType = SCI_VERSION_NONE;
	_moveCountType = kMoveCountUninitialized;
#ifdef ENABLE_SCI32
	_sci21KernelType = SCI_VERSION_NONE;
#endif
	_usesCdTrack = Common::File::exists("cdaudio.map");
	if (!ConfMan.getBool("use_cdaudio"))
		_usesCdTrack = false;
	_forceDOSTracks = false;
	_useWindowsCursors = ConfMan.getBool("windows_cursors");
	_pseudoMouseAbility = kPseudoMouseAbilityUninitialized;
	_useAudioPopfix = Common::checkGameGUIOption(GAMEOPTION_GK1_ENABLE_AUDIO_POPFIX, ConfMan.get("guioptions")) &&
	                  ConfMan.getBool("audio_popfix_enabled");
}

reg_t GameFeatures::getDetectionAddr(const Common::String &objName, Selector slc, int methodNum) {
	// Get address of target object
	reg_t objAddr = _segMan->findObjectByName(objName, 0);
	reg_t addr;

	if (objAddr.isNull()) {
		error("getDetectionAddr: %s object couldn't be found", objName.c_str());
		return NULL_REG;
	}

	if (methodNum == -1) {
		if (lookupSelector(_segMan, objAddr, slc, nullptr, &addr) != kSelectorMethod) {
			error("getDetectionAddr: target selector is not a method of object %s", objName.c_str());
			return NULL_REG;
		}
	} else {
		addr = _segMan->getObject(objAddr)->getFunction(methodNum);
	}

	return addr;
}

bool GameFeatures::autoDetectSoundType() {
	// Look up the script address
	reg_t addr = getDetectionAddr("Sound", SELECTOR(play));

	if (!addr.getSegment())
		return false;

	uint32 offset = addr.getOffset();
	Script *script = _segMan->getScript(addr.getSegment());
	uint16 intParam = 0xFFFF;
	bool foundTarget = false;

	while (true) {
		int16 opparams[4];
		byte extOpcode;
		byte opcode;
		offset += readPMachineInstruction(script->getBuf(offset), extOpcode, opparams);
		opcode = extOpcode >> 1;

		// Check for end of script
		if (opcode == op_ret || offset >= script->getBufSize())
			break;

		// The play method of the Sound object pushes the DoSound command that
		// it will use just before it calls DoSound. We intercept that here in
		// order to check what sound semantics are used, cause the position of
		// the sound commands has changed at some point during SCI1 middle.
		if (opcode == op_pushi) {
			// Load the pushi parameter
			intParam = opparams[0];
		} else if (opcode == op_callk) {
			uint16 kFuncNum = opparams[0];

			// Late SCI1 games call kIsObject before kDoSound
			if (kFuncNum == 6) {	// kIsObject (SCI0-SCI11)
				foundTarget = true;
			} else if (kFuncNum == 45) {	// kDoSound (SCI1)
				// First, check which DoSound function is called by the play
				// method of the Sound object
				switch (intParam) {
				case 1:
					_doSoundType = SCI_VERSION_0_EARLY;
					break;
				case 7:
					_doSoundType = SCI_VERSION_1_EARLY;
					break;
				case 8:
					_doSoundType = SCI_VERSION_1_LATE;
					break;
				default:
					// Unknown case... should never happen. We fall back to
					// alternative detection here, which works in general, apart
					// from some transitive games like Jones CD
					_doSoundType = foundTarget ? SCI_VERSION_1_LATE : SCI_VERSION_1_EARLY;
					break;
				}
				return true;
			}
		}
	}

	return false;	// not found
}

SciVersion GameFeatures::detectDoSoundType() {
	if (_doSoundType == SCI_VERSION_NONE) {
		if (getSciVersion() == SCI_VERSION_0_EARLY) {
			// Almost all of the SCI0EARLY games use different sound resources than
			//  SCI0LATE. Although the last SCI0EARLY game (lsl2) uses SCI0LATE resources
			_doSoundType = g_sci->getResMan()->detectEarlySound() ? SCI_VERSION_0_EARLY : SCI_VERSION_0_LATE;
#ifdef ENABLE_SCI32
		} else if (getSciVersion() >= SCI_VERSION_2_1_MIDDLE &&
				   g_sci->getGameId() != GID_SQ6 &&
				   // Assuming MGDX uses SCI2.1early sound mode since SQ6 does
				   // and it was released earlier, but not verified (Phar Lap
				   // Windows-only release)
				   g_sci->getGameId() != GID_MOTHERGOOSEHIRES) {
			_doSoundType = SCI_VERSION_2_1_MIDDLE;
		} else if (getSciVersion() >= SCI_VERSION_2_1_EARLY) {
			_doSoundType = SCI_VERSION_2_1_EARLY;
		} else if (getSciVersion() >= SCI_VERSION_2) {
			_doSoundType = SCI_VERSION_2;
#endif
		} else if (SELECTOR(nodePtr) == -1) {
			// No nodePtr selector, so this game is definitely using newer
			// SCI0 sound code (i.e. SCI_VERSION_0_LATE)
			_doSoundType = SCI_VERSION_0_LATE;
		} else if (getSciVersion() >= SCI_VERSION_1_LATE) {
			// All SCI1 late games use the newer doSound semantics
			_doSoundType = SCI_VERSION_1_LATE;
		} else {
			if (!autoDetectSoundType()) {
				warning("DoSound detection failed, taking an educated guess");

				if (getSciVersion() >= SCI_VERSION_1_MIDDLE)
					_doSoundType = SCI_VERSION_1_LATE;
				else if (getSciVersion() > SCI_VERSION_01)
					_doSoundType = SCI_VERSION_1_EARLY;
			}
		}

		debugC(1, kDebugLevelSound, "Detected DoSound type: %s", getSciVersionDesc(_doSoundType));
	}

	return _doSoundType;
}

SciVersion GameFeatures::detectSetCursorType() {
	if (_setCursorType == SCI_VERSION_NONE) {
		if (getSciVersion() <= SCI_VERSION_1_MIDDLE) {
			// SCI1 middle and older games never use cursor views
			_setCursorType = SCI_VERSION_0_EARLY;
		} else if (getSciVersion() >= SCI_VERSION_1_1) {
			// SCI1.1 games always use cursor views
			_setCursorType = SCI_VERSION_1_1;
		} else {	// SCI1 late game, detect cursor semantics
			// If the Cursor object doesn't exist, we're using the SCI0 early
			// kSetCursor semantics.
			if (_segMan->findObjectByName("Cursor") == NULL_REG) {
				_setCursorType = SCI_VERSION_0_EARLY;
				debugC(1, kDebugLevelGraphics, "Detected SetCursor type: %s", getSciVersionDesc(_setCursorType));
				return _setCursorType;
			}

			// Check for the existence of the handCursor object (first found).
			// This is based on KQ5.
			reg_t objAddr = _segMan->findObjectByName("handCursor", 0);

			// If that doesn't exist, we assume it uses SCI1.1 kSetCursor semantics
			if (objAddr == NULL_REG) {
				_setCursorType = SCI_VERSION_1_1;
				debugC(1, kDebugLevelGraphics, "Detected SetCursor type: %s", getSciVersionDesc(_setCursorType));
				return _setCursorType;
			}

			// Now we check what the number variable holds in the handCursor
			// object.
			uint16 number = readSelectorValue(_segMan, objAddr, SELECTOR(number));

			// If the number is 0, it uses views and therefore the SCI1.1
			// kSetCursor semantics, otherwise it uses the SCI0 early kSetCursor
			// semantics.
			if (number == 0)
				// KQ5 CD's DOS interpreter contained the new kSetCusor API while
				// the Windows interpreter contained the old. The scripts tested
				// the platform to see which version to call.
				if (g_sci->getGameId() == GID_KQ5 && _useWindowsCursors) {
					_setCursorType = SCI_VERSION_0_EARLY;
				} else {
					_setCursorType = SCI_VERSION_1_1;
				}
			else
				_setCursorType = SCI_VERSION_0_EARLY;
		}

		debugC(1, kDebugLevelGraphics, "Detected SetCursor type: %s", getSciVersionDesc(_setCursorType));
	}

	return _setCursorType;
}

bool GameFeatures::autoDetectLofsType(const Common::String &gameSuperClassName, int methodNum) {
	// Look up the script address
	reg_t addr = getDetectionAddr(gameSuperClassName.c_str(), -1, methodNum);

	if (!addr.getSegment())
		return false;

	uint32 offset = addr.getOffset();
	Script *script = _segMan->getScript(addr.getSegment());

	while (true) {
		int16 opparams[4];
		byte extOpcode;
		byte opcode;
		offset += readPMachineInstruction(script->getBuf(offset), extOpcode, opparams);
		opcode = extOpcode >> 1;

		// Check for end of script
		if (opcode == op_ret || offset >= script->getBufSize())
			break;

		if (opcode == op_lofsa || opcode == op_lofss) {
			// Load lofs operand
			uint16 lofs = opparams[0];

			// Check for going out of bounds when interpreting as abs/rel
			if (lofs >= script->getBufSize())
				_lofsType = SCI_VERSION_0_EARLY;

			if ((signed)offset + (int16)lofs < 0)
				_lofsType = SCI_VERSION_1_MIDDLE;

			if ((signed)offset + (int16)lofs >= (signed)script->getBufSize())
				_lofsType = SCI_VERSION_1_MIDDLE;

			if (_lofsType != SCI_VERSION_NONE)
				return true;

			// If we reach here, we haven't been able to deduce the lofs
			// parameter type so far.
		}
	}

	return false;	// not found
}

SciVersion GameFeatures::detectLofsType() {
	if (_lofsType == SCI_VERSION_NONE) {
		// This detection only works (and is only needed) for SCI 1
		if (getSciVersion() <= SCI_VERSION_01) {
			_lofsType = SCI_VERSION_0_EARLY;
			return _lofsType;
		}

		if (getSciVersion() >= SCI_VERSION_1_1 && getSciVersion() <= SCI_VERSION_2_1_LATE) {
			// SCI1.1 type, i.e. we compensate for the fact that the heap is attached
			// to the end of the script
			_lofsType = SCI_VERSION_1_1;
			return _lofsType;
		}

		if (getSciVersion() == SCI_VERSION_3) {
			// SCI3 type, same as pre-SCI1.1, really, as there is no separate heap
			// resource
			_lofsType = SCI_VERSION_3;
			return _lofsType;
		}

		// Find a function of the "Game" object (which is the game super class) which invokes lofsa/lofss
		const Object *gameObject = _segMan->getObject(g_sci->getGameObject());
		const Object *gameSuperObject = _segMan->getObject(gameObject->getSuperClassSelector());
		bool found = false;
		if (gameSuperObject) {
			Common::String gameSuperClassName = _segMan->getObjectName(gameObject->getSuperClassSelector());

			for (uint m = 0; m < gameSuperObject->getMethodCount(); m++) {
				found = autoDetectLofsType(gameSuperClassName, m);
				if (found)
					break;
			}
		} else {
			warning("detectLofsType(): Could not find superclass of game object");
		}

		if (!found) {
			warning("detectLofsType(): failed, taking an educated guess");

			if (getSciVersion() >= SCI_VERSION_1_MIDDLE)
				_lofsType = SCI_VERSION_1_MIDDLE;
			else
				_lofsType = SCI_VERSION_0_EARLY;
		}

		debugC(1, kDebugLevelVM, "Detected Lofs type: %s", getSciVersionDesc(_lofsType));
	}

	return _lofsType;
}

bool GameFeatures::autoDetectGfxFunctionsType(int methodNum) {
	// Look up the script address
	reg_t addr = getDetectionAddr("Rm", SELECTOR(overlay), methodNum);

	if (!addr.getSegment())
		return false;

	uint32 offset = addr.getOffset();
	Script *script = _segMan->getScript(addr.getSegment());

	while (true) {
		int16 opparams[4];
		byte extOpcode;
		byte opcode;
		offset += readPMachineInstruction(script->getBuf(offset), extOpcode, opparams);
		opcode = extOpcode >> 1;

		// Check for end of script
		if (opcode == op_ret || offset >= script->getBufSize())
			break;

		if (opcode == op_callk) {
			uint16 kFuncNum = opparams[0];
			uint16 argc = opparams[1] / 2;

			if (kFuncNum == 8) {	// kDrawPic	(SCI0 - SCI11)
				// If kDrawPic is called with 3 parameters from the overlay
				// method then the game is using old graphics functions.
				// If instead it's called with 4 parameters then it's using
				// the newer ones. (KQ4 late, SQ3 1.018)
				// Ignore other arg counts as those are unrelated to overlays
				// and this detection gets run on all Rm methods when the
				// overlay selector doesn't exist.
				if (argc == 3) {
					_gfxFunctionsType = SCI_VERSION_0_EARLY;
					return true;
				} else if (argc == 4) {
					_gfxFunctionsType = SCI_VERSION_0_LATE;
					return true;
				}
			}
		}
	}

	return false;	// not found
}

SciVersion GameFeatures::detectGfxFunctionsType() {
	if (_gfxFunctionsType == SCI_VERSION_NONE) {
		if (getSciVersion() == SCI_VERSION_0_EARLY) {
			// Old SCI0 games always used old graphics functions
			_gfxFunctionsType = SCI_VERSION_0_EARLY;
		} else if (getSciVersion() >= SCI_VERSION_01) {
			// SCI01 and newer games always used new graphics functions
			_gfxFunctionsType = SCI_VERSION_0_LATE;
		} else {	// SCI0 late
			// Check if the game is using an overlay
			bool searchRoomObj = false;
			reg_t rmObjAddr = _segMan->findObjectByName("Rm");

			if (SELECTOR(overlay) != -1) {
				// The game has an overlay selector, check how it calls kDrawPic
				// to determine the graphics functions type used
				if (lookupSelector(_segMan, rmObjAddr, SELECTOR(overlay), nullptr, nullptr) == kSelectorMethod) {
					if (!autoDetectGfxFunctionsType()) {
						warning("Graphics functions detection failed, taking an educated guess");

						// Try detecting the graphics function types from the
						// existence of the motionCue selector (which is a bit
						// of a hack)
						if (_kernel->findSelector("motionCue") != -1)
							_gfxFunctionsType = SCI_VERSION_0_LATE;
						else
							_gfxFunctionsType = SCI_VERSION_0_EARLY;
					}
				} else {
					// The game has an overlay selector, but it's not a method
					// of the Rm object (like in Hoyle 1 and 2), so search for
					// other methods
					searchRoomObj = true;
				}
			} else {
				// The game doesn't have an overlay selector, so search for it
				// manually
				searchRoomObj = true;
			}

			if (searchRoomObj) {
				// If requested, check if any method of the Rm object is calling
				// kDrawPic, as the overlay selector might be missing in demos
				bool found = false;

				const Object *obj = _segMan->getObject(rmObjAddr);
				for (uint m = 0; m < obj->getMethodCount(); m++) {
					found = autoDetectGfxFunctionsType(m);
					if (found)
						break;
				}

				if (!found) {
					// No method of the Rm object is calling kDrawPic with
					// 3 or 4 parameters, thus we assume that the game doesn't
					// have overlays and is using older graphics functions.
					_gfxFunctionsType = SCI_VERSION_0_EARLY;
				}
			}
		}

		debugC(1, kDebugLevelVM, "Detected graphics functions type: %s", getSciVersionDesc(_gfxFunctionsType));
	}

	return _gfxFunctionsType;
}

SciVersion GameFeatures::detectMessageFunctionType() {
	if (_messageFunctionType != SCI_VERSION_NONE)
		return _messageFunctionType;

	if (getSciVersion() > SCI_VERSION_1_1) {
		_messageFunctionType = SCI_VERSION_1_1;
		return _messageFunctionType;
	} else if (getSciVersion() < SCI_VERSION_1_1) {
		_messageFunctionType = SCI_VERSION_1_LATE;
		return _messageFunctionType;
	}

	Common::List<ResourceId> resources = g_sci->getResMan()->listResources(kResourceTypeMessage, -1);

	if (resources.empty()) {
		// No messages found, so this doesn't really matter anyway...
		_messageFunctionType = SCI_VERSION_1_1;
		return _messageFunctionType;
	}

	Resource *res = g_sci->getResMan()->findResource(*resources.begin(), false);
	assert(res);

	// Only v2 Message resources use the kGetMessage kernel function.
	// v3-v5 use the kMessage kernel function.

	if (res->getUint32SEAt(0) / 1000 == 2)
		_messageFunctionType = SCI_VERSION_1_LATE;
	else
		_messageFunctionType = SCI_VERSION_1_1;

	debugC(1, kDebugLevelVM, "Detected message function type: %s", getSciVersionDesc(_messageFunctionType));
	return _messageFunctionType;
}

#ifdef ENABLE_SCI32
bool GameFeatures::autoDetectSci21KernelType() {
	// First, check if the Sound object is loaded
	reg_t soundObjAddr = _segMan->findObjectByName("Sound");
	if (soundObjAddr.isNull()) {
		// Usually, this means that the Sound object isn't loaded yet.
		// This case doesn't occur in early SCI2.1 games, and we've only
		// seen it happen in the RAMA demo, thus we can assume that the
		// game is using a SCI2.1 table

		// HACK: The Inside the Chest Demo and King's Questions minigame
		// don't have sounds at all, but they're using a SCI2 kernel
		if (g_sci->getGameId() == GID_CHEST || g_sci->getGameId() == GID_KQUESTIONS) {
			_sci21KernelType = SCI_VERSION_2;
		} else if (g_sci->getGameId() == GID_RAMA && g_sci->isDemo()) {
			_sci21KernelType = SCI_VERSION_2_1_MIDDLE;
		} else {
			warning("autoDetectSci21KernelType(): Sound object not loaded, assuming a SCI2.1 table");
			_sci21KernelType = SCI_VERSION_2_1_EARLY;
		}
		return true;
	}

	// Look up the script address
	reg_t addr = getDetectionAddr("Sound", SELECTOR(play));

	if (!addr.getSegment())
		return false;

	uint32 offset = addr.getOffset();
	Script *script = _segMan->getScript(addr.getSegment());

	while (true) {
		int16 opparams[4];
		byte extOpcode;
		byte opcode;
		offset += readPMachineInstruction(script->getBuf(offset), extOpcode, opparams);
		opcode = extOpcode >> 1;

		// Check for end of script
		// We don't check for op_ret here because the Phantasmagoria Mac script
		// has an op_ret early on in its script (controlled by a branch).
		if (offset >= script->getBufSize())
			break;

		if (opcode == op_callk) {
			uint16 kFuncNum = opparams[0];

			// Here we check for the kDoSound opcode that's used in SCI2.1.
			// Finding 0x40 as kDoSound in the Sound::play() function means the
			// game is using the modified SCI2 kernel table found in some older
			// SCI2.1 games (GK2 demo, KQ7 v1.4).
			// Finding 0x75 as kDoSound means the game is using the regular
			// SCI2.1 kernel table.
			if (kFuncNum == 0x40) {
				_sci21KernelType = SCI_VERSION_2;
				return true;
			} else if (kFuncNum == 0x75) {
				_sci21KernelType = SCI_VERSION_2_1_EARLY;
				return true;
			}
		}
	}

	return false;	// not found
}

SciVersion GameFeatures::detectSci21KernelType() {
	if (_sci21KernelType == SCI_VERSION_NONE) {
		if (!autoDetectSci21KernelType())
			error("Could not detect the SCI2.1 kernel table type");

		debugC(1, kDebugLevelVM, "Detected SCI2.1 kernel type: %s", getSciVersionDesc(_sci21KernelType));
	}
	return _sci21KernelType;
}
#endif

bool GameFeatures::supportsSpeechWithSubtitles() const {
	switch (g_sci->getGameId()) {
	case GID_SQ4:
	case GID_FREDDYPHARKAS:
	case GID_ECOQUEST:
	case GID_LSL6:
	case GID_LAURABOW2:
	case GID_KQ6:
#ifdef ENABLE_SCI32
	case GID_GK1:
	case GID_KQ7:
	case GID_LSL6HIRES:
	case GID_LSL7:
	case GID_PQ4:
	case GID_QFG4:
	case GID_SQ6:
	case GID_TORIN:
#endif
		return true;

	default:
		return false;
	}
}

bool GameFeatures::audioVolumeSyncUsesGlobals() const {
	switch (g_sci->getGameId()) {
	case GID_GK1:
	case GID_GK2:
	case GID_LSL6:
	case GID_LSL6HIRES:
	case GID_LSL7:
	case GID_PHANTASMAGORIA:
	case GID_PHANTASMAGORIA2:
	case GID_RAMA:
	case GID_TORIN:
		return true;
	case GID_HOYLE5:
		// Hoyle school house math does not use a volume global
		return !g_sci->getResMan()->testResource(ResourceId(kResourceTypeView, 21));
	default:
		return false;
	}
}

MessageTypeSyncStrategy GameFeatures::getMessageTypeSyncStrategy() const {
	if (getSciVersion() < SCI_VERSION_1_1) {
		return kMessageTypeSyncStrategyNone;
	}

	if (getSciVersion() == SCI_VERSION_1_1 && g_sci->isCD()) {
		return kMessageTypeSyncStrategyDefault;
	}

#ifdef ENABLE_SCI32
	switch (g_sci->getGameId()) {
	case GID_GK1:
	case GID_PQ4:
	case GID_QFG4:
		return g_sci->isCD() ? kMessageTypeSyncStrategyDefault : kMessageTypeSyncStrategyNone;

	case GID_KQ7:
	case GID_LSL7:
	case GID_MOTHERGOOSEHIRES:
	case GID_PHANTASMAGORIA:
	case GID_TORIN:
		return kMessageTypeSyncStrategyDefault;

	case GID_LSL6HIRES:
		return kMessageTypeSyncStrategyLSL6Hires;

	case GID_SHIVERS:
		return kMessageTypeSyncStrategyShivers;

	case GID_SQ6:
		// don't sync the early demos; they are speechless and
		// require the message type global to remain unchanged.
		return (g_sci->isDemo() && getSciVersion() < SCI_VERSION_2_1_MIDDLE) ?
			kMessageTypeSyncStrategyNone :
			kMessageTypeSyncStrategyDefault;

	case GID_GK2:
	case GID_PQSWAT:
	default:
		break;
	}
#endif

	return kMessageTypeSyncStrategyNone;
}

int GameFeatures::detectPlaneIdBase() {
	if (getSciVersion() == SCI_VERSION_2 &&
	    g_sci->getGameId() != GID_PQ4)
		return 0;
	else
		return 20000;
}

bool GameFeatures::autoDetectMoveCountType() {
	// Look up the script address
	reg_t addr = getDetectionAddr("Motion", SELECTOR(doit));

	if (!addr.getSegment())
		return false;

	uint32 offset = addr.getOffset();
	Script *script = _segMan->getScript(addr.getSegment());
	bool foundTarget = false;

	while (true) {
		int16 opparams[4];
		byte extOpcode;
		byte opcode;
		offset += readPMachineInstruction(script->getBuf(offset), extOpcode, opparams);
		opcode = extOpcode >> 1;

		// Check for end of script
		if (opcode == op_ret || offset >= script->getBufSize())
			break;

		if (opcode == op_callk) {
			uint16 kFuncNum = opparams[0];

			// Games which ignore move count call kAbs before calling kDoBresen
			if (_kernel->getKernelName(kFuncNum) == "Abs") {
				foundTarget = true;
			} else if (_kernel->getKernelName(kFuncNum) == "DoBresen") {
				_moveCountType = foundTarget ? kIgnoreMoveCount : kIncrementMoveCount;
				return true;
			}
		}
	}

	return false;	// not found
}

MoveCountType GameFeatures::detectMoveCountType() {
	if (_moveCountType == kMoveCountUninitialized) {
		// SCI0/SCI01 games always increment move count
		if (getSciVersion() <= SCI_VERSION_01) {
			_moveCountType = kIncrementMoveCount;
		} else if (getSciVersion() >= SCI_VERSION_1_1) {
			// SCI1.1 and newer games always ignore move count
			_moveCountType = kIgnoreMoveCount;
		} else {
			if (!autoDetectMoveCountType()) {
				error("Move count autodetection failed");
			}
		}

		debugC(1, kDebugLevelVM, "Detected move count handling: %s", (_moveCountType == kIncrementMoveCount) ? "increment" : "ignore");
	}

	return _moveCountType;
}

bool GameFeatures::useAltWinGMSound() {
	if (g_sci && g_sci->getPlatform() == Common::kPlatformWindows && g_sci->isCD() && !_forceDOSTracks) {
		SciGameId id = g_sci->getGameId();
		return (id == GID_ECOQUEST ||
				id == GID_JONES ||
				id == GID_KQ5 ||
				//id == GID_FREDDYPHARKAS ||	// Has alternate tracks, but handles them differently
				id == GID_SQ4);
	} else {
		return false;
	}
}

bool GameFeatures::generalMidiOnly() {
#ifdef ENABLE_SCI32
	switch (g_sci->getGameId()) {
	case GID_MOTHERGOOSEHIRES:
		return (g_sci->getPlatform() != Common::kPlatformMacintosh);

	case GID_KQ7: {
		if (g_sci->isDemo()) {
			return false;
		}

		SoundResource sound(13, g_sci->getResMan(), detectDoSoundType());
		return (sound.exists() && sound.getTrackByType(/* AdLib */ 0) == nullptr);
	}
	default:
		 if (g_sci->getPlatform() == Common::kPlatformMacintosh &&
			 getSciVersion() >= SCI_VERSION_2_1_MIDDLE) {
			 return true;
		 }
		break;
	}
#endif

	return false;
}

// PseudoMouse was added during SCI1
// PseudoMouseAbility is about a tiny difference in the keyboard driver, which sets the event type to either
// 40h (old behaviour) or 44h (the keyboard driver actually added 40h to the existing value).
// See engine/kevent.cpp, kMapKeyToDir - also script 933

// SCI1EGA:
// Quest for Glory 2 still used the old way.
//
// SCI1EARLY:
// King's Quest 5 0.000.062 uses the old way.
// Leisure Suit Larry 1 demo uses the new way, but no PseudoMouse class.
// Fairy Tales uses the new way.
// X-Mas 1990 uses the old way, no PseudoMouse class.
// Space Quest 4 floppy (1.1) uses the new way.
// Mixed Up Mother Goose uses the old way, no PseudoMouse class.
//
// SCI1MIDDLE:
// Leisure Suit Larry 5 demo uses the new way.
// Conquests of the Longbow demo uses the new way.
// Leisure Suit Larry 1 (2.0) uses the new way.
// Astro Chicken II uses the new way.
PseudoMouseAbilityType GameFeatures::detectPseudoMouseAbility() {
	if (_pseudoMouseAbility == kPseudoMouseAbilityUninitialized) {
		if (getSciVersion() < SCI_VERSION_1_EARLY) {
			// SCI1 EGA or earlier -> pseudo mouse ability is always disabled
			_pseudoMouseAbility = kPseudoMouseAbilityFalse;

		} else if (getSciVersion() == SCI_VERSION_1_EARLY) {
			// For SCI1 early some games had it enabled, some others didn't.
			// We try to find an object called "PseudoMouse". If it's found, we enable the ability otherwise we don't.
			reg_t pseudoMouseAddr = _segMan->findObjectByName("PseudoMouse", 0);

			if (pseudoMouseAddr != NULL_REG) {
				_pseudoMouseAbility = kPseudoMouseAbilityTrue;
			} else {
				_pseudoMouseAbility = kPseudoMouseAbilityFalse;
			}

		} else {
			// SCI1 middle or later -> pseudo mouse ability is always enabled
			_pseudoMouseAbility = kPseudoMouseAbilityTrue;
		}
	}
	return _pseudoMouseAbility;
}

// GetLongest(), which calculates the number of characters in a string that can fit
//  within a width, had two subtle changes which started to appear in interpreters
//  in late 1990. An off-by-one bug was fixed where the character that exceeds the
//  width would be applied to the result if a space character hadn't been reached.
//  The pixel width test was also changed from a greater than or equals to greater
//  than, but again only if a space character hadn't been reached.
//
// The notebook in LB1 (bug #10000) is currently the only known script that depended
//  on the original behavior. This appears to be an isolated fix to an interpreter
//  edge case, a corresponding script change to allow autodetection hasn't been found.
//
// The Japanese interpreters have their own versions of GetLongest() to support
//  double byte characters which seems to be how QFG1 Japanese reintroduced it
//  even though its interpreter is later than SQ3/LSL3 multilingual versions.
bool GameFeatures::useEarlyGetLongestTextCalculations() const {
	switch (getSciVersion()) {

	// All SCI0, confirmed:
	// - LSL2 English PC 1.000.011
	// - LB1 PC 1.000.046
	// - ICEMAN PC 1.033
	// - SQ3 English PC 1.018
	// - PQ2 Japanese 1.000.052
	case SCI_VERSION_0_EARLY:
	case SCI_VERSION_0_LATE:
		return true;

	// SCI01: confirmed KQ1 and QFG1 Japanese,
	// fixed in SQ3 and LSL3 multilingual PC
	case SCI_VERSION_01:
		return (g_sci->getGameId() == GID_KQ1 || g_sci->getGameId() == GID_QFG1);

	// QFG2, confirmed 1.000 and 1.105 (first and last versions)
	case SCI_VERSION_1_EGA_ONLY:
		return true;

	// SCI1 Early: just KQ5 English PC versions,
	// confirmed fixed in:
	// - LSL1 Demo
	// - XMAS1990 EGA
	// - SQ4 1.052
	case SCI_VERSION_1_EARLY:
		return (g_sci->getGameId() == GID_KQ5);

	// Fixed in all other versions
	default:
		return false;
	}
}

bool GameFeatures::hasScriptObjectNames() const {
	switch (g_sci->getGameId()) {
	case GID_HOYLE4:
	case GID_LSL6:
	case GID_QFG1VGA:
		return (g_sci->getPlatform() != Common::kPlatformMacintosh);
	
	default:
		return true;
	}
}

bool GameFeatures::canSaveFromGMM() const {
	if (!ConfMan.getBool("gmm_save_enabled"))
		return false;

	switch (g_sci->getGameId()) {
	// ==== Demos/mini-games with no saving functionality ====
	case GID_ASTROCHICKEN:
	case GID_CHEST:
	case GID_CHRISTMAS1988:
	case GID_CHRISTMAS1990:
	case GID_CHRISTMAS1992:
	case GID_CNICK_KQ:
	case GID_CNICK_LAURABOW:
	case GID_CNICK_LONGBOW:
	case GID_CNICK_LSL:
	case GID_CNICK_SQ:
	case GID_FUNSEEKER:
	case GID_INNDEMO:
	case GID_KQUESTIONS:
	case GID_MSASTROCHICKEN:
	// ==== Games with a different saving scheme =============
	case GID_HOYLE1:
	case GID_HOYLE2:
	case GID_HOYLE3:
	case GID_HOYLE4:
	case GID_HOYLE5:
	case GID_JONES:
	case GID_MOTHERGOOSE:
	case GID_MOTHERGOOSE256:
	case GID_MOTHERGOOSEHIRES:
	case GID_PHANTASMAGORIA:
	case GID_RAMA:
	case GID_SLATER:
		return false;
	default:
		return true;
	}
}

uint16 GameFeatures::getGameFlagsGlobal() const {
	Common::Platform platform = g_sci->getPlatform();
	switch (g_sci->getGameId()) {
	case GID_CAMELOT: return 250;
	case GID_CASTLEBRAIN: return 250;
	case GID_ECOQUEST: return (getSciVersion() == SCI_VERSION_1_1) ? 152 : 150;
	case GID_ECOQUEST2: return 110;
	case GID_FAIRYTALES: return 250;
	case GID_FREDDYPHARKAS: return 186;
	case GID_GK1: return 127;
	case GID_GK2: return 150;
	// ICEMAN uses object properties
	case GID_ISLANDBRAIN: return 250;
	case GID_KQ1: return 150;
	// KQ4 has no flags
	case GID_KQ5: return 129;
	case GID_KQ6: return 137;
	case GID_KQ7: return 127;
	case GID_LAURABOW: return 440;
	case GID_LAURABOW2: return 186;
	case GID_LIGHTHOUSE: return 116;
	case GID_LONGBOW: return 200;
	case GID_LSL1: return 111;
	// LSL2 has no flags
	case GID_LSL3: return 111;
	case GID_LSL5: return 186;
	case GID_LSL6: return 137;
	// LSL6HIRES uses a flags object
	case GID_PEPPER: return 134;
	case GID_PHANTASMAGORIA: return 250;
	case GID_PHANTASMAGORIA2: return 101;
	case GID_PQ1: return 134;
	case GID_PQ2: return (platform != Common::kPlatformPC98) ? 250 : 245;
	case GID_PQ3: return 165;
	// PQ4 uses object properties
	case GID_PQSWAT: return 150;
	case GID_QFG1: return 350;
	case GID_QFG1VGA: return 290;
	case GID_QFG2: return 700;
	case GID_QFG3: return 500;
	case GID_QFG4: return 500;
	case GID_RAMA: return 300;
	case GID_SHIVERS: return 209;
	case GID_SQ1: return 118;
	case GID_SQ4: return 114;
	case GID_SQ5: return 183;
	case GID_SQ6: return 250;
	// TORIN uses a flags object
	default: return 0;
	}
}

bool GameFeatures::isGameFlagBitOrderNormal() const {
	// Most games store flags in reverse bit order
	switch (g_sci->getGameId()) {
	case GID_KQ5:
	case GID_LAURABOW:
	case GID_PEPPER:
	case GID_PQ1:
	case GID_PQ3:
		return true;
	default:
		return false;
	}
}

} // End of namespace Sci
