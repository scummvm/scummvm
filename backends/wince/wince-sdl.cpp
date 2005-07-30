/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2005 The ScummVM project
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

#include "gui/Actions.h"
#include "CEActionsPocket.h"
#include "CEActionsSmartphone.h"
#include "ItemAction.h"
#include "gui/KeysDialog.h"

#include "gui/message.h"

#include "sound/fmopl.h"

#include "ozone.h"
#include "CEException.h"

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

bool _isSmartphone = false;

// Graphics mode consts

// Low end devices 240x320

static const OSystem::GraphicsMode s_supportedGraphicsModesLow[] = {
	{"1x", "Normal (no scaling)", GFX_NORMAL},
	{0, 0, 0}
};

// High end device 480x640

static const OSystem::GraphicsMode s_supportedGraphicsModesHigh[] = {
	{"1x", "Normal (no scaling)", GFX_NORMAL},
	{"2x", "2x", GFX_DOUBLESIZE},
#ifndef _MSC_VER // EVC breaks template functions, and I'm tired of fixing them :)
	{"2xsai", "2xSAI", GFX_2XSAI},
	{"super2xsai", "Super2xSAI", GFX_SUPER2XSAI},
	{"supereagle", "SuperEagle", GFX_SUPEREAGLE},
#endif
	{"advmame2x", "AdvMAME2x", GFX_ADVMAME2X},
#ifndef _MSC_VER
	{"hq2x", "HQ2x", GFX_HQ2X},
	{"tv2x", "TV2x", GFX_TV2X},
#endif
	{"dotmatrix", "DotMatrix", GFX_DOTMATRIX},
	{0, 0, 0}
};


// ********************************************************************************************

bool isSmartphone() {
	return _isSmartphone;
}

// ********************************************************************************************

// MAIN

extern "C" int scummvm_main(GameDetector &gameDetector, int argc, char **argv);

int handleException(EXCEPTION_POINTERS *exceptionPointers) {
	CEException::writeException(TEXT("\\scummvmCrash"), exceptionPointers);
	drawError("Unrecoverable exception occurred - see crash dump in latest \\scummvmCrash file");
	fclose(stdout_file);
	fclose(stderr_file);
	CEDevice::end();
	SDL_Quit();
	exit(0);
	return EXCEPTION_EXECUTE_HANDLER;
}

int SDL_main(int argc, char **argv) {
	CEDevice::init();
	OSystem_WINCE3::initScreenInfos();
	/* Sanity check */
//#ifndef WIN32_PLATFORM_WFSP
//	if (CEDevice::hasSmartphoneResolution()) {
//		MessageBox(NULL, TEXT("This build was not compiled with Smartphone support"), TEXT("ScummVM error"), MB_OK | MB_ICONERROR);
//		return 0;
//	}
//#endif
	/* Avoid print problems - this file will be put in RAM anyway */
	stdout_file = fopen("\\scummvm_stdout.txt", "w");
	stderr_file = fopen("\\scummvm_stderr.txt", "w");
	GUI::Actions::init(_gameDetector);

	__try {
		return scummvm_main(_gameDetector, argc, argv);
	}
	__except (handleException(GetExceptionInformation())) {
	}

	return 0;
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

// ********************************************************************************************

int OSystem_WINCE3::getScreenWidth() {
	return _platformScreenWidth;
}

int OSystem_WINCE3::getScreenHeight() {
	return _platformScreenHeight;
}

void OSystem_WINCE3::initScreenInfos() {
	// Check if we're running Ozone
	int result;
	RawFrameBufferInfo frameBufferInfo;
	HDC hdc = GetDC(NULL);
	result = ExtEscape(hdc, GETRAWFRAMEBUFFER, 0, NULL, sizeof(RawFrameBufferInfo), (char *)&frameBufferInfo);
	ReleaseDC(NULL, hdc);
	_isOzone = (result > 0);
	// And obtain the real screen size
	_platformScreenWidth = (result > 0 ? frameBufferInfo.cxPixels : GetSystemMetrics(SM_CXSCREEN));
	_platformScreenHeight = (result > 0 ? frameBufferInfo.cyPixels : GetSystemMetrics(SM_CYSCREEN));
}

bool OSystem_WINCE3::isOzone() {
	return _isOzone;
}

// ********************************************************************************************


OSystem *OSystem_WINCE3_create() {
	return new OSystem_WINCE3();
}

OSystem_WINCE3::OSystem_WINCE3() : OSystem_SDL(),
	_orientationLandscape(false), _newOrientation(false), _panelInitialized(false),
	_panelVisible(false), _panelStateForced(false), _forceHideMouse(false),
	_freeLook(false), _forcePanelInvisible(false), _toolbarHighDrawn(false), _zoomUp(false), _zoomDown(false),
	_scalersChanged(false), _monkeyKeyboard(false), _lastKeyPressed(0)
{
	_isSmartphone = CEDevice::hasSmartphoneResolution();
	memset(&_mouseCurState, 0, sizeof(_mouseCurState));
	if (_isSmartphone) {
		_mouseCurState.x = 20;
		_mouseCurState.y = 20;
	}
	create_toolbar();
	// Initialize global key mapping for Smartphones
	GUI_Actions::Instance()->initInstanceMain(this);
	GUI_Actions::Instance()->loadMapping();

	if (_isSmartphone) {
		loadSmartphoneConfiguration();
	}


}

void OSystem_WINCE3::swap_panel_visibility() {
	if (!_forcePanelInvisible && !_panelStateForced) {
		_panelVisible = !_panelVisible;
		_toolbarHandler.setVisible(_panelVisible);
		if (_screenHeight > 240)
			addDirtyRect(0, 400, 640, 80);
		else
			addDirtyRect(0, 200, 320, 40);

		internUpdateScreen();
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

void OSystem_WINCE3::add_right_click(bool pushed) {
	int x, y;
	retrieve_mouse_location(x, y);
	EventsBuffer::simulateMouseRightClick(x, y, pushed);
}

void OSystem_WINCE3::swap_mouse_visibility() {
	_forceHideMouse = !_forceHideMouse;
	if (_forceHideMouse)
		undrawMouse();
}

void OSystem_WINCE3::swap_freeLook() {
	_freeLook = !_freeLook;
}

void OSystem_WINCE3::swap_zoom_up() {
	if (_zoomUp) {
		// restore visibility
		_toolbarHandler.setVisible(_saveToolbarZoom);
		// restore scaler
		_scaleFactorYd = 2;
		_scalerProc = PocketPCHalf;
		_zoomUp = false;
	}
	else
	{
		// only active if running on a PocketPC
		if (_scalerProc != PocketPCHalf && _scalerProc != PocketPCHalfZoom)
			return;
		if (_scalerProc == PocketPCHalf) {
			_saveToolbarZoom = _toolbarHandler.visible();
			_toolbarHandler.setVisible(false);
			// set zoom scaler
			_scaleFactorYd = 1;
			_scalerProc = PocketPCHalfZoom;
		}
		else
			_zoomDown = false;

		_zoomUp = true;
	}
	// redraw whole screen
	addDirtyRect(0, 0, 640, 480);
	internUpdateScreen();
}

void OSystem_WINCE3::swap_zoom_down() {
	if (_zoomDown) {
		// restore visibility
		_toolbarHandler.setVisible(_saveToolbarZoom);
		// restore scaler
		_scaleFactorYd = 2;
		_scalerProc = PocketPCHalf;
		_zoomDown = false;
	}
	else
	{
		// only active if running on a PocketPC
		if (_scalerProc != PocketPCHalf && _scalerProc != PocketPCHalfZoom)
			return;
		if (_scalerProc == PocketPCHalf) {
			_saveToolbarZoom = _toolbarHandler.visible();
			_toolbarHandler.setVisible(false);
			// set zoom scaler
			_scaleFactorYd = 1;
			_scalerProc = PocketPCHalfZoom;
		}
		else
			_zoomUp = false;

		_zoomDown = true;
	}
	// redraw whole screen
	addDirtyRect(0, 0, 640, 480);
	internUpdateScreen();
}

//#ifdef WIN32_PLATFORM_WFSP
// Smartphone actions

void OSystem_WINCE3::initZones() {
        int i;

		_currentZone = 0;
        for (i=0; i<TOTAL_ZONES; i++) {
                _mouseXZone[i] = (_zones[i].x + (_zones[i].width / 2)) * _scaleFactorXm / _scaleFactorXd;
                _mouseYZone[i] = (_zones[i].y + (_zones[i].height / 2)) * _scaleFactorYm / _scaleFactorYd;
        }
}

void OSystem_WINCE3::loadSmartphoneConfigurationElement(String element, int &value, int defaultValue) {
	value = ConfMan.getInt(element, "smartphone");
	if (!value) {
		value = defaultValue;
		ConfMan.set(element, value, "smartphone");
	}
}

void OSystem_WINCE3::loadSmartphoneConfiguration() {
	loadSmartphoneConfigurationElement("repeatTrigger", _keyRepeatTrigger, 200);
	loadSmartphoneConfigurationElement("repeatX", _repeatX, 4);
	loadSmartphoneConfigurationElement("repeatY", _repeatY, 4);
	loadSmartphoneConfigurationElement("stepX1", _stepX1, 2);
	loadSmartphoneConfigurationElement("stepX2", _stepX2, 10);
	loadSmartphoneConfigurationElement("stepX3", _stepX3, 40);
	loadSmartphoneConfigurationElement("stepY1", _stepY1, 2);
	loadSmartphoneConfigurationElement("stepY2", _stepY2, 10);
	loadSmartphoneConfigurationElement("stepY3", _stepY3, 20);
	ConfMan.flushToDisk();
}

void OSystem_WINCE3::add_left_click(bool pushed) {
	int x, y;
	retrieve_mouse_location(x, y);
	EventsBuffer::simulateMouseLeftClick(x, y, pushed);
}

void OSystem_WINCE3::move_cursor_up() {
	int x,y;
	retrieve_mouse_location(x, y);
	if (_keyRepeat > _repeatY)
		y -= _stepY3;
	else
	if (_keyRepeat)
		y -= _stepY2;
	else
		y -= _stepY1;

	if (y < 0)
		y = 0;

	EventsBuffer::simulateMouseMove(x, y);
}

void OSystem_WINCE3::move_cursor_down() {
	int x,y;
	retrieve_mouse_location(x, y);
	if (_keyRepeat > _repeatY)
		y += _stepY3;
	else
	if (_keyRepeat)
		y += _stepY2;
	else
		y += _stepY1;

	if (y > 200)
		y = 200;

	EventsBuffer::simulateMouseMove(x, y);
}

void OSystem_WINCE3::move_cursor_left() {
	int x,y;
	retrieve_mouse_location(x, y);
	if (_keyRepeat > _repeatX)
		x -= _stepX3;
	else
	if (_keyRepeat)
		x -= _stepX2;
	else
		x -= _stepX1;

	if (x < 0)
		x = 0;

	EventsBuffer::simulateMouseMove(x, y);
}

void OSystem_WINCE3::move_cursor_right() {
	int x,y;
	retrieve_mouse_location(x, y);
	if (_keyRepeat > _repeatX)
		x += _stepX3;
	else
	if (_keyRepeat)
		x += _stepX2;
	else
		x += _stepX1;

	if (x > 320)
		x = 320;

	EventsBuffer::simulateMouseMove(x, y);
}

void OSystem_WINCE3::switch_zone() {
	int x,y;
	int i;
	retrieve_mouse_location(x, y);

    for (i=0; i<TOTAL_ZONES; i++)
		if (x >= _zones[i].x && y >= _zones[i].y &&
			x <= _zones[i].x + _zones[i].width && y <= _zones[i].y + _zones[i].height
		   ) {
				_mouseXZone[i] = x;
				_mouseYZone[i] = y;
				break;
		}
	_currentZone++;
	if (_currentZone >= TOTAL_ZONES)
		_currentZone = 0;

	EventsBuffer::simulateMouseMove(_mouseXZone[_currentZone], _mouseYZone[_currentZone]);
}
//#endif


void OSystem_WINCE3::create_toolbar() {
	PanelKeyboard *keyboard;

	// Add the keyboard
	if (!_isSmartphone) {
		keyboard = new PanelKeyboard(PANEL_KEYBOARD);
		_toolbarHandler.add(NAME_PANEL_KEYBOARD, *keyboard);
	}

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
						delete test_ov_file;
                        return highSampleRate;
                }
        }

        // Do not test for OGG samples - too big and too slow anyway :)

		delete test_ov_file;
        return false;
}
#endif

void OSystem_WINCE3::get_sample_rate() {
	// Force at least medium quality FM synthesis for FOTAQ
	if (_gameDetector._targetName == "queen") {
		if (!((ConfMan.hasKey("FM_high_quality") && ConfMan.getBool("FM_high_quality")) ||
			(ConfMan.hasKey("FM_medium_quality") && ConfMan.getBool("FM_medium_quality")))) {
			ConfMan.set("FM_medium_quality", true);
			ConfMan.flushToDisk();
		}
	}
	// See if the output frequency is forced by the game
	if ((_gameDetector._game.features & Scumm::GF_DIGI_IMUSE) ||
		_gameDetector._targetName == "queen" ||
		strncmp(_gameDetector._targetName.c_str(), "sword", 5) == 0 ||
		strncmp(_gameDetector._targetName.c_str(), "sky", 3) == 0)
			_sampleRate = SAMPLES_PER_SEC_NEW;
	else {
		if (ConfMan.hasKey("high_sample_rate") && ConfMan.getBool("high_sample_rate"))
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

int OSystem_WINCE3::getOutputSampleRate() const {
	return _sampleRate;
}

void OSystem_WINCE3::setWindowCaption(const char *caption) {
	check_mappings(); // called here to initialize virtual keys handling
	//update_game_settings();
	get_sample_rate(); // called here to initialize mixer
}

bool OSystem_WINCE3::openCD(int drive) {
	return false;
}

const OSystem::GraphicsMode *OSystem_WINCE3::getSupportedGraphicsModes() const {
	if (CEDevice::hasWideResolution())
		return s_supportedGraphicsModesHigh;
	else
		return s_supportedGraphicsModesLow;
}

bool OSystem_WINCE3::hasFeature(Feature f) {
return
		(f == kFeatureAutoComputeDirtyRects || f == kFeatureVirtualKeyboard);
}

void OSystem_WINCE3::setFeatureState(Feature f, bool enable) {
	switch(f) {
		case kFeatureFullscreenMode:
			return;
		case kFeatureVirtualKeyboard:
			if (_isSmartphone)
				return;
			_toolbarHighDrawn = false;
			if (enable) {
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
			return;
		default:
			OSystem_SDL::setFeatureState(f, enable);
	}
}

bool OSystem_WINCE3::getFeatureState(Feature f) {
	switch(f) {
		case kFeatureFullscreenMode:
			return false;
		case kFeatureVirtualKeyboard:
			return (_panelStateForced);
	}
	return OSystem_SDL::getFeatureState(f);
}

bool OSystem_WINCE3::setSoundCallback(SoundProc proc, void *param) {
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
	if (!ConfMan.hasKey("sound_thread_priority")) {
#ifdef SH3
		thread_priority = THREAD_PRIORITY_NORMAL;
#else
		thread_priority = THREAD_PRIORITY_ABOVE_NORMAL;
#endif
	}
	else
		thread_priority = ConfMan.getInt("sound_thread_priority");

	desired.thread_priority = thread_priority;

	if (SDL_OpenAudio(&desired, NULL) != 0) {
		return false;
	}
	SDL_PauseAudio(0);
	return true;
}

void OSystem_WINCE3::check_mappings() {
		CEActionsPocket *instance;

		if (!_gameDetector._targetName.size() || GUI_Actions::Instance()->initialized())
			return;

		GUI_Actions::Instance()->initInstanceGame();
		instance = (CEActionsPocket*)GUI_Actions::Instance();

		// Some games need to map the right click button, signal it here if it wasn't done
		if (instance->needsRightClickMapping()) {
			GUI::KeysDialog *keysDialog = new GUI::KeysDialog("Map right click action");
			while (!instance->getMapping(POCKET_ACTION_RIGHTCLICK)) {
				keysDialog->runModal();
				if (!instance->getMapping(POCKET_ACTION_RIGHTCLICK)) {
					GUI::MessageDialog alert("You must map a key to the 'Right Click' action to play this game");
					alert.runModal();
				}
			}
			delete keysDialog;
		}

		// Map the "hide toolbar" action if needed
		if (instance->needsHideToolbarMapping()) {
			GUI::KeysDialog *keysDialog = new GUI::KeysDialog("Map hide toolbar action");
			while (!instance->getMapping(POCKET_ACTION_HIDE)) {
				keysDialog->runModal();
				if (!instance->getMapping(POCKET_ACTION_HIDE)) {
					GUI::MessageDialog alert("You must map a key to the 'Hide toolbar' action to play this game");
					alert.runModal();
				}
			}
			delete keysDialog;
		}

		// Map the "zoom" actions if needed
		if (instance->needsZoomMapping()) {
			GUI::KeysDialog *keysDialog = new GUI::KeysDialog("Map Zoom Up action (optional)");
			keysDialog->runModal();
			delete keysDialog;
			keysDialog = new GUI::KeysDialog("Map Zoom Down action (optional)");
			keysDialog->runModal();
			delete keysDialog;
		}

		// Extra warning for Zak Mc Kracken
		if (strncmp(_gameDetector._targetName.c_str(), "zak", 3) == 0 &&
			!GUI_Actions::Instance()->getMapping(POCKET_ACTION_HIDE)) {
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
		panel->add(NAME_ITEM_OPTIONS, new ItemAction(ITEM_OPTIONS, POCKET_ACTION_SAVE));
		// Skip
		panel->add(NAME_ITEM_SKIP, new ItemAction(ITEM_SKIP, POCKET_ACTION_SKIP));
		// sound
		panel->add(NAME_ITEM_SOUND, new ItemSwitch(ITEM_SOUND_OFF, ITEM_SOUND_ON, &_soundMaster));
		// portrait/landscape - screen dependant
		// FIXME : will still display the portrait/landscape icon when using a scaler (but will be disabled)
		if (_screenWidth <= 320 && (isOzone() || !CEDevice::hasDesktopResolution())) {
			_newOrientation = _orientationLandscape = (ConfMan.hasKey("landscape") ? ConfMan.getBool("landscape") : false);
			panel->add(NAME_ITEM_ORIENTATION, new ItemSwitch(ITEM_VIEW_LANDSCAPE, ITEM_VIEW_PORTRAIT, &_newOrientation));
		}
		_toolbarHandler.add(NAME_MAIN_PANEL, *panel);
		_toolbarHandler.setActive(NAME_MAIN_PANEL);

		// Keyboard is active for Monkey 1 or 2 initial copy-protection
		if (strncmp(_gameDetector._targetName.c_str(), "monkey", 6) == 0) {
			_monkeyKeyboard = true;
			_toolbarHandler.setActive(NAME_PANEL_KEYBOARD);
		}

		if (_mode == GFX_NORMAL && ConfMan.hasKey("landscape") && ConfMan.getBool("landscape")) {
			setGraphicsMode(GFX_NORMAL);
			hotswapGFXMode();
		}

		if (_isSmartphone)
			panel->setVisible(false);

		// Set Smush Force Redraw rate for Full Throttle
		if (!ConfMan.hasKey("Smush_force_redraw")) {
			ConfMan.set("Smush_force_redraw", 30);
			ConfMan.flushToDisk();
		}
	}

	get_sample_rate();
}

void OSystem_WINCE3::initSize(uint w, uint h, int overlayScale) {

	if (_isSmartphone && h == 240)
		h = 200;  // mainly for the launcher

	switch (_transactionMode) {
		case kTransactionActive:
			_transactionDetails.w = w;
			_transactionDetails.h = h;
			_transactionDetails.overlayScale = overlayScale;
			_transactionDetails.sizeChanged = true;
			_transactionDetails.needUnload = true;
			return;
			break;
		case kTransactionCommit:
			break;
		default:
			break;
	}

	if (w == 320 && h == 200 && !_isSmartphone)
		h = 240; // use the extra 40 pixels height for the toolbar

	if (!_isSmartphone) {
		if (h == 240)
			_toolbarHandler.setOffset(200);
		else
			_toolbarHandler.setOffset(400);
	}

	if (w != _screenWidth || h != _screenHeight)
		_scalersChanged = false;

	//OSystem_SDL::initSize(w, h, overlayScale);
	OSystem_SDL::initSize(w, h, 1);

	if (_scalersChanged) {
		unloadGFXMode();
		loadGFXMode();
		_scalersChanged = false;
	}

	update_game_settings();
}

int OSystem_WINCE3::getDefaultGraphicsMode() const {
    return GFX_NORMAL;
}

bool OSystem_WINCE3::update_scalers() {

	if (_mode != GFX_NORMAL)
		return false;

	if (CEDevice::hasPocketPCResolution()) {
		if (!_orientationLandscape && (_screenWidth == 320 || !_screenWidth)) {
			_scaleFactorXm = 3;
			_scaleFactorXd = 4;
			_scaleFactorYm = 1;
			_scaleFactorYd = 1;
			_scalerProc = PocketPCPortrait;
			_modeFlags = 0;
		}
		if (_screenWidth == 640 && !(isOzone() && (getScreenWidth() >= 640 || getScreenHeight() >= 640))) {
			_scaleFactorXm = 1;
			_scaleFactorXd = 2;
			_scaleFactorYm = 1;
			_scaleFactorYd = 2;
			_scalerProc = PocketPCHalf;
			_modeFlags = 0;
		}
		if (_screenWidth == 640 && (isOzone() && (getScreenWidth() >= 640 || getScreenHeight() >= 640))) {
			_scaleFactorXm = 1;
			_scaleFactorXd = 1;
			_scaleFactorYm = 1;
			_scaleFactorYd = 1;
			_scalerProc = Normal1x;
			_modeFlags = 0;
		}

		return true;
	}

//#ifdef WIN32_PLATFORM_WFSP
	if (_isSmartphone) {
		if (_screenWidth > 320)
			error("Game resolution not supported on Smartphone");
		_scaleFactorXm = 2;
		_scaleFactorXd = 3;
		_scaleFactorYm = 7;
		_scaleFactorYd = 8;
		_scalerProc = SmartphoneLandscape;
		_modeFlags = 0;
		initZones();
		return true;
	}
//#endif

	return false;
}

bool OSystem_WINCE3::setGraphicsMode(int mode) {

    switch (_transactionMode) {
			case kTransactionActive:
                _transactionDetails.mode = mode;
                _transactionDetails.modeChanged = true;
                return true;
                break;
	        case kTransactionCommit:
                break;
		    default:
                break;
    }

	Common::StackLock lock(_graphicsMutex);
	int oldScaleFactorXm = _scaleFactorXm;
	int oldScaleFactorXd = _scaleFactorXd;
	int oldScaleFactorYm = _scaleFactorYm;
	int oldScaleFactorYd = _scaleFactorYd;

	_scaleFactorXm = -1;
	_scaleFactorXd = -1;
	_scaleFactorYm = -1;
	_scaleFactorYd = -1;

	_newOrientation = _orientationLandscape = (ConfMan.hasKey("landscape") ? ConfMan.getBool("landscape") : false);

	update_scalers();

	// FIXME
	if (isOzone() && (getScreenWidth() >= 640 || getScreenHeight() >= 640) && mode)
		_scaleFactorXm = -1;

	if (CEDevice::hasPocketPCResolution() && !CEDevice::hasWideResolution() && _orientationLandscape)
		_mode = GFX_NORMAL;
	else
		_mode = mode;

	if (_scaleFactorXm < 0) {
		/* Standard scalers, from the SDL backend */
		switch(_mode) {
		case GFX_NORMAL:
			_scaleFactor = 1;
			_scalerProc = Normal1x;
			break;
		case GFX_DOUBLESIZE:
			_scaleFactor = 2;
			_scalerProc = Normal2x;
			break;
		case GFX_TRIPLESIZE:
			_scaleFactor = 3;
			_scalerProc = Normal3x;
			break;
		case GFX_2XSAI:
			_scaleFactor = 2;
			_scalerProc = _2xSaI;
			break;
		case GFX_SUPER2XSAI:
			_scaleFactor = 2;
			_scalerProc = Super2xSaI;
			break;
		case GFX_SUPEREAGLE:
			_scaleFactor = 2;
			_scalerProc = SuperEagle;
			break;
		case GFX_ADVMAME2X:
			_scaleFactor = 2;
			_scalerProc = AdvMame2x;
			break;
		case GFX_ADVMAME3X:
			_scaleFactor = 3;
			_scalerProc = AdvMame3x;
			break;
		case GFX_HQ2X:
			_scaleFactor = 2;
			_scalerProc = HQ2x;
			break;
		case GFX_HQ3X:
			_scaleFactor = 3;
			_scalerProc = HQ3x;
			break;
		case GFX_TV2X:
			_scaleFactor = 2;
			_scalerProc = TV2x;
			break;
		case GFX_DOTMATRIX:
			_scaleFactor = 2;
			_scalerProc = DotMatrix;
			break;

		default:
			error("unknown gfx mode %d", _mode);
		}
	}

	// Check if the scaler can be accepted, if not get back to normal scaler
	if (_scaleFactor && ((_scaleFactor * _screenWidth > getScreenWidth() &&
						  _scaleFactor * _screenWidth > getScreenHeight())
					 || (_scaleFactor * _screenHeight > getScreenWidth() &&
						_scaleFactor * _screenHeight > getScreenHeight()))) {
				_scaleFactor = 1;
				_scalerProc = Normal1x;
	}

	// Common scaler system was used
	if (_scaleFactorXm < 0) {
		_scaleFactorXm = _scaleFactor;
		_scaleFactorXd = 1;
		_scaleFactorYm = _scaleFactor;
		_scaleFactorYd = 1;
	}

	_forceFull = true;

	if (oldScaleFactorXm != _scaleFactorXm ||
		oldScaleFactorXd != _scaleFactorXd ||
		oldScaleFactorYm != _scaleFactorYm ||
		oldScaleFactorYd != _scaleFactorYd) {
		_scalersChanged = true;
	}
	else
		_scalersChanged = false;


	return true;

}

void OSystem_WINCE3::loadGFXMode() {
	int displayWidth;
	int displayHeight;

	_fullscreen = true; // forced
	_forceFull = true;
	_modeFlags |= DF_UPDATE_EXPAND_1_PIXEL;

	_tmpscreen = NULL;

	// Recompute scalers if necessary
	update_scalers();

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

	// FIXME
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
	_tmpscreen = SDL_CreateRGBSurface(SDL_SWSURFACE,
						_screenWidth + 3,
						_screenHeight + 3,
						16,
						_hwscreen->format->Rmask,
						_hwscreen->format->Gmask,
						_hwscreen->format->Bmask,
						_hwscreen->format->Amask);

	if (_tmpscreen == NULL)
		error("_tmpscreen failed");



	// Overlay
	_overlayscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, _overlayWidth, _overlayHeight, 16, 0, 0, 0, 0);
	if (_overlayscreen == NULL)
		error("_overlayscreen failed");
	_tmpscreen2 = SDL_CreateRGBSurface(SDL_SWSURFACE, _overlayWidth + 3, _overlayHeight + 3, 16, 0, 0, 0, 0);
	if (_tmpscreen2 == NULL)
		error("_tmpscreen2 failed");



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
	_km.x_max = _screenWidth * _scaleFactorXm / _scaleFactorXd - 1;
	_km.y_max = _screenHeight * _scaleFactorXm / _scaleFactorXd - 1;
	_km.delay_time = 25;
	_km.last_time = 0;
}

void OSystem_WINCE3::unloadGFXMode() {
	if (_screen) {
		SDL_FreeSurface(_screen);
		_screen = NULL;
	}

	if (_hwscreen) {
		SDL_FreeSurface(_hwscreen);
		_hwscreen = NULL;
	}

	if (_tmpscreen) {
		SDL_FreeSurface(_tmpscreen);
		_tmpscreen = NULL;
	}
}

void OSystem_WINCE3::hotswapGFXMode() {
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
	loadGFXMode();

	// reset palette
	SDL_SetColors(_screen, _currentPalette, 0, 256);

	// Restore old screen content
	SDL_BlitSurface(old_screen, NULL, _screen, NULL);
	SDL_BlitSurface(old_tmpscreen, NULL, _tmpscreen, NULL);

	// Free the old surfaces
	SDL_FreeSurface(old_screen);
	SDL_FreeSurface(old_tmpscreen);

	// Blit everything to the screen
	internUpdateScreen();

	// Make sure that an EVENT_SCREEN_CHANGED gets sent later
	_modeChanged = true;
}

void OSystem_WINCE3::update_keyboard() {

	// Update the forced keyboard for Monkey Island copy protection
	if (_monkeyKeyboard && Scumm::g_scumm->VAR_ROOM != 0xff && Scumm::g_scumm && Scumm::g_scumm->VAR(Scumm::g_scumm->VAR_ROOM) != 108 &&
		Scumm::g_scumm->VAR(Scumm::g_scumm->VAR_ROOM) != 90) {
			// Switch back to the normal panel now that the keyboard is not used anymore
			_monkeyKeyboard = false;
			_toolbarHandler.setActive(NAME_MAIN_PANEL);
	}
}

void OSystem_WINCE3::internUpdateScreen() {
	SDL_Surface *srcSurf, *origSurf;
	assert(_hwscreen != NULL);

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
	drawMouse();

	// Check whether the palette was changed in the meantime and update the
	// screen surface accordingly.
	if (_paletteDirtyEnd != 0) {
		SDL_SetColors(_screen, _currentPalette + _paletteDirtyStart,
			_paletteDirtyStart,
			_paletteDirtyEnd - _paletteDirtyStart);

		_paletteDirtyEnd = 0;

		_forceFull = true;
	}

	if (!_overlayVisible) {
		origSurf = _screen;
		srcSurf = _tmpscreen;
	}
	else {
		origSurf = _overlayscreen;
		srcSurf = _tmpscreen2;
	}

	// Force a full redraw if requested
	if (_forceFull) {
		_numDirtyRects = 1;

		_dirtyRectList[0].x = 0;
		if (!_zoomDown)
			_dirtyRectList[0].y = 0;
		else
			_dirtyRectList[0].y = _screenHeight / 2;
		_dirtyRectList[0].w = _screenWidth;
		if (!_zoomUp && !_zoomDown)
			_dirtyRectList[0].h = _screenHeight;
		else
			_dirtyRectList[0].h = _screenHeight / 2;

		_toolbarHandler.forceRedraw();
	}

	// Only draw anything if necessary
	if (_numDirtyRects > 0) {

		SDL_Rect *r;
		SDL_Rect dst;
		uint32 srcPitch, dstPitch;
		SDL_Rect *last_rect = _dirtyRectList + _numDirtyRects;
		bool toolbarVisible = _toolbarHandler.visible();
		int toolbarOffset = _toolbarHandler.getOffset();

		if (_scalerProc == Normal1x && !_adjustAspectRatio && 0) {
			for (r = _dirtyRectList; r != last_rect; ++r) {
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
				if (SDL_BlitSurface(origSurf, r, _hwscreen, &dst) != 0)
					error("SDL_BlitSurface failed: %s", SDL_GetError());
			}
		} else {
				for (r = _dirtyRectList; r != last_rect; ++r) {
					dst = *r;
					dst.x++;	// Shift rect by one since 2xSai needs to acces the data around
					dst.y++;	// any pixel to scale it, and we want to avoid mem access crashes.
					if (SDL_BlitSurface(origSurf, r, srcSurf, &dst) != 0)
						error("SDL_BlitSurface failed: %s", SDL_GetError());
				}

			SDL_LockSurface(srcSurf);
			SDL_LockSurface(_hwscreen);

			srcPitch = srcSurf->pitch;
			dstPitch = _hwscreen->pitch;

			for (r = _dirtyRectList; r != last_rect; ++r) {
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

					if (!_zoomDown)
						_scalerProc((byte *)srcSurf->pixels + (r->x * 2 + 2) + (r->y + 1) * srcPitch, srcPitch,
							(byte *)_hwscreen->pixels + (r->x * 2 * _scaleFactorXm / _scaleFactorXd) + dst_y * dstPitch, dstPitch, r->w, dst_h);
					else {
						_scalerProc((byte *)srcSurf->pixels + (r->x * 2 + 2) + (r->y + 1) * srcPitch, srcPitch,
							(byte *)_hwscreen->pixels + (r->x * 2 * _scaleFactorXm / _scaleFactorXd) + (dst_y - 240) * dstPitch, dstPitch, r->w, dst_h);
					}
				}

				r->x = r->x * _scaleFactorXm / _scaleFactorXd;
				if (!_zoomDown)
					r->y = dst_y;
				else
					r->y = dst_y - 240;
				r->w = r->w * _scaleFactorXm / _scaleFactorXd;
				r->h = dst_h * _scaleFactorYm / _scaleFactorYd;

				/*if (_adjustAspectRatio && orig_dst_y / _scaleFactor < _screenHeight)
					r->h = stretch200To240((uint8 *) _hwscreen->pixels, dstPitch, r->w, r->h, r->x, r->y, orig_dst_y);
				*/
			}
			SDL_UnlockSurface(srcSurf);
			SDL_UnlockSurface(_hwscreen);
		}

		// Readjust the dirty rect list in case we are doing a full update.
		// This is necessary if shaking is active.
		if (_forceFull) {
			_dirtyRectList[0].y = 0;
			_dirtyRectList[0].h = (_adjustAspectRatio ? 240 : (_zoomUp || _zoomDown ? _screenHeight / 2 : _screenHeight)) * _scaleFactorYm / _scaleFactorYd;
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
			else
				_toolbarHighDrawn = false;
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
		_scalerProc((byte *)toolbarSurface->pixels, srcPitch, (byte *)_hwscreen->pixels + (_toolbarHandler.getOffset() * _scaleFactorYm / _scaleFactorYd * dstPitch), dstPitch, toolbar_rect[0].w, toolbar_rect[0].h);
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
	if (_numDirtyRects > 0)
		SDL_UpdateRects(_hwscreen, _numDirtyRects, _dirtyRectList);

	_numDirtyRects = 0;
	_forceFull = false;
}

bool OSystem_WINCE3::saveScreenshot(const char *filename) {
	assert(_hwscreen != NULL);

	Common::StackLock lock(_graphicsMutex);	// Lock the mutex until this function ends
	SDL_SaveBMP(_hwscreen, filename);
	return true;
}

// FIXME
// Reuse static or proper mapping

static int mapKeyCE(SDLKey key, SDLMod mod, Uint16 unicode)
{
	if (GUI::Actions::Instance()->mappingActive())
		return key;

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


void OSystem_WINCE3::drawMouse() {
	// FIXME
	if (!(_toolbarHandler.visible() && _mouseCurState.y >= _toolbarHandler.getOffset()) && !_forceHideMouse)
		OSystem_SDL::drawMouse();
}

void OSystem_WINCE3::fillMouseEvent(Event &event, int x, int y) {
	event.mouse.x = x;
	event.mouse.y = y;

	// Update the "keyboard mouse" coords
	_km.x = event.mouse.x;
	_km.y = event.mouse.y;

	// Adjust for the screen scaling
	if (_zoomDown)
		event.mouse.y += 240;

	event.mouse.x = event.mouse.x * _scaleFactorXd / _scaleFactorXm;
	event.mouse.y = event.mouse.y * _scaleFactorYd / _scaleFactorYm;
}

void OSystem_WINCE3::retrieve_mouse_location(int &x, int &y) {
	x = _mouseCurState.x;
	y = _mouseCurState.y;

	x = x * _scaleFactorXm / _scaleFactorXd;
	y = y * _scaleFactorYm / _scaleFactorYd;

	if (_zoomDown)
		y -= 240;
}

void OSystem_WINCE3::warpMouse(int x, int y) {
		if (_mouseCurState.x != x || _mouseCurState.y != y) {
		SDL_WarpMouse(x * _scaleFactorXm / _scaleFactorXd, y * _scaleFactorYm / _scaleFactorYd);

		// SDL_WarpMouse() generates a mouse movement event, so
		// set_mouse_pos() would be called eventually. However, the
		// cannon script in CoMI calls this function twice each time
		// the cannon is reloaded. Unless we update the mouse position
		// immediately the second call is ignored, causing the cannon
		// to change its aim.

		setMousePos(x, y);
	}
}

void OSystem_WINCE3::addDirtyRect(int x, int y, int w, int h, bool mouseRect) {
	// Align on boundaries
	if (_scaleFactorXd > 1) {
		while (x % _scaleFactorXd) {
			x--;
			w++;
		}
		while (w % _scaleFactorXd) w++;
	}

	if (_scaleFactorYd > 1) {
		while (y % _scaleFactorYd) {
			y--;
			h++;
		}
		while (h % _scaleFactorYd) h++;
	}

	if (_scalerProc == PocketPCHalfZoom) {
		// Restrict rect if we're zooming
		if (_zoomUp) {
			if (y + h >= 240) {
				if (y >= 240)
					return;
				else
					h = 240 - y;
			}
		}
		else
		if (_zoomDown) {
			if (y + h >= 240) {
				if (y < 240) {
					h = 240 - y;
					y = 240;
				}
			}
			else
				return;
		}
	}

	OSystem_SDL::addDirtyRect(x, y, w, h, mouseRect);
}

// FIXME
// See if some SDL mapping can be useful for HPCs

bool OSystem_WINCE3::pollEvent(Event &event) {
	SDL_Event ev;
	byte b = 0;
	Event temp_event;
	DWORD currentTime;
	bool keyEvent = false;

	memset(&temp_event, 0, sizeof(Event));
	memset(&event, 0, sizeof(Event));

	handleKbdMouse();

	// If the screen mode changed, send an EVENT_SCREEN_CHANGED
	if (_modeChanged) {
		_modeChanged = false;
		event.type = EVENT_SCREEN_CHANGED;
		return true;
	}

	CEDevice::wakeUp();

	if (_isSmartphone)
		currentTime = GetTickCount();

	while(SDL_PollEvent(&ev)) {
		switch(ev.type) {
		case SDL_KEYDOWN:
			// KMOD_RESERVED is used if the key has been injected by an external buffer
			if (ev.key.keysym.mod != KMOD_RESERVED) {
				if (_isSmartphone) {
					keyEvent = true;
					_lastKeyPressed = ev.key.keysym.sym;
					_keyRepeatTime = currentTime;
					_keyRepeat = 0;
				}

				if (!GUI_Actions::Instance()->mappingActive() && GUI_Actions::Instance()->performMapped(ev.key.keysym.sym, true))
					return true;
			}

			event.type = EVENT_KEYDOWN;
			event.kbd.keycode = ev.key.keysym.sym;
			event.kbd.ascii = mapKeyCE(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode);

			if (GUI_Actions::Instance()->mappingActive())
				event.kbd.flags = 0xFF;

			return true;

		case SDL_KEYUP:
			// KMOD_RESERVED is used if the key has been injected by an external buffer
			if (ev.key.keysym.mod != KMOD_RESERVED) {
				if (_isSmartphone) {
					keyEvent = true;
					_lastKeyPressed = 0;
				}

				if (!GUI_Actions::Instance()->mappingActive() && GUI_Actions::Instance()->performMapped(ev.key.keysym.sym, false))
					return true;
			}

			event.type = EVENT_KEYUP;
			event.kbd.keycode = ev.key.keysym.sym;
			event.kbd.ascii = mapKeyCE(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode);

			if (GUI_Actions::Instance()->mappingActive())
				event.kbd.flags = 0xFF;

			return true;

		case SDL_MOUSEMOTION:
			event.type = EVENT_MOUSEMOVE;
			fillMouseEvent(event, ev.motion.x, ev.motion.y);
			setMousePos(event.mouse.x, event.mouse.y);
			return true;

		case SDL_MOUSEBUTTONDOWN:
			if (ev.button.button == SDL_BUTTON_LEFT)
				temp_event.type = EVENT_LBUTTONDOWN;
			else if (ev.button.button == SDL_BUTTON_RIGHT)
				temp_event.type = EVENT_RBUTTONDOWN;
			else
				break;

			fillMouseEvent(temp_event, ev.button.x, ev.button.y);

			if (_toolbarHandler.action(temp_event.mouse.x, temp_event.mouse.y, true)) {
				if (!_toolbarHandler.drawn())
					internUpdateScreen();
				if (_newOrientation != _orientationLandscape && _mode == GFX_NORMAL) {
					_orientationLandscape = _newOrientation;
					ConfMan.set("landscape", _orientationLandscape);
					ConfMan.flushToDisk();
					setGraphicsMode(GFX_NORMAL);
					hotswapGFXMode();
				}
			}
			else {
				if (!_freeLook)
					memcpy(&event, &temp_event, sizeof(Event));
			}

			return true;

		case SDL_MOUSEBUTTONUP:
			if (ev.button.button == SDL_BUTTON_LEFT)
				temp_event.type = EVENT_LBUTTONUP;
			else if (ev.button.button == SDL_BUTTON_RIGHT)
				temp_event.type = EVENT_RBUTTONUP;
			else
				break;

			fillMouseEvent(temp_event, ev.button.x, ev.button.y);

			if (_toolbarHandler.action(temp_event.mouse.x, temp_event.mouse.y, false)) {
				if (!_toolbarHandler.drawn())
					internUpdateScreen();
			}
			else {
				if (!_freeLook)
					memcpy(&event, &temp_event, sizeof(Event));
			}

			return true;

		case SDL_VIDEOEXPOSE:
			_forceFull = true;
			break;

		case SDL_QUIT:
			event.type = EVENT_QUIT;
			return true;
		}
	}

	// Simulate repeated key for Smartphones

	if (!keyEvent) {
		if (_isSmartphone) {

			if (_lastKeyPressed) {
				if (currentTime > _keyRepeatTime + _keyRepeatTrigger) {
					_keyRepeatTime = currentTime;
					_keyRepeat++;
					GUI_Actions::Instance()->performMapped(_lastKeyPressed, true);
				}
			}
		}
	}

	return false;
}

void OSystem_WINCE3::quit() {
	fclose(stdout_file);
	fclose(stderr_file);
	if (gDebugLevel <= 0) {
		DeleteFile(TEXT("\\scummvm_stdout.txt"));
		DeleteFile(TEXT("\\scummvm_stderr.txt"));
	}
	CEDevice::end();
	OSystem_SDL::quit();
}

int OSystem_WINCE3::_platformScreenWidth;
int OSystem_WINCE3::_platformScreenHeight;
bool OSystem_WINCE3::_isOzone;
OSystem_WINCE3::zoneDesc OSystem_WINCE3::_zones[TOTAL_ZONES] = {
        { 0, 0, 320, 145 },
        { 0, 145, 150, 55 },
        { 150, 145, 170, 55 }
};
