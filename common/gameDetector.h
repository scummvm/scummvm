/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
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

#ifndef GAMEDETECTOR_H
#define GAMEDETECTOR_H

#include "common/str.h"

class OSystem;
class MidiDriver;

enum {
	kDefaultMasterVolume = 192,
	kDefaultSFXVolume = 192,
	kDefaultMusicVolume = 192
};

enum GameId {
	GID_TENTACLE = 1,
	GID_MONKEY2 = 2,
	GID_INDY4 = 3,
	GID_MONKEY = 4,
	GID_SAMNMAX = 5,
	GID_MONKEY_EGA = 6,
	GID_LOOM256 = 7,
	GID_ZAK256 = 8,
	GID_INDY3_256 = 9,
	GID_LOOM = 10,
	GID_FT = 11,
	GID_DIG = 12,
	GID_MONKEY_VGA = 13,
	GID_CMI = 14,
	GID_MANIAC = 15,
	GID_ZAK = 16,
	//GID_MANIAC64 = 17,
	//GID_ZAK64 = 18,

	/* Simon the Sorcerer */
	GID_SIMON_FIRST = 20,
	GID_SIMON_LAST = GID_SIMON_FIRST + 8
};

enum GameFeatures {
	GF_NEW_OPCODES = 1,
	GF_AFTER_V6 = 2,
	GF_AFTER_V7 = 4,
	GF_HAS_ROOMTABLE = GF_AFTER_V7,
	GF_USE_KEY = 8,
	GF_NEW_COSTUMES = GF_AFTER_V7,
	GF_USE_ANGLES = GF_AFTER_V7,
	GF_DRAWOBJ_OTHER_ORDER = 16,

	GF_DEFAULT = GF_USE_KEY,

	GF_SMALL_HEADER = 32,
	GF_EXTERNAL_CHARSET = GF_SMALL_HEADER,
	GF_SMALL_NAMES = 64,
	GF_OLD_BUNDLE = 128,
	GF_16COLOR = 256,
	GF_OLD256 = 512,
	GF_AUDIOTRACKS = 1024,
	GF_NO_SCALLING = 2048,
	GF_ADLIB_DEFAULT = 4096,
	GF_AMIGA = 8192,
	GF_HUMONGOUS = 16384,
	GF_AFTER_V8 = 32768
};

struct VersionSettings {
	const char *filename;
	const char *gamename;
	byte id, major, middle, minor;
	uint32 features;
	char *detectname;
};

struct MusicDrivers {
	const char *name;
	const char *description;
	int id;
};

struct GraphicsModes {
	const char *name;
	const char *description;
	int id;
};

extern const VersionSettings version_settings[];


class GameDetector {
	typedef ScummVM::String String;

public:
	static const MusicDrivers *getMusicDrivers();
	static bool isMusicDriverAvailable(int drv);

public:
	GameDetector();
	
	void parseCommandLine(int argc, char **argv);
	int detectMain();
	void setGame(const String &name);
	const String& getGameName(void);
	
	bool _fullScreen;
	byte _gameId;

	bool _use_adlib;

	int _music_volume;
	int _sfx_volume;
	int _master_volume;
	bool _amiga;

	uint16 _talkSpeed;
	uint16 _debugMode;
	uint16 _debugLevel;
	bool _noSubtitles;
	uint16 _bootParam;

	char *_gameDataPath;
	int _gameTempo;
	int _midi_driver;
	String _gameFileName;
	String _gameText;
	String _gameRealName;
	uint32 _features;

	int _gfx_driver;
	int _gfx_mode;
	
	int _cdrom;
	int _save_slot;
	
	bool _saveconfig;

public:
	OSystem *createSystem();
	MidiDriver *createMidi();

	void updateconfig();

protected:
	bool detectGame(void);

	int parseGraphicsMode(const char *s);
	bool parseMusicDriver(const char *s);
	
	void list_games();
};

#endif
