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

#ifndef HDB_FILE_MANAGER_H
#define HDB_FILE_MANAGER_H

#include "common/array.h"
#include "common/file.h"
#include "common/error.h"

namespace HDB {

// Each entry in a MSD file is of the following types

enum DataType {
	TYPE_ERROR,
	TYPE_BINARY,
	TYPE_TILE32,
	TYPE_FONT,
	TYPE_ICON32,
	TYPE_PIC,

	ENDOFTYPES
};

struct MSDEntry {
	char	filename[64];	// filename
	int32	offset;			// offset in MSD file of data
	int32	length;			// compressed length of data
	int32	ulength;		// uncompressed length
	DataType	type;		// type of data
};

// data structure for a TILE32
// the actual 16-bit "565" PocketPC-formatted
// gfx data follows the structure in the .msd
// The imagesize will always be 2048 bytes (32*32*2)
struct Tile32Type {
	int32	flags;			// bit flags
	char	name[64];		// name of graphic
};

// data structure for a PIC
// the actual 16-bit "565" PocketPC-formatted
// gfx data follows the structure in the .msd
struct PicType {
	int		width, height;	// width & height of pic
	char	name[64];		// name of pic
};

// data structure for a FONT
// the actual 16-bit "565" PocketPC-formatted
// gfx data follows the structure in the .msd
struct FontType {
	int		type;		// 0 = mono, 1 = proportional
	int		numChars;	// how many characters in font
	int		height;		// height of entire font
	int		kerning;	// space between chars
	int		leading;	// space between lines
};

// each character in a font has width info and
// an offset from the beginning of the font chunk
struct CharInfo {
	int16	width;		// in pixels, of the character
	int32	offset;		// from the start of the font charInfo chunk
};

class FileMan {
private:

	Common::File *_msdFile;
	Common::Array<MSDEntry *> _dir;
	int _numEntries;
	bool _compressed;

public:

	struct {
		uint32 id;
		uint32 dirSize;
	} dataHeader;

	long readOffset;
	
	bool openMSD(const Common::String &filename);
	void closeMSD();

};

} // End of Namespace HDB

#endif // !HDB_FILE_MANAGER_H
