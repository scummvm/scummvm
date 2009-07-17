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

#include <unistd.h>
#include <pthread.h>

#include <sys/time.h>

#include "common/scummsys.h"
#include "common/util.h"
#include "common/rect.h"
#include "common/file.h"
#include "common/fs.h"

#include "base/main.h"

#include "backends/saves/default/default-saves.h"
#include "backends/timer/default/default-timer.h"
#include "sound/mixer.h"
#include "sound/mixer_intern.h"

#include "osys_main.h"


const OSystem::GraphicsMode OSystem_IPHONE::s_supportedGraphicsModes[] = {
	{0, 0, 0}
};

AQCallbackStruct OSystem_IPHONE::s_AudioQueue;
SoundProc OSystem_IPHONE::s_soundCallback = NULL;
void *OSystem_IPHONE::s_soundParam = NULL;

OSystem_IPHONE::OSystem_IPHONE() :
	_savefile(NULL), _mixer(NULL), _timer(NULL), _offscreen(NULL),
	_overlayVisible(false), _overlayBuffer(NULL), _fullscreen(NULL),
	_mouseHeight(0), _mouseWidth(0), _mouseBuf(NULL), _lastMouseTap(0),
	_secondaryTapped(false), _lastSecondaryTap(0), _screenOrientation(kScreenOrientationFlippedLandscape),
	_needEventRestPeriod(false), _mouseClickAndDragEnabled(false), _touchpadModeEnabled(true),
	_gestureStartX(-1), _gestureStartY(-1), _fullScreenIsDirty(false), _fullScreenOverlayIsDirty(false),
	_mouseDirty(false), _timeSuspended(0), _lastDragPosX(-1), _lastDragPosY(-1), _screenChangeCount(0)

{
	_queuedInputEvent.type = (Common::EventType)0;
	_lastDrawnMouseRect = Common::Rect(0, 0, 0, 0);

	_fsFactory = new POSIXFilesystemFactory();
}

OSystem_IPHONE::~OSystem_IPHONE() {
	AudioQueueDispose(s_AudioQueue.queue, true);

	delete _fsFactory;
	delete _savefile;
	delete _mixer;
	delete _timer;
	delete _offscreen;
	delete _fullscreen;
}

int OSystem_IPHONE::timerHandler(int t) {
	DefaultTimerManager *tm = (DefaultTimerManager *)g_system->getTimerManager();
	tm->handler();
	return t;
}

void OSystem_IPHONE::initBackend() {
#ifdef IPHONE_OFFICIAL
	_savefile = new DefaultSaveFileManager(iPhone_getDocumentsDir());
#else
	_savefile = new DefaultSaveFileManager(SCUMMVM_SAVE_PATH);
#endif

	_timer = new DefaultTimerManager();

	gettimeofday(&_startTime, NULL);

	setupMixer();

	setTimerCallback(&OSystem_IPHONE::timerHandler, 10);

	OSystem::initBackend();
}

bool OSystem_IPHONE::hasFeature(Feature f) {
	return false;
}

void OSystem_IPHONE::setFeatureState(Feature f, bool enable) {
}

bool OSystem_IPHONE::getFeatureState(Feature f) {
	return false;
}

void OSystem_IPHONE::suspendLoop() {
	bool done = false;
	int eventType;
	float xUnit, yUnit;
	uint32 startTime = getMillis();

	stopSoundsystem();

	while (!done) {
		if (iPhone_fetchEvent(&eventType, &xUnit, &yUnit))
			if ((InputEvent)eventType == kInputApplicationResumed)
				done = true;
		usleep(100000);
	}

	startSoundsystem();

	_timeSuspended += getMillis() - startTime;
}

uint32 OSystem_IPHONE::getMillis() {
	//printf("getMillis()\n");

	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	return (uint32)(((currentTime.tv_sec - _startTime.tv_sec) * 1000) +
	                ((currentTime.tv_usec - _startTime.tv_usec) / 1000)) - _timeSuspended;
}

void OSystem_IPHONE::delayMillis(uint msecs) {
	//printf("delayMillis(%d)\n", msecs);
	usleep(msecs * 1000);
}

OSystem::MutexRef OSystem_IPHONE::createMutex(void) {
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

	pthread_mutex_t *mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
	if (pthread_mutex_init(mutex, &attr) != 0) {
		printf("pthread_mutex_init() failed!\n");
		free(mutex);
		return NULL;
	}

	return (MutexRef)mutex;
}

void OSystem_IPHONE::lockMutex(MutexRef mutex) {
	if (pthread_mutex_lock((pthread_mutex_t *) mutex) != 0) {
		printf("pthread_mutex_lock() failed!\n");
	}
}

void OSystem_IPHONE::unlockMutex(MutexRef mutex) {
	if (pthread_mutex_unlock((pthread_mutex_t *) mutex) != 0) {
		printf("pthread_mutex_unlock() failed!\n");
	}
}

void OSystem_IPHONE::deleteMutex(MutexRef mutex) {
	if (pthread_mutex_destroy((pthread_mutex_t *) mutex) != 0) {
		printf("pthread_mutex_destroy() failed!\n");
	} else {
		free(mutex);
	}
}


void OSystem_IPHONE::setTimerCallback(TimerProc callback, int interval) {
	//printf("setTimerCallback()\n");

	if (callback != NULL) {
		_timerCallbackTimer = interval;
		_timerCallbackNext = getMillis() + interval;
		_timerCallback = callback;
	} else
		_timerCallback = NULL;
}

void OSystem_IPHONE::quit() {
}

void OSystem_IPHONE::getTimeAndDate(struct tm &t) const {
	time_t curTime = time(0);
	t = *localtime(&curTime);
}

Common::SaveFileManager *OSystem_IPHONE::getSavefileManager() {
	assert(_savefile);
	return _savefile;
}

Audio::Mixer *OSystem_IPHONE::getMixer() {
	assert(_mixer);
	return _mixer;
}

Common::TimerManager *OSystem_IPHONE::getTimerManager() {
	assert(_timer);
	return _timer;
}

OSystem *OSystem_IPHONE_create() {
	return new OSystem_IPHONE();
}

Common::SeekableReadStream *OSystem_IPHONE::createConfigReadStream() {
#ifdef IPHONE_OFFICIAL
	char buf[256];
	strncpy(buf, iPhone_getDocumentsDir(), 256);
	strncat(buf, "/Preferences", 256 - strlen(buf) );
	Common::FSNode file(buf);
#else
	Common::FSNode file(SCUMMVM_PREFS_PATH);
#endif
	return file.createReadStream();
}

Common::WriteStream *OSystem_IPHONE::createConfigWriteStream() {
#ifdef IPHONE_OFFICIAL
	char buf[256];
	strncpy(buf, iPhone_getDocumentsDir(), 256);
	strncat(buf, "/Preferences", 256 - strlen(buf) );
	Common::FSNode file(buf);
#else
	Common::FSNode file(SCUMMVM_PREFS_PATH);
#endif
	return file.createWriteStream();
}

void OSystem_IPHONE::addSysArchivesToSearchSet(Common::SearchSet &s, int priority) {
	// Get URL of the Resource directory of the .app bundle
	CFURLRef fileUrl = CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle());
	if (fileUrl) {
		// Try to convert the URL to an absolute path
		UInt8 buf[MAXPATHLEN];
		if (CFURLGetFileSystemRepresentation(fileUrl, true, buf, sizeof(buf))) {
			// Success: Add it to the search path
			Common::String bundlePath((const char *)buf);
			s.add("__OSX_BUNDLE__", new Common::FSDirectory(bundlePath), priority);
		}
		CFRelease(fileUrl);
	}
}

void iphone_main(int argc, char *argv[]) {

	//OSystem_IPHONE::migrateApp();

	FILE *newfp = fopen("/var/mobile/.scummvm.log", "a");
	if (newfp != NULL) {
		fclose(stdout);
		fclose(stderr);
		*stdout = *newfp;
		*stderr = *newfp;
		setbuf(stdout, NULL);
		setbuf(stderr, NULL);

		//extern int gDebugLevel;
		//gDebugLevel = 10;
	}

#ifdef IPHONE_OFFICIAL
	chdir( iPhone_getDocumentsDir() );
#else
	system("mkdir " SCUMMVM_ROOT_PATH);
	system("mkdir " SCUMMVM_SAVE_PATH);

	chdir("/var/mobile/");
#endif

	g_system = OSystem_IPHONE_create();
	assert(g_system);

	// Invoke the actual ScummVM main entry point:
	scummvm_main(argc, argv);
	g_system->quit();       // TODO: Consider removing / replacing this!
}
