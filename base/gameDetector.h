/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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

#ifndef GAMEDETECTOR_H
#define GAMEDETECTOR_H

#include "common/str.h"

class Engine;
class GameDetector;
class OSystem;
class Plugin;
namespace Audio {
	class Mixer;
}

struct GameSettings {
	const char *gameid;
	const char *description;	// TODO: Rename this to "title" or so
};

/**
 * This template function allows to easily convert structs that mimic GameSettings
 * to a GameSettings instance.
 *
 * Normally, one would just subclass GameSettings to get this effect much easier.
 * However, subclassing a struct turns it into a non-POD type. One of the
 * consequences is that you can't have inline intialized arrays of that type.
 * But we heavily rely on those, hence we can't subclass GameSettings...
 */
template <class T>
GameSettings toGameSettings(const T &g) {
	GameSettings dummy = { g.gameid, g.description };
	return dummy;
}


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

	bool _force1xOverlay;

	void setTarget(const String &name);

public:
	Engine *createEngine(OSystem *system);

	static Audio::Mixer *createMixer();

	static GameSettings findGame(const String &gameName, const Plugin **plugin = NULL);

protected:
	bool detectGame(void);
};

#endif
