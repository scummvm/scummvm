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

#ifndef PLATFORM_SDL_H
#define PLATFORM_SDL_H

#include "backends/platform/sdl/sdl-sys.h"

#include "backends/modular-backend.h"
#include "backends/mixer/sdl/sdl-mixer.h"
#include "backends/events/sdl/sdl-events.h"
#include "backends/log/log.h"
#include "backends/platform/sdl/sdl-window.h"

#include "common/array.h"

#ifdef USE_DISCORD
class DiscordPresence;
#endif

/**
 * Base OSystem class for all SDL ports.
 */
class OSystem_SDL : public ModularMixerBackend, public ModularGraphicsBackend {
public:
	OSystem_SDL();
	virtual ~OSystem_SDL();

	/**
	 * Pre-initialize backend. It should be called after
	 * instantiating the backend. Early needed managers are
	 * created here.
	 */
	void init() override;

	bool hasFeature(Feature f) override;
	void setFeatureState(Feature f, bool enable) override;
	bool getFeatureState(Feature f) override;

	// Override functions from ModularBackend and OSystem
	void initBackend() override;
	void engineInit() override;
	void engineDone() override;
	void quit() override;
	void fatalError() override;
	Common::KeymapArray getGlobalKeymaps() override;
	Common::HardwareInputSet *getHardwareInputSet() override;

	// Logging
	void logMessage(LogMessageType::Type type, const char *message) override;

	Common::String getSystemLanguage() const override;

#if SDL_VERSION_ATLEAST(2, 0, 0)
	// Clipboard
	bool hasTextInClipboard() override;
	Common::U32String getTextFromClipboard() override;
	bool setTextInClipboard(const Common::U32String &text) override;

	void messageBox(LogMessageType::Type type, const char *message) override;
#endif

#if SDL_VERSION_ATLEAST(2, 0, 14)
	bool openUrl(const Common::String &url) override;
#endif

	void setWindowCaption(const Common::U32String &caption) override;
	void addSysArchivesToSearchSet(Common::SearchSet &s, int priority = 0) override;
	Common::MutexInternal *createMutex() override;
	uint32 getMillis(bool skipRecord = false) override;
	void delayMillis(uint msecs) override;
	void getTimeAndDate(TimeDate &td, bool skipRecord = false) const override;
	MixerManager *getMixerManager() override;
	Common::TimerManager *getTimerManager() override;
	Common::SaveFileManager *getSavefileManager() override;
	uint32 getDoubleClickTime() const override;

	// Default paths
	virtual Common::Path getDefaultIconsPath();
	virtual Common::Path getDefaultDLCsPath();
	virtual Common::Path getScreenshotsPath();

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)
	Common::Array<uint> getSupportedAntiAliasingLevels() const override;
	OpenGL::ContextType getOpenGLType() const override { return _oglType; }
#endif
#if defined(USE_OPENGL) && defined(USE_GLAD)
	void *getOpenGLProcAddress(const char *name) const override;
#endif

protected:
	bool _inited;
	bool _initedSDL;
#ifdef USE_SDL_NET
	bool _initedSDLnet;
#endif

#ifdef USE_DISCORD
	DiscordPresence *_presence;
#endif

	/**
	 * The path of the currently open log file, if any.
	 *
	 * @note This is currently a Path and not an FSNode for simplicity;
	 * e.g. we don't need to include fs.h here, and currently the
	 * only use of this value is to use it to open the log file in an
	 * editor; for that, we need it only as a path anyway.
	 */
	Common::Path _logFilePath;

	/**
	 * The event source we use for obtaining SDL events.
	 */
	SdlEventSource *_eventSource;
	Common::EventSource *_eventSourceWrapper;

	/**
	 * The SDL output window.
	 */
	SdlWindow *_window;

	SdlGraphicsManager::State _gfxManagerState;

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)
	// Graphics capabilities
	void detectOpenGLFeaturesSupport();
	void detectAntiAliasingSupport();

	OpenGL::ContextType _oglType;
	bool _supportsFrameBuffer;
	bool _supportsShaders;
	Common::Array<uint> _antiAliasLevels;
#endif

	/**
	 * Initialize the SDL library.
	 */
	virtual void initSDL();

	/**
	 * Create the audio CD manager
	 */
	virtual AudioCDManager *createAudioCDManager();

	// Logging
	virtual Common::WriteStream *createLogFile();
	Backends::Log::Log *_logger;

#ifdef USE_OPENGL
	typedef Common::Array<GraphicsMode> GraphicsModeArray;
	GraphicsModeArray _graphicsModes;
	Common::Array<int> _graphicsModeIds;
	int _graphicsMode;
	int _firstGLMode;
	int _defaultSDLMode;
	int _defaultGLMode;

	/**
	 * Create the merged graphics modes list.
	 */
	void setupGraphicsModes();

	/**
	 * Clear the merged graphics modes list.
	 */
	void clearGraphicsModes();

	enum GraphicsManagerType { GraphicsManagerSDL, GraphicsManagerOpenGL };
	virtual GraphicsManagerType getDefaultGraphicsManager() const { return GraphicsManagerSDL; }
	const OSystem::GraphicsMode *getSupportedGraphicsModes() const override;
	int getDefaultGraphicsMode() const override;
	bool setGraphicsMode(int mode, uint flags) override;
	int getGraphicsMode() const override;
#endif

	virtual uint32 getOSDoubleClickTime() const { return 0; }
	virtual const char * const *buildHelpDialogData() override;
};

#endif
