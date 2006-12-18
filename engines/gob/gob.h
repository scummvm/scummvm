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
class Anim;
class CDROM;
class DataIO;
class Goblin;
class Init;
class Inter;
class Map;
class Mult;
class Pack;
class PalAnim;
class Parse;
class Scenery;
class GTimer;
class Util;
class Music;

/*
#define	VAR_OFFSET(offs)		(*(uint32 *)(_vm->_global->_inter_variables + (offs)))
#define	VAR(var)			VAR_OFFSET((var) << 2)
#define VAR_ADDRESS(var)		(&VAR(var))

#define	WRITE_VAR_OFFSET(offs, val)	(VAR_OFFSET(offs) = (val))
#define WRITE_VAR(var, val)		WRITE_VAR_OFFSET((var) << 2, (val))
*/

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
	GF_CD = 1 << 4,
	GF_MAC = 1 << 5,
	GF_EGA = 1 << 6
};

enum {
	DEBUG_FUNCOP = 1 << 0,
	DEBUG_DRAWOP = 1 << 1,
	DEBUG_GOBOP = 1 << 2,
	DEBUG_MUSIC = 1 << 3,     // CD and adlib music
	DEBUG_PARSER = 1 << 4,
	DEBUG_GAMEFLOW = 1 << 5,
	DEBUG_FILEIO = 1 << 6,
	DEBUG_GRAPHICS = 1 << 7,
	DEBUG_COLLISIONS = 1 << 8
};

enum SaveFiles {
	SAVE_CAT = 0, // Saves
	SAVE_SAV,     // Draw::_backSurface (as a temporary buffer when using the notepad
	              // and changing the font. TODO: That probably should be worked around
	SAVE_BLO      // Notes
};

class GobEngine : public Engine {
protected:
	char **_saveFiles;
	char *_saveSlotFile;
	char _saveIndex[600];
	byte _saveIndexSizes[600];

	int go();
	int init();

	inline uint32 getSaveSize(Common::InSaveFile &in);
	const char *getSaveSlotFile(int slot);
	bool saveGame(int saveSlot, int16 dataVar, int32 size, int32 offset);
	bool loadGame(int saveSlot, int16 dataVar, int32 size, int32 offset);
	uint32 writeDataEndian(Common::OutSaveFile &out, char *varBuf, byte *sizeBuf, int32 size);
	uint32 readDataEndian(Common::InSaveFile &in, char *varBuf, byte *sizeBuf, int32 size);

public:
	GobEngine(OSystem *syst, uint32 features, Common::Language lang, const char *startTotBase);
	virtual ~GobEngine();

	void shutdown();

	Common::RandomSource _rnd;

	int32 _features;
	Common::Language _language;
	char *_startTot;
	char *_startTot0;
	bool _copyProtection;
	bool _quitRequested;

	Game *_game;
	Snd *_snd;
	Video *_video;
	Global *_global;
	Draw *_draw;
	Anim *_anim;
	CDROM *_cdrom;
	DataIO *_dataio;
	Goblin *_goblin;
	Init *_init;
	Map *_map;
	Mult *_mult;
	Pack *_pack;
	PalAnim *_palanim;
	Parse *_parse;
	Scenery *_scenery;
	GTimer *_gtimer;
	Util *_util;
	Inter *_inter;
	Music *_music;
	GobEngine *_vm;

	void writeVarDebug(uint32 offs, uint32 v);

	int32 getSaveSize(enum SaveFiles sFile);
	void saveGameData(enum SaveFiles sFile, int16 dataVar, int32 size, int32 offset);
	void loadGameData(enum SaveFiles sFile, int16 dataVar, int32 size, int32 offset);
};

} // End of namespace Gob
#endif
