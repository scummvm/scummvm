/* ScummVM - Scumm Interpreter
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

#include "common/stdafx.h"
#include "wince-sdl.h"

#include "common/util.h"
#include "base/gameDetector.h"
#include "base/engine.h"
#include "base/plugins.h"
#include "common/timer.h"

#include "common/config-manager.h"

#include "scumm/scumm.h"

#include "resource.h"

#include "CEActions.h"
#include "ItemAction.h"
#include "CEKeysDialog.h"

#include "gui/message.h"

#include "sound/fmopl.h"

#ifdef USE_VORBIS
#include <vorbis/vorbisfile.h>
#endif

using namespace CEGUI; 

// ********************************************************************************************

// Internal GUI names

#define NAME_MAIN_PANEL			"MainPanel"
#define NAME_PANEL_KEYBOARD		"Keyboard"
#define NAME_ITEM_OPTIONS		"Options"
#define NAME_ITEM_SKIP			"Skip"
#define NAME_ITEM_SOUND			"Sound"
#define NAME_ITEM_ORIENTATION	"Orientation"

// Given to the true main, needed for backend adaptation

static GameDetector _gameDetector;
static FILE *stdout_file;
static FILE *stderr_file;
	

// Static member inits

bool OSystem_WINCE3::_soundMaster = true;
OSystem::SoundProc OSystem_WINCE3::_originalSoundProc = NULL;

// ********************************************************************************************

// MAIN

extern "C" int scummvm_main(GameDetector &gameDetector, int argc, char **argv);

int SDL_main(int argc, char **argv) {
	/* Avoid print problems - this file will be put in RAM anyway */
	stdout_file = fopen("\\scummvm_stdout.txt", "w");
	stderr_file = fopen("\\scummvm_stderr.txt", "w");
	return scummvm_main(_gameDetector, argc, argv);
}    
   
// ********************************************************************************************


// ********************************************************************************************

void pumpMessages() {
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void drawError(char *error) {
	TCHAR errorUnicode[200];
	MultiByteToWideChar(CP_ACP, 0, error, strlen(error) + 1, errorUnicode, sizeof(errorUnicode));
	pumpMessages();
	MessageBox(GetActiveWindow(), errorUnicode, TEXT("ScummVM error"), MB_OK | MB_ICONERROR);
	pumpMessages();
}

bool isSmartphone(void) {
	return false;  
}


// ********************************************************************************************

OSystem *OSystem_WINCE3_create(int gfx_mode) {
	return OSystem_SDL_Common::create(gfx_mode);
}

OSystem *OSystem_WINCE3_create() {
	return OSystem_SDL_Common::create(GFX_NORMAL);
}


OSystem_SDL_Common *OSystem_SDL_Common::create_intern() {
	return new OSystem_WINCE3();
}

OSystem_WINCE3::OSystem_WINCE3()
	 : _hwscreen(0), _scaler_proc(0), _orientationLandscape(false), _newOrientation(false),
	   _panelInitialized(false), _forcePanelInvisible(false), _panelVisible(true),
	   _panelStateForced(false), _addRightClickDown(false), _addRightClickUp(false),
	   _forceHideMouse(false), _freeLook(false), _toolbarHighDrawn(false)
{
	CEDevice::enableHardwareKeyMapping();
	create_toolbar();
}

void OSystem_WINCE3::swap_panel_visibility() {
	if (!_forcePanelInvisible && !_panelStateForced) {
		_panelVisible = !_panelVisible;
		_toolbarHandler.setVisible(_panelVisible);
		if (_screenHeight > 240)
			add_dirty_rect(0, 400, 640, 80);
		else
			add_dirty_rect(0, 200, 320, 40);
			
		update_screen();
	}
}

void OSystem_WINCE3::swap_panel() {
	_toolbarHighDrawn = false;
	if (!_panelStateForced) {
		if (_toolbarHandler.activeName() == NAME_PANEL_KEYBOARD)
			_toolbarHandler.setActive(NAME_MAIN_PANEL);
		else
			_toolbarHandler.setActive(NAME_PANEL_KEYBOARD);
	}
}

void OSystem_WINCE3::swap_sound_master() {
	_soundMaster = !_soundMaster;
	if (_toolbarHandler.activeName() == NAME_MAIN_PANEL)
		_toolbarHandler.forceRedraw(); // redraw sound icon
}

void OSystem_WINCE3::add_right_click() {
	_addRightClickDown = true;
}

void OSystem_WINCE3::swap_mouse_visibility() {
	_forceHideMouse = !_forceHideMouse;
	if (_forceHideMouse)
		undraw_mouse();
}

void OSystem_WINCE3::swap_freeLook() {
	_freeLook = !_freeLook;
}

void OSystem_WINCE3::create_toolbar() {
	PanelKeyboard *keyboard;

	// Add the keyboard
	keyboard = new PanelKeyboard(PANEL_KEYBOARD, _keysBuffer);
	_toolbarHandler.add(NAME_PANEL_KEYBOARD, *keyboard);

}

void OSystem_WINCE3::private_sound_proc(void *param, byte *buf, int len) {
	(*_originalSoundProc)(param, buf, len);
	if (!_soundMaster)
		memset(buf, 0, len);
}

#ifdef USE_VORBIS
bool OSystem_WINCE3::checkOggHighSampleRate() {
        char trackFile[255];
        FILE *testFile;
        OggVorbis_File *test_ov_file = new OggVorbis_File;

        sprintf(trackFile,"%sTrack1.ogg", ConfMan.get("path").c_str());
        // Check if we have an OGG audio track
        testFile = fopen(trackFile, "rb");
        if (testFile) {
                if (!ov_open(testFile, test_ov_file, NULL, 0)) {
                        bool highSampleRate = (ov_info(test_ov_file, -1)->rate == 22050);
                        ov_clear(test_ov_file);
                        return highSampleRate;
                }
        }

        // Do not test for OGG samples - too big and too slow anyway :)

        return false;
}
#endif

void OSystem_WINCE3::get_sample_rate() {
	// See if the output frequency is forced by the game
	if ((_gameDetector._game.features & Scumm::GF_DIGI_IMUSE) ||
		_gameDetector._targetName == "queen" ||
		strncmp(_gameDetector._targetName.c_str(), "sword", 5) == 0 ||
		strncmp(_gameDetector._targetName.c_str(), "sky", 3) == 0)
			_sampleRate = SAMPLES_PER_SEC_NEW;
	else {
		if (ConfMan.getBool("CE_high_sample_rate"))
			_sampleRate = SAMPLES_PER_SEC_NEW;
		else
			_sampleRate = SAMPLES_PER_SEC_OLD;
	}

#ifdef USE_VORBIS
    // Modify the sample rate on the fly if OGG is involved
    if (_sampleRate == SAMPLES_PER_SEC_OLD)
     if (checkOggHighSampleRate())
		 _sampleRate = SAMPLES_PER_SEC_NEW;
#endif
}

bool OSystem_WINCE3::set_sound_proc(SoundProc proc, void *param, SoundFormat format) {
	SDL_AudioSpec desired;
	int thread_priority;

	memset(&desired, 0, sizeof(desired));

	_originalSoundProc = proc;
	desired.freq = _sampleRate;
	desired.format = AUDIO_S16SYS; 
	desired.channels = 2;
	//desired.samples = 2048;
	desired.samples = 128;
	desired.callback = private_sound_proc;
	desired.userdata = param;

	// Add sound thread priority
	if (ConfMan.get("CE_sound_thread_priority").isEmpty()) {
#ifdef SH3
		thread_priority = THREAD_PRIORITY_NORMAL;
#else
		thread_priority = THREAD_PRIORITY_ABOVE_NORMAL;
#endif
	}
	else
		thread_priority = ConfMan.getInt("CE_sound_thread_priority");

	desired.thread_priority = thread_priority;

	if (SDL_OpenAudio(&desired, NULL) != 0) {
		return false;
	}
	SDL_PauseAudio(0);
	return true;
}

void OSystem_WINCE3::check_mappings() {
		if (!_gameDetector._targetName.size())
			return;
		
		CEActions::init(this, _gameDetector);
		// Load key mapping
		CEActions::Instance()->loadMapping();

		// Some games need to map the right click button, signal it here if it wasn't done
		if (CEActions::Instance()->needsRightClickMapping()) {
			while (!CEActions::Instance()->getMapping(ACTION_RIGHTCLICK)) {
				CEKeysDialog *keysDialog = new CEKeysDialog("Map right click action");	
				keysDialog->runModal();
				if (!CEActions::Instance()->getMapping(ACTION_RIGHTCLICK)) {
					GUI::MessageDialog alert("You must map a key to the 'Right Click' action to play this game");
					alert.runModal();
				}					
			}
		}

		// Map the "hide toolbar" action if needed
		if (CEActions::Instance()->needsHideToolbarMapping()) {
			while (!CEActions::Instance()->getMapping(ACTION_HIDE)) {
				CEKeysDialog *keysDialog = new CEKeysDialog("Map hide toolbar action");
				keysDialog->runModal();
				if (!CEActions::Instance()->getMapping(ACTION_HIDE)) {
					GUI::MessageDialog alert("You must map a key to the 'Hide toolbar' action to play this game");
					alert.runModal();
				}
			}
		}

		// Extra warning for Zak Mc Kracken
		if (strncmp(_gameDetector._targetName.c_str(), "zak", 3) == 0 &&  
			!CEActions::Instance()->getMapping(ACTION_HIDE)) {
			GUI::MessageDialog alert("Don't forget to map a key to 'Hide Toolbar' action to see the whole inventory");
			alert.runModal();
		}
}

void OSystem_WINCE3::update_game_settings() {
	// Finish panel initialization
	if (!_panelInitialized && _gameDetector._targetName.size()) {
		Panel *panel;
		_panelInitialized = true;		
		// Add the main panel
		panel = new Panel(10, 40);
		panel->setBackground(IMAGE_PANEL);
		// Save
		panel->add(NAME_ITEM_OPTIONS, new ItemAction(ITEM_OPTIONS, ACTION_SAVE));
		// Skip
		panel->add(NAME_ITEM_SKIP, new ItemAction(ITEM_SKIP, ACTION_SKIP));
		// sound
		panel->add(NAME_ITEM_SOUND, new ItemSwitch(ITEM_SOUND_OFF, ITEM_SOUND_ON, &_soundMaster)); 
		// portrait/landscape - screen dependant
		if (_screenWidth <= 320) 
			panel->add(NAME_ITEM_ORIENTATION, new ItemSwitch(ITEM_VIEW_LANDSCAPE, ITEM_VIEW_PORTRAIT, &_newOrientation));
		_toolbarHandler.add(NAME_MAIN_PANEL, *panel);
		_toolbarHandler.setActive(NAME_MAIN_PANEL);

		// Keyboard is active for Monkey 1 or 2
		if (strncmp(_gameDetector._targetName.c_str(), "monkey", 6) == 0) {
			_monkeyKeyboard = true;
			_toolbarHandler.setActive(NAME_PANEL_KEYBOARD);
		}

		if (ConfMan.getBool("CE_landscape"))
			load_gfx_mode();
	}
}

void OSystem_WINCE3::init_size(uint w, uint h) {
	if (w == 320 && h == 200)
		h = 240; // use the extra 40 pixels height for the toolbar

	check_mappings(); // do it here to have a readable dialog

	if (h == 240)
		_toolbarHandler.setOffset(200);
	else
		_toolbarHandler.setOffset(400);	

	OSystem_SDL_Common::init_size(w, h);	

	update_game_settings();

}

void OSystem_WINCE3::load_gfx_mode() {
	int displayWidth;
	int displayHeight;

	_full_screen = true; // forced  
	_forceFull = true;
	_mode_flags |= DF_UPDATE_EXPAND_1_PIXEL;
   
	_tmpscreen = NULL;
	_tmpScreenWidth = (_screenWidth + 3); 

	_scaleFactorXm = -1;
	_scaleFactorXd = -1;
	_scaleFactorYm = -1;    
	_scaleFactorYd = -1;  
	_scaleFactor = 0;

	_newOrientation = _orientationLandscape = ConfMan.getBool("CE_landscape");

	if (CEDevice::hasPocketPCResolution()) {
		if (!_orientationLandscape && _screenWidth == 320) {
			_scaleFactorXm = 3;
			_scaleFactorXd = 4;
			_scaleFactorYm = 1; 
			_scaleFactorYd = 1;
			_scaler_proc = PocketPCPortrait;
			_mode_flags = 0;
		}
		if (_screenWidth == 640) {
			_scaleFactorXm = 1;
			_scaleFactorXd = 2;
			_scaleFactorYm = 1;
			_scaleFactorYd = 2;
			_scaler_proc = PocketPCHalf;
			_mode_flags = 0;
		}
	}

	if (CEDevice::hasPocketPCResolution() && _orientationLandscape)
		_mode = GFX_NORMAL;

	if (_scaleFactorXm < 0) {
		/* Standard scalers, from the SDL backend */
		switch(_mode) {    
		case GFX_NORMAL:     
			_scaleFactor = 1;
			_scaler_proc = Normal1x;
			break;
		case GFX_DOUBLESIZE: 
			_scaleFactor = 2;
			_scaler_proc = Normal2x;
			break;
		case GFX_TRIPLESIZE:
			_scaleFactor = 3;
			_scaler_proc = Normal3x;
			break;
		case GFX_2XSAI:
			_scaleFactor = 2;
			_scaler_proc = _2xSaI;
			break;
		case GFX_SUPER2XSAI:
			_scaleFactor = 2;
			_scaler_proc = Super2xSaI;
			break;
		case GFX_SUPEREAGLE:
			_scaleFactor = 2;
			_scaler_proc = SuperEagle;
			break;
		case GFX_ADVMAME2X:
			_scaleFactor = 2;
			_scaler_proc = AdvMame2x;
			break;
		case GFX_ADVMAME3X:
			_scaleFactor = 3;
			_scaler_proc = AdvMame3x;
			break;
		case GFX_HQ2X:
			_scaleFactor = 2;
			_scaler_proc = HQ2x;
			break;
		case GFX_HQ3X:
			_scaleFactor = 3;
			_scaler_proc = HQ3x;
			break;
		case GFX_TV2X:
			_scaleFactor = 2;
			_scaler_proc = TV2x;
			break;
		case GFX_DOTMATRIX:
			_scaleFactor = 2;
			_scaler_proc = DotMatrix;
			break;

		default:
			error("unknown gfx mode %d", _mode);
		} 
	}

	// Check if the scaler can be accepted, if not get back to normal scaler
	if (_scaleFactor && ((_scaleFactor * _screenWidth > GetSystemMetrics(SM_CXSCREEN) &&
						  _scaleFactor * _screenWidth > GetSystemMetrics(SM_CYSCREEN))
					  || (_scaleFactor * _screenHeight > GetSystemMetrics(SM_CXSCREEN) &&
						  _scaleFactor * _screenHeight > GetSystemMetrics(SM_CYSCREEN)))) {
				_scaleFactor = 1;
				_scaler_proc = Normal1x;
	}

	// Common scaler system was used
	if (_scaleFactorXm < 0) {
		_scaleFactorXm = _scaleFactor;
		_scaleFactorXd = 1;
		_scaleFactorYm = _scaleFactor;
		_scaleFactorYd = 1;
	}

	// 
	// Create the surface that contains the 8 bit game data
	_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, _screenWidth, _screenHeight, 8, 0, 0, 0, 0);
	if (_screen == NULL)
		error("_screen failed");    

	//
	// Create the surface that contains the scaled graphics in 16 bit mode
	//

	// Always use full screen mode to have a "clean screen"
	displayWidth = _screenWidth * _scaleFactorXm / _scaleFactorXd;
	displayHeight = _screenHeight * _scaleFactorYm / _scaleFactorYd;

	if (!(displayWidth > GetSystemMetrics(SM_CXSCREEN))) { // no rotation
		displayWidth = GetSystemMetrics(SM_CXSCREEN);
		displayHeight = GetSystemMetrics(SM_CYSCREEN);
	}

	_hwscreen = SDL_SetVideoMode(displayWidth, displayHeight, 16, SDL_FULLSCREEN | SDL_SWSURFACE);
	if (_hwscreen == NULL) {
		// DON'T use error(), as this tries to bring up the debug
		// console, which WON'T WORK now that _hwscreen is hosed.

		// FIXME: We should be able to continue the game without
		// shutting down or bringing up the debug console, but at
		// this point we've already screwed up all our member vars.
		// We need to find a way to call SDL_VideoModeOK *before*
		// that happens and revert to all the old settings if we
		// can't pull off the switch to the new settings.
		//
		// Fingolfin says: the "easy" way to do that is not to modify
		// the member vars before we are sure everything is fine. Think
		// of "transactions, commit, rollback" style... we use local vars
		// in place of the member vars, do everything etc. etc.. In case
		// of a failure, rollback is trivial. Only if everything worked fine
		// do we "commit" the changed values to the member vars.
		warning("SDL_SetVideoMode says we can't switch to that mode");
		quit();
	}

	//
	// Create the surface used for the graphics in 16 bit before scaling, and also the overlay
	//

	// Distinguish 555 and 565 mode
	if (_hwscreen->format->Rmask == 0x7C00)
		InitScalers(555);
	else
		InitScalers(565);
	initCEScaler();
	
	// Need some extra bytes around when using 2xSaI
	uint16 *tmp_screen = (uint16 *)calloc(_tmpScreenWidth * (_screenHeight + 3), sizeof(uint16));
	_tmpscreen = SDL_CreateRGBSurfaceFrom(tmp_screen,
						_tmpScreenWidth, _screenHeight + 3, 16, _tmpScreenWidth * 2,
						_hwscreen->format->Rmask,
						_hwscreen->format->Gmask,
						_hwscreen->format->Bmask,
						_hwscreen->format->Amask);

	if (_tmpscreen == NULL)
		error("_tmpscreen failed");

	// Toolbar
	uint16 *toolbar_screen = (uint16 *)calloc(320 * 40, sizeof(uint16));
	_toolbarLow = SDL_CreateRGBSurfaceFrom(toolbar_screen,
						320, 40, 16, 320 * 2,
						_hwscreen->format->Rmask,
						_hwscreen->format->Gmask,
						_hwscreen->format->Bmask,
						_hwscreen->format->Amask
					);
	if (_toolbarLow == NULL)
		error("_toolbarLow failed");

	if (_screenHeight > 240) {
		uint16 *toolbar_screen = (uint16 *)calloc(640 * 80, sizeof(uint16));
		_toolbarHigh = SDL_CreateRGBSurfaceFrom(toolbar_screen,
							640, 80, 16, 640 * 2,
							_hwscreen->format->Rmask,
							_hwscreen->format->Gmask,
							_hwscreen->format->Bmask,
							_hwscreen->format->Amask
						);
		if (_toolbarHigh == NULL)
			error("_toolbarHigh failed");
	}
	else
		_toolbarHigh = NULL;


	// keyboard cursor control, some other better place for it?
	km.x_max = _screenWidth * _scaleFactorXm / _scaleFactorXd - 1;
	km.y_max = _screenHeight * _scaleFactorXm / _scaleFactorXd - 1;
	km.delay_time = 25;
	km.last_time = 0; 
}

void OSystem_WINCE3::unload_gfx_mode() {
	if (_screen) {
		SDL_FreeSurface(_screen);
		_screen = NULL; 
	}

	if (_hwscreen) {
		SDL_FreeSurface(_hwscreen); 
		_hwscreen = NULL;
	}

	if (_tmpscreen) {
		free(_tmpscreen->pixels);
		SDL_FreeSurface(_tmpscreen);
		_tmpscreen = NULL;
	}
}

void OSystem_WINCE3::hotswap_gfx_mode() {
	if (!_screen)
		return;

	// Keep around the old _screen & _tmpscreen so we can restore the screen data
	// after the mode switch.
	SDL_Surface *old_screen = _screen;
	SDL_Surface *old_tmpscreen = _tmpscreen;

	// Release the HW screen surface
	SDL_FreeSurface(_hwscreen); 

	// Release toolbars
	free(_toolbarLow->pixels);
	SDL_FreeSurface(_toolbarLow);
	if (_toolbarHigh) {
		free(_toolbarHigh->pixels);
		SDL_FreeSurface(_toolbarHigh);
	}

	// Setup the new GFX mode
	load_gfx_mode();

	// reset palette
	SDL_SetColors(_screen, _currentPalette, 0, 256);

	// Restore old screen content
	SDL_BlitSurface(old_screen, NULL, _screen, NULL);
	SDL_BlitSurface(old_tmpscreen, NULL, _tmpscreen, NULL);
	
	// Free the old surfaces
	SDL_FreeSurface(old_screen);
	free(old_tmpscreen->pixels);
	SDL_FreeSurface(old_tmpscreen);

	// Blit everything to the screen
	update_screen();
	
	// Make sure that an EVENT_SCREEN_CHANGED gets sent later
	_modeChanged = true;
}

void OSystem_WINCE3::update_keyboard() {

	// Update the forced keyboard for Monkey Island copy protection
	if (_monkeyKeyboard && Scumm::g_scumm->VAR(Scumm::g_scumm->VAR_ROOM) != 108 &&
		Scumm::g_scumm->VAR(Scumm::g_scumm->VAR_ROOM) != 90) {
			// Switch back to the normal panel now that the keyboard is not used anymore
			_monkeyKeyboard = false;
			_toolbarHandler.setActive(NAME_MAIN_PANEL);
	}
}

void OSystem_WINCE3::update_screen() {
	assert(_hwscreen != NULL);

	Common::StackLock lock(_graphicsMutex, this);	// Lock the mutex until this function ends

	update_keyboard();

	// If the shake position changed, fill the dirty area with blackness
	if (_currentShakePos != _newShakePos) {
		SDL_Rect blackrect = {0, 0, _screenWidth * _scaleFactorXm / _scaleFactorXd, _newShakePos * _scaleFactorYm / _scaleFactorYd};

		if (_adjustAspectRatio)
			blackrect.h = real2Aspect(blackrect.h - 1) + 1;

		SDL_FillRect(_hwscreen, &blackrect, 0);

		_currentShakePos = _newShakePos;

		_forceFull = true;
	}

	// Make sure the mouse is drawn, if it should be drawn.
	draw_mouse();
	
	// Check whether the palette was changed in the meantime and update the
	// screen surface accordingly. 
	if (_paletteDirtyEnd != 0) {
		SDL_SetColors(_screen, _currentPalette + _paletteDirtyStart, 
			_paletteDirtyStart,
			_paletteDirtyEnd - _paletteDirtyStart);
		
		_paletteDirtyEnd = 0;

		_forceFull = true;
	}

	// Force a full redraw if requested
	if (_forceFull) {
		_num_dirty_rects = 1;

		_dirty_rect_list[0].x = 0;
		_dirty_rect_list[0].y = 0;
		_dirty_rect_list[0].w = _screenWidth;
		_dirty_rect_list[0].h = _screenHeight;

		_toolbarHandler.forceRedraw();
	}

	// Only draw anything if necessary
	if (_num_dirty_rects > 0) {

		SDL_Rect *r; 
		SDL_Rect dst;
		uint32 srcPitch, dstPitch;
		SDL_Rect *last_rect = _dirty_rect_list + _num_dirty_rects;
		bool toolbarVisible = _toolbarHandler.visible();
		int toolbarOffset = _toolbarHandler.getOffset();

		if (_scaler_proc == Normal1x && !_adjustAspectRatio) {
			SDL_Surface *target = _overlayVisible ? _tmpscreen : _screen;
			for (r = _dirty_rect_list; r != last_rect; ++r) {
				dst = *r;

				// Check if the toolbar is overwritten
				if (!_forceFull && toolbarVisible && r->y + r->h >= toolbarOffset)  {
					_toolbarHandler.forceRedraw();
				}

				if (_overlayVisible) {
					// FIXME: I don't understand why this is necessary...
					dst.x--;
					dst.y--;
				}
				dst.y += _currentShakePos;
				if (SDL_BlitSurface(target, r, _hwscreen, &dst) != 0)
					error("SDL_BlitSurface failed: %s", SDL_GetError());
			}
		} else {
			if (!_overlayVisible) {
				for (r = _dirty_rect_list; r != last_rect; ++r) {
					dst = *r;
					dst.x++;	// Shift rect by one since 2xSai needs to acces the data around
					dst.y++;	// any pixel to scale it, and we want to avoid mem access crashes.
					if (SDL_BlitSurface(_screen, r, _tmpscreen, &dst) != 0)
						error("SDL_BlitSurface failed: %s", SDL_GetError());
				}
			}

			SDL_LockSurface(_tmpscreen);
			SDL_LockSurface(_hwscreen);

			srcPitch = _tmpscreen->pitch;
			dstPitch = _hwscreen->pitch;

			for (r = _dirty_rect_list; r != last_rect; ++r) {
				register int dst_y = r->y + _currentShakePos;
				register int dst_h = 0;
				register int orig_dst_y = 0;

				// Check if the toolbar is overwritten			
				if (!_forceFull && toolbarVisible && r->y + r->h >= toolbarOffset) {
					_toolbarHandler.forceRedraw();
				}
				
				if (dst_y < _screenHeight) {
					dst_h = r->h;
					if (dst_h > _screenHeight - dst_y)
						dst_h = _screenHeight - dst_y;

					dst_y *= _scaleFactorYm;
					dst_y /= _scaleFactorYd;

					if (_adjustAspectRatio) {
						orig_dst_y = dst_y;
						dst_y = real2Aspect(dst_y);
					}
				
					_scaler_proc((byte *)_tmpscreen->pixels + (r->x * 2 + 2) + (r->y + 1) * srcPitch, srcPitch,
						(byte *)_hwscreen->pixels + (r->x * 2 * _scaleFactorXm / _scaleFactorXd) + dst_y * dstPitch, dstPitch, r->w, dst_h);
				}

				r->x = r->x * _scaleFactorXm / _scaleFactorXd;
				r->y = dst_y;
				r->w = r->w * _scaleFactorXm / _scaleFactorXd;
				r->h = dst_h * _scaleFactorYm / _scaleFactorYd;

				/*if (_adjustAspectRatio && orig_dst_y / _scaleFactor < _screenHeight)
					r->h = stretch200To240((uint8 *) _hwscreen->pixels, dstPitch, r->w, r->h, r->x, r->y, orig_dst_y);
				*/
			}
			SDL_UnlockSurface(_tmpscreen);
			SDL_UnlockSurface(_hwscreen);
		}

		// Readjust the dirty rect list in case we are doing a full update.
		// This is necessary if shaking is active.
		if (_forceFull) {
			_dirty_rect_list[0].y = 0;
			_dirty_rect_list[0].h = (_adjustAspectRatio ? 240 : _screenHeight) * _scaleFactorYm / _scaleFactorYd;
		}
	}
	// Add the toolbar if needed
	SDL_Rect toolbar_rect[1];
	if (_toolbarHandler.draw(_toolbarLow, &toolbar_rect[0])) {
		// It can be drawn, scale it			
		uint32 srcPitch, dstPitch;
		SDL_Surface *toolbarSurface;

		if (_screenHeight > 240) {
			if (!_toolbarHighDrawn) {
				// Resize the toolbar
				SDL_LockSurface(_toolbarLow);
				SDL_LockSurface(_toolbarHigh);
				Normal2x((byte*)_toolbarLow->pixels, _toolbarLow->pitch, (byte*)_toolbarHigh->pixels, _toolbarHigh->pitch, toolbar_rect[0].w, toolbar_rect[0].h);
				SDL_UnlockSurface(_toolbarHigh);
				SDL_UnlockSurface(_toolbarLow);
				_toolbarHighDrawn = true;
			}
			toolbar_rect[0].w *= 2;
			toolbar_rect[0].h *= 2;
			toolbarSurface = _toolbarHigh;
		}
		else
			toolbarSurface = _toolbarLow;

		// Apply the appropriate scaler
		SDL_LockSurface(toolbarSurface);
		SDL_LockSurface(_hwscreen);
		srcPitch = toolbarSurface->pitch;
		dstPitch = _hwscreen->pitch;		
		_scaler_proc((byte *)toolbarSurface->pixels, srcPitch, (byte *)_hwscreen->pixels + (_toolbarHandler.getOffset() * _scaleFactorYm / _scaleFactorYd * dstPitch), dstPitch, toolbar_rect[0].w, toolbar_rect[0].h);
		SDL_UnlockSurface(toolbarSurface);
		SDL_UnlockSurface(_hwscreen);

		// And blit it
		toolbar_rect[0].y = _toolbarHandler.getOffset();
		toolbar_rect[0].x = toolbar_rect[0].x * _scaleFactorXm / _scaleFactorXd;
		toolbar_rect[0].y = toolbar_rect[0].y * _scaleFactorYm / _scaleFactorYd;
		toolbar_rect[0].w = toolbar_rect[0].w * _scaleFactorXm / _scaleFactorXd;
		toolbar_rect[0].h = toolbar_rect[0].h * _scaleFactorYm / _scaleFactorYd;

		SDL_UpdateRects(_hwscreen, 1, toolbar_rect);
	}

	// Finally, blit all our changes to the screen
	if (_num_dirty_rects > 0)
		SDL_UpdateRects(_hwscreen, _num_dirty_rects, _dirty_rect_list);

	_num_dirty_rects = 0;
	_forceFull = false;
}

uint32 OSystem_WINCE3::property(int param, Property *value) {

	Common::StackLock lock(_graphicsMutex, this);	// Lock the mutex until this function ends

	if (param == PROP_TOGGLE_FULLSCREEN) {
		// FIXME
		/*
		assert(_hwscreen != 0);
		_full_screen ^= true;
		if (!SDL_WM_ToggleFullScreen(_hwscreen)) {
			// if ToggleFullScreen fails, achieve the same effect with hotswap gfx mode			
			hotswap_gfx_mode();
		}
		*/
		return 1;
	} else if (param == PROP_SET_GFX_MODE) {
		if (value->gfx_mode > 11)	// FIXME! HACK, hard coded threshold, not good
			return 0;

		if (_mode != value->gfx_mode) {
			_mode = value->gfx_mode;
			hotswap_gfx_mode();
		}
		return 1;
	} else if (param == PROP_TOGGLE_ASPECT_RATIO) {
		if (_screenHeight == 200) {
			assert(_hwscreen != 0);
			_adjustAspectRatio ^= true;
			hotswap_gfx_mode();
		}
		return 1;
	} else if (param == PROP_TOGGLE_VIRTUAL_KEYBOARD) {
		_toolbarHighDrawn = false;
		if (value->show_keyboard) {			
			_panelStateForced = true;
			_saveToolbarState = _toolbarHandler.visible();
			_saveActiveToolbar = _toolbarHandler.activeName();
			_toolbarHandler.setActive(NAME_PANEL_KEYBOARD);
			_toolbarHandler.setVisible(true);
		}
		else {
			_panelStateForced = false;
			_toolbarHandler.setActive(_saveActiveToolbar);
			_toolbarHandler.setVisible(_saveToolbarState);
		}
		return 1;
	} else if (param == PROP_HAS_SCALER) {
		switch(value->gfx_mode) {
			case GFX_NORMAL:
				return 1;
			case GFX_DOUBLESIZE:
			case GFX_2XSAI:
			case GFX_SUPER2XSAI:
			case GFX_SUPEREAGLE:
			case GFX_ADVMAME2X:
			case GFX_HQ2X:
			case GFX_TV2X:
			case GFX_DOTMATRIX:
				return (CEDevice::hasWideResolution());
			default:
				return 0;
		}
	} else if (param == PROP_GET_SAMPLE_RATE) {
		if (!_sampleRate)
			get_sample_rate();
		return _sampleRate;
	} else if (param == PROP_GET_FMOPL_ENV_BITS) { // imuse FM quality
		if (ConfMan.getBool("CE_FM_high_quality"))
			return FMOPL_ENV_BITS_HQ;
		else
			return FMOPL_ENV_BITS_LQ;
	} else if (param == PROP_GET_FMOPL_EG_ENT) { // imuse FM quality
		if (ConfMan.getBool("CE_FM_high_quality"))
			return FMOPL_EG_ENT_HQ;
		else
			return FMOPL_EG_ENT_LQ;
	}
	
	return OSystem_SDL_Common::property(param, value);
}

bool OSystem_WINCE3::save_screenshot(const char *filename) {
	assert(_hwscreen != NULL);

	Common::StackLock lock(_graphicsMutex, this);	// Lock the mutex until this function ends
	SDL_SaveBMP(_hwscreen, filename);
	return true;
}

// FIXME
// Reuse static or proper mapping

static int mapKeyCE(SDLKey key, SDLMod mod, Uint16 unicode)
{
	if (key >= SDLK_F1 && key <= SDLK_F9) {
		return key - SDLK_F1 + 315;
	} else if (key >= SDLK_KP0 && key <= SDLK_KP9) {
		return key - SDLK_KP0 + '0';
	} else if (key >= SDLK_UP && key <= SDLK_PAGEDOWN) {
		return key;
	} else if (unicode) {
		return unicode;
	} else if (key >= 'a' && key <= 'z' && mod & KMOD_SHIFT) {
		return key & ~0x20;
	} else if (key >= SDLK_NUMLOCK && key <= SDLK_EURO) {
		return 0;
	}
	return key;
}


void OSystem_WINCE3::draw_mouse() {
	// FIXME
	if (!(_toolbarHandler.visible() && _mouseCurState.y >= _toolbarHandler.getOffset()) && !_forceHideMouse)
		OSystem_SDL_Common::draw_mouse();
}

void OSystem_WINCE3::fillMouseEvent(Event &event, int x, int y) {
	event.mouse.x = x;
	event.mouse.y = y;
	
	// Update the "keyboard mouse" coords
	km.x = event.mouse.x;
	km.y = event.mouse.y;

	// Adjust for the screen scaling
	event.mouse.x = event.mouse.x * _scaleFactorXd / _scaleFactorXm;
	event.mouse.y = event.mouse.y * _scaleFactorYd / _scaleFactorYm;
}

void OSystem_WINCE3::warp_mouse(int x, int y) {
		if (_mouseCurState.x != x || _mouseCurState.y != y) {
		SDL_WarpMouse(x * _scaleFactorXm / _scaleFactorXd, y * _scaleFactorYm / _scaleFactorYd);

		// SDL_WarpMouse() generates a mouse movement event, so
		// set_mouse_pos() would be called eventually. However, the
		// cannon script in CoMI calls this function twice each time
		// the cannon is reloaded. Unless we update the mouse position
		// immediately the second call is ignored, causing the cannon
		// to change its aim.

		set_mouse_pos(x, y);
	}
}

void OSystem_WINCE3::add_dirty_rect(int x, int y, int w, int h) {
	if (_scaler_proc == PocketPCPortrait) {
		// Align on a 4 bytes boundary for the Portrait mode
		if (x != 0) {
			while (x % 4) {
				x--;
				w++;
			}
		}
		while (w % 4) w++;
	}
	else
	if (_scaler_proc == PocketPCHalf) {
		// Align on a 2x2 square
		if (x != 0) {
			while (x % 2) {
				x--;
				w++;
			}
			while (y % 2) {
				y--;
				h++;
			}
			while (w % 2) w++;
			while (h % 2) h++;
		}
	}

	OSystem_SDL_Common::add_dirty_rect(x, y, w, h);
}

// FIXME
// Remove useless mappings

bool OSystem_WINCE3::poll_event(Event *event) {
	SDL_Event ev;
	byte b = 0;
	Event temp_event;

	// Check if the keys queue is empty
	Key *key = _keysBuffer->Instance()->get();
	if (key) {
		event->kbd.flags = key->flags();
		event->kbd.ascii = key->ascii();
		event->kbd.keycode = key->keycode();
		if (key->pushed())
			event->event_code = EVENT_KEYDOWN;
		else
			event->event_code = EVENT_KEYUP;
		return true;
	}
	
	// Check if a right click event must be generated
	if (_addRightClickDown || _addRightClickUp) {
		event->event_code = (_addRightClickDown ? EVENT_RBUTTONDOWN : EVENT_RBUTTONUP);
		event->mouse.x = _mouseCurState.x;
		event->mouse.y = _mouseCurState.y;

		if (_addRightClickDown) {
			_addRightClickDown = false;
			_addRightClickUp = true;
		}
		else {
			_addRightClickUp = false;
		}	
		
		return true;
	}

	kbd_mouse();
	
	// If the screen mode changed, send an EVENT_SCREEN_CHANGED
	if (_modeChanged) {
		_modeChanged = false;
		event->event_code = EVENT_SCREEN_CHANGED;
		return true;
	}

	while(SDL_PollEvent(&ev)) {
		switch(ev.type) {
		case SDL_KEYDOWN:

			if (ev.key.keysym.mod & KMOD_SHIFT)
				b |= KBD_SHIFT;
			if (ev.key.keysym.mod & KMOD_CTRL)
				b |= KBD_CTRL;
			if (ev.key.keysym.mod & KMOD_ALT)
				b |= KBD_ALT;
			event->kbd.flags = b;

			// Alt-Return toggles full screen mode				
			if (b == KBD_ALT && ev.key.keysym.sym == SDLK_RETURN) {
				property(PROP_TOGGLE_FULLSCREEN, NULL);
				break;
			}

			if (b == KBD_ALT && ev.key.keysym.sym == 's') {
				char filename[20];

				for (int n = 0;; n++) {
					SDL_RWops *file;

					sprintf(filename, "scummvm%05d.bmp", n);
					file = SDL_RWFromFile(filename, "r");
					if (!file)
						break;
					SDL_RWclose(file);
				}
				if (save_screenshot(filename))
					printf("Saved '%s'\n", filename);
				else
					printf("Could not save screenshot!\n");
				break;
			}

			// Ctrl-m toggles mouse capture
			if (b == KBD_CTRL && ev.key.keysym.sym == 'm') {
				property(PROP_TOGGLE_MOUSE_GRAB, NULL);
				break;
			}

			// Ctrl-z and Alt-X quit
			if ((b == KBD_CTRL && ev.key.keysym.sym == 'z') || (b == KBD_ALT && ev.key.keysym.sym == 'x')) {
				event->event_code = EVENT_QUIT;
				return true;
			}

			// Ctrl-Alt-<key> will change the GFX mode
			if ((b & (KBD_CTRL|KBD_ALT)) == (KBD_CTRL|KBD_ALT)) {
				static const int gfxModes[][4] = {
						{ GFX_NORMAL, GFX_DOUBLESIZE, GFX_TRIPLESIZE, -1 },
						{ GFX_NORMAL, GFX_ADVMAME2X, GFX_ADVMAME3X, -1 },
						{ GFX_NORMAL, GFX_HQ2X, GFX_HQ3X, -1 },
						{ GFX_NORMAL, GFX_2XSAI, -1, -1 },
						{ GFX_NORMAL, GFX_SUPER2XSAI, -1, -1 },
						{ GFX_NORMAL, GFX_SUPEREAGLE, -1, -1 },
						{ GFX_NORMAL, GFX_TV2X, -1, -1 },
						{ GFX_NORMAL, GFX_DOTMATRIX, -1, -1 }
					};

				// FIXME EVIL HACK: This shouldn't be a static int, rather it
				// should be a member variable. Furthermore, it shouldn't be
				// set in this code, rather it should be set by load_gfx_mode().
				// But for now this quick&dirty hack works.
				static int _scalerType = 0;
				if (_mode != GFX_NORMAL) {
					// Try to figure out which gfx mode "group" we are in
					// This is just a temporary hack until the proper solution
					// (i.e. code in load_gfx_mode()) is in effect.
					for (int i = 0; i < ARRAYSIZE(gfxModes); i++) {
						if (gfxModes[i][1] == _mode || gfxModes[i][2] == _mode) {
							_scalerType = i;
							break;
						}
					}
				}
				

				Property prop;
				int factor = _scaleFactor - 1;

				// Ctrl-Alt-a toggles aspect ratio correction
				if (ev.key.keysym.sym == 'a') {
					property(PROP_TOGGLE_ASPECT_RATIO, NULL);
					break;
				}

				// Increase/decrease the scale factor
				// TODO: Shall we 'wrap around' here?
				if (ev.key.keysym.sym == '=' || ev.key.keysym.sym == '+' || ev.key.keysym.sym == '-') {
					factor += (ev.key.keysym.sym == '-' ? -1 : +1);
					if (0 <= factor && factor < 4 && gfxModes[_scalerType][factor] >= 0) {
						prop.gfx_mode = gfxModes[_scalerType][factor];
						property(PROP_SET_GFX_MODE, &prop);
					}
					break;
				}
				
				if ('1' <= ev.key.keysym.sym && ev.key.keysym.sym <= '9') {
					_scalerType = ev.key.keysym.sym - '1';
					if (_scalerType >= ARRAYSIZE(gfxModes))
						break;
					
					while (gfxModes[_scalerType][factor] < 0) {
						assert(factor > 0);
						factor--;
					}
					prop.gfx_mode = gfxModes[_scalerType][factor];
					property(PROP_SET_GFX_MODE, &prop);
					break;
				}
			}

			// Check mapping
			if (CEActions::Instance()->mappingActive()) {
				event->event_code = EVENT_KEYDOWN;
				event->kbd.ascii = (ev.key.keysym.sym ? ev.key.keysym.sym : ev.key.keysym.unicode);
				event->kbd.flags = 0xff;
				return true;
			}
			else
			if (CEActions::Instance()->performMapped((ev.key.keysym.sym ? ev.key.keysym.sym : ev.key.keysym.unicode), true))
				return true;

			event->event_code = EVENT_KEYDOWN;
			event->kbd.keycode = ev.key.keysym.sym;
			event->kbd.ascii = mapKeyCE(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode);
			
			switch(ev.key.keysym.sym) {
			case SDLK_LEFT:
				km.x_vel = -1;
				km.x_down_count = 1;
				break;
			case SDLK_RIGHT:
				km.x_vel =  1;
				km.x_down_count = 1;
				break;
			case SDLK_UP:
				km.y_vel = -1;
				km.y_down_count = 1;
				break;
			case SDLK_DOWN:
				km.y_vel =  1;
				km.y_down_count = 1;
				break;
			default:
				break;
			}

			return true;
	
		case SDL_KEYUP:
			// Check mapping
			if (CEActions::Instance()->mappingActive()) {
				event->event_code = EVENT_KEYUP;
				event->kbd.ascii = (ev.key.keysym.sym ? ev.key.keysym.sym : ev.key.keysym.unicode);
				event->kbd.flags = 0xff;
				return true;
			}
			else
			if (CEActions::Instance()->performMapped((ev.key.keysym.sym ? ev.key.keysym.sym : ev.key.keysym.unicode), false))
				return true;
			
			event->event_code = EVENT_KEYUP;
			event->kbd.keycode = ev.key.keysym.sym;
			event->kbd.ascii = mapKeyCE(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode);

			switch(ev.key.keysym.sym) {
			case SDLK_LEFT:
				if (km.x_vel < 0) {
					km.x_vel = 0;
					km.x_down_count = 0;
				}
				break;
			case SDLK_RIGHT:
				if (km.x_vel > 0) {
					km.x_vel = 0;
					km.x_down_count = 0;
				}
				break;
			case SDLK_UP:
				if (km.y_vel < 0) {
					km.y_vel = 0;
					km.y_down_count = 0;
				}
				break;
			case SDLK_DOWN:
				if (km.y_vel > 0) {
					km.y_vel = 0;
					km.y_down_count = 0;
				}
				break;
			default:
				break;
			}
			return true;

		case SDL_MOUSEMOTION:
			event->event_code = EVENT_MOUSEMOVE;
			fillMouseEvent(*event, ev.motion.x, ev.motion.y);
			set_mouse_pos(event->mouse.x, event->mouse.y);
			return true;

		case SDL_MOUSEBUTTONDOWN:
			if (ev.button.button == SDL_BUTTON_LEFT)
				temp_event.event_code = EVENT_LBUTTONDOWN;
			else if (ev.button.button == SDL_BUTTON_RIGHT)
				temp_event.event_code = EVENT_RBUTTONDOWN;
			else
				break;

			fillMouseEvent(temp_event, ev.button.x, ev.button.y);

			if (_toolbarHandler.action(temp_event.mouse.x, temp_event.mouse.y, true)) {
				if (!_toolbarHandler.drawn())
					update_screen();
				if (_newOrientation != _orientationLandscape) {
					_orientationLandscape = _newOrientation;
					ConfMan.set("CE_landscape", _orientationLandscape);
					ConfMan.flushToDisk();
					hotswap_gfx_mode();
				}
			}
			else {
				if (!_freeLook)
					memcpy(event, &temp_event, sizeof(Event));
			}

			return true;

		case SDL_MOUSEBUTTONUP:
			if (ev.button.button == SDL_BUTTON_LEFT)
				temp_event.event_code = EVENT_LBUTTONUP;
			else if (ev.button.button == SDL_BUTTON_RIGHT)
				temp_event.event_code = EVENT_RBUTTONUP;
			else
				break;

			fillMouseEvent(temp_event, ev.button.x, ev.button.y);

			if (_toolbarHandler.action(temp_event.mouse.x, temp_event.mouse.y, false)) {
				if (!_toolbarHandler.drawn())
					update_screen();
			}
			else {
				if (!_freeLook)
					memcpy(event, &temp_event, sizeof(Event));
			}

			return true;

		case SDL_VIDEOEXPOSE:
			_forceFull = true;
			break;

		case SDL_QUIT:
			event->event_code = EVENT_QUIT;
			CEDevice::disableHardwareKeyMapping();
			return true;
		}
	}
	return false;
}

void OSystem_WINCE3::quit() {
	fclose(stdout_file);
	fclose(stderr_file);
	DeleteFile(TEXT("\\scummvm_stdout.txt"));
	DeleteFile(TEXT("\\scummvm_stderr.txt"));
	CEDevice::disableHardwareKeyMapping();
	OSystem_SDL_Common::quit();
}