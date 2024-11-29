/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/platform/sdl/sdl.h"
#include "common/config-manager.h"
#include "gui/EventRecorder.h"
#include "common/taskbar.h"
#include "common/textconsole.h"
#include "common/translation.h"

#ifdef USE_DISCORD
#include "backends/presence/discord/discord.h"
#endif

#include "backends/saves/default/default-saves.h"

// Audio CD support was removed with SDL 2.0
#if SDL_VERSION_ATLEAST(2, 0, 0)
#include "backends/audiocd/default/default-audiocd.h"
#else
#include "backends/audiocd/sdl/sdl-audiocd.h"
#endif

#include "backends/mixer/null/null-mixer.h"
#include "backends/events/default/default-events.h"
#include "backends/keymapper/hardware-input.h"
#include "backends/mutex/sdl/sdl-mutex.h"
#include "backends/timer/sdl/sdl-timer.h"
#include "backends/graphics/surfacesdl/surfacesdl-graphics.h"
#ifdef USE_OPENGL
#include "backends/graphics/openglsdl/openglsdl-graphics.h"
#endif
#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)
#include "backends/graphics3d/openglsdl/openglsdl-graphics3d.h"
#include "graphics/opengl/context.h"
#endif
#if defined(USE_SCUMMVMDLC) && defined(USE_LIBCURL)
#include "backends/dlc/scummvmcloud.h"
#endif
#include "graphics/cursorman.h"
#include "graphics/renderer.h"

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
#ifdef USE_MULTIPLE_RENDERERS
	_graphicsModes(),
	_graphicsMode(0),
#endif
	_inited(false),
	_initedSDL(false),
#ifdef USE_SDL_NET
	_initedSDLnet(false),
#endif
	_logger(nullptr),
	_eventSource(nullptr),
	_eventSourceWrapper(nullptr),
	_window(nullptr) {
#if defined(USE_SCUMMVMDLC) && defined(USE_LIBCURL)
	_dlcStore = new DLC::ScummVMCloud::ScummVMCloud();
#endif
}

OSystem_SDL::~OSystem_SDL() {
	SDL_ShowCursor(SDL_ENABLE);

#ifdef USE_MULTIPLE_RENDERERS
	clearGraphicsModes();
#endif

	// Delete the various managers here. Note that the ModularBackend
	// destructors would also take care of this for us. However, various
	// of our managers must be deleted *before* we call SDL_Quit().
	// Hence, we perform the destruction on our own.
	delete _savefileManager;
	_savefileManager = nullptr;
	if (_graphicsManager) {
		dynamic_cast<SdlGraphicsManager *>(_graphicsManager)->deactivateManager();
	}
	delete _graphicsManager;
	_graphicsManager = nullptr;
	delete _window;
	_window = nullptr;
	delete _eventManager;
	_eventManager = nullptr;
	delete _eventSourceWrapper;
	_eventSourceWrapper = nullptr;
	delete _eventSource;
	_eventSource = nullptr;
	delete _audiocdManager;
	_audiocdManager = nullptr;
	delete _mixerManager;
	_mixerManager = nullptr;

#ifdef ENABLE_EVENTRECORDER
	// HACK HACK HACK
	// This is nasty.
	delete g_eventRec.getTimerManager();
#else
	delete _timerManager;
#endif

	_timerManager = nullptr;

	delete _logger;
	_logger = nullptr;

#ifdef USE_DISCORD
	delete _presence;
	_presence = 0;
#endif

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

#if !defined(OPENPANDORA)
	// Disable OS cursor
	SDL_ShowCursor(SDL_DISABLE);
#endif

	if (_window == nullptr)
		_window = new SdlWindow();

#if defined(USE_TASKBAR)
	if (_taskbarManager == nullptr)
		_taskbarManager = new Common::TaskbarManager();
#endif

}

bool OSystem_SDL::hasFeature(Feature f) {
#if SDL_VERSION_ATLEAST(1, 2, 7)
	if (f == kFeatureCpuSSE2) return SDL_HasSSE2();
	if (f == kFeatureCpuAltivec) return SDL_HasAltiVec();
#endif
#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (f == kFeatureClipboardSupport) return true;
	if (f == kFeatureCpuSSE41) return SDL_HasSSE41();
#endif
#if SDL_VERSION_ATLEAST(2, 0, 4)
	if (f == kFeatureCpuAVX2) return SDL_HasAVX2();
#endif
#if SDL_VERSION_ATLEAST(2, 0, 6)
	if (f == kFeatureCpuNEON) return SDL_HasNEON();
#endif
#if SDL_VERSION_ATLEAST(2, 0, 14)
	if (f == kFeatureOpenUrl) return true;
#endif
	if (f == kFeatureJoystickDeadzone || f == kFeatureKbdMouseSpeed) {
		return _eventSource->isJoystickConnected();
	}
#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)
	/* Even if we are using the 2D graphics manager,
	 * we are at one initGraphics3d call of supporting OpenGL */
	if (f == kFeatureOpenGLForGame) return true;
	if (f == kFeatureShadersForGame) return _supportsShaders;
#endif
#if defined(USE_SCUMMVMDLC) && defined(USE_LIBCURL)
	if (f == kFeatureDLC) return true;
#endif
#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (f == kFeatureTouchpadMode) {
		return SDL_GetNumTouchDevices() > 0;
	}
#endif
	return ModularGraphicsBackend::hasFeature(f);
}


void OSystem_SDL::setFeatureState(Feature f, bool enable) {
	switch (f) {
	case kFeatureTouchpadMode:
		ConfMan.setBool("touchpad_mouse_mode", enable);
		break;
	default:
		ModularGraphicsBackend::setFeatureState(f, enable);
		break;
	}
}

bool OSystem_SDL::getFeatureState(Feature f) {
	switch (f) {
	case kFeatureTouchpadMode:
		return ConfMan.getBool("touchpad_mouse_mode");
		break;
	default:
		return ModularGraphicsBackend::getFeatureState(f);
		break;
	}
}

void OSystem_SDL::initBackend() {
	// Check if backend has not been initialized
	assert(!_inited);

	if (!_logger)
		_logger = new Backends::Log::Log(this);

	if (_logger) {
		Common::WriteStream *logFile = createLogFile();
		if (logFile)
			_logger->open(logFile);
	}

#if SDL_VERSION_ATLEAST(2, 0, 0)
	const char *sdlDriverName = SDL_GetCurrentVideoDriver();
	// Allow the screen to turn off
	SDL_EnableScreenSaver();
#else
	const int maxNameLen = 20;
	char sdlDriverName[maxNameLen];
	sdlDriverName[0] = '\0';
	SDL_VideoDriverName(sdlDriverName, maxNameLen);
#endif
	debug(1, "Using SDL Video Driver \"%s\"", sdlDriverName);

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)
	detectOpenGLFeaturesSupport();
	detectAntiAliasingSupport();
#endif

	// Create the default event source, in case a custom backend
	// manager didn't provide one yet.
	if (!_eventSource)
		_eventSource = new SdlEventSource();

#if !SDL_VERSION_ATLEAST(2, 0, 0)
	// SDL 1 does not generate its own keyboard repeat events.
	assert(!_eventSourceWrapper);
	_eventSourceWrapper = makeKeyboardRepeatingEventSource(_eventSource);
#endif

	if (!_eventManager) {
		_eventManager = new DefaultEventManager(_eventSourceWrapper ? _eventSourceWrapper : _eventSource);
	}

	// Search for legacy gfx_mode and replace it
	ScalerMan.updateOldSettings();

	if (_graphicsManager == nullptr) {
#ifdef USE_MULTIPLE_RENDERERS
		// Setup a list with all graphics modes. We only do this whenever the
		// subclass did not already set up a graphics manager yet. This is
		// because we don't know the type of the graphics manager of the subclass,
		// thus we cannot easily switch between the standard ones and the set up
		// one. It also is to be expected that the subclass does not want any
		// switching of graphics managers anyway.
		setupGraphicsModes();

		Common::String gfxMode(ConfMan.get("gfx_mode"));
		// "normal" and "default" are a special case for the default graphics mode.
		// See OSystem::setGraphicsMode(const char *name) implementation.
		if (gfxMode.empty() || !gfxMode.compareToIgnoreCase("normal") || !gfxMode.compareToIgnoreCase("default")) {
			GraphicsManagerType type = getDefaultGraphicsManager();
			assert(type >= GraphicsManagerSurfaceSDL && type < GraphicsManagerCount);
			_graphicsManager = createGraphicsManager(_eventSource, _window, type);
			_graphicsMode = _defaultMode[type];
		} else {
			// If the gfx_mode is from a specific graphics manager, create it
			for (uint i = 0; i < GraphicsManagerCount; ++i) {
				for (int j = _firstMode[i]; j <= _lastMode[i]; ++j) {
					if (!scumm_stricmp(_graphicsModes[j].name, gfxMode.c_str())) {
						_graphicsManager = createGraphicsManager(_eventSource, _window, (GraphicsManagerType)i);
						_graphicsMode = j;
						break;
					}
				}
			}
		}
#endif

		if (_graphicsManager == nullptr) {
			_graphicsManager = new SurfaceSdlGraphicsManager(_eventSource, _window);
		}
	}

	if (_savefileManager == nullptr)
		_savefileManager = new DefaultSaveFileManager();

	if (_mixerManager == nullptr) {
		_mixerManager = new SdlMixerManager();
		// Setup and start mixer
		_mixerManager->init();

		if (_mixerManager->getMixer() == nullptr) {
			// Audio was unavailable or disabled
			delete _mixerManager;
			_mixerManager = new NullMixerManager();
			_mixerManager->init();
		}
	}

#ifdef ENABLE_EVENTRECORDER
	g_eventRec.registerMixerManager(_mixerManager);

	g_eventRec.registerTimerManager(new SdlTimerManager());
#else
	if (_timerManager == nullptr)
		_timerManager = new SdlTimerManager();
#endif

	_audiocdManager = createAudioCDManager();

	// Setup a custom program icon.
	_window->setupIcon();

#ifdef USE_DISCORD
	_presence = new DiscordPresence();
#endif

	ConfMan.registerDefault("iconspath", this->getDefaultIconsPath());
	ConfMan.registerDefault("dlcspath", this->getDefaultDLCsPath());

	_inited = true;

	BaseBackend::initBackend();

	// We have to initialize the graphics manager before the event manager
	// so the virtual keyboard can be initialized, but we have to add the
	// graphics manager as an event observer after initializing the event
	// manager.
	dynamic_cast<SdlGraphicsManager *>(_graphicsManager)->activateManager();
}

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)
void OSystem_SDL::detectOpenGLFeaturesSupport() {
	_oglType = OpenGL::kContextNone;
	_supportsFrameBuffer = false;
	_supportsShaders = false;
#if USE_FORCED_GLES2
	// Framebuffers and shaders are always available with GLES2
	_oglType = OpenGL::kContextGLES2;
	_supportsFrameBuffer = true;
	_supportsShaders = true;
#else
	// Spawn a 32x32 window off-screen with a GL context to test if framebuffers are supported
#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_Window *window = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 32, 32, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
	if (!window) {
		return;
	}

	int glContextProfileMask, glContextMajor;
	if (SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &glContextProfileMask) != 0) {
		SDL_DestroyWindow(window);
		return;
	}
	if (glContextProfileMask == SDL_GL_CONTEXT_PROFILE_ES) {
		if (SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &glContextMajor) != 0) {
			SDL_DestroyWindow(window);
			return;
		}
		if (glContextMajor == 2) {
			_oglType = OpenGL::kContextGLES2;
		} else {
			SDL_DestroyWindow(window);
			return;
		}
	} else {
		_oglType = OpenGL::kContextGL;
	}
	SDL_GLContext glContext = SDL_GL_CreateContext(window);
	if (!glContext) {
		SDL_DestroyWindow(window);
		return;
	}

	OpenGLContext.initialize(_oglType);
	_supportsFrameBuffer = OpenGLContext.framebufferObjectSupported;
	_supportsShaders = OpenGLContext.enginesShadersSupported;
	OpenGLContext.reset();
	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(window);
#else
	SDL_putenv(const_cast<char *>("SDL_VIDEO_WINDOW_POS=9000,9000"));
	SDL_SetVideoMode(32, 32, 0, SDL_OPENGL);
	SDL_putenv(const_cast<char *>("SDL_VIDEO_WINDOW_POS=center"));
	// SDL 1.2 only supports OpenGL
	_oglType = OpenGL::kContextGL;
	OpenGLContext.initialize(_oglType);
	_supportsFrameBuffer = OpenGLContext.framebufferObjectSupported;
	_supportsShaders = OpenGLContext.enginesShadersSupported;
	OpenGLContext.reset();
#endif
#endif
}

void OSystem_SDL::detectAntiAliasingSupport() {
#ifndef NINTENDO_SWITCH
	_antiAliasLevels.clear();

	int requestedSamples = 2;
	while (requestedSamples <= 32) {
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, requestedSamples);

#if SDL_VERSION_ATLEAST(2, 0, 0)
		SDL_Window *window = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 32, 32, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
		if (window) {
			SDL_GLContext glContext = SDL_GL_CreateContext(window);
			if (glContext) {
				int actualSamples = 0;
				SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &actualSamples);

				if (actualSamples == requestedSamples) {
					_antiAliasLevels.push_back(requestedSamples);
				}

				SDL_GL_DeleteContext(glContext);
			}

			SDL_DestroyWindow(window);
		}
#else
		SDL_putenv(const_cast<char *>("SDL_VIDEO_WINDOW_POS=9000,9000"));
		SDL_SetVideoMode(32, 32, 0, SDL_OPENGL);
		SDL_putenv(const_cast<char *>("SDL_VIDEO_WINDOW_POS=center"));

		int actualSamples = 0;
		SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &actualSamples);

		if (actualSamples == requestedSamples) {
			_antiAliasLevels.push_back(requestedSamples);
		}
#endif

		requestedSamples *= 2;
	}

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
#endif
}

#endif // defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)

void OSystem_SDL::engineInit() {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (_graphicsManager) {
		dynamic_cast<SdlGraphicsManager *>(_graphicsManager)->unlockWindowSize();
	}
	// Disable screen saver when engine starts
	SDL_DisableScreenSaver();
#endif
#ifdef USE_TASKBAR
	// Add the started engine to the list of recent tasks
	_taskbarManager->addRecent(ConfMan.getActiveDomainName(), ConfMan.get("description"));

	// Set the overlay icon to the current running engine
	_taskbarManager->setOverlayIcon(ConfMan.getActiveDomainName(), ConfMan.get("description"));
#endif
#ifdef USE_DISCORD
	// Set the presence status to the current running engine
	Common::String qualifiedGameId = Common::String::format("%s-%s", ConfMan.get("engineid").c_str(), ConfMan.get("gameid").c_str());
	_presence->updateStatus(qualifiedGameId, ConfMan.get("description"));
#endif

	_eventSource->setEngineRunning(true);
}

void OSystem_SDL::engineDone() {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (_graphicsManager) {
		dynamic_cast<SdlGraphicsManager *>(_graphicsManager)->unlockWindowSize();
	}
	SDL_EnableScreenSaver();
#endif
#ifdef USE_TASKBAR
	// Remove overlay icon
	_taskbarManager->setOverlayIcon("", "");
#endif
#ifdef USE_DISCORD
	// Reset presence status
	_presence->updateStatus("", "");
#endif
	_eventSource->setEngineRunning(false);
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

		// Initialize SDL (SDL Subsystems are initialized in the corresponding sdl managers)
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

void OSystem_SDL::setWindowCaption(const Common::U32String &caption) {
	Common::String cap = caption.encode();
	_window->setWindowCaption(cap);
}

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)
Common::Array<uint> OSystem_SDL::getSupportedAntiAliasingLevels() const {
	return _antiAliasLevels;
}
#endif

#if defined(USE_OPENGL) && defined(USE_GLAD)
void *OSystem_SDL::getOpenGLProcAddress(const char *name) const {
	return SDL_GL_GetProcAddress(name);
}
#endif

void OSystem_SDL::quit() {
	destroy();
	exit(0);
}

void OSystem_SDL::fatalError() {
	destroy();
	exit(1);
}

Common::KeymapArray OSystem_SDL::getGlobalKeymaps() {
	Common::KeymapArray globalMaps = BaseBackend::getGlobalKeymaps();

	Common::Keymap *keymap = dynamic_cast<SdlGraphicsManager *>(_graphicsManager)->getKeymap();
	globalMaps.push_back(keymap);

	return globalMaps;
}

Common::HardwareInputSet *OSystem_SDL::getHardwareInputSet() {
	using namespace Common;

	CompositeHardwareInputSet *inputSet = new CompositeHardwareInputSet();
	inputSet->addHardwareInputSet(new MouseHardwareInputSet(defaultMouseButtons));
	inputSet->addHardwareInputSet(new KeyboardHardwareInputSet(defaultKeys, defaultModifiers));

	if (_eventSource->isJoystickConnected()) {
		inputSet->addHardwareInputSet(new JoystickHardwareInputSet(defaultJoystickButtons, defaultJoystickAxes));
	}

	return inputSet;
}

void OSystem_SDL::logMessage(LogMessageType::Type type, const char *message) {
	// First log to stdout/stderr
	FILE *output = nullptr;

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

	Common::Path logFile;
	if (ConfMan.hasKey("logfile"))
		logFile = ConfMan.getPath("logfile");
	else
		logFile = getDefaultLogFileName();
	if (logFile.empty())
		return nullptr;

	Common::FSNode file(logFile);
	Common::WriteStream *stream = file.createWriteStream(false);
	if (stream)
		_logFilePath = logFile;
	return stream;
}

Common::String OSystem_SDL::getSystemLanguage() const {

#if SDL_VERSION_ATLEAST(2, 0, 14)
	SDL_Locale *locales = SDL_GetPreferredLocales();
	if (locales) {
		if (locales[0].language != NULL) {
			Common::String str = Common::String::format("%s_%s", locales[0].country, locales[0].language);
			SDL_free(locales);
			return str;
		}
		SDL_free(locales);
	}
#endif // SDL_VERSION_ATLEAST(2, 0, 14)
#if defined(USE_DETECTLANG) && !defined(WIN32)
	// Activating current locale settings
	const Common::String locale = setlocale(LC_ALL, "");

	// Restore default C locale to prevent issues with
	// portability of sscanf(), atof(), etc.
	// See bug #6434
	setlocale(LC_ALL, "C");

	// Detect the language from the locale
	if (locale.empty()) {
		return BaseBackend::getSystemLanguage();
	} else if (locale == "C" || locale == "POSIX") {
		return "en_US";
	} else {
		int length = 0;

		// Assume the locale is in the form language[_territory[.codeset]][@modifier].
		// On macOS the format is different (it looks like C/UTF-8/C/C/C/C), but we
		// have a different implementation of getSystemLanguage for macOS anyway, so
		// we don't have to handle it here.
		// Strip out additional information, like ".UTF-8" or the like.
		for (int size = locale.size(); length < size; ++length) {
			if (locale[length] == '.' || locale[length] == ' ' || locale[length] == '@')
				break;
		}

		return Common::String(locale.c_str(), length);
	}
#else // USE_DETECTLANG
	return BaseBackend::getSystemLanguage();
#endif // USE_DETECTLANG
}

#if SDL_VERSION_ATLEAST(2, 0, 0)
bool OSystem_SDL::hasTextInClipboard() {
	return SDL_HasClipboardText() == SDL_TRUE;
}

Common::U32String OSystem_SDL::getTextFromClipboard() {
	if (!hasTextInClipboard()) return Common::U32String();

	char *text = SDL_GetClipboardText();

	Common::String utf8Text(text);
	Common::U32String strText = utf8Text.decode();
	SDL_free(text);

	return strText;
}

bool OSystem_SDL::setTextInClipboard(const Common::U32String &text) {
	// The encoding we need to use is UTF-8.
	Common::String utf8Text = text.encode();
	return SDL_SetClipboardText(utf8Text.c_str()) == 0;
}

void OSystem_SDL::messageBox(LogMessageType::Type type, const char *message) {
	Uint32 flags = 0;

	switch (type) {
	case LogMessageType::kError:
		flags = SDL_MESSAGEBOX_ERROR;
		break;
	case LogMessageType::kWarning:
		flags = SDL_MESSAGEBOX_WARNING;
		break;
	case LogMessageType::kInfo:
	case LogMessageType::kDebug:
	default:
		flags = SDL_MESSAGEBOX_INFORMATION;
		break;
	}

	SDL_ShowSimpleMessageBox(flags, "ScummVM", message, _window ? _window->getSDLWindow() : nullptr);
}
#endif

#if SDL_VERSION_ATLEAST(2, 0, 14)
bool OSystem_SDL::openUrl(const Common::String &url) {
	if (SDL_OpenURL(url.c_str()) != 0) {
		warning("Failed to open URL: %s", SDL_GetError());
		return false;
	}

	return true;
}
#endif

Common::MutexInternal *OSystem_SDL::createMutex() {
	return createSdlMutexInternal();
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

void OSystem_SDL::getTimeAndDate(TimeDate &td, bool skipRecord) const {
	time_t curTime = time(nullptr);
	struct tm t = *localtime(&curTime);
	td.tm_sec = t.tm_sec;
	td.tm_min = t.tm_min;
	td.tm_hour = t.tm_hour;
	td.tm_mday = t.tm_mday;
	td.tm_mon = t.tm_mon;
	td.tm_year = t.tm_year;
	td.tm_wday = t.tm_wday;

#ifdef ENABLE_EVENTRECORDER
	g_eventRec.processTimeAndDate(td, skipRecord);
#endif
}

MixerManager *OSystem_SDL::getMixerManager() {
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

uint32 OSystem_SDL::getDoubleClickTime() const {
	if (ConfMan.hasKey("double_click_time"))
		return ConfMan.getInt("double_click_time");

	return getOSDoubleClickTime();
}

//Not specified in base class
Common::Path OSystem_SDL::getDefaultIconsPath() {
	return ConfMan.getPath("iconspath");
}

// Not specified in base class
Common::Path OSystem_SDL::getDefaultDLCsPath() {
	Common::Path path(ConfMan.get("dlcspath"));
	return path;
}

//Not specified in base class
Common::Path OSystem_SDL::getScreenshotsPath() {
	return ConfMan.getPath("screenshotpath");
}

#ifdef USE_MULTIPLE_RENDERERS

OSystem_SDL::GraphicsManagerType OSystem_SDL::getDefaultGraphicsManager() const {
	return GraphicsManagerSurfaceSDL;
}

SdlGraphicsManager *OSystem_SDL::createGraphicsManager(SdlEventSource *sdlEventSource, SdlWindow *window, GraphicsManagerType type) {
	switch (type) {
	case GraphicsManagerSurfaceSDL:
		debug(1, "creating SurfaceSDL graphics manager");
		return new SurfaceSdlGraphicsManager(sdlEventSource, window);
#ifdef USE_OPENGL
	case GraphicsManagerOpenGL:
		debug(1, "creating OpenGL graphics manager");
		return new OpenGLSdlGraphicsManager(sdlEventSource, window);
#endif
	default:
		assert(0);
		return NULL;
	}
}

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
		GraphicsManagerType type = getDefaultGraphicsManager();
		assert(type >= GraphicsManagerSurfaceSDL && type < GraphicsManagerCount);
		return _defaultMode[type];
	}
}

bool OSystem_SDL::setGraphicsMode(int mode, uint flags) {
	bool render3d = flags & OSystem::kGfxModeRender3d;

	// In 3d render mode gfx mode param is ignored.
	if (_graphicsModes.empty() && !render3d) {
		return _graphicsManager->setGraphicsMode(mode);
	}

	// Check whether a invalid mode is requested.
	if (mode < 0 || (uint)mode >= _graphicsModeIds.size()) {
		return false;
	}

	// Very hacky way to set up the old graphics manager state, in case we
	// switch from SDL->OpenGL or OpenGL->SDL.
	//
	// This is a probably temporary workaround to fix bugs like #5799
	// "SDL/OpenGL: Crash when switching renderer backend".
	//
	// It's also used to restore state from 3D to 2D GFX manager
	SdlGraphicsManager *sdlGraphicsManager = dynamic_cast<SdlGraphicsManager *>(_graphicsManager);
	_gfxManagerState = sdlGraphicsManager->getState();
	bool supports3D = sdlGraphicsManager->hasFeature(kFeatureOpenGLForGame);

	bool switchedManager = false;

	// If the new mode and the current mode are not from the same graphics
	// manager, delete and create the new mode graphics manager
#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)
	if (render3d && !supports3D) {
		debug(1, "switching to OpenGL 3D graphics");
		sdlGraphicsManager->deactivateManager();
		delete sdlGraphicsManager;
		_graphicsManager = sdlGraphicsManager = new OpenGLSdlGraphics3dManager(_eventSource, _window, _supportsFrameBuffer);
		switchedManager = true;
	} else
#endif
	{
		for (uint i = 0; i < GraphicsManagerCount; ++i) {
			if (!(mode >= _firstMode[i] && mode <= _lastMode[i]))
				continue;
			if (_graphicsMode >= _firstMode[i] && _graphicsMode <= _lastMode[i] && !supports3D)
				break;
			debug(1, "switching graphics manager");
			if (sdlGraphicsManager) {
				sdlGraphicsManager->deactivateManager();
				delete sdlGraphicsManager;
			}
			_graphicsManager = sdlGraphicsManager = createGraphicsManager(_eventSource, _window, (GraphicsManagerType)i);
			switchedManager = true;
			break;
		}
	}

	_graphicsMode = mode;

	if (switchedManager) {
		sdlGraphicsManager->activateManager();

		// Setup the graphics mode and size first
		// This is needed so that we can check the supported pixel formats when
		// restoring the state.
		_graphicsManager->beginGFXTransaction();
		if (!_graphicsManager->setGraphicsMode(_graphicsModeIds[mode], flags))
			return false;
		_graphicsManager->initSize(_gfxManagerState.screenWidth, _gfxManagerState.screenHeight);
		_graphicsManager->endGFXTransaction();

		// This failing will probably have bad consequences...
		if (!sdlGraphicsManager->setState(_gfxManagerState)) {
			return false;
		}

		// Next setup the cursor again
		CursorMan.pushCursor(nullptr, 0, 0, 0, 0, 0);
		CursorMan.popCursor();

		// Next setup cursor palette if needed
		if (_graphicsManager->getFeatureState(kFeatureCursorPalette)) {
			CursorMan.pushCursorPalette(nullptr, 0, 0);
			CursorMan.popCursorPalette();
		}

		_graphicsManager->beginGFXTransaction();
		return true;
	} else {
		return _graphicsManager->setGraphicsMode(_graphicsModeIds[mode], flags);
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
	clearGraphicsModes();
	_graphicsModeIds.clear();

	// Count the number of graphics modes
	const OSystem::GraphicsMode *srcMode;
	int defaultMode;

	GraphicsManager *manager;
	for (uint i = 0; i < GraphicsManagerCount; i++) {
		_defaultMode[i] = -1;
		_firstMode[i] = _graphicsModes.size();
		manager = createGraphicsManager(_eventSource, _window, (GraphicsManagerType)i);
		defaultMode = manager->getDefaultGraphicsMode();
		srcMode = manager->getSupportedGraphicsModes();
		while (srcMode->name) {
			if (defaultMode == srcMode->id) {
				_defaultMode[i] = _graphicsModes.size();
			}
			OSystem::GraphicsMode mode = *srcMode;
			// Do deep copy as we are going to delete the GraphicsManager and this may free
			// the memory used for its graphics modes.
			mode.name = scumm_strdup(srcMode->name);
			mode.description = scumm_strdup(srcMode->description);
			_graphicsModes.push_back(mode);
			srcMode++;
		}
		_lastMode[i] = _graphicsModes.size() - 1;
		delete manager;
		assert(_defaultMode[i] != -1);
	}

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

void OSystem_SDL::clearGraphicsModes() {
	if (!_graphicsModes.empty()) {
		OSystem::GraphicsMode *mode = _graphicsModes.begin();
		while (mode->name) {
			free(const_cast<char *>(mode->name));
			free(const_cast<char *>(mode->description));
			mode++;
		}
		_graphicsModes.clear();
	}
}
#endif

static const char * const helpTabs[] = {
_s("Keyboard"),
"",
_s(
"## Keyboard shortcuts\n"
"\n"
"ScummVM supports various in-game keyboard and mouse shortcuts, and since version 2.2.0 these can be manually configured in the **Keymaps tab**, or in the **configuration file**.\n"
"\n"
"For game-specific controls, see the [wiki entry](https://wiki.scummvm.org/index.php?title=Category:Supported_Games) for the game you are playing.\n"
"\n"
"Default shortcuts are shown in the table.\n"
"\n"
"| Shortcut      | Description      \n"
"| --------------|------------------\n"
"| `Ctrl+F5` | Displays the Global Main Menu\n")
#if defined(MACOSX)
_s("| `Cmd+q`    | Quit (macOS)\n")
#elif defined(WIN32)
_s("| `Alt+F4`  | Quit (Windows)\n")
#else
_s("| `Ctrl+q`  | Quit (Linux/Unix)\n")
_s("| `Ctrl+z`  | Quit (other platforms)\n")
#endif
_s(
"| `Ctrl+u`  | Mutes all sounds\n"
"| `Ctrl+m`  | Toggles mouse capture\n"
"| `Ctrl+Alt` and `9` or `0` | Cycles forwards/backwards between graphics filters\n"
"| `Ctrl+Alt` and `+` or `-` | Increases/decreases the scale factor\n"
"| `Ctrl+Alt+a` | Toggles aspect ratio correction on/off\n"
"| `Ctrl+Alt+f` | Toggles between nearest neighbor and bilinear interpolation (graphics filtering on/off)\n"
"| `Ctrl+Alt+s` | Cycles through stretch modes\n"
"| `Alt+Enter`   | Toggles full screen/windowed mode\n"
"| `Alt+s`          | Takes a screenshot\n"
"| `Ctrl+F7`       | Opens virtual keyboard (if enabled). This can also be opened with a long press of the middle mouse button or wheel.\n"
"| `Ctrl+Alt+d` | Opens the ScummVM debugger\n"
),

0,
	};

const char * const *OSystem_SDL::buildHelpDialogData() {
	return helpTabs;
}
