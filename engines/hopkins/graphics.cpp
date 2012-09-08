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
#include "common/rect.h"
#include "engines/util.h"
#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"

namespace Hopkins {

byte *ObjectManager::CHANGE_OBJET(int objIndex) {
	byte *result = ObjectManager::CAPTURE_OBJET(objIndex, 1);
	GLOBALS.Bufferobjet = result;
	GLOBALS.Nouv_objet = 1;
	GLOBALS.OBJET_EN_COURS = objIndex;
	return result;
}

byte *ObjectManager::CAPTURE_OBJET(int objIndex, int mode) {
	byte *result = NULL;
	byte *dataP;

	dataP = 0;
	int v2 = GLOBALS.ObjetW[objIndex].field0;
	int v3 = GLOBALS.ObjetW[objIndex].field1;

	if (mode == 1)
	    ++v3;
	if (v2 != GLOBALS.NUM_FICHIER_OBJ) {
		if (GLOBALS.ADR_FICHIER_OBJ != PTRNUL)
			ObjectManager::DEL_FICHIER_OBJ();
		if (v2 == 1) {
			FileManager::CONSTRUIT_SYSTEM("OBJET1.SPR");
			GLOBALS.ADR_FICHIER_OBJ = ObjectManager::CHARGE_SPRITE(GLOBALS.NFICHIER);
		}
		GLOBALS.NUM_FICHIER_OBJ = v2;
	}

	int width = ObjectManager::Get_Largeur(GLOBALS.ADR_FICHIER_OBJ, v3);
	int height = ObjectManager::Get_Hauteur(GLOBALS.ADR_FICHIER_OBJ, v3);
	GLOBALS.OBJL = width;
	GLOBALS.OBJH = height;

	switch (mode) {
	case 0:
		dataP = GLOBALS.dos_malloc2(height * width);
		if (dataP == PTRNUL)
			error("CAPTURE_OBJET");
			
		ObjectManager::capture_mem_sprite(GLOBALS.ADR_FICHIER_OBJ, dataP, v3);
		break;

	case 1:
		ObjectManager::sprite_alone(GLOBALS.ADR_FICHIER_OBJ, GLOBALS.Bufferobjet, v3);
		result = GLOBALS.Bufferobjet;
		break;

	case 3:
		ObjectManager::capture_mem_sprite(GLOBALS.ADR_FICHIER_OBJ, GLOBALS.INVENTAIRE_OBJET, v3);
		result = GLOBALS.INVENTAIRE_OBJET;
		break;

	default:
		result = dataP;
		break;
	}

	return result;
}

int ObjectManager::Get_Largeur(const byte *objectData, int objIndex) {
	const byte *objP = objectData + 3;
	for (int i = objIndex; i; --i)
		objP += READ_LE_UINT32(objP) + 16;

	return READ_LE_UINT16(objP + 4);
}

int ObjectManager::Get_Hauteur(const byte *objectData, int objIndex) {
	const byte *objP = objectData + 3;
	for (int i = objIndex; i; --i)
		objP += READ_LE_UINT32(objP) + 16;

	return READ_LE_UINT16(objP + 6);
}

int ObjectManager::sprite_alone(const byte *objectData, byte *sprite, int objIndex) {
	const byte *objP = objectData + 3;
	for (int i = objIndex; i; --i) {
		objP += READ_LE_UINT32(objP) + 16;
	}

	objP += 4;
	int result = READ_LE_UINT16(objP) * READ_LE_UINT16(objP + 2);

	memcpy(sprite + 3, objP - 4, result + 16);
	return result;
}

byte *ObjectManager::DEL_FICHIER_OBJ() {
	GLOBALS.NUM_FICHIER_OBJ = 0;
	if (GLOBALS.ADR_FICHIER_OBJ != PTRNUL)
		GLOBALS.ADR_FICHIER_OBJ = FileManager::LIBERE_FICHIER(GLOBALS.ADR_FICHIER_OBJ);
  
	byte *result = PTRNUL;
	GLOBALS.ADR_FICHIER_OBJ = PTRNUL;
	return result;
}

byte *ObjectManager::CHARGE_SPRITE(const Common::String &file) {
	FileManager::DMESS1();
	return FileManager::CHARGE_FICHIER(file);
}

int ObjectManager::capture_mem_sprite(const byte *objectData, byte *sprite, int objIndex) {
	const byte *objP = objectData + 3;
	for (int i = objIndex; i; --i) {
		objP += READ_LE_UINT32(objP) + 16;
	}

	objP += 4;
	int result = READ_LE_UINT16(objP) * READ_LE_UINT16(objP + 2);

	memcpy(sprite, objP + 12, result);
	return result;
}

int ObjectManager::AJOUTE_OBJET(int objIndex) {
	bool flag = false;
	int arrIndex = 0;
	do {
		++arrIndex;
		if (!GLOBALS.INVENTAIRE[arrIndex])
			flag = true;
		if (arrIndex == 32)
			flag = true;
	} while (!flag);
  
	GLOBALS.INVENTAIRE[arrIndex] = objIndex;
	return arrIndex;
}

/*------------------------------------------------------------------------*/

GraphicsManager::GraphicsManager() {
	SDL_MODEYES = false;
}

GraphicsManager::~GraphicsManager() {
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
			error("TODO: Implement zooming support"); 
			//width = Reel_Zoom(a1, SDL_ECHELLE);
			//height = Reel_Zoom(a2, SDL_ECHELLE);
		}

		if (bpp == 8) {
			initGraphics(width, height, true);
		} else {
			Graphics::PixelFormat pixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0);
			initGraphics(width, height, true, &pixelFormat);
		}

		VideoPtr = NULL;
		XSCREEN = width;
		YSCREEN = height;

		Linear = true;
		Winbpp = bpp;
		WinScan = width;

		PAL_PIXELS = SD_PIXELS;
		nbrligne = width;

		for (int idx = 0; idx < 256; ++idx) {
			cmap[idx].r = cmap[idx].g = cmap[idx].b = 0;
		}

		SDL_MODEYES = true;
	} else {
		error("Called SET_MODE multiple times");
	}
}

void GraphicsManager::DD_Lock() {
	VideoPtr = g_system->lockScreen();
}

void GraphicsManager::DD_Unlock() {
	g_system->unlockScreen();
}

void GraphicsManager::Cls_Video() {
	assert(VideoPtr);

	VideoPtr->fillRect(Common::Rect(0, 0, XSCREEN, YSCREEN), 0);
}

} // End of namespace Hopkins
