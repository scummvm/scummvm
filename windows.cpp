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
 * Change Log:
 * $Log$
 * Revision 1.5  2001/10/16 10:01:48  strigeus
 * preliminary DOTT support
 *
 * Revision 1.4  2001/10/12 07:24:06  strigeus
 * fast mode support
 *
 * Revision 1.3  2001/10/10 10:02:33  strigeus
 * alternative mouse cursor
 * basic save&load
 *
 * Revision 1.2  2001/10/09 19:02:28  strigeus
 * command line parameter support
 *
 * Revision 1.1.1.1  2001/10/09 14:30:13  strigeus
 *
 * initial revision
 *
 *
 */

#if USE_DIRECTX
#define INITGUID
#include <ddraw.h>
#endif

#include "stdafx.h"
#include <assert.h>

#if USE_DRAWDIB
#include <vfw.h>
#endif

#include "scumm.h"

#define SRC_WIDTH 320
#define SRC_HEIGHT 200
#define SRC_PITCH (320)

#define DEST_WIDTH 320
#define DEST_HEIGHT 200

#define USE_DIRECTX 0
#define USE_DRAWDIB 0
#define USE_GDI 1

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
//	BITMAPINFO *biHeader;
//	byte *BmpBG;
public:
	DIB dib;
private:
#endif

#if USE_DRAWDIB
	BITMAPINFOHEADER *biHeader;
	HDRAWDIB hdb;
#endif

public:
	byte *_vgabuf;

	Scumm *_scumm;

#if USE_DIRECTX
	IDirectDrawSurface4 *MainSurf,*Surf2;
	IDirectDraw *DxObject;
	IDirectDraw4 *Dx4Object;
	IDirectDrawPalette *DxPal;
	bool OutOfGame;
	void InitDirectX();
#endif

public:
	void init();

	bool handleMessage();
	void run();
	void setPalette(byte *ctab, int first, int num);
	void writeToScreen();

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
WndMan wm[1];
byte veryFastMode;

void modifyslot(int sel, int what);


static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	WndMan *wm = (WndMan*)GetWindowLong(hWnd, GWL_USERDATA);	
	
	switch (message) 
	{
		case WM_DESTROY:
		case WM_CLOSE:
			PostQuitMessage(0);
			break;
		case WM_CHAR:
			wm->_scumm->_keyPressed = wParam;
			break;

		case WM_KEYDOWN:
			if (wParam>='0' && wParam<='9') {
				wm->_scumm->_saveLoadSlot = wParam - '0';
				if (GetAsyncKeyState(VK_SHIFT)<0)
					wm->_scumm->_saveLoadFlag = 1;
				else if (GetAsyncKeyState(VK_CONTROL)<0)
					wm->_scumm->_saveLoadFlag = 2;
			}

			if (wParam=='F') {
				wm->_scumm->_fastMode ^= 1;
			}

			if (wParam=='G') {
				veryFastMode ^= 1;
			}

			break;

		case WM_MOUSEMOVE:
			wm->_scumm->mouse.x = ((int16*)&lParam)[0];
			wm->_scumm->mouse.y = ((int16*)&lParam)[1];
			break;
		case WM_LBUTTONDOWN:
			wm->_scumm->_leftBtnPressed |= 1;
			break;
		case WM_RBUTTONDOWN:
			wm->_scumm->_rightBtnPressed |= 1;
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

#if USE_GDI

void copy_320x200_to_640x400(byte *s, byte *d) {
	_asm {
		push ebp

		mov esi,s
		mov edi,d
		
		mov ebp,SRC_HEIGHT
		againouter:

		mov ebx,320/4
		againinner:
			mov eax,[esi]
			mov dl,ah
			mov dh,ah
			shl edx,16
			mov dl,al
			mov dh,al
			shr eax,16
			mov cl,ah
			mov	ch,ah
			shl ecx,16
			mov cl,al
			mov ch,al

			mov [edi],edx
			mov [edi+4],ecx
			mov [edi+640],edx
			mov [edi+644],ecx

			add esi,4
			add edi,8

			dec ebx
		jnz againinner
//		add esi,15
		add edi,640
		dec ebp
		jnz againouter
		pop ebp
	}
}

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
//	if (!BmpBG)
//		BmpBG = (byte*)LocalAlloc(LMEM_FIXED, DEST_WIDTH*DEST_HEIGHT);
#if DEST_WIDTH==640
	copy_320x200_to_640x400(_vgabuf, dib.buf);
#endif
#if DEST_WIDTH==320
	if (_vgabuf) {
		for (int y=0; y<200; y++) {
			memcpy(dib.buf + y*320,_vgabuf + y*320, 320);
		}
	}
#endif

//	setsel(dib.buf);

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
#if DEST_WIDTH==640
	StretchBlt(dc, r.left, r.top, r.right-r.left, r.bottom-r.top, bmpdc, 0, 0, 640,480, SRCCOPY);
#endif

#if DEST_WIDTH==320
	StretchBlt(dc, r.left, r.top, r.right-r.left, r.bottom-r.top, bmpdc, 0, 0, 320,200, SRCCOPY);
#endif
//	//StretchDIBits(dc, r.left, r.top, r.right - r.left, r.bottom - r.top, 0, 0, DEST_WIDTH, DEST_HEIGHT, BmpBG, biHeader, DIB_RGB_COLORS, SRCCOPY);


	SelectObject(bmpdc, bmpOld);
	DeleteDC(bmpdc);
	ReleaseDC(hWnd, dc);
}

void WndMan::setPalette(byte *ctab, int first, int num) {
	int i;

#if 1
	for (i=0; i<256; i++) {
		dib.pal[i].rgbRed = ctab[i*3+0]<<2;
		dib.pal[i].rgbGreen = ctab[i*3+1]<<2;
		dib.pal[i].rgbBlue = ctab[i*3+2]<<2;
	}
#else
	for (i=0; i<256; i++) {
		dib.pal[i].rgbRed = i;
		dib.pal[i].rgbGreen = i;
		dib.pal[i].rgbBlue = i;
	}
#endif

	dib.new_pal = true;
}

#endif

#if USE_DIRECTX

void WndMan::writeToScreen() {
	RECT r;
	DDSURFACEDESC2 dd;

	r.left = r.top = 0;
	r.right = SRC_WIDTH;
	r.bottom = SRC_HEIGHT;

	if (OutOfGame) {
		if (GetForegroundWindow() != hWnd) return;
		OutOfGame = false;
	}
	
	dd.dwSize = sizeof(dd);
	
	int j;
	do {
		j = MainSurf->Lock(NULL, &dd, DDLOCK_WRITEONLY, NULL);
		if (j!=DDERR_SURFACELOST) break;
		if (MainSurf->Restore() != DD_OK) {
			OutOfGame = true;
			return;
		}
	} while (1);
	
	if (j == DD_OK) {
		byte *d = (byte*)dd.lpSurface;
		byte *s = _vgabuf;

		for(int h=SRC_HEIGHT;--h>=0; ) {
			memcpy(d, s, SRC_WIDTH);
			d+=dd.lPitch;
			s+=SRC_PITCH;
		}

		MainSurf->Unlock(NULL);
	}
}

void WndMan::setPalette(byte *ctab, int first, int num) {
	PALETTEENTRY pal[256];

	for (int i=0; i<256; i++) {
		pal[i].peFlags = 0;
		pal[i].peRed = *ctab++;
		pal[i].peGreen = *ctab++;
		pal[i].peBlue = *ctab++;
	}

	DxPal->SetEntries(0, 0, 256, (PALETTEENTRY*)&pal);
}

IDirectDrawSurface4 *CreateMainSurface(IDirectDraw4 *dd);

void WndMan::InitDirectX() {
	
	if (DirectDrawCreate(NULL, &DxObject, NULL) != DD_OK) Error("DirectDrawCreate failed!");
	if (DxObject->QueryInterface(IID_IDirectDraw4, (void**)&Dx4Object) != DD_OK) Error("QueryInterface failed!");

if (Dx4Object->SetCooperativeLevel(hWnd,DDSCL_NORMAL) != DD_OK) Error("SetCooperativeLevel failed!");

	DDCAPS ddcaps;
	BOOL bHasOverlay, bHasColorKey, bCanStretch;
	ddcaps.dwSize = sizeof( ddcaps );
	if (Dx4Object->GetCaps(&ddcaps, NULL ) != DD_OK) Error("GetCaps failed!");

	/* Determine if the hardware supports overlay surfaces */
	bHasOverlay = ddcaps.dwCaps & DDCAPS_OVERLAY;

	/* Determine if the hardware supports colorkeying */
	bHasColorKey = ((ddcaps.dwCaps & DDCAPS_COLORKEY) == DDCAPS_COLORKEY) ? TRUE : FALSE;

	/* Determine if the hardware supports scaling of the overlay surface */
	bCanStretch = ((ddcaps.dwCaps & DDCAPS_OVERLAYSTRETCH) == DDCAPS_OVERLAYSTRETCH) ? TRUE : FALSE;

   if ( ( ddcaps.dwCaps & DDCAPS_ALIGNBOUNDARYDEST ) ||
        ( ddcaps.dwCaps & DDCAPS_ALIGNBOUNDARYSRC ) ||
        ( ddcaps.dwCaps & DDCAPS_ALIGNSIZEDEST ) ||
        ( ddcaps.dwCaps & DDCAPS_ALIGNSIZESRC ) ) {

			Error("Alignment restriction!");
	}

    // Are any overlays available for use?
  if ( ddcaps.dwMaxVisibleOverlays == 
				ddcaps.dwCurrVisibleOverlays )
	{
		Error("No overlay free");

	}


	if (!bHasOverlay || !bHasColorKey || !bCanStretch) {
		Error("Bad hardware!");
	}

	/* Create primary surface */
		
	DDSURFACEDESC2 ddsd;
	DDSCAPS ddscaps;
	LPDIRECTDRAWSURFACE4 lpFrontBuffer;
	LPDIRECTDRAWSURFACE4 lpBackBuffer;
	LPDIRECTDRAWSURFACE4 lpPrimary;
	HRESULT LastError;
	RECT rectOverlaySource, rectOverlayDest;
	DDOVERLAYFX ddofx;

	

//	if (!CreateMainSurface(Dx4Object))
//		Error("sad");

	/* Create the primary surface */
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS  | DDSD_BACKBUFFERCOUNT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE|DDSCAPS_FLIP |
                              DDSCAPS_COMPLEX |
                              DDSCAPS_VIDEOMEMORY;
	ddsd.dwBackBufferCount = 1;

	if (Dx4Object->CreateSurface(&ddsd, &lpPrimary, NULL) != DD_OK)
		Error("Main surface creation failed!");
	
	/* Create a flippable scaleable overlay surface */
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_BACKBUFFERCOUNT | DDSD_PIXELFORMAT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_VIDEOMEMORY;
	ddsd.dwBackBufferCount = 1; /* One back buffer for triple buffering, set to 2 */
	ddsd.dwWidth = 320;
	ddsd.dwHeight = 240; 
//	ddsd.ddckCKDestOverlay.dwColorSpaceLowValue = 0x123456;
//	ddsd.ddckCKDestOverlay.dwColorSpaceHighValue = 0x123456;


	ddsd.ddpfPixelFormat.dwSize = sizeof(ddsd.ddpfPixelFormat);

  ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
  ddsd.ddpfPixelFormat.dwFourCC = 0;
  ddsd.ddpfPixelFormat.dwRGBBitCount = 16;
  ddsd.ddpfPixelFormat.dwRBitMask = 0x7C00;	//0x7C00 is a hexadecimal memory address
  ddsd.ddpfPixelFormat.dwGBitMask = 0x03e0;
  ddsd.ddpfPixelFormat.dwBBitMask = 0x001F;
  ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0;

//	ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_PALETTEINDEXED8;
//	ddsd.ddpfPixelFormat.dwRGBBitCount = 8;

	if ((LastError = Dx4Object->CreateSurface(&ddsd, &lpFrontBuffer, NULL)) != DD_OK) {
		if (LastError==DDERR_NOOVERLAYHW )
			Error("No hardware!");
		else
			Error("2nd surface failed");
	}

#if 0
	if (Dx4Object->SetCooperativeLevel(hWnd, DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE) != DD_OK) Error("SetCooperativeLevel failed!");
	//	if (Dx4Object->SetDisplayMode(SRC_WIDTH,SRC_HEIGHT,8,0,DDSDM_STANDARDVGAMODE) != DD_OK) Error("SetDisplayMode failed!");
	if (!(MainSurf = CreateMainSurface(Dx4Object))) Error("CreateMainSurface failed!");
	if (!(Surf2 = Create2ndSurface(Dx4Object, _vgabuf))) Error("Create 2ndSurface failed!");
	if (!(DxPal = CreateGamePalette(Dx4Object))) Error("CreateGamePalette failed!");
	if (MainSurf->SetPalette(DxPal) != DD_OK) Error("SetPalette Failed!");
#endif

}


IDirectDrawSurface4 *CreateMainSurface(IDirectDraw4 *dd) {
	DDSURFACEDESC2 d;
	IDirectDrawSurface4 *ds;

//	if(dd->GetGDISurface(&ds) != DD_OK)
//		return NULL;

	memset(&d, 0, sizeof(d));

	d.dwSize = sizeof(d);
	d.dwFlags = DDSD_CAPS;
	d.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	
	int i;
	if ((i = dd->CreateSurface(&d, &ds, NULL)) != DD_OK)
		return NULL;	
	
	return ds;
}


IDirectDrawSurface4 *Create2ndSurface(IDirectDraw4 *dd, byte *surfmem) {
	DDSURFACEDESC2 d;
	IDirectDrawSurface4 *ds;

	memset(&d, 0, sizeof(d));

	d.dwSize = sizeof(d);
	d.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PITCH | /*DDSD_LPSURFACE |*/ DDSD_PIXELFORMAT;
	d.dwWidth = 640/*SRC_WIDTH*/;
	d.dwHeight = 480/*SRC_HEIGHT*/;
	d.lPitch = 640;
	d.lpSurface = surfmem;

	d.ddpfPixelFormat.dwSize  = sizeof(DDPIXELFORMAT);
	d.ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_PALETTEINDEXED8;
	d.ddpfPixelFormat.dwRGBBitCount = 8;

	d.ddsCaps.dwCaps = DDSCAPS_OVERLAY;

	int i;
	if ((i = dd->CreateSurface(&d, &ds, NULL)) != DD_OK)
		return NULL;
	return ds;
}

IDirectDrawPalette *CreateGamePalette(IDirectDraw4 *dd) {
	PALETTEENTRY pal[256];
	int i;
	IDirectDrawPalette *p;

	memset(&pal, 0, sizeof(pal));
	if ((i=dd->CreatePalette(DDPCAPS_8BIT | DDPCAPS_ALLOW256, pal, &p, NULL)) != DD_OK)
		return NULL;

	return p;
}

#endif

#if USE_DRAWDIB
void WndMan::writeToScreen() {
	RECT r;
	HDC dc;

	r.left = r.top = 0;
	r.right = DEST_WIDTH/2;
	r.bottom = DEST_HEIGHT/2;

	dc = GetDC(hWnd);
	
	DrawDibRealize(hdb, dc, TRUE);
	DrawDibDraw(hdb, dc, r.left, r.top, r.right-r.left, r.bottom-r.top, biHeader, _vgabuf, 0, 0, 320, 240, 0);
	
//		StretchDIBits(dc, r.left, r.top, r.right - r.left, r.bottom - r.top, 0, 0, DEST_WIDTH, DEST_HEIGHT, BmpBG, biHeader, DIB_RGB_COLORS, SRCCOPY);
	ReleaseDC(hWnd, dc);
}

void WndMan::setPalette(byte *ctab, int first, int num) {
	PALETTEENTRY pal[256];
	for (int i=0; i < num; i++,ctab+=3) {
		pal[i].peFlags = 0;
		pal[i].peRed = ctab[0];
		pal[i].peGreen = ctab[1];
		pal[i].peBlue = ctab[2];
	}

	DrawDibChangePalette(hdb, 0, 16, pal);

	GetLastError();
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

#if USE_DIRECTX
	hWnd = CreateWindow("ScummVM", "ScummVM", 0,
      CW_USEDEFAULT, CW_USEDEFAULT, SRC_WIDTH, SRC_HEIGHT, NULL, NULL, hInst, NULL);

	SetWindowLong(hWnd, GWL_USERDATA, (long)this);
	SetWindowLong(hWnd, GWL_STYLE, 0);
	ShowCursor(false);

	
	InitDirectX();

	ShowWindow(hWnd, SW_SHOW);
#endif

#if USE_GDI
	globWnd = hWnd = CreateWindow("ScummVM", "ScummVM", WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT, DEST_WIDTH+10, DEST_HEIGHT+30, NULL, NULL, hInst, NULL);
	SetWindowLong(hWnd, GWL_USERDATA, (long)this);
//	ShowCursor(false);

	dib.pal = (RGBQUAD*)calloc(sizeof(RGBQUAD),256);
	dib.new_pal = false;

	if (!allocateDIB(DEST_WIDTH, DEST_HEIGHT))
		Error("allocateDIB failed!");

#if 0
	biHeader = (BITMAPINFO*)LocalAlloc(LMEM_FIXED, sizeof(BITMAPINFOHEADER) + 1024 );
	memset(biHeader, 0, sizeof(BITMAPINFOHEADER) + 1024);

	biHeader->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	biHeader->bmiHeader.biWidth = DEST_WIDTH;
	biHeader->bmiHeader.biHeight = -DEST_HEIGHT;	/* top down */
	biHeader->bmiHeader.biPlanes = 1;
	biHeader->bmiHeader.biBitCount = 8;	/* 256 colors */
	biHeader->bmiHeader.biCompression = BI_RGB;	/* uncompressed */
#endif

	ShowWindow(hWnd, SW_SHOW);
#endif

#if USE_DRAWDIB
	hdb = DrawDibOpen();

	hWnd = CreateWindow("ScummVM", "ScummVM", WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT, DEST_WIDTH+10, DEST_HEIGHT+30, NULL, NULL, hInst, NULL);
	SetWindowLong(hWnd, GWL_USERDATA, (long)this);
	ShowCursor(false);

	biHeader = (BITMAPINFOHEADER*)LocalAlloc(LMEM_FIXED, sizeof(BITMAPINFOHEADER));
	memset(biHeader, 0, sizeof(BITMAPINFOHEADER));

	biHeader->biSize = sizeof(BITMAPINFOHEADER);
	biHeader->biWidth = SRC_PITCH;
	biHeader->biHeight = SRC_HEIGHT;	/* top down */
	biHeader->biPlanes = 1;
	biHeader->biBitCount = 8;	/* 256 colors */
	biHeader->biCompression = BI_RGB;	/* uncompressed */

	ShowWindow(hWnd, SW_SHOW);

//	int k = DrawDibProfileDisplay(biHeader);
//	printf("%d\n", k&PD_CAN_STRETCHDIB);
		

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




void decompressMask(byte *d, byte *s) {
	int x,y;
	byte bits = 0x80, bdata = *s++;

	for (y=0; y<144; y++)
		for (x=0; x<320; x++) {
			*d++ = (bdata & bits) ? 128 : 0;
			bits>>=1;
			if (!bits) {
				bdata = *s++;
				bits=0x80;
			}
		}
}



void outputdisplay2(Scumm *s, int disp) {
	byte *old = wm->_vgabuf;

	byte buf[64000];

	switch(disp) {
	case 0:
		wm->_vgabuf = buf;
		memcpy(buf, wm->_vgabuf, 64000);
		memcpy(buf+320*144,s->getResourceAddress(0xA, 7),320*56);
		break;
	case 1:
		wm->_vgabuf = buf;
		memcpy(buf, wm->_vgabuf, 64000);
		memcpy(buf+320*144,s->getResourceAddress(0xA, 3),320*56);
		break;
	case 2:
		wm->_vgabuf = NULL;
		decompressMask(wm->dib.buf, s->getResourceAddress(0xA, 9)+s->_screenStartStrip);
		break;
	case 3:
		wm->_vgabuf = NULL;
		decompressMask(wm->dib.buf, s->getResourceAddress(0xA, 9)+5920+s->_screenStartStrip);
		break;
	case 4:
		wm->_vgabuf = NULL;
		decompressMask(wm->dib.buf, s->getResourceAddress(0xA, 9)+5920*2+s->_screenStartStrip);
		break;
	case 5:
		wm->_vgabuf = NULL;
		decompressMask(wm->dib.buf, s->getResourceAddress(0xA, 9)+5920*3+s->_screenStartStrip);
		break;
	}
	wm->writeToScreen();	
	wm->_vgabuf = old;
}


#if 0
void outputdisplay(Scumm *s) {
	s->drawMouse();
	wm->writeToScreen();	
}
#endif

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

int clock;

void updateScreen(Scumm *s) {
	if (s->_palDirtyMax != -1) {
		wm->setPalette(s->_currentPalette, 0, 256);	
		s->_palDirtyMax = -1;
	}

	wm->writeToScreen();
}



void waitForTimer(Scumm *s) {
	if (!veryFastMode) {
		Sleep(5);
	} 
	s->_scummTimer+=2;
	wm->handleMessage();
}

void initGraphics(Scumm *s) {

}

void drawMouse(Scumm *s, int, int, int, byte*, bool) {
}

#undef main
int main(int argc, char* argv[]) {
	scumm._videoMode = 0x13;

#if defined(DOTT)
	scumm._exe_name = "tentacle";
#else
	scumm._exe_name = "monkey2";
#endif

	wm->init();
	wm->_vgabuf = (byte*)calloc(320,200);
	wm->_scumm = &scumm;
	
	scumm.scummMain(argc, argv);

	return 0;
}

