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

#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"
#include "m4/fileio/fileio.h"

namespace M4 {

Common::Stream *f_io_open(const Common::Path &filename, const Common::String &mode) {
	if (mode.hasPrefix("r")) {
		if (filename.empty())
			return nullptr;

		Common::File *f = new Common::File();
		if (f->open(filename))
			return f;

		delete f;
		return nullptr;

	} else {
		return g_system->getSavefileManager()->openForSaving(filename.baseName());
	}
}

void f_io_close(Common::Stream *stream) {
	delete stream;
}

} // namespace M4
