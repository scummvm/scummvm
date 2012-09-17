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
#include "hopkins/hopkins.h"

namespace Hopkins {

byte *ObjectsManager::CHANGE_OBJET(int objIndex) {
	byte *result = ObjectsManager::CAPTURE_OBJET(objIndex, 1);
	GLOBALS.Bufferobjet = result;
	GLOBALS.Nouv_objet = 1;
	GLOBALS.OBJET_EN_COURS = objIndex;
	return result;
}

byte *ObjectsManager::CAPTURE_OBJET(int objIndex, int mode) {
	byte *result = NULL;
	byte *dataP;

	dataP = 0;
	int val1 = GLOBALS.ObjetW[objIndex].field0;
	int val2 = GLOBALS.ObjetW[objIndex].field1;

	if (mode == 1)
	    ++val2;
	if (val1 != GLOBALS.NUM_FICHIER_OBJ) {
		if (GLOBALS.ADR_FICHIER_OBJ != PTRNUL)
			ObjectsManager::DEL_FICHIER_OBJ();
		if (val1 == 1) {
			FileManager::CONSTRUIT_SYSTEM("OBJET1.SPR");
			GLOBALS.ADR_FICHIER_OBJ = ObjectsManager::CHARGE_SPRITE(GLOBALS.NFICHIER);
		}
		GLOBALS.NUM_FICHIER_OBJ = val1;
	}

	int width = ObjectsManager::Get_Largeur(GLOBALS.ADR_FICHIER_OBJ, val2);
	int height = ObjectsManager::Get_Hauteur(GLOBALS.ADR_FICHIER_OBJ, val2);
	GLOBALS.OBJL = width;
	GLOBALS.OBJH = height;

	switch (mode) {
	case 0:
		dataP = GLOBALS.dos_malloc2(height * width);
		if (dataP == PTRNUL)
			error("CAPTURE_OBJET");
			
		ObjectsManager::capture_mem_sprite(GLOBALS.ADR_FICHIER_OBJ, dataP, val2);
		break;

	case 1:
		ObjectsManager::sprite_alone(GLOBALS.ADR_FICHIER_OBJ, GLOBALS.Bufferobjet, val2);
		result = GLOBALS.Bufferobjet;
		break;

	case 3:
		ObjectsManager::capture_mem_sprite(GLOBALS.ADR_FICHIER_OBJ, GLOBALS.INVENTAIRE_OBJET, val2);
		result = GLOBALS.INVENTAIRE_OBJET;
		break;

	default:
		result = dataP;
		break;
	}

	return result;
}

int ObjectsManager::Get_Largeur(const byte *objectData, int objIndex) {
	const byte *objP = objectData + 3;
	for (int i = objIndex; i; --i)
		objP += READ_LE_UINT32(objP) + 16;

	return READ_LE_UINT16(objP + 4);
}

int ObjectsManager::Get_Hauteur(const byte *objectData, int objIndex) {
	const byte *objP = objectData + 3;
	for (int i = objIndex; i; --i)
		objP += READ_LE_UINT32(objP) + 16;

	return READ_LE_UINT16(objP + 6);
}

int ObjectsManager::sprite_alone(const byte *objectData, byte *sprite, int objIndex) {
	const byte *objP = objectData + 3;
	for (int i = objIndex; i; --i) {
		objP += READ_LE_UINT32(objP) + 16;
	}

	objP += 4;
	int result = READ_LE_UINT16(objP) * READ_LE_UINT16(objP + 2);

	memcpy(sprite + 3, objP - 4, result + 16);
	return result;
}

byte *ObjectsManager::DEL_FICHIER_OBJ() {
	GLOBALS.NUM_FICHIER_OBJ = 0;
	if (GLOBALS.ADR_FICHIER_OBJ != PTRNUL)
		GLOBALS.ADR_FICHIER_OBJ = FileManager::LIBERE_FICHIER(GLOBALS.ADR_FICHIER_OBJ);
  
	byte *result = PTRNUL;
	GLOBALS.ADR_FICHIER_OBJ = PTRNUL;
	return result;
}

byte *ObjectsManager::CHARGE_SPRITE(const Common::String &file) {
	FileManager::DMESS1();
	return FileManager::CHARGE_FICHIER(file);
}

int ObjectsManager::capture_mem_sprite(const byte *objectData, byte *sprite, int objIndex) {
	const byte *objP = objectData + 3;
	for (int i = objIndex; i; --i) {
		objP += READ_LE_UINT32(objP) + 16;
	}

	objP += 4;
	int result = READ_LE_UINT16(objP) * READ_LE_UINT16(objP + 2);

	memcpy(sprite, objP + 12, result);
	return result;
}

int ObjectsManager::AJOUTE_OBJET(int objIndex) {
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

} // End of namespace Hopkins
