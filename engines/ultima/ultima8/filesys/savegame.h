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
#include "common/serializer.h"
#include "engines/metaengine.h"
#include "graphics/surface.h"

namespace Ultima {
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
	ExtendedSavegameHeader _header;
	Common::HashMap<Common::String, FileEntry> _index;
	IDataSource *_file;
	uint32 _version;
public:
	explicit SavegameReader(IDataSource *ds, bool metadataOnly = false);
	~SavegameReader();

	enum State { SAVE_CORRUPT, SAVE_VALID, SAVE_OUT_OF_DATE, SAVE_TOO_RECENT };
	State isValid() const;

	uint32 getVersion() const { return _version; }

	std::string getDescription() const { return _header.description; }

	/**
	 * Get an entry/section within the save
	 */
	IDataSource *getDataSource(const std::string &name);
};

class SavegameWriter {
	class FileEntry : public Common::Array<byte> {
	public:
		std::string _name;
		FileEntry() : Common::Array<byte>() {}
	};
private:
	ODataSource *_file;
	Common::Array<FileEntry> _index;
	std::string _description;
public:
	explicit SavegameWriter(ODataSource *ds);
	virtual ~SavegameWriter();

	//! write the savegame's description.
	bool writeDescription(const std::string &desc) {
		_description = desc;
		return true;
	}

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
} // End of namespace Ultima

#endif
