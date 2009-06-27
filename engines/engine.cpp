/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#if defined(WIN32)
#include <windows.h>
#include <direct.h>
// winnt.h defines ARRAYSIZE, but we want our own one...
#undef ARRAYSIZE
#endif

#include "engines/engine.h"
#include "common/config-manager.h"
#include "common/events.h"
#include "common/file.h"
#include "common/timer.h"
#include "common/savefile.h"
#include "common/system.h"
#include "gui/message.h"
#include "gui/GuiManager.h"
#include "sound/mixer.h"
#include "engines/dialogs.h"
#include "engines/metaengine.h"

#ifdef _WIN32_WCE
extern bool isSmartphone(void);
#endif

// FIXME: HACK for MidiEmu & error()
Engine *g_engine = 0;


Engine::Engine(OSystem *syst)
	: _system(syst),
		_mixer(_system->getMixer()),
		_timer(_system->getTimerManager()),
		_eventMan(_system->getEventManager()),
		_saveFileMan(_system->getSavefileManager()),
		_targetName(ConfMan.getActiveDomainName()),
		_gameDataDir(ConfMan.get("path")),
		_pauseLevel(0),
		_mainMenuDialog(NULL) {

	g_engine = this;

	// FIXME: Get rid of the following again. It is only here temporarily.
	// We really should never run with a non-working Mixer, so ought to handle
	// this at a much earlier stage. If we *really* want to support systems
	// without a working mixer, then we need more work. E.g. we could modify the
	// Mixer to immediately drop any streams passed to it. This way, at least
	// we don't crash because heaps of (sound) memory get allocated but never
	// freed. Of course, there still would be problems with many games...
	if (!_mixer->isReady())
		warning("Sound initialization failed. This may cause severe problems in some games.");
}

Engine::~Engine() {
	_mixer->stopAll();

	delete _mainMenuDialog;
	g_engine = NULL;
}

void initCommonGFX(bool defaultTo1XScaler) {
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
		g_system->setGraphicsMode("1x");
	} else {
		// Override global scaler with any game-specific define
		if (ConfMan.hasKey("gfx_mode")) {
			g_system->setGraphicsMode(ConfMan.get("gfx_mode").c_str());
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
		g_system->setFeatureState(OSystem::kFeatureAspectRatioCorrection, ConfMan.getBool("aspect_ratio"));

	// (De)activate fullscreen mode as determined by the config settings
	if (gameDomain && gameDomain->contains("fullscreen"))
		g_system->setFeatureState(OSystem::kFeatureFullscreenMode, ConfMan.getBool("fullscreen"));
}
void initGraphics(int width, int height, bool defaultTo1xScaler, Graphics::PixelFormat *format) {

	g_system->beginGFXTransaction();

		initCommonGFX(defaultTo1xScaler);
#ifdef ENABLE_RGB_COLOR
		g_system->initSize(width, height, format);
#else
		g_system->initSize(width, height);
#endif

	OSystem::TransactionError gfxError = g_system->endGFXTransaction();

	if (gfxError == OSystem::kTransactionSuccess)
		return;

	// Error out on size switch failure
	if (gfxError & OSystem::kTransactionSizeChangeFailed) {
		char buffer[16];
		snprintf(buffer, 16, "%dx%d", width, height);

		Common::String message = "Could not switch to resolution: '";
		message += buffer;
		message += "'.";

		GUIErrorMessage(message);
		error("%s", message.c_str());
	}

	// Just show warnings then these occur:
#ifdef ENABLE_RGB_COLOR
	if (gfxError & OSystem::kTransactionPixelFormatNotSupported) {
		Common::String message = "Could not initialize color format.";

		GUI::MessageDialog dialog(message);
		dialog.runModal();
	}
#endif

	if (gfxError & OSystem::kTransactionModeSwitchFailed) {
		Common::String message = "Could not switch to video mode: '";
		message += ConfMan.get("gfx_mode");
		message += "'.";

		GUI::MessageDialog dialog(message);
		dialog.runModal();
	}

	if (gfxError & OSystem::kTransactionAspectRatioFailed) {
		GUI::MessageDialog dialog("Could not apply aspect ratio setting.");
		dialog.runModal();
	}

	if (gfxError & OSystem::kTransactionFullscreenFailed) {
		GUI::MessageDialog dialog("Could not apply fullscreen setting.");
		dialog.runModal();
	}
}

void GUIErrorMessage(const Common::String msg) {
	g_system->setWindowCaption("Error");
	g_system->beginGFXTransaction();
		initCommonGFX(false);
		g_system->initSize(320, 200);
	if (g_system->endGFXTransaction() == OSystem::kTransactionSuccess) {
		GUI::MessageDialog dialog(msg);
		dialog.runModal();
	} else {
		error("%s", msg.c_str());
	}
}

void Engine::checkCD() {
#if defined (WIN32) && !defined(_WIN32_WCE) && !defined(__SYMBIAN32__)
	// It is a known bug under Windows that games that play CD audio cause
	// ScummVM to crash if the data files are read from the same CD. Check
	// if this appears to be the case and issue a warning.

	// If we can find a compressed audio track, then it should be ok even
	// if it's running from CD.

#ifdef USE_VORBIS
	if (Common::File::exists("track1.ogg") ||
	    Common::File::exists("track01.ogg"))
		return;
#endif
#ifdef USE_FLAC
	if (Common::File::exists("track1.fla") ||
            Common::File::exists("track1.flac") ||
	    Common::File::exists("track01.fla") ||
	    Common::File::exists("track01.flac"))
		return;
#endif
#ifdef USE_MAD
	if (Common::File::exists("track1.mp3") ||
	    Common::File::exists("track01.mp3"))
		return;
#endif

	char buffer[MAXPATHLEN];
	int i;

	if (_gameDataDir.getPath().empty()) {
		// That's it! I give up!
		if (getcwd(buffer, MAXPATHLEN) == NULL)
			return;
	} else
		strncpy(buffer, _gameDataDir.getPath().c_str(), MAXPATHLEN);

	for (i = 0; i < MAXPATHLEN - 1; i++) {
		if (buffer[i] == '\\')
			break;
	}

	buffer[i + 1] = 0;

	if (GetDriveType(buffer) == DRIVE_CDROM) {
		GUI::MessageDialog dialog(
			"You appear to be playing this game directly\n"
			"from the CD. This is known to cause problems,\n"
			"and it is therefore recommended that you copy\n"
			"the data files to your hard disk instead.\n"
			"See the README file for details.", "OK");
		dialog.runModal();
	} else {
		// If we reached here, the game has audio tracks,
		// it's not ran from the CD and the tracks have not
		// been ripped.
		GUI::MessageDialog dialog(
			"This game has audio tracks in its disk. These\n"
			"tracks need to be ripped from the disk using\n"
			"an appropriate CD audio extracting tool in\n"
			"order to listen to the game's music.\n"
			"See the README file for details.", "OK");
		dialog.runModal();
	}
#endif
}

bool Engine::shouldPerformAutoSave(int lastSaveTime) {
	const int diff = _system->getMillis() - lastSaveTime;
	const int autosavePeriod = ConfMan.getInt("autosave_period");
	return autosavePeriod != 0 && diff > autosavePeriod * 1000;
}

void Engine::errorString(const char *buf1, char *buf2, int size) {
	strncpy(buf2, buf1, size);
	if (size > 0)
		buf2[size-1] = '\0';
}

void Engine::pauseEngine(bool pause) {
	assert((pause && _pauseLevel >= 0) || (!pause && _pauseLevel));

	if (pause)
		_pauseLevel++;
	else
		_pauseLevel--;

	if (_pauseLevel == 1) {
		pauseEngineIntern(true);
	} else if (_pauseLevel == 0) {
		pauseEngineIntern(false);
	}
}

void Engine::pauseEngineIntern(bool pause) {
	// By default, just (un)pause all digital sounds
	_mixer->pauseAll(pause);
}

void Engine::openMainMenuDialog() {
	if (!_mainMenuDialog)
		_mainMenuDialog = new MainMenuDialog(this);
	runDialog(*_mainMenuDialog);
	syncSoundSettings();
}

int Engine::runDialog(GUI::Dialog &dialog) {
	pauseEngine(true);
	int result = dialog.runModal();
	pauseEngine(false);

	return result;
}

void Engine::syncSoundSettings() {

	// Sync the engine with the config manager
	int soundVolumeMusic = ConfMan.getInt("music_volume");
	int soundVolumeSFX = ConfMan.getInt("sfx_volume");
	int soundVolumeSpeech = ConfMan.getInt("speech_volume");

	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, soundVolumeMusic);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, soundVolumeSFX);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, soundVolumeSpeech);
}

Common::Error Engine::loadGameState(int slot) {
	// Do nothing by default
	return Common::kNoError;
}

bool Engine::canLoadGameStateCurrently() {
	// Do not allow loading by default
	return false;
}

Common::Error Engine::saveGameState(int slot, const char *desc) {
	// Do nothing by default
	return Common::kNoError;
}

bool Engine::canSaveGameStateCurrently() {
	// Do not allow saving by default
	return false;
}

void Engine::quitGame() {
	Common::Event event;

	event.type = Common::EVENT_QUIT;
	g_system->getEventManager()->pushEvent(event);
}

bool Engine::shouldQuit() {
	Common::EventManager *eventMan = g_system->getEventManager();
	return (eventMan->shouldQuit() || eventMan->shouldRTL());
}

/*
EnginePlugin *Engine::getMetaEnginePlugin() const {

	const EnginePlugin *plugin = 0;
	Common::String gameid = ConfMan.get("gameid");
	gameid.toLowercase();
	EngineMan.findGame(gameid, &plugin);
	return plugin;
}

*/
