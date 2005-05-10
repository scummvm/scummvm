/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef SYSTEMPS2_H
#define SYSTEMPS2_H

#include "stdafx.h"
#include "system.h"

class Gs2dScreen;
class Ps2Input;
class Ps2SaveFileManager;

class OSystem_PS2 : public OSystem {
public:
	OSystem_PS2(void);
	virtual ~OSystem_PS2(void);
	virtual void initSize(uint width, uint height, int overScale = -1);

	virtual int16 getHeight(void);
	virtual int16 getWidth(void);
	virtual void setPalette(const byte *colors, uint start, uint num);
	virtual void copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h);
	
	virtual void updateScreen();
	virtual void setShakePos(int shakeOffset);

	virtual bool showMouse(bool visible);

	virtual void warpMouse(int x, int y);
	virtual void setMouseCursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y, byte keycolor, int cursorTargetScale = 1);

	virtual uint32 getMillis();
	virtual void delayMillis(uint msecs);
	virtual void setTimerCallback(TimerProc callback, int interval);
	virtual bool pollEvent(Event &event);

	virtual bool setSoundCallback(SoundProc proc, void *param);
	virtual void clearSoundCallback();
	virtual int  getOutputSampleRate(void) const;

	virtual bool openCD(int drive);
	virtual bool pollCD();
	virtual void playCD(int track, int num_loops, int start_frame, int duration);
	virtual void stopCD();
	virtual void updateCD();

	virtual MutexRef createMutex(void);
	virtual void lockMutex(MutexRef mutex);
	virtual void unlockMutex(MutexRef mutex);
	virtual void deleteMutex(MutexRef mutex);

	virtual void showOverlay();
	virtual void hideOverlay();
	virtual void clearOverlay();
	virtual void grabOverlay(OverlayColor *buf, int pitch);
	virtual void copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h);

	virtual const GraphicsMode *getSupportedGraphicsModes() const;
	virtual int getDefaultGraphicsMode() const;
	virtual bool setGraphicsMode(int mode);
	virtual int getGraphicsMode() const;

	virtual void quit();

	virtual OverlayColor RGBToColor(uint8 r, uint8 g, uint8 b);

	virtual void colorToRGB(OverlayColor color, uint8 &r, uint8 &g, uint8 &b);

	virtual Common::SaveFileManager *getSavefileManager();

	void timerThread(void);
	void soundThread(void);
private:
	volatile OSystem::TimerProc _scummTimerProc;
	volatile OSystem::SoundProc _scummSoundProc;
	void *_scummSoundParam;
	int16 *_soundBufL, *_soundBufR;
	int _soundSema;

	void initTimer(void);

	bool loadModules(void);
	bool _mouseVisible;
	bool _useHdd, _useMouse, _useKbd;

	Ps2SaveFileManager *_saveManager;

	uint16 _width, _height;

	Gs2dScreen	*_screen;
	Ps2Input	*_input;
	uint16		_oldMouseX, _oldMouseY;

	uint8		*_timerStack, *_soundStack;
	int32		_timerTid, _soundTid;
	static const GraphicsMode _graphicsMode;
};

#endif // SYSTEMPS2_H

