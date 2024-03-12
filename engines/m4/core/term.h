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

#ifndef M4_CORE_TERM_H
#define M4_CORE_TERM_H

#include "common/stream.h"

namespace M4 {

enum TermMode {
	NO_MODE = 0,
	MESSAGE_MODE,
	MEMORY_MODE
};

class Term {
private:
	Common::WriteStream *_file = nullptr;
	bool _using_mono_screen = false;
	bool _use_log_file = false;
	TermMode _mode = NO_MODE;

public:
	/**
	 * Initialization
	 */
	void init(bool use_me, bool use_log);

	~Term() {
		delete _file;
	}

	/**
	 * Set the terminal mode
	 */
	void set_mode(TermMode mode);

	/**
	 * Show a message
	 */
	void message(const char *fmt, ...);
	void vmessage(const char *fmt, va_list va);
};

void term_message(const char *fmt, ...);

} // namespace M4

#endif
