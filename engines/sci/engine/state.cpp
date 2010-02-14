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
#include "sci/engine/selector.h"
#include "sci/engine/vm.h"
#include "sci/engine/script.h"
#include "sci/engine/message.h"

namespace Sci {

EngineState::EngineState(Vocabulary *voc, SegManager *segMan)
: _voc(voc), _segMan(segMan), _dirseeker() {

#ifdef USE_OLD_MUSIC_FUNCTIONS
	sfx_init_flags = 0;
#endif

	restarting_flags = 0;

	last_wait_time = 0;

	_fileHandles.resize(5);

	execution_stack_base = 0;
	_executionStackPosChanged = false;

	r_acc = NULL_REG;
	restAdjust = 0;
	r_prev = NULL_REG;

	stack_base = 0;
	stack_top = 0;

	script_000 = 0;

	_activeBreakpointTypes = 0;
	sys_strings_segment = 0;
	sys_strings = 0;

	_gameObj = NULL_REG;

	gc_countdown = 0;

	successor = 0;

	_throttleCounter = 0;
	_throttleLastTime = 0;
	_throttleTrigger = false;

	_memorySegmentSize = 0;

	_soundCmd = 0;
}

EngineState::~EngineState() {
	delete _msgState;
}

uint16 EngineState::currentRoomNumber() const {
	return script_000->_localsBlock->_locals[13].toUint16();
}

void EngineState::setRoomNumber(uint16 roomNumber) {
	script_000->_localsBlock->_locals[13] = make_reg(0, roomNumber);
}

static kLanguage charToLanguage(const char c) {
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

Common::String SciEngine::getSciLanguageString(const char *str, kLanguage lang) const {
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

kLanguage SciEngine::getSciLanguage() {
	kLanguage lang = (kLanguage)_resMan->getAudioLanguage();
	if (lang != K_LANG_NONE)
		return lang;

	lang = K_LANG_ENGLISH;

	if (_kernel->_selectorCache.printLang != -1) {
		lang = (kLanguage)GET_SEL32V(_gamestate->_segMan, _gamestate->_gameObj, SELECTOR(printLang));

		if ((getSciVersion() >= SCI_VERSION_1_1) || (lang == K_LANG_NONE)) {
			// If language is set to none, we use the language from the game detector.
			// SSCI reads this from resource.cfg (early games do not have a language
			// setting in resource.cfg, but instead have the secondary language number
			// hardcoded in the game script).
			// SCI1.1 games always use the language setting from the config file
			// (essentially disabling runtime language switching).
			// Note: only a limited number of multilanguage games have been tested
			// so far, so this information may not be 100% accurate.
			switch (getLanguage()) {
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
			PUT_SEL32V(_gamestate->_segMan, _gamestate->_gameObj, SELECTOR(printLang), lang);
		}
	}

	return lang;
}

Common::String SciEngine::strSplit(const char *str, const char *sep) {
	kLanguage lang = getSciLanguage();
	kLanguage subLang = K_LANG_NONE;

	if (_kernel->_selectorCache.subtitleLang != -1) {
		subLang = (kLanguage)GET_SEL32V(_gamestate->_segMan, _gamestate->_gameObj, SELECTOR(subtitleLang));
	}

	Common::String retval = getSciLanguageString(str, lang);

	if ((subLang != K_LANG_NONE) && (sep != NULL)) {
		retval += sep;
		retval += getSciLanguageString(str, subLang);
	}

	return retval;
}

} // End of namespace Sci
