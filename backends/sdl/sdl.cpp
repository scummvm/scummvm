/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
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

#include "backends/sdl/sdl-common.h"
#include "common/config-manager.h"
#include "common/util.h"
#include "base/main.h"

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "scummvm.xpm"

#if defined(__amigaos4__)
// Set the stack cookie, 640 KB should be enough for everyone
const char* stackCookie = "$STACK: 655360\0";
#endif

#if !defined(_WIN32_WCE) && !defined(__MAEMO__)

#if defined (WIN32)
int WinMain(HINSTANCE /*hInst*/, HINSTANCE /*hPrevInst*/,  LPSTR /*lpCmdLine*/, int /*iShowCmd*/) {
    return main(__argc, __argv);
}
#endif

int main(int argc, char *argv[]) {

#if defined(__SYMBIAN32__)
	//
	// Set up redirects for stdout/stderr under Windows and Symbian.
	// Code copied from SDL_main.
	//
	
	// Symbian does not like any output to the console through any *print* function
	char STDOUT_FILE[256], STDERR_FILE[256]; // shhh, don't tell anybody :)
	strcpy(STDOUT_FILE, Symbian::GetExecutablePath());
	strcpy(STDERR_FILE, Symbian::GetExecutablePath());
	strcat(STDOUT_FILE, "scummvm.stdout.txt");
	strcat(STDERR_FILE, "scummvm.stderr.txt");

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

#endif // defined(__SYMBIAN32__)


	// Invoke the actual ScummVM main entry point:
	return scummvm_main(argc, argv);
}
#endif

OSystem *OSystem_SDL_create() {
	return new OSystem_SDL();
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

	_cksumValid = false;
#if !defined(_WIN32_WCE) && !defined(__SYMBIAN32__) && !defined(DISABLE_SCALERS)
	_mode = GFX_DOUBLESIZE;
	_scaleFactor = 2;
	_scalerProc = Normal2x;
	_fullscreen = ConfMan.getBool("fullscreen");
	_adjustAspectRatio = ConfMan.getBool("aspect_ratio");
#else // for small screen platforms
	_mode = GFX_NORMAL;
	_scaleFactor = 1;
	_scalerProc = Normal1x;

#if !defined(_WIN32_WCE) && !defined(__SYMBIAN32__) &&  !defined(__MAEMO__)
	_fullscreen = ConfMan.getBool("fullscreen");
#else
	_fullscreen = true;
#endif

	_adjustAspectRatio = false;
#endif
	_scalerType = 0;
	_modeFlags = 0;

#if !defined(MACOSX) && !defined(__SYMBIAN32__)		// Don't set icon on OS X, as we use a nicer external icon there
	setupIcon();									// Don't for Symbian: it uses the EScummVM.aif file for the icon
#endif

	// enable joystick
	if (joystick_num > -1 && SDL_NumJoysticks() > 0) {
		printf("Using joystick: %s\n", SDL_JoystickName(0));
		_joystick = SDL_JoystickOpen(joystick_num);
	}

	_inited = true;
}

OSystem_SDL::OSystem_SDL()
	:
#ifdef USE_OSD
	_osdSurface(0), _osdAlpha(SDL_ALPHA_TRANSPARENT), _osdFadeStartTime(0),
#endif
	_hwscreen(0), _screen(0), _screenWidth(0), _screenHeight(0),
	_tmpscreen(0), _overlayWidth(0), _overlayHeight(0),
	_overlayVisible(false), _overlayScale(1),
	_overlayscreen(0), _tmpscreen2(0),
	_samplesPerSec(0),
	_cdrom(0), _scalerProc(0), _modeChanged(false), _dirtyChecksums(0),
	_mouseVisible(false), _mouseDrawn(false), _mouseData(0), _mouseSurface(0),
	_mouseOrigSurface(0), _mouseHotspotX(0), _mouseHotspotY(0), _cursorTargetScale(1),
	_cursorHasOwnPalette(false), _cursorPaletteDisabled(true),
	_joystick(0),
	_currentShakePos(0), _newShakePos(0),
	_paletteDirtyStart(0), _paletteDirtyEnd(0),
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

OSystem_SDL::~OSystem_SDL() {
	free(_dirtyChecksums);
	free(_currentPalette);
	free(_cursorPalette);
	free(_mouseData);
}

uint32 OSystem_SDL::getMillis() {
	return SDL_GetTicks();
}

void OSystem_SDL::delayMillis(uint msecs) {
	SDL_Delay(msecs);
}

void OSystem_SDL::setTimerCallback(TimerProc callback, int timer) {
	SDL_SetTimer(timer, (SDL_TimerCallback) callback);
}

void OSystem_SDL::setWindowCaption(const char *caption) {
	SDL_WM_SetCaption(caption, caption);
}

bool OSystem_SDL::hasFeature(Feature f) {
	return
		(f == kFeatureFullscreenMode) ||
		(f == kFeatureAspectRatioCorrection) ||
		(f == kFeatureAutoComputeDirtyRects) ||
		(f == kFeatureCursorHasPalette);
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
	default:
		break;
	}
}

bool OSystem_SDL::getFeatureState(Feature f) {
	assert (_transactionMode == kTransactionNone);

	switch (f) {
	case kFeatureFullscreenMode:
		return _fullscreen;
	case kFeatureAspectRatioCorrection:
		return _adjustAspectRatio;
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
	SDL_Quit();

	exit(0);
}

void OSystem_SDL::setupIcon() {
	int w, h, ncols, nbytes, i;
	unsigned int rgba[256], icon[32 * 32];
	unsigned char mask[32][4];

	sscanf(scummvm_icon[0], "%d %d %d %d", &w, &h, &ncols, &nbytes);
	if ((w != 32) || (h != 32) || (ncols > 255) || (nbytes > 1)) {
		warning("Could not load the icon (%d %d %d %d)", w, h, ncols, nbytes);
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
			warning("Could not load the icon (%d %s - %s) ", code, color, scummvm_icon[1 + i]);
			return;
		}

		rgba[code] = col;
	}
	memset(mask, 0, sizeof(mask));
	for (h = 0; h < 32; h++) {
		const char *line = scummvm_icon[1 + ncols + h];
		for (w = 0; w < 32; w++) {
			icon[w + 32 * h] = rgba[(int)line[w]];
			if (rgba[(int)line[w]] & 0xFF000000) {
				mask[h][w >> 3] |= 1 << (7 - (w & 0x07));
			}
		}
	}

	SDL_Surface *sdl_surf = SDL_CreateRGBSurfaceFrom(icon, 32, 32, 32, 32 * 4, 0xFF0000, 0x00FF00, 0x0000FF, 0xFF000000);
	SDL_WM_SetIcon(sdl_surf, (unsigned char *) mask);
	SDL_FreeSurface(sdl_surf);
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

bool OSystem_SDL::setSoundCallback(SoundProc proc, void *param) {
	SDL_AudioSpec desired;
	SDL_AudioSpec obtained;

	memset(&desired, 0, sizeof(desired));

	_samplesPerSec = 0;

	if (ConfMan.hasKey("output_rate"))
		_samplesPerSec = ConfMan.getInt("output_rate");

	if (_samplesPerSec <= 0)
		_samplesPerSec = SAMPLES_PER_SEC;

	// Originally, we always used 2048 samples. This loop will produce the
	// same result at 22050 Hz, and should hopefully produce something
	// sensible for other frequencies. Note that it must be a power of two.

	uint32 samples = 0x8000;

	for (;;) {
		if ((1000 * samples) / _samplesPerSec < 100)
			break;
		samples >>= 1;
	}

	desired.freq = _samplesPerSec;
	desired.format = AUDIO_S16SYS;
	desired.channels = 2;
	desired.samples = (uint16)samples;
	desired.callback = proc;
	desired.userdata = param;
	if (SDL_OpenAudio(&desired, &obtained) != 0) {
		warning("Could not open audio device: %s", SDL_GetError());
		return false;
	}
	// Note: This should be the obtained output rate, but it seems that at
	// least on some platforms SDL will lie and claim it did get the rate
	// even if it didn't. Probably only happens for "weird" rates, though.
	_samplesPerSec = obtained.freq;
	SDL_PauseAudio(0);
	return true;
}

void OSystem_SDL::clearSoundCallback() {
	SDL_CloseAudio();
}

int OSystem_SDL::getOutputSampleRate() const {
	return _samplesPerSec;
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

	return (_cdNumLoops != 0 && (SDL_GetTicks() < _cdEndTime || SDL_CDStatus(_cdrom) != CD_STOPPED));
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
