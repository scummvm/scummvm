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
 */

#ifndef _WII_OSYSTEM_H_
#define _WII_OSYSTEM_H_

#include "base/main.h"
#include "common/fs.h"
#include "common/rect.h"
#include "common/events.h"

#include "backends/base-backend.h"
#include "backends/saves/default/default-saves.h"
#include "backends/timer/default/default-timer.h"
#include "graphics/colormasks.h"
#include "graphics/surface.h"
#include "sound/mixer_intern.h"

#include <gctypes.h>
#include <gccore.h>
#include <ogcsys.h>

#ifdef __cplusplus
extern "C" {
#endif

extern bool reset_btn_pressed;
extern bool power_btn_pressed;

#ifdef DEBUG_WII_MEMSTATS
extern void wii_memstats(void);
#endif

#ifdef __cplusplus
}
#endif

class OSystem_Wii : public BaseBackend {
private:
	s64 _startup_time;

	u16 *_palette;
	u16 *_cursorPalette;
	bool _cursorPaletteDisabled;

	u16 _gameWidth, _gameHeight;
	u8 *_gamePixels;
	Graphics::Surface _surface;

	bool _overlayVisible;
	u16 _overlayWidth, _overlayHeight;
	u32 _overlaySize;
	OverlayColor *_overlayPixels;

	u32 _lastScreenUpdate;
	u16 *_texture;
	u16 _currentWidth, _currentHeight;

	s32 _activeGraphicsMode;

	bool _fullscreen;

	bool _mouseVisible;
	s32 _mouseX, _mouseY;
	u32 _mouseWidth, _mouseHeight;
	s32 _mouseHotspotX, _mouseHotspotY;
	u8 _mouseKeyColor;
	u8 *_mouseCursor;

	bool _kbd_active;

	bool _event_quit;

	u32 _lastPadCheck;

	void initGfx();
	void deinitGfx();

	void initSfx();
	void deinitSfx();

	void initEvents();
	void deinitEvents();
	void updateEventScreenResolution();
	bool pollKeyboard(Common::Event &event);

protected:
	Common::SaveFileManager *_savefile;
	Audio::MixerImpl *_mixer;
	DefaultTimerManager *_timer;

public:
	OSystem_Wii();
	virtual ~OSystem_Wii();

	virtual void initBackend();

	virtual bool hasFeature(Feature f);
	virtual void setFeatureState(Feature f, bool enable);
	virtual bool getFeatureState(Feature f);
	virtual const GraphicsMode *getSupportedGraphicsModes() const;
	virtual int getDefaultGraphicsMode() const;
	virtual bool setGraphicsMode(int mode);
	virtual int getGraphicsMode() const;
	virtual void initSize(uint width, uint height,
							const Graphics::PixelFormat *format);
	virtual int16 getWidth();
	virtual int16 getHeight();
	virtual void setPalette(const byte *colors, uint start, uint num);
	virtual void grabPalette(byte *colors, uint start, uint num);
	virtual void setCursorPalette(const byte *colors, uint start, uint num);
	virtual void disableCursorPalette(bool disable);
	virtual void copyRectToScreen(const byte *buf, int pitch, int x, int y,
									int w, int h);
	virtual void updateScreen();
	virtual Graphics::Surface *lockScreen();
	virtual void unlockScreen();
	virtual void setShakePos(int shakeOffset);

	virtual void showOverlay();
	virtual void hideOverlay();
	virtual void clearOverlay();
	virtual void grabOverlay(OverlayColor *buf, int pitch);
	virtual void copyRectToOverlay(const OverlayColor *buf, int pitch,
									int x, int y, int w, int h);
	virtual int16 getOverlayWidth();
	virtual int16 getOverlayHeight();
	virtual Graphics::PixelFormat getOverlayFormat() const;

	virtual bool showMouse(bool visible);

	virtual void warpMouse(int x, int y);
	virtual void setMouseCursor(const byte *buf, uint w, uint h, int hotspotX,
								int hotspotY, uint32 keycolor,
								int cursorTargetScale,
								const Graphics::PixelFormat *format);

	virtual bool pollEvent(Common::Event &event);
	virtual uint32 getMillis();
	virtual void delayMillis(uint msecs);

	virtual MutexRef createMutex();
	virtual void lockMutex(MutexRef mutex);
	virtual void unlockMutex(MutexRef mutex);
	virtual void deleteMutex(MutexRef mutex);

	typedef void (*SoundProc)(void *param, byte *buf, int len);

	virtual void quit();

	virtual void setWindowCaption(const char *caption);

	virtual Common::SaveFileManager *getSavefileManager();
	virtual Audio::Mixer *getMixer();
	virtual Common::TimerManager *getTimerManager();
	virtual FilesystemFactory *getFilesystemFactory();
	virtual void getTimeAndDate(struct tm &t) const;

	virtual void engineInit();
};

#endif

