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

#ifndef ACCESS_FILES_H
#define ACCESS_FILES_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/file.h"
#include "graphics/managed_surface.h"
#include "access/decompress.h"
#include "access/detection.h"

namespace Access {

class AccessEngine;

struct FileIdent {
	int _fileNum;
	int _subFile;

	FileIdent();
	FileIdent(int fileNum, int subfile) { _fileNum = fileNum; _subFile = subfile; }

	void load(Common::SeekableReadStream &s);
};

struct CellIdent : FileIdent {
	byte _cell;

	CellIdent();
	CellIdent(int cell, int fileNum, int subfile);
};

class FileManager;

class Resource {
	friend class FileManager;
private:
	Common::File _file;
	byte *_data;
public:
	Common::SeekableReadStream *_stream;
	int _size;

	Resource();
	Resource(byte *data, int size);
	~Resource();
	byte *data();

	const char *getFileName() const;
};

class FileManager {
private:
	AccessEngine *_vm;

	/**
	 * Open a raw file, no index data is read.
	 */
	void openFile(Resource *res, const Common::Path &filename);

	/**
	 * Handles setting up the resource with a stream for the located resource
	 */
	void handleFile(Resource *res);

	/**
	 * Handles loading a screen surface and palette with decoded resource
	 */
	void handleScreen(Graphics::ManagedSurface *dest, Resource *res);

	/**
	 * Open up a sub-file container file
	 */
	void setAppended(Resource *file, const Common::Path &fileName);

	/**
	 * Open up a sub-file resource within an alrady opened container file.
	 */
	void gotoAppended(Resource *file, int subfile);

	/**
	 * Read index data from the container file
	 */
	void readIndex(Resource *res);

public:
	Common::Path _indexedFilename;
	Common::Array<uint32> _fileIndex;
	bool _setPaletteFlag;
public:
	FileManager(AccessEngine *vm);
	~FileManager();

	/**
	 * Check the existence of a given file
	 */
	bool existFile(const Common::Path &filename);

	/**
	 * Load a given subfile from a container file
	 */
	Resource *loadFile(int fileNum, int subfile);

	/**
	 * Loads a resource specified by a file identifier
	 */
	Resource *loadFile(const FileIdent &fileIdent);

	/**
	 * Load a given *non-container* file by name directly.
	 */
	Resource *loadRawFile(const Common::Path &filename);

	/**
	 * Load a given scren from a container file
	 */
	void loadScreen(int fileNum, int subfile);

	/**
	* Load a given screen by name
	*/
	void loadScreen(const Common::Path &filename);

	/**
	 * Load a screen resource onto a designated surface
	 */
	void loadScreen(Graphics::ManagedSurface *dest, int fileNum, int subfile);
};

} // End of namespace Access

#endif /* ACCESS_FILES_H */
