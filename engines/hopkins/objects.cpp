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
	int v34;
	int v35;
	int v36;
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
	int v2;
	int v3;
	int v4;
	int v6;
	int v7;
	int v8; 
	int v9; 

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
								if (v6 < v1) {
									v1 = 0;
									if (v9 >= v3 && v9 <= (_vm->_globals.Cache[idx].field6 + v3)) {
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
	int v20;
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
		
		if (v7 < 0) {
			v7 = v7;
			if (v7 < 0)
				v7 = -v7;
			v20 = v7;
			if (v7 > 95)
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
				v22 = -_vm->_graphicsManager.Reel_Zoom(v8, v21);
			}
			if (v6 >= 0) {
				v17 = _vm->_graphicsManager.Reel_Zoom(v6, v21);
			} else {
				v9 = v15;
				if (v15 < 0)
					v9 = -v15;
				v6 = v9;
				v17 = -_vm->_graphicsManager.Reel_Zoom(v9, v21);
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
				v22 = -_vm->_graphicsManager.Reel_Reduc(v10, v20);
			}
			if (v6 >= 0) {
				v17 = _vm->_graphicsManager.Reel_Reduc(v6, v20);
			} else {
				v11 = v15;
				if (v15 < 0)
					v11 = -v15;
				v17 = -_vm->_graphicsManager.Reel_Reduc(v11, v20);
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

	v8 = 0;
	do {
		if (_vm->_globals.Cache[v8].fieldA > 0) {
			v7 = _vm->_globals.Cache[v8].fieldA;
			v10 = 0;
			do {
				if (Sprite[v10].field0 == 1) {
					if (Sprite[v10].field10 != 250) {
						v1 = Sprite[v10].field2C;
						v11 = Sprite[v10].field30 + v1;
						v2 = Sprite[v10].field32 + Sprite[v10].field2E;
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
	int v1;

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
		if (v9 < 0) {
			v9 = v9;
			if (v9 < 0)
				v9 = -v9;
			reducePercent = v9;
			if (v9 > 95)
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
				v22 = -_vm->_graphicsManager.Reel_Zoom(v10, zoomPercent);
			}
      
			if (v8 >= 0) {
				v7 = _vm->_graphicsManager.Reel_Zoom(v8, zoomPercent);
			} else {
				v11 = v4;
				if (v4 < 0)
					v11 = -v4;
				v8 = v11;
				v7 = -_vm->_graphicsManager.Reel_Zoom(v11, zoomPercent);
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
				v22 = -_vm->_graphicsManager.Reel_Reduc(v12, reducePercent);
			}
			if (v8 >= 0) {
				v7 = _vm->_graphicsManager.Reel_Reduc(v8, reducePercent);
			} else {
				v13 = v4;
				if (v4 < 0)
					v13 = -v4;
				v7 = -_vm->_graphicsManager.Reel_Reduc(v13, reducePercent);
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
	int v1;
	int v2;
	int v5;
	int v6;
	int v7;
	int v8;
	int v10;
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
	if (idx > 5)
		error("Tentative d'affichage d'un sprite > MAX_SPRITE.");
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
	int v0;
	int v1; 
	int v2;
	uint16 v3;
	int v4;

	v0 = _vm->_eventsManager.XMOUSE();
	v1 = _vm->_eventsManager.YMOUSE();
	v2 = v1;
	if (_vm->_globals.PLAN_FLAG
	        || _vm->_eventsManager.start_x >= v0
	        || (v1 = _vm->_graphicsManager.ofscroll + 54, v0 >= v1)
	        || (v1 = v2 - 1, (uint16)(v2 - 1) > 0x3Bu)) {
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
			NUMZONE = v4;
			_vm->_globals.old_x_69 = v0;
			_vm->_globals.old_y_70 = v2;
			_vm->_globals.old_zone_68 = v4;
			if (_vm->_globals.NOMARCHE == 1) {
				if (_vm->_eventsManager.btsouris == 4) {
					v1 = v4 + 1;
					if ((uint16)(v4 + 1) > 1u)
						BTDROITE();
				}
			}
			if ((_vm->_globals.PLAN_FLAG == 1 && v4 == -1) || !v4) {
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

void ObjectsManager::CARRE_ZONE() {
	byte *v3;
	int v4; 
	int v5; 
	byte *v6; 
	int v7; 
	int v8; 
	int v10; 
	int v11; 
	int v12; 
	int v13; 
	int v14;

	for (int idx = 0; idx < 100; ++idx) {
		_vm->_globals.CarreZone[idx].field0 = 0;
		_vm->_globals.CarreZone[idx].fieldE = 0;
		_vm->_globals.CarreZone[idx].field2 = 1280;
		_vm->_globals.CarreZone[idx].field4 = 0;
		_vm->_globals.CarreZone[idx].field6 = 460;
		_vm->_globals.CarreZone[idx].field8 = 0;
		_vm->_globals.CarreZone[idx].fieldA = 401;
		_vm->_globals.CarreZone[idx].fieldC = 0;
	}

	for (int idx = 0; idx < 400; ++idx) {
		v3 = _vm->_linesManager.LigneZone[idx].field4;
		if (v3 != PTRNUL) {
			v4 = _vm->_linesManager.LigneZone[idx].field2;
			_vm->_globals.CarreZone[v4].field0 = 1;
			if (_vm->_globals.CarreZone[v4].fieldC < idx)
				_vm->_globals.CarreZone[v4].fieldC = idx;
			if (_vm->_globals.CarreZone[v4].fieldA > idx)
				_vm->_globals.CarreZone[v4].fieldA = idx;

			v13 = 0;
			v12 = _vm->_linesManager.LigneZone[idx].field0;
			if (v12 > 0) {
				do {
					v5 = READ_LE_UINT16(v3);
					v6 = v3 + 2;
					v11 = READ_LE_UINT16(v6);
					v3 = v6 + 2;
					if (_vm->_globals.CarreZone[v4].field2 >= v5)
						_vm->_globals.CarreZone[v4].field2 = v5;
					if (_vm->_globals.CarreZone[v4].field4 <= v5)
						_vm->_globals.CarreZone[v4].field4 = v5;
					if (_vm->_globals.CarreZone[v4].field6 >= v11)
						_vm->_globals.CarreZone[v4].field6 = v11;
					if (_vm->_globals.CarreZone[v4].field8 <= v11)
						_vm->_globals.CarreZone[v4].field8 = v11;
					++v13;
				} while (v13 < v12);
			}
		}
	}

	v7 = 0;
	do {
		v8 = v7;
		v10 = _vm->_globals.CarreZone[v8].field2 - _vm->_globals.CarreZone[v8].field4;
		if (v10 < 0)
			v10 = -v10;
		v14 = _vm->_globals.CarreZone[v8].field6 - _vm->_globals.CarreZone[v8].field8;
		if (v14 < 0)
			v14 = -v14;
		if (v10 == v14)
			_vm->_globals.CarreZone[v8].fieldE = 1;
		++v7;
	} while (v7 <= 99);
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
	int v1;
	int v2;
	byte *v3; 
	byte *v4; 
	byte *v5; 
	int v6; 
	int v7;
	int v8;
	byte *v9; 
	int v10;
	int v11;
	int v12;
	byte *v13; 
	int v14;
	int v15;
	byte *v16; 
	int v17;
	int v18;
	int v19;

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
		if (YSPR(0) <= 374 || YSPR(0) > 410) {
			v10 = XSPR(0);
			v11 = YSPR(0);
			v12 = XSPR(0);
			v13 = _vm->_linesManager.PARCOURS2(v12, v11, v10, 390);
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
			v16 = _vm->_linesManager.PARCOURS2(v15, v14, v19, v0);
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
	int v1;
	char result;
	int v3; 
	uint16 v4;
	int v5; 
	uint16 v6;

	v1 = 0;
	ARRET_PERSO_FLAG = 0;
	ARRET_PERSO_NUM = 0;
	result = _vm->_globals.SAUVEGARDE->data[svField1];
	if (result && _vm->_globals.SAUVEGARDE->data[svField2] && result != 4 && result > 3) {
		_vm->_fontManager.TEXTE_OFF(5);
		if (_vm->_globals.FORET != 1 || ((uint16)(NUMZONE - 20) > 1u && (uint16)(NUMZONE - 22) > 1u)) {
			if (_vm->_graphicsManager.DOUBLE_ECRAN == 1) {
				_vm->_graphicsManager.no_scroll = 2;
				if (_vm->_eventsManager.start_x >= XSPR(0) - 320)
					goto LABEL_64;
				v3 = _vm->_eventsManager.start_x + 320 - XSPR(0);
				if (v3 < 0)
					v3 = -v3;
				if (v3 <= 160) {
LABEL_64:
					if (_vm->_eventsManager.start_x > XSPR(0) - 320) {
						v5 = _vm->_eventsManager.start_x + 320 - XSPR(0);
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
								if (_vm->_eventsManager.XMOUSE() > _vm->_graphicsManager.SCROLL + 620) {
									v6 = _vm->_eventsManager.YMOUSE();
									_vm->_eventsManager.souris_xy(_vm->_eventsManager.souris_x - 4, v6);
								}
								_vm->_eventsManager.VBL();
							} while (v1 != 1 && _vm->_eventsManager.start_x > XSPR(0) - 320);
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
						if (_vm->_eventsManager.XMOUSE() < _vm->_graphicsManager.SCROLL + 10) {
							v4 = _vm->_eventsManager.YMOUSE();
							_vm->_eventsManager.souris_xy(_vm->_eventsManager.souris_x + 4, v4);
						}
						_vm->_eventsManager.VBL();
					} while (v1 != 1 && _vm->_eventsManager.start_x < XSPR(0) - 320);
				}
				if (_vm->_eventsManager.XMOUSE() > _vm->_graphicsManager.SCROLL + 620)
					_vm->_eventsManager.souris_xy(_vm->_graphicsManager.SCROLL + 610, 0);
				if (_vm->_eventsManager.XMOUSE() < _vm->_graphicsManager.SCROLL + 10)
					_vm->_eventsManager.souris_xy(_vm->_graphicsManager.SCROLL + 10, 0);
				_vm->_eventsManager.VBL();
				_vm->_graphicsManager.no_scroll = 0;
			}
			_vm->_talkManager.REPONSE(_vm->_globals.SAUVEGARDE->data[svField2], _vm->_globals.SAUVEGARDE->data[svField1]);
		} else {
			_vm->_talkManager.REPONSE2(_vm->_globals.SAUVEGARDE->data[svField2], _vm->_globals.SAUVEGARDE->data[svField1]);
		}
		_vm->_eventsManager.CHANGE_MOUSE(4);
		if ((uint16)(NUMZONE + 1) > 1u && !_vm->_globals.ZONEP[NUMZONE].field16) {
			NUMZONE = -1;
			FORCEZONE = 1;
		}
		if (NUMZONE != _vm->_globals.SAUVEGARDE->data[svField2] || (uint16)(NUMZONE + 1) <= 1u) {
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
			if ((uint16)(NUMZONE + 1) > 1u)
				BTDROITE();
		}
	}
	GOACTION = 0;
}

void ObjectsManager::CLEAR_ECRAN() {
	int v1;
	int v2;

	CLEAR_SPR();
	_vm->_graphicsManager.FIN_VISU();
	_vm->_fontManager.TEXTE_OFF(5);
	_vm->_fontManager.TEXTE_OFF(9);
	_vm->_globals.CLEAR_VBOB();
	_vm->_animationManager.CLEAR_ANIM();
	_vm->_linesManager.CLEAR_ZONE();
	_vm->_linesManager.RESET_OBSTACLE();
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
	_vm->_linesManager.TOTAL_LIGNES = 0;
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

void ObjectsManager::TEST_INVENT() {
	if (_vm->_globals.PLAN_FLAG)
		KEY_INVENT = 0;
	if (KEY_INVENT == 1) {
		if (!INVENTFLAG) {
			KEY_INVENT = 0;
			INVENTFLAG = 1;
			INVENT();
			INVENTFLAG = 0;
			KEY_INVENT = 0;
		}
	}
}

void ObjectsManager::INVENT() {
	int v1; 
	size_t filesize; 
	int v4;
	int v5; 
	int v6; 
	byte *v7; 
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
		v18 = Get_Largeur(_vm->_globals.Winventaire, 0);
		v17 = Get_Hauteur(_vm->_globals.Winventaire, 0);
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
			v11 = _vm->_linesManager.ZONE_OBJET(v8, v9);
			v13 = v11;
			if (v11 != v10)
				PARAMCADRE(v11);
			if (_vm->_eventsManager.btsouris != 16) {
				if ((uint16)(_vm->_eventsManager.btsouris - 1) > 1u) {
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
				if (_vm->_eventsManager.btsouris == 1 || _vm->_eventsManager.btsouris == 16 || !_vm->_eventsManager.btsouris || (uint16)(_vm->_eventsManager.btsouris - 2) <= 1u)
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
			if ((uint16)(_vm->_globals.ECRAN - 35) <= 5u)
				SPECIAL_JEU();
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
				v3->field4 = Sprite[0].field0;
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
			v5->field4 = Sprite[0].fieldC;
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
		v7->field4 = Sprite[0].fieldC;
	}
	if (a1 == 1) {
		v8 = &_vm->_globals.SAUVEGARDE->field360;
		v8->field0 = XSPR(0);
		v8->field1 = YSPR(0);
		v8->field2 = 64;
		v8->field3 = _vm->_globals.ECRAN;
		v8->field4 = Sprite[0].fieldC;
	}
	if (a1 == 2) {
		v9 = &_vm->_globals.SAUVEGARDE->field380;
		v9->field0 = XSPR(0);
		v9->field1 = YSPR(0);
		v9->field2 = 64;
		v9->field3 = _vm->_globals.ECRAN;
		v9->field4 = Sprite[0].fieldC;
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

void ObjectsManager::VERIFTAILLE() {
	int v0; 
	int v1; 
	int v2; 

	v0 = _vm->_globals.STAILLE[YSPR(0)];
	if (_vm->_globals.PERSO_TYPE == 1) {
		v1 = v0;
		if (v0 < 0)
			v1 = -v0;
		v0 = 20 * (5 * v1 - 100) / -80;
	}
	if (_vm->_globals.PERSO_TYPE == 2) {
		v2 = v0;
		if (v0 < 0)
			v2 = -v0;
		v0 = 20 * (5 * v2 - 165) / -67;
	}
	SETTAILLESPR(0, v0);
}

void ObjectsManager::PACOURS_PROPRE(byte *a1) {
	int v1;
	int v2;
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

	v1 = 0;
	v13 = 0;
	v14 = -1;
	v2 = READ_LE_UINT16(a1 + 2);
	v15 = READ_LE_UINT16(a1 + 4);
	if ((int16)READ_LE_UINT16(a1) != -1 || v2 != -1) {
		while (1) {
			if (v14 != -1 && v15 != v14) {
				v11 = v1;
				v12 = 0;
				v10 = CALC_PROPRE(v2);
				v4 = READ_LE_UINT16(a1 + 2 * v1);
				v9 = READ_LE_UINT16(a1 + 2 * v1);
				v5 = READ_LE_UINT16(a1 + 2 * v1 + 2);
				v6 = 0;
				while (v4 != -1 || v5 != -1) {
					int idx = v1;
					v1 += 4;
					++v12;
					if (READ_LE_UINT16(a1 + 2 * idx + 4) != v15)
						v6 = 1;
					if (v6 == 1)
						break;
					v4 = READ_LE_UINT16(a1 + 2 * v1);
					v9 = READ_LE_UINT16(a1 + 2 * v1);
					v5 = READ_LE_UINT16(a1 + 2 * v1 + 2);
				}
				if (v12 < v10) {
					v7 = v11;
					v8 = 0;
					if (v12 > 0) {
						do {
							WRITE_LE_UINT16(a1 + 2 * v7 + 4, v14);
							v7 += 4;
							++v8;
						} while (v12 > v8);
					}
					v15 = v14;
				}
				v1 = v11;
				if (v9 == -1 && v5 == -1)
					v13 = 1;
			}
			v1 += 4;
			if (v13 == 1)
				break;
			v14 = v15;
			v2 = READ_LE_UINT16(a1 + 2 * v1 + 2);
			v15 = READ_LE_UINT16(a1 + 2 * v1 + 4);
			if (READ_LE_UINT16(a1 + 2 * v1) == -1) {
				if (v2 == -1)
					break;
			}
		}
	}
}

byte *ObjectsManager::PARC_VOITURE(int a1, int a2, int a3, int a4) {
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
	byte *result; 
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
	byte *v33; 
	int v34; 
	int v35; 
	int i; 
	byte *v37;
	int v38;
	int v39; 
	int v40;
	byte *v41; 
	int v42;
	int v43; 
	int k;
	byte *v45; 
	int v46;
	int v47; 
	int v48; 
	int v49;
	byte *v50; 
	int v51;
	int v52; 
	int v53;
	byte *v54; 
	int v55;
	int v56; 
	int v57;
	byte *v58; 
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
	int v69;
	int j; 
	int l; 
	int v72;
	int v73;
	int v74;
	int v75;
	int v76[10];
	int v77; 
	int v78; 
	int v79; 
	int v80; 
	int v81; 
	int v82; 
	int v83 = 0; 
	int v84 = 0; 
	int v85 = 0; 
	int v86 = 0; 
	int v87; 
	int v88 = 0; 
	int v89 = 0; 
	int v90 = 0; 
	int v91 = 0; 

	v4 = a3;
	v5 = a4;
	v67 = 0;
	if (a3 <= 14)
		v4 = 15;
	if (a4 <= 14)
		v5 = 15;
	if (v4 > _vm->_graphicsManager.max_x - 10)
		v4 = _vm->_graphicsManager.max_x - 10;
	if (v5 > 445)
		v5 = 440;
	v75 = v4;
	v74 = v5;
	v6 = 0;
	v7 = v74;
	if (_vm->_graphicsManager.max_y > v74) {
		v8 = 5;
		do {
			v62 = v8;
			v9 = _vm->_linesManager.colision2_ligne(v75, v7, &v85, &v90, 0, DERLIGNE);
			v8 = v62;
			if (v9 == 1 && *(&v87 + v62) <= DERLIGNE)
				break;
			*(&v82 + v62) = 0;
			*(&v87 + v62) = -1;
			++v6;
			++v7;
		} while (_vm->_graphicsManager.max_y > v7);
	}
	v80 = v6;
	v10 = 0;
	v11 = v74;
	if (_vm->_graphicsManager.min_y < v74) {
		v12 = 1;
		do {
			v63 = v12;
			v13 = _vm->_linesManager.colision2_ligne(v75, v11, &v83, &v88, 0, DERLIGNE);
			v12 = v63;
			if (v13 == 1 && *(&v87 + v63) <= DERLIGNE)
				break;
			*(&v82 + v63) = 0;
			*(&v87 + v63) = -1;
			if (v80 < v10) {
				if (v90 != -1)
					break;
			}
			++v10;
			--v11;
		} while (_vm->_graphicsManager.min_y < v11);
	}
	v78 = v10;
	v14 = 0;
	v15 = v75;
	if (_vm->_graphicsManager.max_x > v75) {
		v16 = 3;
		do {
			v64 = v16;
			v17 = _vm->_linesManager.colision2_ligne(v15, v74, &v84, &v89, 0, DERLIGNE);
			v16 = v64;
			if (v17 == 1 && *(&v87 + v64) <= DERLIGNE)
				break;
			*(&v82 + v64) = 0;
			*(&v87 + v64) = -1;
			++v14;
			if (v78 < v14) {
				if (v88 != -1)
					break;
			}
			if (v80 < v14 && v90 != -1)
				break;
			++v15;
		} while (_vm->_graphicsManager.max_x > v15);
	}
	v79 = v14;
	v18 = 0;
	v19 = v75;
	if (_vm->_graphicsManager.min_x < v75) {
		v20 = 7;
		do {
			v65 = v20;
			v21 = _vm->_linesManager.colision2_ligne(v19, v74, &v86, &v91, 0, DERLIGNE);
			v20 = v65;
			if (v21 == 1 && *(&v87 + v65) <= DERLIGNE)
				break;
			*(&v82 + v65) = 0;
			*(&v87 + v65) = -1;
			++v18;
			if (v78 < v18) {
				if (v88 != -1)
					break;
			}
			if (v80 < v18 && v90 != -1)
				break;
			if (v79 < v18 && v89 != -1)
				break;
			--v19;
		} while (_vm->_graphicsManager.min_x < v19);
	}
	v81 = v18;
	if (v88 == -1)
		v78 = 1300;
	if (v89 == -1)
		v79 = 1300;
	if (v90 == -1)
		v80 = 1300;
	if (v91 == -1)
		v81 = 1300;
	if (v88 != -1 || v89 != -1 || v90 != -1 || v91 != -1) {
		v23 = 0;
		if (v90 != -1 && v78 >= v80 && v79 >= v80 && v81 >= v80) {
			v73 = v90;
			v72 = v85;
			v23 = 1;
		}
		if (v88 != -1 && !v23 && v80 >= v78 && v79 >= v78 && v81 >= v78) {
			v73 = v88;
			v72 = v83;
			v23 = 1;
		}
		if (v89 != -1 && !v23 && v78 >= v79 && v80 >= v79 && v81 >= v79) {
			v73 = v89;
			v72 = v84;
			v23 = 1;
		}
		if (v91 != -1 && !v23 && v80 >= v81 && v79 >= v81 && v78 >= v81) {
			v73 = v91;
			v72 = v86;
		}
		v24 = 0;
		do {
			v25 = v24;
			*(&v87 + v25) = -1;
			*(&v82 + v25) = 0;
			*(&v77 + v25) = 1300;
			v76[v25] = 1300;
			++v24;
		} while (v24 <= 8);
		v26 = _vm->_linesManager.colision2_ligne(a1, a2, &v83, &v88, 0, DERLIGNE);
		if (v26 == 1) {
			v69 = v88;
			v68 = v83;
		}
		if (!v26) {
			if (_vm->_linesManager.colision2_ligne(a1, a2, &v83, &v88, 0, _vm->_linesManager.TOTAL_LIGNES) == 1) {
				v27 = 0;
				while (1) {
					v28 = READ_LE_UINT16(_vm->_globals.essai2 + 2 * (signed __int16)v27);
					v29 = READ_LE_UINT16(_vm->_globals.essai2 + 2 * (signed __int16)v27 + 2);
					v66 = READ_LE_UINT16(_vm->_globals.essai2 + 2 * (signed __int16)v27 + 4);
					v27 = v27 + 4;
					v30 = v27;
					v31 = _vm->_linesManager.colision2_ligne(v28, v29, &v83, &v88, 0, DERLIGNE);
					v27 = v30;
					if (v31)
						break;
					v32 = v67;
					_vm->_globals.super_parcours[v32] = v28;
					_vm->_globals.super_parcours[v32 + 1] = v29;
					_vm->_globals.super_parcours[v32 + 2] = v66;
					_vm->_globals.super_parcours[v32 + 3] = 0;
					v33 = _vm->_globals.essai0;
					WRITE_LE_UINT16(_vm->_globals.essai0 + 2 * v32, v28);
					WRITE_LE_UINT16(v33 + 2 * v32 + 2, v29);
					WRITE_LE_UINT16(v33 + 2 * v32 + 4, v66);
					WRITE_LE_UINT16(v33 + 2 * v32 + 6, 0);
					v67 += 4;
					if (v28 == -1)
						goto LABEL_90;
				}
				v69 = v88;
				v68 = v83;
			} else {
				v69 = 1;
				v68 = 1;
				v67 = 0;
			}
		}
LABEL_90:
		if (v69 < v73) {
			v34 = v68;
			v35 = v68;
			for (i = _vm->_linesManager.Ligne[v69].field0; v35 < (i - 2); i = _vm->_linesManager.Ligne[v69].field0) {
				v37 = _vm->_linesManager.Ligne[v69].fieldC;
				v38 = READ_LE_UINT16(v37 + 4 * v35);
				int v37_2 = READ_LE_UINT16(v37 + 4 * v35 + 2);
				v39 = v67;
				_vm->_globals.super_parcours[v39] = v38;
				_vm->_globals.super_parcours[v39 + 1] = v37_2;
				_vm->_globals.super_parcours[v39 + 2] = _vm->_linesManager.Ligne[v69].field6;
				_vm->_globals.super_parcours[v39 + 3] = 0;
				v67 += 4;
				++v34;
				v35 = v34;
			}
			for (j = v69 + 1; j < v73; ++j) {
				if (_vm->_linesManager.PLAN_TEST(
						READ_LE_UINT16(_vm->_linesManager.Ligne[j].fieldC),
						READ_LE_UINT16(_vm->_linesManager.Ligne[j].fieldC + 2),
				        v67,
				        j,
				        v73,
				        0) == 1) {
LABEL_88:
							v69 = _vm->_linesManager.NV_LIGNEDEP;
							v68 = _vm->_linesManager.NV_LIGNEOFS;
							v67 = _vm->_linesManager.NV_POSI;
							goto LABEL_90;
				}

				v40 = 0;
				if (_vm->_linesManager.Ligne[j].field0 - 2 > 0) {
					do {
						v41 = _vm->_linesManager.Ligne[j].fieldC;
						v42 = READ_LE_UINT16(v41 + 4 * v40);
						int v41_2 = READ_LE_UINT16(v41 + 4 * v40 + 2);
						v43 = v67;
						_vm->_globals.super_parcours[v43] = v42;
						_vm->_globals.super_parcours[v43 + 1] = v41_2;
						_vm->_globals.super_parcours[v43 + 2] = _vm->_linesManager.Ligne[j].field6;
						_vm->_globals.super_parcours[v43 + 3] = 0;
						v67 += 4;
						++v40;
					} while (v40 < _vm->_linesManager.Ligne[j].field0 - 2);
				}
			}
			v68 = 0;
			v69 = v73;
		}
		if (v69 > v73) {
			for (k = v68; k > 0; --k) {
				v45 = _vm->_linesManager.Ligne[v69].fieldC;
				v46 = READ_LE_UINT16(v45 + 4 * k);
				int v45_2 = READ_LE_UINT16(v45 + 4 * k + 2);
				v47 = v67;
				_vm->_globals.super_parcours[v47] = v46;
				_vm->_globals.super_parcours[v47 + 1] = v45_2;
				_vm->_globals.super_parcours[v47 + 2] = _vm->_linesManager.Ligne[v69].field8;
				_vm->_globals.super_parcours[v47 + 3] = 0;
				v67 += 4;
			}
			for (l = v69 - 1; l > v73; --l) {
				v48 = l;
				if (_vm->_linesManager.PLAN_TEST(
						READ_LE_UINT16(_vm->_linesManager.Ligne[l].fieldC + 4 * _vm->_linesManager.Ligne[v48].field0 - 4),
						READ_LE_UINT16(_vm->_linesManager.Ligne[l].fieldC + 4 * _vm->_linesManager.Ligne[v48].field0 - 2),
				        v67,
				        l,
				        v73,
				        0) == 1)
					goto LABEL_88;
				v49 = _vm->_linesManager.Ligne[v48].field0 - 2;
				if ((_vm->_linesManager.Ligne[v48].field0 - 2) > 0) {
					do {
						v50 = _vm->_linesManager.Ligne[l].fieldC;
						v51 = READ_LE_UINT16(v50 + 4 * v49);
						int v50_2 = READ_LE_UINT16(v50 + 4 * v49 + 2);
						v52 = v67;
						_vm->_globals.super_parcours[v52] = v51;
						_vm->_globals.super_parcours[v52 + 1] = v50_2;
						_vm->_globals.super_parcours[v52 + 2] = _vm->_linesManager.Ligne[l].field8;
						_vm->_globals.super_parcours[v52 + 3] = 0;
						v67 += 4;
						--v49;
					} while (v49 > 0);
				}
			}
			v68 = _vm->_linesManager.Ligne[v73].field0 - 1;
			v69 = v73;
		}
		if (v69 == v73) {
			if (v68 <= v72) {
				if (v68 < v72) {
					v57 = v68;
					do {
						v58 = _vm->_linesManager.Ligne[v73].fieldC;
						v59 = READ_LE_UINT16(v58 + 4 * v57);
						int v58_2 = READ_LE_UINT16(v58 + 4 * v57 + 2);
						v60 = v67;
						_vm->_globals.super_parcours[v60] = v59;
						_vm->_globals.super_parcours[v60 + 1] = v58_2;
						_vm->_globals.super_parcours[v60 + 2] = _vm->_linesManager.Ligne[v73].field6;
						_vm->_globals.super_parcours[v60 + 3] = 0;
						v67 += 4;
						++v57;
					} while (v72 > v57);
				}
			} else {
				v53 = v68;
				do {
					v54 = _vm->_linesManager.Ligne[v73].fieldC;
					v55 = READ_LE_UINT16(v54 + 4 * v53);
					int v54_2 = READ_LE_UINT16(v54 + 4 * v53 + 2);
					v56 = v67;
					_vm->_globals.super_parcours[2 * v56] = v55;
					_vm->_globals.super_parcours[2 * v56 + 1] = v54_2;
					_vm->_globals.super_parcours[2 * v56 + 2] = _vm->_linesManager.Ligne[v73].field8;
					_vm->_globals.super_parcours[2 * v56 + 3] = 0;
					v67 += 4;
					--v53;
				} while (v72 < v53);
			}
		}
		v61 = v67;
		_vm->_globals.super_parcours[v61] = -1;
		_vm->_globals.super_parcours[v61 + 1] = -1;
		_vm->_globals.super_parcours[v61 + 2] = -1;
		_vm->_globals.super_parcours[v61 + 3] = -1;
		result = (byte *)&_vm->_globals.super_parcours[0];
	} else {
		result = PTRNUL;
	}
	return result;
}

void ObjectsManager::VERBEPLUS() {
	int v;

	v = _vm->_eventsManager.btsouris + 1;
	_vm->_eventsManager.btsouris = v;
	if (v == 4)
		goto LABEL_24;
	if (v == 5)
		goto LABEL_28;
	if (v == 6)
		goto LABEL_29;
	if (v == 7)
		goto LABEL_31;
	if (v == 8)
		goto LABEL_33;
	if (v == 9)
		goto LABEL_35;
	if (v == 10)
		goto LABEL_37;
	if (v == 11)
		goto LABEL_39;
	if (v == 12)
		goto LABEL_41;
	if (v == 13)
		goto LABEL_43;
	if (v == 14)
		goto LABEL_45;
	if (v == 15)
		goto LABEL_47;
	if (v == 16)
		goto LABEL_49;
	if (v == 17)
		goto LABEL_51;
	if (v == 18)
		goto LABEL_53;
	if (v == 19)
		goto LABEL_55;
	if (v == 20)
		goto LABEL_57;
	if (v == 21)
		goto LABEL_59;
	if (v == 22)
		goto LABEL_61;
	if (v == 23)
		goto LABEL_63;
	if (v == 24)
		goto LABEL_65;
	if (v == 25)
		goto LABEL_67;
	do {
		do {
			_vm->_eventsManager.btsouris = 4;
LABEL_24:
			if (_vm->_globals.NOMARCHE != 1 || (v = NUMZONE + 1, (uint16)(NUMZONE + 1) <= 1u)) {
				if (_vm->_eventsManager.btsouris == 4)
					return;
			} else {
				_vm->_eventsManager.btsouris = 5;
			}
LABEL_28:
			++_vm->_eventsManager.btsouris;
			if (_vm->_eventsManager.btsouris == 6) {
LABEL_29:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].field6 == 1)
					return;
			}
			++_vm->_eventsManager.btsouris;
			if (_vm->_eventsManager.btsouris == 7) {
LABEL_31:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].field7 == 1)
					return;
			}
			++_vm->_eventsManager.btsouris;
			if (_vm->_eventsManager.btsouris == 8) {
LABEL_33:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].field8 == 1)
					return;
			}
			++_vm->_eventsManager.btsouris;
			if (_vm->_eventsManager.btsouris == 9) {
LABEL_35:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].field9 == 1)
					return;
			}
			++_vm->_eventsManager.btsouris;
			if (_vm->_eventsManager.btsouris == 10) {
LABEL_37:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].fieldA == 1)
					return;
			}
			++_vm->_eventsManager.btsouris;
			if (_vm->_eventsManager.btsouris == 11) {
LABEL_39:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].fieldB == 1)
					return;
			}
			++_vm->_eventsManager.btsouris;
			if (_vm->_eventsManager.btsouris == 12) {
LABEL_41:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].fieldC == 1)
					return;
			}
			++_vm->_eventsManager.btsouris;
			if (_vm->_eventsManager.btsouris == 13) {
LABEL_43:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].fieldD == 1)
					return;
			}
			++_vm->_eventsManager.btsouris;
			if (_vm->_eventsManager.btsouris == 14) {
LABEL_45:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].fieldE == 1)
					return;
			}
			++_vm->_eventsManager.btsouris;
			if (_vm->_eventsManager.btsouris == 15) {
LABEL_47:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].fieldF == 1)
					return;
			}
			++_vm->_eventsManager.btsouris;
			if (_vm->_eventsManager.btsouris == 16) {
LABEL_49:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].field6 == 2)
					return;
			}
			++_vm->_eventsManager.btsouris;
			if (_vm->_eventsManager.btsouris == 17) {
LABEL_51:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].field9 == 2)
					return;
			}
			++_vm->_eventsManager.btsouris;
			if (_vm->_eventsManager.btsouris == 18) {
LABEL_53:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].fieldA == 2)
					return;
			}
			++_vm->_eventsManager.btsouris;
			if (_vm->_eventsManager.btsouris == 19) {
LABEL_55:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].fieldB == 2)
					return;
			}
			++_vm->_eventsManager.btsouris;
			if (_vm->_eventsManager.btsouris == 20) {
LABEL_57:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].fieldC == 2)
					return;
			}
			++_vm->_eventsManager.btsouris;
			if (_vm->_eventsManager.btsouris == 21) {
LABEL_59:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].fieldF == 2)
					return;
			}
			++_vm->_eventsManager.btsouris;
			if (_vm->_eventsManager.btsouris == 22) {
LABEL_61:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].fieldD == 2)
					return;
			}
			++_vm->_eventsManager.btsouris;
			if (_vm->_eventsManager.btsouris == 23) {
LABEL_63:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].field8 == 2)
					return;
			}
			++_vm->_eventsManager.btsouris;
			if (_vm->_eventsManager.btsouris == 24) {
LABEL_65:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].field9 == 3)
					return;
			}
			v = _vm->_eventsManager.btsouris + 1;
			_vm->_eventsManager.btsouris = v;
		} while (v != 25);
LABEL_67:
		v = 5 * NUMZONE;
	} while (_vm->_globals.ZONEP[NUMZONE].fieldE != 2);
}

void ObjectsManager::BTDROITE() {
	if ((uint16)(NUMZONE + 1) > 1u) {
		VERBEPLUS();
		if (_vm->_eventsManager.btsouris != 23)
			_vm->_eventsManager.CHANGE_MOUSE(_vm->_eventsManager.btsouris);
		verbe = _vm->_eventsManager.btsouris;
	}
}

int ObjectsManager::MZONE() {
	signed int result;
	__int16 v1;
	__int16 v2;
	int v3; 
	__int16 v4;
	__int16 v5;
	int v6; 
	__int16 v7;
	int v8; 
	int v9; 
	__int16 i;
	__int16 v11;
	__int16 j; 
	__int16 k; 
	__int16 l; 
	int v15; 
	__int16 v16;
	__int16 v17;
	__int16 v18;
	__int16 v19;
	__int16 v20;
	__int16 v21;

	v19 = 0;
	v18 = 0;
	v17 = 0;
	v16 = 0;
	v21 = _vm->_eventsManager.souris_x + _vm->_eventsManager.ofset_souris_x;
	v20 = _vm->_eventsManager.souris_y + _vm->_eventsManager.ofset_souris_y;
	if ((_vm->_eventsManager.souris_y + _vm->_eventsManager.ofset_souris_y) > 19) {
		v1 = 0;
		do {
			v2 = _vm->_globals.BOBZONE[v1];
			if (v2) {
				if (_vm->_globals.BOBZONE_FLAG[v1] == 1) {
					v3 = v2;
					v15 = v3;
					if (_vm->_globals.Bob[v3].field0) {
						if (_vm->_globals.Bob[v3].fieldC != 250) {
							if (!_vm->_globals.Bob[v3].field16) {
								v4 = _vm->_globals.Bob[v3].field3E;
								if (v21 > v4) {
									if (v21 < _vm->_globals.Bob[v3].field42 + v4) {
										v5 = _vm->_globals.Bob[v3].field40;
										if (v20 > v5) {
											if (v20 < _vm->_globals.Bob[v3].field44 + v5) {
												v6 = v1;
												if (_vm->_globals.ZONEP[v1].field4 == -1) {
													_vm->_globals.ZONEP[v6].field0 = 0;
													_vm->_globals.ZONEP[v1].field2 = 0;
												}
												if (!_vm->_globals.ZONEP[v6].field0) {
													if (!_vm->_globals.ZONEP[v1].field2) {
														_vm->_globals.ZONEP[v6].field0 = _vm->_globals.Bob[v15].field42 + _vm->_globals.Bob[v15].field3E;
														_vm->_globals.ZONEP[v1].field2 = _vm->_globals.Bob[v15].field44 + _vm->_globals.Bob[v15].field40 + 6;
														_vm->_globals.ZONEP[v1].field4 = -1;
													}
												}
												return v1;
											}
										}
									}
								}
							}
						}
					}
				}
			}
			++v1;
		} while (v1 <= 48);
		_vm->_globals.SegmentEnCours = 0;
		v7 = 0;
		do {
			if (_vm->_globals.ZONEP[v7].field10 == 1) {
				v8 = v7;
				if (_vm->_globals.CarreZone[v8].field0 == 1) {
					if (_vm->_globals.CarreZone[v7].field2 <= v21
					        && _vm->_globals.CarreZone[v7].field4 >= v21
					        && _vm->_globals.CarreZone[v7].field6 <= v20
					        && _vm->_globals.CarreZone[v7].field8 >= v20) {
								if (_vm->_globals.CarreZone[v7].fieldE == 1) {
									_vm->_globals.oldzone_46 = _vm->_linesManager.LigneZone[_vm->_globals.CarreZone[v7].fieldA].field2;
							return _vm->_globals.oldzone_46;
						}
						v9 = _vm->_globals.SegmentEnCours;
						_vm->_globals.Segment[v9].field2 = _vm->_globals.CarreZone[v7].fieldA;
						_vm->_globals.Segment[v9].field4 = _vm->_globals.CarreZone[v7].fieldC;
						++_vm->_globals.SegmentEnCours;
					}
				}
			}
			++v7;
		} while (v7 <= 99);
		if (!_vm->_globals.SegmentEnCours)
			goto LABEL_58;
		for (i = v20; i >= 0; --i) {
			v11 = colision(v21, i);
			v19 = v11;
			if (v11 != -1 && _vm->_globals.ZONEP[v11].field10 == 1)
				break;
		}
		if (v19 == -1)
			goto LABEL_58;
		for (j = v20; _vm->_graphicsManager.max_y > j; ++j) {
			v18 = colision(v21, j);
			if (v18 != -1 && _vm->_globals.ZONEP[v19].field10 == 1)
				break;
		}
		if (v18 == -1)
			goto LABEL_58;
		for (k = v21; k >= 0; --k) {
			v16 = colision(k, v20);
			if (v16 != -1 && _vm->_globals.ZONEP[v19].field10 == 1)
				break;
		}
		if (v16 == -1)
			goto LABEL_58;
		for (l = v21; _vm->_graphicsManager.max_x > l; ++l) {
			v17 = colision(l, v20);
			if (v17 != -1 && _vm->_globals.ZONEP[v19].field10 == 1)
				break;
		}
		if (v19 == v18 && v19 == v16 && v19 == v17) {
			_vm->_globals.oldzone_46 = v19;
			result = v19;
		} else {
LABEL_58:
			_vm->_globals.oldzone_46 = -1;
			result = -1;
		}
	} else {
		result = 0;
	}
	return result;
}

void ObjectsManager::PARAMCADRE(int a1) {
	old_cadx = cadx;
	old_cady = cady;
	old_cadi = cadi;
	if ((uint16)(a1 - 1) <= 5u)
		cady = 120;
	if ((uint16)(a1 - 7) <= 5u)
		cady = 158;
	if ((uint16)(a1 - 13) <= 5u)
		cady = 196;
	if ((uint16)(a1 - 19) <= 5u)
		cady = 234;
	if ((uint16)(a1 - 25) <= 4u)
		cady = 272;
	if (a1 == 1 || a1 == 7 || a1 == 13 || a1 == 19 || a1 == 25)
		cadx = _vm->_graphicsManager.ofscroll + 158;
	if (a1 == 2 || a1 == 8 || a1 == 14 || a1 == 20 || a1 == 26)
		cadx = _vm->_graphicsManager.ofscroll + 212;
	if (a1 == 3 || a1 == 9 || a1 == 15 || a1 == 21 || a1 == 27)
		cadx = _vm->_graphicsManager.ofscroll + 266;
	if (a1 == 4 || a1 == 10 || a1 == 16 || a1 == 22 || a1 == 28)
		cadx = _vm->_graphicsManager.ofscroll + 320;
	if (a1 == 5 || a1 == 11 || a1 == 17 || a1 == 23 || a1 == 29)
		cadx = _vm->_graphicsManager.ofscroll + 374;
	if (a1 == 6 || a1 == 12 || a1 == 18 || a1 == 24 || (uint16)(a1 - 30) <= 1u)
		cadx = _vm->_graphicsManager.ofscroll + 428;
	if ((uint16)(a1 - 1) <= 0x1Cu)
		cadi = 0;
	if ((uint16)(a1 - 30) <= 1u)
		cadi = 2;
	if (a1 == 30)
		cady = 272;
	if (a1 == 31)
		cady = 290;
	if (!a1 || a1 == 32) {
		cadx = 0;
		cady = 0;
		cadi = 0;
	}
	if (!a1)
		_vm->_eventsManager.btsouris = 0;
	if (a1 == 32)
		_vm->_eventsManager.btsouris = 16;
	if (a1 == 30)
		_vm->_eventsManager.btsouris = 2;
	if (a1 == 31)
		_vm->_eventsManager.btsouris = 3;
	if ((uint16)(a1 - 1) <= 0x1Cu)
		_vm->_eventsManager.btsouris = 8;
	if (a1 == 29)
		_vm->_eventsManager.btsouris = 1;
	if ((uint16)(a1 - 1) <= 0x1Bu && !_vm->_globals.INVENTAIRE[a1]) {
		_vm->_eventsManager.btsouris = 0;
		cadx = 0;
		cady = 0;
		cadi = 0;
	}
	if (_vm->_eventsManager.btsouris != 23)
		_vm->_eventsManager.CHANGE_MOUSE(_vm->_eventsManager.btsouris);
	_vm->_eventsManager.XMOUSE();
	_vm->_eventsManager.YMOUSE();
}

void ObjectsManager::OBJETPLUS(int idx) {
	int v1;
	int v2;
	int v3;

	v1 = _vm->_eventsManager.btsouris;
	if (_vm->_eventsManager.btsouris && _vm->_eventsManager.btsouris != 16 && (uint16)(_vm->_eventsManager.btsouris - 2) > 1u) {
		v2 = _vm->_eventsManager.btsouris++ + 1;
		if (v1 == 5)
			goto LABEL_24;
		if (v2 == 7)
			goto LABEL_26;
		if (v2 != 8) {
			if (v2 == 9)
				_vm->_eventsManager.btsouris = 10;
			if (_vm->_eventsManager.btsouris == 10)
				goto LABEL_29;
			if (_vm->_eventsManager.btsouris == 11)
				goto LABEL_31;
			if (_vm->_eventsManager.btsouris == 12)
				_vm->_eventsManager.btsouris = 13;
			if (_vm->_eventsManager.btsouris == 13)
				goto LABEL_33;
			if (_vm->_eventsManager.btsouris == 14)
				_vm->_eventsManager.btsouris = 15;
			if (_vm->_eventsManager.btsouris == 15)
				goto LABEL_35;
			if ((uint16)(_vm->_eventsManager.btsouris - 16) <= 6u)
				_vm->_eventsManager.btsouris = 23;
			if (_vm->_eventsManager.btsouris == 23)
				goto LABEL_37;
			if (_vm->_eventsManager.btsouris == 24)
				_vm->_eventsManager.btsouris = 25;
			if (_vm->_eventsManager.btsouris == 25)
				goto LABEL_39;
			do {
				_vm->_eventsManager.btsouris = 6;
LABEL_24:
				if (_vm->_globals.ObjetW[_vm->_globals.INVENTAIRE[idx]].field2 == 1)
					break;
				++_vm->_eventsManager.btsouris;
				if (_vm->_eventsManager.btsouris == 7) {
LABEL_26:
					if (_vm->_globals.ObjetW[_vm->_globals.INVENTAIRE[idx]].field3 == 1)
						return;
				}
				v3 = _vm->_eventsManager.btsouris++;
				if (_vm->_eventsManager.btsouris == 8)
					break;
				_vm->_eventsManager.btsouris = v3 + 3;
				if (v3 == 7) {
LABEL_29:
					if (_vm->_globals.ObjetW[_vm->_globals.INVENTAIRE[idx]].field7 == 1)
						return;
				}
				++_vm->_eventsManager.btsouris;
				if (_vm->_eventsManager.btsouris == 11) {
LABEL_31:
					if (_vm->_globals.ObjetW[_vm->_globals.INVENTAIRE[idx]].field4 == 1)
						return;
				}
				_vm->_eventsManager.btsouris += 2;
				if (_vm->_eventsManager.btsouris == 13) {
LABEL_33:
					if (_vm->_globals.ObjetW[_vm->_globals.INVENTAIRE[idx]].field5 == 1)
						return;
				}
				_vm->_eventsManager.btsouris += 2;
				if (_vm->_eventsManager.btsouris == 15) {
LABEL_35:
					if (_vm->_globals.ObjetW[_vm->_globals.INVENTAIRE[idx]].field6 == 1)
						return;
				}
				_vm->_eventsManager.btsouris = 23;
LABEL_37:
				if (_vm->_globals.ObjetW[_vm->_globals.INVENTAIRE[idx]].field6 == 2)
					break;
				_vm->_eventsManager.btsouris = 25;
LABEL_39:
				;
			} while (_vm->_globals.ObjetW[_vm->_globals.INVENTAIRE[idx]].field7 != 2);
		}
	}
}

void ObjectsManager::VALID_OBJET(int a1) {
	if (_vm->_eventsManager.btsouris == 8)
		CHANGE_OBJET(a1);
}

void ObjectsManager::OPTI_OBJET() {
	byte *data; 
	Common::String file;
	int v0 = 1;
	int v5;
	int v7;

	file = "OBJET1.ini";
	data = FileManager::RECHERCHE_CAT(file, 1);
	if (data == PTRNUL) {
		FileManager::CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, file);
		data = FileManager::CHARGE_FICHIER(_vm->_globals.NFICHIER);
	}
	
	if ((data == PTRNUL) || *data != 'I' || *(data + 1) != 'N' || *(data + 2) != 'I') {
		error("Not an INI file");
	} else {
		v7 = 0;
		do {
			v5 = Traduction(data + 20 * v0);
			if (v5 == 2)
				v0 = Control_Goto(data + 20 * v0);
			if (v5 == 3)
				v0 = Control_If(data, v0);
			if (v0 == -1)
				error("defective IFF function");
			if (v5 == 1 || v5 == 4)
				++v0;
			if (!v5 || v5 == 5)
				v7 = 1;
		} while (v7 != 1);
	}
	_vm->_globals.dos_free2(data);
}

void ObjectsManager::SPECIAL_JEU() {
	byte *v1;
	byte *v2;
	byte *v3;

	if ((uint16)(_vm->_globals.ECRAN - 35) <= 6u) {
		if (_vm->_globals.OLD_ECRAN == 16 && _vm->_globals.ECRAN == 35)
			TEST_FORET(35, 500, 555, 100, 440, 1);
		if (_vm->_globals.OLD_ECRAN == 36 && _vm->_globals.ECRAN == 35)
			TEST_FORET(35, 6, 84, 100, 440, 4);
		if (_vm->_globals.OLD_ECRAN == 35 && _vm->_globals.ECRAN == 36)
			TEST_FORET(36, 551, 633, 100, 440, 2);
		if (_vm->_globals.OLD_ECRAN == 37 && _vm->_globals.ECRAN == 36)
			TEST_FORET(36, 6, 84, 100, 440, 4);
		if (_vm->_globals.OLD_ECRAN == 36 && _vm->_globals.ECRAN == 37)
			TEST_FORET(37, 551, 633, 100, 440, 1);
		if (_vm->_globals.OLD_ECRAN == 38 && _vm->_globals.ECRAN == 37)
			TEST_FORET(37, 392, 529, 100, 440, 2);
		if (_vm->_globals.OLD_ECRAN == 37 && _vm->_globals.ECRAN == 38)
			TEST_FORET(38, 133, 252, 100, 440, 4);
		if (_vm->_globals.OLD_ECRAN == 39 && _vm->_globals.ECRAN == 38)
			TEST_FORET(38, 6, 84, 100, 440, 3);
		if (_vm->_globals.OLD_ECRAN == 38 && _vm->_globals.ECRAN == 39)
			TEST_FORET(39, 551, 633, 100, 440, 2);
		if (_vm->_globals.OLD_ECRAN == 40 && _vm->_globals.ECRAN == 39)
			TEST_FORET(39, 6, 84, 100, 440, 3);
		if (_vm->_globals.OLD_ECRAN == 39 && _vm->_globals.ECRAN == 40)
			TEST_FORET(40, 133, 252, 100, 440, 4);
		if (_vm->_globals.OLD_ECRAN == 41 && _vm->_globals.ECRAN == 40)
			TEST_FORET(40, 392, 529, 100, 440, 2);
		if (_vm->_globals.OLD_ECRAN == 40 && _vm->_globals.ECRAN == 41)
			TEST_FORET(41, 551, 633, 100, 440, 1);
		if (_vm->_globals.OLD_ECRAN == 17 && _vm->_globals.ECRAN == 41)
			TEST_FORET(41, 6, 84, 100, 440, 3);
	}
	if (_vm->_globals.ECRAN == 5) {
		if (YSPR(0) <= 399) {
			if (!_vm->_globals.SAUVEGARDE->data[svField173]) {
				_vm->_globals.SAUVEGARDE->data[svField173] = 1;
				_vm->_globals.NOPARLE = 1;
				_vm->_talkManager.PARLER_PERSO("flicspe1.pe2");
				_vm->_globals.NOPARLE = 0;
				if (!_vm->_globals.CENSURE) {
					v1 = _vm->_globals.dos_malloc2(0x3E8u);
					memcpy(v1, _vm->_graphicsManager.Palette, 0x301u);
					FileManager::CONSTRUIT_LINUX("TEMP1.SCR");
					FileManager::SAUVE_FICHIER(_vm->_globals.NFICHIER, _vm->_graphicsManager.VESA_SCREEN, 0x4B000u);
					if (!_vm->_graphicsManager.nbrligne)
						_vm->_graphicsManager.ofscroll = 0;
					_vm->_graphicsManager.NB_SCREEN();
					_vm->_soundManager.SPECIAL_SOUND = 198;
					PERSO_ON = 1;
					_vm->_animationManager.NO_SEQ = 1;
					_vm->_animationManager.CLS_ANM = 0;
					_vm->_animationManager.PLAY_ANM("otage.ANM", 1, 24, 500);
					_vm->_animationManager.NO_SEQ = 0;
					_vm->_soundManager.SPECIAL_SOUND = 0;
					_vm->_globals.NECESSAIRE = 1;
					_vm->_graphicsManager.NB_SCREEN();
					_vm->_globals.NECESSAIRE = 0;
					FileManager::CONSTRUIT_LINUX("TEMP1.SCR");
					FileManager::bload(_vm->_globals.NFICHIER, _vm->_graphicsManager.VESA_SCREEN);
					PERSO_ON = 0;
					memcpy(_vm->_graphicsManager.Palette, v1, 0x301u);
					_vm->_graphicsManager.SHOW_PALETTE();
					_vm->_globals.dos_free2(v1);
					_vm->_graphicsManager.DD_Lock();
					if (_vm->_graphicsManager.Winbpp == 2) {
						if (_vm->_graphicsManager.SDL_ECHELLE)
							_vm->_graphicsManager.m_scroll16A(_vm->_graphicsManager.VESA_SCREEN, _vm->_eventsManager.start_x, 0, 640, 480, 0, 0);
						else
							_vm->_graphicsManager.m_scroll16(_vm->_graphicsManager.VESA_SCREEN, _vm->_eventsManager.start_x, 0, 640, 480, 0, 0);
					}
					if (_vm->_graphicsManager.Winbpp == 1) {
						if (_vm->_graphicsManager.SDL_ECHELLE)
							_vm->_graphicsManager.m_scroll2A(_vm->_graphicsManager.VESA_SCREEN, _vm->_eventsManager.start_x, 0, 640, 480, 0, 0);
						else
							_vm->_graphicsManager.m_scroll2(_vm->_graphicsManager.VESA_SCREEN, _vm->_eventsManager.start_x, 0, 640, 480, 0, 0);
					}
					_vm->_graphicsManager.DD_Unlock();
					v2 = _vm->_graphicsManager.VESA_BUFFER;
					v3 = _vm->_graphicsManager.VESA_SCREEN;
					memcpy(_vm->_graphicsManager.VESA_BUFFER, _vm->_graphicsManager.VESA_SCREEN, 0x95FFCu);
					v3 = v3 + 614396;
					v2 = v2 + 614396;
					*v2 = *v3;
					v2 = v2 + 2;
					*v2 = *(v3 + 2);

					_vm->_graphicsManager.no_scroll = 0;
					_vm->_graphicsManager.DD_VBL();
				}
			}
		}
	}
	if (_vm->_globals.ECRAN == 20)
		_vm->_globals.SAUVEGARDE->data[svField132] = XSPR(0) > 65
		                               && XSPR(0) <= 124
		                               && YSPR(0) > 372
		                               && YSPR(0) <= 398;
	if (_vm->_globals.ECRAN == 57) {
		_vm->_globals.DESACTIVE_INVENT = 1;
		if (_vm->_globals.SAUVEGARDE->data[svField261] == 1 && BOBPOSI(5) == 37) {
			BOBANIM_OFF(5);
			SET_BOBPOSI(5, 0);
			BOBANIM_ON(6);
			_vm->_globals.SAUVEGARDE->data[svField261] = 2;
			ZONE_OFF(15);
			_vm->_soundManager.PLAY_SOUND("SOUND75.WAV");
		}
		if (_vm->_globals.SAUVEGARDE->data[svField261] == 2 && BOBPOSI(6) == 6) {
			BOBANIM_OFF(6);
			SET_BOBPOSI(6, 0);
			BOBANIM_ON(7);
			ZONE_ON(14);
			_vm->_globals.SAUVEGARDE->data[svField261] = 3;
		}
		_vm->_globals.DESACTIVE_INVENT = 0;
	}
	if (_vm->_globals.ECRAN == 93 && !_vm->_globals.SAUVEGARDE->data[svField333]) {
		_vm->_globals.DESACTIVE_INVENT = 1;
		do
			_vm->_eventsManager.VBL();
		while (BOBPOSI(8) != 3);
		_vm->_globals.NOPARLE = 1;
		_vm->_talkManager.PARLER_PERSO("GM3.PE2");
		BOBANIM_OFF(8);
		_vm->_globals.SAUVEGARDE->data[svField333] = 1;
		_vm->_globals.DESACTIVE_INVENT = 0;
	}
}

int ObjectsManager::Traduction(byte *a1) {
	int v1; 
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
	int v30; 
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
	char v47; 
	char v48; 
	char v49;
	int v50; 
	Sauvegarde1 *v51; 
	int v52; 
	Sauvegarde1 *v53; 
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
	int v65;
	int v66; 
	int v67; 
	int v68; 
	int v69; 
	int v70;
	int v71; 
	int v72; 
	int v73;
	int v74;
	int v75; 
	int v76; 
	char v77[12]; 
	Common::String s; 
	char v79; 
	char v80;
	char v81;
	char v82[16]; 

	v1 = 0;
	v70 = 0;
	if (*a1 != 'F' || *(a1 + 1) != 'C')
		return 0;
	if (*(a1 + 2) == 'T' && *(a1 + 3) == 'X' && *(a1 + 4) == 'T') {
		v70 = *(a1 + 6);
		v2 = *(a1 + 7);
		v69 = *(a1 + 8);
		v67 = READ_LE_UINT16(a1 + 9);
		v65 = READ_LE_UINT16(a1 + 11);
		v3 = READ_LE_UINT16(a1 + 13);
		v1 = 1;
		if (!TRAVAILOBJET) {
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
				if ((uint16)(v3 - 613) <= 1u || v3 == 134)
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
		if (TRAVAILOBJET == 1) {
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
	if (*(a1 + 2) == 'B' && *(a1 + 3) == 'O' && *(a1 + 4) == 'B') {
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
	if (*(a1 + 2) == 'S') {
		if (*(a1 + 3) == 'T' && *(a1 + 4) == 'P') {
			if (DESACTIVE != 1) {
				DEUXPERSO = 0;
				v5 = *(a1 + 5);
				v6 = *(a1 + 8);
				PERX = READ_LE_UINT16(a1 + 6);
				PERY = v6;
				PERI = v5;
				if (CH_TETE == 1) {
					if (_vm->_globals.SAUVEGARDE->data[svField354] == 1
					        && _vm->_globals.SAUVEGARDE->data[svField180]
					        && _vm->_globals.SAUVEGARDE->data[svField181]
					        && _vm->_globals.SAUVEGARDE->data[svField182]
					        && _vm->_globals.SAUVEGARDE->data[svField183]) {
						PERX = _vm->_globals.SAUVEGARDE->data[svField180];
						PERY = _vm->_globals.SAUVEGARDE->data[svField181];
						PERI = _vm->_globals.SAUVEGARDE->data[svField182];
					}
					if (_vm->_globals.SAUVEGARDE->data[svField356] == 1
					        && _vm->_globals.SAUVEGARDE->data[svField190]
					        && _vm->_globals.SAUVEGARDE->data[svField191]
					        && _vm->_globals.SAUVEGARDE->data[svField192]
					        && _vm->_globals.SAUVEGARDE->data[svField193]) {
						PERX = _vm->_globals.SAUVEGARDE->data[svField190];
						PERY = _vm->_globals.SAUVEGARDE->data[svField191];
						PERI = _vm->_globals.SAUVEGARDE->data[svField192];
					}
					if (_vm->_globals.SAUVEGARDE->data[svField357] == 1
					        && _vm->_globals.SAUVEGARDE->data[svField185]
					        && _vm->_globals.SAUVEGARDE->data[svField186]
					        && _vm->_globals.SAUVEGARDE->data[svField187]
					        && _vm->_globals.SAUVEGARDE->data[svField188]) {
						PERX = _vm->_globals.SAUVEGARDE->data[svField185];
						PERY = _vm->_globals.SAUVEGARDE->data[svField186];
						PERI = _vm->_globals.SAUVEGARDE->data[svField187];
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
			CH_TETE = 0;
		}
		if (*(a1 + 2) == 'S' && *(a1 + 3) == 'T' && *(a1 + 4) == 'E') {
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
	if (*(a1 + 2) == 'B' && *(a1 + 3) == 'O' && *(a1 + 4) == 'F') {
		if (DESACTIVE != 1)
			VBOB_OFF(READ_LE_UINT16(a1 + 5));
		v1 = 1;
	}
	if (*(a1 + 2) == 'P' && *(a1 + 3) == 'E' && *(a1 + 4) == 'R') {
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
	if (*(a1 + 2) == 'M' && *(a1 + 3) == 'U' && *(a1 + 4) == 'S')
		v1 = 1;
	if (*(a1 + 2) == 'W' && *(a1 + 3) == 'A' && *(a1 + 4) == 'I') {
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
	if (*(a1 + 2) == 'O') {
		if (*(a1 + 3) == 'B' && *(a1 + 4) == 'P') {
			v1 = 1;
			AJOUTE_OBJET(READ_LE_UINT16(a1 + 5));
		}
		if (*(a1 + 2) == 'O' && *(a1 + 3) == 'B' && *(a1 + 4) == 'M') {
			v1 = 1;
			DELETE_OBJET(READ_LE_UINT16(a1 + 5));
		}
	}
	if (*(a1 + 2) == 'G' && *(a1 + 3) == 'O' && *(a1 + 4) == 'T')
		v1 = 2;
	if (*(a1 + 2) == 'Z') {
		if (*(a1 + 3) == 'O' && *(a1 + 4) == 'N') {
			ZONE_ON(READ_LE_UINT16(a1 + 5));
			v1 = 1;
		}
		if (*(a1 + 2) == 'Z' && *(a1 + 3) == 'O' && *(a1 + 4) == 'F') {
			ZONE_OFF(READ_LE_UINT16(a1 + 5));
			v1 = 1;
		}
	}
	if (*(a1 + 2) == 'E' && *(a1 + 3) == 'X' && *(a1 + 4) == 'I')
		v1 = 5;
	if (*(a1 + 2) == 'S' && *(a1 + 3) == 'O' && *(a1 + 4) == 'R') {
		_vm->_globals.SORTIE = READ_LE_UINT16(a1 + 5);
		v1 = 5;
	}
	if (*(a1 + 2) == 'B' && *(a1 + 3) == 'C' && *(a1 + 4) == 'A') {
		_vm->_globals.CACHE_OFF(READ_LE_UINT16(a1 + 5));
		v1 = 1;
	}
	if (*(a1 + 2) == 'A' && *(a1 + 3) == 'N' && *(a1 + 4) == 'I') {
		v75 = READ_LE_UINT16(a1 + 5);
		if (v75 <= 100)
			BOBANIM_ON(v75);
		else
			BOBANIM_OFF(v75 - 100);
		v1 = 1;
	}
	if (*(a1 + 2) == 'S' && *(a1 + 3) == 'P' && *(a1 + 4) == 'E') {
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
			BOBANIM_OFF(2);
			BOBANIM_OFF(3);
			BOBANIM_OFF(4);
			BOBANIM_OFF(6);
			BOBANIM_OFF(11);
			BOBANIM_OFF(10);
		}
		if (v76 == 609) {
			BOBANIM_ON(2);
			BOBANIM_ON(3);
			BOBANIM_ON(4);
			BOBANIM_ON(6);
			BOBANIM_ON(11);
			BOBANIM_ON(10);
		}
		if (v76 == 611) {
			BOBANIM_ON(5);
			BOBANIM_ON(7);
			BOBANIM_ON(8);
			BOBANIM_ON(9);
			BOBANIM_ON(12);
			BOBANIM_ON(13);
		}
		if (v76 == 610) {
			BOBANIM_OFF(5);
			BOBANIM_OFF(7);
			BOBANIM_OFF(8);
			BOBANIM_OFF(9);
			BOBANIM_OFF(12);
			BOBANIM_OFF(13);
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
			_vm->_animationManager.PLAY_SEQ2("HELICO.SEQ", 10, 4, 10);
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
			while (BOBPOSI(3) != 100);
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
			_vm->_animationManager.PLAY_SEQ2("ASSOM.SEQ", 10, 4, 500);
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
			_vm->_animationManager.PLAY_SEQ2("corde.SEQ", 32, 32, 100);
			_vm->_graphicsManager.NOFADE = 1;
		}
		if (v76 == 38) {
			_vm->_soundManager.CHARGE_SAMPLE(1, "SOUND44.WAV");
			_vm->_soundManager.CHARGE_SAMPLE(2, "SOUND42.WAV");
			_vm->_soundManager.CHARGE_SAMPLE(3, "SOUND41.WAV");
			_vm->_soundManager.SPECIAL_SOUND = 17;
			_vm->_animationManager.PLAY_SEQ(v1, "grenade.SEQ", 1, 32, 100);
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
			_vm->_talkManager.OBJET_VIVANT("TELEP.pe2");
			_vm->_globals.DESACTIVE_INVENT = 0;
		}
		if (v76 == 22)
			_vm->_talkManager.OBJET_VIVANT("CADAVRE1.pe2");
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
			_vm->_globals.chemin = _vm->_linesManager.PARCOURS2(v14, v13, 564, 420);
			NUMZONE = -1;
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
				if (BOBPOSI(9) == 4 && !v15) {
					_vm->_soundManager.PLAY_SAMPLE2(1);
					v15 = 1;
				}
				if (BOBPOSI(9) == 5)
					v15 = 0;
				if (BOBPOSI(9) == 16 && !v15) {
					_vm->_soundManager.PLAY_SAMPLE2(1);
					v15 = 1;
				}
				if (BOBPOSI(9) == 17)
					v15 = 0;
				if (BOBPOSI(9) == 28 && !v15) {
					_vm->_soundManager.PLAY_SAMPLE2(1);
					v15 = 1;
				}
				if (BOBPOSI(9) == 29)
					v15 = 0;
				if (BOBPOSI(10) == 10 && !v15) {
					_vm->_soundManager.PLAY_SAMPLE2(2);
					v15 = 1;
				}
				if (BOBPOSI(10) == 11)
					v15 = 0;
				if (BOBPOSI(10) == 22 && !v15) {
					_vm->_soundManager.PLAY_SAMPLE2(2);
					v15 = 1;
				}
				if (BOBPOSI(10) == 23)
					v15 = 0;
				if (BOBPOSI(10) == 33 && !v15) {
					_vm->_soundManager.PLAY_SAMPLE2(2);
					v15 = 1;
				}
				if (BOBPOSI(10) == 34)
					v15 = 0;
				if (BOBPOSI(10) == 12)
					_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 513, 249, 1);
				if (BOBPOSI(10) == 23)
					_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 513, 249, 2);
				if (BOBPOSI(10) == 34)
					_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 513, 249, 3);
				_vm->_eventsManager.VBL();
			} while (BOBPOSI(9) != 36);
			SPRITE_ON(0);
			BOBANIM_OFF(9);
			BOBANIM_OFF(10);
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
			_vm->_globals.chemin = _vm->_linesManager.PARCOURS2(v17, v16, 445, 332);
			_vm->_globals.NOT_VERIF = 1;
			do {
				GOHOME();
				_vm->_eventsManager.VBL();
			} while (_vm->_globals.chemin != PTRNUL);
			SPRITE_OFF(0);
			BOBANIM_ON(7);
			SET_BOBPOSI(7, 0);
			v18 = 0;
			_vm->_soundManager.CHARGE_SAMPLE(1, "SOUND40.WAV");
			do {
				if (BOBPOSI(7) == 10 && !v18) {
					_vm->_soundManager.PLAY_SAMPLE2(1);
					v18 = 1;
				}
				if (BOBPOSI(7) == 11)
					v18 = 0;
				if (BOBPOSI(7) == 18 && !v18) {
					_vm->_soundManager.PLAY_SAMPLE2(1);
					v18 = 1;
				}
				if (BOBPOSI(7) == 19)
					v18 = 0;
				if (BOBPOSI(7) == 19)
					BOBANIM_ON(3);
				_vm->_eventsManager.VBL();
			} while (BOBPOSI(3) != 48);
			_vm->_soundManager.DEL_SAMPLE(1);
			SETANISPR(0, 62);
			SPRITE_ON(0);
			BOBANIM_ON(6);
			BOBANIM_OFF(7);
			BOBANIM_OFF(3);
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
				if (BOBPOSI(9) == 4 && !v20) {
					_vm->_soundManager.PLAY_SOUND2("SOUND44.WAV");
					v20 = 1;
				}
				if (BOBPOSI(9) == 5)
					v20 = 0;
				if (BOBPOSI(9) == 18 && !v20) {
					_vm->_soundManager.PLAY_SOUND2("SOUND46.WAV");
					v20 = 1;
				}
				if (BOBPOSI(9) == 19)
					v20 = 0;
				if (BOBPOSI(10) == 11 && !v20) {
					_vm->_soundManager.PLAY_SOUND2("SOUND45.WAV");
					v20 = 1;
				}
				if (BOBPOSI(10) == 12)
					v20 = 0;
				_vm->_eventsManager.VBL();
			} while (BOBPOSI(9) != v19);
			if (v19 == 12) {
				SPRITE_ON(0);
				BOBANIM_OFF(9);
			}
			_vm->_globals.CACHE_ON();
		}
		if (v76 == 80) {
			SPRITE_OFF(0);
			BOBANIM_ON(12);
			BOBANIM_ON(13);
			SET_BOBPOSI(12, 0);
			SET_BOBPOSI(13, 0);
			v21 = 0;
			_vm->_soundManager.LOAD_WAV("SOUND44.WAV", 1);
			_vm->_soundManager.LOAD_WAV("SOUND71.WAV", 2);
			do {
				if (BOBPOSI(12) == 4 && !v21) {
					_vm->_soundManager._vm->_soundManager.PLAY_WAV(1);
					v21 = 1;
				}
				if (BOBPOSI(12) == 5)
					v21 = 0;
				if (BOBPOSI(4) == 5 && !v21) {
					_vm->_soundManager._vm->_soundManager.PLAY_WAV(2);
					v21 = 1;
				}
				if (BOBPOSI(4) == 6)
					v21 = 0;
				if (BOBPOSI(13) == 8) {
					BOBANIM_OFF(13);
					BOBANIM_OFF(3);
					BOBANIM_ON(4);
					SET_BOBPOSI(4, 0);
					SET_BOBPOSI(13, 0);
				}
				_vm->_eventsManager.VBL();
			} while (BOBPOSI(4) != 16);
			BOBANIM_OFF(12);
			BOBANIM_OFF(4);
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
			_vm->_globals.chemin = _vm->_linesManager.PARCOURS2(v23, v22, 119, 268);
			_vm->_globals.NOT_VERIF = 1;
			do {
				GOHOME();
				_vm->_eventsManager.VBL();
			} while (_vm->_globals.chemin != PTRNUL);
			SPRITE_OFF(0);
			BOBANIM_ON(11);
			BOBANIM_ON(8);
			SET_BOBPOSI(11, 0);
			SET_BOBPOSI(8, 0);
			_vm->_soundManager.LOAD_WAV("SOUND44.WAV", 1);
			_vm->_soundManager.LOAD_WAV("SOUND48.WAV", 2);
			_vm->_soundManager.LOAD_WAV("SOUND49.WAV", 3);
			v24 = 0;
			do {
				if (BOBPOSI(11) == 4 && !v24) {
					_vm->_soundManager._vm->_soundManager.PLAY_WAV(1);
					v24 = 1;
				}
				if (BOBPOSI(11) == 5)
					v24 = 0;
				if (BOBPOSI(8) == 11 && !v24) {
					_vm->_soundManager._vm->_soundManager.PLAY_WAV(2);
					v24 = 1;
				}
				if (BOBPOSI(8) == 12)
					v24 = 0;
				_vm->_eventsManager.VBL();
			} while (BOBPOSI(8) != 32);
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 201, 14, 1);
			SPRITE_ON(0);
			BOBANIM_OFF(11);
			BOBANIM_OFF(8);
			BOBANIM_ON(5);
			BOBANIM_ON(6);
			SET_BOBPOSI(5, 0);
			SET_BOBPOSI(6, 0);
			_vm->_soundManager._vm->_soundManager.PLAY_WAV(3);
			do
				_vm->_eventsManager.VBL();
			while (BOBPOSI(5) != 74);
			BOBANIM_OFF(5);
			BOBANIM_OFF(6);
			BOBANIM_ON(9);
			BOBANIM_ON(7);
		}
		if (v76 == 95) {
			BOBANIM_ON(9);
			BOBANIM_ON(10);
			BOBANIM_ON(12);
			SET_BOBPOSI(9, 0);
			SET_BOBPOSI(10, 0);
			SET_BOBPOSI(12, 0);
			SPRITE_OFF(0);
			do
				_vm->_eventsManager.VBL();
			while (BOBPOSI(9) != 15);
			BOBANIM_OFF(9);
			SPRITE_ON(0);
			_vm->_soundManager.PLAY_SOUND("SOUND50.WAV");
			do
				_vm->_eventsManager.VBL();
			while (BOBPOSI(12) != 117);
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 830, 122, 0);
			BOBANIM_OFF(12);
			BOBANIM_OFF(10);
			BOBANIM_ON(11);
		}
		if (v76 == 85) {
			BOBANIM_OFF(3);
			BOBANIM_ON(5);
			SET_BOBPOSI(5, 0);
			do
				_vm->_eventsManager.VBL();
			while (BOBPOSI(5) != 6);
			BOBANIM_OFF(5);
			BOBANIM_ON(6);
			OBSSEUL = 1;
			INILINK("IM24a");
			OBSSEUL = 0;
		}
		if (v76 == 88) {
			if (_vm->_globals.SAUVEGARDE->data[svField183] == 1) {
				SET_BOBPOSI(1, 0);
				SET_BOBPOSI(2, 0);
				BOBANIM_ON(1);
				BOBANIM_ON(2);
				_vm->_soundManager.CHARGE_SAMPLE(1, "SOUND40.WAV");
				v25 = 0;
				do {
					if (BOBPOSI(1) == 1 && !v25) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v25 = 1;
					}
					if (BOBPOSI(1) == 2)
						v25 = 0;
					if (BOBPOSI(1) == 3 && !v25) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v25 = 1;
					}
					if (BOBPOSI(1) == 4)
						v25 = 0;
					if (BOBPOSI(1) == 5 && !v25) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v25 = 1;
					}
					if (BOBPOSI(1) == 6)
						v25 = 0;
					if (BOBPOSI(1) == 7 && !v25) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v25 = 1;
					}
					if (BOBPOSI(1) == 8)
						v25 = 0;
					_vm->_eventsManager.VBL();
				} while (BOBPOSI(1) != 9);
				BOBANIM_OFF(1);
				BOBANIM_OFF(2);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 283, 160, 6);
				_vm->_soundManager.DEL_SAMPLE(1);
			}
			if (_vm->_globals.SAUVEGARDE->data[svField183] == 2) {
				SET_BOBPOSI(1, 0);
				SET_BOBPOSI(3, 0);
				BOBANIM_ON(1);
				BOBANIM_ON(3);
				_vm->_soundManager.CHARGE_SAMPLE(1, "SOUND40.WAV");
				v26 = 0;
				do {
					if (BOBPOSI(1) == 1 && !v26) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v26 = 1;
					}
					if (BOBPOSI(1) == 2)
						v26 = 0;
					if (BOBPOSI(1) == 3 && !v26) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v26 = 1;
					}
					if (BOBPOSI(1) == 4)
						v26 = 0;
					if (BOBPOSI(1) == 5 && !v26) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v26 = 1;
					}
					if (BOBPOSI(1) == 6)
						v26 = 0;
					if (BOBPOSI(1) == 7 && !v26) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v26 = 1;
					}
					if (BOBPOSI(1) == 8)
						v26 = 0;
					_vm->_eventsManager.VBL();
				} while (BOBPOSI(1) != 9);
				BOBANIM_OFF(1);
				BOBANIM_OFF(3);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 283, 161, 8);
				_vm->_soundManager.DEL_SAMPLE(1);
			}
		}
		if (v76 == 90) {
			_vm->_soundManager.PLAY_SOUND("SOUND52.WAV");
			if (!_vm->_globals.SAUVEGARDE->data[svField186]) {
				_vm->_animationManager.PLAY_SEQ(v1, "CIB5A.SEQ", 1, 12, 1);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 155, 29, 0);
			}
			if (_vm->_globals.SAUVEGARDE->data[svField186] == 1) {
				_vm->_animationManager.PLAY_SEQ(v1, "CIB5C.SEQ", 1, 12, 1);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 155, 29, 0);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 329, 87, 2);
			}
		}
		if (v76 == 91) {
			_vm->_soundManager.PLAY_SOUND("SOUND52.WAV");
			if (!_vm->_globals.SAUVEGARDE->data[svField186]) {
				_vm->_animationManager.PLAY_SEQ(v1, "CIB5B.SEQ", 1, 12, 1);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 155, 29, 5);
			}
			if (_vm->_globals.SAUVEGARDE->data[svField186] == 1) {
				_vm->_animationManager.PLAY_SEQ(v1, "CIB5D.SEQ", 1, 12, 1);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 155, 29, 5);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 283, 160, 6);
			}
		}
		if (v76 == 92) {
			_vm->_soundManager.PLAY_SOUND("SOUND52.WAV");
			if (!_vm->_globals.SAUVEGARDE->data[svField184]) {
				_vm->_animationManager.PLAY_SEQ(v1, "CIB6A.SEQ", 1, 12, 1);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 155, 29, 0);
			}
			if (_vm->_globals.SAUVEGARDE->data[svField184] == 1) {
				_vm->_animationManager.PLAY_SEQ(v1, "CIB6C.SEQ", 1, 12, 1);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 155, 29, 0);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 293, 139, 3);
			}
		}
		if (v76 == 93) {
			_vm->_soundManager.PLAY_SOUND("SOUND52.WAV");
			if (!_vm->_globals.SAUVEGARDE->data[svField184]) {
				_vm->_animationManager.PLAY_SEQ(v1, "CIB6B.SEQ", 1, 12, 1);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 155, 29, 5);
			}
			if (_vm->_globals.SAUVEGARDE->data[svField184] == 1) {
				_vm->_animationManager.PLAY_SEQ(v1, "CIB6D.SEQ", 1, 12, 1);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 155, 29, 5);
				_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 283, 161, 8);
			}
		}
		if (v76 == 62)
			_vm->_talkManager.OBJET_VIVANT("SBCADA.pe2");
		if (v76 == 65)
			_vm->_talkManager.OBJET_VIVANT("ScCADA.pe2");
		if (v76 == 105) {
			_vm->_globals.NOT_VERIF = 1;
			g_old_x = XSPR(0);
			g_old_sens = -1;
			_vm->_globals.Compteur = 0;
			_vm->_globals.chemin = PTRNUL;
			if (_vm->_globals.SAUVEGARDE->data[svField253] == 1) {
				v27 = YSPR(0);
				v28 = XSPR(0);
				_vm->_globals.chemin = _vm->_linesManager.PARCOURS2(v28, v27, 201, 294);
			}
			if (_vm->_globals.SAUVEGARDE->data[svField253] == 2) {
				v29 = YSPR(0);
				v30 = XSPR(0);
				_vm->_globals.chemin = _vm->_linesManager.PARCOURS2(v30, v29, 158, 338);
			}
			if (_vm->_globals.SAUVEGARDE->data[svField253] > 2) {
				v31 = YSPR(0);
				v32 = XSPR(0);
				_vm->_globals.chemin = _vm->_linesManager.PARCOURS2(v32, v31, 211, 393);
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
				BOBANIM_ON(4);
				v33 = 0;
				do {
					if (BOBPOSI(4) == 9 && !v33) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v33 = 1;
					}
					if (BOBPOSI(4) == 10)
						v33 = 0;
					if (BOBPOSI(4) == 32 && !v33) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v33 = 1;
					}
					if (BOBPOSI(4) == 33)
						v33 = 0;
					if (BOBPOSI(4) == 55 && !v33) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v33 = 1;
					}
					if (BOBPOSI(4) == 56)
						v33 = 0;
					_vm->_eventsManager.VBL();
				} while (BOBPOSI(4) != 72);
				BOBANIM_OFF(4);
			}
			if (_vm->_globals.SAUVEGARDE->data[svField253] == 1) {
				BOBANIM_ON(6);
				v34 = 0;
				do {
					if (BOBPOSI(6) == 9 && !v34) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v34 = 1;
					}
					if (BOBPOSI(6) == 10)
						v34 = 0;
					if (BOBPOSI(6) == 32 && !v34) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v34 = 1;
					}
					if (BOBPOSI(6) == 33)
						v34 = 0;
					if (BOBPOSI(6) == 55 && !v34) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v34 = 1;
					}
					if (BOBPOSI(6) == 56)
						v34 = 0;
					_vm->_eventsManager.VBL();
				} while (BOBPOSI(6) != 72);
				BOBANIM_OFF(6);
			}
			if (_vm->_globals.SAUVEGARDE->data[svField253] == 2) {
				BOBANIM_ON(5);
				v35 = 0;
				do {
					if (BOBPOSI(5) == 9 && !v35) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v35 = 1;
					}
					if (BOBPOSI(5) == 10)
						v35 = 0;
					if (BOBPOSI(5) == 32 && !v35) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v35 = 1;
					}
					if (BOBPOSI(5) == 33)
						v35 = 0;
					if (BOBPOSI(5) == 55 && !v35) {
						_vm->_soundManager.PLAY_SAMPLE2(1);
						v35 = 1;
					}
					if (BOBPOSI(5) == 56)
						v35 = 0;
					_vm->_eventsManager.VBL();
				} while (BOBPOSI(5) != 72);
				BOBANIM_OFF(5);
			}
			SPRITE_ON(0);
			ACTION_DOS(1);
			_vm->_soundManager.DEL_SAMPLE(1);
		}
		if (v76 == 106) {
			SPRITE_OFF(0);
			BOBANIM_ON(4);
			SET_BOBPOSI(4, 0);
			_vm->_soundManager.LOAD_WAV("SOUND61.WAV", 1);
			_vm->_soundManager.LOAD_WAV("SOUND62.WAV", 2);
			_vm->_soundManager.LOAD_WAV("SOUND61.WAV", 3);
			do
				_vm->_eventsManager.VBL();
			while (BOBPOSI(4) != 10);
			_vm->_soundManager._vm->_soundManager.PLAY_WAV(1);
			do
				_vm->_eventsManager.VBL();
			while (BOBPOSI(4) != 18);
			_vm->_soundManager._vm->_soundManager.PLAY_WAV(2);
			do
				_vm->_eventsManager.VBL();
			while (BOBPOSI(4) != 62);
			_vm->_soundManager._vm->_soundManager.PLAY_WAV(3);
			do
				_vm->_eventsManager.VBL();
			while (BOBPOSI(4) != 77);
			BOBANIM_OFF(4);
			SPRITE_ON(0);
		}
		if (v76 == 107) {
			SPRITE_OFF(0);
			BOBANIM_ON(5);
			SET_BOBPOSI(5, 0);
			_vm->_soundManager.LOAD_WAV("SOUND61.WAV", 1);
			_vm->_soundManager.LOAD_WAV("SOUND62.WAV", 2);
			_vm->_soundManager.LOAD_WAV("SOUND61.WAV", 3);
			do
				_vm->_eventsManager.VBL();
			while (BOBPOSI(5) != 10);
			_vm->_soundManager._vm->_soundManager.PLAY_WAV(1);
			do
				_vm->_eventsManager.VBL();
			while (BOBPOSI(5) != 18);
			_vm->_soundManager._vm->_soundManager.PLAY_WAV(2);
			do
				_vm->_eventsManager.VBL();
			while (BOBPOSI(5) != 38);
			_vm->_soundManager._vm->_soundManager.PLAY_WAV(3);
			do
				_vm->_eventsManager.VBL();
			while (BOBPOSI(5) != 53);
			BOBANIM_OFF(5);
			SPRITE_ON(0);
		}
		if (v76 == 210) {
			_vm->_animationManager.NO_SEQ = 1;
			_vm->_soundManager.SPECIAL_SOUND = 210;
			_vm->_animationManager.PLAY_SEQ2("SECRET1.SEQ", 1, 12, 1);
			_vm->_soundManager.SPECIAL_SOUND = 0;
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 192, 152, 0);
			BOBANIM_ON(9);
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
			_vm->_animationManager.PLAY_SEQ(v1, "SECRET2.SEQ", 1, 12, 100);
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
			_vm->_talkManager.OBJET_VIVANT("PANNEAU.PE2");
		if (v76 == 208) {
			_vm->_globals.DESACTIVE_INVENT = 1;
			if (_vm->_globals.SAUVEGARDE->data[svField6] != _vm->_globals.SAUVEGARDE->data[svField401]) {
				_vm->_soundManager.SPECIAL_SOUND = 208;
				_vm->_eventsManager.NOESC = true;
				_vm->_animationManager.PLAY_SEQ(v1, "SORT.SEQ", 10, 4, 10);
				_vm->_eventsManager.NOESC = false;
				_vm->_soundManager.SPECIAL_SOUND = 0;
			}
			_vm->_globals.NOT_VERIF = 1;
			_vm->_globals.chemin = PTRNUL;
			v37 = YSPR(0);
			v38 = XSPR(0);
			_vm->_globals.chemin = _vm->_linesManager.PARCOURS2(v38, v37, 330, 418);
			_vm->_globals.NOT_VERIF = 1;
			NUMZONE = 0;
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
			BOBANIM_OFF(4);
			BOBANIM_ON(1);
			do
				_vm->_eventsManager.VBL();
			while (BOBPOSI(1) != 9);
			BOBANIM_OFF(1);
			_vm->_globals.NO_VISU = 1;
			_vm->_globals.chemin = PTRNUL;
			_vm->_globals.NOT_VERIF = 1;
			v39 = YSPR(0);
			v40 = XSPR(0);
			_vm->_globals.chemin = _vm->_linesManager.PARCOURS2(v40, v39, 330, 314);
			NUMZONE = 0;
			_vm->_globals.NOT_VERIF = 1;
			do {
				GOHOME();
				_vm->_eventsManager.VBL();
			} while (_vm->_globals.chemin != PTRNUL);
			SETANISPR(0, 64);
			BOBANIM_ON(2);
			_vm->_soundManager.PLAY_SOUND("SOUND66.WAV");
			do
				_vm->_eventsManager.VBL();
			while (BOBPOSI(2) != 10);
			BOBANIM_OFF(2);
			BOBANIM_ON(4);
		}
		if (v76 == 201) {
			BOBANIM_ON(3);
			do
				_vm->_eventsManager.VBL();
			while (BOBPOSI(3) != 18);
			BOBANIM_OFF(3);
			BOBANIM_ON(4);
		}
		if (v76 == 203) {
			_vm->_globals.NO_VISU = 1;
			SPRITE_OFF(0);
			BOBANIM_ON(4);
			do {
				_vm->_eventsManager.VBL();
				if (BOBPOSI(4) == 18)
					_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 18, 334, 0);
			} while (BOBPOSI(4) != 26);
			BOBANIM_OFF(4);
			_vm->_globals.NO_VISU = 0;
			SPRITE_ON(0);
		}
		if (v76 == 204) {
			SPRITE_OFF(0);
			BOBANIM_ON(3);
			_vm->_soundManager.LOAD_WAV("SOUND67.WAV", 1);
			v41 = 0;
			do {
				if (BOBPOSI(3) == 10 && !v41) {
					_vm->_soundManager._vm->_soundManager.PLAY_WAV(1);
					v41 = 1;
				}
				if (BOBPOSI(3) == 11)
					v41 = 0;
				_vm->_eventsManager.VBL();
			} while (BOBPOSI(3) != 50);
			BOBANIM_OFF(3);
			SPRITE_ON(0);
		}
		if (v76 == 205) {
			SPRITE_OFF(0);
			BOBANIM_ON(4);
			_vm->_soundManager.LOAD_WAV("SOUND69.WAV", 1);
			v42 = 0;
			do {
				if (BOBPOSI(4) == 10 && !v42) {
					_vm->_soundManager.PLAY_WAV(1);
					v42 = 1;
				}
				if (BOBPOSI(4) == 11)
					v42 = 0;
				_vm->_eventsManager.VBL();
			} while (BOBPOSI(4) != 24);
			BOBANIM_OFF(4);
			SPRITE_ON(0);
		}
		if (v76 == 175) {
			SETANISPR(0, 55);
			SPRITE_OFF(0);
			BOBANIM_ON(9);
			BOBANIM_ON(10);
			BOB_OFFSET(10, 300);
			_vm->_soundManager.PLAY_SOUND("SOUND44.WAV");
			do
				_vm->_eventsManager.VBL();
			while (BOBPOSI(10) != 7);
			BOBANIM_ON(6);
			BOBANIM_OFF(3);
			do
				_vm->_eventsManager.VBL();
			while (BOBPOSI(6) != 10);
			_vm->_soundManager.PLAY_SOUND("SOUND71.WAV");
			BOBANIM_ON(7);
			BOBANIM_OFF(4);
			do
				_vm->_eventsManager.VBL();
			while (BOBPOSI(7) != 15);
			BOBANIM_OFF(5);
			BOBANIM_ON(8);
			_vm->_soundManager.PLAY_SOUND("SOUND70.WAV");
			do
				_vm->_eventsManager.VBL();
			while (BOBPOSI(8) != 76);
			BOBANIM_OFF(6);
			BOBANIM_OFF(7);
			BOBANIM_OFF(8);
			BOBANIM_OFF(9);
			BOBANIM_OFF(10);
			SPRITE_ON(0);
		}
		if (v76 == 229) {
			_vm->_soundManager.SPECIAL_SOUND = 229;
			_vm->_animationManager.PLAY_SEQ(v1, "MUR.SEQ", 1, 12, 1);
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
			_vm->_globals.chemin = _vm->_linesManager.PARCOURS2(v44, v43, 488, 280);
			_vm->_globals.NOT_VERIF = 1;
			do {
				GOHOME();
				_vm->_eventsManager.VBL();
			} while (_vm->_globals.chemin != PTRNUL);
			SPRITE_OFF(0);
			v45 = 0;
			BOBANIM_ON(7);
			do {
				if (BOBPOSI(7) == 9 && !v45) {
					v45 = 1;
					_vm->_soundManager.PLAY_SOUND("SOUND81.WAV");
				}
				_vm->_eventsManager.VBL();
			} while (BOBPOSI(7) != 15);
			BOBANIM_OFF(7);
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
			BOBANIM_ON(12);
			do
				_vm->_eventsManager.VBL();
			while (BOBPOSI(12) != 6);
			_vm->_globals.NOPARLE = 1;
			_vm->_talkManager.PARLER_PERSO("PRMORT.pe2");
			_vm->_globals.NOPARLE = 0;
			do
				_vm->_eventsManager.VBL();
			while (BOBPOSI(12) != 12);
			SPRITE_ON(0);
			BOBANIM_OFF(12);
			_vm->_globals.CACHE_ON();
		}
		if (v76 == 233) {
			_vm->_globals.CACHE_OFF();
			SPRITE_OFF(0);
			BOBANIM_ON(11);
			v46 = 0;
			do {
				_vm->_eventsManager.VBL();
				if (BOBPOSI(11) == 10 && !v46)
					v46 = 1;
			} while (BOBPOSI(11) != 13);
			BOBANIM_OFF(11);
			_vm->_globals.CACHE_ON();
			BOBANIM_ON(13);
			do
				_vm->_eventsManager.VBL();
			while (BOBPOSI(13) != 48);
			_vm->_globals.NOPARLE = 1;
			_vm->_talkManager.PARLER_PERSO("HRADIO.PE2");
			_vm->_globals.NOPARLE = 0;
			_vm->_graphicsManager.FADE_OUTW();
			BOBANIM_OFF(13);
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
			BOBANIM_ON(1);
			v50 = 0;
			do {
				_vm->_eventsManager.VBL();
				if (BOBPOSI(1) == 12 && !v50) {
					_vm->_soundManager._vm->_soundManager.PLAY_SOUND("SOUND86.WAV");
					v50 = 1;
				}
				if (BOBPOSI(1) == 13)
					v50 = 0;
				if (BOBPOSI(1) == 25 && !v50) {
					_vm->_soundManager._vm->_soundManager.PLAY_SOUND("SOUND85.WAV");
					v50 = 1;
				}
				if (BOBPOSI(1) == 25)
					v50 = 0;
			} while (BOBPOSI(1) != 32);
			BOBANIM_OFF(1);
			BOBANIM_ON(2);
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
				if (BOBPOSI(12) == 5 && !v52) {
					_vm->_soundManager.PLAY_WAV(1);
					v52 = 1;
				}
				_vm->_eventsManager.VBL();
			} while (BOBPOSI(12) != 34);
			BOBANIM_OFF(2);
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
				_vm->_animationManager.PLAY_SEQ(v1, "RESU.SEQ", 2, 24, 2);
				_vm->_animationManager.NO_SEQ = 0;
			} else {
				OPTI_ONE(7, 0, 14, 0);
			}
		}
		if (v76 == 242) {
			_vm->_soundManager.PLAY_SOUND("SOUND87.WAV");
			_vm->_animationManager.NO_SEQ = 1;
			_vm->_animationManager.PLAY_SEQ(v1, "RESUF.SEQ", 1, 24, 1);
			_vm->_animationManager.NO_SEQ = 0;
			v53 = &_vm->_globals.SAUVEGARDE->field380;
			v53->field0 = 404;
			v53->field1 = 395;
			v53->field2 = 64;
			v53->field3 = _vm->_globals.ECRAN;

			v54 = _vm->_globals.STAILLE[790 / 2];
			if (_vm->_globals.STAILLE[790 / 2] < 0)
				v54 = -_vm->_globals.STAILLE[790 / 2];
			v76 = -(100 * (67 - (100 - v54)) / 67);
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
			_vm->_globals.ZONEP[4].field0 = 276;
			VERBE_ON(4, 19);
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_globals.SPRITE_ECRAN, 285, 379, 0);
			_vm->_globals.SAUVEGARDE->data[svField399] = 1;
		}
		if (v76 == 246) {
			SPRITE_OFF(0);
			OPTI_ONE(6, 0, 15, 0);
			PERSO_ON = 1;
			_vm->_graphicsManager.NB_SCREEN();
			_vm->_animationManager.NO_SEQ = 1;
			_vm->_animationManager.PLAY_SEQ2("TUNNEL.SEQ", 1, 18, 20);
			_vm->_animationManager.NO_SEQ = 0;
			_vm->_graphicsManager.NOFADE = 1;
			_vm->_graphicsManager.FADE_OUTW();
			PERSO_ON = 0;
			_vm->_globals.SORTIE = 100;
		}
		if (v76 == 55) {
			BOBANIM_OFF(1);
			OPTI_ONE(15, 0, 12, 0);
			BOBANIM_OFF(15);
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
			_vm->_globals.chemin = _vm->_linesManager.PARCOURS2(v56, v55, 361, 325);
			_vm->_globals.NOT_VERIF = 1;
			NUMZONE = -1;
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
			_vm->_globals.chemin = _vm->_linesManager.PARCOURS2(v58, v57, 361, 325);
			_vm->_globals.NOT_VERIF = 1;
			NUMZONE = -1;
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

void ObjectsManager::BOB_VIVANT(int idx) {
	int v1;
	int v2;
	int v3; 
	int v4; 

	v1 = 5 * idx;
	v2 = READ_LE_UINT16(_vm->_talkManager.ADR_ANIM + 2 * v1);
	v3 = READ_LE_UINT16(_vm->_talkManager.ADR_ANIM + 2 * v1 + 2);
	v4 = *(_vm->_talkManager.ADR_ANIM + 2 * v1 + 8);
	if (READ_LE_UINT16(_vm->_talkManager.ADR_ANIM + 2 * v1 + 4)) {
		if (!_vm->_globals.NO_OFFSET)
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_talkManager.PERSOSPR, 
				_vm->_graphicsManager.ofscroll + v2, v3, 
				*(_vm->_talkManager.ADR_ANIM + 2 * v1 + 8));
		if (_vm->_globals.NO_OFFSET)
			_vm->_graphicsManager.AFFICHE_SPEED(_vm->_talkManager.PERSOSPR, v2, v3, v4);
	}
}

void ObjectsManager::VBOB(byte *a1, int idx, int a3, int a4, int a5) {
	if (idx > 29)
		error("MAX_VBOB exceeded");
	
	if (_vm->_globals.VBob[idx].field4 <= 1u) {
		_vm->_globals.VBob[idx].field4 = 1;
		_vm->_globals.VBob[idx].field6 = a3;
		_vm->_globals.VBob[idx].field8 = a4;
		_vm->_globals.VBob[idx].fieldA = a5;
		_vm->_globals.VBob[idx].field14 = a3;
		_vm->_globals.VBob[idx].field16 = a4;
		_vm->_globals.VBob[idx].field18 = a5;
		_vm->_globals.VBob[idx].field0 = a1;
		_vm->_globals.VBob[idx].field1C = a1;
		if (_vm->_globals.VBob[idx].field10 != PTRNUL)
			_vm->_globals.VBob[idx].field10 = _vm->_globals.dos_free2(_vm->_globals.VBob[idx].field10);
	}
	
	int f4 = _vm->_globals.VBob[idx].field4;
	if (f4 == 2 || f4 == 4) {
		_vm->_globals.VBob[idx].field4 = 3;
		_vm->_globals.VBob[idx].field14 = _vm->_globals.VBob[idx].field6;
		_vm->_globals.VBob[idx].field16 = _vm->_globals.VBob[idx].field8;
		_vm->_globals.VBob[idx].field1C = _vm->_globals.VBob[idx].field0;
		_vm->_globals.VBob[idx].field18 = _vm->_globals.VBob[idx].fieldA;
		_vm->_globals.VBob[idx].field6 = a3;
		_vm->_globals.VBob[idx].field8 = a4;
		_vm->_globals.VBob[idx].fieldA = a5;
		_vm->_globals.VBob[idx].field0 = a1;
	}
}

void ObjectsManager::VBOB_OFF(int idx) {
	if (idx > 29)
		error("MAX_VBOB exceeded");
	
	if (_vm->_globals.VBob[idx].field4 <= 1)
		_vm->_globals.VBob[idx].field4 = 0;
	else
		_vm->_globals.VBob[idx].field4 = 4;
}

void ObjectsManager::ACTION_DOS(int idx) {
	if (_vm->_globals.GESTE_FLAG != 1) {
		if (PTRNUL != _vm->_globals.GESTE)
			_vm->_globals.GESTE = _vm->_globals.dos_free2(_vm->_globals.GESTE);
		_vm->_globals.GESTE_FLAG = 1;

		FileManager::CONSTRUIT_SYSTEM("DOS.SPR");
		_vm->_globals.GESTE = FileManager::CHARGE_FICHIER(_vm->_globals.NFICHIER);
	}
	if (idx == 1)
		ACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,8,8,8,8,8,8,7,6,5,4,3,2,1,0,-1,", 0, 0, 8, 0);
	if (idx == 2)
		SPACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,8,9,10,11,12,13,-1,", 0, 0, 8, 0);
	if (idx == 3)
		SPACTION1(_vm->_globals.GESTE, "12,11,10,9,8,7,6,5,4,3,2,1,0,-1,", 0, 0, 8);
	if (idx == 4)
		ACTION(
		    _vm->_globals.GESTE,
		    "0,1,2,3,4,5,6,7,8,8,8,8,8,8,9,10,11,12,13,12,11,12,13,12,11,12,13,12,11,10,9,8,7,6,5,4,3,2,1,0,-1,",
		    0,
		    0,
		    8,
		    0);
	if (idx == 5)
		SPACTION(_vm->_globals.GESTE, "15,16,17,18,19,20,21,-1,", 0, 0, 8, 0);
	if (idx == 6)
		SPACTION1(_vm->_globals.GESTE, "20,19,18,17,16,15,-1,", 0, 0, 8);
	if (idx == 7)
		SPACTION(_vm->_globals.GESTE, "15,16,17,18,19,20,21,22,23,24,-1,", 0, 0, 8, 0);
	if (idx == 8)
		SPACTION1(_vm->_globals.GESTE, "23,22,21,20,19,18,17,16,15,-1,", 0, 0, 8);
	if (idx == 9)
		SPACTION(_vm->_globals.GESTE, "15,16,17,18,19,20,21,22,23,24,-1,", 0, 0, 8, 0);
	if (idx == 10)
		SPACTION1(_vm->_globals.GESTE, "23,22,21,20,19,18,17,16,15,-1,", 0, 0, 8);
}

void ObjectsManager::ACTION_DROITE(int idx) {
	if (_vm->_globals.GESTE_FLAG != 3) {
		if (_vm->_globals.GESTE != PTRNUL)
			_vm->_globals.GESTE = _vm->_globals.dos_free2(_vm->_globals.GESTE);
		_vm->_globals.GESTE_FLAG = 3;
		FileManager::CONSTRUIT_SYSTEM("PROFIL.SPR");
		_vm->_globals.GESTE = FileManager::CHARGE_FICHIER(_vm->_globals.NFICHIER);
	}
	if (idx == 1)
		ACTION(_vm->_globals.GESTE, "20,19,18,17,16,15,14,13,13,13,13,13,14,15,16,17,18,19,20,-1,", 0, 0, 8, 0);
	if (idx == 2)
		SPACTION(_vm->_globals.GESTE, "1,2,3,4,5,6,7,8,-1,", 0, 0, 8, 0);
	if (idx == 3)
		SPACTION1(_vm->_globals.GESTE, "9,10,11,12,13,14,15,16,17,18,19,20,-1,", 0, 0, 8);
	if (idx == 4)
		ACTION(_vm->_globals.GESTE, "1,2,3,4,5,6,7,8,8,7,6,5,4,3,2,1,-1,", 0, 0, 8, 0);
	if (idx == 5)
		SPACTION(_vm->_globals.GESTE, "23,24,25,-1,", 0, 0, 8, 0);
	if (idx == 6)
		SPACTION1(_vm->_globals.GESTE, "24,,23,-1,", 0, 0, 8);
	if (idx == 7)
		SPACTION(_vm->_globals.GESTE, "23,24,25,26,27,-1,", 0, 0, 8, 0);
	if (idx == 8)
		SPACTION1(_vm->_globals.GESTE, "26,25,24,23,-1,", 0, 0, 8);
	if (idx == 9)
		SPACTION(_vm->_globals.GESTE, "23,24,25,26,27,28,29,-1,", 0, 0, 8, 0);
	if (idx == 10)
		SPACTION1(_vm->_globals.GESTE, "28,27,26,25,24,23,-1,", 0, 0, 8);
}

void ObjectsManager::Q_DROITE(int idx) {
	if (_vm->_globals.GESTE_FLAG != 4) {
		if (_vm->_globals.GESTE != PTRNUL)
			_vm->_globals.GESTE = _vm->_globals.dos_free2(_vm->_globals.GESTE);
		_vm->_globals.GESTE_FLAG = 4;
		FileManager::CONSTRUIT_SYSTEM("3Q.SPR");
		_vm->_globals.GESTE = FileManager::CHARGE_FICHIER(_vm->_globals.NFICHIER);
	}
	if (idx == 1)
		ACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,8,8,8,8,8,7,6,5,4,3,2,1,0,-1,", 0, 0, 8, 0);
	if (idx == 2)
		SPACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,8,9,10,11,12,-1,", 0, 0, 8, 0);
	if (idx == 3)
		SPACTION1(_vm->_globals.GESTE, "11,10,9,8,7,6,5,4,3,2,1,0,-1,", 0, 0, 8);
	if (idx == 4)
		ACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,8,9,10,11,12,11,12,11,12,11,12,11,10,9,8,7,6,5,4,3,2,1,0,-1,", 0, 0, 8, 0);
	if (idx == 5)
		SPACTION(_vm->_globals.GESTE, "15,16,17,18,-1,", 0, 0, 8, 0);
	if (idx == 6)
		SPACTION1(_vm->_globals.GESTE, "17,16,15,-1,", 0, 0, 8);
	if (idx == 7)
		SPACTION(_vm->_globals.GESTE, "15,16,17,18,19,20-1,", 0, 0, 8, 0);
	if (idx == 8)
		SPACTION1(_vm->_globals.GESTE, "19,18,17,16,15,-1,", 0, 0, 8);
	if (idx == 9)
		SPACTION(_vm->_globals.GESTE, "15,16,17,18,19,20,21,-1,", 0, 0, 8, 0);
	if (idx == 10)
		SPACTION1(_vm->_globals.GESTE, "20,19,18,17,15,-1,", 0, 0, 8);
}

void ObjectsManager::ACTION_FACE(int idx) {
	if (_vm->_globals.GESTE_FLAG != 2) {
		if (_vm->_globals.GESTE != PTRNUL)
			_vm->_globals.GESTE = _vm->_globals.dos_free2(_vm->_globals.GESTE);
		_vm->_globals.GESTE_FLAG = 2;
		FileManager::CONSTRUIT_SYSTEM("FACE.SPR");
		_vm->_globals.GESTE = FileManager::CHARGE_FICHIER(_vm->_globals.NFICHIER);
	}
	if (idx == 1)
		ACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,9,9,9,9,9,9,7,6,5,4,3,2,1,0,-1,", 0, 0, 8, 0);
	if (idx == 2)
		SPACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,9,10,11,12,13,14,15,-1,", 0, 0, 8, 0);
	if (idx == 3)
		SPACTION1(_vm->_globals.GESTE, "14,13,12,11,10,9,7,6,5,4,3,2,1,0,-1,", 0, 0, 8);
	if (idx == 4)
		ACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,9,10,11,12,13,14,13,12,11,10,9,7,6,5,4,3,2,1,0,-1,", 0, 0, 8, 0);
}

void ObjectsManager::Q_GAUCHE(int idx) {
	if (_vm->_globals.GESTE_FLAG != 4) {
		if (_vm->_globals.GESTE != PTRNUL)
			_vm->_globals.GESTE = _vm->_globals.dos_free2(_vm->_globals.GESTE);
		_vm->_globals.GESTE_FLAG = 4;
		FileManager::CONSTRUIT_SYSTEM("3Q.SPR");
		_vm->_globals.GESTE = FileManager::CHARGE_FICHIER(_vm->_globals.NFICHIER);
	}
	if (idx == 1)
		ACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,8,8,8,8,8,7,6,5,4,3,2,1,0,-1,", 0, 0, 8, 1);
	if (idx == 2)
		SPACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,8,9,10,11,12,-1,", 0, 0, 8, 1);
	if (idx == 3)
		SPACTION1(_vm->_globals.GESTE, "11,10,9,8,7,6,5,4,3,2,1,0,-1,", 0, 0, 8);
	if (idx == 4)
		ACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,8,9,10,11,12,11,12,11,12,11,12,11,10,9,8,7,6,5,4,3,2,1,0,-1,", 0, 0, 8, 1);
	if (idx == 5)
		SPACTION(_vm->_globals.GESTE, "15,16,17,18,-1,", 0, 0, 8, 1);
	if (idx == 6)
		SPACTION1(_vm->_globals.GESTE, "17,16,15,-1,", 0, 0, 8);
	if (idx == 7)
		SPACTION(_vm->_globals.GESTE, "15,16,17,18,19,20-1,", 0, 0, 8, 1);
	if (idx == 8)
		SPACTION1(_vm->_globals.GESTE, "19,18,17,16,15,-1,", 0, 0, 8);
	if (idx == 9)
		SPACTION(_vm->_globals.GESTE, "15,16,17,18,19,20,21,-1,", 0, 0, 8, 1);
	if (idx == 10)
		SPACTION1(_vm->_globals.GESTE, "20,19,18,17,15,-1,", 0, 0, 8);
}

void ObjectsManager::ACTION_GAUCHE(int idx) {
	if (_vm->_globals.GESTE_FLAG != 3) {
		if (_vm->_globals.GESTE != PTRNUL)
			_vm->_globals.GESTE = _vm->_globals.dos_free2(_vm->_globals.GESTE);
		_vm->_globals.GESTE_FLAG = 3;
		FileManager::CONSTRUIT_SYSTEM("PROFIL.SPR");
		_vm->_globals.GESTE = FileManager::CHARGE_FICHIER(_vm->_globals.NFICHIER);
	}
	if (idx == 1)
		ACTION(_vm->_globals.GESTE, "20,19,18,17,16,15,14,13,13,13,13,13,14,15,16,17,18,19,20,-1,", 0, 0, 8, 1);
	if (idx == 2)
		SPACTION(_vm->_globals.GESTE, "1,2,3,4,5,6,7,8,-1,", 0, 0, 8, 1);
	if (idx == 3)
		SPACTION1(_vm->_globals.GESTE, "9,10,11,12,13,14,15,16,17,18,19,20,-1,", 0, 0, 8);
	if (idx == 4)
		ACTION(_vm->_globals.GESTE, "1,2,3,4,5,6,7,8,8,7,6,5,4,3,2,1,-1,", 0, 0, 8, 1);
	if (idx == 5)
		SPACTION(_vm->_globals.GESTE, "23,24,25,-1,", 0, 0, 8, 1);
	if (idx == 6)
		SPACTION1(_vm->_globals.GESTE, "24,,23,-1,", 0, 0, 8);
	if (idx == 7)
		SPACTION(_vm->_globals.GESTE, "23,24,25,26,27,-1,", 0, 0, 8, 1);
	if (idx == 8)
		SPACTION1(_vm->_globals.GESTE, "26,25,24,23,-1,", 0, 0, 8);
	if (idx == 9)
		SPACTION(_vm->_globals.GESTE, "23,24,25,26,27,28,29,-1,", 0, 0, 8, 1);
	if (idx == 10)
		SPACTION1(_vm->_globals.GESTE, "28,27,26,25,24,23,-1,", 0, 0, 8);
}

void ObjectsManager::ZONE_ON(int idx) {
	if (_vm->_globals.BOBZONE[idx]) {
		_vm->_globals.BOBZONE_FLAG[idx] = 1;
	} else {
		_vm->_globals.ZONEP[idx].field10 = 1;
	}
}

void ObjectsManager::ZONE_OFF(int idx) {
	if (_vm->_globals.BOBZONE[idx]) {
		_vm->_globals.BOBZONE_FLAG[idx] = 0;
	} else {
		_vm->_globals.ZONEP[idx].field10 = 0;
	}

}

void ObjectsManager::OPTI_ONE(int idx, int a2, int a3, int a4) {
	if (a4 != 3) {
		BOBANIM_ON(idx);
		SET_BOBPOSI(idx, a2);
	}

	do {
		_vm->_eventsManager.VBL();
	} while (a3 != BOBPOSI(idx));

	if (!a4)
		BOBANIM_OFF(idx);
	if (a4 == 4) {
		_vm->_graphicsManager.AFFICHE_SPEED(
		    _vm->_globals.Bob[idx].field4,
		    _vm->_globals.Bob[idx].field3E,
		    _vm->_globals.Bob[idx].field40,
		    _vm->_globals.Bob[idx].fieldC);
		BOBANIM_OFF(idx);
		_vm->_eventsManager.VBL();
	}
}


void ObjectsManager::AFFICHE_SPEED1(byte *speedData, int xp, int yp, int img) {
	SPEED_FLAG = true;
	SPEED_PTR = speedData;
	SPEED_X = xp;
	SPEED_Y = yp;
	SPEED_IMAGE = img;
}

int ObjectsManager::BOBPOSI(int idx) {
	return _vm->_globals.Bob[idx].field10 / 5;
}

void ObjectsManager::BOBANIM_ON(int idx) {
	if (_vm->_globals.Bob[idx].field16) {
		_vm->_globals.Bob[idx].field16 = 0;
		_vm->_globals.Bob[idx].field10 = 5;
		_vm->_globals.Bob[idx].fieldC = 250;
		_vm->_globals.Bob[idx].field12 = 0;
		_vm->_globals.Bob[idx].field14 = 0;
	}
}

void ObjectsManager::BOBANIM_OFF(int idx) {
	_vm->_globals.Bob[idx].field16 = 1;

}

void ObjectsManager::SET_BOBPOSI(int idx, int a2) {
	_vm->_globals.Bob[idx].field10 = 5 * a2;
	_vm->_globals.Bob[idx].field12 = 0;
	_vm->_globals.Bob[idx].field14 = 0;
}

int ObjectsManager::BOBX(int idx) {
	return _vm->_globals.Bob[idx].field8;
}

int ObjectsManager::BOBY(int idx) {
	return _vm->_globals.Bob[idx].fieldA;
}

int ObjectsManager::BOBA(int idx) {
	return _vm->_globals.Bob[idx].fieldC;
}

void ObjectsManager::INILINK(const Common::String &file) {
	int v1;
	int v2;
	int v8; 
	int v9; 
	int v10;
	int v11;
	int v12;
	byte *v13;
	int v14;
	int v15;
	byte *v16;
	byte *v17;
	int v18;
	int v19;
	int v20;
	int v21;
	byte *v22;
	int v23;
	int v24;
	int v25;
	int v27;
	int v28;
	int v29;
	int v30;
	int v31;
	int v32;
	int v33;
	int v34;
	int v35;
	int v36;
	byte *v37;
	int v40;
	size_t v41;
	size_t v42;
	size_t nbytes;
	byte *ptr; 
	Common::String filename, filename2;
	Common::File f;

	v1 = 0;
	v2 = 0;

	filename = file + ".LNK";
	ptr = FileManager::RECHERCHE_CAT(filename, 3);
	nbytes = _vm->_globals.CAT_TAILLE;
	if (ptr == PTRNUL) {
		FileManager::CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, filename);
		
		if (!f.open(_vm->_globals.NFICHIER))
			error("Error opening file - %s", _vm->_globals.NFICHIER.c_str());

		nbytes = f.size();
		ptr = _vm->_globals.dos_malloc2(nbytes);
		if (PTRNUL == ptr)
			error("INILINK");
		FileManager::bload_it(f, ptr, nbytes);
		f.close();
	}
	if (!OBSSEUL) {
		for (int idx = 0; idx < 500; ++idx)
			_vm->_globals.STAILLE[idx] = READ_LE_UINT16((uint16 *)ptr + idx);

		_vm->_globals.RESET_CACHE();
		
		filename2 = Common::String((const char *)ptr + 1000);
		if (!filename2.empty()) {
			_vm->_globals.CACHE_BANQUE[1] = FileManager::RECHERCHE_CAT(filename2, 8);
			if (_vm->_globals.CACHE_BANQUE[1]) {
				_vm->_globals.CAT_FLAG = 0;
				FileManager::CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, filename2);
			} else {
				_vm->_globals.CAT_FLAG = 1;
				FileManager::CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, "RES_SLI.RES");
			}
			_vm->_globals.CACHE_BANQUE[1] = FileManager::CHARGE_FICHIER(_vm->_globals.NFICHIER);
			_vm->_globals.CAT_FLAG = 0;
			v36 = 60;
			v37 = ptr + 1000;
			v40 = 0;

			do {
				v8 = READ_LE_UINT16(v37 + 2 * v36);
				v9 = READ_LE_UINT16(v37 + 2 * v36 + 2);
				v10 = READ_LE_UINT16(v37 + 2 * v36 + 4);
				v11 = v40;

				_vm->_globals.Cache[v11].field14 = READ_LE_UINT16(v37 + 2 * v36 + 8);
				_vm->_globals.Cache[v11].field2 = v8;
				_vm->_globals.Cache[v11].field0 = v9;
				_vm->_globals.Cache[v11].field4 = v10;
				_vm->_globals.Cache[v11].field12 = 1;
				
				if (_vm->_globals.CACHE_BANQUE[1]) {
					_vm->_globals.Cache[v40].fieldA = 0;
				} else {
					v12 = v8;
					v13 = _vm->_globals.CACHE_BANQUE[1];
					v14 = Get_Largeur(v13, v8);
					v15 = Get_Hauteur(v13, v12);
					_vm->_globals.Cache[v40].fieldC = v13;
					_vm->_globals.Cache[v40].field6 = v14;
					_vm->_globals.Cache[v40].field8 = v15;
					_vm->_globals.Cache[v40].fieldA = 1;
				}
				if (!_vm->_globals.Cache[v40].field0 && !_vm->_globals.Cache[v40].field4 
							&& !_vm->_globals.Cache[v40].field2)
					_vm->_globals.Cache[v40].fieldA = 0;
				v36 += 5;
				++v40;
			} while (v40 <= 21);
			_vm->_globals.CACHE_ON();
		}
	}

	_vm->_linesManager.RESET_OBSTACLE();
	v41 = 0;
	if (nbytes) {
		do {
			if (*(ptr + v41) == 79 && *(ptr + v41 + 1) == 66 && *(ptr + v41 + 2) == 50) {
				v16 = ptr + v41 + 4;
				v32 = 0;
				v34 = 0;
				_vm->_linesManager.TOTAL_LIGNES = 0;
				do {
					v27 = READ_LE_UINT16(v16 + 2 * v32);
					if (v27 != -1) {
						_vm->_linesManager.AJOUTE_LIGNE(
						    v34,
						    v27,
						    READ_LE_UINT16(v16 + 2 * v32 + 2),
						    READ_LE_UINT16(v16 + 2 * v32 + 4),
						    READ_LE_UINT16(v16 + 2 * v32 + 6),
						    READ_LE_UINT16(v16 + 2 * v32 + 8),
						    1);
						++_vm->_linesManager.TOTAL_LIGNES;
					}
					v32 += 5;
					++v34;
				} while (v27 != -1);
				_vm->_linesManager.INIPARCOURS();
			}
			++v41;
		} while (nbytes > v41);
	}
	if (!OBSSEUL) {
		v42 = 0;
		if (nbytes) {
			do {
				if (*(ptr + v42) == 'Z' && *(ptr + v42 + 1) == 'O' && *(ptr + v42 + 2) == '2') {
					v17 = ptr + v42 + 4;
					v33 = 0;
					v35 = 0;
					v18 = 1;
					do {
						v19 = 20 * v18;
						_vm->_globals.ZONEP[v19].field0 = 0;
						_vm->_globals.ZONEP[v19].field2 = 0;
						_vm->_globals.ZONEP[v19].field4 = 0;
						_vm->_globals.ZONEP[v19].field6 = 0;
						_vm->_globals.ZONEP[v19].field7 = 0;
						_vm->_globals.ZONEP[v19].field8 = 0;
						_vm->_globals.ZONEP[v19].field9 = 0;
						_vm->_globals.ZONEP[v19].fieldA = 0;
						_vm->_globals.ZONEP[v19].fieldB = 0;
						_vm->_globals.ZONEP[v19].fieldC = 0;
						_vm->_globals.ZONEP[v19].fieldD = 0;
						_vm->_globals.ZONEP[v19].fieldE = 0;
						_vm->_globals.ZONEP[v19].fieldF = 0;
						_vm->_globals.ZONEP[v19].field12 = 0;
						++v18;
					} while (v18 <= 100);

					v31 = 0;
					do {
						v28 = READ_LE_UINT16(v17 + 2 * v33);
						if (v28 != -1) {
							_vm->_linesManager.AJOUTE_LIGNE_ZONE(
							    v35,
							    READ_LE_UINT16(v17 + 2 * v33 + 2),
							    READ_LE_UINT16(v17 + 2 * v33 + 4),
							    READ_LE_UINT16(v17 + 2 * v33 + 6),
							    READ_LE_UINT16(v17 + 2 * v33 + 8),
							    v28);
							_vm->_globals.ZONEP[v28].field10 = 1;
						}
						v33 += 5;
						++v35;
						++v31;
					} while (v28 != -1);
					v20 = 1;
					do {
						v21 = 20 * v20;
						_vm->_globals.ZONEP[v21].field0 = READ_LE_UINT16(v17 + 2 * v33);
						_vm->_globals.ZONEP[v21].field2 = READ_LE_UINT16(v17 + 2 * v33 + 2);
						_vm->_globals.ZONEP[v21].field4 = READ_LE_UINT16(v17 + 2 * v33 + 4);
						v33 += 3;
						++v20;
					} while (v20 <= 100);

					v22 = ptr + v42 + (10 * v31 + 606) + 4;
					v29 = 0;
					v23 = 1;
					do {
						v24 = 20 * v23;
						_vm->_globals.ZONEP[v24].field6 = *(v29 + v22);
						_vm->_globals.ZONEP[v24].field7 = *(v22 + v29 + 1);
						_vm->_globals.ZONEP[v24].field8 = *(v22 + v29 + 2);
						_vm->_globals.ZONEP[v24].field9 = *(v22 + v29 + 3);
						_vm->_globals.ZONEP[v24].fieldA = *(v22 + v29 + 4);
						_vm->_globals.ZONEP[v24].fieldB = *(v22 + v29 + 5);
						_vm->_globals.ZONEP[v24].fieldC = *(v22 + v29 + 6);
						_vm->_globals.ZONEP[v24].fieldD = *(v22 + v29 + 7);
						_vm->_globals.ZONEP[v24].fieldE = *(v22 + v29 + 8);
						_vm->_globals.ZONEP[v24].fieldF = *(v22 + v29 + 9);
						v29 += 10;
						++v23;
					} while (v23 <= 100);
					v30 = 0;
					v25 = 1;
					do
						_vm->_globals.ZONEP[v25].field12 = READ_LE_UINT16(v22 + 1010 + 2 * v30++);
					while (++v25 <= 100);
					CARRE_ZONE();
				}
				++v42;
			} while (nbytes > v42);
		}
	}
	_vm->_globals.dos_free2(ptr);	
}

void ObjectsManager::SPECIAL_INI(const Common::String &a1) {
	int v2; 
	int v3; 
	int v4; 
	int v5; 
	int v6; 
	int v7; 

	if (_vm->_globals.ECRAN == 73 && !_vm->_globals.SAUVEGARDE->data[svField318]) {
		_vm->_globals.CACHE_SUB(0);
		_vm->_globals.CACHE_SUB(1);
	}
	if ((uint16)(_vm->_globals.ECRAN - 35) <= 6u) {
		_vm->_globals.BOBZONE[20] = 1;
		_vm->_globals.BOBZONE[21] = 2;
		_vm->_globals.BOBZONE[22] = 3;
		_vm->_globals.BOBZONE[23] = 4;
		_vm->_globals.BOBZONE_FLAG[20] = 1;
		_vm->_globals.BOBZONE_FLAG[21] = 1;
		_vm->_globals.BOBZONE_FLAG[22] = 1;
		_vm->_globals.BOBZONE_FLAG[23] = 1;
		_vm->_objectsManager.VERBE_ON(20, 5);
		_vm->_objectsManager.VERBE_ON(21, 5);
		_vm->_objectsManager.VERBE_ON(22, 5);
		_vm->_objectsManager.VERBE_ON(23, 5);
		_vm->_globals.ZONEP[20].field12 = 30;
		_vm->_globals.ZONEP[21].field12 = 30;
		_vm->_globals.ZONEP[22].field12 = 30;
		_vm->_globals.ZONEP[23].field12 = 30;
		v2 = 200;
		do {
			if (_vm->_globals.SAUVEGARDE->data[v2] != 2)
				_vm->_globals.SAUVEGARDE->data[v2] = 0;
			++v2;
		} while (v2 <= 214);
	}
	if (_vm->_globals.ECRAN == 93) {
		if (!_vm->_globals.SAUVEGARDE->data[svField333])
			_vm->_objectsManager.BOBANIM_ON(8);
	}
	if (_vm->_globals.ECRAN == 18 && _vm->_globals.OLD_ECRAN == 17) {
		_vm->_eventsManager.souris_n = 4;
		_vm->_globals.BPP_NOAFF = 1;
		v3 = 0;
		do {
			_vm->_eventsManager.VBL();
			++v3;
		} while (v3 <= 4);
		_vm->_globals.BPP_NOAFF = 0;
		_vm->_graphicsManager.FADE_INW();
		_vm->_globals.iRegul = 1;
		_vm->_globals.DESACTIVE_INVENT = 0;
		_vm->_graphicsManager.NOFADE = 1;
		_vm->_globals.NOPARLE = 1;
		_vm->_talkManager.PARLER_PERSO("MAGE1.pe2");
		_vm->_graphicsManager.NOFADE = 1;
		_vm->_globals.DESACTIVE_INVENT = 0;
	}
	if (_vm->_globals.ECRAN == 17 && _vm->_globals.OLD_ECRAN == 20) {
		_vm->_globals.DESACTIVE_INVENT = 1;
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
		_vm->_graphicsManager.FADE_INW();
		SPRITE_ON(0);
		v5 = 0;
		do {
			_vm->_eventsManager.VBL();
			++v5;
		} while (v5 <= 4);
		VBOB(_vm->_globals.SPRITE_ECRAN, 5, 15, 28, 1);
		_vm->_fontManager.TEXTE_OFF(9);
		if (!_vm->_soundManager.TEXTOFF) {
			_vm->_fontManager.DOS_TEXT(9, 383, _vm->_globals.FICH_TEXTE, 220, 72, 20, 25, 6, 36, 253);
			if (!_vm->_soundManager.TEXTOFF)
				_vm->_fontManager.TEXTE_ON(9);
		}
		if (!_vm->_soundManager.VOICEOFF)
			_vm->_soundManager.VOICE_MIX(383, 4);
		_vm->_globals.SAUVEGARDE->data[svField270] = 1;
		_vm->_globals.SAUVEGARDE->data[svField300] = 1;
		_vm->_globals.SAUVEGARDE->data[svField320] = 1;
		if (_vm->_soundManager.VOICEOFF == 1) {
			v6 = 0;
			do {
				_vm->_eventsManager.VBL();
				++v6;
			} while (v6 <= 199);
		}
		_vm->_fontManager.TEXTE_OFF(9);
		VBOB_OFF(5);
		v7 = 0;
		do {
			_vm->_eventsManager.VBL();
			++v7;
		} while (v7 <= 3);
		_vm->_graphicsManager.NOFADE = 1;
		_vm->_globals.DESACTIVE_INVENT = 0;
	}
}

void ObjectsManager::OPTI_BOBON(int a1, int a2, int a3, int a4, int a5, int a6, int a7) {
	if (a1 != -1)
		BOBANIM_ON(a1);
	if (a2 != -1)
		BOBANIM_ON(a2);
	if (a3 != -1)
		BOBANIM_ON(a3);
	if (!a7) {
		if (a1 != -1)
			SET_BOBPOSI(a1, a4);
		if (a2 != -1)
			SET_BOBPOSI(a2, a5);
		if (a3 != -1)
			SET_BOBPOSI(a3, a6);
	}
}

void ObjectsManager::SCI_OPTI_ONE(int a1, int a2, int a3, int a4) {
	_vm->_eventsManager.souris_bb = 0;
	_vm->_eventsManager.souris_b = 0;

	if (a4 != 3) {
		BOBANIM_ON(a1);
		SET_BOBPOSI(a1, a2);
	}

	do {
		_vm->_eventsManager.VBL();
		if (_vm->_eventsManager.souris_bb)
			break;
	} while (a3 != BOBPOSI(a1));
	if (!a4)
		BOBANIM_OFF(a1);
}

int ObjectsManager::Control_Goto(const byte *dataP) {
	return READ_LE_UINT16(dataP + 5);
}

int ObjectsManager::Control_If(const byte *dataP, int a2) {
	int v2; 
	int v3; 
	int v4; 
	int v5; 
	int v6; 
	int v7; 
	const byte *v8; 
	int v9; 
	int v10; 
	int v11; 
	int v12; 
	int v14; 
	int v15; 
	int v16; 
	int v17; 
	int v18; 
	int v19; 
	int v20; 

	v2 = a2;
LABEL_2:
	v3 = v2;
	do {
		++v3;
		v4 = Traduction2(dataP + 20 * v3);
		if (v3 > 400) {
			v5 = v4;
			error("Control if failed");
			v4 = v5;
		}
	} while (v4 != 4);
	v20 = v3;
	v6 = v2;
	v7 = 0;
	do {
		++v6;
		if (Traduction2(dataP + 20 * v6) == 3)
			v7 = 1;
		if (v6 > 400)
			error("Control if failed ");
		if (v7 == 1) {
			v2 = v20;
			goto LABEL_2;
		}
	} while (v20 != v6);
	v8 = dataP + 20 * a2;
	v9 = *(v8 + 13);
	v17 = *(v8 + 14);
	v16 = *(v8 + 15);
	v10 = READ_LE_UINT16(v8 + 5);
	v11 = READ_LE_UINT16(v8 + 7);
	v19 = READ_LE_UINT16(v8 + 9);
	v18 = READ_LE_UINT16(v8 + 11);
	v14 = 0;
	v15 = 0;
	if (v9 == 1 && _vm->_globals.SAUVEGARDE->data[v10] == v11)
		v14 = 1;
	if (v9 == 2 && _vm->_globals.SAUVEGARDE->data[v10] != v11)
		v14 = 1;
	if (v9 == 3 && _vm->_globals.SAUVEGARDE->data[v10] <= v11)
		v14 = 1;
	if (v9 == 4 && _vm->_globals.SAUVEGARDE->data[v10] >= v11)
		v14 = 1;
	if (v9 == 5 && _vm->_globals.SAUVEGARDE->data[v10] > v11)
		v14 = 1;
	if (v9 == 6 && _vm->_globals.SAUVEGARDE->data[v10] < v11)
		v14 = 1;
	if (v16 == 3)
		goto LABEL_68;
	if (v17 == 1 && v18 == _vm->_globals.SAUVEGARDE->data[v19])
		v15 = 1;
	if (v17 == 2 && v18 != _vm->_globals.SAUVEGARDE->data[v19])
		v15 = 1;
	if (v17 == 3 && v18 >= _vm->_globals.SAUVEGARDE->data[v19])
		v15 = 1;
	if (v17 == 4 && v18 <= _vm->_globals.SAUVEGARDE->data[v19])
		v15 = 1;
	if (v17 == 5 && v18 < _vm->_globals.SAUVEGARDE->data[v19])
		v15 = 1;
	if (v17 == 6 && v18 > _vm->_globals.SAUVEGARDE->data[v19])
		v15 = 1;
	if (v16 == 3) {
LABEL_68:
		if (v14 == 1) {
			v12 = a2;
			return (v12 + 1);
		}
		if (!v14) {
LABEL_63:
			v12 = v20;
			return (v12 + 1);
		}
	}
	if (v16 == 1) {
		if (v14 == 1 && v15 == 1) {
			v12 = a2;
			return (v12 + 1);
		}
		goto LABEL_63;
	}
	if (v16 == 2) {
		if (v14 == 1 || v15 == 1) {
			v12 = a2;
			return (v12 + 1);
		}
		goto LABEL_63;
	}
	return -1;
}

int ObjectsManager::Traduction2(const byte *dataP) {
	signed __int16 result; 

	result = 0;
	if (*dataP != 'F' || *(dataP + 1) != 'C') {
		result = 0;
	} else {
		if (*(dataP + 'T') == 84 && *(dataP + 3) == 'X' && *(dataP + 4) == 'T')
			result = 1;
		if (*(dataP + 2) == 'B') {
			if (*(dataP + 3) == 'O' && *(dataP + 4) == 'B')
				result = 1;
			if (*(dataP + 2) == 'B' && *(dataP + 3) == 'O' && *(dataP + 4) == 'F')
				result = 1;
		}
		if (*(dataP + 2) == 'P' && *(dataP + 3) == 'E' && *(dataP + 4) == 'R')
			result = 1;
		if (*(dataP + 2) == 'S' && *(dataP + 3) == 'O' && *(dataP + 4) == 'U')
			result = 1;
		if (*(dataP + 2) == 'M' && *(dataP + 3) == 'U' && *(dataP + 4) == 'S')
			result = 1;
		if (*(dataP + 2) == 'O') {
			if (*(dataP + 3) == 'B' && *(dataP + 4) == 'P')
				result = 1;
			if (*(dataP + 2) == 'O' && *(dataP + 3) == 'B' && *(dataP + 4) == 'M')
				result = 1;
		}
		if (*(dataP + 2) == 'G' && *(dataP + 3) == 'O' && *(dataP + 4) == 'T')
			result = 2;
		if (*(dataP + 2) == 'Z') {
			if (*(dataP + 3) == 'O' && *(dataP + 4) == 'N')
				result = 1;
			if (*(dataP + 2) == 'Z' && *(dataP + 3) == 'O' && *(dataP + 4) == 'F')
				result = 1;
		}
		if (*(dataP + 2) == 'E' && *(dataP + 3) == 'X' && *(dataP + 4) == 'I')
			result = 5;
		if (*(dataP + 2) == 'S' && *(dataP + 3) == 'O' && *(dataP + 4) == 'R')
			result = 5;
		if (*(dataP + 2) == 'A' && *(dataP + 3) == 'N' && *(dataP + 4) == 'I')
			result = 1;
		if (*(dataP + 2) == 'S' && *(dataP + 3) == 'P' && *(dataP + 4) == 'E')
			result = 1;
		if (*(dataP + 2) == 'E' && *(dataP + 3) == 'I' && *(dataP + 4) == 'F')
			result = 4;
		if (*(dataP + 2) == 'V') {
			if (*(dataP + 3) == 'A' && *(dataP + 4) == 'L')
				result = 1;
			if (*(dataP + 2) == 'V') {
				if (*(dataP + 3) == 'O' && *(dataP + 4) == 'N')
					result = 1;
				if (*(dataP + 2) == 'V' && *(dataP + 3) == 'O' && *(dataP + 4) == 'F')
					result = 1;
			}
		}
		if (*(dataP + 2) == 'I' && *(dataP + 3) == 'I' && *(dataP + 4) == 'F')
			result = 3;
		if (*(dataP + 2) == 'J' && *(dataP + 3) == 'U' && *(dataP + 4) == 'M')
			result = 6;
		if (*(dataP + 2) == 'B') {
			if (*(dataP + 3) == 'C' && *(dataP + 4) == 'A')
				result = 1;
			if (*(dataP + 2) == 'B' && *(dataP + 3) == 'O' && *(dataP + 4) == 'S')
				result = 1;
		}
		if (*(dataP + 2) == 'Z' && *(dataP + 3) == 'C') {
			if (*(dataP + 4) == 'H')
				result = 1;
		}
	}
	return result;
}

void ObjectsManager::VERBE_OFF(int idx, int a2) {
	if (a2 == 6) {
		_vm->_globals.ZONEP[idx].field6 = 0;
	}
	if (a2 == 7) {
		_vm->_globals.ZONEP[idx].field7 = 0;
	}
	if (a2 == 8) {
		_vm->_globals.ZONEP[idx].field8 = 0;
	}
	if (a2 == 9) {
		_vm->_globals.ZONEP[idx].field9 = 0;
	}
	if (a2 == 10) {
		_vm->_globals.ZONEP[idx].fieldA = 0;
	}
	if (a2 == 11) {
		_vm->_globals.ZONEP[idx].fieldB = 0;
	}
	if (a2 == 12) {
		_vm->_globals.ZONEP[idx].fieldC = 0;
	}
	if (a2 == 13) {
		_vm->_globals.ZONEP[idx].fieldD = 0;
	}
	if (a2 == 14 || a2 == 25) {
		_vm->_globals.ZONEP[idx].fieldE = 0;
	}
	if (a2 == 15) {
		_vm->_globals.ZONEP[idx].fieldF = 0;
	}
	if (a2 == 16) {
		_vm->_globals.ZONEP[idx].field6 = 0;
	}
	if (a2 == 5) {
		_vm->_globals.ZONEP[idx].field8 = 0;
	}
	if (a2 == 17) {
		_vm->_globals.ZONEP[idx].field9 = 0;
	}
	if (a2 == 18) {
		_vm->_globals.ZONEP[idx].fieldA = 0;
	}
	if (a2 == 19) {
		_vm->_globals.ZONEP[idx].fieldB = 0;
	}
	if (a2 == 20) {
		_vm->_globals.ZONEP[idx].fieldC = 0;
	}
	if (a2 == 22) {
		_vm->_globals.ZONEP[idx].fieldD = 0;
	}
	if (a2 == 21) {
		_vm->_globals.ZONEP[idx].fieldE = 0;
	}
	if (a2 == 24) {
		_vm->_globals.ZONEP[idx].field9 = 0;
	}
	CHANGEVERBE = 1;
}

void ObjectsManager::VERBE_ON(int idx, int a2) {
	if (a2 == 6) {
		_vm->_globals.ZONEP[idx].field6 = 1;
	}
	if (a2 == 7) {
		_vm->_globals.ZONEP[idx].field7 = 1;
	}
	if (a2 == 8) {
		_vm->_globals.ZONEP[idx].field8 = 1;
	}
	if (a2 == 9) {
		_vm->_globals.ZONEP[idx].field9 = 1;
	}
	if (a2 == 10) {
		_vm->_globals.ZONEP[idx].fieldA = 1;
	}
	if (a2 == 11) {
		_vm->_globals.ZONEP[idx].fieldB = 1;
	}
	if (a2 == 12) {
		_vm->_globals.ZONEP[idx].fieldC = 1;
	}
	if (a2 == 13) {
		_vm->_globals.ZONEP[idx].fieldD = 1;
	}
	if (a2 == 14) {
		_vm->_globals.ZONEP[idx].fieldD = 1;
	}
	if (a2 == 15) {
		_vm->_globals.ZONEP[idx].fieldE = 1;
	}
	if (a2 == 16) {
		_vm->_globals.ZONEP[idx].field6 = 2;
	}
	if (a2 == 5) {
		_vm->_globals.ZONEP[idx].field8 = 2;
	}
	if (a2 == 17) {
		_vm->_globals.ZONEP[idx].field9 = 2;
	}
	if (a2 == 18) {
		_vm->_globals.ZONEP[idx].fieldA = 2;
	}
	if (a2 == 19) {
		_vm->_globals.ZONEP[idx].fieldB = 2;
	}
	if (a2 == 20) {
		_vm->_globals.ZONEP[idx].fieldC = 2;
	}
	if (a2 == 22) {
		_vm->_globals.ZONEP[idx].fieldD = 2;
	}
	if (a2 == 21) {
		_vm->_globals.ZONEP[idx].fieldE = 2;
	}
	if (a2 == 24) {
		_vm->_globals.ZONEP[idx].field9 = 3;
	}
	if (a2 == 25) {
		_vm->_globals.ZONEP[idx].fieldE = 2;
	}
}

int ObjectsManager::CALC_PROPRE(int idx) {
	int v1; 
	int v2; 
	int v3; 
	int v4; 

	v1 = 25;
	v2 = _vm->_globals.STAILLE[idx];
	if (_vm->_globals.PERSO_TYPE == 1) {
		v3 = _vm->_globals.STAILLE[idx];
		if (v2 < 0)
			v3 = -v2;
		v2 = 20 * (5 * v3 - 100) / -80;
	}
	if (_vm->_globals.PERSO_TYPE == 2) {
		v4 = v2;
		if (v2 < 0)
			v4 = -v2;
		v2 = 20 * (5 * v4 - 165) / -67;
	}
	if (v2 < 0)
		return _vm->_graphicsManager.Reel_Reduc(0x19u, -v2);
	if (v2 > 0)
		return _vm->_graphicsManager.Reel_Zoom(0x19u, v2);

	return v1;
}

int ObjectsManager::colision(int a1, int a2) {
	if (_vm->_globals.SegmentEnCours <= 0)
		return -1;

	int v2 = a1;
	int v3 = v2;
	int v4 = v2 + 4;
	int v5 = v2 - 4;

	for (int idx = 0; idx <= _vm->_globals.SegmentEnCours; ++idx) {
		int v8 = _vm->_globals.Segment[idx].field2;
		if (v8 < _vm->_globals.Segment[idx].field4)
			continue;
		
		int v6 = a2 + 4;
		int v7 = a2 - 4;
		int v9 = v3 + 1;

		do {
			byte *srcP = _vm->_linesManager.LigneZone[v8].field4;
			if (srcP != PTRNUL) {
				bool flag = true;
				int v11 = READ_LE_UINT16(srcP + _vm->_linesManager.LigneZone[v8].field0 * 2);
				int dataV2 = READ_LE_UINT16(srcP + _vm->_linesManager.LigneZone[v8].field0 * 2 + 2);
				int v12 = READ_LE_UINT16(srcP + _vm->_linesManager.LigneZone[v8].field0 * 4 - 4);
				int v13 = READ_LE_UINT16(srcP + _vm->_linesManager.LigneZone[v8].field0 * 4 - 2);
				int v10 = v13;

				if (v11 < v12 && v4 >= v11 && v5 > v13)
					flag = false;
				if (v11 >= v12 && v5 <= v13 && v4 < v13)
					flag = false;
				if (dataV2 < v10 && v6 >= dataV2 && v7 > v10)
					flag = false;
				if (dataV2 >= v10 && v7 <= dataV2 && v6 < v10)
					flag = false;


				if (!flag) {
					if (v11 < _vm->_linesManager.LigneZone[v8].field0) {
						for (; v11 < _vm->_linesManager.LigneZone[v8].field0; ++idx) {
							int v11 = READ_LE_UINT16(srcP);
							int v12 = READ_LE_UINT16(srcP + 2);
							srcP += 4;
							
							if ((v2 == v11 || v9 == v11) && a2 == v12)
								return _vm->_linesManager.LigneZone[v8].field2;
						}
					}
				}
			}
		} while (++v8 < _vm->_globals.Segment[idx].field0);
	}

	return -1;
}

void ObjectsManager::ACTION(const byte *spriteData, const Common::String &a2, int a3, int a4, int a5, int a6) {
	int v6; 
	int v7; 
	char v8; 
	int v9; 
	int v10; 
	int v11; 
	int v13; 
	const byte *v14; 
	char v15; 
	int v16; 
	int v17; 
	int v18; 
	Common::String v19; 

	v18 = 0;
	v6 = 0;
	v13 = 0;
	v15 = a5;
	v19 = "     ";
	if (_vm->_globals.vitesse == 2)
		v15 = a5 / 2;
	if (_vm->_globals.vitesse == 3)
		v15 = a5 / 3;
	v14 = Sprite[0].spriteData;
	v17 = Sprite[0].field10;
	v16 = Sprite[0].fieldE;
	Sprite[0].field12 += a3;
	Sprite[0].field14 += a4;
	Sprite[0].fieldE = a6;

	for (;;) {
		v7 = 0;
		v8 = a2[v18];
		if (v8 == ',') {
			v9 = atoi(v19.c_str());
			v13 = v9;
			v6 = 0;
			v19 = "     ";
			v7 = 1;
		} else {
			v9 = v6;
			v19 += v8;
			v6 = v6 + 1;
		}
		++v18;
		if (v7 == 1) {
			if (v13 == -1) {
				Sprite[0].spriteData = v14;
				Sprite[0].field10 = v17;
				Sprite[0].field12 -= a3;
				Sprite[0].field14 -= a4;
				Sprite[0].fieldE = v16;
			} else {
				Sprite[0].spriteData = spriteData;
				Sprite[0].field10 = v13;
			}
			v10 = 0;
			v9 = v15;
			if (v15 > 0) {
				do {
					v11 = v9;
					_vm->_eventsManager.VBL();
					++v10;
					v9 = v11;
				} while (v10 < v11);
			}
			if (v13 == -1)
				break;
		}
	}
	return;
}

void ObjectsManager::SPACTION(byte *a1, const Common::String &a2, int a3, int a4, int a5, int a6) {
	int v6; 
	signed __int16 v7; 
	char v8; 
	__int32 v9; 
	signed __int16 v10; 
	__int32 v11; 
	__int16 v13; 
	char v14; 
	__int16 v15; 
	Common::String v16; 

	v15 = 0;
	v6 = 0;
	v13 = 0;
	v16 = "     ";
	v14 = a5;
	if (_vm->_globals.vitesse == 2)
		v14 = a5 / 2;
	if (_vm->_globals.vitesse == 3)
		v14 = a5 / 3;
	S_old_spr = Sprite[0].spriteData;
	S_old_ani = Sprite[0].field10;
	S_old_ret = Sprite[0].fieldE;
	Sprite[0].field12 += a3;
	Sprite[0].field14 += a4;
	Sprite[0].fieldE = a6;
	do {
		do {
			v7 = 0;
			v8 = a2[v15];
			if (v8 == ',') {
				v9 = atoi(v16.c_str());
				v13 = v9;
				v6 = 0;
				v16 = "     ";
				v7 = 1;
			} else {
				v9 = v6;
				v16.setChar(v8, v6);
				v6 = v6 + 1;
			}
			++v15;
		} while (v7 != 1);
		if (v13 != -1) {
			Sprite[0].spriteData = a1;
			Sprite[0].field10 = v13;
		}
		v10 = 0;
		v9 = v14;
		if (v14 > 0) {
			do {
				v11 = v9;
				_vm->_eventsManager.VBL();
				++v10;
				v9 = v11;
			} while (v10 < v11);
		}
	} while (v13 != -1);
}

void ObjectsManager::SPACTION1(byte *a1, const Common::String &a2, int a3, int a4, int a5) {
	int v5; 
	int v6; 
	char v7; 
	int v8; 
	int v9; 
	int v10; 
	int v12; 
	int v13; 
	int v14; 
	Common::String v15; 
	int v16;

	v14 = 0;
	v5 = 0;
	v12 = 0;
	v15 = "     ";
	v13 = a5;
	if (_vm->_globals.vitesse == 2)
		v13 = a5 / 2;
	if (_vm->_globals.vitesse == 3)
		v13 = a5 / 3;
	while (1) {
		v6 = 0;
		v7 = a2[v14];
		if (v7 == 44) {
			v8 = atoi(v15.c_str());
			v12 = v8;
			v5 = 0;
			v15 = "     ";
			v6 = 1;
		} else {
			v8 = v5;
			v15.setChar(v7, v5);
			v5 = v5 + 1;
		}
		++v14;
		if (v6 == 1) {
			if (v12 == -1) {
				Sprite[0].spriteData = S_old_spr;
				Sprite[0].field10 = S_old_ani;
				Sprite[0].field12 -= a3;
				Sprite[0].field14 -= a4;
				Sprite[0].fieldE = S_old_ret;
			} else {
				Sprite[0].spriteData = a1;
				Sprite[0].field10 = v12;
			}

			v9 = 0;
			v16 = v13;
			if (v13 > 0) {
				do {
					v10 = v8;
					_vm->_eventsManager.VBL();
					++v9;
					v16 = v10;
				} while (v9 < v10);
			}
			if (v12 == -1)
				break;
		}
	}
}

void ObjectsManager::TEST_FORET(int a1, int a2, int a3, int a4, int a5, int a6) {
	signed int v6; 
	char v7; 

	v6 = a1;
	if (_vm->_globals.ECRAN == a1) {
		if (a1 == 35) {
			if (a6 > 2 || (v6 = 200, a6 > 2))
				v6 = 201;
		}
		if (_vm->_globals.ECRAN == 36) {
			if (a6 > 2 || (v6 = 202, a6 > 2))
				v6 = 203;
		}
		if (_vm->_globals.ECRAN == 37) {
			if (a6 > 2 || (v6 = 204, a6 > 2))
				v6 = 205;
		}
		if (_vm->_globals.ECRAN == 38) {
			if (a6 > 2 || (v6 = 206, a6 > 2))
				v6 = 207;
		}
		if (_vm->_globals.ECRAN == 39) {
			if (a6 > 2 || (v6 = 208, a6 > 2))
				v6 = 209;
		}
		if (_vm->_globals.ECRAN == 40) {
			if (a6 > 2 || (v6 = 210, a6 > 2))
				v6 = 211;
		}
		if (_vm->_globals.ECRAN == 41) {
			if (a6 > 2 || (v6 = 212, a6 > 2))
				v6 = 213;
		}
		v7 = _vm->_globals.SAUVEGARDE->data[v6];
		if (v7 != 2) {
			if (v7) {
				if (v7 == 1) {
					if (a6 == 1 && BOBPOSI(1) == 26) {
						VIRE_INVENT = 1;
						_vm->_soundManager.PLAY_SAMPLE2(1);
						_vm->_globals.SAUVEGARDE->data[v6] = 4;
					}
					if (a6 == 2 && BOBPOSI(2) == 26) {
						VIRE_INVENT = 1;
						_vm->_soundManager.PLAY_SAMPLE2(1);
						_vm->_globals.SAUVEGARDE->data[v6] = 4;
					}
					if (a6 == 3 && BOBPOSI(3) == 27) {
						VIRE_INVENT = 1;
						_vm->_soundManager.PLAY_SAMPLE2(1);
						_vm->_globals.SAUVEGARDE->data[v6] = 4;
					}
					if (a6 == 4 && BOBPOSI(4) == 27) {
						VIRE_INVENT = 1;
						_vm->_soundManager.PLAY_SAMPLE2(1);
						VIRE_INVENT = 1;
						_vm->_globals.SAUVEGARDE->data[v6] = 4;
					}
				}
				if (_vm->_globals.SAUVEGARDE->data[v6] == 4) {
					if (a6 == 1 && (signed int)BOBPOSI(1) > 30)
						_vm->_globals.SAUVEGARDE->data[v6] = 3;
					if (a6 == 2 && (signed int)BOBPOSI(2) > 30)
						_vm->_globals.SAUVEGARDE->data[v6] = 3;
					if (a6 == 3 && (signed int)BOBPOSI(3) > 30)
						_vm->_globals.SAUVEGARDE->data[v6] = 3;
					if (a6 == 4 && (signed int)BOBPOSI(4) > 30)
						_vm->_globals.SAUVEGARDE->data[v6] = 3;
				}
				if (_vm->_globals.SAUVEGARDE->data[v6] == 3) {
					_vm->_graphicsManager.FADE_LINUX = 2;
					_vm->_animationManager.PLAY_ANM("CREVE2.ANM", 100, 24, 500);
					_vm->_globals.SORTIE = 150;
					_vm->_graphicsManager.NOFADE = 1;
					BOB_OFF(1);
					BOB_OFF(2);
					BOB_OFF(3);
					BOB_OFF(4);
				}
			} else if (a2 < XSPR(0)
			           && a3 > XSPR(0)
			           && a4 < YSPR(0)
			           && a5 > YSPR(0)) {
				if (a6 == 1)
					BOBANIM_ON(1);
				if (a6 == 2)
					BOBANIM_ON(2);
				if (a6 == 3)
					BOBANIM_ON(3);
				if (a6 == 4)
					BOBANIM_ON(4);
				_vm->_globals.SAUVEGARDE->data[v6] = 1;
			}
		}
	}
}

void ObjectsManager::BLOQUE_ANIMX(int idx, int a2) {
	_vm->_globals.BL_ANIM[idx].v1 = 1;
	_vm->_globals.BL_ANIM[idx].v2 = a2;
}

void ObjectsManager::PERSONAGE(const Common::String &s1, const Common::String &s2, const Common::String &s3,
							   const Common::String &s4, int v) {
	int v5; 
	int v6; 
	int v7; 
	int v8; 
	int v9; 

	v5 = 0;
	INVENTFLAG = 0;
	KEY_INVENT = 0;
	VIRE_INVENT = 0;
	_vm->_graphicsManager.ofscroll = 0;
	_vm->_globals.PLAN_FLAG = 0;
	_vm->_globals.iRegul = 1;
	_vm->_soundManager.WSOUND(v);
	_vm->_globals.chemin = PTRNUL;
	_vm->_globals.NOMARCHE = 1;
	_vm->_globals.SORTIE = 0;
	_vm->_globals.AFFLI = 0;
	_vm->_globals.AFFIVBL = 0;
	if (!s1.empty())
		_vm->_graphicsManager.LOAD_IMAGE(s1);
	if (!s2.empty())
		INILINK(s2);
	if (!s3.empty())
		_vm->_animationManager.CHARGE_ANIM(s3);
	_vm->_graphicsManager.VISU_ALL();
	if (!s4.empty()) {
		if (!_vm->_globals.NOSPRECRAN)
			_vm->_graphicsManager.INI_ECRAN(s4);
		if (!s4.empty() && _vm->_globals.NOSPRECRAN == 1)
			_vm->_graphicsManager.INI_ECRAN2(s4);
	}
	_vm->_eventsManager.MOUSE_ON();
	if (_vm->_globals.ECRAN == 61) {
		SPRITE(_vm->_globals.PERSO, 330, 418, 0, 60, 0, 0, 34, 190);
		SPRITE_ON(0);
		_vm->_globals.chemin = PTRNUL;
		VERIFTAILLE();
	}
	_vm->_graphicsManager.SETCOLOR3(252, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(253, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(251, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(254, 0, 0, 0);
	_vm->_eventsManager.CHANGE_MOUSE(4);
	_vm->_globals.BPP_NOAFF = 1;
	v6 = 0;
	do {
		_vm->_eventsManager.VBL();
		++v6;
	} while (v6 <= 4);
	_vm->_globals.BPP_NOAFF = 0;
	_vm->_graphicsManager.FADE_INW();
	if (_vm->_globals.ECRAN == 61) {
		_vm->_animationManager.PLAY_SEQ(0, "OUVRE.SEQ", 10, 4, 10);
		BOBANIM_OFF(3);
		_vm->_globals.NOT_VERIF = 1;
		g_old_x = XSPR(0);
		g_old_sens = -1;
		_vm->_globals.Compteur = 0;
		_vm->_globals.chemin = PTRNUL;
		v7 = YSPR(0);
		v8 = XSPR(0);
		_vm->_globals.chemin = _vm->_linesManager.PARCOURS2(v8, v7, 330, 345);
		_vm->_globals.NOT_VERIF = 1;
		do {
			GOHOME();
			_vm->_eventsManager.VBL();
		} while (_vm->_globals.chemin != PTRNUL);
		SETANISPR(0, 64);
	}
	do {
		v9 = _vm->_eventsManager.BMOUSE();
		if (v9 == 1) {
			BTGAUCHE();
			v9 = 1;
		}
		if (v9 == 2)
			BTDROITE();
		TEST_INVENT();
		VERIFZONE();
		if (GOACTION == 1)
			PARADISE();
		if (!_vm->_globals.SORTIE) {
			_vm->_eventsManager.VBL();
			if (!_vm->_globals.SORTIE)
				continue;
		}
		v5 = 1;
	} while (v5 != 1);
	_vm->_graphicsManager.FADE_OUTW();
	if (!s3.empty())
		_vm->_graphicsManager.FIN_VISU();
	if (_vm->_globals.ECRAN == 61)
		SPRITE_OFF(0);
	CLEAR_ECRAN();
	_vm->_globals.iRegul = 0;
}

void ObjectsManager::PERSONAGE2(const Common::String &s1, const Common::String &s2, const Common::String &s3,
							   const Common::String &s4, int v) {
	Common::String v5; 
	int v6; 
	int v7; 
	int v8; 
	int v10; 
	int v11; 
	int v12; 
	int v13; 
	int xp, yp;

	v5 = s2;
	INVENTFLAG = 0;
	KEY_INVENT = 0;
	_vm->_objectsManager.verbe = 4;
	_vm->_globals.MAX_COMPTE = 6;
	_vm->_graphicsManager.ofscroll = 0;
	VIRE_INVENT = 0;
	v11 = 0;
	v12 = 0;
	_vm->_globals.PLAN_FLAG = 0;
	_vm->_graphicsManager.NOFADE = 0;
	_vm->_globals.NOMARCHE = 0;
	_vm->_globals.SORTIE = 0;
	_vm->_globals.AFFLI = 0;
	_vm->_globals.AFFIVBL = 0;
	_vm->_globals.NOT_VERIF = 1;
	_vm->_soundManager.WSOUND(v);
	_vm->_globals.iRegul = 1;
	if (!s1.empty())
		_vm->_graphicsManager.LOAD_IMAGE(s1);
	if (!s2.empty())
		_vm->_objectsManager.INILINK(s2);
	if (!s3.empty()) {
		_vm->_animationManager.CHARGE_ANIM(s3);
		_vm->_graphicsManager.VISU_ALL();
	}
	if (!s4.empty()) {
		if (!_vm->_globals.NOSPRECRAN)
			_vm->_graphicsManager.INI_ECRAN(s4);
		if (!s4.empty() && _vm->_globals.NOSPRECRAN == 1)
			_vm->_graphicsManager.INI_ECRAN2(s4);
	}
	_vm->_eventsManager.MOUSE_ON();
	_vm->_eventsManager.btsouris = 4;
	_vm->_graphicsManager.SETCOLOR3(252, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(253, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(251, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(254, 0, 0, 0);
	if (!_vm->_globals.PERSO_TYPE)
		goto LABEL_70;
	if (!_vm->_globals.SAUVEGARDE->data[svField122] && !_vm->_globals.SAUVEGARDE->data[svField356]) {
		FileManager::CONSTRUIT_SYSTEM("PERSO.SPR");
		FileManager::CHARGE_FICHIER2(_vm->_globals.NFICHIER, _vm->_globals.PERSO);
		_vm->_globals.PERSO_TYPE = 0;
	}
	if (!_vm->_globals.PERSO_TYPE) {
LABEL_70:
		if (_vm->_globals.SAUVEGARDE->data[svField122] == 1) {
			FileManager::CONSTRUIT_SYSTEM("HOPFEM.SPR");
			FileManager::CHARGE_FICHIER2(_vm->_globals.NFICHIER, _vm->_globals.PERSO);
			_vm->_globals.PERSO_TYPE = 1;
		}
	}
	if (_vm->_globals.PERSO_TYPE != 2 && _vm->_globals.SAUVEGARDE->data[svField356] == 1) {
		FileManager::CONSTRUIT_SYSTEM("PSAMAN.SPR");
		FileManager::CHARGE_FICHIER2(_vm->_globals.NFICHIER, _vm->_globals.PERSO);
		_vm->_globals.PERSO_TYPE = 2;
	}
	_vm->_globals.HOPKINS_DATA();
	if (!_vm->_globals.PERSO_TYPE)
		SPRITE(_vm->_globals.PERSO, _vm->_objectsManager.PERX, _vm->_objectsManager.PERY, 0, _vm->_objectsManager.PERI, 0, 0, 34, 190);
	if (_vm->_globals.PERSO_TYPE == 1)
		SPRITE(_vm->_globals.PERSO, _vm->_objectsManager.PERX, _vm->_objectsManager.PERY, 0, _vm->_objectsManager.PERI, 0, 0, 28, 155);
	if (_vm->_globals.PERSO_TYPE == 2)
		SPRITE(_vm->_globals.PERSO, _vm->_objectsManager.PERX, _vm->_objectsManager.PERY, 0, _vm->_objectsManager.PERI, 0, 0, 20, 127);
	_vm->_eventsManager.souris_xy(_vm->_objectsManager.PERX, _vm->_objectsManager.PERY);
	if (_vm->_graphicsManager.DOUBLE_ECRAN == 1)
		_vm->_graphicsManager.SCROLL = (signed __int16)XSPR(0) - 320;
	_vm->_objectsManager.VERIFTAILLE();
	SPRITE_ON(0);
	_vm->_globals.CACHE_ON();
	_vm->_globals.chemin = PTRNUL;
	_vm->_objectsManager.VERIFTAILLE();
	SPECIAL_INI(s2);
	_vm->_eventsManager.souris_n = 4;
	g_old_x = _vm->_objectsManager.PERX;
	g_old_y = _vm->_objectsManager.PERY;
	g_old_sens = -1;
	_vm->_globals.Compteur = 0;
	_vm->_globals.BPP_NOAFF = 1;
	v6 = 0;
	do {
		_vm->_eventsManager.VBL();
		++v6;
	} while (v6 <= 4);
	_vm->_globals.BPP_NOAFF = 0;
	_vm->_globals.iRegul = 1;
	if (!_vm->_graphicsManager.NOFADE)
		_vm->_graphicsManager.FADE_INW();
	_vm->_graphicsManager.NOFADE = 0;
	_vm->_eventsManager.CHANGE_MOUSE(4);
	v13 = 0;
	do {
		v7 = _vm->_eventsManager.BMOUSE();
		v8 = v7;
		if (v7) {
			if (v7 == 1) {
				if (_vm->_objectsManager.verbe == 16 && _vm->_eventsManager.btsouris == 16) {
					xp = _vm->_eventsManager.XMOUSE();
					yp = _vm->_eventsManager.YMOUSE();
					v10 = yp;
					if (v12 == xp) {
						if (v11 == yp) {
							_vm->_globals.chemin = PTRNUL;
							_vm->_objectsManager.PARADISE();
							if (_vm->_globals.SORTIE)
								v13 = 1;
						}
					}
					v12 = xp;
					v11 = v10;
				}
				_vm->_objectsManager.BTGAUCHE();
			}
			if (v8 == 2)
				_vm->_objectsManager.BTDROITE();
		}
		if (!_vm->_globals.SORTIE) {
			TEST_INVENT();
			_vm->_objectsManager.VERIFZONE();
			if (_vm->_globals.chemin == PTRNUL || (_vm->_objectsManager.GOHOME(), _vm->_globals.chemin == PTRNUL)) {
				if (_vm->_objectsManager.GOACTION == 1)
					_vm->_objectsManager.PARADISE();
			}
			SPECIAL_JEU();
			_vm->_eventsManager.VBL();
			if (!_vm->_globals.SORTIE)
				continue;
		}
		v13 = 1;
	} while (v13 != 1);
	if (_vm->_globals.SORTIE != 8 || _vm->_globals.ECRAN != 5 || _vm->_globals.HELICO != 1) {
		if (!_vm->_graphicsManager.NOFADE)
			_vm->_graphicsManager.FADE_OUTW();
		_vm->_graphicsManager.NOFADE = 0;
		SPRITE_OFF(0);
		_vm->_globals.AFFLI = 0;
		if (DEUXPERSO == 1) {
			SPRITE_OFF(1);
			DEUXPERSO = 0;
		}
		if (!s3.empty())
			_vm->_graphicsManager.FIN_VISU();
		CLEAR_ECRAN();
	} else {
		_vm->_globals.HELICO = 0;
	}
	_vm->_globals.iRegul = 0;
}

} // End of namespace Hopkins
