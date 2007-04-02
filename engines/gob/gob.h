/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
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

#ifndef GOB_GOB_H
#define GOB_GOB_H

#include "common/stdafx.h"
#include "common/system.h"
#include "common/savefile.h"

#include "engines/engine.h"

namespace Gob {

class Game;
class Snd;
class Video;
class Global;
class Draw;
class CDROM;
class DataIO;
class Goblin;
class ImdPlayer;
class Init;
class Inter;
class Map;
class Mult;
class PalAnim;
class Parse;
class Scenery;
class Util;
class Adlib;

#define VARP(offs)			(_vm->_global->_inter_variables + (offs))
#define WRITE_VARO_UINT32(offs, val)	_vm->_global->writeVar(offs, (uint32) (val))
#define WRITE_VARO_UINT16(offs, val)	_vm->_global->writeVar(offs, (uint16) (val))
#define WRITE_VARO_UINT8(offs, val)	_vm->_global->writeVar(offs, (uint8) (val))
#define WRITE_VARO_STR(offs, str)	_vm->_global->writeVar(offs, (const char *) (str))
#define WRITE_VAR_UINT32(var, val)	WRITE_VARO_UINT32((var) << 2, (val))
#define WRITE_VAR_UINT16(var, val)	WRITE_VARO_UINT16((var) << 2, (val))
#define WRITE_VAR_UINT8(var, val)	WRITE_VARO_UINT8((var) << 2, (val))
#define WRITE_VAR_STR(var, str)		WRITE_VARO_STR((var) << 2, (str))
#define READ_VARO_UINT32(offs)		(*((uint32 *) VARP(offs)))
#define READ_VARO_UINT16(offs)		(*((uint16 *) VARP(offs)))
#define READ_VARO_UINT8(offs)		(*((uint8 *) VARP(offs)))
#define READ_VAR_UINT32(var)		READ_VARO_UINT32((var) << 2)
#define READ_VAR_UINT16(var)		READ_VARO_UINT16((var) << 2)
#define READ_VAR_UINT8(var)		READ_VARO_UINT8((var) << 2)
#define GET_VARO_STR(offs)		((char *) VARP(offs))
#define GET_VAR_STR(var)		GET_VARO_STR((var) << 2)

#define WRITE_VAR_OFFSET(offs, val)	WRITE_VARO_UINT32((offs), (val))
#define WRITE_VAR(var, val)		WRITE_VAR_UINT32((var), (val))
#define VAR_OFFSET(offs)		READ_VARO_UINT32(offs)
#define VAR(var)			READ_VAR_UINT32(var)
#define VAR_ADDRESS(var)		((uint32 *) VARP((var) << 2))

enum {
	GF_GOB1 = 1 << 0,
	GF_GOB2 = 1 << 1,
	GF_GOB3 = 1 << 2,
	GF_WOODRUFF = 1 << 3,
	GF_BARGON = 1 << 4,
	GF_CD = 1 << 5,
	GF_EGA = 1 << 6
};

enum {
	kDebugFuncOp = 1 << 0,
	kDebugDrawOp = 1 << 1,
	kDebugGobOp = 1 << 2,
	kDebugMusic = 1 << 3,     // CD, Adlib and Infogrames music
	kDebugParser = 1 << 4,
	kDebugGameFlow = 1 << 5,
	kDebugFileIO = 1 << 6,
	kDebugGraphics = 1 << 7,
	kDebugCollisions = 1 << 8
};

enum SaveFiles {
	SAVE_CAT = 0, // Saves
	SAVE_SAV,     // Holds a sprite, normally a cache for Draw::_backBuffer
	              // (see Global::_savedBack)
	SAVE_BLO      // Notes
};

inline char *strncpy0(char *dest, const char *src, size_t n) {
	strncpy(dest, src, n);
	dest[n] = 0;
	return dest;
}

// A "smart" reference counting templated class
template<typename T>
class ReferenceCounter {
public:
	class Ptr {
	public:
		bool operator==(const Ptr &p) const { return _p == p._p; }
		bool operator==(const ReferenceCounter *p) const { return _p == p; }

		T *operator-> () { return _p; }
		T &operator* () { return *_p; }
		operator T*() { return _p; }

		Ptr(T *p) : _p(p) { ++_p->_references; }
		Ptr() : _p(0) { }

		~Ptr() {
			if (_p && (--_p->_references == 0))
				delete _p;
		}

		Ptr(const Ptr &p) : _p(p._p) { ++_p->_references; }

		Ptr &operator= (const Ptr &p) {
			++p._p->_references;
			if (_p && (--_p->_references == 0))
				delete _p;
			_p = p._p;
			return *this;
		}
		Ptr *operator= (const Ptr *p) {
			if (p)
				++p->_p->_references;
			if (_p && (--_p->_references == 0))
				delete _p;

			_p = p ? p->_p : 0;
			return this;
		}

	private:
		T *_p;
	};

	ReferenceCounter() : _references(0) { }
	virtual ~ReferenceCounter() {}

private:
	unsigned _references;
	friend class Ptr;
};

class GobEngine : public Engine {
protected:
	char **_saveFiles;
	char *_saveSlotFile;
	byte _saveIndex[600];
	byte _saveIndexSizes[600];
	GobEngine *_vm;

	int go();
	int init();

	const char *getSaveSlotFile(int slot);
	bool saveGame(int saveSlot, int16 dataVar, int32 size, int32 offset);
	bool loadGame(int saveSlot, int16 dataVar, int32 size, int32 offset);
	uint32 writeDataEndian(Common::OutSaveFile &out, byte *varBuf, byte *sizeBuf, int32 size);
	uint32 readDataEndian(Common::InSaveFile &in, byte *varBuf, byte *sizeBuf, int32 size);

	bool detectGame();

public:
	static const Common::Language _gobToScummVMLang[];

	Common::RandomSource _rnd;

	int32 _features;
	Common::Language _language;
	Common::Platform _platform;
	char *_startTot;
	char *_startTot0;
	bool _copyProtection;
	bool _noMusic;
	bool _quitRequested;

	Global *_global;
	Util *_util;
	DataIO *_dataIO;
	Game *_game;
	Snd *_snd;
	Video *_video;
	Draw *_draw;
	CDROM *_cdrom;
	Goblin *_goblin;
	Init *_init;
	Map *_map;
	Mult *_mult;
	PalAnim *_palAnim;
	Parse *_parse;
	Scenery *_scenery;
	Inter *_inter;
	Adlib *_adlib;
	ImdPlayer *_imdPlayer;

	void shutdown();

	int32 getSaveSize(enum SaveFiles sFile);
	void saveGameData(enum SaveFiles sFile, int16 dataVar, int32 size, int32 offset);
	void loadGameData(enum SaveFiles sFile, int16 dataVar, int32 size, int32 offset);
	const char *getLangDesc(int16 language) {
		if ((language < 0) || (language > 8))
			language = 2;
		return Common::getLanguageDescription(_gobToScummVMLang[language]);
	}
	void validateLanguage();
	void validateVideoMode(int16 videoMode);

	GobEngine(OSystem *syst);
	virtual ~GobEngine();
};

} // End of namespace Gob

#endif // GOB_GOB_H
