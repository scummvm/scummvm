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

#include <stdio.h>
#include "base/engine.h"
#include "common/util.h"
#include "common/timer.h"
#include "sound/mixer.h"
#include "queen/resource.h"
#include "queen/logic.h"
#include "common/config-file.h"

namespace Queen {

class Logic;

class QueenEngine : public Engine {
	void errorString(const char *buf_input, char *buf_output);
protected:
	byte _game;
	byte _key_pressed;
	bool _quickLaunch; // set when starting with -x

	uint16 _debugMode;
	int _numScreenUpdates;

	int _number_of_savegames;
	int _sdl_mouse_x, _sdl_mouse_y;

	FILE *_dump_file;
	
	Resource *_resource;
	Logic *_logic;

	GameDetector *_detector; // necessary for music
	
public:
	QueenEngine(GameDetector *detector, OSystem *syst);
	virtual ~QueenEngine();

protected:
	byte _fastMode;

	void delay(uint amount);
	void go();

	void initialise();

	static int CDECL game_thread_proc(void *param);
};

} // End of namespace Queen

#endif
