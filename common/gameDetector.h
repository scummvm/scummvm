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

/** Default sound/music volumes.
 * @todo move this to a better place.
 */
enum {
	kDefaultMasterVolume = 192,
	kDefaultSFXVolume = 192,
	kDefaultMusicVolume = 192
};

/** Global (shared) game feature flags. */
enum {
	GF_DEFAULT_TO_1X_SCALER = 1 << 31
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
	SE_SWE = 9,
	EN_GRB = 10,
	HB_HEB = 20
};

struct VersionSettings {
	const char *filename;
	const char *gamename;
	byte id, version;
	enum {
		ADLIB_DONT_CARE = 0,
		ADLIB_PREFERRED = 1,
		ADLIB_ALWAYS    = 2,
		ADLIB_NEVER     = 3
	} adlib;
	uint32 features;
	const char *detectname;
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
	bool _aspectRatio;

	bool _use_adlib;

	int _master_volume;
	int _music_volume;
	int _sfx_volume;
	bool _amiga;
	int _language;

	bool _floppyIntro;

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
	VersionSettings _game;

	int _gfx_mode;
	bool _default_gfx_mode;

	bool _multi_midi;
	bool _native_mt32;
	
	int _cdrom;
	int _save_slot;
	
	bool _saveconfig;
	bool _confirmExit;

public:
	OSystem *createSystem();
	MidiDriver *createMidi();
	int getMidiDriverType();

	int parseGraphicsMode(const char *s);
	void updateconfig();

protected:
	String _gameText;

	bool detectGame(void);
	bool parseMusicDriver(const char *s);
	int parseLanguage(const char *s);
	void list_games();
};

#endif
