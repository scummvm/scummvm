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
#include "hopkins/objects.h"
#include "hopkins/dialogs.h"
#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/hopkins.h"

namespace Hopkins {

ObjectsManager::ObjectsManager() {
	PRIORITY = 0;
	inventairex = inventairey = 0;
	inventairel = inventaireh = 0;
	old_cadx = old_cady = old_cadi = 0;
	cadx = cady = cadi = 0;
	SL_X = SL_Y = 0;
	I_old_x = I_old_y = 0;
	FLAG_VISIBLE_EFFACE = 0;
	_vm->_globals.Winventaire = PTRNUL;
	inventaire2 = PTRNUL;
	SL_SPR = PTRNUL;
	SL_SPR2 = PTRNUL;
	PERSO_ON = false;
	SL_FLAG = false;
	FLAG_VISIBLE = false;
	DESACTIVE_INVENT = false;
	BOBTOUS = false;
}

void ObjectsManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
}

byte *ObjectsManager::CHANGE_OBJET(int objIndex) {
	byte *result = ObjectsManager::CAPTURE_OBJET(objIndex, 1);
	_vm->_globals.Bufferobjet = result;
	_vm->_globals.Nouv_objet = 1;
	_vm->_globals.OBJET_EN_COURS = objIndex;
	return result;
}

byte *ObjectsManager::CAPTURE_OBJET(int objIndex, int mode) {
	byte *result = NULL;
	byte *dataP;

	dataP = 0;
	int val1 = _vm->_globals.ObjetW[objIndex].field0;
	int val2 = _vm->_globals.ObjetW[objIndex].field1;

	if (mode == 1)
	    ++val2;
	if (val1 != _vm->_globals.NUM_FICHIER_OBJ) {
		if (_vm->_globals.ADR_FICHIER_OBJ != PTRNUL)
			ObjectsManager::DEL_FICHIER_OBJ();
		if (val1 == 1) {
			FileManager::CONSTRUIT_SYSTEM("OBJET1.SPR");
			_vm->_globals.ADR_FICHIER_OBJ = ObjectsManager::CHARGE_SPRITE(_vm->_globals.NFICHIER);
		}
		_vm->_globals.NUM_FICHIER_OBJ = val1;
	}

	int width = ObjectsManager::Get_Largeur(_vm->_globals.ADR_FICHIER_OBJ, val2);
	int height = ObjectsManager::Get_Hauteur(_vm->_globals.ADR_FICHIER_OBJ, val2);
	_vm->_globals.OBJL = width;
	_vm->_globals.OBJH = height;

	switch (mode) {
	case 0:
		dataP = _vm->_globals.dos_malloc2(height * width);
		if (dataP == PTRNUL)
			error("CAPTURE_OBJET");
			
		ObjectsManager::capture_mem_sprite(_vm->_globals.ADR_FICHIER_OBJ, dataP, val2);
		break;

	case 1:
		ObjectsManager::sprite_alone(_vm->_globals.ADR_FICHIER_OBJ, _vm->_globals.Bufferobjet, val2);
		result = _vm->_globals.Bufferobjet;
		break;

	case 3:
		ObjectsManager::capture_mem_sprite(_vm->_globals.ADR_FICHIER_OBJ, _vm->_globals.INVENTAIRE_OBJET, val2);
		result = _vm->_globals.INVENTAIRE_OBJET;
		break;

	default:
		result = dataP;
		break;
	}

	return result;
}

void ObjectsManager::DELETE_OBJET(int objIndex) {
	int v1;
	int v2; 
	int i; 

	v1 = 0;
	v2 = 0;
	do {
		++v2;
		if (_vm->_globals.INVENTAIRE[v2] == objIndex)
			v1 = 1;
		if (v2 > 32)
			v1 = 1;
	} while (v1 != 1);
	if (v2 <= 32) {
		if (v2 == 32) {
			_vm->_globals.INVENTAIRE[32] = 0;
		} else {
			for (i = v2; i < 32; ++i)
				_vm->_globals.INVENTAIRE[i] = _vm->_globals.INVENTAIRE[i + 1];
		}
	}
	CHANGE_OBJET(14);

}

void ObjectsManager::set_offsetxy(byte *data, int idx, int xp, int yp, bool isSize) {
	byte *startP = data + 3;
	for (int i = idx; i; --i)
		startP += READ_LE_UINT32(startP) + 16;
	
	byte *rectP = startP + 8;
	if (isSize == 1) {
		// Set size
		byte *pointP = rectP + 4;
		WRITE_LE_UINT16(pointP, xp);
		WRITE_LE_UINT16(pointP + 2, yp);
	} else {
		// Set position
		WRITE_LE_UINT16(rectP, xp);
		WRITE_LE_UINT16(rectP + 2, yp);
	}
}

int ObjectsManager::get_offsetx(const byte *spriteData, int spriteIndex, bool isSize) {
	const byte *v3 = spriteData + 3;
	for (int i = spriteIndex; i; --i)
		v3 += READ_LE_UINT16(v3) + 16;
  
	const byte *v5 = v3 + 8;
	int result = READ_LE_UINT16(v5);
	if (isSize)
		result = READ_LE_UINT16(v5 + 4);

	return result;
}

int ObjectsManager::get_offsety(const byte *spriteData, int spriteIndex, bool isSize) {
	const byte *v3 = spriteData + 3;
	for (int i = spriteIndex; i; --i)
		v3 += READ_LE_UINT16(v3) + 16;
  
	const byte *v5 = v3 + 10;
	int result = READ_LE_UINT16(v5);
	if (isSize)
		result = READ_LE_UINT16(v5 + 4);

	return result;
}

int ObjectsManager::Get_Largeur(const byte *objectData, int idx) {
	const byte *rectP = objectData + 3;
	for (int i = idx; i; --i)
		rectP += READ_LE_UINT32(rectP) + 16;
	
	return (int16)READ_LE_UINT16(rectP + 4);
}

int ObjectsManager::Get_Hauteur(const byte *objectData, int idx) {
	const byte *rectP = objectData + 3;
	for (int i = idx; i; --i)
		rectP += READ_LE_UINT32(rectP) + 16;
	
	return (int16)READ_LE_UINT16(rectP + 6);
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
	_vm->_globals.NUM_FICHIER_OBJ = 0;
	if (_vm->_globals.ADR_FICHIER_OBJ != PTRNUL)
		_vm->_globals.ADR_FICHIER_OBJ = FileManager::LIBERE_FICHIER(_vm->_globals.ADR_FICHIER_OBJ);
  
	byte *result = PTRNUL;
	_vm->_globals.ADR_FICHIER_OBJ = PTRNUL;
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
		if (!_vm->_globals.INVENTAIRE[arrIndex])
			flag = true;
		if (arrIndex == 32)
			flag = true;
	} while (!flag);
  
	_vm->_globals.INVENTAIRE[arrIndex] = objIndex;
	return arrIndex;
}

void ObjectsManager::AFF_SPRITES() {
	int v0; 
	int v1;
	int v2;
	int destX;
	int destY;
	int v5; 
	int v6;
	int v7;
	int v8;
	int v9;
	int v10; 
	int v11;
	uint16 *v12;
	int v13; 
	int v14; 
	int v15; 
	int v16; 
	int v17; 
	int v18; 
	int v19; 
	int v20;
	int v21;
	int v23;
	int v24;
	int v25;
	int v26;
	int v27;
	int v28;
	int v29;
	int v30;
	int v31;
	int v32;
	int v33;
	signed int v34;
	signed int v35;
	signed int v36;
	int v37; 
	int v38; 
	uint16 arr[50];

	_vm->_globals.NBTRI = 0;
	v30 = 0;
	do {
		v0 = v30;
		if (_vm->_fontManager.ListeTxt[v0].field0 && _vm->_fontManager.Txt[v30].field3FC != 2) {
			v1 = _vm->_fontManager.ListeTxt[v30].x1;
			v28 = v1 - 2;
      
			if ((int16)(v1 - 2) < _vm->_graphicsManager.min_x)
				v28 = _vm->_graphicsManager.min_x;
			v2 = _vm->_fontManager.ListeTxt[v30].y1;
			v23 = v2 - 2;
      
			if ((int16)(v2 - 2) < _vm->_graphicsManager.min_y)
				v23 = _vm->_graphicsManager.min_y;
			destX = v1 - 2;
			if (destX < _vm->_graphicsManager.min_x)
				destX = _vm->_graphicsManager.min_x;
			destY = v2 - 2;
			if (destY < _vm->_graphicsManager.min_y)
				destY = _vm->_graphicsManager.min_y;
			
			_vm->_graphicsManager.SCOPY(_vm->_graphicsManager.VESA_SCREEN, v28, v23,
				_vm->_fontManager.ListeTxt[v30].x2 + 4, _vm->_fontManager.ListeTxt[destX].y2 + 4,
				_vm->_graphicsManager.VESA_BUFFER,
				destX, destY);
			_vm->_fontManager.ListeTxt[v0].field0 = 0;
		}
		++v30;
	} while (v30 <= 10);

	if (!PERSO_ON) {
		v31 = 0;
		do {
			v5 = v31;
			if (_vm->_globals.Liste[v5].field0) {
				v6 = _vm->_globals.Liste[v5].field2;
				v29 = v6 - 2;
				if ((int16)(v6 - 2) < _vm->_graphicsManager.min_x)
					v29 = _vm->_graphicsManager.min_x;
				v7 = _vm->_globals.Liste[v31].field4;
				v24 = v7 - 2;
				if ((int16)(v7 - 2) < _vm->_graphicsManager.min_y)
					v24 = _vm->_graphicsManager.min_y;
				v8 = v6 - 2;
				if (v8 < _vm->_graphicsManager.min_x)
					v8 = _vm->_graphicsManager.min_x;
				v9 = v7 - 2;
				if (v9 < _vm->_graphicsManager.min_y)
					v9 = _vm->_graphicsManager.min_y;
	        
				_vm->_graphicsManager.SCOPY(_vm->_graphicsManager.VESA_SCREEN, v29, v24, 
					_vm->_globals.Liste[v31].width + 4, _vm->_globals.Liste[v31].height + 4, 
					_vm->_graphicsManager.VESA_BUFFER, v8, v9);
				_vm->_globals.Liste[v5].field0 = 0;
			}
			++v31;
		} while (v31 <= 4);
	}
  
	AFF_BOB_ANIM();
	AFF_VBOB();
	if (!PERSO_ON) {
		v32 = 0;
		do {
			_vm->_globals.Liste[v32].field0 = 0;
			v10 = v32;
			if (Sprite[v10].field0 == 1) {
				CALCUL_SPRITE(v32);
				if (Sprite[v10].field2A == 1)
					AvantTri(2, v32, Sprite[v10].field32 + Sprite[v10].field2E);
			}
			++v32;
		} while (v32 <= 4);
    
		if (_vm->_globals.CACHEFLAG)
			VERIFCACHE();
	}
  
	if (PRIORITY == 1 && _vm->_globals.NBTRI) {
		v33 = 1;
		do {
			arr[v33] = v33;
			++v33;
		} while (v33 <= 48);
    
		v25 = _vm->_globals.NBTRI;
		do {
			v27 = 0;
			v34 = 1;
			if (v25 > 1) {
				v26 = _vm->_globals.NBTRI;

				do {
					v11 = arr[v34];
					v12 = &arr[v34 + 1];
					if (_vm->_globals.Tri[arr[v34]].field4 > _vm->_globals.Tri[*v12].field4) {
						arr[v34] = *v12;
						*v12 = v11;
						++v27;
					}
					++v34;
				} while (v34 < v26);
			}
		} while (v27);
    
		v35 = 1;
		if (_vm->_globals.NBTRI + 1 > 1) {
			do {
				v13 = arr[v35];
				if (_vm->_globals.Tri[v13].field0 == 1)
					DEF_BOB(_vm->_globals.Tri[v13].field2);
				if (_vm->_globals.Tri[v13].field0 == 2)
					DEF_SPRITE(_vm->_globals.Tri[v13].field2);
				if (_vm->_globals.Tri[v13].field0 == 3)
					DEF_CACHE(_vm->_globals.Tri[v13].field2);
				_vm->_globals.Tri[v13].field0 = 0;
				++v35;
			} while (v35 < _vm->_globals.NBTRI + 1);
		}
	} else {
		v36 = 1;
		if (_vm->_globals.NBTRI + 1 > 1) {
			do {
				v14 = v36;
				if (_vm->_globals.Tri[v14].field0 == 1)
					DEF_BOB(_vm->_globals.Tri[v14].field2);
				if (_vm->_globals.Tri[v14].field0 == 2)
					DEF_SPRITE(_vm->_globals.Tri[v14].field2);
				if (_vm->_globals.Tri[v14].field0 == 3)
					DEF_CACHE(_vm->_globals.Tri[v14].field2);
				_vm->_globals.Tri[v14].field0 = 0;
				++v36;
			} while (v36 < _vm->_globals.NBTRI + 1);
		}
	}

	v37 = 0;
	do {
		v15 = v37;
		_vm->_globals.Tri[v15].field0 = 0;
		_vm->_globals.Tri[v15].field4 = 0;
		_vm->_globals.Tri[v15].field2 = 0;
		_vm->_globals.Tri[v15].field6 = 0;
		++v37;
	} while (v37 <= 49);
  
	_vm->_globals.NBTRI = 0;
	if (_vm->_globals.AFFINVEN == 1) {
		_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.Winventaire, inventairex, inventairey, inventairel, inventaireh);
		if (old_cadx && old_cady)
			_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager.VESA_BUFFER, inventaire2, old_cadx + 300, old_cady + 300, old_cadi + 1);
		if (cadx && cady)
			_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager.VESA_BUFFER, inventaire2, cadx + 300, cady + 300, cadi);
		_vm->_graphicsManager.Ajoute_Segment_Vesa(inventairex, inventairey, inventairex + inventairel, inventairey + inventaireh);
	}
  
	if (SL_FLAG == 1) {
		_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager.VESA_BUFFER, SL_SPR, _vm->_eventsManager.start_x + 183, 60, 0x112u, 353);
		if (SL_X && SL_Y)
			_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager.VESA_BUFFER, SL_SPR2, SL_X + _vm->_eventsManager.start_x + 300, SL_Y + 300, 0);
    
		_vm->_graphicsManager.Ajoute_Segment_Vesa(_vm->_eventsManager.start_x + 183, 60, _vm->_eventsManager.start_x + 457, 413);
	}
  
	if (_vm->_globals.OPTION_FLAG) {
		_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.OPTION_SPR, 
			_vm->_eventsManager.start_x + 464, 407, 0);
		_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.OPTION_SPR, 
			_vm->_eventsManager.start_x + 657, 556, _vm->_globals.opt_vitesse);
		_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.OPTION_SPR, 
			_vm->_eventsManager.start_x + 731, 495, _vm->_globals.opt_txt);
		_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.OPTION_SPR, 
			_vm->_eventsManager.start_x + 731, 468, _vm->_globals.opt_voice);
		_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.OPTION_SPR, 
			_vm->_eventsManager.start_x + 731, 441, _vm->_globals.opt_sound);
		_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.OPTION_SPR, 
			_vm->_eventsManager.start_x + 731, 414, _vm->_globals.opt_music);
		_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.OPTION_SPR, 
			_vm->_eventsManager.start_x + 600, 522, _vm->_globals.opt_anm);
		_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.OPTION_SPR, 
			_vm->_eventsManager.start_x + 611, 502, _vm->_globals.opt_scrspeed);
		_vm->_graphicsManager.Ajoute_Segment_Vesa(_vm->_eventsManager.start_x + 164, 107, _vm->_eventsManager.start_x + 498, 320);
	}

	v38 = 0;
	do {
		v16 = v38;
		if (_vm->_fontManager.Txt[v16].field0 == 1) {
			if ((uint16)(_vm->_fontManager.Txt[v16].field3FC - 2) > 1)
				_vm->_fontManager.BOITE(v38,
					_vm->_fontManager.Txt[v16].fieldC, _vm->_fontManager.Txt[v16].field4,
					_vm->_eventsManager.start_x + _vm->_fontManager.Txt[v16].field8, _vm->_fontManager.Txt[v16].fieldA);
			else
				_vm->_fontManager.BOITE(
				  v38,
				  _vm->_fontManager.Txt[v16].fieldC,
				  _vm->_fontManager.Txt[v16].field4,
				  _vm->_fontManager.Txt[v16].field8,
				  _vm->_fontManager.Txt[v16].fieldA);
			_vm->_fontManager.ListeTxt[v38].field0 = 1;
			
			v17 = v38;
			if ((uint16)(_vm->_fontManager.Txt[v17].field3FC - 2) > 1)
				_vm->_fontManager.ListeTxt[v38].x1 = _vm->_eventsManager.start_x + _vm->_fontManager.Txt[v17].field8;
			else
				_vm->_fontManager.ListeTxt[v38].x1 = _vm->_fontManager.Txt[v17].field8;
      
			v18 = v38;
			v19 = v38;
			_vm->_fontManager.ListeTxt[v18].y1 = _vm->_fontManager.Txt[v19].fieldA;
			_vm->_fontManager.ListeTxt[v18].x2 = _vm->_fontManager.Txt[v19].field404;
			_vm->_fontManager.ListeTxt[v18].y2 = _vm->_fontManager.Txt[v19].field406;

			if (_vm->_fontManager.ListeTxt[v38].x1 < _vm->_graphicsManager.min_x)
				_vm->_fontManager.ListeTxt[v38].x1 = _vm->_graphicsManager.min_x - 1;
			if (_vm->_fontManager.ListeTxt[v38].y1 < _vm->_graphicsManager.min_y)
				_vm->_fontManager.ListeTxt[v38].y1 = _vm->_graphicsManager.min_y - 1;
			v20 = _vm->_fontManager.ListeTxt[v38].x1;
			if (_vm->_fontManager.ListeTxt[v38].x2 + v20 > _vm->_graphicsManager.max_x)
				_vm->_fontManager.ListeTxt[v38].x2 = _vm->_graphicsManager.max_x - v20;
			v21 = _vm->_fontManager.ListeTxt[v38].y1;
			if (_vm->_fontManager.ListeTxt[v38].y2 + v21 > _vm->_graphicsManager.max_y)
				_vm->_fontManager.ListeTxt[v38].y2 = _vm->_graphicsManager.max_y - v21;
			if (_vm->_fontManager.ListeTxt[v38].x2 <= 0 || _vm->_fontManager.ListeTxt[v38].y2 <= 0)
				_vm->_fontManager.ListeTxt[v18].field0 = 0;
		}
		++v38;
	} while (v38 <= 10);
  
	INVENT_ANIM();
}

void ObjectsManager::INIT_BOB() {
	for (int idx = 0; idx < 35; ++idx) {
		BOB_ZERO(idx);
	}
}

void ObjectsManager::BOB_ZERO(int idx) {
	BobItem &bob = _vm->_globals.Bob[idx];
	Liste2Item &item = _vm->_globals.Liste2[idx];

	bob.field0 = 0;
	bob.field4 = PTRNUL;
	bob.field8 = 0;
	bob.fieldA = 0;
	bob.fieldC = 0;
	bob.fieldE = 0;
	bob.field10 = 0;
	bob.field12 = 0;
	bob.field14 = 0;
	bob.field16 = 0;
	bob.field18 = PTRNUL;
	bob.field1A = 0;
	bob.field1C = 0;
	bob.field1E = 0;
	bob.field20 = 0;
	bob.field22 = 0;
	bob.field24 = 0;
	bob.field26 = 0;
	bob.field28 = 0;
	bob.field2A = 0;
	bob.field2C = 0;
	bob.field30 = PTRNUL;
	bob.field34 = 0;
	bob.field36 = 0;
	bob.field38 = 0;
	bob.field46 = 0;

	item.field0 = 0;
	item.field2 = 0;
	item.field4 = 0;
	item.width = 0;
	item.height = 0;
}

void ObjectsManager::DEF_BOB(int idx) {
	int v2;
	int v3;
	int v4;
	int v5;
	int v6;
	int v7;
	int v8;
	int v9;
	int v10;
	int v11; 
	int v12;
	int v13;

	v2 = idx;
	if (_vm->_globals.Bob[v2].field40) {
		v12 = _vm->_globals.Bob[v2].field42;
		v13 = _vm->_globals.Bob[v2].field44;
		if (_vm->_globals.Bob[v2].field3E)
			_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.Bob[v2].field4, 
				v12 + 300, v13 + 300, _vm->_globals.Bob[v2].fieldC);
		else
			_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager.VESA_BUFFER, 
				_vm->_globals.Bob[v2].field4, v12 + 300, 
				_vm->_globals.Bob[v2].field44 + 300, _vm->_globals.Bob[v2].fieldC,
				_vm->_globals.Bob[v2].field4A, _vm->_globals.Bob[v2].field48,
				_vm->_globals.Bob[v2].field38);
    
		v3 = idx;
		_vm->_globals.Liste2[v3].field0 = 1;
		_vm->_globals.Liste2[v3].field2 = v12;
		_vm->_globals.Liste2[v3].field4 = v13;
		
		v4 = idx;
		_vm->_globals.Liste2[v3].width = _vm->_globals.Bob[v4].field46;
		_vm->_globals.Liste2[v3].height = _vm->_globals.Bob[v4].field48;
		v5 = _vm->_globals.Liste2[v3].field2;
    
		v6 = _vm->_graphicsManager.min_x;
		if (v5 < _vm->_graphicsManager.min_x) {
			_vm->_globals.Liste2[v3].width -= _vm->_graphicsManager.min_x - v5;
			_vm->_globals.Liste2[v3].field2 = v6;
		}
    
		v7 = _vm->_globals.Liste2[v3].field4;
		v8 = _vm->_graphicsManager.min_y;
		if (v7 < _vm->_graphicsManager.min_y) {
			_vm->_globals.Liste2[v3].height -= _vm->_graphicsManager.min_y - v7;
			_vm->_globals.Liste2[v3].field4 = v8;
		}
    
		v9 = _vm->_globals.Liste2[v3].field2;
		if (_vm->_globals.Liste2[v3].width + v9 > _vm->_graphicsManager.max_x)
			_vm->_globals.Liste2[v3].width = _vm->_graphicsManager.max_x - v9;
		v10 = _vm->_globals.Liste2[v3].field4;
		if (_vm->_globals.Liste2[v3].height + v10 > _vm->_graphicsManager.max_y)
			_vm->_globals.Liste2[v3].height = _vm->_graphicsManager.max_y - v10;
    
		if (_vm->_globals.Liste2[v3].width <= 0 || _vm->_globals.Liste2[v3].height <= 0)
		_vm->_globals.Liste2[v3].field0 = 0;

		v11 = idx;
		if (_vm->_globals.Liste2[v11].field0 == 1)
			_vm->_graphicsManager.Ajoute_Segment_Vesa(
                 _vm->_globals.Liste2[v11].field2,
                 _vm->_globals.Liste2[v11].field4,
                 _vm->_globals.Liste2[v11].field2 + _vm->_globals.Liste2[v11].width,
                 _vm->_globals.Liste2[v11].field4 + _vm->_globals.Liste2[v11].height);
	}
}

void ObjectsManager::BOB_VISU(int idx) {
	int v1;
	const byte *data;
	int16 v6;
	int16 v7;
	int16 v8;
	int16 v9;

	PRIORITY = 1;

	if (!_vm->_globals.Bob[idx].field0) {
		BOB_ZERO(idx);

		data = _vm->_globals.Bqe_Anim[idx].data;
		v1 = READ_LE_UINT16(data);
		v9 = READ_LE_UINT16(data + 2);
		v8 = READ_LE_UINT16(data + 4);
		v7 = READ_LE_UINT16(data + 6);
		v6 = READ_LE_UINT16(data + 8);
		if (READ_LE_UINT16(data)) {
			if (_vm->_globals.Bank[idx].field4) {
				if (!v9)
					v9 = 1;
				if (!v6)
					v6 = -1;
			
				if (READ_LE_UINT16(data + 24)) {
					_vm->_globals.Bob[idx].field3A = 0;
          
					if (_vm->_globals.Bank[idx].fileHeader == 1) {
						_vm->_globals.Bob[idx].field3A = 1;
						_vm->_globals.Bob[idx].field36 = 0;
						_vm->_globals.Bob[idx].field38 = 0;
					}

					_vm->_globals.Bob[idx].field18 = _vm->_globals.Bqe_Anim[idx].data;
					_vm->_globals.Bob[idx].field0 = 10;
					_vm->_globals.Bob[idx].field4 = _vm->_globals.Bank[idx].data;

					_vm->_globals.Bob[idx].field1E = v9;
					_vm->_globals.Bob[idx].field20 = v6;
					_vm->_globals.Bob[idx].field22 = v8;
					_vm->_globals.Bob[idx].field24 = v7;
				}
			}
		}
	}
}

void ObjectsManager::BOB_OFF(int idx) {
	if (_vm->_globals.Bob[idx].field0 == 3)
		_vm->_globals.Bob[idx].field0 = 4;
	else if (_vm->_globals.Bob[idx].field0 == 10)
		_vm->_globals.Bob[idx].field0 = 11;
}

void ObjectsManager::BOB_OFFSET(int idx, int v) {
	_vm->_globals.Bob[idx].field46 = v;
}

void ObjectsManager::BOB_ADJUST(int idx, int v) {
	_vm->_globals.Bob[idx].field46 = v;
}

void ObjectsManager::BOB_OFFSETY(int idx, int v) {
	_vm->_globals.Bob[idx].field24 = v;
}

void ObjectsManager::SCBOB(int idx) {
	int v1;
	signed int v2;
	signed int v3;
	signed int v4;
	signed int v6;
	signed int v7;
	int v8; 
	signed int v9; 

	v1 = idx;
	if (_vm->_globals.Cache[idx].fieldA > 0) {
		v8 = 0;
		do {
			v1 = v8;
			if (_vm->_globals.Bob[v1].field0) {
				if (!_vm->_globals.Bob[v8].field16) {
					if (!_vm->_globals.Bob[v8].field34) {
						if ( _vm->_globals.Bob[v8].fieldC != 250) {
							v2 = _vm->_globals.Bob[v8].field42;;
							v9 = _vm->_globals.Bob[v8].field46 + _vm->_globals.Bob[v8].field42;
							v6 = _vm->_globals.Bob[v8].field48 + _vm->_globals.Bob[v8].field44;
							v3 =_vm->_globals.Cache[idx].field0;
							v4 =_vm->_globals.Cache[idx].field4;;
							v7 =_vm->_globals.Cache[idx].field6 + v3;
							v1 =_vm->_globals.Cache[idx].field14 +_vm->_globals.Cache[idx].field8 + v4;
              
							if (v6 > v4) {
								if (v6 < (signed int)v1) {
									v1 = 0;
									if (v9 >= v3 && v9 <= (signed int)(_vm->_globals.Cache[idx].field6 + v3)) {
										++_vm->_globals.Cache[idx].fieldA;
										v1 = 1;
									}
                  
									if (!(uint16)v1) {
										if (v2 >= v3 && v7 >= v2) {
											++_vm->_globals.Cache[idx].fieldA;
											v1 = 1;
										}
										if (!(uint16)v1) {
											if ( v7 >= v2 && v2 >= v3 ) {
												++_vm->_globals.Cache[idx].fieldA;
												v1 = 1;
											}
											if (!(uint16)v1) {
												if (v2 >= v3 && v9 <= v7) {
													++_vm->_globals.Cache[idx].fieldA;
													v1 = 1;
												}
												if (!(uint16)v1 && v2 <= v3 && v9 >= v7)
													++_vm->_globals.Cache[idx].fieldA;
											}
										}
									}
								}
							}
						}
					}
				}
			}
			++v8;
		} while (v8 <= 20);
	}
}

void ObjectsManager::CALCUL_BOB(int idx) {
	int result;
	int v3;
	int v4;
	int v5;
	int v6;
	int v7; 
	int v8; 
	int v9; 
	int v10; 
	int v11; 
	int v13;
	int v14;
	int v15;
	int v17;
	signed int v20;
	int v21;
	int v22;

	_vm->_globals.Bob[idx].field3C = 0;
	if (_vm->_globals.Bob[idx].field3A == 1) {
		_vm->_globals.Bob[idx].field38 = 0;
		_vm->_globals.Bob[idx].field36 = 0;
	}
  
	result = _vm->_globals.Bob[idx].fieldC;
	if (result != 250) {
		if (_vm->_globals.Bob[idx].field38) {
			v4 = get_offsetx(_vm->_globals.Bob[idx].field4, result, 1);
			v22 = v4;
			v15 = v4;
			v5 = get_offsety(_vm->_globals.Bob[idx].field4, _vm->_globals.Bob[idx].fieldC, 1);
		} else {
			v3 = get_offsetx(_vm->_globals.Bob[idx].field4, result, 0);
			v22 = v3;
			v15 = v3;
			v5 = get_offsety(_vm->_globals.Bob[idx].field4, _vm->_globals.Bob[idx].fieldC, 0);
		}
    
		v17 = v5;
		v6 = v5;
		v21 = 0;
		v20 = 0;
		v7 = _vm->_globals.Bob[idx].field36;
		
		if ((signed int)v7 < 0) {
			v7 = (signed int)v7;
			if ((signed int)v7 < 0)
				v7 = -v7;
			v20 = v7;
			if ((signed int)v7 > 95)
				v20 = 95;
		}
		if (_vm->_globals.Bob[idx].field36 > 0)
			v21 = _vm->_globals.Bob[idx].field36;
		if (v21) {
			if (v15 >= 0) {
				v22 = _vm->_graphicsManager.Reel_Zoom(v15, v21);
			} else {
				v8 = v15;
				if (v15 < 0)
					v8 = -v15;
				v15 = v8;
				v22 = -(signed int)_vm->_graphicsManager.Reel_Zoom((signed int)v8, v21);
			}
			if (v6 >= 0) {
				v17 = _vm->_graphicsManager.Reel_Zoom(v6, v21);
			} else {
				v9 = v15;
				if (v15 < 0)
					v9 = -v15;
				v6 = v9;
				v17 = -(signed int)_vm->_graphicsManager.Reel_Zoom((signed int)v9, v21);
			}
		}
    
		if (v20) {
			if (v15 >= 0) {
				v22 = _vm->_graphicsManager.Reel_Reduc(v15, v20);
			} else {
				v10 = v15;
				if (v15 < 0)
					v10 = -v15;
				v15 = v10;
				v22 = -(signed int)_vm->_graphicsManager.Reel_Reduc((signed int)v10, v20);
			}
			if (v6 >= 0) {
				v17 = _vm->_graphicsManager.Reel_Reduc(v6, v20);
			} else {
				v11 = v15;
				if (v15 < 0)
					v11 = -v15;
				v17 = -(signed int)_vm->_graphicsManager.Reel_Reduc((signed int)v11, v20);
			}
		}
    
		idx = idx;
		v13 = _vm->_globals.Bob[idx].field8 - v22;
		v14 = _vm->_globals.Bob[idx].fieldA - v17;
		_vm->_globals.Bob[idx].field3C = 1;
		_vm->_globals.Bob[idx].field3E = v13;
		_vm->_globals.Bob[idx].field40 = v14;
		_vm->_globals.Bob[idx].field48 = v21;
		_vm->_globals.Bob[idx].field4A = v20;

		_vm->_globals.Liste2[idx].field0 = 1;
		_vm->_globals.Liste2[idx].field2 = v13;
		_vm->_globals.Liste2[idx].field4 = v14;

		int width = Get_Largeur(_vm->_globals.Bob[idx].field4, _vm->_globals.Bob[idx].fieldC);
		int height = Get_Hauteur(_vm->_globals.Bob[idx].field4, _vm->_globals.Bob[idx].fieldC);

		if (v21) {
			width = _vm->_graphicsManager.Reel_Zoom(width, v21);
			height = _vm->_graphicsManager.Reel_Zoom(height, v21);
		}
		if (v20) {
			height = _vm->_graphicsManager.Reel_Reduc(height, v20);
			width = _vm->_graphicsManager.Reel_Reduc(width, v20);
		}
		
		_vm->_globals.Liste2[idx].width = width;
		_vm->_globals.Liste2[idx].height = height;
		_vm->_globals.Bob[idx].field42 = width;
		_vm->_globals.Bob[idx].field44 = height;
	}
}

void ObjectsManager::VERIFCACHE() {
	int v1;
	signed int v2;
	signed int v3;
	signed int v4;
	int v5; 
	int v6; 
	int v7; 
	int v8; 
	int v9; 
	int v10;
	int v11;

	v8 = 0;
	do {
		if (_vm->_globals.Cache[v8].fieldA > 0) {
			v7 = _vm->_globals.Cache[v8].fieldA;
			v10 = 0;
			do {
				if (_vm->_objectsManager.Sprite[v10].field0 == 1) {
					if (_vm->_objectsManager.Sprite[v10].field10 != 250) {
						v1 = _vm->_objectsManager.Sprite[v10].field2C;
						v11 = _vm->_objectsManager.Sprite[v10].field30 + v1;
						v2 = _vm->_objectsManager.Sprite[v10].field32 + _vm->_objectsManager.Sprite[v10].field2E;
						v6 = _vm->_globals.Cache[v8].field0;
						v3 = _vm->_globals.Cache[v8].field4;
						v9 = _vm->_globals.Cache[v8].field6 + v6;
            
						if (v2 > v3) {
							if (v2 < (_vm->_globals.Cache[v8].field14 + _vm->_globals.Cache[v8].field8 + v3)) {
								v4 = 0;
								if (v11 >= v6 && v11 <= v9) {
									++_vm->_globals.Cache[v8].fieldA;
									v4 = 1;
								}
								if (!v4) {
									if (v6 <= v1 && v9 >= v1) {
										++_vm->_globals.Cache[v8].fieldA;
										v4 = 1;
									}
									if (!v4) {
										if (v9 >= v1 && v6 <= v1) {
											++_vm->_globals.Cache[v8].fieldA;
											v4 = 1;
										}
										if (!v4) {
											if (v6 <= v1 && v11 <= v9) {
												++_vm->_globals.Cache[v8].fieldA;
												v4 = 1;
											}
											if (!v4 && v6 >= v1 && v11 >= v9)
												++_vm->_globals.Cache[v8].fieldA;
										}
									}
								}
							}
						}
					}
				}
			
				++v10;
			} while (v10 <= 4);
      
			SCBOB(v8);
			if (_vm->_globals.Cache[v8].fieldA == v7) {
				if (_vm->_globals.Cache[v8].field10 == 1) {
					_vm->_globals.Cache[v8].field10 = 0;
					_vm->_globals.Cache[v8].fieldA = 1;
				}
			} else {
				v5 = _vm->_globals.Cache[v8].field14 + _vm->_globals.Cache[v8].field8 + 
					_vm->_globals.Cache[v8].field4;
				if (v5 > 440)
					v5 = 500;
				
				AvantTri(3, v8, v5);
				_vm->_globals.Cache[v8].fieldA = 1;
				_vm->_globals.Cache[v8].field10 = 1;
			}
		}
		++v8;
	} while (v8 <= 19);
}

void ObjectsManager::INVENT_ANIM() {
	int v0; 
	signed int v1;

	if (!DESACTIVE_INVENT) {
		if (FLAG_VISIBLE_EFFACE && !FLAG_VISIBLE) {
			_vm->_graphicsManager.SCOPY(_vm->_graphicsManager.VESA_SCREEN, I_old_x, 27, 48, 38, 
				_vm->_graphicsManager.VESA_BUFFER, I_old_x, 27);
			_vm->_graphicsManager.Ajoute_Segment_Vesa(I_old_x, 27, I_old_x + 48, 65);
			--FLAG_VISIBLE_EFFACE;
		}
    
		if (FLAG_VISIBLE) {
			if (I_old_x <= 1)
				I_old_x = 2;
			_vm->_graphicsManager.SCOPY(_vm->_graphicsManager.VESA_SCREEN, I_old_x, 27, 48, 38, 
				_vm->_graphicsManager.VESA_BUFFER, I_old_x, 27);
      
			_vm->_graphicsManager.Ajoute_Segment_Vesa(I_old_x, 27, I_old_x + 48, 65);
			v0 = _vm->_graphicsManager.ofscroll + 2;
			v1 = _vm->_graphicsManager.ofscroll + 2;
			_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.ICONE, v1 + 300, 327, 0);
			_vm->_graphicsManager.Ajoute_Segment_Vesa(v1, 27, v1 + 45, 62);
			I_old_x = v0;
		}
    
		if (_vm->_globals.SAUVEGARDE->data[svField357] == 1) {
			if (_vm->_globals.SAUVEGARDE->data[svField353] == 1)
				_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.TETE, 832, 325, 0, 0, 0, 0);
			if (_vm->_globals.SAUVEGARDE->data[svField355] == 1)
				_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.TETE, 866, 325, 1, 0, 0, 0);
			_vm->_graphicsManager.Ajoute_Segment_Vesa(532, 25, 560, 60);
			_vm->_graphicsManager.Ajoute_Segment_Vesa(566, 25, 594, 60);
		}
		if (_vm->_globals.SAUVEGARDE->data[svField356] == 1) {
			_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.TETE, 832, 325, 0, 0, 0, 0);
			_vm->_graphicsManager.Ajoute_Segment_Vesa(532, 25, 560, 60);
		}

		if (_vm->_globals.SAUVEGARDE->data[svField354] == 1) {
			_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.TETE, 832, 325, 0, 0, 0, 0);
			_vm->_graphicsManager.Ajoute_Segment_Vesa(532, 25, 560, 60);
		}
	}
}

void ObjectsManager::DEF_SPRITE(int idx) {
	int v2; 
	int v3;
	int v4;
	int v5; 
	int v6; 
	int v7; 
	int v8; 
	int v9; 
	int v10;
	int v11;
	int v12;
	int v13; 

	v2 = idx;
	if (Sprite[v2].field2A) {
		v3 = Sprite[v2].field2C;
		v4 = Sprite[v2].field2E;
		if (Sprite[v2].field28)
			_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager.VESA_BUFFER, Sprite[v2].spriteData, 
				v3 + 300, v4 + 300, Sprite[v2].field10);
		else
			_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager.VESA_BUFFER, Sprite[v2].spriteData, 
				v3 + 300, v4 + 300,  Sprite[v2].field10, Sprite[v2].field36, Sprite[v2].field34, Sprite[v2].fieldE);

		v5 = idx;
		v6 = idx;
		_vm->_globals.Liste[v5].width = Sprite[v6].field30;
		_vm->_globals.Liste[v5].height = Sprite[v6].field32;
		v7 = _vm->_globals.Liste[v5].field2;
		v8 = _vm->_graphicsManager.min_x;
		
		if (v7 < _vm->_graphicsManager.min_x) {
			_vm->_globals.Liste[v5].width -= _vm->_graphicsManager.min_x - v7;
			_vm->_globals.Liste[v5].field2 = v8;
		}

		v9 = _vm->_globals.Liste[v5].field4;
		v10 = _vm->_graphicsManager.min_y;
		if (v9 < _vm->_graphicsManager.min_y) {
			_vm->_globals.Liste[v5].height -= _vm->_graphicsManager.min_y - v9;
			_vm->_globals.Liste[v5].field4 = v10;
		}
		v11 = _vm->_globals.Liste[v5].field2;
		if (_vm->_globals.Liste[v5].width + v11 > _vm->_graphicsManager.max_x)
			_vm->_globals.Liste[v5].width = _vm->_graphicsManager.max_x - v11;
		v12 = _vm->_globals.Liste[v5].field4;
		if ( _vm->_globals.Liste[v5].height + v12 > _vm->_graphicsManager.max_y)
			_vm->_globals.Liste[v5].height = _vm->_graphicsManager.max_y - v12;
		if ( _vm->_globals.Liste[v5].width <= 0 || _vm->_globals.Liste[v5].height <= 0)
			_vm->_globals.Liste[v5].field0 = 0;

		v13 = idx;
		if (_vm->_globals.Liste[v13].field0 == 1)
			_vm->_graphicsManager.Ajoute_Segment_Vesa(
			_vm->_globals.Liste[v13].field2,
				_vm->_globals.Liste[v13].field4,
				_vm->_globals.Liste[v13].field2 + _vm->_globals.Liste[v13].width,
				_vm->_globals.Liste[v13].field4 + _vm->_globals.Liste[v13].height);
	}
}

void ObjectsManager::DEF_CACHE(int idx) {
	_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.CACHE_BANQUE[1],
		_vm->_globals.Cache[idx].field0 + 300, _vm->_globals.Cache[idx].field4 + 300,
		_vm->_globals.Cache[idx].field2);
  
	_vm->_graphicsManager.Ajoute_Segment_Vesa(_vm->_globals.Cache[idx].field0, 
			_vm->_globals.Cache[idx].field4, 
			_vm->_globals.Cache[idx].field0 + _vm->_globals.Cache[idx].field6,
           _vm->_globals.Cache[idx].field4 + _vm->_globals.Cache[idx].field8);
}

void ObjectsManager::CALCUL_SPRITE(int idx) {
	int width, height;
	int v3;
	int v4;
	int v5;
	int v6;
	int v7;
	int v8;
	int v9; 
	int v10; 
	int v11; 
	int v12; 
	int v13; 
	int v15;
	int v16;
	int v17; 
	int v22;

	Sprite[idx + 42].field2A = 0;
	int v0 = Sprite[idx].field10;
	if (v0 != 250) {
		if (Sprite[idx].fieldE) {
			v5 = get_offsetx(Sprite[idx].spriteData, v0, 1);
			v22 = Sprite[idx].field12 + v5;
			v4 = Sprite[idx].field12 + v5;
			v6 = get_offsety(Sprite[idx].spriteData, Sprite[idx].field10, 1);
		} else {
			v3 = get_offsetx(Sprite[idx].spriteData, v0, 0);
			v22 = Sprite[idx].field12 + v3;
			v4 = Sprite[idx].field12 + v3;
			v6 = get_offsety(Sprite[idx].spriteData, Sprite[idx].field10, 0);
		}
    
		v9 = Sprite[idx].field14 + v6;
		v7 = v9;
		v8 = v9;
		int zoomPercent = 0;
		int reducePercent = 0;
    
		v9 = Sprite[idx].fieldC;
		if ((signed int)v9 < 0) {
			v9 = (signed int)v9;
			if ((signed int)v9 < 0)
				v9 = -v9;
			reducePercent = v9;
			if ((signed int)v9 > 95)
				reducePercent = 95;
		}
		if (Sprite[idx].fieldC > 0)
			zoomPercent = Sprite[idx].fieldC;
    
		if (zoomPercent) {
			if (v4 >= 0) {
				v22 = _vm->_graphicsManager.Reel_Zoom(v4, zoomPercent);
			} else {
				v10 = v4;
        
				if (v4 < 0)
					v10 = -v4;
				v4 = v10;
				v22 = -_vm->_graphicsManager.Reel_Zoom((signed int)v10, zoomPercent);
			}
      
			if (v8 >= 0) {
				v7 = _vm->_graphicsManager.Reel_Zoom(v8, zoomPercent);
			} else {
				v11 = v4;
				if (v4 < 0)
					v11 = -v4;
				v8 = v11;
				v7 = -_vm->_graphicsManager.Reel_Zoom((signed int)v11, zoomPercent);
			}
		}
		if (reducePercent) {
			if (v4 >= 0) {
				v22 = _vm->_graphicsManager.Reel_Reduc(v4, reducePercent);
			} else {
				v12 = v4;
				if (v4 < 0)
					v12 = -v4;
				v4 = v12;
				v22 = -_vm->_graphicsManager.Reel_Reduc((signed int)v12, reducePercent);
			}
			if (v8 >= 0) {
				v7 = _vm->_graphicsManager.Reel_Reduc(v8, reducePercent);
			} else {
				v13 = v4;
				if (v4 < 0)
					v13 = -v4;
				v7 = -_vm->_graphicsManager.Reel_Reduc((signed int)v13, reducePercent);
			}
		}
    
		v15 = Sprite[idx].field8 - v22;
		v16 = Sprite[idx].fieldA - v7;
		Sprite[idx].field2C = v15;
		Sprite[idx].field2E = v16;
		Sprite[idx].field2A = 1;
		Sprite[idx].field34 = zoomPercent;
		Sprite[idx].field36 = reducePercent;
	
		v17 = idx;
		_vm->_globals.Liste[v17].field0 = 1;
		_vm->_globals.Liste[v17].field2 = v15;
		_vm->_globals.Liste[v17].field4 = v16;
		width = Get_Largeur(Sprite[idx].spriteData, Sprite[idx].field10);
		height = Get_Hauteur(Sprite[idx].spriteData, Sprite[idx].field10);

		if (zoomPercent) {
			width = _vm->_graphicsManager.Reel_Zoom(width, zoomPercent);
			height = _vm->_graphicsManager.Reel_Zoom(height, zoomPercent);
		}
    
		if (reducePercent) {
			height = _vm->_graphicsManager.Reel_Reduc(height, reducePercent);
			width = _vm->_graphicsManager.Reel_Reduc(width, reducePercent);
		}
    
		Sprite[idx].field30 = width;
		Sprite[idx].field32 = height;
	}
}

int ObjectsManager::AvantTri(int a1, int a2, int a3) {
	int result;

	++_vm->_globals.NBTRI;
	if (_vm->_globals.NBTRI > 48)
		error("NBTRI TROP GRAND");
  
	result = _vm->_globals.NBTRI;
	_vm->_globals.Tri[result].field0 = a1;
	_vm->_globals.Tri[result].field2 = a2;
	_vm->_globals.Tri[result].field4 = a3;
  
	return result;
}

void ObjectsManager::AFF_BOB_ANIM() {
	signed int v1;
	int v2;
	signed int v5;
	int v6;
	int v7;
	int v8;
	signed int v10;
	int v11;
	int v12; 
	int v13; 
	int v14;
	int v18;
	int v19;
	byte *v20; 
	byte *v21; 
	int v22; 
	int v24;
	int v26;
	int v27;
	int v28;

	int idx = 0;
	do {
		++idx;
		if (idx <= 20 && PERSO_ON == 1) {
			_vm->_globals.Bob[idx].field1C = 0;
			continue;
		}
    
		if (_vm->_globals.Bob[idx].field0 == 10) {
			_vm->_globals.Bob[idx].field1C = 0;
			v1 = _vm->_globals.Bob[idx].field20;
			if (v1 == -1)
				v1 = 50;
			if (_vm->_globals.Bob[idx].field18 == PTRNUL || _vm->_globals.Bob[idx].field16 || v1 <= 0)
				goto LABEL_38;
      
			v2 = _vm->_globals.Bob[idx].field14;
			if (_vm->_globals.Bob[idx].field12 == v2) {
				_vm->_globals.Bob[idx].field1C = 1;
			} else {
				_vm->_globals.Bob[idx].field14 = v2 + 1;
				_vm->_globals.Bob[idx].field1C = 0;
			}
      
			if (_vm->_globals.Bob[idx].field1C != 1)
				goto LABEL_38;
      
			v20 = _vm->_globals.Bob[idx].field18 + 20;
			v24 = _vm->_globals.Bob[idx].field10;
			_vm->_globals.Bob[idx].field8 = READ_LE_UINT16(v20 + 2 * v24);
			if (_vm->_globals.BL_ANIM[idx].v1 == 1)
				_vm->_globals.Bob[idx].field8 = _vm->_globals.BL_ANIM[idx].v2;
			if ( PERSO_ON == 1 && idx > 20 )
				_vm->_globals.Bob[idx].field8 += _vm->_eventsManager.start_x;
      
			_vm->_globals.Bob[idx].fieldA = READ_LE_UINT16(v20 + 2 * v24 + 2);
			_vm->_globals.Bob[idx].field12 = READ_LE_UINT16(v20 + 2 * v24 + 4);
			_vm->_globals.Bob[idx].field36 = READ_LE_UINT16(v20 + 2 * v24 + 6);
			_vm->_globals.Bob[idx].fieldC = *(v20 + 2 * v24 + 8);
			_vm->_globals.Bob[idx].field38 = *(v20 + 2 * v24 + 9);
			_vm->_globals.Bob[idx].field10 += 5;
			v5 = _vm->_globals.Bob[idx].field12;
			
			if (v5 > 0) {
				v6 = v5 / _vm->_globals.vitesse;
				_vm->_globals.Bob[idx].field12 = v5 / _vm->_globals.vitesse;
				if (v6 > 0) {
LABEL_37:
					_vm->_globals.Bob[idx].field14 = 1;
LABEL_38:
					v12 = idx;
          
					if ((unsigned int)(_vm->_globals.Bob[v12].field1E - 1) <= 1u)
						_vm->_globals.Bob[v12].field1C = 1;
					continue;
				}
        
				_vm->_globals.Bob[idx].field12 = 1;
			}
			if (!_vm->_globals.Bob[idx].field12) {
				v7 = _vm->_globals.Bob[idx].field20;
				if (v7 > 0)
					_vm->_globals.Bob[idx].field20 = v7 - 1;
				v8 = _vm->_globals.Bob[idx].field20;
				if (v8 != -1 && v8 <= 0) {
					_vm->_globals.Bob[idx].field0 = 11;
				} else {
					_vm->_globals.Bob[idx].field10 = 0;
					v21 = _vm->_globals.Bob[idx].field18 + 20;
					_vm->_globals.Bob[idx].field8 = READ_LE_UINT16(v21);
					
					if (_vm->_globals.BL_ANIM[idx].v1 == 1)
						_vm->_globals.Bob[idx].field8 = _vm->_globals.BL_ANIM[idx].v2;
					if (PERSO_ON == 1 && idx > 20)
						_vm->_globals.Bob[idx].field8 += _vm->_eventsManager.start_x;

					_vm->_globals.Bob[idx].fieldA = READ_LE_UINT16(v21 + 2);
					_vm->_globals.Bob[idx].field12 = READ_LE_UINT16(v21 + 4);
					_vm->_globals.Bob[idx].field36 = READ_LE_UINT16(v21 + 6);
					_vm->_globals.Bob[idx].fieldC = *(v21 + 8);
					_vm->_globals.Bob[idx].field38 = *(v21 + 9);
					_vm->_globals.Bob[idx].field10 += 5;
					v10 = _vm->_globals.Bob[idx].field12;
			
					if (v10 > 0) {
						v11 = v10 / _vm->_globals.vitesse;
						_vm->_globals.Bob[idx].field12 = v10 / _vm->_globals.vitesse;
						if (v11 <= 0)
							_vm->_globals.Bob[idx].field12 = 1;
					}
				}
			}
			
			goto LABEL_37;
		}
	} while (idx != 35);

	if (!PERSO_ON && BOBTOUS == 1) {
		v26 = 0;
		do {
			v13 = v26;
			if (_vm->_globals.Bob[v13].field0 == 10 && !_vm->_globals.Bob[v13].field16)
				_vm->_globals.Bob[v13].field1C = 1;
			++v26;
		} while (v26 != 35);
	}
  
	BOBTOUS = 0;
	v27 = 0;
  
	do {
		++v27;
		if (v27 > 20 || PERSO_ON != 1) {
			if (_vm->_globals.Bob[v27].field0 == 10) {
				if (_vm->_globals.Bob[v27].field1C == 1) {
					v14 = _vm->_globals.Bob[v27].field1E;
          
					if (v14 != 2) {
						if (v14 != 4) {
							if (_vm->_globals.Liste2[v27].field0) {
								_vm->_graphicsManager.SCOPY(_vm->_graphicsManager.VESA_SCREEN,
									_vm->_globals.Liste2[v27].field2, _vm->_globals.Liste2[v27].field4,
									_vm->_globals.Liste2[v27].width, _vm->_globals.Liste2[v27].height,
									_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.Liste2[v27].field2,
									_vm->_globals.Liste2[v27].field4);
								_vm->_globals.Liste2[v27].field0 = 0;
							}
						}
					}
				}
			}
      
			v22 = v27;
			if (_vm->_globals.Bob[v22].field0 == 11) {
				if (_vm->_globals.Liste2[v27].field0) {
					_vm->_graphicsManager.SCOPY(_vm->_graphicsManager.VESA_SCREEN,
						_vm->_globals.Liste2[v27].field2, _vm->_globals.Liste2[v27].field4,
						_vm->_globals.Liste2[v27].width, _vm->_globals.Liste2[v27].height,
						_vm->_graphicsManager.VESA_BUFFER,
						_vm->_globals.Liste2[v27].field2, _vm->_globals.Liste2[v27].field4);
					_vm->_globals.Liste2[v27].field0 = 0;
				}
        
				_vm->_globals.Bob[v22].field0 = 0;
			}
		}
	} while (v27 != 35);
  
	v28 = 0;
	do {
		++v28;
		v18 = v28;
		_vm->_globals.Bob[v18].field40 = 0;
		if (_vm->_globals.Bob[v18].field0 == 10 && !_vm->_globals.Bob[v18].field16 && _vm->_globals.Bob[v18].field1C == 1) {
			CALCUL_BOB(v28);
			int v = _vm->_globals.Bob[v18].field44 + _vm->_globals.Bob[v18].field40;
			v19 = _vm->_globals.Bob[v18].field46 + v;
		
			if (v19 > 450)
				v19 = 600;
			if (_vm->_globals.Bob[v18].field3C == 1)
			AvantTri(1, v28, v19);
		}
	} while (v28 != 35);
}

void ObjectsManager::AFF_VBOB() {
	int width, height; 

	int idx = 0;
	do {
		if (_vm->_globals.VBob[idx].field4 == 4) {
			width = Get_Largeur(_vm->_globals.VBob[idx].field0, _vm->_globals.VBob[idx].fieldA);
			height = Get_Hauteur(_vm->_globals.VBob[idx].field0, _vm->_globals.VBob[idx].fieldA);
			
			_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager.VESA_SCREEN,
				_vm->_globals.VBob[idx].field10, _vm->_globals.VBob[idx].field6,
				_vm->_globals.VBob[idx].field8,
				width, height);
      
			_vm->_graphicsManager.Restore_Mem(
				_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.VBob[idx].field10,
				_vm->_globals.VBob[idx].field6, _vm->_globals.VBob[idx].field8,
				width, height);

			_vm->_graphicsManager.Ajoute_Segment_Vesa(
				_vm->_globals.VBob[idx].field6, _vm->_globals.VBob[idx].field8,
				_vm->_globals.VBob[idx].field6 + width,
				height + _vm->_globals.VBob[idx].field8);
      
			if (PTRNUL != _vm->_globals.VBob[idx].field10)
				_vm->_globals.dos_free2(_vm->_globals.VBob[idx].field10);
      
			_vm->_globals.VBob[idx].field4 = 0;
			_vm->_globals.VBob[idx].field10 = PTRNUL;
			_vm->_globals.VBob[idx].field0 = PTRNUL;
			_vm->_globals.VBob[idx].field6 = 0;
			_vm->_globals.VBob[idx].field8 = 0;
			_vm->_globals.VBob[idx].field14 = 0;
			_vm->_globals.VBob[idx].field16 = 0;
			_vm->_globals.VBob[idx].fieldA = 0;
			_vm->_globals.VBob[idx].field18 = 0;
			_vm->_globals.VBob[idx].field1C = PTRNUL;
		}

		if (_vm->_globals.VBob[idx].field4 == 3) {
			width = Get_Largeur(_vm->_globals.VBob[idx].field1C, _vm->_globals.VBob[idx].field18);
			height = Get_Hauteur(_vm->_globals.VBob[idx].field1C, _vm->_globals.VBob[idx].field18);
      
			_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager.VESA_SCREEN,
				_vm->_globals.VBob[idx].field10, _vm->_globals.VBob[idx].field14,
				_vm->_globals.VBob[idx].field16,
				width, height);
      
			_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager.VESA_BUFFER,
				_vm->_globals.VBob[idx].field10, _vm->_globals.VBob[idx].field14,
				_vm->_globals.VBob[idx].field16,
				width, height);
      
			_vm->_graphicsManager.Ajoute_Segment_Vesa(_vm->_globals.VBob[idx].field14,
				_vm->_globals.VBob[idx].field16, _vm->_globals.VBob[idx].field14 + width,
				_vm->_globals.VBob[idx].field16 + height);
      
			_vm->_globals.VBob[idx].field4 = 1;
			_vm->_globals.VBob[idx].field1C = _vm->_globals.VBob[idx].field0;
      
			if (PTRNUL != _vm->_globals.VBob[idx].field10)
				_vm->_globals.dos_free2(_vm->_globals.VBob[idx].field10);
      
			_vm->_globals.VBob[idx].field10 = PTRNUL;
			_vm->_globals.VBob[idx].field14 = _vm->_globals.VBob[idx].field6;
			_vm->_globals.VBob[idx].field16 = _vm->_globals.VBob[idx].field8;
			_vm->_globals.VBob[idx].field18 = _vm->_globals.VBob[idx].fieldA;
		}

		if (_vm->_globals.VBob[idx].field4 == 1) {
			width = Get_Largeur(_vm->_globals.VBob[idx].field0, _vm->_globals.VBob[idx].fieldA);
			height = Get_Hauteur(_vm->_globals.VBob[idx].field0, _vm->_globals.VBob[idx].fieldA);
      
			if (PTRNUL != _vm->_globals.VBob[idx].field10)
				_vm->_globals.dos_free2(_vm->_globals.VBob[idx].field10);
      
			byte *surface = _vm->_globals.dos_malloc2(height * width);
			_vm->_globals.VBob[idx].field10 = surface;
      
			_vm->_graphicsManager.Capture_Mem(_vm->_graphicsManager.VESA_SCREEN, surface, 
				_vm->_globals.VBob[idx].field6, _vm->_globals.VBob[idx].field8, width, height);
      
			byte *v10 = _vm->_globals.VBob[idx].field0;
			if (*v10 == 78) {
				_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager.VESA_SCREEN, v10, 
					_vm->_globals.VBob[idx].field6 + 300,
					_vm->_globals.VBob[idx].field8 + 300,
					_vm->_globals.VBob[idx].fieldA,
					0, 0, 0);
        
				_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager.VESA_BUFFER,
					_vm->_globals.VBob[idx].field0,
					_vm->_globals.VBob[idx].field6 + 300, _vm->_globals.VBob[idx].field8 + 300,
					_vm->_globals.VBob[idx].fieldA,
					0, 0, 0);
			} else {
				_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager.VESA_BUFFER,
					v10, _vm->_globals.VBob[idx].field6 + 300, _vm->_globals.VBob[idx].field8 + 300,
					_vm->_globals.VBob[idx].fieldA);
        
				_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager.VESA_SCREEN, _vm->_globals.VBob[idx].field0,
					_vm->_globals.VBob[idx].field6 + 300, _vm->_globals.VBob[idx].field8 + 300,
					_vm->_globals.VBob[idx].fieldA);
			}
      
			_vm->_graphicsManager.Ajoute_Segment_Vesa(_vm->_globals.VBob[idx].field6,
				_vm->_globals.VBob[idx].field8, _vm->_globals.VBob[idx].field6 + width,
				_vm->_globals.VBob[idx].field8 + height);
			_vm->_globals.VBob[idx].field4 = 2;
		}
		++idx;
	} while ( idx <= 29 );
}

int ObjectsManager::XSPR(int idx) {
	if (idx > 5)
		error("request of the Coord. x a sprite > MAX_SPRITE.");
	return Sprite[idx].field8;
}

int ObjectsManager::YSPR(int idx) {
	if (idx > 5)
		error("request of the Coord. y a sprite > MAX_SPRITE.");
	return Sprite[idx].fieldA;
}

void ObjectsManager::SPRITE_NOW(const byte *spriteData, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
	_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager.VESA_BUFFER, spriteData, a2, a3, a5, a6, a7, a8);
}

void ObjectsManager::CLEAR_SPR() {
	int idx;

	idx = 0;
	do {
		Sprite[idx].field1C = PTRNUL;
		Sprite[idx].spriteData = PTRNUL;
		Sprite[idx].field0 = 0;
		++idx;
	} while (idx <= 4);

	idx = 0;
	do {
		_vm->_globals.Liste[idx].field0 = 0;
		_vm->_globals.Liste[idx].field2 = 0;
		_vm->_globals.Liste[idx].field4 = 0;
		_vm->_globals.Liste[idx].width = 0;
		_vm->_globals.Liste[idx].height = 0;
		_vm->_globals.Liste[idx].fieldA = 0;
		++idx;
	} while (idx <= 4);
}

void ObjectsManager::SPRITE_ON(int idx) {
	if (idx > 5)
		error("Tentative d'affichage d'un sprite > MAX_SPRITE.");
  
	Sprite[idx].field0 = 1;
}

void ObjectsManager::SPRITE(const byte *spriteData, int a2, int a3, int idx, int a5, int a6, int a7, int a8, int a9) {
	if (idx > 5 )
		(8, (int)"Tentative d'affichage d'un sprite > MAX_SPRITE.");
	Sprite[idx].spriteData = spriteData;
	Sprite[idx].field8 = a2;
	Sprite[idx].fieldA = a3;
	Sprite[idx].field10 = a5;
	Sprite[idx].fieldC = a6;
	Sprite[idx].field12 = a8;
	Sprite[idx].field14 = a9;
	Sprite[idx].field1C = PTRNUL;
	Sprite[idx].field20 = 0;
	Sprite[idx].field24 = 0;
	Sprite[idx].field26 = 0;
	Sprite[idx].field22 = 0;
	Sprite[idx].field0 = 0;
	Sprite[idx].field28 = 0;
	Sprite[idx].fieldE = a7;
	if (*spriteData == 'R' && *(spriteData + 1) == 'L' && *(spriteData + 2) == 'E')
		Sprite[idx].field28 = 1;

	if (Sprite[idx].field28 == 1) {
		Sprite[idx].fieldC = 0;
		Sprite[idx].fieldE = 0;
	}
}

void ObjectsManager::SPRITE2(const byte *spriteData, int idx, byte *a3, int a4, int a5) {
	Sprite[idx].spriteData = spriteData;
	Sprite[idx].field1C = a3;
	Sprite[idx].field20 = a4;
	Sprite[idx].field24 = 0;
	Sprite[idx].field26 = 0;
	Sprite[idx].fieldC = 0;
	Sprite[idx].fieldE = 0;
	Sprite[idx].field0 = 1;
	Sprite[idx].field22 = 0;
	Sprite[idx].field14 = a5;
	if (*spriteData == 'R' && *(spriteData + 1) == 'L' && *(spriteData + 2) == 'E')
		Sprite[idx].field28 = 1;

	if (Sprite[idx].field28 == 1) {
		Sprite[idx].fieldC = 0;
		Sprite[idx].fieldE = 0;
	}
}

void ObjectsManager::SPRITE_OFF(int idx) {
	Sprite[idx].field0 = 3;
	if (Sprite[idx].field1C != PTRNUL)
		_vm->_globals.dos_free2(Sprite[idx].field1C);
	Sprite[idx].field1C = PTRNUL;
}

void ObjectsManager::SPRITE_GEL(int idx) {
	Sprite[idx].field0 = 3;
}

int ObjectsManager::SXSPR(int idx) {
	if (idx > 5)
		error("request of the size of a sprite x > MAX_SPRITE.");
	return Get_Largeur(Sprite[idx].spriteData, Sprite[idx].field10);
}

int ObjectsManager::SYSPR(int idx) {
	if (idx > 5)
		error("request of the size of a sprite y > MAX_SPRITE.");
	return Get_Hauteur(Sprite[idx].spriteData, Sprite[idx].field10);
}

int ObjectsManager::POSISPR(int idx) {
  return Sprite[idx].field22 / 6;
}

void ObjectsManager::SETPOSISPR(int idx, int a2) {
	Sprite[idx].field22 = 6 * a2;
	Sprite[idx].field24 = 0;
	Sprite[idx].field26 = 0;
}

void ObjectsManager::SETXSPR(int idx, int xp) {
	if (idx > 5)
		error("Set the Coord. x a sprite> MAX_SPRITE.");
	Sprite[idx].field8 = xp;
}

void ObjectsManager::SETANISPR(int idx, int a2) {
	if (idx > 5)
		error("Set the Coord. x a sprite> MAX_SPRITE.");

	Sprite[idx].field10 = a2;
}

void ObjectsManager::SETYSPR(int idx, int yp) {
	if ( idx > 5 )
		error("Set the Coord. y a sprite> MAX_SPRITE.");
	Sprite[idx].fieldA = yp;
}

void ObjectsManager::SETTAILLESPR(int idx, int a2) {
	if (idx > 5)
		error("Set the Coord. there a sprite> MAX_SPRITE.");
  
	if (Sprite[idx].field28 != 1)
		Sprite[idx].fieldC = a2;
}

void ObjectsManager::SETFLIPSPR(int idx, int a2) {
	if (Sprite[idx].field28 != 1) {
		if (idx > 5)
			error("Set the Coord. there a sprite> MAX_SPRITE.");
		Sprite[idx].fieldE = a2;
	}
}

void ObjectsManager::VERIFZONE() {
	__int16 v0;
	int v1; 
	__int16 v2;
	unsigned __int16 v3;
	__int16 v4;

	v0 = _vm->_eventsManager.XMOUSE();
	v1 = _vm->_eventsManager.YMOUSE();
	v2 = v1;
	if (_vm->_globals.PLAN_FLAG
	        || _vm->_eventsManager.start_x >= v0
	        || (v1 = _vm->_graphicsManager.ofscroll + 54, v0 >= v1)
	        || (v1 = v2 - 1, (unsigned __int16)(v2 - 1) > 0x3Bu)) {
		if (FLAG_VISIBLE == 1)
			FLAG_VISIBLE_EFFACE = 4;
		FLAG_VISIBLE = 0;
	} else {
		FLAG_VISIBLE = 1;
	}
	if (FORCEZONE == 1) {
		_vm->_globals.compteur_71 = 100;
		_vm->_globals.old_zone_68 = -1;
		_vm->_globals.old_x_69 = -200;
		_vm->_globals.old_y_70 = -220;
		FORCEZONE = 0;
	}
	v3 = _vm->_globals.compteur_71 + 1;
	_vm->_globals.compteur_71 = v3;
	if (v3 > 1u) {
		if (_vm->_globals.NOMARCHE || (_vm->_globals.chemin == PTRNUL) || v3 > 4u) {
			_vm->_globals.compteur_71 = 0;
			if (_vm->_globals.old_x_69 != v0 || _vm->_globals.old_y_70 != v2) {
				v4 = MZONE();
			} else {
				v4 = _vm->_globals.old_zone_68;
			}
			if (_vm->_globals.old_zone_68 != v4) {
				_vm->_graphicsManager.SETCOLOR4(251, 100, 100, 100);
				_vm->_eventsManager.btsouris = 4;
				_vm->_eventsManager.CHANGE_MOUSE(4);
				if (_vm->_globals.zozo_73 == 1) {
					_vm->_fontManager.TEXTE_OFF(5);
					_vm->_globals.zozo_73 = 0;
					return;
				}
				if (_vm->_globals.old_zone_68 != v4)
					goto LABEL_54;
			}
			if (v4 != -1) {
LABEL_54:
				if (v4 != -1
				        && ((_vm->_globals.ZONEP[v4].field6)
				            || _vm->_globals.ZONEP[v4].field7
				            || _vm->_globals.ZONEP[v4].field8
				            || _vm->_globals.ZONEP[v4].field9
				            || _vm->_globals.ZONEP[v4].fieldA
				            || _vm->_globals.ZONEP[v4].fieldB
				            || _vm->_globals.ZONEP[v4].fieldC
				            || _vm->_globals.ZONEP[v4].fieldD
				            || _vm->_globals.ZONEP[v4].fieldE
				            || _vm->_globals.ZONEP[v4].fieldF)) {
					if (_vm->_globals.old_zone_68 != v4) {
						_vm->_fontManager.DOS_TEXT(5, _vm->_globals.ZONEP[v4].field12, _vm->_globals.FICH_ZONE, 0, 430, 20, 25, 0, 0, 252);
						_vm->_fontManager.TEXTE_ON(5);
						_vm->_globals.zozo_73 = 1;
					}
					_vm->_globals.force_to_data_0 += 25;
					if (_vm->_globals.force_to_data_0 > 100)
						_vm->_globals.force_to_data_0 = 0;
					_vm->_graphicsManager.SETCOLOR4(251, _vm->_globals.force_to_data_0, _vm->_globals.force_to_data_0, 
						_vm->_globals.force_to_data_0);
					if (_vm->_eventsManager.btsouris == 4) {
						v1 = 5 * v4;
						if (_vm->_globals.ZONEP[v4].field6 == 2) {
							_vm->_eventsManager.CHANGE_MOUSE(16);
							_vm->_eventsManager.btsouris = 16;
							verbe = 16;
						}
					}
				} else {
					_vm->_graphicsManager.SETCOLOR4(251, 100, 100, 100);
					_vm->_eventsManager.btsouris = 4;
					_vm->_eventsManager.CHANGE_MOUSE(4);
				}
			}
			_vm->_objectsManager.NUMZONE = v4;
			_vm->_globals.old_x_69 = v0;
			_vm->_globals.old_y_70 = v2;
			_vm->_globals.old_zone_68 = v4;
			if (_vm->_globals.NOMARCHE == 1) {
				if (_vm->_eventsManager.btsouris == 4) {
					v1 = v4 + 1;
					if ((unsigned __int16)(v4 + 1) > 1u)
						BTDROITE();
				}
			}
			if (_vm->_globals.PLAN_FLAG == 1 && v4 == -1 || !v4) {
				verbe = 0;
				_vm->_eventsManager.btsouris = 0;
				_vm->_eventsManager.CHANGE_MOUSE(0);
			}
		}
	}
}

void ObjectsManager::GOHOME() {
	warning("GOHOME");
}

void ObjectsManager::GOHOME2() {
	warning("GOHOME2");
}

void ObjectsManager::CHARGE_OBSTACLE(const Common::String &file) {
	warning("CHARGE_OBSTACLE");
}

void ObjectsManager::CHARGE_CACHE(const Common::String &file) {
	warning("CHARGE_CACHE");
}

void ObjectsManager::CHARGE_ZONE(const Common::String &file) {
	warning("CHARGE_ZONE");
}

void ObjectsManager::PLAN_BETA() {
	int v1;
	int v2;
	int v3;
	int v4;
	int v5;

	v1 = 0;
	INVENTFLAG = false;
	KEY_INVENT = 0;
	_vm->_globals.Max_Propre = 1;
	_vm->_globals.Max_Ligne_Long = 1;
	_vm->_globals.Max_Propre_Gen = 1;
	_vm->_globals.Max_Perso_Y = 440;
	_vm->_globals.NOSPRECRAN = 1;
	_vm->_globals.PLAN_FLAG = 1;
	_vm->_graphicsManager.NOFADE = false;
	_vm->_globals.NOMARCHE = 0;
	sprite_ptr = PTRNUL;
	_vm->_globals.SORTIE = 0;
	_vm->_globals.AFFLI = 0;
	_vm->_globals.AFFIVBL = 0;
	_vm->_globals.NOT_VERIF = 1;
	_vm->_soundManager.WSOUND(31);
	_vm->_globals.iRegul = 1;
	_vm->_graphicsManager.LOAD_IMAGE("PLAN");
	CHARGE_OBSTACLE("PLAN.OB2");
	CHARGE_CACHE("PLAN.CA2");
	CHARGE_ZONE("PLAN.ZO2");
	FileManager::CONSTRUIT_SYSTEM("VOITURE.SPR");
	sprite_ptr = FileManager::CHARGE_FICHIER(_vm->_globals.NFICHIER);
	_vm->_animationManager.CHARGE_ANIM("PLAN");
	_vm->_graphicsManager.VISU_ALL();
	_vm->_graphicsManager.INI_ECRAN2("PLAN");
	v2 = 0;
	do {
		_vm->_globals.CACHE_OFF(v2++);
	} while (v2 <= 15);
	_vm->_globals.CACHE_OFF(19);
	_vm->_globals.CACHE_OFF(20);
	_vm->_globals.CACHE_ON();
	
	if (!_vm->_globals.PLANX && !_vm->_globals.PLANY) {
		_vm->_globals.PLANX = 900;
		_vm->_globals.PLANY = 319;
		_vm->_globals.PLANI = 1;
	}
	SPRITE(sprite_ptr, _vm->_globals.PLANX, _vm->_globals.PLANY, 0, _vm->_globals.PLANI, 0, 0, 5, 5);
	_vm->_eventsManager.souris_xy(_vm->_globals.PLANX, _vm->_globals.PLANY);
	my_anim = 0;
	_vm->_eventsManager.MOUSE_ON();
	v3 = XSPR(0);
	_vm->_graphicsManager.SCROLL_ECRAN(v3 - 320);
	_vm->_graphicsManager.ofscroll = XSPR(0) - 320;
	SPRITE_ON(0);
	_vm->_globals.chemin = PTRNUL;
	_vm->_graphicsManager.SETCOLOR3(252, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(253, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(251, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(254, 0, 0, 0);
	_vm->_globals.BPP_NOAFF = 1;
	
	v4 = 0;
	do {
		_vm->_eventsManager.VBL();
		++v4;
	} while (v4 <= 4);
  
	_vm->_globals.BPP_NOAFF = 0;
	_vm->_globals.iRegul = 1;
	_vm->_graphicsManager.FADE_INW();
	_vm->_eventsManager.CHANGE_MOUSE(4);
	_vm->_graphicsManager.NOFADE = false;
  
	do {
		v5 = _vm->_eventsManager.BMOUSE();
		if (v5) {
			if (_vm->_globals.SAUVEGARDE->data[svField170] == 1 && !_vm->_globals.SAUVEGARDE->data[svField171]) {
				_vm->_globals.SAUVEGARDE->data[svField171] = 1;
				_vm->_globals.NOPARLE = true;
				_vm->_talkManager.PARLER_PERSO("APPEL1.pe2");
				_vm->_globals.NOPARLE = false;
				v5 = 0;
			}
			if (_vm->_globals.SAUVEGARDE->data[svField180] == 1 && !_vm->_globals.SAUVEGARDE->data[svField172]) {
				_vm->_globals.SAUVEGARDE->data[svField172] = 1;
				_vm->_globals.NOPARLE = 1;
				_vm->_talkManager.PARLER_PERSO("APPEL2.pe2");
				_vm->_globals.NOPARLE = 0;
				v5 = 0;
				_vm->_eventsManager.souris_bb = false;
			}
			if (v5 == 1)
				BTGAUCHE();
		}
    
		VERIFZONE();
		GOHOME2();
		
		if (_vm->_globals.chemin == PTRNUL && _vm->_globals.GOACTION == 1)
			PARADISE();
		_vm->_eventsManager.VBL();
    
		if (_vm->_globals.SORTIE)
			v1 = 1;
	} while (v1 != 1);

	if (!_vm->_graphicsManager.NOFADE)
		_vm->_graphicsManager.FADE_OUTW();
	_vm->_globals.iRegul = 0;
	_vm->_graphicsManager.NOFADE = false;
	_vm->_globals.PLANX = XSPR(0);
	_vm->_globals.PLANY = YSPR(0);
	_vm->_globals.PLANI = 1;
	SPRITE_OFF(0);
	_vm->_globals.AFFLI = 0;
	sprite_ptr = _vm->_globals.LIBERE_FICHIER(sprite_ptr);
	CLEAR_ECRAN();
	_vm->_globals.NOSPRECRAN = 0;
	_vm->_globals.PLAN_FLAG = 0;
}	

void ObjectsManager::BTGAUCHE() {
	int v0;
	__int16 v1;
	__int16 v2;
	byte *v3; 
	byte *v4; 
	byte *v5; 
	int v6; 
	__int16 v7;
	__int16 v8;
	byte *v9; 
	__int16 v10;
	__int16 v11;
	__int16 v12;
	byte *v13; 
	__int16 v14;
	__int16 v15;
	byte *v16; 
	__int16 v17;
	__int16 v18;
	__int16 v19;

	_vm->_fontManager.TEXTE_OFF(9);
	v19 = _vm->_eventsManager.XMOUSE();
	v0 = _vm->_eventsManager.YMOUSE();
	if (!INVENTFLAG && !_vm->_globals.PLAN_FLAG && v19 > _vm->_graphicsManager.ofscroll - 30 && v19 < _vm->_graphicsManager.ofscroll + 50 && (uint16)(v0 + 29) <= 0x4Eu) {
		v1 = _vm->_eventsManager.btsouris;
		INVENTFLAG = 1;
		INVENT();
		INVENTFLAG = 0;
		KEY_INVENT = 0;
		if (!_vm->_globals.SORTIE) {
			INVENTFLAG = 0;
			_vm->_eventsManager.btsouris = v1;
		}
		return;
	}
	if (_vm->_globals.SAUVEGARDE->data[svField354] == 1
	        && !_vm->_globals.PLAN_FLAG
	        && (uint16)(v19 - 533) <= 0x1Au
	        && (uint16)(v0 - 26) <= 0x21u) {
		CHANGE_TETE(1, 0);
		return;
	}
	if (_vm->_globals.SAUVEGARDE->data[svField356] == 1
	        && !_vm->_globals.PLAN_FLAG
	        && (uint16)(v19 - 533) <= 0x1Au
	        && (uint16)(v0 - 26) <= 0x21u) {
		CHANGE_TETE(2, 0);
		return;
	}
	if (_vm->_globals.SAUVEGARDE->data[svField357] == 1) {
		if (_vm->_globals.SAUVEGARDE->data[svField353] == 1
		        && !_vm->_globals.PLAN_FLAG
		        && (uint16)(v19 - 533) <= 0x1Au
		        && (uint16)(v0 - 26) <= 0x21u) {
			CHANGE_TETE(0, 1);
			return;
		}
		if (_vm->_globals.SAUVEGARDE->data[svField355] == 1
		        && !_vm->_globals.PLAN_FLAG
		        && (uint16)(v19 - 567) <= 0x1Au
		        && (uint16)(v0 - 26) <= 0x21u) {
			CHANGE_TETE(0, 2);
			return;
		}
	}
	if (_vm->_globals.PLAN_FLAG == 1) {
		if (GOACTION != 1)
			goto LABEL_38;
		VERIFZONE();
		if (NUMZONE <= 0)
			return;
		v2 = 0;
		v3 = _vm->_globals.essai2;
		v4 = _vm->_globals.chemin;
		do {
			WRITE_LE_UINT16(v3 + 2 * v2, READ_LE_UINT16(v4 + 2 * v2));
			++v2;
		} while ((int16)READ_LE_UINT16(v4 + 2 * v2) != -1);
		v5 = _vm->_globals.essai2;
		WRITE_LE_UINT16(_vm->_globals.essai2 + 2 * v2, (uint16)-1);
		WRITE_LE_UINT16(v5 + 2 * v2 + 2, (uint16)-1);
		WRITE_LE_UINT16(v5 + 2 * v2 + 4, (uint16)-1);
		WRITE_LE_UINT16(v5 + 2 * v2 + 6, (uint16)-1);
	}
	if (GOACTION == 1) {
		VERIFZONE();
		GOACTION = 0;
		_vm->_globals.SAUVEGARDE->data[svField1] = 0;
		_vm->_globals.SAUVEGARDE->data[svField2] = 0;
	}
LABEL_38:
	if (_vm->_globals.PLAN_FLAG == 1 && (_vm->_eventsManager.btsouris != 4 || NUMZONE <= 0))
		return;
	if ((uint16)(NUMZONE + 1) > 1u) {
		v6 = NUMZONE;
		v7 = _vm->_globals.ZONEP[v6].field0;
		if (v7) {
			v8 = _vm->_globals.ZONEP[v6].field2;
			if (v8) {
				if (v8 != 31) {
					v19 = v7;
					v0 = v8;
				}
			}
		}
	}
	GOACTION = 0;
	v9 = _vm->_globals.chemin;
	_vm->_globals.chemin = PTRNUL;
	if (_vm->_globals.FORET && ((uint16)(NUMZONE - 20) <= 1u || (uint16)(NUMZONE - 22) <= 1u)) {
		if ((signed __int16)YSPR(0) <= 374 || (signed __int16)YSPR(0) > 410) {
			v10 = XSPR(0);
			v11 = YSPR(0);
			v12 = XSPR(0);
			v13 = PARCOURS2(v12, v11, v10, 390);
			_vm->_globals.chemin = v13;
			if (PTRNUL != v13)
				PACOURS_PROPRE(v13);
			g_old_x = XSPR(0);
			g_old_y = YSPR(0);
			_vm->_globals.Compteur = 0;
			if (PTRNUL != _vm->_globals.chemin || v9 == _vm->_globals.chemin) {
LABEL_64:
				_vm->_globals.g_old_sens = -1;
				goto LABEL_65;
			}
			goto LABEL_63;
		}
		_vm->_globals.chemin = PTRNUL;
		SETANISPR(0, _vm->_globals.g_old_sens2 + 59);
		_vm->_globals.ACTION_SENS = 0;
		_vm->_globals.chemin = PTRNUL;
		VERIFTAILLE();
		SETFLIPSPR(0, 0);
		_vm->_globals.Compteur = 0;
		_vm->_globals.g_old_sens = -1;
		goto LABEL_65;
	}
	if (!_vm->_globals.NOMARCHE) {
		if (!_vm->_globals.PLAN_FLAG) {
			v14 = YSPR(0);
			v15 = XSPR(0);
			v16 = PARCOURS2(v15, v14, v19, v0);
			_vm->_globals.chemin = v16;
			if (PTRNUL != v16)
				PACOURS_PROPRE(v16);
			g_old_x = XSPR(0);
			g_old_y = YSPR(0);
			_vm->_globals.Compteur = 0;
			if (PTRNUL != _vm->_globals.chemin || v9 == _vm->_globals.chemin)
				goto LABEL_64;
LABEL_63:
			_vm->_globals.chemin = v9;
		}
LABEL_65:
		if (!_vm->_globals.NOMARCHE && _vm->_globals.PLAN_FLAG == 1) {
			v17 = YSPR(0);
			v18 = XSPR(0);
			_vm->_globals.chemin = PARC_VOITURE(v18, v17, v19, v0);
		}
	}
	if ((uint16)(NUMZONE + 1) > 1u) {
		// TODO: Reformat the weird if statement generated by the decompiler
		if (_vm->_eventsManager.btsouris == 23 || (_vm->_globals.SAUVEGARDE->data[svField1] = _vm->_eventsManager.btsouris, _vm->_eventsManager.btsouris == 23))
			_vm->_globals.SAUVEGARDE->data[svField1] = 5;
		if (_vm->_globals.PLAN_FLAG == 1)
			_vm->_globals.SAUVEGARDE->data[svField1] = 6;
		_vm->_globals.SAUVEGARDE->data[svField2] = NUMZONE;
		_vm->_globals.SAUVEGARDE->data[svField3] = _vm->_globals.OBJET_EN_COURS;
		GOACTION = 1;
	}
	_vm->_fontManager.TEXTE_OFF(5);
	_vm->_graphicsManager.SETCOLOR4(251, 100, 100, 100);
	ARRET_PERSO_FLAG = 0;
	if (_vm->_eventsManager.btsouris == 21 && _vm->_globals.BOBZONE[NUMZONE]) {
		ARRET_PERSO_FLAG = 1;
		ARRET_PERSO_NUM = _vm->_globals.BOBZONE[NUMZONE];
	}
	if (_vm->_globals.ECRAN == 20 && _vm->_globals.SAUVEGARDE->data[svField13] == 1 && _vm->_globals.OBJET_EN_COURS == 20 && NUMZONE == 12 
				&& _vm->_eventsManager.btsouris == 23) {
		_vm->_globals.chemin = PTRNUL;
		XSPR(0);
		YSPR(0);
	}
}

void ObjectsManager::PARADISE() {
	signed int v1; // esi@1
	char result; // al@1
	int v3; // eax@11
	unsigned __int16 v4; // ax@19
	int v5; // eax@24
	unsigned __int16 v6; // ax@33

	v1 = 0;
	ARRET_PERSO_FLAG = 0;
	ARRET_PERSO_NUM = 0;
	result = _vm->_globals.SAUVEGARDE->data[svField1];
	if (result && _vm->_globals.SAUVEGARDE->data[svField2] && result != 4 && result > 3) {
		_vm->_fontManager.TEXTE_OFF(5);
		if (_vm->_globals.FORET != 1 || (unsigned __int16)(NUMZONE - 20) > 1u && (unsigned __int16)(NUMZONE - 22) > 1u) {
			if (_vm->_graphicsManager.DOUBLE_ECRAN == 1) {
				_vm->_graphicsManager.no_scroll = 2;
				if (_vm->_eventsManager.start_x >= (signed __int16)XSPR(0) - 320)
					goto LABEL_64;
				v3 = _vm->_eventsManager.start_x + 320 - (signed __int16)XSPR(0);
				if (v3 < 0)
					v3 = -v3;
				if (v3 <= 160) {
LABEL_64:
					if (_vm->_eventsManager.start_x > (signed __int16)XSPR(0) - 320) {
						v5 = _vm->_eventsManager.start_x + 320 - (signed __int16)XSPR(0);
						if (v5 < 0)
							v5 = -v5;
						if (v5 > 160) {
							_vm->_graphicsManager.no_scroll = 2;
							do {
								_vm->_graphicsManager.SCROLL -= _vm->_graphicsManager.SPEED_SCROLL;
								if (_vm->_graphicsManager.SCROLL < 0) {
									_vm->_graphicsManager.SCROLL = 0;
									v1 = 1;
								}
								if (_vm->_graphicsManager.SCROLL > 640) {
									_vm->_graphicsManager.SCROLL = 640;
									v1 = 1;
								}
								if ((signed __int16)_vm->_eventsManager.XMOUSE() > _vm->_graphicsManager.SCROLL + 620) {
									v6 = _vm->_eventsManager.YMOUSE();
									_vm->_eventsManager.souris_xy(_vm->_eventsManager.souris_x - 4, v6);
								}
								_vm->_eventsManager.VBL();
							} while (v1 != 1 && _vm->_eventsManager.start_x > (signed __int16)XSPR(0) - 320);
						}
					}
				} else {
					do {
						_vm->_graphicsManager.SCROLL += _vm->_graphicsManager.SPEED_SCROLL;
						if (_vm->_graphicsManager.SCROLL < 0) {
							_vm->_graphicsManager.SCROLL = 0;
							v1 = 1;
						}
						if (_vm->_graphicsManager.SCROLL > 640) {
							_vm->_graphicsManager.SCROLL = 640;
							v1 = 1;
						}
						if ((signed __int16)_vm->_eventsManager.XMOUSE() < _vm->_graphicsManager.SCROLL + 10) {
							v4 = _vm->_eventsManager.YMOUSE();
							_vm->_eventsManager.souris_xy(_vm->_eventsManager.souris_x + 4, v4);
						}
						_vm->_eventsManager.VBL();
					} while (v1 != 1 && _vm->_eventsManager.start_x < (signed __int16)XSPR(0) - 320);
				}
				if ((signed __int16)_vm->_eventsManager.XMOUSE() > _vm->_graphicsManager.SCROLL + 620)
					_vm->_eventsManager.souris_xy(_vm->_graphicsManager.SCROLL + 610, 0);
				if ((signed __int16)_vm->_eventsManager.XMOUSE() < _vm->_graphicsManager.SCROLL + 10)
					_vm->_eventsManager.souris_xy(_vm->_graphicsManager.SCROLL + 10, 0);
				_vm->_eventsManager.VBL();
				_vm->_graphicsManager.no_scroll = 0;
			}
			_vm->_talkManager.REPONSE(_vm->_globals.SAUVEGARDE->data[svField2], _vm->_globals.SAUVEGARDE->data[svField1]);
		} else {
			_vm->_talkManager.REPONSE2(_vm->_globals.SAUVEGARDE->data[svField2], _vm->_globals.SAUVEGARDE->data[svField1]);
		}
		_vm->_eventsManager.CHANGE_MOUSE(4);
		if ((unsigned __int16)(NUMZONE + 1) > 1u && !_vm->_globals.ZONEP[NUMZONE].field16) {
			NUMZONE = -1;
			FORCEZONE = 1;
		}
		if (NUMZONE != _vm->_globals.SAUVEGARDE->data[svField2] || (unsigned __int16)(NUMZONE + 1) <= 1u) {
			_vm->_eventsManager.btsouris = 4;
			CHANGEVERBE = 0;
		} else {
			_vm->_eventsManager.btsouris = _vm->_globals.SAUVEGARDE->data[svField1];
			if (CHANGEVERBE == 1) {
				VERBEPLUS();
				CHANGEVERBE = 0;
			}
			if (_vm->_eventsManager.btsouris == 5)
				_vm->_eventsManager.btsouris = 4;
		}
		if (_vm->_eventsManager.btsouris != 23)
			_vm->_eventsManager.CHANGE_MOUSE(_vm->_eventsManager.btsouris);
		NUMZONE = 0;
		_vm->_globals.SAUVEGARDE->data[svField1] = 0;
		_vm->_globals.SAUVEGARDE->data[svField2] = 0;
	}
	if (_vm->_globals.PLAN_FLAG == 1) {
		_vm->_eventsManager.btsouris = 0;
		_vm->_eventsManager.CHANGE_MOUSE(0);
	}
	if (_vm->_globals.NOMARCHE == 1) {
		if (_vm->_eventsManager.btsouris == 4) {
			result = NUMZONE + 1;
			if ((unsigned __int16)(NUMZONE + 1) > 1u)
				BTDROITE();
		}
	}
	GOACTION = 0;
}

void ObjectsManager::CLEAR_ECRAN() {
	__int16 v1;
	int v2;

	CLEAR_SPR();
	_vm->_graphicsManager.FIN_VISU();
	_vm->_fontManager.TEXTE_OFF(5);
	_vm->_fontManager.TEXTE_OFF(9);
	_vm->_globals.CLEAR_VBOB();
	_vm->_animationManager.CLEAR_ANIM();
	CLEAR_ZONE();
	RESET_OBSTACLE();
	_vm->_globals.RESET_CACHE();

	v1 = 0;
	do {
		v2 = v1;
		_vm->_globals.BOBZONE[v2] = 0;
		_vm->_globals.BOBZONE_FLAG[v2] = 0;
		++v1;
	} while (v1 <= 48);
	_vm->_eventsManager.btsouris = 4;
	verbe = 4;
	NUMZONE = 0;
	Vold_taille = 0;
	SPEED_FLAG = 0;
	SPEED_PTR = PTRNUL;
	SPEED_X = 0;
	SPEED_Y = 0;
	SPEED_IMAGE = 0;
	FORCEZONE = 1;
	TOTAL_LIGNES = 0;
	DERLIGNE = 0;
	_vm->_globals.chemin = PTRNUL;
	if (_vm->_globals.COUCOU != PTRNUL)
		_vm->_globals.COUCOU = FileManager::LIBERE_FICHIER(_vm->_globals.COUCOU);
	if (PTRNUL != _vm->_globals.SPRITE_ECRAN)
		_vm->_globals.SPRITE_ECRAN = FileManager::LIBERE_FICHIER(_vm->_globals.SPRITE_ECRAN);
	_vm->_eventsManager.start_x = 0;
	_vm->_eventsManager.souris_n = 0;
	Vold_taille = 200;
	_vm->_globals.SAUVEGARDE->data[svField1] = 0;
	_vm->_globals.SAUVEGARDE->data[svField2] = 0;
	GOACTION = 0;
	FORCEZONE = 1;
	CHANGEVERBE = 0;
	_vm->_globals.NOSPRECRAN = 0;
	_vm->_globals.chemin = PTRNUL;
	g_old_sens = -1;
	my_anim = 1;
	A_ANIM = 0;
	MA_ANIM = 0;
	MA_ANIM1 = 0;
	A_DEPA = 0;
	MAX_DEPA = 0;
	MAX_DEPA1 = 0;
	_vm->_graphicsManager.RESET_SEGMENT_VESA();
}

void ObjectsManager::INVENT() {
	__int16 v1; 
	size_t filesize; 
	__int16 v4;
	signed __int16 v5; 
	__int16 v6; 
	byte *v7; 
	__int16 v8; 
	signed int v9; 
	__int16 v10; 
	__int16 v11; 
	__int16 v12; 
	__int16 v13; 
	signed __int16 v14; 
	__int16 v15; 
	__int16 v16;
	__int16 v17;
	__int16 v18;
	__int16 v19;
	signed __int16 v20; 
	Common::File f;

	v13 = 0;
	if (VIRE_INVENT != 1 && AFFINVEN != 1 && _vm->_globals.DESACTIVE_INVENT != 1) {
		_vm->_graphicsManager.no_scroll = 1;
		FLAG_VISIBLE_EFFACE = 4;
		FLAG_VISIBLE = 0;
		v1 = 0;
		do {
			INVENT_ANIM();
			_vm->_eventsManager.XMOUSE();
			_vm->_eventsManager.YMOUSE();
			_vm->_eventsManager.VBL();
			++v1;
		} while (v1 <= 1);
		_vm->_globals.Winventaire = PTRNUL;
LABEL_7:
		_vm->_eventsManager.souris_bb = 0;
		_vm->_eventsManager.souris_b = 0;
		_vm->_globals.DESACTIVE_INVENT = 1;
		_vm->_graphicsManager.SETCOLOR4(251, 100, 100, 100);
		if (_vm->_globals.FR == 1)
			FileManager::CONSTRUIT_SYSTEM("INVENTFR.SPR");
		if (!_vm->_globals.FR)
			FileManager::CONSTRUIT_SYSTEM("INVENTAN.SPR");
		if (_vm->_globals.FR == 2)
			FileManager::CONSTRUIT_SYSTEM("INVENTES.SPR");

		if (!f.open(_vm->_globals.NFICHIER))
			error("Error opening file - %s", _vm->_globals.NFICHIER.c_str());

		filesize = f.size();
		_vm->_globals.Winventaire = _vm->_globals.dos_malloc2(filesize);
		FileManager::bload_it(f, _vm->_globals.Winventaire, filesize);
		f.close();

		FileManager::CONSTRUIT_SYSTEM("INVENT2.SPR");
		inventaire2 = FileManager::CHARGE_FICHIER(_vm->_globals.NFICHIER);
		v19 = _vm->_graphicsManager.ofscroll + 152;
		v18 = _vm->_objectsManager.Get_Largeur(_vm->_globals.Winventaire, 0);
		v17 = _vm->_objectsManager.Get_Hauteur(_vm->_globals.Winventaire, 0);
		inventairex = v19;
		inventairey = 114;
		inventairel = v18;
		inventaireh = v17;
		_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.Winventaire, v19 + 300, 414, 0, 0, 0, 0);
		v15 = 0;
		v4 = 0;
		v14 = 1;
		do {
			v16 = 0;
			v5 = 1;
			do {
				++v4;
				v6 = _vm->_globals.INVENTAIRE[v4];
				if (v6 && v4 <= 29) {
					v7 = CAPTURE_OBJET(v6, 0);
					_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager.VESA_BUFFER, v7, v19 + v16 + 6, v15 + 120, _vm->_globals.OBJL, _vm->_globals.OBJH);
					_vm->_globals.dos_free2(v7);
				}
				v16 += 54;
				++v5;
			} while (v5 <= 6);
			v15 += 38;
			++v14;
		} while (v14 <= 5);
		_vm->_graphicsManager.Capture_Mem(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.Winventaire, inventairex, inventairey, inventairel, inventaireh);
		_vm->_eventsManager.souris_bb = 0;
		v20 = 0;
		while (1) {
			AFFINVEN = 1;
			v8 = _vm->_eventsManager.XMOUSE();
			v9 = _vm->_eventsManager.YMOUSE();
			v12 = _vm->_eventsManager.BMOUSE();
			v10 = v13;
			v11 = ZONE_OBJET(v8, v9);
			v13 = v11;
			if (v11 != v10)
				PARAMCADRE(v11);
			if (_vm->_eventsManager.btsouris != 16) {
				if ((unsigned __int16)(_vm->_eventsManager.btsouris - 1) > 1u) {
					if (_vm->_eventsManager.btsouris != 3) {
						if (v12 == 2) {
							OBJETPLUS(v13);
							if (_vm->_eventsManager.btsouris != 23)
								_vm->_eventsManager.CHANGE_MOUSE(_vm->_eventsManager.btsouris);
						}
					}
				}
			}
			if (v12 == 1) {
				if (_vm->_eventsManager.btsouris == 1 || _vm->_eventsManager.btsouris == 16 || !_vm->_eventsManager.btsouris || (unsigned __int16)(_vm->_eventsManager.btsouris - 2) <= 1u)
					break;
				v9 = v13;
				VALID_OBJET(_vm->_globals.INVENTAIRE[v13]);
				if (_vm->_eventsManager.btsouris == 8)
					v20 = 1;
				if (v20 != 1) {
					TRAVAILOBJET = 1;
					_vm->_globals.SAUVEGARDE->data[svField3] = _vm->_globals.OBJET_EN_COURS;
					_vm->_globals.SAUVEGARDE->data[svField8] = _vm->_globals.INVENTAIRE[v13];
					_vm->_globals.SAUVEGARDE->data[svField9] = _vm->_eventsManager.btsouris;
					OPTI_OBJET();
					TRAVAILOBJET = 0;
					if (_vm->_soundManager.VOICEOFF == 1) {
						do
							_vm->_eventsManager.VBL();
						while (!_vm->_globals.SORTIE && _vm->_eventsManager.BMOUSE() != 1);
						_vm->_fontManager.TEXTE_OFF(9);
					}
					if (_vm->_globals.SORTIE) {
						if (_vm->_globals.SORTIE == 2)
							v20 = 1;
						_vm->_globals.SORTIE = 0;
						if (v20 != 1) {
							inventaire2 = _vm->_globals.dos_free2(inventaire2);
							if (PTRNUL != _vm->_globals.Winventaire)
								_vm->_globals.Winventaire = _vm->_globals.dos_free2(_vm->_globals.Winventaire);
							goto LABEL_7;
						}
					} else if (v20 != 1) {
						AFFINVEN = 1;
					}
				}
			}
			if (VIRE_INVENT == 1)
				v20 = 1;
			if (v20 == 1)
				break;
			_vm->_eventsManager.VBL();
			if ((unsigned __int16)(_vm->_globals.ECRAN - 35) <= 5u)
				SPECIAL_JEU(v9);
		}
		_vm->_fontManager.TEXTE_OFF(9);
		if (AFFINVEN == 1) {
			AFFINVEN = 0;
			v9 = 114;
			_vm->_graphicsManager.SCOPY(_vm->_graphicsManager.VESA_SCREEN, v19, 114, v18, v17, _vm->_graphicsManager.VESA_BUFFER, v19, 114);
			_vm->_graphicsManager.Ajoute_Segment_Vesa(v19, 114, v19 + v18, v18 + 114);
			BOBTOUS = 1;
		}
		if (PTRNUL != _vm->_globals.Winventaire)
			_vm->_globals.Winventaire = _vm->_globals.dos_free2(_vm->_globals.Winventaire);
		inventaire2 = _vm->_globals.dos_free2(inventaire2);
		if (_vm->_eventsManager.btsouris == 1)
			OptionsDialog::show(_vm);
		if (_vm->_eventsManager.btsouris == 3)
			_vm->_menuManager.CHARGE_PARTIE();//v9);
		if (_vm->_eventsManager.btsouris == 2)
			_vm->_menuManager.SAUVE_PARTIE();//v9);
		_vm->_eventsManager.btsouris = 4;
		_vm->_eventsManager.CHANGE_MOUSE(4);
		old_cady = 0;
		cady = 0;
		old_cadx = 0;
		cadx = 0;
		_vm->_globals.DESACTIVE_INVENT = 0;
		_vm->_graphicsManager.no_scroll = 0;
	}
}

void ObjectsManager::CHANGE_TETE(int a1, int a2) {
	int v2; 
	Sauvegarde1 *v3; 
	Sauvegarde1 *v4; 
	Sauvegarde1 *v5; 
	Sauvegarde1 *v6; 
	Sauvegarde1 *v7; 
	Sauvegarde1 *v8; 
	Sauvegarde1 *v9; 

	CH_TETE = 1;
	_vm->_graphicsManager.SCOPY(_vm->_graphicsManager.VESA_SCREEN, 532, 25, 65, 40, _vm->_graphicsManager.VESA_BUFFER, 532, 25);
	_vm->_graphicsManager.Ajoute_Segment_Vesa(532, 25, 597, 65);
	_vm->_globals.NOT_VERIF = 1;
	_vm->_globals.chemin = PTRNUL;
	if (a1 == 2) {
		if (!a2) {
			if (_vm->_globals.SAUVEGARDE->data[svField188] == _vm->_globals.ECRAN) {
				CH_TETE = 0;
				v3 = &_vm->_globals.SAUVEGARDE->field380;
				v3->field0 = XSPR(0);
				v3->field1 = YSPR(0);
				v3->field2 = 64;
				v3->field3 = _vm->_globals.ECRAN;
				v3->field4 = Sprite[12].field0;	// TODO: Double-check this
				SPRITE_OFF(1);
				SPRITE(_vm->_globals.TETE, v3->field0, v3->field1, 1, 3, v3->field4, 0, 20, 127);
				SPRITE_ON(1);
				SPRITE_OFF(0);
				_vm->_globals.SAUVEGARDE->data[svField354] = 0;
				_vm->_globals.SAUVEGARDE->data[svField356] = 0;
				_vm->_globals.SAUVEGARDE->data[svField357] = 1;
				T_RECTIF = 0;
				v4 = &_vm->_globals.SAUVEGARDE->field370;
				FileManager::CONSTRUIT_SYSTEM("PERSO.SPR");
				FileManager::CHARGE_FICHIER2(_vm->_globals.NFICHIER, _vm->_globals.PERSO);
				_vm->_globals.PERSO_TYPE = 0;
				SPRITE(_vm->_globals.PERSO, v4->field0, v4->field1, 0, 64, v4->field4, 0, 34, 190);
LABEL_9:
				SPRITE_ON(0);
				return;
			}
		}
	}
	if (!a1) {
		if (a2 == 2 && _vm->_globals.SAUVEGARDE->data[svField193] == _vm->_globals.ECRAN) {
			CH_TETE = 0;
			v5 = &_vm->_globals.SAUVEGARDE->field370;
			v5->field0 = XSPR(0);
			v5->field1 = YSPR(0);
			v5->field2 = 64;
			v5->field3 = _vm->_globals.ECRAN;
			v5->field4 = Sprite[12].field0;
			SPRITE_OFF(1);
			SPRITE(_vm->_globals.TETE, v5->field0, v5->field1, 1, 2, v5->field4, 0, 34, 190);
			SPRITE_ON(1);
			SPRITE_OFF(0);
			_vm->_globals.SAUVEGARDE->data[svField354] = 0;
			_vm->_globals.SAUVEGARDE->data[svField356] = 1;
			_vm->_globals.SAUVEGARDE->data[svField357] = 0;
			v6 = &_vm->_globals.SAUVEGARDE->field380;
			FileManager::CONSTRUIT_SYSTEM("PSAMAN.SPR");
			FileManager::CHARGE_FICHIER2(_vm->_globals.NFICHIER, _vm->_globals.PERSO);
			_vm->_globals.PERSO_TYPE = 2;
			SPRITE(_vm->_globals.PERSO, v6->field0, v6->field1, 0, 64, v6->field4, 0, 20, 127);
			goto LABEL_9;
		}
		v7 = &_vm->_globals.SAUVEGARDE->field370;
		v7->field0 = XSPR(0);
		v7->field1 = YSPR(0);
		v7->field2 = 64;
		v7->field3 = _vm->_globals.ECRAN;
		v7->field4 = Sprite[12].field0;
	}
	if (a1 == 1) {
		v8 = &_vm->_globals.SAUVEGARDE->field360;
		v8->field0 = XSPR(0);
		v8->field1 = YSPR(0);
		v8->field2 = 64;
		v8->field3 = _vm->_globals.ECRAN;
		v8->field4 = Sprite[12].field0;
	}
	if (a1 == 2) {
		v9 = &_vm->_globals.SAUVEGARDE->field380;
		v9->field0 = XSPR(0);
		v9->field1 = YSPR(0);
		v9->field2 = 64;
		v9->field3 = _vm->_globals.ECRAN;
		v9->field4 = Sprite[12].field0;
	}
	if (!a2) {
		_vm->_globals.SAUVEGARDE->data[svField121] = 0;
		_vm->_globals.SAUVEGARDE->data[svField354] = 0;
		_vm->_globals.SAUVEGARDE->data[svField356] = 0;
		_vm->_globals.SAUVEGARDE->data[svField357] = 1;
		_vm->_globals.SORTIE = _vm->_globals.SAUVEGARDE->data[svField188];
	}
	if (a2 == 1) {
		_vm->_globals.SAUVEGARDE->data[svField121] = 1;
		_vm->_globals.SAUVEGARDE->data[svField354] = 1;
		_vm->_globals.SAUVEGARDE->data[svField356] = 0;
		_vm->_globals.SAUVEGARDE->data[svField357] = 0;
		_vm->_globals.SORTIE = _vm->_globals.SAUVEGARDE->data[svField183];
	}
	if (a2 == 2) {
		_vm->_globals.SAUVEGARDE->data[svField121] = 0;
		_vm->_globals.SAUVEGARDE->data[svField354] = 0;
		_vm->_globals.SAUVEGARDE->data[svField356] = 1;
		_vm->_globals.SAUVEGARDE->data[svField357] = 0;
		v2 = _vm->_globals.SAUVEGARDE->data[svField193];
		_vm->_globals.SORTIE = _vm->_globals.SAUVEGARDE->data[svField193];
	}
}

byte *ObjectsManager::PARCOURS2(int a1, int a2, int a3, int a4) {
	return NULL; //_vm->_globals.STOP_BUG == 0 ? NULL : NULL;
	/*
	int v4; 
	int v5; 
	int v6; 
	int v7; 
	int v8; 
	int v9; 
	int v10;
	int v11; 
	int v12; 
	int v13; 
	int v14; 
	int v15; 
	int v16; 
	int v17; 
	int v18; 
	int v19; 
	int v20;
	int v21; 
	int v22;
	int v23; 
	int v24; 
	int v25; 
	int v26;
	int v27; 
	int v28; 
	int v29; 
	int v31;
	int v32; 
	int v33; 
	int v34; 
	int v35; 
	int v36;
	int v37;
	int v38;
	int v39;
	int v40; 
	int v41; 
	int v42;
	int v43; 
	int v44; 
	int v45; 
	int v46;
	int v47;
	int v48; 
	int v49; 
	int v50; 
	int v51;
	int v52; 
	int v53; 
	int v54; 
	int v55;
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
	int v66; 
	int v67;
	int v68;
	int i;
	byte *v70; 
	int v71; 
	int v72; 
	int j; 
	byte *v74;
	int v75; 
	int v76;
	int v77; 
	int v78; 
	int v79; 
	int v80;
	byte *v81; 
	int v82; 
	int v83; 
	byte *v84;
	int v85; 
	int v86;
	int v87; 
	int v88; 
	int v89; 
	int v90; 
	byte *v91; 
	int v92; 
	int v93; 
	int v94;
	byte *v95; 
	int v96; 
	int v97;
	int v98; 
	int v99; 
	int v100; 
	int v101; 
	int v102; 
	int v103; 
	int v104;
	int v105; 
	int v106; 
	int v107; 
	int v108;
	int v109;
	int v110; 
	int v111; 
	int v112; 
	int v113; 
	int v114; 
	int v115; 
	int v116;
	int v117; 
	int v118; 
	int v119; 
	int v120; 
	int v121; 
	int v122;
	int v123; 
	int v124; 
	int v125;
	int v126; 
	int v127;
	int v128; 
	int v129; 
	int v130; 
	int v131; 
	int v132; 
	int v133; 
	int v134; 
	int v135;
	int v136; 
	int v137; 
	int v138;
	int v139;
	int v140;
	int v141; 
	int v142;
	int v143; 
	int v144; 
	int v145; 

	v123 = a3;
	v122 = a4;
	v121 = 0;
	v120 = 0;
	v115 = 0;
	v114 = 0;
	v113 = 0;
	v111 = 0;
	if (a4 <= 24)
		v122 = 25;
	if (!_vm->_globals.NOT_VERIF) {
		v4 = a1 - _vm->_globals.old_x1_65;
		if (v4 < 0)
			v4 = -v4;
		if (v4 <= 4) {
			v5 = a2 - _vm->_globals.old_y1_66;
			if (v5 < 0)
				v5 = -v5;
			if (v5 <= 4) {
				v6 = _vm->_globals.old_x2_67 - a3;
				if (v6 < 0)
					v6 = -v6;
				if (v6 <= 4) {
					v7 = _vm->_globals.old_y2_68 - v122;
					if (v7 < 0)
						v7 = -v7;
					if (v7 <= 4)
						return PTRNUL;
				}
			}
		}
		v8 = a1 - a3;
		if (v8 < 0)
			v8 = -v8;
		if (v8 <= 4) {
			v9 = a2 - v122;
			if (v9 < 0)
				v9 = -v9;
			if (v9 <= 4)
				return PTRNUL;
		}
		if (_vm->_globals.old_z_69 > 0 && _vm->_objectsManager.NUMZONE > 0 && _vm->_globals.old_z_69 == _vm->_objectsManager.NUMZONE)
			return PTRNUL;
	}
	_vm->_globals.NOT_VERIF = 0;
	_vm->_globals.old_z_69 = _vm->_objectsManager.NUMZONE;
	_vm->_globals.old_x1_65 = a1;
	_vm->_globals.old_x2_67 = a3;
	_vm->_globals.old_y1_66 = a2;
	_vm->_globals.old_y2_68 = v122;
	_vm->_globals.STOP_BUG = 0;
	v112 = 0;
	if (a3 <= 19)
		v123 = 20;
	if (v122 <= 19)
		v122 = 20;
	if (v123 > _vm->_graphicsManager.max_x - 10)
		v123 = _vm->_graphicsManager.max_x - 10;
	if (v122 > _vm->_globals.Max_Perso_Y)
		v122 = _vm->_globals.Max_Perso_Y;
	v10 = a1 - v123;
	if (v10 < 0)
		v10 = -v10;
	if (v10 <= 3) {
		v11 = a2 - v122;
		if (v11 < 0)
			v11 = -v11;
		if (v11 <= 3)
			return PTRNUL;
	}
	v12 = 0;
	do {
		v13 = v12;
		*(&v141 + v13) = -1;
		*(&v136 + v13) = 0;
		*(&v131 + v13) = 1300;
		*(&v126 + v13) = 1300;
		++v12;
	} while (v12 <= 8);
	if (PARC_PERS(a1, a2, v123, v122, -1, -1, 0) != 1) {
		v14 = 0;
		v15 = v122;
		if (_vm->_graphicsManager.max_y > v122) {
			v16 = 5;
			do {
				v101 = v16;
				v17 = _vm->_graphicsManager.colision2_ligne(v123, v15, (int)&v139, (int)&v144, 0, _vm->_objectsManager.DERLIGNE);
				v16 = v101;
				if (v17 == 1 && *(&v141 + v101) <= _vm->_objectsManager.DERLIGNE)
					break;
				*(&v136 + v101) = 0;
				*(&v141 + v101) = -1;
				++v14;
				++v15;
			} while (_vm->_graphicsManager.max_y > v15);
		}
		v134 = v14;
		v18 = 0;
		v19 = v122;
		if (_vm->_graphicsManager.min_y < v122) {
			v20 = 1;
			do {
				v102 = v20;
				v21 = _vm->_graphicsManager.colision2_ligne(v123, v19, (int)&v137, (int)&v142, 0, _vm->_objectsManager.DERLIGNE);
				v20 = v102;
				if (v21 == 1 && *(&v141 + v102) <= _vm->_objectsManager.DERLIGNE)
					break;
				*(&v136 + v102) = 0;
				*(&v141 + v102) = -1;
				if (v134 < v18) {
					if (v144 != -1)
						break;
				}
				++v18;
				--v19;
			} while (_vm->_graphicsManager.min_y < v19);
		}
		v132 = v18;
		v22 = 0;
		v23 = v123;
		if (_vm->_graphicsManager.max_x > v123) {
			v24 = 3;
			do {
				v103 = v24;
				v25 = _vm->_graphicsManager.colision2_ligne(v23, v122, (int)&v138, (int)&v143, 0, _vm->_objectsManager.DERLIGNE);
				v24 = v103;
				if (v25 == 1 && *(&v141 + v103) <= _vm->_objectsManager.DERLIGNE)
					break;
				*(&v136 + v103) = 0;
				*(&v141 + v103) = -1;
				++v22;
				if (v132 < v22) {
					if (v142 != -1)
						break;
				}
				if (v134 < v22 && v144 != -1)
					break;
				++v23;
			} while (_vm->_graphicsManager.max_x > v23);
		}
		v133 = v22;
		v26 = 0;
		v27 = v123;
		if (_vm->_graphicsManager.min_x < v123) {
			v28 = 7;
			do {
				v104 = v28;
				v29 = _vm->_graphicsManager.colision2_ligne(v27, v122, &v140, &v145, 0, _vm->_objectsManager.DERLIGNE);
				v28 = v104;
				if (v29 == 1 && *(&v141 + v104) <= _vm->_objectsManager.DERLIGNE)
					break;
				*(&v136 + v104) = 0;
				*(&v141 + v104) = -1;
				++v26;
				if (v132 < v26) {
					if (v142 != -1)
						break;
				}
				if (v134 < v26 && v144 != -1)
					break;
				if (v133 < v26 && v143 != -1)
					break;
				--v27;
			} while (_vm->_graphicsManager.min_x < v27);
		}
		v135 = v26;
		if (v142 < 0 || _vm->_objectsManager.DERLIGNE < v142)
			v142 = -1;
		if (v143 < 0 || _vm->_objectsManager.DERLIGNE < v143)
			v143 = -1;
		if (v144 < 0 || _vm->_objectsManager.DERLIGNE < v144)
			v144 = -1;
		if (v145 < 0 || _vm->_objectsManager.DERLIGNE < v145)
			v145 = -1;
		if (v142 < 0)
			v132 = 1300;
		if (v143 < 0)
			v133 = 1300;
		if (v144 < 0)
			v134 = 1300;
		if (v145 < 0)
			v135 = 1300;
		if (v142 == -1 && v143 == -1 && v144 == -1 && v145 == -1)
			return PTRNUL;
		v31 = 0;
		if (v144 != -1 && v132 >= v134 && v133 >= v134 && v135 >= v134) {
			v121 = v144;
			v120 = v139;
			v31 = 1;
		}
		if (v142 != -1 && !v31 && v134 >= v132 && v133 >= v132 && v135 >= v132) {
			v121 = v142;
			v120 = v137;
			v31 = 1;
		}
		if (v143 != -1 && !v31 && v132 >= v133 && v134 >= v133 && v135 >= v133) {
			v121 = v143;
			v120 = v138;
			v31 = 1;
		}
		if (v145 != -1 && !v31 && v134 >= v135 && v133 >= v135 && v132 >= v135) {
			v121 = v145;
			v120 = v140;
		}
		v32 = 0;
		do {
			v33 = v32;
			*(&v141 + v33) = -1;
			*(&v136 + v33) = 0;
			*(&v131 + v33) = 1300;
			*(&v126 + v33) = 1300;
			++v32;
		} while (v32 <= 8);
		v34 = 0;
		v35 = a2;
		if (_vm->_graphicsManager.max_y > a2) {
			v36 = 5;
			do {
				v105 = v36;
				v37 = _vm->_graphicsManager.colision2_ligne(a1, v35, (int)&v139, (int)&v144, 0, _vm->_objectsManager.DERLIGNE);
				v36 = v105;
				if (v37 == 1 && *(&v141 + v105) <= _vm->_objectsManager.DERLIGNE)
					break;
				*(&v136 + v105) = 0;
				*(&v141 + v105) = -1;
				++v34;
				++v35;
			} while (_vm->_graphicsManager.max_y > v35);
		}
		v134 = v34 + 1;
		v38 = 0;
		v39 = a2;
		if (_vm->_graphicsManager.min_y < a2) {
			v40 = 1;
			do {
				v106 = v40;
				v41 = _vm->_graphicsManager.colision2_ligne(a1, v39, (int)&v137, (int)&v142, 0, _vm->_objectsManager.DERLIGNE);
				v40 = v106;
				if (v41 == 1 && *(&v141 + v106) <= _vm->_objectsManager.DERLIGNE)
					break;
				*(&v136 + v106) = 0;
				*(&v141 + v106) = -1;
				++v38;
				if (v144 != -1) {
					if (v38 > 80)
						break;
				}
				--v39;
			} while (_vm->_graphicsManager.min_y < v39);
		}
		v132 = v38 + 1;
		v42 = 0;
		v43 = a1;
		if (_vm->_graphicsManager.max_x > a1) {
			v44 = 3;
			do {
				v107 = v44;
				v45 = _vm->_graphicsManager.colision2_ligne(v43, a2, (int)&v138, (int)&v143, 0, _vm->_objectsManager.DERLIGNE);
				v44 = v107;
				if (v45 == 1 && *(&v141 + v107) <= _vm->_objectsManager.DERLIGNE)
					break;
				*(&v136 + v107) = 0;
				*(&v141 + v107) = -1;
				++v42;
				if (v144 != -1 || v142 != -1) {
					if (v42 > 100)
						break;
				}
				++v43;
			} while (_vm->_graphicsManager.max_x > v43);
		}
		v133 = v42 + 1;
		v46 = 0;
		v47 = a1;
		if (_vm->_graphicsManager.min_x < a1) {
			v48 = 7;
			do {
				v108 = v48;
				v49 = _vm->_graphicsManager.colision2_ligne(v47, a2, (int)&v140, (int)&v145, 0, _vm->_objectsManager.DERLIGNE);
				v48 = v108;
				if (v49 == 1 && *(&v141 + v108) <= _vm->_objectsManager.DERLIGNE)
					break;
				*(&v136 + v108) = 0;
				*(&v141 + v108) = -1;
				++v46;
				if (v144 != -1 || v142 != -1 || v143 != -1) {
					if (v46 > 100)
						break;
				}
				--v47;
			} while (_vm->_graphicsManager.min_x < v47);
		}
		v135 = v46 + 1;
		if (v142 != -1) {
			v50 = v142 - v121;
			if (v50 < 0)
				v50 = -v50;
			v127 = v50;
		}
		if (v143 != -1) {
			v51 = v143 - v121;
			if (v51 < 0)
				v51 = -v51;
			v128 = v51;
		}
		if (v144 != -1) {
			v52 = v144 - v121;
			if (v52 < 0)
				v52 = -v52;
			v129 = v52;
		}
		if (v145 != -1) {
			v53 = v145 - v121;
			if (v53 < 0)
				v53 = -v53;
			v130 = v53;
		}
		if (v142 == -1 && v143 == -1 && v144 == -1 && v145 == -1)
			error("ERREUR POINT PLUS PROCHE Du perso NON TROUVE");
		v54 = 0;
		if (v142 != -1 && v128 >= v127 && v129 >= v127 && v130 >= v127) {
			v54 = 1;
			v115 = v142;
			v111 = v132;
			v113 = 1;
			v114 = v137;
		}
		if (!v54) {
			if (v144 != -1 && v128 >= v129 && v127 >= v129 && v130 >= v129) {
				v54 = 1;
				v115 = v144;
				v111 = v134;
				v113 = 5;
				v114 = v139;
			}
			if (!v54) {
				if (v143 != -1 && v127 >= v128 && v129 >= v128 && v130 >= v128) {
					v54 = 1;
					v115 = v143;
					v111 = v133;
					v113 = 3;
					v114 = v138;
				}
				if (!v54 && v145 != -1 && v127 >= v130 && v129 >= v130 && v128 >= v130) {
					v115 = v145;
					v111 = v135;
					v113 = 7;
					v114 = v140;
				}
			}
		}
		v55 = PARC_PERS(a1, a2, v123, v122, v115, v121, 0);
		if (v55 != 1) {
			if (v55 == 2) {
LABEL_201:
				v115 = NV_LIGNEDEP;
				v114 = NV_LIGNEOFS;
				v112 = NV_POSI;
			} else {
				if (v113 == 1) {
					v56 = 0;
					if (v111 > 0) {
						do {
							if ((unsigned __int16)_vm->_graphicsManager.colision2_ligne(a1, a2 - v56, (int)&v125, (int)&v124, _vm->_objectsManager.DERLIGNE + 1, TOTAL_LIGNES) == 1
							        && _vm->_objectsManager.DERLIGNE < v124) {
								v57 = v112;
								v58 = GENIAL(v124, v125, a1, a2 - v56, a1, a2 - v111, v112, _vm->_globals.super_parcours, 4);
								if (v58 == -1)
									goto LABEL_282;
								v112 = v58;
								if (NVPY != -1)
									v56 = a2 - NVPY;
							}
							v59 = v112;
							_vm->_globals.super_parcours[v59] = a1;
							_vm->_globals.super_parcours[v59 + 1] = a2 - v56;
							_vm->_globals.super_parcours[v59 + 2] = 1;
							_vm->_globals.super_parcours[v59 + 3] = 0;
							v112 += 4;
							++v56;
						} while (v111 > v56);
					}
				}
				if (v113 == 5) {
					v60 = 0;
					if (v111 > 0) {
						do {
							if ((unsigned __int16)_vm->_graphicsManager.colision2_ligne(a1, v60 + a2, (int)&v125, (int)&v124, _vm->_objectsManager.DERLIGNE + 1, TOTAL_LIGNES) == 1
							        && _vm->_objectsManager.DERLIGNE < v124) {
								v57 = v112;
								v61 = GENIAL(v124, v125, a1, v60 + a2, a1, v111 + a2, v112, _vm->_globals.super_parcours, 4);
								if (v61 == -1)
									goto LABEL_282;
								v112 = v61;
								if (NVPY != -1)
									v60 = NVPY - a2;
							}
							v62 = v112;
							_vm->_globals.super_parcours[v62] = a1;
							_vm->_globals.super_parcours[v62 + 1] = v60 + a2;
							_vm->_globals.super_parcours[v62 + 2] = 5;
							_vm->_globals.super_parcours[v62 + 3] = 0;
							v112 += 4;
							++v60;
						} while (v111 > v60);
					}
				}
				if (v113 == 7) {
					v63 = 0;
					if (v111 > 0) {
						do {
							if ((unsigned __int16)_vm->_graphicsManager.colision2_ligne(a1 - v63, a2, (int)&v125, (int)&v124, _vm->_objectsManager.DERLIGNE + 1, TOTAL_LIGNES) == 1
							        && _vm->_objectsManager.DERLIGNE < v124) {
								v57 = v112;
								v64 = GENIAL(v124, v125, a1 - v63, a2, a1 - v111, a2, v112, _vm->_globals.super_parcours, 4);
								if (v64 == -1)
									goto LABEL_282;
								v112 = v64;
								if (NVPX != -1)
									v63 = a1 - NVPX;
							}
							v65 = v112;
							_vm->_globals.super_parcours[v65] = a1 - v63;
							_vm->_globals.super_parcours[v65 + 1] = a2;
							_vm->_globals.super_parcours[v65 + 2] = 7;
							_vm->_globals.super_parcours[v65 + 3] = 0;
							v112 += 4;
							++v63;
						} while (v111 > v63);
					}
				}
				if (v113 == 3) {
					v66 = 0;
					if (v111 > 0) {
						do {
							if ((unsigned __int16)_vm->_graphicsManager.colision2_ligne(v66 + a1, a2, (int)&v125, (int)&v124, _vm->_objectsManager.DERLIGNE + 1, TOTAL_LIGNES) == 1
							        && _vm->_objectsManager.DERLIGNE < v124) {
								v57 = v112;
								v67 = GENIAL(v124, v125, v66 + a1, a2, v111 + a1, a2, v112, _vm->_globals.super_parcours, 4);
								if (v67 == -1)
									goto LABEL_282;
								v112 = v67;
								if (NVPX != -1)
									v66 = NVPX - a1;
							}
							v68 = v112;
							_vm->_globals.super_parcours[v68] = v66 + a1;
							_vm->_globals.super_parcours[v68 + 1] = a2;
							_vm->_globals.super_parcours[v68 + 2] = 3;
							_vm->_globals.super_parcours[v68 + 3] = 0;
							v112 += 4;
							++v66;
						} while (v111 > v66);
					}
				}
			}
LABEL_234:
			if (v115 < v121) {
				for (i = v114; _vm->_globals.Ligne[v115].field0 > i; ++i) {
					v70 = _vm->_globals.Ligne[v115].fieldC;
					v119 = READ_LE_UINT16(v70 + 4 * i);
					v118 = READ_LE_UINT16(v70 + 4 * i + 2);
					v71 = v112;
					_vm->_globals.super_parcours[v71] = v119;
					_vm->_globals.super_parcours[v71 + 1] = v118;
					_vm->_globals.super_parcours[v71 + 2] = _vm->_globals.Ligne[v115].field6;
					_vm->_globals.super_parcours[v71 + 3] = 0;
					v112 += 4;
				}
				v116 = v115 + 1;
				if ((signed __int16)(v115 + 1) < v121) {
					do {
						v72 = 0;
						v110 = v116;
						for (j = v116; _vm->_globals.Ligne[j].field0 > v72; j = v116) {
							v74 = _vm->_globals.Ligne[v110].fieldC;
							v119 = READ_LE_UINT16(v74 + 4 * v72);
							v118 = READ_LE_UINT16(v74 + 4 * v72 + 2);
							v75 = v112;
							_vm->_globals.super_parcours[v75] = v119;
							_vm->_globals.super_parcours[v75 + 1] = v118;
							_vm->_globals.super_parcours[v75 + 2] = _vm->_globals.Ligne[v110].field6;
							_vm->_globals.super_parcours[v75 + 3] = 0;
							v112 += 4;
							v76 = _vm->_globals.Ligne[v110].field0;
							if (v76 > 30) {
								v77 = v76 / 2;
								if (v77 < 0)
									v77 = -v77;
								if (v72 == v77) {
									v78 = PARC_PERS(v119, v118, v123, v122, v110, v121, v112);
									if (v78 == 1)
										return &_vm->_globals.super_parcours[0];
									if (v78 == 2)
										goto LABEL_200;
									if (MIRACLE(v119, v118, v110, v121, v112) == 1)
										goto LABEL_201;
								}
							}
							++v72;
							v110 = v116;
						}
						v79 = PARC_PERS(v119, v118, v123, v122, v116, v121, v112);
						if (v79 == 1)
							return &_vm->_globals.super_parcours[0];
						if (v79 == 2) {
LABEL_200:
							v115 = NV_LIGNEDEP;
							v114 = NV_LIGNEOFS;
							v112 = NV_POSI;
							goto LABEL_234;
						}
						if (MIRACLE(v119, v118, v116, v121, v112) == 1)
							goto LABEL_201;
						++v116;
					} while (v116 < v121);
				}
				v114 = 0;
				v115 = v121;
			}
			if (v115 > v121) {
				v80 = v114;
				if (v114 > 0) {
					v98 = 16 * v115;
					do {
						v81 = _vm->_globals.Ligne[v98].fieldC;
						v119 = READ_LE_UINT16(v81 + 4 * v80);
						v118 = READ_LE_UINT16(v81 + 4 * v80 + 2);
						v82 = v112;
						_vm->_globals.super_parcours[v82] = v119;
						_vm->_globals.super_parcours[v82 + 1] = v118;
						_vm->_globals.super_parcours[v82 + 2] = _vm->_globals.Ligne[v98].field8;
						_vm->_globals.super_parcours[v82 + 3] = 0;
						v112 += 4;
						--v80;
					} while (v80 > 0);
				}
				v117 = v115 - 1;
				if ((signed __int16)(v115 - 1) > v121) {
					do {
						v83 = _vm->_globals.Ligne[v117].field0 - 1;
						if (v83 > -1) {
							v109 = 16 * v117;
							do {
								v84 = _vm->_globals.Ligne[v109].fieldC;
								v119 = READ_LE_UINT16(v84 + 4 * v83);
								v118 = READ_LE_UINT16(v84 + 4 * v83 + 2);
								v85 = v112;
								_vm->_globals.super_parcours[v85] = v119;
								_vm->_globals.super_parcours[v85 + 1] = v118;
								_vm->_globals.super_parcours[v85 + 2] = _vm->_globals.Ligne[v109].field8;
								_vm->_globals.super_parcours[v85 + 3] = 0;
								v112 += 4;
								v86 = _vm->_globals.Ligne[v109].field0;
								if (v86 > 30) {
									v87 = v86 / 2;
									if (v87 < 0)
										v87 = -v87;
									if (v83 == v87) {
										v88 = PARC_PERS(v119, v118, v123, v122, v117, v121, v112);
										if (v88 == 1)
											return &_vm->_globals.super_parcours[0];
										if (v88 == 2)
											goto LABEL_200;
										if (MIRACLE(v119, v118, v117, v121, v112) == 1)
											goto LABEL_201;
									}
								}
								--v83;
							} while (v83 > -1);
						}
						v89 = PARC_PERS(v119, v118, v123, v122, v117, v121, v112);
						if (v89 == 1)
							return &_vm->_globals.super_parcours[0];
						if (v89 == 2)
							goto LABEL_200;
						if (MIRACLE(v119, v118, v117, v121, v112) == 1)
							goto LABEL_201;
						--v117;
					} while (v117 > v121);
				}
				v114 = _vm->_globals.Ligne[v121].field0 - 1;
				v115 = v121;
			}
			if (v115 == v121) {
				if (v114 <= v120) {
					if (v114 < v120) {
						v94 = v114;
						v100 = 16 * v121;
						do {
							v95 = _vm->_globals.Ligne[v100].fieldC;;
							v96 = READ_LE_UINT16(v95 + 4 * v94 + 2);
							v97 = v112;
							_vm->_globals.super_parcours[v97] = READ_LE_UINT16(v95 + 4 * v94);
							_vm->_globals.super_parcours[v97 + 1] = v96;
							_vm->_globals.super_parcours[v97 + 2] = _vm->_globals.Ligne[v100].field6;
							_vm->_globals.super_parcours[v97 + 3] = 0;
							v112 += 4;
							++v94;
						} while (v120 > v94);
					}
				} else {
					v90 = v114;
					v99 = 16 * v121;
					do {
						v91 = _vm->_globals.Ligne[v99].fieldC;
						v92 = READ_LE_UINT16(v91 + 4 * v90 + 2);
						v93 = v112;
						_vm->_globals.super_parcours[v93] = READ_LE_UINT16(v91 + 4 * v90);
						_vm->_globals.super_parcours[v93 + 1] = v92;
						_vm->_globals.super_parcours[v93 + 2] = _vm->_globals.Ligne[v99].field8;
						_vm->_globals.super_parcours[v93 + 3] = 0;
						v112 += 4;
						--v90;
					} while (v120 < v90);
				}
			}
			v57 = v112;
			if (PARC_PERS(
			            _vm->_globals.Param[v112 + 4192 / 2],
			            _vm->_globals.Param[v112 + 4194 / 2],
			            v123,
			            v122,
			            -1,
			            -1,
			            v112) != 1) {
LABEL_282:
				_vm->_globals.super_parcours[v57] = -1;
				_vm->_globals.super_parcours[v57 + 1] = -1;
				_vm->_globals.super_parcours[v57 + 2] = -1;
				_vm->_globals.super_parcours[v57 + 3] = -1;
			}
			return &_vm->_globals.super_parcours[0];
		}
	}
	return &_vm->_globals.super_parcours[0];
	*/
}

void ObjectsManager::VERIFTAILLE() {
	warning("TODO: VERIFTAILLE");
}

void ObjectsManager::PACOURS_PROPRE(byte *a1) {
	warning("TODO: PACOURS_PROPRE");
}

byte *ObjectsManager::PARC_VOITURE(int a1, int a2, int a3, int a4) {
	warning("TODO: PARC_VOITURE");
	return NULL;
}

void ObjectsManager::VERBEPLUS() {
	warning("TODO: VERBEPLUS");
}

void ObjectsManager::BTDROITE() {
	warning("TODO: BTDROITE");
}

int ObjectsManager::MZONE() {
	warning("TODO: MZONE");
	return 0;
}

void ObjectsManager::CLEAR_ZONE() {
	warning("TODO: CLEAR_ZONE");
}

void ObjectsManager::RESET_OBSTACLE() {
	warning("TODO: CLEAR_ZONE");
}

int ObjectsManager::ZONE_OBJET(int a1, int a2) {
	warning("TODO: ZONE_OBJET");
	return 0;
}

void ObjectsManager::PARAMCADRE(int a1) {
	warning("TODO: PARAMCADRE");
}

void ObjectsManager::OBJETPLUS(int a1) {
	warning("TODO: OBJETPLUS");
}

void ObjectsManager::VALID_OBJET(int a1) {
	if (_vm->_eventsManager.btsouris == 8)
		CHANGE_OBJET(a1);
}

void ObjectsManager::OPTI_OBJET() {
	warning("TODO: OPTI_OBJET");
}

void ObjectsManager::SPECIAL_JEU(int a1) {
	warning("TODO: SPECIAL_JEU");
}

int ObjectsManager::Traduction(byte *a1) {
	signed int v1; 
	__int16 v2; 
	signed __int16 v3; 
	signed __int16 v4; 
	__int16 v5; 
	__int16 v6; 
	__int16 v7; 
	__int16 v8; 
	__int16 v9; 
	__int16 v10; 
	__int16 v11; 
	__int16 v12; 
	__int16 v13; 
	__int16 v14; 
	signed __int16 v15; 
	__int16 v16; 
	__int16 v17; 
	signed __int16 v18; 
	signed __int16 v19; 
	signed __int16 v20; 
	signed __int16 v21; 
	__int16 v22; 
	__int16 v23; 
	signed __int16 v24;
	signed __int16 v25;
	signed __int16 v26; 
	__int16 v27; 
	__int16 v28;
	__int16 v29;
	__int16 v30; 
	__int16 v31;
	__int16 v32; 
	signed __int16 v33; 
	signed __int16 v34; 
	signed __int16 v35; 
	signed __int16 v36;
	__int16 v37; 
	__int16 v38; 
	__int16 v39; 
	__int16 v40; 
	signed __int16 v41; 
	signed __int16 v42; 
	__int16 v43; 
	__int16 v44; 
	signed __int16 v45; 
	signed __int16 v46; 
	char v47; 
	char v48; 
	char v49;
	signed __int16 v50; 
	Sauvegarde1 *v51; 
	signed __int16 v52; 
	Sauvegarde1 *v53; 
	int v54; 
	__int16 v55; 
	__int16 v56; 
	__int16 v57; 
	__int16 v58; 
	__int16 v59; 
	__int16 v60; 
	signed __int16 v61;
	__int16 v62; 
	int v63; 
	__int16 v65;
	__int16 v66; 
	__int16 v67; 
	__int16 v68; 
	__int16 v69; 
	__int16 v70;
	__int16 v71; 
	__int16 v72; 
	signed __int16 v73;
	signed __int16 v74;
	__int16 v75; 
	__int16 v76; 
	char v77[12]; 
	Common::String s; 
	char v79; 
	char v80;
	char v81;
	char v82[16]; 

	v1 = 0;
	v70 = 0;
	if (*a1 != 70 || *(a1 + 1) != 67)
		return 0;
	// TODO: Change to chars
	if (*(a1 + 2) == 84 && *(a1 + 3) == 88 && *(a1 + 4) == 84) {
		v70 = *(a1 + 6);
		v2 = *(a1 + 7);
		v69 = *(a1 + 8);
		v67 = READ_LE_UINT16(a1 + 9);
		v65 = READ_LE_UINT16(a1 + 11);
		v3 = READ_LE_UINT16(a1 + 13);
		v1 = 1;
		if (!_vm->_objectsManager.TRAVAILOBJET) {
			if (_vm->_globals.SAUVEGARDE->data[svField356] == 1) {
				if (v3 == 53)
					v3 = 644;
				if (v3 == 624)
					v3 = 639;
				if (v3 == 627)
					v3 = 630;
				if (v3 == 625)
					v3 = 639;
				if (v3 == 8)
					v3 = 637;
				if (v3 == 53)
					v3 = 644;
				if (v3 == 557)
					v3 = 636;
				if (v3 == 51)
					v3 = 644;
				if (v3 == 287)
					v3 = 636;
				if (v3 == 619)
					v3 = 633;
				if (v3 == 620)
					v3 = 634;
				if (v3 == 622)
					v3 = 644;
				if (v3 == 297)
					v3 = 636;
				if (v3 == 612)
					v3 = 636;
				if ((unsigned __int16)(v3 - 613) <= 1u || v3 == 134)
					v3 = 636;
				if (v3 == 615)
					v3 = 635;
				if (v3 == 618)
					v3 = 632;
				if (v3 == 611)
					v3 = 642;
				if (v3 == 610)
					v3 = 641;
				if (v3 == 18)
					v3 = 643;
				if (v3 == 602)
					v3 = 645;
				if (v3 == 603)
					v3 = 646;
				if (v3 == 604)
					v3 = 647;
				if (v3 == 51)
					v3 = 644;
				if (v3 == 607)
					v3 = 650;
				if (v3 == 605)
					v3 = 648;
				if (v3 == 606)
					v3 = 649;
				if (v3 == 601)
					v3 = 652;
				if (v3 == 37)
					v3 = 636;
				if (v3 == 595)
					v3 = 633;
				if (v3 == 596)
					v3 = 634;
				if (v3 == 532)
					v3 = 636;
				if (v3 == 599)
					v3 = 636;
				if (v3 == 363)
					v3 = 636;
			}
			if (!_vm->_soundManager.SOUNDOFF && _vm->_soundManager.SOUND_FLAG == 1) {
				do
					_vm->_eventsManager.VBL();
				while (_vm->_soundManager.SOUND_FLAG);
			}
			if (!_vm->_soundManager.TEXTOFF) {
				_vm->_fontManager.DOS_TEXT(9, v3, _vm->_globals.FICH_TEXTE, 2 * v67, 2 * v65 + 40, 20, 25, 6, v2, 253);
				if (!_vm->_soundManager.TEXTOFF)
					_vm->_fontManager.TEXTE_ON(9);
			}
			if (!_vm->_soundManager.VOICEOFF)
				_vm->_soundManager.VOICE_MIX(v3, 4);
		}
		if (_vm->_objectsManager.TRAVAILOBJET == 1) {
			if (_vm->_globals.SAUVEGARDE->data[svField356]) {
				_vm->_fontManager.DOS_TEXT(9, 635, _vm->_globals.FICH_TEXTE, 55, 20, 20, 25, v69, 35, 253);
				if (!_vm->_soundManager.TEXTOFF)
					_vm->_fontManager.TEXTE_ON(9);
				if (!_vm->_soundManager.VOICEOFF)
					_vm->_soundManager.VOICE_MIX(635, 4);
				goto LABEL_104;
			}
			if (_vm->_globals.FR == 1 && !_vm->_soundManager.TEXTOFF)
				_vm->_fontManager.DOS_TEXT(9, v3, "OBJET1.TXT", 2 * v67, 60, 20, 25, 6, v2, 253);
			if (!_vm->_globals.FR && !_vm->_soundManager.TEXTOFF)
				_vm->_fontManager.DOS_TEXT(9, v3, "OBJETAN.TXT", 2 * v67, 60, 20, 25, 6, v2, 253);
			if (_vm->_globals.FR == 2) {
				if (_vm->_soundManager.TEXTOFF) {
LABEL_98:
					if (!_vm->_soundManager.VOICEOFF)
						_vm->_soundManager.VOICE_MIX(v3, 5);
					goto LABEL_104;
				}
				_vm->_fontManager.DOS_TEXT(9, v3, "OBJETES.TXT", 2 * v67, 60, 20, 25, 6, v2, 253);
			}
			if (!_vm->_soundManager.TEXTOFF)
				_vm->_fontManager.TEXTE_ON(9);
			goto LABEL_98;
		}
	}
LABEL_104:
	// TODO
	if (*(a1 + 2) == 66 && *(a1 + 3) == 79 && *(a1 + 4) == 66) {
		if (DESACTIVE != 1) {
			v72 = *(a1 + 5);
			v70 = *(a1 + 6);
			v4 = *(a1 + 7);
			v68 = READ_LE_UINT16(a1 + 8);
			v66 = READ_LE_UINT16(a1 + 10);
			if (v72 == 52) {
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, v68, READ_LE_UINT16(a1 + 10), v70);
			} else if (v72 == 51) {
				BOB_VIVANT(v70);
			} else {
				if (v72 == 50)
					goto LABEL_1141;
				VBOB(_vm->_globals.SPRITE_ECRAN, v72, v68, v66, v70);
				if (v4)
					v4 /= _vm->_globals.vitesse;
				if (v4 > 1) {
					do {
						--v4;
						_vm->_eventsManager.VBL();
					} while (v4);
				}
				if (v72 == 50)
LABEL_1141:
					AFFICHE_SPEED1(_vm->_globals.SPRITE_ECRAN, v68, v66, v70);
			}
		}
		v1 = 1;
	}
	if (*(a1 + 2) == 83) {
		if (*(a1 + 3) == 84 && *(a1 + 4) == 80) {
			if (DESACTIVE != 1) {
				DEUXPERSO = 0;
				v5 = *(a1 + 5);
				v6 = *(a1 + 8);
				_vm->_objectsManager.PERX = READ_LE_UINT16(a1 + 6);
				_vm->_objectsManager.PERY = v6;
				_vm->_objectsManager.PERI = v5;
				if (_vm->_objectsManager.CH_TETE == 1) {
					if (_vm->_globals.SAUVEGARDE->data[svField354] == 1
					        && _vm->_globals.SAUVEGARDE->data[svField180]
					        && _vm->_globals.SAUVEGARDE->data[svField181]
					        && _vm->_globals.SAUVEGARDE->data[svField182]
					        && _vm->_globals.SAUVEGARDE->data[svField183]) {
						_vm->_objectsManager.PERX = _vm->_globals.SAUVEGARDE->data[svField180];
						_vm->_objectsManager.PERY = _vm->_globals.SAUVEGARDE->data[svField181];
						_vm->_objectsManager.PERI = _vm->_globals.SAUVEGARDE->data[svField182];
					}
					if (_vm->_globals.SAUVEGARDE->data[svField356] == 1
					        && _vm->_globals.SAUVEGARDE->data[svField190]
					        && _vm->_globals.SAUVEGARDE->data[svField191]
					        && _vm->_globals.SAUVEGARDE->data[svField192]
					        && _vm->_globals.SAUVEGARDE->data[svField193]) {
						_vm->_objectsManager.PERX = _vm->_globals.SAUVEGARDE->data[svField190];
						_vm->_objectsManager.PERY = _vm->_globals.SAUVEGARDE->data[svField191];
						_vm->_objectsManager.PERI = _vm->_globals.SAUVEGARDE->data[svField192];
					}
					if (_vm->_globals.SAUVEGARDE->data[svField357] == 1
					        && _vm->_globals.SAUVEGARDE->data[svField185]
					        && _vm->_globals.SAUVEGARDE->data[svField186]
					        && _vm->_globals.SAUVEGARDE->data[svField187]
					        && _vm->_globals.SAUVEGARDE->data[svField188]) {
						_vm->_objectsManager.PERX = _vm->_globals.SAUVEGARDE->data[svField185];
						_vm->_objectsManager.PERY = _vm->_globals.SAUVEGARDE->data[svField186];
						_vm->_objectsManager.PERI = _vm->_globals.SAUVEGARDE->data[svField187];
					}
				}
				if (_vm->_globals.SAUVEGARDE->data[svField356] == 1 && _vm->_globals.SAUVEGARDE->data[svField188] == _vm->_globals.ECRAN) {
					SPRITE(
					    _vm->_globals.TETE,
					    _vm->_globals.SAUVEGARDE->data[svField185],
					    _vm->_globals.SAUVEGARDE->data[svField186],
					    1,
					    2,
					    _vm->_globals.SAUVEGARDE->data[svField189],
					    0,
					    34,
					    190);
					SPRITE_ON(1);
					DEUXPERSO = 1;
				}
				if (_vm->_globals.SAUVEGARDE->data[svField357] == 1
				        && _vm->_globals.SAUVEGARDE->data[svField355] == 1
				        && _vm->_globals.SAUVEGARDE->data[svField193] == _vm->_globals.ECRAN) {
					SPRITE(
					    _vm->_globals.TETE,
					    _vm->_globals.SAUVEGARDE->data[svField190],
					    _vm->_globals.SAUVEGARDE->data[svField191],
					    1,
					    3,
					    _vm->_globals.SAUVEGARDE->data[svField194],
					    0,
					    20,
					    127);
					SPRITE_ON(1);
					DEUXPERSO = 1;
				}
			}
			v1 = 1;
			_vm->_objectsManager.CH_TETE = 0;
		}
		if (*(a1 + 2) == 83 && *(a1 + 3) == 84 && *(a1 + 4) == 69) {
			if (DESACTIVE != 1) {
				v7 = *(a1 + 5);
				v70 = *(a1 + 6);
				v8 = *(a1 + 7);
				v9 = *(a1 + 8);
				RECALL = 0;
				_vm->_globals.OLD_ECRAN = _vm->_globals.ECRAN;
				_vm->_globals.SAUVEGARDE->data[svField6] = _vm->_globals.ECRAN;
				_vm->_globals.ECRAN = v7;
				_vm->_globals.SAUVEGARDE->data[svField5] = v7;
				PTAILLE = v70;
				PEROFX = v8;
				PEROFY = v9;
			}
			v1 = 1;
		}
	}
	if (*(a1 + 2) == 66 && *(a1 + 3) == 79 && *(a1 + 4) == 70) {
		if (DESACTIVE != 1)
			VBOB_OFF(READ_LE_UINT16(a1 + 5));
		v1 = 1;
	}
	if (*(a1 + 2) == 80 && *(a1 + 3) == 69 && *(a1 + 4) == 82) {
		v73 = READ_LE_UINT16(a1 + 5);
		if (!_vm->_globals.SAUVEGARDE->data[svField122] && !_vm->_globals.SAUVEGARDE->data[svField356]) {
			v70 = 0;
			if (READ_LE_UINT16(a1 + 5) == 14)
				v73 = 1;
			if (v73 == 17)
				v73 = 7;
			if (v73 == 18)
				v73 = 8;
			if (v73 == 19)
				v73 = 4;
			if (v73 == 15)
				v73 = 11;
			if (v73 == 16)
				v73 = 12;
			if (v73 == 1) {
				if (_vm->_globals.ACTION_SENS == 1)
					ACTION_DOS(4);
				if (_vm->_globals.ACTION_SENS == 3)
					ACTION_DROITE(4);
				if (_vm->_globals.ACTION_SENS == 2)
					Q_DROITE(4);
				if (_vm->_globals.ACTION_SENS == 5)
					ACTION_FACE(4);
				if (_vm->_globals.ACTION_SENS == 8)
					Q_GAUCHE(4);
				if (_vm->_globals.ACTION_SENS == 7)
					ACTION_GAUCHE(4);
			}
			if (v73 == 2) {
				if (_vm->_globals.ACTION_SENS == 1)
					ACTION_DOS(7);
				if (_vm->_globals.ACTION_SENS == 3)
					ACTION_DROITE(7);
				if (_vm->_globals.ACTION_SENS == 2)
					Q_DROITE(7);
				if (_vm->_globals.ACTION_SENS == 5)
					ACTION_FACE(7);
				if (_vm->_globals.ACTION_SENS == 8)
					Q_GAUCHE(7);
				if (_vm->_globals.ACTION_SENS == 7)
					ACTION_GAUCHE(7);
				if (_vm->_globals.ACTION_SENS == 1)
					ACTION_DOS(8);
				if (_vm->_globals.ACTION_SENS == 3)
					ACTION_DROITE(8);
				if (_vm->_globals.ACTION_SENS == 2)
					Q_DROITE(8);
				if (_vm->_globals.ACTION_SENS == 5)
					ACTION_FACE(8);
				if (_vm->_globals.ACTION_SENS == 8)
					Q_GAUCHE(8);
				if (_vm->_globals.ACTION_SENS == 7)
					ACTION_GAUCHE(8);
			}
			if (v73 == 4) {
				if (_vm->_globals.ACTION_SENS == 1)
					ACTION_DOS(1);
				if (_vm->_globals.ACTION_SENS == 3)
					ACTION_DROITE(1);
				if (_vm->_globals.ACTION_SENS == 2)
					Q_DROITE(1);
				if (_vm->_globals.ACTION_SENS == 5)
					ACTION_FACE(1);
				if (_vm->_globals.ACTION_SENS == 8)
					Q_GAUCHE(1);
				if (_vm->_globals.ACTION_SENS == 7)
					ACTION_GAUCHE(1);
			}
			if (v73 == 5) {
				if (_vm->_globals.ACTION_SENS == 1)
					ACTION_DOS(5);
				if (_vm->_globals.ACTION_SENS == 3)
					ACTION_DROITE(5);
				if (_vm->_globals.ACTION_SENS == 2)
					Q_DROITE(5);
				if (_vm->_globals.ACTION_SENS == 5)
					ACTION_FACE(5);
				if (_vm->_globals.ACTION_SENS == 8)
					Q_GAUCHE(5);
				if (_vm->_globals.ACTION_SENS == 7)
					ACTION_GAUCHE(5);
				if (_vm->_globals.ACTION_SENS == 1)
					ACTION_DOS(6);
				if (_vm->_globals.ACTION_SENS == 3)
					ACTION_DROITE(6);
				if (_vm->_globals.ACTION_SENS == 2)
					Q_DROITE(6);
				if (_vm->_globals.ACTION_SENS == 5)
					ACTION_FACE(6);
				if (_vm->_globals.ACTION_SENS == 8)
					Q_GAUCHE(6);
				if (_vm->_globals.ACTION_SENS == 7)
					ACTION_GAUCHE(6);
			}
			if (v73 == 7) {
				if (_vm->_globals.ACTION_SENS == 1)
					ACTION_DOS(2);
				if (_vm->_globals.ACTION_SENS == 3)
					ACTION_DROITE(2);
				if (_vm->_globals.ACTION_SENS == 2)
					Q_DROITE(2);
				if (_vm->_globals.ACTION_SENS == 5)
					ACTION_FACE(2);
				if (_vm->_globals.ACTION_SENS == 8)
					Q_GAUCHE(2);
				if (_vm->_globals.ACTION_SENS == 7)
					ACTION_GAUCHE(2);
			}
			if (v73 == 8) {
				if (_vm->_globals.ACTION_SENS == 1)
					ACTION_DOS(3);
				if (_vm->_globals.ACTION_SENS == 3)
					ACTION_DROITE(3);
				if (_vm->_globals.ACTION_SENS == 2)
					Q_DROITE(3);
				if (_vm->_globals.ACTION_SENS == 5)
					ACTION_FACE(3);
				if (_vm->_globals.ACTION_SENS == 8)
					Q_GAUCHE(3);
				if (_vm->_globals.ACTION_SENS == 7)
					ACTION_GAUCHE(3);
			}
			if (v73 == 9) {
				if (_vm->_globals.ACTION_SENS == 1)
					ACTION_DOS(5);
				if (_vm->_globals.ACTION_SENS == 3)
					ACTION_DROITE(5);
				if (_vm->_globals.ACTION_SENS == 2)
					Q_DROITE(5);
				if (_vm->_globals.ACTION_SENS == 5)
					ACTION_FACE(5);
				if (_vm->_globals.ACTION_SENS == 8)
					Q_GAUCHE(5);
				if (_vm->_globals.ACTION_SENS == 7)
					ACTION_GAUCHE(5);
			}
			if (v73 == 10) {
				if (_vm->_globals.ACTION_SENS == 1)
					ACTION_DOS(6);
				if (_vm->_globals.ACTION_SENS == 3)
					ACTION_DROITE(6);
				if (_vm->_globals.ACTION_SENS == 2)
					Q_DROITE(6);
				if (_vm->_globals.ACTION_SENS == 5)
					ACTION_FACE(6);
				if (_vm->_globals.ACTION_SENS == 8)
					Q_GAUCHE(6);
				if (_vm->_globals.ACTION_SENS == 7)
					ACTION_GAUCHE(6);
			}
			if (v73 == 11) {
				if (_vm->_globals.ACTION_SENS == 1)
					ACTION_DOS(7);
				if (_vm->_globals.ACTION_SENS == 3)
					ACTION_DROITE(7);
				if (_vm->_globals.ACTION_SENS == 2)
					Q_DROITE(7);
				if (_vm->_globals.ACTION_SENS == 5)
					ACTION_FACE(7);
				if (_vm->_globals.ACTION_SENS == 8)
					Q_GAUCHE(7);
				if (_vm->_globals.ACTION_SENS == 7)
					ACTION_GAUCHE(7);
			}
			if (v73 == 12) {
				if (_vm->_globals.ACTION_SENS == 1)
					ACTION_DOS(8);
				if (_vm->_globals.ACTION_SENS == 3)
					ACTION_DROITE(8);
				if (_vm->_globals.ACTION_SENS == 2)
					Q_DROITE(8);
				if (_vm->_globals.ACTION_SENS == 5)
					ACTION_FACE(8);
				if (_vm->_globals.ACTION_SENS == 8)
					Q_GAUCHE(8);
				if (_vm->_globals.ACTION_SENS == 7)
					ACTION_GAUCHE(8);
			}
		}
		v1 = 1;
	}
	if (*(a1 + 2) == 77 && *(a1 + 3) == 85 && *(a1 + 4) == 83)
		v1 = 1;
	if (*(a1 + 2) == 87 && *(a1 + 3) == 65 && *(a1 + 4) == 73) {
		v74 = READ_LE_UINT16(a1 + 5) / _vm->_globals.vitesse;
		if (!v74)
			v74 = 1;
		v10 = 0;
		if (v74 + 1 > 0) {
			do {
				_vm->_eventsManager.VBL();
				++v10;
			} while (v10 < v74 + 1);
		}
		v1 = 1;
	}
	if (*(a1 + 2) == 79) {
		if (*(a1 + 3) == 66 && *(a1 + 4) == 80) {
			v1 = 1;
			AJOUTE_OBJET(READ_LE_UINT16(a1 + 5));
		}
		if (*(a1 + 2) == 79 && *(a1 + 3) == 66 && *(a1 + 4) == 77) {
			v1 = 1;
			DELETE_OBJET(READ_LE_UINT16(a1 + 5));
		}
	}
	if (*(a1 + 2) == 71 && *(a1 + 3) == 79 && *(a1 + 4) == 84)
		v1 = 2;
	if (*(a1 + 2) == 90) {
		if (*(a1 + 3) == 79 && *(a1 + 4) == 78) {
			ZONE_ON(READ_LE_UINT16(a1 + 5));
			v1 = 1;
		}
		if (*(a1 + 2) == 90 && *(a1 + 3) == 79 && *(a1 + 4) == 70) {
			ZONE_OFF(READ_LE_UINT16(a1 + 5));
			v1 = 1;
		}
	}
	if (*(a1 + 2) == 69 && *(a1 + 3) == 88 && *(a1 + 4) == 73)
		v1 = 5;
	if (*(a1 + 2) == 83 && *(a1 + 3) == 79 && *(a1 + 4) == 82) {
		_vm->_globals.SORTIE = READ_LE_UINT16(a1 + 5);
		v1 = 5;
	}
	if (*(a1 + 2) == 66 && *(a1 + 3) == 67 && *(a1 + 4) == 65) {
		_vm->_globals.CACHE_OFF(READ_LE_UINT16(a1 + 5));
		v1 = 1;
	}
	if (*(a1 + 2) == 65 && *(a1 + 3) == 78 && *(a1 + 4) == 73) {
		v75 = READ_LE_UINT16(a1 + 5);
		if (v75 <= 100)
			_vm->_animationManager.BOBANIM_ON(v75);
		else
			_vm->_animationManager.BOBANIM_OFF(v75 - 100);
		v1 = 1;
	}
	if (*(a1 + 2) == 83 && *(a1 + 3) == 80 && *(a1 + 4) == 69) {
		v76 = READ_LE_UINT16(a1 + 5);
		if (v76 == 7)
			_vm->_talkManager.PARLER_PERSO("rueh1.pe2");
		if (v76 == 8)
			_vm->_talkManager.PARLER_PERSO("ruef1.pe2");
		if (v76 == 6) {
			SPRITE_OFF(0);
			OPTI_ONE(20, 0, 14, 4);
		}
		if (v76 == 12) {
			_vm->_fontManager.TEXTE_OFF(9);
			_vm->_eventsManager.VBL();
			_vm->_eventsManager.VBL();
			_vm->_talkManager.PARLER_PERSO("bqetueur.pe2");
		}
		if (v76 == 600) {
			if (!_vm->_globals.internet) {
				_vm->_graphicsManager.FADE_LINUX = 2;
				_vm->_graphicsManager.FADESPD = 1;
				if (_vm->_globals.SVGA == 2)
					_vm->_animationManager.PLAY_ANM("BOMBE1A.ANM", 100, 18, 100);
				if (_vm->_globals.SVGA == 1)
					_vm->_animationManager.PLAY_ANM("BOMBE1.ANM", 100, 18, 100);
			}
			_vm->_graphicsManager.LOAD_IMAGE("BOMBEB");
			_vm->_graphicsManager.SETCOLOR3(252, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR3(253, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR3(251, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR3(254, 0, 0, 0);
			_vm->_graphicsManager.OPTI_INI("BOMBE", 2);
			_vm->_graphicsManager.FADE_INS();
		}
		if (v76 == 601) {
			_vm->_globals.NO_VISU = 1;
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 513, 163, 7);
			_vm->_globals.NO_VISU = 0;
			OPTI_ONE(2, 0, 16, 4);
		}
		if (v76 == 602) {
			_vm->_globals.NO_VISU = 1;
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 513, 163, 7);
			_vm->_globals.NO_VISU = 0;
			OPTI_ONE(4, 0, 16, 4);
		}
		if (v76 == 603) {
			_vm->_globals.NO_VISU = 1;
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 513, 163, 7);
			_vm->_globals.NO_VISU = 0;
			OPTI_ONE(3, 0, 16, 4);
			_vm->_soundManager.SPECIAL_SOUND = 199;
			_vm->_graphicsManager.FADE_LINUX = 2;
			if (_vm->_globals.SVGA == 1)
				_vm->_animationManager.PLAY_ANM("BOMBE2.ANM", 50, 14, 500);
			if (_vm->_globals.SVGA == 2)
				_vm->_animationManager.PLAY_ANM("BOMBE2A.ANM", 50, 14, 500);
			_vm->_soundManager.SPECIAL_SOUND = 0;
			memset(_vm->_graphicsManager.VESA_BUFFER, 0, 0x96000u);
			_vm->_graphicsManager.NOFADE = 1;
			_vm->_globals.SORTIE = 151;
		}
		if (v76 == 604) {
			_vm->_globals.NO_VISU = 1;
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 513, 163, 7);
			_vm->_globals.NO_VISU = 0;
			OPTI_ONE(1, 0, 16, 4);
			_vm->_soundManager.SPECIAL_SOUND = 199;
			if (_vm->_globals.SVGA == 1)
				_vm->_animationManager.PLAY_ANM("BOMBE2.ANM", 50, 14, 500);
			if (_vm->_globals.SVGA == 2)
				_vm->_animationManager.PLAY_ANM("BOMBE2A.ANM", 50, 14, 500);
			_vm->_soundManager.SPECIAL_SOUND = 0;
			_vm->_graphicsManager.NOFADE = 1;
			memset(_vm->_graphicsManager.VESA_BUFFER, 0, 0x96000u);
			_vm->_globals.SORTIE = 151;
		}
		if (v76 == 605) {
			_vm->_globals.NO_VISU = 1;
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 513, 163, 7);
			_vm->_globals.NO_VISU = 0;
			OPTI_ONE(5, 0, 16, 4);
			_vm->_graphicsManager.FADE_OUTS();
			_vm->_soundManager.SPECIAL_SOUND = 199;
			_vm->_graphicsManager.FADE_LINUX = 2;
			if (_vm->_globals.SVGA == 1)
				_vm->_animationManager.PLAY_ANM("BOMBE2.ANM", 50, 14, 500);
			if (_vm->_globals.SVGA == 2)
				_vm->_animationManager.PLAY_ANM("BOMBE2A.ANM", 50, 14, 500);
			_vm->_soundManager.SPECIAL_SOUND = 0;
			_vm->_graphicsManager.NOFADE = 1;
			memset(_vm->_graphicsManager.VESA_BUFFER, 0, 0x96000u);
			_vm->_globals.SORTIE = 151;
		}
		if (v76 == 606) {
			_vm->_globals.NO_VISU = 1;
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 513, 163, 7);
			_vm->_globals.NO_VISU = 0;
			OPTI_ONE(6, 0, 16, 4);
			if (_vm->_globals.SVGA == 1)
				_vm->_animationManager.PLAY_ANM("BOMBE3.ANM", 50, 14, 500);
			if (_vm->_globals.SVGA == 2)
				_vm->_animationManager.PLAY_ANM("BOMBE3A.ANM", 50, 14, 500);
			memset(_vm->_graphicsManager.VESA_BUFFER, 0, 0x96000u);
			_vm->_globals.SORTIE = 6;
		}
		if (v76 == 607) {
			if (!_vm->_globals.internet) {
				memcpy(_vm->_graphicsManager.OLD_PAL, _vm->_graphicsManager.Palette, 0x301u);
				v1 = _vm->_graphicsManager.OLD_PAL[769];
				_vm->_animationManager.PLAY_ANM2("plan.ANm", 50, 10, 800);
			}
			_vm->_globals.NBBLOC = 0;
		}
		if (v76 == 608) {
			_vm->_animationManager.BOBANIM_OFF(2);
			_vm->_animationManager.BOBANIM_OFF(3);
			_vm->_animationManager.BOBANIM_OFF(4);
			_vm->_animationManager.BOBANIM_OFF(6);
			_vm->_animationManager.BOBANIM_OFF(11);
			_vm->_animationManager.BOBANIM_OFF(10);
		}
		if (v76 == 609) {
			_vm->_animationManager.BOBANIM_ON(2);
			_vm->_animationManager.BOBANIM_ON(3);
			_vm->_animationManager.BOBANIM_ON(4);
			_vm->_animationManager.BOBANIM_ON(6);
			_vm->_animationManager.BOBANIM_ON(11);
			_vm->_animationManager.BOBANIM_ON(10);
		}
		if (v76 == 611) {
			_vm->_animationManager.BOBANIM_ON(5);
			_vm->_animationManager.BOBANIM_ON(7);
			_vm->_animationManager.BOBANIM_ON(8);
			_vm->_animationManager.BOBANIM_ON(9);
			_vm->_animationManager.BOBANIM_ON(12);
			_vm->_animationManager.BOBANIM_ON(13);
		}
		if (v76 == 610) {
			_vm->_animationManager.BOBANIM_OFF(5);
			_vm->_animationManager.BOBANIM_OFF(7);
			_vm->_animationManager.BOBANIM_OFF(8);
			_vm->_animationManager.BOBANIM_OFF(9);
			_vm->_animationManager.BOBANIM_OFF(12);
			_vm->_animationManager.BOBANIM_OFF(13);
		}
		if (v76 == 10)
			_vm->_talkManager.PARLER_PERSO("bqeflic1.pe2");
		if (v76 == 11)
			_vm->_talkManager.PARLER_PERSO("bqeflic2.pe2");
		if (v76 == 16)
			_vm->_talkManager.PARLER_PERSO("ftoubib.pe2");
		if (v76 == 17)
			_vm->_talkManager.PARLER_PERSO("flic2b.pe2");
		if (v76 == 18)
			_vm->_talkManager.PARLER_PERSO("fjour.pe2");
		if (v76 == 13) {
			_vm->_eventsManager.souris_b = _vm->_eventsManager.souris_bb;
			_vm->_globals.DESACTIVE_INVENT = 1;
			_vm->_graphicsManager.FADE_OUTW();
			_vm->_globals.CACHE_OFF();
			SPRITE_OFF(0);
			_vm->_fontManager.TEXTE_OFF(5);
			_vm->_fontManager.TEXTE_OFF(9);
			_vm->_graphicsManager.FIN_VISU();
			CLEAR_ECRAN();
			_vm->_soundManager.PLAY_SOUND("SOUND17.WAV");
			_vm->_graphicsManager.FADE_LINUX = 2;
			_vm->_soundManager.PLAY_SEQ2("HELICO.SEQ", 10, 4, 10);
			_vm->_animationManager.CHARGE_ANIM("otage");
			_vm->_graphicsManager.LOAD_IMAGE("IM05");
			_vm->_graphicsManager.VISU_ALL();
			v11 = 0;
			do {
				_vm->_eventsManager.VBL();
				++v11;
			} while (v11 <= 4);
			_vm->_eventsManager.MOUSE_OFF();
			_vm->_graphicsManager.FADE_INW_LINUX(_vm->_graphicsManager.VESA_BUFFER);
			do
				_vm->_eventsManager.VBL();
			while ((unsigned __int16)BOBPOSI(3) != 100);
			_vm->_graphicsManager.FADE_OUTW_LINUX(_vm->_graphicsManager.VESA_BUFFER);
			_vm->_graphicsManager.FIN_VISU();
			if (!_vm->_globals.CENSURE) {
				_vm->_soundManager.SPECIAL_SOUND = 16;
				_vm->_graphicsManager.FADE_LINUX = 2;
				_vm->_animationManager.PLAY_ANM("EGORGE.ANM", 50, 28, 500);
				_vm->_soundManager.SPECIAL_SOUND = 0;
			}
			_vm->_animationManager.CHARGE_ANIM("ASCEN");
			_vm->_eventsManager.MOUSE_OFF();
			_vm->_graphicsManager.LOAD_IMAGE("ASCEN");
			_vm->_graphicsManager.VISU_ALL();
			v12 = 0;
			do {
				_vm->_eventsManager.VBL();
				++v12;
			} while (v12 <= 4);
			_vm->_eventsManager.MOUSE_OFF();
			_vm->_graphicsManager.FADE_INW_LINUX(_vm->_graphicsManager.VESA_BUFFER);
			SCI_OPTI_ONE(1, 0, 17, 3);
			_vm->_graphicsManager.FADE_OUTW_LINUX(_vm->_graphicsManager.VESA_BUFFER);
			_vm->_graphicsManager.FIN_VISU();
			_vm->_soundManager.SPECIAL_SOUND = 14;
			_vm->_graphicsManager.FADE_LINUX = 2;
			_vm->_soundManager.PLAY_SEQ2("ASSOM.SEQ", 10, 4, 500);
			_vm->_soundManager.SPECIAL_SOUND = 0;
			_vm->_globals.DESACTIVE_INVENT = 0;
			_vm->_globals.HELICO = 1;
		}
		if (v76 == 36) {
			if (_vm->_globals.SAUVEGARDE->data[svField270] == 2 && _vm->_globals.SAUVEGARDE->data[svField94] == 1 && _vm->_globals.SAUVEGARDE->data[svField95] == 1)
				_vm->_globals.SAUVEGARDE->data[svField270] = 3;
			if (!_vm->_globals.SAUVEGARDE->data[svField270])
				_vm->_talkManager.PARLER_PERSO2("PATRON0.pe2");
			if (_vm->_globals.SAUVEGARDE->data[svField270] == 1)
				_vm->_talkManager.PARLER_PERSO2("PATRON1.pe2");
			if (_vm->_globals.SAUVEGARDE->data[svField270] == 2)
				_vm->_talkManager.PARLER_PERSO2("PATRON2.pe2");
			if (_vm->_globals.SAUVEGARDE->data[svField270] == 3)
				_vm->_talkManager.PARLER_PERSO2("PATRON3.pe2");
			if (_vm->_globals.SAUVEGARDE->data[svField270] > 3) {
				_vm->_talkManager.PARLER_PERSO2("PATRON4.pe2");
				_vm->_globals.SAUVEGARDE->data[svField270] = 5;
			}
		}
		if (v76 == 37) {
			_vm->_graphicsManager.FADE_LINUX = 2;
			_vm->_soundManager.PLAY_SEQ2("corde.SEQ", 32, 32, 100);
			_vm->_graphicsManager.NOFADE = 1;
		}
		if (v76 == 38) {
			_vm->_soundManager.CHARGE_SAMPLE(1, "SOUND44.WAV");
			_vm->_soundManager.CHARGE_SAMPLE(2, "SOUND42.WAV");
			_vm->_soundManager.CHARGE_SAMPLE(3, "SOUND41.WAV");
			_vm->_soundManager.SPECIAL_SOUND = 17;
			_vm->_soundManager.PLAY_SEQ(v1, "grenade.SEQ", 1, 32, 100);
			_vm->_soundManager.SPECIAL_SOUND = 0;
			_vm->_graphicsManager.FADE_LINUX = 2;
			_vm->_animationManager.PLAY_ANM("CREVE17.ANM", 24, 24, 200);
			_vm->_soundManager.DEL_SAMPLE(1);
			_vm->_soundManager.DEL_SAMPLE(2);
			_vm->_soundManager.DEL_SAMPLE(3);
			_vm->_graphicsManager.NOFADE = 1;
		}
		if (v76 == 29) {
			_vm->_globals.DESACTIVE_INVENT = 1;
			OBJET_VIVANT("TELEP.pe2");
			_vm->_globals.DESACTIVE_INVENT = 0;
		}
		if (v76 == 22)
			OBJET_VIVANT("CADAVRE1.pe2");
		if (v76 == 20)
			_vm->_talkManager.PARLER_PERSO("PUNK.pe2");
		if (v76 == 23)
			_vm->_talkManager.PARLER_PERSO2("CHERCHE1.pe2");
		if (v76 == 35) {
			if (!_vm->_soundManager.SOUNDOFF) {
				do
					_vm->_eventsManager.VBL();
				while (_vm->_soundManager.SOUND_FLAG);
			}
			_vm->_talkManager.PARLER_PERSO("PTLAB.pe2");
		}
		if (v76 == 46) {
			_vm->_globals.NOT_VERIF = 1;
			_vm->_globals.chemin = PTRNUL;
			v13 = YSPR(0);
			v14 = XSPR(0);
			_vm->_globals.chemin = PARCOURS2(v14, v13, 564, 420);
			_vm->_objectsManager.NUMZONE = -1;
			do {
				GOHOME();
				_vm->_eventsManager.VBL();
			} while (_vm->_globals.chemin != PTRNUL);
			SPRITE_OFF(0);
			_vm->_globals.NOT_VERIF = 1;
			_vm->_soundManager.CHARGE_SAMPLE(1, "SOUND44.WAV");
			_vm->_soundManager.CHARGE_SAMPLE(2, "SOUND45.WAV");
			OPTI_BOBON(9, 10, -1, 0, 0, 0, 0);
			v15 = 0;
			do {
				if ((unsigned __int16)BOBPOSI(9) == 4 && !v15) {
					_vm->_soundManager.PLAY_SAMPLE2(1);
					v15 = 1;
				}
				if ((unsigned __int16)BOBPOSI(9) == 5)
					v15 = 0;
				if ((unsigned __int16)BOBPOSI(9) == 16 && !v15) {
					_vm->_soundManager.PLAY_SAMPLE2(1);
					v15 = 1;
				}
				if ((unsigned __int16)BOBPOSI(9) == 17)
					v15 = 0;
				if ((unsigned __int16)BOBPOSI(9) == 28 && !v15) {
					_vm->_soundManager.PLAY_SAMPLE2(1);
					v15 = 1;
				}
				if ((unsigned __int16)BOBPOSI(9) == 29)
					v15 = 0;
				if ((unsigned __int16)BOBPOSI(10) == 10 && !v15) {
					_vm->_soundManager.PLAY_SAMPLE2(2);
					v15 = 1;
				}
				if ((unsigned __int16)BOBPOSI(10) == 11)
					v15 = 0;
				if ((unsigned __int16)BOBPOSI(10) == 22 && !v15) {
					_vm->_soundManager.PLAY_SAMPLE2(2);
					v15 = 1;
				}
				if ((unsigned __int16)BOBPOSI(10) == 23)
					v15 = 0;
				if ((unsigned __int16)BOBPOSI(10) == 33 && !v15) {
					_vm->_soundManager.PLAY_SAMPLE2(2);
					v15 = 1;
				}
				if ((unsigned __int16)BOBPOSI(10) == 34)
					v15 = 0;
				if ((unsigned __int16)BOBPOSI(10) == 12)
					_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 513, 249, 1);
				if ((unsigned __int16)BOBPOSI(10) == 23)
					_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 513, 249, 2);
				if ((unsigned __int16)BOBPOSI(10) == 34)
					_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 513, 249, 3);
				_vm->_eventsManager.VBL();
			} while ((unsigned __int16)BOBPOSI(9) != 36);
			SPRITE_ON(0);
			_vm->_animationManager.BOBANIM_OFF(9);
			_vm->_animationManager.BOBANIM_OFF(10);
			_vm->_soundManager.DEL_SAMPLE(1);
			_vm->_soundManager.DEL_SAMPLE(2);
		}
		if (v76 == 59) {
			_vm->_globals.NOT_VERIF = 1;
			g_old_x = XSPR(0);
			g_old_sens = -1;
			_vm->_globals.Compteur = 0;
			_vm->_globals.chemin = PTRNUL;
			v16 = YSPR(0);
			v17 = XSPR(0);
			_vm->_globals.chemin = PARCOURS2(v17, v16, 445, 332);
			_vm->_globals.NOT_VERIF = 1;
			do {
				GOHOME();
				_vm->_eventsManager.VBL();
			} while (_vm->_globals.chemin != PTRNUL);
			SPRITE_OFF(0);
			_vm->_animationManager.BOBANIM_ON(7);
			SET_BOBPOSI(7, 0);
			v18 = 0;
			_vm->_soundManager.CHARGE_SAMPLE(1, "SOUND40.WAV");
			do {
				if ((unsigned __int16)BOBPOSI(7) == 10 && !v18) {
					_vm->_soundManager.PLAY_SAMPLE2(1);
					v18 = 1;
				}
				if ((unsigned __int16)BOBPOSI(7) == 11)
					v18 = 0;
				if ((unsigned __int16)BOBPOSI(7) == 18 && !v18) {
					_vm->_soundManager.PLAY_SAMPLE2(1);
					v18 = 1;
				}
				if ((unsigned __int16)BOBPOSI(7) == 19)
					v18 = 0;
				if ((unsigned __int16)BOBPOSI(7) == 19)
					_vm->_animationManager.BOBANIM_ON(3);
				_vm->_eventsManager.VBL();
			} while ((unsigned __int16)BOBPOSI(3) != 48);
			_vm->_soundManager.DEL_SAMPLE(1);
			SETANISPR(0, 62);
			SPRITE_ON(0);
			_vm->_animationManager.BOBANIM_ON(6);
			_vm->_animationManager.BOBANIM_OFF(7);
			_vm->_animationManager.BOBANIM_OFF(3);
		}
		if (v76 == 50) {
			_vm->_soundManager.PLAY_SOUND("SOUND46.WAv");
			OPTI_ONE(11, 0, 23, 0);
		}
		if (v76 == 49) {
			_vm->_globals.CACHE_OFF();
			SPRITE_OFF(0);
			OPTI_BOBON(9, 10, -1, 0, 0, 0, 0);
			v19 = 12;
			if (_vm->_globals.SAUVEGARDE->data[svField133] == 1)
				v19 = 41;
			v20 = 0;
			do {
				if ((unsigned __int16)BOBPOSI(9) == 4 && !v20) {
					_vm->_soundManager.PLAY_SOUND2("SOUND44.WAV");
					v20 = 1;
				}
				if ((unsigned __int16)BOBPOSI(9) == 5)
					v20 = 0;
				if ((unsigned __int16)BOBPOSI(9) == 18 && !v20) {
					_vm->_soundManager.PLAY_SOUND2("SOUND46.WAV");
					v20 = 1;
				}
				if ((unsigned __int16)BOBPOSI(9) == 19)
					v20 = 0;
				if ((unsigned __int16)BOBPOSI(10) == 11 && !v20) {
					_vm->_soundManager.PLAY_SOUND2("SOUND45.WAV");
					v20 = 1;
				}
				if ((unsigned __int16)BOBPOSI(10) == 12)
					v20 = 0;
				_vm->_eventsManager.VBL();
			} while ((unsigned __int16)BOBPOSI(9) != v19);
			if (v19 == 12) {
				SPRITE_ON(0);
				_vm->_animationManager.BOBANIM_OFF(9);
			}
			_vm->_globals.CACHE_ON();
		}
		if (v76 == 80) {
			SPRITE_OFF(0);
			_vm->_animationManager.BOBANIM_ON(12);
			_vm->_animationManager.BOBANIM_ON(13);
			SET_BOBPOSI(12, 0);
			SET_BOBPOSI(13, 0);
			v21 = 0;
			_vm->_soundManager.LOAD_WAV("SOUND44.WAV", 1);
			_vm->_soundManager.LOAD_WAV("SOUND71.WAV", 2);
			do {
				if ((unsigned __int16)BOBPOSI(12) == 4 && !v21) {
					_vm->_soundManager._vm->_soundManager.PLAY_WAV(1);
					v21 = 1;
				}
				if ((unsigned __int16)BOBPOSI(12) == 5)
					v21 = 0;
				if ((unsigned __int16)BOBPOSI(4) == 5 && !v21) {
					_vm->_soundManager._vm->_soundManager.PLAY_WAV(2);
					v21 = 1;
				}
				if ((unsigned __int16)BOBPOSI(4) == 6)
					v21 = 0;
				if ((unsigned __int16)BOBPOSI(13) == 8) {
					_vm->_animationManager.BOBANIM_OFF(13);
					_vm->_animationManager.BOBANIM_OFF(3);
					_vm->_animationManager.BOBANIM_ON(4);
					SET_BOBPOSI(4, 0);
					SET_BOBPOSI(13, 0);
				}
				_vm->_eventsManager.VBL();
			} while ((unsigned __int16)BOBPOSI(4) != 16);
			_vm->_animationManager.BOBANIM_OFF(12);
			_vm->_animationManager.BOBANIM_OFF(4);
			SPRITE_ON(0);
			OBSSEUL = 1;
			INILINK("IM27a");
			OBSSEUL = 0;
		}
		if (v76 == 81) {
			_vm->_globals.NOT_VERIF = 1;
			g_old_x = XSPR(0);
			g_old_sens = -1;
			_vm->_globals.Compteur = 0;
			_vm->_globals.chemin = PTRNUL;
			v22 = YSPR(0);
			v23 = XSPR(0);
			_vm->_globals.chemin = PARCOURS2(v23, v22, 119, 268);
			_vm->_globals.NOT_VERIF = 1;
			do {
				GOHOME();
				_vm->_eventsManager.VBL();
			} while (_vm->_globals.chemin != PTRNUL);
			SPRITE_OFF(0);
			_vm->_animationManager.BOBANIM_ON(11);
			_vm->_animationManager.BOBANIM_ON(8);
			SET_BOBPOSI(11, 0);
			SET_BOBPOSI(8, 0);
			_vm->_soundManager.LOAD_WAV("SOUND44.WAV", 1);
			_vm->_soundManager.LOAD_WAV("SOUND48.WAV", 2);
			_vm->_soundManager.LOAD_WAV("SOUND49.WAV", 3);
			v24 = 0;
			do {
				if ((unsigned __int16)BOBPOSI(11) == 4 && !v24) {
					_vm->_soundManager._vm->_soundManager.PLAY_WAV(1);
					v24 = 1;
				}
				if ((unsigned __int16)BOBPOSI(11) == 5)
					v24 = 0;
				if ((unsigned __int16)BOBPOSI(8) == 11 && !v24) {
					_vm->_soundManager._vm->_soundManager.PLAY_WAV(2);
					v24 = 1;
				}
				if ((unsigned __int16)BOBPOSI(8) == 12)
					v24 = 0;
				_vm->_eventsManager.VBL();
			} while ((unsigned __int16)BOBPOSI(8) != 32);
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 201, 14, 1);
			SPRITE_ON(0);
			_vm->_animationManager.BOBANIM_OFF(11);
			_vm->_animationManager.BOBANIM_OFF(8);
			_vm->_animationManager.BOBANIM_ON(5);
			_vm->_animationManager.BOBANIM_ON(6);
			SET_BOBPOSI(5, 0);
			SET_BOBPOSI(6, 0);
			_vm->_soundManager._vm->_soundManager.PLAY_WAV(3);
			do
				_vm->_eventsManager.VBL();
			while ((unsigned __int16)BOBPOSI(5) != 74);
			_vm->_animationManager.BOBANIM_OFF(5);
			_vm->_animationManager.BOBANIM_OFF(6);
			_vm->_animationManager.BOBANIM_ON(9);
			_vm->_animationManager.BOBANIM_ON(7);
		}
		if (v76 == 95) {
			_vm->_animationManager.BOBANIM_ON(9);
			_vm->_animationManager.BOBANIM_ON(10);
			_vm->_animationManager.BOBANIM_ON(12);
			SET_BOBPOSI(9, 0);
			SET_BOBPOSI(10, 0);
			SET_BOBPOSI(12, 0);
			SPRITE_OFF(0);
			do
				_vm->_eventsManager.VBL();
			while ((unsigned __int16)BOBPOSI(9) != 15);
			_vm->_animationManager.BOBANIM_OFF(9);
			SPRITE_ON(0);
			_vm->_soundManager.PLAY_SOUND("SOUND50.WAV");
			do
				_vm->_eventsManager.VBL();
			while ((unsigned __int16)BOBPOSI(12) != 117);
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 830, 122, 0);
			_vm->_animationManager.BOBANIM_OFF(12);
			_vm->_animationManager.BOBANIM_OFF(10);
			_vm->_animationManager.BOBANIM_ON(11);
		}
		if (v76 == 85) {
			_vm->_animationManager.BOBANIM_OFF(3);
			_vm->_animationManager.BOBANIM_ON(5);
			SET_BOBPOSI(5, 0);
			do
				_vm->_eventsManager.VBL();
			while ((unsigned __int16)BOBPOSI(5) != 6);
			_vm->_animationManager.BOBANIM_OFF(5);
			_vm->_animationManager.BOBANIM_ON(6);
			OBSSEUL = 1;
			INILINK("IM24a");
			OBSSEUL = 0;
		}
		if (v76 == 88) {
			if (_vm->_globals.SAUVEGARDE->data[svField183] == 1) {
				SET_BOBPOSI(1, 0);
				SET_BOBPOSI(2, 0);
				_vm->_animationManager.BOBANIM_ON(1);
				_vm->_animationManager.BOBANIM_ON(2);
				_vm->_soundManager.CHARGE_SAMPLE(1, "SOUND40.WAV");
				v25 = 0;
				do {
					if ((unsigned __int16)BOBPOSI(1) == 1 && !v25) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v25 = 1;
					}
					if ((unsigned __int16)BOBPOSI(1) == 2)
						v25 = 0;
					if ((unsigned __int16)BOBPOSI(1) == 3 && !v25) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v25 = 1;
					}
					if ((unsigned __int16)BOBPOSI(1) == 4)
						v25 = 0;
					if ((unsigned __int16)BOBPOSI(1) == 5 && !v25) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v25 = 1;
					}
					if ((unsigned __int16)BOBPOSI(1) == 6)
						v25 = 0;
					if ((unsigned __int16)BOBPOSI(1) == 7 && !v25) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v25 = 1;
					}
					if ((unsigned __int16)BOBPOSI(1) == 8)
						v25 = 0;
					_vm->_eventsManager.VBL();
				} while ((unsigned __int16)BOBPOSI(1) != 9);
				_vm->_animationManager.BOBANIM_OFF(1);
				_vm->_animationManager.BOBANIM_OFF(2);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 283, 160, 6);
				_vm->_soundManager.DEL_SAMPLE(1);
			}
			if (_vm->_globals.SAUVEGARDE->data[svField183] == 2) {
				SET_BOBPOSI(1, 0);
				SET_BOBPOSI(3, 0);
				_vm->_animationManager.BOBANIM_ON(1);
				_vm->_animationManager.BOBANIM_ON(3);
				_vm->_soundManager.CHARGE_SAMPLE(1, "SOUND40.WAV");
				v26 = 0;
				do {
					if ((unsigned __int16)BOBPOSI(1) == 1 && !v26) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v26 = 1;
					}
					if ((unsigned __int16)BOBPOSI(1) == 2)
						v26 = 0;
					if ((unsigned __int16)BOBPOSI(1) == 3 && !v26) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v26 = 1;
					}
					if ((unsigned __int16)BOBPOSI(1) == 4)
						v26 = 0;
					if ((unsigned __int16)BOBPOSI(1) == 5 && !v26) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v26 = 1;
					}
					if ((unsigned __int16)BOBPOSI(1) == 6)
						v26 = 0;
					if ((unsigned __int16)BOBPOSI(1) == 7 && !v26) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v26 = 1;
					}
					if ((unsigned __int16)BOBPOSI(1) == 8)
						v26 = 0;
					_vm->_eventsManager.VBL();
				} while ((unsigned __int16)BOBPOSI(1) != 9);
				_vm->_animationManager.BOBANIM_OFF(1);
				_vm->_animationManager.BOBANIM_OFF(3);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 283, 161, 8);
				_vm->_soundManager.DEL_SAMPLE(1);
			}
		}
		if (v76 == 90) {
			_vm->_soundManager.PLAY_SOUND("SOUND52.WAV");
			if (!_vm->_globals.SAUVEGARDE->data[svField186]) {
				_vm->_soundManager.PLAY_SEQ(v1, "CIB5A.SEQ", 1, 12, 1);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 155, 29, 0);
			}
			if (_vm->_globals.SAUVEGARDE->data[svField186] == 1) {
				_vm->_soundManager.PLAY_SEQ(v1, "CIB5C.SEQ", 1, 12, 1);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 155, 29, 0);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 329, 87, 2);
			}
		}
		if (v76 == 91) {
			_vm->_soundManager.PLAY_SOUND("SOUND52.WAV");
			if (!_vm->_globals.SAUVEGARDE->data[svField186]) {
				_vm->_soundManager.PLAY_SEQ(v1, "CIB5B.SEQ", 1, 12, 1);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 155, 29, 5);
			}
			if (_vm->_globals.SAUVEGARDE->data[svField186] == 1) {
				_vm->_soundManager.PLAY_SEQ(v1, "CIB5D.SEQ", 1, 12, 1);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 155, 29, 5);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 283, 160, 6);
			}
		}
		if (v76 == 92) {
			_vm->_soundManager.PLAY_SOUND("SOUND52.WAV");
			if (!_vm->_globals.SAUVEGARDE->data[svField184]) {
				_vm->_soundManager.PLAY_SEQ(v1, "CIB6A.SEQ", 1, 12, 1);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 155, 29, 0);
			}
			if (_vm->_globals.SAUVEGARDE->data[svField184] == 1) {
				_vm->_soundManager.PLAY_SEQ(v1, "CIB6C.SEQ", 1, 12, 1);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 155, 29, 0);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 293, 139, 3);
			}
		}
		if (v76 == 93) {
			_vm->_soundManager.PLAY_SOUND("SOUND52.WAV");
			if (!_vm->_globals.SAUVEGARDE->data[svField184]) {
				_vm->_soundManager.PLAY_SEQ(v1, "CIB6B.SEQ", 1, 12, 1);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 155, 29, 5);
			}
			if (_vm->_globals.SAUVEGARDE->data[svField184] == 1) {
				_vm->_soundManager.PLAY_SEQ(v1, "CIB6D.SEQ", 1, 12, 1);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 155, 29, 5);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 283, 161, 8);
			}
		}
		if (v76 == 62)
			OBJET_VIVANT("SBCADA.pe2");
		if (v76 == 65)
			OBJET_VIVANT("ScCADA.pe2");
		if (v76 == 105) {
			_vm->_globals.NOT_VERIF = 1;
			g_old_x = XSPR(0);
			g_old_sens = -1;
			_vm->_globals.Compteur = 0;
			_vm->_globals.chemin = PTRNUL;
			if (_vm->_globals.SAUVEGARDE->data[svField253] == 1) {
				v27 = YSPR(0);
				v28 = XSPR(0);
				_vm->_globals.chemin = PARCOURS2(v28, v27, 201, 294);
			}
			if (_vm->_globals.SAUVEGARDE->data[svField253] == 2) {
				v29 = YSPR(0);
				v30 = XSPR(0);
				_vm->_globals.chemin = PARCOURS2(v30, v29, 158, 338);
			}
			if (_vm->_globals.SAUVEGARDE->data[svField253] > 2) {
				v31 = YSPR(0);
				v32 = XSPR(0);
				_vm->_globals.chemin = PARCOURS2(v32, v31, 211, 393);
			}
			_vm->_globals.NOT_VERIF = 1;
			do {
				GOHOME();
				_vm->_eventsManager.VBL();
			} while (_vm->_globals.chemin != PTRNUL);
			SPRITE_OFF(0);
			SETANISPR(0, 60);
			_vm->_soundManager.CHARGE_SAMPLE(1, "SOUND63.WAV");
			if (_vm->_globals.SAUVEGARDE->data[svField253] > 2) {
				_vm->_animationManager.BOBANIM_ON(4);
				v33 = 0;
				do {
					if ((unsigned __int16)BOBPOSI(4) == 9 && !v33) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v33 = 1;
					}
					if ((unsigned __int16)BOBPOSI(4) == 10)
						v33 = 0;
					if ((unsigned __int16)BOBPOSI(4) == 32 && !v33) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v33 = 1;
					}
					if ((unsigned __int16)BOBPOSI(4) == 33)
						v33 = 0;
					if ((unsigned __int16)BOBPOSI(4) == 55 && !v33) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v33 = 1;
					}
					if ((unsigned __int16)BOBPOSI(4) == 56)
						v33 = 0;
					_vm->_eventsManager.VBL();
				} while ((unsigned __int16)BOBPOSI(4) != 72);
				_vm->_animationManager.BOBANIM_OFF(4);
			}
			if (_vm->_globals.SAUVEGARDE->data[svField253] == 1) {
				_vm->_animationManager.BOBANIM_ON(6);
				v34 = 0;
				do {
					if ((unsigned __int16)BOBPOSI(6) == 9 && !v34) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v34 = 1;
					}
					if ((unsigned __int16)BOBPOSI(6) == 10)
						v34 = 0;
					if ((unsigned __int16)BOBPOSI(6) == 32 && !v34) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v34 = 1;
					}
					if ((unsigned __int16)BOBPOSI(6) == 33)
						v34 = 0;
					if ((unsigned __int16)BOBPOSI(6) == 55 && !v34) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v34 = 1;
					}
					if ((unsigned __int16)BOBPOSI(6) == 56)
						v34 = 0;
					_vm->_eventsManager.VBL();
				} while ((unsigned __int16)BOBPOSI(6) != 72);
				_vm->_animationManager.BOBANIM_OFF(6);
			}
			if (_vm->_globals.SAUVEGARDE->data[svField253] == 2) {
				_vm->_animationManager.BOBANIM_ON(5);
				v35 = 0;
				do {
					if ((unsigned __int16)BOBPOSI(5) == 9 && !v35) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v35 = 1;
					}
					if ((unsigned __int16)BOBPOSI(5) == 10)
						v35 = 0;
					if ((unsigned __int16)BOBPOSI(5) == 32 && !v35) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v35 = 1;
					}
					if ((unsigned __int16)BOBPOSI(5) == 33)
						v35 = 0;
					if ((unsigned __int16)BOBPOSI(5) == 55 && !v35) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v35 = 1;
					}
					if ((unsigned __int16)BOBPOSI(5) == 56)
						v35 = 0;
					_vm->_eventsManager.VBL();
				} while ((unsigned __int16)BOBPOSI(5) != 72);
				_vm->_animationManager.BOBANIM_OFF(5);
			}
			SPRITE_ON(0);
			ACTION_DOS(1);
			_vm->_soundManager.DEL_SAMPLE(1);
		}
		if (v76 == 106) {
			SPRITE_OFF(0);
			_vm->_animationManager.BOBANIM_ON(4);
			SET_BOBPOSI(4, 0);
			_vm->_soundManager.LOAD_WAV("SOUND61.WAV", 1);
			_vm->_soundManager.LOAD_WAV("SOUND62.WAV", 2);
			_vm->_soundManager.LOAD_WAV("SOUND61.WAV", 3);
			do
				_vm->_eventsManager.VBL();
			while ((unsigned __int16)BOBPOSI(4) != 10);
			_vm->_soundManager._vm->_soundManager.PLAY_WAV(1);
			do
				_vm->_eventsManager.VBL();
			while ((unsigned __int16)BOBPOSI(4) != 18);
			_vm->_soundManager._vm->_soundManager.PLAY_WAV(2);
			do
				_vm->_eventsManager.VBL();
			while ((unsigned __int16)BOBPOSI(4) != 62);
			_vm->_soundManager._vm->_soundManager.PLAY_WAV(3);
			do
				_vm->_eventsManager.VBL();
			while ((unsigned __int16)BOBPOSI(4) != 77);
			_vm->_animationManager.BOBANIM_OFF(4);
			SPRITE_ON(0);
		}
		if (v76 == 107) {
			SPRITE_OFF(0);
			_vm->_animationManager.BOBANIM_ON(5);
			SET_BOBPOSI(5, 0);
			_vm->_soundManager.LOAD_WAV("SOUND61.WAV", 1);
			_vm->_soundManager.LOAD_WAV("SOUND62.WAV", 2);
			_vm->_soundManager.LOAD_WAV("SOUND61.WAV", 3);
			do
				_vm->_eventsManager.VBL();
			while ((unsigned __int16)BOBPOSI(5) != 10);
			_vm->_soundManager._vm->_soundManager.PLAY_WAV(1);
			do
				_vm->_eventsManager.VBL();
			while ((unsigned __int16)BOBPOSI(5) != 18);
			_vm->_soundManager._vm->_soundManager.PLAY_WAV(2);
			do
				_vm->_eventsManager.VBL();
			while ((unsigned __int16)BOBPOSI(5) != 38);
			_vm->_soundManager._vm->_soundManager.PLAY_WAV(3);
			do
				_vm->_eventsManager.VBL();
			while ((unsigned __int16)BOBPOSI(5) != 53);
			_vm->_animationManager.BOBANIM_OFF(5);
			SPRITE_ON(0);
		}
		if (v76 == 210) {
			_vm->_animationManager.NO_SEQ = 1;
			_vm->_soundManager.SPECIAL_SOUND = 210;
			_vm->_soundManager.PLAY_SEQ2("SECRET1.SEQ", 1, 12, 1);
			_vm->_soundManager.SPECIAL_SOUND = 0;
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 192, 152, 0);
			_vm->_animationManager.BOBANIM_ON(9);
			OBSSEUL = 1;
			INILINK("IM73a");
			OBSSEUL = 0;
			_vm->_globals.CACHE_ON();
			_vm->_animationManager.NO_SEQ = 0;
			_vm->_globals.CACHE_ADD(0);
			_vm->_globals.CACHE_ADD(1);
			_vm->_graphicsManager.SETCOLOR4(252, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR4(253, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR4(251, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR4(254, 0, 0, 0);
		}
		if (v76 == 211) {
			SPRITE_OFF(0);
			_vm->_globals.CACHE_OFF();
			_vm->_animationManager.NO_SEQ = 1;
			_vm->_globals.NO_VISU = 0;
			_vm->_soundManager.SPECIAL_SOUND = 211;
			_vm->_soundManager.PLAY_SEQ(v1, "SECRET2.SEQ", 1, 12, 100);
			_vm->_soundManager.SPECIAL_SOUND = 0;
			_vm->_animationManager.NO_SEQ = 0;
			_vm->_graphicsManager.NOFADE = 1;
			_vm->_graphicsManager.FADE_OUTW();
			v36 = 1;
			do {
				_vm->_eventsManager.VBL();
				++v36;
			} while (v36 <= 39);
			_vm->_graphicsManager.SETCOLOR4(252, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR4(253, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR4(251, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR4(254, 0, 0, 0);
		}
		if (v76 == 207)
			OBJET_VIVANT("PANNEAU.PE2");
		if (v76 == 208) {
			_vm->_globals.DESACTIVE_INVENT = 1;
			if (_vm->_globals.SAUVEGARDE->data[svField6] != _vm->_globals.SAUVEGARDE->data[svField401]) {
				_vm->_soundManager.SPECIAL_SOUND = 208;
				_vm->_eventsManager.NOESC = true;
				_vm->_soundManager.PLAY_SEQ(v1, "SORT.SEQ", 10, 4, 10);
				_vm->_eventsManager.NOESC = false;
				_vm->_soundManager.SPECIAL_SOUND = 0;
			}
			_vm->_globals.NOT_VERIF = 1;
			_vm->_globals.chemin = PTRNUL;
			v37 = YSPR(0);
			v38 = XSPR(0);
			_vm->_globals.chemin = PARCOURS2(v38, v37, 330, 418);
			_vm->_globals.NOT_VERIF = 1;
			_vm->_objectsManager.NUMZONE = 0;
			do {
				GOHOME();
				_vm->_eventsManager.VBL();
			} while (_vm->_globals.chemin != PTRNUL);
			SETANISPR(0, 64);
			_vm->_globals.SORTIE = _vm->_globals.SAUVEGARDE->data[svField401];
			_vm->_globals.DESACTIVE_INVENT = 0;
		}
		if (v76 == 209) {
			SET_BOBPOSI(1, 0);
			SET_BOBPOSI(2, 0);
			SETANISPR(0, 60);
			_vm->_animationManager.BOBANIM_OFF(4);
			_vm->_animationManager.BOBANIM_ON(1);
			do
				_vm->_eventsManager.VBL();
			while ((unsigned __int16)BOBPOSI(1) != 9);
			_vm->_animationManager.BOBANIM_OFF(1);
			_vm->_globals.NO_VISU = 1;
			_vm->_globals.chemin = PTRNUL;
			_vm->_globals.NOT_VERIF = 1;
			v39 = YSPR(0);
			v40 = XSPR(0);
			_vm->_globals.chemin = PARCOURS2(v40, v39, 330, 314);
			_vm->_objectsManager.NUMZONE = 0;
			_vm->_globals.NOT_VERIF = 1;
			do {
				GOHOME();
				_vm->_eventsManager.VBL();
			} while (_vm->_globals.chemin != PTRNUL);
			SETANISPR(0, 64);
			_vm->_animationManager.BOBANIM_ON(2);
			_vm->_soundManager.PLAY_SOUND("SOUND66.WAV");
			do
				_vm->_eventsManager.VBL();
			while ((unsigned __int16)BOBPOSI(2) != 10);
			_vm->_animationManager.BOBANIM_OFF(2);
			_vm->_animationManager.BOBANIM_ON(4);
		}
		if (v76 == 201) {
			_vm->_animationManager.BOBANIM_ON(3);
			do
				_vm->_eventsManager.VBL();
			while ((unsigned __int16)BOBPOSI(3) != 18);
			_vm->_animationManager.BOBANIM_OFF(3);
			_vm->_animationManager.BOBANIM_ON(4);
		}
		if (v76 == 203) {
			_vm->_globals.NO_VISU = 1;
			SPRITE_OFF(0);
			_vm->_animationManager.BOBANIM_ON(4);
			do {
				_vm->_eventsManager.VBL();
				if ((unsigned __int16)BOBPOSI(4) == 18)
					_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 18, 334, 0);
			} while ((unsigned __int16)BOBPOSI(4) != 26);
			_vm->_animationManager.BOBANIM_OFF(4);
			_vm->_globals.NO_VISU = 0;
			SPRITE_ON(0);
		}
		if (v76 == 204) {
			SPRITE_OFF(0);
			_vm->_animationManager.BOBANIM_ON(3);
			_vm->_soundManager.LOAD_WAV("SOUND67.WAV", 1);
			v41 = 0;
			do {
				if ((unsigned __int16)BOBPOSI(3) == 10 && !v41) {
					_vm->_soundManager._vm->_soundManager.PLAY_WAV(1);
					v41 = 1;
				}
				if ((unsigned __int16)BOBPOSI(3) == 11)
					v41 = 0;
				_vm->_eventsManager.VBL();
			} while ((unsigned __int16)BOBPOSI(3) != 50);
			_vm->_animationManager.BOBANIM_OFF(3);
			SPRITE_ON(0);
		}
		if (v76 == 205) {
			SPRITE_OFF(0);
			_vm->_animationManager.BOBANIM_ON(4);
			_vm->_soundManager.LOAD_WAV("SOUND69.WAV", 1);
			v42 = 0;
			do {
				if ((unsigned __int16)BOBPOSI(4) == 10 && !v42) {
					_vm->_soundManager.PLAY_WAV(1);
					v42 = 1;
				}
				if ((unsigned __int16)BOBPOSI(4) == 11)
					v42 = 0;
				_vm->_eventsManager.VBL();
			} while ((unsigned __int16)BOBPOSI(4) != 24);
			_vm->_animationManager.BOBANIM_OFF(4);
			SPRITE_ON(0);
		}
		if (v76 == 175) {
			SETANISPR(0, 55);
			SPRITE_OFF(0);
			_vm->_animationManager.BOBANIM_ON(9);
			_vm->_animationManager.BOBANIM_ON(10);
			BOB_OFFSET(10, 300);
			_vm->_soundManager.PLAY_SOUND("SOUND44.WAV");
			do
				_vm->_eventsManager.VBL();
			while ((unsigned __int16)BOBPOSI(10) != 7);
			_vm->_animationManager.BOBANIM_ON(6);
			_vm->_animationManager.BOBANIM_OFF(3);
			do
				_vm->_eventsManager.VBL();
			while ((unsigned __int16)BOBPOSI(6) != 10);
			_vm->_soundManager.PLAY_SOUND("SOUND71.WAV");
			_vm->_animationManager.BOBANIM_ON(7);
			_vm->_animationManager.BOBANIM_OFF(4);
			do
				_vm->_eventsManager.VBL();
			while ((unsigned __int16)BOBPOSI(7) != 15);
			_vm->_animationManager.BOBANIM_OFF(5);
			_vm->_animationManager.BOBANIM_ON(8);
			_vm->_soundManager.PLAY_SOUND("SOUND70.WAV");
			do
				_vm->_eventsManager.VBL();
			while ((unsigned __int16)BOBPOSI(8) != 76);
			_vm->_animationManager.BOBANIM_OFF(6);
			_vm->_animationManager.BOBANIM_OFF(7);
			_vm->_animationManager.BOBANIM_OFF(8);
			_vm->_animationManager.BOBANIM_OFF(9);
			_vm->_animationManager.BOBANIM_OFF(10);
			SPRITE_ON(0);
		}
		if (v76 == 229) {
			_vm->_soundManager.SPECIAL_SOUND = 229;
			_vm->_soundManager.PLAY_SEQ(v1, "MUR.SEQ", 1, 12, 1);
			_vm->_soundManager.SPECIAL_SOUND = 0;
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 340, 157, 2);
		}
		if (v76 == 230) {
			OBSSEUL = 1;
			INILINK("IM93a");
			OBSSEUL = 0;
			_vm->_globals.CACHE_ON();
			_vm->_globals.NOT_VERIF = 1;
			g_old_x = XSPR(0);
			g_old_sens = -1;
			_vm->_globals.Compteur = 0;
			_vm->_globals.chemin = PTRNUL;
			_vm->_globals.NOT_VERIF = 1;
			v43 = YSPR(0);
			v44 = XSPR(0);
			_vm->_globals.chemin = PARCOURS2(v44, v43, 488, 280);
			_vm->_globals.NOT_VERIF = 1;
			do {
				GOHOME();
				_vm->_eventsManager.VBL();
			} while (_vm->_globals.chemin != PTRNUL);
			SPRITE_OFF(0);
			v45 = 0;
			_vm->_animationManager.BOBANIM_ON(7);
			do {
				if ((unsigned __int16)BOBPOSI(7) == 9 && !v45) {
					v45 = 1;
					_vm->_soundManager.PLAY_SOUND("SOUND81.WAV");
				}
				_vm->_eventsManager.VBL();
			} while ((unsigned __int16)BOBPOSI(7) != 15);
			_vm->_animationManager.BOBANIM_OFF(7);
			SETXSPR(0, 476);
			SETYSPR(0, 278);
			SPRITE_ON(0);
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 337, 154, 3);
			OBSSEUL = 1;
			INILINK("IM93c");
			OBSSEUL = 0;
			_vm->_globals.CACHE_ON();
		}
		if (v76 == 231) {
			_vm->_globals.CACHE_OFF();
			SPRITE_OFF(0);
			_vm->_animationManager.BOBANIM_ON(12);
			do
				_vm->_eventsManager.VBL();
			while ((unsigned __int16)BOBPOSI(12) != 6);
			_vm->_globals.NOPARLE = 1;
			_vm->_talkManager.PARLER_PERSO("PRMORT.pe2");
			_vm->_globals.NOPARLE = 0;
			do
				_vm->_eventsManager.VBL();
			while ((unsigned __int16)BOBPOSI(12) != 12);
			SPRITE_ON(0);
			_vm->_animationManager.BOBANIM_OFF(12);
			_vm->_globals.CACHE_ON();
		}
		if (v76 == 233) {
			_vm->_globals.CACHE_OFF();
			SPRITE_OFF(0);
			_vm->_animationManager.BOBANIM_ON(11);
			v46 = 0;
			do {
				_vm->_eventsManager.VBL();
				if ((unsigned __int16)BOBPOSI(11) == 10 && !v46)
					v46 = 1;
			} while ((unsigned __int16)BOBPOSI(11) != 13);
			_vm->_animationManager.BOBANIM_OFF(11);
			_vm->_globals.CACHE_ON();
			_vm->_animationManager.BOBANIM_ON(13);
			do
				_vm->_eventsManager.VBL();
			while ((unsigned __int16)BOBPOSI(13) != 48);
			_vm->_globals.NOPARLE = 1;
			_vm->_talkManager.PARLER_PERSO("HRADIO.PE2");
			_vm->_globals.NOPARLE = 0;
			_vm->_graphicsManager.FADE_OUTW();
			_vm->_animationManager.BOBANIM_OFF(13);
			_vm->_graphicsManager.NOFADE = 1;
			_vm->_globals.SORTIE = 94;
		}
		if (v76 == 52) {
			_vm->_globals.NOPARLE = 1;
			_vm->_talkManager.PARLER_PERSO("GARDE.PE2");
			_vm->_globals.NOPARLE = 0;
		}
		if (v76 == 53)
			_vm->_talkManager.PARLER_PERSO("GARDE1.pe2");
		if (v76 == 54)
			_vm->_talkManager.PARLER_PERSO("GARDE2.pe2");
		if (v76 == 40)
			_vm->_talkManager.PARLER_PERSO("MAGE.pe2");
		if (v76 == 236) {
			v47 = _vm->_globals.SAUVEGARDE->data[svField341];
			if (v47) {
				if (v47 == 2)
					v70 = 5;
				if (v47 == 3)
					v70 = 4;
				if (v47 == 1)
					v70 = 6;
				_vm->_soundManager._vm->_soundManager.PLAY_SOUND("SOUND83.WAV");
				OPTI_ONE(v70, 26, 50, 0);
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 1)
					_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 27, 117, 0);
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 2)
					_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 145, 166, 2);
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 3)
					_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 296, 212, 4);
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 1)
					_vm->_globals.SAUVEGARDE->data[svField338] = 0;
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 2)
					_vm->_globals.SAUVEGARDE->data[svField339] = 0;
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 3)
					_vm->_globals.SAUVEGARDE->data[svField340] = 0;
			}
			_vm->_soundManager._vm->_soundManager.PLAY_SOUND("SOUND83.WAV");
			OPTI_ONE(6, 0, 23, 0);
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 27, 117, 1);
		}
		if (v76 == 237) {
			v48 = _vm->_globals.SAUVEGARDE->data[svField341];
			if (v48) {
				if (v48 == 2)
					v70 = 5;
				if (v48 == 3)
					v70 = 4;
				if (v48 == 1)
					v70 = 6;
				_vm->_soundManager._vm->_soundManager.PLAY_SOUND("SOUND83.WAV");
				OPTI_ONE(v70, 26, 50, 0);
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 1)
					_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 27, 117, 0);
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 2)
					_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 145, 166, 2);
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 3)
					_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 296, 212, 4);
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 1)
					_vm->_globals.SAUVEGARDE->data[svField338] = 0;
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 2)
					_vm->_globals.SAUVEGARDE->data[svField339] = 0;
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 3)
					_vm->_globals.SAUVEGARDE->data[svField340] = 0;
			}
			_vm->_soundManager._vm->_soundManager.PLAY_SOUND("SOUND83.WAV");
			OPTI_ONE(5, 0, 23, 0);
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 145, 166, 3);
		}
		if (v76 == 238) {
			v49 = _vm->_globals.SAUVEGARDE->data[svField341];
			if (v49) {
				if (v49 == 2)
					v70 = 5;
				if (v49 == 3)
					v70 = 4;
				if (v49 == 1)
					v70 = 6;
				_vm->_soundManager._vm->_soundManager.PLAY_SOUND("SOUND83.WAV");
				OPTI_ONE(v70, 26, 50, 0);
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 1)
					_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 27, 117, 0);
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 2)
					_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 145, 166, 2);
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 3)
					_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 296, 212, 4);
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 1)
					_vm->_globals.SAUVEGARDE->data[svField338] = 0;
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 2)
					_vm->_globals.SAUVEGARDE->data[svField339] = 0;
				if (_vm->_globals.SAUVEGARDE->data[svField341] == 3)
					_vm->_globals.SAUVEGARDE->data[svField340] = 0;
			}
			_vm->_soundManager._vm->_soundManager.PLAY_SOUND("SOUND83.WAV");
			OPTI_ONE(4, 0, 23, 0);
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 296, 212, 5);
		}
		if (v76 == 239) {
			SPRITE_OFF(0);
			_vm->_soundManager._vm->_soundManager.PLAY_SOUND("SOUND84.WAV");
			OPTI_ONE(16, 0, 10, 0);
		}
		if (v76 == 240) {
			_vm->_animationManager.BOBANIM_ON(1);
			v50 = 0;
			do {
				_vm->_eventsManager.VBL();
				if ((unsigned __int16)BOBPOSI(1) == 12 && !v50) {
					_vm->_soundManager._vm->_soundManager.PLAY_SOUND("SOUND86.WAV");
					v50 = 1;
				}
				if ((unsigned __int16)BOBPOSI(1) == 13)
					v50 = 0;
				if ((unsigned __int16)BOBPOSI(1) == 25 && !v50) {
					_vm->_soundManager._vm->_soundManager.PLAY_SOUND("SOUND85.WAV");
					v50 = 1;
				}
				if ((unsigned __int16)BOBPOSI(1) == 25)
					v50 = 0;
			} while ((unsigned __int16)BOBPOSI(1) != 32);
			_vm->_animationManager.BOBANIM_OFF(1);
			_vm->_animationManager.BOBANIM_ON(2);
			_vm->_fontManager.TEXTE_OFF(9);
			if (!_vm->_soundManager.TEXTOFF) {
				_vm->_fontManager.DOS_TEXT(9, 617, _vm->_globals.FICH_TEXTE, 91, 41, 20, 25, 3, 30, 253);
				_vm->_fontManager.TEXTE_ON(9);
			}
			if (!_vm->_soundManager.VOICEOFF)
				_vm->_soundManager.VOICE_MIX(617, 4);
			v71 = 0;
			do {
				_vm->_eventsManager.VBL();
				++v71;
			} while (v71 <= 29);
			v51 = &_vm->_globals.SAUVEGARDE->field370;
			v51->field0 = XSPR(0);
			v51->field1 = YSPR(0);
			v51->field2 = 57;
			v51->field3 = 97;
			_vm->_globals.SAUVEGARDE->data[svField121] = 1;
			_vm->_globals.SAUVEGARDE->data[svField352] = 1;
			_vm->_globals.SAUVEGARDE->data[svField353] = 1;
			_vm->_globals.SAUVEGARDE->data[svField354] = 1;
		}
		if (v76 == 56) {
			FileManager::CONSTRUIT_SYSTEM("HOPFEM.SPR");
			FileManager::CHARGE_FICHIER2(_vm->_globals.NFICHIER, _vm->_globals.PERSO);
			_vm->_globals.PERSO_TYPE = 1;
			_vm->_globals.SAUVEGARDE->data[svField122] = 1;
			_vm->_globals.HOPKINS_DATA();
			Sprite[18].field0 = 28;
			Sprite[20].field0 = 155;
			VERIFTAILLE();
		}
		if (v76 == 57) {
			FileManager::CONSTRUIT_SYSTEM("PERSO.SPR");
			FileManager::CHARGE_FICHIER2(_vm->_globals.NFICHIER, _vm->_globals.PERSO);
			_vm->_globals.PERSO_TYPE = 0;
			_vm->_globals.SAUVEGARDE->data[svField122] = 0;
			_vm->_globals.HOPKINS_DATA();
			Sprite[18].field0 = 34;
			Sprite[20].field0 = 190;
			VERIFTAILLE();
		}
		if (v76 == 25)
			_vm->_talkManager.PARLER_PERSO("AGENT1.pe2");
		if (v76 == 26)
			_vm->_talkManager.PARLER_PERSO("AGENT2.pe2");
		if (v76 == 87) {
			if (_vm->_globals.SAUVEGARDE->data[svField188])
				_vm->_talkManager.PARLER_PERSO("stand2.pe2");
			else
				_vm->_talkManager.PARLER_PERSO("stand1.pe2");
		}
		if (v76 == 86) {
			if (_vm->_globals.SAUVEGARDE->data[svField231] == 1) {
				_vm->_talkManager.PARLER_PERSO("chotess1.pe2");
			} else {
				_vm->_globals.NOPARLE = 1;
				_vm->_talkManager.PARLER_PERSO("chotesse.pe2");
				_vm->_globals.NOPARLE = 0;
			}
		}
		if (v76 == 51) {
			_vm->_graphicsManager.FADE_OUTW();
			_vm->_globals.CACHE_OFF();
			SPRITE_OFF(0);
			_vm->_fontManager.TEXTE_OFF(5);
			_vm->_fontManager.TEXTE_OFF(9);
			_vm->_graphicsManager.FIN_VISU();
			_vm->_graphicsManager.LOAD_IMAGE("IM20f");
			_vm->_animationManager.CHARGE_ANIM("ANIM20f");
			_vm->_graphicsManager.VISU_ALL();
			_vm->_eventsManager.MOUSE_OFF();
			_vm->_graphicsManager.FADE_INW();
			v52 = 0;
			_vm->_soundManager.LOAD_WAV("SOUND46.WAV", 1);
			do {
				if ((unsigned __int16)BOBPOSI(12) == 5 && !v52) {
					_vm->_soundManager.PLAY_WAV(1);
					v52 = 1;
				}
				_vm->_eventsManager.VBL();
			} while ((unsigned __int16)BOBPOSI(12) != 34);
			_vm->_animationManager.BOBANIM_OFF(2);
			_vm->_graphicsManager.FADE_OUTW();
			_vm->_graphicsManager.NOFADE = 1;
			_vm->_globals.SORTIE = 20;
		}
		if (v76 == 41)
			_vm->_talkManager.PARLER_PERSO("MORT3.pe2");
		if (v76 == 44)
			_vm->_talkManager.PARLER_PERSO("MORT3A.pe2");
		if (v76 == 42)
			_vm->_talkManager.PARLER_PERSO("MORT2.pe2");
		if (v76 == 43)
			_vm->_talkManager.PARLER_PERSO("MORT1.pe2");
		if (v76 == 47)
			_vm->_talkManager.PARLER_PERSO("BARMAN.pe2");
		if (v76 == 45)
			_vm->_talkManager.PARLER_PERSO("FEM3.pe2");
		if (v76 == 48)
			_vm->_talkManager.PARLER_PERSO("SAMAN2.pe2");
		if (v76 == 21)
			_vm->_talkManager.PARLER_PERSO("MEDLEG.pe2");
		if (v76 == 94) {
			if (!_vm->_globals.SAUVEGARDE->data[svField228])
				_vm->_talkManager.PARLER_PERSO("flicn.pe2");
			if (_vm->_globals.SAUVEGARDE->data[svField228] == 1)
				_vm->_talkManager.PARLER_PERSO("flicn1.pe2");
		}
		if (v76 == 27) {
			if (_vm->_globals.SAUVEGARDE->data[svField94] != 1 || _vm->_globals.SAUVEGARDE->data[svField95] != 1)
				_vm->_talkManager.PARLER_PERSO("STANDAR.pe2");
			else
				_vm->_talkManager.PARLER_PERSO("STANDAR1.pe2");
		}
		if (v76 == 58) {
			_vm->_globals.NOPARLE = 1;
			_vm->_talkManager.PARLER_PERSO("Gm1.PE2");
			_vm->_globals.SAUVEGARDE->data[svField176] = 1;
			_vm->_globals.SAUVEGARDE->data[svField270] = 2;
			_vm->_globals.NOPARLE = 0;
		}
		if (v76 == 200) {
			_vm->_globals.NOPARLE = 1;
			_vm->_talkManager.PARLER_PERSO("Gm2.PE2");
			_vm->_globals.NOPARLE = 0;
		}
		if (v76 == 84) {
			_vm->_globals.NOPARLE = 1;
			_vm->_talkManager.PARLER_PERSO("CVIGIL1.PE2");
			_vm->_globals.NOPARLE = 0;
		}
		if (v76 == 98) {
			_vm->_globals.NOPARLE = 1;
			_vm->_talkManager.PARLER_PERSO("CVIGIL2.PE2");
			_vm->_globals.NOPARLE = 0;
		}
		if (v76 == 83)
			_vm->_talkManager.PARLER_PERSO("CVIGIL.pe2");
		if (v76 == 32)
			_vm->_talkManager.PARLER_PERSO("SAMAN.pe2");
		if (v76 == 215) {
			_vm->_globals.NOPARLE = 1;
			_vm->_talkManager.PARLER_PERSO("aviat.pe2");
			_vm->_globals.NOPARLE = 0;
		}
		if (v76 == 216) {
			_vm->_globals.NOPARLE = 1;
			_vm->_talkManager.PARLER_PERSO("aviat1.pe2");
			_vm->_globals.NOPARLE = 0;
		}
		if (v76 == 170)
			_vm->_talkManager.PARLER_PERSO("GRED.pe2");
		if (v76 == 172)
			_vm->_talkManager.PARLER_PERSO("GBLEU.pe2");
		if (v76 == 100)
			_vm->_talkManager.PARLER_PERSO("tourist.pe2");
		if (v76 == 103) {
			_vm->_globals.NOPARLE = 1;
			_vm->_talkManager.PARLER_PERSO("tourist1.pe2");
			_vm->_globals.NOPARLE = 0;
			if (_vm->_globals.SVGA == 1)
				_vm->_animationManager.PLAY_ANM2("T421.ANM", 100, 14, 500);
			if (_vm->_globals.SVGA == 2)
				_vm->_animationManager.PLAY_ANM2("T421a.ANM", 100, 14, 500);
			_vm->_eventsManager.VBL();
			_vm->_eventsManager.VBL();
			_vm->_eventsManager.VBL();
			_vm->_globals.NOPARLE = 1;
			_vm->_talkManager.PARLER_PERSO("tourist2.pe2");
			_vm->_globals.NOPARLE = 0;
		}
		if (v76 == 104) {
			_vm->_globals.NOPARLE = 1;
			_vm->_talkManager.PARLER_PERSO("tourist3.pe2");
			_vm->_globals.NOPARLE = 0;
		}
		if (v76 == 108) {
			_vm->_globals.NOPARLE = 1;
			_vm->_talkManager.PARLER_PERSO("peche1.pe2");
			_vm->_globals.NOPARLE = 0;
		}
		if (v76 == 109) {
			_vm->_globals.NOPARLE = 1;
			_vm->_talkManager.PARLER_PERSO("peche2.pe2");
			_vm->_globals.NOPARLE = 0;
		}
		if (v76 == 110) {
			_vm->_globals.NOPARLE = 1;
			_vm->_talkManager.PARLER_PERSO("peche3.pe2");
			_vm->_globals.NOPARLE = 0;
		}
		if (v76 == 111) {
			_vm->_globals.NOPARLE = 1;
			_vm->_talkManager.PARLER_PERSO("peche4.pe2");
			_vm->_globals.NOPARLE = 0;
		}
		if (v76 == 112) {
			_vm->_globals.NOPARLE = 1;
			_vm->_talkManager.PARLER_PERSO("teint1.pe2");
			_vm->_globals.NOPARLE = 0;
		}
		if (v76 == 176) {
			_vm->_globals.NOPARLE = 1;
			_vm->_talkManager.PARLER_PERSO("gred2.pe2");
			_vm->_globals.NOPARLE = 0;
		}
		if (v76 == 177) {
			_vm->_globals.NOPARLE = 1;
			_vm->_talkManager.PARLER_PERSO("gbleu2.pe2");
			_vm->_globals.NOPARLE = 0;
		}
		if (v76 == 113)
			_vm->_talkManager.PARLER_PERSO("teint.pe2");
		if (v76 == 114)
			_vm->_talkManager.PARLER_PERSO("tahibar.pe2");
		if (v76 == 115)
			_vm->_talkManager.PARLER_PERSO("ilebar.pe2");
		if (v76 == 116)
			_vm->_talkManager.PARLER_PERSO("Profred.pe2");
		if (v76 == 101)
			_vm->_talkManager.PARLER_PERSO("tahi1.pe2");
		if (v76 == 243) {
			_vm->_soundManager._vm->_soundManager.PLAY_SOUND("SOUND88.WAV");
			if (_vm->_globals.SAUVEGARDE->data[svField341] == 2) {
				_vm->_animationManager.NO_SEQ = 1;
				_vm->_soundManager.PLAY_SEQ(v1, "RESU.SEQ", 2, 24, 2);
				_vm->_animationManager.NO_SEQ = 0;
			} else {
				OPTI_ONE(7, 0, 14, 0);
			}
		}
		if (v76 == 242) {
			_vm->_soundManager.PLAY_SOUND("SOUND87.WAV");
			_vm->_animationManager.NO_SEQ = 1;
			_vm->_soundManager.PLAY_SEQ(v1, "RESUF.SEQ", 1, 24, 1);
			_vm->_animationManager.NO_SEQ = 0;
			v53 = &_vm->_globals.SAUVEGARDE->field380;
			v53->field0 = 404;
			v53->field1 = 395;
			v53->field2 = 64;
			v53->field3 = _vm->_globals.ECRAN;

			v54 = _vm->_globals.STAILLE[790 / 2];
			if (_vm->_globals.STAILLE[790 / 2] < 0)
				v54 = -_vm->_globals.STAILLE[790 / 2];
			v76 = -(signed __int16)(100 * (67 - (signed __int16)(100 - v54)) / 67);
			v53->field4 = v76;
			_vm->_globals.SAUVEGARDE->data[svField357] = 1;
			_vm->_globals.SAUVEGARDE->data[svField354] = 0;
			_vm->_globals.SAUVEGARDE->data[svField356] = 0;
			_vm->_globals.SAUVEGARDE->data[svField355] = 1;
			DEUXPERSO = 1;
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 373, 191, 3);
			SPRITE(_vm->_globals.TETE, v53->field0, v53->field1, 1, 3, v53->field4, 0, 20, 127);
			SPRITE_ON(1);
		}
		if (v76 == 245) {
			_vm->_soundManager.PLAY_SOUND("SOUND89.WAV");
			OPTI_ONE(5, 0, 6, 0);
			// TODO: is 98 correct?
			_vm->_globals.ZONEP[98].field0 = 276;
			VERBE_ON(4, 19);
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 285, 379, 0);
			_vm->_globals.SAUVEGARDE->data[svField399] = 1;
		}
		if (v76 == 246) {
			SPRITE_OFF(0);
			OPTI_ONE(6, 0, 15, 0);
			_vm->_objectsManager.PERSO_ON = 1;
			_vm->_graphicsManager.NB_SCREEN();
			_vm->_animationManager.NO_SEQ = 1;
			_vm->_soundManager.PLAY_SEQ2("TUNNEL.SEQ", 1, 18, 20);
			_vm->_animationManager.NO_SEQ = 0;
			_vm->_graphicsManager.NOFADE = 1;
			_vm->_graphicsManager.FADE_OUTW();
			_vm->_objectsManager.PERSO_ON = 0;
			_vm->_globals.SORTIE = 100;
		}
		if (v76 == 55) {
			_vm->_animationManager.BOBANIM_OFF(1);
			OPTI_ONE(15, 0, 12, 0);
			_vm->_animationManager.BOBANIM_OFF(15);
			OBSSEUL = 1;
			INILINK("IM19a");
			OBSSEUL = 0;
		}
		if (v76 == 241)
			_vm->_talkManager.PARLER_PERSO("RECEP.PE2");
		if (v76 == 171) {
			_vm->_globals.NOPARLE = 1;
			_vm->_talkManager.PARLER_PERSO("gred1.pe2");
			_vm->_globals.NOPARLE = 0;
			_vm->_globals.NOT_VERIF = 1;
			g_old_x = XSPR(0);
			g_old_sens = -1;
			_vm->_globals.Compteur = 0;
			_vm->_globals.chemin = PTRNUL;
			_vm->_globals.NOT_VERIF = 1;
			v55 = YSPR(0);
			v56 = XSPR(0);
			_vm->_globals.chemin = PARCOURS2(v56, v55, 361, 325);
			_vm->_globals.NOT_VERIF = 1;
			_vm->_objectsManager.NUMZONE = -1;
			do {
				GOHOME();
				_vm->_eventsManager.VBL();
			} while (_vm->_globals.chemin != PTRNUL);
			_vm->_globals.SORTIE = 59;
		}
		if (v76 == 173) {
			_vm->_globals.NOPARLE = 1;
			_vm->_talkManager.PARLER_PERSO("gbleu1.pe2");
			_vm->_globals.NOPARLE = 0;
			_vm->_globals.NOT_VERIF = 1;
			g_old_x = XSPR(0);
			g_old_sens = -1;
			_vm->_globals.Compteur = 0;
			_vm->_globals.chemin = PTRNUL;
			_vm->_globals.NOT_VERIF = 1;
			v57 = YSPR(0);
			v58 = XSPR(0);
			_vm->_globals.chemin = PARCOURS2(v58, v57, 361, 325);
			_vm->_globals.NOT_VERIF = 1;
			_vm->_objectsManager.NUMZONE = -1;
			do {
				GOHOME();
				_vm->_eventsManager.VBL();
			} while (_vm->_globals.chemin != PTRNUL);
			_vm->_globals.SORTIE = 59;
		}
		if (v76 == 174)
			_vm->_talkManager.PARLER_PERSO("Profbl.pe2");
		if (v76 == 202) {
			_vm->_globals.NOPARLE = 1;
			_vm->_talkManager.PARLER_PERSO("SVGARD2.PE2");
			_vm->_globals.NOPARLE = 0;
		}
		v1 = 1;
	}
	if (*(a1 + 2) == 69 && *(a1 + 3) == 73 && *(a1 + 4) == 70)
		v1 = 4;
	if (*(a1 + 2) == 86 && *(a1 + 3) == 65 && *(a1 + 4) == 76) {
		v1 = 1;
		_vm->_globals.SAUVEGARDE->data[READ_LE_UINT16(a1 + 5)] = READ_LE_UINT16(a1 + 7);
	}
	if (*(a1 + 2) == 65 && *(a1 + 3) == 68 && *(a1 + 4) == 68) {
		v1 = 1;
		_vm->_globals.SAUVEGARDE->data[READ_LE_UINT16(a1 + 5)] += *(a1 + 7);
	}
	if (*(a1 + 2) == 66 && *(a1 + 3) == 79 && *(a1 + 4) == 83) {
		v1 = 1;
		BOB_OFFSET(READ_LE_UINT16(a1 + 5), READ_LE_UINT16(a1 + 7));
	}
	if (*(a1 + 2) == 86 && *(a1 + 3) == 79 && *(a1 + 4) == 78) {
		VERBE_ON(READ_LE_UINT16(a1 + 5), READ_LE_UINT16(a1 + 7));
		v1 = 1;
	}
	if (*(a1 + 2) == 90 && *(a1 + 3) == 67 && *(a1 + 4) == 72) {
		_vm->_globals.ZONEP[READ_LE_UINT16(a1 + 5)].field12 = READ_LE_UINT16(a1 + 7);
		v1 = 1;
	}
	if (*(a1 + 2) == 74 && *(a1 + 3) == 85 && *(a1 + 4) == 77) {
		v59 = READ_LE_UINT16(a1 + 7);
		NVZONE = READ_LE_UINT16(a1 + 5);
		NVVERBE = v59;
		v1 = 6;
	}
	if (*(a1 + 2) == 83 && *(a1 + 3) == 79 && *(a1 + 4) == 85) {
		v60 = READ_LE_UINT16(a1 + 5);
		memset(&s, 0, 0x13u);
		memset(v77, 0, 5u);
		sprintf(v77, "%d", v60);
		s = 83;
		v79 = 79;
		v80 = 85;
		v81 = 78;
		v82[0] = 68;
		v61 = 5;
		v62 = 0;
		do
			*(&s + v61++) = v77[v62++];
		while (v77[v62]);
		v63 = v61;
		*(&s + v63) = 46;
		*(&v79 + v63) = 87;
		*(&v80 + v63) = 65;
		*(&v81 + v63) = 86;
		v82[v63] = 0;
		_vm->_soundManager.PLAY_SOUND(s);
		v1 = 1;
	}
	if (*(a1 + 2) == 86 && *(a1 + 3) == 79 && *(a1 + 4) == 70) {
		VERBE_OFF(READ_LE_UINT16(a1 + 5), READ_LE_UINT16(a1 + 7));
		v1 = 1;
	}
	if (*(a1 + 2) == 73 && *(a1 + 3) == 73) {
		if (*(a1 + 4) == 70)
			v1 = 3;
	}

	return v1;
}

void ObjectsManager::BOB_VIVANT(int a1) {
	warning("TODO: BOB_VIVANT");
}

void ObjectsManager::VBOB(byte *a1, int a2, int a3, int a4, int a5) {
	warning("TODO: VBOB");
}

void ObjectsManager::VBOB_OFF(int idx) {
	warning("TODO: VBOB_OFF");
}

void ObjectsManager::ACTION_DOS(int idx) {
	warning("TODO: ACTION_DOS");
}

void ObjectsManager::ACTION_DROITE(int idx) {
	warning("TODO: ACTION_DROITE");
}

void ObjectsManager::Q_DROITE(int idx) {
	warning("TODO: Q_DROITE");
}

void ObjectsManager::ACTION_FACE(int idx) {
	warning("TODO: ACTION_FACE");
}

void ObjectsManager::Q_GAUCHE(int idx) {
	warning("TODO: Q_GAUCHE");
}

void ObjectsManager::ACTION_GAUCHE(int idx) {
	warning("TODO: ACTION_GAUCHE");
}

void ObjectsManager::ZONE_ON(int idx) {
	warning("TODO: ZONE_ON");
}

void ObjectsManager::ZONE_OFF(int idx) {
	warning("TODO: ZONE_OFF");
}

void ObjectsManager::OPTI_ONE(int a1, int a2, int a3, int a4) {
	warning("TODO: OPTI_ONE");
}

int ObjectsManager::BOBPOSI(int a1) {
	warning("BOBPOSI");
	return 0;
}

void ObjectsManager::AFFICHE_SPEED1(byte *speedData, int xp, int yp, int img) {
	SPEED_FLAG = true;
	SPEED_PTR = speedData;
	_vm->_objectsManager.SPEED_X = xp;
	_vm->_objectsManager.SPEED_Y = yp;
	SPEED_IMAGE = img;
}

void ObjectsManager::SET_BOBPOSI(int a1, int a2) {
	warning("TODO: SET_BOBPOSI");
}

void ObjectsManager::INILINK(const Common::String &file) {
	warning("TODO: INILINK");
}

void ObjectsManager::OBJET_VIVANT(const Common::String &a2) {
	warning("TODO: OBJET_VIVANT");
}

void ObjectsManager::OPTI_BOBON(int a1, int a2, int a3, int a4, int a5, int a6, int a7) {
	warning("TODO: OPTI_BOBON");
}

void ObjectsManager::SCI_OPTI_ONE(int a1, int a2, int a3, int a4) {
	warning("TODO: SCI_OPTI_ONE");
}

int ObjectsManager::Control_Goto(const byte *dataP) {
	return READ_LE_UINT16(dataP + 5);
}

int ObjectsManager::Control_If(const byte *dataP, int a2) {
	warning("TODO: Control_If");
	return 0;
}

void ObjectsManager::VERBE_OFF(int a1, int a2) {
	warning("TODO: VERBE_OFF");
}

void ObjectsManager::VERBE_ON(int a1, int a2) {
	warning("TODO: VERBE_ON");
}
/*
int ObjectsManager::PARC_PERS(int a1, int a2, int a3, int a4, int a5, int a6, int a7) {
	warning("TODO: PARC_PERS");
	return 0;
}

int ObjectsManager::MIRACLE(int a1, int a2, int a3, int a4, int a5) {
	warning("TODO: MIRACLE");
	return 0;
}
*/

} // End of namespace Hopkins
