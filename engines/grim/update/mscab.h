/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef MSCAB_H
#define MSCAB_H

#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/stream.h"
#include "common/archive.h"
#include "common/str.h"
#include "common/util.h"

namespace Grim {

class MsCabinet : public Common::Archive {
public:
	MsCabinet(Common::SeekableReadStream *data);
	~MsCabinet();

	// Common::Archive API implementation
	bool hasFile(const Common::String &name) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::String &name) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const override;

private:
	Common::SeekableReadStream *_data;

	struct FolderEntry {
		uint16 comp_type;           //The compression type
		uint16 num_blocks;          //The total number of data blocks used by this folder
		uint32 offset;              //The cabinet offset of first datablock
	};

	struct FileEntry {
		uint32 length;              //Uncompressed size of the file in bytes
		FolderEntry *folder;        //Folder holding this file
		uint32 folderOffset;        //Uncompressed offset in the folder
	};

	typedef Common::HashMap<Common::String, FileEntry, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> FileMap;
	FileMap _fileMap;

	typedef Common::HashMap<uint16, FolderEntry> FolderMap;
	FolderMap _folderMap;

	Common::String readString(Common::ReadStream *stream);

	//Decompressor
	class Decompressor {
	public:
		Decompressor(const FolderEntry *folder, Common::SeekableReadStream *_data);
		~Decompressor();
		bool decompressFile(byte *&fileBuf, const FileEntry &entry);
		const FolderEntry *getFolder() const {
			return _curFolder;
		}
	private:
		Common::SeekableReadStream *_data;
		const FolderEntry *_curFolder;
		int16 _curBlock;
		byte *_compressedBlock, *_decompressedBlock;
		byte *_fileBuf;
		uint16 _startBlock, _inBlockStart, _endBlock, _inBlockEnd;

		void copyBlock(byte *&data_ptr) const;

		enum {
			kMszipCompression = 1,
			kCabBlockSize = 0x8000,
			kCabInputmax = kCabBlockSize + 12
		};
	};

	mutable Decompressor *_decompressor;

	//Cache
	typedef Common::HashMap<Common::String, byte *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> CacheMap;
	mutable CacheMap _cache;
};

} // End of namespace Grim

#endif
