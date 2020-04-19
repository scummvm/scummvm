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
#include "common/debug.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/unzip.h"

namespace Ultima {
namespace Ultima4 {

using Std::map;
using Common::String;
using Std::vector;

bool u4isUpgradeAvailable() {
	Common::File *pal = u4fopen("u4vga.pal");
	bool avail = pal != nullptr;
	delete pal;

	return avail;
}

bool u4isUpgradeInstalled() {
	int filelength;
	bool result = false;

	Common::File *u4f = u4fopen("ega.drv");
	if (u4f) {
		filelength = u4f->size();
		u4fclose(u4f);

		// See if (ega.drv > 5k).  If so, the upgrade is installed
		if (filelength > (5 * 1024))
			result = true;
	}

	debug(1, "u4isUpgradeInstalled %d\n", (int) result);

	return result;
}

/*-------------------------------------------------------------------*/

/**
 * Open a data file
 */
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

/**
 * Closes a data file from the Ultima 4 for DOS installation.
 */
void u4fclose(Common::File *f) {
	f->close();
	delete f;
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

/**
 * Returns the length in bytes of a file.
 */
long u4flength(Common::File *f) {
	return f->size();
}

/**
 * Read a series of zero terminated strings from a file.  The strings
 * are read from the given offset, or the current file position if
 * offset is -1.
 */
vector<Common::String> u4read_stringtable(Common::File *f, long offset, int nstrings) {
	Common::String buffer;
	int i;
	vector<Common::String> strs;

	ASSERT(offset < u4flength(f), "offset begins beyond end of file");

	if (offset != -1)
		f->seek(offset, SEEK_SET);
	for (i = 0; i < nstrings; i++) {
		char c;
		buffer.clear();

		while ((c = f->readByte()) != '\0')
			buffer += c;

		strs.push_back(buffer);
	}

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
