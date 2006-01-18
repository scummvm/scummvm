/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2006 The ScummVM project
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
 * $Header$
 *
 */

#ifndef RESOURCE_H
#define RESOURCE_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "common/list.h"
#include "common/map.h"

#include "kyra/kyra.h"

namespace Kyra {

// standard Package format for Kyrandia games
class PAKFile {
	struct PakChunk {
		char* _name;
		uint32 _start;
		uint32 _size;
	};

public:

	PAKFile(const Common::String& file);
	~PAKFile();

	uint8* getFile(const char* file);
	uint32 getFileSize(const char* file);

	bool isValid(void) const { return (_filename != 0); }
	bool isOpen(void) const { return _open; }

private:

	bool _open;
	char *_filename;
	Common::List<PakChunk*> _files; // the entries
};

// some resource types
class Movie;
class VMContext;

class Resource {
public:

	Resource(KyraEngine* engine);
	~Resource();
	
	bool loadPakFile(const char *filename);
	void unloadPakFile(const char *filename);
	bool isInPakList(const char *filename);

	uint8* fileData(const char* file, uint32* size);

protected:
	struct PakFileEntry {
		PAKFile *_file;
		char _filename[32];
	};

	KyraEngine* _engine;
	Common::List<PakFileEntry> _pakfiles;
};

} // end of namespace Kyra

#endif
