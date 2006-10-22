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

#include "gui/message.h"

#include "kyra/resource.h"
#include "kyra/script.h"
#include "kyra/wsamovie.h"
#include "kyra/screen.h"

namespace Kyra {
Resource::Resource(KyraEngine *engine) {
	_engine = engine;

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
		"ADL.PAK", "CHAPTER1.VRM", "COL.PAK", "DRAGON1.APK", "DRAGON2.APK", "FINALE.PAK",
		"INTRO1.PAK", "INTRO2.PAK", "INTRO3.PAK", "INTRO4.PAK", "MISC.PAK",	"SND.PAK",
		"STARTUP.PAK", "XMI.PAK", 0
	};

	/*static const char *kyra2CDFilelist[] = {
		"ALLEY.PAK", "COST3_SH.PAK", "DINOD.PAK", "GARDEN.PAK", "INJAIL2.PAK", "MISC_EMC.PAK", "PHONE_B.PAK",
		"STATION.PAK", "VOLCANO.PAK", "VOLC_L.PAK", "ALTAR.PAK", "COST4_SH.PAK", "DOCK.PAK", "GEARS.PAK",
		"INSHOP.PAK", "MYSTRM.PAK", "PHONE_C.PAK", "STREET.PAK", "VOLC_A.PAK", "VOLC_M.PAK", "ANCHOR.PAK", 
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
		"MISC_EMC.PAK", "MISC_CPS.PAK", "OTHER.PAK", "ONETIME.PAK", 0
	};*/
	
	const char **usedFilelist = 0;

	if (_engine->game() == GI_KYRA1) {
		/*if (_engine->features() & GF_AMIGA)
			usedFilelist = kyra1AmigaFilelist;
		else*/ if (_engine->features() & GF_FLOPPY)
			usedFilelist = kyra1Filelist;
		else if (_engine->features() & GF_TALKIE)
			usedFilelist = kyra1CDFilelist;
	} /*else if (_engine->game() == GI_KYRA2) {
		// TODO: add kyra2 floppy file list
		usedFilelist = kyra2CDFilelist;
	} else if (_engine->game() == GI_KYRA3) {
		usedFilelist = kyra3Filelist;
	}*/

	// we're loading KYRA.DAT here too (but just for Kyrandia 1)
	if (_engine->game() == GI_KYRA1) {
		if (!loadPakFile("KYRA.DAT") || !StaticResource::checkKyraDat()) {
			GUI::MessageDialog errorMsg("You're missing the 'KYRA.DAT' file or it got corrupted, (re)get it from the ScummVM website");
			errorMsg.runModal();
			error("You're missing the 'KYRA.DAT' file or it got corrupted, (re)get it from the ScummVM website");
		}
	}
	
	// We only need kyra.dat for the demo.
	if (_engine->features() & GF_DEMO)
		return;
		
	if (!usedFilelist)
		error("no filelist found for this game");

	for (uint32 tmp = 0; usedFilelist[tmp]; ++tmp) {
		// prefetch file
		PAKFile *file = new PAKFile(usedFilelist[tmp], (_engine->features() & GF_AMIGA) != 0);
		assert(file);

		PakFileEntry newPak(file, usedFilelist[tmp]);
		if (file->isOpen() && file->isValid()) {
			_pakfiles.push_back(newPak);
		} else {
			delete file;
			debug(3, "couldn't load file '%s' correctly", usedFilelist[tmp]);
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

bool Resource::loadPakFile(const Common::String &filename) {
	Common::List<PakFileEntry>::iterator start = _pakfiles.begin();
	for (;start != _pakfiles.end(); ++start) {
		if (scumm_stricmp(start->_filename.c_str(), filename.c_str()) == 0) {
			start->_file->open();
			return true;
		}
	}

	PAKFile *file = new PAKFile(filename.c_str());
	if (!file)
		return false;
	if (!file->isValid()) {
		warning("couldn't load file: '%s'", filename.c_str());
		return false;
	}
	PakFileEntry newPak(file, filename);
	_pakfiles.push_back(newPak);
	return true;
}

void Resource::unloadPakFile(const Common::String &filename) {
	Common::List<PakFileEntry>::iterator start = _pakfiles.begin();
	for (;start != _pakfiles.end(); ++start) {
		if (scumm_stricmp(start->_filename.c_str(), filename.c_str()) == 0) {
			start->_file->close();
			break;
		}
	}
	return;
}

bool Resource::isInPakList(const Common::String &filename) {
	Common::List<PakFileEntry>::iterator start = _pakfiles.begin();
	for (;start != _pakfiles.end(); ++start) {
		if (scumm_stricmp(start->_filename.c_str(), filename.c_str()) == 0)
			return start->_file->isOpen();
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
			if (!start->_file->isOpen())
				continue;

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
		if (!start->_file->isOpen())
			continue;

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
#define PAKFile_Iterate Common::List<PakChunk>::iterator start=_files.begin();start != _files.end(); ++start
PAKFile::PAKFile(const char *file, bool isAmiga) {
	_isAmiga = isAmiga;

	Common::File pakfile;
	_open = false;

	if (!pakfile.open(file)) {
		debug(3, "couldn't open pakfile '%s'\n", file);
		return;
	}

	uint32 filesize = pakfile.size();

	// works with the file
	uint32 pos = 0, startoffset = 0, endoffset = 0;

	if (!_isAmiga) {
		startoffset = pakfile.readUint32LE();
	} else {
		startoffset = pakfile.readUint32BE();
	}
	pos += 4;

	while (pos < filesize) {
		PakChunk chunk;
		uint8 buffer[256];
		uint32 nameLength;

		// Move to the position of the next file entry
		pakfile.seek(pos);

		// Read in the header
		pakfile.read(&buffer, sizeof(buffer)); 

		// Quit now if we encounter an empty string
		if (!(*((const char*)buffer)))
			break;

		chunk._name = (const char*)buffer;
		nameLength = strlen(chunk._name.c_str()) + 1;

		if (!_isAmiga) {
			endoffset = READ_LE_UINT32(buffer + nameLength);
		} else {
			endoffset = READ_BE_UINT32(buffer + nameLength);
		}

		if (endoffset == 0) {
			endoffset = filesize;
		}

		chunk._start = startoffset;
		chunk._size = endoffset - startoffset;

		_files.push_back(chunk);

		if (endoffset == filesize)
			break;

		startoffset = endoffset;
		pos += nameLength + 4;
	}
	_open = true;
	
	_filename = file;
}

PAKFile::~PAKFile() {
	_filename.clear();
	_open = false;

	_files.clear();
}

uint8 *PAKFile::getFile(const char *file) {
	for (PAKFile_Iterate) {
		if (!scumm_stricmp(start->_name.c_str(), file)) {
			Common::File pakfile;
			if (!pakfile.open(_filename)) {
				debug(3, "couldn't open pakfile '%s'\n", _filename.c_str());
				return 0;
			}
			pakfile.seek(start->_start);
			uint8 *buffer = new uint8[start->_size];
			assert(buffer);
			pakfile.read(buffer, start->_size);
			return buffer;
		}
	}
	return 0;
}

bool PAKFile::getFileHandle(const char *file, Common::File &filehandle) {
	filehandle.close();

	for (PAKFile_Iterate) {
		if (!scumm_stricmp(start->_name.c_str(), file)) {
			if (!filehandle.open(_filename)) {
				debug(3, "couldn't open pakfile '%s'\n", _filename.c_str());
				return 0;
			}
			filehandle.seek(start->_start);
			return true;
		}
	}
	return false;
}

uint32 PAKFile::getFileSize(const char* file) {
	for (PAKFile_Iterate) {
		if (!scumm_stricmp(start->_name.c_str(), file))
			return start->_size;
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

} // end of namespace Kyra
