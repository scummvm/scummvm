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

#if defined(WIN32)
#include <windows.h>
// winnt.h defines ARRAYSIZE, but we want our own one... - this is needed before including util.h
#undef ARRAYSIZE
#endif

#include "backends/platform/sdl/sdl.h"
#include "common/archive.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/util.h"

#ifdef UNIX
  #include "backends/saves/posix/posix-saves.h"
#else
  #include "backends/saves/default/default-saves.h"
#endif
#include "backends/timer/default/default-timer.h"
#include "sound/mixer_intern.h"

#include "icons/scummvm.xpm"

#include <time.h>	// for getTimeAndDate()

//#define SAMPLES_PER_SEC 11025
#define SAMPLES_PER_SEC 22050
//#define SAMPLES_PER_SEC 44100


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


#if defined(UNIX)
#ifdef MACOSX
#define DEFAULT_CONFIG_FILE "Library/Preferences/ScummVM Preferences"
#else
#define DEFAULT_CONFIG_FILE ".scummvmrc"
#endif
#else
#define DEFAULT_CONFIG_FILE "scummvm.ini"
#endif

#if defined(MACOSX) || defined(IPHONE)
#include "CoreFoundation/CoreFoundation.h"
#endif


static Uint32 timer_handler(Uint32 interval, void *param) {
	((DefaultTimerManager *)param)->handler();
	return interval;
}

void OSystem_SDL::initBackend() {
	assert(!_inited);

	int joystick_num = ConfMan.getInt("joystick_num");
	uint32 sdlFlags = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER;

	if (ConfMan.hasKey("disable_sdl_parachute"))
		sdlFlags |= SDL_INIT_NOPARACHUTE;

#ifdef _WIN32_WCE
	if (ConfMan.hasKey("use_GDI") && ConfMan.getBool("use_GDI")) {
		SDL_VideoInit("windib", 0);
		sdlFlags ^= SDL_INIT_VIDEO;
	}
#endif

	if (joystick_num > -1)
		sdlFlags |= SDL_INIT_JOYSTICK;

	if (SDL_Init(sdlFlags) == -1) {
		error("Could not initialize SDL: %s", SDL_GetError());
	}

	_graphicsMutex = createMutex();

	SDL_ShowCursor(SDL_DISABLE);

	// Enable unicode support if possible
	SDL_EnableUNICODE(1);

	memset(&_oldVideoMode, 0, sizeof(_oldVideoMode));
	memset(&_videoMode, 0, sizeof(_videoMode));
	memset(&_transactionDetails, 0, sizeof(_transactionDetails));

	_cksumValid = false;
#if !defined(_WIN32_WCE) && !defined(__SYMBIAN32__) && !defined(DISABLE_SCALERS)
	_videoMode.mode = GFX_DOUBLESIZE;
	_videoMode.scaleFactor = 2;
	_videoMode.aspectRatioCorrection = ConfMan.getBool("aspect_ratio");
	_scalerProc = Normal2x;
#else // for small screen platforms
	_videoMode.mode = GFX_NORMAL;
	_videoMode.scaleFactor = 1;
	_videoMode.aspectRatioCorrection = false;
	_scalerProc = Normal1x;
#endif
	_scalerType = 0;
	_modeFlags = 0;

#if !defined(_WIN32_WCE) && !defined(__SYMBIAN32__)
	_videoMode.fullscreen = ConfMan.getBool("fullscreen");
#else
	_videoMode.fullscreen = true;
#endif

#if !defined(MACOSX) && !defined(__SYMBIAN32__)
	// Setup a custom program icon.
	// Don't set icon on OS X, as we use a nicer external icon there.
	// Don't for Symbian: it uses the EScummVM.aif file for the icon.
	setupIcon();
#endif

	// enable joystick
	if (joystick_num > -1 && SDL_NumJoysticks() > 0) {
		printf("Using joystick: %s\n", SDL_JoystickName(0));
		_joystick = SDL_JoystickOpen(joystick_num);
	}


	// Create the savefile manager, if none exists yet (we check for this to
	// allow subclasses to provide their own).
	if (_savefile == 0) {
#ifdef UNIX
	_savefile = new POSIXSaveFileManager();
#else
	_savefile = new DefaultSaveFileManager();
#endif
	}

	// Create and hook up the mixer, if none exists yet (we check for this to
	// allow subclasses to provide their own).
	if (_mixer == 0) {
		setupMixer();
	}

	// Create and hook up the timer manager, if none exists yet (we check for
	// this to allow subclasses to provide their own).
	if (_timer == 0) {
		// Note: We could implement a custom SDLTimerManager by using
		// SDL_AddTimer. That might yield better timer resolution, but it would
		// also change the semantics of a timer: Right now, ScummVM timers
		// *never* run in parallel, due to the way they are implemented. If we
		// switched to SDL_AddTimer, each timer might run in a separate thread.
		// However, not all our code is prepared for that, so we can't just
		// switch. Still, it's a potential future change to keep in mind.
		_timer = new DefaultTimerManager();
		_timerID = SDL_AddTimer(10, &timer_handler, _timer);
	}

	// Invoke parent implementation of this method
	OSystem::initBackend();

	_inited = true;
}

OSystem_SDL::OSystem_SDL()
	:
#ifdef USE_OSD
	_osdSurface(0), _osdAlpha(SDL_ALPHA_TRANSPARENT), _osdFadeStartTime(0),
#endif
	_hwscreen(0), _screen(0), _tmpscreen(0),
	_overlayVisible(false),
	_overlayscreen(0), _tmpscreen2(0),
	_samplesPerSec(0),
	_cdrom(0), _scalerProc(0), _modeChanged(false), _screenChangeCount(0), _dirtyChecksums(0),
	_mouseVisible(false), _mouseNeedsRedraw(false), _mouseData(0), _mouseSurface(0),
	_mouseOrigSurface(0), _cursorTargetScale(1), _cursorPaletteDisabled(true),
	_joystick(0),
	_currentShakePos(0), _newShakePos(0),
	_paletteDirtyStart(0), _paletteDirtyEnd(0),
#ifdef MIXER_DOUBLE_BUFFERING
	_soundMutex(0), _soundCond(0), _soundThread(0),
	_soundThreadIsRunning(false), _soundThreadShouldQuit(false),
#endif
	_fsFactory(0),
	_savefile(0),
	_mixer(0),
	_timer(0),
	_screenIsLocked(false),
	_graphicsMutex(0), _transactionMode(kTransactionNone) {

	// allocate palette storage
	_currentPalette = (SDL_Color *)calloc(sizeof(SDL_Color), 256);
	_cursorPalette = (SDL_Color *)calloc(sizeof(SDL_Color), 256);

	_mouseBackup.x = _mouseBackup.y = _mouseBackup.w = _mouseBackup.h = 0;

	// reset mouse state
	memset(&_km, 0, sizeof(_km));
	memset(&_mouseCurState, 0, sizeof(_mouseCurState));

	_inited = false;


	#if defined(__amigaos4__)
		_fsFactory = new AmigaOSFilesystemFactory();
	#elif defined(UNIX)
		_fsFactory = new POSIXFilesystemFactory();
	#elif defined(WIN32)
		_fsFactory = new WindowsFilesystemFactory();
	#elif defined(__SYMBIAN32__)
		// Do nothing since its handled by the Symbian SDL inheritance
	#else
		#error Unknown and unsupported FS backend
	#endif
}

OSystem_SDL::~OSystem_SDL() {
	SDL_RemoveTimer(_timerID);
	closeMixer();

	free(_dirtyChecksums);
	free(_currentPalette);
	free(_cursorPalette);
	free(_mouseData);

	delete _savefile;
	delete _timer;
}

uint32 OSystem_SDL::getMillis() {
	uint32 millis = SDL_GetTicks();
	getEventManager()->processMillis(millis);
	return millis;
}

void OSystem_SDL::delayMillis(uint msecs) {
	SDL_Delay(msecs);
}

void OSystem_SDL::getTimeAndDate(struct tm &t) const {
	time_t curTime = time(0);
	t = *localtime(&curTime);
}

Common::TimerManager *OSystem_SDL::getTimerManager() {
	assert(_timer);
	return _timer;
}

Common::SaveFileManager *OSystem_SDL::getSavefileManager() {
	assert(_savefile);
	return _savefile;
}

FilesystemFactory *OSystem_SDL::getFilesystemFactory() {
	assert(_fsFactory);
	return _fsFactory;
}

void OSystem_SDL::addSysArchivesToSearchSet(Common::SearchSet &s, int priority) {

#ifdef DATA_PATH
	// Add the global DATA_PATH to the directory search list
	// FIXME: We use depth = 4 for now, to match the old code. May want to change that
	Common::FSNode dataNode(DATA_PATH);
	if (dataNode.exists() && dataNode.isDirectory()) {
		s.add(DATA_PATH, new Common::FSDirectory(dataNode, 4), priority);
	}
#endif

#ifdef MACOSX
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

#endif

}


static Common::String getDefaultConfigFileName() {
	char configFile[MAXPATHLEN];
#if defined (WIN32) && !defined(_WIN32_WCE) && !defined(__SYMBIAN32__)
	OSVERSIONINFO win32OsVersion;
	ZeroMemory(&win32OsVersion, sizeof(OSVERSIONINFO));
	win32OsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&win32OsVersion);
	// Check for non-9X version of Windows.
	if (win32OsVersion.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS) {
		// Use the Application Data directory of the user profile.
		if (win32OsVersion.dwMajorVersion >= 5) {
			if (!GetEnvironmentVariable("APPDATA", configFile, sizeof(configFile)))
				error("Unable to access application data directory");
		} else {
			if (!GetEnvironmentVariable("USERPROFILE", configFile, sizeof(configFile)))
				error("Unable to access user profile directory");

			strcat(configFile, "\\Application Data");
			CreateDirectory(configFile, NULL);
		}

		strcat(configFile, "\\ScummVM");
		CreateDirectory(configFile, NULL);
		strcat(configFile, "\\" DEFAULT_CONFIG_FILE);

		FILE *tmp = NULL;
		if ((tmp = fopen(configFile, "r")) == NULL) {
			// Check windows directory
			char oldConfigFile[MAXPATHLEN];
			GetWindowsDirectory(oldConfigFile, MAXPATHLEN);
			strcat(oldConfigFile, "\\" DEFAULT_CONFIG_FILE);
			if ((tmp = fopen(oldConfigFile, "r"))) {
				strcpy(configFile, oldConfigFile);

				fclose(tmp);
			}
		} else {
			fclose(tmp);
		}
	} else {
		// Check windows directory
		GetWindowsDirectory(configFile, MAXPATHLEN);
		strcat(configFile, "\\" DEFAULT_CONFIG_FILE);
	}
#elif defined(UNIX)
	// On UNIX type systems, by default we store the config file inside
	// to the HOME directory of the user.
	//
	// GP2X is Linux based but Home dir can be read only so do not use
	// it and put the config in the executable dir.
	//
	// On the iPhone, the home dir of the user when you launch the app
	// from the Springboard, is /. Which we don't want.
	const char *home = getenv("HOME");
	if (home != NULL && strlen(home) < MAXPATHLEN)
		snprintf(configFile, MAXPATHLEN, "%s/%s", home, DEFAULT_CONFIG_FILE);
	else
		strcpy(configFile, DEFAULT_CONFIG_FILE);
#else
	strcpy(configFile, DEFAULT_CONFIG_FILE);
#endif

	return configFile;
}

Common::SeekableReadStream *OSystem_SDL::createConfigReadStream() {
	Common::FSNode file(getDefaultConfigFileName());
	return file.createReadStream();
}

Common::WriteStream *OSystem_SDL::createConfigWriteStream() {
	Common::FSNode file(getDefaultConfigFileName());
	return file.createWriteStream();
}

void OSystem_SDL::setWindowCaption(const char *caption) {
	Common::String cap;
	byte c;

	// The string caption is supposed to be in LATIN-1 encoding.
	// SDL expects UTF-8. So we perform the conversion here.
	while ((c = *(const byte *)caption++)) {
		if (c < 0x80)
			cap += c;
		else {
			cap += 0xC0 | (c >> 6);
			cap += 0x80 | (c & 0x3F);
		}
	}

	SDL_WM_SetCaption(cap.c_str(), cap.c_str());
}

bool OSystem_SDL::hasFeature(Feature f) {
	return
		(f == kFeatureFullscreenMode) ||
		(f == kFeatureAspectRatioCorrection) ||
		(f == kFeatureAutoComputeDirtyRects) ||
		(f == kFeatureCursorHasPalette) ||
		(f == kFeatureIconifyWindow);
}

void OSystem_SDL::setFeatureState(Feature f, bool enable) {
	switch (f) {
	case kFeatureFullscreenMode:
		setFullscreenMode(enable);
		break;
	case kFeatureAspectRatioCorrection:
		setAspectRatioCorrection(enable);
		break;
	case kFeatureAutoComputeDirtyRects:
		if (enable)
			_modeFlags |= DF_WANT_RECT_OPTIM;
		else
			_modeFlags &= ~DF_WANT_RECT_OPTIM;
		break;
	case kFeatureIconifyWindow:
		if (enable)
			SDL_WM_IconifyWindow();
		break;
	default:
		break;
	}
}

bool OSystem_SDL::getFeatureState(Feature f) {
	assert (_transactionMode == kTransactionNone);

	switch (f) {
	case kFeatureFullscreenMode:
		return _videoMode.fullscreen;
	case kFeatureAspectRatioCorrection:
		return _videoMode.aspectRatioCorrection;
	case kFeatureAutoComputeDirtyRects:
		return _modeFlags & DF_WANT_RECT_OPTIM;
	default:
		return false;
	}
}

void OSystem_SDL::quit() {
	if (_cdrom) {
		SDL_CDStop(_cdrom);
		SDL_CDClose(_cdrom);
	}
	unloadGFXMode();
	deleteMutex(_graphicsMutex);

	if (_joystick)
		SDL_JoystickClose(_joystick);
	SDL_ShowCursor(SDL_ENABLE);

	SDL_RemoveTimer(_timerID);
	closeMixer();

	free(_dirtyChecksums);
	free(_currentPalette);
	free(_cursorPalette);
	free(_mouseData);

	delete _timer;

	SDL_Quit();

	// Even Manager requires save manager for storing
	// recorded events
	delete getEventManager();
	delete _savefile;

	exit(0);
}

void OSystem_SDL::setupIcon() {
	int x, y, w, h, ncols, nbytes, i;
	unsigned int rgba[256];
	unsigned int *icon;

	sscanf(scummvm_icon[0], "%d %d %d %d", &w, &h, &ncols, &nbytes);
	if ((w > 512) || (h > 512) || (ncols > 255) || (nbytes > 1)) {
		warning("Could not load the built-in icon (%d %d %d %d)", w, h, ncols, nbytes);
		return;
	}
	icon = (unsigned int*)malloc(w*h*sizeof(unsigned int));
	if (!icon) {
		warning("Could not allocate temp storage for the built-in icon");
		return;
	}

	for (i = 0; i < ncols; i++) {
		unsigned char code;
		char color[32];
		unsigned int col;
		sscanf(scummvm_icon[1 + i], "%c c %s", &code, color);
		if (!strcmp(color, "None"))
			col = 0x00000000;
		else if (!strcmp(color, "black"))
			col = 0xFF000000;
		else if (color[0] == '#') {
			sscanf(color + 1, "%06x", &col);
			col |= 0xFF000000;
		} else {
			warning("Could not load the built-in icon (%d %s - %s) ", code, color, scummvm_icon[1 + i]);
			free(icon);
			return;
		}

		rgba[code] = col;
	}
	for (y = 0; y < h; y++) {
		const char *line = scummvm_icon[1 + ncols + y];
		for (x = 0; x < w; x++) {
			icon[x + w * y] = rgba[(int)line[x]];
		}
	}

	SDL_Surface *sdl_surf = SDL_CreateRGBSurfaceFrom(icon, w, h, 32, w * 4, 0xFF0000, 0x00FF00, 0x0000FF, 0xFF000000);
	if (!sdl_surf) {
		warning("SDL_CreateRGBSurfaceFrom(icon) failed");
	}
	SDL_WM_SetIcon(sdl_surf, NULL);
	SDL_FreeSurface(sdl_surf);
	free(icon);
}

OSystem::MutexRef OSystem_SDL::createMutex(void) {
	return (MutexRef) SDL_CreateMutex();
}

void OSystem_SDL::lockMutex(MutexRef mutex) {
	SDL_mutexP((SDL_mutex *) mutex);
}

void OSystem_SDL::unlockMutex(MutexRef mutex) {
	SDL_mutexV((SDL_mutex *) mutex);
}

void OSystem_SDL::deleteMutex(MutexRef mutex) {
	SDL_DestroyMutex((SDL_mutex *) mutex);
}

#pragma mark -
#pragma mark --- Audio ---
#pragma mark -

#ifdef MIXER_DOUBLE_BUFFERING

void OSystem_SDL::mixerProducerThread() {
	byte nextSoundBuffer;

	SDL_LockMutex(_soundMutex);
	while (true) {
		// Wait till we are allowed to produce data
		SDL_CondWait(_soundCond, _soundMutex);

		if (_soundThreadShouldQuit)
			break;

		// Generate samples and put them into the next buffer
		nextSoundBuffer = _activeSoundBuf ^ 1;
		_mixer->mixCallback(_soundBuffers[nextSoundBuffer], _soundBufSize);

		// Swap buffers
		_activeSoundBuf = nextSoundBuffer;
	}
	SDL_UnlockMutex(_soundMutex);
}

int SDLCALL OSystem_SDL::mixerProducerThreadEntry(void *arg) {
	OSystem_SDL *this_ = (OSystem_SDL *)arg;
	assert(this_);
	this_->mixerProducerThread();
	return 0;
}


void OSystem_SDL::initThreadedMixer(Audio::MixerImpl *mixer, uint bufSize) {
	_soundThreadIsRunning = false;
	_soundThreadShouldQuit = false;

	// Create mutex and condition variable
	_soundMutex = SDL_CreateMutex();
	_soundCond = SDL_CreateCond();

	// Create two sound buffers
	_activeSoundBuf = 0;
	_soundBufSize = bufSize;
	_soundBuffers[0] = (byte *)calloc(1, bufSize);
	_soundBuffers[1] = (byte *)calloc(1, bufSize);

	_soundThreadIsRunning = true;

	// Finally start the thread
	_soundThread = SDL_CreateThread(mixerProducerThreadEntry, this);
}

void OSystem_SDL::deinitThreadedMixer() {
	// Kill thread?? _soundThread

	if (_soundThreadIsRunning) {
		// Signal the producer thread to end, and wait for it to actually finish.
		_soundThreadShouldQuit = true;
		SDL_CondBroadcast(_soundCond);
		SDL_WaitThread(_soundThread, NULL);

		// Kill the mutex & cond variables.
		// Attention: AT this point, the mixer callback must not be running
		// anymore, else we will crash!
		SDL_DestroyMutex(_soundMutex);
		SDL_DestroyCond(_soundCond);

		_soundThreadIsRunning = false;

		free(_soundBuffers[0]);
		free(_soundBuffers[1]);
	}
}


void OSystem_SDL::mixCallback(void *arg, byte *samples, int len) {
	OSystem_SDL *this_ = (OSystem_SDL *)arg;
	assert(this_);
	assert(this_->_mixer);

	assert((int)this_->_soundBufSize == len);

	// Lock mutex, to ensure our data is not overwritten by the producer thread
	SDL_LockMutex(this_->_soundMutex);

	// Copy data from the current sound buffer
	memcpy(samples, this_->_soundBuffers[this_->_activeSoundBuf], len);

	// Unlock mutex and wake up the produced thread
	SDL_UnlockMutex(this_->_soundMutex);
	SDL_CondSignal(this_->_soundCond);
}

#else

void OSystem_SDL::mixCallback(void *sys, byte *samples, int len) {
	OSystem_SDL *this_ = (OSystem_SDL *)sys;
	assert(this_);
	assert(this_->_mixer);

	this_->_mixer->mixCallback(samples, len);
}

#endif

void OSystem_SDL::setupMixer() {
	SDL_AudioSpec desired;

	// Determine the desired output sampling frequency.
	_samplesPerSec = 0;
	if (ConfMan.hasKey("output_rate"))
		_samplesPerSec = ConfMan.getInt("output_rate");
	if (_samplesPerSec <= 0)
		_samplesPerSec = SAMPLES_PER_SEC;

	// Determine the sample buffer size. We want it to store enough data for
	// about 1/16th of a second. Note that it must be a power of two.
	// So e.g. at 22050 Hz, we request a sample buffer size of 2048.
	int samples = 8192;
	while (16 * samples >= _samplesPerSec) {
		samples >>= 1;
	}

	memset(&desired, 0, sizeof(desired));
	desired.freq = _samplesPerSec;
	desired.format = AUDIO_S16SYS;
	desired.channels = 2;
	desired.samples = (uint16)samples;
	desired.callback = mixCallback;
	desired.userdata = this;

	// Create the mixer instance
	assert(!_mixer);
	_mixer = new Audio::MixerImpl(this);
	assert(_mixer);

	if (SDL_OpenAudio(&desired, &_obtainedRate) != 0) {
		warning("Could not open audio device: %s", SDL_GetError());
		_samplesPerSec = 0;
		_mixer->setReady(false);
	} else {
		// Note: This should be the obtained output rate, but it seems that at
		// least on some platforms SDL will lie and claim it did get the rate
		// even if it didn't. Probably only happens for "weird" rates, though.
		_samplesPerSec = _obtainedRate.freq;
		debug(1, "Output sample rate: %d Hz", _samplesPerSec);

		// Tell the mixer that we are ready and start the sound processing
		_mixer->setOutputRate(_samplesPerSec);
		_mixer->setReady(true);

#ifdef MIXER_DOUBLE_BUFFERING
		initThreadedMixer(_mixer, _obtainedRate.samples * 4);
#endif

		// start the sound system
		SDL_PauseAudio(0);
	}
}

void OSystem_SDL::closeMixer() {
	if (_mixer)
		_mixer->setReady(false);

	SDL_CloseAudio();

	delete _mixer;
	_mixer = 0;

#ifdef MIXER_DOUBLE_BUFFERING
	deinitThreadedMixer();
#endif

}

Audio::Mixer *OSystem_SDL::getMixer() {
	assert(_mixer);
	return _mixer;
}

#pragma mark -
#pragma mark --- CD Audio ---
#pragma mark -

bool OSystem_SDL::openCD(int drive) {
	if (SDL_InitSubSystem(SDL_INIT_CDROM) == -1)
		_cdrom = NULL;
	else {
		_cdrom = SDL_CDOpen(drive);
		// Did it open? Check if _cdrom is NULL
		if (!_cdrom) {
			warning("Couldn't open drive: %s", SDL_GetError());
		} else {
			_cdNumLoops = 0;
			_cdStopTime = 0;
			_cdEndTime = 0;
		}
	}

	return (_cdrom != NULL);
}

void OSystem_SDL::stopCD() {	/* Stop CD Audio in 1/10th of a second */
	_cdStopTime = SDL_GetTicks() + 100;
	_cdNumLoops = 0;
}

void OSystem_SDL::playCD(int track, int num_loops, int start_frame, int duration) {
	if (!num_loops && !start_frame)
		return;

	if (!_cdrom)
		return;

	if (duration > 0)
		duration += 5;

	_cdTrack = track;
	_cdNumLoops = num_loops;
	_cdStartFrame = start_frame;

	SDL_CDStatus(_cdrom);
	if (start_frame == 0 && duration == 0)
		SDL_CDPlayTracks(_cdrom, track, 0, 1, 0);
	else
		SDL_CDPlayTracks(_cdrom, track, start_frame, 0, duration);
	_cdDuration = duration;
	_cdStopTime = 0;
	_cdEndTime = SDL_GetTicks() + _cdrom->track[track].length * 1000 / CD_FPS;
}

bool OSystem_SDL::pollCD() {
	if (!_cdrom)
		return false;

	return (_cdNumLoops != 0 && (SDL_GetTicks() < _cdEndTime || SDL_CDStatus(_cdrom) == CD_PLAYING));
}

void OSystem_SDL::updateCD() {
	if (!_cdrom)
		return;

	if (_cdStopTime != 0 && SDL_GetTicks() >= _cdStopTime) {
		SDL_CDStop(_cdrom);
		_cdNumLoops = 0;
		_cdStopTime = 0;
		return;
	}

	if (_cdNumLoops == 0 || SDL_GetTicks() < _cdEndTime)
		return;

	if (_cdNumLoops != 1 && SDL_CDStatus(_cdrom) != CD_STOPPED) {
		// Wait another second for it to be done
		_cdEndTime += 1000;
		return;
	}

	if (_cdNumLoops > 0)
		_cdNumLoops--;

	if (_cdNumLoops != 0) {
		if (_cdStartFrame == 0 && _cdDuration == 0)
			SDL_CDPlayTracks(_cdrom, _cdTrack, 0, 1, 0);
		else
			SDL_CDPlayTracks(_cdrom, _cdTrack, _cdStartFrame, 0, _cdDuration);
		_cdEndTime = SDL_GetTicks() + _cdrom->track[_cdTrack].length * 1000 / CD_FPS;
	}
}
