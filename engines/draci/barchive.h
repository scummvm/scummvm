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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef BARCHIVE_H
#define BARCHIVE_H

#include "common/str.h"

namespace Draci {

/**
 *  Represents individual files inside the archive
 */

struct BAFile {
	uint16 _length;
	uint32 _offset; //!< Offset of file inside archive	
	byte *_data;
	byte _crc;

	void closeFile(void) { //!< Releases the file data (for memory considerations)
		delete _data;
		_data = NULL;
	}
};

class BArchive {
public:
	BArchive() : _files(NULL), _fileCount(0) {}
	BArchive(Common::String &path) : _files(NULL), _fileCount(0) { openArchive(path); }
	~BArchive() { closeArchive(); }

	void openArchive(const Common::String &path);
	void closeArchive(void);
	uint16 size() const { return _fileCount; }

	BAFile *operator[](unsigned int i) const;

private:
	// Archive header data
	static const char _magicNumber[];
	static const unsigned int _archiveHeaderSize = 10;
	
	// File stream header data
	static const unsigned int _fileHeaderSize = 6;

	Common::String _path;    //!< Path to file
	BAFile *_files;          //!< Internal array of files
	uint16 _fileCount;       //!< Number of files in archive
};

} // End of namespace Draci

#endif // BARCHIVE_H
