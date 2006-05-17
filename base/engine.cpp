/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#include "common/stdafx.h"
#if defined(WIN32)
#include <malloc.h>
#endif
#include "base/engine.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/timer.h"
#include "common/savefile.h"
#include "common/system.h"
#include "sound/mixer.h"
#include "gui/message.h"

/* FIXME - BIG HACK for MidiEmu */
Engine *g_engine = 0;

Engine::Engine(OSystem *syst)
	: _system(syst),
		_gameDataPath(ConfMan.get("path")),
		_targetName(ConfMan.getActiveDomainName()){
	g_engine = this;
	_mixer = new Audio::Mixer();

	_timer = Common::g_timer;

	_saveFileMan = _system->getSavefileManager();

	_autosavePeriod = ConfMan.getInt("autosave_period");
}

Engine::~Engine() {
	delete _mixer;
	delete _saveFileMan;

	g_engine = NULL;
}

void Engine::initCommonGFX(bool defaultTo1XScaler) {
	const Common::ConfigManager::Domain *transientDomain = ConfMan.getDomain(Common::ConfigManager::kTransientDomain);
	const Common::ConfigManager::Domain *gameDomain = ConfMan.getActiveDomain();

	assert(transientDomain);

	const bool useDefaultGraphicsMode =
		!transientDomain->contains("gfx_mode") &&
		(
		!gameDomain ||
		!gameDomain->contains("gfx_mode") ||
		!scumm_stricmp(gameDomain->get("gfx_mode").c_str(), "normal") ||
		!scumm_stricmp(gameDomain->get("gfx_mode").c_str(), "default")
		);

	// See if the game should default to 1x scaler
	if (useDefaultGraphicsMode && defaultTo1XScaler) {
		// FIXME: As a hack, we use "1x" here. Would be nicer to use
		// getDefaultGraphicsMode() instead, but right now, we do not specify
		// whether that is a 1x scaler or not...
		_system->setGraphicsMode("1x");
	} else {
		// Override global scaler with any game-specific define
		if (ConfMan.hasKey("gfx_mode")) {
			_system->setGraphicsMode(ConfMan.get("gfx_mode").c_str());
		}
	}

	// Note: The following code deals with the fullscreen / ASR settings. This
	// is a bit tricky, because there are three ways the user can affect these
	// settings: Via the config file, via the command line, and via in-game
	// hotkeys.
	// Any global or command line settings already have been applied at the time
	// we get here. Hence we only do something 

	// (De)activate aspect-ratio correction as determined by the config settings
	if (gameDomain && gameDomain->contains("aspect_ratio"))
		_system->setFeatureState(OSystem::kFeatureAspectRatioCorrection, ConfMan.getBool("aspect_ratio"));

	// (De)activate fullscreen mode as determined by the config settings
	if (gameDomain && gameDomain->contains("fullscreen"))
		_system->setFeatureState(OSystem::kFeatureFullscreenMode, ConfMan.getBool("fullscreen"));
}

void Engine::checkCD() {
#if defined (WIN32) && !defined(_WIN32_WCE)
	// It is a known bug under Windows that games that play CD audio cause
	// ScummVM to crash if the data files are read from the same CD. Check
	// if this appears to be the case and issue a warning.

	// If we can find a compressed audio track, then it should be ok even
	// if it's running from CD.

#ifdef USE_VORBIS
	if (Common::File::exists("track1.ogg"))
		return;
#endif
#ifdef USE_FLAC
	if (Common::File::exists("track1.fla") || Common::File::exists("track1.flac"))
		return;
#endif
#ifdef USE_MAD
	if (Common::File::exists("track1.mp3"))
		return;
#endif

	char buffer[MAXPATHLEN];
	int i;

	if (strlen(_gameDataPath.c_str()) == 0) {
		// That's it! I give up!
		if (getcwd(buffer, MAXPATHLEN) == NULL)
			return;
	} else
		strncpy(buffer, _gameDataPath.c_str(), MAXPATHLEN);

	for (i = 0; i < MAXPATHLEN - 1; i++) {
		if (buffer[i] == '\\')
			break;
	}

	buffer[i + 1] = 0;

	if (GetDriveType(buffer) == DRIVE_CDROM) {
		GUI::MessageDialog dialog(
			"You appear to be playing this game directly\n"
			"from the CD. This is known to cause problems,\n"
			"and it's therefore recommended that you copy\n"
			"the data files to your hard disk instead.\n"
			"See the README file for details.", "OK");
		dialog.runModal();
	}
#endif
}

bool Engine::shouldPerformAutoSave(int lastSaveTime) {
	const int diff = _system->getMillis() - lastSaveTime;
	return _autosavePeriod != 0 && diff > _autosavePeriod * 1000;
}

void Engine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);
}

void NORETURN CDECL error(const char *s, ...) {
	char buf_input[STRINGBUFLEN];
	char buf_output[STRINGBUFLEN];
	va_list va;

	va_start(va, s);
	vsnprintf(buf_input, STRINGBUFLEN, s, va);
	va_end(va);

#ifndef __GP32__
	if (g_engine) {
		g_engine->errorString(buf_input, buf_output);
	} else {
		strcpy(buf_output, buf_input);
	}
#else
	strcpy(buf_output, buf_input);
#endif
#ifdef __GP32__
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

#ifdef PALMOS_MODE
	PalmFatalError(buf_output);
#endif

#ifdef __SYMBIAN32__
	Symbian::FatalError(buf_output);
#endif
	// Finally exit. quit() will terminate the program if g_system is present
	if (g_system)
		g_system->quit();

	exit(1);
}

void CDECL warning(const char *s, ...) {
	char buf[STRINGBUFLEN];
	va_list va;

	va_start(va, s);
	vsnprintf(buf, STRINGBUFLEN, s, va);
	va_end(va);

#ifdef __GP32__ //ph0x FIXME: implement fprint?
	printf("WARNING: %s\n", buf);
#else
#if !defined (_WIN32_WCE) && !defined (__SYMBIAN32__)
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

void checkHeap() {
#if defined(WIN32)
	if (_heapchk() != _HEAPOK) {
		error("Heap is invalid!");
	}
#endif
}

void Engine::GUIErrorMessage(const Common::String msg) {
	_system->setWindowCaption("Error");
	_system->beginGFXTransaction();
		initCommonGFX(false);
		_system->initSize(320, 200);
	_system->endGFXTransaction();

	GUI::MessageDialog dialog(msg);
	dialog.runModal();
}
