#include "stdafx.h"
#include <assert.h>

#include "scumm.h"
#include "screen.h"
#include "gui.h"
#include "sound/mididrv.h"
#include "gameDetector.h"
#include "simon/simon.h"
#include "gapi_keys.h"
#include "config-file.h"


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


#define MAX(a,b) (((a)<(b)) ? (b) : (a))
#define MIN(a,b) (((a)>(b)) ? (b) : (a))
#define POCKETSCUMM_BUILD "051502"

#define VERSION "Build " POCKETSCUMM_BUILD " (VM " SCUMMVM_CVS ")"

typedef int (*tTimeCallback)(int);

GameDetector detector;
Gui gui;
Scumm *g_scumm;
Config *scummcfg;
tTimeCallback timer_callback;
int timer_interval;


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

	// Add a new callback timer
	void set_timer(int timer, int (*callback)(int));

	// Quit
	void quit();

	// Set a parameter
	uint32 property(int param, Property *value);

	static OSystem *create(int gfx_mode, bool full_screen);

	// Added for hardware keys mapping

	void addEventKeyPressed(int ascii_code);

	void addEventRightButtonClicked();

private:
	// Windows callbacks & stuff
	//bool handleMessage();
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	
	byte *_gfx_buf;
	uint32 _start_time;
	Event _event;
	Event _last_mouse_event;
	HMODULE hInst;
	HWND hWnd;
	bool _display_cursor;	

	enum {
		DF_FORCE_FULL_ON_PALETTE = 1,
		DF_WANT_RECT_OPTIM = 2,
		DF_2xSAI = 4,
		DF_SEPARATE_HWSCREEN = 8,
		DF_UPDATE_EXPAND_1_PIXEL = 16,
	};

	int _mode;
	bool _full_screen;
	bool _mouse_visible;
	bool _mouse_drawn;
	uint32 _mode_flags;

	byte _internal_scaling;

	bool force_full; //Force full redraw on next update_screen
	bool cksum_valid;

	enum {
		NUM_DIRTY_RECT = 100,
		SCREEN_WIDTH = 320,
		SCREEN_HEIGHT = 200,
		CKSUM_NUM = (SCREEN_WIDTH*SCREEN_HEIGHT/(8*8)),

		MAX_MOUSE_W = 40,
		MAX_MOUSE_H = 40,
		MAX_SCALING = 3,

		TMP_SCREEN_OFFS = 320*2 + 8,
	};

	/* CD Audio */
	int cd_track, cd_num_loops, cd_start_frame, cd_end_frame;
	Uint32 cd_end_time, cd_stop_time, cd_next_second;

	struct MousePos {
		int16 x,y,w,h;
	};

	byte *_ms_buf;
	byte *_ms_backup;
	MousePos _ms_cur;
	MousePos _ms_old;
	int16 _ms_hotspot_x;
	int16 _ms_hotspot_y;
	int _current_shake_pos;

	
	static void fill_sound(void *userdata, Uint8 * stream, int len);
	

	void draw_mouse();
	void undraw_mouse();

	void load_gfx_mode();
	void unload_gfx_mode();

	void hotswap_gfx_mode();

	void get_320x200_image(byte *buf);
};

/************* WinCE Specifics *****************/
byte veryFastMode;

bool sound_activated, terminated;
HWND hWnd_MainMenu;
HWND hWnd_Window;

void drawAllToolbar(bool);
void redrawSoundItem();
ToolbarSelected getToolbarSelection(int, int);

extern bool toolbar_drawn;
extern bool draw_keyboard;
bool hide_toolbar;
bool hide_cursor;

bool get_key_mapping;
static char _directory[MAX_PATH];

const char KEYBOARD_MAPPING_ALPHA_HIGH[] = {"ABCDEFGHIJKLM"};
const char KEYBOARD_MAPPING_NUMERIC_HIGH[] = {"12345"};
const char KEYBOARD_MAPPING_ALPHA_LOW[] = {"NOPQRSTUVWXYZ"};
const char KEYBOARD_MAPPING_NUMERIC_LOW[] = {"67890"};

extern void startFindGame();
extern void displayGameInfo();
extern bool loadGameSettings(void);
extern void setFindGameDlgHandle(HWND);
extern void getSelectedGame(int, char*, TCHAR*);

extern void palette_update();
#define SHMenuBar_GetMenu(hWndMB,ID_MENU) (HMENU)SendMessage((hWndMB), SHCMBM_GETSUBMENU, (WPARAM)0, (LPARAM)ID_MENU)

void do_quit() {
	GXCloseInput();
	GXCloseDisplay();
	SDL_AudioQuit();
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

		if (LOWORD(wParam) == IDC_LISTAVAILABLE && HIWORD(wParam) == LBN_SELCHANGE) {
			if (!isPrescanning()) 
				displayGameInfo();
			else
				changeScanPath();
		}

		if (wParam == IDC_SCAN) {
			if (!isPrescanning()) 
				startScan();
			else
				endScanPath();
		}
		
		if (wParam == IDC_PLAY) {
			int item;

			item = SendMessage(GetDlgItem(hwndDlg, IDC_LISTAVAILABLE), LB_GETCURSEL, 0, 0);
			if (item == LB_ERR) {
				MessageBox(hwndDlg, TEXT("Please select a game"), TEXT("Error"), MB_OK);
			}
			else
				EndDialog(hwndDlg, item + 1000);
		}

		if (wParam == IDC_EXIT) {
			if (!isPrescanning()) 
				EndDialog(hwndDlg, 0);	
			else
				abortScanPath();
		}
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
	
	int argc = 3;
	char* argv[3];
	char argdir[MAX_PATH];
	char *game_name;

	sound_activated = true;
	hide_toolbar = false;

	scummcfg = new Config("scummvm.ini", "scummvm");
	scummcfg->set_writing(true);

	game_name = GameSelector();
	if (!game_name)
		return 0;

	argv[0] = NULL;	
	sprintf(argdir, "-p%s", _directory);
	argv[1] = argdir;
	argv[2] = game_name;

	if (!argv[2])
		return 0;

	// No default toolbar for zak256
	if (strcmp(game_name, "zak256") == 0)
		hide_toolbar = true;

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

		g_scumm->_sound_volume_master = 0;
		g_scumm->_sound_volume_music = detector._music_volume;
		g_scumm->_sound_volume_sfx = detector._sfx_volume;

		keypad_init();
		load_key_mapping();
		
		hide_cursor = TRUE;
		if (scumm->_gameId == GID_SAMNMAX || scumm->_gameId == GID_FT || scumm->_gameId == GID_DIG)
			hide_cursor = FALSE;

		/* bind to Gui */
		scumm->_gui = &gui;
		gui.init(scumm);	/* Reinit GUI after loading a game */
		scumm->go();
	}
	
	return 0;
}



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
			if (!GetScreenMode()) {
				GetClientRect(hWnd, &rc);
				rc.top = 200;
				hDC = GetDC(hWnd);
				if(rc.top < rc.bottom)
					FillRect(hDC, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
				ReleaseDC(hWnd, hDC);			
			}			
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
			SHSipPreference(hWnd, SIP_FORCEDOWN);
		}
//		SHSipPreference(hWnd, SIP_UP); /* Hack! */
		/* It does not happen often but I don't want to see tooltip traces */
		wm->update_screen();
		return 0;

	case WM_ACTIVATE:
	case WM_SETFOCUS:	
		GraphicsResume();
		if (!hide_toolbar)
			toolbar_drawn = false;
//		SHHandleWMActivate(hWnd, wParam, lParam, &sai, SHA_INPUTDIALOG);

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

		return 0;

	case WM_HIBERNATE:
	case WM_KILLFOCUS:
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
			if (g_scumm->vm.cutScenePtr[g_scumm->vm.cutSceneStackPointer])
				wm->_event.kbd.ascii = g_scumm->_vars[g_scumm->VAR_CUTSCENEEXIT_KEY];
			else
				wm->_event.kbd.ascii = g_scumm->_vars[g_scumm->VAR_TALKSTOP_KEY];						
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
			sound_activated = !sound_activated;
			CheckMenuItem (
				SHMenuBar_GetMenu (hWnd_MainMenu, IDM_POCKETSCUMM),
				IDC_SOUND, 
				MF_BYCOMMAND | (sound_activated ? MF_CHECKED : MF_UNCHECKED));	
			g_scumm->pauseSounds(!sound_activated);
			break;     
		
      break;

		case IDC_LANDSCAPE:
			//HWND taskbar;
			//SHFullScreen (hWnd, SHFS_HIDESIPBUTTON | SHFS_HIDETASKBAR | SHFS_HIDESTARTICON);
			//InvalidateRect(HWND_DESKTOP, NULL, TRUE);
			SetScreenMode(!GetScreenMode());
			//SHSipPreference(hWnd,SIP_FORCEDOWN);
			//MoveWindow(hWnd, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), TRUE);
			//SetCapture(hWnd); // to prevent input panel from getting taps						
			/*taskbar = FindWindow(TEXT("HHTaskBar"), NULL);
			if (taskbar)
				ShowWindow(taskbar, SW_HIDE);*/
			/*SHSipPreference(hWnd, SIP_FORCEDOWN);
			SHFullScreen(hWnd, SHFS_HIDETASKBAR);*/
			SetForegroundWindow(hWnd);
			MoveWindow(hWnd, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), TRUE);
			SetCapture(hWnd);
			UpdateWindow(hWnd);
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

	case WM_KEYUP:
		break;

	case WM_MOUSEMOVE:
		{
			int x = ((int16*)&lParam)[0];
			int y = ((int16*)&lParam)[1];
			Translate(&x, &y);
			wm->_event.event_code = EVENT_MOUSEMOVE;
			wm->_event.mouse.x = x;
			wm->_event.mouse.y = y;
			wm->_last_mouse_event = wm->_event;
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
				wm->_last_mouse_event = wm->_event;
				break;

			}
					

			toolbar_selection = (hide_toolbar || get_key_mapping ? ToolbarNone : 
									getToolbarSelection(x, y));
			if (toolbar_selection == ToolbarNone) {				
				wm->_event.event_code = EVENT_LBUTTONDOWN;
				wm->_event.mouse.x = x;
				wm->_event.mouse.y = y;
				wm->_last_mouse_event = wm->_event;
			
				if(y > 200 && !hide_toolbar)
				{
					if(x<160) {
					   wm->_event.event_code = EVENT_KEYDOWN;
					   wm->_event.kbd.ascii = mapKey(VK_ESCAPE);
					}
					else
					{
						HDC hDC;
						PAINTSTRUCT ps;

						SetScreenMode(0); // restore normal tap logic
						//SHSipPreference(hWnd,SIP_UP);
						ReleaseCapture();
						//InvalidateRect(HWND_DESKTOP, NULL, TRUE);		
						SHFullScreen(hWnd, SHFS_HIDESIPBUTTON | SHFS_HIDETASKBAR | SHFS_HIDESTARTICON);
						MoveWindow(hWnd, 0, 0, GetSystemMetrics(SM_CYSCREEN), GetSystemMetrics(SM_CXSCREEN), TRUE);
						SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
						SetForegroundWindow(hWnd);						
						hDC = BeginPaint (hWnd, &ps);
						EndPaint (hWnd, &ps);
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
						if (g_scumm->vm.cutScenePtr[g_scumm->vm.cutSceneStackPointer])
							wm->_event.kbd.ascii = g_scumm->_vars[g_scumm->VAR_CUTSCENEEXIT_KEY];
						else
							wm->_event.kbd.ascii = g_scumm->_vars[g_scumm->VAR_TALKSTOP_KEY];						
						break;
					case ToolbarSound:
						sound_activated = !sound_activated;
						g_scumm->pauseSounds(!sound_activated);
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
			int x = ((int16*)&lParam)[0];
			int y = ((int16*)&lParam)[1];
			Translate(&x, &y);
			wm->_event.event_code = EVENT_LBUTTONUP;
			wm->_event.mouse.x = x;
			wm->_event.mouse.y = y;
			wm->_last_mouse_event = wm->_event;
		}
		break;
	case WM_LBUTTONDBLCLK:  // doesn't seem to work right now
		//wm->_scumm->_rightBtnPressed |= msClicked | msDown;
		break;
	case WM_TIMER:
		timer_callback(timer_interval);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

/*************** Specific config support ***********/

void load_key_mapping() {
	 unsigned char actions[NUMBER_ACTIONS];
	 const char		*current;
	 int			i;

	 memset(actions, 0, NUMBER_ACTIONS);

	 current = scummcfg->get("ActionKeys", "wince");
	 if (current) {
		for (i=0; i<NUMBER_ACTIONS; i++) {
			char x[6];
			int j;

			memset(x, 0, sizeof(x));
			memcpy(x, current + 3 * i, 2);
			sscanf(x, "%x", &j);
			actions[i] = j;
		}
	 }
	 setActionKeys(actions);

	 memset(actions, 0, NUMBER_ACTIONS);

	 actions[0] = ACTION_PAUSE;
	 actions[1] = ACTION_SAVE;
	 actions[2] = ACTION_QUIT;
	 actions[3] = ACTION_SKIP;
	 actions[4] = ACTION_HIDE;

	 current = scummcfg->get("ActionTypes", "wince");
	 if (current) {
		for (i=0; i<NUMBER_ACTIONS; i++) {
			char x[6];
			int j;

			memset(x, 0, sizeof(x));
			memcpy(x, current + 3 * i, 2);
			sscanf(x, "%x", &j);
			actions[i] = j;
		}
	 }
	 setActionTypes(actions);
}
					
void save_key_mapping() {
	 char tempo[1024];
	 const unsigned char *work;
	 int i;

	 tempo[0] = '\0';
	 work = getActionKeys();
	 for (i=0; i<NUMBER_ACTIONS; i++) {
		 char x[4];
		 sprintf(x, "%.2x ", work[i]);
		 strcat(tempo, x);
	 }
	 scummcfg->set("ActionKeys", tempo, "wince");
	 tempo[0] = '\0';
	 work = getActionTypes();
	 for (i=0; i<NUMBER_ACTIONS; i++) {
		 char x[3];
		 sprintf(x, "%.2x ", work[i]);
		 strcat(tempo, x);
	 }
	 scummcfg->set("ActionTypes", tempo, "wince");

	 scummcfg->flush();
}

/*************** Hardware keys support ***********/

void OSystem_WINCE3::addEventKeyPressed(int ascii_code) {
	_event.event_code = EVENT_KEYDOWN;
	_event.kbd.ascii = ascii_code;
}

void OSystem_WINCE3::addEventRightButtonClicked() {
	OSystem_WINCE3* system;
	system = (OSystem_WINCE3*)g_scumm->_system;
	
	system->addEventKeyPressed(9);
}

void action_right_click() {
	OSystem_WINCE3* system;
	system = (OSystem_WINCE3*)g_scumm->_system;

	system->addEventRightButtonClicked();	
}

void action_pause() {
	OSystem_WINCE3* system;
	system = (OSystem_WINCE3*)g_scumm->_system;

	system->addEventKeyPressed(mapKey(VK_SPACE));
}

void action_save() {
	OSystem_WINCE3* system;
	system = (OSystem_WINCE3*)g_scumm->_system;

	if (GetScreenMode()) {
		draw_keyboard = true;
		if (!hide_toolbar)
			toolbar_drawn = false;
	}

	system->addEventKeyPressed(mapKey(VK_F5));
}

void action_quit() {
	do_quit();
}

void action_skip() {
	OSystem_WINCE3* system;
	system = (OSystem_WINCE3*)g_scumm->_system;

	if (g_scumm->vm.cutScenePtr[g_scumm->vm.cutSceneStackPointer])
		system->addEventKeyPressed(g_scumm->_vars[g_scumm->VAR_CUTSCENEEXIT_KEY]);
	else
		system->addEventKeyPressed(g_scumm->_vars[g_scumm->VAR_TALKSTOP_KEY]);						
}

void action_hide() {
	hide_toolbar = !hide_toolbar;
	if (hide_toolbar)
		RestoreScreenGeometry();
	else
		LimitScreenGeometry();
	Cls();
	toolbar_drawn = hide_toolbar;
	g_scumm->_system->update_screen();
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
	g_scumm->pauseSounds(!sound_activated);
}

void action_cursoronoff() {
	hide_cursor = !hide_cursor;
}

void action_subtitleonoff() {
	g_scumm->_noSubtitles = !g_scumm->_noSubtitles;
}

void keypad_init() {
	static pAction actions[TOTAL_ACTIONS] =
	{ action_pause, action_save, action_quit, action_skip, action_hide, 
	  action_keyboard, action_sound, action_right_click, action_cursoronoff,
	  action_subtitleonoff
	};
	
	GAPIKeysInit(actions);
	
}

void keypad_close() {
	GXCloseInput();	
}


/************* OSystem Main **********************/
OSystem *OSystem_WINCE3::create(int gfx_mode, bool full_screen) {
	OSystem_WINCE3 *syst = new OSystem_WINCE3();
	syst->_mode = gfx_mode;
	syst->_full_screen = full_screen;
	syst->_event.event_code = -1;
	syst->_start_time = GetTickCount();

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
	UpdateWindow(syst->hWnd);

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
		IDC_SOUND, MF_BYCOMMAND | MF_CHECKED);

	GraphicsOn(syst->hWnd);

	SetWindowPos(syst->hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	SetForegroundWindow(syst->hWnd);
	SHFullScreen(syst->hWnd, SHFS_HIDESIPBUTTON | SHFS_HIDETASKBAR | SHFS_HIDESTARTICON);

	
	// Mini SDL init

	if (SDL_Init(SDL_INIT_AUDIO)==-1) {		
	    exit(1);
	}

	return syst;
}

OSystem *OSystem_WINCE3_create() {
	return OSystem_WINCE3::create(0, 0);
}

void OSystem_WINCE3::set_timer(int timer, int (*callback)(int)) {
	SetTimer(hWnd, 1, timer, NULL);
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
}

void OSystem_WINCE3::load_gfx_mode() {
	force_full = true;

	_gfx_buf = (byte*)malloc((320 * 240) * sizeof(byte));	
	_ms_backup = (byte*)malloc((40 * 40 * 3) * sizeof(byte));
}

void OSystem_WINCE3::unload_gfx_mode() {
 // FIXME: Free the _gfx_buf here
}

void OSystem_WINCE3::init_size(uint w, uint h) {
	load_gfx_mode();
	SetScreenGeometry(w, h);
}

void OSystem_WINCE3::copy_rect(const byte *buf, int pitch, int x, int y, int w, int h) {
	byte *dst;

	if (!hide_cursor && _mouse_drawn)
		undraw_mouse();

	dst = _gfx_buf + y * 320 + x;
	do {
		memcpy(dst, buf, w);
		dst += 320;
		buf += pitch;
	} while (--h);
}

void OSystem_WINCE3::update_screen() {

	if (!hide_cursor)
		draw_mouse();

	Blt(_gfx_buf);
}

bool OSystem_WINCE3::show_mouse(bool visible) {
	if (_mouse_visible == visible)
		return visible;
	
	bool last = _mouse_visible;
	_mouse_visible = visible;

	return last;
}

// From X11 port

void OSystem_WINCE3::draw_mouse() {
	if (_mouse_drawn || !_mouse_visible)
		return;
	_mouse_drawn = true;

	int xdraw = _ms_cur.x - _ms_hotspot_x;
	int ydraw = _ms_cur.y - _ms_hotspot_y;
	int w = _ms_cur.w;
	int h = _ms_cur.h;
	int real_w;
	int real_h;
	int real_h_2;

	byte *dst;
	byte *dst2;
	const byte *buf = _ms_buf;
	byte *bak = _ms_backup;

	assert(w <= 40 && h <= 40);

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

	dst = _gfx_buf + (ydraw * 320) + xdraw;
	dst2 = dst;

	if ((real_h == 0) || (real_w == 0)) {
		_mouse_drawn = false;
		return;
	}

	_ms_old.x = xdraw;
	_ms_old.y = ydraw;
	_ms_old.w = real_w;
	_ms_old.h = real_h;

	real_h_2 = real_h;
	while (real_h_2 > 0) {
		memcpy(bak, dst, real_w);
		bak += 40;
		dst += 320;
		real_h_2--;
	}
	while (real_h > 0) {
		int width = real_w;
		while (width > 0) {
			byte color = *buf;
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

void OSystem_WINCE3::undraw_mouse() {
	if (!_mouse_drawn)
		return;
	_mouse_drawn = false;

	int old_h = _ms_old.h;

	byte *dst = _gfx_buf + (_ms_old.y * 320) + _ms_old.x;
	byte *bak = _ms_backup;

	while (old_h > 0) {
		memcpy(dst, bak, _ms_old.w);
		bak += 40;
		dst += 320;
		old_h--;
	}
}

	
void OSystem_WINCE3::set_mouse_pos(int x, int y) {
	if (x != _ms_cur.x || y != _ms_cur.y) {
		_ms_cur.x = x;
		_ms_cur.y = y;
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
	Sleep(msecs);
}
	
void *OSystem_WINCE3::create_thread(ThreadProc *proc, void *param) {
	// needed for emulated MIDI support (Sam'n'Max)
	return CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)proc, param, 0, NULL);
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

		_event.event_code = -1;

		if (!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			return false;

		if (msg.message==WM_QUIT) {
			terminated=true;
			do_quit();
			return false;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);

		*event = _event;

		return true;
	}
	
	return false;
}
	
bool OSystem_WINCE3::set_sound_proc(void *param, SoundProc *proc, byte format) {
	SDL_AudioSpec desired;

	/* only one format supported at the moment */

	desired.freq = SAMPLES_PER_SEC;
	desired.format = AUDIO_S16SYS;
	desired.channels = 1;
	desired.samples = 128;
	desired.callback = proc;
	desired.userdata = param;
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
		return SAMPLES_PER_SEC;
	}

	return 0;
}
		
void OSystem_WINCE3::quit() {
	unload_gfx_mode();		
	exit(1);
}

/* CDRom Audio */
void OSystem_WINCE3::stop_cdrom() {;}
void OSystem_WINCE3::play_cdrom(int track, int num_loops, int start_frame, int end_frame) {
	/* Reset sync count */
	g_scumm->_vars[g_scumm->VAR_MI1_TIMER] = 0;
}

bool OSystem_WINCE3::poll_cdrom() {return 0;}
void OSystem_WINCE3::update_cdrom() {;}

void ScummDebugger::attach(Scumm *s) {;}
