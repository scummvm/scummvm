/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef ENGINE_H
#define ENGINE_H

#include "scummsys.h"
#include "system.h"

class SoundMixer;
class GameDetector;

/* FIXME - BIG HACK for MidiEmu */
extern OSystem *g_system;
extern SoundMixer *g_mixer;

class Engine {
public:
	OSystem *_system;

	SoundMixer *_mixer;

	Engine(GameDetector *detector, OSystem *syst);
	virtual ~Engine();

	virtual void go() = 0;
};


#endif
