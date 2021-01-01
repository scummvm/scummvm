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
class OSystem_SDL : public ModularMutexBackend, public ModularMixerBackend, public ModularGraphicsBackend {
public:
	OSystem_SDL();
	virtual ~OSystem_SDL();

	/**
	 * Pre-initialize backend. It should be called after
	 * instantiating the backend. Early needed managers are
	 * created here.
	 */
	virtual void init() override;

	virtual bool hasFeature(Feature f) override;

	// Override functions from ModularBackend and OSystem
	virtual void initBackend() override;
	virtual void engineInit() override;
	virtual void engineDone() override;
	virtual void quit() override;
	virtual void fatalError() override;
	Common::KeymapArray getGlobalKeymaps() override;
	Common::HardwareInputSet *getHardwareInputSet() override;

	// Logging
	virtual void logMessage(LogMessageType::Type type, const char *message) override;

	virtual Common::String getSystemLanguage() const override;

#if SDL_VERSION_ATLEAST(2, 0, 0)
	// Clipboard
	virtual bool hasTextInClipboard() override;
	virtual Common::U32String getTextFromClipboard() override;
	virtual bool setTextInClipboard(const Common::U32String &text) override;
#endif

	virtual void setWindowCaption(const Common::U32String &caption) override;
	virtual void addSysArchivesToSearchSet(Common::SearchSet &s, int priority = 0) override;
	virtual uint32 getMillis(bool skipRecord = false) override;
	virtual void delayMillis(uint msecs) override;
	virtual void getTimeAndDate(TimeDate &td) const override;
	virtual MixerManager *getMixerManager() override;
	virtual Common::TimerManager *getTimerManager() override;
	virtual Common::SaveFileManager *getSavefileManager() override;

	//Screenshots
	virtual Common::String getScreenshotsPath();

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS) || defined(USE_GLES2)
	Common::Array<uint> getSupportedAntiAliasingLevels() const override;
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
	 * @note This is currently a string and not an FSNode for simplicity;
	 * e.g. we don't need to include fs.h here, and currently the
	 * only use of this value is to use it to open the log file in an
	 * editor; for that, we need it only as a string anyway.
	 */
	Common::String _logFilePath;

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

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS) || defined(USE_GLES2)
	// Graphics capabilities
	void detectFramebufferSupport();
	void detectAntiAliasingSupport();

	bool _supportsFrameBuffer;
	Common::Array<uint> _antiAliasLevels;
#endif

	/**
	 * Initialze the SDL library.
	 */
	virtual void initSDL();

	/**
	 * Create the audio CD manager
	 */
	virtual AudioCDManager *createAudioCDManager();

	// Logging
	virtual Common::String getDefaultLogFileName() { return Common::String(); }
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
	 * Creates the merged graphics modes list
	 */
	void setupGraphicsModes();

	virtual const OSystem::GraphicsMode *getSupportedGraphicsModes() const override;
	virtual int getDefaultGraphicsMode() const override;
	virtual bool setGraphicsMode(int mode, uint flags) override;
	virtual int getGraphicsMode() const override;
#endif
};

#endif
