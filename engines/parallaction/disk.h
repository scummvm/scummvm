/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */

#ifndef PARALLACTION_DISK_H
#define PARALLACTION_DISK_H

#include "parallaction/defs.h"

namespace Parallaction {

//------------------------------------------------------
//		ARCHIVE MANAGEMENT
//------------------------------------------------------

struct ArchivedFile {
	uint16	_index;
	uint32	_offset;
	uint32	_cursor;
	uint16	field_A;	// unused
	uint16	field_C;	// unused
	uint32	_endOffset;
};

void openArchive(const char *file);
void closeArchive();

ArchivedFile *openArchivedFile(const char *name);
void closeArchivedFile(ArchivedFile*);

uint16 getArchivedFileLength(const char *name);

int16 readArchivedFile(ArchivedFile*, void *buffer, uint16 size);
char *readArchivedFileText(char *buf, uint16 size, void*);




} // namespace Parallaction



#endif
