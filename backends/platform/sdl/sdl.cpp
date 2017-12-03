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

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef ARRAYSIZE // winnt.h defines ARRAYSIZE, but we want our own one...
#endif

#include "backends/platform/sdl/sdl.h"
#include "common/config-manager.h"
#include "gui/EventRecorder.h"
#include "common/taskbar.h"
#include "common/textconsole.h"

#include "backends/saves/default/default-saves.h"

// Audio CD support was removed with SDL 2.0
#if SDL_VERSION_ATLEAST(2, 0, 0)
#include "backends/audiocd/default/default-audiocd.h"
#else
#include "backends/audiocd/sdl/sdl-audiocd.h"
#endif
// ResidualVM:
// #include "backends/events/sdl/sdl-events.h"
#include "backends/events/sdl/resvm-sdl-events.h"
#include "backends/mutex/sdl/sdl-mutex.h"
#include "backends/timer/sdl/sdl-timer.h"
#include "backends/graphics/surfacesdl/surfacesdl-graphics.h"

#ifdef USE_OPENGL
#include "backends/graphics/openglsdl/openglsdl-graphics.h"
//#include "graphics/cursorman.h" // ResidualVM
#include "graphics/opengl/context.h" // ResidualVM specific
#endif
#include "graphics/renderer.h" // ResidualVM specific

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
	_inited(false),
	_initedSDL(false),
#ifdef USE_SDL_NET
	_initedSDLnet(false),
#endif
	_logger(0),
	_mixerManager(0),
	_eventSource(0),
	_window(0) {

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
#ifdef JOY_ANALOG
	if (f == kFeatureJoystickDeadzone) return true;
#endif
	if (f == kFeatureKbdMouseSpeed) return true;

	// ResidualVM specific code:
	if (f == kFeatureSideTextures)
		return true;

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
	// Using printf rather than debug() here as debug()/logging
	// is not active by this point.
	debug(1, "Using SDL Video Driver \"%s\"", sdlDriverName);

// ResidualVM specific code start
	detectDesktopResolution();
#ifdef USE_OPENGL
	detectFramebufferSupport();
#endif
// ResidualVM specific code end

	// Create the default event source, in case a custom backend
	// manager didn't provide one yet.
	if (_eventSource == 0)
		_eventSource = new ResVmSdlEventSource(); // ResidualVm: was SdlEventSource

	if (_graphicsManager == 0) {
		if (_graphicsManager == 0) {
			_graphicsManager = new SurfaceSdlGraphicsManager(_eventSource, _window, _capabilities);
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

	if (!ConfMan.hasKey("kbdmouse_speed")) {
		ConfMan.registerDefault("kbdmouse_speed", 3);
		ConfMan.setInt("kbdmouse_speed", 3);
	}
#ifdef JOY_ANALOG
	if (!ConfMan.hasKey("joystick_deadzone")) {
		ConfMan.registerDefault("joystick_deadzone", 3);
		ConfMan.setInt("joystick_deadzone", 3);
	}
#endif
	ModularBackend::initBackend();

	// We have to initialize the graphics manager before the event manager
	// so the virtual keyboard can be initialized, but we have to add the
	// graphics manager as an event observer after initializing the event
	// manager.
	dynamic_cast<SdlGraphicsManager *>(_graphicsManager)->activateManager();
}

// ResidualVM specific code
void OSystem_SDL::detectDesktopResolution() {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_DisplayMode displayMode;
	if (!SDL_GetDesktopDisplayMode(0, &displayMode)) {
		_capabilities.desktopWidth = displayMode.w;
		_capabilities.desktopHeight = displayMode.h;
	}
#else
	// Query the desktop resolution. We simply hope nothing tried to change
	// the resolution so far.
	const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo();
	if (videoInfo && videoInfo->current_w > 0 && videoInfo->current_h > 0) {
		_capabilities.desktopWidth = videoInfo->current_w;
		_capabilities.desktopHeight = videoInfo->current_h;
	}
#endif
}

#ifdef USE_OPENGL
void OSystem_SDL::detectFramebufferSupport() {
	_capabilities.openGLFrameBuffer = false;
#if defined(USE_GLES2)
	// Framebuffers are always available with GLES2
	_capabilities.openGLFrameBuffer = true;
#elif !defined(AMIGAOS)
	// Spawn a 32x32 window off-screen with a GL context to test if framebuffers are supported
#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_Window *window = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 32, 32, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
	if (window) {
		SDL_GLContext glContext = SDL_GL_CreateContext(window);
		if (glContext) {
			OpenGLContext.initialize(OpenGL::kContextGL);
			_capabilities.openGLFrameBuffer = OpenGLContext.framebufferObjectSupported;
			OpenGLContext.reset();
			SDL_GL_DeleteContext(glContext);
		}
		SDL_DestroyWindow(window);
	}
#else
	SDL_putenv(const_cast<char *>("SDL_VIDEO_WINDOW_POS=9000,9000"));
	SDL_SetVideoMode(32, 32, 0, SDL_OPENGL);
	SDL_putenv(const_cast<char *>("SDL_VIDEO_WINDOW_POS=center"));
	OpenGLContext.initialize(OpenGL::kContextGL);
	_capabilities.openGLFrameBuffer = OpenGLContext.framebufferObjectSupported;
	OpenGLContext.reset();
#endif
#endif
}
#endif // USE_OPENGL
// End of ResidualVM specific code

void OSystem_SDL::engineInit() {
#ifdef USE_TASKBAR
	// Add the started engine to the list of recent tasks
	_taskbarManager->addRecent(ConfMan.getActiveDomainName(), ConfMan.get("description"));

	// Set the overlay icon the current running engine
	_taskbarManager->setOverlayIcon(ConfMan.getActiveDomainName(), ConfMan.get("description"));
#endif
}

void OSystem_SDL::engineDone() {
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

// ResidualVM specific code
void OSystem_SDL::setupScreen(uint screenW, uint screenH, bool fullscreen, bool accel3d) {
#ifdef USE_OPENGL
	bool switchedManager = false;
	if (accel3d && !dynamic_cast<OpenGLSdlGraphicsManager *>(_graphicsManager)) {
		switchedManager = true;
	} else if (!accel3d && !dynamic_cast<SurfaceSdlGraphicsManager *>(_graphicsManager)) {
		switchedManager = true;
	}

	if (switchedManager) {
		SdlGraphicsManager *sdlGraphicsManager = dynamic_cast<SdlGraphicsManager *>(_graphicsManager);
		sdlGraphicsManager->deactivateManager();
		delete _graphicsManager;

		if (accel3d) {
			_graphicsManager = sdlGraphicsManager = new OpenGLSdlGraphicsManager(_eventSource, _window, _capabilities);
		} else {
			_graphicsManager = sdlGraphicsManager = new SurfaceSdlGraphicsManager(_eventSource, _window, _capabilities);
		}
		sdlGraphicsManager->activateManager();
	}
#endif

	ModularBackend::setupScreen(screenW, screenH, fullscreen, accel3d);
}

void OSystem_SDL::launcherInitSize(uint w, uint h) {
	Common::String rendererConfig = ConfMan.get("renderer");
	Graphics::RendererType desiredRendererType = Graphics::parseRendererTypeCode(rendererConfig);
	Graphics::RendererType matchingRendererType = Graphics::getBestMatchingAvailableRendererType(desiredRendererType);

	bool fullscreen = ConfMan.getBool("fullscreen");

	setupScreen(w, h, fullscreen, matchingRendererType != Graphics::kRendererTypeTinyGL);
}
// End of ResidualVM specific code

void OSystem_SDL::quit() {
	delete this;
	exit(0);
}

void OSystem_SDL::fatalError() {
	delete this;
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

	// Finally, some Windows / WinCE specific logging code.
#if defined( USE_WINDBG )
#if defined( _WIN32_WCE )
	TCHAR buf_unicode[1024];
	MultiByteToWideChar(CP_ACP, 0, message, strlen(message) + 1, buf_unicode, sizeof(buf_unicode));
	OutputDebugString(buf_unicode);

	if (type == LogMessageType::kError) {
#ifndef DEBUG
		drawError(message);
#else
		int cmon_break_into_the_debugger_if_you_please = *(int *)(message + 1);	// bus error
		printf("%d", cmon_break_into_the_debugger_if_you_please);			// don't optimize the int out
#endif
	}

#else
	OutputDebugString(message);
#endif
#endif
}

Common::String OSystem_SDL::getSystemLanguage() const {
#if defined(USE_DETECTLANG) && !defined(_WIN32_WCE)
#ifdef WIN32
	// We can not use "setlocale" (at least not for MSVC builds), since it
	// will return locales like: "English_USA.1252", thus we need a special
	// way to determine the locale string for Win32.
	char langName[9];
	char ctryName[9];

	const LCID languageIdentifier = GetUserDefaultUILanguage();

	if (GetLocaleInfo(languageIdentifier, LOCALE_SISO639LANGNAME, langName, sizeof(langName)) != 0 &&
		GetLocaleInfo(languageIdentifier, LOCALE_SISO3166CTRYNAME, ctryName, sizeof(ctryName)) != 0) {
		Common::String localeName = langName;
		localeName += "_";
		localeName += ctryName;

		return localeName;
	} else {
		return ModularBackend::getSystemLanguage();
	}
#else // WIN32
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
#endif // WIN32
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
	Common::String strText = text;
	SDL_free(text);

	// FIXME: The string returned by SDL is in UTF-8, it is not clear
	// what encoding should be used for the returned string.
	return strText;
#else
	return "";
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
