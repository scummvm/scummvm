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

/** Global (shared) game feature flags. */
enum {
	GF_DEFAULT_TO_1X_SCALER = 1 << 31
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

public:
	void setTarget(const String &name);

	Engine *createEngine(OSystem *system);

	static OSystem *createSystem();
	static SoundMixer *createMixer();
	static MidiDriver *createMidi(int midiDriver);

	static int parseGraphicsMode(const String &s);	// Used in main()
	static int detectMusicDriver(int midiFlags);
	
	static GameSettings findGame(const String &gameName, const Plugin **plugin = NULL);

protected:
	bool detectGame(void);
};

#endif
