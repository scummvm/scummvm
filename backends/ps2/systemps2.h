/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
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
 * $Header$
 *
 */

#ifndef SYSTEMPS2_H
#define SYSTEMPS2_H

#include "common/stdafx.h"
#include "common/system.h"

class Gs2dScreen;
class Ps2Input;
class Ps2SaveFileManager;

extern void sioprintf(const char *zFormat, ...);

#define MAX_MUTEXES 16

struct Ps2Mutex {
	int sema;
	int owner;
	int count;
};

class OSystem_PS2 : public OSystem {
public:
	OSystem_PS2(void);
	virtual ~OSystem_PS2(void);
	virtual void initSize(uint width, uint height, int overScale = -1);

	virtual int16 getHeight(void);
	virtual int16 getWidth(void);
	virtual void setPalette(const byte *colors, uint start, uint num);
	virtual void copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h);
	virtual void setShakePos(int shakeOffset);
	virtual void grabPalette(byte *colors, uint start, uint num);
	virtual bool grabRawScreen(Graphics::Surface *surf);
	virtual void updateScreen();

	virtual void showOverlay();
	virtual void hideOverlay();
	virtual void clearOverlay();
	virtual void grabOverlay(OverlayColor *buf, int pitch);
	virtual void copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h);

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
	void msgPrintf(int millis, char *format, ...);
	void makeConfigPath(char *dest);

	void powerOffCallback(void);
	bool hddPresent(void);

private:
	volatile OSystem::TimerProc _scummTimerProc;
	volatile OSystem::SoundProc _scummSoundProc;
	void *_scummSoundParam;
	int _soundSema;

	void initTimer(void);
	void readRtcTime(void);

	void loadModules(void);
	bool _mouseVisible;
	bool _useMouse, _useKbd, _useHdd;

	Ps2SaveFileManager *_saveManager;

	Gs2dScreen	*_screen;
	Ps2Input	*_input;
	uint16		_oldMouseX, _oldMouseY;
	uint32		_msgClearTime;
	uint16		_printY;

	int			_mutexSema;
	Ps2Mutex	_mutex[MAX_MUTEXES];

	uint8		*_timerStack, *_soundStack;
	int			_timerTid, _soundTid;
	int			_intrId;
	volatile bool _systemQuit;
	static const GraphicsMode _graphicsMode;
};

#endif // SYSTEMPS2_H

