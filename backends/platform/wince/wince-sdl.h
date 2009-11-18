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

#ifndef WINCE_SDL_H
#define WINCE_SDL_H

#include "common/scummsys.h"
#include "common/system.h"
#include "graphics/scaler.h"
#include "backends/platform/sdl/sdl.h"

#include "backends/platform/wince/CEgui/CEGUI.h"
#include "backends/platform/wince/CEkeys/CEKeys.h"
#include "backends/platform/wince/CEDevice.h"

#define TOTAL_ZONES 3

// defines used for implementing the raw frame buffer access method (2003+)
#define GETRAWFRAMEBUFFER   0x00020001
#define FORMAT_565 1
#define FORMAT_555 2
#define FORMAT_OTHER 3

class OSystem_WINCE3 : public OSystem_SDL {
public:
	OSystem_WINCE3();

	// Update the dirty areas of the screen
	void internUpdateScreen();

	void setGraphicsModeIntern();
	void initSize(uint w, uint h, const Graphics::PixelFormat *format);
	void initBackend();

	// Overloaded from SDL backend (toolbar handling)
	bool pollEvent(Common::Event &event);
	// Overloaded from SDL backend (toolbar handling)
	void drawMouse();
	// Overloaded from SDL backend (mouse and new scaler handling)
	void fillMouseEvent(Common::Event &event, int x, int y);
	// Overloaded from SDL backend (new scaler handling)
	void addDirtyRect(int x, int y, int w, int h, bool mouseRect = false);
	// Overloaded from SDL backend (new scaler handling)
	void warpMouse(int x, int y);
	// Overloaded from SDL backend
	void quit();
	// Overloaded from SDL backend (master volume and sample rate subtleties)
	void setupMixer();
	// Overloaded from OSystem
	void engineInit();
	void getTimeAndDate(TimeDate &t) const;
	virtual Common::SeekableReadStream *createConfigReadStream();
	virtual Common::WriteStream *createConfigWriteStream();


	// Overloaded from SDL_Common (FIXME)
	void setMouseCursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y, uint32 keycolor, int cursorTargetScale, const Graphics::PixelFormat *format); // overloaded by CE backend
	void undrawMouse();
	void blitCursor();
	bool showMouse(bool visible);
	void setMousePos(int x, int y);
	void copyRectToScreen(const byte *src, int pitch, int x, int y, int w, int h); // overloaded by CE backend (FIXME)
	void copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h);
	void showOverlay();
	void hideOverlay();
	Graphics::Surface *lockScreen();
	void unlockScreen();

	// GUI and action stuff
	void swap_panel_visibility();
	void swap_panel();
	void swap_sound_master();
	void add_right_click(bool pushed);
	void swap_mouse_visibility();
	void swap_freeLook();
	void swap_zoom_up();
	void swap_zoom_down();
	void swap_smartphone_keyboard();

//#ifdef WIN32_PLATFORM_WFSP
	// Smartphone actions

	void initZones();
	void loadDeviceConfigurationElement(String element, int &value, int defaultValue);
	void loadDeviceConfiguration();
	void add_left_click(bool pushed);
	void move_cursor_up();
	void move_cursor_down();
	void move_cursor_left();
	void move_cursor_right();
	void switch_zone();
	void smartphone_rotate_display();
//#endif

	static int getScreenWidth();
	static int getScreenHeight();
	static void initScreenInfos();
	static bool isOzone();

protected:
	bool loadGFXMode();
	void unloadGFXMode();
	bool hotswapGFXMode();
	bool saveScreenshot(const char *filename);


	const GraphicsMode *getSupportedGraphicsModes() const;
	bool setGraphicsMode(int mode);
	//int getGraphicsMode() const;
	int getDefaultGraphicsMode() const;

	bool openCD(int drive);
	int getOutputSampleRate() const;

	bool hasFeature(Feature f);
	void setFeatureState(Feature f, bool enable);
	bool getFeatureState(Feature f);

	void internDrawMouse();
	void drawToolbarMouse(SDL_Surface *surf, bool draw);

private:

#ifdef USE_VORBIS
	bool checkOggHighSampleRate();
#endif

	static void private_sound_proc(void *param, byte *buf, int len);

	bool update_scalers();
	void create_toolbar();
	void update_game_settings();
	void check_mappings();
	void compute_sample_rate();

	void retrieve_mouse_location(int &x, int &y);

	CEGUI::ToolbarHandler _toolbarHandler;

	SDL_Surface *_toolbarLow;	// toolbar 320x40
	SDL_Surface *_toolbarHigh;	// toolbar 640x80
	bool _toolbarHighDrawn;		// cache toolbar 640x80

	uint16 _sampleRate;		// current audio sample rate

	bool _freeLook;			// freeLook mode (do not send mouse button events)

	bool _forceHideMouse;		// force invisible mouse cursor

	bool _forcePanelInvisible;	// force panel visibility for some cases
	bool _panelVisible;		// panel visibility
	bool _panelStateForced;		// panel visibility forced by external call

	bool _panelInitialized;		// only initialize the toolbar once

	bool _unfilteredkeys;		// discard key mapping temporarily (agi pred. dialog)
	static bool _soundMaster;	// turn off sound after all calculations
								// static since needed by the SDL callback
	int _orientationLandscape;	// current orientation
	int _newOrientation;		// new orientation

	bool _saveToolbarState;		// save visibility when forced
	String _saveActiveToolbar;	// save active toolbar when forced

	bool _saveToolbarZoom;		// save visibility when zooming
	bool _zoomUp;			// zooming up mode
	bool _zoomDown;			// zooming down mode

	bool _noDoubleTapRMB;	// disable double tap -> rmb click
	bool _rbutton;			// double tap -> right button simulation
	bool _closeClick;		// flag when taps are spatially close together

	bool _usesEmulatedMouse;	// emulated mousemove ever been used in this session

	bool _canBeAspectScaled;	// game screen size allows for aspect scaling

	int _scaleFactorXm;		// scaler X *
	int _scaleFactorXd;		// scaler X /
	int _scaleFactorYm;		// scaler Y *
	int _scaleFactorYd;		// scaler Y /
	SDL_Rect _dirtyRectOut[NUM_DIRTY_RECT];
	bool _scalersChanged;
	bool _hasfocus;			// scummvm has the top window

	static int _platformScreenWidth;
	static int _platformScreenHeight;
	static bool _isOzone;		// true if running on Windows 2003 SE

	// Keyboard tap
	int _tapX;
	int _tapY;
	long _tapTime;

	// Mouse
	int	_mouseHotspotX, _mouseHotspotY;
	byte *_mouseBackupOld;
	uint16 *_mouseBackupToolbar;
	uint16 _mouseBackupDim;

	// Smartphone specific variables

	int _lastKeyPressed;			// last key pressed
	int _keyRepeat;				// number of time the last key was repeated
	int _keyRepeatTime;			// elapsed time since the key was pressed
	int _keyRepeatTrigger;			// minimum time to consider the key was repeated

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

	struct zoneDesc {
		int x;
		int y;
		int width;
		int height;
	};

	static zoneDesc _zones[TOTAL_ZONES];
};

#endif
