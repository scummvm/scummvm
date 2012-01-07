/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef MYST3_ARCHIVE_H
#define MYST3_ARCHIVE_H

#include "engines/myst3/directoryentry.h"
#include "common/stream.h"
#include "common/array.h"
#include "common/file.h"

namespace Myst3 {

class Archive {
	private:
		bool _multipleRoom;
		char _roomName[5];
		Common::File _file;
		Common::Array<DirectoryEntry> _directory;
		
		void _decryptHeader(Common::SeekableReadStream &inStream, Common::WriteStream &outStream);
		void _readDirectory();
	public:

		const DirectorySubEntry *getDescription(uint16 index, uint16 face, DirectorySubEntry::ResourceType type);
		Common::MemoryReadStream *dumpToMemory(uint32 offset, uint32 size);
		void dumpDirectory();
		void dumpToFiles();
		
		bool open(const char *fileName, const char *room);
		void close();
};

} // end of namespace Myst3

#endif
