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
 */
/**************************************************************************
 *                                     ออออออออออออออออออออออออออออออออออ *
 *                                             Nayma Software srl         *
 *                    e                -= We create much MORE than ALL =- *
 *        u-        z$$$c        '.    ออออออออออออออออออออออออออออออออออ *
 *      .d"        d$$$$$b        "b.                                     *
 *   .z$*         d$$$$$$$L        ^*$c.                                  *
 *  #$$$.         $$$$$$$$$         .$$$" Project: Roasted Moths........  *
 *    ^*$b       4$$$$$$$$$F      .d$*"                                   *
 *      ^$$.     4$$$$$$$$$F     .$P"     Module:  Window.HPP...........  *
 *        *$.    '$$$$$$$$$     4$P 4                                     *
 *     J   *$     "$$$$$$$"     $P   r    Author:  Giovanni Bajo........  *
 *    z$   '$$$P*4c.*$$$*.z@*R$$$    $.                                   *
 *   z$"    ""       #$F^      ""    '$c  Desc:    Classi per la gestione *
 *  z$$beu     .ue="  $  "=e..    .zed$$c          di una finestra Direct 
 *      "#$e z$*"   .  `.   ^*Nc e$""              Draw.................  *
 *         "$$".  .r"   ^4.  .^$$"                 .....................  *
 *          ^.@*"6L=\ebu^+C$"*b."                                         *
 *        "**$.  "c 4$$$  J"  J$P*"    OS:  [ ] DOS  [X] WIN95  [ ] PORT  *
 *            ^"--.^ 9$"  .--""      COMP:  [ ] WATCOM  [X] VISUAL C++    *
 *                    "                     [ ] EIFFEL  [ ] GCC/GXX/DJGPP *
 *                                                                        *
 * This source code is Copyright (C) Nayma Software.  ALL RIGHTS RESERVED *
 *                                                                        *
 **************************************************************************/

#include "tony/window.h"
#include "tony/game.h"
#include "tony/tony.h"

namespace Tony {

#define DDRELEASE(x)		if (x) { (x)->Release(); (x)=NULL; }

// Tabella per il b&w. Globale per accederci dalla roba ASM
static uint16 m_wPrecalcTable[0x10000];

/****************************************************************************\
*       Metodi di RMWindow
\****************************************************************************/

#ifdef REFACTOR_ME
LRESULT CALLBACK GlobalWindowProc(HWND hWnd, uint32 msg, uint16 wParam, int32 lParam) {
	if ((HWND)theGame.m_wnd == NULL)
			return DefWindowProc(hWnd, msg, wParam, lParam);

	switch (msg) {
	case WM_CREATE:
		return 0;

	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	
	default:
		return theGame.m_wnd.WindowProc(msg, wParam, lParam);
	}
}
#endif

#ifdef REFACTOR_ME
LRESULT RMWindow::WindowProc(uint32 msg, uint16 wParam, int32 lParam) {
	switch (msg) {
	case WM_ACTIVATE:
		if (LOWORD(wParam)!=WA_INACTIVE)
			theGame.Pause(false);
		else
			theGame.Pause(true);
		return 0;

	case WM_ENTERMENULOOP:
	case WM_ENTERSIZEMOVE:
		if (!m_bFullscreen)
			theGame.Pause(true);
		return 0;

	case WM_EXITMENULOOP:
	case WM_EXITSIZEMOVE:
		if (!m_bFullscreen)
			theGame.Pause(false);
		return 0;
/*
		case WM_KEYDOWN:
			switch (wParam) {
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					if (GetAsyncKeyState(VK_CONTROL)&0x8000)
						theGame.SaveState(wParam-'0');
					else if (GetAsyncKeyState(VK_SHIFT)&0x8000)
						theGame.LoadState(wParam-'0');
					return 0;
			}
			return 0;
*/			

	case WM_PAINT:
//			Repaint();
		ValidateRect(m_hWnd, NULL);
		return 0;

	case WM_COMMAND:
		switch LOWORD(wParam) {
/*
		case ID_OPTIONS:
			theGame.OptionScreen();
			break;
*/

		case ID_FULLSCREEN:
			theGame.SwitchFullscreen(!m_bFullscreen);
			break;
/*
		case ID_ENABLEINPUT:
			theGame.GetEngine()->EnableInput();
			break;
*/
		case ID_SCREENSHOT:
			m_bGrabScreenshot = true;
//					m_bGrabMovie = !m_bGrabMovie;
			break;

		case ID_MOVIE:
//					m_bGrabMovie = !m_bGrabMovie;
			break;
/*
		case ID_BLACKWHITE:
			m_bBlackWhite = !m_bBlackWhite;
			break;
*/
		default:
			return DefWindowProc(m_hWnd, msg, wParam, lParam);
		}
		return 0;

	case WM_SYSKEYDOWN:
		if (m_bFullscreen)
			return 0;
		else
			return DefWindowProc(m_hWnd ,msg, wParam, lParam);

	default:
		if (m_hWnd != NULL) // Fix del bug visto da BoundsChecker
			return DefWindowProc(m_hWnd, msg, wParam, lParam);
		return 0;
	}
}
#endif

/*
HANDLE hWaitFlip;
bool bExitThread;

void DoFlipThread(LPDIRECTDRAWSURFACE lpDDSPrimary) {
	bExitThread=false;

	while (1) {
		WaitForSingleObject(hWaitFlip,INFINITE);
		if (bExitThread) _endthread();
		lpDDSPrimary->Flip(NULL,DDFLIP_WAIT);
	}
}
*/

void RMWindow::InitDirectDraw(void) {
#ifdef REFACTOR_ME
	DDInit();

	ShowCursor(false);
	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);
#endif
}

void RMWindow::Init(/*HINSTANCE hInst*/) {
#ifdef REFACTOR_ME
	WNDCLASS wc;

	// Registra la classe di finestra
	wc.style = CS_HREDRAW|CS_VREDRAW;
	wc.lpfnWndProc = GlobalWindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_ICON1));
	wc.hCursor = LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL; 
	wc.lpszClassName = "RMClass";
	RegisterClass(&wc);
 
	m_hWnd = CreateWindow(
		"RMClass",
		"Tony Tough and the night of Roasted Moths",
		WS_OVERLAPPEDWINDOW & (~WS_THICKFRAME) & (~WS_BORDER) & (~WS_MAXIMIZEBOX),
		50, 50,
		RM_SX + GetSystemMetrics(SM_CXDLGFRAME) * 2,
		RM_SY + GetSystemMetrics(SM_CYDLGFRAME) * 2 + GetSystemMetrics(SM_CYMENU) + GetSystemMetrics(SM_CYCAPTION),
		NULL,
		NULL,
		hInst,
		NULL
	);
  
	if (m_hWnd == NULL) {
		int err = GetLastError();
		RMString str;

		str.Format("error: %u",err);
		MessageBox(NULL,str,"sux",MB_OK);

		assert(0);
	}
 
	// Inizializza la finestra directdraw
	//DDInit();

	// Inizializza i conteggi degli FPS
	fps = lastfcount = fcount = lastsecond = 0;

	m_bGrabScreenshot = false;
	m_bGrabThumbnail = false;
	m_bGrabMovie = false;
	
	//hWaitFlip = CreateEvent(NULL,false,false, NULL);
#endif
};

void RMWindow::Close(void) {
	DDClose();
#ifdef REFACTOR_ME
	DestroyWindow(m_hWnd);
#endif
}

void RMWindow::GrabThumbnail(uint16 *thumbmem) {
	m_bGrabThumbnail = true;
	m_wThumbBuf = thumbmem;
}

float RGB_to_HSL(float r,float g, float b, float *h, float *s, float *l) {
    float v;
    float m;
    float vm;
    float r2, g2, b2;

    if (r > g) v = r; else v = g;
    if (v > b) v = v; else v = b;
    if (r < b) m = r; else m = b;
    if (m < b) m = m; else m = b;

    if ((*l = (m + v) / 2.0f) <= 0.0f) return *l;
    if ((*s = vm = v - m) > 0.0f) {
		*s /= (*l <= 0.5f) ? (v + m) : (2.0f - v - m) ;
    } else
		return *l;

    r2 = (v - r) / vm;
    g2 = (v - g) / vm;
    b2 = (v - b) / vm;

    if (r == v)
		*h = (g == m ? 5.0f + b2 : 1.0f - g2);
    else if (g == v)
		*h = (b == m ? 1.0f + r2 : 3.0f - b2);
    else
		*h = (r == m ? 3.0f + g2 : 5.0f - r2);

	*h /= 6;

	return *l;
}

#define ITOF(val)	(float)((float)(val) / 31.0f)
#define FTOI(val)	(int)(((float)(val) * 31.0f + 0.5f))

void RMWindow::CreateBWPrecalcTable(void) {
#define CLAMP(var, min, max)	var = (var < min ? min : (var > max ? max : var));

	int i;
	int r, g, b;
	int min, max;
	int shiftR, shiftG, shiftB;

	// Calcola i valori di shift in base alle maschere
	for (shiftR = 15; (mskRed & (1 << shiftR)) == 0; shiftR--)
		;
	for (shiftG = 15; (mskGreen & (1 << shiftG)) == 0; shiftG--)
		;
	for (shiftB = 15; (mskBlue & (1 << shiftB)) == 0; shiftB--)
		;

	shiftR -= 4; 
	shiftG -= 4; 
	shiftB -= 4; 

	// @@@ CLAMP inutile (in teoria)
	CLAMP(shiftR, 0, 15);
	CLAMP(shiftG, 0, 15);
	CLAMP(shiftB, 0, 15);

	for (i = 0; i < 0x10000; i++) {
		r=(i >> shiftR) & 0x1F;
		g=(i >> shiftG) & 0x1F;
		b=(i >> shiftB) & 0x1F;

#if 1
		if (r < g) min=r; else min = g;
		if (b < min) min = b;
		if (r > g) max = r; else max = g;
		if (b > max) max = b;
		min = (min + max) / 2;
#else
		// Nuova formula B&W. L'immagine ่ pi๙ fedele all'originale, ma l'effetto ่ peggiore
		float fr, fg, fb;

		fr = (float)r / 63.0f;
		fg = (float)g / 63.0f;
		fb = (float)b / 63.0f;

		min = (int)((fr*0.11f + fg*0.69f + fb*0.33f)*63.f);
#endif
		
		/*
		RGB_to_HSL(ITOF(r), ITOF(g), ITOF(b), &h, &s, &l);
		min = FTOI(l);
		*/

		r = min + 8 - 8;
		g = min + 5 - 8;
		b = min + 0 - 8;

		CLAMP(r, 0, 31);
		CLAMP(g, 0, 31);
		CLAMP(b, 0, 31);

		m_wPrecalcTable[i] = (b << shiftB) | (g << shiftG) | (r << shiftR);
	}
}


void RMWindow::DDClose(void) {
#ifdef REFACTOR_ME
	DDRELEASE(m_Back);
	DDRELEASE(m_Primary);
	DDRELEASE(m_MainClipper);
	DDRELEASE(m_BackClipper);
	DDRELEASE(m_DD);
#endif
}

void RMWindow::DDInit(void) {
#ifdef REFACTOR_ME
	HRESULT err;

	// Crea DirectDraw
	err = DirectDrawCreate(NULL, &m_DD, NULL);
	assert(err == DD_OK);

	// Crea il clipper
	err = DirectDrawCreateClipper(0, &m_MainClipper, NULL);
	err=DirectDrawCreateClipper(0, &m_BackClipper, NULL);

	// Lo associa alla finestra
	m_MainClipper->SetHWnd(0, m_hWnd);

	// Setta la cooperazione a normal
	m_DD->SetCooperativeLevel(m_hWnd, DDSCL_NORMAL);

#ifdef STARTFULLSCREEN
	// Di default in fullscreen
	m_bFullscreen=false;		// Fa finta, per obbligarlo a cambiare
	m_Primary=m_Back=NULL;
	SwitchFullscreen(true);
#else
	// Di default in finestra
	m_bFullscreen = true;		// Fa finta, per obbligarlo a cambiare
	m_Primary = m_Back = NULL;
	SwitchFullscreen(false);
#endif

/*
	if (!ISMODE1() && !ISMODE2() && !ISMODE3() && !ISMODE4()) {
		RMString str;
		str.Format("Graphic mode not supported: %04x %04x %04x",mskRed,mskGreen,mskBlue);
		MessageBox(m_hWnd,str,"Debug",MB_OK);
	}
*/
#endif
}


void RMWindow::SwitchFullscreen(bool bFull) {
#ifdef REFACTOR_ME
	HRESULT err;
	DDSURFACEDESC ddsd;
	DDSCAPS ddscaps;
	DDBLTFX ddbltfx;
	Common::Rect rcWin;

	// Se non c'e' niente da fare, esci
	if (bFull == m_bFullscreen)
		return;

	// Termina il thread di flipping
	//bExitThread = true;

	// Rilascia le superfici create in precedenza (se ce n'erano)
	DDRELEASE(m_Back);
	DDRELEASE(m_Primary);
	
	// Legge le coordinate della finestra
	if (m_bFullscreen) {
		rcWin.left = 50;
		rcWin.top = 50;
	} else {
		GetWindowRect(m_hWnd, &rcWin);		
	}

	if (bFull) {
		// Setta la risoluzione
		m_DD->SetCooperativeLevel(m_hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT);
		m_DD->SetDisplayMode(RM_SX, RM_SY, 16);
		
		// A tutto schermo, possiamo creare una catena di flipping
    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS|DDSD_BACKBUFFERCOUNT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_COMPLEX | DDSCAPS_FLIP;
    ddsd.dwBackBufferCount = 1;
    if ((err=m_DD->CreateSurface(&ddsd, &m_Primary, NULL)) != DD_OK) {
      //wsprintf(errbuf,"Error creating primary surface2 (%lx)",err);
      //MessageBox(hWnd,errbuf,"grSwitchFullscreen()",MB_OK);
		assert(0);
    }

    ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
    if ((err=m_Primary->GetAttachedSurface(&ddscaps, &m_Back)) != DD_OK) {
      //wsprintf(errbuf,"Error getting attached surface2 (%lx)",err);
      //MessageBox(hWnd,errbuf,"grSwitchFullscreen()",MB_OK);
		assert(0);
	}

    // Pulisce i buffer
	ddbltfx.dwSize = sizeof(ddbltfx);
    ddbltfx.dwFillColor = 0;
    m_Back->Blt(NULL, NULL, NULL,DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);

    ddbltfx.dwSize=sizeof(ddbltfx);
    ddbltfx.dwFillColor = 0;
    m_Primary->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);

		// Inizializza il thread
		//_beginthread((void (*)(void*))DoFlipThread,10240,(void*)m_Primary);
	} else {
		// In windowed, non possiamo fare flipping (purtroppo)
		m_DD->RestoreDisplayMode();
		m_DD->SetCooperativeLevel(m_hWnd,DDSCL_NORMAL);

		ZeroMemory(&ddsd,sizeof(ddsd));
		ddsd.dwSize=sizeof(ddsd);
		ddsd.dwFlags=DDSD_CAPS;
		ddsd.ddsCaps.dwCaps=DDSCAPS_PRIMARYSURFACE;
		if ((err=m_DD->CreateSurface(&ddsd,&m_Primary, NULL)) != DD_OK) {
		  //wsprintf(errbuf,"Error creating primary surface (%lx)",err);
		  //MessageBox(hWnd,errbuf,"ChangeResolution()",MB_OK);
				assert(0);
		}

		ZeroMemory(&ddsd,sizeof(ddsd));
		ddsd.dwSize=sizeof(ddsd);
		ddsd.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT;
		ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN; // se puo', in video memory
		ddsd.dwWidth=RM_SX;
		ddsd.dwHeight=RM_SY;
		if ((err=m_DD->CreateSurface(&ddsd,&m_Back, NULL)) != DD_OK) {
      //wsprintf(errbuf,"Error creating backbuffer surface (%lx)",err);
      //MessageBox(hWnd,errbuf,"ChangeResolution()",MB_OK);
			assert(0);
		}

		// Pulizia per favore
		ddbltfx.dwSize = sizeof(ddbltfx);
		ddbltfx.dwFillColor = 0;
		m_Back->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
	}

	// Posiziona e ridimensiona la finestra
	if (bFull) {
		SetWindowPos(m_hWnd, HWND_TOP, 0, 0, RM_SX, RM_SY, 0);

		// Disabilita il clipper (non necessario)
		m_Primary->SetClipper(NULL);
	} else {
		SetWindowPos(m_hWnd, HWND_TOP, rcWin.left, rcWin.top, RM_SX + GetSystemMetrics(SM_CXDLGFRAME) * 2,
			RM_SY + GetSystemMetrics(SM_CYDLGFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION), 0);		

		m_Primary->SetClipper(m_MainClipper);
		//m_Primary->SetClipper(NULL);
	}

	// Si ricorda il nuovo stato
	m_bFullscreen = bFull;
	InvalidateRect(m_hWnd, NULL, false);

	// Legge il nuovo pixel format
	UpdatePixelFormat();
#endif
}

void RMWindow::UpdatePixelFormat(void) {
#ifdef REFACTOR_ME
	DDPIXELFORMAT ddpf;

	// Si fa ridare il nuovo pixel format
	ddpf.dwSize = sizeof(ddpf);
	m_Primary->GetPixelFormat(&ddpf);

	// Copia le componenti dei colori
	mskRed = ddpf.dwRBitMask;
	mskGreen = ddpf.dwGBitMask;
	mskBlue = ddpf.dwBBitMask;

	// Ricalcola la tabella per l'effetto b&w
	CreateBWPrecalcTable();	
#endif
}


void RMWindow::Repaint(void) {
#ifdef REFACTOR_ME
	Common::Rect rcWin;
	HRESULT err;

	if (m_Primary==NULL)
		return;
	
	// Se siamo a tutto schermo, basta un flip
	if (m_bFullscreen) {
		m_DD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);

		// Flippa flappa
		while (1) {
			//err=m_Primary->Flip(NULL,DDFLIP_WAIT);
			err = m_Primary->BltFast(0, 0, m_Back, NULL, DDBLTFAST_WAIT);

			if (err==DD_OK)
				break;
			else if (err == DDERR_SURFACELOST) {
				//MessageBox(NULL,"Primary lost!","RMWindow::Repaint()",MB_OK);
				m_Primary->Restore();
				return;
			}
		}
	}	
	// Altrimenti bisogna andare di blit
	else {	
		// Si calcola il rettangolo effettivamente visibile
		GetWindowRect(m_hWnd, &rcWin);
		OffsetRect(&rcWin, GetSystemMetrics(SM_CXDLGFRAME), GetSystemMetrics(SM_CYDLGFRAME) + GetSystemMetrics(SM_CYCAPTION));

		// Aggiusta le dimensioni del rettangolo
		rcWin.right = rcWin.left + RM_SX;
		rcWin.bottom = rcWin.top + RM_SY;

		// Blit del back nel primary
		while (1) {
			err = m_Primary->Blt(&rcWin, m_Back, NULL,DDBLT_WAIT, NULL);
			if (err == DD_OK)
				break;
			else if (err == DDERR_SURFACELOST) {
				//MessageBox(NULL,"Primary lost!","RMWindow::Repaint()",MB_OK);
				m_Primary->Restore();
				return;
			}
		}
	}
#endif
}


void RMWindow::Unlock(DDSURFACEDESC &ddsd) {
#ifdef REFACTOR_ME
	m_Back->Unlock(ddsd.lpSurface);
#endif
}

void RMWindow::WipeEffect(Common::Rect &rcBoundEllipse) {
#ifdef REFACTOR_ME
	Common::Rect rcScreen;
	HRGN hrgnCombined;
	HRGN hrgnScreen;
	HRGN hrgnEllipse;
	uint32 dwDataSize;
	RGNDATA *rgnData;

	SetRect(&rcScreen, 0, 0, 640, 480);

	hrgnScreen = CreateRectRgnIndirect(&rcScreen);
	hrgnEllipse = CreateEllipticRgnIndirect(&rcBoundEllipse);
	hrgnCombined = CreateRectRgnIndirect(&rcScreen);

	CombineRgn(hrgnCombined, hrgnScreen, hrgnEllipse, RGN_DIFF);

	dwDataSize = GetRegionData(hrgnCombined, 0, NULL);
	rgnData = (RGNDATA *)new char[dwDataSize];
	GetRegionData(hrgnCombined, dwDataSize, rgnData);
	
	m_BackClipper->SetClipList(rgnData, 0);
	m_Back->SetClipper(m_BackClipper);	

	DDBLTFX ddbltfx;
	ddbltfx.dwSize=sizeof(ddbltfx);
	ddbltfx.dwFillColor = 0;
	m_Back->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);

	m_Back->SetClipper(NULL);
	delete [] rgnData;
	DeleteObject(hrgnCombined);
	DeleteObject(hrgnEllipse);
	DeleteObject(hrgnScreen);
#endif
}

bool RMWindow::Lock(DDSURFACEDESC &ddsd) {
#ifdef REFACTOR_ME
	HRESULT err;

	// Lock della surface
	ddsd.dwSize = sizeof(ddsd);
	while (1) {
		err = m_Back->Lock(NULL,&ddsd,DDLOCK_WAIT|DDLOCK_WRITEONLY|DDLOCK_NOSYSLOCK, NULL);
		if (err==DD_OK)
			break;
		else if (err==DDERR_SURFACELOST) {
//			MessageBox(NULL,"Primary lost!","RMWindow::Repaint()",MB_OK);
			m_Back->Restore();
			return false;
		}
	}
#endif
	return true;
}

void RMWindow::GetNewFrame(byte *lpBuf, Common::Rect *rcBoundEllipse) {
#ifdef REFACTOR_ME
	HRESULT err;
	DDSURFACEDESC ddsd;
	int x,y;
	byte *dest;
	uint16 *destw;
	RMString s;
	uint16 *src,src2;

	if (GetAsyncKeyState(VK_F7)&0x8001)
		goto DOFRAMERATE;

	if (!Lock(ddsd))
		return;

	// Copia del buffer
	src = (uint16 *)lpBuf;
	dest = (byte *)ddsd.lpSurface;
//	src+=RM_SKIPY*RM_BBX; Lo skipY e' integrato nell'offseting del bigbuffer

	if (ISMODE2()) {
		// 565 RGB, Marco - Matrox G200 
		int lineinc = (ddsd.lPitch / 2 - RM_SX) * 2;
		int lineinc2 = (RM_BBX - RM_SKIPX - RM_SX) * 2;
		destw = (uint16 *)dest;

		if (bCfgAnni30) {
			__asm {
				pushad
				
				mov y,RM_SY
				mov esi,src
				mov edi,dest

			//ALIGN 4
			line30:
				mov x,RM_SX
				add esi,RM_SKIPX*2
			
			line302:
				mov eax,[esi]					; U 1
				mov ebx,[esi+4]				; V 
				mov ecx,eax						; U 2
				mov edx,ebx						; V
				and eax,0x7FE07FE0		; U 3
				and ebx,0x7FE07FE0		; V
				and ecx,0x001F001F		; U 4
				and edx,0x001F001F		; V 
				shl eax,1							; U 5
				shl ebx,1							; V  
				or eax,ecx						; U 6
				or ebx,edx						; V

				// Codice B&W - Pairato
				mov ecx,eax
				mov edx,ebx
				and eax,0xFFFF
				and ebx,0xFFFF
				shr ecx,16
				shr edx,16
				mov eax,[offset m_wPrecalcTable + eax*2]
				mov ebx,[offset m_wPrecalcTable + ebx*2]
				mov ecx,[offset m_wPrecalcTable + ecx*2]
				mov edx,[offset m_wPrecalcTable + edx*2]
				shl ecx,16
				shl edx,16
				and eax,0xFFFF
				and ebx,0xFFFF
				or eax,ecx
				or ebx,edx
	
				mov [edi],eax					; U 7
				mov [edi+4],ebx				; V 
				add esi,8							; U 8 
				add edi,8							; V
				sub x,4								; U 9
				jnz line302							; V	

				add esi,lineinc2
				add edi,lineinc

				dec y
				jnz line30

				popad
			}
	
		} else {
			__asm {
				pushad
				
				mov y,RM_SY
				mov esi,src
				mov edi,dest

			//ALIGN 4
			line:
				mov x,RM_SX
				add esi,RM_SKIPX*2
			
			line2:
				mov eax,[esi]					; U 1
				mov ebx,[esi+4]				; V 
				mov ecx,eax						; U 2
				mov edx,ebx						; V
				and eax,0x7FE07FE0		; U 3
				and ebx,0x7FE07FE0		; V
				and ecx,0x001F001F		; U 4
				and edx,0x001F001F		; V 
				shl eax,1							; U 5
				shl ebx,1							; V  
				or eax,ecx						; U 6
				or ebx,edx						; V
				mov [edi],eax					; U 7
				mov [edi+4],ebx				; V 
				add esi,8							; U 8 
				add edi,8							; V
				sub x,4								; U 9
				jnz line2							; V	

				add esi,lineinc2
				add edi,lineinc

				dec y
				jnz line

				popad
			}
		}
	} else if (ISMODE3()) {
		// 5551 RGB
		int lineinc = (ddsd.lPitch / 2 - RM_SX) * 2;
		int lineinc2 = (RM_BBX - RM_SKIPX - RM_SX) * 2;
		destw=(uint16 *)dest;

		if (bCfgAnni30) {
			__asm {
				mov y,RM_SY
				mov esi,src
				mov edi,dest
			
			aline30:
				mov x,RM_SX
				add esi,RM_SKIPX*2
			
			aline302:
				mov eax,[esi]					; U 1
				mov ebx,[esi+4]				; V
				shl eax,1							; U 2
				shl ebx,1							; V

				// Codice B&W - Pairato
				mov ecx,eax
				mov edx,ebx
				and eax,0xFFFF
				and ebx,0xFFFF
				shr ecx,16
				shr edx,16
				mov eax,[offset m_wPrecalcTable + eax*2]
				mov ebx,[offset m_wPrecalcTable + ebx*2]
				mov ecx,[offset m_wPrecalcTable + ecx*2]
				mov edx,[offset m_wPrecalcTable + edx*2]
				shl ecx,16
				shl edx,16
				and eax,0xFFFF
				and ebx,0xFFFF
				or eax,ecx
				or ebx,edx

				mov [edi],eax					; U 3
				mov [edi+4],ebx				; V
				add esi,8							; U 4
				add edi,8							; V
				sub x,4								; U 5
				jnz aline302					; V

				add esi,lineinc2
				add edi,lineinc

				dec y
				jnz aline30
			}
		} else {
			__asm {
				mov y,RM_SY
				mov esi,src
				mov edi,dest
			
			aline:
				mov x,RM_SX
				add esi,RM_SKIPX*2
			
			aline2:
				mov eax,[esi]					; U 1
				mov ebx,[esi+4]				; V
				shl eax,1							; U 2
				shl ebx,1							; V
				mov [edi],eax					; U 3
				mov [edi+4],ebx				; V
				add esi,8							; U 4
				add edi,8							; V
				sub x,4								; U 5
				jnz aline2						; V

				add esi,lineinc2
				add edi,lineinc

				dec y
				jnz aline
			}
		}
	} else if (ISMODE4()) {
		// 565 BGR - Intergraph
		int lineinc=(ddsd.lPitch / 2 - RM_SX) * 2;
		int lineinc2=(RM_BBX - RM_SKIPX - RM_SX) * 2;
		destw=(uint16 *)dest;

		if (bCfgAnni30)		// Manca supporto per Intergraph ... non so pairarlo!!! :)
		{
			__asm {
				mov y,RM_SY
				mov esi,src
				mov edi,dest

			//ALIGN 4
			intersux30:
				mov ecx,RM_SX
				add esi,RM_SKIPX*2
			
			intersux302:
				mov eax,[esi]         ; U 1   0BBBBBGGGGGRRRRR 0BBBBBGGGGGRRRRR
				add esi,4             ; V
				mov ebx,eax           ; U 2   0BBBBBGGGGGRRRRR 0BBBBBGGGGGRRRRR
				mov edx,eax           ; V     0BBBBBGGGGGRRRRR 0BBBBBGGGGGRRRRR
				shl eax,11            ; U 3   RRRRR0BBBBBGGGGG RRRRR00000000000
				and ebx,0x03E003E0    ; V     000000GGGGG00000 000000GGGGG00000
				and edx,0x7C007C00    ; U 4   0BBBBB0000000000 0BBBBB0000000000
				and eax,0xF800F800    ; V     RRRRR00000000000 RRRRR00000000000
				shr edx,10            ; U 5   00000000000BBBBB 00000000000BBBBB
				add ebx,ebx           ; V     00000GGGGG000000 00000GGGGG000000
				or eax,edx            ; U 6   RRRRR000000BBBBB RRRRR000000BBBBB
				add edi,4             ; V
				or eax,ebx            ; U 7   RRRRRGGGGG0BBBBB RRRRRGGGGG0BBBBB
				sub ecx,2             ; V
				mov [edi-4],eax       ; U 8
				jnz intersux302				; V

				add esi,lineinc2
				add edi,lineinc

				dec y
				jnz intersux30
			}
		} else {
			__asm {
				mov y,RM_SY
				mov esi,src
				mov edi,dest

			//ALIGN 4
			intersux:
				mov ecx,RM_SX
				add esi,RM_SKIPX*2
			
			intersux2:
				mov eax,[esi]         ; U 1   0BBBBBGGGGGRRRRR 0BBBBBGGGGGRRRRR
				add esi,4             ; V
				mov ebx,eax           ; U 2   0BBBBBGGGGGRRRRR 0BBBBBGGGGGRRRRR
				mov edx,eax           ; V     0BBBBBGGGGGRRRRR 0BBBBBGGGGGRRRRR
				shl eax,11            ; U 3   RRRRR0BBBBBGGGGG RRRRR00000000000
				and ebx,0x03E003E0    ; V     000000GGGGG00000 000000GGGGG00000
				and edx,0x7C007C00    ; U 4   0BBBBB0000000000 0BBBBB0000000000
				and eax,0xF800F800    ; V     RRRRR00000000000 RRRRR00000000000
				shr edx,10            ; U 5   00000000000BBBBB 00000000000BBBBB
				add ebx,ebx           ; V     00000GGGGG000000 00000GGGGG000000
				or eax,edx            ; U 6   RRRRR000000BBBBB RRRRR000000BBBBB
				add edi,4             ; V
				or eax,ebx            ; U 7   RRRRRGGGGG0BBBBB RRRRRGGGGG0BBBBB
				sub ecx,2             ; V
				mov [edi-4],eax       ; U 8
				jnz intersux2					; V

				add esi,lineinc2
				add edi,lineinc

				dec y
				jnz intersux
			}
		}
	} else {
		// 555 RGB - Computer di Bagio, nVidia Riva 128
		int lineinc = (ddsd.lPitch / 2 - RM_SX) * 2;
		int lineinc2 = (RM_BBX - RM_SKIPX - RM_SX) * 2;
		destw=(uint16 *)dest;

		if (bCfgAnni30) {
			__asm {
				mov y,RM_SY
				mov esi,src
				mov edi,dest
			
			bagioline30:
				mov x,RM_SX
				add esi,RM_SKIPX*2
			
			bagioline302:
				mov eax,[esi]					; U 1
				mov ebx,[esi+4]				; V

				// Codice B&W - Pairato
				mov ecx,eax
				mov edx,ebx
				and eax,0xFFFF
				and ebx,0xFFFF
				shr ecx,16
				shr edx,16
				mov eax,[offset m_wPrecalcTable + eax*2]
				mov ebx,[offset m_wPrecalcTable + ebx*2]
				mov ecx,[offset m_wPrecalcTable + ecx*2]
				mov edx,[offset m_wPrecalcTable + edx*2]
				shl ecx,16
				shl edx,16
				and eax,0xFFFF
				and ebx,0xFFFF
				or eax,ecx
				or ebx,edx

				mov [edi],eax					; U 3
				mov [edi+4],ebx				; V
				add esi,8							; U 4
				add edi,8							; V
				sub x,4								; U 5
				jnz bagioline302					; V

				add esi,lineinc2
				add edi,lineinc

				dec y
				jnz bagioline30
			}
		} else {
			for (y = 0; y < RM_SY; y++) {
				CopyMemory(dest, src + RM_SKIPX, RM_SX * 2);
				dest += ddsd.lPitch;
				src += RM_BBX;
			}
		}
	}


	// Unlock
	Unlock(ddsd);

	// Effetto di wipe
	if (rcBoundEllipse) {
		WipeEffect(*rcBoundEllipse);
	}

	// Repaint
	Repaint();

DOFRAMERATE:

	// Conteggio per gli FPS
	fcount++;

	if (lastsecond + 1000 < _vm->GetTime()) {
		lastsecond = _vm->GetTime();
		fps=fcount - lastfcount;
		lastfcount = fcount;

		if (!m_bFullscreen) {
			s.Format("Tony Tough and the night of Roasted Moths - %u FPS",fps);
			SetWindowText(m_hWnd,s);
		}
	} if (!_vm->getIsDemo()) {
		if (m_bGrabScreenshot || m_bGrabMovie) {
			RMSnapshot snapshot;

			snapshot.GrabScreenshot(lpBuf);
			m_bGrabScreenshot = false;
		}

		if (m_bGrabThumbnail) {
			RMSnapshot snapshot;

			snapshot.GrabScreenshot(lpBuf, 4, m_wThumbBuf);
			m_bGrabThumbnail = false;
		}
	}
#endif
}


/****************************************************************************\
*       Metodi di RMSnapshot
\****************************************************************************/

char RMSnapshot::bufDrive[MAX_DRIVE];
char RMSnapshot::bufDir[MAX_DIR];
char RMSnapshot::bufName[MAX_FNAME];
char RMSnapshot::bufExt[MAX_EXT];
char RMSnapshot::filename[512];
byte RMSnapshot::rgb[RM_SX * RM_SY * 3];

bool RMSnapshot::GetFreeSnapName(char *fn) {
#ifdef REFACTOR_ME
	int i, j, k;
	HANDLE h;

	theGame.GetDataDirectory(RMGame::DD_SHOTS, fn);
	_splitpath(fn, bufDrive, bufDir, NULL, NULL);

	for (i = 1; i < 10; i++) {
		wsprintf(bufName,"rm%d00",i);
		_makepath(fn,bufDrive,bufDir,bufName,".bmp");
		h = CreateFile(fn,GENERIC_READ,FILE_SHARE_READ, NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL, NULL);
		if (h == INVALID_HANDLE_VALUE)
			break;
		CloseHandle(h);
	}

	i--;

	for (j = 1; j < 10; j++) {
		wsprintf(bufName,"rm%d%d0",i,j);
		_makepath(fn,bufDrive,bufDir,bufName,".bmp");
		h=CreateFile(fn,GENERIC_READ,FILE_SHARE_READ, NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL, NULL);
		if (h==INVALID_HANDLE_VALUE)
			break;
		CloseHandle(h);
	}

	j--;

	for (k = 0; k < 10; k++) {
		wsprintf(bufName,"rm%d%d%d",i,j,k);
		_makepath(fn,bufDrive,bufDir,bufName,".bmp");
		h = CreateFile(fn,GENERIC_READ,FILE_SHARE_READ, NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL, NULL);
		if (h==INVALID_HANDLE_VALUE)
			break;
		CloseHandle(h);
	}

	if (k == 10) {
		k = 0;
		j++;
		if (j == 10) {
			j = 0;
			i++;
			if (i == 10)
				return false;
		}

		wsprintf(bufName,"rm%d%d%d",i,j,k);
		_makepath(fn,bufDrive,bufDir,bufName,".bmp");
	}
#endif
	return true;
}

void RMSnapshot::GrabScreenshot(byte *lpBuf, int dezoom, uint16 *lpDestBuf) {
#ifdef REFACTOR_ME
	uint16 *src = (uint16 *)lpBuf;
	
	int dimx = RM_SX / dezoom;
	int dimy = RM_SY / dezoom;

	int u, v, curv;

	uint16	appo;
	uint32	k = 0;
	int sommar, sommab, sommag;
	uint16 *cursrc;
		
	if (lpDestBuf == NULL)
		src += (RM_SY - 1) * RM_BBX;

	if (dezoom == 1 && 0) { // @@@ NON E' TESTATA MOLTO BENE!!!
		byte *curOut = rgb;
		
		for (int y = 0; y < dimy; y++) {
			for (int x = 0; x < dimx; x++) {
				cursrc = &src[RM_SKIPX + x];

				*curOut++ = ((*cursrc) & 0x1F) << 3;
				*curOut++ = (((*cursrc) >> 5) & 0x1F) << 3;
				*curOut++ = (((*cursrc) >> 10) & 0x1F) << 3;

				if (lpDestBuf)
					*lpDestBuf++ = *cursrc;
			}

			if (lpDestBuf == NULL)
				src -= RM_BBX;
			else
				src += RM_BBX;
		}			
	} else {
		for (int y = 0; y < dimy; y++) {
			for(int x = 0; x < dimx; x++) {
				cursrc = &src[RM_SKIPX + x * dezoom];
				sommar = sommab = sommag = 0;
				
				for (v = 0; v < dezoom; v++)
					for (u = 0; u < dezoom; u++) {
						if (lpDestBuf == NULL)
							curv = -v;
						else
							curv = v;
						
						sommab += cursrc[curv*RM_BBX + u] & 0x1F;
						sommag += (cursrc[curv*RM_BBX + u] >> 5) & 0x1F;
						sommar += (cursrc[curv*RM_BBX + u] >> 10) & 0x1F;
					}

					rgb[k + 0] = (byte) (sommab * 8 / (dezoom * dezoom));
					rgb[k + 1] = (byte) (sommag * 8 / (dezoom * dezoom));
					rgb[k + 2] = (byte) (sommar * 8 / (dezoom * dezoom));

					if (lpDestBuf!=NULL)
						lpDestBuf[k/3] = ((int)rgb[k+0]>>3) | (((int)rgb[k+1]>>3)<<5) | (((int)rgb[k+2]>>3)<<10);

					k+=3;
			}

			if (lpDestBuf == NULL)
				src -= RM_BBX * dezoom;
			else
				src += RM_BBX * dezoom;
		}
	}


	if (lpDestBuf == NULL) {
		if (!GetFreeSnapName(filename))
			return;

		HANDLE	hFile = CreateFile(filename,
									GENERIC_WRITE,
									0,
									NULL,
									CREATE_ALWAYS,
									FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
									NULL);

		BITMAPFILEHEADER bmfHeader;
		bmfHeader.bfType = ((uint16) ('M' << 8) | 'B');
		bmfHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dimx * dimy * 3;
		bmfHeader.bfReserved1 = 0;
		bmfHeader.bfReserved2 = 0;
		bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		BITMAPINFOHEADER	bmiHeader;
		bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmiHeader.biWidth = dimx;
		bmiHeader.biHeight = dimy;
		bmiHeader.biPlanes = 1;
		bmiHeader.biBitCount = 24;
		bmiHeader.biCompression = BI_RGB;
		bmiHeader.biSizeImage = dimx * dimy * 3;
		bmiHeader.biXPelsPerMeter = 0xB12;
		bmiHeader.biYPelsPerMeter = 0xB12;
		bmiHeader.biClrUsed = 0;
		bmiHeader.biClrImportant = 0;

		uint32	dwWritten;
		WriteFile(hFile, &bmfHeader, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
		WriteFile(hFile, &bmiHeader, sizeof(BITMAPINFOHEADER), &dwWritten, NULL);

		WriteFile(hFile, rgb, dimx * dimy * 3, &dwWritten, NULL);
		CloseHandle(hFile);
	}
#endif
}

} // End of namespace Tony
