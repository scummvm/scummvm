/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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
#include "config-file.h"
#include "gameDetector.h"
#include "timer.h"
#include "sound/mixer.h"

/* FIXME - BIG HACK for MidiEmu */
OSystem *g_system = 0;
SoundMixer *g_mixer = 0;
Engine *g_engine = 0;

Engine::Engine(GameDetector *detector, OSystem *syst)
	: _system(syst) {
	g_engine = this;
	_mixer = new SoundMixer();

	_gameDataPath = detector->_gameDataPath;

	/* FIXME - BIG HACK for MidiEmu */
	g_system = _system;
	g_mixer = _mixer;
	_timer = new Timer(this);
}

Engine::~Engine() {
	delete _mixer;
	delete _timer;
}

const char *Engine::getSavePath() const {
	const char *dir = NULL;

#ifdef _WIN32_WCE
	dir = _gameDataPath;
#elif defined(__PALM_OS__)
	dir = SCUMMVM_SAVEPATH;
#else

#if !defined(MACOS_CARBON)
	dir = getenv("SCUMMVM_SAVEPATH");
#endif

	// If SCUMMVM_SAVEPATH was not specified, try to use game specific savepath from config
	if (!dir || dir[0] == 0)
		dir = g_config->get("savepath");

	// If SCUMMVM_SAVEPATH was not specified, try to use general path from config
	if (!dir || dir[0] == 0)
		dir = g_config->get("savepath", "scummvm");

	// If no save path was specified, use no directory prefix
	if (dir == NULL)
		dir = "";
#endif

	return dir;
}

Engine *Engine::createFromDetector(GameDetector *detector, OSystem *syst) {
	Engine *engine = NULL;

#ifndef DISABLE_SCUMM
	if (detector->_game.id >= GID_SCUMM_FIRST && detector->_game.id <= GID_SCUMM_LAST) {
		// Some kind of Scumm game
		engine = Engine_SCUMM_create(detector, syst);
	}
#endif

#ifndef DISABLE_SIMON
	if (detector->_game.id >= GID_SIMON_FIRST && detector->_game.id <= GID_SIMON_LAST) {
		// Simon the Sorcerer
		engine = Engine_SIMON_create(detector, syst);
	}
#endif

#ifndef DISABLE_SKY
	if (detector->_game.id >= GID_SKY_FIRST && detector->_game.id <= GID_SKY_LAST) {
		// Beneath a Steel Sky
		engine = Engine_SKY_create(detector, syst);
	}
#endif

	return engine;
}

void NORETURN CDECL error(const char *s, ...) {
#ifdef __PALM_OS__
	char buf_input[256]; // 1024 is too big overflow the stack
	char buf_output[256];
#else
	char buf_input[1024];
	char buf_output[1024];
#endif
	va_list va;

	va_start(va, s);
	vsprintf(buf_input, s, va);
	va_end(va);

	if (g_engine) {
		g_engine->errorString(buf_input, buf_output);
	} else {
		strcpy(buf_output, buf_input);
	}

#ifdef __GP32__ //ph0x FIXME?
	printf("ERROR: %s\n", buf_output);
#else
	fprintf(stderr, "%s!\n", buf_output);
#endif

#if defined( USE_WINDBG )
	OutputDebugString(buf_output);
#endif

#if defined ( _WIN32_WCE )
	drawError(buf_output);
#endif

#ifdef __PALM_OS__
	PalmFatalError(buf_output);
#endif

	// Finally exit. quit() will terminate the program if g_system iss present
	if (g_system)
		g_system->quit();
	
	exit(1);
}

void CDECL warning(const char *s, ...) {
#ifdef __PALM_OS__
	char buf[256]; // 1024 is too big overflow the stack
#else
	char buf[1024];
#endif
	va_list va;

	va_start(va, s);
	vsprintf(buf, s, va);
	va_end(va);

#ifdef __GP32__ //ph0x FIXME: implement fprint?
	printf("WARNING: %s\n", buf);
#else
	fprintf(stderr, "WARNING: %s!\n", buf);
#endif
#if defined( USE_WINDBG )
	strcat(buf, "\n");
	OutputDebugString(buf);
#endif
}

uint16 _debugLevel = 1;

void CDECL debug(int level, const char *s, ...) {
#ifdef __PALM_OS__
	char buf[256]; // 1024 is too big overflow the stack
#else
	char buf[1024];
#endif
	va_list va;

	if (level > _debugLevel)
		return;

	va_start(va, s);
	vsprintf(buf, s, va);
	va_end(va);
	printf("%s\n", buf);

#if defined( USE_WINDBG )
	strcat(buf, "\n");
	OutputDebugString(buf);
#endif

	fflush(stdout);
}

void checkHeap() {
#if defined(WIN32)
	if (_heapchk() != _HEAPOK) {
		error("Heap is invalid!");
	}
#endif
}
