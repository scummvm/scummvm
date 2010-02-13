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

#include "sci/engine/features.h"
#include "sci/engine/script.h"
#include "sci/engine/selector.h"
#include "sci/engine/vm.h"

namespace Sci {

GameFeatures::GameFeatures(SegManager *segMan, Kernel *kernel) : _segMan(segMan), _kernel(kernel) {
	_setCursorType = SCI_VERSION_NONE;
	_doSoundType = SCI_VERSION_NONE;
	_lofsType = SCI_VERSION_NONE;
	_gfxFunctionsType = SCI_VERSION_NONE;
	_moveCountType = kMoveCountUninitialized;

#ifdef ENABLE_SCI32
	_sci21KernelType = SCI_VERSION_NONE;
#endif
	_usesCdTrack = Common::File::exists("cdaudio.map");
}

reg_t GameFeatures::getDetectionAddr(const Common::String &objName, Selector slc, int methodNum) {
	// Get address of target object
	reg_t objAddr = _segMan->findObjectByName(objName);
	reg_t addr;

	if (objAddr.isNull()) {
		warning("getDetectionAddr: %s object couldn't be found", objName.c_str());
		return NULL_REG;
	}

	if (methodNum == -1) {
		if (lookup_selector(_segMan, objAddr, slc, NULL, &addr) != kSelectorMethod) {
			warning("getDetectionAddr: target selector is not a method of object %s", objName.c_str());
			return NULL_REG;
		}
	} else {
		addr = _segMan->getObject(objAddr)->getFunction(methodNum);
	}

	return addr;
}

bool GameFeatures::autoDetectSoundType() {
	// Look up the script address
	reg_t addr = getDetectionAddr("Sound", _kernel->_selectorCache.play);

	uint16 offset = addr.offset;
	Script *script = _segMan->getScript(addr.segment);
	uint16 intParam = 0xFFFF;
	bool foundTarget = false;

	while (true) {
		int16 opparams[4];
		byte extOpcode;
		byte opcode;
		offset += readPMachineInstruction(script->_buf + offset, extOpcode, opparams);
		opcode = extOpcode >> 1;

		// Check for end of script
		if (opcode == op_ret || offset >= script->_bufSize)
			break;

		// The play method of the Sound object pushes the DoSound command
		// that it'll use just before it calls DoSound. We intercept that here
		// in order to check what sound semantics are used, cause the position
		// of the sound commands has changed at some point during SCI1 middle
		if (opcode == op_pushi) {
			// Load the pushi parameter
			intParam = opparams[0];
		} else if (opcode == op_callk) {
			uint16 kFuncNum = opparams[0];

			// Late SCI1 games call kIsObject before kDoSound
			if (kFuncNum == 6) {	// kIsObject (SCI0-SCI11)
				foundTarget = true;
			} else if (kFuncNum == 45) {	// kDoSound (SCI1)
				// First, check which DoSound function is called by the play method of
				// the Sound object
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
					// alternative detection here, which works in general, apart from
					// some transitive games like Jones CD
					_doSoundType = foundTarget ? SCI_VERSION_1_LATE : SCI_VERSION_1_EARLY;
					break;
				}

				if (_doSoundType != SCI_VERSION_NONE)
					return true;
			}
		}
	}

	return false;	// not found
}

SciVersion GameFeatures::detectDoSoundType() {
	if (_doSoundType == SCI_VERSION_NONE) {
		if (getSciVersion() == SCI_VERSION_0_EARLY) {
			// This game is using early SCI0 sound code (different headers than SCI0 late)
			_doSoundType = SCI_VERSION_0_EARLY;
		} else if (_kernel->_selectorCache.nodePtr == -1) {
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
			// If the Cursor object doesn't exist, we're using the SCI0 early kSetCursor semantics.
			if (_segMan->findObjectByName("Cursor") == NULL_REG) {
				_setCursorType = SCI_VERSION_0_EARLY;
				debugC(1, kDebugLevelGraphics, "Detected SetCursor type: %s", getSciVersionDesc(_setCursorType));
				return _setCursorType;
			}

			// Check for the existence of the handCursor object (first found). This is based on KQ5.
			reg_t objAddr = _segMan->findObjectByName("handCursor", 0);

			// If that doesn't exist, we assume it uses SCI1.1 kSetCursor semantics
			if (objAddr == NULL_REG) {
				_setCursorType = SCI_VERSION_1_1;
				debugC(1, kDebugLevelGraphics, "Detected SetCursor type: %s", getSciVersionDesc(_setCursorType));
				return _setCursorType;
			}

			// Now we check what the number variable holds in the handCursor object.
			uint16 number = GET_SEL32V(_segMan, objAddr, SELECTOR(number));

			// If the number is 0, it uses views and therefore the SCI1.1 kSetCursor semantics,
			// otherwise it uses the SCI0 early kSetCursor semantics.
			if (number == 0)
				_setCursorType = SCI_VERSION_1_1;
			else
				_setCursorType = SCI_VERSION_0_EARLY;
		}

		debugC(1, kDebugLevelGraphics, "Detected SetCursor type: %s", getSciVersionDesc(_setCursorType));
	}

	return _setCursorType;
}

bool GameFeatures::autoDetectLofsType(int methodNum) {
	// Look up the script address
	reg_t addr = getDetectionAddr("Game", -1, methodNum);

	uint16 offset = addr.offset;
	Script *script = _segMan->getScript(addr.segment);

	while (true) {
		int16 opparams[4];
		byte extOpcode;
		byte opcode;
		offset += readPMachineInstruction(script->_buf + offset, extOpcode, opparams);
		opcode = extOpcode >> 1;

		// Check for end of script
		if (opcode == op_ret || offset >= script->_bufSize)
			break;

		if (opcode == op_lofsa || opcode == op_lofss) {
			// Load lofs operand
			uint16 lofs = opparams[0];

			// Check for going out of bounds when interpreting as abs/rel
			if (lofs >= script->_bufSize)
				_lofsType = SCI_VERSION_0_EARLY;

			if ((signed)offset + (int16)lofs < 0)
				_lofsType = SCI_VERSION_1_MIDDLE;

			if ((signed)offset + (int16)lofs >= (signed)script->_bufSize)
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

		if (getSciVersion() >= SCI_VERSION_1_1) {
			_lofsType = SCI_VERSION_1_1;
			return _lofsType;
		}

		// Find a function of the game object which invokes lofsa/lofss
		reg_t gameClass = _segMan->findObjectByName("Game");
		Object *obj = _segMan->getObject(gameClass);
		bool found = false;

		for (uint m = 0; m < obj->getMethodCount(); m++) {
			found = autoDetectLofsType(m);

			if (found)
				break;
		}

		if (!found) {
			warning("Lofs detection failed, taking an educated guess");

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
	reg_t addr = getDetectionAddr("Rm", _kernel->_selectorCache.overlay, methodNum);

	uint16 offset = addr.offset;
	Script *script = _segMan->getScript(addr.segment);

	while (true) {
		int16 opparams[4];
		byte extOpcode;
		byte opcode;
		offset += readPMachineInstruction(script->_buf + offset, extOpcode, opparams);
		opcode = extOpcode >> 1;

		// Check for end of script
		if (opcode == op_ret || offset >= script->_bufSize)
			break;

		if (opcode == op_callk) {
			uint16 kFuncNum = opparams[0];
			uint16 argc = opparams[1];

			if (kFuncNum == 8) {	// kDrawPic	(SCI0 - SCI11)
				// If kDrawPic is called with 6 parameters from the
				// overlay selector, the game is using old graphics functions.
				// Otherwise, if it's called with 8 parameters, it's using new
				// graphics functions
				_gfxFunctionsType = (argc == 8) ? SCI_VERSION_0_LATE : SCI_VERSION_0_EARLY;
				return true;
			}
		}
	}

	return false;	// not found
}

SciVersion GameFeatures::detectGfxFunctionsType() {
	if (_gfxFunctionsType == SCI_VERSION_NONE) {
		// This detection only works (and is only needed) for SCI0 games
		if (getSciVersion() >= SCI_VERSION_01) {
			_gfxFunctionsType = SCI_VERSION_0_LATE;
		} else if (getSciVersion() > SCI_VERSION_0_EARLY) {
			// Check if the game is using an overlay
			bool found = false;

			if (_kernel->_selectorCache.overlay == -1) {
				// No overlay selector found, check if any method of the Rm object
				// is calling kDrawPic, as the overlay selector might be missing in demos

				Object *obj = _segMan->getObject(_segMan->findObjectByName("Rm"));
				for (uint m = 0; m < obj->getMethodCount(); m++) {
					found = autoDetectGfxFunctionsType(m);
					if (found)
						break;
				}

				if (!found) {
					// No overlay selector found, therefore the game is definitely
					// using old graphics functions
					_gfxFunctionsType = SCI_VERSION_0_EARLY;
				}
			} else {	// _kernel->_selectorCache.overlay != -1
				// An in-between case: The game does not have a shiftParser
				// selector, but it does have an overlay selector, so it uses an
				// overlay. Therefore, check it to see how it calls kDrawPic to
				// determine the graphics functions type used

				if (!autoDetectGfxFunctionsType()) {
					warning("Graphics functions detection failed, taking an educated guess");

					// Try detecting the graphics function types from the existence of the motionCue
					// selector (which is a bit of a hack)
					if (_kernel->findSelector("motionCue") != -1)
						_gfxFunctionsType = SCI_VERSION_0_LATE;
					else
						_gfxFunctionsType = SCI_VERSION_0_EARLY;
				}
			}
		} else {	// (getSciVersion() == SCI_VERSION_0_EARLY)
			// Old SCI0 games always used old graphics functions
			_gfxFunctionsType = SCI_VERSION_0_EARLY;
		}

		debugC(1, kDebugLevelVM, "Detected graphics functions type: %s", getSciVersionDesc(_gfxFunctionsType));
	}

	return _gfxFunctionsType;
}

#ifdef ENABLE_SCI32
bool GameFeatures::autoDetectSci21KernelType() {
	// Look up the script address
	reg_t addr = getDetectionAddr("Sound", _kernel->_selectorCache.play);

	uint16 offset = addr.offset;
	Script *script = _segMan->getScript(addr.segment);

	while (true) {
		int16 opparams[4];
		byte extOpcode;
		byte opcode;
		offset += readPMachineInstruction(script->_buf + offset, extOpcode, opparams);
		opcode = extOpcode >> 1;

		// Check for end of script
		if (opcode == op_ret || offset >= script->_bufSize)
			break;

		if (opcode == op_callk) {
			uint16 kFuncNum = opparams[0];

			// Here we check for the kDoSound opcode that's used in SCI2.1.
			// Finding 0x40 as kDoSound in the Sound::play() function means the game is using
			// the modified SCI2 kernel table found in some older SCI2.1 games (GK2 demo, KQ7 v1.4).
			// Finding 0x75 as kDoSound means the game is using the regular SCI2.1 kernel table.
			if (kFuncNum == 0x40) {
				_sci21KernelType = SCI_VERSION_2;
				return true;
			} else if (kFuncNum == 0x75) {
				_sci21KernelType = SCI_VERSION_2_1;
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

bool GameFeatures::autoDetectMoveCountType() {
	// Look up the script address
	reg_t addr = getDetectionAddr("Motion", _kernel->_selectorCache.doit);

	uint16 offset = addr.offset;
	Script *script = _segMan->getScript(addr.segment);
	bool foundTarget = false;

	while (true) {
		int16 opparams[4];
		byte extOpcode;
		byte opcode;
		offset += readPMachineInstruction(script->_buf + offset, extOpcode, opparams);
		opcode = extOpcode >> 1;

		// Check for end of script
		if (opcode == op_ret || offset >= script->_bufSize)
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
		} else {
			if (!autoDetectMoveCountType()) {
				warning("Move count autodetection failed");
				_moveCountType = kIncrementMoveCount;	// Most games do this, so best guess
			}
		}

		debugC(1, kDebugLevelVM, "Detected move count handling: %s", (_moveCountType == kIncrementMoveCount) ? "increment" : "ignore");
	}

	return _moveCountType;
}

} // End of namespace Sci
