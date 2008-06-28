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

/*
 * GP2X: Main backend.
 *
 */

#include "backends/platform/gp2x/gp2x-common.h"
#include "backends/platform/gp2x/gp2x-hw.h"
#include "backends/platform/gp2x/gp2x-mem.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/util.h"
#include "base/main.h"

#include "backends/saves/default/default-saves.h"
#include "backends/timer/default/default-timer.h"
#include "backends/plugins/posix/posix-provider.h"
#include "backends/fs/posix/posix-fs-factory.h" // for getFilesystemFactory()
#include "sound/mixer_intern.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h> // for getTimeAndDate()

// Disable for normal serial logging.
#define DUMP_STDOUT

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#define SAMPLES_PER_SEC 11025
//#define SAMPLES_PER_SEC 22050
//#define SAMPLES_PER_SEC 44100



static Uint32 timer_handler(Uint32 interval, void *param) {
	((DefaultTimerManager *)param)->handler();
	return interval;
}

int main(int argc, char *argv[]) {
	//extern OSystem *OSystem_GP2X_create();
	//g_system = OSystem_GP2X_create();
	g_system = new OSystem_GP2X();
	assert(g_system);

	// Check if Plugins are enabled (Using the hacked up GP2X provider)
	#ifdef DYNAMIC_MODULES
		PluginManager::instance().addPluginProvider(new GP2XPluginProvider());
	#endif

	// Invoke the actual ScummVM main entry point:
	int res = scummvm_main(argc, argv);
	g_system->quit();
	return res;
}

OSystem *OSystem_GP2X_create() {
	return new OSystem_GP2X();
}

void OSystem_GP2X::initBackend() {
	assert(!_inited);

	ConfMan.setInt("joystick_num", 0);
	int joystick_num = ConfMan.getInt("joystick_num");
	uint32 sdlFlags = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_EVENTTHREAD;

	if (ConfMan.hasKey("disable_sdl_parachute"))
		sdlFlags |= SDL_INIT_NOPARACHUTE;

	if (joystick_num > -1)
		sdlFlags |= SDL_INIT_JOYSTICK;

	if (SDL_Init(sdlFlags) == -1) {
		error("Could not initialize SDL: %s", SDL_GetError());
	}

	SDL_ShowCursor(SDL_DISABLE);

	// Setup default save path to be workingdir/saves
	#ifndef PATH_MAX
		#define PATH_MAX 255
	#endif

	char savePath[PATH_MAX+1];
	char workDirName[PATH_MAX+1]; /* To be passed to getcwd system call. */
	if (getcwd(workDirName, PATH_MAX) == NULL) {
		error("Could not obtain current working directory.");
	} else {
		printf("Current working directory: %s\n", workDirName);
	}

	strcpy(savePath, workDirName);
	strcat(savePath, "/saves");
	printf("Current save directory: %s\n", savePath);
	struct stat sb;
	if (stat(savePath, &sb) == -1)
		if (errno == ENOENT) // Create the dir if it does not exist
			if (mkdir(savePath, 0755) != 0)
				warning("mkdir for '%s' failed!", savePath);

	ConfMan.registerDefault("savepath", savePath);

	// Setup default extra data path for engine data files to be workingdir/engine-data

	char enginedataPath[PATH_MAX+1];

	strcpy(enginedataPath, workDirName);
	strcat(enginedataPath, "/engine-data");
	printf("Current engine-data directory: %s\n", enginedataPath);
	//struct stat sb;
	if (stat(enginedataPath, &sb) == -1)
		if (errno == ENOENT) // Create the dir if it does not exist
			if (mkdir(enginedataPath, 0755) != 0)
				warning("mkdir for '%s' failed!", enginedataPath);

	Common::File::addDefaultDirectory(enginedataPath);

	// Note: Review and clean this, it's OTT at the moment.

	#if defined(DUMP_STDOUT)
		// The GP2X has a serial console but most users do not use this so we
		// output all our STDOUT and STDERR to files for debug purposes.
		char STDOUT_FILE[PATH_MAX+1];
		char STDERR_FILE[PATH_MAX+1];

		strcpy(STDOUT_FILE, workDirName);
		strcpy(STDERR_FILE, workDirName);
		strcat(STDOUT_FILE, "/scummvm.stdout.txt");
		strcat(STDERR_FILE, "/scummvm.stderr.txt");

		/* Flush the output in case anything is queued */
		fclose(stdout);
		fclose(stderr);

		/* Redirect standard input and standard output */
		FILE *newfp = freopen(STDOUT_FILE, "w", stdout);
		if (newfp == NULL) {	/* This happens on NT */
		#if !defined(stdout)
			stdout = fopen(STDOUT_FILE, "w");
		#else
			newfp = fopen(STDOUT_FILE, "w");
			if (newfp) {
				*stdout = *newfp;
			}
		#endif
		}
		newfp = freopen(STDERR_FILE, "w", stderr);
		if (newfp == NULL) {	/* This happens on NT */
		#if !defined(stderr)
			stderr = fopen(STDERR_FILE, "w");
		#else
			newfp = fopen(STDERR_FILE, "w");
			if (newfp) {
				*stderr = *newfp;
			}
		#endif
		}
		setbuf(stderr, NULL);			/* No buffering */
	#endif // DUMP_STDOUT

	// Setup other defaults.

	ConfMan.registerDefault("aspect_ratio", true);
	ConfMan.registerDefault("music_volume", 220); // Up default volume as we use a seperate volume system anyway.
	ConfMan.registerDefault("sfx_volume", 220);
	ConfMan.registerDefault("speech_volume", 220);
	ConfMan.registerDefault("autosave_period", 3 * 60);	// Trigger autosave every 3 minutes - On low batts 4 mins is about your warning time.

	_graphicsMutex = createMutex();

	_cksumValid = false;
	_mode = GFX_NORMAL;
	_scaleFactor = 0;
	_scalerProc = Normal1x;
	_fullscreen = true;
	_adjustAspectRatio = ConfMan.getBool("aspect_ratio");
	_scalerType = 0;
	_modeFlags = 0;
	_adjustZoomOnMouse = false;
	ConfMan.setBool("FM_low_quality", true);

	// enable joystick
	if (joystick_num > -1 && SDL_NumJoysticks() > 0) {
		_joystick = SDL_JoystickOpen(joystick_num);
	}

	// Create the savefile manager, if none exists yet (we check for this to
	// allow subclasses to provide their own).
	if (_savefile == 0) {
		_savefile = new DefaultSaveFileManager();
	}

	// Create and hook up the mixer, if none exists yet (we check for this to
	// allow subclasses to provide their own).
	if (_mixer == 0) {
		setupMixer();
	}

	// Create and hook up the timer manager, if none exists yet (we check for
	// this to allow subclasses to provide their own).
	if (_timer == 0) {
		// TODO: We could implement a custom SDLTimerManager by using
		// SDL_AddTimer. That might yield better timer resolution, but it would
		// also change the semantics of a timer: Right now, ScummVM timers
		// *never* run in parallel, due to the way they are implemented. If we
		// switched to SDL_AddTimer, each timer might run in a separate thread.
		// Unfortunately, not all our code is prepared for that, so we can't just
		// switch. But it's a long term goal to do just that!
		_timer = new DefaultTimerManager();
		_timerID = SDL_AddTimer(10, &timer_handler, _timer);

	// Initialise any GP2X specific stuff we may want (Volume, Batt Status etc.)
	GP2X_device_init();

	// Set Default hardware mixer volume to a plesent level.
	// This is done to 'reset' volume level if set by other apps.
	GP2X_mixer_set_volume(70, 70);

	}

	//if (SDL_GP2X_MouseType() == 0) {
	//	// No mouse, F100 default state.
	//	_gp2xInputType = 0;
	//	displayMessageOnOSD("F100 GP2X Found");
	//}

	//if (SDL_GP2X_MouseType() == 1) {
	//	// USB mouse found.
	//	_gp2xInputType = 1;
	//	displayMessageOnOSD("USB Mouse Found");
	//}

	//if (SDL_GP2X_MouseType() == 2) {
	//	// F200 touch screen found. - F200 default state.
	//	_gp2xInputType = 2;
	//	displayMessageOnOSD("Touch Screen Found");
	//}

	OSystem::initBackend();

	_inited = true;
}

OSystem_GP2X::OSystem_GP2X()
	:
	_osdSurface(0), _osdAlpha(SDL_ALPHA_TRANSPARENT), _osdFadeStartTime(0),
	_hwscreen(0), _screen(0), _screenWidth(0), _screenHeight(0),
	_tmpscreen(0), _overlayWidth(0), _overlayHeight(0),
	_overlayVisible(false),
	_overlayscreen(0), _tmpscreen2(0),
	_samplesPerSec(0),
	_cdrom(0), _scalerProc(0), _modeChanged(false), _screenChangeCount(0), _dirtyChecksums(0),
	_mouseVisible(false), _mouseDrawn(false), _mouseData(0), _mouseSurface(0),
	_mouseOrigSurface(0), _cursorTargetScale(1), _cursorPaletteDisabled(true),
	_joystick(0),
	_currentShakePos(0), _newShakePos(0),
	_paletteDirtyStart(0), _paletteDirtyEnd(0),
	_savefile(0),
	_mixer(0),
	_timer(0),
	_graphicsMutex(0), _transactionMode(kTransactionNone) {

	// allocate palette storage
	_currentPalette = (SDL_Color *)calloc(sizeof(SDL_Color), 256);
	_cursorPalette = (SDL_Color *)calloc(sizeof(SDL_Color), 256);

	_mouseBackup.x = _mouseBackup.y = _mouseBackup.w = _mouseBackup.h = 0;

	// reset mouse state
	memset(&_km, 0, sizeof(_km));
	memset(&_mouseCurState, 0, sizeof(_mouseCurState));

	_inited = false;
}

OSystem_GP2X::~OSystem_GP2X() {
	SDL_RemoveTimer(_timerID);
	SDL_CloseAudio();

	free(_dirtyChecksums);
	free(_currentPalette);
	free(_cursorPalette);
	free(_mouseData);

	delete _savefile;
	delete _mixer;
	delete _timer;
}

uint32 OSystem_GP2X::getMillis() {
	return SDL_GetTicks();
}

void OSystem_GP2X::delayMillis(uint msecs) {
	SDL_Delay(msecs);
}

void OSystem_GP2X::getTimeAndDate(struct tm &t) const {
	time_t curTime = time(0);
	t = *localtime(&curTime);
}

Common::TimerManager *OSystem_GP2X::getTimerManager() {
	assert(_timer);
	return _timer;
}

Common::SaveFileManager *OSystem_GP2X::getSavefileManager() {
	assert(_savefile);
	return _savefile;
}

FilesystemFactory *OSystem_GP2X::getFilesystemFactory() {
	return &POSIXFilesystemFactory::instance();
}

//void OSystem_GP2X::setTimerCallback(TimerProc callback, int timer) {
//	SDL_SetTimer(timer, (SDL_TimerCallback) callback);
//}

bool OSystem_GP2X::hasFeature(Feature f) {
	return
		(f == kFeatureFullscreenMode) ||
		(f == kFeatureAspectRatioCorrection) ||
		(f == kFeatureAutoComputeDirtyRects) ||
		(f == kFeatureCursorHasPalette);
}

void OSystem_GP2X::setFeatureState(Feature f, bool enable) {
	switch (f) {
	case kFeatureFullscreenMode:
		return;
	case kFeatureAspectRatioCorrection:
		setAspectRatioCorrection(enable);
		break;
	case kFeatureAutoComputeDirtyRects:
		if (enable)
			_modeFlags |= DF_WANT_RECT_OPTIM;
		else
			_modeFlags &= ~DF_WANT_RECT_OPTIM;
		break;
	case kFeatureDisableKeyFiltering:
		// TODO: Extend as more support for this is added to engines.
		return;
	default:
		break;
	}
}

bool OSystem_GP2X::getFeatureState(Feature f) {
	assert (_transactionMode == kTransactionNone);

	switch (f) {
	case kFeatureFullscreenMode:
		return false;
	//case kFeatureFullscreenMode:
	//	return _fullscreen;
	case kFeatureAspectRatioCorrection:
		return _adjustAspectRatio;
	case kFeatureAutoComputeDirtyRects:
		return _modeFlags & DF_WANT_RECT_OPTIM;
	default:
		return false;
	}
}

void OSystem_GP2X::quit() {
	unloadGFXMode();
	deleteMutex(_graphicsMutex);

	if (_joystick)
		SDL_JoystickClose(_joystick);
	//CloseRam();
	GP2X_device_deinit();

	SDL_RemoveTimer(_timerID);
	SDL_CloseAudio();

	free(_dirtyChecksums);
	free(_currentPalette);
	free(_cursorPalette);
	free(_mouseData);

	delete _savefile;
	delete _mixer;
	delete _timer;

	SDL_Quit();

	delete getEventManager();

	exit(0);
}

OSystem::MutexRef OSystem_GP2X::createMutex(void) {
	return (MutexRef) SDL_CreateMutex();
}

void OSystem_GP2X::lockMutex(MutexRef mutex) {
	SDL_mutexP((SDL_mutex *) mutex);
}

void OSystem_GP2X::unlockMutex(MutexRef mutex) {
	SDL_mutexV((SDL_mutex *) mutex);
}

void OSystem_GP2X::deleteMutex(MutexRef mutex) {
	SDL_DestroyMutex((SDL_mutex *) mutex);
}

#pragma mark -
#pragma mark --- Audio ---
#pragma mark -

void OSystem_GP2X::setupMixer() {
	SDL_AudioSpec desired;
	SDL_AudioSpec obtained;

	memset(&desired, 0, sizeof(desired));

	_samplesPerSec = 0;

	if (ConfMan.hasKey("output_rate"))
		_samplesPerSec = ConfMan.getInt("output_rate");

	if (_samplesPerSec <= 0)
		_samplesPerSec = SAMPLES_PER_SEC;

	//Quick EVIL Hack - DJWillis
	_samplesPerSec = 11025;

	desired.freq = _samplesPerSec;
	desired.format = AUDIO_S16SYS;
	desired.channels = 2;
	//desired.samples = (uint16)samples;
	desired.samples = 128; // Samples hack
	desired.callback = proc;
	desired.userdata = param;
	if (SDL_OpenAudio(&desired, &obtained) != 0) {
		warning("Could not open audio device: %s", SDL_GetError());
		return false;
	}
	_samplesPerSec = obtained.freq;
	SDL_PauseAudio(0);
	return true;
}

int OSystem_GP2X::getOutputSampleRate() const {
	return _samplesPerSec;
}

Audio::Mixer *OSystem_GP2X::getMixer() {
	assert(_mixer);
	return _mixer;
}

#pragma mark -
#pragma mark --- CD Audio ---
#pragma mark -

bool OSystem_GP2X::openCD(int drive) {
	return (_cdrom = NULL);
}

void OSystem_GP2X::stopCD() {
}

void OSystem_GP2X::playCD(int track, int num_loops, int start_frame, int duration) {
	return;
}

bool OSystem_GP2X::pollCD() {
		return false;
}

void OSystem_GP2X::updateCD() {
	return;
}
