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
#include <aygshell.h>

#define IDM_SMARTFON_LIST_BASE 99100

#define IDM_SMARTFON_MAP_BASE 99200

#define SMARTFON_VERSION "Smartphone build 03.01.27/1"

#define SCAN_LOCATION "\\Storage Card"

#define KEYS_VERSION 2
#define TOTAL_KEYS 10
#define TOTAL_ZONES 3

typedef struct zoneDesc {
	int x;
	int y;
	int width;
	int height;
} zoneDesc;

typedef BOOL (*tSHCreateMenuBar)(void*);
typedef void (*tSmartfonAction)(OSystem_WINCE3 *wm, BOOL repeat);

extern tSHCreateMenuBar dynamicSHCreateMenuBar;
extern bool need_GAPI;
extern void startFindGame(BOOL display, TCHAR *path);
extern bool loadGameSettings(BOOL display);
extern void sortFoundGames(void);
extern int getTotalGames(void);
extern void getSelectedGame(int result, char *id, TCHAR *directory);
extern TCHAR* getGameName(int);
extern Engine *engine;
extern bool is_simon;
extern bool sound_activated;
extern void Cls();

int game_chosen;

HMENU gamesMenu;
HMENU optionsMenu;
HMENU gamesListMenu;
HMENU keysMappingMenu;
HWND _hWnd;
HWND _hWndMenu;

extern Config *g_config;

int mouseX;
int mouseY;
int lastKeyPressed = 0;
int mapping = -1;
int mappingDone = -1;

int mouseXZone[TOTAL_ZONES];
int mouseYZone[TOTAL_ZONES];
int currentZone = 0;

zoneDesc ZONES[TOTAL_ZONES] = {
	{ 0, 0, 320, 145 },
	{ 0, 145, 150, 55 },
	{ 150, 145, 170, 55 }
};

const char* SMARTFON_KEYS_MAPPING[TOTAL_KEYS] = {
	"Up",
	"Down",
	"Left",
	"Right",
	"Left click",
	"Right click",
	"Options",
	"Skip",
	"Zone",
	"Boss"
};


void SmartfonUp(OSystem_WINCE3 *wm, BOOL repeat);
void SmartfonDown(OSystem_WINCE3 *wm, BOOL repeat);
void SmartfonLeft(OSystem_WINCE3 *wm, BOOL repeat);
void SmartfonRight(OSystem_WINCE3 *wm, BOOL repeat);
void SmartfonLeftClick(OSystem_WINCE3 *wm, BOOL repeat);
void SmartfonLeftUp(OSystem_WINCE3 *wm, BOOL repeat);
void SmartfonRightClick(OSystem_WINCE3 *wm, BOOL repeat);
void SmartfonRightUp(OSystem_WINCE3 *wm, BOOL repeat);
void SmartfonSave(OSystem_WINCE3 *wm, BOOL repeat);
void SmartfonSkip(OSystem_WINCE3 *wm, BOOL repeat);
void SmartfonBoss(OSystem_WINCE3 *wm, BOOL repeat);
void SmartfonZone(OSystem_WINCE3 *wm, BOOL repeat);


const tSmartfonAction SMARTFON_ACTIONS[TOTAL_KEYS] = {
	SmartfonUp,
	SmartfonDown,
	SmartfonLeft,
	SmartfonRight,
	SmartfonLeftClick,
	SmartfonRightClick,
	SmartfonSave,
	SmartfonSkip,
	SmartfonZone,
	SmartfonBoss,
};

const tSmartfonAction SMARTFON_DEACTIONS[TOTAL_KEYS] = {
	NULL,
	NULL,
	NULL,
	NULL,
	SmartfonLeftUp,
	SmartfonRightUp,
	NULL,
	NULL,
	NULL,
	NULL
};


const char* KEYS_NAME[] = {
	"1", "2", "3","4", "5", "6", "7", "8", "9", "*", "0", "#",
	"Home", "Back", "Up", "Down", "Left", "Right", "Action", "Hang up", "Call",
	"Soft 1", "Soft 2", "Power", "Volume Up" ,"Volume Down", "Record", "None", 0
};

const int KEYS_MAPPING[] = {
	'1', '2', '3', '4', '5', '6', '7', '8', '9', VK_F8, '0', VK_F9,
	VK_LWIN, VK_ESCAPE, VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT, VK_RETURN, VK_F4, VK_F3,
	VK_F1, VK_F2, VK_F18, VK_F6, VK_F7, VK_F10, 0xff, 0
};

const int DEFAULT_MAPPING[TOTAL_KEYS] = {
	'4',
	'6',
	'8',
	'2',
	VK_F1,
	VK_F2,
	VK_LWIN,
	VK_ESCAPE,
	0xff,
	0xff
};

int current_mapping[TOTAL_KEYS];

// Actions

void SmartfonUp(OSystem_WINCE3 *wm, BOOL repeat) {
	if (repeat)
		mouseY -= 10;
	else
		mouseY -= 2;
		
	if (mouseY < 0)
		mouseY = 0;

	wm->_event.event_code = OSystem::EVENT_MOUSEMOVE;
	wm->_event.mouse.x = mouseX;
	wm->_event.mouse.y = mouseY;
}

void SmartfonDown(OSystem_WINCE3 *wm, BOOL repeat) {
	if (repeat)
		mouseY += 10;
	else
		mouseY += 2;
		
	if (mouseY > 200)
		mouseY = 200;

	wm->_event.event_code = OSystem::EVENT_MOUSEMOVE;
	wm->_event.mouse.x = mouseX;
	wm->_event.mouse.y = mouseY;
}

void SmartfonLeft(OSystem_WINCE3 *wm, BOOL repeat) {
	if (repeat)
		mouseX -= 10;
	else
		mouseX -= 2;
		
	if (mouseX < 0)
		mouseX = 0;

	wm->_event.event_code = OSystem::EVENT_MOUSEMOVE;
	wm->_event.mouse.x = mouseX;
	wm->_event.mouse.y = mouseY;
}

void SmartfonRight(OSystem_WINCE3 *wm, BOOL repeat) {
	if (repeat)
		mouseX += 10;
	else
		mouseX += 2;
		
	if (mouseX > 320)
		mouseX = 320;

	wm->_event.event_code = OSystem::EVENT_MOUSEMOVE;
	wm->_event.mouse.x = mouseX;
	wm->_event.mouse.y = mouseY;
}

void SmartfonLeftClick(OSystem_WINCE3 *wm, BOOL repeat) {

	wm->_event.event_code = OSystem::EVENT_LBUTTONDOWN;
	wm->_event.mouse.x = mouseX;
	wm->_event.mouse.y = mouseY;
}

void SmartfonLeftUp(OSystem_WINCE3 *wm, BOOL repeat) {

	wm->_event.event_code = OSystem::EVENT_LBUTTONUP;
	wm->_event.mouse.x = mouseX;
	wm->_event.mouse.y = mouseY;
}


void SmartfonRightClick(OSystem_WINCE3 *wm, BOOL repeat) {

	wm->_event.event_code = OSystem::EVENT_RBUTTONDOWN;
	wm->_event.mouse.x = mouseX;
	wm->_event.mouse.y = mouseY;
}

void SmartfonRightUp(OSystem_WINCE3 *wm, BOOL repeat) {

	wm->_event.event_code = OSystem::EVENT_RBUTTONUP;
	wm->_event.mouse.x = mouseX;
	wm->_event.mouse.y = mouseY;
}


void SmartfonSave(OSystem_WINCE3 *wm, BOOL repeat) {

		wm->_event.event_code = OSystem::EVENT_KEYDOWN;
		if (g_scumm->_features & GF_OLD256)
			wm->_event.kbd.ascii = 319;
		else
			wm->_event.kbd.ascii = g_scumm->_vars[g_scumm->VAR_SAVELOADDIALOG_KEY];
}

void SmartfonSkip(OSystem_WINCE3 *wm, BOOL repeat) {

		if (is_simon) {
			((SimonState*)engine)->_exit_cutscene = true;
			return;
		}
		wm->_event.event_code = OSystem::EVENT_KEYDOWN;
		if (g_scumm->vm.cutScenePtr[g_scumm->vm.cutSceneStackPointer] || g_scumm->_insaneState)
			wm->_event.kbd.ascii = g_scumm->_vars[g_scumm->VAR_CUTSCENEEXIT_KEY];
		else
			wm->_event.kbd.ascii = g_scumm->_vars[g_scumm->VAR_TALKSTOP_KEY];						
}

void SmartfonBoss(OSystem_WINCE3 *wm, BOOL repeat) {
	SHELLEXECUTEINFO se;    

	sound_activated = false;
	Cls();
	g_scumm->_saveLoadSlot = 0;
	g_scumm->_saveLoadCompatible = false;
	g_scumm->_saveLoadFlag = 1;
	strcpy(g_scumm->_saveLoadName, "BOSS");
	g_scumm->saveState(g_scumm->_saveLoadSlot, g_scumm->_saveLoadCompatible);
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

void updateZone(int x, int y) {
	int i;

	for (i=0; i<TOTAL_ZONES; i++) 
		if (x >= ZONES[i].x && y >= ZONES[i].y &&
			x <= ZONES[i].x + ZONES[i].width && y <= ZONES[i].y + ZONES[i].height
			) {
				mouseXZone[i] = x;
				mouseYZone[i] = y;
				break;
		}
}

void initZone() {
	int i;

	for (i=0; i<TOTAL_ZONES; i++) {
		mouseXZone[i] = ZONES[i].x + (ZONES[i].width / 2);
		mouseYZone[i] = ZONES[i].y + (ZONES[i].height / 2);
	}
	mouseX = mouseXZone[0];
	mouseY = mouseYZone[0];
}

void SmartfonZone(OSystem_WINCE3 *wm, BOOL repeat) {
	updateZone(mouseX, mouseY);
	currentZone++;
	if (currentZone >= TOTAL_ZONES)
		currentZone = 0;
	mouseX = mouseXZone[currentZone];
	mouseY = mouseYZone[currentZone];
	wm->_event.event_code = OSystem::EVENT_MOUSEMOVE;
	wm->_event.mouse.x = mouseX;
	wm->_event.mouse.y = mouseY;
}

//

char *getKeyName(int key) {
	int i;

	for (i=0; KEYS_MAPPING[i]; i++) 
		if (KEYS_MAPPING[i] == key)
			return (char*)KEYS_NAME[i];

	return NULL;
}

void updateMapping(int mapping, int key) {
	int i;

	for (i=0; i<TOTAL_KEYS; i++) 
		if (current_mapping[i] == key)
			current_mapping[i] = 0xff;

	current_mapping[mapping] = key;
}

void disableHotKeys() {
	dynamicGXOpenInput();
}

void enableHotKeys() {
	dynamicGXCloseInput();
}

void eraseKeysMappingMenu() {
	int i;

	for (i=0; i<TOTAL_KEYS; i++)
		DeleteMenu(keysMappingMenu, 0, MF_BYPOSITION);
}

void buildKeysMappingMenu() {
	int i;

	for (i=0; i<TOTAL_KEYS; i++) {
		char mapping[100];
		TCHAR mappingUnicode[100];

		sprintf(mapping, "Map %s (%s)", SMARTFON_KEYS_MAPPING[i], getKeyName(current_mapping[i]));
		MultiByteToWideChar(CP_ACP, 0, mapping, strlen(mapping) + 1, mappingUnicode, sizeof(mappingUnicode));

		InsertMenu(keysMappingMenu, 0, MF_BYPOSITION, IDM_SMARTFON_MAP_BASE + i, mappingUnicode);
	}
}

BOOL saveKeyMapping() {
	int i;
	char tempo[1024];
	
	tempo[0] = '\0';
	g_config->setInt("KeysVersion", KEYS_VERSION, "smartfon");
	for (i=0; i<TOTAL_KEYS; i++) {
		 char x[4];
		 sprintf(x, "%.4x ", current_mapping[i]);
		 strcat(tempo, x);
	}
	g_config->set("ActionKeys", tempo, "smartfon");
	g_config->flush();

	return TRUE;
}

BOOL loadKeyMapping() {
	int version;
	const char *current;

	version = g_config->getInt("KeysVersion", 0, "smartfon");
	current = g_config->get("ActionKeys", "smartfon");
	if (current && version == KEYS_VERSION) {
		int i;
		for (i=0; i<TOTAL_KEYS; i++) {
			char x[6];
			int j;

			memset(x, 0, sizeof(x));
			memcpy(x, current + 5 * i, 4);
			sscanf(x, "%x", &j);
			current_mapping[i] = j;
		}
		return TRUE;
	}
	else {
			memcpy(current_mapping, DEFAULT_MAPPING, TOTAL_KEYS * sizeof(int));
			return FALSE;
	}
}

void addGames() {
	int i;

	sortFoundGames();

	for (i=0; i<getTotalGames(); i++) 
		InsertMenu(gamesListMenu, 0, MF_BYPOSITION, IDM_SMARTFON_LIST_BASE + i, getGameName(i));

	DrawMenuBar(_hWnd);
}


int SmartphoneInitialMenu(HINSTANCE hInstance, HWND hWnd, char *game_name, TCHAR *directory) {


	SHMENUBARINFO menuBarInfo;
	TBBUTTONINFO buttonInfo;
	MENUITEMINFO menuItemInfo;

	_hWnd = hWnd;

	// Create "games" menu, and get a real handle to access it

	memset(&menuBarInfo, 0, sizeof(menuBarInfo));
	menuBarInfo.cbSize = sizeof(menuBarInfo);
	menuBarInfo.hwndParent = hWnd;
	menuBarInfo.nToolBarId = IDR_SMARTFON_MENUBAR_GAMES;
	menuBarInfo.hInstRes = hInstance;
	dynamicSHCreateMenuBar(&menuBarInfo);

	_hWndMenu = menuBarInfo.hwndMB;

	buttonInfo.cbSize = sizeof(buttonInfo);
	buttonInfo.dwMask = TBIF_LPARAM;
	SendMessage(menuBarInfo.hwndMB, TB_GETBUTTONINFO, ID_MENU_GAMES, (LPARAM)&buttonInfo);
	gamesMenu = (HMENU)buttonInfo.lParam;

	// Get a handle to the popup list of games

	memset(&menuItemInfo, 0, sizeof(menuItemInfo));
	menuItemInfo.cbSize = sizeof(menuItemInfo);
	menuItemInfo.fMask = MIIM_SUBMENU;
	GetMenuItemInfo(gamesMenu, 0, TRUE, &menuItemInfo);
	gamesListMenu = menuItemInfo.hSubMenu;

	// Same for options

	buttonInfo.cbSize = sizeof(buttonInfo);
	buttonInfo.dwMask = TBIF_LPARAM;
	SendMessage(menuBarInfo.hwndMB, TB_GETBUTTONINFO, ID_MENU_OPTIONS, (LPARAM)&buttonInfo);
	optionsMenu = (HMENU)buttonInfo.lParam;

	memset(&menuItemInfo, 0, sizeof(menuItemInfo));
	menuItemInfo.cbSize = sizeof(menuItemInfo);
	menuItemInfo.fMask = MIIM_SUBMENU;
	GetMenuItemInfo(optionsMenu, 0, TRUE, &menuItemInfo);
	keysMappingMenu = menuItemInfo.hSubMenu;

	// Build key mapping menu

	loadKeyMapping();
	buildKeysMappingMenu();
	initZone();

	DrawMenuBar(hWnd);

	// See if some games are already installed
	if (loadGameSettings(FALSE))
		addGames();

	game_chosen = -1;

	for (;game_chosen < 0;) {
		MSG msg;

		if (!GetMessage(&msg, NULL, 0, 0))
			return 1;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	getSelectedGame(game_chosen, game_name, directory);

	return 0;
}

void doSmartphoneScan() {
	int i;
	for (i=0; i<getTotalGames(); i++)
		DeleteMenu(gamesListMenu, 0, MF_BYPOSITION);
	startFindGame(FALSE, TEXT(SCAN_LOCATION));
	addGames();
}

void doPaint() {
	RECT rcString, rcClient;
	POINT ptTrig[4]; 
	INT i, cy;
	HDC hDC;
	HDC copyhDC;
	PAINTSTRUCT ps;
	HBITMAP bitmap;

	GetClientRect (_hWnd, &rcClient);
	hDC = BeginPaint (_hWnd, &ps); 

	FillRect(hDC, &rcClient, (HBRUSH)GetStockObject(BLACK_BRUSH));

	SetTextColor (hDC, 0x000077FF);
	SetBkColor (hDC, 0x00000000);
	rcClient.left = 0;
	rcClient.top = 40;
	DrawText(hDC, TEXT("For Smartphone 2002"), -1, &rcClient, DT_CENTER | DT_SINGLELINE);
	SetTextColor (hDC, 0x0000FF77);
	rcClient.left = 0;
	rcClient.top = 70;
	DrawText(hDC, TEXT("http://www.scummvm.org"), -1, &rcClient, DT_CENTER | DT_SINGLELINE);
	rcClient.left = 0;
	rcClient.top = 90;
	DrawText(hDC, TEXT(SCUMMVM_VERSION), -1, &rcClient, DT_CENTER | DT_SINGLELINE);
	rcClient.left = 0;
	rcClient.top = 110;
	DrawText(hDC, TEXT("http://arisme.free.fr"), -1, &rcClient, DT_CENTER | DT_SINGLELINE);
	rcClient.left = 0;
	rcClient.top = 130;
	DrawText(hDC, TEXT(SMARTFON_VERSION), -1, &rcClient, DT_CENTER | DT_SINGLELINE);

	if (mapping != -1) {
		char mappingInfo[100];
		TCHAR mappingInfoUnicode[100];

		if (mappingDone == -1) {
			sprintf(mappingInfo, "Press key to map to %s", SMARTFON_KEYS_MAPPING[mapping]);
		}
		else {
			sprintf(mappingInfo, "Key %s mapped to %s", getKeyName(mappingDone), SMARTFON_KEYS_MAPPING[mapping]);
			updateMapping(mapping, mappingDone);
			eraseKeysMappingMenu();
			buildKeysMappingMenu();
			mapping = -1;
			mappingDone = -1;
		}

		MultiByteToWideChar(CP_ACP, 0, mappingInfo, strlen(mappingInfo) + 1, mappingInfoUnicode, sizeof(mappingInfoUnicode));
		rcClient.left = 0;
		rcClient.top = 150;
		SetTextColor (hDC, 0x000077FF);
		DrawText(hDC, mappingInfoUnicode, -1, &rcClient, DT_CENTER | DT_SINGLELINE);
	}
		
	bitmap = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_LOGO));
	copyhDC = CreateCompatibleDC(hDC);
	SelectObject(copyhDC, bitmap);
	BitBlt(hDC, 15, 0, 150, 35, copyhDC, 0, 0, SRCCOPY);
	DeleteDC(copyhDC);
	DeleteObject(bitmap);


	EndPaint (_hWnd, &ps);
}

BOOL SmartphoneWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, OSystem_WINCE3 *wm) {

	switch(message) {
		case WM_ERASEBKGND:

			if (need_GAPI)
				return FALSE;

			doPaint();
			break;

		case WM_PAINT:

			if (need_GAPI)
				return FALSE;

			doPaint();

			break;

		case WM_COMMAND:
			if (need_GAPI)
				return FALSE;

			if (wParam >= IDM_SMARTFON_MAP_BASE) {
				mapping = wParam - IDM_SMARTFON_MAP_BASE;
				disableHotKeys();
				return TRUE;
			}

			if (wParam >= IDM_SMARTFON_LIST_BASE) {
				game_chosen = wParam - IDM_SMARTFON_LIST_BASE;
				saveKeyMapping();
				return TRUE;
			}

			switch(wParam) {
				case IDM_SMARTFON_QUIT:
					exit(0);
					break;
				case IDM_SMARTFON_SCAN:
					doSmartphoneScan();
					break;
				default:
					return FALSE;
			}
			break;


		case WM_KEYDOWN:

			int i;

			if (!need_GAPI && mapping == -1)
				break;

			if (mapping != -1) {
				mappingDone = wParam;
				InvalidateRect(_hWnd, NULL, FALSE);
				enableHotKeys();
				return TRUE;
			}

			for (i=0; i<TOTAL_KEYS; i++) 
				if (current_mapping[i] == wParam) {
					SMARTFON_ACTIONS[i](wm, lastKeyPressed == wParam);
					lastKeyPressed = wParam;
					return TRUE;
				}
			lastKeyPressed = wParam;

			wm->_event.event_code = OSystem::EVENT_KEYDOWN;
			wm->_event.kbd.ascii = wParam;
			wm->_event.kbd.keycode = wParam;

			break;

		case WM_KEYUP:
			if (!need_GAPI)
				break;

			for (i=0; i<TOTAL_KEYS; i++) 
				if (current_mapping[i] == wParam) {
					if (SMARTFON_DEACTIONS[i]) {
						SMARTFON_DEACTIONS[i](wm, FALSE);
						break;
					}
				}
			lastKeyPressed = 0;
			break;

		default:
			return FALSE;
	}

	return TRUE;
}