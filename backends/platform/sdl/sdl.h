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

/**
 * Base OSystem class for all SDL ports.
 */
class OSystem_SDL : public ModularBackend {
public:
	OSystem_SDL();
	virtual ~OSystem_SDL();

	/**
	 * Pre-initialize backend. It should be called after
	 * instantiating the backend. Early needed managers are
	 * created here.
	 */
	virtual void init();

	/**
	 * Get the Mixer Manager instance. Not to confuse with getMixer(),
	 * that returns Audio::Mixer. The Mixer Manager is a SDL wrapper class
	 * for the Audio::Mixer. Used by other managers.
	 */
	virtual SdlMixerManager *getMixerManager();

	virtual bool hasFeature(Feature f);

	// Override functions from ModularBackend and OSystem
	virtual void initBackend();
	virtual void engineInit();
	virtual void engineDone();
	virtual void quit();
	virtual void fatalError();

	// Logging
	virtual void logMessage(LogMessageType::Type type, const char *message);

	virtual Common::String getSystemLanguage() const;

#if SDL_VERSION_ATLEAST(2, 0, 0)
	// Clipboard
	virtual bool hasTextInClipboard();
	virtual Common::String getTextFromClipboard();
	virtual bool setTextInClipboard(const Common::String &text);
#endif

	virtual void setWindowCaption(const char *caption);
	virtual void addSysArchivesToSearchSet(Common::SearchSet &s, int priority = 0);
	virtual uint32 getMillis(bool skipRecord = false);
	virtual void delayMillis(uint msecs);
	virtual void getTimeAndDate(TimeDate &td) const;
	virtual Audio::Mixer *getMixer();
	virtual Common::TimerManager *getTimerManager();
	virtual Common::SaveFileManager *getSavefileManager();

	//Screenshots
	virtual Common::String getScreenshotsPath();

protected:
	bool _inited;
	bool _initedSDL;
#ifdef USE_SDL_NET
	bool _initedSDLnet;
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
	 * Mixer manager that configures and setups SDL for
	 * the wrapped Audio::Mixer, the true mixer.
	 */
	SdlMixerManager *_mixerManager;

	/**
	 * The event source we use for obtaining SDL events.
	 */
	SdlEventSource *_eventSource;

	/**
	 * The SDL output window.
	 */
	SdlWindow *_window;

	virtual Common::EventSource *getDefaultEventSource() { return _eventSource; }

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
	int _desktopWidth, _desktopHeight;

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

	virtual const OSystem::GraphicsMode *getSupportedGraphicsModes() const;
	virtual int getDefaultGraphicsMode() const;
	virtual bool setGraphicsMode(int mode);
	virtual int getGraphicsMode() const;
#endif
protected:
	virtual char *convertEncoding(const char *to, const char *from, const char *string, size_t length);
};

#endif
