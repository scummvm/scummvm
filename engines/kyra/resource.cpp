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
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/endian.h"
#include "common/file.h"
#include "kyra/resource.h"
#include "kyra/script.h"
#include "kyra/wsamovie.h"
#include "kyra/screen.h"

namespace Kyra {
Resource::Resource(KyraEngine *engine) {
	_engine = engine;

	// No PAK files in the demo version
	if (_engine->features() & GF_DEMO)
		return;

	// prefetches all PAK Files

	// ugly a hardcoded list
	// TODO: use the FS Backend to get all .PAK Files and load them
	// or any other thing to get all files
	static const char *kyra1Filelist[] = {
		"A_E.PAK", "DAT.PAK", "F_L.PAK", "MAP_5.PAK", "MSC.PAK", "M_S.PAK",
		"S_Z.PAK", "WSA1.PAK", "WSA2.PAK", "WSA3.PAK", "WSA4.PAK", "WSA5.PAK",
		"WSA6.PAK", 0
	};

	/*static const char *kyra1AmigaFilelist[] = {
		"alchemy.pak",  "chasm.pak",    "finale1.pak",   "gen_cav.pak",  "mix.pak",      "spring.pak",
		"algae.pak",    "cliff.pak",    "finale2.pak",   "genhall.pak",  "mooncav.pak",  "square.pak",
		"altar.pak",    "darms.pak",    "fnorth.pak",    "glade.pak",    "ncliffb.pak",  "startup.pak",
		"arch.pak",     "dead.pak",     "foresta.pak",   "grave.pak",    "ncliff.pak",   "stump.pak",
		"balcony.pak",  "dnstair.pak",  "forestb.pak",   "grthall.pak",  "nwclifb.pak",  "temple.pak",
		"belroom.pak",  "dragon.pak",   "forestc.pak",   "healer.pak",   "nwcliff.pak",  "trunk.pak",
		"bonkbg.pak",   "drgnwsa.pak",  "fountn.pak",    "herman.pak",   "oaks.pak",     "upstair.pak",
		"bridge.pak",   "edgeb.pak",    "foyer.pak",     "intro1.pak",   "plateau.pak",  "well.pak",
		"brins.pak",    "edge.pak",     "frstawsa.pak",  "kitchen.pak",  "pltcave.pak",  "willow.pak",
		"broken.pak",   "emcav.pak",    "fsouthb.pak",   "kyragem.pak",  "potion.pak",   "wise.pak",
		"burn.pak",     "enter.pak",    "fsouth.pak",    "lagoon.pak",   "potwsa.pak",   "xedgeb.pak",
		"castle.pak",   "extheal.pak",  "fwststh.pak",   "landing.pak",  "ruby.pak",     "xedgec.pak",
		"catacom.pak",  "extpot.pak",   "gatecv.pak",    "lava.pak",     "sickwil.pak",  "xedge.pak",
		"caveb.pak",    "extspel.pak",  "gemcut.pak",    "lephole.pak",  "song.pak",     "zrock.pak",
		"cave.pak",     "falls.pak",    "gem.pak",       "library.pak",  "sorrow.pak",
		"cgate.pak",    "fgowest.pak",  "gencavb.pak",   "misc.pak",     "spell.pak", 0
	};*/

	static const char *kyra1CDFilelist[] = {
		"ALTAR.APK", "BELROOM.APK", "BONKBG.APK", "BROKEN.APK", "CASTLE.APK", "CAVE.APK", "CGATE.APK",
		"DEAD.APK", "DNSTAIR.APK", "DRAGON1.APK", "DRAGON2.APK", "EXTPOT.APK", "FORESTA.APK", "FORESTB.APK",
		"FOUNTN.APK", "FOYER.APK", "GATECV.APK", "GEM.APK", "GEMCUT.APK", "GENHALL.APK", "GLADE.APK", 
		"GRAVE.APK", "HEALER.APK", "LAGOON.APK", "LANDING.APK", "LAVA.APK", "LEPHOLE.APK", "LIBRARY.APK",
		"MIX.APK", "MOONCAV.APK", "POTION.APK", "SONG.APK", "SORROW.APK", "SPELL.APK", "STUMP.APK",
		"TEMPLE.APK", "TRUNK.APK", "WILLOW.APK", "XEDGE.APK",

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
		"GATECV.PAK", "HEALER.PAK", "LAVA.PAK", "NWCLIFF.PAK", "SORROW.PAK", "WELL.PAK",
		
		"CHAPTER1.VRM", 0
	};

	static const char *kyra2CDFilelist[] = {
		"ALLEY.PAK", "COST3_SH.PAK", "DINOD.PAK", "GARDEN.PAK", "INJAIL2.PAK", "MISC_EMC.PAK", "PHONE_B.PAK",
		"STATION.PAK", "VOLCANO.PAK", "VOLC_L.PAK", "ALTAR.PAK", "COST4_SH.PAK", "DOCK.PAK", "GEARS.PAK",
		"INSHOP.PAK", "MYSTRM.PAK," "PHONE_C.PAK", "STREET.PAK", "VOLC_A.PAK", "VOLC_M.PAK", "ANCHOR.PAK", 
		"COST5_SH.PAK", "DOOR.PAK", "GEARS2.PAK", "INSTORE.PAK", "NEST.PAK", "PHONE_D.PAK", "SULFUR.PAK",
		"VOLC_B.PAK", "VOLC_N.PAK", "AUDIO.PAK", "COST6_SH.PAK", "DRIVERS.PAK", "GEARS3.PAK", "INTRODRV.PAK",
		"ONBOAT.PAK", "QUICK.PAK", "TALKENG.PAK", "VOLC_C.PAK", "VOLC_O.PAK", "BRIDGE.PAK", "COST7_SH.PAK",
		"FALL.PAK", "GNARL.PAK", "INTROGEN.PAK", "OTHER.PAK", "RAINA.PAK", "TALKFRE.PAK", "VOLC_D.PAK",
		"VOLC_P.PAK", "CAULDRON.PAK", "COST8_SH.PAK", "FATE.PAK", "HANOI.PAK", "INTROTLK.PAK", "OUTCAVE.PAK",
		"RAINB.PAK", "TALKGER.PAK", "VOLC_E.PAK", "WHARF.PAK", "CELLAR.PAK", "COST9_SH.PAK", "FERRY.PAK",
		"HOLE.PAK", "INTROVOC.PAK", "OUTCAVE2.PAK", "RAT.PAK", "TAVERN.PAK", "VOLC_F.PAK", "WHEEL.PAK",
		"CLEARNG.PAK", "CRICKET.PAK", "FIGHT.PAK", "INCAVE.PAK", "ISLE.PAK", "OUTFARM.PAK", "ROAD.PAK",
		"TIMBER.PAK", "VOLC_G.PAK", "CLIFF.PAK", "CROC.PAK", "FISHER.PAK", "INGATE.PAK", "JUNGLE.PAK",
		"OUTGATE.PAK", "ROPE.PAK", "TRAM.PAK", "VOLC_H.PAK", "CLOSE.PAK", "DINOA.PAK", "FLOAT.PAK", "INHERB.PAK",
		"MARKHME.PAK", "OUTHERB.PAK", "SCORCH.PAK", "TREE.PAK", "VOLC_I.PAK", "COST1_SH.PAK", "DINOB.PAK",
		"FLYTRAP.PAK", "INHOME.PAK", "MEADOW.PAK", "OUTHOME.PAK", "SKULL.PAK", "TREE2.PAK", "VOLC_J.PAK",
		"COST2_SH.PAK", "DINOC.PAK", "FOOT.PAK", "INJAIL.PAK", "MISC_CPS.PAK", "PHONE_A.PAK", "SKY.PAK", "VOC.PAK",
		"VOLC_K.PAK", 0
	};
	
	static const char *kyra3Filelist[] = {
		// enough for now
		"ONETIME.PAK", 0
	};
	
	const char **usedFilelist = 0;

	if (_engine->game() == GI_KYRA1) {
		/*if (_engine->features() & GF_AMIGA)
			usedFilelist = kyra1AmigaFilelist;
			else*/ if (_engine->features() & GF_FLOPPY)
				usedFilelist = kyra1Filelist;
			else if (_engine->features() & GF_TALKIE)
				usedFilelist = kyra1CDFilelist;
	} else if (_engine->game() == GI_KYRA2) {
		// TODO: add kyra2 floppy file list
		usedFilelist = kyra2CDFilelist;
	} else if (_engine->game() == GI_KYRA3) {
		usedFilelist = kyra3Filelist;
	}
		
	if (!usedFilelist)
		error("no filelist found for this game");

	for (uint32 tmp = 0; usedFilelist[tmp]; ++tmp) {
		// prefetch file
		PAKFile *file = new PAKFile(usedFilelist[tmp], (_engine->features() & GF_AMIGA) != 0);
		assert(file);

		PakFileEntry newPak;
		newPak._file = file;
		strncpy(newPak._filename, usedFilelist[tmp], 32);
		if (file->isOpen() && file->isValid()) {
			_pakfiles.push_back(newPak);
		} else {
			delete file;
			debug(3, "couldn't load file '%s' correctly", usedFilelist[tmp]);
		}
	}

	// we're loading KYRA.DAT here too (but just for Kyrandia 1)
	if (_engine->game() == GI_KYRA1) {
		if (!loadPakFile("KYRA.DAT")) {
			error("couldn't open Kyrandia resource file ('KYRA.DAT') make sure you got one file for your version");
		}
	}
}

Resource::~Resource() {
	Common::List<PakFileEntry>::iterator start = _pakfiles.begin();

	for (;start != _pakfiles.end(); ++start) {
		delete start->_file;
		start->_file = 0;
	}
}

bool Resource::loadPakFile(const char *filename) {
	if (isInPakList(filename))
		return true;
	PAKFile *file = new PAKFile(filename);
	if (!file) {
		warning("couldn't load file: '%s'", filename);
		return false;
	}
	PakFileEntry newPak;
	newPak._file = file;
	strncpy(newPak._filename, filename, 32);
	_pakfiles.push_back(newPak);
	return true;
}

void Resource::unloadPakFile(const char *filename) {
	Common::List<PakFileEntry>::iterator start = _pakfiles.begin();
	for (;start != _pakfiles.end(); ++start) {
		if (scumm_stricmp(start->_filename, filename) == 0) {
			delete start->_file;
			_pakfiles.erase(start);
			break;
		}
	}
	return;
}

bool Resource::isInPakList(const char *filename) {
	Common::List<PakFileEntry>::iterator start = _pakfiles.begin();
	for (;start != _pakfiles.end(); ++start) {
		if (scumm_stricmp(start->_filename, filename) == 0)
			return true;
	}
	return false;
}

uint8 *Resource::fileData(const char *file, uint32 *size) {
	uint8 *buffer = 0;
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
		Common::List<PakFileEntry>::iterator start = _pakfiles.begin();

		for (;start != _pakfiles.end(); ++start) {
			*size = start->_file->getFileSize(file);
			
			if (!(*size))
				continue;
			
			buffer = start->_file->getFile(file);
			break;
		}
	}

	if (!buffer || !(*size)) {
		return 0;
	}

	return buffer;
}

bool Resource::fileHandle(const char *file, uint32 *size, Common::File &filehandle) {
	filehandle.close();

	if (filehandle.open(file))
		return true;

	Common::List<PakFileEntry>::iterator start = _pakfiles.begin();

	for (;start != _pakfiles.end(); ++start) {
		*size = start->_file->getFileSize(file);
		
		if (!(*size))
			continue;

		if (start->_file->getFileHandle(file, filehandle)) {
			return true;
		}
	}
	
	return false;
}

///////////////////////////////////////////
// Pak file manager
#define PAKFile_Iterate Common::List<PakChunk*>::iterator start=_files.begin();start != _files.end(); ++start
PAKFile::PAKFile(const Common::String& file, bool isAmiga) {
	_filename = 0;
	_isAmiga = isAmiga;

	Common::File pakfile;
	uint8 *buffer = 0;
	_open = false;

	if (!pakfile.open(file)) {
		debug(3, "couldn't open pakfile '%s'\n", file.c_str());
		return;
	}

	uint32 filesize = pakfile.size();
	buffer = new uint8[filesize];
	assert(buffer);

	pakfile.read(buffer, filesize);
	pakfile.close();

	// works with the file
	uint32 pos = 0, startoffset = 0, endoffset = 0;

	if (!_isAmiga) {
		startoffset = READ_LE_UINT32(buffer + pos);
	} else {
		startoffset = READ_BE_UINT32(buffer + pos);
	}
	pos += 4;

	while (pos < filesize) {
		PakChunk* chunk = new PakChunk;
		assert(chunk);

		// saves the name
		chunk->_name = new char[strlen((const char*)buffer + pos) + 1];
		assert(chunk->_name);
		strcpy(chunk->_name, (const char*)buffer + pos);
		pos += strlen(chunk->_name) + 1;
		if (!(*chunk->_name))
			break;

		if (!_isAmiga) {
			endoffset = READ_LE_UINT32(buffer + pos);
		} else {
			endoffset = READ_BE_UINT32(buffer + pos);
		}
		pos += 4;

		if (endoffset == 0) {
			endoffset = filesize;
		}

		chunk->_start = startoffset;
		chunk->_size = endoffset - startoffset;

		_files.push_back(chunk);

		if (endoffset == filesize)
			break;

		startoffset = endoffset;
	}
	_open = true;
	delete [] buffer;
	
	_filename = new char[file.size()+1];
	assert(_filename);
	strcpy(_filename, file.c_str());
}

PAKFile::~PAKFile() {
	delete [] _filename;
	_filename = 0;
	_open = false;

	for (PAKFile_Iterate) {
		delete [] (*start)->_name;
		(*start)->_name = 0;
		delete *start;
		*start = 0;
	}
}

uint8 *PAKFile::getFile(const char *file) {
	for (PAKFile_Iterate) {
		if (!scumm_stricmp((*start)->_name, file)) {
			Common::File pakfile;
			if (!pakfile.open(_filename)) {
				debug(3, "couldn't open pakfile '%s'\n", _filename);
				return 0;
			}
			pakfile.seek((*start)->_start);
			uint8 *buffer = new uint8[(*start)->_size];
			assert(buffer);
			pakfile.read(buffer, (*start)->_size);
			return buffer;
		}
	}
	return 0;
}

bool PAKFile::getFileHandle(const char *file, Common::File &filehandle) {
	filehandle.close();

	for (PAKFile_Iterate) {
		if (!scumm_stricmp((*start)->_name, file)) {
			if (!filehandle.open(_filename)) {
				debug(3, "couldn't open pakfile '%s'\n", _filename);
				return 0;
			}
			filehandle.seek((*start)->_start);
			return true;
		}
	}
	return false;
}

uint32 PAKFile::getFileSize(const char* file) {
	for (PAKFile_Iterate) {
		if (!scumm_stricmp((*start)->_name, file))
			return (*start)->_size;
	}
	return 0;
}

void KyraEngine::loadPalette(const char *filename, uint8 *palData) {
	debugC(9, kDebugLevelMain, "KyraEngine::loadPalette('%s' %p)", filename, (void *)palData);
	uint32 fileSize = 0;
	uint8 *srcData = _res->fileData(filename, &fileSize);

	if (palData && fileSize) {
		debugC(9, kDebugLevelMain,"Loading a palette of size %i from '%s'", fileSize, filename);
		memcpy(palData, srcData, fileSize);
	}
	delete [] srcData;
}

void KyraEngine::loadBitmap(const char *filename, int tempPage, int dstPage, uint8 *palData) {
	debugC(9, kDebugLevelMain, "KyraEngine::loadBitmap('%s', %d, %d, %p)", filename, tempPage, dstPage, (void *)palData);
	uint32 fileSize;
	uint8 *srcData = _res->fileData(filename, &fileSize);

	if (!srcData) {
		warning("coudln't load bitmap: '%s'", filename);
		return;
	}

	uint8 compType = srcData[2];
	uint32 imgSize = READ_LE_UINT32(srcData + 4);
	uint16 palSize = READ_LE_UINT16(srcData + 8);

	if (palData && palSize) {
		debugC(9, kDebugLevelMain,"Loading a palette of size %i from %s", palSize, filename);
		memcpy(palData, srcData + 10, palSize);		
	}

	uint8 *srcPtr = srcData + 10 + palSize;
	uint8 *dstData = _screen->getPagePtr(dstPage);

	switch (compType) {
	case 0:
		memcpy(dstData, srcPtr, imgSize);
		break;
	case 3:
		Screen::decodeFrame3(srcPtr, dstData, imgSize);
		break;
	case 4:
		Screen::decodeFrame4(srcPtr, dstData, imgSize);
		break;
	default:
		error("Unhandled bitmap compression %d", compType);
		break;
	}

	delete [] srcData;
}

} // end of namespace Kyra

