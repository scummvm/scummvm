/* ScummVM - Kyrandia Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "resource.h"
#include "wsamovie.h"

#include "common/file.h"
#include "script.h"

namespace Kyra {
	Resourcemanager::Resourcemanager(KyraEngine* engine, const char* gamedir) {
		_gameDir = gamedir;
		_engine = engine;
		
		// prefetches all PAK Files
		
		// ugly a hardcoded list
		// TODO: use the FS Backend to get all .PAK Files and load them
		static const char* kyraFilelist[] = {
			"A_E.PAK", "DAT.PAK", "F_L.PAK", "MAP_5.PAK", "MSC.PAK", "M_S.PAK",
			"S_Z.PAK", "WSA1.PAK", "WSA2.PAK", "WSA3.PAK", "WSA4.PAK", "WSA5.PAK",
			"WSA6.PAK", "startup.pak", "intro1.pak", 0
		};
		
		for (uint32 tmp = 0; kyraFilelist[tmp]; ++tmp)	{
			// prefetch file
			PAKFile* file = new PAKFile(getPath() + kyraFilelist[tmp]);
			assert(file);			

			if (file->isOpen() && file->isValid())		
				_pakfiles.push_back(file);
			else
				warning("couldn't load file '%s' correctly", kyraFilelist[tmp]);
		}
	}
	
	Resourcemanager::~Resourcemanager() {
		Common::List<PAKFile*>::iterator start = _pakfiles.begin();

		for (;start != _pakfiles.end(); ++start) {
			delete *start;
			*start = 0;
		}
	}
	
	uint8* Resourcemanager::fileData(const char* file, uint32* size) {
		uint8* buffer = 0;
		
		debug("looking for file '%s'", file);
		
		File file_;
		
		// test to open it in the main dir
		if (file_.open((getPath() + file).c_str())) {
		
			*size = file_.size();
			
			buffer = new uint8[*size];
			assert(buffer);
			
			file_.read(buffer, *size);
			
			file_.close();
		
		} else {
			// opens the file in a PAK File
			Common::List<PAKFile*>::iterator start = _pakfiles.begin();
			
			for (;start != _pakfiles.end(); ++start) {
				*size = (*start)->getFileSize(file);
				
				if (!*size)
					continue;
					
				buffer = new uint8[*size];
				assert(buffer);
				
				// creates a copy of the file
				memcpy(buffer, (*start)->getFile(file), *size);
				
				break;
			}
			
		}
		
		if (!buffer || !(*size)) {
			warning("couldn't find file '%s'", file);
		}
		
		return buffer;
	}
	
	Palette* Resourcemanager::loadPalette(const char* file) {
		uint32 size = 0;
		uint8* buffer = 0;
		buffer = fileData(file, &size);
		if (!buffer)
			return 0;
		return new Palette(buffer, size);
	}
	
	CPSImage* Resourcemanager::loadImage(const char* file) {
		uint32 size = 0;
		uint8* buffer = 0;
		buffer = fileData(file, &size);
		if (!buffer)
			return 0;
		return new CPSImage(buffer, size);
	}
	
	Font* Resourcemanager::loadFont(const char* file) {
		uint32 size = 0;
		uint8* buffer = 0;
		buffer = fileData(file, &size);
		if (!buffer)
			return 0;
		return new Font(buffer, size);
	}
	
	Movie* Resourcemanager::loadMovie(const char* file) {
		// TODO: we have to check the Extenion to create the right movie
		uint32 size = 0;
		uint8* buffer = 0;
		buffer = fileData(file, &size);
		if (!buffer)
			return 0;
		return new WSAMovieV1(buffer, size);
	}

	VMContext* Resourcemanager::loadScript(const char* file) {
		VMContext* context = new VMContext(_engine);
		context->loadScript(file);
		return context;
	}
	
	Common::String Resourcemanager::getPath(void) {
		assert(_gameDir);
		int32 len = strlen(_gameDir);
		
		if(len < 1)
			error("no valid gamedir");
			
		// tests for an path seperator at the end
		if (_gameDir[len - 1] == '\\') {
			return string(_gameDir);
		} else if (_gameDir[len - 1 ] == '/') {
			return string(_gameDir);
		}
	
		// creates a path seperator at the end
		// we are always using the path seperator from the system
		// even if Windows shoudl accept '/'
#ifdef WIN32	
		return string(_gameDir) + "\\";
#else
		return string(_gameDir) + "/";
#endif
	}

///////////////////////////////////////////
// Pak file manager
	#define PAKFile_Iterate Common::List<PakChunk*>::iterator start=_files.begin();start != _files.end(); ++start
	PAKFile::PAKFile(const Common::String& file) {
		File pakfile;
		_buffer = 0;
		_open = false;

		if (!pakfile.open(file.c_str())) {
			warning("PAKFile couldn't open: '%s'", file.c_str());
			return;
		}

		uint32 filesize = pakfile.size();
		_buffer = new uint8[filesize];
		assert(_buffer);

		pakfile.read(_buffer, filesize);
		pakfile.close();

		// works with the file
		uint32 pos = 0, startoffset = 0, endoffset = 0;

		startoffset = READ_LE_UINT32(_buffer + pos);
		pos += 4;

		while (pos < filesize) {
			PakChunk* chunk = new PakChunk;
			assert(chunk);

			// saves the name
			chunk->_name = reinterpret_cast<const char*>(_buffer + pos);
			pos += strlen(chunk->_name) + 1;
			if(!chunk->_name)
				break;

			endoffset = READ_LE_UINT32(_buffer + pos);
			pos += 4;

			chunk->_data = _buffer + startoffset;
			chunk->_size = endoffset - startoffset;

			startoffset = endoffset;

			_files.push_back(chunk);
		}
		_open = true;
	}

	PAKFile::~PAKFile() {
		delete [] _buffer;
		_buffer = 0;
		_open = false;

		for (PAKFile_Iterate) {
			delete *start;
			*start = 0;
		}
	}

	const uint8* PAKFile::getFile(const char* file) {
		for (PAKFile_Iterate) { 
			if (!scumm_stricmp((*start)->_name, file))
				return (*start)->_data;
		}

		return 0;
	}

	uint32 PAKFile::getFileSize(const char* file) {
		for (PAKFile_Iterate) {
			if (!scumm_stricmp((*start)->_name, file))
				return (*start)->_size;
		}

		return 0;
	}
} // end of namespace Kyra

