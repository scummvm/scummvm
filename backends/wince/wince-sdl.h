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

#ifndef WINCE_SDL_H
#define WINCE_SDL_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "common/scaler.h"
#include "backends/intern.h"
#include "backends/sdl/sdl-common.h"

#include "CEgui.h"
#include "CEkeys.h"
#include "CEDevice.h"
#include "CEScaler.h"

#include <SDL.h>



#define TOTAL_ZONES 3

class OSystem_WINCE3 : public OSystem_SDL {
public:
	OSystem_WINCE3();

	// Update the dirty areas of the screen
	void internUpdateScreen();

	void initSize(uint w, uint h);

	// Overloaded from SDL_Common (toolbar handling)
	bool pollEvent(Event &event);
	// Overloaded from SDL_Common (toolbar handling)
	void drawMouse();
	// Overloaded from SDL_Common (mouse and new scaler handling)
	void fillMouseEvent(Event &event, int x, int y);
	// Overloaded from SDL_Common (new scaler handling)
	void addDirtyRect(int x, int y, int w, int h, bool mouseRect = false);
	// Overloaded from SDL_Common (new scaler handling)
	void warpMouse(int x, int y);
	// Overloaded from SDL_Commmon
	void quit();
	// Overloaded from SDL_Commmon (master volume and sample rate subtleties)
	bool setSoundCallback(SoundProc proc, void *param);

	// Overloaded from SDL_Common (FIXME)
	void setMouseCursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y, byte keycolor, int cursorTargetScale); // overloaded by CE backend
	void undrawMouse();
	void blitCursor();
	void setMousePos(int x, int y);
    void copyRectToScreen(const byte *src, int pitch, int x, int y, int w, int h); // overloaded by CE backend (FIXME)
	void copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h);
	void showOverlay();
	void hideOverlay();

	// GUI and action stuff
	void swap_panel_visibility();
	void swap_panel();
	void swap_sound_master();
	void add_right_click(bool pushed);
	void swap_mouse_visibility();
	void swap_freeLook();
	void swap_zoom_up();
	void swap_zoom_down();

//#ifdef WIN32_PLATFORM_WFSP
	// Smartphone actions

	void initZones();
	void loadSmartphoneConfigurationElement(String element, int &value, int defaultValue);
	void loadSmartphoneConfiguration();
	void add_left_click(bool pushed);
	void move_cursor_up();
	void move_cursor_down();
	void move_cursor_left();
	void move_cursor_right();
	void switch_zone();
//#endif

	static int getScreenWidth();
	static int getScreenHeight();
	static void initScreenInfos();
	static bool isOzone();

protected:
	void loadGFXMode();
	void unloadGFXMode();
	void hotswapGFXMode();
	bool saveScreenshot(const char *filename);


	const GraphicsMode *getSupportedGraphicsModes() const;
	bool setGraphicsMode(int mode);
	//int getGraphicsMode() const;
	int getDefaultGraphicsMode() const;

	void setWindowCaption(const char *caption);
	bool openCD(int drive);
	int getOutputSampleRate() const;

	bool hasFeature(Feature f);
	void setFeatureState(Feature f, bool enable);
	bool getFeatureState(Feature f);

	void internDrawMouse();

private:

#ifdef USE_VORBIS
	bool checkOggHighSampleRate();
#endif

	static void private_sound_proc(void *param, byte *buf, int len);
	static SoundProc _originalSoundProc;

	bool update_scalers();
	void create_toolbar();
	void update_game_settings();
	void check_mappings();
	void update_keyboard();
	void get_sample_rate();

	void retrieve_mouse_location(int &x, int &y);

	CEGUI::ToolbarHandler _toolbarHandler;

	SDL_Surface *_toolbarLow;	// toolbar 320x40
	SDL_Surface *_toolbarHigh;  // toolbar 640x80
	bool _toolbarHighDrawn;		// cache toolbar 640x80

	uint16 _sampleRate;			// current audio sample rate

	bool _freeLook;				// freeLook mode (do not send mouse button events)

	bool _forceHideMouse;		// force invisible mouse cursor

	bool _forcePanelInvisible;  // force panel visibility for some cases
	bool _panelVisible;			// panel visibility
	bool _panelStateForced;		// panel visibility forced by external call

	bool _panelInitialized;		// only initialize the toolbar once

	bool _monkeyKeyboard;		// forced keyboard for Monkey Island copy protection
	static bool _soundMaster;	// turn off sound after all calculations
								// static since needed by the SDL callback
	bool _orientationLandscape; // current orientation
	bool _newOrientation;		// new orientation

	bool _saveToolbarState;		// save visibility when forced
	String _saveActiveToolbar;	// save active toolbar when forced

	bool _saveToolbarZoom;		// save visibility when zooming
	bool _zoomUp;				// zooming up mode
	bool _zoomDown;				// zooming down mode

	int _scaleFactorXm;			// scaler X *
	int _scaleFactorXd;			// scaler X /
	int _scaleFactorYm;			// scaler Y *
	int _scaleFactorYd;			// scaler Y /
	bool _scalersChanged;

	static int _platformScreenWidth;
	static int _platformScreenHeight;
	static bool _isOzone;		// true if running on Windows 2003 SE

	// Keyboard tap
	int _tapX;
	int _tapY;
	long _tapTime;

	// Mouse

	byte *_mouseBackupOld;

	// Smartphone specific variables

	int _lastKeyPressed;		// last key pressed
	int _keyRepeat;				// number of time the last key was repeated
	int _keyRepeatTime;			// elapsed time since the key was pressed
	int _keyRepeatTrigger;		// minimum time to consider the key was repeated

	int _repeatX;				// repeat trigger for left and right cursor moves
	int _repeatY;				// repeat trigger for up and down cursor moves
	int _stepX1;				// offset for left and right cursor moves (slowest)
	int _stepX2;				// offset for left and right cursor moves (faster)
	int _stepX3;				// offset for left and right cursor moves (fastest)
	int _stepY1;				// offset for up and down cursor moves (slowest)
	int _stepY2;				// offset for up and down cursor moves (faster)
	int _stepY3;				// offset for up and down cursor moves (fastest)

	int _mouseXZone[TOTAL_ZONES];
	int _mouseYZone[TOTAL_ZONES];
	int _currentZone;

	typedef struct zoneDesc {
		int x;
		int y;
		int width;
		int height;
	} zoneDesc;

	static zoneDesc _zones[TOTAL_ZONES];
};

#endif
