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

#ifndef GOB_GOB_H
#define GOB_GOB_H

#include "common/system.h"
#include "common/savefile.h"

#include "gui/dialog.h"

#include "engines/engine.h"

namespace GUI {
	class StaticTextWidget;
}

namespace Gob {

class Game;
class Sound;
class Video;
class Global;
class Draw;
class DataIO;
class Goblin;
class VideoPlayer;
class Init;
class Inter;
class Map;
class Mult;
class PalAnim;
class Scenery;
class Util;
class SaveLoad;

#define WRITE_VAR_UINT32(var, val)  _vm->_inter->_variables->writeVar32(var, val)
#define WRITE_VAR_UINT16(var, val)  _vm->_inter->_variables->writeVar16(var, val)
#define WRITE_VAR_UINT8(var, val)   _vm->_inter->_variables->writeVar8(var, val)
#define WRITE_VAR_STR(var, str)     _vm->_inter->_variables->writeVarString(var, str)
#define WRITE_VARO_UINT32(off, val) _vm->_inter->_variables->writeOff32(off, val)
#define WRITE_VARO_UINT16(off, val) _vm->_inter->_variables->writeOff16(off, val)
#define WRITE_VARO_UINT8(off, val)  _vm->_inter->_variables->writeOff8(off, val)
#define WRITE_VARO_STR(off, str)    _vm->_inter->_variables->writeOffString(off, str)
#define READ_VAR_UINT32(var)        _vm->_inter->_variables->readVar32(var)
#define READ_VAR_UINT16(var)        _vm->_inter->_variables->readVar16(var)
#define READ_VAR_UINT8(var)         _vm->_inter->_variables->readVar8(var)
#define READ_VARO_UINT32(off)       _vm->_inter->_variables->readOff32(off)
#define READ_VARO_UINT16(off)       _vm->_inter->_variables->readOff16(off)
#define READ_VARO_UINT8(off)        _vm->_inter->_variables->readOff8(off)
#define GET_VAR_STR(var)            _vm->_inter->_variables->getAddressVarString(var)
#define GET_VARO_STR(off)           _vm->_inter->_variables->getAddressOffString(off)
#define GET_VAR_FSTR(var)           _vm->_inter->_variables->getAddressVarString(var)
#define GET_VARO_FSTR(off)          _vm->_inter->_variables->getAddressOffString(off)

#define VAR_ADDRESS(var)            _vm->_inter->_variables->getAddressVar32(var)

#define WRITE_VAR_OFFSET(off, val)  WRITE_VARO_UINT32((off), (val))
#define WRITE_VAR(var, val)         WRITE_VAR_UINT32((var), (val))
#define VAR_OFFSET(off)             READ_VARO_UINT32(off)
#define VAR(var)                    READ_VAR_UINT32(var)


// WARNING: Reordering these will invalidate save games!
enum Endianness {
	kEndiannessLE,
	kEndiannessBE
};

// WARNING: Reordering these will invalidate save games!
//          Add new games to the bottom of the list.
enum GameType {
	kGameTypeNone = 0,
	kGameTypeGob1,
	kGameTypeGob2,
	kGameTypeGob3,
	kGameTypeWoodruff,
	kGameTypeBargon,
	kGameTypeWeen,
	kGameTypeLostInTime,
	kGameTypeInca2,
	kGameTypeDynasty,
	kGameTypeUrban,
	kGameTypePlaytoon,
	kGameTypePlaytnCk,
	kGameTypeBambou,
	kGameTypeFascination,
	kGameTypeGeisha,
	kGameTypeMagicStones,
	kGameTypeAdibou4,
	kGameTypeAdibouUnknown
};

enum Features {
	kFeaturesNone    =      0,
	kFeaturesCD      = 1 << 0,
	kFeaturesEGA     = 1 << 1,
	kFeaturesAdlib   = 1 << 2,
	kFeatures640     = 1 << 3,
	kFeaturesSCNDemo = 1 << 4,
	kFeaturesBATDemo = 1 << 5,
	kFeatures800     = 1 << 6
};

enum {
	kDebugFuncOp     = 1 <<  0,
	kDebugDrawOp     = 1 <<  1,
	kDebugGobOp      = 1 <<  2,
	kDebugSound      = 1 <<  3,
	kDebugExpression = 1 <<  4,
	kDebugGameFlow   = 1 <<  5,
	kDebugFileIO     = 1 <<  6,
	kDebugSaveLoad   = 1 <<  7,
	kDebugGraphics   = 1 <<  8,
	kDebugVideo      = 1 <<  9,
	kDebugHotspots   = 1 << 10,
	kDebugDemo       = 1 << 11
};

struct GOBGameDescription;

class PauseDialog : public GUI::Dialog {
public:
	PauseDialog();

  virtual void reflowLayout();
	virtual void handleKeyDown(Common::KeyState state);

private:
	Common::String _message;
	GUI::StaticTextWidget *_text;
};

class GobEngine : public Engine {
private:
	GameType _gameType;
	int32 _features;
	Common::Platform _platform;

	uint32 _pauseStart;

	// Engine APIs
	virtual Common::Error run();
	virtual bool hasFeature(EngineFeature f) const;
	virtual void pauseEngineIntern(bool pause);

	bool initGameParts();
	void deinitGameParts();

public:
	static const Common::Language _gobToScummVMLang[];

	Common::RandomSource _rnd;

	Common::Language _language;
	uint16 _width;
	uint16 _height;
	uint8 _mode;

	Common::String _startStk;
	Common::String _startTot;
	uint32 _demoIndex;

	bool _copyProtection;
	bool _noMusic;

	Global *_global;
	Util *_util;
	DataIO *_dataIO;
	Game *_game;
	Sound *_sound;
	Video *_video;
	Draw *_draw;
	Goblin *_goblin;
	Init *_init;
	Map *_map;
	Mult *_mult;
	PalAnim *_palAnim;
	Scenery *_scenery;
	Inter *_inter;
	SaveLoad *_saveLoad;
	VideoPlayer *_vidPlayer;

	const char *getLangDesc(int16 language) const;
	void validateLanguage();
	void validateVideoMode(int16 videoMode);

	void pauseGame();

	Endianness getEndianness() const;
	Common::Platform getPlatform() const;
	GameType getGameType() const;
	bool isCD() const;
	bool isEGA() const;
	bool is640() const;
	bool hasAdlib() const;
	bool isSCNDemo() const;
	bool isBATDemo() const;
	bool is800() const;
	bool isDemo() const;

	GobEngine(OSystem *syst);
	virtual ~GobEngine();

	void initGame(const GOBGameDescription *gd);
};

} // End of namespace Gob

#endif // GOB_GOB_H
