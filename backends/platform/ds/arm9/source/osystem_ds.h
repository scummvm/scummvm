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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
 
 
#ifndef _OSYSTEM_DS_H_
#define _OSYSTEM_DS_H_
#include "common/system.h"
#include "common/events.h"
#include "nds.h"
#include "ramsave.h"
#include "gbampsave.h"
#include "backends/saves/default/default-saves.h"
#include "backends/timer/default/default-timer.h"
#include "sound/mixer_intern.h"
#include "graphics/surface.h"

class DSAudioMixer : public Audio::MixerImpl {	

public:
	DSAudioMixer(OSystem* system) : Audio::MixerImpl(system) { }
};

class DSTimerManager : public DefaultTimerManager {	
};


class OSystem_DS : public OSystem {
protected:

	int eventNum;
	int lastPenFrame;
	
	Common::Event eventQueue[96];
	int queuePos;
	
	DSSaveFileManager saveManager;
	GBAMPSaveFileManager mpSaveManager;
	DSAudioMixer* _mixer;
	DSTimerManager* _timer;
	Graphics::Surface _framebuffer;
	bool _frameBufferExists;
	bool _graphicsEnable;

	static OSystem_DS* _instance;

	u16 _palette[256];
	u16 _cursorPalette[256];

	u8 _cursorImage[64 * 64];
	uint _cursorW;
	uint _cursorH;
	int _cursorHotX;
	int _cursorHotY;
	byte _cursorKey;
	int _cursorScale;

	
	Graphics::Surface* createTempFrameBuffer();
	bool _disableCursorPalette;

public:
	typedef void (*SoundProc)(byte *buf, int len);
	typedef int  (*TimerProc)(int interval);

	OSystem_DS();
	virtual ~OSystem_DS();

	static OSystem_DS *instance() { return _instance; }

	virtual bool hasFeature(Feature f);
	virtual void setFeatureState(Feature f, bool enable);
	virtual bool getFeatureState(Feature f);
	virtual const GraphicsMode *getSupportedGraphicsModes() const;
	virtual int getDefaultGraphicsMode() const;
	virtual bool setGraphicsMode(int mode);
	bool setGraphicsMode(const char *name);
	virtual int getGraphicsMode() const;
	virtual void initSize(uint width, uint height);
	virtual int16 getHeight();
	virtual int16 getWidth();
	virtual void setPalette(const byte *colors, uint start, uint num);
	virtual void grabPalette(unsigned char* colors, uint start, uint num);

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

	inline virtual OverlayColor RGBToColor(uint8 r, uint8 g, uint8 b);
	inline virtual void colorToRGB(OverlayColor color, uint8 &r, uint8 &g, uint8 &b);
	
	virtual bool showMouse(bool visible);

	virtual void warpMouse(int x, int y);
	virtual void setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, byte keycolor = 255, int targetCursorScale = 1);

	virtual bool pollEvent(Common::Event &event);
	virtual uint32 getMillis();
	virtual void delayMillis(uint msecs);
	virtual void getTimeAndDate(struct tm &t) const;

	virtual MutexRef createMutex(void);
	virtual void lockMutex(MutexRef mutex);
	virtual void unlockMutex(MutexRef mutex);
	virtual void deleteMutex(MutexRef mutex);

	virtual int getOutputSampleRate() const;

	virtual bool openCD(int drive);
	virtual bool pollCD();

	virtual void playCD(int track, int num_loops, int start_frame, int duration);
	virtual void stopCD();
	virtual void updateCD();

	virtual void quit();

	virtual void setWindowCaption(const char *caption);

	virtual void displayMessageOnOSD(const char *msg);

	virtual Common::SaveFileManager *getSavefileManager();
	
	void addEvent(Common::Event& e);
	bool isEventQueueEmpty() { return queuePos == 0; }
	
	virtual bool grabRawScreen(Graphics::Surface* surf);
	
	virtual void setFocusRectangle(const Common::Rect& rect);
	
	virtual void clearFocusRectangle();
	
	virtual void initBackend();
	
	virtual Graphics::Surface *lockScreen();
	virtual void unlockScreen();
	
	virtual Audio::Mixer* getMixer() { return _mixer; }
	Audio::MixerImpl* getMixerImpl() { return _mixer; }

	virtual Common::TimerManager* getTimerManager() { return _timer; }
	static int timerHandler(int t);


	virtual void addAutoComplete(const char *word);
	virtual void clearAutoComplete();
	virtual void setCharactersEntered(int count);

	u16 getDSPaletteEntry(u32 entry) { return _palette[entry]; }
	u16 getDSCursorPaletteEntry(u32 entry) { return !_disableCursorPalette? _cursorPalette[entry]: _palette[entry]; }

	virtual void setCursorPalette(const byte *colors, uint start, uint num);

	virtual void disableCursorPalette(bool dis) { _disableCursorPalette = dis; refreshCursor(); }

	FilesystemFactory *getFilesystemFactory();

	void refreshCursor();
};

static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
	{0, 0, 0},
};

OverlayColor OSystem_DS::RGBToColor(uint8 r, uint8 g, uint8 b)
{
	return 0x8000 | (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10);
	//consolePrintf("rgbtocol\n");
	//return 0;
}

void OSystem_DS::colorToRGB(OverlayColor color, uint8 &r, uint8 &g, uint8 &b)
{
	r = (color & 0x001F) << 3;
	g = ((color & 0x03E0) >> 5) << 3;
	b = ((color & 0x7C00) >> 10) << 3;
	//consolePrintf("coltorgb\n");
}

#endif
