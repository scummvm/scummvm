// ScummVM - Scumm Interpreter
// PocketSCUMM - PocketPC port of ScummVM. Based on the original Win32
// implementation by Ludvig Strigeus.
// Ported by Vasyl Tsvirkunov (vasyl@pacbell.net).
// Note: this is the very first version, implementing only basic functionality.
//       Keyboard is not implemented, there is no way to access save/load game
//       and the interpreter is hardcoded to one game (MI1 in this case). Later
//       versions will get these limitations removed. Right now you should
//       consider this port a proof of concept.
// To run PocketSCUMM, put PocketSCUMM.exe and game resources (MONKEY.000, MONKEY.001)
// in one folder (can be on storage card) and run the executable. Unused part of
// the screen below the image is split to two halves - tap on the left to press
// Escape (skip intro, etc.), tap on the right to change screen rotation.
// Another note: This file is very similar to windows.cpp in the core project. I was
//       even thinking about integrating WinCE code there, I still may do it later.
//       For ease of updating, non-trivial blocks identical to windows.cpp are marked
//       with //{{ and //}} comments
// Consistent with 1.18

#include "stdafx.h"
#include <assert.h>

#include "scumm.h"
#include "screen.h"
#include "gui.h"
#include "sound/mididrv.h"
#include "gameDetector.h"
#include "simon/simon.h"
#include "gapi_keys.h"


#include "commctrl.h"
#include <Winuser.h>
#include <Winnls.h>
#include <sipapi.h>
#include <Aygshell.h>
#include <gx.h>
#include <aygshell.h>
#include "resource.h"

#include "SDL.h"
#include "SDL_audio.h"
#include "SDL_timer.h"
#include "SDL_thread.h"

#define VERSION "(VM " SCUMMVM_CVS ")"

#define SHMenuBar_GetMenu(hWndMB,ID_MENU) (HMENU)SendMessage((hWndMB), SHCMBM_GETSUBMENU, (WPARAM)0, (LPARAM)ID_MENU)

#define SDL_INIT_AUDIO          0x00000010
extern "C" DECLSPEC int SDL_Init(Uint32 flags);

#define SAMPLES_PER_SEC 22050
#define BUFFER_SIZE (8192)
#define BITS_PER_SAMPLE 16

void drawAllToolbar(bool);
void redrawSoundItem();
ToolbarSelected getToolbarSelection(int, int);

extern void palette_update();
extern void Cls();

extern void startFindGame();
extern void displayGameInfo();
extern bool loadGameSettings(void);
extern void setFindGameDlgHandle(HWND);
extern void getSelectedGame(int, char*, TCHAR*);

const char KEYBOARD_MAPPING_ALPHA_HIGH[] = {
	"ABCDEFGHIJKLM"
};

const char KEYBOARD_MAPPING_NUMERIC_HIGH[] = {
	"12345"
};

const char KEYBOARD_MAPPING_ALPHA_LOW[] = {
	"NOPQRSTUVWXYZ"
};

const char KEYBOARD_MAPPING_NUMERIC_LOW[] = {
	"67890"
};

/* Added from generic X-Win port - not used right now */

#define MAX_NUMBER_OF_DIRTY_SQUARES 32
typedef struct {
  int x, y, w, h;
} dirty_square;
static dirty_square ds[MAX_NUMBER_OF_DIRTY_SQUARES];
static int num_of_dirty_square;

static int old_mouse_x, old_mouse_y;
static int old_mouse_h, old_mouse_w;
static bool has_mouse, hide_mouse;

GameDetector detector;
Gui gui;

Scumm *g_scumm;


#define BAK_WIDTH 40
#define BAK_HEIGHT 40
unsigned char old_backup[BAK_WIDTH * BAK_HEIGHT];


#define AddDirtyRec(xi,yi,wi,hi) 				\
  if (num_of_dirty_square < MAX_NUMBER_OF_DIRTY_SQUARES) {	\
    ds[num_of_dirty_square].x = xi;				\
    ds[num_of_dirty_square].y = yi;				\
    ds[num_of_dirty_square].w = wi;				\
    ds[num_of_dirty_square].h = hi;				\
    num_of_dirty_square++;					\
  }

// Similar to Error in windows.cpp but has to take Unicode in account
void Error(LPCTSTR msg)
{
	OutputDebugString(msg);
	MessageBox(HWND_DESKTOP, msg, TEXT("Error"), MB_ICONSTOP);
	exit(1);
}

//{{
Scumm scumm;

//IMuse sound;
//SOUND_DRIVER_TYPE snd_driv;

byte veryFastMode;
//}}

bool sound_activated;
HWND hWnd_MainMenu;
HWND hWnd_Window;

extern bool toolbar_drawn;
extern bool draw_keyboard;
bool hide_toolbar;

bool get_key_mapping;

//long TEMPO_BASE;

static char _directory[MAX_PATH];

// WndProc is significantly port-specific
int mapKey(int key) {
	if (key>=VK_F1 && key<=VK_F9) {
		return key - VK_F1 + 315;
	}
	return key;
}

void error_handler(char *text, char is_error) {
	if (is_error) {
		TCHAR	error[1024];
		GXCloseInput();
		GXCloseDisplay();
		ShowWindow(hWnd_Window, SW_HIDE);		
		MultiByteToWideChar(CP_ACP, 0, text, strlen(text) + 1, error, sizeof(error));
		MessageBox(GetForegroundWindow(), error, TEXT("ScummVM error"), MB_OK);
	}
}


void do_quit() {
	GXCloseInput();
	GXCloseDisplay();
	SDL_AudioQuit();
	exit(1);
}

/* Registry support */

void registry_init() {
	 HKEY	hkey;
	 DWORD	disposition;
	 DWORD  keyType, keySize, dummy;
	 unsigned char actions[NUMBER_ACTIONS];

	 memset(actions, 0, NUMBER_ACTIONS);

	 if(RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software\\PocketSCUMM"), 
		 0, NULL, 0, 0, NULL, &hkey, &disposition) == ERROR_SUCCESS) {

		 keyType = REG_DWORD;
		 keySize = sizeof(DWORD);
		 if (RegQueryValueEx(hkey, TEXT("VolumeMaster"), NULL, &keyType, 
						 (unsigned char*)&dummy, &keySize) == ERROR_SUCCESS) 
					scumm._sound_volume_master = (uint16)dummy;
		 else
					scumm._sound_volume_master = 100;
		 
		 if (RegQueryValueEx(hkey, TEXT("VolumeMusic"), NULL, &keyType, 
						 (unsigned char*)&dummy, &keySize) == ERROR_SUCCESS) 
					scumm._sound_volume_music = (uint16)dummy;
		 else
					scumm._sound_volume_music = 60;		 
		 if (RegQueryValueEx(hkey, TEXT("VolumeSfx"), NULL, &keyType, 
						 (unsigned char*)&dummy, &keySize) == ERROR_SUCCESS) 
					scumm._sound_volume_sfx = (uint16)dummy;
		 else
					scumm._sound_volume_sfx = 100;		 
		 keyType = REG_BINARY;
		 keySize = NUMBER_ACTIONS;
		 memset(actions, 0, sizeof(actions));
		 RegQueryValueEx(hkey, TEXT("ActionsKeys"), NULL, &keyType, 
						 actions, &keySize);
		 setActionKeys(actions);		 
		 actions[0] = ACTION_PAUSE;
		 actions[1] = ACTION_SAVE;
		 actions[2] = ACTION_QUIT;
		 actions[3] = ACTION_SKIP;
		 actions[4] = ACTION_HIDE;
		 RegQueryValueEx(hkey, TEXT("ActionsTypes"), NULL, &keyType,
						 actions, &keySize);
		 setActionTypes(actions);

		 RegCloseKey(hkey);
	 }
}
					
void registry_save() {
	 HKEY	hkey;
	 DWORD	disposition;
	 DWORD  keyType, keySize, dummy;

	 if(RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software\\PocketSCUMM"), 
		 0, NULL, 0, 0, NULL, &hkey, &disposition) == ERROR_SUCCESS) {

		 keyType = REG_DWORD;
		 keySize = sizeof(DWORD);
		 dummy = scumm._sound_volume_master;
		 RegSetValueEx(hkey, TEXT("VolumeMaster"), 0, keyType, (unsigned char*)&dummy, keySize);
		 dummy = scumm._sound_volume_music;
		 RegSetValueEx(hkey, TEXT("VolumeMusic"), 0, keyType, (unsigned char*)&dummy, keySize);		 
		 dummy = scumm._sound_volume_sfx;
		 RegSetValueEx(hkey, TEXT("VolumeSfx"), 0, keyType, (unsigned char*)&dummy, keySize);		 
		 keyType = REG_BINARY;
		 keySize = NUMBER_ACTIONS;
		 RegSetValueEx(hkey, TEXT("ActionsKeys"), 0, keyType, getActionKeys(), 
						keySize);	
		 RegSetValueEx(hkey, TEXT("ActionsTypes"), 0, keyType, getActionTypes(),
						keySize);

		 RegCloseKey(hkey);
	 }
}

/* Action functions */

// FIX ACTIONS

void action_right_click() {
	//wm->_scumm->_rightBtnPressed |= msDown|msClicked;
}

void action_pause() {
	//wm->_scumm->_keyPressed = mapKey(VK_SPACE);
}

void action_save() {
	if (GetScreenMode()) {
		draw_keyboard = true;
		if (!hide_toolbar)
			toolbar_drawn = false;
	}

	//wm->_scumm->_keyPressed = mapKey(VK_F5);
}

void action_quit() {
	do_quit();
}

void action_skip() {
	//wm->_scumm->_keyPressed = mapKey(VK_ESCAPE);
}

void action_hide() {
	hide_toolbar = !hide_toolbar;
	Cls();
	toolbar_drawn = hide_toolbar;
	//wm->writeToScreen();
}

void action_keyboard() {
	if (GetScreenMode()) {
		draw_keyboard = !draw_keyboard;
		if (!hide_toolbar)
			toolbar_drawn = false;
	}
}

void action_sound() {
	sound_activated = !sound_activated;
	//wm->_scumm->_soundsPaused2 = !sound_activated;
}

/* Initialization */

void keypad_init() {
	static pAction actions[TOTAL_ACTIONS] =
	{ action_pause, action_save, action_quit, action_skip, action_hide, 
	  action_keyboard, action_sound, action_right_click };
	
	GAPIKeysInit(actions);
	
}

void keypad_close() {
	GXCloseInput();	
}

// This function is very similar to the one in windows.cpp except for
// one line removed.

// TODO : use dirty rects

/*
void blitToScreen(Scumm *s, byte *src,int x, int y, int w, int h) {
	byte *dst;
	//int i;

	dst = (byte*)wm->_vgabuf + y*320 + x;

	if (h<=0)	return;

	hide_mouse = true;
	if (has_mouse) {
	 s->drawMouse();
	}
  
	AddDirtyRec(x, y, w, h);

	do {
		memcpy(dst, src, w);
		dst += 320;
		src += 320;
	} while (--h);

}
*/

//{{
int clock;

// TODO : use dirty rects for faster screen updates

/*
void updateScreen(Scumm *s) {

  if (hide_mouse) {
    hide_mouse = false;
    s->drawMouse();
  }

	if (s->_palDirtyMax != -1) {
		wm->setPalette(s->_currentPalette, 0, 256);	
		s->_palDirtyMax = -1;
	}

	wm->writeToScreen();
}

void waitForTimer(Scumm *s, int delay) {
	wm->handleMessage();
	if (!veryFastMode) {
		assert(delay<5000);
		if(!delay)
			delay++;
		Sleep(delay);
	} 
}
*/

// Copy/Paste from X11
// Dirty rects not managed now

/*

void drawMouse(int xdraw, int ydraw, int w, int h, byte *buf, bool visible) {
  unsigned char *dst,*bak;

  if ((xdraw >= 320) || ((xdraw + w) <= 0) ||
      (ydraw >= 200) || ((ydraw + h) <= 0)) {
    if (hide_mouse) visible = false;
    if (has_mouse) has_mouse = false;
    if (visible) has_mouse = true;
    return;
  }

  if (hide_mouse)
    visible = false;
  
  assert(w<=BAK_WIDTH && h<=BAK_HEIGHT);

  if (has_mouse) {
    int old_h = old_mouse_h;

    has_mouse = false;
    AddDirtyRec(old_mouse_x, old_mouse_y, old_mouse_w, old_mouse_h);

    dst = wm->_vgabuf + (old_mouse_y * 320) + old_mouse_x;
    bak = old_backup;
    
    while (old_h > 0) {
      memcpy(dst, bak, old_mouse_w);
      bak += BAK_WIDTH;
      dst += 320;
      old_h--;
    }
  }

  if (visible) {
    int real_w;
    int real_h;
    int real_h_2;
    unsigned char *dst2;

    if (ydraw < 0) {
      real_h = h + ydraw;
      buf += (-ydraw) * w;
      ydraw = 0;
    } else {
      real_h = (ydraw + h) > 200 ? (200 - ydraw) : h;
    }
    if (xdraw < 0) {
      real_w = w + xdraw;
      buf += (-xdraw);
      xdraw = 0;
    } else {
      real_w = (xdraw + w) > 320 ? (320 - xdraw) : w;
    }
    
    dst =  wm->_vgabuf + (ydraw * 320) + xdraw;
    dst2 = dst;
    bak = old_backup;
        
    has_mouse = true;

    AddDirtyRec(xdraw, ydraw, real_w, real_h);
    old_mouse_x = xdraw;
    old_mouse_y = ydraw;
    old_mouse_w = real_w;
    old_mouse_h = real_h;
    
    real_h_2 = real_h;
    while (real_h_2 > 0) {
      memcpy(bak, dst, real_w);
      bak += BAK_WIDTH;
      dst += 320;
      real_h_2--;
    }
    while (real_h > 0) {
      int width = real_w;
      while (width > 0) {
	unsigned char color = *buf;
	if (color != 0xFF) {
	  *dst2 = color;
	}
	buf++;
	dst2++;
	width--;
      }
      buf += w - real_w;
      dst2 += 320 - real_w;
      real_h--;
    }
  }
}
*/


BOOL CALLBACK SelectDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			TCHAR work[1024];
			RECT rc; GetWindowRect(hwndDlg, &rc);
			MoveWindow(hwndDlg,
				(GetSystemMetrics(SM_CXSCREEN)-rc.right+rc.left)/2,
				(GetSystemMetrics(SM_CYSCREEN)-rc.bottom+rc.top)/2,
				rc.right-rc.left, rc.bottom-rc.top, TRUE);
			BringWindowToTop(hwndDlg);
			setFindGameDlgHandle(hwndDlg);
			MultiByteToWideChar(CP_ACP, 0, VERSION, strlen(VERSION) + 1, work, sizeof(work));
			SetDlgItemText(hwndDlg, IDC_GAMEDESC, work);
			loadGameSettings();
		}
		return TRUE;

	case WM_COMMAND:

		if (LOWORD(wParam) == IDC_LISTAVAILABLE && HIWORD(wParam) == LBN_SELCHANGE)
			displayGameInfo();

		if (wParam == IDC_SCAN)
			startFindGame();

		if (wParam == IDC_PLAY) {
			int item;

			item = SendMessage(GetDlgItem(hwndDlg, IDC_LISTAVAILABLE), LB_GETCURSEL, 0, 0);
			if (item == LB_ERR) {
				MessageBox(hwndDlg, TEXT("Please select a game"), TEXT("Error"), MB_OK);
			}
			else
				EndDialog(hwndDlg, item + 1000);
		}

		if (wParam == IDC_EXIT)
			EndDialog(hwndDlg, 0);	
		return TRUE;
	default:
		return FALSE;
	}
}

char* GameSelector()
{
	TCHAR directory[MAX_PATH];	
	static char  id[100];

	DWORD result = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_GAMESELECT), HWND_DESKTOP, SelectDlgProc);
	if (result < 1000)
		return NULL;
	result -= 1000;

	getSelectedGame(result, id, directory);

	WideCharToMultiByte(CP_ACP, 0, directory, wcslen(directory) + 1, _directory, sizeof(_directory), NULL, NULL);

	strcat(_directory, "\\");
	
	return id;
	
}

// Directly corresponds to main in windows.cpp
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
	
	int argc = 3;
	char* argv[3];
	char argdir[MAX_PATH];
	sound_activated = true;
	hide_toolbar = false;

	argv[0] = NULL;
	argv[1] = GameSelector();
	sprintf(argdir, "-p%s", _directory);
	argv[2] = argdir;

	if (!argv[1])
		return 0;

	if (detector.detectMain(argc, argv))
		return (-1);

	OSystem *system = detector.createSystem();
	
	/* Simon the Sorcerer? */
	if (detector._gameId >= GID_SIMON_FIRST && detector._gameId <= GID_SIMON_LAST) {
		/* Simon the Sorcerer. Completely different initialization */
		MidiDriver *midi = detector.createMidi();
		
		SimonState *simon = SimonState::create(system, midi);
		simon->_game = detector._gameId - GID_SIMON_FIRST;
		simon->set_volume(detector._sfx_volume);
		simon->_game_path = detector._gameDataPath;
		simon->go();

	} else {
		Scumm *scumm = Scumm::createFromDetector(&detector, system);
		g_scumm = scumm;

		/* bind to Gui */
		scumm->_gui = &gui;
		gui.init(scumm);	/* Reinit GUI after loading a game */

		scumm->go();
	}
	
	return 0;
}

class OSystem_WINCE3 : public OSystem {
public:
	// Set colors of the palette
	void set_palette(const byte *colors, uint start, uint num);

	// Set the size of the video bitmap.
	// Typically, 320x200
	void init_size(uint w, uint h);

	// Draw a bitmap to screen.
	// The screen will not be updated to reflect the new bitmap
	void copy_rect(const byte *buf, int pitch, int x, int y, int w, int h);

	// Update the dirty areas of the screen
	void update_screen();

	// Either show or hide the mouse cursor
	bool show_mouse(bool visible);
	
	// Set the position of the mouse cursor
	void set_mouse_pos(int x, int y);
	
	// Set the bitmap that's used when drawing the cursor.
	void set_mouse_cursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y);
	
	// Shaking is used in SCUMM. Set current shake position.
	void set_shake_pos(int shake_pos);
		
	// Get the number of milliseconds since the program was started.
	uint32 get_msecs();
	
	// Delay for a specified amount of milliseconds
	void delay_msecs(uint msecs);
	
	// Create a thread
	void *create_thread(ThreadProc *proc, void *param);
	
	// Get the next event.
	// Returns true if an event was retrieved.	
	bool poll_event(Event *event);
	
	// Set function that generates samples 
	bool set_sound_proc(void *param, SoundProc *proc, byte sound);
		
	// Poll cdrom status
	// Returns true if cd audio is playing
	bool poll_cdrom();

	// Play cdrom audio track
	void play_cdrom(int track, int num_loops, int start_frame, int end_frame);

	// Stop cdrom audio track
	void stop_cdrom();

	// Update cdrom audio status
	void update_cdrom();

	// Quit
	void quit();

	// Set a parameter
	uint32 property(int param, uint32 value);

	// Windows callbacks & stuff

	bool handleMessage();
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	// Constructor

	static OSystem *create();

private:

	byte *_vgabuf;
	uint32 _start_time;
	Event _event;
	HMODULE hInst;
	HWND hWnd;
	bool terminated;	
};

// Create class

	OSystem* OSystem_WINCE3_create() {
	return OSystem_WINCE3::create();
}

// Constructor

OSystem* OSystem_WINCE3::create() {

	OSystem_WINCE3 *syst = new OSystem_WINCE3();

	syst->_vgabuf = (byte*)calloc(320,200);
	syst->_event.event_code = -1;

	/* Retrieve the handle of this module */
	syst->hInst = GetModuleHandle(NULL);

	/* Register the window class */
	WNDCLASS wcex;
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= syst->hInst;
	wcex.hIcon			= 0;
	wcex.hCursor		= NULL;
	wcex.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName	= 0;	
	wcex.lpszClassName	= TEXT("ScummVM");
	if (!RegisterClass(&wcex))
		Error(TEXT("Cannot register window class!"));

	syst->hWnd = CreateWindow(TEXT("ScummVM"), TEXT("ScummVM"), WS_VISIBLE,
      0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), NULL, NULL, syst->hInst, NULL);
	hWnd_Window = syst->hWnd;
	SetWindowLong(syst->hWnd, GWL_USERDATA, (long)syst);

	ShowWindow(syst->hWnd, SW_SHOW);

	SHMENUBARINFO smbi;
	smbi.cbSize = sizeof(smbi); 
	smbi.hwndParent = syst->hWnd; 
	smbi.dwFlags = 0; 
	smbi.nToolBarId = IDM_MENU; 
	smbi.hInstRes = GetModuleHandle(NULL); 
	smbi.nBmpId = 0; 
	smbi.cBmpImages = 0; 
	smbi.hwndMB = NULL;
	BOOL res = SHCreateMenuBar(&smbi);
	hWnd_MainMenu = smbi.hwndMB;

	/* Sound is activated on default - initialize it in the menu */
	CheckMenuItem((HMENU)SHMenuBar_GetMenu (hWnd_MainMenu, IDM_POCKETSCUMM),
		IDC_SOUND, 
		MF_BYCOMMAND | MF_CHECKED);

	GraphicsOn(syst->hWnd);

	SetWindowPos(syst->hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	SetForegroundWindow(syst->hWnd);
//	SHFullScreen(hWnd, SHFS_SHOWSIPBUTTON);
	SHFullScreen(syst->hWnd, SHFS_HIDESIPBUTTON | SHFS_HIDETASKBAR | SHFS_HIDESTARTICON);

	//Cls();

// Mini SDL init

	if (SDL_Init(SDL_INIT_AUDIO)==-1) {		
	    exit(1);
	}

	return syst;
}

// Windows specific callbacks

LRESULT CALLBACK OSystem_WINCE3::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static		 SHACTIVATEINFO sai;

	OSystem_WINCE3 *wm = (OSystem_WINCE3*)GetWindowLong(hWnd, GWL_USERDATA);	

	switch (message) 
	{
	case WM_CREATE:
		memset(&sai, 0, sizeof(sai));
		SHSipPreference(hWnd, SIP_FORCEDOWN);
//		SHSipPreference(hWnd, SIP_INPUTDIALOG);

		return 0;

	case WM_DESTROY:
	case WM_CLOSE:
		GraphicsOff();
		PostQuitMessage(0);
		break;

	case WM_ERASEBKGND:
		{
			RECT rc;
			HDC hDC;
			GetClientRect(hWnd, &rc);
			rc.top = 200;
			hDC = GetDC(hWnd);
			if(rc.top < rc.bottom)
				FillRect(hDC, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
			ReleaseDC(hWnd, hDC);			
		}
		return 1;

	case WM_PAINT:
		{
			HDC hDC;
			PAINTSTRUCT ps;
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
		}
//		SHSipPreference(hWnd, SIP_UP); /* Hack! */
		/* It does not happen often but I don't want to see tooltip traces */
		wm->update_screen();
		return 0;

	case WM_ACTIVATE:
		GraphicsResume();
		if (!hide_toolbar)
			toolbar_drawn = false;
//		SHHandleWMActivate(hWnd, wParam, lParam, &sai, SHA_INPUTDIALOG);
		return 0;

	case WM_HIBERNATE:
		GraphicsSuspend();
		if (!hide_toolbar)
			toolbar_drawn = false;
		return 0;

	case WM_SETTINGCHANGE:
		SHHandleWMSettingChange(hWnd, wParam, lParam, &sai);
		if (!hide_toolbar)
			toolbar_drawn = false;
		return 0;

	case WM_COMMAND:
		switch(wParam)
		{
		case IDC_OPTIONS:			
			wm->_event.kbd.ascii = KEY_SET_OPTIONS;
			wm->_event.event_code = EVENT_KEYDOWN;
			break;
		case IDC_EXIT:
			DestroyWindow(hWnd);
			do_quit();			
			break;
		case IDC_SKIP:
			wm->_event.kbd.ascii = mapKey(VK_ESCAPE);;
			wm->_event.event_code = EVENT_KEYDOWN;
			break;
		case IDC_LOADSAVE:
			if (GetScreenMode()) {
				draw_keyboard = true;
				if (!hide_toolbar)
					toolbar_drawn = false;
			}
			wm->_event.kbd.ascii = mapKey(VK_F5);
			wm->_event.event_code = EVENT_KEYDOWN;
			break;
		
		case IDC_SOUND:
			// FIXME
			sound_activated = !sound_activated;
			CheckMenuItem (
				SHMenuBar_GetMenu (hWnd_MainMenu, IDM_POCKETSCUMM),
				IDC_SOUND, 
				MF_BYCOMMAND | (sound_activated ? MF_CHECKED : MF_UNCHECKED));	
			//wm->_scumm->_soundsPaused2 = !sound_activated;
			break;     
		
      break;

		case IDC_LANDSCAPE:
			SetScreenMode(!GetScreenMode());
			SHSipPreference(hWnd,SIP_FORCEDOWN);
			SetCapture(hWnd); // to prevent input panel from getting taps
			SHFullScreen (hWnd, SHFS_HIDESIPBUTTON | SHFS_HIDETASKBAR | SHFS_HIDESTARTICON);
			InvalidateRect(HWND_DESKTOP, NULL, TRUE);
			/*
			SipShowIM(SIPF_OFF);
			SHSipPreference(hWnd, SIP_FORCEDOWN);
			*/
			if (!hide_toolbar)
				toolbar_drawn = false;
			break;

		}
		return 0;
	
	case WM_KEYDOWN:
		if(wParam) { // gets rid of zero that seems to preceed GAPI events.

			unsigned char GAPI_key;

			GAPI_key = getGAPIKeyMapping((short)wParam);
			if (GAPI_key) {
				if (get_key_mapping) {
					wm->_event.kbd.ascii = GAPI_KEY_BASE + GAPI_key;
					wm->_event.event_code = EVENT_KEYDOWN;
				}					
				else
					processAction((short)wParam);
			}
			else {
				wm->_event.kbd.ascii = mapKey(wParam);
				wm->_event.event_code = EVENT_KEYDOWN;								
			}
		}

		break;

	case WM_MOUSEMOVE:
		{
			int x = ((int16*)&lParam)[0];
			int y = ((int16*)&lParam)[1];
			Translate(&x, &y);
			wm->_event.event_code = EVENT_MOUSEMOVE;
			wm->_event.mouse.x = x;
			wm->_event.mouse.y = y;
		}
		break;
	case WM_LBUTTONDOWN:
		{
			ToolbarSelected toolbar_selection;
			int x = ((int16*)&lParam)[0];
			int y = ((int16*)&lParam)[1];
			Translate(&x, &y);

			if (draw_keyboard) {
				// Handle keyboard selection

				if (x<185 && y>=200) {
					//Alpha selection
					wm->_event.event_code = EVENT_KEYDOWN;
					wm->_event.kbd.ascii = 
						(y <= 220 ? KEYBOARD_MAPPING_ALPHA_HIGH[((x + 10) / 14) - 1] :
									KEYBOARD_MAPPING_ALPHA_LOW[((x + 10) / 14) - 1]);
					break;
				} 
				else
				if (x>=186 && y>=200 && x<=255) {
				   // Numeric selection
				   wm->_event.event_code = EVENT_KEYDOWN;
				   wm->_event.kbd.ascii =
					   (y <= 220 ? KEYBOARD_MAPPING_NUMERIC_HIGH[((x - 187 + 10) / 14) - 1] :
								   KEYBOARD_MAPPING_NUMERIC_LOW[((x - 187 + 10) / 14) - 1]);
				   break;
				}
				else
				if (x>=302 && x <= 316 && y >= 200 && y <= 220) {
				  // Backspace
				  wm->_event.event_code = EVENT_KEYDOWN;
				  wm->_event.kbd.ascii = mapKey(VK_BACK);
				  break;
				}

				wm->_event.event_code = EVENT_LBUTTONDOWN;
				wm->_event.mouse.x = x;
				wm->_event.mouse.y = y;
				break;

			}
					

			toolbar_selection = (hide_toolbar || get_key_mapping ? ToolbarNone : 
									getToolbarSelection(x, y));
			if (toolbar_selection == ToolbarNone) {				
				wm->_event.event_code = EVENT_LBUTTONDOWN;
				wm->_event.mouse.x = x;
				wm->_event.mouse.y = y;
			
				if(y > 200 && !hide_toolbar)
				{
					if(x<160) {
					   wm->_event.event_code = EVENT_KEYDOWN;
					   wm->_event.kbd.ascii = mapKey(VK_ESCAPE);
					}
					else
					{
						SetScreenMode(0); // restore normal tap logic
						SHSipPreference(hWnd,SIP_UP);
						ReleaseCapture();
						InvalidateRect(HWND_DESKTOP, NULL, TRUE);
					}				
				}			
			}
			else {
				switch(toolbar_selection) {
					case ToolbarSaveLoad:
						if (GetScreenMode()) {
							draw_keyboard = true;
							if (!hide_toolbar)
								toolbar_drawn = false;
						}
						wm->_event.event_code = EVENT_KEYDOWN;
						wm->_event.kbd.ascii = mapKey(VK_F5);
						break;
					case ToolbarExit:
						wm->_event.event_code = EVENT_KEYDOWN;
						wm->_event.kbd.ascii = KEY_SET_OPTIONS;
						break;
					case ToolbarSkip:
						wm->_event.event_code = EVENT_KEYDOWN;
						wm->_event.kbd.ascii = mapKey(VK_ESCAPE);				
						break;
					case ToolbarSound:
						// FIXME !!!!!
						sound_activated = !sound_activated;
						//wm->_scumm->_soundsPaused2 = !sound_activated;
						redrawSoundItem();
						break;
					default:
						break;
				}
			}
		}
		break;
	case WM_LBUTTONUP:
		{
			// pinched from the SDL code. Distinguishes between taps and not
			wm->_event.event_code = EVENT_LBUTTONUP;

		}
		break;
	case WM_LBUTTONDBLCLK:  // doesn't seem to work right now
		//wm->_scumm->_rightBtnPressed |= msClicked | msDown;
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}


bool OSystem_WINCE3::handleMessage() {
	MSG msg;

	if (!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		return false;

	if (msg.message==WM_QUIT) {
		terminated=true;
		do_quit();
		return true;
	}

	TranslateMessage(&msg);
	DispatchMessage(&msg);

	return true;
}

void OSystem_WINCE3::set_palette(const byte *colors, uint start, uint num) {
	int i;

	for (i=0; i<256; i++)
		SetPalEntry(i, colors[i*3+0], colors[i*3+1], colors[i*3+2]);

	palette_update();

}

void OSystem_WINCE3::init_size(uint w, uint h) {
}

void OSystem_WINCE3::copy_rect(const byte *buf, int pitch, int x, int y, int w, int h) {
	byte *dst;

	dst = _vgabuf + y * 320 + x;
	do {
		memcpy(dst, buf, w);
		dst += 320;
		buf += pitch;
	} while (--h);
		
}

void OSystem_WINCE3::update_screen() {
	Blt(_vgabuf);
}

// FIXME : implement mouse functions

bool OSystem_WINCE3::show_mouse(bool visible) {
	return false;
}
	

void OSystem_WINCE3::set_mouse_pos(int x, int y) {
}
	

void OSystem_WINCE3::set_mouse_cursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y) {
}
	
void OSystem_WINCE3::set_shake_pos(int shake_pos) {
}
		

uint32 OSystem_WINCE3::get_msecs() {
	return GetTickCount() - _start_time;
}
		
void OSystem_WINCE3::delay_msecs(uint msecs) {
	handleMessage();
	Sleep(msecs);
}
	

void* OSystem_WINCE3::create_thread(ThreadProc *proc, void *param) {
	// Implement if needed by Midi Music
	return NULL;
}
	
//FIXME : add a minimal queue (just testing here)

bool OSystem_WINCE3::poll_event(Event *event) {
	if (_event.event_code < 0)
		return false;

	*event = _event;
	_event.event_code = -1;

	return true;
}
	
	
bool OSystem_WINCE3::set_sound_proc(void *param, SoundProc *proc, byte sound) {
	SDL_AudioSpec desired;

	desired.freq = 11025;
	desired.format = AUDIO_S16SYS;
	desired.channels = 1;
	desired.samples = 128;               // seems correct
	desired.callback = proc;
	desired.userdata = param;
	SDL_OpenAudio(&desired, NULL);
	SDL_PauseAudio(0);
	desired.userdata = param;

	return true;
}
		
// No CD functions to implement

bool OSystem_WINCE3::poll_cdrom() {
	return false;
}

void OSystem_WINCE3::play_cdrom(int track, int num_loops, int start_frame, int end_frame) {
}

void OSystem_WINCE3::stop_cdrom() {
}

void OSystem_WINCE3::update_cdrom() {
}

void OSystem_WINCE3::quit() {
	do_quit();
}

uint32 OSystem_WINCE3::property(int param, uint32 value) {
	switch(param) {
		case PROP_GET_SAMPLE_RATE:
			return 11025;
	}

	return 0;
}

OSystem *OSystem_NULL_create() {
	return NULL;
}

void ScummDebugger::attach(Scumm *s) {
}
