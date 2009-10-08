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

#ifndef BE_BASE_H
#define BE_BASE_H

#include <time.h>

#include "PalmVersion.h"
#include "globals.h"

#include "backends/base-backend.h"
#include "common/scummsys.h"
#include "common/events.h"
#include "graphics/surface.h"
#include "graphics/colormasks.h"

namespace Audio {
	class MixerImpl;
}

namespace Common {
	class SaveFileManager;
	class TimerManager;
}

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

// Mouse button event
enum {
	vchrMouseLeft = vchrHardKeyMax - 2,
	vchrMouseRight = vchrHardKeyMax - 1
};

// OSD resource id
#define kDrawKeyState	3000
#define kDrawNumPad		3010
#define kDrawBatLow		3020
#define kDrawFight		3030

#define computeMsecs(x) ((SysTicksPerSecond() * x) / 1000)


//typedef void (*SoundProc)(void *param, byte *buf, int len);
typedef void (*SoundProc)(byte *buf, uint len);
typedef int (*TimerProc)(int interval);

typedef struct {
	UInt32 duration, nextExpiry;
	Boolean active;
	TimerProc callback;
} TimerType, *TimerPtr;

typedef struct {
	Boolean	active;
	void *proc;
	void *param;
} SoundType, *SoundPtr;

class OSystem_PalmBase : public BaseBackend {
private:
	virtual void int_initBackend() { }

	virtual const GraphicsMode *int_getSupportedGraphicsModes() const;
	virtual void int_updateScreen() = 0;
	virtual void int_initSize(uint w, uint h) = 0;
	virtual void int_setShakePos(int shakeOffset) { }

	virtual void extras_palette(uint8 index, uint8 r, uint8 g, uint8 b) { }
	virtual void int_quit() { }

	virtual void unload_gfx_mode() = 0;
	virtual void load_gfx_mode() = 0;

	virtual void draw_mouse() = 0;
	virtual void undraw_mouse() = 0;

	virtual bool check_event(Common::Event &event, EventPtr ev) = 0;

	virtual void timer_handler();
	void battery_handler();
	virtual void get_coordinates(EventPtr ev, Coord &x, Coord &y) = 0;
	void simulate_mouse(Common::Event &event, Int8 iHoriz, Int8 iVert, Coord *xr, Coord *yr);

	virtual void sound_handler() = 0;
	virtual bool setSoundCallback(SoundProc proc, void *param) = 0;
	virtual void clearSoundCallback() = 0;

protected:
	OSystem_PalmBase();

	virtual void draw_osd(UInt16 id, Int32 x, Int32 y, Boolean show, UInt8 color = 0);
	virtual void clear_screen() = 0;

	struct MousePos {
		int16 x,y,w,h;
	};

	TimerType _timer;
	SoundType _sound;

	Common::SaveFileManager *_saveMgr;
	Audio::MixerImpl *_mixerMgr;
	Common::TimerManager *_timerMgr;

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

	UInt32 _keyExtraMask, _keyExtraPressed, _keyExtraRepeat, _keyExtraDelay;
	struct {
		UInt32 bitUp;
		UInt32 bitDown;
		UInt32 bitLeft;
		UInt32 bitRight;
		UInt32 bitActionA;	// left mouse button
		UInt32 bitActionB;	// right mouse button
	} _keyExtra;

	bool _mouseVisible;
	bool _mouseDrawn;
	MousePos _mouseCurState;
	MousePos _mouseOldState;
	int16 _mouseHotspotX;
	int16 _mouseHotspotY;
	byte _mouseKeyColor;
	byte *_mouseDataP, *_mouseBackupP;


	bool _wasKey;
	UInt8 _lastKeyModifier;
	UInt32 _lastKeyRepeat;
	Boolean _useNumPad, _showBatLow;
	UInt32 _batCheckTicks, _batCheckLast;

	int _samplesPerSec;

public:
	void initBackend();

/*
	virtual void setFeatureState(Feature f, bool enable) {};


	bool hasFeature(Feature f);
	bool getFeatureState(Feature f);

	virtual void beginGFXTransaction();
	virtual void endGFXTransaction();

	virtual int16 getOverlayHeight();
	virtual int16 getOverlayWidth();

	virtual void setCursorPalette(const byte *colors, uint start, uint num);
	virtual void disableCursorPalette(bool disable);

	virtual void displayMessageOnOSD(const char *msg);
*/
	const GraphicsMode *getSupportedGraphicsModes() const;
	int getGraphicsMode() const;
	virtual int getDefaultGraphicsMode() const;
	virtual bool setGraphicsMode(int mode);

	void initSize(uint w, uint h);
	int16 getWidth() { return _screenWidth; }
	int16 getHeight() { return _screenHeight; }

	void setShakePos(int shakeOffset);
	virtual void copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h) = 0;
	virtual void updateScreen();

	bool showMouse(bool visible);
	void warpMouse(int x, int y);
	void setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, byte keycolor, int cursorTargetScale);

	virtual void showOverlay() = 0;
	virtual void hideOverlay() = 0;
	virtual void clearOverlay() = 0;
	virtual void grabOverlay(OverlayColor *buf, int pitch) = 0;
	virtual void copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h) = 0;

	int16 getOverlayWidth() { return getWidth(); }
	int16 getOverlayHeight() { return getHeight(); }

	void setPalette(const byte *colors, uint start, uint num);
	void grabPalette(byte *colors, uint start, uint num);
	virtual Graphics::PixelFormat getOverlayFormat() const { return Graphics::createPixelFormat<565>(); }

	bool pollEvent(Common::Event &event);

	void getTimeAndDate(TimeDate &t) const;
	virtual uint32 getMillis();
	virtual void delayMillis(uint msecs);

	virtual void setTimerCallback(TimerProc callback, int interval);

	virtual MutexRef createMutex() { return NULL; }
	virtual void lockMutex(MutexRef mutex) {}
	virtual void unlockMutex(MutexRef mutex) {}
	virtual void deleteMutex(MutexRef mutex) {}

	int getOutputSampleRate() const { return _samplesPerSec; }
	virtual Audio::Mixer *getMixer();

	void quit();
	virtual void setWindowCaption(const char *caption) = 0;

	Common::SaveFileManager *getSavefileManager();
	Common::TimerManager *getTimerManager();
	FilesystemFactory *getFilesystemFactory();

	virtual Common::SeekableReadStream *createConfigReadStream();
	virtual Common::WriteStream *createConfigWriteStream();
};

#endif
