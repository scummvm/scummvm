/* Residual - Virtual machine to run LucasArts' 3D adventure games
 * Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
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

#ifndef SAVEGAME_H
#define SAVEGAME_H

#include "debug.h"
#include "lua.h"
#include <zlib.h>

class SaveGame {
public:
	SaveGame(char *filename, bool saving);
	~SaveGame();

//	int read(void *data, int size);
//	int checkTag(uint32 tag);
//	int write(void *data, int size);
//	int writeTag(uint32 tag);
//	gzFile fileHandle() { return _fileHandle; }

	uint32 beginSection(uint32 sectionTag);
	void endSection();
	void read(void *data, int size);
	void write(void *data, int size);

protected:
	bool _saving;
	gzFile _fileHandle;
	uint32 _currentSection;
	uint32 _sectionSize;
	uint32 _sectionPtr;
	char *_sectionBuffer;
};

#endif
