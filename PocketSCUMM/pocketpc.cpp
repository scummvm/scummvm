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

#include "stdafx.h"
#include <assert.h>

#include "scumm.h"
#include "screen.h"

class WndMan
{
	HMODULE hInst;
	HWND hWnd;

	bool terminated;	

public:
	byte *_vgabuf;
	Scumm *_scumm;

public:
	void init();

	bool handleMessage();
	void run();
	void setPalette(byte *ctab, int first, int num);
	void writeToScreen();
};

void Error(LPCTSTR msg)
{
	MessageBox(0, msg, TEXT("Error"), MB_ICONSTOP);
	exit(1);
}

Scumm scumm;
ScummDebugger debugger;
WndMan wm[1];
byte veryFastMode;

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WndMan *wm = (WndMan*)GetWindowLong(hWnd, GWL_USERDATA);	

	switch (message) 
	{
		case WM_DESTROY:
		case WM_CLOSE:
			GraphicsOff();
			PostQuitMessage(0);
			break;

		case WM_CHAR:
			if(wParam == 'Q' || wParam == 'q')
				wParam = VK_ESCAPE;
			wm->_scumm->_keyPressed = wParam;
			break;

		case WM_KEYDOWN:
			if (wParam>='0' && wParam<='9')
			{
				wm->_scumm->_saveLoadSlot = wParam - '0';
				if (GetAsyncKeyState(VK_SHIFT)<0)
					wm->_scumm->_saveLoadFlag = 1;
				else if (GetAsyncKeyState(VK_CONTROL)<0)
					wm->_scumm->_saveLoadFlag = 2;
				wm->_scumm->_saveLoadCompatible = false;
			}

			if (wParam=='F')
			{
				wm->_scumm->_fastMode ^= 1;
			}

			if (wParam=='G')
			{
				veryFastMode ^= 1;
			}

			if (wParam=='D')
			{
				debugger.attach(wm->_scumm);
			}
			if(wParam == 'R')
				SetScreenMode(GetScreenMode()+1);

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
						SetScreenMode(GetScreenMode()+1);
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

void WndMan::writeToScreen()
{
	Blt(_vgabuf);
}

void WndMan::setPalette(byte *ctab, int first, int num) {
	int i;

#if 1
	for (i=0; i<256; i++)
		SetPalEntry(i, ctab[i*3+0]<<2, ctab[i*3+1]<<2, ctab[i*3+2]<<2);
#else
	for (i=0; i<256; i++)
		SetPalEntry(i, i, i, i);
#endif
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
	wcex.hCursor		= ::LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= 0;	
	wcex.lpszClassName	= TEXT("ScummVM");
	if (!RegisterClass(&wcex))
		Error(TEXT("Cannot register window class!"));

	hWnd = CreateWindow(TEXT("ScummVM"), TEXT("ScummVM"), WS_POPUP,
      0, 0, 240, 320, NULL, NULL, hInst, NULL);
	SetWindowLong(hWnd, GWL_USERDATA, (long)this);
//	ShowCursor(false);

	ShowWindow(hWnd, SW_SHOW);
	GraphicsOn(hWnd);
}


bool WndMan::handleMessage()
{
	MSG msg;

	if (!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		return false;

	if(msg.message==WM_QUIT)
	{
		terminated=true;
		exit(1);
		return true;
	}

	TranslateMessage(&msg);
	DispatchMessage(&msg);

	return true;
}


void blitToScreen(Scumm *s, byte *src,int x, int y, int w, int h)
{
	byte *dst;
	int i;

	dst = (byte*)wm->_vgabuf + y*320 + x;

	do
	{
		memcpy(dst, src, w);
		dst += 320;
		src += 320;
	}
	while (--h);

}

void updateScreen(Scumm *s)
{
	if(s->_palDirtyMax != -1)
	{
		wm->setPalette(s->_currentPalette, 0, 256);	
		s->_palDirtyMax = -1;
	}

	wm->writeToScreen();
}

void waitForTimer(Scumm *s)
{
	if(!veryFastMode)
	{
		Sleep(5);
	} 
	s->_scummTimer+=2;
	wm->handleMessage();
}

void initGraphics(Scumm *s)
{
}

void drawMouse(Scumm *s, int, int, int, byte*, bool)
{
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
	scumm._videoMode = 0x13;

	wm->init();
	wm->_vgabuf = (byte*)calloc(320,200);
	wm->_scumm = &scumm;
	
	char* argv[] = { "", "MONKEY", NULL };
	int argc = 2;

	scumm.scummMain(argc, argv);

	return 0;
}

bool isSfxFinished()
{ 
	return true; 
} 
    
void playSfxSound(void *sound, uint32 size, uint rate)
{ 
} 
