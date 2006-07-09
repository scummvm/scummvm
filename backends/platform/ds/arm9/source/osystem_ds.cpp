/* ScummVMDS - Scumm Interpreter DS Port
 * Copyright (C) 2002-2004 The ScummVM project and Neil Millstone
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
 */


#include "stdafx.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "system.h"

#include "common/util.h"
#include "common/rect.h"
#include "common/savefile.h"

#include "osystem_ds.h"
#include "nds.h"
#include "dsmain.h"
#include "nds/registers_alt.h"
#include "config-manager.h"
#include "common/str.h"
#include "cdaudio.h"
#include "graphics/surface.h"

OSystem_DS* OSystem_DS::_instance = NULL;

OSystem_DS::OSystem_DS()
{
	eventNum = 0;
	lastPenFrame = 0;
	queuePos = 0;
	_instance = this;
}

OSystem_DS::~OSystem_DS() {
}

void OSystem_DS::initBackend() {
	ConfMan.setInt("autosave_period", 0);
	ConfMan.setBool("FM_medium_quality", true);
}

bool OSystem_DS::hasFeature(Feature f) {
//	consolePrintf("hasfeature\n");
	return (f == kFeatureVirtualKeyboard);
}

void OSystem_DS::setFeatureState(Feature f, bool enable) {
//	consolePrintf("setfeature f=%d e=%d\n", f, enable);
	if (f == kFeatureVirtualKeyboard) DS::setKeyboardIcon(enable);
}

bool OSystem_DS::getFeatureState(Feature f) {
//	consolePrintf("getfeat\n");
	if (f == kFeatureVirtualKeyboard) return DS::getKeyboardIcon();
	return false;
}

const OSystem::GraphicsMode* OSystem_DS::getSupportedGraphicsModes() const {
	return s_supportedGraphicsModes;
}


int OSystem_DS::getDefaultGraphicsMode() const {
	return 0;
}

bool OSystem_DS::setGraphicsMode(int mode) {
	return true;
}

bool OSystem_DS::setGraphicsMode(const char *name) {
//	consolePrintf("Set gfx mode %s\n", name);
	return true;
}

int OSystem_DS::getGraphicsMode() const {
	return -1;
}

void OSystem_DS::initSize(uint width, uint height) {
//	consolePrintf("Set gfx mode %d x %d\n", width, height);
	DS::setGameSize(width, height);
}

int16 OSystem_DS::getHeight() {
	return 200;
}

int16 OSystem_DS::getWidth() {
	return 320;
}

void OSystem_DS::setPalette(const byte *colors, uint start, uint num) {
//	consolePrintf("Set palette %d, %d colours\n", start, num);
	if (!DS::getIsDisplayMode8Bit()) return;
	
	for (unsigned int r = start; r < start + num; r++) {
		int red = *colors;
		int green = *(colors + 1);
		int blue = *(colors + 2);
		
		red >>= 3;
		green >>= 3;
		blue >>= 3;
		
		BG_PALETTE[r] = red | (green << 5) | (blue << 10);
		if (!DS::getKeyboardEnable()) {
			BG_PALETTE_SUB[r] = red | (green << 5) | (blue << 10);
		}
//		if (num == 16) consolePrintf("pal:%d r:%d g:%d b:%d\n", r, red, green, blue);
		
		colors += 4;
	}
}


void OSystem_DS::grabPalette(unsigned char *colors, uint start, uint num) {
//	consolePrintf("Grabpalette");
	
	for (unsigned int r = start; r < start + num; r++) {
		*colors++ = (BG_PALETTE[r] & 0x001F) << 3;
		*colors++ = (BG_PALETTE[r] & 0x03E0) >> 5 << 3;
		*colors++ = (BG_PALETTE[r] & 0x7C00) >> 10 << 3;
	}
}


void OSystem_DS::copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h)
{
//	consolePrintf("Copy rect %d, %d   %d, %d ", x, y, w, h);
	
	if (w <= 1) return;
	if (h < 0) return;
	if (!DS::getIsDisplayMode8Bit()) return;
	
	u16* bgSub = (u16 *) BG_GFX_SUB;
	u16* bg = (u16 *) DS::get8BitBackBuffer();
	u16* src = (u16 *) buf;
	
	if (DS::getKeyboardEnable()) {
	
		for (int dy = y; dy < y + h; dy++) {
			u16* dest = bg + (dy << 8) + (x >> 1);
		
			DC_FlushRange(src, w << 1);
			DC_FlushRange(dest, w << 1);
			dmaCopyHalfWords(3, src, dest, w);
					
			src += pitch >> 1;
		}
	
	} else {
		for (int dy = y; dy < y + h; dy++) {
			u16* dest1 = bg + (dy << 8) + (x >> 1);
			u16* dest2 = bgSub + (dy << 8) + (x >> 1);
			
			DC_FlushRange(src, w << 1);
			DC_FlushRange(dest1, w << 1);
			DC_FlushRange(dest2, w << 1);
					
			dmaCopyHalfWords(3, src, dest1, w);
			dmaCopyHalfWords(3, src, dest2, w);
					
			src += pitch >> 1;
		}
	}
	
//	consolePrintf("Done\n");
	
			
	
}

void OSystem_DS::updateScreen()
{
	DS::displayMode16BitFlipBuffer();
	DS::doSoundCallback();
//	DS::doTimerCallback();
	DS::addEventsToQueue();
}

void OSystem_DS::setShakePos(int shakeOffset) {
	DS::setShakePos(shakeOffset);
}

void OSystem_DS::showOverlay ()
{
//	consolePrintf("showovl\n");
	DS::displayMode16Bit();
}

void OSystem_DS::hideOverlay ()
{
	DS::displayMode8Bit();
}

void OSystem_DS::clearOverlay ()
{
	memset((u16 *) DS::get16BitBackBuffer(), 0, 512 * 256 * 2);
//	consolePrintf("clearovl\n");
}

void OSystem_DS::grabOverlay (OverlayColor *buf, int pitch)
{
//	consolePrintf("grabovl\n");
}

void OSystem_DS::copyRectToOverlay (const OverlayColor *buf, int pitch, int x, int y, int w, int h)
{
	u16* bg = (u16 *) DS::get16BitBackBuffer();
	u16* src = (u16 *) buf;
		
//	if (x + w > 256) w = 256 - x;
	//if (x + h > 256) h = 256 - y;

//	consolePrintf("Copy rect ovl %d, %d   %d, %d  %d\n", x, y, w, h, pitch);

	
	
	for (int dy = y; dy < y + h; dy++) {
		
		
		// Slow but save copy:
		for (int dx = x; dx < x + w; dx++) {
			
			*(bg + (dy * 512) + dx) = *src;
			//if ((*src) != 0) consolePrintf("%d,%d: %d   ", dx, dy, *src);
			//consolePrintf("%d,", *src);
			src++;
		}
		src += (pitch - w);
		
		// Fast but broken copy: (why?)
		/*
		REG_IME = 0;
		dmaCopy(src, bg + (dy << 9) + x, w * 2);
		REG_IME = 1;
		
		src += pitch;*/
	}
			
//	consolePrintf("Copy rect ovl done");

}

int16 OSystem_DS::getOverlayHeight()
{
//	consolePrintf("getovlheight\n");
	return getHeight();
}

int16 OSystem_DS::getOverlayWidth()
{
//	consolePrintf("getovlwid\n");
	return getWidth();
}

	
bool OSystem_DS::showMouse(bool visible)
{
	return true;
}

void OSystem_DS::warpMouse(int x, int y)
{
}

void OSystem_DS::setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, byte keycolor, int targetCursorScale) {
	DS::setCursorIcon(buf, w, h, keycolor);
}

void OSystem_DS::addEvent(Event& e) {
	eventQueue[queuePos++] = e;
}

bool OSystem_DS::pollEvent(Event &event)
{

	if (lastPenFrame != DS::getMillis()) {

		if (eventNum == queuePos) {
			eventNum = 0;
			queuePos = 0;
			// Bodge - this last event seems to be processed sometimes and not others.
			// So we make it something harmless which won't cause any adverse effects.
			event.type = EVENT_KEYUP;
			event.kbd.ascii = 0;
			event.kbd.keycode = 0;
			event.kbd.flags = 0;
			return false;
		} else {
			event = eventQueue[eventNum++];
			return true;
		}
	}
	
	return false;

/*	if (lastPenFrame != DS::getMillis()) {
		if ((eventNum == 0)) {
			event.type = EVENT_MOUSEMOVE;
			event.mouse = Common::Point(DS::getPenX(), DS::getPenY());
			eventNum = 1;
			return true;
		}
		if (eventNum == 1) {
			eventNum = 0;
			lastPenFrame = DS::getMillis();
			if (DS::getPenDown()) {	
				event.type = EVENT_LBUTTONDOWN;
				event.mouse = Common::Point(DS::getPenX(), DS::getPenY());
				consolePrintf("Down %d, %d  ", event.mouse.x, event.mouse.y);
				return true;
			} else if (DS::getPenReleased()) {
				event.type = EVENT_LBUTTONUP;
				event.mouse = Common::Point(DS::getPenX(), DS::getPenY());
				consolePrintf("Up %d, %d ", event.mouse.x, event.mouse.y);
				return true;
			} else {
				return false;
			}
		}
	}*/

	return false;
}

uint32 OSystem_DS::getMillis()
{
	return DS::getMillis();
}

void OSystem_DS::delayMillis(uint msecs)
{
	int st = getMillis();
	DS::addEventsToQueue();
	DS::CD::update();
	
	DS::doSoundCallback();
	while (st + msecs >= getMillis()) {
		DS::doSoundCallback();
	}
	
	DS::doTimerCallback();
	DS::checkSleepMode();
	DS::addEventsToQueue();
}

void OSystem_DS::setTimerCallback(TimerProc callback, int interval)
{
//	consolePrintf("Settimercallback interval=%d\n", interval);
	DS::setTimerCallback(callback, interval);
}

OSystem::MutexRef OSystem_DS::createMutex(void)
{
	return NULL;
}

void OSystem_DS::lockMutex(MutexRef mutex)
{
}

void OSystem_DS::unlockMutex(MutexRef mutex)
{
}

void OSystem_DS::deleteMutex(MutexRef mutex)
{
}

bool OSystem_DS::setSoundCallback(SoundProc proc, void *param)
{
//	consolePrintf("Setsoundcallback");
	DS::setSoundProc(proc, param);
	return true;
}

void OSystem_DS::clearSoundCallback()
{
	consolePrintf("Clearing sound callback");
//	DS::setSoundProc(NULL, NULL);
}

int OSystem_DS::getOutputSampleRate() const
{
	return 11025;
}

bool OSystem_DS::openCD(int drive)
{
	return DS::CD::checkCD();
}

bool OSystem_DS::pollCD()
{
	return DS::CD::isPlaying();
}

void OSystem_DS::playCD(int track, int num_loops, int start_frame, int duration)
{
	DS::CD::playTrack(track, num_loops, start_frame, duration);
}

void OSystem_DS::stopCD()
{
	DS::CD::stopTrack();
}

void OSystem_DS::updateCD()
{
}

void OSystem_DS::quit()
{
/*	consolePrintf("Soft resetting...");
	IPC->reset = 1;
	REG_IE = 0;
	
	asm("swi 0x26\n");
	swiSoftReset();*/
}

void OSystem_DS::setWindowCaption(const char *caption)
{
}

void OSystem_DS::displayMessageOnOSD(const char *msg)
{
}

Common::SaveFileManager* OSystem_DS::getSavefileManager()
{
	bool forceSram;

	if (ConfMan.hasKey("forcesramsave", "ds")) {
		forceSram = ConfMan.getBool("forcesramsave", "ds");
	} else {
		forceSram = false;
	}
	if (forceSram) { 
		consolePrintf("Using SRAM save method!\n");
	}
	
	if (DS::isGBAMPAvailable() && (!forceSram)) {
		return &mpSaveManager;
	} else {
		return &saveManager;
	}
}

bool OSystem_DS::grabRawScreen(Graphics::Surface* surf) {
	surf->create(DS::getGameWidth(), DS::getGameHeight(), 1);
	memcpy(surf->pixels, DS::get8BitBackBuffer(), DS::getGameWidth() * DS::getGameHeight());
	return true;
}

void OSystem_DS::setFocusRectangle(const Common::Rect& rect) {
	DS::setTalkPos(rect.left + rect.width() / 2, rect.top + rect.height() / 2);
}

void OSystem_DS::clearFocusRectangle() {

}


OSystem *OSystem_DS_create() {
	return new OSystem_DS();
}

/* ScummVMDS - Scumm Interpreter DS Port
 * Copyright (C) 2002-2004 The ScummVM project and Neil Millstone
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
 */


#include "stdafx.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "system.h"

#include "common/util.h"
#include "common/rect.h"
#include "common/savefile.h"

#include "osystem_ds.h"
#include "nds.h"
#include "dsmain.h"
#include "nds/registers_alt.h"
#include "config-manager.h"
#include "common/str.h"
#include "cdaudio.h"
#include "graphics/surface.h"

OSystem_DS* OSystem_DS::_instance = NULL;

OSystem_DS::OSystem_DS()
{
	eventNum = 0;
	lastPenFrame = 0;
	queuePos = 0;
	_instance = this;
}

OSystem_DS::~OSystem_DS() {
}

void OSystem_DS::initBackend() {
	ConfMan.setInt("autosave_period", 0);
	ConfMan.setBool("FM_medium_quality", true);
}

bool OSystem_DS::hasFeature(Feature f) {
//	consolePrintf("hasfeature\n");
	return (f == kFeatureVirtualKeyboard);
}

void OSystem_DS::setFeatureState(Feature f, bool enable) {
//	consolePrintf("setfeature f=%d e=%d\n", f, enable);
	if (f == kFeatureVirtualKeyboard) DS::setKeyboardIcon(enable);
}

bool OSystem_DS::getFeatureState(Feature f) {
//	consolePrintf("getfeat\n");
	if (f == kFeatureVirtualKeyboard) return DS::getKeyboardIcon();
	return false;
}

const OSystem::GraphicsMode* OSystem_DS::getSupportedGraphicsModes() const {
	return s_supportedGraphicsModes;
}


int OSystem_DS::getDefaultGraphicsMode() const {
	return 0;
}

bool OSystem_DS::setGraphicsMode(int mode) {
	return true;
}

bool OSystem_DS::setGraphicsMode(const char *name) {
//	consolePrintf("Set gfx mode %s\n", name);
	return true;
}

int OSystem_DS::getGraphicsMode() const {
	return -1;
}

void OSystem_DS::initSize(uint width, uint height) {
//	consolePrintf("Set gfx mode %d x %d\n", width, height);
	DS::setGameSize(width, height);
}

int16 OSystem_DS::getHeight() {
	return 200;
}

int16 OSystem_DS::getWidth() {
	return 320;
}

void OSystem_DS::setPalette(const byte *colors, uint start, uint num) {
//	consolePrintf("Set palette %d, %d colours\n", start, num);
	if (!DS::getIsDisplayMode8Bit()) return;
	
	for (unsigned int r = start; r < start + num; r++) {
		int red = *colors;
		int green = *(colors + 1);
		int blue = *(colors + 2);
		
		red >>= 3;
		green >>= 3;
		blue >>= 3;
		
		BG_PALETTE[r] = red | (green << 5) | (blue << 10);
		if (!DS::getKeyboardEnable()) {
			BG_PALETTE_SUB[r] = red | (green << 5) | (blue << 10);
		}
//		if (num == 16) consolePrintf("pal:%d r:%d g:%d b:%d\n", r, red, green, blue);
		
		colors += 4;
	}
}


void OSystem_DS::grabPalette(unsigned char *colors, uint start, uint num) {
//	consolePrintf("Grabpalette");
	
	for (unsigned int r = start; r < start + num; r++) {
		*colors++ = (BG_PALETTE[r] & 0x001F) << 3;
		*colors++ = (BG_PALETTE[r] & 0x03E0) >> 5 << 3;
		*colors++ = (BG_PALETTE[r] & 0x7C00) >> 10 << 3;
	}
}


void OSystem_DS::copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h)
{
//	consolePrintf("Copy rect %d, %d   %d, %d ", x, y, w, h);
	
	if (w <= 1) return;
	if (h < 0) return;
	if (!DS::getIsDisplayMode8Bit()) return;
	
	u16* bgSub = (u16 *) BG_GFX_SUB;
	u16* bg = (u16 *) DS::get8BitBackBuffer();
	u16* src = (u16 *) buf;
	
	if (DS::getKeyboardEnable()) {
	
		for (int dy = y; dy < y + h; dy++) {
			u16* dest = bg + (dy << 8) + (x >> 1);
		
			DC_FlushRange(src, w << 1);
			DC_FlushRange(dest, w << 1);
			dmaCopyHalfWords(3, src, dest, w);
					
			src += pitch >> 1;
		}
	
	} else {
		for (int dy = y; dy < y + h; dy++) {
			u16* dest1 = bg + (dy << 8) + (x >> 1);
			u16* dest2 = bgSub + (dy << 8) + (x >> 1);
			
			DC_FlushRange(src, w << 1);
			DC_FlushRange(dest1, w << 1);
			DC_FlushRange(dest2, w << 1);
					
			dmaCopyHalfWords(3, src, dest1, w);
			dmaCopyHalfWords(3, src, dest2, w);
					
			src += pitch >> 1;
		}
	}
	
//	consolePrintf("Done\n");
	
			
	
}

void OSystem_DS::updateScreen()
{
	DS::displayMode16BitFlipBuffer();
	DS::doSoundCallback();
//	DS::doTimerCallback();
	DS::addEventsToQueue();
}

void OSystem_DS::setShakePos(int shakeOffset) {
	DS::setShakePos(shakeOffset);
}

void OSystem_DS::showOverlay ()
{
//	consolePrintf("showovl\n");
	DS::displayMode16Bit();
}

void OSystem_DS::hideOverlay ()
{
	DS::displayMode8Bit();
}

void OSystem_DS::clearOverlay ()
{
	memset((u16 *) DS::get16BitBackBuffer(), 0, 512 * 256 * 2);
//	consolePrintf("clearovl\n");
}

void OSystem_DS::grabOverlay (OverlayColor *buf, int pitch)
{
//	consolePrintf("grabovl\n");
}

void OSystem_DS::copyRectToOverlay (const OverlayColor *buf, int pitch, int x, int y, int w, int h)
{
	u16* bg = (u16 *) DS::get16BitBackBuffer();
	u16* src = (u16 *) buf;
		
//	if (x + w > 256) w = 256 - x;
	//if (x + h > 256) h = 256 - y;

//	consolePrintf("Copy rect ovl %d, %d   %d, %d  %d\n", x, y, w, h, pitch);

	
	
	for (int dy = y; dy < y + h; dy++) {
		
		
		// Slow but save copy:
		for (int dx = x; dx < x + w; dx++) {
			
			*(bg + (dy * 512) + dx) = *src;
			//if ((*src) != 0) consolePrintf("%d,%d: %d   ", dx, dy, *src);
			//consolePrintf("%d,", *src);
			src++;
		}
		src += (pitch - w);
		
		// Fast but broken copy: (why?)
		/*
		REG_IME = 0;
		dmaCopy(src, bg + (dy << 9) + x, w * 2);
		REG_IME = 1;
		
		src += pitch;*/
	}
			
//	consolePrintf("Copy rect ovl done");

}

int16 OSystem_DS::getOverlayHeight()
{
//	consolePrintf("getovlheight\n");
	return getHeight();
}

int16 OSystem_DS::getOverlayWidth()
{
//	consolePrintf("getovlwid\n");
	return getWidth();
}

	
bool OSystem_DS::showMouse(bool visible)
{
	return true;
}

void OSystem_DS::warpMouse(int x, int y)
{
}

void OSystem_DS::setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, byte keycolor, int targetCursorScale) {
	DS::setCursorIcon(buf, w, h, keycolor);
}

void OSystem_DS::addEvent(Event& e) {
	eventQueue[queuePos++] = e;
}

bool OSystem_DS::pollEvent(Event &event)
{

	if (lastPenFrame != DS::getMillis()) {

		if (eventNum == queuePos) {
			eventNum = 0;
			queuePos = 0;
			// Bodge - this last event seems to be processed sometimes and not others.
			// So we make it something harmless which won't cause any adverse effects.
			event.type = EVENT_KEYUP;
			event.kbd.ascii = 0;
			event.kbd.keycode = 0;
			event.kbd.flags = 0;
			return false;
		} else {
			event = eventQueue[eventNum++];
			return true;
		}
	}
	
	return false;

/*	if (lastPenFrame != DS::getMillis()) {
		if ((eventNum == 0)) {
			event.type = EVENT_MOUSEMOVE;
			event.mouse = Common::Point(DS::getPenX(), DS::getPenY());
			eventNum = 1;
			return true;
		}
		if (eventNum == 1) {
			eventNum = 0;
			lastPenFrame = DS::getMillis();
			if (DS::getPenDown()) {	
				event.type = EVENT_LBUTTONDOWN;
				event.mouse = Common::Point(DS::getPenX(), DS::getPenY());
				consolePrintf("Down %d, %d  ", event.mouse.x, event.mouse.y);
				return true;
			} else if (DS::getPenReleased()) {
				event.type = EVENT_LBUTTONUP;
				event.mouse = Common::Point(DS::getPenX(), DS::getPenY());
				consolePrintf("Up %d, %d ", event.mouse.x, event.mouse.y);
				return true;
			} else {
				return false;
			}
		}
	}*/

	return false;
}

uint32 OSystem_DS::getMillis()
{
	return DS::getMillis();
}

void OSystem_DS::delayMillis(uint msecs)
{
	int st = getMillis();
	DS::addEventsToQueue();
	DS::CD::update();
	
	DS::doSoundCallback();
	while (st + msecs >= getMillis()) {
		DS::doSoundCallback();
	}
	
	DS::doTimerCallback();
	DS::checkSleepMode();
	DS::addEventsToQueue();
}

void OSystem_DS::setTimerCallback(TimerProc callback, int interval)
{
//	consolePrintf("Settimercallback interval=%d\n", interval);
	DS::setTimerCallback(callback, interval);
}

OSystem::MutexRef OSystem_DS::createMutex(void)
{
	return NULL;
}

void OSystem_DS::lockMutex(MutexRef mutex)
{
}

void OSystem_DS::unlockMutex(MutexRef mutex)
{
}

void OSystem_DS::deleteMutex(MutexRef mutex)
{
}

bool OSystem_DS::setSoundCallback(SoundProc proc, void *param)
{
//	consolePrintf("Setsoundcallback");
	DS::setSoundProc(proc, param);
	return true;
}

void OSystem_DS::clearSoundCallback()
{
	consolePrintf("Clearing sound callback");
//	DS::setSoundProc(NULL, NULL);
}

int OSystem_DS::getOutputSampleRate() const
{
	return 11025;
}

bool OSystem_DS::openCD(int drive)
{
	return DS::CD::checkCD();
}

bool OSystem_DS::pollCD()
{
	return DS::CD::isPlaying();
}

void OSystem_DS::playCD(int track, int num_loops, int start_frame, int duration)
{
	DS::CD::playTrack(track, num_loops, start_frame, duration);
}

void OSystem_DS::stopCD()
{
	DS::CD::stopTrack();
}

void OSystem_DS::updateCD()
{
}

void OSystem_DS::quit()
{
/*	consolePrintf("Soft resetting...");
	IPC->reset = 1;
	REG_IE = 0;
	
	asm("swi 0x26\n");
	swiSoftReset();*/
}

void OSystem_DS::setWindowCaption(const char *caption)
{
}

void OSystem_DS::displayMessageOnOSD(const char *msg)
{
}

Common::SaveFileManager* OSystem_DS::getSavefileManager()
{
	bool forceSram;

	if (ConfMan.hasKey("forcesramsave", "ds")) {
		forceSram = ConfMan.getBool("forcesramsave", "ds");
	} else {
		forceSram = false;
	}
	if (forceSram) { 
		consolePrintf("Using SRAM save method!\n");
	}
	
	if (DS::isGBAMPAvailable() && (!forceSram)) {
		return &mpSaveManager;
	} else {
		return &saveManager;
	}
}

bool OSystem_DS::grabRawScreen(Graphics::Surface* surf) {
	surf->create(DS::getGameWidth(), DS::getGameHeight(), 1);
	memcpy(surf->pixels, DS::get8BitBackBuffer(), DS::getGameWidth() * DS::getGameHeight());
	return true;
}

void OSystem_DS::setFocusRectangle(Common::Rect& rect) {
	DS::setTalkPos(rect.left + rect.width() / 2, rect.top + rect.height() / 2);
}

void OSystem_DS::clearFocusRectangle() {

}


OSystem *OSystem_DS_create() {
	return new OSystem_DS();
}

