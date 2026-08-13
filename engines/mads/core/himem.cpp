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

#include "mads/core/general.h"
#include "mads/core/himem.h"

namespace MADS {

// EMS/XMS support doesn't exist in ScummVM, so the himem directory that
// preloaded resources into expanded/extended memory can never activate;
// these entry points are kept only because callers throughout the engine
// use them as preload hints.

void himem_shutdown() {
}

void himem_startup() {
}

int himem_preload(char *filename, int level) {
	return -1;
}

int himem_preload_series(const char *filename, int level) {
	return -1;
}

void himem_flush(int level) {
}

} // namespace MADS
