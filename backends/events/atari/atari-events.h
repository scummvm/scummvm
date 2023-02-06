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

#ifndef BACKEND_EVENTS_ATARI_H
#define BACKEND_EVENTS_ATARI_H

#include "common/events.h"

#include "common/hashmap.h"

class AtariGraphicsManager;
class OSystem_Atari;

/**
 * The Atari event source.
 */
class AtariEventSource : public Common::EventSource {
public:
	AtariEventSource();

	bool pollEvent(Common::Event &event) override;

	void setGraphicsManager(AtariGraphicsManager *graphicsManager) { _graphicsManager = graphicsManager; }

private:
	OSystem_Atari *_system = nullptr;
	AtariGraphicsManager *_graphicsManager = nullptr;

	bool _oldLmbDown = false;
	bool _oldRmbDown = false;

	bool _lshiftActive = false;
	bool _rshiftActive = false;
	bool _ctrlActive = false;
	bool _altActive = false;
	bool _capslockActive = false;

	byte _unshiftToAscii[128];
	byte _shiftToAscii[128];
	byte _capsToAscii[128];

	const Common::KeyCode _asciiToKeycode[128 - 32 - 1] = {
		Common::KEYCODE_SPACE,
		Common::KEYCODE_EXCLAIM,
		Common::KEYCODE_QUOTEDBL,
		Common::KEYCODE_HASH,
		Common::KEYCODE_DOLLAR,
		Common::KEYCODE_PERCENT,
		Common::KEYCODE_AMPERSAND,
		Common::KEYCODE_QUOTE,
		Common::KEYCODE_LEFTPAREN,
		Common::KEYCODE_RIGHTPAREN,
		Common::KEYCODE_ASTERISK,
		Common::KEYCODE_PLUS,
		Common::KEYCODE_COMMA,
		Common::KEYCODE_MINUS,
		Common::KEYCODE_PERIOD,
		Common::KEYCODE_SLASH,
		Common::KEYCODE_0,
		Common::KEYCODE_1,
		Common::KEYCODE_2,
		Common::KEYCODE_3,
		Common::KEYCODE_4,
		Common::KEYCODE_5,
		Common::KEYCODE_6,
		Common::KEYCODE_7,
		Common::KEYCODE_8,
		Common::KEYCODE_9,
		Common::KEYCODE_COLON,
		Common::KEYCODE_SEMICOLON,
		Common::KEYCODE_LESS,
		Common::KEYCODE_EQUALS,
		Common::KEYCODE_GREATER,
		Common::KEYCODE_QUESTION,
		Common::KEYCODE_AT,
		Common::KEYCODE_a,
		Common::KEYCODE_b,
		Common::KEYCODE_c,
		Common::KEYCODE_d,
		Common::KEYCODE_e,
		Common::KEYCODE_f,
		Common::KEYCODE_g,
		Common::KEYCODE_h,
		Common::KEYCODE_i,
		Common::KEYCODE_j,
		Common::KEYCODE_k,
		Common::KEYCODE_l,
		Common::KEYCODE_m,
		Common::KEYCODE_n,
		Common::KEYCODE_o,
		Common::KEYCODE_p,
		Common::KEYCODE_q,
		Common::KEYCODE_r,
		Common::KEYCODE_s,
		Common::KEYCODE_t,
		Common::KEYCODE_u,
		Common::KEYCODE_v,
		Common::KEYCODE_w,
		Common::KEYCODE_x,
		Common::KEYCODE_y,
		Common::KEYCODE_z,
		Common::KEYCODE_LEFTBRACKET,
		Common::KEYCODE_BACKSLASH,
		Common::KEYCODE_RIGHTBRACKET,
		Common::KEYCODE_CARET,
		Common::KEYCODE_UNDERSCORE,
		Common::KEYCODE_BACKQUOTE,
		Common::KEYCODE_a,
		Common::KEYCODE_b,
		Common::KEYCODE_c,
		Common::KEYCODE_d,
		Common::KEYCODE_e,
		Common::KEYCODE_f,
		Common::KEYCODE_g,
		Common::KEYCODE_h,
		Common::KEYCODE_i,
		Common::KEYCODE_j,
		Common::KEYCODE_k,
		Common::KEYCODE_l,
		Common::KEYCODE_m,
		Common::KEYCODE_n,
		Common::KEYCODE_o,
		Common::KEYCODE_p,
		Common::KEYCODE_q,
		Common::KEYCODE_r,
		Common::KEYCODE_s,
		Common::KEYCODE_t,
		Common::KEYCODE_u,
		Common::KEYCODE_v,
		Common::KEYCODE_w,
		Common::KEYCODE_x,
		Common::KEYCODE_y,
		Common::KEYCODE_z,
		Common::KEYCODE_INVALID,	// {
		Common::KEYCODE_INVALID,	// |
		Common::KEYCODE_INVALID,	// }
		Common::KEYCODE_TILDE
	};
	Common::HashMap<byte, Common::KeyCode> _scancodeToKeycode;
};

#endif
