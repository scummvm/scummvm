/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
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
#include "common/scaler.h"

class Engine;
class GameDetector;
class MidiDriver;
class OSystem;
class SoundMixer;
class Plugin;

/** Global (shared) game feature flags. */
enum {
//	GF_HAS_SPEECH = 1 << 28,
//	GF_HAS_SUBTITLES = 1 << 29,
	GF_DEFAULT_TO_1X_SCALER = 1 << 30
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
	const char *name;
	const char *description;
	uint32 features;
};

struct GraphicsMode {
	const char *name;
	const char *description;
	int id;
};

/**
 * List of graphic 'modes' we potentially support. Potentially because not all
 * backends actually support all the filters listed here. At this point only
 * the SDL backend supports all (except for the PalmOS ones of course).
 * @todo Remove this explicit list of graphic modes and rather extend the
 * OSystem API to allow querying a backend for the modes it supports.
 */
static const struct GraphicsMode gfx_modes[] = {
	{"normal", "Normal (no scaling)", GFX_NORMAL},
	{"1x", "Normal (no scaling)", GFX_NORMAL},
#ifndef __PALM_OS__     // reduce contant data size
	{"2x", "2x", GFX_DOUBLESIZE},
	{"3x", "3x", GFX_TRIPLESIZE},
	{"2xsai", "2xSAI", GFX_2XSAI},
	{"super2xsai", "Super2xSAI", GFX_SUPER2XSAI},
	{"supereagle", "SuperEagle", GFX_SUPEREAGLE},
	{"advmame2x", "AdvMAME2x", GFX_ADVMAME2X},
	{"advmame3x", "AdvMAME3x", GFX_ADVMAME3X},
	{"hq2x", "HQ2x", GFX_HQ2X},
	{"hq3x", "HQ3x", GFX_HQ3X},
	{"tv2x", "TV2x", GFX_TV2X},
	{"dotmatrix", "DotMatrix", GFX_DOTMATRIX},
#else
	{"flipping", "Page Flipping", GFX_FLIPPING},
	{"buffered", "Buffered", GFX_BUFFERED},
	{"wide", "Wide (HiRes+ only)", GFX_WIDE},
#endif
	{0, 0, 0}
};


class GameDetector {
	typedef Common::String String;

public:
	GameDetector();

	void parseCommandLine(int argc, char **argv);
	bool detectMain();

	String _targetName;
	GameSettings _game;	// TODO: Eventually get rid of this?!
	const Plugin *_plugin;
	
	bool _dumpScripts;

public:
	void setTarget(const String &name);

	Engine *createEngine(OSystem *system);

	static SoundMixer *createMixer();
	static MidiDriver *createMidi(int midiDriver);

	static int parseGraphicsMode(const String &s);	// Used in main()
	static int detectMusicDriver(int midiFlags);

	static GameSettings findGame(const String &gameName, const Plugin **plugin = NULL);

protected:
	bool detectGame(void);
};

#endif
