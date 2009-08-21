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

#include <SDL.h>

#include <pspctrl.h>

enum GraphicModeID {
	CENTERED_320X200,
	CENTERED_435X272,
	STRETCHED_480X272,
	CENTERED_362X272
};

class OSystem_PSP : public BaseBackend {
public:
	static const OSystem::GraphicsMode s_supportedGraphicsModes[];
	static OSystem *instance();

protected:
	struct Vertex {
		float u,v;
		float x,y,z;
	};

	uint16	_screenWidth;
	uint16	_screenHeight;
	uint16  _overlayWidth;
	uint16  _overlayHeight;
	byte	*_offscreen;
	OverlayColor  *_overlayBuffer;
	uint16  _palette[256];
	bool	_overlayVisible;
	uint32	_shakePos;
	uint32	_lastScreenUpdate;

	Graphics::Surface _framebuffer;

	bool	_mouseVisible;
	int	_mouseX, _mouseY;
	int	_mouseWidth, _mouseHeight;
	int	_mouseHotspotX, _mouseHotspotY;
	byte	_mouseKeyColour;
	byte	*_mouseBuf;
	bool	_cursorPaletteDisabled;

	int _graphicMode;
	Vertex *_vertices;
	unsigned short* _clut;
	unsigned short* _kbdClut;
	bool _keyboardVisible;
	int _keySelected;
	int _keyboardMode;

	uint32	_prevButtons;
	uint32	_lastPadCheck;
	uint32	_padAccel;

	uint32		_samplesPerSec;
	SceCtrlData pad;

	Common::SaveFileManager *_savefile;
	Audio::MixerImpl *_mixer;
	Common::TimerManager *_timer;

public:

	OSystem_PSP();
	virtual ~OSystem_PSP();

	virtual void initBackend();

	virtual bool hasFeature(Feature f);
	virtual void setFeatureState(Feature f, bool enable);
	virtual bool getFeatureState(Feature f);
	virtual const GraphicsMode *getSupportedGraphicsModes() const;
	virtual int getDefaultGraphicsMode() const;
	virtual bool setGraphicsMode(int mode);
	bool setGraphicsMode(const char *name);
	virtual int getGraphicsMode() const;
	virtual void initSize(uint width, uint height, const Graphics::PixelFormat *format);
	virtual int16 getWidth();
	virtual int16 getHeight();
	virtual void setPalette(const byte *colors, uint start, uint num);
	virtual void setCursorPalette(const byte *colors, uint start, uint num);
	virtual void disableCursorPalette(bool disable);
	virtual void copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h);
	virtual Graphics::Surface *lockScreen();
	virtual void unlockScreen();
	virtual void updateScreen();
	virtual void setShakePos(int shakeOffset);

	virtual void showOverlay();
	virtual void hideOverlay();
	virtual void clearOverlay();
	virtual void grabOverlay(OverlayColor *buf, int pitch);
	virtual void copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h);
	virtual int16 getOverlayHeight();
	virtual int16 getOverlayWidth();
	virtual void grabPalette(byte *colors, uint start, uint num);
	virtual Graphics::PixelFormat getOverlayFormat() const { return Graphics::createPixelFormat<4444>(); }

	virtual bool showMouse(bool visible);

	virtual void warpMouse(int x, int y);
	virtual void setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, int cursorTargetScale, const Graphics::PixelFormat *format);

	virtual bool pollEvent(Common::Event &event);
	virtual bool processInput(Common::Event &event);
	virtual uint32 getMillis();
	virtual void delayMillis(uint msecs);

	typedef int (*TimerProc)(int interval);
	virtual void setTimerCallback(TimerProc callback, int interval);

	virtual MutexRef createMutex(void);
	virtual void lockMutex(MutexRef mutex);
	virtual void unlockMutex(MutexRef mutex);
	virtual void deleteMutex(MutexRef mutex);

	static void mixCallback(void *sys, byte *samples, int len);
	virtual void setupMixer(void);

	Common::SaveFileManager *getSavefileManager() { return _savefile; }
	Audio::Mixer *getMixer() { return _mixer; }
	Common::TimerManager *getTimerManager() { return _timer; }
	FilesystemFactory *getFilesystemFactory() { return &PSPFilesystemFactory::instance(); }
	void getTimeAndDate(struct tm &t) const;

	virtual void quit();

	virtual Common::SeekableReadStream *createConfigReadStream();
	virtual Common::WriteStream *createConfigWriteStream();
};


#endif /* OSYS_PSP_H */
