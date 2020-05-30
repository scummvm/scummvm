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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/comprehend/util.h"
#include "common/debug.h"
#include "common/str.h"
#include "common/textconsole.h"

namespace Glk {
namespace Comprehend {

static unsigned debug_flags;

void __fatal_error(const char *func, unsigned line, const char *fmt, ...) {
	error("TODO");
}

void fatal_strerror(int err, const char *fmt, ...) {
	error("TODO");
}

void *xmalloc(size_t size) {
	void *p;

	p = malloc(size);
	if (!p)
		fatal_error("Out of memory");

	memset(p, 0, size);
	return p;
}

char *xstrndup(const char *str, size_t len) {
	char *p;

	Common::String s(str, len);
	p = scumm_strdup(s.c_str());
	if (!p)
		fatal_error("Out of memory");
	return p;
}

void debug_printf(unsigned flags, const char *fmt, ...) {
	va_list args;

	if (debug_flags & flags) {
		va_start(args, fmt);
		Common::String msg = Common::String::vformat(fmt, args);
		va_end(args);

		debug(1, "%s", msg.c_str());
	}
}

void debug_enable(unsigned flags) {
	debug_flags |= flags;
}

void debug_disable(unsigned flags) {
	debug_flags &= ~flags;
}

bool debugging_enabled(void) {
	// FIXME
	return debug_flags;
}

} // namespace Comprehend
} // namespace Glk
