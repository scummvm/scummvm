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

namespace Sci {

EngineState::EngineState(ResourceManager *res, sci_version_t version, uint32 flags)
: resmgr(res), _version(version), _flags(flags), _dirseeker(this) {
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

	animation_granularity = 0;

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

	seg_manager = 0;
	gc_countdown = 0;

	successor = 0;
}

EngineState::~EngineState() {
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
		EngineState *s = this;

		lang = (kLanguage)GET_SEL32V(s->game_obj, printLang);

		if ((_version == SCI_VERSION_1_1) || (lang == K_LANG_NONE)) {
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
			PUT_SEL32V(s->game_obj, printLang, lang);
		}
	}

	return lang;
}

Common::String EngineState::strSplit(const char *str, const char *sep) {
	EngineState *s = this;

	kLanguage lang = getLanguage();
	kLanguage subLang = K_LANG_NONE;

	if (((SciEngine*)g_engine)->getKernel()->_selectorMap.subtitleLang != -1)
		subLang = (kLanguage)GET_SEL32V(s->game_obj, subtitleLang);

	Common::String retval = getLanguageString(str, lang);

	if ((subLang != K_LANG_NONE) && (sep != NULL)) {
		retval += sep;
		retval += getLanguageString(str, subLang);
	}

	return retval;
}

} // End of namespace Sci
