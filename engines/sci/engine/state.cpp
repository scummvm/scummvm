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

#include "sci/engine/state.h"
#include "sci/engine/vm.h"
#include "sci/engine/script.h"
#include "sci/console.h" // For parse_reg_t

namespace Sci {

EngineState::EngineState(ResourceManager *res, uint32 flags)
: resMan(res), _flags(flags), _dirseeker(this) {
	widget_serial_counter = 0;

	game_version = 0;

	gfx_state = 0;
	old_screen = 0;

	sfx_init_flags = 0;
	sound_volume = 0;
	sound_mute = 0;

	restarting_flags = 0;

	pic_not_valid = 0;
	pic_is_new = 0;
	
	pic_priority_table = 0;

	status_bar_foreground = 0;
	status_bar_background = 0;

	game_time = 0;

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

	_menubar = 0;

	priority_first = 0;
	priority_last = 0;

	last_wait_time = 0;

	kernel_opt_flags = 0;

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
	string_frag_segment = 0;

	memset(parser_nodes, 0, sizeof(parser_nodes));

	parser_valid = 0;

	game_obj = NULL_REG;

	segMan = 0;
	gc_countdown = 0;

	successor = 0;

	speedThrottler = new SpeedThrottler(res->sciVersion());

	_setCursorType = SCI_VERSION_AUTODETECT;
	_doSoundType = SCI_VERSION_AUTODETECT;
	_lofsType = SCI_VERSION_AUTODETECT;
}

EngineState::~EngineState() {
	delete speedThrottler;
}

uint16 EngineState::currentRoomNumber() const {
	return script_000->locals_block->_locals[13].toUint16();
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

	if (((SciEngine*)g_engine)->getKernel()->_selectorMap.printLang != -1) {
		SegManager *segManager = this->segMan;

		lang = (kLanguage)GET_SEL32V(this->game_obj, printLang);

		if ((segManager->sciVersion() == SCI_VERSION_1_1) || (lang == K_LANG_NONE)) {
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
			PUT_SEL32V(this->game_obj, printLang, lang);
		}
	}

	return lang;
}

Common::String EngineState::strSplit(const char *str, const char *sep) {
	SegManager *segManager = this->segMan;

	kLanguage lang = getLanguage();
	kLanguage subLang = K_LANG_NONE;

	if (((SciEngine*)g_engine)->getKernel()->_selectorMap.subtitleLang != -1)
		subLang = (kLanguage)GET_SEL32V(this->game_obj, subtitleLang);

	Common::String retval = getLanguageString(str, lang);

	if ((subLang != K_LANG_NONE) && (sep != NULL)) {
		retval += sep;
		retval += getLanguageString(str, subLang);
	}

	return retval;
}

int EngineState::methodChecksum(reg_t objAddress, Selector sel, int offset, uint size) const {
	reg_t fptr;

	Object *obj = obj_get(segMan, objAddress);
	SelectorType selType = lookup_selector(this->segMan, objAddress, sel, NULL, &fptr);

	if (!obj || (selType != kSelectorMethod))
		return -1;

	Script *script = segMan->getScript(fptr.segment);

	if (!script->buf || (fptr.offset + offset < 0))
		return -1;

	fptr.offset += offset;

	if (fptr.offset + size > script->buf_size)
		return -1;

	byte *buf = script->buf + fptr.offset;

	uint sum = 0;
	for (uint i = 0; i < size; i++)
		sum += buf[i];

	return sum;
}

SciVersion EngineState::detectDoSoundType() {
	if (_doSoundType == SCI_VERSION_AUTODETECT) {
		reg_t soundClass;

		if (!parse_reg_t(this, "?Sound", &soundClass)) {
			int sum = methodChecksum(soundClass, ((SciEngine *)g_engine)->getKernel()->_selectorMap.play, -6, 6);

			switch(sum) {
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

			if (resMan->sciVersion() >= SCI_VERSION_1_MIDDLE)
				_doSoundType = SCI_VERSION_1_LATE;
			else if (resMan->sciVersion() > SCI_VERSION_01)
				_doSoundType = SCI_VERSION_1_EARLY;
			else
				_doSoundType = SCI_VERSION_0_EARLY;
		}

		debugC(1, kDebugLevelSound, "Detected DoSound type: %s", ((SciEngine *)g_engine)->getSciVersionDesc(_doSoundType).c_str());
	}

	return _doSoundType;
}

SciVersion EngineState::detectSetCursorType() {
	if (_setCursorType == SCI_VERSION_AUTODETECT) {
		int sum = methodChecksum(game_obj, ((SciEngine *)g_engine)->getKernel()->_selectorMap.setCursor, 0, 21);

		if ((sum == 0x4D5) || (sum == 0x552)) {
			// Standard setCursor
			_setCursorType = SCI_VERSION_0_EARLY;
		} else if (sum != -1) {
			// Assume that others use fancy cursors
			_setCursorType = SCI_VERSION_1_1;
		} else {
			warning("SetCursor detection failed, taking an educated guess");

			if (resMan->sciVersion() >= SCI_VERSION_1_1)
				_setCursorType = SCI_VERSION_1_1;
			else
				_setCursorType = SCI_VERSION_0_EARLY;
		}

		debugC(1, kDebugLevelGraphics, "Detected SetCursor type: %s", ((SciEngine *)g_engine)->getSciVersionDesc(_setCursorType).c_str());
	}

	return _setCursorType;
}

SciVersion EngineState::detectLofsType() {
	if (_lofsType == SCI_VERSION_AUTODETECT) {
		SciVersion version = segMan->sciVersion(); // FIXME: for VM_OBJECT_READ_FUNCTION

		// This detection only works (and is only needed) pre-SCI1.1
		if (version >= SCI_VERSION_1_1) {
			_lofsType = SCI_VERSION_1_1;
			return _lofsType;
		}

		reg_t gameClass;
		Object *obj = NULL;

		if (!parse_reg_t(this, "?Game", &gameClass))
			obj = obj_get(segMan, gameClass);

		bool couldBeAbs = true;
		bool couldBeRel = true;

		// Check methods of the Game class for lofs operations
		if (obj) {
			for (int m = 0; m < obj->methods_nr; m++) {
				reg_t fptr = VM_OBJECT_READ_FUNCTION(obj, m);

				Script *script = segMan->getScript(fptr.segment);

				if ((script == NULL) || (script->buf == NULL))
					continue;

				uint offset = fptr.offset;
				bool done = false;

				while (!done) {
					// Read opcode
					if (offset >= script->buf_size)
						break;

					byte opcode = script->buf[offset++];
					byte opnumber = opcode >> 1;

					if ((opnumber == 0x39) || (opnumber == 0x3a)) {
						uint16 lofs;

						// Load lofs operand
						if (opcode & 1) {
							if (offset >= script->buf_size)
								break;
							lofs = script->buf[offset++];
						} else {
							if (offset + 1 >= script->buf_size)
								break;
							lofs = READ_LE_UINT16(script->buf + offset);
							offset += 2;
						}

						// Check for going out of bounds when interpreting as abs/rel
						if (lofs >= script->buf_size)
							couldBeAbs = false;

						if ((signed)offset + (int16)lofs < 0)
							couldBeRel = false;

						if ((signed)offset + (int16)lofs >= (signed)script->buf_size)
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

			if (version >= SCI_VERSION_1_MIDDLE)
				_lofsType = SCI_VERSION_1_MIDDLE;
			else
				_lofsType = SCI_VERSION_0_EARLY;
		} else if (couldBeAbs) {
			_lofsType = SCI_VERSION_1_MIDDLE;
		} else {
			_lofsType = SCI_VERSION_0_EARLY;
		}

		debugC(1, kDebugLevelVM, "Detected Lofs type: %s", ((SciEngine *)g_engine)->getSciVersionDesc(_lofsType).c_str());
	}

	return _lofsType;
}

} // End of namespace Sci
