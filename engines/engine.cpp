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
 */

#define FORBIDDEN_SYMBOL_EXCEPTION_getcwd

#if defined(WIN32) && !defined(__SYMBIAN32__)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>
#endif

#include "engines/engine.h"
#include "engines/dialogs.h"
#include "engines/util.h"
#include "engines/metaengine.h"

#include "common/config-manager.h"
#include "common/events.h"
#include "common/file.h"
#include "common/system.h"
#include "common/str.h"
#include "common/ustr.h"
#include "common/error.h"
#include "common/list.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/scummsys.h"
#include "common/taskbar.h"
#include "common/textconsole.h"
#include "common/translation.h"
#include "common/singleton.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "base/version.h"

#include "gui/gui-manager.h"
#include "gui/debugger.h"
#include "gui/dialog.h"
#include "gui/message.h"
#include "gui/saveload.h"

#include "audio/mixer.h"

#include "graphics/cursorman.h"
#include "graphics/fontman.h"
#include "graphics/pixelformat.h"
#include "image/bmp.h"

#ifdef USE_TTS
#include "common/text-to-speech.h"
#endif

// FIXME: HACK for error()
Engine *g_engine = 0;

// Output formatter for debug() and error() which invokes
// the errorString method of the active engine, if any.
static void defaultOutputFormatter(char *dst, const char *src, size_t dstSize) {
	if (g_engine) {
		g_engine->errorString(src, dst, dstSize);
	} else {
		Common::strlcpy(dst, src, dstSize);
	}
}

static void defaultErrorHandler(const char *msg) {
	// Unless this error -originated- within the debugger itself, we
	// now invoke the debugger, if available / supported.
	if (g_engine) {
		GUI::Debugger *debugger = g_engine->getOrCreateDebugger();

#if defined(USE_TASKBAR)
		g_system->getTaskbarManager()->notifyError();
#endif

		if (debugger && !debugger->isActive()) {
			debugger->attach(msg);
			debugger->onFrame();
		}


#if defined(USE_TASKBAR)
		g_system->getTaskbarManager()->clearError();
#endif

	}
}

// Chained games manager

ChainedGamesManager::ChainedGamesManager() {
	clear();
}

void ChainedGamesManager::clear() {
	_chainedGames.clear();
}

void ChainedGamesManager::push(const Common::String target, const int slot) {
	Game game;
	game.target = target;
	game.slot = slot;
	_chainedGames.push(game);
}

bool ChainedGamesManager::pop(Common::String &target, int &slot) {
	if (_chainedGames.empty()) {
		return false;
	}
	Game game = _chainedGames.pop();
	target = game.target;
	slot = game.slot;
	return true;
}

namespace Common {
DECLARE_SINGLETON(ChainedGamesManager);
}

Engine::Engine(OSystem *syst)
	: _system(syst),
		_mixer(_system->getMixer()),
		_timer(_system->getTimerManager()),
		_eventMan(_system->getEventManager()),
		_saveFileMan(_system->getSavefileManager()),
		_targetName(ConfMan.getActiveDomainName()),
		_pauseLevel(0),
		_pauseStartTime(0),
		_saveSlotToLoad(-1),
		_engineStartTime(_system->getMillis()),
		_mainMenuDialog(NULL),
		_debugger(NULL),
		_autosaveInterval(ConfMan.getInt("autosave_period")),
		_lastAutosaveTime(_system->getMillis()) {

	g_engine = this;
	Common::setErrorOutputFormatter(defaultOutputFormatter);
	Common::setErrorHandler(defaultErrorHandler);

	// FIXME: Get rid of the following again. It is only here
	// temporarily. We really should never run with a non-working Mixer,
	// so ought to handle this at a much earlier stage. If we *really*
	// want to support systems without a working mixer, then we need
	// more work. E.g. we could modify the Mixer to immediately drop any
	// streams passed to it. This way, at least we don't crash because
	// heaps of (sound) memory get allocated but never freed. Of course,
	// there still would be problems with many games...
	if (!_mixer->isReady())
		warning("Sound initialization failed. This may cause severe problems in some games");

	// Setup a dummy cursor and palette, so that all engines can use
	// CursorMan.replace without having any headaches about memory leaks.
	//
	// If an engine only used CursorMan.replaceCursor and no cursor has
	// been setup before, then replaceCursor just uses pushCursor. This
	// means that that the engine's cursor is never again removed from
	// CursorMan. Hence we setup a fake cursor here and remove it again
	// in the destructor.
	CursorMan.pushCursor(NULL, 0, 0, 0, 0, 0);
	// Note: Using this dummy palette will actually disable cursor
	// palettes till the user enables it again.
	CursorMan.pushCursorPalette(NULL, 0, 0);
}

Engine::~Engine() {
	_mixer->stopAll();

	delete _debugger;
	delete _mainMenuDialog;
	g_engine = NULL;

	// Remove our cursors again to prevent memory leaks
	CursorMan.popCursor();
	CursorMan.popCursorPalette();
}

void Engine::initializePath(const Common::FSNode &gamePath) {
	SearchMan.addDirectory(gamePath.getPath(), gamePath, 0, 4);
}

void initCommonGFX() {
	const Common::ConfigManager::Domain *gameDomain = ConfMan.getActiveDomain();

	// Any global or command line settings already have been applied at the time
	// we get here, so we only do something if the game domain overrides those
	// values
	if (gameDomain) {
		if (gameDomain->contains("aspect_ratio"))
			g_system->setFeatureState(OSystem::kFeatureAspectRatioCorrection, ConfMan.getBool("aspect_ratio"));

		if (gameDomain->contains("fullscreen"))
			g_system->setFeatureState(OSystem::kFeatureFullscreenMode, ConfMan.getBool("fullscreen"));

		if (gameDomain->contains("filtering"))
			g_system->setFeatureState(OSystem::kFeatureFilteringMode, ConfMan.getBool("filtering"));

		if (gameDomain->contains("stretch_mode"))
			g_system->setStretchMode(ConfMan.get("stretch_mode").c_str());

		if (gameDomain->contains("shader"))
			g_system->setShader(ConfMan.get("shader").c_str());
	}
}

// Please leave the splash screen in working order for your releases, even if they're commercial.
// This is a proper and good way to show your appreciation for our hard work over these years.
bool splash = false;

#include "logo_data.h"

void splashScreen() {
	Common::MemoryReadStream stream(logo_data, ARRAYSIZE(logo_data));

	Image::BitmapDecoder bitmap;

	if (!bitmap.loadStream(stream)) {
		warning("Error loading logo file");
		return;
	}

	g_system->showOverlay();

	// Fill with orange
	Graphics::Surface screen;
	screen.create(g_system->getOverlayWidth(), g_system->getOverlayHeight(), g_system->getOverlayFormat());
	screen.fillRect(Common::Rect(screen.w, screen.h), screen.format.ARGBToColor(0xff, 0xcc, 0x66, 0x00));

	// Load logo
	Graphics::Surface *logo = bitmap.getSurface()->convertTo(g_system->getOverlayFormat(), bitmap.getPalette());
	int lx = MAX((g_system->getOverlayWidth() - logo->w) / 2, 0);
	int ly = MAX((g_system->getOverlayHeight() - logo->h) / 2, 0);

	// Print version information
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont);
	int w = font->getStringWidth(gScummVMVersionDate);
	int x = g_system->getOverlayWidth() - w - 5; // lx + logo->w - w + 5;
	int y = g_system->getOverlayHeight() - font->getFontHeight() - 5; //ly + logo->h + 5;
	font->drawString(&screen, gScummVMVersionDate, x, y, w, screen.format.ARGBToColor(0xff, 0, 0, 0));

	g_system->copyRectToOverlay(screen.getPixels(), screen.pitch, 0, 0, screen.w, screen.h);
	screen.free();

	// Draw logo
	int lw = MIN<uint16>(logo->w, g_system->getOverlayWidth() - lx);
	int lh = MIN<uint16>(logo->h, g_system->getOverlayHeight() - ly);

	g_system->copyRectToOverlay(logo->getPixels(), logo->pitch, lx, ly, lw, lh);
	logo->free();
	delete logo;

	g_system->updateScreen();

	// Delay 0.6 secs
	uint time0 = g_system->getMillis();
	Common::Event event;

	// We must poll an event in order to have the window shown at least on Mac
	g_system->getEventManager()->pollEvent(event);

	while (time0 + 600 > g_system->getMillis()) {
		g_system->delayMillis(10);
	}
	g_system->hideOverlay();

	splash = true;
}

void initGraphicsModes(const Graphics::ModeList &modes) {
	g_system->initSizeHint(modes);
}

void initGraphics(int width, int height, const Graphics::PixelFormat *format) {

	g_system->beginGFXTransaction();

		initCommonGFX();
#ifdef USE_RGB_COLOR
		if (format)
			g_system->initSize(width, height, format);
		else {
			Graphics::PixelFormat bestFormat = g_system->getSupportedFormats().front();
			g_system->initSize(width, height, &bestFormat);
		}
#else
		g_system->initSize(width, height);
#endif

	OSystem::TransactionError gfxError = g_system->endGFXTransaction();

	if (!splash && !GUI::GuiManager::instance()._launched)
		splashScreen();

	if (gfxError == OSystem::kTransactionSuccess)
		return;

	// Error out on size switch failure
	if (gfxError & OSystem::kTransactionSizeChangeFailed) {
		Common::U32String message;
		message = Common::U32String::format(_("Could not switch to resolution '%dx%d'."), width, height);

		GUIErrorMessage(message);
		error("Could not switch to resolution '%dx%d'.", width, height);
	}

	// Just show warnings then these occur:
#ifdef USE_RGB_COLOR
	if (gfxError & OSystem::kTransactionFormatNotSupported) {
		Common::U32String message = _("Could not initialize color format.");

		GUI::MessageDialog dialog(message);
		dialog.runModal();
	}
#endif

	if (gfxError & OSystem::kTransactionModeSwitchFailed) {
		Common::U32String message;
		message = Common::U32String::format(_("Could not switch to video mode '%s'."), ConfMan.get("gfx_mode").c_str());

		GUI::MessageDialog dialog(message);
		dialog.runModal();
	}

	if (gfxError & OSystem::kTransactionStretchModeSwitchFailed) {
		Common::U32String message;
		message = Common::U32String::format(_("Could not switch to stretch mode '%s'."), ConfMan.get("stretch_mode").c_str());

		GUI::MessageDialog dialog(message);
		dialog.runModal();
	}

	if (gfxError & OSystem::kTransactionAspectRatioFailed) {
		GUI::MessageDialog dialog(_("Could not apply aspect ratio setting."));
		dialog.runModal();
	}

	if (gfxError & OSystem::kTransactionFullscreenFailed) {
		GUI::MessageDialog dialog(_("Could not apply fullscreen setting."));
		dialog.runModal();
	}

	if (gfxError & OSystem::kTransactionFilteringFailed) {
		GUI::MessageDialog dialog(_("Could not apply filtering setting."));
		dialog.runModal();
	}
}

/**
 * Determines the first matching format between two lists.
 *
 * @param backend	The higher priority list, meant to be a list of formats supported by the backend
 * @param frontend	The lower priority list, meant to be a list of formats supported by the engine
 * @return			The first item on the backend list that also occurs on the frontend list
 *					or PixelFormat::createFormatCLUT8() if no matching formats were found.
 */
inline Graphics::PixelFormat findCompatibleFormat(const Common::List<Graphics::PixelFormat> &backend, const Common::List<Graphics::PixelFormat> &frontend) {
#ifdef USE_RGB_COLOR
	for (Common::List<Graphics::PixelFormat>::const_iterator i = backend.begin(); i != backend.end(); ++i) {
		for (Common::List<Graphics::PixelFormat>::const_iterator j = frontend.begin(); j != frontend.end(); ++j) {
			if (*i == *j)
				return *i;
		}
	}
#endif
	return Graphics::PixelFormat::createFormatCLUT8();
}


void initGraphics(int width, int height, const Common::List<Graphics::PixelFormat> &formatList) {
	Graphics::PixelFormat format = findCompatibleFormat(g_system->getSupportedFormats(), formatList);
	initGraphics(width, height, &format);
}

void initGraphics(int width, int height) {
	Graphics::PixelFormat format = Graphics::PixelFormat::createFormatCLUT8();
	initGraphics(width, height, &format);
}

void initGraphics3d(int width, int height) {
	g_system->beginGFXTransaction();
		g_system->setGraphicsMode(0, OSystem::kGfxModeRender3d);
		g_system->initSize(width, height);
		g_system->setFeatureState(OSystem::kFeatureFullscreenMode, ConfMan.getBool("fullscreen")); // TODO: Replace this with initCommonGFX()
		g_system->setFeatureState(OSystem::kFeatureAspectRatioCorrection, ConfMan.getBool("aspect_ratio")); // TODO: Replace this with initCommonGFX()
	g_system->endGFXTransaction();
}

void GUIErrorMessageWithURL(const Common::U32String &msg, const char *url) {
	GUIErrorMessage(msg, url);
}

void GUIErrorMessageWithURL(const Common::String &msg, const char *url) {
	GUIErrorMessage(Common::U32String(msg), url);
}

void GUIErrorMessage(const Common::String &msg, const char *url) {
	GUIErrorMessage(Common::U32String(msg), url);
}

void GUIErrorMessage(const Common::U32String &msg, const char *url) {
	g_system->setWindowCaption(_("Error"));
	g_system->beginGFXTransaction();
		initCommonGFX();
		g_system->initSize(320, 200);
	if (g_system->endGFXTransaction() == OSystem::kTransactionSuccess) {
		if (url) {
			GUI::MessageDialogWithURL dialog(msg, url);
			dialog.runModal();
		} else {
			GUI::MessageDialog dialog(msg);
			dialog.runModal();
		}
	} else {
		error("%s", msg.encode().c_str());
	}
}

void GUIErrorMessageFormat(const char *fmt, ...) {
	Common::String msg;

	va_list va;
	va_start(va, fmt);
	msg = Common::String::vformat(fmt, va);
	va_end(va);

	GUIErrorMessage(msg);
}

void GUIErrorMessageFormat(Common::U32String fmt, ...) {
	Common::U32String msg("");

	va_list va;
	va_start(va, fmt);
	Common::U32String::vformat(fmt.begin(), fmt.end(), msg, va);
	va_end(va);

	GUIErrorMessage(msg);
}

void Engine::checkCD() {
#if defined(WIN32) && !defined(__SYMBIAN32__)
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

	const Common::FSNode gameDataDir(ConfMan.get("path"));

	if (gameDataDir.getPath().empty()) {
		// That's it! I give up!
		if (getcwd(buffer, MAXPATHLEN) == NULL)
			return;
	} else
		Common::strlcpy(buffer, gameDataDir.getPath().c_str(), sizeof(buffer));

	for (i = 0; i < MAXPATHLEN - 1; i++) {
		if (buffer[i] == '\\')
			break;
	}

	buffer[i + 1] = 0;

	if (GetDriveType(buffer) == DRIVE_CDROM) {
		GUI::MessageDialog dialog(
			_("You appear to be playing this game directly\n"
			"from the CD. This is known to cause problems,\n"
			"and it is therefore recommended that you copy\n"
			"the data files to your hard disk instead.\n"
			"See the README file for details."), _("OK"));
		dialog.runModal();
	} else {
		// If we reached here, the game has audio tracks,
		// it's not ran from the CD and the tracks have not
		// been ripped.
		GUI::MessageDialog dialog(
			_("This game has audio tracks in its disk. These\n"
			"tracks need to be ripped from the disk using\n"
			"an appropriate CD audio extracting tool in\n"
			"order to listen to the game's music.\n"
			"See the README file for details."), _("OK"));
		dialog.runModal();
	}
#endif
}

void Engine::handleAutoSave() {
	const int diff = _system->getMillis() - _lastAutosaveTime;

	if (_autosaveInterval != 0 && diff > (_autosaveInterval * 1000)) {
		// Save the autosave
		saveAutosaveIfEnabled();
	}
}

void Engine::saveAutosaveIfEnabled() {
	if (_autosaveInterval != 0) {
		bool saveFlag = canSaveAutosaveCurrently();

		if (saveFlag) {
			// First check for an existing savegame in the slot, and if present, if it's an autosave
			SaveStateDescriptor desc = getMetaEngine().querySaveMetaInfos(
				_targetName.c_str(), getAutosaveSlot());
			saveFlag = desc.getSaveSlot() == -1 || desc.isAutosave();
		}

		if (saveFlag && saveGameState(getAutosaveSlot(), Common::convertFromU32String(_("Autosave")), true).getCode() != Common::kNoError) {
			// Couldn't autosave at the designated time
			g_system->displayMessageOnOSD(_("Error occurred making autosave"));
			saveFlag = false;
		}

		if (!saveFlag) {
			// Set the next autosave interval to be in 5 minutes, rather than whatever
			// full autosave interval the user has selected
			_lastAutosaveTime = _system->getMillis() + (5 * 60 * 1000) - _autosaveInterval;
			return;
		}
	}

	// Reset the last autosave time
	_lastAutosaveTime = _system->getMillis();
}

void Engine::errorString(const char *buf1, char *buf2, int size) {
	Common::strlcpy(buf2, buf1, size);
}

PauseToken Engine::pauseEngine() {
	assert(_pauseLevel >= 0);

	_pauseLevel++;

	if (_pauseLevel == 1) {
		_pauseStartTime = _system->getMillis();
		pauseEngineIntern(true);
	}

	return PauseToken(this);
}

void Engine::resumeEngine() {
	assert(_pauseLevel > 0);

	_pauseLevel--;

	if (_pauseLevel == 0) {
		pauseEngineIntern(false);
		_engineStartTime += _system->getMillis() - _pauseStartTime;
		_pauseStartTime = 0;
	}
}

void Engine::pauseEngineIntern(bool pause) {
	// By default, just (un)pause all digital sounds
	_mixer->pauseAll(pause);
}

void Engine::openMainMenuDialog() {
	if (!_mainMenuDialog)
		_mainMenuDialog = new MainMenuDialog(this);
#ifdef USE_TTS
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	if (ttsMan != nullptr) {
		ttsMan->pushState();
		g_gui.initTextToSpeech();
	}
#endif

	setGameToLoadSlot(-1);

	bool hasVKeyb = g_system->getFeatureState(OSystem::kFeatureVirtualKeyboard);
	if (hasVKeyb)
		g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);

	runDialog(*_mainMenuDialog);

	if (hasVKeyb)
		g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);

	// Load savegame after main menu execution
	// (not from inside the menu loop to avoid
	// mouse cursor glitches and similar bugs,
	// e.g. #2822778).
	if (_saveSlotToLoad >= 0) {
		Common::Error status = loadGameState(_saveSlotToLoad);
		if (status.getCode() != Common::kNoError) {
			Common::U32String failMessage = Common::U32String::format(_("Failed to load saved game (%s)! "
				  "Please consult the README for basic information, and for "
				  "instructions on how to obtain further assistance."), status.getDesc().c_str());
			GUI::MessageDialog dialog(failMessage);
			dialog.runModal();
		}
	}

#ifdef USE_TTS
	if (ttsMan != nullptr)
		ttsMan->popState();
#endif

	g_system->applyBackendSettings();
	applyGameSettings();
	syncSoundSettings();
}

bool Engine::warnUserAboutUnsupportedGame() {
	if (ConfMan.getBool("enable_unsupported_game_warning")) {
		GUI::MessageDialog alert(_("WARNING: The game you are about to start is"
			" not yet fully supported by ScummVM. As such, it is likely to be"
			" unstable, and any saved game you make might not work in future"
			" versions of ScummVM."), _("Start anyway"), _("Cancel"));
		return alert.runModal() == GUI::kMessageOK;
	}
	return true;
}

void Engine::errorUnsupportedGame(Common::String extraMsg) {
	Common::String message = extraMsg.empty() ? _("This game is not supported.") : _("This game is not supported for the following reason:\n\n");
	message += _(extraMsg);
	message += "\n\n";
	GUI::MessageDialog(message).runModal();
}

uint32 Engine::getTotalPlayTime() const {
	if (!_pauseLevel)
		return _system->getMillis() - _engineStartTime;
	else
		return _pauseStartTime - _engineStartTime;
}

void Engine::setTotalPlayTime(uint32 time) {
	const uint32 currentTime = _system->getMillis();

	// We need to reset the pause start time here in case the engine is already
	// paused to avoid any incorrect play time counting.
	if (_pauseLevel > 0)
		_pauseStartTime = currentTime;

	_engineStartTime = currentTime - time;
}

int Engine::runDialog(GUI::Dialog &dialog) {
	PauseToken pt = pauseEngine();
	int result = dialog.runModal();

	return result;
}

void Engine::setGameToLoadSlot(int slot) {
	_saveSlotToLoad = slot;
}

void Engine::syncSoundSettings() {
	// Sync the engine with the config manager
	int soundVolumeMusic = ConfMan.getInt("music_volume");
	int soundVolumeSFX = ConfMan.getInt("sfx_volume");
	int soundVolumeSpeech = ConfMan.getInt("speech_volume");

	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	// We need to handle the speech mute separately here. This is because the
	// engine code should be able to rely on all speech sounds muted when the
	// user specified subtitles only mode, which results in "speech_mute" to
	// be set to "true". The global mute setting has precedence over the
	// speech mute setting though.
	bool speechMute = mute;
	if (!speechMute)
		speechMute = ConfMan.getBool("speech_mute");

	_mixer->muteSoundType(Audio::Mixer::kPlainSoundType, mute);
	_mixer->muteSoundType(Audio::Mixer::kMusicSoundType, mute);
	_mixer->muteSoundType(Audio::Mixer::kSFXSoundType, mute);
	_mixer->muteSoundType(Audio::Mixer::kSpeechSoundType, speechMute);

	_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, Audio::Mixer::kMaxMixerVolume);
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, soundVolumeMusic);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, soundVolumeSFX);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, soundVolumeSpeech);
}

void Engine::flipMute() {
	// Mute will be set to true by default here. This has two reasons:
	// - if the game already has an "mute" config entry, it will be overwritten anyway.
	// - if it does not have a "mute" config entry, the sound is unmuted currently and should be muted now.
	bool mute = true;

	if (ConfMan.hasKey("mute")) {
		mute = !ConfMan.getBool("mute");
	}

	ConfMan.setBool("mute", mute);

	syncSoundSettings();
}

Common::Error Engine::loadGameState(int slot) {
	// In case autosaves are on, do a save first before loading the new save
	saveAutosaveIfEnabled();

	Common::InSaveFile *saveFile = _saveFileMan->openForLoading(getSaveStateName(slot));

	if (!saveFile)
		return Common::kReadingFailed;

	Common::Error result = loadGameStream(saveFile);
	if (result.getCode() == Common::kNoError) {
		ExtendedSavegameHeader header;
		if (MetaEngine::readSavegameHeader(saveFile, &header))
			setTotalPlayTime(header.playtime);
	}

	delete saveFile;
	return result;
}

Common::Error Engine::loadGameStream(Common::SeekableReadStream *stream) {
	// Default to returning an error when not implemented
	return Common::kReadingFailed;
}

bool Engine::canLoadGameStateCurrently() {
	// Do not allow loading by default
	return false;
}

Common::Error Engine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	Common::OutSaveFile *saveFile = _saveFileMan->openForSaving(getSaveStateName(slot));

	if (!saveFile)
		return Common::kWritingFailed;

	Common::Error result = saveGameStream(saveFile, isAutosave);
	if (result.getCode() == Common::kNoError) {
		MetaEngine::appendExtendedSave(saveFile, getTotalPlayTime() / 1000, desc, isAutosave);

		saveFile->finalize();
	}

	delete saveFile;
	return result;
}

Common::Error Engine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	// Default to returning an error when not implemented
	return Common::kWritingFailed;
}

bool Engine::canSaveGameStateCurrently() {
	// Do not allow saving by default
	return false;
}

bool Engine::loadGameDialog() {
	if (!canLoadGameStateCurrently()) {
		g_system->displayMessageOnOSD(_("Loading game is currently unavailable"));
		return false;
	}

	GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Load game:"), _("Load"), false);

	int slotNum;
	{
		PauseToken pt = pauseEngine();
		slotNum = dialog->runModalWithCurrentTarget();
	}

	delete dialog;

	if (slotNum < 0)
		return false;

	Common::Error loadError = loadGameState(slotNum);
	if (loadError.getCode() != Common::kNoError) {
		GUI::MessageDialog errorDialog(loadError.getDesc());
		errorDialog.runModal();
		return false;
	}

	return true;
}

bool Engine::saveGameDialog() {
	if (!canSaveGameStateCurrently()) {
		g_system->displayMessageOnOSD(_("Saving game is currently unavailable"));
		return false;
	}

	GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);
	int slotNum;
	{
		PauseToken pt = pauseEngine();
		slotNum = dialog->runModalWithCurrentTarget();
	}

	Common::String desc = dialog->getResultString();
	if (desc.empty())
		desc = dialog->createDefaultSaveDescription(slotNum);

	delete dialog;

	if (slotNum < 0)
		return false;

	Common::Error saveError = saveGameState(slotNum, desc);
	if (saveError.getCode() != Common::kNoError) {
		GUI::MessageDialog errorDialog(saveError.getDesc());
		errorDialog.runModal();
		return false;
	}

	return true;
}

void Engine::quitGame() {
	Common::Event event;

	event.type = Common::EVENT_QUIT;
	g_system->getEventManager()->pushEvent(event);
}

bool Engine::shouldQuit() {
	Common::EventManager *eventMan = g_system->getEventManager();
	return (eventMan->shouldQuit() || eventMan->shouldReturnToLauncher());
}

GUI::Debugger *Engine::getOrCreateDebugger() {
	if (!_debugger)
		// Create a bare-bones debugger. This is useful for engines without their own
		// debugger when an error occurs
		_debugger = new GUI::Debugger();

	return _debugger;
}

/*
EnginePlugin *Engine::getMetaEnginePlugin() const {
	return EngineMan.findPlugin(ConfMan.get("engineid"));
}

*/

MetaEngineDetection &Engine::getMetaEngineDetection() {
	const Plugin *plugin = EngineMan.findPlugin(ConfMan.get("engineid"));
	assert(plugin);
	return plugin->get<MetaEngineDetection>();
}

MetaEngine &Engine::getMetaEngine() {
	const Plugin *metaEnginePlugin = EngineMan.findPlugin(ConfMan.get("engineid"));
	assert(metaEnginePlugin);

	const Plugin *enginePlugin = PluginMan.getEngineFromMetaEngine(metaEnginePlugin);
	assert(enginePlugin);

	return enginePlugin->get<MetaEngine>();
}

PauseToken::PauseToken() : _engine(nullptr) {}

PauseToken::PauseToken(Engine *engine) : _engine(engine) {}

void PauseToken::operator=(const PauseToken &t2) {
	if (_engine) {
		error("Tried to assign to an already busy PauseToken");
	}
	_engine = t2._engine;
	if (_engine) {
		_engine->_pauseLevel++;
	}
}

PauseToken::PauseToken(const PauseToken &t2) : _engine(t2._engine) {
	if (_engine) {
		_engine->_pauseLevel++;
	}
}

void PauseToken::clear() {
	if (!_engine) {
		error("Tried to clear an already cleared PauseToken");
	}
	_engine->resumeEngine();
	_engine = nullptr;
}

PauseToken::~PauseToken() {
	if (_engine) {
		_engine->resumeEngine();
	}
}

#if __cplusplus >= 201103L
PauseToken::PauseToken(PauseToken &&t2) : _engine(t2._engine) {
	t2._engine = nullptr;
}

void PauseToken::operator=(PauseToken &&t2) {
	if (_engine) {
		error("Tried to assign to an already busy PauseToken");
	}
	_engine = t2._engine;
	t2._engine = nullptr;
}
#endif
