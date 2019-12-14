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

#ifndef ULTIMA8_FILESYS_SAVEGAME_H
#define ULTIMA8_FILESYS_SAVEGAME_H

#include "ultima/ultima8/std/string.h"
#include "common/hashmap.h"
#include "common/memstream.h"
#include "common/hash-str.h"

namespace Ultima8 {

class ZipFile;
class IDataSource;
class ODataSource;
class OAutoBufferDataSource;

class SavegameReader {
	struct FileEntry {
		uint _offset;
		uint _size;
		FileEntry() : _offset(0), _size(0) {}
	};
private:
	IDataSource *_file;
	Common::HashMap<Common::String, FileEntry> _index;
	std::string _comments;
public:
	explicit SavegameReader(IDataSource *ds);
	~SavegameReader();

	//! get the savegame's global version
	uint32 getVersion();

	//! get the savegame's description
	std::string getDescription() const;

	IDataSource *getDataSource(const std::string &name);
};

class SavegameWriter {
	class FileEntry : public Common::MemoryWriteStreamDynamic {
	public:
		std::string _name;
		FileEntry() : Common::MemoryWriteStreamDynamic(DisposeAfterUse::YES) {}
	};
private:
	ODataSource *_file;
	Common::Array<FileEntry> _index;
	std::string _comments;
public:
	explicit SavegameWriter(ODataSource *ds);
	virtual ~SavegameWriter();

	//! write the savegame's description.
	bool writeDescription(const std::string &desc);

	//! write the savegame's global version
	bool writeVersion(uint32 version);

	//! write a file to the savegame
	//! \param name name of the file
	//! \param data the data
	//! \param size (in bytes) of data
	bool writeFile(const std::string &name, const uint8 *data, uint32 size);

	//! write a file to the savegame from an OAutoBufferDataSource
	//! \param name name of the file
	//! \param buf the OBufferDataSource to save
	bool writeFile(const std::string &name, OAutoBufferDataSource *buf);

	//! finish savegame
	bool finish();
};

} // End of namespace Ultima8

#endif
