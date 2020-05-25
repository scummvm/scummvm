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

#include "ultima/ultima4/filesys/u4file.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/shared/core/file.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/unzip.h"

namespace Ultima {
namespace Ultima4 {

Common::File *u4fopen(const Common::String &fname) {
	Common::File *u4f = nullptr;

	if (!fname.empty()) {
		u4f = new Common::File();
		if (u4f->open(fname)) {
			debug(1, "%s successfully opened\n", fname.c_str());
		} else {
			delete u4f;
			u4f = nullptr;
		}
	}

	return u4f;
}

void u4fclose(Common::File *&f) {
	f->close();
	delete f;
	f = nullptr;
}

int u4fseek(Common::File *f, long offset, int whence) {
	return f->seek(offset, whence);
}

long u4ftell(Common::File *f) {
	return f->pos();
}

size_t u4fread(void *ptr, size_t size, size_t nmemb, Common::File *f) {
	int count = f->read(ptr, size * nmemb);
	return count / size;
}

int u4fgetc(Common::File *f) {
	return f->readByte();
}

int u4fgetshort(Common::File *f) {
	return f->readUint16LE();
}

long u4flength(Common::File *f) {
	return f->size();
}

Std::vector<Common::String> u4read_stringtable(const Common::String &filename) {
	Shared::File f(Common::String::format("data/text/%s.dat", filename.c_str()));
	Std::vector<Common::String> strs;
	Common::String line;

	while (!f.eof())
		strs.push_back(f.readString());

	return strs;
}

Common::String u4find_path(const Common::String &fname, Common::List<Common::String> specificSubPaths) {
	return fname;
}

Common::String u4find_music(const Common::String &fname) {
	return "data/mid/" + fname;
}

Common::String u4find_sound(const Common::String &fname) {
	return "data/sound/" + fname;
}

Common::String u4find_conf(const Common::String &fname) {
	return "data/conf/" + fname;
}

Common::String u4find_graphics(const Common::String &fname) {
	return "data/graphics/" + fname;
}

} // End of namespace Ultima4
} // End of namespace Ultima
