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

#include "stdafx.h"
#include "engine.h"
#include "gameDetector.h"
#include "config-file.h"
#include "scumm.h"
#include "simon/simon.h"
#include "sound/mixer.h"

/* FIXME - BIG HACK for MidiEmu */
OSystem *g_system = 0;
SoundMixer *g_mixer = 0;

Engine::Engine(GameDetector *detector, OSystem *syst)
	: _system(syst)
{
	_mixer = new SoundMixer();

	_gameDataPath = detector->_gameDataPath;

	/* FIXME - BIG HACK for MidiEmu */
	g_system = _system;
	g_mixer = _mixer;
}

Engine::~Engine()
{
	delete _mixer;
}

const char *Engine::getSavePath() const
{
	const char *dir = NULL;

#ifdef _WIN32_WCE
	dir = _gameDataPath;
#else

#if !defined(MACOS_CARBON)
	dir = getenv("SCUMMVM_SAVEPATH");
#endif

	// If SCUMMVM_SAVEPATH was not specified, try to use game specific savepath from config
	if (!dir || dir[0] == 0)
		dir = scummcfg->get("savepath");

	// If SCUMMVM_SAVEPATH was not specified, try to use general path from config
	if (!dir || dir[0] == 0)
		dir = scummcfg->get("savepath", "scummvm");

	// If no save path was specified, use no directory prefix
	if (dir == NULL)
		dir = "";
#endif

	return dir;
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
