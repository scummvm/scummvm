/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
class VideoPlayer;
class Init;
class Inter;
class Map;
class Mult;
class PalAnim;
class Parse;
class Scenery;
class Util;
class SaveLoad;
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

enum GameType {
	kGameTypeNone = 0,
	kGameTypeGob1,
	kGameTypeGob2,
	kGameTypeGob3,
	kGameTypeWoodruff,
	kGameTypeBargon,
	kGameTypeWeen,
	kGameTypeLostInTime
};

enum Features {
	kFeaturesNone = 0,
	kFeaturesCD = 1 << 0,
	kFeaturesEGA = 1 << 1,
	kFeaturesAdlib = 1 << 2,
	kFeatures640 = 1 << 3
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
	GobEngine *_vm;

	int go();
	int init();

	bool initGameParts();
	void deinitGameParts();

	bool detectGame();

public:
	static const Common::Language _gobToScummVMLang[];

	Common::RandomSource _rnd;

	GameType _gameType;
	int32 _features;
	Common::Language _language;
	Common::Platform _platform;

	uint16 _width;
	uint16 _height;
	uint8 _mode;

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
	SaveLoad *_saveLoad;
	Adlib *_adlib;
	VideoPlayer *_vidPlayer;

	void shutdown();

	const char *getLangDesc(int16 language) {
		if ((language < 0) || (language > 8))
			language = 2;
		return Common::getLanguageDescription(_gobToScummVMLang[language]);
	}
	void validateLanguage();
	void validateVideoMode(int16 videoMode);

	GameType getGameType() { return _gameType; }
	bool isCD() { return (_features & kFeaturesCD) != 0; }
	bool isEGA() { return (_features & kFeaturesEGA) != 0; }
	bool is640() { return (_features & kFeatures640) != 0; }
	bool hasAdlib() { return (_features & kFeaturesAdlib) != 0; }

	GobEngine(OSystem *syst);
	virtual ~GobEngine();
};

} // End of namespace Gob

#endif // GOB_GOB_H
