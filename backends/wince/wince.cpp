/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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

#include "wince.h"

#ifdef USE_VORBIS
#include <vorbis/vorbisfile.h>
#endif

#if _WIN32_WCE >= 300

#include <Aygshell.h>

#else

// Put in include file

typedef enum tagSIPSTATE
{
    SIP_UP = 0,
    SIP_DOWN,
	SIP_FORCEDOWN,
    SIP_UNCHANGED,
    SIP_INPUTDIALOG,
} SIPSTATE;

#define SHFS_SHOWTASKBAR            0x0001
#define SHFS_HIDETASKBAR            0x0002
#define SHFS_SHOWSIPBUTTON          0x0004
#define SHFS_HIDESIPBUTTON          0x0008
#define SHFS_SHOWSTARTICON          0x0010
#define SHFS_HIDESTARTICON          0x0020

typedef struct
{
    DWORD cbSize;
    HWND hwndLastFocus;
    UINT fSipUp :1;
    UINT fSipOnDeactivation :1;
    UINT fActive :1;
    UINT fReserved :29;
} SHACTIVATEINFO, *PSHACTIVATEINFO;

#endif


#define CURRENT_KEYS_VERSION 3

typedef int (*tTimeCallback)(int);
typedef void SoundProc(void *param, byte *buf, int len);

// Dynamically linked Aygshell
typedef BOOL (*tSHFullScreen)(HWND,DWORD);
//typedef BOOL (WINSHELLAPI *tSHHandleWMSettingChange)(HWND,WPARAM,LPARAM,SHACTIVATEINFO*);
typedef BOOL (*tSHSipPreference)(HWND,SIPSTATE);
typedef BOOL (*tSHCreateMenuBar)(void*);

/*
// Dynamically linked SDLAudio
typedef void (*tSDL_AudioQuit)();
typedef int (*tSDL_Init)(Uint32);
typedef void (*tSDL_PauseAudio)(int);
typedef int (*tSDL_OpenAudio)(SDL_AudioSpec*, SDL_AudioSpec*);
*/

// GAPI "emulation"
typedef struct pseudoGAPI {
	const TCHAR *device;
	void *buffer;
	int xWidth;
	int yHeight;
	int xPitch;
	int yPitch;
	int BPP;
	int format;
} pseudoGAPI;

typedef struct {
	int x, y, w, h;
} dirty_square;

#define AddDirtyRect(xi,yi,wi,hi) 				\
  if (num_of_dirty_square < MAX_NUMBER_OF_DIRTY_SQUARES) {	\
	if (smartphone) { /* Align dirty rect for smartphone */ \
		register int offsetX = xi % 3; \
		register int offsetY = yi % 8; \
		register int newX = (xi < 3 ? 0 : xi - offsetX); \
		register int newY = (yi < 8 ? 0 : yi - offsetY); \
	    ds[num_of_dirty_square].x = newX; \
	    ds[num_of_dirty_square].y = newY;				\
	    ds[num_of_dirty_square].w = wi + offsetX;				\
	    ds[num_of_dirty_square].h = hi + offsetY;				\
	} \
	else {											\
		ds[num_of_dirty_square].x = xi;             \
		ds[num_of_dirty_square].y = yi;				\
		ds[num_of_dirty_square].w = wi;				\
		ds[num_of_dirty_square].h = hi;				\
	} \
    num_of_dirty_square++;					    \
  }


/* Hardcode the video buffer for some devices for which there is no GAPI */
/* and no GameX support */

pseudoGAPI availablePseudoGAPI[] = {
	{ TEXT("HP, Jornada 710"),
      (void*)0x82200000,
	  640,
	  240,
	  2,
	  1280,
	  16,
      0xA8
	},
	{ TEXT("HP, Jornada 720"),
      (void*)0x82200000,
	  640,
	  240,
	  2,
	  1280,
	  16,
      0xA8
	},
	{ TEXT("Compaq iPAQ H3600"),   /* this is just a test for my device :) */
	  (void*)0xAC05029E,
	  240,
	  320,
	  640,
	  -2,
	  16,
	  0xA8
	},
	{ TEXT("ORG_FR"),			   /* smartphone SPV - more tests :) */
	  (void*)0x46000020,
	  176,
	  220,
	  2,
	  352,
	  16,
	  0xA0
	},
	{ 0, 0, 0, 0, 0, 0, 0, 0 }
};

OSystem_WINCE3 *mainClass;

int _pseudoGAPI_device;

int _thread_priority;

bool canCacheGAPIBuffer;


extern char noGAPI;

extern bool wide_screen;
extern bool extra_wide_screen;

extern float _screen_factor;

/* Default SDLAUDIO */

/*

void defaultSDL_AudioQuit() {
}

int defaultSDL_Init(Uint32 flags) {
	return 0;
}

void defaultSDL_PauseAudio(int pause) {
}

int defaultSDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained) {
	return 0;
}

*/

/* Default AYGSHELL */

BOOL defaultSHFullScreen(HWND handle, DWORD action) {
	if ((action & SHFS_HIDETASKBAR) != 0 || (action & SHFS_SHOWTASKBAR) != 0) {
		// Hide taskbar, WinCE 2.x style - from EasyCE
		HKEY hKey=0;
		DWORD dwValue = 0;
		unsigned long lSize = sizeof( DWORD );
		DWORD dwType = REG_DWORD;
		MSG msg;

		
		RegOpenKeyEx( HKEY_LOCAL_MACHINE, TEXT("\\software\\microsoft\\shell"), 0, KEY_ALL_ACCESS, &hKey );
		RegQueryValueEx( hKey, TEXT("TBOpt"), 0, &dwType, (BYTE*)&dwValue, &lSize );
		if ((action & SHFS_SHOWTASKBAR) != 0)
			 dwValue &= 0xFFFFFFFF - 8;// reset bit to show taskbar
	 	else dwValue |= 8;			   // set bit to hide taskbar
		RegSetValueEx( hKey, TEXT("TBOpt"), 0, REG_DWORD, (BYTE*)&dwValue, lSize );
		msg.hwnd = FindWindow( TEXT("HHTaskBar"), NULL );
		SendMessage( msg.hwnd, WM_COMMAND, 0x03EA, 0 );
		if (handle)
			SetForegroundWindow( handle );
	}

	return TRUE;
}

/*
BOOL WINSHELLAPI defaultSHHandleWMSettingChange(HWND handle, WPARAM param1, LPARAM param2, SHACTIVATEINFO *info) {
	return TRUE;
}
*/

BOOL defaultSHSipPreference(HWND handle, SIPSTATE state) {
	return TRUE;
}

/* Default GAPI */

int defaultGXOpenDisplay(HWND hWnd, DWORD dwFlags) {
	return GAPI_SIMU;
}

int defaultGXCloseDisplay() {
	return 0;
}


void* defaultGXBeginDraw() {
	return availablePseudoGAPI[_pseudoGAPI_device].buffer;
}

int defaultGXEndDraw() {
	return 0;
}

int defaultGXOpenInput() {
	return 0;
}

int defaultGXCloseInput() {
	return 0;
}

GXDisplayProperties defaultGXGetDisplayProperties() {
	GXDisplayProperties result;

	result.cxWidth = availablePseudoGAPI[_pseudoGAPI_device].xWidth;
	result.cyHeight = availablePseudoGAPI[_pseudoGAPI_device].yHeight;
	result.cbxPitch = availablePseudoGAPI[_pseudoGAPI_device].xPitch;
	result.cbyPitch = availablePseudoGAPI[_pseudoGAPI_device].yPitch;
	result.cBPP = availablePseudoGAPI[_pseudoGAPI_device].BPP;
	result.ffFormat = availablePseudoGAPI[_pseudoGAPI_device].format;

	return result;
}

GXKeyList defaultGXGetDefaultKeys(int options) {
	GXKeyList result;

	memset(&result, 0xff, sizeof(result));

	return result;
}

int defaultGXSuspend() {
	return 0;
}

int defaultGXResume() {
	return 0;
}

/* GAMEX GAPI emulation */

#ifdef GAMEX

GameX *gameX;

int gameXGXOpenDisplay(HWND hWnd, DWORD dwFlags) {
	gameX = new GameX();
	if (!gameX || !gameX->OpenGraphics() || !gameX->GetFBAddress()) {
		//MessageBox(NULL, TEXT("Couldn't initialize GameX. Reverting to GDI graphics"), TEXT("PocketScumm rendering"), MB_OK);
		noGAPI = 1;
	}
	return 0;
}

int gameXGXCloseDisplay() {
	gameX->CloseGraphics();
	delete gameX;
	return 0;
}


void* gameXGXBeginDraw() {
	gameX->BeginDraw();
	return (gameX->GetFBAddress());
}

int gameXGXEndDraw() {
	return gameX->EndDraw();
}

int gameXGXOpenInput() {
	return 0;
}

int gameXGXCloseInput() {
	return 0;
}

GXDisplayProperties gameXGXGetDisplayProperties() {
	GXDisplayProperties result;

	result.cBPP = gameX->GetFBBpp();
	if (result.cBPP == 16)
		result.cbxPitch = 2;
	else
		result.cbxPitch = 1;
	result.cbyPitch = gameX->GetFBModulo();

	return result;
}

GXKeyList gameXGXGetDefaultKeys(int options) {
	GXKeyList result;

	memset(&result, 0xff, sizeof(result));

	return result;
}

int gameXGXSuspend() {
	return 0;
}

int gameXGXResume() {
	return 0;
}

#endif

GameDetector detector;
Engine *engine;
bool is_simon;
bool is_bass;
NewGui *g_gui;
extern Scumm *g_scumm;
//extern SimonEngine *g_simon;
//OSystem *g_system;
//SoundMixer *g_mixer;
Config *g_config;
tTimeCallback timer_callback;
int timer_interval;

tSHFullScreen dynamicSHFullScreen = NULL;
//tSHHandleWMSettingChange dynamicSHHandleWMSettingChange = NULL;
tSHSipPreference dynamicSHSipPreference = NULL;
tSHCreateMenuBar dynamicSHCreateMenuBar = NULL;
tGXOpenInput dynamicGXOpenInput = NULL;
tGXGetDefaultKeys dynamicGXGetDefaultKeys = NULL;
tGXCloseDisplay dynamicGXCloseDisplay = NULL;
tGXCloseInput dynamicGXCloseInput = NULL;
tGXSuspend dynamicGXSuspend = NULL;
tGXResume dynamicGXResume = NULL;
tGXGetDisplayProperties dynamicGXGetDisplayProperties = NULL;
tGXOpenDisplay dynamicGXOpenDisplay = NULL;
tGXEndDraw dynamicGXEndDrawInternal = NULL;
tGXBeginDraw dynamicGXBeginDrawInternal = NULL;
tGXEndDraw dynamicGXEndDraw = NULL;
tGXBeginDraw dynamicGXBeginDraw = NULL;


extern void Cls();

extern BOOL isPrescanning();
extern void changeScanPath();
extern void startScan();
extern void endScanPath();
extern void abortScanPath();

void load_key_mapping();
void keypad_init();

extern void Cls();

extern BOOL isPrescanning();
extern void changeScanPath();
extern void startScan();
extern void endScanPath();
extern void abortScanPath();

void keypad_init();

/************* WinCE Specifics *****************/
byte veryFastMode;

bool sound_activated, terminated;
HWND hWnd_MainMenu;
HWND hWnd_Window;

void drawAllToolbar(bool);
void redrawSoundItem();

extern bool toolbar_drawn;
extern bool draw_keyboard;
bool hide_toolbar;
bool hide_cursor;
bool save_hide_cursor;
bool freelook;
bool save_hide_toolbar;
bool keyboard_override;

bool _get_key_mapping;
static char _directory[MAX_PATH];
bool select_game;
bool need_GAPI;
char is_demo;

bool gfx_mode_switch;

dirty_square ds[MAX_NUMBER_OF_DIRTY_SQUARES];
int num_of_dirty_square;


SoundProc *real_soundproc;

extern void startFindGame();
extern void displayGameInfo();
extern bool loadGameSettings();
extern void setFindGameDlgHandle(HWND);
extern void getSelectedGame(int, char*, TCHAR*, char*);
extern void runGame(char*);

extern void palette_update();

extern void own_soundProc(void *buffer, byte *samples, int len);

extern int chooseGame();

//#define SHMenuBar_GetMenu(hWndMB,ID_MENU) (HMENU)SendMessage((hWndMB), SHCMBM_GETSUBMENU, (WPARAM)0, (LPARAM)ID_MENU)

/* Monkey Island 1 and 2 keyboard stuff (copy protection) */
bool monkey_keyboard;

bool new_audio_rate;
bool FM_high_quality;

bool closing = false;

/* Check platform */
bool smartphone = false;
bool high_res = false;

void close_GAPI() {
	g_config->setBool("Sound", sound_activated, "wince");
	g_config->setInt("DisplayMode", GetScreenMode(), "wince");
	g_config->flush();
	dynamicSHFullScreen(hWnd_Window, SHFS_SHOWTASKBAR | SHFS_SHOWSIPBUTTON | SHFS_SHOWSTARTICON);
	dynamicGXCloseInput();
	dynamicGXCloseDisplay();
	SDL_AudioQuit();
	UpdateWindow(hWnd_Window);
	closing = true;
}

void do_quit() {
	close_GAPI();
	exit(1);
}

void Error(LPCTSTR msg)
{
	OutputDebugString(msg);
	MessageBox(HWND_DESKTOP, msg, TEXT("Error"), MB_ICONSTOP);
	exit(1);
}

void Warning(LPCTSTR msg)
{
	OutputDebugString(msg);
	MessageBox(HWND_DESKTOP, msg, TEXT("Error"), MB_ICONSTOP);	
}

int mapKey(int key) {
	if (key>=VK_F1 && key<=VK_F9) {
		return key - VK_F1 + 315;
	}
	return key;
}


#define IMPORT(Handle,Variable,Type,Function, Replace) \
	Variable = (Type)GetProcAddress(Handle, TEXT(Function)); \
	if (!Variable) { \
		if (Replace) { \
			Variable = Replace; \
		} \
		else { \
			MessageBox(NULL, TEXT(Function), TEXT("Error importing DLL function"), MB_OK); \
			exit(1); \
		} \
	}


void* dynamicGXBeginDrawCached() {
	static bool checked = false;
	static void* buffer;

	if (!checked) {
		checked = true;
		canCacheGAPIBuffer = !dynamicGXIsDisplayDRAMBuffer();
		if (canCacheGAPIBuffer) {
			buffer = dynamicGXBeginDrawInternal();
			return buffer;
		}
		else {
			dynamicGXBeginDraw = dynamicGXBeginDrawInternal;
			return dynamicGXBeginDrawInternal();
		}		
	}

	return buffer;
}

int dynamicGXEndDrawCached() {
	if (!canCacheGAPIBuffer)
		return dynamicGXEndDrawInternal();

	return 1;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{

	TCHAR directory[MAX_PATH];
	char game_name[100];
	bool sound;
	int version;
	int result;
	
	HMODULE aygshell_handle;
	//HMODULE SDLAudio_handle;
	HMODULE GAPI_handle;

	// Check platform
	smartphone = (GetSystemMetrics(SM_CXSCREEN) < 320 && GetSystemMetrics(SM_CYSCREEN) < 320); 

	hide_toolbar = false;
	freelook = false;
	noGAPI = 0;

	g_config = new Config("scummvm.ini", "scummvm");
	g_config->set_writing(true);

	// See if we're running on a Windows CE version supporting aygshell
	aygshell_handle = LoadLibrary(TEXT("aygshell.dll"));
	if (aygshell_handle) {
		IMPORT(aygshell_handle, dynamicSHFullScreen, tSHFullScreen, "SHFullScreen", defaultSHFullScreen)
		IMPORT(aygshell_handle, dynamicSHSipPreference, tSHSipPreference, "SHSipPreference", NULL)
		if (smartphone) {
			IMPORT(aygshell_handle, dynamicSHCreateMenuBar, tSHCreateMenuBar, "SHCreateMenuBar", NULL)
		}
		// This function doesn't seem to be implemented on my 3630 !
		//IMPORT(aygshell_handle, dynamicSHHandleWMSettingChange, tSHHandleWMSettingChange, "SHHandleWMSettingChange")

	} else {
		dynamicSHFullScreen = defaultSHFullScreen;
		dynamicSHSipPreference = defaultSHSipPreference;
		//dynamicSHHandleWMSettingChange = defaultSHHandleWMSettingChange;
	}

	// See if GX.dll is present 
	GAPI_handle = LoadLibrary(TEXT("gx.dll"));

	if (GAPI_handle) {
		IMPORT(GAPI_handle, dynamicGXOpenInput, tGXOpenInput, "?GXOpenInput@@YAHXZ", NULL)
		IMPORT(GAPI_handle, dynamicGXGetDefaultKeys, tGXGetDefaultKeys, "?GXGetDefaultKeys@@YA?AUGXKeyList@@H@Z", NULL)
		IMPORT(GAPI_handle, dynamicGXCloseDisplay, tGXCloseDisplay, "?GXCloseDisplay@@YAHXZ", NULL)
		IMPORT(GAPI_handle, dynamicGXCloseInput, tGXCloseInput, "?GXCloseInput@@YAHXZ", NULL)
		IMPORT(GAPI_handle, dynamicGXSuspend, tGXSuspend, "?GXSuspend@@YAHXZ", NULL)
		IMPORT(GAPI_handle, dynamicGXResume, tGXResume, "?GXResume@@YAHXZ", NULL)
		IMPORT(GAPI_handle, dynamicGXGetDisplayProperties, tGXGetDisplayProperties, "?GXGetDisplayProperties@@YA?AUGXDisplayProperties@@XZ", NULL)
		IMPORT(GAPI_handle, dynamicGXOpenDisplay, tGXOpenDisplay, "?GXOpenDisplay@@YAHPAUHWND__@@K@Z", NULL)
		IMPORT(GAPI_handle, dynamicGXEndDrawInternal, tGXEndDraw, "?GXEndDraw@@YAHXZ", NULL)
		IMPORT(GAPI_handle, dynamicGXBeginDrawInternal, tGXBeginDraw, "?GXBeginDraw@@YAPAXXZ", NULL)
		IMPORT(GAPI_handle, dynamicGXIsDisplayDRAMBuffer, tGXIsDisplayDRAMBuffer, "?GXIsDisplayDRAMBuffer@@YAHXZ", NULL);
		dynamicGXBeginDraw = dynamicGXBeginDrawCached;
		dynamicGXEndDraw = dynamicGXEndDrawCached;
		gfx_mode_switch = !smartphone;
	} else {

#ifndef GAMEX

		TCHAR oeminfo[MAX_PATH];
		int i = 0;

		SystemParametersInfo(SPI_GETOEMINFO, sizeof(oeminfo), oeminfo, 0);

		while (availablePseudoGAPI[i].device) {
			if (!_tcsncmp(oeminfo, availablePseudoGAPI[i].device, _tcslen(availablePseudoGAPI[i].device))) {
				_pseudoGAPI_device = i;
				break;
			}
			i++;
		}

		if (!availablePseudoGAPI[i].device) {
			noGAPI = 1;
		}
		else {
			FILE *test;

			test = fopen("NoDirectVideo", "r");
			if (test) {
				noGAPI = 1;
				fclose(test);
			}
			else
			if (g_config->getBool("NoDirectVideo", false, "wince")) 
				noGAPI = 1;
		}

		dynamicGXOpenInput = defaultGXOpenInput;
		dynamicGXGetDefaultKeys = defaultGXGetDefaultKeys;
		dynamicGXCloseDisplay = defaultGXCloseDisplay;
		dynamicGXCloseInput = defaultGXCloseInput;
		dynamicGXSuspend = defaultGXSuspend;
		dynamicGXResume = defaultGXResume;
		dynamicGXGetDisplayProperties = defaultGXGetDisplayProperties;
		dynamicGXOpenDisplay = defaultGXOpenDisplay;
		dynamicGXEndDraw = defaultGXEndDraw;
		dynamicGXBeginDraw = defaultGXBeginDraw;

#else
		dynamicGXOpenInput = gameXGXOpenInput;
		dynamicGXGetDefaultKeys = gameXGXGetDefaultKeys;
		dynamicGXCloseDisplay = gameXGXCloseDisplay;
		dynamicGXCloseInput = gameXGXCloseInput;
		dynamicGXSuspend = gameXGXSuspend;
		dynamicGXResume = gameXGXResume;
		dynamicGXGetDisplayProperties = gameXGXGetDisplayProperties;
		dynamicGXOpenDisplay = gameXGXOpenDisplay;
		dynamicGXEndDraw = gameXGXEndDraw;
		dynamicGXBeginDraw = gameXGXBeginDraw;

		FILE *test;

		test = fopen("GameX", "r");
		if (!test) 
			noGAPI = 1;
		else
			fclose(test);

#endif

		gfx_mode_switch = false;
	}

	sound = g_config->getBool("Sound", true, "wince");
	if (sound) 
		sound_activated = sound;
	else
		sound_activated = true;

	FM_high_quality = g_config->getBool("FMHighQuality", false, "wince");
	g_config->setBool("FMHighQuality", FM_high_quality, "wince");

	_thread_priority = g_config->getInt("SoundThreadPriority", -1, "wince");
	if (_thread_priority < 0) {
#ifdef SH3
		_thread_priority = THREAD_PRIORITY_NORMAL;
#else
		_thread_priority = THREAD_PRIORITY_ABOVE_NORMAL;
#endif
		g_config->setInt("SoundThreadPriority", _thread_priority, "wince");
		g_config->flush();
	}

	select_game = true;
	need_GAPI = !smartphone;

	/* Create the main window */
	WNDCLASS wcex;
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)OSystem_WINCE3::WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= 0;
	wcex.hCursor		= NULL;
	wcex.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName	= 0;	
	wcex.lpszClassName	= TEXT("ScummVM");
	if (!RegisterClass(&wcex))
		Error(TEXT("Cannot register window class!"));

	hWnd_Window = CreateWindow(TEXT("ScummVM"), TEXT("ScummVM"), WS_VISIBLE,
      0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), NULL, NULL, GetModuleHandle(NULL), NULL);

	ShowWindow(hWnd_Window, SW_SHOW);
	UpdateWindow(hWnd_Window);

	if (smartphone) {
		if (SmartphoneInitialMenu(hInstance, hWnd_Window, game_name, directory))
			return 0;
		need_GAPI = true;
	}

	if (GraphicsOn(hWnd_Window, gfx_mode_switch)) {  // open GAPI in Portrait mode

		MessageBox(NULL, TEXT("Couldn't initialize display !"), TEXT("PocketSCUMM error"), MB_OK);

		return 0;

	}
	GAPIKeysInit();
	Cls();

	// Hide taskbar
	SetWindowPos(hWnd_Window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	SetForegroundWindow(hWnd_Window);
	dynamicSHFullScreen(hWnd_Window, SHFS_HIDESIPBUTTON | SHFS_HIDETASKBAR | SHFS_HIDESTARTICON);

	if (!smartphone) {

		result = chooseGame();

		if (result < 0)
			return 0;

		getSelectedGame(result, game_name, directory, &is_demo);
	}

	WideCharToMultiByte(CP_ACP, 0, directory, wcslen(directory) + 1, _directory, sizeof(_directory), NULL, NULL);
	strcat(_directory, "\\");
	
	runGame(game_name);

	return 0;
}


#ifdef USE_VORBIS
bool checkOggSampleRate(char *directory) {
	char trackFile[255];
	FILE *testFile;
	OggVorbis_File *test_ov_file = new OggVorbis_File;

	sprintf(trackFile,"%s/Track1.ogg", directory);
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


void runGame(char *game_name) {
	int argc = 4;
	char* argv[4];
	char argdir[MAX_PATH];
	char music[100];
	bool no_music;

	select_game = false;

	argv[0] = NULL;	
	sprintf(argdir, "-p%s", _directory);
	argv[1] = argdir;

	
	no_music = g_config->getBool("NoMusic", false, "wince");
	//sprintf(music, "-e%s", (no_music ? "null" : "wince"));
	sprintf(music, "-e%s", (no_music ? "null" : "adlib"));

	//sprintf(music, "-enull");

	argv[2] = music;
	argv[3] = game_name;

	if (!argv[3])
		return;

	// No default toolbar for zak256
	/*
	if (strcmp(game_name, "zak256") == 0)
		hide_toolbar = true;
	*/

	// Keyboard activated for Monkey Island 2 and Monkey 1 floppy
	if (strcmp(game_name, "monkey2") == 0 ||
		strcmp(game_name, "monkeyvga") == 0 ||
		strcmp(game_name, "monkeyega") == 0) {
		draw_keyboard = true;
		monkey_keyboard = true;
	}

	if (strcmp(game_name, "comi") == 0) {
		high_res = true;
	}

	//new_audio_rate = (strcmp(game_name, "dig") == 0 || strcmp(game_name, "monkey") == 0);
	new_audio_rate = (strcmp(game_name, "dig") == 0 || strcmp(game_name, "ft") == 0 || strcmp(game_name, "comi") == 0);

#ifdef USE_VORBIS
	// Modify the sample rate on the fly if OGG is involved 

	if (!new_audio_rate)
		new_audio_rate = checkOggSampleRate(_directory);
#endif

	detector.parseCommandLine(argc, argv);

	if (detector.detectMain())
		//return (-1);
		return;

	OSystem *system = detector.createSystem();

	mainClass = (OSystem_WINCE3*)system;

	//g_system = system;
	g_gui = new NewGui(system);

	/* Start the engine */

	is_simon = (detector._game.id >= GID_SIMON_FIRST && detector._game.id <= GID_SIMON_LAST);
	is_bass = (detector._game.id >= GID_SKY_FIRST && detector._game.id <= GID_SKY_LAST);

	if (smartphone || detector._game.id == GID_SAMNMAX || detector._game.id == GID_FT || detector._game.id == GID_DIG || detector._game.id == GID_CMI)
		hide_cursor = FALSE;
	else
		hide_cursor = TRUE;	

	engine = Engine::createFromDetector(&detector, system);

	keypad_init();
	load_key_mapping();

	engine->go();

	//return 0;
}



LRESULT CALLBACK OSystem_WINCE3::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static		 SHACTIVATEINFO sai;

	OSystem_WINCE3 *wm = NULL;
	
	if (!select_game)
		wm = (OSystem_WINCE3*)GetWindowLong(hWnd, GWL_USERDATA);
	
	if (!select_game && monkey_keyboard && (
			g_scumm->VAR(g_scumm->VAR_ROOM) != 108 &&		// monkey 2
			g_scumm->VAR(g_scumm->VAR_ROOM) != 90)) {		// monkey 1 floppy
		monkey_keyboard = false;
		draw_keyboard = false;
		toolbar_drawn = false;
	}

	if (smartphone) {
		if (SmartphoneWndProc(hWnd, message, wParam, lParam, wm))
			return 0;
	}
	else
		if (PPCWndProc(hWnd, message, wParam, lParam, wm))
			return 0;

	switch (message) 
	{
	case WM_CREATE:

		memset(&sai, 0, sizeof(sai));
		if (need_GAPI)
			dynamicSHSipPreference(hWnd, SIP_FORCEDOWN);
//		SHSipPreference(hWnd, SIP_INPUTDIALOG);

		return 0;

	case WM_DESTROY:
	case WM_CLOSE:
		if (need_GAPI)
			GraphicsOff();
		PostQuitMessage(0);
		break;

	case WM_ERASEBKGND:
		{

			if (!need_GAPI) {			
				RECT rc;
				HDC hDC;
				GetClientRect(hWnd, &rc);
				hDC = GetDC(hWnd);
				FillRect(hDC, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
				ReleaseDC(hWnd, hDC);			
			}
		}
		return 1;

	case WM_PAINT:
		{
			HDC hDC;
			PAINTSTRUCT ps;

			if (!need_GAPI)
				return DefWindowProc(hWnd, message, wParam, lParam);

			hDC = BeginPaint (hWnd, &ps);
			EndPaint (hWnd, &ps);
			
			if (!hide_toolbar)
				toolbar_drawn = false;

			/*
			if(!GetScreenMode()) {
				SHSipPreference(hWnd, SIP_UP);
			} else {
				SHSipPreference(hWnd, SIP_FORCEDOWN);
			} 
			*/
			dynamicSHSipPreference(hWnd, SIP_FORCEDOWN);
		}
//		SHSipPreference(hWnd, SIP_UP); /* Hack! */
		/* It does not happen often but I don't want to see tooltip traces */
		if (!select_game)
			wm->update_screen();
		return 0;

	case WM_ACTIVATE:
	case WM_SETFOCUS:	
		if (!need_GAPI) {
			SetFocus(hWnd);
			break;
		}
		GraphicsResume();
		if (!hide_toolbar)
			toolbar_drawn = false;
//		SHHandleWMActivate(hWnd, wParam, lParam, &sai, SHA_INPUTDIALOG);

		dynamicSHSipPreference(hWnd, SIP_FORCEDOWN);
		dynamicSHFullScreen(hWnd, SHFS_HIDETASKBAR);
		MoveWindow(hWnd, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), TRUE);
		SetCapture(hWnd);
		
		/*
		if (LOWORD(wParam) == WA_ACTIVE) {
			if (GetScreenMode()) {		
				SHSipPreference(hWnd, SIP_FORCEDOWN);
				SHFullScreen(hWnd, SHFS_HIDETASKBAR);
				MoveWindow(hWnd, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), TRUE);
				SetCapture(hWnd);
			}
			else {
				SHFullScreen(hWnd, SHFS_SHOWTASKBAR);
				MoveWindow(hWnd, 0, 0, GetSystemMetrics(SM_CYSCREEN), GetSystemMetrics(SM_CXSCREEN), TRUE);
			}
		}
		*/

		return 0;

	case WM_HIBERNATE:
	case WM_KILLFOCUS:
		GraphicsSuspend();
		if (!hide_toolbar)
			toolbar_drawn = false;
		return 0;

	case WM_SETTINGCHANGE:
		//not implemented ?
		//dynamicSHHandleWMSettingChange(hWnd, wParam, lParam, &sai);
		if (!hide_toolbar)
			toolbar_drawn = false;
		return 0;
	
	case WM_COMMAND:		
		return 0;
	
	case WM_TIMER:
		timer_callback(timer_interval);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
   }
   return 0;
}

/*************** Specific config support ***********/

void load_key_mapping() {
//	 unsigned char actions[TOTAL_ACTIONS];
	 unsigned int actions_keys[TOTAL_ACTIONS];
	 const char		*current;
	 int			version;
	 int			i;
	 
	 memset(actions_keys, 0, sizeof(actions_keys));
	 
	 version = g_config->getInt("KeysVersion", 0, "wince");

	 memset(actions_keys, 0, TOTAL_ACTIONS);

	 current = g_config->get("ActionKeys", "wince");
	 if (current && version == CURRENT_KEYS_VERSION) {
		for (i=0; i<TOTAL_ACTIONS; i++) {
			char x[6];
			int j;

			memset(x, 0, sizeof(x));
			memcpy(x, current + 5 * i, 4);
			sscanf(x, "%x", &j);
			actions_keys[i] = j;
		}
	 }
	 setActionKeys(actions_keys);

	 /*
	 memset(actions, 0, TOTAL_ACTIONS);

	 actions[0] = ACTION_PAUSE;
	 actions[1] = ACTION_SAVE;
	 actions[2] = ACTION_BOSS;
	 actions[3] = ACTION_SKIP;
	 actions[4] = ACTION_HIDE;

	 current = g_config->get("ActionTypes", "wince");
	 if (current && version) {
		for (i=0; i<TOTAL_ACTIONS; i++) {
			char x[6];
			int j;

			memset(x, 0, sizeof(x));
			memcpy(x, current + 3 * i, 2);
			sscanf(x, "%x", &j);
			actions[i] = j;
		}
	 }
	 setActionTypes(actions);
	 */

	 if (!version || version != CURRENT_KEYS_VERSION) {
		 g_config->setInt("KeysVersion", CURRENT_KEYS_VERSION, "wince");
		 g_config->flush();
	 }
}
					
void save_key_mapping() {
	 char tempo[1024];
	 const unsigned int *work_keys;
//	 const unsigned char *work;
	 int i;

	 tempo[0] = '\0';
	 work_keys = getActionKeys();
	 for (i=0; i<TOTAL_ACTIONS; i++) {
		 char x[4];
		 sprintf(x, "%.4x ", work_keys[i]);
		 strcat(tempo, x);
	 }
	 g_config->set("ActionKeys", tempo, "wince");

/*
	 tempo[0] = '\0';

	 work = getActionTypes();
	 for (i=0; i<TOTAL_ACTIONS; i++) {
		 char x[3];
		 sprintf(x, "%.2x ", work[i]);
		 strcat(tempo, x);
	 }
	 g_config->set("ActionTypes", tempo, "wince");
*/

	 g_config->flush();
}

/*************** Hardware keys support ***********/

void OSystem_WINCE3::addEventKeyPressed(int ascii_code) {
	_event.event_code = EVENT_KEYDOWN;
	_event.kbd.ascii = ascii_code;
	_event.kbd.keycode = ascii_code;
}

void OSystem_WINCE3::addEventRightButtonClicked() {
	//OSystem_WINCE3* system;
	//system = (OSystem_WINCE3*)g_scumm->_system;
	
	//system->addEventKeyPressed(9);
	if (is_bass) 
		_event.event_code = EVENT_RBUTTONDOWN;
	else
		addEventKeyPressed(9);
}

void action_right_click() {
	//OSystem_WINCE3* system;
	//system = (OSystem_WINCE3*)g_scumm->_system;

	//system->addEventRightButtonClicked();	
	mainClass->addEventRightButtonClicked();
}

void action_pause() {
	//OSystem_WINCE3* system;
	//system = (OSystem_WINCE3*)g_scumm->_system;

	//system->addEventKeyPressed(mapKey(VK_SPACE));
	mainClass->addEventKeyPressed(mapKey(VK_SPACE));
}

void action_save() {
	//OSystem_WINCE3* system;
	//system = (OSystem_WINCE3*)g_scumm->_system;

	/*if (GetScreenMode()) {*/
	/*
		draw_keyboard = true;
		if (!hide_toolbar)
			toolbar_drawn = false;
	*/
	/*}*/

	if (is_simon)
		return;
	else
	if (is_bass)
		mainClass->addEventKeyPressed(63);
	else
	if (g_scumm->_version <= 2)
		mainClass->addEventKeyPressed(5);
	else
	if ((g_scumm->_features & GF_OLD256) || (g_scumm->_gameId == GID_CMI) || (g_scumm->_features & GF_16COLOR))
		//system->addEventKeyPressed(319);
		mainClass->addEventKeyPressed(319);
	else
		//system->addEventKeyPressed(g_scumm->VAR(g_scumm->VAR_SAVELOADDIALOG_KEY));
		mainClass->addEventKeyPressed(g_scumm->VAR(g_scumm->VAR_SAVELOADDIALOG_KEY));
}

void action_quit() {
	do_quit();
}

void action_freelook() {
	if (!freelook) {
		save_hide_cursor = hide_cursor;
		hide_cursor = false;
	}
	else {
		hide_cursor = save_hide_cursor;
	}
	freelook = !freelook;
}

void action_boss() {
	SHELLEXECUTEINFO se;    

	g_config->setBool("Sound", sound_activated, "wince");
	g_config->setInt("DisplayMode", GetScreenMode(), "wince");
	g_config->flush();
	sound_activated = false;
	toolbar_drawn = false;
	hide_toolbar = true;
	Cls();
	g_scumm->requestSave(0, "BOSS");
	g_scumm->scummLoop(0);
	dynamicGXCloseInput();
	dynamicGXCloseDisplay();
	SDL_AudioQuit();
	memset(&se, 0, sizeof(se));
	se.cbSize = sizeof(se);
	se.hwnd = NULL;
	se.lpFile = TEXT("tasks.exe");
	se.lpVerb = TEXT("open");
	se.lpDirectory = TEXT("\\windows");
	ShellExecuteEx(&se);
	exit(1);
}

void action_skip() {
	//OSystem_WINCE3* system;
	//system = (OSystem_WINCE3*)g_scumm->_system;

	if (is_simon || is_bass) {
		//system->addEventKeyPressed(mapKey(VK_ESCAPE));
		mainClass->addEventKeyPressed(mapKey(VK_ESCAPE));
		return;
	}

	//system->addEventKeyPressed(KEY_MAGIC_SKIP);
	mainClass->addEventKeyPressed(KEY_ALL_SKIP);
/*
	if (g_scumm->vm.cutScenePtr[g_scumm->vm.cutSceneStackPointer] || g_scumm->_insaneState)
		system->addEventKeyPressed(g_scumm->_vars[g_scumm->VAR_CUTSCENEEXIT_KEY]);
	else 
	if (g_scumm->_talkDelay > 0)
		system->addEventKeyPressed(g_scumm->VAR_TALKSTOP_KEY);						
	else
		system->addEventKeyPressed(mapKey(VK_ESCAPE));
*/
}

void do_hide(bool hide_state) {
	hide_toolbar = hide_state;
	if (hide_toolbar)
		RestoreScreenGeometry();
	else
		LimitScreenGeometry();
	Cls();
	num_of_dirty_square = MAX_NUMBER_OF_DIRTY_SQUARES;
	toolbar_drawn = hide_toolbar;
	//g_scumm->_system->update_screen();
	mainClass->update_screen();
}

void action_hide() {
	do_hide(!hide_toolbar);
}

void action_keyboard() {
	/*if (GetScreenMode()) {*/
		draw_keyboard = !draw_keyboard;
		if (!hide_toolbar)
			toolbar_drawn = false;
	/*}*/
}

void action_sound() {
	sound_activated = !sound_activated;
}

void action_cursoronoff() {
	hide_cursor = !hide_cursor;
}

void action_subtitleonoff() {
	g_scumm->_noSubtitles = !g_scumm->_noSubtitles;
}

void keypad_init() {
	static pAction actions[TOTAL_ACTIONS] =
	{ NULL, action_pause, action_save, action_quit, action_skip, action_hide, 
	  action_keyboard, action_sound, action_right_click, action_cursoronoff,
	  action_subtitleonoff, action_boss, action_freelook
	};
	
	GAPIKeysInitActions(actions);
	
}

void keypad_close() {
	dynamicGXCloseInput();	
}

void force_keyboard(bool activate) {

if (activate) {
	save_hide_toolbar = hide_toolbar;
	if (save_hide_toolbar) {
		// Display the keyboard while the dialog is running
		do_hide(false);
	}
	if (!draw_keyboard) {
		keyboard_override = true;
		draw_keyboard = true;
		toolbar_drawn = false;
	}
}
else {
	if (save_hide_toolbar) {
		do_hide(true);
		save_hide_toolbar = false;
	}
	if (keyboard_override) {
		keyboard_override = false;
		draw_keyboard = false;
		toolbar_drawn = false;
	}
}
}

/************* OSystem Main **********************/
OSystem *OSystem_WINCE3::create(int gfx_mode, bool full_screen) {
	const char *display_mode;
	OSystem_WINCE3 *syst = new OSystem_WINCE3();
	syst->_mode = gfx_mode;
	syst->_full_screen = full_screen;
	syst->_event.event_code = EVENT_WHEELUP;   /* FIXME lazy */
	syst->_start_time = GetTickCount();

	/* Retrieve the handle of this module */
	syst->hInst = GetModuleHandle(NULL);

	syst->hWnd = hWnd_Window;
	SetWindowLong(syst->hWnd, GWL_USERDATA, (long)syst);
	
	// Mini SDL init

	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER)==-1) {		
	    exit(1);
	}

	reducePortraitGeometry();

	if (smartphone || (high_res && !wide_screen) || ((noGAPI || !gfx_mode_switch) && GetSystemMetrics(SM_CXSCREEN) < 320)) 
		SetScreenMode(1);

	Cls();
	drawWait();

	// Set mode, portrait or landscape
	display_mode = g_config->get("DisplayMode", "wince");

	if (display_mode && !(high_res || noGAPI || !gfx_mode_switch))
		SetScreenMode(atoi(display_mode));

	return syst;
}

OSystem *OSystem_WINCE3_create() {
	return OSystem_WINCE3::create(0, 0);
}

void OSystem_WINCE3::set_timer(int timer, int (*callback)(int)) {
	if (!SetTimer(hWnd, 1, timer, NULL))
		exit(1);
	timer_interval = timer;
	timer_callback = callback;
}

void OSystem_WINCE3::set_palette(const byte *colors, uint start, uint num) {
	const byte *b = colors;
	uint i;
	for(i=0;i!=num;i++) {
		SetPalEntry(i + start, b[0], b[1], b[2]);
		b += 4;
	}

	palette_update();

	num_of_dirty_square = MAX_NUMBER_OF_DIRTY_SQUARES;
}

int16 OSystem_WINCE3::get_height() {
        return _screenHeight;
}

int16 OSystem_WINCE3::get_width() {
        return _screenWidth;
}

void OSystem_WINCE3::clear_sound_proc() {
        SDL_CloseAudio();
}

void OSystem_WINCE3::load_gfx_mode() {
	force_full = true;

	if (!high_res) {
		_gfx_buf = (byte*)malloc((320 * 240) * sizeof(byte));	
		if (!_gfx_buf) {
			drawError("Not enough memory - main buffer");
			exit(1);
		}
		_overlay_buf = (byte*)malloc((320 * 240) * sizeof(uint16));
		if (!_overlay_buf) {
			drawError("Not enough memory - overlay buffer");
			exit(1);
		}
	}
	else {
		_gfx_buf = (byte*)malloc((640 * 480) * sizeof(byte));	
		if (!_gfx_buf) {
			drawError("Not enough memory - main buffer");
			exit(1);
		}
		_overlay_buf = (byte*)malloc((320 * 240) * sizeof(uint16));
		if (!_overlay_buf) {
			drawError("Not enough memory - overlay buffer");
			exit(1);
		}
	}
	//_ms_backup = (byte*)malloc((40 * 40 * 3) * sizeof(byte));
	_ms_backup = (byte*)malloc((MAX_MOUSE_W * MAX_MOUSE_H) * sizeof(uint16));
	if (!_ms_backup) {
		drawError("Not enogh memory - mouse cursor");
		exit(1);
	}
}

void OSystem_WINCE3::unload_gfx_mode() {
 // FIXME: Free the _gfx_buf here
}

void OSystem_WINCE3::init_size(uint w, uint h) {
	load_gfx_mode();
	SetScreenGeometry(w, h);
	LimitScreenGeometry();
	_screenWidth = w;
	_screenHeight = h;
	num_of_dirty_square = MAX_NUMBER_OF_DIRTY_SQUARES;
}

void OSystem_WINCE3::copy_rect(const byte *buf, int pitch, int x, int y, int w, int h) {
	byte *dst;

	if (!hide_cursor && _mouse_drawn)
		undraw_mouse();

	AddDirtyRect(x, y, w, h);

	if (x == 0 && y == 0 && w == _screenWidth && h == _screenHeight) {
		memcpy(_gfx_buf, buf, _screenWidth * _screenHeight);
		return;
	}

	dst = _gfx_buf + y * (high_res ? 640 : 320) + x;
	do {
		memcpy(dst, buf, w);
		dst += (high_res ? 640 : 320);
		buf += pitch;
	} while (--h);

}

void OSystem_WINCE3::update_screen() {

	if (!hide_cursor)
		draw_mouse();

	if (_overlay_visible) {
			Set_565((int16*)_overlay_buf, 320, 0, 0, 320, 200);
		checkToolbar();
	}
	else {
		if (num_of_dirty_square >= MAX_NUMBER_OF_DIRTY_SQUARES) {
			Blt(_gfx_buf);  // global redraw
			num_of_dirty_square = 0;
		}
		else {
			int i;
			
			for (i=0; i<num_of_dirty_square; i++) {
				if (wide_screen && extra_wide_screen) 
					Blt_part(_gfx_buf + ((high_res ? 640 : 320) * ds[i].y) + ds[i].x, ds[i].x, ds[i].y, ds[i].w, ds[i].h, (high_res ? 640 : 320), false);
				else
				if (wide_screen)
					Blt_part(_gfx_buf + ((high_res ? 640 : 320) * ds[i].y) + ds[i].x, (!high_res ? ds[i].x : ds[i].x/2), (!high_res ? ds[i].y : ds[i].y/2), ds[i].w, ds[i].h, (high_res ? 640 : 320), false);
				else
				if (smartphone)
					Blt_part(_gfx_buf + (320 * ds[i].y) + ds[i].x, ds[i].x * 2 / 3, ds[i].y * 7 / 8, ds[i].w, ds[i].h, 320, false);
				else
				if (high_res)
					Blt_part(_gfx_buf + (640 * ds[i].y) + ds[i].x, ds[i].x/2, ds[i].y/2, ds[i].w, ds[i].h, 640, false);
				else
					Blt_part(_gfx_buf + (320 * ds[i].y) + ds[i].x, (GetScreenMode() ? ds[i].x : ds[i].x * 3/4), ds[i].y, ds[i].w, ds[i].h, 320, false);
			}
			num_of_dirty_square = 0;
		}
	}
}

bool OSystem_WINCE3::show_mouse(bool visible) {
	if (_mouse_visible == visible)
		return visible;
	
	bool last = _mouse_visible;
	_mouse_visible = visible;

	return last;
}

// From SDL backend 

void OSystem_WINCE3::draw_mouse() {
	if (_mouse_drawn || !_mouse_visible)
		return;

	int x = _ms_cur.x - _ms_hotspot_x;
	int y = _ms_cur.y - _ms_hotspot_x;
	int w = _ms_cur.w;
	int h = _ms_cur.h;
	byte color;
	byte *src = _ms_buf;		// Image representing the mouse
	int toolbar_offset = (high_res && wide_screen ? 440 : 200);

	if (_overlay_visible && (x >= 320 || y>=240))
		return;

	
	// Do not draw the mouse over the toolbar
	if (!hide_toolbar && (y >= toolbar_offset))
		return;

	// clip the mouse rect, and addjust the src pointer accordingly
	if (x < 0) {
		w += x;
		src -= x;
		x = 0;
	}
	if (y < 0) {
		h += y;
		src -= y * _ms_cur.w;
		y = 0;
	}

	if (w > _screenWidth - x)
		w = _screenWidth - x;
	if (h > _screenHeight - y)
		h = _screenHeight - y;

	// Quick check to see if anything has to be drawn at all
	if (w <= 0 || h <= 0)
		return;

	// Store the bounding box so that undraw mouse can restore the area the
	// mouse currently covers to its original content.
	_ms_old.x = x;
	_ms_old.y = y;
	_ms_old.w = w;
	_ms_old.h = h;

	// Mark as dirty
	AddDirtyRect(x, y, w, h);

	if (!_overlay_visible) {
		byte *bak = _ms_backup;		// Surface used to backup the area obscured by the mouse
		byte *dst;					// Surface we are drawing into
	
		dst = (byte *)_gfx_buf + y * _screenWidth + x;
		while (h > 0) {
			int width = w;
			while (width > 0) {
				*bak++ = *dst;
				color = *src++;
				if (color != 0xFF)	// 0xFF = transparent, don't draw
					*dst = color;
				dst++;
				width--;
			}
			src += _ms_cur.w - w;
			bak += MAX_MOUSE_W - w;
			dst += _screenWidth - w;
			h--;
		}
	} 
	else {
		uint16 *bak = (uint16 *)_ms_backup;	// Surface used to backup the area obscured by the mouse
		uint16 *dst;					// Surface we are drawing into
	
		dst = (uint16*)_overlay_buf + y * 320 + x;
		while (h > 0) {
			int width = w;
			while (width > 0) {
				*bak++ = *dst;
				color = *src++;
				if (color != 0xFF)	{ // 0xFF = transparent, don't draw 
					uint16 x = getColor565(color);
					memcpy(dst, &x, sizeof(uint16));
				}
				dst++;
				width--;
			}
			src += _ms_cur.w - w;
			bak += MAX_MOUSE_W - w;
			dst += 320 - w;
			h--;
		}
	}

	// Finally, set the flag to indicate the mouse has been drawn
	_mouse_drawn = true;
}


void OSystem_WINCE3::undraw_mouse() {
	if (!_mouse_drawn)
		return;
	_mouse_drawn = false;

	const int old_mouse_x = _ms_old.x;
	const int old_mouse_y = _ms_old.y;
	const int old_mouse_w = _ms_old.w;
	const int old_mouse_h = _ms_old.h;
	int x, y;

	if (!_overlay_visible) {
		byte *dst, *bak = _ms_backup;

		// No need to do clipping here, since draw_mouse() did that already
		dst = (byte *)_gfx_buf + old_mouse_y * _screenWidth + old_mouse_x;
		for (y = 0; y < old_mouse_h; ++y, bak += MAX_MOUSE_W, dst += _screenWidth) {
			for (x = 0; x < old_mouse_w; ++x) {
				dst[x] = bak[x];
			}
		}	
	}
	else {
		uint16 *dst, *bak = (uint16 *)_ms_backup;
	
		// No need to do clipping here, since draw_mouse() did that already
		dst = (uint16 *)_overlay_buf + old_mouse_y * 320 + old_mouse_x;
		for (y = 0; y < old_mouse_h; ++y, bak += MAX_MOUSE_W, dst += 320) {
			for (x = 0; x < old_mouse_w; ++x) {
				dst[x] = bak[x];
			}
		}
	}

	AddDirtyRect(old_mouse_x, old_mouse_y, old_mouse_w, old_mouse_h);
}

void OSystem_WINCE3::warp_mouse(int x, int y) {
}
	
void OSystem_WINCE3::set_mouse_pos(int x, int y) {
	if (x != _ms_cur.x || y != _ms_cur.y) {
		_ms_cur.x = x;
		_ms_cur.y = y;
		undraw_mouse();
	}
}
	
void OSystem_WINCE3::set_mouse_cursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y) {
	_ms_cur.w = w;
	_ms_cur.h = h;

	_ms_hotspot_x = hotspot_x;
	_ms_hotspot_y = hotspot_y;

	_ms_buf = (byte*)buf;

	// Refresh mouse cursor

	if (!hide_cursor) {
		undraw_mouse();
		draw_mouse();
	}
}
	
void OSystem_WINCE3::set_shake_pos(int shake_pos) {;}
		
uint32 OSystem_WINCE3::get_msecs() {
	return GetTickCount() - _start_time;
}
	
void OSystem_WINCE3::delay_msecs(uint msecs) {
	//handleMessage();
	if (msecs <= 0 || msecs >= 10000)
		exit(1);
	Sleep(msecs);
}
	
void OSystem_WINCE3::create_thread(ThreadProc *proc, void *param) {
	// needed for emulated MIDI support (Sam'n'Max)
	HANDLE handle;
	handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)proc, param, 0, NULL);
	SetThreadPriority(handle, THREAD_PRIORITY_NORMAL);
}

int mapKey(int key, byte mod)
{
	if (key>=VK_F1 && key<=VK_F9) {
		return key - VK_F1 + 315;
	}
	return key;
}
	
bool OSystem_WINCE3::poll_event(Event *event) {

	for (;;) {
		MSG msg;

		_event.event_code = EVENT_WHEELUP; /* FIXME lazy */

		if (!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			return false;

		if (msg.message==WM_QUIT) {
			terminated=true;
			do_quit();
			return false;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (_event.event_code == EVENT_WHEELUP)  /* FIXME lazy */
			return false;

		*event = _event;

		_event.event_code = EVENT_WHEELUP;

		return true;
	}
	
	return false;
}

void own_soundProc(void *buffer, byte *samples, int len) {

	(*real_soundproc)(buffer, samples, len);

	if (!sound_activated)
		memset(samples, 0, len);
}

bool OSystem_WINCE3::set_sound_proc(SoundProc *proc, void *param, SoundFormat format) {
	SDL_AudioSpec desired;

	/* only one format supported at the moment */

	real_soundproc = proc;
	desired.freq = (new_audio_rate ? SAMPLES_PER_SEC_NEW : SAMPLES_PER_SEC_OLD);
	desired.format = AUDIO_S16SYS;
	desired.channels = 2;
	desired.samples = 128;
	desired.callback = own_soundProc;
	desired.userdata = param;
	desired.thread_priority = _thread_priority;

	if (SDL_OpenAudio(&desired, NULL) != 0) {
		return false;
	}
	SDL_PauseAudio(0);

	return true;
}

/* Hotswap graphics modes */
void OSystem_WINCE3::get_320x200_image(byte *buf) {;}
void OSystem_WINCE3::hotswap_gfx_mode() {;}
uint32 OSystem_WINCE3::property(int param, Property *value) {
	switch(param) {

	case PROP_TOGGLE_FULLSCREEN:
		return 1;

	case PROP_SET_WINDOW_CAPTION:
		return 1;

	case PROP_OPEN_CD:		
		break;

	case PROP_SET_GFX_MODE:
		return 1;

	case PROP_SHOW_DEFAULT_CURSOR:
		break;

	case PROP_GET_SAMPLE_RATE:
		return (new_audio_rate ? SAMPLES_PER_SEC_NEW : SAMPLES_PER_SEC_OLD);

	case PROP_GET_FMOPL_ENV_BITS:
		return (FM_high_quality ? FMOPL_ENV_BITS_HQ : FMOPL_ENV_BITS_LQ);

	case PROP_GET_FMOPL_EG_ENT:
		return (FM_high_quality ? FMOPL_EG_ENT_HQ : FMOPL_EG_ENT_LQ);
	}

	return 0;
}
		
void OSystem_WINCE3::quit() {
	unload_gfx_mode();		
	do_quit();
}

/* CDRom Audio */
void OSystem_WINCE3::stop_cdrom() {;}
void OSystem_WINCE3::play_cdrom(int track, int num_loops, int start_frame, int end_frame) {;}

bool OSystem_WINCE3::poll_cdrom() {return 0;}
void OSystem_WINCE3::update_cdrom() {;}

//void ScummDebugger::attach(Scumm *s) {;}

/* Mutex stuff */
OSystem::MutexRef OSystem_WINCE3::create_mutex() {
	return (MutexRef)CreateMutex(NULL, FALSE, NULL);
}
void OSystem_WINCE3::lock_mutex(MutexRef handle) {
	WaitForSingleObject((HANDLE)handle, INFINITE);
}

void OSystem_WINCE3::unlock_mutex(MutexRef handle) {
	ReleaseMutex((HANDLE)handle);
}

void OSystem_WINCE3::delete_mutex(MutexRef handle) {
	CloseHandle((HANDLE)handle);
}

/* Overlay stuff */

void OSystem_WINCE3::show_overlay() {
	undraw_mouse();
	_overlay_visible = true;
	clear_overlay();

}

void OSystem_WINCE3::hide_overlay() {
	undraw_mouse();
	_overlay_visible = false;
	toolbar_drawn = false;
	num_of_dirty_square = MAX_NUMBER_OF_DIRTY_SQUARES;
}

void OSystem_WINCE3::clear_overlay() {

	if (!_overlay_visible)
		return;

	Blt(_gfx_buf);
}

void OSystem_WINCE3::grab_overlay(int16 *buf, int pitch) {
	//FIXME : it'd be better with a REAL surface :)
	//Blt(_gfx_buf);
	Get_565(_gfx_buf, buf, pitch, 0, 0, 320, 200);
	memcpy(_overlay_buf, buf, 320 * 200 * sizeof(int16));
}

void OSystem_WINCE3::copy_rect_overlay(const int16 *buf, int pitch, int x, int y, int w, int h) {
	int i;

	UBYTE *dest = _overlay_buf;
	dest += y * 320  * sizeof(int16);
	for (i=0; i<h; i++) {
		memcpy(dest + (x * sizeof(int16)), buf, w * 2);
		dest += 320 * sizeof(int16);
		buf += pitch;
	}	
}

void OSystem_WINCE3::move_screen(int dx, int dy, int height) {
	// FIXME : to be implemented
}

/* NECESSARY operators redefinition */

/*
void *operator new(size_t size) {
        return memset(malloc(size), 0xE7, size);
}

void operator delete(void *ptr) {
        free(ptr);
}

#undef free
void free_check(void *ptr) {
        if ((uint)ptr == 0xE7E7E7E7UL) {
                printf("ERROR: freeing 0xE7E7E7E7\n");
                exit(1);
        }
        if ((int)ptr & 1) {
                warning("Freeing odd address 0x%x\n", ptr);
        }
        free(ptr);
}
*/


void *operator new(size_t size) {
	return calloc(size, 1);
}

void operator delete(void *ptr) {
	free(ptr);
}

const char* getBuildDate() {
	static char buildDate[100];
	int i;

	strcpy(buildDate, __DATE__);
	for (i=strlen(buildDate) - 1; buildDate[i] != ' '; i--)
	buildDate[i] = '\0';
	strcat(buildDate, __TIME__);
	buildDate[strlen(buildDate) - 3] = '\0';

	return buildDate;
}
