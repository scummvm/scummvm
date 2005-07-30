/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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

#include "common/stdafx.h"
#include "kyra/resource.h"
#include "kyra/wsamovie.h"

#include "common/file.h"
#include "kyra/script.h"

namespace Kyra {
Resourcemanager::Resourcemanager(KyraEngine* engine) {
	_engine = engine;

	// prefetches all PAK Files

	// ugly a hardcoded list
	// TODO: use the FS Backend to get all .PAK Files and load them
	// or any other thing to get all files
	static const char* kyra1Filelist[] = {
		"A_E.PAK", "DAT.PAK", "F_L.PAK", "MAP_5.PAK", "MSC.PAK", "M_S.PAK",
		"S_Z.PAK", "WSA1.PAK", "WSA2.PAK", "WSA3.PAK", "WSA4.PAK", "WSA5.PAK",
		"WSA6.PAK", 0
	};

	static const char* kyra1CDFilelist[] = {
		"ADL.PAK", "BRINS.PAK", "CLIFF.PAK", "ENTER.PAK", "FORESTA.PAK", "GEM.PAK", "INTRO1.PAK",
		"LEPHOLE.PAK", "OAKS.PAK", "SPELL.PAK", "WILLOW.PAK", "ALCHEMY.PAK", "BROKEN.PAK", "COL.PAK",
		"EXTHEAL.PAK", "FORESTB.PAK", "GEMCUT.PAK", "INTRO2.PAK", "LIBRARY.PAK", "PLATEAU.PAK", "SPRING.PAK",
		"WISE.PAK", "ALGAE.PAK", "BURN.PAK", "DARMS.PAK", "EXTPOT.PAK", "FORESTC.PAK", "GENCAVB.PAK",
		"INTRO3.PAK", "MISC.PAK", "PLTCAVE.PAK", "SQUARE.PAK", "XEDGE.PAK", "ALTAR.PAK", "CASTLE.PAK",
		"DEAD.PAK", "EXTSPEL.PAK", "FOUNTN.PAK", "GENHALL.PAK", "INTRO4.PAK", "MIX.PAK", "POTION.PAK",
		"STARTUP.PAK", "XEDGEB.PAK", "ARCH.PAK", "CATACOM.PAK", "DNSTAIR.PAK", "FALLS.PAK", "FOYER.PAK",
		"GEN_CAV.PAK", "KITCHEN.PAK", "MOONCAV.PAK", "RUBY.PAK", "STUMP.PAK", "XEDGEC.PAK", "BALCONY.PAK",
		"CAVE.PAK", "DRAGON.PAK", "FESTSTH.PAK", "FSOUTH.PAK", "GLADE.PAK", "KYRAGEM.PAK", "NCLIFF.PAK",
		"SICKWIL.PAK", "TEMPLE.PAK", "XMI.PAK", "BELROOM.PAK",  "CAVEB.PAK", "EDGE.PAK", "FGOWEST.PAK",
		"FSOUTHB.PAK", "GRAVE.PAK", "LAGOON.PAK", "NCLIFFB.PAK", "SND.PAK", "TRUNK.PAK", "ZROCK.PAK",
		"BONKBG.PAK", "CGATE.PAK", "EDGEB.PAK", "FINALE.PAK", "FWSTSTH.PAK", "GRTHALL.PAK", "LANDING.PAK",
		"NWCLIFB.PAK", "SONG.PAK", "UPSTAIR.PAK", "BRIDGE.PAK", "CHASM.PAK", "EMCAV.PAK", "FNORTH.PAK",
		"GATECV.PAK", "HEALER.PAK", "LAVA.PAK", "NWCLIFF.PAK", "SORROW.PAK", "WELL.PAK", 0
	};

	const char** usedFilelist = 0;

	if (_engine->game() == KYRA1)
		usedFilelist = kyra1Filelist;
	else if (_engine->game() == KYRA1CD)
		usedFilelist = kyra1CDFilelist;
	else
		error("no filelist found for this game");

	for (uint32 tmp = 0; usedFilelist[tmp]; ++tmp) {
		// prefetch file
		PAKFile* file = new PAKFile(usedFilelist[tmp]);
		assert(file);

		if (file->isOpen() && file->isValid())
			_pakfiles.push_back(file);
		else
			debug("couldn't load file '%s' correctly", usedFilelist[tmp]);
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
	Common::File file_;

	// test to open it in the main dir
	if (file_.open(file)) {

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

			if (!(*size))
				continue;

			buffer = new uint8[*size];
			assert(buffer);

			// creates a copy of the file
			memcpy(buffer, (*start)->getFile(file), *size);

			break;
		}

	}

	if (!buffer || !(*size)) {
		return 0;
	}

	return buffer;
}

Palette* Resourcemanager::loadPalette(const char* file)	{
	uint32 size = 0;
	uint8* buffer = 0;
	buffer = fileData(file, &size);
	if (!buffer) {
		warning("ResMgr: Failed loading palette %s", file);
		return 0;
	}
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
	if (!buffer || !size)
		return 0;
	if (_engine->game() == KYRA1 || _engine->game() == KYRA1CD)
		return new WSAMovieV1(buffer, size, _engine->game());
	else
		return new WSAMovieV2(buffer, size);
}

VMContext* Resourcemanager::loadScript(const char* file) {
	VMContext* context = new VMContext(_engine);
	context->loadScript(file);
	return context;
}

///////////////////////////////////////////
// Pak file manager
#define PAKFile_Iterate Common::List<PakChunk*>::iterator start=_files.begin();start != _files.end(); ++start
PAKFile::PAKFile(/*const Common::String &path, */const Common::String& file) {
	Common::File pakfile;
	_buffer = 0;
	_open = false;

	if (!pakfile.open(file.c_str())){ /*, Common::File::kFileReadMode, path.c_str())) {*/
		printf("pakfile couldn't open %s\n", file.c_str());
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
		if (!(*chunk->_name))
			break;

		endoffset = READ_LE_UINT32(_buffer + pos);
		pos += 4;

		if (endoffset == 0) {
			endoffset = filesize;
		}

		chunk->_data = _buffer + startoffset;
		chunk->_size = endoffset - startoffset;

		_files.push_back(chunk);

		if (endoffset == filesize)
			break;

		startoffset = endoffset;
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

