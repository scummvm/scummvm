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


#ifndef _OSYSTEM_DS_H_
#define _OSYSTEM_DS_H_

// Allow use of stuff in <nds.h>
#define FORBIDDEN_SYMBOL_EXCEPTION_printf
#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h

#include "backends/base-backend.h"
#include "backends/events/ds/ds-events.h"
#include "nds.h"
#include "audio/mixer_intern.h"
#include "graphics/surface.h"
#include "graphics/palette.h"

class OSystem_DS : public BaseBackend, public PaletteManager {
protected:
	Audio::MixerImpl *_mixer;
	Graphics::Surface _framebuffer, _overlay;
	bool _graphicsEnable, _isOverlayShown;

	static OSystem_DS *_instance;

	u16 _palette[256];
	u16 _cursorPalette[256];

	u8 _cursorImage[64 * 64];
	uint _cursorW;
	uint _cursorH;
	int _cursorHotX;
	int _cursorHotY;
	byte _cursorKey;
	int _cursorScale;

	DSEventSource *_eventSource;

	Graphics::Surface *createTempFrameBuffer();
	bool _disableCursorPalette;

	int _gammaValue;

public:
	typedef int  (*TimerProc)(int interval);

	OSystem_DS();
	virtual ~OSystem_DS();

	static OSystem_DS *instance() { return _instance; }

	virtual bool hasFeature(Feature f);
	virtual void setFeatureState(Feature f, bool enable);
	virtual bool getFeatureState(Feature f);
	virtual void initSize(uint width, uint height, const Graphics::PixelFormat *format);
	virtual int16 getHeight();
	virtual int16 getWidth();

	virtual PaletteManager *getPaletteManager() { return this; }
protected:
	// PaletteManager API
	virtual void setPalette(const byte *colors, uint start, uint num);
	virtual void grabPalette(byte *colors, uint start, uint num) const;

public:
	virtual void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h);
	virtual void updateScreen();
	virtual void setShakePos(int shakeXOffset, int shakeYOffset);

	virtual void showOverlay();
	virtual void hideOverlay();
	virtual bool isOverlayVisible() const;
	virtual void clearOverlay();
	virtual void grabOverlay(void *buf, int pitch);
	virtual void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h);
	virtual int16 getOverlayHeight();
	virtual int16 getOverlayWidth();
	virtual Graphics::PixelFormat getOverlayFormat() const;

	Common::Point transformPoint(uint16 x, uint16 y);
	virtual bool showMouse(bool visible);

	virtual void warpMouse(int x, int y);
	virtual void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, u32 keycolor, bool dontScale, const Graphics::PixelFormat *format);

	virtual uint32 getMillis(bool skipRecord = false);
	virtual void delayMillis(uint msecs);
	virtual void getTimeAndDate(TimeDate &t) const;

	virtual Common::EventSource *getDefaultEventSource() { return _eventSource; }
	virtual Common::HardwareInputSet *getHardwareInputSet();

	virtual MutexRef createMutex(void);
	virtual void lockMutex(MutexRef mutex);
	virtual void unlockMutex(MutexRef mutex);
	virtual void deleteMutex(MutexRef mutex);

	virtual void quit();

	virtual void setFocusRectangle(const Common::Rect& rect);
	virtual void clearFocusRectangle();

	virtual void engineInit();
	virtual void engineDone();

	virtual void initBackend();

	virtual Graphics::Surface *lockScreen();
	virtual void unlockScreen();

	virtual Audio::Mixer *getMixer() { return _mixer; }

	static int timerHandler(int t);

	u16 getDSPaletteEntry(u32 entry) const { return _palette[entry]; }
	u16 getDSCursorPaletteEntry(u32 entry) const { return !_disableCursorPalette? _cursorPalette[entry]: _palette[entry]; }

	virtual void setCursorPalette(const byte *colors, uint start, uint num);

	void refreshCursor();

	virtual void logMessage(LogMessageType::Type type, const char *message);

	u16 applyGamma(u16 color);
	void setGammaValue(int gamma) { _gammaValue = gamma; }
};

#endif
