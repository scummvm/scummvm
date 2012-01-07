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

#ifndef MYST3_DIRECTORYENTRY_H
#define MYST3_DIRECTORYENTRY_H

#include "engines/myst3/directorysubentry.h"
#include "common/stream.h"
#include "common/array.h"

namespace Myst3 {

class DirectoryEntry {
	private:
		char _roomName[5];
		uint16 _index;
		uint8 _unk;
		Common::Array<DirectorySubEntry> _subentries;

		Archive *_archive;

	public:
		DirectoryEntry() {}
		DirectoryEntry(Archive *archive);

		void readFromStream(Common::SeekableReadStream &inStream, const char *room);
		void dump();
		void dumpToFiles(Common::SeekableReadStream &inStream);
		DirectorySubEntry *getItemDescription(uint16 face, DirectorySubEntry::ResourceType type);
		uint16 getIndex() { return _index; }
		const char *getRoom() { return _roomName; }
};

} // end of namespace Myst3

#endif // MYST3_DIRECTORYENTRY_H
