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

#include <aygshell.h>
#include "resource.h"

#include "scumm.h"
#include "screen.h"

#include "sound.h"

#include "gui.h"

#define USE_F_KEYS
#ifdef USE_F_KEYS
void setup_extra_windows(HWND hwndTop);
void adjust_extra_windows();
HWND hwndFKeys;
#endif

#define SAMPLES_PER_SEC 22050
#define BUFFER_SIZE (8192)
#define BITS_PER_SAMPLE 16

// Practically identical to the one in windows.cpp
class WndMan
{
	HMODULE hInst;
	HWND hWnd;

	bool terminated;	

public:
	byte *_vgabuf;
	Scumm *_scumm;

	HANDLE _event;
	DWORD _threadId;
	HWAVEOUT _handle;
	WAVEHDR _hdr[2];

public:
	void init();

	bool handleMessage();
	void run();
	void setPalette(byte *ctab, int first, int num);
	void writeToScreen();

	void prepare_header(WAVEHDR *wh, int i);
	void sound_init();
	static DWORD _stdcall sound_thread(WndMan *wm);
};

// Similar to Error in windows.cpp but has to take Unicode in account
void Error(LPCTSTR msg)
{
	OutputDebugString(msg);
	MessageBox(0, msg, TEXT("Error"), MB_ICONSTOP);
	exit(1);
}

//{{
Scumm scumm;
ScummDebugger debugger;
Gui gui;

SoundEngine sound;

WndMan wm[1];
byte veryFastMode;
//}}

// WndProc is significantly port-specific
int mapKey(int key) {
	if (key>=VK_F1 && key<=VK_F9) {
		return key - VK_F1 + 315;
	}
	return key;
}


static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static		 SHACTIVATEINFO sai;

	WndMan *wm = (WndMan*)GetWindowLong(hWnd, GWL_USERDATA);	

	switch (message) 
	{
	case WM_CREATE:
		memset(&sai, 0, sizeof(sai));
		SHSipPreference(hWnd, SIP_INPUTDIALOG);
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
		}
		SHSipPreference(hWnd, SIP_UP); /* Hack! */
#ifdef USE_F_KEYS
		adjust_extra_windows();
#endif
		/* It does not happen often but I don't want to see tooltip traces */
		wm->writeToScreen();
		return 0;

	case WM_ACTIVATE:
		GraphicsResume();
		SHHandleWMActivate(hWnd, wParam, lParam, &sai, SHA_INPUTDIALOG);
#ifdef USE_F_KEYS
		adjust_extra_windows();
#endif
		return 0;

	case WM_HIBERNATE:
		GraphicsSuspend();
		return 0;

	case WM_SETTINGCHANGE:
		SHHandleWMSettingChange(hWnd, wParam, lParam, &sai);
#ifdef USE_F_KEYS
		adjust_extra_windows();
#endif
		return 0;

	case WM_COMMAND:
		switch(wParam)
		{
		case IDC_ABOUT:
			break;
		case IDC_EXIT:
			DestroyWindow(hWnd);
			break;
// Landscape mode is broken. This will be uncommented when it works
/*
		case IDC_LANDSCAPE:
			SetScreenMode(1);
			SetCapture(hWnd); // to prevent input panel from getting taps
			InvalidateRect(HWND_DESKTOP, NULL, TRUE);
			break;
*/
		}
		return 0;


	case WM_KEYDOWN:
		if(wParam == VK_BACKSLASH)
			wParam = VK_ESCAPE;

		wm->_scumm->_keyPressed = mapKey(wParam);
		break;

	case WM_MOUSEMOVE:
		{
			int x = ((int16*)&lParam)[0];
			int y = ((int16*)&lParam)[1];
			Translate(&x, &y);
			wm->_scumm->mouse.x = x;
			wm->_scumm->mouse.y = y;
		}
		break;
	case WM_LBUTTONDOWN:
		{
			int x = ((int16*)&lParam)[0];
			int y = ((int16*)&lParam)[1];
			Translate(&x, &y);
			wm->_scumm->mouse.x = x;
			wm->_scumm->mouse.y = y;
			wm->_scumm->_leftBtnPressed |= 1;
			if(y > 200)
			{
				if(x<160)
					wm->_scumm->_keyPressed = VK_ESCAPE;
				else
				{
					SetScreenMode(0); // restore normal tap logic
					ReleaseCapture();
					InvalidateRect(HWND_DESKTOP, NULL, TRUE);
				}
			}
		}
		break;
	case WM_RBUTTONDOWN:
		wm->_scumm->_rightBtnPressed |= 1;
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

// writeToScreen, setPalette and init are very port-specific
void WndMan::writeToScreen()
{
	Blt(_vgabuf);
}

void WndMan::setPalette(byte *ctab, int first, int num) {
	int i;

	for (i=0; i<256; i++)
		SetPalEntry(i, ctab[i*3+0]<<2, ctab[i*3+1]<<2, ctab[i*3+2]<<2);
}


void WndMan::init()
{
	/* Retrieve the handle of this module */
	hInst = GetModuleHandle(NULL);

	/* Register the window class */
	WNDCLASS wcex;
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInst;
	wcex.hIcon			= 0;
	wcex.hCursor		= NULL;
	wcex.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName	= 0;	
	wcex.lpszClassName	= TEXT("ScummVM");
	if (!RegisterClass(&wcex))
		Error(TEXT("Cannot register window class!"));

	hWnd = CreateWindow(TEXT("ScummVM"), TEXT("ScummVM"), WS_VISIBLE,
      0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), NULL, NULL, hInst, NULL);
	SetWindowLong(hWnd, GWL_USERDATA, (long)this);

	ShowWindow(hWnd, SW_SHOW);

	SHMENUBARINFO smbi;
	smbi.cbSize = sizeof(smbi); 
	smbi.hwndParent = hWnd; 
	smbi.dwFlags = 0; 
	smbi.nToolBarId = IDM_MENU; 
	smbi.hInstRes = GetModuleHandle(NULL); 
	smbi.nBmpId = 0; 
	smbi.cBmpImages = 0; 
	smbi.hwndMB = NULL;
	BOOL res = SHCreateMenuBar(&smbi);

	GraphicsOn(hWnd);

	SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	SetForegroundWindow(hWnd);
	SHFullScreen(hWnd, SHFS_SHOWSIPBUTTON);
	SHFullScreen(hWnd, SHFS_HIDETASKBAR);

#ifdef USE_F_KEYS
	setup_extra_windows(hWnd);
#endif
}


//{{
bool WndMan::handleMessage() {
	MSG msg;

	if (!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		return false;

	if (msg.message==WM_QUIT) {
		terminated=true;
		exit(1);
		return true;
	}

	TranslateMessage(&msg);
	DispatchMessage(&msg);

	return true;
}
//}}

// This function is very similar to the one in windows.cpp except for
// one line removed.
void blitToScreen(Scumm *s, byte *src,int x, int y, int w, int h) {
	byte *dst;
	int i;

	dst = (byte*)wm->_vgabuf + y*320 + x;

	do {
		memcpy(dst, src, w);
		dst += 320;
		src += 320;
	} while (--h);

}

//{{
int clock;

void updateScreen(Scumm *s) {
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
		Sleep(delay);
	} 
}

void initGraphics(Scumm *s, bool fullScreen) {
	if(fullScreen)
		warning("Use SDL for fullscreen support");
}

void drawMouse(Scumm *s, int, int, int, byte*, bool) {
}

void drawMouse(Scumm *s, int x, int y, int w, int h, byte *buf, bool visible) {
}

void fill_buffer(int16 *buf, int len) {
	memset(buf, 0, len*2);
	scumm.mixWaves(buf, len);
}

void WndMan::prepare_header(WAVEHDR *wh, int i) {
	memset(wh, 0, sizeof(WAVEHDR));
	wh->lpData = (char*)malloc(BUFFER_SIZE);
	wh->dwBufferLength = BUFFER_SIZE;

	waveOutPrepareHeader(_handle, wh, sizeof(WAVEHDR));

	fill_buffer((int16*)wh->lpData, wh->dwBufferLength>>1);
	waveOutWrite(_handle, wh, sizeof(WAVEHDR));
}

void WndMan::sound_init() {
	WAVEFORMATEX wfx;

	memset(&wfx, 0, sizeof(wfx));
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = 1;
	wfx.nSamplesPerSec = SAMPLES_PER_SEC;
	wfx.nAvgBytesPerSec = SAMPLES_PER_SEC * BITS_PER_SAMPLE / 8;
	wfx.wBitsPerSample = BITS_PER_SAMPLE;
	wfx.nBlockAlign = BITS_PER_SAMPLE * 1 / 8;

	CreateThread(NULL, 0, (unsigned long (__stdcall *)(void *))&sound_thread, this, 0, &_threadId);
	SetThreadPriority((void*)_threadId, THREAD_PRIORITY_HIGHEST);

	_event = CreateEvent(NULL, false, false, NULL);

	memset(_hdr,0,sizeof(_hdr));

	waveOutOpen(&_handle, WAVE_MAPPER, &wfx, (long)_event, (long)this, CALLBACK_EVENT );

	prepare_header(&_hdr[0], 0);
	prepare_header(&_hdr[1], 1);
}

DWORD _stdcall WndMan::sound_thread(WndMan *wm) {
	int i;
	bool signaled;
	int time = GetTickCount(), cur;

	while (1) {
		cur = GetTickCount();
		while (time < cur) {
			sound.on_timer();
			time += 10;
		}

		signaled = WaitForSingleObject(wm->_event, time - cur) == WAIT_OBJECT_0;

		if (signaled) {
			for(i=0; i<2; i++) {
				WAVEHDR *hdr = &wm->_hdr[i];
				if (hdr->dwFlags & WHDR_DONE) {
					fill_buffer((int16*)hdr->lpData, hdr->dwBufferLength>>1);
					waveOutWrite(wm->_handle, hdr, sizeof(WAVEHDR));
				}
			}
		}
	}
	return 1; // not in windows.cpp - a bug!
}
//}}

BOOL CALLBACK SelectDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			RECT rc; GetWindowRect(hwndDlg, &rc);
			MoveWindow(hwndDlg,
				(GetSystemMetrics(SM_CXSCREEN)-rc.right+rc.left)/2,
				(GetSystemMetrics(SM_CYSCREEN)-rc.bottom+rc.top)/2,
				rc.right-rc.left, rc.bottom-rc.top, TRUE);
			BringWindowToTop(hwndDlg);
		}
		return TRUE;
	case WM_COMMAND:
		EndDialog(hwndDlg, wParam);
		return TRUE;
	default:
		return FALSE;
	}
}

char* GameSelector()
{
	switch(DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_GAMESELECT), HWND_DESKTOP, SelectDlgProc))
	{
	case IDC_MONKEY:   return "monkey";
	case IDC_MONKEY2:  return "monkey2";
	case IDC_ATLANTIS: return "atlantis";
	case IDC_PLAYFATE: return "playfate";
	case IDC_TENTACLE: return "tentacle";
	case IDC_DOTTDEMO: return "dottdemo";
	case IDC_SAMNMAX:  return "samnmax";
	case IDC_SNMDEMO:  return "snmdemo";
	default:           return NULL;
	}
}

// Directly corresponds to main in windows.cpp
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
	int delta;
	int tmp;

	int argc = 2;
	char* argv[2];
	argv[0] = NULL;
	argv[1] = GameSelector();

	if(argv[1] == NULL)
		return 0;

	wm->init();
	wm->sound_init();
	wm->_vgabuf = (byte*)calloc(320,200);
	wm->_scumm = &scumm;
	

//	sound.initialize(&scumm);
//	scumm._soundDriver = &sound;
	scumm._soundDriver = NULL; // sound is not working yet and has been disabled

	scumm._gui = &gui;

	scumm.scummMain(argc, argv);
	gui.init(&scumm);

	delta = 0;
	tmp = 0;
	do
	{
		updateScreen(&scumm);

		waitForTimer(&scumm, tmp*10);

		if(gui._active)
		{
			gui.loop();
			tmp = 5;
		}
		else
		{
			tmp = delta = scumm.scummLoop(delta);

			tmp += tmp>>1;
			
			if(scumm._fastMode)
				tmp=1;
		}
	}
	while(1);

	return 0;
}

#ifdef USE_F_KEYS
/* Window for F-key input. Hack but some people asked for it. */
LRESULT CALLBACK FWindowProc(HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	switch (nMsg)
	{
	case WM_CREATE:
		return 0;
	case WM_PAINT:
		{
			HDC          hDC;
			PAINTSTRUCT  ps;
			RECT		 rc;
			hDC = BeginPaint(hwnd, &ps);
			GetClientRect(hwnd, &rc);
			HGDIOBJ oldPen = SelectObject(hDC, (HGDIOBJ)GetStockObject(BLACK_PEN));
			HGDIOBJ oldBr = SelectObject(hDC, (HGDIOBJ)GetStockObject(WHITE_BRUSH));
			HGDIOBJ oldFont = SelectObject(hDC, (HGDIOBJ)GetStockObject(SYSTEM_FONT));
			int rcWidth = rc.right-rc.left;
			RECT rcItem;
			rcItem.top = rc.top;
			rcItem.bottom = rc.bottom;
			POINT pts[2];
			pts[0].y = rc.top;
			pts[1].y = rc.bottom;
			TCHAR text[4];
			for(int i=0; i<10; i++)
			{
				wsprintf(text, TEXT("F%d"), i+1);
				rcItem.left = rc.left+rcWidth*i/10;
				rcItem.right = rc.left+rcWidth*(i+1)/10;
				pts[0].x = pts[1].x = rcItem.right;
				Polyline(hDC, pts, 2);
				DrawText(hDC, text, -1, &rcItem, DT_CENTER|DT_VCENTER);
			}
			SelectObject(hDC, oldPen);
			SelectObject(hDC, oldBr);
			SelectObject(hDC, oldFont);
			EndPaint(hwnd, &ps);
		}
		return 0;
	case WM_LBUTTONDOWN:
		{
			int x = LOWORD(lParam);
			RECT rc; GetWindowRect(hwnd, &rc);
			int fnum = x*10/(rc.right-rc.left);
			PostMessage(GetParent(hwnd), WM_KEYDOWN, VK_F1+fnum, 0);
		}
		return 0;
	}

	return DefWindowProc(hwnd, nMsg, wParam, lParam);
}

void setup_extra_windows(HWND hwndTop)
{
	LPTSTR fkeyname = TEXT("fkeys");

	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = FWindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = NULL;
	wc.hCursor = NULL;
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = fkeyname;
	RegisterClass(&wc);
	
	hwndFKeys = CreateWindow(fkeyname,
		fkeyname,
		WS_VISIBLE|WS_CHILD,
		0,
		200,
		GetSystemMetrics(SM_CXSCREEN),
		20,
		hwndTop,
		(HMENU)100,
		GetModuleHandle(NULL),
		NULL);
}

void adjust_extra_windows()
{
	SIPINFO si;
	si.cbSize = sizeof(SIPINFO);
	SHSipInfo(SPI_GETSIPINFO, 0, &si, 0);
	if(si.fdwFlags & SIPF_ON)
	{
		int bottom = si.rcVisibleDesktop.bottom;
		SetWindowPos(hwndFKeys, 0, 0, 200, GetSystemMetrics(SM_CXSCREEN), bottom-200,
			SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER);
	}
}
#endif
