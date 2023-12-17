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

#ifndef ULTIMA8_FILESYS_FLEXFILE_H
#define ULTIMA8_FILESYS_FLEXFILE_H

namespace Ultima {
namespace Ultima8 {

class FlexFile {
public:
	//! create FlexFile from datasource; FlexFile takes ownership of ds
	//! and deletes it when destructed
	explicit FlexFile(Common::SeekableReadStream *rs);
	~FlexFile();

	//! Check if constructed object is indeed a valid archive
	bool isValid() const {
		return _valid;
	}

	//! Check if numbered object exists
	//! \param index index of object to check for
	bool exists(uint32 index) {
		return getSize(index) > 0;
	}

	//! Get object as a Common::SeekableReadStream
	//! Delete the SeekableReadStream afterwards; that will delete the data as well
	Common::SeekableReadStream *getDataSource(uint32 index, bool is_text = false);

	//! Get object from file; returns NULL if index is invalid.
	//! Must delete the returned buffer afterwards.
	//! See also exists(uint32 index)
	//! \param index index of object to fetch
	//! \param size if non-NULL, size of object is stored in *size
	uint8 *getObject(uint32 index, uint32 *size = nullptr);

	//! Get size of object; returns zero if index is invalid.
	//! See also exists(uint32 index)
	//! \param index index of object to get size of
	uint32 getSize(uint32 index) const;

	//! Get upper bound for number of objects.
	//! In an indexed file this is (probably) the highest index plus one,
	//! while in a named file it's (probably) the actual count
	uint32 getCount() const {
		return _entries.size();
	}

	static bool isFlexFile(Common::SeekableReadStream *rs);

protected:
	Common::SeekableReadStream *_rs;
	bool _valid;

	struct FileEntry {
		uint32 _offset;
		uint32 _size;
		FileEntry() : _offset(0), _size(0) {}
	};

	Common::Array<FileEntry> _entries;

private:
	bool readMetadata();
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
