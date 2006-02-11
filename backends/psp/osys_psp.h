/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
 * Copyright (C) 2005 Joost Peters PSP Backend
 * Copyright (C) 2005 Thomas Mayer PSP Backend
 * Copyright (C) 2005 Paolo Costabel PSP Backend
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

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "backends/intern.h"

#include <SDL.h>

enum GraphicModeID {
	CENTERED_320X200,
	CENTERED_435X272,
	STRETCHED_480X272,
	CENTERED_362X272
};

class OSystem_PSP : public OSystem {
public:
	static const OSystem::GraphicsMode s_supportedGraphicsModes[];
	static OSystem *instance();

protected:
	uint16	_screenWidth;
	uint16	_screenHeight;
	uint16	_overlayScale;
	byte	*_offscreen;
	OverlayColor  *_overlayBuffer;
	uint16  _palette[256];
	bool	_overlayVisible;
	uint32	_shakePos;
	
	
	bool	_mouseVisible;
	int	_mouseX, _mouseY;
	int	_mouseWidth, _mouseHeight;
	int	_mouseHotspotX, _mouseHotspotY;
	byte	_mouseKeyColour;
	byte	*_mouseBuf;

	uint32	_prevButtons;
	uint32	_lastPadCheck;
	uint32	_padAccel;
	
	uint32		_samplesPerSec;
	SceCtrlData pad;
	
public:

	OSystem_PSP();
	virtual ~OSystem_PSP();

	virtual bool hasFeature(Feature f);
	virtual void setFeatureState(Feature f, bool enable);
	virtual bool getFeatureState(Feature f);
	virtual const GraphicsMode *getSupportedGraphicsModes() const;
	virtual int getDefaultGraphicsMode() const;
	virtual bool setGraphicsMode(int mode);
	bool setGraphicsMode(const char *name);
	virtual int getGraphicsMode() const;
	virtual void initSize(uint width, uint height, int overlayScale);
	virtual int16 getHeight();
	virtual int16 getWidth();
	virtual void setPalette(const byte *colors, uint start, uint num);
	virtual void copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h);
	virtual void updateScreen();
	virtual void setShakePos(int shakeOffset);

	virtual void showOverlay();
	virtual void hideOverlay();
	virtual void clearOverlay();
	virtual void grabOverlay(OverlayColor *buf, int pitch);
	virtual void copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h);
	virtual int16 getOverlayHeight();
	virtual int16 getOverlayWidth();
	virtual OverlayColor RGBToColor(uint8 r, uint8 g, uint8 b);
	virtual void colorToRGB(OverlayColor color, uint8 &r, uint8 &g, uint8 &b);
	virtual OverlayColor ARGBToColor(uint8 a, uint8 r, uint8 g, uint8 b);
	virtual void colorToARGB(OverlayColor color, uint8 &a, uint8 &r, uint8 &g, uint8 &b);
	virtual void grabPalette(byte *colors, uint start, uint num);
	
	virtual bool showMouse(bool visible);

	virtual void warpMouse(int x, int y);
	virtual void setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, byte keycolor = 255, int cursorTargetScale = 1);

	virtual bool pollEvent(Event &event);
	virtual uint32 getMillis();
	virtual void delayMillis(uint msecs);

	virtual void setTimerCallback(TimerProc callback, int interval);

	virtual MutexRef createMutex(void);
	virtual void lockMutex(MutexRef mutex);
	virtual void unlockMutex(MutexRef mutex);
	virtual void deleteMutex(MutexRef mutex);

	virtual bool setSoundCallback(SoundProc proc, void *param);
	virtual void clearSoundCallback();
	virtual int getOutputSampleRate() const;

	virtual bool openCD(int drive);
	virtual bool pollCD();

	virtual void playCD(int track, int num_loops, int start_frame, int duration);
	virtual void stopCD();
	virtual void updateCD();

	virtual void quit();

	virtual void setWindowCaption(const char *caption);

	virtual void displayMessageOnOSD(const char *msg);

};

