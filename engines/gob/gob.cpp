/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2006 The ScummVM project
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
#include "common/stdafx.h"
#include "common/endian.h"

#include "base/plugins.h"
#include "common/config-manager.h"
#include "common/fs.h"
#include "common/md5.h"

#include "gob/gob.h"

#include "gob/global.h"
#include "gob/game.h"
#include "gob/sound.h"
#include "gob/init.h"
#include "gob/inter.h"
#include "gob/draw.h"
#include "gob/anim.h"
#include "gob/cdrom.h"
#include "gob/goblin.h"
#include "gob/map.h"
#include "gob/mult.h"
#include "gob/pack.h"
#include "gob/palanim.h"
#include "gob/parse.h"
#include "gob/scenery.h"
#include "gob/timer.h"
#include "gob/util.h"
#include "gob/music.h"

#include "sound/mididrv.h"

// Use the original saves. Just for testing purposes, will be removed later
// The new method is more convenient, and, more importantly, endian-safe
//#define GOB_ORIGSAVES

namespace Gob {

enum {
	// We only compute MD5 of the first 5000 bytes of our data files.
	kMD5FileSizeLimit = 5000
};

struct GameSettings {
	const char *gameid;
	const char *description;
	uint32 features;
	Common::Language lang;
	const char *md5sum;
	const char *startTotBase;
};

static const GameSettings gob_games[] = {
	// Supplied by Florian Zeitz on scummvm-devel
	{"gob1", "Gobliiins (DOS EGA)", GF_GOB1 | GF_EGA, Common::UNK_LANG, "c65e9cc8ba23a38456242e1f2b1caad4", "intro"},
	{"gob1", "Gobliiins (DOS EGA Ru)", GF_GOB1 | GF_EGA, Common::RU_RUS, "f9233283a0be2464248d83e14b95f09c", "intro"},
	//{"gob1", "Gobliiins (Windows)", GF_GOB1, Common::UNK_LANG, "8a5e850c49d7cacdba5f5eb1fcc77b89", "intro"},

	// Supplied by Theruler76 in bug report #1201233
	{"gob1", "Gobliiins (DOS VGA)", GF_GOB1, Common::UNK_LANG, "26a9118c0770fa5ac93a9626761600b2", "intro"},

	// CD 1.000 version. Multilingual
	{"gob1", "Gobliiins (CD)", GF_GOB1 | GF_CD, Common::UNK_LANG, "2fbf4b5b82bbaee87eb45d4404c28998", "intro"},
	// CD 1.02 version. Multilingual
	{"gob1", "Gobliiins (CD)", GF_GOB1 | GF_CD, Common::UNK_LANG, "8bd873137b6831c896ee8ad217a6a398", "intro"},

	{"gob1", "Gobliiins (Amiga)", GF_GOB1, Common::UNK_LANG, "c65e9cc8ba23a38456242e1f2b1caad4", "intro"},
	{"gob1", "Gobliiins (Amiga)", GF_GOB1, Common::UNK_LANG, "972f22c6ff8144a6636423f0354ca549", "intro"},

	{"gob1", "Gobliiins (Interactive Demo)", GF_GOB1, Common::UNK_LANG, "e72bd1e3828c7dec4c8a3e58c48bdfdb", "intro"},
	
	{"gob1", "Gobliiins (Mac)", GF_GOB1 | GF_MAC, Common::UNK_LANG, "00a42a7d2d22e6b6ab1b8c673c4ed267", "intro"},

	{"gob2", "Gobliins 2 (DOS Fra)", GF_GOB2, Common::FR_FRA, "a13ecb4f6d8fd881ebbcc02e45cb5475", "intro"},
	{"gob2", "Gobliins 2 (DOS Grb)", GF_GOB2, Common::EN_GRB, "b45b984ee8017efd6ea965b9becd4d66", "intro"},
	{"gob2", "Gobliins 2 (DOS USA)", GF_GOB2, Common::EN_USA, "dedb5d31d8c8050a8cf77abedcc53dae", "intro"},
	{"gob2", "Gobliins 2 (DOS Deu)", GF_GOB2, Common::DE_DEU, "a13892cdf4badda85a6f6fb47603a128", "intro"},
	{"gob2", "Gobliins 2 (DOS Ru)", GF_GOB2, Common::RU_RUS, "cd3e1df8b273636ee32e34b7064f50e8", "intro"},

	// Supplied by blackwhiteeagle in bug report #1605235
	{"gob2", "Gobliins 2 (DOS Deu)", GF_GOB2, Common::DE_DEU, "3e4e7db0d201587dd2df4003b2993ef6", "intro"},

	// CD 1.000.
	{"gob2", "Gobliins 2 (CD)", GF_GOB2, Common::EN_USA, "9de5fbb41cf97182109e5fecc9d90347", "intro"},
	// CD 1.01
	{"gob2", "Gobliins 2 (CD)", GF_GOB2, Common::UNK_LANG, "24a6b32757752ccb1917ce92fd7c2a04", "intro"},

	{"gob2", "Gobliins 2 (Demo)", GF_GOB2, Common::UNK_LANG, "8b1c98ff2ab2e14f47a1b891e9b92217", "usa"},
	{"gob2", "Gobliins 2 (Interactive Demo)", GF_GOB2, Common::UNK_LANG, "cf1c95b2939bd8ff58a25c756cb6125e", "intro"},
	{"gob2", "Gobliins 2 (Amiga Interactive Demo)", GF_GOB2, Common::UNK_LANG, "4b278c2678ea01383fd5ca114d947eea", "intro"},

	{"gob2", "Ween: The Prohpecy", GF_GOB2, Common::UNK_LANG, "2bb8878a8042244dd2b96ff682381baa", "intro"},
	{"gob2", "Ween: The Prophecy (Fr)", GF_GOB2, Common::UNK_LANG, "4b10525a3782aa7ecd9d833b5c1d308b", "intro"},
	{"gob2", "Ween: The Prophecy (Demo)", GF_GOB2, Common::UNK_LANG, "2e9c2898f6bf206ede801e3b2e7ee428", "intro"},

	{"gob2", "Bargon Attack", GF_GOB2, Common::UNK_LANG, "da3c54be18ab73fbdb32db24624a9c23", "intro"},

#if 0
	{"gob3", "Goblins Quest 3", GF_GOB3, Common::UNK_LANG, "32b0f57f5ae79a9ae97e8011df38af42", "intro"},
	//{"gob3", "Goblins Quest 3", GF_GOB3, Common::UNK_LANG, "d129f639f6ca8d6b5f0f4e15edb91058", "intro"},
	{"gob3", "Goblins Quest 3", GF_GOB3, Common::UNK_LANG, "1e2f64ec8dfa89f42ee49936a27e66e7", "intro"},
	{"gob3", "Goblins Quest 3 (Fr)", GF_GOB3, Common::FR_FRA, "e42a4f2337d6549487a80864d7826972", "intro"},
	{"gob3", "Goblins Quest 3 (Ru)", GF_GOB3, Common::RU_RUS, "4e3af248a48a2321364736afab868527", "intro"},
	{"gob3", "Goblins Quest 3 (Mac)", GF_GOB3, Common::UNK_LANG, "8d28ce1591b0e9cc79bf41cad0fc4c9c", "intro"},
	// CD 1.000
	{"gob3", "Goblins Quest 3 (CD)", GF_GOB3, Common::UNK_LANG, "6f2c226c62dd7ab0ab6f850e89d3fc47", "intro"},
	// CD 1.02. Spanish "Computer Gaming World"* distribution in Spain
	{"gob3", "Goblins Quest 3 (CD)", GF_GOB3, Common::UNK_LANG, "c3e9132ea9dc0fb866b6d60dcda10261", "intro"},

	{"gob3", "Goblins Quest 3 (Interactive Demo)", GF_GOB3, Common::UNK_LANG, "7aebd94e49c2c5c518c9e7b74f25de9d", "intro"},
	{"gob3", "Goblins Quest 3 (Demo)", GF_GOB3, "b9b898fccebe02b69c086052d5024a55", "intro"},
	{"gob3", "Goblins Quest 3 (Interactive Demo)", GF_GOB3, Common::UNK_LANG, "e5dcbc9f6658ebb1e8fe26bc4da0806d", "intro"},

	// CD 1.0
	{"woodruff", "The Bizarre Adventures of Woodruff and the Schnibble", GF_WOODRUFF, Common::UNK_LANG, "dccf9d31cb720b34d75487408821b77e", "intro"},

	// CD 1.00, German release (INTRO.STRK seems to be multilingual, though?)
	{"woodruff", "The Bizarre Adventures of Woodruff and the Schnibble", GF_WOODRUFF, Common::UNK_LANG, "5f5f4e0a72c33391e67a47674b120cc6", "intro"},
#endif
	{0, 0, 0, Common::UNK_LANG, 0, 0}
};

// Keep list of different supported games
static const PlainGameDescriptor gob_list[] = {
	{"gob1", "Gobliiins"},
	{"gob2", "Gobliins 2"},
	{0, 0}
};


#define MAX_TIME_DELTA 100

GobEngine::GobEngine(OSystem * syst, uint32 features, Common::Language lang,
		const char *startTotBase)
 : Engine(syst) {
	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	_features = features;
	_language = lang;
	_copyProtection = ConfMan.getBool("copy_protection");
	_quitRequested = false;

	if (startTotBase == 0) {
		_startTot = new char[10];
		_startTot0 = new char[11];
		strcpy(_startTot, "intro.tot");
		strcpy(_startTot0, "intro0.tot");
	} else {
		_startTot = new char[strlen(startTotBase) + 5];
		_startTot0 = new char[strlen(startTotBase) + 6];
		strcpy(_startTot, startTotBase);
		strcpy(_startTot0, startTotBase);
		strcat(_startTot, ".tot");
		strcat(_startTot0, "0.tot");
	}

	int i;
	_saveFiles = new char*[3];
	for (i = 0; i < 3; i++)
		_saveFiles[i] = new char[_targetName.size() + 5];
	sprintf(_saveFiles[0], "%s.cat", _targetName.c_str());
	sprintf(_saveFiles[1], "%s.sav", _targetName.c_str());
	sprintf(_saveFiles[2], "%s.blo", _targetName.c_str());
	_saveSlotFile = new char[_targetName.size() + 5];
	sprintf(_saveSlotFile, "%s.s00", _targetName.c_str());
	memset(_saveIndex, 0, 600);
	memset(_saveIndexSizes, 0, 600);

	Common::addSpecialDebugLevel(DEBUG_FUNCOP, "FuncOpcodes", "Script FuncOpcodes debug level");
	Common::addSpecialDebugLevel(DEBUG_DRAWOP, "DrawOpcodes", "Script DrawOpcodes debug level");
	Common::addSpecialDebugLevel(DEBUG_GOBOP, "GoblinOpcodes", "Script GoblinOpcodes debug level");
	Common::addSpecialDebugLevel(DEBUG_MUSIC, "Music", "CD and adlib music debug level");
	Common::addSpecialDebugLevel(DEBUG_PARSER, "Parser", "Parser debug level");
	Common::addSpecialDebugLevel(DEBUG_GAMEFLOW, "Gameflow", "Gameflow debug level");
	Common::addSpecialDebugLevel(DEBUG_FILEIO, "FileIO", "File Input/Output debug level");
	Common::addSpecialDebugLevel(DEBUG_GRAPHICS, "Graphics", "Graphics debug level");
	Common::addSpecialDebugLevel(DEBUG_COLLISIONS, "Collisions", "Collisions debug level");
}

GobEngine::~GobEngine() {
	delete _mult;
	delete _game;
	delete _snd;
	delete _global;
	delete _draw;
	delete _anim;
	delete _cdrom;
	delete _dataio;
	delete _goblin;
	delete _init;
	delete _inter;
	delete _map;
	delete _pack;
	delete _palanim;
	delete _parse;
	delete _scenery;
	delete _gtimer;
	delete _util;
	delete _music;
	delete _video;
	delete[] _startTot;
	delete[] _startTot0;

	int i;
	for (i = 0; i < 3; i++)
		delete[] _saveFiles[i];
	delete[] _saveFiles;
	delete[] _saveSlotFile;
}

int GobEngine::go() {
	_init->initGame(0);

	return 0;
}

void GobEngine::shutdown() {
	_quitRequested = true;
}

// Seeking with SEEK_END (and therefore also pos()) doesn't work with
// gzip'd save files, so reading the whole thing in is necessary
uint32 GobEngine::getSaveSize(Common::InSaveFile &in) {
	char buf[1024];
	uint32 size;
	uint32 i;
	uint32 pos;

	size = 0;
	pos = in.pos();
	in.seek(0, SEEK_SET);
	while ((i = in.read(buf, 1024)) > 0)
		size += i;
	in.seek(0, pos);

	return size;
}

int32 GobEngine::getSaveSize(enum SaveFiles sFile) {
	int32 size;
	Common::InSaveFile *in;

	size = -1;
#ifndef GOB_ORIGSAVES
	int i;
	if (sFile == SAVE_CAT) {
		for (i = 14; i >= 0; i--)
			if ((in = _saveFileMan->openForLoading(getSaveSlotFile(i)))) {
				size = (i + 1) * READ_LE_UINT32(_game->_totFileData + 0x2C) * 4 + 600;
				delete in;
				break;
			}
		debugC(1, DEBUG_FILEIO, "Requested save games size: %d", size);
		return size;
	}
#endif // GOB_ORIGSAVES

	if ((in = _saveFileMan->openForLoading(_saveFiles[(int) sFile]))) {
		size = getSaveSize(*in);
		delete in;
	}

	debugC(1, DEBUG_FILEIO, "Requested size of file \"%s\": %d", _saveFiles[(int) sFile], size);

	return size;
}

const char *GobEngine::getSaveSlotFile(int slot) {
	static char *slotBase = _saveSlotFile + strlen(_targetName.c_str()) + 2;

	snprintf(slotBase, 3, "%02d", slot);
	return _saveSlotFile;
}

void GobEngine::saveGameData(enum SaveFiles sFile, int16 dataVar, int32 size, int32 offset) {
	int32 retSize;
	int16 index;
	int16 top;
	bool writePal;
	bool needEnforceEndian;
	char *sName;
	char *buf;
	char *oBuf;
	int32 iSize;
	int32 oSize;
	int32 oOff;
	Video::SurfaceDesc *destDesc;
	Video::SurfaceDesc *srcDesc;
	Common::InSaveFile *in;
	Common::OutSaveFile *out;

	index = 0;
	oBuf = 0;
	in = 0;
	writePal = false;
	sName = _saveFiles[(int) sFile];
	needEnforceEndian = false;

	WRITE_VAR(1, 1);

	if (size < 0) {
		if (size < -1000) {
			writePal = true;
			size += 1000;
		}
		index = -size - 1;
		assert((index >= 0) && (index < 50)); // Just to be sure...
		buf = (char *) _draw->_spritesArray[index]->vidPtr;
		size = _draw->getSpriteRectSize(index);
		if ((_draw->_spritesArray[index]->vidMode & 0x80) == 0)
			size = -size;
	} else {
		int32 varSize = READ_LE_UINT32(_game->_totFileData + 0x2C) * 4;

		if (size == 0) {
			dataVar = 0;
			size = varSize;
		}
		buf = _global->_inter_variables + dataVar;
#ifndef GOB_ORIGSAVES
		if (sFile == SAVE_CAT) {
			if(saveGame((offset - 600) / varSize, dataVar, size, offset))
				WRITE_VAR(1, 0);
			return;
		} else if (offset != 0) {
			warning("Can't write file \"%s\": Can't correctly enfore endianness with offset", sName);
			return;
		}
		needEnforceEndian = true;
#endif // GOB_ORIGSAVES
	}

	if ((in = _saveFileMan->openForLoading(sName)))
		iSize = getSaveSize(*in);
	else
		iSize = 0;

	oOff = offset < 0 ? MAX((int32) 0, iSize - (-offset - 1)) : offset;
	oSize = MAX(iSize, oOff + ABS(size));
	oBuf = new char[oSize];
	memset(oBuf, 0, oSize);

	if (in) {
		in->read(oBuf, iSize);
		delete in;
	}

	if(!(out = _saveFileMan->openForSaving(sName))) {
		warning("Can't open file \"%s\" for writing", sName);
		delete[] oBuf;
		return;
	}
	
	if (writePal) {
		memcpy(oBuf + oOff, (char *) _global->_pPaletteDesc->vgaPal, 768);
		oOff += 768;
	}

	if (!needEnforceEndian) {
		if (size < 0) {
			srcDesc = _draw->_spritesArray[index];
			destDesc = _video->initSurfDesc(_global->_videoMode, srcDesc->width, 25, 0);
			for (top = 0, retSize = 0; top < srcDesc->height; top += 25) {
				int16 height = MIN(25, srcDesc->height - top);
				_video->drawSprite(srcDesc, destDesc, 0, top, srcDesc->width - 1,
						top + height - 1, 0, 0, 0);
				memcpy(oBuf + oOff, (char *) destDesc->vidPtr, srcDesc->width * 25);
				oOff += srcDesc->width * 25;
			}
			_video->freeSurfDesc(destDesc);
		} else
			memcpy(oBuf + oOff, buf, size);

		out->write(oBuf, oSize);
	} else
		writeDataEndian(*out, buf, _global->_inter_variablesSizes + dataVar, size);

	out->flush();

	if (out->ioFailed())
		warning("Can't write file \"%s\"", sName);
	else {
		debugC(1, DEBUG_FILEIO, "Saved file \"%s\" (%d, %d bytes at %d)",
				sName, dataVar, size, offset);
		WRITE_VAR(1, 0);
	}

	delete out;
	delete[] oBuf;
}

bool GobEngine::saveGame(int saveSlot, int16 dataVar, int32 size, int32 offset) {
	int32 varSize;
	char *varBuf;
	byte *sizeBuf;
	Common::OutSaveFile *out;

	varBuf = _global->_inter_variables + dataVar;
	sizeBuf = _global->_inter_variablesSizes + dataVar;
	varSize = READ_LE_UINT32(_game->_totFileData + 0x2C) * 4;
	if ((offset == 0) && (size == 600)) {
		memcpy(_saveIndex, varBuf, size);
		memcpy(_saveIndexSizes, sizeBuf, size);
		return true;
	} else if((((offset - 600) % varSize) == 0) && (size == varSize)) {
		if (!(out = _saveFileMan->openForSaving(getSaveSlotFile(saveSlot)))) {
			warning("Can't open file \"%s\" for writing", getSaveSlotFile(saveSlot));
			return false;
		}
		writeDataEndian(*out, _saveIndex + saveSlot * 40, _saveIndexSizes + saveSlot * 40, 40);
		writeDataEndian(*out, varBuf, sizeBuf, size);
		out->flush();
		if (out->ioFailed()) {
			warning("Can't save to slot %d", saveSlot);
			return false;
		}
		debugC(1, DEBUG_FILEIO, "Saved to slot %d", saveSlot);
		delete out;
		return true;
	} else {
		warning("Invalid saving procedure");
		return false;
	}
}

uint32 GobEngine::writeDataEndian(Common::OutSaveFile &out, char *varBuf, byte *sizeBuf,
		int32 size) {

#ifndef GOB_ORIGSAVES

	int i;
	char tmp[4];
	uint32 written;

	written = 0;
	for (i = 0; i < size; i++, varBuf++) {
		if (sizeBuf[i] == 3)
			WRITE_LE_UINT32(tmp, *((uint32 *) varBuf));
		else if (sizeBuf[i] == 1)
			WRITE_LE_UINT16(tmp, *((uint16 *) varBuf));
		else if (sizeBuf[i] == 0)
			*tmp = *varBuf;
		else {
			warning("Can't write data, corrupted variables sizes");
			return 0;
		}
		written += out.write(tmp, sizeBuf[i] + 1);
		varBuf += sizeBuf[i];
		i += sizeBuf[i];
	}

	out.write(sizeBuf, size);

	return written;

#else // GOB_ORIGSAVES

	return out.write(varBuf, size);

#endif // GOB_ORIGSAVES

}

void GobEngine::loadGameData(enum SaveFiles sFile, int16 dataVar, int32 size, int32 offset) {
	int32 sSize;
	int32 retSize;
	int16 index;
	int16 y;
	char *buf;
	char *sName;
	bool readPal;
	bool needEnforceEndian;
	Video::SurfaceDesc *destDesc;
	Video::SurfaceDesc *srcDesc;
	Common::InSaveFile *in;

	index = 0;
	readPal = false;
	sName = _saveFiles[(int) sFile];
	needEnforceEndian = false;

	WRITE_VAR(1, 1);

	if (size < 0) {
		if (size < -1000) {
			readPal = true;
			size += 1000;
		}
		index = -size - 1;
		assert((index >= 0) && (index < 50)); // Just to be sure...
		buf = (char *) _draw->_spritesArray[index]->vidPtr;
		size = _draw->getSpriteRectSize(index);
		if ((_draw->_spritesArray[index]->vidMode & 0x80) == 0)
			size = -size;
	} else {
		int32 varSize;
		varSize = READ_LE_UINT32(_game->_totFileData + 0x2C) * 4;
		if (size == 0) {
			dataVar = 0;
			size = varSize;
		}
		buf = _global->_inter_variables + dataVar;
#ifndef GOB_ORIGSAVES
		if (sFile == SAVE_CAT) {
			if(loadGame((offset - 600) / varSize, dataVar, size, offset))
				WRITE_VAR(1, 0);
			return;
		} else if (offset != 0) {
			warning("Can't read file \"%s\": Can't correctly enfore endianness with offset", sName);
			return;
		}
		needEnforceEndian = true;
#endif // GOB_ORIGSAVES
	}

	if (_global->_inter_resStr[0] == 0) {
		if (readPal)
			size += 768;
		WRITE_VAR(1, size);
		return;
	}

	if(!(in = _saveFileMan->openForLoading(sName))) {
		warning("Can't open file \"%s\" for reading", sName);
		return;
	}

	debugC(1, DEBUG_FILEIO, "Loading file \"%s\" (%d, %d bytes at %d)",
			sName, dataVar, size, offset);

	sSize = getSaveSize(*in);
	_draw->animateCursor(4);
	if (offset < 0)
		in->seek(sSize - (-offset - 1), 0);
	else
		in->seek(offset, 0);

	if (readPal) {
		retSize = in->read((char *) _global->_pPaletteDesc->vgaPal, 768);
		_draw->_applyPal = 1;
	}

	if (!needEnforceEndian) {
		if (size < 0) {
			destDesc = _draw->_spritesArray[index];
			srcDesc = _video->initSurfDesc(_global->_videoMode, destDesc->width, 25, 0);
			for (y = 0, retSize = 0; y < destDesc->height; y += 25) {
				int16 height = MIN(25, destDesc->height - y);
				retSize += in->read((char *) srcDesc->vidPtr, destDesc->width * 25);
				_video->drawSprite(srcDesc, destDesc, 0, 0, destDesc->width - 1, height - 1, 0, y, 0);
			}
			_video->freeSurfDesc(srcDesc);
		} else
			retSize = in->read(buf, size);
		if (index == 21) {
			_video->drawSprite(_draw->_backSurface, _draw->_frontSurface, 0, 0,
					_draw->_frontSurface->width - 1, _draw->_frontSurface->height - 1, 0, 0, 0);
			_video->waitRetrace(_global->_videoMode);
		}
	} else
		retSize = readDataEndian(*in, buf, _global->_inter_variablesSizes + dataVar, size);

	if (retSize == size)
		WRITE_VAR(1, 0);

	delete in;
	return;
}

bool GobEngine::loadGame(int saveSlot, int16 dataVar, int32 size, int32 offset) {
	int i;
	int32 varSize;
	char *varBuf;
	byte *sizeBuf;
	Common::InSaveFile *in;

	varBuf = _global->_inter_variables + dataVar;
	sizeBuf = _global->_inter_variablesSizes + dataVar;
	varSize = READ_LE_UINT32(_game->_totFileData + 0x2C) * 4;
	if ((offset == 0) && (size == 600)) {
		for (i = 0; i < 15; i++, varBuf += 40) {
			if ((in = _saveFileMan->openForLoading(getSaveSlotFile(i)))) {
				in->read(varBuf, 40);
				delete in;
			} else
				memset(varBuf, 0, 40);
		}
		return true;
	} else if((((offset - 600) % varSize) == 0) && (size == varSize)) {
		if (!(in = _saveFileMan->openForLoading(getSaveSlotFile(saveSlot)))) {
			warning("Can't load from slot %d", saveSlot);
			return false;
		}
		if (((getSaveSize(*in) / 2) - 40) != (uint32) varSize) {
			warning("Can't load from slot %d: Wrong size", saveSlot);
			return false;
		}
		in->seek(80);
		readDataEndian(*in, varBuf, sizeBuf, size);
		delete in;
		debugC(1, DEBUG_FILEIO, "Loading from slot %d", saveSlot);
		return true;
	} else {
		warning("Invalid loading procedure");
		return false;
	}
}

uint32 GobEngine::readDataEndian(Common::InSaveFile &in, char *varBuf, byte *sizeBuf,
		int32 size) {

#ifndef GOB_ORIGSAVES

	uint32 read;
	char *vars;
	char *sizes;
	int i;

	vars = new char[size];
	sizes = new char[size];

	read = in.read(vars, size);
	if (in.read(sizes, size) != read) {
		warning("Can't read data: Corrupted variables sizes");
		delete[] vars;
		delete[] sizes;
		return 0;
	}

	for (i = 0; i < size; i++) {
		if (sizes[i] == 3)
			*((uint32 *) (vars + i)) = READ_LE_UINT32(vars + i);
		else if (sizes[i] == 1)
			*((uint16 *) (vars + i)) = READ_LE_UINT16(vars + i);
		else if (sizes[i] != 0) {
			warning("Can't read data: Corrupted variables sizes");
			return 0;
		}
		i += sizes[i];
	}

	memcpy(varBuf, vars, size);
	memcpy(sizeBuf, sizes, size);
	delete[] vars;
	delete[] sizes;

	return read;
	
#else // GOB_ORIGSAVES

	return in.read(varBuf, size);

#endif // GOB_ORIGSAVES
}

int GobEngine::init() {
	_snd = new Snd(this);
	_global = new Global(this);
	_anim = new Anim();
	_cdrom = new CDROM(this);
	_dataio = new DataIO(this);
	_pack = new Pack();
	_palanim = new PalAnim(this);
	_gtimer = new GTimer();
	_util = new Util(this);
	if (_features & Gob::GF_GOB1) {
		_inter = new Inter_v1(this);
		_parse = new Parse_v1(this);
		_mult = new Mult_v1(this);
		_draw = new Draw_v1(this);
		_game = new Game_v1(this);
		_video = new Video_v1(this);
		_init = new Init_v1(this);
		_map = new Map_v1(this);
		_goblin = new Goblin_v1(this);
		_scenery = new Scenery_v1(this);
	}
	else if (_features & Gob::GF_GOB2) {
		_inter = new Inter_v2(this);
		_parse = new Parse_v2(this);
		_mult = new Mult_v2(this);
		_draw = new Draw_v2(this);
		_game = new Game_v2(this);
		_video = new Video_v2(this);
		_init = new Init_v2(this);
		_map = new Map_v2(this);
		_goblin = new Goblin_v2(this);
		_scenery = new Scenery_v2(this);
	}
	else
		error("GobEngine::init(): Unknown version of game engine");
	if ((_features & Gob::GF_MAC) || (_features & Gob::GF_GOB1) || (_features & Gob::GF_GOB2)) {
		if (MidiDriver::parseMusicDriver(ConfMan.get("music_driver")) == MD_NULL)
			_music = new Music_Dummy(this);
		else
			_music = new Music(this);
	}
	_vm = this;

	_system->beginGFXTransaction();
		initCommonGFX(false);
		_system->initSize(320, 200);
	_system->endGFXTransaction();

	// On some systems it's not safe to run CD audio games from the CD.
	if (_features & GF_CD) 
		checkCD();

	int cd_num = ConfMan.getInt("cdrom");
	if (cd_num >= 0)
		_system->openCD(cd_num);

	_global->_debugFlag = 1;
	_global->_doRangeClamp = 1;

	_global->_videoMode = 0x13;
	_global->_useMouse = 1;
	_global->_soundFlags = 0;

	if (ConfMan.hasKey("language"))
		_language = Common::parseLanguage(ConfMan.get("language"));

	switch (_language) {
	case Common::FR_FRA:
	case Common::RU_RUS:
		_global->_language = 0;
		break;
	case Common::DE_DEU:
		_global->_language = 1;
		break;
	case Common::EN_GRB:
		_global->_language = 2;
		break;
	case Common::ES_ESP:
		_global->_language = 3;
		break;
	case Common::IT_ITA:
		_global->_language = 4;
		break;
	case Common::EN_USA:
		_global->_language = 5;
		break;
	case Common::NL_NLD:
		_global->_language = 6;
		break;
	case Common::KO_KOR:
		_global->_language = 7;
		break;
	case Common::HB_ISR:
		_global->_language = 8;
		break;
	default:
		// Default to English
		_global->_language = 2;
		break;
	}

	// FIXME: This is the ugly way of reducing redraw overhead. It works
	//        well for 320x200 but it's unclear how well it will work for
	//        640x480.

	g_system->setFeatureState(OSystem::kFeatureAutoComputeDirtyRects, true);

	return 0;
}

} // End of namespace Gob

using namespace Gob;

GameList Engine_GOB_gameIDList() {
	GameList games;
	const PlainGameDescriptor *g = gob_list;

	while (g->gameid) {
		games.push_back(*g);
		g++;
	}

	return games;
}

GameDescriptor Engine_GOB_findGameID(const char *gameid) {
	const PlainGameDescriptor *g = gob_list;
	while (g->gameid) {
		if (0 == scumm_stricmp(gameid, g->gameid))
			break;
		g++;
	}
	return *g;
}

DetectedGameList Engine_GOB_detectGames(const FSList &fslist) {
	DetectedGameList detectedGames;
	const GameSettings *g;
	FSList::const_iterator file;

	// Iterate over all files in the given directory
	for (file = fslist.begin(); file != fslist.end(); file++) {
		if (file->isDirectory())
			continue;

		// All the supported games have an intro.stk file.
		if (scumm_stricmp(file->name().c_str(), "intro.stk") == 0)
			break;
	}

	if (file == fslist.end())
		return detectedGames;

	uint8 md5sum[16];
	char md5str[32 + 1];

	if (Common::md5_file(*file, md5sum, kMD5FileSizeLimit)) {
		for (int i = 0; i < 16; i++) {
			sprintf(md5str + i * 2, "%02x", (int)md5sum[i]);
		}
		for (g = gob_games; g->gameid; g++) {
			if (strcmp(g->md5sum, (char *)md5str) == 0) {
				detectedGames.push_back(DetectedGame(g->gameid, g->description));
			}
		}
		if (detectedGames.empty()) {
			printf("Unknown MD5 (%s)! Please report the details (language, platform, etc.) of this game to the ScummVM team\n", md5str);

			const PlainGameDescriptor *g1 = gob_list;
			while (g1->gameid) {
				detectedGames.push_back(*g1);
				g1++;
			}
		}
	}
	return detectedGames;
}

PluginError Engine_GOB_create(OSystem *syst, Engine **engine) {
	// Detect game features based on MD5
	uint8 md5sum[16];
	char md5str[32 + 1];

	if (Common::md5_file("intro.stk", md5sum, kMD5FileSizeLimit)) {
		for (int j = 0; j < 16; j++) {
			sprintf(md5str + j*2, "%02x", (int)md5sum[j]);
		}
	} else {
		error("Engine_GOB_create(): Cannot find intro.stk");
	}

	const GameSettings *g;
	bool found = false;

	// TODO
	// Fallback. Maybe we will be able to determine game type from game
	// data contents
	Common::String realGame(ConfMan.get("gameid"));
	uint32 features;
	const char *startTotBase=NULL;

	if (!scumm_stricmp(realGame.c_str(), "gob2"))
		features = GF_GOB2;
	else
		features = GF_GOB1;

	for (g = gob_games; g->gameid; g++) {
		if (strcmp(g->md5sum, (char *)md5str) == 0) {
			features = g->features;

			if (g->description)
				g_system->setWindowCaption(g->description);

			startTotBase = g->startTotBase;
			found = true;
			break;
		}
	}

	if (!found) {
		printf("Unknown MD5 (%s)! Please report the details (language, platform, etc.) of this game to the ScummVM team\n", md5str);
	}

	assert(engine);
	*engine = new GobEngine(syst, features, g->lang, startTotBase);
	return kNoError;
}

REGISTER_PLUGIN(GOB, "Gob Engine", "Goblins Games (C) Coktel Vision");
