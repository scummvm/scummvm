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
 */

#ifndef ACCESS_FILES_H
#define ACCESS_FILES_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/file.h"
#include "graphics/surface.h"
#include "access/decompress.h"

namespace Access {

class AccessEngine;

struct FileIdent {
	int _fileNum;
	int _subfile;

	FileIdent();

	void load(Common::SeekableReadStream &s);
};

struct CellIdent : FileIdent {
	byte _cell;

	CellIdent() {}
	CellIdent(int cell, int fileNum, int subfile);
};

class FileManager {
private:
	AccessEngine *_vm;
	const char * const *_filenames;

	void openFile(const Common::String &filename);

	byte *handleFile();
public:
	int _fileNumber;
	Common::File _file;
	Common::SeekableReadStream *_stream;
	Common::Array<uint32> _fileIndex;
	uint32 _filesize;
public:
	FileManager(AccessEngine *vm);
	~FileManager();

	/**
	 * Check the existence of a given file
	 */
	bool FileManager::existFile(const Common::String &filename);

	/**
	 * Load a given subfile from a container file
	 */
	byte *loadFile(int fileNum, int subfile);

	/**
	 * Loads a resource specified by a file identifier
	 */
	byte *loadFile(FileIdent &fileIdent);

	/**
	 * Load a given file by name
	 */
	byte *loadFile(const Common::String &filename);

	/**
	 * Load a given scren from a container file
	 */
	void loadScreen(int fileNum, int subfile);

	/**
	* Load a given screen by name
	*/
	void loadScreen(const Common::String &filename);

	/**
	 * Load a screen resource onto a designated surface
	 */
	void loadScreen(Graphics::Surface *dest, int fileNum, int subfile);

	/**
	 * Open up a sub-file container file
	 */
	void setAppended(int fileNum);

	/**
	 * Open up a sub-file resource within an alrady opened container file.
	 */
	void gotoAppended(int subfile);
};

} // End of namespace Access

#endif /* ACCESS_FILES_H */
