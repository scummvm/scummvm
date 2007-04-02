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
#include "sound/mididrv.h"

#include "gob/gob.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/game.h"
#include "gob/sound.h"
#include "gob/init.h"
#include "gob/inter.h"
#include "gob/draw.h"
#include "gob/cdrom.h"
#include "gob/goblin.h"
#include "gob/map.h"
#include "gob/mult.h"
#include "gob/palanim.h"
#include "gob/parse.h"
#include "gob/scenery.h"
#include "gob/music.h"
#include "gob/imd.h"

// Use the original saves. Just for testing purposes, will be removed later
// The new method is more convenient, and, more importantly, endian-safe
//#define GOB_ORIGSAVES

namespace Gob {

#define MAX_TIME_DELTA 100

const Common::Language GobEngine::_gobToScummVMLang[] = {
	Common::FR_FRA,
	Common::DE_DEU,
	Common::EN_GRB,
	Common::ES_ESP,
	Common::IT_ITA,
	Common::EN_USA,
	Common::NL_NLD,
	Common::KO_KOR,
	Common::HB_ISR
};

GobEngine::GobEngine(OSystem *syst) : Engine(syst) {
	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	_copyProtection = ConfMan.getBool("copy_protection");
	_quitRequested = false;

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

	Common::addSpecialDebugLevel(kDebugFuncOp, "FuncOpcodes", "Script FuncOpcodes debug level");
	Common::addSpecialDebugLevel(kDebugDrawOp, "DrawOpcodes", "Script DrawOpcodes debug level");
	Common::addSpecialDebugLevel(kDebugGobOp, "GoblinOpcodes", "Script GoblinOpcodes debug level");
	Common::addSpecialDebugLevel(kDebugMusic, "Music", "CD, Adlib and Infogrames music debug level");
	Common::addSpecialDebugLevel(kDebugParser, "Parser", "Parser debug level");
	Common::addSpecialDebugLevel(kDebugGameFlow, "Gameflow", "Gameflow debug level");
	Common::addSpecialDebugLevel(kDebugFileIO, "FileIO", "File Input/Output debug level");
	Common::addSpecialDebugLevel(kDebugGraphics, "Graphics", "Graphics debug level");
	Common::addSpecialDebugLevel(kDebugCollisions, "Collisions", "Collisions debug level");
}

GobEngine::~GobEngine() {
	// Stop all mixer streams (except for the permanent ones).
	_vm->_mixer->stopAll();

	delete _snd;
	delete _adlib;
	delete _mult;
	delete _game;
	delete _global;
	delete _cdrom;
	delete _dataIO;
	delete _goblin;
	delete _imdPlayer;
	delete _init;
	delete _inter;
	delete _map;
	delete _palAnim;
	delete _parse;
	delete _scenery;
	delete _draw;
	delete _util;
	delete _video;
	delete[] _startTot;
	delete[] _startTot0;

	for (int i = 0; i < 3; i++)
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
		debugC(1, kDebugFileIO, "Requested save games size: %d", size);
		return size;
	}
#endif // GOB_ORIGSAVES

	if (sFile == SAVE_SAV)
		size = _global->_savedBack == 0 ? -1 : _global->_savedBackSize;
	else if ((in = _saveFileMan->openForLoading(_saveFiles[(int) sFile]))) {
		size = in->size();
		delete in;
	}

	debugC(1, kDebugFileIO, "Requested size of file \"%s\": %d", _saveFiles[(int) sFile], size);

	return size;
}

const char *GobEngine::getSaveSlotFile(int slot) {
	static char *slotBase = _saveSlotFile + strlen(_targetName.c_str()) + 2;

	snprintf(slotBase, 3, "%02d", slot);
	return _saveSlotFile;
}

void GobEngine::saveGameData(enum SaveFiles sFile, int16 dataVar, int32 size, int32 offset) {
	int16 index;
	bool writePal;
	char *sName;
	byte *buf;
	byte *oBuf;
	int32 retSize;
	int32 iSize;
	int32 oSize;
	int32 oOff;
	SurfaceDesc *srcDesc;
	Common::InSaveFile *in;
	Common::OutSaveFile *out;

	retSize = 0;
	index = 0;
	oBuf = 0;
	in = 0;
	writePal = false;
	sName = _saveFiles[(int) sFile];

	WRITE_VAR(1, 1);

	if (sFile == SAVE_SAV) {
		_global->_savedBackSize = -1;
		if (size >= 0) {
			warning("Invalid attempt at saving a sprite");
			return;
		}
		if (size < -1000) {
			size += 1000;
			writePal = true;
			memcpy((char *) _global->_savedPal, (char *) _global->_pPaletteDesc->vgaPal, 768);
		}
		index = -size - 1;
		if ((index < 0) || (index >= 50)) {
			warning("Invalid attempt at saving a sprite");
			return;
		}
		srcDesc = _draw->_spritesArray[index];

		_global->_savedBack =
			_video->initSurfDesc(_vm->_global->_videoMode, srcDesc->getWidth(), srcDesc->getHeight(), 0);
		_vm->_video->drawSprite(srcDesc, _global->_savedBack, 0, 0,
				srcDesc->getWidth() - 1, srcDesc->getHeight() - 1, 0, 0, 0);

		_global->_savedBackSize = _draw->getSpriteRectSize(index);
		if (writePal)
			_global->_savedBackSize += 768;

		WRITE_VAR(1, 0);
		return;
	}

	if (size < 0) {
		warning("Invalid saving procedure");
		return;
	}

	int32 varSize = READ_LE_UINT32(_game->_totFileData + 0x2C) * 4;

	if (size == 0) {
		dataVar = 0;
		size = varSize;
	}
	buf = _global->_inter_variables + dataVar;
#ifndef GOB_ORIGSAVES
	if (sFile == SAVE_CAT) {
		if (saveGame((offset - 600) / varSize, dataVar, size, offset))
			WRITE_VAR(1, 0);
		return;
	} else if (offset != 0) {
		warning("Can't write file \"%s\": Can't correctly enfore endianness with offset", sName);
		return;
	}
#endif // GOB_ORIGSAVES

	if ((in = _saveFileMan->openForLoading(sName)))
		iSize = in->size();
	else
		iSize = 0;

	oOff = offset < 0 ? MAX((int32) 0, iSize - (-offset - 1)) : offset;
	oSize = MAX(iSize, oOff + size);
	oBuf = new byte[oSize];
	memset(oBuf, 0, oSize);

	if (in) {
		in->read(oBuf, iSize);
		delete in;
	}

	if (!(out = _saveFileMan->openForSaving(sName))) {
		warning("Can't open file \"%s\" for writing", sName);
		delete[] oBuf;
		return;
	}
	
	retSize = writeDataEndian(*out, buf, _global->_inter_variablesSizes + dataVar, size);

	out->finalize();

	if (out->ioFailed() || (retSize != size))
		warning("Can't write file \"%s\"", sName);
	else {
		debugC(1, kDebugFileIO, "Saved file \"%s\" (%d, %d bytes at %d)",
				sName, dataVar, size, offset);
		WRITE_VAR(1, 0);
	}

	delete out;
	delete[] oBuf;
}

bool GobEngine::saveGame(int saveSlot, int16 dataVar, int32 size, int32 offset) {
	int32 varSize;
	byte *varBuf;
	byte *sizeBuf;
	Common::OutSaveFile *out;

	varBuf = _global->_inter_variables + dataVar;
	sizeBuf = _global->_inter_variablesSizes + dataVar;
	varSize = READ_LE_UINT32(_game->_totFileData + 0x2C) * 4;
	if ((offset == 0) && (size == 600)) {
		memcpy(_saveIndex, varBuf, size);
		memcpy(_saveIndexSizes, sizeBuf, size);
		return true;
	} else if ((((offset - 600) % varSize) == 0) && (size == varSize)) {
		if (!(out = _saveFileMan->openForSaving(getSaveSlotFile(saveSlot)))) {
			warning("Can't open file \"%s\" for writing", getSaveSlotFile(saveSlot));
			return false;
		}
		writeDataEndian(*out, _saveIndex + saveSlot * 40, _saveIndexSizes + saveSlot * 40, 40);
		writeDataEndian(*out, varBuf, sizeBuf, size);
		out->finalize();
		if (out->ioFailed()) {
			warning("Can't save to slot %d", saveSlot);
			return false;
		}
		debugC(1, kDebugFileIO, "Saved to slot %d", saveSlot);
		delete out;
		return true;
	} else {
		warning("Invalid saving procedure");
		return false;
	}
}

uint32 GobEngine::writeDataEndian(Common::OutSaveFile &out, byte *varBuf, byte *sizeBuf,
		int32 size) {

#ifndef GOB_ORIGSAVES

	int i;
	byte tmp[4];
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
	byte *buf;
	char *sName;
	bool readPal;
	SurfaceDesc *destDesc;
	Common::InSaveFile *in;

	index = 0;
	readPal = false;
	sName = _saveFiles[(int) sFile];

	WRITE_VAR(1, 1);
	
	if (sFile == SAVE_SAV) {
		if (size >= 0) {
			warning("Invalid attempt at loading a sprite");
			return;
		}
		if (size < -1000) {
			size += 1000;
			readPal = true;
			memcpy((char *) _global->_pPaletteDesc->vgaPal, (char *) _global->_savedPal, 768);
		}
		index = -size - 1;
		if ((index < 0) || (index >= 50)) {
			warning("Invalid attempt at loading a sprite");
			return;
		}
		destDesc = _draw->_spritesArray[index];

		if ((destDesc->getWidth() != _global->_savedBack->getWidth()) ||
		    (destDesc->getHeight() != _global->_savedBack->getHeight())) {
			warning("Resolution doesn't match while loading a sprite");
			return;
		}

		_vm->_video->drawSprite(_global->_savedBack, destDesc, 0, 0,
				destDesc->getWidth() - 1, destDesc->getHeight() - 1, 0, 0, 0);
		if (index == 21) {
			_vm->_draw->forceBlit();
			_video->waitRetrace();
		}

		WRITE_VAR(1, 0);
		return;
	}

	if (size < 0) {
		warning("Invalid loading procedure");
		return;
	}

	int32 varSize;
	varSize = READ_LE_UINT32(_game->_totFileData + 0x2C) * 4;
	if (size == 0) {
		dataVar = 0;
		size = varSize;
	}
	buf = _global->_inter_variables + dataVar;
#ifndef GOB_ORIGSAVES
	if (sFile == SAVE_CAT) {
		if (loadGame((offset - 600) / varSize, dataVar, size, offset))
			WRITE_VAR(1, 0);
		return;
	} else if (offset != 0) {
		warning("Can't read file \"%s\": Can't correctly enfore endianness with offset", sName);
		return;
	}
#endif // GOB_ORIGSAVES

	if (_global->_inter_resStr[0] == 0) {
		WRITE_VAR(1, size);
		return;
	}

	if (!(in = _saveFileMan->openForLoading(sName))) {
		warning("Can't open file \"%s\" for reading", sName);
		return;
	}

	debugC(1, kDebugFileIO, "Loading file \"%s\" (%d, %d bytes at %d)",
			sName, dataVar, size, offset);

	sSize = in->size();
	_draw->animateCursor(4);
	if (offset < 0)
		in->seek(sSize - (-offset - 1), 0);
	else
		in->seek(offset, 0);

	retSize = readDataEndian(*in, buf, _global->_inter_variablesSizes + dataVar, size);

	if (retSize == size)
		WRITE_VAR(1, 0);

	delete in;
	return;
}

bool GobEngine::loadGame(int saveSlot, int16 dataVar, int32 size, int32 offset) {
	int i;
	int32 varSize;
	byte *varBuf;
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
	} else if ((((offset - 600) % varSize) == 0) && (size == varSize)) {
		if (!(in = _saveFileMan->openForLoading(getSaveSlotFile(saveSlot)))) {
			warning("Can't load from slot %d", saveSlot);
			return false;
		}
		if (((in->size() / 2) - 40) != (uint32) varSize) {
			warning("Can't load from slot %d: Wrong size", saveSlot);
			return false;
		}
		in->seek(80);
		readDataEndian(*in, varBuf, sizeBuf, size);
		delete in;
		debugC(1, kDebugFileIO, "Loading from slot %d", saveSlot);
		return true;
	} else {
		warning("Invalid loading procedure");
		return false;
	}
}

uint32 GobEngine::readDataEndian(Common::InSaveFile &in, byte *varBuf, byte *sizeBuf,
		int32 size) {

#ifndef GOB_ORIGSAVES

	uint32 read;
	byte *vars;
	byte *sizes;
	int i;

	vars = new byte[size];
	sizes = new byte[size];

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

void GobEngine::validateLanguage() {
	if (_vm->_global->_languageWanted != _vm->_global->_language) {
		warning("Your game version doesn't support the requested language");
		warning("Using the first language available: %s",
				getLangDesc(_vm->_global->_language));
		_vm->_global->_languageWanted = _vm->_global->_language;
	}
}

void GobEngine::validateVideoMode(int16 videoMode) {
	if ((videoMode != 0x13) && (videoMode != 0x14))
		error("Video mode 0x%X is not supported!", videoMode);
}

int GobEngine::init() {
	// Detect game
	if (!detectGame()) {
		GUIErrorMessage("No valid games were found in the specified directory.");
		return -1;
	}

	_adlib = 0;
	_global = new Global(this);
	_util = new Util(this);
	_dataIO = new DataIO(this);
	_palAnim = new PalAnim(this);
	_imdPlayer = new ImdPlayer(this);
	_cdrom = new CDROM(this);
	_snd = new Snd(this);
	if (_features & Gob::GF_GOB1) {
		_init = new Init_v1(this);
		_video = new Video_v1(this);
		_inter = new Inter_v1(this);
		_parse = new Parse_v1(this);
		_mult = new Mult_v1(this);
		_draw = new Draw_v1(this);
		_game = new Game_v1(this);
		_map = new Map_v1(this);
		_goblin = new Goblin_v1(this);
		_scenery = new Scenery_v1(this);
	} else if (_features & Gob::GF_GOB2) {
		_init = new Init_v2(this);
		_video = new Video_v2(this);
		_inter = new Inter_v2(this);
		_parse = new Parse_v2(this);
		_mult = new Mult_v2(this);
		_draw = new Draw_v2(this);
		_game = new Game_v2(this);
		_map = new Map_v2(this);
		_goblin = new Goblin_v2(this);
		_scenery = new Scenery_v2(this);
	} else if (_features & Gob::GF_BARGON) {
		_init = new Init_v2(this);
		_video = new Video_v2(this);
		_inter = new Inter_Bargon(this);
		_parse = new Parse_v2(this);
		_mult = new Mult_v2(this);
		_draw = new Draw_Bargon(this);
		_game = new Game_v2(this);
		_map = new Map_v2(this);
		_goblin = new Goblin_v2(this);
		_scenery = new Scenery_v2(this);
	} else
		error("GobEngine::init(): Unknown version of game engine");

	_noMusic = MidiDriver::parseMusicDriver(ConfMan.get("music_driver")) == MD_NULL;
	if (!_noMusic && !(_platform == Common::kPlatformAmiga) &&
		 !(_platform == Common::kPlatformAtariST) &&
	   (((_platform == Common::kPlatformMacintosh) && (_features & Gob::GF_GOB1)) ||
	     (_features & Gob::GF_GOB2)))
		_adlib = new Adlib(this);
	_vm = this;

	_map->init();

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
	_video->_doRangeClamp = true;

	// WORKAROUND: Some versions check the video mode to detect the system
	if (_platform == Common::kPlatformAmiga)
		_global->_fakeVideoMode = 0x11;
	else if (_platform == Common::kPlatformAtariST)
		_global->_fakeVideoMode = 0x10;
	else
		_global->_fakeVideoMode = 0x13;

	_global->_videoMode = 0x13;
	_global->_useMouse = 1;
	_global->_soundFlags = MIDI_FLAG | SPEAKER_FLAG | BLASTER_FLAG | ADLIB_FLAG;

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
	case Common::EN_ANY:
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
	_global->_languageWanted = _global->_language;

	// FIXME: This is the ugly way of reducing redraw overhead. It works
	//        well for 320x200 but it's unclear how well it will work for
	//        640x480.

	g_system->setFeatureState(OSystem::kFeatureAutoComputeDirtyRects, true);
	return 0;
}

} // End of namespace Gob
