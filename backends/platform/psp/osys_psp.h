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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef OSYS_PSP_H
#define OSYS_PSP_H

#include "common/scummsys.h"
#include "graphics/surface.h"
#include "graphics/colormasks.h"
#include "sound/mixer_intern.h"
#include "backends/base-backend.h"
#include "backends/fs/psp/psp-fs-factory.h"
#include "backends/platform/psp/display_client.h"
#include "backends/platform/psp/default_display_client.h"
#include "backends/platform/psp/cursor.h"
#include "backends/platform/psp/pspkeyboard.h"
#include "backends/platform/psp/display_manager.h"
#include "backends/platform/psp/input.h"
#include "backends/platform/psp/audio.h"
#include "backends/timer/psp/timer.h"
#include "backends/platform/psp/thread.h"

#include <SDL.h>

class OSystem_PSP : public BaseBackend {
private:

	Common::SaveFileManager *_savefile;
	Audio::MixerImpl *_mixer;
	Common::TimerManager *_timer;
	bool _pendingUpdate;  			// save an update we couldn't perform
	uint32 _pendingUpdateCounter;	// prevent checking for pending update too often, in a cheap way

	// All needed sub-members
	Screen _screen;
	Overlay _overlay;
	Cursor _cursor;
	DisplayManager _displayManager;
	PSPKeyboard _keyboard;
	InputHandler _inputHandler;
	PspAudio _audio;
	PspTimer _pspTimer;
	PspRtc _pspRtc;

	void initSDL();	

public:
	OSystem_PSP() : _savefile(0), _mixer(0), _timer(0), _pendingUpdate(false), _pendingUpdateCounter(0) {}
	~OSystem_PSP();

	static OSystem *instance();

	void initBackend();

	// Feature related
	bool hasFeature(Feature f);
	void setFeatureState(Feature f, bool enable);
	bool getFeatureState(Feature f);

	// Graphics related
	const GraphicsMode *getSupportedGraphicsModes() const;
	int getDefaultGraphicsMode() const;
	bool setGraphicsMode(int mode);
	bool setGraphicsMode(const char *name);
	int getGraphicsMode() const;
#ifdef USE_RGB_COLOR
	virtual Graphics::PixelFormat getScreenFormat() const;
	virtual Common::List<Graphics::PixelFormat> getSupportedFormats() const;
#endif

	// Screen size
	void initSize(uint width, uint height, const Graphics::PixelFormat *format);
	int16 getWidth();
	int16 getHeight();

	// Palette related
	void setPalette(const byte *colors, uint start, uint num);
	void grabPalette(byte *colors, uint start, uint num);
	void setCursorPalette(const byte *colors, uint start, uint num);
	void disableCursorPalette(bool disable);

	// Screen related
	void copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h);
	Graphics::Surface *lockScreen();
	void unlockScreen();
	void updateScreen();
	void setShakePos(int shakeOffset);

	// Overlay related
	void showOverlay();
	void hideOverlay();
	void clearOverlay();
	void grabOverlay(OverlayColor *buf, int pitch);
	void copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h);
	int16 getOverlayHeight();
	int16 getOverlayWidth();
	Graphics::PixelFormat getOverlayFormat() const { return Graphics::createPixelFormat<4444>(); }

	// Mouse related
	bool showMouse(bool visible);
	void warpMouse(int x, int y);
	void setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, int cursorTargetScale, const Graphics::PixelFormat *format);

	// Events and input
	bool pollEvent(Common::Event &event);
	bool processInput(Common::Event &event);

	// Time
	uint32 getMillis();
	void delayMillis(uint msecs);

	// Timer
	typedef int (*TimerProc)(int interval);
	void setTimerCallback(TimerProc callback, int interval);
	Common::TimerManager *getTimerManager() { return _timer; }

	// Mutex
	MutexRef createMutex(void);
	void lockMutex(MutexRef mutex);
	void unlockMutex(MutexRef mutex);
	void deleteMutex(MutexRef mutex);

	// Sound
	static void mixCallback(void *sys, byte *samples, int len);
	void setupMixer(void);
	Audio::Mixer *getMixer() { return _mixer; }

	// Misc
	Common::SaveFileManager *getSavefileManager() { return _savefile; }
	FilesystemFactory *getFilesystemFactory() { return &PSPFilesystemFactory::instance(); }
	void getTimeAndDate(TimeDate &t) const;

	void quit();

	Common::SeekableReadStream *createConfigReadStream();
	Common::WriteStream *createConfigWriteStream();

};

#endif /* OSYS_PSP_H */
