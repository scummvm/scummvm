/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2004 The ScummVM project
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
#include "base/engine.h"
#include "base/gameDetector.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/timer.h"
#include "sound/mixer.h"

/* FIXME - BIG HACK for MidiEmu */
Engine *g_engine = 0;

uint16 g_debugLevel = 0;

Engine::Engine(OSystem *syst)
	: _system(syst), _gameDataPath(ConfMan.get("path")) {
	g_engine = this;
	_mixer = GameDetector::createMixer();

	_timer = g_timer;

	// Add default file directory
	File::addDefaultDirectory(_gameDataPath);

	g_debugLevel = ConfMan.getInt("debuglevel");

	_saveFileMan = _system->getSavefileManager();
}

Engine::~Engine() {
	File::resetDefaultDirectories();

	delete _mixer;
	delete _saveFileMan;

	g_engine = NULL;
}

const char *Engine::getSavePath() const {

#if defined(__PALM_OS__)
	return SCUMMVM_SAVEPATH;
#else

	const char *dir = NULL;

#if !defined(MACOS_CARBON) && !defined(_WIN32_WCE)
	dir = getenv("SCUMMVM_SAVEPATH");
#endif

	// If SCUMMVM_SAVEPATH was not specified, try to use game specific savepath from config
	if (!dir || dir[0] == 0) {
		dir = ConfMan.get("savepath").c_str();
		
		// Work around a bug (#999122) in the original 0.6.1 release of
		// ScummVM, which would insert a bad savepath value into config files.
		if (0 == strcmp(dir, "None")) {
			ConfMan.removeKey("savepath", ConfMan.getActiveDomain());
			ConfMan.flushToDisk();
			dir = ConfMan.get("savepath").c_str();
		}
	}

#ifdef _WIN32_WCE
	if (dir[0] == 0)
		dir = _gameDataPath.c_str();
#endif

	assert(dir);

	return dir;
#endif
}

const char *Engine::getGameDataPath() const {
	return _gameDataPath.c_str();
}

void NORETURN CDECL error(const char *s, ...) {
	char buf_input[STRINGBUFLEN];
	char buf_output[STRINGBUFLEN];
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
#ifndef _WIN32_WCE
	fprintf(stderr, "%s!\n", buf_output);
#endif
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
	char buf[STRINGBUFLEN];
	va_list va;

	va_start(va, s);
	vsprintf(buf, s, va);
	va_end(va);

#ifdef __GP32__ //ph0x FIXME: implement fprint?
	printf("WARNING: %s\n", buf);
#else
#ifndef _WIN32_WCE
	fprintf(stderr, "WARNING: %s!\n", buf);
#endif
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

void CDECL debug(int level, const char *s, ...) {
	char buf[STRINGBUFLEN];
	va_list va;

	if (level > g_debugLevel)
		return;

	va_start(va, s);
	vsprintf(buf, s, va);
	va_end(va);
#ifndef _WIN32_WCE
	printf("%s\n", buf);
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

	fflush(stdout);
}

void CDECL debug(const char *s, ...) {
	char buf[STRINGBUFLEN];
	va_list va;

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
