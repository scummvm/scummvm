/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "backends/sdl/sdl-common.h"
#include "common/config-manager.h"
#include "common/util.h"

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "scummvm.xpm"


OSystem *OSystem_SDL_create() {
	return new OSystem_SDL();
}

void OSystem_SDL::init_intern() {

	int joystick_num = ConfMan.getInt("joystick_num");
	uint32 sdlFlags = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER;

	if (joystick_num > -1)
		sdlFlags |= SDL_INIT_JOYSTICK;

	if (SDL_Init(sdlFlags) == -1) {
		error("Could not initialize SDL: %s", SDL_GetError());
	}

	_graphicsMutex = createMutex();

	SDL_ShowCursor(SDL_DISABLE);
	
	// Enable unicode support if possible
	SDL_EnableUNICODE(1); 

	cksum_valid = false;
	_mode = GFX_DOUBLESIZE;
	_full_screen = ConfMan.getBool("fullscreen");
	_adjustAspectRatio = ConfMan.getBool("aspect_ratio");
	_mode_flags = 0;


#ifndef MACOSX		// Don't set icon on OS X, as we use a nicer external icon there
	// Setup the icon
	setup_icon();
#endif

	// enable joystick
	if (joystick_num > -1 && SDL_NumJoysticks() > 0) {
		printf("Using joystick: %s\n", SDL_JoystickName(0));
		init_joystick(joystick_num);
	}
}

OSystem_SDL::OSystem_SDL()
	: _hwscreen(0), _screen(0), _screenWidth(0), _screenHeight(0),
	_tmpscreen(0), _tmpScreenWidth(0), _overlayVisible(false),
	_cdrom(0), _scaler_proc(0), _modeChanged(false), _dirty_checksums(0),
	_mouseVisible(false), _mouseDrawn(false), _mouseData(0),
	_mouseHotspotX(0), _mouseHotspotY(0),
	_currentShakePos(0), _newShakePos(0),
	_paletteDirtyStart(0), _paletteDirtyEnd(0),
	_graphicsMutex(0) {

	// allocate palette storage
	_currentPalette = (SDL_Color *)calloc(sizeof(SDL_Color), 256);

	// allocate the dirty rect storage
	_mouseBackup = (byte *)malloc(MAX_MOUSE_W * MAX_MOUSE_H * MAX_SCALING * 2);

	// reset mouse state
	memset(&km, 0, sizeof(km));
	
	init_intern();
}

OSystem_SDL::~OSystem_SDL() {
//	unload_gfx_mode();

	if (_dirty_checksums)
		free(_dirty_checksums);
	free(_currentPalette);
	free(_mouseBackup);
	deleteMutex(_graphicsMutex);

	SDL_ShowCursor(SDL_ENABLE);
	SDL_Quit();
}

uint32 OSystem_SDL::get_msecs() {
	return SDL_GetTicks();	
}

void OSystem_SDL::delay_msecs(uint msecs) {
	SDL_Delay(msecs);
}

void OSystem_SDL::set_timer(TimerProc callback, int timer) {
	SDL_SetTimer(timer, (SDL_TimerCallback) callback);
}

void OSystem_SDL::setWindowCaption(const char *caption) {
	SDL_WM_SetCaption(caption, caption);
}

bool OSystem_SDL::hasFeature(Feature f) {
	return
		(f == kFeatureFullscreenMode) ||
		(f == kFeatureAspectRatioCorrection) ||
		(f == kFeatureAutoComputeDirtyRects);
}

void OSystem_SDL::setFeatureState(Feature f, bool enable) {
	switch (f) {
	case kFeatureFullscreenMode:
		setFullscreenMode(enable);
		break;
	case kFeatureAspectRatioCorrection:
		if (_screenHeight == 200 && _adjustAspectRatio != enable) {
			Common::StackLock lock(_graphicsMutex, this);

			//assert(_hwscreen != 0);
			_adjustAspectRatio ^= true;
			hotswap_gfx_mode();
		}
		break;
	case kFeatureAutoComputeDirtyRects:
		if (enable)
			_mode_flags |= DF_WANT_RECT_OPTIM;		
		else
			_mode_flags &= ~DF_WANT_RECT_OPTIM;		
		break;
	default:
		break;
	}
}

bool OSystem_SDL::getFeatureState(Feature f) {
	switch (f) {
	case kFeatureFullscreenMode:
		return _full_screen;
	case kFeatureAspectRatioCorrection:
		return _adjustAspectRatio;
	case kFeatureAutoComputeDirtyRects:
		return _mode_flags & DF_WANT_RECT_OPTIM;
	default:
		return false;
	}
}

void OSystem_SDL::quit() {
	if(_cdrom) {
		SDL_CDStop(_cdrom);
		SDL_CDClose(_cdrom);
	}
	unload_gfx_mode();

	SDL_ShowCursor(SDL_ENABLE);
	SDL_Quit();

	exit(0);
}

void OSystem_SDL::setup_icon() {
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

	memset(&desired, 0, sizeof(desired));

	desired.freq = SAMPLES_PER_SEC;
	desired.format = AUDIO_S16SYS;
	desired.channels = 2;
	desired.samples = 2048;
	desired.callback = proc;
	desired.userdata = param;
	if (SDL_OpenAudio(&desired, NULL) != 0) {
		return false;
	}
	SDL_PauseAudio(0);
	return true;
}

void OSystem_SDL::clearSoundCallback() {
	SDL_CloseAudio();
}

int OSystem_SDL::getOutputSampleRate() const {
	return SAMPLES_PER_SEC;
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
			cd_num_loops = 0;
			cd_stop_time = 0;
			cd_end_time = 0;
		}
	}
	
	return (_cdrom != NULL);
}

void OSystem_SDL::stop_cdrom() {	/* Stop CD Audio in 1/10th of a second */
	cd_stop_time = SDL_GetTicks() + 100;
	cd_num_loops = 0;
}

void OSystem_SDL::play_cdrom(int track, int num_loops, int start_frame, int duration) {
	if (!num_loops && !start_frame)
		return;

	if (!_cdrom)
		return;
	
	if (duration > 0)
		duration += 5;

	cd_track = track;
	cd_num_loops = num_loops;
	cd_start_frame = start_frame;

	SDL_CDStatus(_cdrom);
	if (start_frame == 0 && duration == 0)
		SDL_CDPlayTracks(_cdrom, track, 0, 1, 0);
	else
		SDL_CDPlayTracks(_cdrom, track, start_frame, 0, duration);
	cd_duration = duration;
	cd_stop_time = 0;
	cd_end_time = SDL_GetTicks() + _cdrom->track[track].length * 1000 / CD_FPS;
}

bool OSystem_SDL::poll_cdrom() {
	if (!_cdrom)
		return false;

	return (cd_num_loops != 0 && (SDL_GetTicks() < cd_end_time || SDL_CDStatus(_cdrom) != CD_STOPPED));
}

void OSystem_SDL::update_cdrom() {
	if (!_cdrom)
		return;

	if (cd_stop_time != 0 && SDL_GetTicks() >= cd_stop_time) {
		SDL_CDStop(_cdrom);
		cd_num_loops = 0;
		cd_stop_time = 0;
		return;
	}

	if (cd_num_loops == 0 || SDL_GetTicks() < cd_end_time)
		return;

	if (cd_num_loops != 1 && SDL_CDStatus(_cdrom) != CD_STOPPED) {
		// Wait another second for it to be done
		cd_end_time += 1000;
		return;
	}

	if (cd_num_loops > 0)
		cd_num_loops--;

	if (cd_num_loops != 0) {
		if (cd_start_frame == 0 && cd_duration == 0)
			SDL_CDPlayTracks(_cdrom, cd_track, 0, 1, 0);
		else
			SDL_CDPlayTracks(_cdrom, cd_track, cd_start_frame, 0, cd_duration);
		cd_end_time = SDL_GetTicks() + _cdrom->track[cd_track].length * 1000 / CD_FPS;
	}
}
