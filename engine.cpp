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
