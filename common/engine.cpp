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
#if defined(_MSC_VER)
#include <malloc.h>
#endif
#include "common/config-file.h"
#include "common/engine.h"
#include "common/gameDetector.h"
#include "common/timer.h"
#include "sound/mixer.h"

/*
 * Version string and build date string. These can be used by anything that
 * wants to display this information to the user (e.g. about dialog).
 *
 * Note: it would be very nice if we could instead of (or in addition to) the
 * build date present a date which corresponds to the date our source files
 * were last changed. To understand the difference, imagine that a user
 * makes a checkout of CVS on January 1, then after a week compiles it
 * (e.g. after doing a 'make clean'). The build date then will say January 8
 * even though the files were last changed on January 1.
 *
 * Another problem is that __DATE__/__TIME__ depend on the local time zone.
 *
 * It's clear that such a "last changed" date would be much more useful to us
 * for feedback purposes. After all, when somebody files a bug report, we
 * don't care about the build date, we want to know which date their checkout
 * was made. This is even more important now since anon CVS lags a few
 * days behind developer CVS.
 *
 * So, how could we implement this? At least on unix systems, a special script
 * could do it. Basically, that script would run over all .cpp/.h files and
 * parse the CVS 'Header' keyword we have in our file headers.
 * That line contains a date/time in GMT. Now, the script just has to collect
 * all these times and find the latest. This time then would be inserted into
 * a header file or so (common/date.h ?) which engine.cpp then could
 * include and put into a global variable analog to gScummVMBuildDate.
 *
 * Drawback: scanning all source/header files will be rather slow. Also, this
 * only works on systems which can run powerful enough scripts (so I guess
 * Visual C++ would be out of the game here? don't know VC enough to be sure).
 *
 * Another approach would be to somehow get CVS to update a global file
 * (e.g. LAST_CHANGED) whenever any checkins are made. That would be
 * faster and work w/o much "logic" on the client side, in particular no
 * scripts have to be run. The problem with this is that I am not even
 * sure it's actually possible! Modifying files during commit time is trivial
 * to setup, but I have no idea if/how one can also change files which are not
 * currently being commit'ed.
 */
const char *gScummVMVersion = "0.5.3cvs";
const char *gScummVMBuildDate = __DATE__ " " __TIME__;
const char *gScummVMFullVersion = "ScummVM 0.5.3cvs (" __DATE__ " " __TIME__ ")";

/* FIXME - BIG HACK for MidiEmu */
OSystem *g_system = 0;
Engine *g_engine = 0;

Engine::Engine(GameDetector *detector, OSystem *syst)
	: _system(syst) {
	g_engine = this;
	_mixer = new SoundMixer();

	_gameDataPath = detector->_gameDataPath;

	g_system = _system; // FIXME - BIG HACK for MidiEmu

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

#ifndef DISABLE_SWORD2
	if (detector->_game.id >= GID_SWORD2_FIRST && detector->_game.id <= GID_SWORD2_LAST) {
		// Broken Sword 2
		engine = Engine_SWORD2_create(detector, syst);
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
#if defined( _WIN32_WCE )
	TCHAR buf_output_unicode[1024];
	MultiByteToWideChar(CP_ACP, 0, buf_output, strlen(buf_output) + 1, buf_output_unicode, sizeof(buf_output_unicode));
	OutputDebugString(buf_output_unicode);
#else
	OutputDebugString(buf_output);
#endif
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
#if defined( _WIN32_WCE )
	TCHAR buf_unicode[1024];
	MultiByteToWideChar(CP_ACP, 0, buf, strlen(buf) + 1, buf_unicode, sizeof(buf_unicode));
	OutputDebugString(buf_unicode);
#else
	OutputDebugString(buf);
#endif
#endif
}

uint16 _debugLevel = 0;

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
#if defined( _WIN32_WCE )
	TCHAR buf_unicode[1024];
	MultiByteToWideChar(CP_ACP, 0, buf, strlen(buf) + 1, buf_unicode, sizeof(buf_unicode));
	OutputDebugString(buf_unicode);
#else
	OutputDebugString(buf);
#endif
#endif

	fflush(stdout);
}

void checkHeap() {
#if defined(_MSC_VER)
	if (_heapchk() != _HEAPOK) {
		error("Heap is invalid!");
	}
#endif
}
