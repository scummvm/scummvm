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
 *                                     様様様様様様様様様様様様様様様様様 *
 *                                             Nayma Software srl         *
 *                    e                -= We create much MORE than ALL =- *
 *        u-        z$$$c        '.    様様様様様様様様様様様様様様様様様 *
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

#include "common/scummsys.h"
#include "util.h"
#include "tony/window.h"
#include "tony/game.h"
#include "tony/tony.h"

namespace Tony {

#define DDRELEASE(x)		if (x) { (x)->Release(); (x)=NULL; }

// Tabella per il b&w. Globale per accederci dalla roba ASM
static uint16 m_wPrecalcTable[0x10000];

/****************************************************************************\
*       RMWindow Methods
\****************************************************************************/

RMWindow::RMWindow() { 

}

RMWindow::~RMWindow() {
	Close();
	RMText::Unload();
}


void RMWindow::Init() {
	initGraphics(RM_SX, RM_SY, false);

	// Inizializza i conteggi degli FPS
	fps = lastfcount = fcount = lastsecond = 0;

	m_bGrabScreenshot = false;
	m_bGrabThumbnail = false;
	m_bGrabMovie = false;
}

void RMWindow::Close(void) {
}

void RMWindow::GrabThumbnail(uint16 *thumbmem) {
	warning("TODO: RMWindow::GrabThumbnail");
}

/**
 * Repaint the screen
 */
void RMWindow::Repaint(void) {
	g_system->updateScreen();
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
