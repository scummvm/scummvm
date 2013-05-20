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

#ifndef VOYEUR_FILES_H
#define VOYEUR_FILES_H

#include "common/scummsys.h"
#include "common/file.h"
#include "common/str.h"

namespace Voyeur {

class VoyeurEngine;
class BoltGroup;
class BoltEntry;

class BoltFile {
private:
	static BoltFile *_curLibPtr;
	static BoltGroup *_curGroupPtr;
	static byte *_curMemInfoPtr;
	static int _fromGroupFlag;
private:
	Common::File _curFd;
	int _curFilePosition;
	Common::Array<BoltGroup> _groups;
private:
	bool getBoltGroup(uint32 id);
	void resolveAll() {}
	byte *getBoltMember(uint32 id) { return NULL; }
public:
	BoltFile();
	~BoltFile();

	// Methods copied into bolt virtual table
	void initType() {}
	void termType() {}
	void initMem() {}
	void termMem() {}
	void initGro() {}
	void termGro() {}
};

class BoltGroup {
private:
	Common::SeekableReadStream *_file;
public:
	bool _loaded;
	bool _callInitGro;
	int _count;
	int _fileOffset;
	byte *_groupPtr;
	Common::Array<BoltEntry> _entries;
public:
	BoltGroup(Common::SeekableReadStream *f); 

	void load();
};


class BoltEntry {
private:
	Common::SeekableReadStream *_file;
public:
	int _fileOffset;
public:
	BoltEntry(Common::SeekableReadStream *f);

	void load();
};

class FilesManager {
private:
	int _decompressSize;
public:
	BoltFile *_curLibPtr;
public:
	FilesManager();

	bool openBOLTLib(const Common::String &filename, BoltFile *&boltFile);

};

} // End of namespace Voyeur

#endif /* VOYEUR_VOYEUR_H */
