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

#include "common/system.h"
#include "graphics/palette.h"
#include "common/file.h"
#include "common/rect.h"
#include "engines/util.h"
#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"

namespace Hopkins {

GraphicsManager::GraphicsManager() {
	_lockCtr = 0;
	SDL_MODEYES = false;
	SDL_ECHELLE = 0;
	XSCREEN = YSCREEN = 0;
	WinScan = 0;
	Winbpp = 0;
	PAL_PIXELS = NULL;
	nbrligne = 0;
	Linear = false;
	VideoPtr = NULL;
	_vm->_eventsManager.start_x = 0;
	ofscroll = 0;
	SCROLL = 0;
	PCX_L = PCX_H = 0;
	DOUBLE_ECRAN = false;
	OLD_SCROLL = 0;
	MANU_SCROLL = 1;
	SPEED_SCROLL = 16;
	nbrligne2 = 0;
	Agr_x = Agr_y = 0;
	Agr_Flag_x = Agr_Flag_y = 0;
	FADESPD = 15;
	FADE_LINUX = 0;
	NOLOCK = false;
	no_scroll = 0;
	REDRAW = false;
	min_x = 0;
	min_y = 20;
	max_x = SCREEN_WIDTH * 2;
	max_y = SCREEN_HEIGHT - 20 * 2;
	clip_x = clip_y = 0;
	clip_x1 = clip_y1 = 0;
	clip_flag = false;
	SDL_NBLOCS = 0;
	Red_x = Red_y = 0;
	Red = 0;
	Largeur = 0;
	Compteur_y = 0;
	spec_largeur = 0;

	Common::fill(&SD_PIXELS[0], &SD_PIXELS[PALETTE_SIZE * 2], 0);
	Common::fill(&TABLE_COUL[0], &TABLE_COUL[PALETTE_SIZE], 0);
	Common::fill(&cmap[0], &cmap[PALETTE_BLOCK_SIZE], 0);
	Common::fill(&Palette[0], &Palette[PALETTE_BLOCK_SIZE], 0);
}

GraphicsManager::~GraphicsManager() {
	GLOBALS.dos_free2(VESA_SCREEN);
	GLOBALS.dos_free2(VESA_BUFFER);
}

void GraphicsManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
}

void GraphicsManager::SET_MODE(int width, int height) {
	if (!SDL_MODEYES) {
		SDL_ECHELLE = 0;

		if (GLOBALS.XSETMODE == 1)
			SDL_ECHELLE = 0;
		if (GLOBALS.XSETMODE == 2)
			SDL_ECHELLE = 25;
		if (GLOBALS.XSETMODE == 3)
			SDL_ECHELLE = 50;
		if (GLOBALS.XSETMODE == 4)
			SDL_ECHELLE = 75;
		if (GLOBALS.XSETMODE == 5)
			SDL_ECHELLE = GLOBALS.XZOOM;
		
		int bpp = 8;
		if (GLOBALS.XFORCE8 == 1)
			bpp = 8;
		if (GLOBALS.XFORCE16 == 1)
			bpp = 16;

		if (SDL_ECHELLE) {
			width = Reel_Zoom(width, SDL_ECHELLE);
			height = Reel_Zoom(height, SDL_ECHELLE);
		}

		Graphics::PixelFormat pixelFormat16(2, 5, 6, 5, 0, 11, 5, 0, 0);

		if (bpp == 8) {
			initGraphics(width, height, true);
		} else {
			initGraphics(width, height, true, &pixelFormat16);
		}

		// Init surfaces
		VESA_SCREEN = GLOBALS.dos_malloc2(SCREEN_WIDTH * 2 * SCREEN_HEIGHT);
		VESA_BUFFER = GLOBALS.dos_malloc2(SCREEN_WIDTH * 2 * SCREEN_HEIGHT);

		VideoPtr = NULL;
		XSCREEN = width;
		YSCREEN = height;

		Linear = true;
		Winbpp = bpp / 8;
		WinScan = width * Winbpp;

		PAL_PIXELS = SD_PIXELS;
		nbrligne = width;

		Common::fill(&cmap[0], &cmap[256 * 3], 0);
		SDL_MODEYES = true;
	} else {
		error("Called SET_MODE multiple times");
	}
}

void GraphicsManager::DD_Lock() {
	if (!NOLOCK) {
		if (_lockCtr++ == 0)
			VideoPtr = g_system->lockScreen();
	}
}

void GraphicsManager::DD_Unlock() {
	assert(VideoPtr);
	if (--_lockCtr == 0) {
		g_system->unlockScreen();
		VideoPtr = NULL;
	}
}

void GraphicsManager::Cls_Video() {
	assert(VideoPtr);

	VideoPtr->fillRect(Common::Rect(0, 0, XSCREEN, YSCREEN), 0);
}

void GraphicsManager::LOAD_IMAGE(const Common::String &file) {
	Common::String filename	= Common::String::format("%s.PCX", file.c_str());
	CHARGE_ECRAN(filename);
	INIT_TABLE(165, 170, Palette);
}

void GraphicsManager::CHARGE_ECRAN(const Common::String &file) {
	bool flag;
	Common::File f;

	FileManager::DMESS1();

	flag = true;
	if (FileManager::RECHERCHE_CAT(file, 6)) {
		FileManager::CONSTRUIT_FICHIER(GLOBALS.HOPIMAGE, file);
		if (!f.open(GLOBALS.NFICHIER))
			error("CHARGE_ECRAN - %s", file.c_str());

		f.seek(0, SEEK_END);
		f.close();
		flag = false;
	}

	SCROLL_ECRAN(0);
	A_PCX640_480((byte *)VESA_SCREEN, file, Palette, flag);

	SCROLL = 0;
	OLD_SCROLL = 0;
	Cls_Pal();

	if (!DOUBLE_ECRAN) {
		souris_max();
		SCANLINE(SCREEN_WIDTH);
		max_x = SCREEN_WIDTH;
		DD_Lock();
		Cls_Video();
		if (Winbpp == 2) {
			if (SDL_ECHELLE)
				m_scroll16A(VESA_SCREEN, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			else
				m_scroll16(VESA_SCREEN, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		} else if (Winbpp == 1) {
			if (!SDL_ECHELLE)
				m_scroll2(VESA_SCREEN, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			else 
				m_scroll2A(VESA_SCREEN, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		}

		DD_Unlock();
	} else {
		SCANLINE(SCREEN_WIDTH * 2);
		max_x = SCREEN_WIDTH * 2;
		DD_Lock();
		Cls_Video();
		DD_Unlock();

		if (MANU_SCROLL == 1) {
			DD_Lock();
			if (Winbpp == 2) {
				if (SDL_ECHELLE)
					m_scroll16A(VESA_SCREEN, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
				else
					m_scroll16(VESA_SCREEN, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			}
			if (Winbpp == 1) {
				if (!SDL_ECHELLE)
					m_scroll2(VESA_SCREEN, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
				else
					m_scroll2A(VESA_SCREEN, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			}

			DD_Unlock();
		}
	}

	memcpy(VESA_BUFFER, VESA_SCREEN, SCREEN_WIDTH * 2 * SCREEN_HEIGHT);
}

void GraphicsManager::INIT_TABLE(int a1, int a2, byte *palette) {
	for (int idx = 0; idx < 256; ++idx)
		TABLE_COUL[idx] = idx;
  
	Trans_bloc(TABLE_COUL, palette, 256, a1, a2);

	for (int idx = 0; idx < 256; ++idx) {
		byte v = TABLE_COUL[idx];
		if (v > 27)
			TABLE_COUL[idx] = 0;
		if (!v)
			TABLE_COUL[idx] = 0;
	}

	TABLE_COUL[0] = 1;
}

int GraphicsManager::SCROLL_ECRAN(int amount) {
	int result = CLIP(amount, 0, SCREEN_WIDTH);
	_vm->_eventsManager.start_x = result;
	ofscroll = result;
	SCROLL = result;
	return result;
}

void GraphicsManager::Trans_bloc(byte *destP, byte *srcP, int count, int param1, int param2) {
	byte *v5;
	int v6;
	int v7;
	int v8;
	unsigned int v11;
	int v12;
	int v13;
	int v14;
	int v15;
	int v16;
	int v17;
	unsigned int v18;
	char v19;
	int v20;
	bool breakFlag;

	v5 = destP;
	v6 = count - 1;
	do {
		breakFlag = v6;
		v7 = *(byte *)v5++;
		v8 = (unsigned int)(3 * v7);

		// TODO: Ensure this is the right calculation
		v11 = *(byte *)(v8 + srcP) + *(byte *)(v8 + srcP + 1)
				+ *(byte *)(v8 + srcP + 2);

		v12 = 0;
		for (;;) {
			v13 = v12 + 1;
			if ( v13 == 38 )
				break;

			v20 = v13;
			v8 = 3 * v8;
			v14 = *(byte *)(v8 + srcP);
			v15 = v14;
			v14 = *(byte *)(v8 + srcP + 1);
			v16 = v14 + v15;
			v14 = *(byte *)(v8 + srcP + 2);
			v17 = v14 + v16;
			v12 = v20;
			v18 = param1 + v17;
			if (v18 >= v11 && (unsigned int)(v18 - param2) <= v11) {
				v19 = v20;
				if (!v20)
					v19 = 1;
				*(byte *)(v5 - 1) = v19;
				break;
			}
		}

		v6 = breakFlag - 1;
	} while ( !breakFlag);
}

void GraphicsManager::Trans_bloc2(byte *surface, byte *col, int size) {
	byte *dataP; 
	int count; 
	byte dataVal; 

	dataP = surface;
	count = size - 1;
	do {
		dataVal = *dataP++;
		*(dataP - 1) = *(dataVal + col);
		--count;
	} while (count);
}

// TODO: See if it's feasible and/or desirable to change this to use the Common PCX decoder
void GraphicsManager::A_PCX640_480(byte *surface, const Common::String &file, byte *palette, bool typeFlag) {
	int filesize; 
	int v6;
	int v7;
	int v8;
	int v9; 
	int v10;
	int v11;
	byte v12; 
	int v13;
	int v14;
	char v15; 
	int v18;
	int v19; 
	int v20;
	unsigned int v21;
	int v22;
	int32 v23;
	byte *ptr;
	Common::File f;

	// Clear the passed surface
	memset(surface, 0, SCREEN_WIDTH * 2 * SCREEN_HEIGHT);

	if (typeFlag) {
		FileManager::CONSTRUIT_FICHIER(GLOBALS.HOPIMAGE, "PIC.RES");
		if (!f.open(GLOBALS.NFICHIER))
			error("(nom)Erreur en cours de lecture.");
		f.seek(GLOBALS.CAT_POSI);

		v7 = GLOBALS.CAT_TAILLE - 896;
		v8 = f.read(HEADER_PCX, 128);

		v6 = READ_LE_UINT16(&HEADER_PCX[8]) + 1;
		v20 = READ_LE_UINT16(&HEADER_PCX[10]) + 1;
		if ((READ_LE_UINT16(&HEADER_PCX[8]) + 1) <= SCREEN_WIDTH) {
			DOUBLE_ECRAN = false;
		} else {
			v6 = SCREEN_WIDTH * 2;
			DOUBLE_ECRAN = true;
		}
		if (v20 > SCREEN_HEIGHT)
			v20 = SCREEN_HEIGHT;
		PCX_L = v6;
		PCX_H = v20;
		if (v8 == -1)
		  error("Erreur en cours de lecture.");
	} else {
		FileManager::CONSTRUIT_FICHIER(GLOBALS.HOPIMAGE, file);
		if (!f.open(GLOBALS.NFICHIER))
		  error("(nom)Erreur en cours de lecture.");

		filesize = f.size();
		int bytesRead = f.read(HEADER_PCX, 128);
		if (bytesRead < 128)
			error("Erreur en cours de lecture.");

		v6 = READ_LE_UINT16(&HEADER_PCX[8]) + 1;
		v20 = READ_LE_UINT16(&HEADER_PCX[10]) + 1;
		if (v6 <= SCREEN_WIDTH) {
			DOUBLE_ECRAN = false;
		} else {
			v6 = SCREEN_WIDTH * 2;
			DOUBLE_ECRAN = true;
		}
		if (v20 > SCREEN_HEIGHT)
			v20 = SCREEN_HEIGHT;
		PCX_L = v6;
		PCX_H = v20;
		v7 = filesize - 896;
	}

	ptr = GLOBALS.dos_malloc2(0xEE60u);
	if (v7 >= 60000) {
		v21 = v7 / 60000 + 1;
		v23 = 60000 * (v7 / 60000) - v7;
    
		if (((uint32)v23 & 0x80000000u) != 0)
			v23 = -v23;
		f.read(ptr, 60000);
		v7 = 60000;
	} else {
		v21 = 1;
		v23 = v7;
		f.read(ptr, v7);
	}
	v22 = v21 - 1;
	v18 = 0;
	v9 = 0;
	v10 = 0;
	v19 = v6;
  
	do {
		if (v9 == v7) {
			v9 = 0;
			--v22;
			v7 = 60000;
			if ( !v22 )
				v7 = v23;
			v11 = v10;
			f.read(ptr, v7);
			v10 = v11;
		}

		v12 = *((byte *)ptr + v9++);
		if (v12 > 0xC0u) {
			v13 = v12 - 192;
			if (v9 == v7) {
				v9 = 0;
				--v22;
				v7 = 60000;
				if ( v22 == 1 )
					v7 = v23;
				v14 = v10;
				f.read(ptr, v7);
				v10 = v14;
			}
			v15 = *((byte *)ptr + v9++);

			do {
				*((byte *)surface + v10++) = v15;
				++v18;
				--v13;
			} while (v13);
		} else {
			*((byte *)surface + v10++) = v12;
			++v18;
		}
	} while (v18 < v19 * v20);

	if (typeFlag) {
		f.seek(GLOBALS.CAT_TAILLE + GLOBALS.CAT_POSI - 768);
	} else {
		filesize = f.size();
		f.seek(filesize - 768);
	}

	if (f.read(palette, PALETTE_BLOCK_SIZE) != (PALETTE_BLOCK_SIZE))
		error("A_PCX640_480");
  
	f.close();
	GLOBALS.dos_free2(ptr);
}

void GraphicsManager::Cls_Pal() {
	Common::fill(&cmap[0], &cmap[PALETTE_BLOCK_SIZE], 0);
	SD_PIXELS[0] = 0;

	if (Winbpp == 1) {
		g_system->getPaletteManager()->setPalette(cmap, 0, 256);
	}
}

void GraphicsManager::souris_max() {
	// Original has no implementation
}

void GraphicsManager::SCANLINE(int pitch) {
	nbrligne = nbrligne2 = pitch;
}

void GraphicsManager::m_scroll(const byte *surface, int xs, int ys, int width, int height, int destX, int destY) {
	const byte *v7;
	byte *v8;
	int v9;
	int v10;
	byte *v11;
	const byte *v12;
	unsigned int v13;

	assert(VideoPtr);
	v7 = xs + nbrligne2 * ys + surface;
	v8 = destX + WinScan * destY + (byte *)VideoPtr->pixels;
	v9 = height;
	do {
		v10 = v9;
		memcpy((byte *)v8, (const byte *)v7, 4 * (width >> 2));
		v12 = (const byte *)(v7 + 4 * (width >> 2));
		v11 = (byte *)(v8 + 4 * (width >> 2));
		v13 = width - 4 * (width >> 2);
		memcpy(v11, v12, v13);
		v8 = v11 + v13 + WinScan - width;
		v7 = v12 + v13 + nbrligne2 - width;
		v9 = v10 - 1;
	} while (v10 != 1);
}

void GraphicsManager::m_scroll2(const byte *surface, int xs, int ys, int width, int height, int destX, int destY) {
	const byte *v7;
	byte *v8;
	int v9;
	int v10;
	int v11;

	assert(VideoPtr);
	v7 = xs + nbrligne2 * ys + surface;
	v8 = destX + WinScan * destY + (byte *)VideoPtr->pixels;
	v9 = WinScan - SCREEN_WIDTH;
	v10 = nbrligne2 - SCREEN_WIDTH;
	v11 = height;

	do {
		memcpy(v8, v7, SCREEN_WIDTH);
		v8 = v8 + v9 + SCREEN_WIDTH;
		v7 = v7 + v10 + SCREEN_WIDTH;
		--v11;
	} while (v11);
}

void GraphicsManager::m_scroll2A(const byte *surface, int xs, int ys, int width, int height, int destX, int destY) {
	const byte *v7;
	byte *v8;
	int v9;
	int v10;
	byte v11;
	const byte *v12;
	byte *v13;

	assert(VideoPtr);
	v7 = xs + nbrligne2 * ys + surface;
	v8 = destX + WinScan * destY + (byte *)VideoPtr->pixels;
	v9 = height;
	Agr_x = 0;
	Agr_y = 0;
	Agr_Flag_y = 0;
	do {
		for (;;) {
			v13 = v8;
			v12 = v7;
			v10 = width;
			Agr_x = 0;
			do {
				v11 = *v7;
				*v8++ = *v7++;
				Agr_x += SDL_ECHELLE;
				if ((unsigned int)Agr_x >= 100) {
					Agr_x -= 100;
					*v8++ = v11;
				}
				--v10;
			} while ( v10 );
	      
			v7 = v12;
			v8 = WinScan + v13;
			if (Agr_Flag_y)
				break;

			Agr_y += SDL_ECHELLE;
			if ((unsigned int)Agr_y < 100)
				break;

			Agr_y -= 100;
			Agr_Flag_y = 1;
		}

		Agr_Flag_y = 0;
		v7 = nbrligne2 + v12;
		--v9;
	} while (v9);
}

/**
 * Copies data from a 8-bit palette surface into the 16-bit screen
 */
void GraphicsManager::m_scroll16(const byte *surface, int xs, int ys, int width, int height, int destX, int destY) {
	DD_Lock();

	assert(VideoPtr);
	const byte *srcP = xs + nbrligne2 * ys + surface;
	uint16 *destP = (uint16 *)((byte *)VideoPtr->pixels + destX * 2 + WinScan * destY);

	for (int yp = 0; yp < height; ++yp) {
		// Copy over the line, using the source pixels as lookups into the pixels palette
		const byte *lineSrcP = srcP;
		uint16 *lineDestP = destP;

		for (int xp = 0; xp < width; ++xp)
			*lineDestP++ = *(uint16 *)&PAL_PIXELS[*lineSrcP++ * 2];

		// Move to the start of the next line
		srcP += nbrligne2;
		destP += WinScan / 2;
	}

	DD_Unlock();
}

void GraphicsManager::m_scroll16A(const byte *surface, int xs, int ys, int width, int height, int destX, int destY) {
	const byte *v7;
	const byte *v8;
	int v9;
	int v10;
	const byte *v11;
	int v12;
	int v13;
	const byte *v14;
	const byte *v15;

	assert(VideoPtr);
	v7 = xs + nbrligne2 * ys + surface;
	v8 = destX + destX + WinScan * destY + (byte *)VideoPtr->pixels;
	v9 = height;
	Agr_x = 0;
	Agr_y = 0;
	Agr_Flag_y = 0;

	do {
		for (;;) {
			v15 = v8;
			v14 = v7;
			v10 = width;
			v13 = v9;
			v11 = PAL_PIXELS;
			Agr_x = 0;

			do {
				v12 = *(uint16 *)(v11 + 2 * *v7);
				*(uint16 *)v8 = v12;
				++v7;
				v8 += 2;
				Agr_x += SDL_ECHELLE;
				if ((unsigned int)Agr_x >= 100) {
					Agr_x -= 100;
					*(uint16 *)v8 = v12;
					v8 += 2;
				}
        
				--v10;
			} while (v10);

			v9 = v13;
			v7 = v14;
			v8 = WinScan + v15;
			if (Agr_Flag_y == 1)
				break;
			
			Agr_y += SDL_ECHELLE;
			
			if ((unsigned int)Agr_y < 100)
				break;
      
			Agr_y -= 100;
			Agr_Flag_y = 1;
		}

		Agr_Flag_y = 0;
		v7 = nbrligne2 + v14;
		v9 = v13 - 1;
	} while (v13 != 1);
}

void GraphicsManager::Copy_Vga(const byte *surface, int xp, int yp, int width, int height, int destX, int destY) {
	const byte *v7;
	byte *v8;
	int v9;
	int v10;
	byte v11;
	byte *v12; 
	byte *v13; 
	byte *v14; 
	byte *v15; 
	const byte *v16; 
	int v17; 

	assert(VideoPtr);
	v7 = xp + 320 * yp + surface;
	v8 = 30 * WinScan + destX + destX + WinScan * 2 * destY + (byte *)VideoPtr->pixels;
	v9 = height;
  
	do {
		v17 = v9;
		v10 = width;
		v16 = v7;
		v15 = v8;
		do {
			v11 = *v7;
			*v8 = *v7;
			v12 = WinScan + v8;
			*v12 = v11;
			v13 = v12 - WinScan + 1;
			*v13 = v11;
			v14 = WinScan + v13;
			*v14 = v11;
			++v7;
			v8 = v14 - WinScan + 1;
			--v10;
		} while (v10);
    
		v8 = WinScan + WinScan + v15;
		v7 = v16 + 320;
		v9 = v17 - 1;
	} while (v17 != 1);
}

void GraphicsManager::Copy_Vga16(const byte *surface, int xp, int yp, int width, int height, int destX, int destY) {
	const byte *v7; 
	uint16 *v8; 
	int v9; 
	int v10;
	int v11;
	const byte *v12;
	uint16 *v13;
	uint16 v14;
	uint16 *v15;
	int v16;
	uint16 *v17;
	const byte *v18;
	int v19;

	assert(VideoPtr);
	v7 = xp + 320 * yp + surface;
	v8 = (uint16 *)(30 * WinScan + destX + destX + destX + destX + WinScan * 2 * destY + (byte *)VideoPtr->pixels);
	v9 = height;
	v10 = width;
  
	do {
		v19 = v9;
		v11 = v10;
		v18 = v7;
		v17 = v8;
		v16 = v10;
		v12 = PAL_PIXELS;
		
		do {
			v13 = (uint16 *)(v12 + 2 * *v7);
			v14 = *v13;
			*v8 = *v13;
			*(v8 + 1) = v14;
			
			v15 = (uint16 *)((byte *)v8 + WinScan);
			*v15 = v14;
			*(v15 + 1) = v14;
			++v7;
			v8 = (uint16 *)((byte *)v15 - WinScan + 4);
			--v11;
		} while (v11);
    
		v10 = v16;
		v8 = (uint16 *)((byte *)v17 + WinScan * 2);
		v7 = v18 + 320;
		v9 = v19 - 1;
	} while (v19 != 1);
}

void GraphicsManager::fade_in(const byte *palette, int step, const byte *surface) {
	uint16 palData1[PALETTE_BLOCK_SIZE * 2];
	byte palData2[PALETTE_BLOCK_SIZE];

	// Initialise temporary palettes
	Common::fill(&palData1[0], &palData1[PALETTE_BLOCK_SIZE], 0);
	Common::fill(&palData2[0], &palData2[PALETTE_BLOCK_SIZE], 0);

	// Set current palette to black
	setpal_vga256(palData2);
  
	// Loop through fading in the palette
	uint16 *pTemp1 = &palData1[2];
	for (int fadeIndex = 0; fadeIndex < FADESPD; ++fadeIndex) {
		uint16 *pTemp2 = &palData1[4];

		for (int palOffset = 0; palOffset < PALETTE_BLOCK_SIZE; palOffset += 3) {
			if (palData2[palOffset] < palette[palOffset]) {
				uint16 v = (palette[palOffset] & 0xff) * 256 / FADESPD;
				palData1[palOffset] = v;
				palData2[palOffset] = (v >> 8) & 0xff;
			}

			if (palData2[palOffset + 1] < palette[palOffset + 1]) {
				uint16 *pDest = &pTemp1[palOffset];
				uint16 v = (palette[palOffset] & 0xff) * 256 / FADESPD + *pDest; 
				*pDest = v;
				palData2[palOffset + 1] = (v >> 8) & 0xff;
			}

			if (palData2[palOffset + 1] < palette[palOffset + 1]) {
				uint16 *pDest = &pTemp2[palOffset];
				uint16 v = (palette[palOffset] & 0xff) * 256 / FADESPD + *pDest; 
				*pDest = v;
				palData2[palOffset + 1] = (v >> 8) & 0xff;
			}
		}

		setpal_vga256(palData2);
		if (Winbpp == 2) {
			if (SDL_ECHELLE)
				m_scroll16A(surface, _vm->_eventsManager.start_x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			else
				m_scroll16(surface, _vm->_eventsManager.start_x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			DD_VBL();
		}
	}

	// Set the final palette
	setpal_vga256(palette);

	// Refresh the screen
	if (Winbpp == 2) {
		if (SDL_ECHELLE)
			m_scroll16A(surface, _vm->_eventsManager.start_x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		else
			m_scroll16(surface, _vm->_eventsManager.start_x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		DD_VBL();
	}
}

void GraphicsManager::fade_out(const byte *palette, int step, const byte *surface) {
	int v3;
	int v4;
	int v5; 
	int v6;
	int v7;
	int v8;
	int v9;
	int v10;
	int v12;
	uint16 v13;
	byte palData[PALETTE_BLOCK_SIZE];
	int v15[PALETTE_BLOCK_SIZE];

	v13 = v3 = FADESPD;
	if (palette) {
		v4 = 0;
		do {
			v5 = v4;
			v3 = *(v4 + palette);
			v3 <<= 8;
			v15[v5] = v3;
			palData[v5] = *(v4++ + palette);
		} while (v4 < PALETTE_BLOCK_SIZE);
    
		setpal_vga256(palData);
		if (Winbpp == 2) {
			if (SDL_ECHELLE)
				m_scroll16A(surface, _vm->_eventsManager.start_x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			else
				m_scroll16(surface, _vm->_eventsManager.start_x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			DD_VBL();
		}
	
		v6 = 0;
		if ((int)v13 > 0) {
			do {
				v7 = 0;
				do {
					v8 = v7;
					v9 = v15[v7] - (*(v7 + palette) << 8) / v13;
					v15[v8] = v9;
					palData[v8] = (v9 >> 8) & 0xff;
					++v7;
				} while (v7 < (PALETTE_BLOCK_SIZE));
				
				setpal_vga256(palData);
				if (Winbpp == 2) {
					if (SDL_ECHELLE)
						m_scroll16A(surface, _vm->_eventsManager.start_x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
					else
						m_scroll16(surface, _vm->_eventsManager.start_x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			
					DD_VBL();
				}
				++v6;
			} while ((int)v13 > v6);
		}

		v10 = 0;
		do {
			palData[v10++] = 0;
		} while (v10 < (PALETTE_BLOCK_SIZE));

		setpal_vga256(palData);
    
		if (Winbpp == 2) {
			if (!SDL_ECHELLE) {
				m_scroll16(surface, _vm->_eventsManager.start_x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
				return DD_VBL();
			}
			goto LABEL_28;
		}
	} else {
		v12 = 0;
		do {
			palData[v12++] = 0;
		} while (v12 < (PALETTE_BLOCK_SIZE));

		setpal_vga256(palData);
		if (Winbpp == 2) {
			if (!SDL_ECHELLE) {
				m_scroll16(surface, _vm->_eventsManager.start_x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
				return DD_VBL();
			}

LABEL_28:
			m_scroll16A(surface, _vm->_eventsManager.start_x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			return DD_VBL();
		}
	}
}

void GraphicsManager::FADE_INS() {
	FADESPD = 1;
	fade_in(Palette, 1, (const byte *)VESA_BUFFER);
}

void GraphicsManager::FADE_OUTS() {
  FADESPD = 1;
  fade_out(Palette, 1, (const byte *)VESA_BUFFER);
}

void GraphicsManager::FADE_INW() {
	FADESPD = 15;
	fade_in(Palette, 20, (const byte *)VESA_BUFFER);
}

void GraphicsManager::FADE_OUTW() {
	FADESPD = 15;
	fade_out(Palette, 20, (const byte *)VESA_BUFFER);
}

void GraphicsManager::setpal_vga256(const byte *palette) {
	CHANGE_PALETTE(palette);
}

void GraphicsManager::setpal_vga256_linux(const byte *palette, const byte *surface) {
	CHANGE_PALETTE(palette);
  
	if (Winbpp == 2) {
		if (SDL_ECHELLE)
			m_scroll16A(surface, _vm->_eventsManager.start_x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		else
			m_scroll16(surface, _vm->_eventsManager.start_x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		
		DD_VBL();
	}
}

void GraphicsManager::SETCOLOR(int palIndex, int r, int g, int b) {
	_vm->_graphicsManager.Palette[palIndex * 3] = 255 * r / 100;
	_vm->_graphicsManager.Palette[palIndex * 3 + 1] = 255 * g / 100;
	_vm->_graphicsManager.Palette[palIndex * 3 + 2] = 255 * b / 100;
	
	setpal_vga256(Palette);
}

void GraphicsManager::SETCOLOR2(int palIndex, int r, int g, int b) {
	return SETCOLOR(palIndex, r, g, b);
}

void GraphicsManager::SETCOLOR3(int palIndex, int r, int g, int b) {
	Palette[palIndex * 3] = 255 * r / 100;
	Palette[palIndex * 3 + 1] = 255 * g / 100;
	Palette[palIndex * 3 + 2] = 255 * b / 100;
}

void GraphicsManager::SETCOLOR4(int palIndex, int r, int g, int b) {
	int rv, gv, bv;
	int palOffset; 
	int v8; 

	rv = 255 * r / 100;
	gv = 255 * g / 100;
	bv = 255 * b / 100;
	palOffset = 3 * palIndex;
	Palette[palOffset] = 255 * r / 100;
	Palette[palOffset + 1] = gv;
	Palette[palOffset + 2] = bv;

	v8 = 4 * palIndex;
	cmap[v8] = rv;
	cmap[v8 + 1] = gv;
	cmap[v8 + 2] = bv;
	
	WRITE_LE_UINT16(&SD_PIXELS[2 * palIndex], MapRGB(rv, gv, bv));
	g_system->getPaletteManager()->setPalette(cmap, palIndex, 1);
}

void GraphicsManager::CHANGE_PALETTE(const byte *palette) {
	// Copy the palette into the PALPCX block
// TODO: Likely either one or both of the destination arrays can be removed,
// since PALPCX is only used in SAVE_IMAGE, and cmap in the original was a RGBA
// array specifically intended just for passing to the SDL palette setter
	Common::copy(&palette[0], &palette[PALETTE_BLOCK_SIZE], &PALPCX[0]);
	Common::copy(&palette[0], &palette[PALETTE_BLOCK_SIZE], &cmap[0]);

	const byte *srcP = &palette[0];
	for (int idx = 0; idx < PALETTE_SIZE; ++idx, srcP += 3) {
		*(uint16 *)&SD_PIXELS[2 * idx] = MapRGB(*srcP, *(srcP + 1), *(srcP + 2));
	}

	if (Winbpp == 1)
		g_system->getPaletteManager()->setPalette(cmap, 0, PALETTE_SIZE);
}

uint16 GraphicsManager::MapRGB(byte r, byte g, byte b) {
	if (Winbpp == 1) {
		error("TODO: Support in 8-bit graphics mode");
	} else {
		Graphics::PixelFormat format = g_system->getScreenFormat();

		return (r >> format.rLoss) << format.rShift
				| (g >> format.gLoss) << format.gShift
				| (b >> format.bLoss) << format.bShift;
	}
}

void GraphicsManager::DD_VBL() {
	// TODO: Is this okay here?
	g_system->updateScreen();
}

void GraphicsManager::FADE_OUTW_LINUX(const byte *surface) {
	fade_out(Palette, FADESPD, surface);
}

void GraphicsManager::FADE_INW_LINUX(const byte *surface) {
	return fade_in(Palette, FADESPD, surface);
}

void GraphicsManager::Copy_WinScan_Vbe3(const byte *sourceSurface, byte *destSurface) {
	int result;
	int v3;
	int v4;
	const byte *v5;
	uint8 v6;
	int v7;
	unsigned int v8;
	byte *v9;
	int v10; 
	unsigned int v11;
	byte *v12;

	result = 0;
	v3 = 0;
	v4 = 0;
	v5 = sourceSurface;
	for (;;) {
		v6 = *v5;
		if (*v5 < 222)
			goto Video_Cont3_wVbe;
		
		if (v6 == 252)
			return;
    
		if (v6 < 251) {
			v3 += *v5 + 35;
			v6 = *(v5++ + 1);
		} else if (v6 == 253) {
			v3 += *(v5 + 1);
			v6 = *(v5 + 2);
			v5 += 2;
		} else if (v6 == 254) {
			v3 += READ_LE_UINT16(v5 + 1);
			v6 = *(v5 + 3);
			v5 += 3;
		} else {
			v3 += READ_LE_UINT32(v5 + 1);
			v6 = *(v5 + 5);
			v5 += 5;
		}

Video_Cont3_wVbe:
		if (v6 > 210) {
			if (v6 == 211) {
				v7 = v4;
				v8 = *(v5 + 1);
				result = *(v5 + 2);
				v9 = v3 + destSurface;
				v3 += v8;
				memset(v9, result, v8);
				v5 += 3;
				v4 = v7;
			} else {
				v10 = v4;
				v11 = *v5 + 45;
				result = *(v5 + 1);
				v12 = v3 + destSurface;
				v3 += v11;
				memset(v12, result, v11);
				v5 += 2;
				v4 = v10;
			}
		} else {
			*(v3 + destSurface) = v6;
			++v5;
			++v3;
		}
	}
}

void GraphicsManager::Copy_Video_Vbe3(const byte *surface) {
	int result;
	int v2;
	int v3;
	const byte *v4;
	uint8 v5;
	int v6;
	unsigned int v7;
	byte *v8;
	int v9;
	unsigned int v10;
	byte *v11;

	assert(VideoPtr);
	result = 0;
	v2 = 0;
	v3 = 0;
	v4 = surface;
	for (;;) {
		v5 = *v4;
		if (*v4 < 222)
			goto Video_Cont3_Vbe;
    
		if (v5 == 252)
			return;
		if (v5 < 251) {
			v2 += *v4 + 35;
			v5 = *(v4++ + 1);
		} else if (v5 == (byte)-3) {
			v2 += *(v4 + 1);
			v5 = *(v4 + 2);
			v4 += 2;
		} else if (v5 == (byte)-2) {
			v2 += READ_LE_UINT16(v4 + 1);
			v5 = *(v4 + 3);
			v4 += 3;
		} else {
			v2 += READ_LE_UINT32(v4 + 1);
			v5 = *(v4 + 5);
			v4 += 5;
		}
Video_Cont3_Vbe:
		if (v5 > 210) {
			if (v5 == 211) {
				v6 = v3;
				v7 = *(v4 + 1);
				result = *(v4 + 2);
				v8 = v2 + (byte *)VideoPtr->pixels;
				v2 += v7;
				memset(v8, result, v7);
				v4 += 3;
				v3 = v6;
			} else {
				v9 = v3;
				v10 = (byte)(*v4 + 45);
				result = *(v4 + 1);
				v11 = (byte *)(v2 + (byte *)VideoPtr->pixels);
				v2 += v10;
				memset(v11, result, v10);
				v4 += 2;
				v3 = v9;
			}
		} else {
			*(v2 + (byte *)VideoPtr->pixels) = v5;
			++v4;
			++v2;
		}
	}
}

void GraphicsManager::Copy_Video_Vbe16(const byte *surface) {
	const byte *srcP = surface;
	int destOffset = 0;
	assert(VideoPtr);

	for (;;) {
		byte srcByte = *srcP;
		if (srcByte >= 222) {
			if (srcByte == 252)
				return;
			if (srcByte < 251) {
				destOffset += srcByte - 221;
				srcByte = *++srcP;
			} else if (srcByte == 253) {
				destOffset += *(const byte *)(srcP + 1);
				srcByte = *(const byte *)(srcP + 2);
				srcP += 2;
			} else if (srcByte == 254) {
				destOffset += READ_LE_UINT16(srcP + 1);
				srcByte = *(const byte *)(srcP + 3);
				srcP += 3;
			} else {
				destOffset += READ_LE_UINT32(srcP + 1);
				srcByte = *(const byte *)(srcP + 5);
				srcP += 5;
			}
		}

		if (srcByte > 210) {
			if (srcByte == 211) {
				int pixelCount = *(srcP + 1);
				int pixelIndex = *(srcP + 2);
				uint16 *destP = (uint16 *)((byte *)VideoPtr->pixels + destOffset * 2);
				uint16 pixelValue = *(uint16 *)(PAL_PIXELS + 2 * pixelIndex);
				destOffset += pixelCount;

				while (pixelCount--)
					*destP++ = pixelValue;

				srcP += 3;
			} else {
				int pixelCount = srcByte - 211;
				int pixelIndex = *(srcP + 1);
				uint16 *destP = (uint16 *)((byte *)VideoPtr->pixels + destOffset * 2);
				uint16 pixelValue = *(uint16 *)(PAL_PIXELS + 2 * pixelIndex);
				destOffset += pixelCount;

				while (pixelCount--)
					*destP++ = pixelValue;

				srcP += 2;
			}
		} else {
			*((uint16 *)VideoPtr->pixels + destOffset) = *(uint16 *)(PAL_PIXELS + 2 * srcByte);
			++srcP;
			++destOffset;
		}
	}
}

void GraphicsManager::Capture_Mem(const byte *srcSurface, byte *destSurface, int xs, int ys, unsigned int width, int height) {
	const byte *srcP;
	byte *destP;
	int yCtr; 
	unsigned int i;
	int yTemp; 

	srcP = srcSurface + xs + nbrligne2 * ys;
	destP = destSurface;

	yCtr = height;
	do {
		yTemp = yCtr;
		if (width & 1) {
			memcpy(destP, srcP, width);
			srcP += width;
			destP += width;
		} else if (width & 2) {
			for (i = width >> 1; i; --i) {
				*(uint16 *)destP = *(uint16 *)srcP;
				srcP += 2;
				destP = (byte *)destP + 2;
			}
		} else {
			memcpy(destP, srcP, 4 * (width >> 2));
			srcP += 4 * (width >> 2);
			destP = (byte *)destP + 4 * (width >> 2);
		}
		srcP = nbrligne2 + srcP - width;
		yCtr = yTemp - 1;
	} while (yTemp != 1);
}

void GraphicsManager::Sprite_Vesa(byte *surface, const byte *spriteData, int xp, int yp, int spriteIndex) {
	// Get a pointer to the start of the desired sprite
	const byte *spriteP = spriteData + 3;
	for (int i = spriteIndex; i; --i)
		spriteP += READ_LE_UINT32(spriteP) + 16;

	clip_x = 0;
	clip_y = 0;
	clip_flag = false;

	spriteP += 4;
	int width = READ_LE_UINT16(spriteP);
	spriteP += 2;
	int height = READ_LE_UINT16(spriteP);
	
	// Clip X
	clip_x1 = width;
	if ((xp + width) <= (min_x + 300))
		return;
	if (xp < (min_x + 300)) {
		clip_x = min_x + 300 - xp;
		clip_flag = true;
	}

	// Clip Y
	// TODO: This is weird, but it's that way in the original. Original game bug?
	if ((yp + height) <= height)
		return;
	if (yp < (min_y + 300)) {
		clip_y = min_y + 300 - yp;
		clip_flag = true;
	}

	// Clip X1
	if (xp >= (max_x + 300))
		return;
	if ((xp + width) > (max_x + 300)) {
		int xAmount = width + 10 - (xp + width - (max_x + 300));
		if (xAmount <= 10)
			return;

		clip_x1 = xAmount - 10;
		clip_flag = true;
	}

	// Clip Y1
	if (yp >= (max_y + 300))
		return;
	if ((yp + height) > (max_y + 300)) {
		int yAmount = height + 10 - (yp + height - (max_y + 300));
		if (yAmount <= 10)
			return;

		clip_y1 = yAmount - 10;
		clip_flag = true;
	}

	// Sprite display

	// Set up source
	spriteP += 6;
	int srcOffset = READ_LE_UINT16(spriteP);
	spriteP += 4;
	const byte *srcP = spriteP;
	spriteP += srcOffset;

	// Set up surface destination
	byte *destP = surface + (yp - 300) * nbrligne2 + (xp - 300);
	
	// Handling for clipped versus non-clipped
	if (clip_flag) {
		// Clipped version
		for (int yc = 0; yc < height; ++yc, destP += nbrligne2) {
			byte *tempDestP = destP;
			byte byteVal;
			int xc = 0;

			while ((byteVal = *srcP) != 253) {
				++srcP;
				width = READ_LE_UINT16(srcP);
				srcP += 2;

				if (byteVal == 254) {
					// Copy pixel range
					for (int xv = 0; xv < width; ++xv, ++xc, ++spriteP, ++tempDestP) {
						if (clip_y == 0 && xc >= clip_x && xc < clip_x1)
							*tempDestP = *spriteP;
					}
				} else {
					// Skip over bytes
					tempDestP += width;
					xc += width;
				}
			}

			if (clip_y > 0)
				--clip_y;
			srcP += 3;
		}
	} else {
		// Non-clipped
		for (int yc = 0; yc < height; ++yc, destP += nbrligne2) {
			byte *tempDestP = destP;
			byte byteVal;

			while ((byteVal = *srcP) != 253) {
				++srcP;
				width = READ_LE_UINT16(srcP);
				srcP += 2;

				if (byteVal == 254) {
					// Copy pixel range
					Common::copy(spriteP, spriteP + width, tempDestP);
					spriteP += width;
				}
				
				tempDestP += width;
			}

			// Skip over control byte and width
			srcP += 3;
		}
	}
}

void GraphicsManager::FIN_VISU() {
	for (int idx = 1; idx <= 20; ++idx) {
		if (_vm->_globals.Bqe_Anim[idx].field4 == 1)
			_vm->_objectsManager.BOB_OFF(idx);
	}

	_vm->_eventsManager.VBL();
	_vm->_eventsManager.VBL();

	for (int idx = 1; idx <= 20; ++idx) {
		if (_vm->_globals.Bqe_Anim[idx].field4 == 1)
			_vm->_objectsManager.BOB_ZERO(idx);
	}

	for (int idx = 1; idx <= 29; ++idx) {
		_vm->_globals.BL_ANIM[idx].v1 = 0;
	}

	for (int idx = 1; idx <= 20; ++idx) {
		_vm->_globals.Bqe_Anim[idx].field4 = 0;
	}
}

void GraphicsManager::VISU_ALL() {
	for (int idx = 1; idx <= 20; ++idx) {
		if (_vm->_globals.Bqe_Anim[idx].field4 == 1)
			_vm->_objectsManager.BOB_VISU(idx);
	}
}

void GraphicsManager::RESET_SEGMENT_VESA() {
	int v1;
	int v2;

	if (_vm->_globals.NBBLOC > 0) {
		v1 = 0;
		v2 = _vm->_globals.NBBLOC;
		do {
			_vm->_globals.BLOC[v1++].field0 = 0;
		} while (v1 != v2);
    
		_vm->_globals.NBBLOC = 0;
	}
}

void GraphicsManager::Ajoute_Segment_Vesa(int x1, int y1, int x2, int y2) {
	int v4;
	int v5;
	int16 v6;
	int16 v10;

	v4 = x1;
	v10 = 1;
	if (x2 > max_x)
		x2 = max_x;
	if (y2 > max_y)
		y2 = max_y;
	if (x1 < min_x)
		v4 = min_x;
	if (y1 < min_y)
		y1 = min_y;

	v5 = _vm->_globals.NBBLOC;
	if (_vm->_globals.NBBLOC > 1) {

		v6 = 0;
		do {
			BlocItem &bloc = _vm->_globals.BLOC[v6];

			if (bloc.field0 == 1 
					&& v4 >= bloc.x1 && x2 <= bloc.x2
					&& y1 >= bloc.y1 && y2 <= bloc.y2)
				v10 = 0;
			++v6;
			v5 = v6;
		} while (_vm->_globals.NBBLOC + 1 != v6);
	}
	
	if (v10 == 1) {
		BlocItem &bloc = _vm->_globals.BLOC[++_vm->_globals.NBBLOC];

		bloc.field0 = 1;
		bloc.x1 = v4;
		bloc.x2 = x2;
		bloc.y1 = y1;
		bloc.y2 = y2;
	}
}

int GraphicsManager::Magic_Number(signed int v) {
	int result = v;

	if (!v)
		result = 4;
	if (result & 1)
		++result;
	if (result & 2)
		result += 2;
  
	return result;
}

void GraphicsManager::Affiche_Segment_Vesa() {
	if (_vm->_globals.NBBLOC == 0)
		return;

	SDL_NBLOCS = _vm->_globals.NBBLOC;

	for (int idx = 1; idx <= _vm->_globals.NBBLOC; ++idx) {
		BlocItem &bloc = _vm->_globals.BLOC[idx];
		Common::Rect &dstRect = dstrect[idx - 1];
		if (bloc.field0 != 1)
			continue;
	
		if (_vm->_eventsManager.CASSE != 0) {
			if (Winbpp == 1) {
				Copy_Vga(VESA_BUFFER, bloc.x1, bloc.y1, bloc.x2 - bloc.x1, bloc.y2 - bloc.y1, bloc.x1, bloc.y1);
			} else if (Winbpp == 2) {
				Copy_Vga16(VESA_BUFFER, bloc.x1, bloc.y1, bloc.x2 - bloc.x1, bloc.y2 - bloc.y1, bloc.x1, bloc.y1);
			}
			
			dstRect.left = bloc.x1 * 2;
			dstRect.top = bloc.y1 * 2 + 30;
			dstRect.setWidth((bloc.x2 - bloc.x1) * 2);
			dstRect.setHeight((bloc.y2 - bloc.y1) * 2);
		} else if (bloc.x2 > _vm->_eventsManager.start_x && bloc.x1 < (_vm->_eventsManager.start_x + SCREEN_WIDTH)) {
			if (bloc.x1 < _vm->_eventsManager.start_x)
				bloc.x1 = _vm->_eventsManager.start_x;
			if (bloc.x2 > (_vm->_eventsManager.start_x + SCREEN_WIDTH))
				bloc.x2 = _vm->_eventsManager.start_x + SCREEN_WIDTH;
			
			if (!SDL_ECHELLE) {
				// Calculate the bounds
				int xp = Magic_Number(bloc.x1) - 4;
				if (xp < _vm->_eventsManager.start_x)
					xp = _vm->_eventsManager.start_x;
				int yp = Magic_Number(bloc.y1) - 4;
				if (yp < 0)
					yp = 0;
				int width = Magic_Number(bloc.x2) + 4 - xp;
				if (width < 4)
					width = 4;
				int height = Magic_Number(bloc.y2) + 4 - yp;
				if (height < 4)
					height = 4;

				if ((xp - _vm->_eventsManager.start_x + width) > SCREEN_WIDTH)
					xp -= 4;
				if ((height - yp) > (SCREEN_HEIGHT - 40))
					yp -= 4;

				if (Winbpp == 2) {
					m_scroll16A(VESA_BUFFER, xp, yp, width, height, 
						Reel_Zoom(xp - _vm->_eventsManager.start_x, SDL_ECHELLE), Reel_Zoom(yp, SDL_ECHELLE));
				} else {
					m_scroll2A(VESA_BUFFER, xp, yp, width, height, 
						Reel_Zoom(xp - _vm->_eventsManager.start_x, SDL_ECHELLE), Reel_Zoom(yp, SDL_ECHELLE));
				}

				dstRect.left = Reel_Zoom(xp - _vm->_eventsManager.start_x, SDL_ECHELLE);
				dstRect.top = Reel_Zoom(yp, SDL_ECHELLE);
				dstRect.setWidth(Reel_Zoom(width, SDL_ECHELLE));
				dstRect.setHeight(Reel_Zoom(height, SDL_ECHELLE));
			} else {
				if (Winbpp == 2) {
					m_scroll16(VESA_BUFFER, bloc.x1, bloc.y1, bloc.x2 - bloc.x1, bloc.y2 - bloc.y1,
						bloc.x1 - _vm->_eventsManager.start_x, bloc.y1);
				} else {
					m_scroll(VESA_BUFFER, bloc.x1, bloc.y1, bloc.x2 - bloc.x1, bloc.y2 - bloc.y1,
						bloc.x1 - _vm->_eventsManager.start_x, bloc.y1);
				}
			}
		}
		
		_vm->_globals.BLOC[idx].field0 = 0;
	}
}

void GraphicsManager::CopyAsm(const byte *surface) {
	const byte *v1;
	byte srcByte;
	byte *v3; 
	signed int v4;
	signed int v5;
	byte *v6; 
	byte *v7; 
	byte *v8; 
	byte *v9; 
	const byte *v10;

	assert(VideoPtr);
	v1 = surface;
	srcByte = 30 * WinScan;
	v3 = (byte *)VideoPtr->pixels + 30 * WinScan;
	v4 = 200;
	do {
		v10 = v1;
		v9 = v3;
		v5 = 320;
    
		do {
			srcByte = *v1;
			*v3 = *v1;
			v6 = WinScan + v3;
			*v6 = srcByte;
			v7 = v6 - WinScan + 1;
			*v7 = srcByte;
			v8 = WinScan + v7;
			*v8 = srcByte;
			v3 = v8 - WinScan + 1;
			++v1;
			--v5;
		} while (v5);
		
		v1 = v10 + 320;
		v3 = WinScan + WinScan + v9;
		--v4;
	} while (v4);
}

void GraphicsManager::Restore_Mem(byte *a1, const byte *a2, int a3, int a4, unsigned int a5, int a6) {
	byte *v6;
	int v7;
	const byte *v8;
	unsigned int i;
	int v10;

	v6 = a3 + nbrligne2 * a4 + a1;
	v7 = a6;
	v8 = a2;
	do {
		v10 = v7;
		if (a5 & 1) {
			memcpy(v6, v8, a5);
			v8 += a5;
			v6 += a5;
		} else if (a5 & 2) {
			for (i = a5 >> 1; i; --i) {
				*(uint16 *)v6 = *(uint16 *)v8;
				v8 += 2;
				v6 += 2;
			}
		} else {
			memcpy(v6, v8, 4 * (a5 >> 2));
			v8 += 4 * (a5 >> 2);
			v6 += 4 * (a5 >> 2);
		}
		v6 = nbrligne2 + v6 - a5;
		v7 = v10 - 1;
	} while (v10 != 1);
}

int GraphicsManager::Reel_Zoom(int v, int percentage) {
	return Asm_Zoom(v, percentage);
}

int GraphicsManager::Asm_Zoom(int v, int percentage) {
	if (v)
		v += percentage * (long int)v / 100;
  
	return v;
}

int GraphicsManager::Reel_Reduc(int v, int percentage) {
	return Asm_Reduc(v, percentage);
}

int GraphicsManager::Asm_Reduc(int v, int percentage) {
	if (v)
		v -= percentage * (long int)v / 100;
  
	return v;
}

void GraphicsManager::Affiche_Perfect(byte *destSurface, const byte *srcData, int a3, int a4, int a5, int a6, int a7, int a8) {
	const byte *v8; 
	int i; 
	const byte *v10; 
	int v11;
	int v12; 
	const byte *v13; 
	byte *v14; 
	int v15;
	byte *v16; 
	int v17;
	int v18; 
	int v19;
	int v20;
	int v21 = 0;
	int v22;
	int v23; 
	int v24;
	int v25;
	int v26; 
	int v27; 
	int v28; 
	byte *v29; 
	int v30; 
	int v31; 
	int v32; 
	int v33; 
	int v34; 
	int v35;
	int v36 = 0; 
	int v37;
	int v38; 
	int v39;
	byte *v40; 
	int v41; 
	int v42;
	const byte *v43;
	const byte *v44;
	const byte *v45;
	const byte *v46;
	byte *v47;
	byte *v48;
	int v49; 
	int v50; 
	byte *v51; 
	unsigned int v52;
	byte *v53;
	byte *v54;
	byte *v55;
	int v56;
	int v57;
	int v58;
	int v59;
	int v60;
	int v61;
	int v62;
	int v63;
	int v64;
	int v65;

	v8 = srcData + 3;
	for (i = a5; i; --i)
		v8 += READ_LE_UINT32(v8) + 16;
  
	v11 = 0;
	v12 = 0;
	v10 = v8 + 4;
	v11 = *(uint16 *)v10;
	v10 += 2;
	v12 = *(uint16 *)v10;
	v13 = v10 + 10;
	clip_x = clip_y = 0;
	clip_x1 = clip_y1 = 0;

	if (a3 > min_x) {
		if ((uint)a3 < (uint)(min_x + 300))
			clip_x = min_x + 300 - a3;
		if (a4 > min_y) {
			if ((uint)a4 < (uint)(min_y + 300))
				clip_y = min_y + 300 - a4;
			if ((uint)a3 < (uint)(max_x + 300)) {
				clip_x1 = max_x + 300 - a3;
			if ((uint)a4 < (uint)(max_y + 300)) {
				clip_y1 = max_y + 300 - a4;
				v14 = a3 + nbrligne2 * (a4 - 300) - 300 + destSurface;
				
				if ((uint16)a7) {
					Compteur_y = 0;
					Agr_x = 0;
					Agr_y = 0;
					Agr_Flag_y = 0;
					Agr_Flag_x = 0;
					Largeur = v11;
					v20 = Asm_Zoom(v11, a7);
					v22 = Asm_Zoom(v21, a7);
				
					if (a8) {
						v29 = v20 + v14;
						if (clip_y) {
							if (clip_y >= v22)
								return;
							v61 = v22;
							v52 = v20;
							v30 = 0;
							v31 = (uint)clip_y;
							
							while (Asm_Zoom(v30 + 1, a7) < v31)
								;
							v20 = v52;
							v13 += Largeur * v30;
							v29 += nbrligne2 * (uint)clip_y;
							v22 = v61 - (uint)clip_y;
						}
              
						if (v22 > clip_y1)
							v22 = clip_y1;
						if (clip_x) {
							if (clip_x >= v20)
								return;
							v20 -= (uint)clip_x;
						}
              
						if (v20 > clip_x1) {
							v32 = v20 - clip_x1;
							v29 -= v32;
							v62 = v22;
							v33 = 0;
							
							while (Asm_Zoom(v33 + 1, a7) < v32)
								;
							v34 = v33;
							v22 = v62;
							v13 += v34;
							v20 = (uint)clip_x1;
						}
					
						do {
							for (;;) {
								v63 = v22;
								v53 = v29;
								v46 = v13;
								Agr_Flag_x = 0;
								Agr_x = 0;
								v35 = v20;
                  
								do {
									for (;;) {
										if (*v13)
											*v29 = *v13;
										--v29;
										++v13;
										if (!Agr_Flag_x)
											Agr_x = a7 + Agr_x;
                      
										if ((uint)Agr_x < 100)
											break;
										
										Agr_x = Agr_x - 100;
										--v13;
										Agr_Flag_x = 1;
										--v35;
										if (!v35)
											goto R_Aff_Zoom_Larg_Cont1;
									}
									
									Agr_Flag_x = 0;
									--v35;
								} while (v35);

R_Aff_Zoom_Larg_Cont1:
								v13 = Largeur + v46;
								v29 = nbrligne2 + v53;
								++Compteur_y;
								if (!(uint16)Agr_Flag_y)
									Agr_y = a7 + Agr_y;
								
								if ((uint)Agr_y < 100)
									break;
					
								Agr_y = Agr_y - 100;
								v13 = v46;
								Agr_Flag_y = 1;
								v22 = v63 - 1;
								if (v63 == 1)
									return;
							}
                
							Agr_Flag_y = 0;
							v22 = v63 - 1;
						} while (v63 != 1);
					} else {
						if (clip_y) {
							if (clip_y >= v22)
								return;
							v58 = v22;
							v49 = v20;
							v23 = 0;
							v24 = (uint)clip_y;

							while (Asm_Zoom(v23 + 1, a7) < v24)
								;
							v20 = v49;
							v13 += Largeur * v23;
							v14 += nbrligne2 * (uint)clip_y;
							v22 = v58 - (uint)clip_y;
						}
              
						if (v22 > clip_y1)
							v22 = clip_y1;
              
						if (clip_x) {
							if (clip_x >= v20)
								return;
							v59 = v22;
							v50 = v20;
							v25 = (uint)clip_x;
							v26 = 0;
							
							while (Asm_Zoom(v26 + 1, a7) < v25)
								;
							v27 = v26;
							v22 = v59;
							v13 += v27;
							v14 += (uint)clip_x;
							v20 = v50 - (uint)clip_x;
						}

						if (v20 > clip_x1)
							v20 = clip_x1;
				
						do {
							for (;;) {
								v60 = v22;
								v51 = v14;
								v45 = v13;
								v28 = v20;
								Agr_Flag_x = 0;
								Agr_x = 0;
                  
								do {
									for (;;) {
										if (*v13)
											*v14 = *v13;
										++v14;
										++v13;
										
										if (!Agr_Flag_x)
											Agr_x = a7 + Agr_x;
										if ((uint)Agr_x < 100)
											break;
                      
										Agr_x = Agr_x - 100;
										--v13;
										Agr_Flag_x = 1;
										--v28;
                      
										if (!v28)
											goto Aff_Zoom_Larg_Cont1;
									}
                    
									Agr_Flag_x = 0;
									--v28;
								}
								while (v28);

Aff_Zoom_Larg_Cont1:
								v13 = Largeur + v45;
								v14 = nbrligne2 + v51;
								if (!(uint16)Agr_Flag_y)
									Agr_y = a7 + Agr_y;
                  
								if ((uint)Agr_y < 100)
									break;
                  
								Agr_y = Agr_y - 100;
								v13 = v45;
								Agr_Flag_y = 1;
								v22 = v60 - 1;

								if (v60 == 1)
									return;
							}
                
							Agr_Flag_y = 0;
							v22 = v60 - 1;
						} while (v60 != 1);
					}
				} else if ((uint16)a6) {
					Compteur_y = 0;
					Red_x = 0;
					Red_y = 0;
					Largeur = v11;
					Red = a6;
					
					if ((uint)a6 < 100) {
						v37 = Asm_Reduc(v11, Red);
						if (a8) {
							v40 = v37 + v14;
                
							do {
								v65 = v36;
								v55 = v40;
								Red_y = Red + Red_y;
								
								if ((uint)Red_y < 100) {
									Red_x = 0;
									v41 = Largeur;
									v42 = v37;
                    
									do {
										Red_x = Red + Red_x;
										if (Red_x < 100) {
											if (v42 >= clip_x && v42 < clip_x1 && *v13)
												*v40 = *v13;
											--v40;
											++v13;
											--v42;
										} else {
											Red_x = Red_x - 100;
											++v13;
										}
										--v41;
									} while (v41);
                    
									v36 = v65;
									v40 = nbrligne2 + v55;
								} else {
									Red_y = Red_y - 100;
									v13 += Largeur;
								}
								
								--v36;
							} while ( v36 );
						} else {
							do {
								v64 = v36;
								v54 = v14;
								Red_y = Red + Red_y;

								if ((uint)Red_y < 100) {
									Red_x = 0;
									v38 = Largeur;
									v39 = 0;
                    
									do {
										Red_x = Red + Red_x;
										if ((uint)Red_x < 100) {
											if (v39 >= clip_x && v39 < clip_x1 && *v13)
												*v14 = *v13;
											++v14;
											++v13;
											++v39;
										} else {
											Red_x = Red_x - 100;
											++v13;
										}
										
										--v38;
									} while ( v38 );
									
									v36 = v64;
									v14 = nbrligne2 + v54;
								} else {
									Red_y = Red_y - 100;
									v13 += Largeur;
								}
                  
								--v36;
							} while (v36);
						}
					}
				} else {
					Largeur = v11;
					Compteur_y = 0;
					if (a8) {
						v16 = v11 + v14;
						spec_largeur = v11;
						if (clip_y) {
							if ((uint)clip_y >= (unsigned int)v12)
								return;
							v13 += v11 * (uint)clip_y;
							v16 += nbrligne2 * (uint)clip_y;
							v12 -= (uint)clip_y;
						}

						v17 = (uint)clip_y1;
						if (v12 > clip_y1)
							v12 = clip_y1;
              
						v17 = clip_x;
						if (clip_x) {
							if (v17 >= v11)
								return;
							v11 -= v17;
						}
              
						if (v11 > clip_x1) {
							v18 = v11 - clip_x1;
							v13 += v18;
							v16 -= v18;
							v11 = (uint)clip_x1;
						}
              
						do {
							v57 = v12;
							v48 = v16;
							v44 = v13;
							v19 = v11;
                
							do {
								if (*v13)
									*v16 = *v13;
								++v13;
								--v16;
								--v19;
							} while (v19);

							v13 = spec_largeur + v44;
							v16 = nbrligne2 + v48;
							v12 = v57 - 1;
						} while (v57 != 1);
					} else {
						spec_largeur = v11;
						if (clip_y) {
							if ((uint)clip_y >= (unsigned int)v12)
								return;
					
							v13 += v11 * (uint)clip_y;
							v14 += nbrligne2 * (uint)clip_y;
							v12 -= (uint)clip_y;
						}
              
						if (v12 > clip_y1)
							v12 = clip_y1;
						if (clip_x) {
							if (clip_x >= v11)
								return;
					
							v13 += (uint)clip_x;
							v14 += (uint)clip_x;
							v11 -= (uint)clip_x;
						}
              
						if (v11 > clip_x1)
							v11 = clip_x1;
              
						do {
							v56 = v12;
							v47 = v14;
							v43 = v13;
							v15 = v11;
					
							do {
								if (*v13)
									*v14 = *v13;
								++v14;
								++v13;
								--v15;
							} while (v15);
                
							v13 = spec_largeur + v43;
							v14 = nbrligne2 + v47;
							v12 = v56 - 1;
						} while (v56 != 1);
					}
				}
			}
		}
	}
	}
}

void GraphicsManager::AFFICHE_SPEED(const byte *spriteData, int xp, int yp, int spriteIndex) {
	int width, height;

	width = _vm->_objectsManager.Get_Largeur(spriteData, spriteIndex);
	height = _vm->_objectsManager.Get_Hauteur(spriteData, spriteIndex);
	if (*spriteData == 78) {
		Affiche_Perfect(VESA_SCREEN, spriteData, xp + 300, yp + 300, spriteIndex, 0, 0, 0);
		Affiche_Perfect(VESA_BUFFER, spriteData, xp + 300, yp + 300, spriteIndex, 0, 0, 0);
	} else {
		Sprite_Vesa(VESA_BUFFER, spriteData, xp + 300, yp + 300, spriteIndex);
		Sprite_Vesa(VESA_SCREEN, spriteData, xp + 300, yp + 300, spriteIndex);
	}
	if (!_vm->_globals.NO_VISU)
		Ajoute_Segment_Vesa(xp, yp, xp + width, yp + height);
}

void GraphicsManager::SCOPY(const byte *surface, int x1, int y1, int width, int height, byte *destSurface, int destX, int destY) {
	int top; 
	int croppedWidth; 
	int croppedHeight;
	int v11; 
	int height2; 
	int top2; 
	int left;

	left = x1;
	top = y1;
	croppedWidth = width;
	croppedHeight = height;
  
	if (x1 < min_x) {
		croppedWidth = width - (min_x - x1);
		left = min_x;
	}
	if (y1 < min_y) {
		croppedHeight = height - (min_y - y1);
		top = min_y;
	}
	top2 = top;
	if (top + croppedHeight > max_y)
		croppedHeight = max_y - top;
	v11 = left + croppedWidth;
	if (v11 > max_x)
		croppedWidth = max_x - left;

	if (croppedWidth > 0 && croppedHeight > 0) {
		height2 = croppedHeight;
		Copy_Mem(surface, left, top2, croppedWidth, croppedHeight, destSurface, destX, destY);
		Ajoute_Segment_Vesa(left, top2, left + croppedWidth, top2 + height2);
	}
}

void GraphicsManager::Copy_Mem(const byte *srcSurface, int x1, int y1, unsigned int width, int height, byte *destSurface, int destX, int destY) {
	const byte *srcP; 
	byte *destP; 
	int yp;
	int yCurrent;
	byte *dest2P;
	const byte *src2P;
	unsigned int pitch; 

	srcP = x1 + nbrligne2 * y1 + srcSurface;
	destP = destX + nbrligne2 * destY + destSurface;
	yp = height;
	do {
		yCurrent = yp;
		memcpy(destP, srcP, 4 * (width >> 2));
		src2P = (srcP + 4 * (width >> 2));
		dest2P = (destP + 4 * (width >> 2));
		pitch = width - 4 * (width >> 2);
		memcpy(dest2P, src2P, pitch);
		destP = (dest2P + pitch + nbrligne2 - width);
		srcP = (src2P + pitch + nbrligne2 - width);
		yp = yCurrent - 1;
	} while (yCurrent != 1);
}

void GraphicsManager::Affiche_Fonte(byte *surface, const byte *spriteData, int xp, int yp, 
									int characterIndex, int transColour) {
	const byte *v6;
	int i; 
	const byte *v8; 
	int v9; 
	int v10;
	const byte *v11;
	byte *destP;
	int v13; 
	byte v14;
	byte *destLineP;
	int v16;

	v6 = spriteData + 3;
	for (i = characterIndex; i; --i)
		v6 += READ_LE_UINT32(v6) + 16;

	v9 = 0;
	v10 = 0;
	v8 = v6 + 4;
	v9 = READ_LE_UINT16(v8);
	v8 += 2;
	v10 = READ_LE_UINT16(v8);
	v11 = v8 + 10;
	destP = surface + xp + nbrligne2 * yp;
	Largeur = v9;

	do {
		v16 = v10;
		destLineP = destP;
		v13 = v9;
		do {
			v14 = *v11;
			if (*v11) {
				if (v14 == (byte)-4)
					v14 = transColour;
				*destP = v14;
			}
	      
			++destP;
			++v11;
			--v13;
		} while (v13);
		destP = nbrligne2 + destLineP;
		v10 = v16 - 1;
	} while (v16 != 1);
}

void GraphicsManager::INI_ECRAN(const Common::String &file) {
	OPTI_INI(file, 0);
}

void GraphicsManager::INI_ECRAN2(const Common::String &file) {
	OPTI_INI(file, 2);
}

void GraphicsManager::OPTI_INI(const Common::String &file, int a2) {
	int v2; 
	unsigned int v3; 
	int v6; 
	unsigned int v9; 
	signed int v11; 
	byte *ptr; 
	Common::String v13; 

	v2 = 1;
	v3 = 0;
	v9 = 0;
	
	v13 = file + ".ini";
	ptr = FileManager::RECHERCHE_CAT(v13, 1);
	if (PTRNUL == ptr) {
		FileManager::CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, v13);
		ptr = FileManager::CHARGE_FICHIER(_vm->_globals.NFICHIER);
	}
	if (!a2) {
		v13 = file + ".spr";
		if (PTRNUL != _vm->_globals.SPRITE_ECRAN)
			_vm->_globals.SPRITE_ECRAN = FileManager::LIBERE_FICHIER(_vm->_globals.SPRITE_ECRAN);
		if (!_vm->_globals.NOSPRECRAN) {
			_vm->_globals.SPRITE_ECRAN = FileManager::RECHERCHE_CAT(v13, 8);
			if (_vm->_globals.SPRITE_ECRAN) {
				_vm->_globals.CAT_FLAG = 0;
				FileManager::CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, v13);
			} else {
				_vm->_globals.CAT_FLAG = 1;
				FileManager::CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, "RES_SLI.RES");
			}
			_vm->_globals.SPRITE_ECRAN = FileManager::CHARGE_FICHIER(_vm->_globals.NFICHIER);
			_vm->_globals.CAT_FLAG = 0;
		}
	}
	if (*ptr != 'I' || *(ptr + 1) != 'N' || *(ptr + 2) != 'I') {
		error("Error, file not ini");
	} else {
		v11 = 0;
		do {
			v6 = _vm->_objectsManager.Traduction(ptr + 20 * v2);
			if (v6 == 2)
				v2 = _vm->_objectsManager.Control_Goto((ptr + 20 * v2));
			if (v6 == 3)
				v2 = _vm->_objectsManager.Control_If(ptr, v2);
			if (v2 == -1)
				error("Error, defective IFF");
			if (v6 == 1 || v6 == 4)
				++v2;
			if (!v6 || v6 == 5)
				v11 = 1;
		} while (v11 != 1);
	}
	_vm->_globals.dos_free2(ptr);
	if (a2 != 1) {
		if (PTRNUL != _vm->_globals.COUCOU)
			_vm->_globals.COUCOU = _vm->_globals.dos_free2(_vm->_globals.COUCOU);
		
		v13 = file + ".rep";
		byte *dataP = FileManager::RECHERCHE_CAT(v13, 2);
		_vm->_globals.COUCOU = dataP;
		if (PTRNUL == dataP) {
			FileManager::CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, v13);
			dataP = FileManager::CHARGE_FICHIER(_vm->_globals.NFICHIER);
			_vm->_globals.COUCOU = dataP;
		}
	}
	_vm->_objectsManager.FORCEZONE = 1;
	_vm->_objectsManager.CHANGEVERBE = 0;
}

void GraphicsManager::NB_SCREEN() {
	byte *v0; 
	const byte *v1;

	if (!_vm->_globals.NECESSAIRE)
		INIT_TABLE(50, 65, Palette);
	if (nbrligne == SCREEN_WIDTH)
		Trans_bloc2(VESA_BUFFER, TABLE_COUL, 307200);
	if (nbrligne == 1280)
		Trans_bloc2(VESA_BUFFER, TABLE_COUL, 614400);
	_vm->_graphicsManager.DD_Lock();
	if (Winbpp == 2) {
		if (SDL_ECHELLE)
			m_scroll16A(VESA_BUFFER, _vm->_eventsManager.start_x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		else
			m_scroll16(VESA_BUFFER, _vm->_eventsManager.start_x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	}
	if (Winbpp == 1) {
		if (SDL_ECHELLE)
			m_scroll2A(VESA_BUFFER, _vm->_eventsManager.start_x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		else
			m_scroll2(VESA_BUFFER, _vm->_eventsManager.start_x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	}
	_vm->_graphicsManager.DD_Unlock();
	
	v0 = VESA_SCREEN;
	v1 = VESA_BUFFER;
	memcpy(VESA_SCREEN, VESA_BUFFER, 0x95FFCu);
	v1 = v1 + 614396;
	v0 = v0 + 614396;
	*v0 = *v1;
	*(v0 + 2) = *(v1 + 2);
	DD_VBL();
}

int GraphicsManager::colision2_ligne(int a1, int a2, int *a3, int *a4, int a5, int a6) {
	warning("TODO: colision2_ligne");
	return 0;
}

} // End of namespace Hopkins
