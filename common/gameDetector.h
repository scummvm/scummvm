/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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

/* Languages
 * note: values 0->8 are are _needed_ for scripts in comi please don't
 * remove/change fixed numbers from this enum
 */
enum {
	EN_USA = 0,
	DE_DEU = 1,
	FR_FRA = 2,
	IT_ITA = 3,
	PT_BRA = 4,
	ES_ESP = 5,
	JA_JPN = 6,
	ZH_TWN = 7,
	KO_KOR = 8,
	HB_HEB = 20
};

enum GameId {
	GID_SCUMM_FIRST = 1,
	GID_SCUMM_LAST = GID_SCUMM_FIRST + 99,

	// Simon the Sorcerer
	GID_SIMON_FIRST,
	GID_SIMON_LAST = GID_SIMON_FIRST + 99,

	// Beneath a Steel Sky
	GID_SKY_FIRST,
	GID_SKY_LAST = GID_SKY_FIRST + 99
};

// TODO: the GameFeatures really should be moved to scumm/scumm.h, too
// but right now, gameDetector.h still uses GF_ADLIB_DEFAULT, so we can't.
enum GameFeatures {
	// SCUMM features
	GF_NEW_OPCODES         = 1 << 0,
	GF_AFTER_V6            = 1 << 1,
	GF_AFTER_V7            = 1 << 2,
	GF_AFTER_V8            = 1 << 3,
	GF_USE_KEY             = 1 << 4,
	GF_DRAWOBJ_OTHER_ORDER = 1 << 5,
	GF_SMALL_HEADER        = 1 << 6,
	GF_SMALL_NAMES         = 1 << 7,
	GF_OLD_BUNDLE          = 1 << 8,
	GF_16COLOR             = 1 << 9,
	GF_OLD256              = 1 << 10,
	GF_AUDIOTRACKS         = 1 << 11,
	GF_NO_SCALLING         = 1 << 12,
	GF_ADLIB_DEFAULT       = 1 << 13,
	GF_AMIGA               = 1 << 14,
	GF_HUMONGOUS           = 1 << 15,
	GF_NEW_COSTUMES        = 1 << 16,
	GF_DEFAULT_TO_1X_SCALER = 1 << 17,

	GF_HAS_ROOMTABLE       = GF_AFTER_V7,
	GF_USE_ANGLES          = GF_AFTER_V7,
	GF_DEFAULT             = GF_USE_KEY,
	GF_EXTERNAL_CHARSET    = GF_SMALL_HEADER
};

struct VersionSettings {
	const char *filename;
	const char *gamename;
	byte id, major, middle, minor;
	uint32 features;
	char *detectname;
};

struct MusicDriver {
	const char *name;
	const char *description;
	int id;
};

struct GraphicsMode {
	const char *name;
	const char *description;
	int id;
};

struct Language {
	const char *name;
	const char *description;
	int id;
};

extern const VersionSettings *version_settings;


class GameDetector {
	typedef ScummVM::String String;

public:
	static const MusicDriver *getMusicDrivers();
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

	int _master_volume;
	int _music_volume;
	int _sfx_volume;
	bool _amiga;
	int _language;

	uint16 _talkSpeed;
	uint16 _debugMode;
	uint16 _debugLevel;
	bool _dumpScripts;
	bool _noSubtitles;
	uint16 _bootParam;

	char *_gameDataPath;
	int _gameTempo;
	int _midi_driver;
	String _gameFileName;
	String _gameText;
	String _gameRealName;
	uint32 _features;

	int _gfx_mode;
	bool _default_gfx_mode;
	
	int _cdrom;
	int _save_slot;
	
	bool _saveconfig;

public:
	OSystem *createSystem();
	MidiDriver *createMidi();

	int parseGraphicsMode(const char *s);
	void updateconfig();

protected:
	bool detectGame(void);
	bool parseMusicDriver(const char *s);
	int parseLanguage(const char *s);
	void list_games();
};

#endif
