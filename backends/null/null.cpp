/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
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

#if defined(USE_NULL_DRIVER)

#include "common/rect.h"
#include "common/savefile.h"

class OSystem_NULL : public OSystem {
public:
	static OSystem *instance();

public:

	OSystem_NULL();
	virtual ~OSystem_NULL();

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

	virtual bool showMouse(bool visible);

	virtual void warpMouse(int x, int y);
	virtual void setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, byte keycolor = 255);

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

	virtual SaveFileManager *getSavefileManager();

};

static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
	{0, 0, 0}
};

int main(int argc, char *argv[]) {
	g_system = OSystem_NULL_create();
	assert(g_system);

	// Invoke the actual ScummVM main entry point:
	int res = scummvm_main(argc, argv);
	g_system->quit();	// TODO: Consider removing / replacing this!
	return res;
}

OSystem_NULL::OSystem_NULL()
{
}

OSystem_NULL::~OSystem_NULL()
{
}

bool OSystem_NULL::hasFeature(Feature f)
{
	return false;
}

void OSystem_NULL::setFeatureState(Feature f, bool enable)
{
}

bool OSystem_NULL::getFeatureState(Feature f)
{
	return false;
}

const OSystem::GraphicsMode* OSystem_NULL::getSupportedGraphicsModes() const
{
	return s_supportedGraphicsModes;
}


int OSystem_NULL::getDefaultGraphicsMode() const
{
	return -1;
}

bool OSystem_NULL::setGraphicsMode(int mode)
{
	return true;
}

bool OSystem_NULL::setGraphicsMode(const char *name)
{
	return true;
}

int OSystem_NULL::getGraphicsMode() const
{
	return -1;
}

void OSystem_NULL::initSize(uint width, uint height, int overlayScale)
{
}

int16 OSystem_NULL::getHeight()
{
	return 320;
}

int16 OSystem_NULL::getWidth()
{
	return 200;
}

void OSystem_NULL::setPalette(const byte *colors, uint start, uint num)
{
}

void OSystem_NULL::copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h)
{
}

void OSystem_NULL::updateScreen()
{
}

void OSystem_NULL::setShakePos(int shakeOffset)
{
}

void OSystem_NULL::showOverlay ()
{
}

void OSystem_NULL::hideOverlay ()
{
}

void OSystem_NULL::clearOverlay ()
{
}

void OSystem_NULL::grabOverlay (OverlayColor *buf, int pitch)
{
}

void OSystem_NULL::copyRectToOverlay (const OverlayColor *buf, int pitch, int x, int y, int w, int h)
{
}

int16 OSystem_NULL::getOverlayHeight()
{
	return getHeight();
}

int16 OSystem_NULL::getOverlayWidth()
{
	return getWidth();
}

OverlayColor OSystem_NULL::RGBToColor(uint8 r, uint8 g, uint8 b)
{
	return 0;
}

void OSystem_NULL::colorToRGB(OverlayColor color, uint8 &r, uint8 &g, uint8 &b)
{
}

bool OSystem_NULL::showMouse(bool visible)
{
	return true;
}

void OSystem_NULL::warpMouse(int x, int y)
{
}

void OSystem_NULL::setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, byte keycolor)
{
}

bool OSystem_NULL::pollEvent(Event &event)
{
	return false;
}

uint32 OSystem_NULL::getMillis()
{
	return 0;
}

void OSystem_NULL::delayMillis(uint msecs)
{
}

void OSystem_NULL::setTimerCallback(TimerProc callback, int interval)
{
}

OSystem::MutexRef OSystem_NULL::createMutex(void)
{
	return NULL;
}

void OSystem_NULL::lockMutex(MutexRef mutex)
{
}

void OSystem_NULL::unlockMutex(MutexRef mutex)
{
}

void OSystem_NULL::deleteMutex(MutexRef mutex)
{
}

bool OSystem_NULL::setSoundCallback(SoundProc proc, void *param)
{
	return true;
}

void OSystem_NULL::clearSoundCallback()
{
}

int OSystem_NULL::getOutputSampleRate() const
{
	return 22050;
}

bool OSystem_NULL::openCD(int drive)
{
	return false;
}

bool OSystem_NULL::pollCD()
{
	return false;
}

void OSystem_NULL::playCD(int track, int num_loops, int start_frame, int duration)
{
}

void OSystem_NULL::stopCD()
{
}

void OSystem_NULL::updateCD()
{
}

void OSystem_NULL::quit()
{
}

void OSystem_NULL::setWindowCaption(const char *caption)
{
}

void OSystem_NULL::displayMessageOnOSD(const char *msg)
{
}

SaveFileManager* OSystem_NULL::getSavefileManager()
{
	return NULL;
}


OSystem *OSystem_NULL_create() {
	return new OSystem_NULL();
}
#else /* USE_NULL_DRIVER */

OSystem *OSystem_NULL_create() {
	return NULL;
}

#endif
