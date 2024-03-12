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

#include "common/debug.h"
#include "common/savefile.h"
#include "common/system.h"
#include "m4/core/term.h"
#include "m4/vars.h"

namespace M4 {

void Term::init(bool use_me, bool use_log) {
	_using_mono_screen = use_me;
	_use_log_file = use_log;
	set_mode(MESSAGE_MODE);

	if (use_log) {
		_file = g_system->getSavefileManager()->openForSaving("term.log");
		if (!_file)
			error("Fail to create term.log file");
	}
}

void Term::set_mode(TermMode mode) {
	if (_using_mono_screen && mode != _mode) {
		_mode = mode;

		switch (mode) {
		case MESSAGE_MODE:
			message("********Message Mode");
			break;
		case MEMORY_MODE:
			message("********Memory Mode");
			break;
		default:
			break;
		}
	}
}

void Term::message(const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	vmessage(fmt, va);
	va_end(va);
}

void Term::vmessage(const char *fmt, va_list va) {
	if (_mode != MEMORY_MODE) {
		Common::String msg = Common::String::vformat(fmt, va);
		debug(1, "%s", msg.c_str());

		if (_file) {
			_file->writeString(msg);
			_file->writeByte('\n');
			_file->writeByte(0);
		}
	}
}

void term_message(const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	_G(term).vmessage(fmt, va);
	va_end(va);
}

} // namespace M4
