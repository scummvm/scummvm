/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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

#ifndef QUEEN_H
#define QUEEN_H

#include "base/engine.h"

class GameDetector;

namespace Queen {

class Command;
class Display;
class Graphics;
class Input;
class Logic;
class Music;
class Resource;
class Sound;
class Walk;

class QueenEngine : public Engine {
public:

	QueenEngine(GameDetector *detector, OSystem *syst);
	virtual ~QueenEngine();

	Command *command() const { return _command; }
	Display *display() const { return _display; }
	Graphics *graphics() const { return _graphics; }
	Input *input() const { return _input; }
	Logic *logic() const { return _logic; }
	Music *music() const { return _music; }
	Resource *resource() const { return _resource; }
	Sound *sound() const { return _sound; }
	Walk *walk() const { return _walk; }

	Common::RandomSource randomizer;

protected:

	void errorString(const char *buf_input, char *buf_output);

	void go();

	void initialise();

	static void timerHandler(void *ptr);
	void gotTimerTick();

	Command *_command;
	Display *_display;
	Graphics *_graphics;
	Input *_input;
	Logic *_logic;
	Music *_music;
	Resource *_resource;
	Sound *_sound;
	Walk *_walk;
};

} // End of namespace Queen

#endif
