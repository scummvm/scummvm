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
	max_y = SCREEN_HEIGHT - 20;
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
	Common::fill(&TABLE_COUL[0], &TABLE_COUL[PALETTE_EXT_BLOCK_SIZE], 0);
	Common::fill(&cmap[0], &cmap[PALETTE_BLOCK_SIZE], 0);
	Common::fill(&Palette[0], &Palette[PALETTE_EXT_BLOCK_SIZE], 0);
	Common::fill(&OLD_PAL[0], &OLD_PAL[PALETTE_EXT_BLOCK_SIZE], 0);
}

GraphicsManager::~GraphicsManager() {
	_vm->_globals.dos_free2(VESA_SCREEN);
	_vm->_globals.dos_free2(VESA_BUFFER);
}

void GraphicsManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
}

void GraphicsManager::SET_MODE(int width, int height) {
	if (!SDL_MODEYES) {
		SDL_ECHELLE = 0;

		if (_vm->_globals.XSETMODE == 1)
			SDL_ECHELLE = 0;
		if (_vm->_globals.XSETMODE == 2)
			SDL_ECHELLE = 25;
		if (_vm->_globals.XSETMODE == 3)
			SDL_ECHELLE = 50;
		if (_vm->_globals.XSETMODE == 4)
			SDL_ECHELLE = 75;
		if (_vm->_globals.XSETMODE == 5)
			SDL_ECHELLE = _vm->_globals.XZOOM;
		
		int bpp = 8;
		if (_vm->_globals.XFORCE8 == true)
			bpp = 8;
		if (_vm->_globals.XFORCE16 == true)
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
		VESA_SCREEN = _vm->_globals.dos_malloc2(SCREEN_WIDTH * 2 * SCREEN_HEIGHT);
		VESA_BUFFER = _vm->_globals.dos_malloc2(SCREEN_WIDTH * 2 * SCREEN_HEIGHT);

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

// Clear Screen
void GraphicsManager::Cls_Video() {
	assert(VideoPtr);

	VideoPtr->fillRect(Common::Rect(0, 0, XSCREEN, YSCREEN), 0);
}

void GraphicsManager::LOAD_IMAGE(const Common::String &file) {
	Common::String filename	= Common::String::format("%s.PCX", file.c_str());
	CHARGE_ECRAN(filename);
	INIT_TABLE(165, 170, Palette);
}

// Load Screen
void GraphicsManager::CHARGE_ECRAN(const Common::String &file) {
	bool flag;
	Common::File f;

	_vm->_fileManager.DMESS1();

	flag = true;
	if (_vm->_fileManager.RECHERCHE_CAT(file, 6)) {
		_vm->_fileManager.CONSTRUIT_FICHIER(_vm->_globals.HOPIMAGE, file);
		if (!f.open(_vm->_globals.NFICHIER))
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
		_vm->_eventsManager.souris_max();
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

void GraphicsManager::INIT_TABLE(int minIndex, int maxIndex, byte *palette) {
	for (int idx = 0; idx < 256; ++idx)
		TABLE_COUL[idx] = idx;
  
	Trans_bloc(TABLE_COUL, palette, 256, minIndex, maxIndex);

	for (int idx = 0; idx < 256; ++idx) {
		byte v = TABLE_COUL[idx];
		if (v > 27)
			TABLE_COUL[idx] = 0;
		if (!v)
			TABLE_COUL[idx] = 0;
	}

	TABLE_COUL[0] = 1;
}

// Scroll Screen
int GraphicsManager::SCROLL_ECRAN(int amount) {
	int result = CLIP(amount, 0, SCREEN_WIDTH);
	_vm->_eventsManager.start_x = result;
	ofscroll = result;
	SCROLL = result;
	return result;
}

void GraphicsManager::Trans_bloc(byte *destP, const byte *srcP, int count, int minThreshold, int maxThreshold) {
	byte *destPosP;
	int palIndex;
	int srcOffset;
	int col1, col2;

	destPosP = destP;
	for (int idx = 0; idx < count; ++idx) {
		palIndex = *(byte *)destPosP++;
		srcOffset = 3 * palIndex;
		col1 = *(srcP + srcOffset) + *(srcP + srcOffset + 1) + *(srcP + srcOffset + 2);

		for (int idx2 = 0; idx2 < 38; ++idx2) {
			srcOffset = 3 * idx2;
			col2 = *(srcP + srcOffset) + *(srcP + srcOffset + 1) + *(srcP + srcOffset + 2);

			col2 += minThreshold;
			if (col2 < col1)
				continue;
			
			col2 -= maxThreshold;
			if (col2 > col1)
				continue;

			*(destPosP - 1) = (idx2 == 0) ? 1 : idx2;
			break;
		}
	}
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
		_vm->_fileManager.CONSTRUIT_FICHIER(_vm->_globals.HOPIMAGE, "PIC.RES");
		if (!f.open(_vm->_globals.NFICHIER))
			error("(nom)Erreur en cours de lecture.");
		f.seek(_vm->_globals.CAT_POSI);

		v7 = _vm->_globals.CAT_TAILLE - 896;
		v8 = f.read(HEADER_PCX, 128);

		v6 = (int16)READ_LE_UINT16(&HEADER_PCX[8]) + 1;
		v20 = (int16)READ_LE_UINT16(&HEADER_PCX[10]) + 1;
		if (((int16)READ_LE_UINT16(&HEADER_PCX[8]) + 1) <= SCREEN_WIDTH) {
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
		_vm->_fileManager.CONSTRUIT_FICHIER(_vm->_globals.HOPIMAGE, file);
		if (!f.open(_vm->_globals.NFICHIER))
		  error("(nom)Erreur en cours de lecture.");

		filesize = f.size();
		int bytesRead = f.read(HEADER_PCX, 128);
		if (bytesRead < 128)
			error("Erreur en cours de lecture.");

		v6 = (int16)READ_LE_UINT16(&HEADER_PCX[8]) + 1;
		v20 = (int16)READ_LE_UINT16(&HEADER_PCX[10]) + 1;
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

	ptr = _vm->_globals.dos_malloc2(0xEE60u);
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
		f.seek(_vm->_globals.CAT_TAILLE + _vm->_globals.CAT_POSI - 768);
	} else {
		filesize = f.size();
		f.seek(filesize - 768);
	}

	if (f.read(palette, PALETTE_BLOCK_SIZE) != (PALETTE_BLOCK_SIZE))
		error("A_PCX640_480");
  
	f.close();
	_vm->_globals.dos_free2(ptr);
}

// Clear Palette
void GraphicsManager::Cls_Pal() {
	Common::fill(&cmap[0], &cmap[PALETTE_BLOCK_SIZE], 0);
	SD_PIXELS[0] = 0;

	if (Winbpp == 1) {
		g_system->getPaletteManager()->setPalette(cmap, 0, 256);
	}
}

void GraphicsManager::SCANLINE(int pitch) {
	nbrligne = nbrligne2 = pitch;
}

void GraphicsManager::m_scroll(const byte *surface, int xs, int ys, int width, int height, int destX, int destY) {
	const byte *srcP;
	byte *destP;
	int yNext;
	int yCtr;
	byte *dest2P;
	const byte *src2P;
	unsigned int widthRemaining;

	assert(VideoPtr);
	srcP = xs + nbrligne2 * ys + surface;
	destP = destX + WinScan * destY + (byte *)VideoPtr->pixels;
	yNext = height;
	do {
		yCtr = yNext;
		memcpy((byte *)destP, (const byte *)srcP, 4 * (width >> 2));
		src2P = (const byte *)(srcP + 4 * (width >> 2));
		dest2P = (byte *)(destP + 4 * (width >> 2));
		widthRemaining = width - 4 * (width >> 2);
		memcpy(dest2P, src2P, widthRemaining);
		destP = dest2P + widthRemaining + WinScan - width;
		srcP = src2P + widthRemaining + nbrligne2 - width;
		yNext = yCtr - 1;
	} while (yCtr != 1);
}

void GraphicsManager::m_scroll2(const byte *surface, int xs, int ys, int width, int height, int destX, int destY) {
	const byte *srcP;
	byte *destP;
	int destPitch;
	int srcPitch;
	int yCtr;

	assert(VideoPtr);
	srcP = xs + nbrligne2 * ys + surface;
	destP = destX + WinScan * destY + (byte *)VideoPtr->pixels;
	destPitch = WinScan - SCREEN_WIDTH;
	srcPitch = nbrligne2 - SCREEN_WIDTH;
	yCtr = height;

	do {
		memcpy(destP, srcP, SCREEN_WIDTH);
		destP = destP + destPitch + SCREEN_WIDTH;
		srcP = srcP + srcPitch + SCREEN_WIDTH;
		--yCtr;
	} while (yCtr);
}

void GraphicsManager::m_scroll2A(const byte *surface, int xs, int ys, int width, int height, int destX, int destY) {
	const byte *srcP;
	byte *destP;
	int yCtr;
	int xCtr;
	byte srcByte;
	const byte *srcCopyP;
	byte *destCopyP;

	assert(VideoPtr);
	srcP = xs + nbrligne2 * ys + surface;
	destP = destX + WinScan * destY + (byte *)VideoPtr->pixels;
	yCtr = height;
	Agr_x = 0;
	Agr_y = 0;
	Agr_Flag_y = 0;
	do {
		for (;;) {
			destCopyP = destP;
			srcCopyP = srcP;
			xCtr = width;
			Agr_x = 0;
			do {
				srcByte = *srcP;
				*destP++ = *srcP++;
				Agr_x += SDL_ECHELLE;
				if ((unsigned int)Agr_x >= 100) {
					Agr_x -= 100;
					*destP++ = srcByte;
				}
				--xCtr;
			} while ( xCtr );
	      
			srcP = srcCopyP;
			destP = WinScan + destCopyP;
			if (Agr_Flag_y)
				break;

			Agr_y += SDL_ECHELLE;
			if ((unsigned int)Agr_y < 100)
				break;

			Agr_y -= 100;
			Agr_Flag_y = 1;
		}

		Agr_Flag_y = 0;
		srcP = nbrligne2 + srcCopyP;
		--yCtr;
	} while (yCtr);
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

// TODO: See if PAL_PIXELS can be converted to a uint16 array
void GraphicsManager::m_scroll16A(const byte *surface, int xs, int ys, int width, int height, int destX, int destY) {
	const byte *srcP;
	const byte *destP;
	int yNext;
	int xCtr;
	const byte *palette;
	int pixelWord;
	int yCtr;
	const byte *srcCopyP;
	const byte *destCopyP;

	assert(VideoPtr);
	srcP = xs + nbrligne2 * ys + surface;
	destP = destX + destX + WinScan * destY + (byte *)VideoPtr->pixels;
	yNext = height;
	Agr_x = 0;
	Agr_y = 0;
	Agr_Flag_y = 0;

	do {
		for (;;) {
			destCopyP = destP;
			srcCopyP = srcP;
			xCtr = width;
			yCtr = yNext;
			palette = PAL_PIXELS;
			Agr_x = 0;

			do {
				pixelWord = *(uint16 *)(palette + 2 * *srcP);
				*(uint16 *)destP = pixelWord;
				++srcP;
				destP += 2;
				Agr_x += SDL_ECHELLE;
				if ((unsigned int)Agr_x >= 100) {
					Agr_x -= 100;
					*(uint16 *)destP = pixelWord;
					destP += 2;
				}
        
				--xCtr;
			} while (xCtr);

			yNext = yCtr;
			srcP = srcCopyP;
			destP = WinScan + destCopyP;
			if (Agr_Flag_y == 1)
				break;
			
			Agr_y += SDL_ECHELLE;
			
			if ((unsigned int)Agr_y < 100)
				break;
      
			Agr_y -= 100;
			Agr_Flag_y = 1;
		}

		Agr_Flag_y = 0;
		srcP = nbrligne2 + srcCopyP;
		yNext = yCtr - 1;
	} while (yCtr != 1);
}

void GraphicsManager::Copy_Vga(const byte *surface, int xp, int yp, int width, int height, int destX, int destY) {
	const byte *srcP;
	byte *destP;
	int yCount;
	int xCount;
	byte srcByte;
	byte *loopDestP; 
	byte *loopSrcP; 
	byte *loopSrc2P; 
	byte *tempDestP; 
	const byte *tempSrcP; 
	int yCtr; 

	assert(VideoPtr);
	srcP = xp + 320 * yp + surface;
	destP = 30 * WinScan + destX + destX + WinScan * 2 * destY + (byte *)VideoPtr->pixels;
	yCount = height;
  
	do {
		yCtr = yCount;
		xCount = width;
		tempSrcP = srcP;
		tempDestP = destP;
		do {
			srcByte = *srcP;
			*destP = *srcP;
			loopDestP = WinScan + destP;
			*loopDestP = srcByte;
			loopSrcP = loopDestP - WinScan + 1;
			*loopSrcP = srcByte;
			loopSrc2P = WinScan + loopSrcP;
			*loopSrc2P = srcByte;
			++srcP;
			destP = loopSrc2P - WinScan + 1;
			--xCount;
		} while (xCount);
    
		destP = WinScan + WinScan + tempDestP;
		srcP = tempSrcP + 320;
		yCount = yCtr - 1;
	} while (yCtr != 1);
}

void GraphicsManager::Copy_Vga16(const byte *surface, int xp, int yp, int width, int height, int destX, int destY) {
	const byte *srcP; 
	uint16 *destP; 
	int yCount; 
	int xCount;
	int xCtr;
	const byte *palette;
	uint16 *tempSrcP;
	uint16 srcByte;
	uint16 *tempDestP;
	int savedXCount;
	uint16 *loopDestP;
	const byte *loopSrcP;
	int yCtr;

	assert(VideoPtr);
	srcP = xp + 320 * yp + surface;
	destP = (uint16 *)(30 * WinScan + destX + destX + destX + destX + WinScan * 2 * destY + (byte *)VideoPtr->pixels);
	yCount = height;
	xCount = width;
  
	do {
		yCtr = yCount;
		xCtr = xCount;
		loopSrcP = srcP;
		loopDestP = destP;
		savedXCount = xCount;
		palette = PAL_PIXELS;
		
		do {
			tempSrcP = (uint16 *)(palette + 2 * *srcP);
			srcByte = *tempSrcP;
			*destP = *tempSrcP;
			*(destP + 1) = srcByte;
			
			tempDestP = (uint16 *)((byte *)destP + WinScan);
			*tempDestP = srcByte;
			*(tempDestP + 1) = srcByte;
			++srcP;
			destP = (uint16 *)((byte *)tempDestP - WinScan + 4);
			--xCtr;
		} while (xCtr);
    
		xCount = savedXCount;
		destP = (uint16 *)((byte *)loopDestP + WinScan * 2);
		srcP = loopSrcP + 320;
		yCount = yCtr - 1;
	} while (yCtr != 1);
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
	uint16 *pTemp1 = &palData1[1];
	for (int fadeIndex = 0; fadeIndex < FADESPD; ++fadeIndex) {
		uint16 *pTemp2 = &palData1[2];

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

			if (palData2[palOffset + 2] < palette[palOffset + 2]) {
				uint16 *pDest = &pTemp2[palOffset];
				uint16 v = (palette[palOffset] & 0xff) * 256 / FADESPD + *pDest; 
				*pDest = v;
				palData2[palOffset + 2] = (v >> 8) & 0xff;
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
	int palByte;
	int palIndex;
	int palDataIndex; 
	int palCtr3;
	int palCtr4;
	int palCtr5;
	int palValue;
	int palCtr2;
	int palCtr1;
	uint16 palMax;
	byte palData[PALETTE_BLOCK_SIZE];
	int tempPalette[PALETTE_BLOCK_SIZE];

	palMax = palByte = FADESPD;
	if (palette) {
		palIndex = 0;
		do {
			palDataIndex = palIndex;
			palByte = *(palIndex + palette);
			palByte <<= 8;
			tempPalette[palDataIndex] = palByte;
			palData[palDataIndex] = *(palIndex++ + palette);
		} while (palIndex < PALETTE_BLOCK_SIZE);
    
		setpal_vga256(palData);
		if (Winbpp == 2) {
			if (SDL_ECHELLE)
				m_scroll16A(surface, _vm->_eventsManager.start_x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			else
				m_scroll16(surface, _vm->_eventsManager.start_x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			DD_VBL();
		}
	
		palCtr3 = 0;
		if (palMax > 0) {
			do {
				palCtr4 = 0;
				do {
					palCtr5 = palCtr4;
					palValue = tempPalette[palCtr4] - (*(palCtr4 + palette) << 8) / palMax;
					tempPalette[palCtr5] = palValue;
					palData[palCtr5] = (palValue >> 8) & 0xff;
					++palCtr4;
				} while (palCtr4 < (PALETTE_BLOCK_SIZE));
				
				setpal_vga256(palData);
				if (Winbpp == 2) {
					if (SDL_ECHELLE)
						m_scroll16A(surface, _vm->_eventsManager.start_x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
					else
						m_scroll16(surface, _vm->_eventsManager.start_x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			
					DD_VBL();
				}
				++palCtr3;
			} while (palMax > palCtr3);
		}

		palCtr2 = 0;
		do {
			palData[palCtr2++] = 0;
		} while (palCtr2 < (PALETTE_BLOCK_SIZE));

		setpal_vga256(palData);
    
		if (Winbpp == 2) {
			if (!SDL_ECHELLE) {
				m_scroll16(surface, _vm->_eventsManager.start_x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
				return DD_VBL();
			}
			goto LABEL_28;
		}
	} else {
		palCtr1 = 0;
		do {
			palData[palCtr1++] = 0;
		} while (palCtr1 < (PALETTE_BLOCK_SIZE));

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
	
	if (Winbpp == 1)
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
	assert(surface);
	fade_out(Palette, FADESPD, surface);
}

void GraphicsManager::FADE_INW_LINUX(const byte *surface) {
	assert(surface);
	fade_in(Palette, FADESPD, surface);
}

void GraphicsManager::Copy_WinScan_Vbe3(const byte *srcData, byte *destSurface) {
	int rleValue;
	int destOffset; 
	const byte *srcP; 
	byte srcByte;
	byte destLen1;
	byte *destSlice1P; 
	byte destLen2; 
	byte *destSlice2P; 

	rleValue = 0;
	destOffset = 0;
	srcP = srcData;
	for (;;) {
		srcByte = *srcP;
		if (*srcP < 0xDEu)
			goto Video_Cont3_wVbe;
		if (srcByte == (byte)-4)
			return;

		if (srcByte < 0xFBu) {
			destOffset += (byte)(*srcP + 35);
			srcByte = *(srcP++ + 1);
		} else if (srcByte == (byte)-3) {
			destOffset += *(srcP + 1);
			srcByte = *(srcP + 2);
			srcP += 2;
		} else if (srcByte == (byte)-2) {
			destOffset += READ_LE_UINT16(srcP + 1);
			srcByte = *(srcP + 3);
			srcP += 3;
		} else {
			destOffset += READ_LE_UINT32(srcP + 1);
			srcByte = *(srcP + 5);
			srcP += 5;
		}
Video_Cont3_wVbe:
		if (srcByte > 0xD2u) {
			if (srcByte == (byte)-45) {
				destLen1 = *(srcP + 1);
				rleValue = *(srcP + 2);
				destSlice1P = destOffset + destSurface;
				destOffset += destLen1;
				memset(destSlice1P, rleValue, destLen1);
				srcP += 3;
			} else {
				destLen2 = (byte)(*srcP + 45);
				rleValue = *(srcP + 1);
				destSlice2P = destOffset + destSurface;
				destOffset += destLen2;
				memset(destSlice2P, rleValue, destLen2);
				srcP += 2;
			}
		} else {
			*(destOffset + destSurface) = srcByte;
			++srcP;
			++destOffset;
		}
	}}

void GraphicsManager::Copy_Video_Vbe3(const byte *srcData) {
	int rleValue;
	int destOffset;
	const byte *srcP;
	uint8 srcByte;
	int destLen1;
	byte *destSlice1P;
	int destLen2;
	byte *destSlice2P;

	assert(VideoPtr);
	rleValue = 0;
	destOffset = 0;
	srcP = srcData;
	for (;;) {
		srcByte = *srcP;
		if (*srcP < 222)
			goto Video_Cont3_Vbe;
    
		if (srcByte == 252)
			return;
		if (srcByte < 251) {
			destOffset += *srcP + 35;
			srcByte = *(srcP++ + 1);
		} else if (srcByte == (byte)-3) {
			destOffset += *(srcP + 1);
			srcByte = *(srcP + 2);
			srcP += 2;
		} else if (srcByte == (byte)-2) {
			destOffset += READ_LE_UINT16(srcP + 1);
			srcByte = *(srcP + 3);
			srcP += 3;
		} else {
			destOffset += READ_LE_UINT32(srcP + 1);
			srcByte = *(srcP + 5);
			srcP += 5;
		}
Video_Cont3_Vbe:
		if (srcByte > 210) {
			if (srcByte == 211) {
				destLen1 = *(srcP + 1);
				rleValue = *(srcP + 2);
				destSlice1P = destOffset + (byte *)VideoPtr->pixels;
				destOffset += destLen1;
				memset(destSlice1P, rleValue, destLen1);
				srcP += 3;
			} else {
				destLen2 = (byte)(*srcP + 45);
				rleValue = *(srcP + 1);
				destSlice2P = (byte *)(destOffset + (byte *)VideoPtr->pixels);
				destOffset += destLen2;
				memset(destSlice2P, rleValue, destLen2);
				srcP += 2;
			}
		} else {
			*(destOffset + (byte *)VideoPtr->pixels) = srcByte;
			++srcP;
			++destOffset;
		}
	}
}

void GraphicsManager::Copy_Video_Vbe16(const byte *srcData) {
	const byte *srcP = srcData;
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

void GraphicsManager::Copy_Video_Vbe16a(const byte *srcData) {
	int destOffset; 
	const byte *srcP; 
	byte pixelIndex; 

	destOffset = 0;
	srcP = srcData;
	for (;;) {
		pixelIndex = *srcP;
		if (*srcP < 0xFCu)
			goto Video_Cont_Vbe16a;
		if (pixelIndex == (byte)-4)
			return;
		if (pixelIndex == (byte)-3) {
			destOffset += *(srcP + 1);
			pixelIndex = *(srcP + 2);
			srcP += 2;
		} else if (pixelIndex == (byte)-2) {
			destOffset += READ_LE_UINT16(srcP + 1);
			pixelIndex = *(srcP + 3);
			srcP += 3;
		} else {
			destOffset += READ_LE_UINT32(srcP + 1);
			pixelIndex = *(srcP + 5);
			srcP += 5;
		}
Video_Cont_Vbe16a:
		WRITE_LE_UINT16((byte *)VideoPtr->pixels + destOffset * 2, READ_LE_UINT16(PAL_PIXELS + 2 * pixelIndex));
		++srcP;
		++destOffset;
	}
}

void GraphicsManager::Capture_Mem(const byte *srcSurface, byte *destSurface, int xs, int ys, int width, int height) {
	const byte *srcP;
	byte *destP;
	int rowCount; 
	int i;
	int rowCount2; 

	// TODO: This code in the original is potentially dangerous, as it doesn't clip the area to within
	// the screen, and so thus can read areas outside of the allocated surface buffer
	srcP = xs + nbrligne2 * ys + srcSurface;
	destP = destSurface;
	rowCount = height;
	do {
		rowCount2 = rowCount;
		if (width & 1) {
			memcpy(destP, srcP, width);
			srcP += width;
			destP += width;
		} else if (width & 2) {
			for (i = width >> 1; i; --i) {
				*(uint16 *)destP = *(uint16 *)srcP;
				srcP += 2;
				destP += 2;
			}
		} else {
			memcpy(destP, srcP, 4 * (width >> 2));
			srcP += 4 * (width >> 2);
			destP += 4 * (width >> 2);
		}
		srcP = nbrligne2 + srcP - width;
		rowCount = rowCount2 - 1;
	} while (rowCount2 != 1);
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
	int idx;
	int blocCount;

	if (_vm->_globals.NBBLOC > 0) {
		idx = 0;
		blocCount = _vm->_globals.NBBLOC;
		do {
			_vm->_globals.BLOC[idx++].field0 = 0;
		} while (idx != blocCount);
    
		_vm->_globals.NBBLOC = 0;
	}
}

// Add VESA Segment
void GraphicsManager::Ajoute_Segment_Vesa(int x1, int y1, int x2, int y2) {
	int tempX;
	int blocCount;
	int16 blocIndex;
	bool addFlag;

	tempX = x1;
	addFlag = true;
	if (x2 > max_x)
		x2 = max_x;
	if (y2 > max_y)
		y2 = max_y;
	if (x1 < min_x)
		tempX = min_x;
	if (y1 < min_y)
		y1 = min_y;

	blocCount = _vm->_globals.NBBLOC;
	if (_vm->_globals.NBBLOC > 1) {

		blocIndex = 0;
		do {
			BlocItem &bloc = _vm->_globals.BLOC[blocIndex];

			if (bloc.field0 == 1 
					&& tempX >= bloc.x1 && x2 <= bloc.x2
					&& y1 >= bloc.y1 && y2 <= bloc.y2)
				addFlag = false;
			++blocIndex;
			blocCount = blocIndex;
		} while (_vm->_globals.NBBLOC + 1 != blocIndex);
	}
	
	if (addFlag) {
		assert(_vm->_globals.NBBLOC < 50);
		BlocItem &bloc = _vm->_globals.BLOC[++_vm->_globals.NBBLOC];

		bloc.field0 = 1;
		bloc.x1 = tempX;
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

// Display VESA Segment
void GraphicsManager::Affiche_Segment_Vesa() {
	if (_vm->_globals.NBBLOC == 0)
		return;

	SDL_NBLOCS = _vm->_globals.NBBLOC;
	DD_Lock();

	for (int idx = 1; idx <= _vm->_globals.NBBLOC; ++idx) {
		BlocItem &bloc = _vm->_globals.BLOC[idx];
		Common::Rect &dstRect = dstrect[idx - 1];
		if (bloc.field0 != 1)
			continue;
	
		if (_vm->_eventsManager.CASSE != false) {
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
			
			if (SDL_ECHELLE) {
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

				// WORKAROUND: Original didn't lock the screen for access
				DD_Lock();

				if (Winbpp == 2) {
					m_scroll16A(VESA_BUFFER, xp, yp, width, height, 
						Reel_Zoom(xp - _vm->_eventsManager.start_x, SDL_ECHELLE), Reel_Zoom(yp, SDL_ECHELLE));
				} else {
					m_scroll2A(VESA_BUFFER, xp, yp, width, height, 
						Reel_Zoom(xp - _vm->_eventsManager.start_x, SDL_ECHELLE), Reel_Zoom(yp, SDL_ECHELLE));
				}

				DD_Unlock();

				dstRect.left = Reel_Zoom(xp - _vm->_eventsManager.start_x, SDL_ECHELLE);
				dstRect.top = Reel_Zoom(yp, SDL_ECHELLE);
				dstRect.setWidth(Reel_Zoom(width, SDL_ECHELLE));
				dstRect.setHeight(Reel_Zoom(height, SDL_ECHELLE));
			} else {
				// WORKAROUND: Original didn't lock the screen for access
				DD_Lock();

				if (Winbpp == 2) {
					m_scroll16(VESA_BUFFER, bloc.x1, bloc.y1, bloc.x2 - bloc.x1, bloc.y2 - bloc.y1,
						bloc.x1 - _vm->_eventsManager.start_x, bloc.y1);
				} else {
					m_scroll(VESA_BUFFER, bloc.x1, bloc.y1, bloc.x2 - bloc.x1, bloc.y2 - bloc.y1,
						bloc.x1 - _vm->_eventsManager.start_x, bloc.y1);
				}

				dstRect.left = bloc.x1 - _vm->_eventsManager.start_x;
				dstRect.top = bloc.y1;
				dstRect.setWidth(bloc.x2 - bloc.x1);
				dstRect.setHeight(bloc.y2 - bloc.y1);

				DD_Unlock();
			}
		}
		
		_vm->_globals.BLOC[idx].field0 = 0;
	}

	_vm->_globals.NBBLOC = 0;
	DD_Unlock();
	if (!_vm->_globals.BPP_NOAFF) {
//		SDL_UpdateRects(LinuxScr, SDL_NBLOCS, dstrect);
	}
	SDL_NBLOCS = 0;
}

void GraphicsManager::CopyAsm(const byte *surface) {
	const byte *srcP;
	byte srcByte;
	byte *destP; 
	int yCtr;
	int xCtr;
	byte *dest1P; 
	byte *dest2P; 
	byte *dest3P; 
	byte *destPitch; 
	const byte *srcPitch;

	assert(VideoPtr);
	srcP = surface;
	srcByte = 30 * WinScan;
	destP = (byte *)VideoPtr->pixels + 30 * WinScan;
	yCtr = 200;
	do {
		srcPitch = srcP;
		destPitch = destP;
		xCtr = 320;
    
		do {
			srcByte = *srcP;
			*destP = *srcP;
			dest1P = WinScan + destP;
			*dest1P = srcByte;
			dest2P = dest1P - WinScan + 1;
			*dest2P = srcByte;
			dest3P = WinScan + dest2P;
			*dest3P = srcByte;
			destP = dest3P - WinScan + 1;
			++srcP;
			--xCtr;
		} while (xCtr);
		
		srcP = srcPitch + 320;
		destP = WinScan + WinScan + destPitch;
		--yCtr;
	} while (yCtr);
}

void GraphicsManager::Restore_Mem(byte *destSurface, const byte *src, int xp, int yp, int width, int height) {
	byte *destP;
	int yNext;
	const byte *srcP;
	int i;
	int yCtr;

	destP = xp + nbrligne2 * yp + destSurface;
	yNext = height;
	srcP = src;
	do {
		yCtr = yNext;
		if (width & 1) {
			memcpy(destP, srcP, width);
			srcP += width;
			destP += width;
		} else if (width & 2) {
			for (i = width >> 1; i; --i) {
				*(uint16 *)destP = *(uint16 *)srcP;
				srcP += 2;
				destP += 2;
			}
		} else {
			memcpy(destP, srcP, 4 * (width >> 2));
			srcP += 4 * (width >> 2);
			destP += 4 * (width >> 2);
		}
		destP = nbrligne2 + destP - width;
		yNext = yCtr - 1;
	} while (yCtr != 1);
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

// Display 'Perfect?' 
void GraphicsManager::Affiche_Perfect(byte *surface, const byte *srcData, int xp300, int yp300, int frameIndex, int a6, int a7, int a8) {
	const byte *spriteStartP; 
	int i; 
	const byte *spriteSizeP;
	int spriteWidth;
	int spriteHeight1; 
	const byte *spritePixelsP; 
	byte *dest1P; 
	int xCtr1; 
	byte *dest2P; 
	int xLeft; 
	int clippedWidth; 
	int xCtr2; 
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
	int spriteHeight2; 
	int v37;
	int v38; 
	uint16 v39; 
	byte *v40; 
	int v41; 
	uint16 v42; 
	const byte *spritePixelsCopyP; 
	const byte *spritePixelsCopy2P; 
	const byte *v45; 
	const byte *v46; 
	byte *dest1CopyP; 
	byte *destCopy2P; 
	int v49; 
	int v50; 
	byte *v51; 
	int v52; 
	byte *v53; 
	byte *v54; 
	byte *v55; 
	int yCtr1; 
	int yCtr2; 
	int v58; 
	int v59; 
	int v60; 
	int v61; 
	int v62; 
	int v63; 
	int v64; 
	int v65; 

	spriteStartP = srcData + 3;
	for (i = frameIndex; i; --i)
		spriteStartP += READ_LE_UINT32(spriteStartP) + 16;

	spriteWidth = 0;
	spriteHeight1 = 0;
	spriteSizeP = spriteStartP + 4;
	spriteWidth = (int16)READ_LE_UINT16(spriteSizeP);
	spriteSizeP += 2;
	spriteHeight1 = spriteHeight2 = (int16)READ_LE_UINT16(spriteSizeP);
	spritePixelsP = spriteSizeP + 10;
	clip_x = 0;
	clip_y = 0;
	clip_x1 = 0;
	clip_y1 = 0;
	if ((uint16)xp300 > min_x) {
		if ((uint16)xp300 < (uint16)(min_x + 300))
			clip_x = min_x + 300 - xp300;
		if ((uint16)yp300 > min_y) {
			if ((uint16)yp300 < (uint16)(min_y + 300))
				clip_y = min_y + 300 - yp300;
			if ((uint16)xp300 < (uint16)(max_x + 300)) {
				clip_x1 = max_x + 300 - xp300;
				if ((uint16)yp300 < (uint16)(max_y + 300)) {
					clip_y1 = max_y + 300 - yp300;
					dest1P = xp300 + nbrligne2 * (yp300 - 300) - 300 + surface;
					if (a7) {
						Compteur_y = 0;
						Agr_x = 0;
						Agr_y = 0;
						Agr_Flag_y = 0;
						Agr_Flag_x = 0;
						Largeur = spriteWidth;
						v20 = Asm_Zoom(spriteWidth, a7);
						v22 = Asm_Zoom(v21, a7);
						if (a8) {
							v29 = v20 + dest1P;
							if (clip_y) {
								if ((uint16)clip_y >= v22)
									return;
								v61 = v22;
								v52 = v20;
								v30 = 0;
								v31 = (uint16)clip_y;
								while (Asm_Zoom(v30 + 1, a7) < v31)
									;
								v20 = v52;
								spritePixelsP += Largeur * v30;
								v29 += nbrligne2 * (uint16)clip_y;
								v22 = v61 - (uint16)clip_y;
							}
							if (v22 > (uint16)clip_y1)
								v22 = (uint16)clip_y1;
							if (clip_x) {
								if ((uint16)clip_x >= v20)
									return;
								v20 -= (uint16)clip_x;
							}
							if (v20 > (uint16)clip_x1) {
								v32 = v20 - (uint16)clip_x1;
								v29 -= v32;
								v62 = v22;
								v33 = 0;
								while (Asm_Zoom(v33 + 1, a7) < v32)
									;
								v34 = v33;
								v22 = v62;
								spritePixelsP += v34;
								v20 = (uint16)clip_x1;
							}
							do {
								while (1) {
									v63 = v22;
									v53 = v29;
									v46 = spritePixelsP;
									Agr_Flag_x = 0;
									Agr_x = 0;
									v35 = v20;
									do {
										while (1) {
											if (*spritePixelsP)
												*v29 = *spritePixelsP;
											--v29;
											++spritePixelsP;
											if (!Agr_Flag_x)
												Agr_x = a7 + Agr_x;
											if ((uint16)Agr_x < 0x64u)
												break;
											Agr_x = Agr_x - 100;
											--spritePixelsP;
											Agr_Flag_x = 1;
											--v35;
											if (!v35)
												goto R_Aff_Zoom_Larg_Cont1;
										}
										Agr_Flag_x = 0;
										--v35;
									} while (v35);
R_Aff_Zoom_Larg_Cont1:
									spritePixelsP = Largeur + v46;
									v29 = nbrligne2 + v53;
									++Compteur_y;
									if (!Agr_Flag_y)
										Agr_y = a7 + Agr_y;
									if ((uint16)Agr_y < 0x64u)
										break;
									Agr_y = Agr_y - 100;
									spritePixelsP = v46;
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
								if ((uint16)clip_y >= v22)
									return;
								v58 = v22;
								v49 = v20;
								v23 = 0;
								v24 = (uint16)clip_y;
								while (Asm_Zoom(v23 + 1, a7) < v24)
									;
								v20 = v49;
								spritePixelsP += Largeur * v23;
								dest1P += nbrligne2 * (uint16)clip_y;
								v22 = v58 - (uint16)clip_y;
							}
							if (v22 > (uint16)clip_y1)
								v22 = (uint16)clip_y1;
							if (clip_x) {
								if ((uint16)clip_x >= v20)
									return;
								v59 = v22;
								v50 = v20;
								v25 = (uint16)clip_x;
								v26 = 0;
								while (Asm_Zoom(v26 + 1, a7) < v25)
									;
								v27 = v26;
								v22 = v59;
								spritePixelsP += v27;
								dest1P += (uint16)clip_x;
								v20 = v50 - (uint16)clip_x;
							}
							if (v20 > (uint16)clip_x1)
								v20 = (uint16)clip_x1;
							do {
								while (1) {
									v60 = v22;
									v51 = dest1P;
									v45 = spritePixelsP;
									v28 = v20;
									Agr_Flag_x = 0;
									Agr_x = 0;
									do {
										while (1) {
											if (*spritePixelsP)
												*dest1P = *spritePixelsP;
											++dest1P;
											++spritePixelsP;
											if (!Agr_Flag_x)
												Agr_x = a7 + Agr_x;
											if ((uint16)Agr_x < 0x64u)
												break;
											Agr_x = Agr_x - 100;
											--spritePixelsP;
											Agr_Flag_x = 1;
											--v28;
											if (!v28)
												goto Aff_Zoom_Larg_Cont1;
										}
										Agr_Flag_x = 0;
										--v28;
									} while (v28);
Aff_Zoom_Larg_Cont1:
									spritePixelsP = Largeur + v45;
									dest1P = nbrligne2 + v51;
									if (!Agr_Flag_y)
										Agr_y = a7 + Agr_y;
									if ((uint16)Agr_y < 0x64u)
										break;
									Agr_y = Agr_y - 100;
									spritePixelsP = v45;
									Agr_Flag_y = 1;
									v22 = v60 - 1;
									if (v60 == 1)
										return;
								}
								Agr_Flag_y = 0;
								v22 = v60 - 1;
							} while (v60 != 1);
						}
					} else if (a6) {
						Compteur_y = 0;
						Red_x = 0;
						Red_y = 0;
						Largeur = spriteWidth;
						Red = a6;
						if (a6 < 100) {
							v37 = Asm_Reduc(spriteWidth, Red);
							if (a8) {
								v40 = v37 + dest1P;
								do {
									v65 = spriteHeight2;
									v55 = v40;
									Red_y = Red + Red_y;
									if ((uint16)Red_y < 0x64u) {
										Red_x = 0;
										v41 = Largeur;
										v42 = v37;
										do {
											Red_x = Red + Red_x;
											if ((uint16)Red_x < 0x64u) {
												if (v42 >= clip_x && v42 < clip_x1 && *spritePixelsP)
													*v40 = *spritePixelsP;
												--v40;
												++spritePixelsP;
												--v42;
											} else {
												Red_x = Red_x - 100;
												++spritePixelsP;
											}
											--v41;
										} while (v41);
										spriteHeight2 = v65;
										v40 = nbrligne2 + v55;
									} else {
										Red_y = Red_y - 100;
										spritePixelsP += Largeur;
									}
									--spriteHeight2;
								} while (spriteHeight2);
							} else {
								do {
									v64 = spriteHeight2;
									v54 = dest1P;
									Red_y = Red + Red_y;
									if ((uint16)Red_y < 0x64u) {
										Red_x = 0;
										v38 = Largeur;
										v39 = 0;
										do {
											Red_x = Red + Red_x;
											if ((uint16)Red_x < 0x64u) {
												if (v39 >= clip_x && v39 < clip_x1 && *spritePixelsP)
													*dest1P = *spritePixelsP;
												++dest1P;
												++spritePixelsP;
												++v39;
											} else {
												Red_x = Red_x - 100;
												++spritePixelsP;
											}
											--v38;
										} while (v38);
										spriteHeight2 = v64;
										dest1P = nbrligne2 + v54;
									} else {
										Red_y = Red_y - 100;
										spritePixelsP += Largeur;
									}
									--spriteHeight2;
								} while (spriteHeight2);
							}
						}
					} else {
						Largeur = spriteWidth;
						Compteur_y = 0;
						if (a8) {
							dest2P = spriteWidth + dest1P;
							spec_largeur = spriteWidth;
							if (clip_y) {
								if ((uint16)clip_y >= (unsigned int)spriteHeight1)
									return;
								spritePixelsP += spriteWidth * (uint16)clip_y;
								dest2P += nbrligne2 * (uint16)clip_y;
								spriteHeight1 -= (uint16)clip_y;
							}
							xLeft = (uint16)clip_y1;
							if (spriteHeight1 > clip_y1)
								spriteHeight1 = clip_y1;
							xLeft = clip_x;
							if (clip_x) {
								if (xLeft >= spriteWidth)
									return;
								spriteWidth -= xLeft;
							}
							if (spriteWidth > (uint16)clip_x1) {
								clippedWidth = spriteWidth - (uint16)clip_x1;
								spritePixelsP += clippedWidth;
								dest2P -= clippedWidth;
								spriteWidth = (uint16)clip_x1;
							}
							do {
								yCtr2 = spriteHeight1;
								destCopy2P = dest2P;
								spritePixelsCopy2P = spritePixelsP;
								xCtr2 = spriteWidth;
								do {
									if (*spritePixelsP)
										*dest2P = *spritePixelsP;
									++spritePixelsP;
									--dest2P;
									--xCtr2;
								} while (xCtr2);
								spritePixelsP = spec_largeur + spritePixelsCopy2P;
								dest2P = nbrligne2 + destCopy2P;
								spriteHeight1 = yCtr2 - 1;
							} while (yCtr2 != 1);
						} else {
							spec_largeur = spriteWidth;
							if (clip_y) {
								if ((uint16)clip_y >= (unsigned int)spriteHeight1)
									return;
								spritePixelsP += spriteWidth * (uint16)clip_y;
								dest1P += nbrligne2 * (uint16)clip_y;
								spriteHeight1 -= (uint16)clip_y;
							}
							if (spriteHeight1 > clip_y1)
								spriteHeight1 = clip_y1;
							if (clip_x) {
								if ((uint16)clip_x >= spriteWidth)
									return;
								spritePixelsP += (uint16)clip_x;
								dest1P += (uint16)clip_x;
								spriteWidth -= (uint16)clip_x;
							}
							if (spriteWidth > (uint16)clip_x1)
								spriteWidth = (uint16)clip_x1;
							do {
								yCtr1 = spriteHeight1;
								dest1CopyP = dest1P;
								spritePixelsCopyP = spritePixelsP;
								xCtr1 = spriteWidth;
								do {
									if (*spritePixelsP)
										*dest1P = *spritePixelsP;
									++dest1P;
									++spritePixelsP;
									--xCtr1;
								} while (xCtr1);
								spritePixelsP = spec_largeur + spritePixelsCopyP;
								dest1P = nbrligne2 + dest1CopyP;
								spriteHeight1 = yCtr1 - 1;
							} while (yCtr1 != 1);
						}
					}
				}
			}
		}
	}
}

// Display Speed
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
	int xRight; 
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
	xRight = left + croppedWidth;
	if (xRight > max_x)
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

// Display Font
void GraphicsManager::Affiche_Fonte(byte *surface, const byte *spriteData, int xp, int yp, 
									int characterIndex, int colour) {
	const byte *spriteDataP;
	int i; 
	const byte *spriteSizeP; 
	int spriteWidth; 
	int spriteHeight;
	const byte *spritePixelsP;
	byte *destP;
	int xCtr; 
	byte destByte;
	byte *destLineP;
	int yCtr;

	spriteDataP = spriteData + 3;
	for (i = characterIndex; i; --i)
		spriteDataP += READ_LE_UINT32(spriteDataP) + 16;

	spriteWidth = 0;
	spriteHeight = 0;
	spriteSizeP = spriteDataP + 4;
	spriteWidth = (int16)READ_LE_UINT16(spriteSizeP);
	spriteSizeP += 2;
	spriteHeight = (int16)READ_LE_UINT16(spriteSizeP);
	spritePixelsP = spriteSizeP + 10;
	destP = surface + xp + nbrligne2 * yp;
	Largeur = spriteWidth;

	do {
		yCtr = spriteHeight;
		destLineP = destP;
		xCtr = spriteWidth;
		do {
			destByte = *spritePixelsP;
			if (*spritePixelsP) {
				if (destByte == (byte)-4)
					destByte = colour;
				*destP = destByte;
			}
	      
			++destP;
			++spritePixelsP;
			--xCtr;
		} while (xCtr);
		destP = nbrligne2 + destLineP;
		spriteHeight = yCtr - 1;
	} while (yCtr != 1);
}

// Init Screen
void GraphicsManager::INI_ECRAN(const Common::String &file) {
	OPTI_INI(file, 0);
}

// Init Screen 2
void GraphicsManager::INI_ECRAN2(const Common::String &file) {
	OPTI_INI(file, 2);
}

void GraphicsManager::OPTI_INI(const Common::String &file, int mode) {
	int dataOffset; 
	int dataVal1; 
	bool doneFlag; 
	byte *ptr; 
	Common::String filename; 

	dataOffset = 1;
	
	filename = file + ".ini";
	ptr = _vm->_fileManager.RECHERCHE_CAT(filename, 1);
	if (ptr == g_PTRNUL) {
		_vm->_fileManager.CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, filename);
		ptr = _vm->_fileManager.CHARGE_FICHIER(_vm->_globals.NFICHIER);
	}
	if (!mode) {
		filename = file + ".spr";
		if (g_PTRNUL != _vm->_globals.SPRITE_ECRAN)
			_vm->_globals.SPRITE_ECRAN = _vm->_fileManager.LIBERE_FICHIER(_vm->_globals.SPRITE_ECRAN);
		if (!_vm->_globals.NOSPRECRAN) {
			_vm->_globals.SPRITE_ECRAN = _vm->_fileManager.RECHERCHE_CAT(filename, 8);
			if (_vm->_globals.SPRITE_ECRAN) {
				_vm->_globals.CAT_FLAG = false;
				_vm->_fileManager.CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, filename);
			} else {
				_vm->_globals.CAT_FLAG = true;
				_vm->_fileManager.CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, "RES_SLI.RES");
			}
			_vm->_globals.SPRITE_ECRAN = _vm->_fileManager.CHARGE_FICHIER(_vm->_globals.NFICHIER);
			_vm->_globals.CAT_FLAG = false;
		}
	}
	if (*ptr != 'I' || *(ptr + 1) != 'N' || *(ptr + 2) != 'I') {
		error("Error, file not ini");
	} else {
		doneFlag = false;
		do {
			dataVal1 = _vm->_scriptManager.Traduction(ptr + 20 * dataOffset);
			if (_vm->shouldQuit())
				return;

			if (dataVal1 == 2)
				dataOffset =  _vm->_scriptManager.Control_Goto((ptr + 20 * dataOffset));
			if (dataVal1 == 3)
				dataOffset =  _vm->_scriptManager.Control_If(ptr, dataOffset);
			if (dataOffset == -1)
				error("Error, defective IFF");
			if (dataVal1 == 1 || dataVal1 == 4)
				++dataOffset;
			if (!dataVal1 || dataVal1 == 5)
				doneFlag = true;
		} while (!doneFlag);
	}
	_vm->_globals.dos_free2(ptr);
	if (mode != 1) {
		if (g_PTRNUL != _vm->_globals.COUCOU)
			_vm->_globals.COUCOU = _vm->_globals.dos_free2(_vm->_globals.COUCOU);
		
		filename = file + ".rep";
		byte *dataP = _vm->_fileManager.RECHERCHE_CAT(filename, 2);
		_vm->_globals.COUCOU = dataP;
		if (g_PTRNUL == dataP) {
			_vm->_fileManager.CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, filename);
			dataP = _vm->_fileManager.CHARGE_FICHIER(_vm->_globals.NFICHIER);
			_vm->_globals.COUCOU = dataP;
		}
	}
	_vm->_objectsManager.FORCEZONE = 1;
	_vm->_objectsManager.CHANGEVERBE = 0;
}

void GraphicsManager::NB_SCREEN() {
	byte *destP; 
	const byte *srcP;

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
	
	destP = VESA_SCREEN;
	srcP = VESA_BUFFER;
	memcpy(VESA_SCREEN, VESA_BUFFER, 0x95FFCu);
	srcP = srcP + 614396;
	destP = destP + 614396;
	*destP = *srcP;
	*(destP + 2) = *(srcP + 2);
	DD_VBL();
}

void GraphicsManager::SHOW_PALETTE() {
	setpal_vga256(Palette);
}

void GraphicsManager::videkey() {
	// Empty in original
}

void GraphicsManager::Copy_WinScan_Vbe(const byte *src, byte *dest) {
	int result; 
	int destOffset; 
	const byte *srcPtr; 
	byte byteVal; 

	result = 0;
	destOffset = 0;
	srcPtr = src;
	while (1) {
		byteVal = *srcPtr;
		if (*srcPtr < 0xFCu)
			goto Video_Cont_wVbe;
		if (byteVal == (byte)-4)
			return;
		if (byteVal == (byte)-3) {
			destOffset += *(srcPtr + 1);
			byteVal = *(srcPtr + 2);
			srcPtr += 2;
		} else if (byteVal == (byte)-2) {
			destOffset += READ_LE_UINT16(srcPtr + 1);
			byteVal = *(srcPtr + 3);
			srcPtr += 3;
		} else {
			destOffset += READ_LE_UINT32(srcPtr + 1);
			byteVal = *(srcPtr + 5);
			srcPtr += 5;
		}
Video_Cont_wVbe:
		*(dest + destOffset) = byteVal;
		++srcPtr;
		++destOffset;
	}
}

void GraphicsManager::Copy_Video_Vbe(const byte *src) {
	int destOffset; 
	const byte *srcP; 
	byte byteVal; 
	
	assert(VideoPtr);
	destOffset = 0;
	srcP = src;
	while (1) {
		byteVal = *srcP;
		if (*srcP < 0xFCu)
			goto Video_Cont_Vbe;
		if (byteVal == (byte)-4)
			return;
		if (byteVal == (byte)-3) {
			destOffset += *(srcP + 1);
			byteVal = *(srcP + 2);
			srcP += 2;
		} else if (byteVal == (byte)-2) {
			destOffset += READ_LE_UINT16(srcP + 1);
			byteVal = *(srcP + 3);
			srcP += 3;
		} else {
			destOffset += READ_LE_UINT32(srcP + 1);
			byteVal = *(srcP + 5);
			srcP += 5;
		}
Video_Cont_Vbe:
		*((byte *)VideoPtr->pixels + destOffset) = byteVal;
		++srcP;
		++destOffset;
	}
}

// Reduce Screen
void GraphicsManager::Reduc_Ecran(const byte *srcSurface, byte *destSurface, int xp, int yp, int width, int height, int zoom) {
	const byte *srcP; 
	byte *destP; 
	const byte *lineSrcP;

	srcP = xp + nbrligne2 * yp + srcSurface;
	destP = destSurface;
	Red = zoom;
	Largeur = width;
	Red_x = 0;
	Red_y = 0;
	if (zoom < 100) {
		Reduc_Ecran_L = Asm_Reduc(width, Red);
		Reduc_Ecran_H = Asm_Reduc(height, Red);

		for (int yCtr = 0; yCtr < height; ++yCtr, srcP += nbrligne2) {
			Red_y += Red;
			if (Red_y < 100) {
				Red_x = 0;
				lineSrcP = srcP;

				for (int xCtr = 0; xCtr < Largeur; ++xCtr) {
					Red_x += Red;
					if (Red_x < 100) {
						*destP++ = *lineSrcP++;
					} else {
						Red_x -= 100;
						++lineSrcP;
					}
				}
			} else {
				Red_y -= 100;
			}
		}
	}
}

void GraphicsManager::Plot_Hline(byte *surface, int xp, int yp, unsigned int width, byte col) {
	memset(surface + xp + nbrligne2 * yp, col, width);
}

void GraphicsManager::Plot_Vline(byte *surface, int xp, int yp, int height, byte col) {
	byte *destP;
	int yCtr;

	destP = surface + xp + nbrligne2 * yp;
	yCtr = height;
	do {
		*destP = col;
		destP += nbrligne2;
		--yCtr;
	} while (yCtr);
}


} // End of namespace Hopkins
