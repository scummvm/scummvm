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

#include "engine.h"
#include "sound/mixer.h"
#include "gameDetector.h"
#include "scumm.h"
#include "simon/simon.h"

/* FIXME - BIG HACK for MidiEmu */
OSystem *g_system = 0;
SoundMixer *g_mixer = 0;

Engine::Engine(GameDetector *detector, OSystem *syst)
	: _system(syst)
{
	_mixer = new SoundMixer();

	/* FIXME - BIG HACK for MidiEmu */
	g_system = _system;
	g_mixer = _mixer;
}

Engine::~Engine()
{
	delete _mixer;
}

Engine *Engine::createFromDetector(GameDetector *detector, OSystem *syst)
{
	Engine *engine;

	if (detector->_gameId >= GID_SIMON_FIRST && detector->_gameId <= GID_SIMON_LAST) {
		// Simon the Sorcerer
		detector->_gameId -= GID_SIMON_FIRST;
		engine = new SimonState(detector, syst);
	} else {
		// Some kind of Scumm game
		if (detector->_features & GF_OLD256)
			engine = new Scumm_v3(detector, syst);
		else if (detector->_features & GF_SMALL_HEADER)	// this force loomCD as v4
			engine = new Scumm_v4(detector, syst);
		else if (detector->_features & GF_AFTER_V7)
			engine = new Scumm_v7(detector, syst);
		else if (detector->_features & GF_AFTER_V6)	// this force SamnmaxCD as v6
			engine = new Scumm_v6(detector, syst);
		else
			engine = new Scumm_v5(detector, syst);
	}

	return engine;
}
