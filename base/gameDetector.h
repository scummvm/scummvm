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

class Engine;
class GameDetector;
class MidiDriver;
class OSystem;
class SoundMixer;
class Plugin;

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

/**
 * List of language ids.
 * @note The order and mappings of the values 0..8 are *required* to stay the
 * way they are now, as scripts in COMI rely on them. So don't touch them.
 */
enum Language {
	UNK_LANG = -1,	// Use default language (i.e. none specified)
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

enum Platform {
	kPlatformUnknown = -1,
	kPlatformPC = 0,
	kPlatformAmiga = 1,
	kPlatformAtariST = 2,
	kPlatformMacintosh = 3
};

enum MidiDriverType {
	MDT_NONE   = 0,
	MDT_PCSPK  = 1, // MD_PCSPK and MD_PCJR
	MDT_ADLIB  = 2, // MD_ADLIB
	MDT_TOWNS  = 4, // MD_TOWNS
	MDT_NATIVE = 8, // Everything else
	MDT_PREFER_NATIVE = 16
};

struct GameSettings {
	const char *gameName;
	const char *description;
	byte id, version;
	int midi; // MidiDriverType values
	uint32 features;
	const char *detectname;
};

class GameDetector {
	typedef Common::String String;

public:
	GameDetector();

	void parseCommandLine(int argc, char **argv);
	bool detectMain();

	String _targetName;
	GameSettings _game;
	const Plugin *_plugin;
	
	bool _dumpScripts;
	bool _saveconfig;

	int _midi_driver;

public:
	OSystem *createSystem();
	Engine *createEngine(OSystem *system);

	SoundMixer *createMixer();
	MidiDriver *createMidi();
	int getMidiDriverType();	// FIXME: Try to get rid of this, only Sky frontend uses it

	void setTarget(const String &name);

	static int parseGraphicsMode(const String &s);	// Used in main()
	static int parseMusicDriver(const String &s);
	static Language parseLanguage(const String &s);
	static Platform parsePlatform(const String &s);
	
	const GameSettings *findGame(const String &gameName, const Plugin **plugin = NULL) const;

protected:
	bool detectGame(void);
	void list_games();
};

#endif
