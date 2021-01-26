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

#include "backends/modular-backend.h"
#include "backends/events/ds/ds-events.h"
#include "backends/mixer/mixer.h"
#include "backends/platform/ds/background.h"
#include "graphics/surface.h"
#include "graphics/palette.h"

enum {
	GFX_NOSCALE = 0,
	GFX_HWSCALE = 1,
	GFX_SWSCALE = 2
};

class OSystem_DS : public ModularMutexBackend, public ModularMixerBackend, public PaletteManager {
protected:
	DS::Background _framebuffer, _overlay;
#ifdef DISABLE_TEXT_CONSOLE
	DS::Background _subScreen;
#endif
	Graphics::Surface _cursor;
	int _graphicsMode, _stretchMode;
	bool _paletteDirty, _cursorDirty;

	static OSystem_DS *_instance;

	u16 _palette[256];
	u16 _cursorPalette[256];

	u16 *_cursorSprite;
	Common::Point _cursorPos;
	int _cursorHotX;
	int _cursorHotY;
	uint32 _cursorKey;
	bool _cursorVisible;

	DSEventSource *_eventSource;

	void initGraphics();

	bool _disableCursorPalette;

	const Graphics::PixelFormat _pfCLUT8, _pfABGR1555;

public:
	OSystem_DS();
	virtual ~OSystem_DS();

	static OSystem_DS *instance() { return _instance; }

	virtual bool hasFeature(Feature f);
	virtual void setFeatureState(Feature f, bool enable);
	virtual bool getFeatureState(Feature f);

	virtual const OSystem::GraphicsMode *getSupportedGraphicsModes() const;
	virtual int getDefaultGraphicsMode() const;
	virtual bool setGraphicsMode(int mode, uint flags);
	virtual int getGraphicsMode() const;

	virtual const GraphicsMode *getSupportedStretchModes() const;
	virtual int getDefaultStretchMode() const;
	virtual bool setStretchMode(int mode);
	virtual int getStretchMode() const;

	virtual Graphics::PixelFormat getScreenFormat() const;
	virtual Common::List<Graphics::PixelFormat> getSupportedFormats() const;

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

	Common::Point transformPoint(int16 x, int16 y);
	virtual bool showMouse(bool visible);

	virtual void warpMouse(int x, int y);
	virtual void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, u32 keycolor, bool dontScale, const Graphics::PixelFormat *format);

	virtual void addSysArchivesToSearchSet(Common::SearchSet &s, int priority);

	virtual uint32 getMillis(bool skipRecord = false);
	virtual void delayMillis(uint msecs);
	virtual void getTimeAndDate(TimeDate &t) const;
	void doTimerCallback(int interval = 10);

	virtual Common::EventSource *getDefaultEventSource() { return _eventSource; }
	virtual Common::HardwareInputSet *getHardwareInputSet();

	virtual Common::String getSystemLanguage() const;

	virtual void quit();

	virtual void setFocusRectangle(const Common::Rect& rect);
	virtual void clearFocusRectangle();

	virtual void initBackend();

	virtual Graphics::Surface *lockScreen();
	virtual void unlockScreen();

	virtual void setCursorPalette(const byte *colors, uint start, uint num);

	void refreshCursor(u16 *dst, const Graphics::Surface &src, const uint16 *palette);

	virtual void logMessage(LogMessageType::Type type, const char *message);

	void setMainScreen(int32 x, int32 y, int32 sx, int32 sy);
	void setSubScreen(int32 x, int32 y, int32 sx, int32 sy);

	int _currentTimeMillis, _callbackTimer;
};

#endif
