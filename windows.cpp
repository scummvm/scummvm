/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
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
 */

#include "stdafx.h"
#include <assert.h>

#include "scumm.h"
#include "sound.h"
#include "gui.h"

#if !defined(ALLOW_GDI)
#error The GDI driver is not as complete as the SDL driver. You need to define ALLOW_GDI to use this driver.
#endif

#define SRC_WIDTH 320
#define SRC_HEIGHT 200
#define SRC_PITCH (320)

#define DEST_WIDTH 320
#define DEST_HEIGHT 200

#define USE_DIRECTX 0
#define USE_DRAWDIB 0
#define USE_GDI 1

#define SAMPLES_PER_SEC 22050
#define BUFFER_SIZE (8192)
#define BITS_PER_SAMPLE 16

static bool shutdown;
static unsigned int scale;

#if USE_GDI
typedef struct DIB {
	HBITMAP hSect;
	byte *buf;
	RGBQUAD *pal;
	bool new_pal;
} DIB;
#endif

class WndMan {
	HMODULE hInst;
	HWND hWnd;


	bool terminated;	

#if USE_GDI
public:
	DIB dib;
private:
#endif

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

#if USE_GDI
	bool allocateDIB(int w, int h);
#endif
};

void Error(const char *msg) {
	OutputDebugString(msg);
	MessageBoxA(0, msg, "Error", MB_ICONSTOP);
	exit(1);
}

int sel;
Scumm scumm;
ScummDebugger debugger;
Gui gui;
SoundEngine sound;
SOUND_DRIVER_TYPE snd_driv;

WndMan wm[1];
byte veryFastMode;

void modifyslot(int sel, int what);

int mapKey(int key) {
	if (key>=VK_F1 && key<=VK_F9) {
		return key - VK_F1 + 315;
	}
	return key;
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	WndMan *wm = (WndMan*)GetWindowLong(hWnd, GWL_USERDATA);	
	
	switch (message) 
	{
		case WM_DESTROY:
		case WM_CLOSE:
			exit(0);
			break;

		case WM_KEYDOWN:
			if (wParam>='0' && wParam<='9') {
				wm->_scumm->_saveLoadSlot = wParam - '0';
				if (GetAsyncKeyState(VK_SHIFT)<0) {
					sprintf(wm->_scumm->_saveLoadName, "Quicksave %d", wm->_scumm->_saveLoadSlot);
					wm->_scumm->_saveLoadFlag = 1;
				} else if (GetAsyncKeyState(VK_CONTROL)<0)
					wm->_scumm->_saveLoadFlag = 2;
				wm->_scumm->_saveLoadCompatible = false;
			}

			if (GetAsyncKeyState(VK_CONTROL)<0) {
				if (wParam=='F') {
					wm->_scumm->_fastMode ^= 1;
				}

				if (wParam=='G') {
					veryFastMode ^= 1;
				}

				if (wParam=='D') {
					debugger.attach(wm->_scumm);
				}
				
				if (wParam=='S') {
					wm->_scumm->resourceStats();
				}
			}

			wm->_scumm->_keyPressed = mapKey(wParam);
			break;

		case WM_MOUSEMOVE:
			wm->_scumm->mouse.x = ((int16*)&lParam)[0];
			wm->_scumm->mouse.y = ((int16*)&lParam)[1];
			break;
		case WM_LBUTTONDOWN:
			wm->_scumm->_leftBtnPressed |= msClicked|msDown;
			break;
		case WM_LBUTTONUP:
			wm->_scumm->_leftBtnPressed &= ~msDown;
			break;
		case WM_RBUTTONDOWN:
			wm->_scumm->_rightBtnPressed |= msClicked|msDown;
			break;
		case WM_RBUTTONUP:
			wm->_scumm->_rightBtnPressed &= ~msDown;
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

#if USE_GDI

bool WndMan::allocateDIB(int w, int h) {
	struct {
		BITMAPINFOHEADER bih;
		RGBQUAD rgb[256];
	} d;

	if (dib.hSect)
		return true;

	memset(&d.bih, 0, sizeof(d.bih));
	d.bih.biSize = sizeof(d.bih);
	d.bih.biWidth = w;
	d.bih.biHeight = -h;
	d.bih.biPlanes = 1;
	d.bih.biBitCount = 8;
	d.bih.biCompression = BI_RGB;

	memcpy(d.rgb, dib.pal, 256*sizeof(RGBQUAD));
	dib.new_pal=false;

	dib.hSect = CreateDIBSection(0, (BITMAPINFO*)&d, DIB_RGB_COLORS, (void**)&dib.buf,
		NULL, NULL);

	return dib.hSect != NULL;
}

void WndMan::writeToScreen() {
	RECT r;
	HDC dc,bmpdc;
	HBITMAP bmpOld;
#if DEST_WIDTH==320
	if (_vgabuf) {
		for (int y=0; y<200; y++) {
			memcpy(dib.buf + y*320,_vgabuf + y*320, 320);
		}
	}
#endif

	r.left = r.top = 0;
	r.right = DEST_WIDTH;
	r.bottom = DEST_HEIGHT;

	dc = GetDC(hWnd);
	
	bmpdc = CreateCompatibleDC(dc);
	bmpOld = (HBITMAP)SelectObject(bmpdc, dib.hSect);

	if (dib.new_pal) {
		dib.new_pal = false;
		SetDIBColorTable(bmpdc, 0, 256, dib.pal);
	}

	SetStretchBltMode(dc, BLACKONWHITE);

#if DEST_WIDTH==320
	StretchBlt(dc, r.left, r.top, r.right-r.left, r.bottom-r.top, bmpdc, 0, 0, 320,200, SRCCOPY);
#endif


	SelectObject(bmpdc, bmpOld);
	DeleteDC(bmpdc);
	ReleaseDC(hWnd, dc);
}

void WndMan::setPalette(byte *ctab, int first, int num) {
	int i;

	for (i=0; i<256; i++) {
		dib.pal[i].rgbRed = ctab[i*3+0];
		dib.pal[i].rgbGreen = ctab[i*3+1];
		dib.pal[i].rgbBlue = ctab[i*3+2];
	}

	dib.new_pal = true;
}

#endif

HWND globWnd;

void WndMan::init() {

	/* Retrieve the handle of this module */
	hInst = GetModuleHandle(NULL);

	/* Register the window class */
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInst;
	wcex.hIcon			= 0;
	wcex.hCursor		= ::LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= 0;	
	wcex.lpszClassName	= "ScummVM";
	wcex.hIconSm		= 0;
	if (!RegisterClassEx(&wcex))
		Error("Cannot register window class!");

#if USE_GDI
	globWnd = hWnd = CreateWindow("ScummVM", "ScummVM", WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT, DEST_WIDTH+10, DEST_HEIGHT+30, NULL, NULL, hInst, NULL);
	SetWindowLong(hWnd, GWL_USERDATA, (long)this);

	dib.pal = (RGBQUAD*)calloc(sizeof(RGBQUAD),256);
	dib.new_pal = false;

	if (!allocateDIB(DEST_WIDTH, DEST_HEIGHT))
		Error("allocateDIB failed!");

	ShowWindow(hWnd, SW_SHOW);
#endif

}


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


unsigned long rdtsc_timer;

void _declspec(naked) beginpentiumtest() {
	_asm {
		rdtsc
		mov rdtsc_timer,eax
		ret
	}
}

int _declspec(naked) endpentiumtest() {
	_asm {
		rdtsc
		sub eax,rdtsc_timer
		ret
	}
}


void decompressMask(byte *d, byte *s, int w=320, int h=144) {
	int x,y;
	
	for (y=0; y<h; y++) {
		byte *p = s+y*40;
		byte *pd = d + y*320;
		byte bits = 0x80, bdata = *p++;
		for (x=0; x<w; x++) {
			*pd++ = (bdata & bits) ? 128 : 0;
			bits>>=1;
			if (!bits) {
				bdata = *p++;
				bits=0x80;
			}
		}
	}
}

void outputlittlemask(byte *mask, int w, int h) {
	byte *old = wm->_vgabuf;
	wm->_vgabuf = NULL;
	decompressMask(wm->dib.buf, mask, w, h);
	wm->writeToScreen();	
	wm->_vgabuf = old;
}

void outputdisplay2(Scumm *s, int disp) {
	byte *old = wm->_vgabuf;

	byte buf[64000];

	switch(disp) {
	case 0:
		wm->_vgabuf = buf;
		memcpy(buf, wm->_vgabuf, 64000);
		memcpy(buf,s->getResourceAddress(rtBuffer, 5),320*200);
		break;
	case 1:
		wm->_vgabuf = buf;
		memcpy(buf, wm->_vgabuf, 64000);
		memcpy(buf,s->getResourceAddress(rtBuffer, 1),320*200);
		break;
	case 2:
		wm->_vgabuf = NULL;
		decompressMask(wm->dib.buf, s->getResourceAddress(rtBuffer, 9)+s->_screenStartStrip);
		break;
	case 3:
		wm->_vgabuf = NULL;
		decompressMask(wm->dib.buf, s->getResourceAddress(rtBuffer, 9)+8160+s->_screenStartStrip);
		break;
	case 4:
		wm->_vgabuf = NULL;
		decompressMask(wm->dib.buf, s->getResourceAddress(rtBuffer, 9)+8160*2+s->_screenStartStrip);
		break;
	case 5:
		wm->_vgabuf = NULL;
		decompressMask(wm->dib.buf, s->getResourceAddress(rtBuffer, 9)+8160*3+s->_screenStartStrip);
		break;
	}
	wm->writeToScreen();	
	wm->_vgabuf = old;
}

void blitToScreen(Scumm *s, byte *src,int x, int y, int w, int h) {
	byte *dst;
	SDL_Rect *r;
	int i;

	dst = (byte*)wm->_vgabuf + y*320 + x;

	do {
		memcpy(dst, src, w);
		dst += 320;
		src += 320;
	} while (--h);

}

void setShakePos(Scumm *s, int shake_pos) {}


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
		if (s->_fastMode)
			delay=10;
		Sleep(delay);
	} 
}

void initGraphics(Scumm *s, bool fullScreen, unsigned int scaleFactor) {
	if(fullScreen)
		warning("Use SDL for fullscreen support");
	scale = scaleFactor;  // not supported yet! ignored.
}

void drawMouse(Scumm *s, int, int, int, byte*, bool) {
}

void drawMouse(Scumm *s, int x, int y, int w, int h, byte *buf, bool visible) {
}

void fill_buffer(int16 *buf, int len) {
	scumm.mixWaves(buf, len);
}

void cd_playtrack(int track, int offset, int delay) { // FIXME: Add for Win32
;
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

		if (!snd_driv.wave_based()) {
			cur = GetTickCount();
			while (time < cur) {
				sound.on_timer();
				time += 10;
			}
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
}


#undef main
int main(int argc, char* argv[]) {
	int delta;

	wm->init();
	wm->_vgabuf = (byte*)calloc(320,200);
	wm->_scumm = &scumm;

	sound.initialize(&scumm,&snd_driv);

	wm->sound_init();

	scumm._gui = &gui;
	scumm.scummMain(argc, argv);

        if (!(scumm._features & GF_SMALL_HEADER))
                gui.init(&scumm);

	delta = 0;
	do {
		updateScreen(&scumm);

		waitForTimer(&scumm, delta*15);

		if (gui._active) {
			gui.loop();
			delta = 3;
		} else {
			delta = scumm.scummLoop(delta);
		}
	} while(1);

	return 0;
}

void BoxTest(int num) {;}       // Test code
