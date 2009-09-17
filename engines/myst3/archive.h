/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
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
		Common::File _file;
		Common::Array<DirectoryEntry> _directory;
		
		void _decryptHeader(Common::SeekableReadStream &inStream, Common::WriteStream &outStream);
		void _readDirectory();
	public:

		Common::MemoryReadStream *dumpToMemory(uint16 index, uint16 face, DirectorySubEntry::ResourceType type);
		void dumpDirectory();
		void dumpToFiles();
		
		bool open(const char *fileName);
		void close();
};

} // end of namespace Myst3

#endif
