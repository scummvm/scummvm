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

void GameFeatures::setGameInfo(reg_t gameObj, Common::String gameId) {
	_gameObj = gameObj;
	_gameId = gameId;
}

bool GameFeatures::autoDetectFeature(FeatureDetection featureDetection, int methodNum) {
	Common::String objName;
	Selector slc = 0;
	reg_t objAddr;
	bool foundTarget = false;

	// Get address of target script
	switch (featureDetection) {
	case kDetectGfxFunctions:
		objName = "Rm";
		objAddr = _segMan->findObjectByName(objName);
		slc = _kernel->_selectorCache.overlay;
		break;
	case kDetectMoveCountType:
		objName = "Motion";
		objAddr = _segMan->findObjectByName(objName);
		slc = _kernel->_selectorCache.doit;
		break;
	case kDetectSoundType:
		objName = "Sound";
		objAddr = _segMan->findObjectByName(objName);
		slc = _kernel->_selectorCache.play;
		break;
	case kDetectSetCursorType:
		objName = "Game";
		objAddr = _segMan->findObjectByName(objName);
		// KQ5CD overrides the default setCursor selector of the Game object,
		// so we need to handle this separately
		// KQ5 PC floppy is early SCI1, Amiga middle SCI1, and CD late SCI1
		assert(!_gameId.empty());
		if (_gameId == "kq5" && getSciVersion() == SCI_VERSION_1_LATE)
			objAddr = _gameObj;
		slc = _kernel->_selectorCache.setCursor;
		break;
	case kDetectLofsType:
		objName = "Game";
		objAddr = _segMan->findObjectByName(objName);
		break;
#ifdef ENABLE_SCI32
	case kDetectSci21KernelTable:
		objName = "Sound";
		objAddr = _segMan->findObjectByName(objName);
		slc = _kernel->_selectorCache.play;
		break;
#endif
	default:
		warning("autoDetectFeature: invalid featureDetection value %x", featureDetection);
		return false;
	}

	reg_t addr;
	if (objAddr.isNull()) {
		warning("autoDetectFeature: %s object couldn't be found", objName.c_str());
		return false;
	}

	if (methodNum == -1) {
		if (lookup_selector(_segMan, objAddr, slc, NULL, &addr) != kSelectorMethod) {
			warning("autoDetectFeature: target selector is not a method of object %s", objName.c_str());
			return false;
		}
	} else {
		addr = _segMan->getObject(objAddr)->getFunction(methodNum);
	}

	uint16 offset = addr.offset;
	Script *script = _segMan->getScript(addr.segment);
	uint16 intParam = 0xFFFF;

	do {
		uint16 kFuncNum;
		int opsize = script->_buf[offset++];
		uint opcode = opsize >> 1;
		int i = 0;
		byte argc;

		if (featureDetection == kDetectLofsType) {
			if (opcode == op_lofsa || opcode == op_lofss) {
				uint16 lofs;

				// Load lofs operand
				if (opsize & 1) {
					if (offset >= script->_bufSize)
						break;
					lofs = script->_buf[offset++];
				} else {
					if ((uint32)offset + 1 >= (uint32)script->_bufSize)
						break;
					lofs = READ_LE_UINT16(script->_buf + offset);
					offset += 2;
				}

				// Check for going out of bounds when interpreting as abs/rel
				if (lofs >= script->_bufSize)
					_lofsType = SCI_VERSION_0_EARLY;

				if ((signed)offset + (int16)lofs < 0)
					_lofsType = SCI_VERSION_1_MIDDLE;

				if ((signed)offset + (int16)lofs >= (signed)script->_bufSize)
					_lofsType = SCI_VERSION_1_MIDDLE;

				if (_lofsType != SCI_VERSION_NONE)
					return true;

				// If we reach here, we haven't been able to deduce the lofs parameter
				// type, but we have advanced the offset pointer already. So move on
				// to the next opcode
				continue;
			}
		}

		if (featureDetection == kDetectSoundType) {
			// The play method of the Sound object pushes the DoSound command
			// that it'll use just before it calls DoSound. We intercept that here
			// in order to check what sound semantics are used, cause the position
			// of the sound commands has changed at some point during SCI1 middle
			if (opcode == op_pushi) {
				// Load the pushi parameter
				if (opsize & 1) {
					if (offset >= script->_bufSize)
						break;
					intParam = script->_buf[offset++];
				} else {
					if ((uint32)offset + 1 >= (uint32)script->_bufSize)
						break;
					intParam = READ_LE_UINT16(script->_buf + offset);
					offset += 2;
				}

				continue;
			}
		}

		while (g_opcode_formats[opcode][i]) {
			switch (g_opcode_formats[opcode][i++]) {
			case Script_Invalid:
				break;
			case Script_SByte:
			case Script_Byte:
				offset++;
				break;
			case Script_Word:
			case Script_SWord:
				offset += 2;
				break;
			case Script_SVariable:
			case Script_Variable:
			case Script_Property:
			case Script_Global:
			case Script_Local:
			case Script_Temp:
			case Script_Param:
				if (opsize & 1)
					kFuncNum = script->_buf[offset++];
				else {
					kFuncNum = 0xffff & (script->_buf[offset] | (script->_buf[offset + 1] << 8));
					offset += 2;
				}

				if (opcode == op_callk) {
					argc = script->_buf[offset++];

					switch (featureDetection) {
					case kDetectGfxFunctions:
						if (kFuncNum == 8) {	// kDrawPic	(SCI0 - SCI11)
							// If kDrawPic is called with 6 parameters from the
							// overlay selector, the game is using old graphics functions.
							// Otherwise, if it's called with 8 parameters, it's using new
							// graphics functions
							_gfxFunctionsType = (argc == 8) ? SCI_VERSION_0_LATE : SCI_VERSION_0_EARLY;
							return true;
						}
						break;
					case kDetectMoveCountType:
						// Games which ignore move count call kAbs before calling kDoBresen
						if (_kernel->getKernelName(kFuncNum) == "Abs") {
							foundTarget = true;
						} else if (_kernel->getKernelName(kFuncNum) == "DoBresen") {
							_moveCountType = foundTarget ? kIgnoreMoveCount : kIncrementMoveCount;
							return true;
						}
						break;
					case kDetectSoundType:
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
						break;
					case kDetectSetCursorType:
						// Games with colored mouse cursors call kIsObject before kSetCursor
						if (kFuncNum == 6) {	// kIsObject (SCI0-SCI11)
							foundTarget = true;
						} else if (kFuncNum == 40) {	// kSetCursor (SCI0-SCI11)
							_setCursorType = foundTarget ? SCI_VERSION_1_1 : SCI_VERSION_0_EARLY;
							return true;
						}
						break;
#ifdef ENABLE_SCI32
					case kDetectSci21KernelTable:
							if (kFuncNum == 0x40) {
								_sci21KernelType = SCI_VERSION_2;
								return true;
							} else if (kFuncNum == 0x75) {
								_sci21KernelType = SCI_VERSION_2_1;
								return true;
							}
						break;
#endif
					default:
						break;
					}
				}
				break;

			case Script_Offset:
			case Script_SRelative:
				offset++;
				if (!opsize & 1)
					offset++;
				break;
			case Script_End:
				offset = 0;	// exit loop
				break;
			default:
				warning("opcode %02x: Invalid", opcode);

			}
		}
	} while (offset > 0);

	// Some games, like KQ5CD, never actually call SetCursor inside Game::setCursor
	// but call isObject. Cover this case here, if we're actually reading the selector
	// itself, and not iterating through the Game object (i.e. when the selector
	// dictionary is missing)
	if (featureDetection == kDetectSetCursorType && methodNum == -1 && foundTarget) {
		_setCursorType = SCI_VERSION_1_1;
		return true;
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
		} else {
			if (getSciVersion() >= SCI_VERSION_1_LATE) {
				// All SCI1 late games use the newer doSound semantics
				_doSoundType = SCI_VERSION_1_LATE;
			} else {
				if (!autoDetectFeature(kDetectSoundType)) {
					warning("DoSound detection failed, taking an educated guess");

					if (getSciVersion() >= SCI_VERSION_1_MIDDLE)
						_doSoundType = SCI_VERSION_1_LATE;
					else if (getSciVersion() > SCI_VERSION_01)
						_doSoundType = SCI_VERSION_1_EARLY;
				}
			}
		}

		debugC(1, kDebugLevelSound, "Detected DoSound type: %s", getSciVersionDesc(_doSoundType).c_str());
	}

	return _doSoundType;
}

SciVersion GameFeatures::detectSetCursorType() {
	if (_setCursorType == SCI_VERSION_NONE) {
		if (getSciVersion() <= SCI_VERSION_01) {
			// SCI0/SCI01 games never use cursor views
			_setCursorType = SCI_VERSION_0_EARLY;
		} else if (getSciVersion() >= SCI_VERSION_1_EARLY && getSciVersion() <= SCI_VERSION_1_MIDDLE) {
			// SCI1 early/SCI1 middle games never use cursor views
			_setCursorType = SCI_VERSION_0_EARLY;
		} else if (getSciVersion() >= SCI_VERSION_1_1) {
			// SCI1.1 games always use cursor views
			_setCursorType = SCI_VERSION_1_1;
		} else {	// SCI1 late game, detect cursor semantics
			bool found = false;

			if (_kernel->_selectorCache.setCursor == -1) {
				// Find which function of the Game object calls setCursor

				Object *obj = _segMan->getObject(_gameObj);
				for (uint m = 0; m < obj->getMethodCount(); m++) {
					found = autoDetectFeature(kDetectSetCursorType, m);
					if (found)
						break;
				}
			} else {
				found = autoDetectFeature(kDetectSetCursorType);
			}

			if (!found) {
				// Quite normal in several demos which don't have a cursor
				warning("SetCursor detection failed, taking an educated guess");

				if (getSciVersion() >= SCI_VERSION_1_1)
					_setCursorType = SCI_VERSION_1_1;
				else
					_setCursorType = SCI_VERSION_0_EARLY;
			}
		}

		debugC(1, kDebugLevelGraphics, "Detected SetCursor type: %s", getSciVersionDesc(_setCursorType).c_str());
	}

	return _setCursorType;
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
			found = autoDetectFeature(kDetectLofsType, m);

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

		debugC(1, kDebugLevelVM, "Detected Lofs type: %s", getSciVersionDesc(_lofsType).c_str());
	}

	return _lofsType;
}

SciVersion GameFeatures::detectGfxFunctionsType() {
	if (_gfxFunctionsType == SCI_VERSION_NONE) {
		// This detection only works (and is only needed) for SCI0 games
		if (getSciVersion() >= SCI_VERSION_01) {
			_gfxFunctionsType = SCI_VERSION_0_LATE;
			return _gfxFunctionsType;
		}

		if (getSciVersion() > SCI_VERSION_0_EARLY) {
			// Check if the game is using an overlay
			bool found = false;

			if (_kernel->_selectorCache.overlay == -1) {
				// No overlay selector found, check if any method of the Rm object
				// is calling kDrawPic, as the overlay selector might be missing in demos

				Object *obj = _segMan->getObject(_segMan->findObjectByName("Rm"));
				for (uint m = 0; m < obj->getMethodCount(); m++) {
					found = autoDetectFeature(kDetectGfxFunctions, m);
					if (found)
						break;
				}
			}

			if (_kernel->_selectorCache.overlay == -1 && !found) {
				// No overlay selector found, therefore the game is definitely
				// using old graphics functions
				_gfxFunctionsType = SCI_VERSION_0_EARLY;
			} else if (_kernel->_selectorCache.overlay == -1 && found) {
				// Detection already done above
			} else {	// _kernel->_selectorCache.overlay != -1
				// An in-between case: The game does not have a shiftParser
				// selector, but it does have an overlay selector, so it uses an
				// overlay. Therefore, check it to see how it calls kDrawPic to
				// determine the graphics functions type used

				if (!autoDetectFeature(kDetectGfxFunctions)) {
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

		debugC(1, kDebugLevelVM, "Detected graphics functions type: %s", getSciVersionDesc(_gfxFunctionsType).c_str());
	}

	return _gfxFunctionsType;
}

#ifdef ENABLE_SCI32
SciVersion GameFeatures::detectSci21KernelType() {
	if (_sci21KernelType == SCI_VERSION_NONE)
		if (!autoDetectFeature(kDetectSci21KernelTable))
			error("Could not detect the SCI2.1 kernel table type");

	debugC(1, kDebugLevelVM, "Detected SCI2.1 kernel type: %s", getSciVersionDesc(_sci21KernelType).c_str());

	return _sci21KernelType;
}
#endif

MoveCountType GameFeatures::detectMoveCountType() {
	if (_moveCountType == kMoveCountUninitialized) {
		// SCI0/SCI01 games always increment move count
		if (getSciVersion() <= SCI_VERSION_01) {
			_moveCountType = kIncrementMoveCount;
		} else {
			if (!autoDetectFeature(kDetectMoveCountType)) {
				warning("Move count autodetection failed");
				_moveCountType = kIncrementMoveCount;	// Most games do this, so best guess
			}
		}

		debugC(1, kDebugLevelVM, "Detected move count handling: %s", (_moveCountType == kIncrementMoveCount) ? "increment" : "ignore");
	}

	return _moveCountType;
}

} // End of namespace Sci
