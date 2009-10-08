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

#include "backends/base-backend.h"
#include "base/main.h"

#if defined(USE_NULL_DRIVER)

#ifdef UNIX
#include <unistd.h>
#include <sys/time.h>
#endif

#include "common/rect.h"
#include "graphics/colormasks.h"

#include "backends/saves/default/default-saves.h"
#include "backends/timer/default/default-timer.h"
#include "sound/mixer_intern.h"

/*
 * Include header files needed for the getFilesystemFactory() method.
 */
#if defined(__amigaos4__)
	#include "backends/fs/amigaos4/amigaos4-fs-factory.h"
#elif defined(UNIX)
	#include "backends/fs/posix/posix-fs-factory.h"
#elif defined(WIN32)
	#include "backends/fs/windows/windows-fs-factory.h"
#endif

class OSystem_NULL : public BaseBackend {
protected:
	Common::SaveFileManager *_savefile;
	Audio::MixerImpl *_mixer;
	Common::TimerManager *_timer;
	FilesystemFactory *_fsFactory;

	timeval _startTime;
public:

	OSystem_NULL();
	virtual ~OSystem_NULL();

	virtual void initBackend();

	virtual bool hasFeature(Feature f);
	virtual void setFeatureState(Feature f, bool enable);
	virtual bool getFeatureState(Feature f);
	virtual const GraphicsMode *getSupportedGraphicsModes() const;
	virtual int getDefaultGraphicsMode() const;
	bool setGraphicsMode(const char *name);
	virtual bool setGraphicsMode(int mode);
	virtual int getGraphicsMode() const;
	virtual void initSize(uint width, uint height);
	virtual int16 getHeight();
	virtual int16 getWidth();
	virtual void setPalette(const byte *colors, uint start, uint num);
	virtual void grabPalette(byte *colors, uint start, uint num);
	virtual void copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h);
	virtual void updateScreen();
	virtual Graphics::Surface *lockScreen();
	virtual void unlockScreen();
	virtual void setShakePos(int shakeOffset);

	virtual void showOverlay();
	virtual void hideOverlay();
	virtual void clearOverlay();
	virtual void grabOverlay(OverlayColor *buf, int pitch);
	virtual void copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h);
	virtual int16 getOverlayHeight();
	virtual int16 getOverlayWidth();
	virtual Graphics::PixelFormat getOverlayFormat() const { return Graphics::createPixelFormat<565>(); }

	virtual bool showMouse(bool visible);

	virtual void warpMouse(int x, int y);
	virtual void setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, byte keycolor = 255, int cursorTargetScale = 1);

	virtual bool pollEvent(Common::Event &event);
	virtual uint32 getMillis();
	virtual void delayMillis(uint msecs);

	virtual MutexRef createMutex(void);
	virtual void lockMutex(MutexRef mutex);
	virtual void unlockMutex(MutexRef mutex);
	virtual void deleteMutex(MutexRef mutex);

	typedef void (*SoundProc)(void *param, byte *buf, int len);
	virtual bool setSoundCallback(SoundProc proc, void *param);

	virtual void quit();

	virtual Common::SaveFileManager *getSavefileManager();
	virtual Audio::Mixer *getMixer();
	virtual void getTimeAndDate(TimeDate &t) const;
	virtual Common::TimerManager *getTimerManager();
	FilesystemFactory *getFilesystemFactory();

};

static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
	{0, 0, 0}
};

OSystem_NULL::OSystem_NULL() {
	_savefile = 0;
	_mixer = 0;
	_timer = 0;

	#if defined(__amigaos4__)
		_fsFactory = new AmigaOSFilesystemFactory();
	#elif defined(UNIX)
		_fsFactory = new POSIXFilesystemFactory();
	#elif defined(WIN32)
		_fsFactory = new WindowsFilesystemFactory();
	#else
		#error Unknown and unsupported FS backend
	#endif
}

OSystem_NULL::~OSystem_NULL() {
	delete _savefile;
	delete _mixer;
	delete _timer;
	delete _fsFactory;
}

void OSystem_NULL::initBackend() {
	_savefile = new DefaultSaveFileManager();
	_mixer = new Audio::MixerImpl(this);
	_timer = new DefaultTimerManager();

	_mixer->setOutputRate(22050);
	_mixer->setReady(false);

	gettimeofday(&_startTime, NULL);

	// Note that both the mixer and the timer manager are useless
	// this way; they need to be hooked into the system somehow to
	// be functional. Of course, can't do that in a NULL backend :).

	OSystem::initBackend();
}

bool OSystem_NULL::hasFeature(Feature f) {
	return false;
}

void OSystem_NULL::setFeatureState(Feature f, bool enable) {
}

bool OSystem_NULL::getFeatureState(Feature f) {
	return false;
}

const OSystem::GraphicsMode* OSystem_NULL::getSupportedGraphicsModes() const {
	return s_supportedGraphicsModes;
}


int OSystem_NULL::getDefaultGraphicsMode() const {
	return -1;
}

bool OSystem_NULL::setGraphicsMode(const char *mode) {
	return true;
}

bool OSystem_NULL::setGraphicsMode(int mode) {
	return true;
}

int OSystem_NULL::getGraphicsMode() const {
	return -1;
}

void OSystem_NULL::initSize(uint width, uint height) {
}

int16 OSystem_NULL::getHeight() {
	return 200;
}

int16 OSystem_NULL::getWidth() {
	return 320;
}

void OSystem_NULL::setPalette(const byte *colors, uint start, uint num) {
}

void OSystem_NULL::grabPalette(byte *colors, uint start, uint num) {

}

void OSystem_NULL::copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h) {
}

void OSystem_NULL::updateScreen() {
}

Graphics::Surface *OSystem_NULL::lockScreen() {
	return 0;
}

void OSystem_NULL::unlockScreen() {
}

void OSystem_NULL::setShakePos(int shakeOffset) {
}

void OSystem_NULL::showOverlay() {
}

void OSystem_NULL::hideOverlay() {
}

void OSystem_NULL::clearOverlay() {
}

void OSystem_NULL::grabOverlay(OverlayColor *buf, int pitch) {
}

void OSystem_NULL::copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h) {
}

int16 OSystem_NULL::getOverlayHeight() {
	return getHeight();
}

int16 OSystem_NULL::getOverlayWidth() {
	return getWidth();
}


bool OSystem_NULL::showMouse(bool visible) {
	return true;
}

void OSystem_NULL::warpMouse(int x, int y) {
}

void OSystem_NULL::setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, byte keycolor, int cursorTargetScale) {
}

bool OSystem_NULL::pollEvent(Common::Event &event) {
	return false;
}

uint32 OSystem_NULL::getMillis() {
#ifdef UNIX
	timeval curTime;
	gettimeofday(&curTime, NULL);
	return (uint32)(((curTime.tv_sec - _startTime.tv_sec) * 1000) + \
			((curTime.tv_usec - _startTime.tv_usec) / 1000));
#else
	return 0;
#endif
}

void OSystem_NULL::delayMillis(uint msecs) {
#ifdef UNIX
	usleep(msecs * 1000);
#endif
}

OSystem::MutexRef OSystem_NULL::createMutex(void) {
	return NULL;
}

void OSystem_NULL::lockMutex(MutexRef mutex) {
}

void OSystem_NULL::unlockMutex(MutexRef mutex) {
}

void OSystem_NULL::deleteMutex(MutexRef mutex) {
}

bool OSystem_NULL::setSoundCallback(SoundProc proc, void *param) {
	return true;
}

void OSystem_NULL::quit() {
}

Common::SaveFileManager *OSystem_NULL::getSavefileManager() {
	assert(_savefile);
	return _savefile;
}

Audio::Mixer *OSystem_NULL::getMixer() {
	assert(_mixer);
	return _mixer;
}

Common::TimerManager *OSystem_NULL::getTimerManager() {
	assert(_timer);
	return _timer;
}

void OSystem_NULL::getTimeAndDate(TimeDate &t) const {
}

FilesystemFactory *OSystem_NULL::getFilesystemFactory() {
	return _fsFactory;
}

OSystem *OSystem_NULL_create() {
	return new OSystem_NULL();
}

int main(int argc, char *argv[]) {
	g_system = OSystem_NULL_create();
	assert(g_system);

	// Invoke the actual ScummVM main entry point:
	int res = scummvm_main(argc, argv);
	g_system->quit();       // TODO: Consider removing / replacing this!
	return res;
}

#else /* USE_NULL_DRIVER */

OSystem *OSystem_NULL_create() {
	return NULL;
}

#endif
