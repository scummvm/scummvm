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

#include "backends/platform/atari/atari-debug.h"

#ifdef DISABLE_TEXT_CONSOLE

void atari_debug(const char *s, ...) {
	va_list va;

	va_start(va, s);

	Common::String buf = Common::String::vformat(s, va);
	buf += '\n';

	if (g_system)
		g_system->logMessage(LogMessageType::kDebug, buf.c_str());

	va_end(va);
}

#endif
