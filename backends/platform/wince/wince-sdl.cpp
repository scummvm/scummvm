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

#include "backends/platform/wince/wince-sdl.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/util.h"
#include "common/timer.h"

#include "engines/engine.h"

#include "base/main.h"
#include "base/plugins.h"

#include "sound/mixer_intern.h"
#include "sound/fmopl.h"

#include "backends/timer/default/default-timer.h"

#include "gui/Actions.h"
#include "gui/KeysDialog.h"
#include "gui/message.h"

#include "backends/platform/wince/resource.h"
#include "backends/platform/wince/CEActionsPocket.h"
#include "backends/platform/wince/CEActionsSmartphone.h"
#include "backends/platform/wince/CEgui/ItemAction.h"


#include "backends/platform/wince/CEException.h"

#ifdef USE_VORBIS
#ifndef USE_TREMOR
#include <vorbis/vorbisfile.h>
#else
#include <tremor/ivorbisfile.h>
#endif
#endif

#ifdef DYNAMIC_MODULES
#include "backends/plugins/win32/win32-provider.h"
#endif

#ifdef __GNUC__
extern "C" _CRTIMP FILE* __cdecl   _wfreopen (const wchar_t*, const wchar_t*, FILE*);
#endif

#define SAMPLES_PER_SEC_OLD 11025
#define SAMPLES_PER_SEC_NEW 22050

using namespace CEGUI;

// ********************************************************************************************

// Internal GUI names

#define NAME_MAIN_PANEL			"MainPanel"
#define NAME_PANEL_KEYBOARD		"Keyboard"
#define NAME_ITEM_OPTIONS		"Options"
#define NAME_ITEM_SKIP			"Skip"
#define NAME_ITEM_SOUND			"Sound"
#define NAME_ITEM_ORIENTATION	"Orientation"
#define NAME_ITEM_BINDKEYS		"Bindkeys"

// stdin/err redirection
#define STDOUT_FNAME "\\scummvm_stdout.txt"
#define STDERR_FNAME "\\scummvm_stderr.txt"
static FILE *stdout_file = NULL, *stderr_file = NULL;
static char stdout_fname[MAX_PATH], stderr_fname[MAX_PATH];

// Static member inits
typedef void (*SoundProc)(void *param, byte *buf, int len);
bool OSystem_WINCE3::_soundMaster = true;

bool _isSmartphone = false;
bool _hasSmartphoneResolution = false;

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

#define DEFAULT_CONFIG_FILE "scummvm.ini"

// ********************************************************************************************

bool isSmartphone() {
	//return _isSmartphone;
	return _hasSmartphoneResolution;
}

const TCHAR *ASCIItoUnicode(const char *str) {
	static TCHAR ustr[MAX_PATH];	// size good enough

	MultiByteToWideChar(CP_ACP, 0, str, strlen(str) + 1, ustr, sizeof(ustr) / sizeof(TCHAR));
	return ustr;
}

// MAIN
#ifndef __GNUC__
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
#endif

OSystem *OSystem_WINCE3_create() {
	return new OSystem_WINCE3();
}

extern "C" char *getcwd(char *buf, int size);
int SDL_main(int argc, char **argv) {
	FILE *newfp = NULL;
#ifdef __GNUC__
	// Due to incomplete crt0.o implementation, we go through the constructor function
	// list provided by the linker and init all of them
	// thanks to joostp and DJWillis
	extern void (*__CTOR_LIST__)() ;
	void (**constructor)() = &__CTOR_LIST__ ;
	constructor++ ;
	while (*constructor) {
            (*constructor)() ;
            constructor++ ;
        }
#endif

	CEDevice::init();

	/* Redirect standard input and standard output */
	strcpy(stdout_fname, getcwd(NULL, MAX_PATH));
	strcpy(stderr_fname, getcwd(NULL, MAX_PATH));
	strcat(stdout_fname, STDOUT_FNAME);
	strcat(stderr_fname, STDERR_FNAME);
#ifndef __GNUC__
	stdout_file = fopen(stdout_fname, "w");
	stderr_file = fopen(stderr_fname, "w");
#else
	stdout_file = newfp = _wfreopen(ASCIItoUnicode(stdout_fname), TEXT("w"), stdout);
	if (newfp == NULL) {
#if !defined(stdout)
		stdout = fopen(stdout_fname, "w");
		stdout_file = stdout;
#else
		newfp = fopen(stdout_fname, "w");
		if (newfp) {
			//*stdout = *newfp;
			stdout_file = stdout;
		}
#endif
	}
	stderr_file = newfp = _wfreopen(ASCIItoUnicode(stderr_fname), TEXT("w"), stderr);
	if (newfp == NULL) {
#if !defined(stderr)
		stderr = fopen(stderr_fname, "w");
		stderr_file = stderr;
#else
		newfp = fopen(stderr_fname, "w");
		if (newfp) {
			//*stderr = *newfp;
			stderr_file = stderr;
		}
#endif
	}
#endif

#ifdef DYNAMIC_MODULES
	PluginManager::instance().addPluginProvider(new Win32PluginProvider());
#endif


	int res = 0;
#if !defined(DEBUG) && !defined(__GNUC__)
	__try {
#endif
		g_system = OSystem_WINCE3_create();
		assert(g_system);

		// Invoke the actual ScummVM main entry point:
		res = scummvm_main(argc, argv);
		g_system->quit();	// TODO: Consider removing / replacing this!
#if !defined(DEBUG) && !defined(__GNUC__)
	}
	__except (handleException(GetExceptionInformation())) {
	}
#endif

	return res;
}

#ifdef DYNAMIC_MODULES

/* This is the OS startup code in the case of a plugin-enabled build.
 * It contains copied and slightly modified parts of SDL's win32/ce startup functions.
 * We copy these here because the calling stub already has a WinMain procedure
 * which overrides SDL's one and hence we essentially re-implement the startup procedure.
 * Note also that this has to be here and not in the stub because SDL is statically
 * linked in the scummvm.dll archive.
 * Take a look at the comments in stub.cpp as well.
 */

int console_main(int argc, char *argv[])
{
	int n;
	char *bufp, *appname;

	appname = argv[0];
	if ( (bufp=strrchr(argv[0], '\\')) != NULL )
		appname = bufp + 1;
	else if ( (bufp=strrchr(argv[0], '/')) != NULL )
		appname = bufp + 1;

	if ( (bufp=strrchr(appname, '.')) == NULL )
		n = strlen(appname);
	else
		n = (bufp-appname);

	bufp = (char *) alloca(n + 1);
	strncpy(bufp, appname, n);
	bufp[n] = '\0';
	appname = bufp;

	if ( SDL_Init(SDL_INIT_NOPARACHUTE) < 0 ) {
		error("WinMain() error: %d", SDL_GetError());
		return(FALSE);
	}

	SDL_SetModuleHandle(GetModuleHandle(NULL));

	// Run the application main() code
	SDL_main(argc, argv);

	return(0);
}

static int ParseCommandLine(char *cmdline, char **argv)
{
	char *bufp;
	int argc;

	argc = 0;
	for (bufp = cmdline; *bufp;) {
		// Skip leading whitespace
		while (isspace(*bufp))
			++bufp;

		// Skip over argument
		if (*bufp == '"') {
			++bufp;
			if (*bufp) {
				if (argv)
					argv[argc] = bufp;
				++argc;
			}
			// Skip over word
			while (*bufp && (*bufp != '"'))
				++bufp;
		} else {
			if (*bufp) {
				if (argv)
					argv[argc] = bufp;
				++argc;
			}
			// Skip over word
			while (*bufp && ! isspace(*bufp))
				++bufp;
		}
		if (*bufp) {
			if (argv)
				*bufp = '\0';
			++bufp;
		}
	}
	if (argv)
		argv[argc] = NULL;

	return(argc);
}

int dynamic_modules_main(HINSTANCE hInst, HINSTANCE hPrev, LPWSTR szCmdLine, int sw) {
	HINSTANCE handle;
	char **argv;
	int argc;
	char *cmdline;
	wchar_t *bufp;
	int nLen;

	if (wcsncmp(szCmdLine, TEXT("\\"), 1)) {
		nLen = wcslen(szCmdLine)+128+1;
		bufp = (wchar_t *) alloca(nLen*2);
		wcscpy (bufp, TEXT("\""));
		GetModuleFileName(NULL, bufp+1, 128-3);
		wcscpy (bufp+wcslen(bufp), TEXT("\" "));
		wcsncpy(bufp+wcslen(bufp), szCmdLine,nLen-wcslen(bufp));
	} else
		bufp = szCmdLine;

	nLen = wcslen(bufp)+1;
	cmdline = (char *) alloca(nLen);
	WideCharToMultiByte(CP_ACP, 0, bufp, -1, cmdline, nLen, NULL, NULL);

	// Parse command line into argv and argc
	argc = ParseCommandLine(cmdline, NULL);
	argv = (char **) alloca((argc+1)*(sizeof *argv));
	ParseCommandLine(cmdline, argv);

	// fix gdb-emulator combo
	while (argc > 1 && !strstr(argv[0], ".exe")) {
		OutputDebugString(TEXT("SDL: gdb argv[0] fixup\n"));
		*(argv[1]-1) = ' ';
		int i;
		for (i=1; i<argc; i++)
			argv[i] = argv[i+1];
		argc--;
	}

	// Run the main program (after a little SDL initialization)
	return(console_main(argc, argv));

}
#endif

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
static DefaultTimerManager *_int_timer = NULL;
static Uint32 timer_handler_wrapper(Uint32 interval) {
	_int_timer->handler();
	return interval;
}

void OSystem_WINCE3::initBackend()
{
	// Instantiate our own sound mixer
	// mixer init is rerun when a game engine is selected.
	setupMixer();

	// Create the timer. CE SDL does not support multiple timers (SDL_AddTimer).
	// We work around this by using the SetTimer function, since we only use
	// one timer in scummvm (for the time being)
	_timer = _int_timer = new DefaultTimerManager();
	_timerID = NULL;	// OSystem_SDL will call removetimer with this, it's ok
	SDL_SetTimer(10, &timer_handler_wrapper);

	// Chain init
	OSystem_SDL::initBackend();

	// Query SDL for screen size and init screen dependent stuff
	OSystem_WINCE3::initScreenInfos();
	_isSmartphone = CEDevice::isSmartphone();
	create_toolbar();
	_hasSmartphoneResolution = CEDevice::hasSmartphoneResolution() || CEDevice::isSmartphone();
	if (_hasSmartphoneResolution) _panelVisible = false;	// init correctly in smartphones

	// Initialize global key mapping
	GUI::Actions::init();
	GUI_Actions::Instance()->initInstanceMain(this);
	if (!GUI_Actions::Instance()->loadMapping()) {	// error during loading means not present/wrong version
		warning("Setting default action mappings.");
		GUI_Actions::Instance()->saveMapping();	// write defaults
	}

	loadDeviceConfiguration();
}

int OSystem_WINCE3::getScreenWidth() {
	return _platformScreenWidth;
}

int OSystem_WINCE3::getScreenHeight() {
	return _platformScreenHeight;
}

void OSystem_WINCE3::initScreenInfos() {
	// sdl port ensures that we use correctly full screen
	_isOzone = 0;
	SDL_Rect **r;
	r = SDL_ListModes(NULL, 0);
	_platformScreenWidth = r[0]->w;
	_platformScreenHeight = r[0]->h;
}

bool OSystem_WINCE3::isOzone() {
	return _isOzone;
}

static Common::String getDefaultConfigFileName() {
	char configFile[MAXPATHLEN];
	strcpy(configFile, getcwd(NULL, MAX_PATH));
	strcat(configFile, "\\");
	strcat(configFile, DEFAULT_CONFIG_FILE);
	return configFile;
}

Common::SeekableReadStream *OSystem_WINCE3::createConfigReadStream() {
	Common::FSNode file(getDefaultConfigFileName());
	return file.createReadStream();
}

Common::WriteStream *OSystem_WINCE3::createConfigWriteStream() {
	Common::FSNode file(getDefaultConfigFileName());
	return file.createWriteStream();
}

// ********************************************************************************************


OSystem_WINCE3::OSystem_WINCE3() : OSystem_SDL(),
	_orientationLandscape(0), _newOrientation(0), _panelInitialized(false), _canBeAspectScaled(false),
	_panelVisible(true), _panelStateForced(false), _forceHideMouse(false), _unfilteredkeys(false),
	_freeLook(false), _forcePanelInvisible(false), _toolbarHighDrawn(false), _zoomUp(false), _zoomDown(false),
	_scalersChanged(false), _lastKeyPressed(0), _tapTime(0), _closeClick(false), _noDoubleTapRMB(false),
	_saveToolbarState(false), _saveActiveToolbar(NAME_MAIN_PANEL), _rbutton(false), _hasfocus(true),
	_usesEmulatedMouse(false), _mouseBackupOld(NULL), _mouseBackupToolbar(NULL), _mouseBackupDim(0)
{
	memset(&_mouseCurState, 0, sizeof(_mouseCurState));
	if (_isSmartphone) {
		_mouseCurState.x = 20;
		_mouseCurState.y = 20;
	}

	_mixer = 0;
	_screen = NULL;
}

void OSystem_WINCE3::swap_panel_visibility() {
	//if (!_forcePanelInvisible && !_panelStateForced) {
		if (_zoomDown || _zoomUp)	return;

		if (_panelVisible) {
			if (_toolbarHandler.activeName() == NAME_PANEL_KEYBOARD)
				_panelVisible = !_panelVisible;
			else
				_toolbarHandler.setActive(NAME_PANEL_KEYBOARD);
		}
		else {
			_toolbarHandler.setActive(NAME_MAIN_PANEL);
			_panelVisible = !_panelVisible;
		}
		_toolbarHandler.setVisible(_panelVisible);
		_toolbarHighDrawn = false;

		if (_videoMode.screenHeight > 240)
			addDirtyRect(0, 400, 640, 80);
		else
			addDirtyRect(0, 200, 320, 40);

		if (_toolbarHandler.activeName() == NAME_PANEL_KEYBOARD && _panelVisible)
			internUpdateScreen();
		else {
			update_scalers();
			hotswapGFXMode();
		}
	//}
}

void OSystem_WINCE3::swap_panel() {
	_toolbarHighDrawn = false;
	//if (!_panelStateForced) {
		if (_toolbarHandler.activeName() == NAME_PANEL_KEYBOARD && _panelVisible)
			_toolbarHandler.setActive(NAME_MAIN_PANEL);
		else
			_toolbarHandler.setActive(NAME_PANEL_KEYBOARD);

		if (_videoMode.screenHeight > 240)
			addDirtyRect(0, 400, 640, 80);
		else
			addDirtyRect(0, 200, 320, 40);

		_toolbarHandler.setVisible(true);
		if (!_panelVisible) {
			_panelVisible = true;
			update_scalers();
			hotswapGFXMode();
		}
	//}
}

void OSystem_WINCE3::swap_smartphone_keyboard() {
	_toolbarHandler.setActive(NAME_PANEL_KEYBOARD);
	_panelVisible = !_panelVisible;
	_toolbarHandler.setVisible(_panelVisible);
	if (_videoMode.screenHeight > 240)
		addDirtyRect(0, 0, 640, 80);
	else
		addDirtyRect(0, 0, 320, 40);
	internUpdateScreen();
}

void OSystem_WINCE3::smartphone_rotate_display() {
	_orientationLandscape = _newOrientation = _orientationLandscape == 1 ? 2 : 1;
	ConfMan.setInt("landscape", _orientationLandscape);
	ConfMan.flushToDisk();
	hotswapGFXMode();
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

		_zoomUp = false;
		_zoomDown = true;
	}
	// redraw whole screen
	addDirtyRect(0, 0, 640, 480);
	internUpdateScreen();
}

// Smartphone actions
void OSystem_WINCE3::initZones() {
        int i;

		_currentZone = 0;
        for (i=0; i<TOTAL_ZONES; i++) {
                _mouseXZone[i] = (_zones[i].x + (_zones[i].width / 2)) * _scaleFactorXm / _scaleFactorXd;
                _mouseYZone[i] = (_zones[i].y + (_zones[i].height / 2)) * _scaleFactorYm / _scaleFactorYd;
        }
}

void OSystem_WINCE3::loadDeviceConfigurationElement(String element, int &value, int defaultValue) {
	value = ConfMan.getInt(element, ConfMan.kApplicationDomain);
	if (!value) {
		value = defaultValue;
		ConfMan.setInt(element, value, ConfMan.kApplicationDomain);
	}
}

void OSystem_WINCE3::loadDeviceConfiguration() {
	loadDeviceConfigurationElement("repeatTrigger", _keyRepeatTrigger, 200);
	loadDeviceConfigurationElement("repeatX", _repeatX, 4);
	loadDeviceConfigurationElement("repeatY", _repeatY, 4);
	loadDeviceConfigurationElement("stepX1", _stepX1, 2);
	loadDeviceConfigurationElement("stepX2", _stepX2, 10);
	loadDeviceConfigurationElement("stepX3", _stepX3, 40);
	loadDeviceConfigurationElement("stepY1", _stepY1, 2);
	loadDeviceConfigurationElement("stepY2", _stepY2, 10);
	loadDeviceConfigurationElement("stepY3", _stepY3, 20);
	ConfMan.flushToDisk();
}

void OSystem_WINCE3::add_left_click(bool pushed) {
	int x, y;
	retrieve_mouse_location(x, y);
	EventsBuffer::simulateMouseLeftClick(x, y, pushed);
}

void OSystem_WINCE3::move_cursor_up() {
	int x,y;
	_usesEmulatedMouse = true;
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
	_usesEmulatedMouse = true;
	retrieve_mouse_location(x, y);
	if (_keyRepeat > _repeatY)
		y += _stepY3;
	else
	if (_keyRepeat)
		y += _stepY2;
	else
		y += _stepY1;

	if (y > _videoMode.screenHeight*_scaleFactorYm/_scaleFactorYd)
		y = _videoMode.screenHeight*_scaleFactorYm/_scaleFactorYd;

	EventsBuffer::simulateMouseMove(x, y);
}

void OSystem_WINCE3::move_cursor_left() {
	int x,y;
	_usesEmulatedMouse = true;
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
	_usesEmulatedMouse = true;
	retrieve_mouse_location(x, y);
	if (_keyRepeat > _repeatX)
		x += _stepX3;
	else
	if (_keyRepeat)
		x += _stepX2;
	else
		x += _stepX1;

	if (x > _videoMode.screenWidth*_scaleFactorXm/_scaleFactorXd)
		x = _videoMode.screenWidth*_scaleFactorXm/_scaleFactorXd;

	EventsBuffer::simulateMouseMove(x, y);
}

void OSystem_WINCE3::switch_zone() {
	int x,y;
	int i;
	retrieve_mouse_location(x, y);

    for (i=0; i<TOTAL_ZONES; i++)
		if (x >= _zones[i].x && y >= _zones[i].y &&
			x <= _zones[i].x + _zones[i].width && y <= _zones[i].y + _zones[i].height) {
				_mouseXZone[i] = x;
				_mouseYZone[i] = y;
				break;
		}
	_currentZone = i + 1;
	if (_currentZone >= TOTAL_ZONES)
		_currentZone = 0;

	EventsBuffer::simulateMouseMove(_mouseXZone[_currentZone], _mouseYZone[_currentZone]);
}

void OSystem_WINCE3::create_toolbar() {
	PanelKeyboard *keyboard;

	// Add the keyboard
	keyboard = new PanelKeyboard(PANEL_KEYBOARD);
	_toolbarHandler.add(NAME_PANEL_KEYBOARD, *keyboard);
	_toolbarHandler.setVisible(false);
}

void OSystem_WINCE3::setupMixer() {
	SDL_AudioSpec desired;
	int thread_priority;

	compute_sample_rate();
	if (_sampleRate == 0)
		warning("setSoundCallback called with 0 _sampleRate. Audio will not work.");
	else if (_mixer && _mixer->getOutputRate() == _sampleRate) {
		debug(1, "Skipping sound mixer re-init: samplerate is good");
		return;
	}

	memset(&desired, 0, sizeof(desired));
	desired.freq = _sampleRate;
	desired.format = AUDIO_S16SYS;
	desired.channels = 2;
	desired.samples = 128;
	desired.callback = private_sound_proc;
	desired.userdata = this;

	// Create the mixer instance
	if (_mixer == 0)
		_mixer = new Audio::MixerImpl(this);

	// Add sound thread priority
	if (!ConfMan.hasKey("sound_thread_priority"))
		thread_priority = THREAD_PRIORITY_NORMAL;
	else
		thread_priority = ConfMan.getInt("sound_thread_priority");

	desired.thread_priority = thread_priority;

	SDL_CloseAudio();
	if (SDL_OpenAudio(&desired, NULL) != 0) {
		warning("Could not open audio device: %s", SDL_GetError());
		_mixer->setReady(false);

	} else {
		debug(1, "Sound opened OK, mixing at %d Hz", _sampleRate);

		// Re-create mixer to match the output rate
		int vol1 = _mixer->getVolumeForSoundType(Audio::Mixer::kPlainSoundType);
		int vol2 = _mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType);
		int vol3 = _mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType);
		int vol4 = _mixer->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType);
		delete(_mixer);
		_mixer = new Audio::MixerImpl(this);
		_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, vol1);
		_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, vol2);
		_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, vol3);
		_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, vol4);
		_mixer->setOutputRate(_sampleRate);
		_mixer->setReady(true);
		SDL_PauseAudio(0);
	}
}

void OSystem_WINCE3::private_sound_proc(void *param, byte *buf, int len) {
	OSystem_WINCE3 *this_ = (OSystem_WINCE3 *)param;
	assert(this_);

	if (this_->_mixer)
		this_->_mixer->mixCallback(buf, len);
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

void OSystem_WINCE3::compute_sample_rate() {
	// Force at least medium quality FM synthesis for FOTAQ
	Common::String gameid(ConfMan.get("gameid"));
	if (gameid == "queen") {
		if (!((ConfMan.hasKey("FM_high_quality") && ConfMan.getBool("FM_high_quality")) ||
			(ConfMan.hasKey("FM_medium_quality") && ConfMan.getBool("FM_medium_quality")))) {
			ConfMan.setBool("FM_medium_quality", true);
			ConfMan.flushToDisk();
		}
	}
	// See if the output frequency is forced by the game
	if (gameid == "ft" || gameid == "dig" || gameid == "comi" || gameid == "queen" || gameid == "sword" || gameid == "agi")
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

void OSystem_WINCE3::engineInit() {
	check_mappings(); // called here to initialize virtual keys handling

	//update_game_settings();
	// finalize mixer init
	setupMixer();
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
	return (f == kFeatureAutoComputeDirtyRects || f == kFeatureVirtualKeyboard);
}

void OSystem_WINCE3::setFeatureState(Feature f, bool enable) {
	switch(f) {
		case kFeatureFullscreenMode:
			return;

		case kFeatureVirtualKeyboard:
			if (_hasSmartphoneResolution)
				return;
			_toolbarHighDrawn = false;
			if (enable) {
				_panelStateForced = true;
				if (!_toolbarHandler.visible()) swap_panel_visibility();
				//_saveToolbarState = _toolbarHandler.visible();
				_saveActiveToolbar = _toolbarHandler.activeName();
				_toolbarHandler.setActive(NAME_PANEL_KEYBOARD);
				_toolbarHandler.setVisible(true);
			}
			else
				if (_panelStateForced) {
					_panelStateForced = false;
					_toolbarHandler.setActive(_saveActiveToolbar);
					//_toolbarHandler.setVisible(_saveToolbarState);
				}
			return;

		case kFeatureDisableKeyFiltering:
			if (_hasSmartphoneResolution)
				_unfilteredkeys = enable;
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

void OSystem_WINCE3::check_mappings() {
	CEActionsPocket *instance;

	Common::String gameid(ConfMan.get("gameid"));

	if (gameid.empty() || GUI_Actions::Instance()->initialized())
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
	if (strncmp(gameid.c_str(), "zak", 3) == 0 &&
		!GUI_Actions::Instance()->getMapping(POCKET_ACTION_HIDE)) {
		GUI::MessageDialog alert("Don't forget to map a key to 'Hide Toolbar' action to see the whole inventory");
		alert.runModal();
	}

}

void OSystem_WINCE3::update_game_settings() {
	Common::String gameid(ConfMan.get("gameid"));

	// Finish panel initialization
	if (!_panelInitialized && !gameid.empty()) {
		Panel *panel;
		_panelInitialized = true;
		// Add the main panel
		panel = new Panel(0, 32);
		panel->setBackground(IMAGE_PANEL);
		// Save
		panel->add(NAME_ITEM_OPTIONS, new ItemAction(ITEM_OPTIONS, POCKET_ACTION_SAVE));
		// Skip
		panel->add(NAME_ITEM_SKIP, new ItemAction(ITEM_SKIP, POCKET_ACTION_SKIP));
		// sound
		panel->add(NAME_ITEM_SOUND, new ItemSwitch(ITEM_SOUND_OFF, ITEM_SOUND_ON, &_soundMaster));
		// bind keys
		panel->add(NAME_ITEM_BINDKEYS, new ItemAction(ITEM_BINDKEYS, POCKET_ACTION_BINDKEYS));
		// portrait/landscape - screen dependent
		// FIXME : will still display the portrait/landscape icon when using a scaler (but will be disabled)
		if (ConfMan.hasKey("landscape"))
			if (ConfMan.get("landscape")[0] > 57) {
				_newOrientation = _orientationLandscape = ConfMan.getBool("landscape");
				//ConfMan.removeKey("landscape", "");
				ConfMan.setInt("landscape", _orientationLandscape);
			} else
				_newOrientation = _orientationLandscape = ConfMan.getInt("landscape");
		else
			_newOrientation = _orientationLandscape = 0;
		panel->add(NAME_ITEM_ORIENTATION, new ItemSwitch(ITEM_VIEW_LANDSCAPE, ITEM_VIEW_PORTRAIT, &_newOrientation, 2));
		_toolbarHandler.add(NAME_MAIN_PANEL, *panel);
		_toolbarHandler.setActive(NAME_MAIN_PANEL);
		_toolbarHandler.setVisible(true);

		if (_videoMode.mode == GFX_NORMAL && ConfMan.hasKey("landscape") && ConfMan.getInt("landscape")) {
			setGraphicsMode(GFX_NORMAL);
			hotswapGFXMode();
		}

		if (_hasSmartphoneResolution)
			panel->setVisible(false);

		_saveToolbarState = true;
	}

	if (ConfMan.hasKey("no_doubletap_rightclick"))
		_noDoubleTapRMB = ConfMan.getBool("no_doubletap_rightclick");
	else if (gameid == "tinsel") {
		_noDoubleTapRMB = true;
		ConfMan.setBool("no_doubletap_rightclick", true);
		ConfMan.flushToDisk();
	}
}

void OSystem_WINCE3::initSize(uint w, uint h) {
	if (_hasSmartphoneResolution && h == 240)
		h = 200;  // mainly for the launcher

	if (_isSmartphone && !ConfMan.hasKey("landscape"))
	{
		ConfMan.setInt("landscape", 1);
		ConfMan.flushToDisk();
	}

	_canBeAspectScaled = false;
	if (w == 320 && h == 200 && !_hasSmartphoneResolution) {
		_canBeAspectScaled = true;
		h = 240; // use the extra 40 pixels height for the toolbar
	}

	if (h == 400)	// touche engine fixup
		h += 80;

	if (!_hasSmartphoneResolution)
		if (h == 240)
			_toolbarHandler.setOffset(200);
		else
			_toolbarHandler.setOffset(400);
	else
		if (h == 240)
			_toolbarHandler.setOffset(200);
		else	// 176x220
			_toolbarHandler.setOffset(0);

	if (w != (uint) _videoMode.screenWidth || h != (uint) _videoMode.screenHeight)
		_scalersChanged = false;

	_videoMode.overlayWidth = w;
	_videoMode.overlayHeight = h;

	OSystem_SDL::initSize(w, h);

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

void OSystem_WINCE3::setGraphicsModeIntern() {
	// Scalers have been pre-selected for the desired mode.
	// No further tuning required.
}

bool OSystem_WINCE3::update_scalers() {
	_videoMode.aspectRatioCorrection = false;

	if (CEDevice::hasPocketPCResolution()) {
		if (_videoMode.mode != GFX_NORMAL)
			return false;

		if ((!_orientationLandscape && (_videoMode.screenWidth == 320 || !_videoMode.screenWidth))
			|| CEDevice::hasSquareQVGAResolution() ) {
			if (getScreenWidth() != 320) {
				_scaleFactorXm = 3;
				_scaleFactorXd = 4;
				_scaleFactorYm = 1;
				_scaleFactorYd = 1;
				_scalerProc = PocketPCPortrait;
				_modeFlags = 0;
			} else {
				_scaleFactorXm = 1;
				_scaleFactorXd = 1;
				_scaleFactorYm = 1;
				_scaleFactorYd = 1;
				_scalerProc = Normal1x;
				_modeFlags = 0;
			}
		} else if ( _orientationLandscape && (_videoMode.screenWidth == 320 || !_videoMode.screenWidth)) {
			if (!_panelVisible && !_hasSmartphoneResolution  && !_overlayVisible && _canBeAspectScaled) {
				_scaleFactorXm = 1;
				_scaleFactorXd = 1;
				_scaleFactorYm = 6;
				_scaleFactorYd = 5;
				_scalerProc = PocketPCLandscapeAspect;
				_modeFlags = 0;
				_videoMode.aspectRatioCorrection = true;
			} else {
				_scaleFactorXm = 1;
				_scaleFactorXd = 1;
				_scaleFactorYm = 1;
				_scaleFactorYd = 1;
				_scalerProc = Normal1x;
				_modeFlags = 0;
			}
		} else	if (_videoMode.screenWidth == 640 && !(isOzone() && (getScreenWidth() >= 640 || getScreenHeight() >= 640))) {
			_scaleFactorXm = 1;
			_scaleFactorXd = 2;
			_scaleFactorYm = 1;
			_scaleFactorYd = 2;
			_scalerProc = PocketPCHalf;
			_modeFlags = 0;
		} else	if (_videoMode.screenWidth == 640 && (isOzone() && (getScreenWidth() >= 640 || getScreenHeight() >= 640))) {
			_scaleFactorXm = 1;
			_scaleFactorXd = 1;
			_scaleFactorYm = 1;
			_scaleFactorYd = 1;
			_scalerProc = Normal1x;
			_modeFlags = 0;
		}

		return true;
	} else if (CEDevice::hasWideResolution()) {
#ifdef USE_ARM_SCALER_ASM
		if ( _videoMode.mode == GFX_DOUBLESIZE && (_videoMode.screenWidth == 320 || !_videoMode.screenWidth) ) {
			if ( !_panelVisible && !_overlayVisible && _canBeAspectScaled ) {
				_scaleFactorXm = 2;
				_scaleFactorXd = 1;
				_scaleFactorYm = 12;
				_scaleFactorYd = 5;
				_scalerProc = Normal2xAspect;
				_modeFlags = 0;
				_videoMode.aspectRatioCorrection = true;
			} else if ( (_panelVisible || _overlayVisible) && _canBeAspectScaled ) {
				_scaleFactorXm = 2;
				_scaleFactorXd = 1;
				_scaleFactorYm = 2;
				_scaleFactorYd = 1;
				_scalerProc = Normal2x;
				_modeFlags = 0;
			}
			return true;
		}
#endif
	} else if (CEDevice::hasSmartphoneResolution()) {
		if (_videoMode.mode != GFX_NORMAL)
			return false;

		if (_videoMode.screenWidth > 320)
			error("Game resolution not supported on Smartphone");
#ifdef ARM
		_scaleFactorXm = 11;
		_scaleFactorXd = 16;
#else
		_scaleFactorXm = 2;
		_scaleFactorXd = 3;
#endif
		_scaleFactorYm = 7;
		_scaleFactorYd = 8;
		_scalerProc = SmartphoneLandscape;
		_modeFlags = 0;
		initZones();
		return true;
	}

	return false;
}

bool OSystem_WINCE3::setGraphicsMode(int mode) {

	Common::StackLock lock(_graphicsMutex);
	int oldScaleFactorXm = _scaleFactorXm;
	int oldScaleFactorXd = _scaleFactorXd;
	int oldScaleFactorYm = _scaleFactorYm;
	int oldScaleFactorYd = _scaleFactorYd;

	_scaleFactorXm = -1;
	_scaleFactorXd = -1;
	_scaleFactorYm = -1;
	_scaleFactorYd = -1;

	if (ConfMan.hasKey("landscape"))
		if (ConfMan.get("landscape")[0] > 57) {
			_newOrientation = _orientationLandscape = ConfMan.getBool("landscape");
			ConfMan.setInt("landscape", _orientationLandscape);
		} else
			_newOrientation = _orientationLandscape = ConfMan.getInt("landscape");
	else
		_newOrientation = _orientationLandscape = 0;

	update_scalers();

	if (isOzone() && (getScreenWidth() >= 640 || getScreenHeight() >= 640) && mode)
		_scaleFactorXm = -1;

	if (CEDevice::hasPocketPCResolution() && !CEDevice::hasWideResolution() && _orientationLandscape)
		_videoMode.mode = GFX_NORMAL;
	else
		_videoMode.mode = mode;

	if (_scaleFactorXm < 0) {
		/* Standard scalers, from the SDL backend */
		switch(_videoMode.mode) {
		case GFX_NORMAL:
			_videoMode.scaleFactor = 1;
			_scalerProc = Normal1x;
			break;
		case GFX_DOUBLESIZE:
			_videoMode.scaleFactor = 2;
			_scalerProc = Normal2x;
			break;
		case GFX_TRIPLESIZE:
			_videoMode.scaleFactor = 3;
			_scalerProc = Normal3x;
			break;
		case GFX_2XSAI:
			_videoMode.scaleFactor = 2;
			_scalerProc = _2xSaI;
			break;
		case GFX_SUPER2XSAI:
			_videoMode.scaleFactor = 2;
			_scalerProc = Super2xSaI;
			break;
		case GFX_SUPEREAGLE:
			_videoMode.scaleFactor = 2;
			_scalerProc = SuperEagle;
			break;
		case GFX_ADVMAME2X:
			_videoMode.scaleFactor = 2;
			_scalerProc = AdvMame2x;
			break;
		case GFX_ADVMAME3X:
			_videoMode.scaleFactor = 3;
			_scalerProc = AdvMame3x;
			break;
#ifndef DISABLE_HQ_SCALERS
		case GFX_HQ2X:
			_videoMode.scaleFactor = 2;
			_scalerProc = HQ2x;
			break;
		case GFX_HQ3X:
			_videoMode.scaleFactor = 3;
			_scalerProc = HQ3x;
			break;
#endif
		case GFX_TV2X:
			_videoMode.scaleFactor = 2;
			_scalerProc = TV2x;
			break;
		case GFX_DOTMATRIX:
			_videoMode.scaleFactor = 2;
			_scalerProc = DotMatrix;
			break;

		default:
			error("unknown gfx mode %d", mode);
		}
	}

	// Check if the scaler can be accepted, if not get back to normal scaler
	if (_videoMode.scaleFactor && ((_videoMode.scaleFactor * _videoMode.screenWidth > getScreenWidth() && _videoMode.scaleFactor * _videoMode.screenWidth > getScreenHeight())
		 || (_videoMode.scaleFactor * _videoMode.screenHeight > getScreenWidth() &&	_videoMode.scaleFactor * _videoMode.screenHeight > getScreenHeight()))) {
				_videoMode.scaleFactor = 1;
				_scalerProc = Normal1x;
	}

	// Common scaler system was used
	if (_scaleFactorXm < 0) {
		_scaleFactorXm = _videoMode.scaleFactor;
		_scaleFactorXd = 1;
		_scaleFactorYm = _videoMode.scaleFactor;
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

bool OSystem_WINCE3::loadGFXMode() {
	int displayWidth;
	int displayHeight;
	unsigned int flags = SDL_FULLSCREEN | SDL_SWSURFACE;

	_videoMode.fullscreen = true; // forced
	_forceFull = true;

	_tmpscreen = NULL;

	// Recompute scalers if necessary
	update_scalers();

	// Create the surface that contains the 8 bit game data
	_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.screenWidth, _videoMode.screenHeight, 8, 0, 0, 0, 0);
	if (_screen == NULL)
		error("_screen failed (%s)", SDL_GetError());

	// Create the surface that contains the scaled graphics in 16 bit mode
	// Always use full screen mode to have a "clean screen"
	if (!_videoMode.aspectRatioCorrection) {
		displayWidth = _videoMode.screenWidth * _scaleFactorXm / _scaleFactorXd;
		displayHeight = _videoMode.screenHeight * _scaleFactorYm / _scaleFactorYd;
	} else {
		displayWidth = _videoMode.screenWidth * _videoMode.scaleFactor;
		displayHeight = _videoMode.screenHeight* _videoMode.scaleFactor;
	}

	switch (_orientationLandscape) {
		case 1:
			flags |= SDL_LANDSCVIDEO;
			break;
		case 2:
			flags |= SDL_INVLNDVIDEO;
			break;
		default:
			flags |= SDL_PORTRTVIDEO;
	}
	_hwscreen = SDL_SetVideoMode(displayWidth, displayHeight, 16, flags);

	if (_hwscreen == NULL) {
		warning("SDL_SetVideoMode says we can't switch to that mode (%s)", SDL_GetError());
		quit();
	}

	// see what orientation sdl finally accepted
	if (_hwscreen->flags & SDL_PORTRTVIDEO)
		_orientationLandscape = _newOrientation	= 0;
	else if (_hwscreen->flags & SDL_LANDSCVIDEO)
		_orientationLandscape = _newOrientation	= 1;
	else
		_orientationLandscape = _newOrientation	= 2;

	// Create the surface used for the graphics in 16 bit before scaling, and also the overlay
	// Distinguish 555 and 565 mode
	if (_hwscreen->format->Rmask == 0x7C00)
		InitScalers(555);
	else
		InitScalers(565);
	_overlayFormat.bytesPerPixel = _hwscreen->format->BytesPerPixel;
	_overlayFormat.rLoss = _hwscreen->format->Rloss;
	_overlayFormat.gLoss = _hwscreen->format->Gloss;
	_overlayFormat.bLoss = _hwscreen->format->Bloss;
	_overlayFormat.aLoss = _hwscreen->format->Aloss;
	_overlayFormat.rShift = _hwscreen->format->Rshift;
	_overlayFormat.gShift = _hwscreen->format->Gshift;
	_overlayFormat.bShift = _hwscreen->format->Bshift;
	_overlayFormat.aShift = _hwscreen->format->Ashift;

	// Need some extra bytes around when using 2xSaI
	_tmpscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.screenWidth + 3, _videoMode.screenHeight + 3, 16, _hwscreen->format->Rmask, _hwscreen->format->Gmask, _hwscreen->format->Bmask, _hwscreen->format->Amask);

	if (_tmpscreen == NULL)
		error("_tmpscreen creation failed (%s)", SDL_GetError());

	// Overlay
	if (CEDevice::hasDesktopResolution()) {
		_overlayscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.overlayWidth * _scaleFactorXm / _scaleFactorXd, _videoMode.overlayHeight * _scaleFactorYm / _scaleFactorYd, 16, 0, 0, 0, 0);
		if (_overlayscreen == NULL)
			error("_overlayscreen failed (%s)", SDL_GetError());
		_tmpscreen2 = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.overlayWidth * _scaleFactorXm / _scaleFactorXd + 3, _videoMode.overlayHeight * _scaleFactorYm / _scaleFactorYd + 3, 16, 0, 0, 0, 0);
		if (_tmpscreen2 == NULL)
			error("_tmpscreen2 failed (%s)", SDL_GetError());
	} else {
		_overlayscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.overlayWidth, _videoMode.overlayHeight, 16, 0, 0, 0, 0);
		if (_overlayscreen == NULL)
			error("_overlayscreen failed (%s)", SDL_GetError());
		_tmpscreen2 = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.overlayWidth + 3, _videoMode.overlayHeight + 3, 16, 0, 0, 0, 0);
		if (_tmpscreen2 == NULL)
			error("_tmpscreen2 failed (%s)", SDL_GetError());
	}

	// Toolbar
	_toolbarHighDrawn = false;
	uint16 *toolbar_screen = (uint16 *)calloc(320 * 40, sizeof(uint16));	// *not* leaking memory here
	_toolbarLow = SDL_CreateRGBSurfaceFrom(toolbar_screen, 320, 40, 16, 320 * 2, _hwscreen->format->Rmask, _hwscreen->format->Gmask, _hwscreen->format->Bmask, _hwscreen->format->Amask);

	if (_toolbarLow == NULL)
		error("_toolbarLow failed (%s)", SDL_GetError());

	if (_videoMode.screenHeight > 240) {
		uint16 *toolbar_screen = (uint16 *)calloc(640 * 80, sizeof(uint16));
		_toolbarHigh = SDL_CreateRGBSurfaceFrom(toolbar_screen, 640, 80, 16, 640 * 2, _hwscreen->format->Rmask, _hwscreen->format->Gmask, _hwscreen->format->Bmask, _hwscreen->format->Amask);

		if (_toolbarHigh == NULL)
			error("_toolbarHigh failed (%s)", SDL_GetError());
	} else
		_toolbarHigh = NULL;


	// keyboard cursor control, some other better place for it?
	_km.x_max = _videoMode.screenWidth * _scaleFactorXm / _scaleFactorXd - 1;
	_km.y_max = _videoMode.screenHeight * _scaleFactorXm / _scaleFactorXd - 1;
	_km.delay_time = 25;
	_km.last_time = 0;

	return true;
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

bool OSystem_WINCE3::hotswapGFXMode() {
	if (!_screen)
		return false;

	// Keep around the old _screen & _tmpscreen so we can restore the screen data
	// after the mode switch. (also for the overlay)
	SDL_Surface *old_screen = _screen;
	SDL_Surface *old_tmpscreen = _tmpscreen;
	SDL_Surface *old_overlayscreen = _overlayscreen;
	SDL_Surface *old_tmpscreen2 = _tmpscreen2;

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
	if (!loadGFXMode()) {
		unloadGFXMode();

		_screen = old_screen;
		_overlayscreen = old_overlayscreen;

		return false;
	}

	// reset palette
	SDL_SetColors(_screen, _currentPalette, 0, 256);

	// Restore old screen content
	SDL_BlitSurface(old_screen, NULL, _screen, NULL);
	SDL_BlitSurface(old_tmpscreen, NULL, _tmpscreen, NULL);
	if (_overlayVisible) {
		SDL_BlitSurface(old_overlayscreen, NULL, _overlayscreen, NULL);
		SDL_BlitSurface(old_tmpscreen2, NULL, _tmpscreen2, NULL);
	}

	// Free the old surfaces
	SDL_FreeSurface(old_screen);
	SDL_FreeSurface(old_tmpscreen);
	SDL_FreeSurface(old_overlayscreen);
	SDL_FreeSurface(old_tmpscreen2);

	// Blit everything back to the screen
	_toolbarHighDrawn = false;
	internUpdateScreen();

	// Make sure that a Common::EVENT_SCREEN_CHANGED gets sent later -> FIXME this crashes when no game has been loaded.
//	_modeChanged = true;

	return true;
}

void OSystem_WINCE3::internUpdateScreen() {
	SDL_Surface *srcSurf, *origSurf;
	static bool old_overlayVisible = false;
	int numRectsOut = 0;
	int16 routx, routy, routw, routh, stretch, shakestretch;

	assert(_hwscreen != NULL);

	// bail if the application is minimized, be nice to OS
	if (!_hasfocus) {
		Sleep(20);
		return;
	}

	// If the shake position changed, fill the dirty area with blackness
	if (_currentShakePos != _newShakePos) {
		SDL_Rect blackrect = {0, 0, _videoMode.screenWidth * _scaleFactorXm / _scaleFactorXd, _newShakePos * _scaleFactorYm / _scaleFactorYd};
		if (_videoMode.aspectRatioCorrection)
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
		SDL_SetColors(_screen, _currentPalette + _paletteDirtyStart, _paletteDirtyStart, _paletteDirtyEnd - _paletteDirtyStart);
		_paletteDirtyEnd = 0;
		_forceFull = true;
	}

	if (!_overlayVisible) {
		origSurf = _screen;
		srcSurf = _tmpscreen;
	} else {
		origSurf = _overlayscreen;
		srcSurf = _tmpscreen2;
	}

	if (old_overlayVisible != _overlayVisible) {
		old_overlayVisible = _overlayVisible;
		update_scalers();
	}

	// Force a full redraw if requested
	if (_forceFull) {
		_numDirtyRects = 1;

		_dirtyRectList[0].x = 0;
		if (!_zoomDown)
			_dirtyRectList[0].y = 0;
		else
			_dirtyRectList[0].y = _videoMode.screenHeight / 2;
		_dirtyRectList[0].w = _videoMode.screenWidth;
		if (!_zoomUp && !_zoomDown)
			_dirtyRectList[0].h = _videoMode.screenHeight;
		else
			_dirtyRectList[0].h = _videoMode.screenHeight / 2;

		_toolbarHandler.forceRedraw();
	}

	// Only draw anything if necessary
	if (_numDirtyRects > 0) {

		SDL_Rect *r, *rout;
		SDL_Rect dst;
		uint32 srcPitch, dstPitch;
		SDL_Rect *last_rect = _dirtyRectList + _numDirtyRects;
		bool toolbarVisible = _toolbarHandler.visible();
		int toolbarOffset = _toolbarHandler.getOffset();

		for (r = _dirtyRectList; r != last_rect; ++r) {
			dst = *r;
			dst.x++;	// Shift rect by one since 2xSai needs to access the data around
			dst.y++;	// any pixel to scale it, and we want to avoid mem access crashes.
					// NOTE: This is also known as BLACK MAGIC, copied from the sdl backend
			if (SDL_BlitSurface(origSurf, r, srcSurf, &dst) != 0)
				error("SDL_BlitSurface failed: %s", SDL_GetError());
		}

		SDL_LockSurface(srcSurf);
		SDL_LockSurface(_hwscreen);

		srcPitch = srcSurf->pitch;
		dstPitch = _hwscreen->pitch;

		for (r = _dirtyRectList, rout = _dirtyRectOut; r != last_rect; ++r) {

			// always clamp to enclosing, downsampled-grid-aligned rect in the downscaled image
			if (_scaleFactorXd != 1) {
				stretch = r->x % _scaleFactorXd;
				r->x -= stretch;
				r->w += stretch;
				r->w = (r->x + r->w + _scaleFactorXd - 1) / _scaleFactorXd * _scaleFactorXd - r->x;
			}
			if (_scaleFactorYd != 1) {
				stretch = r->y % _scaleFactorYd;
				r->y -= stretch;
				r->h += stretch;
				r->h = (r->y + r->h + _scaleFactorYd - 1) / _scaleFactorYd * _scaleFactorYd - r->y;
			}

			// transform
			shakestretch = _currentShakePos * _scaleFactorYm / _scaleFactorYd;
			routx = r->x * _scaleFactorXm / _scaleFactorXd;					// locate position in scaled screen
			routy = r->y * _scaleFactorYm / _scaleFactorYd + shakestretch;	// adjust for shake offset
			routw = r->w * _scaleFactorXm / _scaleFactorXd;
			routh = r->h * _scaleFactorYm / _scaleFactorYd - shakestretch;

			// clipping destination rectangle inside device screen (more strict, also more tricky but more stable)
			// note that all current scalers do not make dst rect exceed left/right, unless chosen badly (FIXME)
			if (_zoomDown)	routy -= 240;			// adjust for zoom position
			if (routy + routh < 0)	continue;
			if (routy < 0) {
				routh += routy;
				r->y -= routy * _scaleFactorYd / _scaleFactorYm;
				routy = 0;
				r->h = routh * _scaleFactorYd / _scaleFactorYm;
			}
			if (_orientationLandscape) {
				if (routy > _platformScreenWidth)	continue;
				if (routy + routh > _platformScreenWidth) {
					routh = _platformScreenWidth - routy;
					r->h = routh * _scaleFactorYd / _scaleFactorYm;
				}
			} else {
				if (routy > _platformScreenHeight)	continue;
				if (routy + routh > _platformScreenHeight) {
					routh = _platformScreenHeight - routy;
					r->h = routh * _scaleFactorYd / _scaleFactorYm;
				}
			}

			// check if the toolbar is overwritten
			if (toolbarVisible && r->y + r->h >= toolbarOffset)
				_toolbarHandler.forceRedraw();

			// blit it (with added voodoo from the sdl backend, shifting the source rect again)
			_scalerProc(	(byte *)srcSurf->pixels + (r->x * 2 + 2)+ (r->y + 1) * srcPitch, srcPitch,
					(byte *)_hwscreen->pixels + routx * 2 + routy * dstPitch, dstPitch,
					r->w, r->h - _currentShakePos);

			// add this rect to output
			rout->x = routx;	rout->y = routy - shakestretch;
			rout->w = routw;	rout->h = routh + shakestretch;
			numRectsOut++;
			rout++;

		}
		SDL_UnlockSurface(srcSurf);
		SDL_UnlockSurface(_hwscreen);
	}
	// Add the toolbar if needed
	SDL_Rect toolbar_rect[1];
	if (_panelVisible && _toolbarHandler.draw(_toolbarLow, &toolbar_rect[0])) {
		// It can be drawn, scale it
		uint32 srcPitch, dstPitch;
		SDL_Surface *toolbarSurface;
		ScalerProc *toolbarScaler;

		if (_videoMode.screenHeight > 240) {
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

		drawToolbarMouse(toolbarSurface, true);		// draw toolbar mouse if applicable

		// Apply the appropriate scaler
		SDL_LockSurface(toolbarSurface);
		SDL_LockSurface(_hwscreen);
		srcPitch = toolbarSurface->pitch;
		dstPitch = _hwscreen->pitch;

		toolbarScaler = _scalerProc;
		if (_videoMode.scaleFactor == 2)
			toolbarScaler = Normal2x;
		else if (_videoMode.scaleFactor == 3)
			toolbarScaler = Normal3x;
		toolbarScaler((byte *)toolbarSurface->pixels, srcPitch,
					(byte *)_hwscreen->pixels + (_toolbarHandler.getOffset() * _scaleFactorYm / _scaleFactorYd * dstPitch),
					dstPitch, toolbar_rect[0].w, toolbar_rect[0].h);
		SDL_UnlockSurface(toolbarSurface);
		SDL_UnlockSurface(_hwscreen);

		// And blit it
		toolbar_rect[0].y = _toolbarHandler.getOffset();
		toolbar_rect[0].x = toolbar_rect[0].x * _scaleFactorXm / _scaleFactorXd;
		toolbar_rect[0].y = toolbar_rect[0].y * _scaleFactorYm / _scaleFactorYd;
		toolbar_rect[0].w = toolbar_rect[0].w * _scaleFactorXm / _scaleFactorXd;
		toolbar_rect[0].h = toolbar_rect[0].h * _scaleFactorYm / _scaleFactorYd;

		SDL_UpdateRects(_hwscreen, 1, toolbar_rect);

		drawToolbarMouse(toolbarSurface, false);	// undraw toolbar mouse
	}

	// Finally, blit all our changes to the screen
	if (numRectsOut > 0)
		SDL_UpdateRects(_hwscreen, numRectsOut, _dirtyRectOut);

	_numDirtyRects = 0;
	_forceFull = false;
}

Graphics::Surface *OSystem_WINCE3::lockScreen() {
	// Make sure mouse pointer is not painted over the playfield at the time of locking
	undrawMouse();
	return OSystem_SDL::lockScreen();
}

void OSystem_WINCE3::unlockScreen() {
	OSystem_SDL::unlockScreen();
}

bool OSystem_WINCE3::saveScreenshot(const char *filename) {
	assert(_hwscreen != NULL);

	Common::StackLock lock(_graphicsMutex);	// Lock the mutex until this function ends
	SDL_SaveBMP(_hwscreen, filename);
	return true;
}

void OSystem_WINCE3::copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h) {
	assert (_transactionMode == kTransactionNone);

	if (_overlayscreen == NULL)
		return;

	// Clip the coordinates
	if (x < 0) {
		w += x;
		buf -= x;
		x = 0;
	}

	if (y < 0) {
		h += y; buf -= y * pitch;
		y = 0;
	}

	if (w > _videoMode.overlayWidth - x) {
		w = _videoMode.overlayWidth - x;
	}

	if (h > _videoMode.overlayHeight - y) {
		h = _videoMode.overlayHeight - y;
	}

	if (w <= 0 || h <= 0)
		return;

	// Mark the modified region as dirty
	_cksumValid = false;
	addDirtyRect(x, y, w, h);

	undrawMouse();

	if (SDL_LockSurface(_overlayscreen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

	byte *dst = (byte *)_overlayscreen->pixels + y * _overlayscreen->pitch + x * 2;
	do {
		memcpy(dst, buf, w * 2);
		dst += _overlayscreen->pitch;
		buf += pitch;
	} while (--h);

	SDL_UnlockSurface(_overlayscreen);
}

void OSystem_WINCE3::copyRectToScreen(const byte *src, int pitch, int x, int y, int w, int h) {
	assert (_transactionMode == kTransactionNone);
	assert(src);

	if (_screen == NULL)
		return;

	Common::StackLock lock(_graphicsMutex);	// Lock the mutex until this function ends

	if (((long)src & 3) == 0 && pitch == _videoMode.screenWidth && x == 0 && y == 0 &&
			w == _videoMode.screenWidth && h == _videoMode.screenHeight && _modeFlags & DF_WANT_RECT_OPTIM) {
		/* Special, optimized case for full screen updates.
		 * It tries to determine what areas were actually changed,
		 * and just updates those, on the actual display. */
		addDirtyRgnAuto(src);
	} else {
		/* Clip the coordinates */
		if (x < 0) {
			w += x;
			src -= x;
			x = 0;
		}

		if (y < 0) {
			h += y;
			src -= y * pitch;
			y = 0;
		}

		if (w > _videoMode.screenWidth - x) {
			w = _videoMode.screenWidth - x;
		}

		if (h > _videoMode.screenHeight - y) {
			h = _videoMode.screenHeight - y;
		}

		if (w <= 0 || h <= 0)
			return;

		_cksumValid = false;
		addDirtyRect(x, y, w, h);
	}

	undrawMouse();

	// Try to lock the screen surface
	if (SDL_LockSurface(_screen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

	byte *dst = (byte *)_screen->pixels + y * _videoMode.screenWidth + x;

	if (_videoMode.screenWidth == pitch && pitch == w) {
		memcpy(dst, src, h*w);
	} else {
		do {
			memcpy(dst, src, w);
			src += pitch;
			dst += _videoMode.screenWidth;
		} while (--h);
	}

	// Unlock the screen surface
	SDL_UnlockSurface(_screen);
}

void OSystem_WINCE3::setMouseCursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y, byte keycolor, int cursorTargetScale) {

	undrawMouse();
	if (w == 0 || h == 0)
		return;

	_mouseCurState.w = w;
	_mouseCurState.h = h;

	_mouseHotspotX = hotspot_x;
	_mouseHotspotY = hotspot_y;

	_mouseKeyColor = keycolor;

	free(_mouseData);

	_mouseData = (byte *) malloc(w * h);
	memcpy(_mouseData, buf, w * h);

	if (w > _mouseBackupDim || h > _mouseBackupDim)
	{
		// mouse has been undrawn, adjust sprite backup area
		free(_mouseBackupOld);
		free(_mouseBackupToolbar);
		uint16 tmp = (w > h) ? w : h;
		_mouseBackupOld = (byte *) malloc(tmp * tmp * 2);	// can hold 8bpp (playfield) or 16bpp (overlay) data
		_mouseBackupToolbar = (uint16 *) malloc(tmp * tmp * 2); // 16 bpp
		_mouseBackupDim = tmp;
	}
}

void OSystem_WINCE3::setMousePos(int x, int y) {
	if (x != _mouseCurState.x || y != _mouseCurState.y) {
		undrawMouse();
		_mouseCurState.x = x;
		_mouseCurState.y = y;
		updateScreen();
	}
}


void OSystem_WINCE3::internDrawMouse() {
	if (!_mouseNeedsRedraw || !_mouseVisible || !_mouseData)
		return;

	int x = _mouseCurState.x - _mouseHotspotX;
	int y = _mouseCurState.y - _mouseHotspotY;
	int w = _mouseCurState.w;
	int h = _mouseCurState.h;
	byte color;
	const byte *src = _mouseData;		// Image representing the mouse
	int width;

	// clip the mouse rect, and adjust the src pointer accordingly
	if (x < 0) {
		w += x;
		src -= x;
		x = 0;
	}
	if (y < 0) {
		h += y;
		src -= y * _mouseCurState.w;
		y = 0;
	}

	if (w > _videoMode.screenWidth - x)
		w = _videoMode.screenWidth - x;
	if (h > _videoMode.screenHeight - y)
		h = _videoMode.screenHeight - y;

	// Quick check to see if anything has to be drawn at all
	if (w <= 0 || h <= 0)
		return;

	// Draw the mouse cursor; backup the covered area in "bak"
	if (SDL_LockSurface(_overlayVisible ? _overlayscreen : _screen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

	// Mark as dirty
	addDirtyRect(x, y, w, h);

	if (!_overlayVisible) {
		byte *bak = _mouseBackupOld;		// Surface used to backup the area obscured by the mouse
		byte *dst;					// Surface we are drawing into

		dst = (byte *)_screen->pixels + y * _videoMode.screenWidth + x;
		while (h > 0) {
			width = w;
			while (width > 0) {
				*bak++ = *dst;
				color = *src++;
				if (color != _mouseKeyColor)	// transparent, don't draw
					*dst = color;
				dst++;
				width--;
			}
			src += _mouseCurState.w - w;
			bak += _mouseBackupDim - w;
			dst += _videoMode.screenWidth - w;
			h--;
		}

	} else {
		uint16 *bak = (uint16 *)_mouseBackupOld;	// Surface used to backup the area obscured by the mouse
		byte *dst;					// Surface we are drawing into

		dst = (byte *)_overlayscreen->pixels + (y + 1) * _overlayscreen->pitch + (x + 1) * 2;
		while (h > 0) {
			width = w;
			while (width > 0) {
				*bak++ = *(uint16 *)dst;
				color = *src++;
				if (color != 0xFF)	// 0xFF = transparent, don't draw
					*(uint16 *)dst = SDL_MapRGB(_overlayscreen->format, _currentPalette[color].r, _currentPalette[color].g, _currentPalette[color].b);
				dst += 2;
				width--;
			}
			src += _mouseCurState.w - w;
			bak += _mouseBackupDim - w;
			dst += _overlayscreen->pitch - w * 2;
			h--;
		}
	}

	SDL_UnlockSurface(_overlayVisible ? _overlayscreen : _screen);

	// Finally, set the flag to indicate the mouse has been drawn
	_mouseNeedsRedraw = false;
}

void OSystem_WINCE3::undrawMouse() {
	assert (_transactionMode == kTransactionNone);

	if (_mouseNeedsRedraw)
		return;
	_mouseNeedsRedraw = true;

	int old_mouse_x = _mouseCurState.x - _mouseHotspotX;
	int old_mouse_y = _mouseCurState.y - _mouseHotspotY;
	int old_mouse_w = _mouseCurState.w;
	int old_mouse_h = _mouseCurState.h;

	// clip the mouse rect, and adjust the src pointer accordingly
	if (old_mouse_x < 0) {
		old_mouse_w += old_mouse_x;
		old_mouse_x = 0;
	}
	if (old_mouse_y < 0) {
		old_mouse_h += old_mouse_y;
		old_mouse_y = 0;
	}

	if (old_mouse_w > _videoMode.screenWidth - old_mouse_x)
		old_mouse_w = _videoMode.screenWidth - old_mouse_x;
	if (old_mouse_h > _videoMode.screenHeight - old_mouse_y)
		old_mouse_h = _videoMode.screenHeight - old_mouse_y;

	// Quick check to see if anything has to be drawn at all
	if (old_mouse_w <= 0 || old_mouse_h <= 0)
		return;


	if (SDL_LockSurface(_overlayVisible ? _overlayscreen : _screen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

	int x, y;
	if (!_overlayVisible) {
		byte *dst, *bak = _mouseBackupOld;

		// No need to do clipping here, since drawMouse() did that already
		dst = (byte *)_screen->pixels + old_mouse_y * _videoMode.screenWidth + old_mouse_x;
		for (y = 0; y < old_mouse_h; ++y, bak += _mouseBackupDim, dst += _videoMode.screenWidth)
			memcpy(dst, bak, old_mouse_w);
	} else {
		byte *dst;
		uint16 *bak = (uint16 *)_mouseBackupOld;

		// No need to do clipping here, since drawMouse() did that already
		dst = (byte *)_overlayscreen->pixels + (old_mouse_y + 1) * _overlayscreen->pitch + (old_mouse_x + 1) * 2;
		for (y = 0; y < old_mouse_h; ++y, bak += _mouseBackupDim, dst += _overlayscreen->pitch)
			memcpy(dst, bak, old_mouse_w << 1);
	}

	addDirtyRect(old_mouse_x, old_mouse_y, old_mouse_w, old_mouse_h);

	SDL_UnlockSurface(_overlayVisible ? _overlayscreen : _screen);
}

void OSystem_WINCE3::drawToolbarMouse(SDL_Surface *surf, bool draw) {

	if (!_mouseData || !_usesEmulatedMouse) return;

	int x = _mouseCurState.x - _mouseHotspotX;
	int y = _mouseCurState.y - _mouseHotspotY - _toolbarHandler.getOffset();
	int w = _mouseCurState.w;
	int h = _mouseCurState.h;
	byte color;
	const byte *src = _mouseData;
	int width;

	// clip
	if (x < 0) {
		w += x;
		src -= x;
		x = 0;
	}
	if (y < 0) {
		h += y;
		src -= y * _mouseCurState.w;
		y = 0;
	}
	if (w > surf->w - x)
		w = surf->w - x;
	if (h > surf->h - y)
		h = surf->h - y;
	if (w <= 0 || h <= 0)
		return;

	if (SDL_LockSurface(surf) == -1)
		error("SDL_LockSurface failed at internDrawToolbarMouse: %s", SDL_GetError());

	uint16 *bak = _mouseBackupToolbar;	// toolbar surfaces are 16bpp
	uint16 *dst;
	dst = (uint16 *)surf->pixels + y * surf->w + x;

	if (draw) {		// blit it
		while (h > 0) {
			width = w;
			while (width > 0) {
				*bak++ = *dst;
				color = *src++;
				if (color != _mouseKeyColor)	// transparent color
					*dst = 0xFFFF;
				dst++;
				width--;
			}
			src += _mouseCurState.w - w;
			bak += _mouseBackupDim - w;
			dst += surf->w - w;
			h--;
		}
	} else {		// restore bg
		for (y = 0; y < h; ++y, bak += _mouseBackupDim, dst += surf->w)
			memcpy(dst, bak, w << 1);
	}

	SDL_UnlockSurface(surf);
}

void OSystem_WINCE3::blitCursor() {
}

void OSystem_WINCE3::showOverlay() {
	assert (_transactionMode == kTransactionNone);

	if (_overlayVisible)
		return;

	undrawMouse();
	_overlayVisible = true;
	update_scalers();
	clearOverlay();
}

void OSystem_WINCE3::hideOverlay() {
	assert (_transactionMode == kTransactionNone);

	if (!_overlayVisible)
		return;

	undrawMouse();
	_overlayVisible = false;
	clearOverlay();
	_forceFull = true;
}

void OSystem_WINCE3::drawMouse() {
	if (!(_toolbarHandler.visible() && _mouseCurState.y >= _toolbarHandler.getOffset() && !_usesEmulatedMouse) && !_forceHideMouse)
		internDrawMouse();
}

void OSystem_WINCE3::fillMouseEvent(Common::Event &event, int x, int y) {
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

	if (_forceFull || _paletteDirtyEnd) return;

	OSystem_SDL::addDirtyRect(x, y, w, h, false);
}

static int mapKeyCE(SDLKey key, SDLMod mod, Uint16 unicode, bool unfilter)
{
	if (GUI::Actions::Instance()->mappingActive())
		return key;

	if (unfilter) {
		switch (key) {
			case SDLK_ESCAPE:
				return SDLK_BACKSPACE;
			case SDLK_F8:
				return SDLK_ASTERISK;
			case SDLK_F9:
				return SDLK_HASH;
		}
		return key;
	}

	if (key >= SDLK_KP0 && key <= SDLK_KP9) {
		return key - SDLK_KP0 + '0';
	} else if (key >= SDLK_UP && key <= SDLK_PAGEDOWN) {
		return key;
	} else if (key >= SDLK_NUMLOCK && key <= SDLK_EURO) {
		return 0;
	}
	return key;
}

bool OSystem_WINCE3::pollEvent(Common::Event &event) {
	SDL_Event ev;
	byte b = 0;
	DWORD currentTime;
	bool keyEvent = false;
	int deltaX, deltaY;

	memset(&event, 0, sizeof(Common::Event));

	handleKbdMouse();

	// If the screen mode changed, send an Common::EVENT_SCREEN_CHANGED
	if (_modeChanged) {
		_modeChanged = false;
		event.type = Common::EVENT_SCREEN_CHANGED;
		_screenChangeCount++;
		return true;
	}

	CEDevice::wakeUp();

	currentTime = GetTickCount();

	while (SDL_PollEvent(&ev)) {
		switch(ev.type) {
		case SDL_KEYDOWN:
			debug(1, "Key down %X %s", ev.key.keysym.sym, SDL_GetKeyName((SDLKey)ev.key.keysym.sym));
			// KMOD_RESERVED is used if the key has been injected by an external buffer
			if (ev.key.keysym.mod != KMOD_RESERVED && !_unfilteredkeys) {
				keyEvent = true;
				_lastKeyPressed = ev.key.keysym.sym;
				_keyRepeatTime = currentTime;
				_keyRepeat = 0;

				if (!GUI_Actions::Instance()->mappingActive() && GUI_Actions::Instance()->performMapped(ev.key.keysym.sym, true))
					return true;
			}

			if (GUI_Actions::Instance()->mappingActive())
				event.kbd.flags = 0xFF;
			else if (ev.key.keysym.sym == SDLK_PAUSE) {
				_lastKeyPressed = 0;
				event.type = Common::EVENT_PREDICTIVE_DIALOG;
				return true;
			} 			event.type = Common::EVENT_KEYDOWN;
			if (!_unfilteredkeys)
				event.kbd.keycode = (Common::KeyCode)ev.key.keysym.sym;
			else
				event.kbd.keycode = (Common::KeyCode)mapKeyCE(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode, _unfilteredkeys);
			event.kbd.ascii = mapKeyCE(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode, _unfilteredkeys);

			if (ev.key.keysym.mod == KMOD_RESERVED && ev.key.keysym.unicode == KMOD_SHIFT) {
				event.kbd.ascii ^= 0x20;
				event.kbd.flags = Common::KBD_SHIFT;
			}

			return true;

		case SDL_KEYUP:
			debug(1, "Key up %X %s", ev.key.keysym.sym, SDL_GetKeyName((SDLKey)ev.key.keysym.sym));
			// KMOD_RESERVED is used if the key has been injected by an external buffer
			if (ev.key.keysym.mod != KMOD_RESERVED && !_unfilteredkeys) {
				keyEvent = true;
				_lastKeyPressed = 0;

				if (!GUI_Actions::Instance()->mappingActive() && GUI_Actions::Instance()->performMapped(ev.key.keysym.sym, false))
					return true;
			}

			if (GUI_Actions::Instance()->mappingActive())
				event.kbd.flags = 0xFF;
			else if (ev.key.keysym.sym == SDLK_PAUSE) {
				_lastKeyPressed = 0;
				return false;	// chew up the show agi dialog key up event
			}

			event.type = Common::EVENT_KEYUP;
			if (!_unfilteredkeys)
				event.kbd.keycode = (Common::KeyCode)ev.key.keysym.sym;
			else
				event.kbd.keycode = (Common::KeyCode)mapKeyCE(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode, _unfilteredkeys);
			event.kbd.ascii = mapKeyCE(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode, _unfilteredkeys);

			if (ev.key.keysym.mod == KMOD_RESERVED && ev.key.keysym.unicode == KMOD_SHIFT) {
				event.kbd.ascii ^= 0x20;
				event.kbd.flags = Common::KBD_SHIFT;
			}

			return true;

		case SDL_MOUSEMOTION:
			event.type = Common::EVENT_MOUSEMOVE;
			fillMouseEvent(event, ev.motion.x, ev.motion.y);
			setMousePos(event.mouse.x, event.mouse.y);
			return true;

		case SDL_MOUSEBUTTONDOWN:
			if (ev.button.button == SDL_BUTTON_LEFT)
				event.type = Common::EVENT_LBUTTONDOWN;
			else if (ev.button.button == SDL_BUTTON_RIGHT)
				event.type = Common::EVENT_RBUTTONDOWN;
			else
				break;
			fillMouseEvent(event, ev.button.x, ev.button.y);


			if (event.mouse.x > _tapX)
				deltaX = event.mouse.x - _tapX;
			else
				deltaX = _tapX - event.mouse.x;
			if (event.mouse.y > _tapY)
				deltaY = event.mouse.y - _tapY;
			else
				deltaY = _tapY - event.mouse.y;
			_closeClick = (deltaX <= 5 && deltaY <= 5);

			if (!_isSmartphone) {
				// handle double-taps
				if (_tapTime) {		// second tap
					if (_closeClick && (GetTickCount() - _tapTime < 1000)) {
						if (event.mouse.y <= 20 && _panelInitialized) {		// top of screen (show panel)
							swap_panel_visibility();
						} else if (!_noDoubleTapRMB) {		// right click
							event.type = Common::EVENT_RBUTTONDOWN;
							_rbutton = true;
						}
					}
					_tapTime = 0;
				} else {
					_tapTime = GetTickCount();
					_tapX = event.mouse.x;
					_tapY = event.mouse.y;
				}
			}

			if (_freeLook && !_closeClick) {
				_rbutton = false;
				_tapTime = 0;
				_tapX = event.mouse.x;
				_tapY = event.mouse.y;
				event.type = Common::EVENT_MOUSEMOVE;
				setMousePos(event.mouse.x, event.mouse.y);
			}


			if (_toolbarHandler.action(event.mouse.x, event.mouse.y, true)) {
				if (!_toolbarHandler.drawn()) {
					_toolbarHighDrawn = false;
					internUpdateScreen();
				}
				if (_newOrientation != _orientationLandscape){
					_orientationLandscape = _newOrientation;
					_toolbarHighDrawn = false;
					ConfMan.setInt("landscape", _orientationLandscape);
					ConfMan.flushToDisk();
					hotswapGFXMode();
				}
				return false;
			}

			return true;

		case SDL_MOUSEBUTTONUP:
			if (ev.button.button == SDL_BUTTON_LEFT)
				event.type = Common::EVENT_LBUTTONUP;
			else if (ev.button.button == SDL_BUTTON_RIGHT)
				event.type = Common::EVENT_RBUTTONUP;
			else
				break;

			if (_rbutton) {
				event.type = Common::EVENT_RBUTTONUP;
				_rbutton = false;
			}

			fillMouseEvent(event, ev.button.x, ev.button.y);

			if (_freeLook && !_closeClick) {
				_tapX = event.mouse.x;
				_tapY = event.mouse.y;
				event.type = Common::EVENT_MOUSEMOVE;
				setMousePos(event.mouse.x, event.mouse.y);
			}

			if (_toolbarHandler.action(event.mouse.x, event.mouse.y, false)) {
				if (!_toolbarHandler.drawn())
					_toolbarHighDrawn = false;
					internUpdateScreen();
			}
			return true;

		case SDL_VIDEOEXPOSE:
			_forceFull = true;
			break;

		case SDL_QUIT:
			event.type = Common::EVENT_QUIT;
			return true;

		case SDL_ACTIVEEVENT:
			if (ev.active.state & SDL_APPMOUSEFOCUS)
				debug(2, "%s mouse focus.", ev.active.gain ? "Got" : "Lost");
			if (ev.active.state & SDL_APPINPUTFOCUS)
				debug(2, "%s input focus.", ev.active.gain ? "Got" : "Lost");
			if (ev.active.state & SDL_APPACTIVE)
				debug(2, "%s total focus.", ev.active.gain ? "Got" : "Lost");
			if (ev.active.state & SDL_APPINPUTFOCUS) {
				_hasfocus = ev.active.gain;
				SDL_PauseAudio(!_hasfocus);
				_forceFull |= _hasfocus;
			}
			break;
		}
	}

	// Simulate repeated key for backend
	if (!keyEvent && _lastKeyPressed && currentTime > _keyRepeatTime + _keyRepeatTrigger) {
		_keyRepeatTime = currentTime;
		_keyRepeat++;
		GUI_Actions::Instance()->performMapped(_lastKeyPressed, true);
	}

	return false;
}

void OSystem_WINCE3::quit() {
	fclose(stdout_file);
	fclose(stderr_file);
	if (gDebugLevel <= 0) {
		DeleteFile(ASCIItoUnicode(stdout_fname));
		DeleteFile(ASCIItoUnicode(stderr_fname));
	}
	CEDevice::end();
	OSystem_SDL::quit();
}

void OSystem_WINCE3::getTimeAndDate(struct tm &t) const {
	SYSTEMTIME systime;

	GetLocalTime(&systime);
	t.tm_year	= systime.wYear - 1900;
	t.tm_mon	= systime.wMonth - 1;
	t.tm_wday	= systime.wDayOfWeek;
	t.tm_mday	= systime.wDay;
	t.tm_hour	= systime.wHour;
	t.tm_min	= systime.wMinute;
	t.tm_sec	= systime.wSecond;
}

int OSystem_WINCE3::_platformScreenWidth;
int OSystem_WINCE3::_platformScreenHeight;
bool OSystem_WINCE3::_isOzone;
OSystem_WINCE3::zoneDesc OSystem_WINCE3::_zones[TOTAL_ZONES] = {
        { 0, 0, 320, 145 },
        { 0, 145, 150, 55 },
        { 150, 145, 170, 55 }
};
