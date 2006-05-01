/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef __lure_disk_h__
#define __lure_disk_h__

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "lure/memory.h"
#include "lure/res_struct.h"

namespace Common {
	class File;
}

namespace Lure {

#define NUM_ENTRIES_IN_HEADER 0xBF
#define HEADER_IDENT_STRING "heywow"
#define HEADER_ENTRY_UNUSED_ID 0xffff

class Disk {
private:
	uint8 _fileNum;
	Common::File *_fileHandle;
	FileEntry _entries[NUM_ENTRIES_IN_HEADER];

	uint8 indexOf(uint16 id, bool suppressError = false);
public:
	Disk();
	~Disk();
	static Disk &getReference();

	void openFile(uint8 fileNum);
	uint32 getEntrySize(uint16 id);
	MemoryBlock *getEntry(uint16 id);
	bool exists(uint16 id);

	uint8 numEntries();
	FileEntry *getIndex(uint8 entryIndex);
};

} // end of namespace Lure

#endif
