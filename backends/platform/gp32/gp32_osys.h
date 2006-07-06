/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2002 Ph0x - GP32 Backend
 * Copyright (C) 2003/2004 DJWillis - GP32 Backend
 * Copyright (C) 2005 Won Star - GP32 Backend
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

#ifndef GP32_OSYS_H
#define GP32_OSYS_H

//Standard ScummVM includes.
#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "backends/intern.h"

#include "base/engine.h"

#include "backends/gp32/gp32std.h"
#include "backends/gp32/gp32std_grap.h"
#include "backends/gp32/gp32std_input.h"
#include "backends/gp32/gp32std_sound.h"

class OSystem_GP32 : public OSystem {
public:
	static OSystem *instance();

protected:
	uint16 _screenWidth, _screenHeight;
	uint8 *_gameScreen;
	uint16 *_tmpScreen, *_hwScreen;
	OverlayColor *_overlayBuffer;

	int _overlayWidth, _overlayHeight;
	bool _overlayVisible;
	uint32 _shakePos;

	// Keyboard mouse emulation
	struct KbdMouse {	
		int16 x, y, x_vel, y_vel, x_max, y_max, x_down_count, y_down_count;
		uint32 last_time, delay_time, x_down_time, y_down_time;
	};

	struct MousePos {
		int16 x, y, w, h;
	};

	// mouse
	KbdMouse _km;
	bool _mouseVisible;
	int	_mouseX, _mouseY;
	int	_mouseWidth, _mouseHeight;
	int	_mouseHotspotX, _mouseHotspotY;
	byte _mouseKeyColor;
	byte *_mouseBuf;

	bool _adjustAspectRatio;
	
	/** Force full redraw on next updateScreen */
	bool _forceFull;

	uint16 _currentPalette[256];
//	uint _paletteDirtyStart, _paletteDirtyEnd;

	int32 _samplesPerSec;

public:

	OSystem_GP32();
	~OSystem_GP32();

	bool hasFeature(Feature f);
	void setFeatureState(Feature f, bool enable);
	bool getFeatureState(Feature f);
	const GraphicsMode *getSupportedGraphicsModes() const;
	int getDefaultGraphicsMode() const;
	bool setGraphicsMode(int mode);
	bool setGraphicsMode(const char *name);
	int getGraphicsMode() const;
	void initSize(uint width, uint height);
	int16 getHeight();
	int16 getWidth();
	void setPalette(const byte *colors, uint start, uint num);
	void grabPalette(byte *colors, uint start, uint num);

	void copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h);
	void updateScreen();
	void setShakePos(int shakeOffset);

	void showOverlay();
	void hideOverlay();
	void clearOverlay();
	void grabOverlay(OverlayColor *buf, int pitch);
	void copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h);
	int16 getOverlayHeight();
	int16 getOverlayWidth();

	OverlayColor RGBToColor(uint8 r, uint8 g, uint8 b);
	void colorToRGB(OverlayColor color, uint8 &r, uint8 &g, uint8 &b);

	bool showMouse(bool visible);

	void warpMouse(int x, int y);
	void setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, byte keycolor = 255, int cursorTargetScale = 1);

	bool pollEvent(Event &event);
	uint32 getMillis();
	void delayMillis(uint msecs);

	void setTimerCallback(TimerProc callback, int interval);

	MutexRef createMutex(void);
	void lockMutex(MutexRef mutex);
	void unlockMutex(MutexRef mutex);
	void deleteMutex(MutexRef mutex);

	bool setSoundCallback(SoundProc proc, void *param);
	void clearSoundCallback();
	int getOutputSampleRate() const;

	void quit();

	void setWindowCaption(const char *caption);

	void displayMessageOnOSD(const char *msg);

	void fillMouseEvent(Event &event, int x, int y);
	void handleKbdMouse();
};

#else
	#warning GP32_OSYS.H Called more then once.
#endif /* GP32_H */
