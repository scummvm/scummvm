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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/platform/sdl/sdl.h"
#include "common/config-manager.h"
#include "gui/EventRecorder.h"
#include "common/taskbar.h"
#include "common/textconsole.h"
#include "common/translation.h"
#include "common/encoding.h"

#include "backends/saves/default/default-saves.h"

// Audio CD support was removed with SDL 2.0
#if SDL_VERSION_ATLEAST(2, 0, 0)
#include "backends/audiocd/default/default-audiocd.h"
#else
#include "backends/audiocd/sdl/sdl-audiocd.h"
#endif

#include "backends/events/default/default-events.h"
#include "backends/events/sdl/sdl-events.h"
#include "backends/mutex/sdl/sdl-mutex.h"
#include "backends/timer/sdl/sdl-timer.h"
#include "backends/graphics/surfacesdl/surfacesdl-graphics.h"
#ifdef USE_OPENGL
#include "backends/graphics/openglsdl/openglsdl-graphics.h"
#include "graphics/cursorman.h"
#endif

#include <time.h>	// for getTimeAndDate()

#ifdef USE_DETECTLANG
#ifndef WIN32
#include <locale.h>
#endif // !WIN32
#endif

#ifdef USE_SDL_NET
#include <SDL_net.h>
#endif

#if SDL_VERSION_ATLEAST(2, 0, 0)
#include <SDL_clipboard.h>
#endif

OSystem_SDL::OSystem_SDL()
	:
#ifdef USE_OPENGL
	_desktopWidth(0),
	_desktopHeight(0),
	_graphicsModes(),
	_graphicsMode(0),
	_firstGLMode(0),
	_defaultSDLMode(0),
	_defaultGLMode(0),
#endif
	_inited(false),
	_initedSDL(false),
#ifdef USE_SDL_NET
	_initedSDLnet(false),
#endif
	_logger(0),
	_mixerManager(0),
	_eventSource(0),
	_window(0) {

	ConfMan.registerDefault("kbdmouse_speed", 3);
	ConfMan.registerDefault("joystick_deadzone", 3);
}

OSystem_SDL::~OSystem_SDL() {
	SDL_ShowCursor(SDL_ENABLE);

	// Delete the various managers here. Note that the ModularBackend
	// destructor would also take care of this for us. However, various
	// of our managers must be deleted *before* we call SDL_Quit().
	// Hence, we perform the destruction on our own.
	delete _savefileManager;
	_savefileManager = 0;
	if (_graphicsManager) {
		dynamic_cast<SdlGraphicsManager *>(_graphicsManager)->deactivateManager();
	}
	delete _graphicsManager;
	_graphicsManager = 0;
	delete _window;
	_window = 0;
	delete _eventManager;
	_eventManager = 0;
	delete _eventSource;
	_eventSource = 0;
	delete _audiocdManager;
	_audiocdManager = 0;
	delete _mixerManager;
	_mixerManager = 0;

#ifdef ENABLE_EVENTRECORDER
	// HACK HACK HACK
	// This is nasty.
	delete g_eventRec.getTimerManager();
#else
	delete _timerManager;
#endif

	_timerManager = 0;
	delete _mutexManager;
	_mutexManager = 0;

	delete _logger;
	_logger = 0;

#ifdef USE_SDL_NET
	if (_initedSDLnet) SDLNet_Quit();
#endif

	SDL_Quit();
}

void OSystem_SDL::init() {
	// Initialize SDL
	initSDL();

#if !SDL_VERSION_ATLEAST(2, 0, 0)
	// Enable unicode support if possible
	SDL_EnableUNICODE(1);
#endif

	// Disable OS cursor
	SDL_ShowCursor(SDL_DISABLE);

	if (!_logger)
		_logger = new Backends::Log::Log(this);

	if (_logger) {
		Common::WriteStream *logFile = createLogFile();
		if (logFile)
			_logger->open(logFile);
	}


	// Creates the early needed managers, if they don't exist yet
	// (we check for this to allow subclasses to provide their own).
	if (_mutexManager == 0)
		_mutexManager = new SdlMutexManager();

	if (_window == 0)
		_window = new SdlWindow();

#if defined(USE_TASKBAR)
	if (_taskbarManager == 0)
		_taskbarManager = new Common::TaskbarManager();
#endif

}

bool OSystem_SDL::hasFeature(Feature f) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (f == kFeatureClipboardSupport) return true;
#endif
	if (f == kFeatureJoystickDeadzone || f == kFeatureKbdMouseSpeed) {
		bool joystickSupportEnabled = ConfMan.getInt("joystick_num") >= 0;
		return joystickSupportEnabled;
	}
	return ModularBackend::hasFeature(f);
}

void OSystem_SDL::initBackend() {
	// Check if backend has not been initialized
	assert(!_inited);

#if SDL_VERSION_ATLEAST(2, 0, 0)
	const char *sdlDriverName = SDL_GetCurrentVideoDriver();
#else
	const int maxNameLen = 20;
	char sdlDriverName[maxNameLen];
	sdlDriverName[0] = '\0';
	SDL_VideoDriverName(sdlDriverName, maxNameLen);
#endif
	debug(1, "Using SDL Video Driver \"%s\"", sdlDriverName);

	// Create the default event source, in case a custom backend
	// manager didn't provide one yet.
	if (_eventSource == 0)
		_eventSource = new SdlEventSource();

	if (_eventManager == nullptr) {
		DefaultEventManager *eventManager = new DefaultEventManager(_eventSource);
#if SDL_VERSION_ATLEAST(2, 0, 0)
		// SDL 2 generates its own keyboard repeat events.
		eventManager->setGenerateKeyRepeatEvents(false);
#endif
		_eventManager = eventManager;
	}


#ifdef USE_OPENGL
#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_DisplayMode displayMode;
	if (!SDL_GetDesktopDisplayMode(0, &displayMode)) {
		_desktopWidth  = displayMode.w;
		_desktopHeight = displayMode.h;
	}
#else
	// Query the desktop resolution. We simply hope nothing tried to change
	// the resolution so far.
	const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo();
	if (videoInfo && videoInfo->current_w > 0 && videoInfo->current_h > 0) {
		_desktopWidth  = videoInfo->current_w;
		_desktopHeight = videoInfo->current_h;
	}
#endif
#endif

	if (_graphicsManager == 0) {
#ifdef USE_OPENGL
		// Setup a list with both SDL and OpenGL graphics modes. We only do
		// this whenever the subclass did not already set up an graphics
		// manager yet. This is because we don't know the type of the graphics
		// manager of the subclass, thus we cannot easily switch between the
		// OpenGL one and the set up one. It also is to be expected that the
		// subclass does not want any switching of graphics managers anyway.
		setupGraphicsModes();

		if (ConfMan.hasKey("gfx_mode")) {
			// If the gfx_mode is from OpenGL, create the OpenGL graphics manager
			Common::String gfxMode(ConfMan.get("gfx_mode"));
			for (uint i = _firstGLMode; i < _graphicsModeIds.size(); ++i) {
				if (!scumm_stricmp(_graphicsModes[i].name, gfxMode.c_str())) {
					_graphicsManager = new OpenGLSdlGraphicsManager(_desktopWidth, _desktopHeight, _eventSource, _window);
					_graphicsMode = i;
					break;
				}
			}
		}
#endif

		if (_graphicsManager == 0) {
			_graphicsManager = new SurfaceSdlGraphicsManager(_eventSource, _window);
		}
	}

	if (_savefileManager == 0)
		_savefileManager = new DefaultSaveFileManager();

	if (_mixerManager == 0) {
		_mixerManager = new SdlMixerManager();
		// Setup and start mixer
		_mixerManager->init();
	}

#ifdef ENABLE_EVENTRECORDER
	g_eventRec.registerMixerManager(_mixerManager);

	g_eventRec.registerTimerManager(new SdlTimerManager());
#else
	if (_timerManager == 0)
		_timerManager = new SdlTimerManager();
#endif

	_audiocdManager = createAudioCDManager();

	// Setup a custom program icon.
	_window->setupIcon();

	_inited = true;

	ModularBackend::initBackend();

	// We have to initialize the graphics manager before the event manager
	// so the virtual keyboard can be initialized, but we have to add the
	// graphics manager as an event observer after initializing the event
	// manager.
	dynamic_cast<SdlGraphicsManager *>(_graphicsManager)->activateManager();
}

void OSystem_SDL::engineInit() {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	dynamic_cast<SdlGraphicsManager *>(_graphicsManager)->unlockWindowSize();
#endif
#ifdef USE_TASKBAR
	// Add the started engine to the list of recent tasks
	_taskbarManager->addRecent(ConfMan.getActiveDomainName(), ConfMan.get("description"));

	// Set the overlay icon the current running engine
	_taskbarManager->setOverlayIcon(ConfMan.getActiveDomainName(), ConfMan.get("description"));
#endif
}

void OSystem_SDL::engineDone() {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	dynamic_cast<SdlGraphicsManager *>(_graphicsManager)->unlockWindowSize();
#endif
#ifdef USE_TASKBAR
	// Remove overlay icon
	_taskbarManager->setOverlayIcon("", "");
#endif
}

void OSystem_SDL::initSDL() {
	// Check if SDL has not been initialized
	if (!_initedSDL) {
		// We always initialize the video subsystem because we will need it to
		// be initialized before the graphics managers to retrieve the desktop
		// resolution, for example. WebOS also requires this initialization
		// or otherwise the application won't start.
		uint32 sdlFlags = SDL_INIT_VIDEO;

		if (ConfMan.hasKey("disable_sdl_parachute"))
			sdlFlags |= SDL_INIT_NOPARACHUTE;

		// Initialize SDL (SDL Subsystems are initiliazed in the corresponding sdl managers)
		if (SDL_Init(sdlFlags) == -1)
			error("Could not initialize SDL: %s", SDL_GetError());

		_initedSDL = true;
	}

#ifdef USE_SDL_NET
	// Check if SDL_net has not been initialized
	if (!_initedSDLnet) {
		// Initialize SDL_net
		if (SDLNet_Init() == -1)
			error("Could not initialize SDL_net: %s", SDLNet_GetError());

		_initedSDLnet = true;
	}
#endif
}

void OSystem_SDL::addSysArchivesToSearchSet(Common::SearchSet &s, int priority) {

#ifdef DATA_PATH
	// Add the global DATA_PATH to the directory search list
	// FIXME: We use depth = 4 for now, to match the old code. May want to change that
	Common::FSNode dataNode(DATA_PATH);
	if (dataNode.exists() && dataNode.isDirectory()) {
		s.add(DATA_PATH, new Common::FSDirectory(dataNode, 4), priority);
	}
#endif

}

void OSystem_SDL::setWindowCaption(const char *caption) {
	Common::String cap;
	byte c;

	// The string caption is supposed to be in LATIN-1 encoding.
	// SDL expects UTF-8. So we perform the conversion here.
	while ((c = *(const byte *)caption++)) {
		if (c < 0x80)
			cap += c;
		else {
			cap += 0xC0 | (c >> 6);
			cap += 0x80 | (c & 0x3F);
		}
	}

	_window->setWindowCaption(cap);
}

void OSystem_SDL::quit() {
	destroy();
	exit(0);
}

void OSystem_SDL::fatalError() {
	destroy();
	exit(1);
}


void OSystem_SDL::logMessage(LogMessageType::Type type, const char *message) {
	// First log to stdout/stderr
	FILE *output = 0;

	if (type == LogMessageType::kInfo || type == LogMessageType::kDebug)
		output = stdout;
	else
		output = stderr;

	fputs(message, output);
	fflush(output);

	// Then log into file (via the logger)
	if (_logger)
		_logger->print(message);
}

Common::WriteStream *OSystem_SDL::createLogFile() {
	// Start out by resetting _logFilePath, so that in case
	// of a failure, we know that no log file is open.
	_logFilePath.clear();

	Common::String logFile = getDefaultLogFileName();
	if (logFile.empty())
		return nullptr;

	Common::FSNode file(logFile);
	Common::WriteStream *stream = file.createWriteStream();
	if (stream)
		_logFilePath = logFile;
	return stream;
}

Common::String OSystem_SDL::getSystemLanguage() const {
#if defined(USE_DETECTLANG) && !defined(WIN32)
	// Activating current locale settings
	const Common::String locale = setlocale(LC_ALL, "");

	// Restore default C locale to prevent issues with
	// portability of sscanf(), atof(), etc.
	// See bug #3615148
	setlocale(LC_ALL, "C");

	// Detect the language from the locale
	if (locale.empty()) {
		return ModularBackend::getSystemLanguage();
	} else {
		int length = 0;

		// Strip out additional information, like
		// ".UTF-8" or the like. We do this, since
		// our translation languages are usually
		// specified without any charset information.
		for (int size = locale.size(); length < size; ++length) {
			// TODO: Check whether "@" should really be checked
			// here.
			if (locale[length] == '.' || locale[length] == ' ' || locale[length] == '@')
				break;
		}

		return Common::String(locale.c_str(), length);
	}
#else // USE_DETECTLANG
	return ModularBackend::getSystemLanguage();
#endif // USE_DETECTLANG
}

bool OSystem_SDL::hasTextInClipboard() {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	return SDL_HasClipboardText() == SDL_TRUE;
#else
	return false;
#endif
}

Common::String OSystem_SDL::getTextFromClipboard() {
	if (!hasTextInClipboard()) return "";

#if SDL_VERSION_ATLEAST(2, 0, 0)
	char *text = SDL_GetClipboardText();
	// The string returned by SDL is in UTF-8. Convert to the
	// current TranslationManager encoding or ISO-8859-1.
#ifdef USE_TRANSLATION
	char *conv_text = SDL_iconv_string(TransMan.getCurrentCharset().c_str(), "UTF-8", text, SDL_strlen(text) + 1);
#else
	char *conv_text = SDL_iconv_string("ISO-8859-1", "UTF-8", text, SDL_strlen(text) + 1);
#endif
	if (conv_text) {
		SDL_free(text);
		text = conv_text;
	}
	Common::String strText = text;
	SDL_free(text);

	return strText;
#else
	return "";
#endif
}

bool OSystem_SDL::setTextInClipboard(const Common::String &text) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	// The encoding we need to use is UTF-8. Assume we currently have the
	// current TranslationManager encoding or ISO-8859-1.
#ifdef USE_TRANSLATION
	char *utf8_text = SDL_iconv_string("UTF-8", TransMan.getCurrentCharset().c_str(), text.c_str(), text.size() + 1);
#else
	char *utf8_text = SDL_iconv_string("UTF-8", "ISO-8859-1", text.c_str(), text.size() + 1);
#endif
	if (utf8_text) {
		int status = SDL_SetClipboardText(utf8_text);
		SDL_free(utf8_text);
		return status == 0;
	}
	return SDL_SetClipboardText(text.c_str()) == 0;
#else
	return false;
#endif
}

uint32 OSystem_SDL::getMillis(bool skipRecord) {
	uint32 millis = SDL_GetTicks();

#ifdef ENABLE_EVENTRECORDER
	g_eventRec.processMillis(millis, skipRecord);
#endif

	return millis;
}

void OSystem_SDL::delayMillis(uint msecs) {
#ifdef ENABLE_EVENTRECORDER
	if (!g_eventRec.processDelayMillis())
#endif
		SDL_Delay(msecs);
}

void OSystem_SDL::getTimeAndDate(TimeDate &td) const {
	time_t curTime = time(0);
	struct tm t = *localtime(&curTime);
	td.tm_sec = t.tm_sec;
	td.tm_min = t.tm_min;
	td.tm_hour = t.tm_hour;
	td.tm_mday = t.tm_mday;
	td.tm_mon = t.tm_mon;
	td.tm_year = t.tm_year;
	td.tm_wday = t.tm_wday;
}

Audio::Mixer *OSystem_SDL::getMixer() {
	assert(_mixerManager);
	return getMixerManager()->getMixer();
}

SdlMixerManager *OSystem_SDL::getMixerManager() {
	assert(_mixerManager);

#ifdef ENABLE_EVENTRECORDER
	return g_eventRec.getMixerManager();
#else
	return _mixerManager;
#endif
}

Common::TimerManager *OSystem_SDL::getTimerManager() {
#ifdef ENABLE_EVENTRECORDER
	return g_eventRec.getTimerManager();
#else
	return _timerManager;
#endif
}

AudioCDManager *OSystem_SDL::createAudioCDManager() {
	// Audio CD support was removed with SDL 2.0
#if SDL_VERSION_ATLEAST(2, 0, 0)
	return new DefaultAudioCDManager();
#else
	return new SdlAudioCDManager();
#endif
}

Common::SaveFileManager *OSystem_SDL::getSavefileManager() {
#ifdef ENABLE_EVENTRECORDER
    return g_eventRec.getSaveManager(_savefileManager);
#else
    return _savefileManager;
#endif
}

//Not specified in base class
Common::String OSystem_SDL::getScreenshotsPath() {
	Common::String path = ConfMan.get("screenshotpath");
	if (!path.empty() && !path.hasSuffix("/"))
		path += "/";
	return path;
}

#ifdef USE_OPENGL

const OSystem::GraphicsMode *OSystem_SDL::getSupportedGraphicsModes() const {
	if (_graphicsModes.empty()) {
		return _graphicsManager->getSupportedGraphicsModes();
	} else {
		return _graphicsModes.begin();
	}
}

int OSystem_SDL::getDefaultGraphicsMode() const {
	if (_graphicsModes.empty()) {
		return _graphicsManager->getDefaultGraphicsMode();
	} else {
		// Return the default graphics mode from the current graphics manager
		if (_graphicsMode < _firstGLMode)
			return _defaultSDLMode;
		else
			return _defaultGLMode;
	}
}

bool OSystem_SDL::setGraphicsMode(int mode) {
	if (_graphicsModes.empty()) {
		return _graphicsManager->setGraphicsMode(mode);
	}

	// Check whether a invalid mode is requested.
	if (mode < 0 || (uint)mode >= _graphicsModeIds.size()) {
		return false;
	}

	// Very hacky way to set up the old graphics manager state, in case we
	// switch from SDL->OpenGL or OpenGL->SDL.
	//
	// This is a probably temporary workaround to fix bugs like #3368143
	// "SDL/OpenGL: Crash when switching renderer backend".
	SdlGraphicsManager *sdlGraphicsManager = dynamic_cast<SdlGraphicsManager *>(_graphicsManager);
	SdlGraphicsManager::State state = sdlGraphicsManager->getState();

	bool switchedManager = false;

	// If the new mode and the current mode are not from the same graphics
	// manager, delete and create the new mode graphics manager
	if (_graphicsMode >= _firstGLMode && mode < _firstGLMode) {
		debug(1, "switching to plain SDL graphics");
		sdlGraphicsManager->deactivateManager();
		delete _graphicsManager;
		_graphicsManager = sdlGraphicsManager = new SurfaceSdlGraphicsManager(_eventSource, _window);

		switchedManager = true;
	} else if (_graphicsMode < _firstGLMode && mode >= _firstGLMode) {
		debug(1, "switching to OpenGL graphics");
		sdlGraphicsManager->deactivateManager();
		delete _graphicsManager;
		_graphicsManager = sdlGraphicsManager = new OpenGLSdlGraphicsManager(_desktopWidth, _desktopHeight, _eventSource, _window);

		switchedManager = true;
	}

	_graphicsMode = mode;

	if (switchedManager) {
		sdlGraphicsManager->activateManager();

		// This failing will probably have bad consequences...
		if (!sdlGraphicsManager->setState(state)) {
			return false;
		}

		// Next setup the cursor again
		CursorMan.pushCursor(0, 0, 0, 0, 0, 0);
		CursorMan.popCursor();

		// Next setup cursor palette if needed
		if (_graphicsManager->getFeatureState(kFeatureCursorPalette)) {
			CursorMan.pushCursorPalette(0, 0, 0);
			CursorMan.popCursorPalette();
		}

		_graphicsManager->beginGFXTransaction();
		// Oh my god if this failed the client code might just explode.
		return _graphicsManager->setGraphicsMode(_graphicsModeIds[mode]);
	} else {
		return _graphicsManager->setGraphicsMode(_graphicsModeIds[mode]);
	}
}

int OSystem_SDL::getGraphicsMode() const {
	if (_graphicsModes.empty()) {
		return _graphicsManager->getGraphicsMode();
	} else {
		return _graphicsMode;
	}
}

void OSystem_SDL::setupGraphicsModes() {
	_graphicsModes.clear();
	_graphicsModeIds.clear();
	_defaultSDLMode = _defaultGLMode = -1;

	// Count the number of graphics modes
	const OSystem::GraphicsMode *srcMode;
	int defaultMode;

	GraphicsManager *manager = new SurfaceSdlGraphicsManager(_eventSource, _window);
	srcMode = manager->getSupportedGraphicsModes();
	defaultMode = manager->getDefaultGraphicsMode();
	while (srcMode->name) {
		if (defaultMode == srcMode->id) {
			_defaultSDLMode = _graphicsModes.size();
		}
		_graphicsModes.push_back(*srcMode);
		srcMode++;
	}
	delete manager;
	assert(_defaultSDLMode != -1);

	_firstGLMode = _graphicsModes.size();
	manager = new OpenGLSdlGraphicsManager(_desktopWidth, _desktopHeight, _eventSource, _window);
	srcMode = manager->getSupportedGraphicsModes();
	defaultMode = manager->getDefaultGraphicsMode();
	while (srcMode->name) {
		if (defaultMode == srcMode->id) {
			_defaultGLMode = _graphicsModes.size();
		}
		_graphicsModes.push_back(*srcMode);
		srcMode++;
	}
	delete manager;
	manager = nullptr;
	assert(_defaultGLMode != -1);

	// Set a null mode at the end
	GraphicsMode nullMode;
	memset(&nullMode, 0, sizeof(nullMode));
	_graphicsModes.push_back(nullMode);

	// Set new internal ids for all modes
	int i = 0;
	OSystem::GraphicsMode *mode = _graphicsModes.begin();
	while (mode->name) {
		_graphicsModeIds.push_back(mode->id);
		mode->id = i++;
		mode++;
	}
}
#endif

#if SDL_VERSION_ATLEAST(2, 0, 0)
int SDL_SetColors(SDL_Surface *surface, SDL_Color *colors, int firstcolor, int ncolors) {
	if (surface->format->palette) {
		return !SDL_SetPaletteColors(surface->format->palette, colors, firstcolor, ncolors) ? 1 : 0;
	} else {
		return 0;
	}
}

int SDL_SetAlpha(SDL_Surface *surface, Uint32 flag, Uint8 alpha) {
	if (SDL_SetSurfaceAlphaMod(surface, alpha)) {
		return -1;
	}

	if (alpha == 255 || !flag) {
		if (SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE)) {
			return -1;
		}
	} else {
		if (SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND)) {
			return -1;
		}
	}

	return 0;
}

#undef SDL_SetColorKey
int SDL_SetColorKey_replacement(SDL_Surface *surface, Uint32 flag, Uint32 key) {
	return SDL_SetColorKey(surface, SDL_TRUE, key) ? -1 : 0;
}
#endif

char *OSystem_SDL::convertEncoding(const char *to, const char *from, const char *string, size_t length) {
	int zeroBytes = 1;
	if (Common::String(from).hasPrefixIgnoreCase("utf-16"))
		zeroBytes = 2;
	else if (Common::String(from).hasPrefixIgnoreCase("utf-32"))
		zeroBytes = 4;

	char *result;
	// SDL_iconv_string() takes char * instead of const char * as it's third parameter
	// with some older versions of SDL.
#if SDL_VERSION_ATLEAST(2, 0, 0)
	result = SDL_iconv_string(to, from, string, length + zeroBytes);
#else
	char *stringCopy = (char *) calloc(sizeof(char), length + zeroBytes);
	memcpy(stringCopy, string, length);
	result = SDL_iconv_string(to, from, stringCopy, length + zeroBytes);
	free(stringCopy);
#endif
	if (result == nullptr)
		return nullptr;

	// We need to copy the result, so that we can use SDL_free()
	// on the string returned by SDL_iconv_string() and free()
	// can then be used on the copyed and returned string.
	// Sometimes free() and SDL_free() aren't compatible and
	// using free() instead of SDL_free() can cause crashes.
	size_t newLength = Common::Encoding::stringLength(result, to);
	zeroBytes = 1;
	if (Common::String(to).hasPrefixIgnoreCase("utf-16"))
		zeroBytes = 2;
	else if (Common::String(to).hasPrefixIgnoreCase("utf-32"))
		zeroBytes = 4;
	char *finalResult = (char *) malloc(newLength + zeroBytes);
	if (!finalResult) {
		warning("Could not allocate memory for encoding conversion");
		SDL_free(result);
		return nullptr;
	}
	memcpy(finalResult, result, newLength + zeroBytes);
	SDL_free(result);
	return finalResult;
}

