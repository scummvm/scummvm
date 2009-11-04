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

#include "sci/sci.h"	// for INCLUDE_OLDGFX

#include "sci/engine/state.h"
#include "sci/engine/vm.h"
#include "sci/engine/script.h"
#include "sci/engine/message.h"

namespace Sci {

EngineState::EngineState(ResourceManager *res, Kernel *kernel, Vocabulary *voc, SciGui *gui, SciGuiCursor *cursor, AudioPlayer *audio)
: resMan(res), _kernel(kernel), _voc(voc), _gui(gui), _cursor(cursor), _audio(audio), _dirseeker(this) {

	gfx_state = 0;

	sfx_init_flags = 0;
	sound_volume = 0;
	sound_mute = 0;

	restarting_flags = 0;

#ifdef INCLUDE_OLDGFX
	pic_priority_table = 0;
	pic_not_valid = 0;
	pic_is_new = 0;
	old_screen = 0;
	port = 0;
	memset(ega_colors, 0, sizeof(ega_colors));
	visual = 0;
	titlebar_port = 0;
	wm_port = 0;
	picture_port = 0;
	iconbar_port = 0;

	pic_visible_map = GFX_MASK_NONE;
	pic_animate = 0;

	dyn_views = 0;
	drop_views = 0;
#endif

	_menubar = 0;

	priority_first = 0;
	priority_last = 0;

	last_wait_time = 0;

	_fileHandles.resize(5);

	execution_stack_base = 0;
	_executionStackPosChanged = false;

	r_acc = NULL_REG;
	restAdjust = 0;
	r_prev = NULL_REG;

	stack_segment = 0;
	stack_base = 0;
	stack_top = 0;

	parser_base = NULL_REG;
	parser_event = NULL_REG;
	script_000 = 0;

	bp_list = 0;
	have_bp = 0;
	sys_strings_segment = 0;
	sys_strings = 0;

	parserIsValid = false;

	_gameObj = NULL_REG;

	_segMan = 0;
	gc_countdown = 0;

	successor = 0;

	_lastAnimateCounter = 0;
	_lastAnimateTime = 0;

	_setCursorType = SCI_VERSION_AUTODETECT;
	_doSoundType = SCI_VERSION_AUTODETECT;
	_lofsType = SCI_VERSION_AUTODETECT;
	_gfxFunctionsType = SCI_VERSION_AUTODETECT;
	_moveCountType = kMoveCountUninitialized;
	
	_audioCdStart = 0;
	_usesCdTrack = Common::File::exists("cdaudio.map");
}

EngineState::~EngineState() {
	delete _msgState;
}

uint16 EngineState::currentRoomNumber() const {
	return script_000->_localsBlock->_locals[13].toUint16();
}

kLanguage EngineState::charToLanguage(const char c) const {
	switch (c) {
	case 'F':
		return K_LANG_FRENCH;
	case 'S':
		return K_LANG_SPANISH;
	case 'I':
		return K_LANG_ITALIAN;
	case 'G':
		return K_LANG_GERMAN;
	case 'J':
	case 'j':
		return K_LANG_JAPANESE;
	case 'P':
		return K_LANG_PORTUGUESE;
	default:
		return K_LANG_NONE;
	}
}

Common::String EngineState::getLanguageString(const char *str, kLanguage lang) const {
	kLanguage secondLang = K_LANG_NONE;

	const char *seeker = str;
	while (*seeker) {
		if ((*seeker == '%') || (*seeker == '#')) {
			secondLang = charToLanguage(*(seeker + 1));

			if (secondLang != K_LANG_NONE)
				break;
		}

		seeker++;
	}

	if ((secondLang == K_LANG_JAPANESE) && (*(seeker + 1) == 'J')) {
		// FIXME: Add Kanji support
		lang = K_LANG_ENGLISH;
	}

	if (secondLang == lang)
		return Common::String(seeker + 2);

	if (seeker)
		return Common::String(str, seeker - str);
	else
		return Common::String(str);
}

kLanguage EngineState::getLanguage() {
	kLanguage lang = K_LANG_ENGLISH;

	if (_kernel->_selectorCache.printLang != -1) {
		lang = (kLanguage)GET_SEL32V(_segMan, _gameObj, printLang);

		if ((getSciVersion() == SCI_VERSION_1_1) || (lang == K_LANG_NONE)) {
			// If language is set to none, we use the language from the game detector.
			// SSCI reads this from resource.cfg (early games do not have a language
			// setting in resource.cfg, but instead have the secondary language number
			// hardcoded in the game script).
			// SCI1.1 games always use the language setting from the config file
			// (essentially disabling runtime language switching).
			// Note: only a limited number of multilanguage games have been tested
			// so far, so this information may not be 100% accurate.
			switch (((Sci::SciEngine*)g_engine)->getLanguage()) {
			case Common::FR_FRA:
				lang = K_LANG_FRENCH;
				break;
			case Common::ES_ESP:
				lang = K_LANG_SPANISH;
				break;
			case Common::IT_ITA:
				lang = K_LANG_ITALIAN;
				break;
			case Common::DE_DEU:
				lang = K_LANG_GERMAN;
				break;
			case Common::JA_JPN:
				lang = K_LANG_JAPANESE;
				break;
			case Common::PT_BRA:
				lang = K_LANG_PORTUGUESE;
				break;
			default:
				lang = K_LANG_ENGLISH;
			}

			// Store language in printLang selector
			PUT_SEL32V(_segMan, _gameObj, printLang, lang);
		}
	}

	return lang;
}

Common::String EngineState::strSplit(const char *str, const char *sep) {
	kLanguage lang = getLanguage();
	kLanguage subLang = K_LANG_NONE;

	if (_kernel->_selectorCache.subtitleLang != -1) {
		subLang = (kLanguage)GET_SEL32V(_segMan, _gameObj, subtitleLang);
	}

	Common::String retval = getLanguageString(str, lang);

	if ((subLang != K_LANG_NONE) && (sep != NULL)) {
		retval += sep;
		retval += getLanguageString(str, subLang);
	}

	return retval;
}

int EngineState::methodChecksum(reg_t objAddress, Selector sel, int offset, uint size) const {
	reg_t fptr;

	Object *obj = _segMan->getObject(objAddress);
	SelectorType selType = lookup_selector(_segMan, objAddress, sel, NULL, &fptr);

	if (!obj || (selType != kSelectorMethod))
		return -1;

	Script *script = _segMan->getScript(fptr.segment);

	if (!script->_buf || (fptr.offset + offset < 0))
		return -1;

	fptr.offset += offset;

	if (fptr.offset + size > script->_bufSize)
		return -1;

	byte *buf = script->_buf + fptr.offset;

	uint sum = 0;
	for (uint i = 0; i < size; i++)
		sum += buf[i];

	return sum;
}

uint16 EngineState::firstRetOffset(reg_t objectAddress) const {
	Script *script = _segMan->getScript(objectAddress.segment);

	if ((script == NULL) || (script->_buf == NULL))
		return 0;

	uint16 offset = objectAddress.offset;

	while (offset < script->_bufSize) {
		byte opcode = script->_buf[offset++];
		byte opnumber = opcode >> 1;

		if (opnumber == 0x24)	// ret
			return offset - 1;

		// Skip operands for non-ret opcodes
		for (int i = 0; g_opcode_formats[opnumber][i]; i++) {
			switch (g_opcode_formats[opnumber][i]) {
			case Script_Byte:
			case Script_SByte:
				offset++;
				break;
			case Script_Word:
			case Script_SWord:
				offset += 2;
				break;
			case Script_Variable:
			case Script_Property:
			case Script_Local:
			case Script_Temp:
			case Script_Global:
			case Script_Param:
			case Script_SVariable:
			case Script_SRelative:
			case Script_Offset:
				offset++;
				if (!(opcode & 1))
					offset++;
				break;
			case Script_End:
				return offset;
				break;
			case Script_Invalid:
			default:
				warning("opcode %02x: Invalid", opcode);
			}
		}	// end for
	}	// end while

	return 0;
}

SciVersion EngineState::detectDoSoundType() {
	if (_doSoundType == SCI_VERSION_AUTODETECT) {
		reg_t soundClass = _segMan->findObjectByName("Sound");

		if (!soundClass.isNull()) {
			int sum = methodChecksum(soundClass, _kernel->_selectorCache.play, -6, 6);

			switch (sum) {
			case 0x1B2: // SCI0
			case 0x1AE: // SCI01
				_doSoundType = SCI_VERSION_0_EARLY;
				break;
			case 0x13D:
				_doSoundType = SCI_VERSION_1_EARLY;
				break;
			case 0x13E:
#ifdef ENABLE_SCI32
			case 0x14B:
#endif
				_doSoundType = SCI_VERSION_1_LATE;
			}
		}

		if (_doSoundType == SCI_VERSION_AUTODETECT) {
			warning("DoSound detection failed, taking an educated guess");

			if (getSciVersion() >= SCI_VERSION_1_MIDDLE)
				_doSoundType = SCI_VERSION_1_LATE;
			else if (getSciVersion() > SCI_VERSION_01)
				_doSoundType = SCI_VERSION_1_EARLY;
			else
				_doSoundType = SCI_VERSION_0_EARLY;
		}

		debugC(1, kDebugLevelSound, "Detected DoSound type: %s", getSciVersionDesc(_doSoundType).c_str());
	}

	return _doSoundType;
}

SciVersion EngineState::detectSetCursorType() {
	if (_setCursorType == SCI_VERSION_AUTODETECT) {
		int sum = methodChecksum(_gameObj, _kernel->_selectorCache.setCursor, 0, 21);

		if ((sum == 0x4D5) || (sum == 0x552)) {
			// Standard setCursor
			_setCursorType = SCI_VERSION_0_EARLY;
		} else if (sum != -1) {
			// Assume that others use fancy cursors
			_setCursorType = SCI_VERSION_1_1;
		} else {
			warning("SetCursor detection failed, taking an educated guess");

			if (getSciVersion() >= SCI_VERSION_1_1)
				_setCursorType = SCI_VERSION_1_1;
			else
				_setCursorType = SCI_VERSION_0_EARLY;
		}

		debugC(1, kDebugLevelGraphics, "Detected SetCursor type: %s", getSciVersionDesc(_setCursorType).c_str());
	}

	return _setCursorType;
}

SciVersion EngineState::detectLofsType() {
	if (_lofsType == SCI_VERSION_AUTODETECT) {
		// This detection only works (and is only needed) pre-SCI1.1
		if (getSciVersion() >= SCI_VERSION_1_1) {
			_lofsType = SCI_VERSION_1_1;
			return _lofsType;
		}

		Object *obj = NULL;
		reg_t gameClass = _segMan->findObjectByName("Game");

		if (!gameClass.isNull())
			obj = _segMan->getObject(gameClass);

		bool couldBeAbs = true;
		bool couldBeRel = true;

		// Check methods of the Game class for lofs operations
		if (obj) {
			for (uint m = 0; m < obj->getMethodCount(); m++) {
				reg_t fptr = obj->getFunction(m);

				Script *script = _segMan->getScript(fptr.segment);

				if ((script == NULL) || (script->_buf == NULL))
					continue;

				uint offset = fptr.offset;
				bool done = false;

				while (!done) {
					// Read opcode
					if (offset >= script->_bufSize)
						break;

					byte opcode = script->_buf[offset++];
					byte opnumber = opcode >> 1;

					if ((opnumber == 0x39) || (opnumber == 0x3a)) {
						uint16 lofs;

						// Load lofs operand
						if (opcode & 1) {
							if (offset >= script->_bufSize)
								break;
							lofs = script->_buf[offset++];
						} else {
							if (offset + 1 >= script->_bufSize)
								break;
							lofs = READ_LE_UINT16(script->_buf + offset);
							offset += 2;
						}

						// Check for going out of bounds when interpreting as abs/rel
						if (lofs >= script->_bufSize)
							couldBeAbs = false;

						if ((signed)offset + (int16)lofs < 0)
							couldBeRel = false;

						if ((signed)offset + (int16)lofs >= (signed)script->_bufSize)
							couldBeRel = false;

						continue;
					}

					// Skip operands for non-lofs opcodes
					for (int i = 0; g_opcode_formats[opnumber][i]; i++) {
						switch (g_opcode_formats[opnumber][i]) {
						case Script_Byte:
						case Script_SByte:
							offset++;
							break;
						case Script_Word:
						case Script_SWord:
							offset += 2;
							break;
						case Script_Variable:
						case Script_Property:
						case Script_Local:
						case Script_Temp:
						case Script_Global:
						case Script_Param:
						case Script_SVariable:
						case Script_SRelative:
						case Script_Offset:
							offset++;
							if (!(opcode & 1))
								offset++;
							break;
						case Script_End:
							done = true;
							break;
						case Script_Invalid:
						default:
							warning("opcode %02x: Invalid", opcode);
						}
					}
				}
			}
		}

		if (couldBeRel == couldBeAbs) {
			warning("Lofs detection failed, taking an educated guess");

			if (getSciVersion() >= SCI_VERSION_1_MIDDLE)
				_lofsType = SCI_VERSION_1_MIDDLE;
			else
				_lofsType = SCI_VERSION_0_EARLY;
		} else if (couldBeAbs) {
			_lofsType = SCI_VERSION_1_MIDDLE;
		} else {
			_lofsType = SCI_VERSION_0_EARLY;
		}

		debugC(1, kDebugLevelVM, "Detected Lofs type: %s", getSciVersionDesc(_lofsType).c_str());
	}

	return _lofsType;
}

SciVersion EngineState::detectGfxFunctionsType() {
	if (_gfxFunctionsType == SCI_VERSION_AUTODETECT) {
		// This detection only works (and is only needed) for SCI0 games
		if (getSciVersion() >= SCI_VERSION_01) {
			_gfxFunctionsType = SCI_VERSION_0_LATE;
			return _gfxFunctionsType;
		}

		if (getSciVersion() > SCI_VERSION_0_EARLY) {
			if (_kernel->findSelector("shiftParser") != -1) {
				// The shiftParser selector was introduced just a bit after the
				// changes to the graphics functions, so if it exists, the game is
				// definitely using newer graphics functions
				_gfxFunctionsType = SCI_VERSION_0_LATE;
			} else {
				// No shiftparser selector, check if the game is using an overlay
				if (_kernel->_selectorCache.overlay == -1) {
					// No overlay selector found, therefore the game is definitely
					// using old graphics functions
					_gfxFunctionsType = SCI_VERSION_0_EARLY;
				} else {
					// An in-between case: The game does not have a shiftParser
					// selector, but it does have an overlay selector, so it uses an
					// overlay. Therefore, check it to see how it calls kDrawPic to
					// determine the graphics functions type used

					reg_t roomObjAddr = _segMan->findObjectByName("Rm");

					bool found = false;

					if (!roomObjAddr.isNull()) {
						reg_t addr;

						if (lookup_selector(_segMan, roomObjAddr, _kernel->_selectorCache.overlay, NULL, &addr) == kSelectorMethod) {
							uint16 offset = addr.offset;
							byte *scr = _segMan->getScript(addr.segment)->_buf;
							do {
								uint16 kFuncNum;
								int opsize = scr[offset++];
								uint opcode = opsize >> 1;
								int i = 0;
								byte argc;

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
											kFuncNum = scr[offset++];
										else {
											kFuncNum = 0xffff & (scr[offset] | (scr[offset + 1] << 8));
											offset += 2;
										}

										if (opcode == op_callk) {
											argc = scr[offset++];

											if (kFuncNum == 8) {	// kDrawPic
												// If kDrawPic is called with 6 parameters from the
												// overlay selector, the game is using old graphics functions.
												// Otherwise, if it's called with 8 parameters, it's using new
												// graphics functions
												if (argc == 6) {
													_gfxFunctionsType = SCI_VERSION_0_EARLY;
													found = true;
												} else if (argc == 8) {
													_gfxFunctionsType = SCI_VERSION_0_LATE;
													found = true;
												} else {
													warning("overlay selector calling kDrawPic with %d parameters", argc);
												}
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
							} while (offset > 0 && !found);
						}
					}

					if (!found) {
						warning("Graphics functions detection failed, taking an educated guess");

						// Try detecting the graphics function types from the existence of the motionCue
						// selector (which is a bit of a hack)
						if (_kernel->findSelector("motionCue") != -1)
							_gfxFunctionsType = SCI_VERSION_0_LATE;
						else
							_gfxFunctionsType = SCI_VERSION_0_EARLY;
					}
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

MoveCountType EngineState::detectMoveCountType() {
	if (_moveCountType == kMoveCountUninitialized) {
		// SCI0/SCI01 games always increment move count
		if (getSciVersion() <= SCI_VERSION_01) {
			_moveCountType = kIncrementMoveCount;
			return _moveCountType;
		}

		reg_t motionClass = _segMan->findObjectByName("Motion");
		bool found = false;

		if (!motionClass.isNull()) {
			Object *obj = _segMan->getObject(motionClass);
			reg_t fptr;

			if (obj && lookup_selector(_segMan, motionClass, _kernel->_selectorCache.doit, NULL, &fptr) == kSelectorMethod) {
				byte *buf = _segMan->getScript(fptr.segment)->_buf + fptr.offset;
				int checksum = 0;
				for (int i = 0; i < 8; i++)
					checksum += *(buf++);
				_moveCountType = (checksum == 0x216) ? kIncrementMoveCount : kIgnoreMoveCount;
				found = true;
			}
		}

		if (!found) {
			warning("Move count autodetection failed");
			_moveCountType = kIncrementMoveCount;	// Most games do this, so best guess
		}

		debugC(1, kDebugLevelVM, "Detected move count handling: %s", (_moveCountType == kIncrementMoveCount) ? "increment" : "ignore");
	}

	return _moveCountType;
}

} // End of namespace Sci
