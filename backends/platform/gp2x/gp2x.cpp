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
#include "common/archive.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/EventRecorder.h"
#include "common/events.h"
#include "common/util.h"

#include "common/file.h"
#include "base/main.h"

#include "backends/saves/default/default-saves.h"

#include "backends/timer/default/default-timer.h"
#include "backends/plugins/posix/posix-provider.h"
#include "sound/mixer_intern.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>	// for getTimeAndDate()

// Disable for normal serial logging.
#define DUMP_STDOUT

#define SAMPLES_PER_SEC 11025
//#define SAMPLES_PER_SEC 22050
//#define SAMPLES_PER_SEC 44100

#define DEFAULT_CONFIG_FILE ".scummvmrc"

#include "backends/fs/posix/posix-fs-factory.h"

static Uint32 timer_handler(Uint32 interval, void *param) {
	((DefaultTimerManager *)param)->handler();
	return interval;
}

int main(int argc, char *argv[]) {
	g_system = new OSystem_GP2X();
	assert(g_system);

#ifdef DYNAMIC_MODULES
	PluginManager::instance().addPluginProvider(new POSIXPluginProvider());
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

	// Setup default save path to be workingdir/saves
	#ifndef PATH_MAX
		#define PATH_MAX 255
	#endif

	char savePath[PATH_MAX+1];
	char workDirName[PATH_MAX+1];

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
				warning("mkdir for '%s' failed", savePath);

	ConfMan.registerDefault("savepath", savePath);

	_savefile = new DefaultSaveFileManager(savePath);

	#ifdef DUMP_STDOUT
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
		if (newfp == NULL) {
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
		if (newfp == NULL) {
		#if !defined(stderr)
			stderr = fopen(STDERR_FILE, "w");
		#else
			newfp = fopen(STDERR_FILE, "w");
			if (newfp) {
				*stderr = *newfp;
			}
		#endif
		}

		setbuf(stderr, NULL);
		printf("%s\n", "Debug: STDOUT and STDERR redirected to text files.");
	#endif /* DUMP_STDOUT */

	_graphicsMutex = createMutex();

	SDL_ShowCursor(SDL_DISABLE);

	// Setup other defaults.

	ConfMan.registerDefault("aspect_ratio", true);

	/* Up default volume values as we use a seperate system level volume anyway. */
	ConfMan.registerDefault("music_volume", 220);
	ConfMan.registerDefault("sfx_volume", 220);
	ConfMan.registerDefault("speech_volume", 220);
	ConfMan.registerDefault("autosave_period", 3 * 60);	// Trigger autosave every 3 minutes - On low batts 4 mins is about your warning time.

	memset(&_oldVideoMode, 0, sizeof(_oldVideoMode));
	memset(&_videoMode, 0, sizeof(_videoMode));
	memset(&_transactionDetails, 0, sizeof(_transactionDetails));

	_cksumValid = false;
	_videoMode.mode = GFX_NORMAL;
	_videoMode.scaleFactor = 1;
	_scalerProc = Normal1x;
	_videoMode.aspectRatio  = ConfMan.getBool("aspect_ratio");
	_scalerType = 0;
	_modeFlags = 0;
	_adjustZoomOnMouse = false;
	ConfMan.setBool("FM_low_quality", true);

	// enable joystick
	if (joystick_num > -1 && SDL_NumJoysticks() > 0) {
		_joystick = SDL_JoystickOpen(joystick_num);
	}

	// Create and hook up the mixer, if none exists yet (we check for this to
	// allow subclasses to provide their own).
	if (_mixer == 0) {
		setupMixer();
	}

	// Setup the keymapper with backend's set of keys
	// NOTE: must be done before creating TimerManager
	// to avoid race conditions in creating EventManager
	setupKeymapper();

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

	/* Initialise any GP2X specific stuff we may want (Batt Status, scaler etc.) */
	GP2X_HW::deviceInit();

	/* Set Default hardware mixer volume to a preset level (VOLUME_INITIAL). This is done to 'reset' volume level if set by other apps. */
	GP2X_HW::mixerMoveVolume(0);

	// Set Default hardware mixer volume to a plesent level.
	// This is done to 'reset' volume level if set by other apps.

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
	_hwscreen(0), _screen(0), _tmpscreen(0),
	_overlayVisible(false),
	_overlayscreen(0), _tmpscreen2(0),
	_samplesPerSec(0),
	_cdrom(0), _scalerProc(0), _modeChanged(false), _screenChangeCount(0), _dirtyChecksums(0),
	_mouseVisible(false), _mouseDrawn(false), _mouseData(0), _mouseSurface(0),
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
	_fsFactory = new POSIXFilesystemFactory();
}

OSystem_GP2X::~OSystem_GP2X() {
	SDL_RemoveTimer(_timerID);
	closeMixer();

	free(_dirtyChecksums);
	free(_currentPalette);
	free(_cursorPalette);
	free(_mouseData);

	delete _savefile;
	delete _timer;
}

uint32 OSystem_GP2X::getMillis() {
	uint32 millis = SDL_GetTicks();
	g_eventRec.processMillis(millis);
	return millis;
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
	assert(_fsFactory);
	return _fsFactory;
}

void OSystem_GP2X::addSysArchivesToSearchSet(Common::SearchSet &s, int priority) {
	/* Setup default extra data paths for engine data files and plugins */
	char workDirName[PATH_MAX+1];

	if (getcwd(workDirName, PATH_MAX) == NULL) {
		error("Error: Could not obtain current working directory.");
	}

	Common::FSNode workdirNode(workDirName);
	if (workdirNode.exists() && workdirNode.isDirectory()) {
		s.add("__GP2X_WORKDIR__", new Common::FSDirectory(workDirName), priority);
	}

	char enginedataPath[PATH_MAX+1];

	strcpy(enginedataPath, workDirName);
	strcat(enginedataPath, "/engine-data");

	Common::FSNode engineNode(enginedataPath);
	if (engineNode.exists() && engineNode.isDirectory()) {
		s.add("__GP2X_ENGDATA__", new Common::FSDirectory(enginedataPath), priority);
	}

	char pluginsPath[PATH_MAX+1];

	strcpy(pluginsPath, workDirName);
	strcat(pluginsPath, "/plugins");

	Common::FSNode pluginsNode(pluginsPath);
	if (pluginsNode.exists() && pluginsNode.isDirectory()) {
		s.add("__GP2X_PLUGINS__", new Common::FSDirectory(pluginsPath), priority);
	}
}

static Common::String getDefaultConfigFileName() {
	char configFile[MAXPATHLEN];
	strcpy(configFile, DEFAULT_CONFIG_FILE);
	return configFile;
}

Common::SeekableReadStream *OSystem_GP2X::createConfigReadStream() {
	Common::FSNode file(getDefaultConfigFileName());
	return file.createReadStream();
}

Common::WriteStream *OSystem_GP2X::createConfigWriteStream() {
	Common::FSNode file(getDefaultConfigFileName());
	return file.createWriteStream();
}

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
	case kFeatureAspectRatioCorrection:
		return _videoMode.aspectRatio;
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
	GP2X_HW::deviceDeinit();

	SDL_RemoveTimer(_timerID);
	closeMixer();

	free(_dirtyChecksums);
	free(_currentPalette);
	free(_cursorPalette);
	free(_mouseData);

	delete _savefile;
	delete _timer;
	SDL_ShowCursor(SDL_ENABLE);
	SDL_Quit();

	delete getEventManager();

	#ifdef DUMP_STDOUT
		printf("%s\n", "Debug: STDOUT and STDERR text files closed.");
		fclose(stdout);
		fclose(stderr);
	#endif /* DUMP_STDOUT */

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

#ifdef MIXER_DOUBLE_BUFFERING

void OSystem_GP2X::mixerProducerThread() {
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

int SDLCALL OSystem_GP2X::mixerProducerThreadEntry(void *arg) {
	OSystem_GP2X *this_ = (OSystem_GP2X *)arg;
	assert(this_);
	this_->mixerProducerThread();
	return 0;
}


void OSystem_GP2X::initThreadedMixer(Audio::MixerImpl *mixer, uint bufSize) {
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

void OSystem_GP2X::deinitThreadedMixer() {
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


void OSystem_GP2X::mixCallback(void *arg, byte *samples, int len) {
	OSystem_GP2X *this_ = (OSystem_GP2X *)arg;
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

void OSystem_GP2X::mixCallback(void *sys, byte *samples, int len) {
	OSystem_GP2X *this_ = (OSystem_GP2X *)sys;
	assert(this_);
	assert(this_->_mixer);

	this_->_mixer->mixCallback(samples, len);
}

#endif

void OSystem_GP2X::setupMixer() {
	SDL_AudioSpec desired;
	SDL_AudioSpec obtained;

	// Determine the desired output sampling frequency.
	_samplesPerSec = 0;
	if (ConfMan.hasKey("output_rate"))
		_samplesPerSec = ConfMan.getInt("output_rate");
	if (_samplesPerSec <= 0)
		_samplesPerSec = SAMPLES_PER_SEC;

	//Quick EVIL Hack - DJWillis
	_samplesPerSec = 11025;

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
	//desired.samples = (uint16)samples;
	desired.samples = 128; // Samples hack
	desired.callback = mixCallback;
	desired.userdata = this;

	// Create the mixer instance
	assert(!_mixer);
	_mixer = new Audio::MixerImpl(this);
	assert(_mixer);

	if (SDL_OpenAudio(&desired, &obtained) != 0) {
		warning("Could not open audio device: %s", SDL_GetError());
		_samplesPerSec = 0;
		_mixer->setReady(false);
	} else {
		// Note: This should be the obtained output rate, but it seems that at
		// least on some platforms SDL will lie and claim it did get the rate
		// even if it didn't. Probably only happens for "weird" rates, though.
		_samplesPerSec = obtained.freq;
		debug(1, "Output sample rate: %d Hz", _samplesPerSec);

		// Tell the mixer that we are ready and start the sound processing
		_mixer->setOutputRate(_samplesPerSec);
		_mixer->setReady(true);

#ifdef MIXER_DOUBLE_BUFFERING
		initThreadedMixer(_mixer, obtained.samples * 4);
#endif

		// start the sound system
		SDL_PauseAudio(0);
	}
}

void OSystem_GP2X::closeMixer() {
	if (_mixer)
		_mixer->setReady(false);

	SDL_CloseAudio();

	delete _mixer;
	_mixer = 0;

#ifdef MIXER_DOUBLE_BUFFERING
	deinitThreadedMixer();
#endif

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
