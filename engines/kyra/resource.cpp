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

	if (_engine->game() == GI_KYRA1) {
		// we're loading KYRA.DAT here too (but just for Kyrandia 1)
		if (!loadPakFile("KYRA.DAT")) {
			GUI::MessageDialog errorMsg("You're missing the 'KYRA.DAT' file, get it from the ScummVM website");
			errorMsg.runModal();
			error("couldn't open Kyrandia resource file ('KYRA.DAT') make sure you got one file for your version");
		}
	} else if (_engine->game() == GI_KYRA3) {
		// load the installation package file for kyra3
		INSFile *insFile = new INSFile("WESTWOOD.001");
		assert(insFile);
		if (!insFile->isValid())
			return;
		_pakfiles.push_back(insFile);
	}
	
	// We only need kyra.dat for the demo.
	if (_engine->features() & GF_DEMO)
		return;
		
	if (!usedFilelist)
		error("no filelist found for this game");

	for (uint32 tmp = 0; usedFilelist[tmp]; ++tmp) {
		loadPakFile(usedFilelist[tmp]);
	}
}

Resource::~Resource() {
	Common::List<ResourceFile*>::iterator start = _pakfiles.begin();

	for (;start != _pakfiles.end(); ++start) {
		delete *start;
		*start = 0;
	}
}

bool Resource::loadPakFile(const Common::String &filename) {
	if (isInPakList(filename))
		return true;

	uint32 size = 0;
	Common::File handle;
	if (!fileHandle(filename.c_str(), &size, handle)) {
		warning("couldn't load file: '%s'", filename.c_str());
		return false;
	}

	PAKFile *file = 0;

	if (handle.name() == filename) {
		file = new PAKFile(filename.c_str(), (_engine->features() & GF_AMIGA) != 0);
	} else {
		uint32 offset = handle.pos();
		uint8 *buf = new uint8[size];
		handle.read(buf, size);
		file = new PAKFile(filename.c_str(), handle.name(), offset, buf, size, (_engine->features() & GF_AMIGA) != 0);
		delete [] buf;
	}
	handle.close();

	if (!file)
		return false;
	if (!file->isValid()) {
		warning("'%s' is no valid pak file!", filename.c_str());
		delete file;
		return false;
	}

	_pakfiles.push_back(file);
	return true;
}

void Resource::unloadPakFile(const Common::String &filename) {
	Common::List<ResourceFile*>::iterator start = _pakfiles.begin();
	for (;start != _pakfiles.end(); ++start) {
		if (scumm_stricmp((*start)->filename().c_str(), filename.c_str()) == 0) {
			delete *start;
			_pakfiles.erase(start);
			break;
		}
	}
	return;
}

bool Resource::isInPakList(const Common::String &filename) {
	Common::List<ResourceFile*>::iterator start = _pakfiles.begin();
	for (;start != _pakfiles.end(); ++start) {
		if (scumm_stricmp((*start)->filename().c_str(), filename.c_str()) == 0)
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
		Common::List<ResourceFile*>::iterator start = _pakfiles.begin();

		for (;start != _pakfiles.end(); ++start) {
			*size = (*start)->getFileSize(file);
			
			if (!(*size))
				continue;
			
			buffer = (*start)->getFile(file);
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

	Common::List<ResourceFile*>::iterator start = _pakfiles.begin();

	for (;start != _pakfiles.end(); ++start) {
		*size = (*start)->getFileSize(file);
		
		if (!(*size))
			continue;

		if ((*start)->getFileHandle(file, filehandle)) {
			return true;
		}
	}
	
	return false;
}

///////////////////////////////////////////
// Pak file manager
#define PAKFile_Iterate Common::List<PakChunk>::iterator start=_files.begin();start != _files.end(); ++start
PAKFile::PAKFile(const char *file, bool isAmiga) : ResourceFile() {
	_isAmiga = isAmiga;

	Common::File pakfile;
	uint8 *buffer = 0;
	_open = false;

	if (!pakfile.open(file)) {
		debug(3, "couldn't open pakfile '%s'\n", file);
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
		PakChunk chunk;

		// saves the name
		chunk._name = (const char*)buffer + pos;
		pos += strlen(chunk._name.c_str()) + 1;
		if (!(chunk._name[0]))
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

		chunk._start = startoffset;
		chunk._size = endoffset - startoffset;

		_files.push_back(chunk);

		if (endoffset == filesize)
			break;

		startoffset = endoffset;
	}
	_open = true;
	delete [] buffer;
	
	_filename = file;
	_physfile = "";
}

PAKFile::PAKFile(const char *file, const char *physfile, const uint32 off, const uint8 *buffer, uint32 filesize, bool isAmiga) : ResourceFile() {
	_isAmiga = isAmiga;
	_open = false;

	// works with the file
	uint32 pos = 0, startoffset = 0, endoffset = 0;

	if (!_isAmiga) {
		startoffset = READ_LE_UINT32(buffer + pos);
	} else {
		startoffset = READ_BE_UINT32(buffer + pos);
	}
	pos += 4;

	while (pos < filesize) {
		PakChunk chunk;

		// saves the name
		chunk._name = (const char*)buffer + pos;
		pos += strlen(chunk._name.c_str()) + 1;
		if (!(chunk._name[0]))
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

		chunk._start = startoffset;
		chunk._size = endoffset - startoffset;

		_files.push_back(chunk);

		if (endoffset == filesize)
			break;

		startoffset = endoffset;
	}

	_open = true;
	_physfile = physfile;
	_physOffset = off;
	_filename = file;
}

PAKFile::~PAKFile() {
	_filename.clear();
	_physfile.clear();
	_open = false;

	_files.clear();
}

uint8 *PAKFile::getFile(const char *file) {
	for (PAKFile_Iterate) {
		if (!scumm_stricmp(start->_name.c_str(), file)) {
			Common::File pakfile;
			if (!openFile(pakfile))
				return false;
			pakfile.seek(start->_start, SEEK_CUR);
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
			if (!openFile(filehandle))
				return false;
			filehandle.seek(start->_start, SEEK_CUR);
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

bool PAKFile::openFile(Common::File &filehandle) {
	filehandle.close();

	if (!filehandle.open(_physfile == "" ? _filename : _physfile)) {
		debug(3, "couldn't open pakfile '%s'\n", _filename.c_str());
		return false;
	}

	if (_physfile != "") {
		filehandle.seek(_physOffset, SEEK_CUR);
	}

	return true;
}

///////////////////////////////////////////
// Ins file manager
#define INSFile_Iterate Common::List<FileEntry>::iterator start=_files.begin();start != _files.end(); ++start
INSFile::INSFile(const char *file) : ResourceFile(), _files() {
	Common::File pakfile;
	_open = false;

	if (!pakfile.open(file)) {
		debug(3, "couldn't open insfile '%s'\n", file);
		return;
	}

	// thanks to eriktorbjorn for this code (a bit modified though)

	// skip first three bytes
	pakfile.seek(3);

	// first file is the index table
	uint32 filesize = pakfile.readUint32LE();

	Common::String temp = "";

	for (uint i = 0; i < filesize; ++i) {
		byte c = pakfile.readByte();

		if (c == '\\') {
			temp = "";
		} else if (c == 0x0D) {
			// line endings are CRLF
			c = pakfile.readByte();
			assert(c == 0x0A);
			++i;

			FileEntry newEntry;
			newEntry._name = temp;
			newEntry._start = 0;
			newEntry._size = 0;
			_files.push_back(newEntry);

			temp = "";
		} else {
			temp += (char)c;
		}
	}

	pakfile.seek(3);

	for (INSFile_Iterate) {
		filesize = pakfile.readUint32LE();
		start->_size = filesize;
		start->_start = pakfile.pos();
		pakfile.seek(filesize, SEEK_CUR);
	}

	_filename = file;
	_open = true;
}

INSFile::~INSFile() {
	_filename.clear();
	_open = false;

	_files.clear();
}

uint8 *INSFile::getFile(const char *file) {
	for (INSFile_Iterate) {
		if (!scumm_stricmp(start->_name.c_str(), file)) {
			Common::File pakfile;
			if (!pakfile.open(_filename)) {
				debug(3, "couldn't open insfile '%s'\n", _filename.c_str());
				return false;
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

bool INSFile::getFileHandle(const char *file, Common::File &filehandle) {
	for (INSFile_Iterate) {
		if (!scumm_stricmp(start->_name.c_str(), file)) {
			if (!filehandle.open(_filename)) {
				debug(3, "couldn't open insfile '%s'\n", _filename.c_str());
				return false;
			}
			filehandle.seek(start->_start, SEEK_CUR);
			return true;
		}
	}
	return false;
}

uint32 INSFile::getFileSize(const char *file) {
	for (INSFile_Iterate) {
		if (!scumm_stricmp(start->_name.c_str(), file))
			return start->_size;
	}
	return 0;
}

////////////////////////////////////////////
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
