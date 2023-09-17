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

#ifndef ULTIMA8_FILESYS_SAVEGAME_H
#define ULTIMA8_FILESYS_SAVEGAME_H

#include "ultima/shared/std/string.h"
#include "common/hashmap.h"
#include "common/stream.h"
#include "common/memstream.h"
#include "engines/metaengine.h"

namespace Ultima {
namespace Ultima8 {

class ZipFile;
class IDataSource;

class SavegameReader {
private:
	ExtendedSavegameHeader _header;
	Common::Archive *_archive;
	uint32 _version;
public:
	explicit SavegameReader(Common::SeekableReadStream *rs, bool metadataOnly = false);
	~SavegameReader();

	enum State { SAVE_CORRUPT, SAVE_VALID, SAVE_OUT_OF_DATE, SAVE_TOO_RECENT };
	State isValid() const;

	uint32 getVersion() const { return _version; }

	Std::string getDescription() const { return _header.description; }

	/**
	 * Get an entry/section within the save
	 */
	Common::SeekableReadStream *getDataSource(const Common::Path &name);
};

class SavegameWriter {
	class FileEntry : public Common::Array<byte> {
	public:
		Std::string _name;
		FileEntry() : Common::Array<byte>() {}
	};
private:
	Common::WriteStream *_file;
	Common::Array<FileEntry> _index;
public:
	explicit SavegameWriter(Common::WriteStream *ws);
	virtual ~SavegameWriter();

	//! write a file to the savegame
	//! \param name name of the file
	//! \param data the data
	//! \param size (in bytes) of data
	bool writeFile(const Std::string &name, const uint8 *data, uint32 size);

	//! write a file to the savegame from a memory stream
	//! \param name name of the file
	//! \param buf the MemoryWriteStreamDynamic to save
	bool writeFile(const Std::string &name, Common::MemoryWriteStreamDynamic *buf);

	//! finish savegame
	bool finish();
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
