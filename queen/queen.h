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

class Graphics;
class Input;
class Resource;
class Logic;
class Display;
class Sound;

class QueenEngine : public Engine {
public:

	QueenEngine(GameDetector *detector, OSystem *syst);
	virtual ~QueenEngine();

protected:

	void errorString(const char *buf_input, char *buf_output);

	//! Called when we go from one room to another
	void roomChanged(); // SETUP_ROOM

	void go();

	void initialise();

	static void timerHandler(void *ptr);
	void gotTimerTick();

	
	Graphics *_graphics;
	Input *_input;
	Resource *_resource;
	Logic *_logic;
	Display *_display;
	Sound *_sound;
	
	const char *_detectname; // necessary for music
};

} // End of namespace Queen

#endif
