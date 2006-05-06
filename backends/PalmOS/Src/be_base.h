/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2002-2006 Chris Apers - PalmOS Backend
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

#ifndef BE_BASE_H
#define BE_BASE_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/system.h"

enum {
	GFX_NORMAL = 0,
	GFX_WIDE,
};

// OSD key state
enum {
	kModifierNone = 0,
	kModifierCommand,
	kModifierAlt,
	kModifierCtrl,
	kModifierCount
};

// OSD resource id
#define kDrawKeyState	3000
#define kDrawNumPad		3010
#define kDrawBatLow		3020
#define kDrawFight		3030

typedef struct {
	UInt32 duration, nextExpiry;
	Boolean active;
	OSystem::TimerProc callback;
} TimerType, *TimerPtr;

typedef struct {
	Boolean	active;
	void *proc;
	void *param;
} SoundType, *SoundPtr;

class OSystem_PalmBase : public OSystem {
private:
	virtual void int_initBackend() { }
	
	virtual const GraphicsMode *int_getSupportedGraphicsModes() const;
	virtual void int_updateScreen() = 0;
	virtual void int_initSize(uint w, uint h, int overlayScale) = 0;
	virtual void int_setShakePos(int shakeOffset) { }

	virtual void extras_palette(uint8 index, uint8 r, uint8 g, uint8 b) { }
	virtual void int_quit() { }

	virtual void unload_gfx_mode() = 0;
	virtual void load_gfx_mode() = 0;

	virtual void draw_mouse() = 0;
	virtual void undraw_mouse() = 0;
	
//	virtual bool check_hard_keys() = 0;
	virtual bool check_event(Event &event, EventPtr ev) = 0;
	
	virtual void timer_handler();
	void battery_handler();
	virtual void get_coordinates(EventPtr ev, Coord &x, Coord &y) = 0;
	void simulate_mouse(Event &event, Int8 iHoriz, Int8 iVert, Coord *xr, Coord *yr);
	virtual void sound_handler() = 0;

protected:
	virtual void draw_osd(UInt16 id, Int32 x, Int32 y, Boolean show, UInt8 color = 0);

	enum {
		kKeyNone			= 0,
		kKeyMouseMove		= 1	<< 0,
		kKeyMouseLButton	= 1	<< 1,
	
		kKeyCalc			= 1 << 30,
		kKeyAny				= 1 << 31
	};
	enum {
		MAX_MOUSE_W = 40,
		MAX_MOUSE_H = 40
	};
	struct MousePos {
		int16 x,y,w,h;
	};

	TimerType _timer;
	SoundType _sound;

	RGBColorType _currentPalette[256];
	uint _paletteDirtyStart, _paletteDirtyEnd;
	
	int _mode, _setMode;
	int16 _screenWidth, _screenHeight;
	Boolean _modeChanged, _gfxLoaded;
	UInt32 _screenPitch;
	
	PointType _screenOffset;
	struct {
		Coord w, h;
	} _screenDest;
	byte *_screenP, *_offScreenP;
	WinHandle _screenH, _offScreenH;

	int _current_shake_pos;
	int _new_shake_pos;

	Boolean _overlayVisible;
	Boolean _redawOSD, _setPalette;

	UInt32 _keyMouseMask;
	struct {
		UInt32 bitUp;
		UInt32 bitDown;
		UInt32 bitLeft;
		UInt32 bitRight;
		UInt32 bitButLeft;
		Boolean hasMore;
	} _keyMouse;

	bool _mouseVisible;
	bool _mouseDrawn;
	MousePos _mouseCurState;
	MousePos _mouseOldState;
	int16 _mouseHotspotX;
	int16 _mouseHotspotY;
	byte _mouseKeyColor;
	byte *_mouseDataP, *_mouseBackupP;
	WinHandle _mouseDataH, _mouseBackupH;

	eventsEnum _lastEvent;
	WChar _lastKey;
	UInt8 _lastKeyModifier;
	UInt32 _lastKeyRepeat;
	Boolean _useNumPad, _showBatLow;
	UInt32 _batCheckTicks, _batCheckLast;
	
	int _samplesPerSec;

public:
	OSystem_PalmBase();
	void initBackend();

/*
	virtual void setFeatureState(Feature f, bool enable) {};


	bool hasFeature(Feature f);
	bool getFeatureState(Feature f);
	
	virtual void beginGFXTransaction();
	virtual void endGFXTransaction();
	
	virtual int16 getOverlayHeight();
	virtual int16 getOverlayWidth();
	virtual int screenToOverlayX(int x);
	virtual int screenToOverlayY(int y);
	virtual int overlayToScreenX(int x);
	virtual int overlayToScreenY(int y);
	
	virtual OverlayColor ARGBToColor(uint8 a, uint8 r, uint8 g, uint8 b);
	virtual void colorToARGB(OverlayColor color, uint8 &a, uint8 &r, uint8 &g, uint8 &b);
	
	virtual void setCursorPalette(const byte *colors, uint start, uint num);
	virtual void disableCursorPalette(bool disable);
	
	virtual void displayMessageOnOSD(const char *msg);
*/
	const GraphicsMode *getSupportedGraphicsModes() const;
	int getGraphicsMode() const;
	virtual int getDefaultGraphicsMode() const;
	virtual bool setGraphicsMode(int mode);
	virtual bool grabRawScreen(Graphics::Surface *surf) { return false; }

	void initSize(uint w, uint h, int overlayScale);
	int16 getWidth() { return _screenWidth; }
	int16 getHeight() { return _screenHeight; }

	void setShakePos(int shakeOffset);
	virtual void copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h) = 0;
	virtual void updateScreen();
	virtual void clearScreen();

	bool showMouse(bool visible);
	void warpMouse(int x, int y);
	virtual void setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, byte keycolor, int cursorTargetScale) = 0;

	virtual void showOverlay() = 0;
	virtual void hideOverlay() = 0;
	virtual void clearOverlay() = 0;
	virtual void grabOverlay(OverlayColor *buf, int pitch) = 0;
	virtual void copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h) = 0;

	void setPalette(const byte *colors, uint start, uint num);
	void grabPalette(byte *colors, uint start, uint num);
	virtual OverlayColor RGBToColor(uint8 r, uint8 g, uint8 b) = 0;
	virtual void colorToRGB(OverlayColor color, uint8 &r, uint8 &g, uint8 &b) = 0;
	
	bool pollEvent(Event &event);
	
	virtual uint32 getMillis();
	virtual void delayMillis(uint msecs);
	
	virtual void setTimerCallback(TimerProc callback, int interval);

	virtual MutexRef createMutex() { return NULL; }
	virtual void lockMutex(MutexRef mutex) {}
	virtual void unlockMutex(MutexRef mutex) {}
	virtual void deleteMutex(MutexRef mutex) {}
	
	virtual bool setSoundCallback(SoundProc proc, void *param) = 0;
	virtual void clearSoundCallback() = 0;
	int getOutputSampleRate() const { return _samplesPerSec; }

	void quit();
	virtual void setWindowCaption(const char *caption) = 0;
	
	Common::SaveFileManager *getSavefileManager();
};

#endif
