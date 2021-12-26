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

#include "ags/lib/std/utility.h"
#include "ags/shared/script/script_common.h"  // current_line
#include "ags/shared/util/string.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

// Returns full script error message and callstack (if possible)
extern std::pair<String, String> cc_error_at_line(const char *error_msg);
// Returns script error message without location or callstack
extern String cc_error_without_line(const char *error_msg);

void cc_error(const char *descr, ...) {
	_G(ccErrorIsUserError) = false;
	if (descr[0] == '!') {
		_G(ccErrorIsUserError) = true;
		descr++;
	}

	va_list ap;
	va_start(ap, descr);
	String displbuf = String::FromFormatV(descr, ap);
	va_end(ap);

	if (_G(currentline) > 0) {
		// [IKM] Implementation is project-specific
		std::pair<String, String> errinfo = cc_error_at_line(displbuf.GetCStr());
		_G(ccErrorString) = errinfo.first;
		_G(ccErrorCallStack) = errinfo.second;
	} else {
		_G(ccErrorString) = cc_error_without_line(displbuf.GetCStr());
		_G(ccErrorCallStack) = "";
	}

	_G(ccError) = 1;
	_G(ccErrorLine) = _G(currentline);
}

} // namespace AGS3
