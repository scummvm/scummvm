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

#ifndef ULTIMA4_FILE_H
#define ULTIMA4_FILE_H

#include "common/file.h"
#include "common/hash-str.h"
#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima4 {

/**
 * Open a data file
 */
extern Common::File *u4fopen(const Common::String &fname);

/**
 * Closes a data file from the Ultima 4 for DOS installation.
 */
extern void u4fclose(Common::File *&f);

extern int u4fseek(Common::File *f, long offset, int whence);
extern long u4ftell(Common::File *f);
extern size_t u4fread(void *ptr, size_t size, size_t nmemb, Common::File *f);
extern int u4fgetc(Common::File *f);
extern int u4fgetshort(Common::File *f);
extern long u4flength(Common::File *f);

/**
 * Read a series of zero terminated strings from a file.  The strings
 * are read from the given offset, or the current file position if
 * offset is -1.
 */
extern Std::vector<Common::String> u4read_stringtable(const Common::String &filename);

extern Common::String u4find_music(const Common::String &fname);
extern Common::String u4find_sound(const Common::String &fname);
extern Common::String u4find_conf(const Common::String &fname);
extern Common::String u4find_graphics(const Common::String &fname);

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
