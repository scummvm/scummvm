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

ObjectsManager::ObjectsManager() {
	PRIORITY = 0;
	inventairex = inventairey = 0;
	inventairel = inventaireh = 0;
	old_cadx = old_cady = old_cadi = 0;
	cadx = cady = cadi = 0;
	SL_X = SL_Y = 0;
	I_old_x = I_old_y = 0;
	FLAG_VISIBLE_EFFACE = 0;
	Winventaire = PTRNUL;
	inventaire2 = PTRNUL;
	SL_SPR = PTRNUL;
	SL_SPR2 = PTRNUL;
	PERSO_ON = false;
	SL_FLAG = false;
	FLAG_VISIBLE = false;
	DESACTIVE_INVENT = false;
}

void ObjectsManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
}

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

void ObjectsManager::AFF_SPRITES() {
	int v0; 
	__int16 v1;
	__int16 v2;
	__int16 destX;
	__int16 destY;
	int v5; 
	__int16 v6;
	__int16 v7;
	__int16 v8;
	__int16 v9;
	int v10; 
	__int16 v11;
	uint16 *v12;
	int v13; 
	int v14; 
	int v15; 
	int v16; 
	int v17; 
	int v18; 
	int v19; 
	__int16 v20;
	__int16 v21;
	__int16 v23;
	__int16 v24;
	__int16 v25;
	__int16 v26;
	__int16 v27;
	__int16 v28;
	__int16 v29;
	__int16 v30;
	__int16 v31;
	__int16 v32;
	__int16 v33;
	signed __int16 v34;
	signed __int16 v35;
	signed __int16 v36;
	__int16 v37; 
	__int16 v38; 
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
	        
				_vm->_graphicsManager.SCOPY(_vm->_graphicsManager.VESA_SCREEN, v29, v24, _vm->_globals.Liste[v31].x2 + 4, 
					_vm->_globals.Liste[v31].y2 + 4, _vm->_graphicsManager.VESA_BUFFER, v8, v9);
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
		_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager.VESA_BUFFER, Winventaire, inventairex, inventairey, inventairel, inventaireh);
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
				BOITE(v38,
					_vm->_fontManager.Txt[v16].fieldC, _vm->_fontManager.Txt[v16].field4,
					_vm->_eventsManager.start_x + _vm->_fontManager.Txt[v16].field8, _vm->_fontManager.Txt[v16].fieldA);
			else
				BOITE(
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
	item.field6 = 0;
	item.field8 = 0;
}

void ObjectsManager::DEF_BOB(int idx) {
	int v2;
	int v3;
	int v4;
	__int16 v5;
	__int16 v6;
	__int16 v7;
	__int16 v8;
	__int16 v9;
	__int16 v10;
	int v11; 
	__int16 v12;
	__int16 v13;

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
		_vm->_globals.Liste2[v3].field6 = _vm->_globals.Bob[v4].field46;
		_vm->_globals.Liste2[v3].field8 = _vm->_globals.Bob[v4].field48;
		v5 = _vm->_globals.Liste2[v3].field2;
    
		v6 = _vm->_graphicsManager.min_x;
		if (v5 < _vm->_graphicsManager.min_x) {
			_vm->_globals.Liste2[v3].field6 -= _vm->_graphicsManager.min_x - v5;
			_vm->_globals.Liste2[v3].field2 = v6;
		}
    
		v7 = _vm->_globals.Liste2[v3].field4;
		v8 = _vm->_graphicsManager.min_y;
		if (v7 < _vm->_graphicsManager.min_y) {
			_vm->_globals.Liste2[v3].field8 -= _vm->_graphicsManager.min_y - v7;
			_vm->_globals.Liste2[v3].field4 = v8;
		}
    
		v9 = _vm->_globals.Liste2[v3].field2;
		if (_vm->_globals.Liste2[v3].field6 + v9 > _vm->_graphicsManager.max_x)
			_vm->_globals.Liste2[v3].field6 = _vm->_graphicsManager.max_x - v9;
		v10 = _vm->_globals.Liste2[v3].field4;
		if (_vm->_globals.Liste2[v3].field8 + v10 > _vm->_graphicsManager.max_y)
			_vm->_globals.Liste2[v3].field8 = _vm->_graphicsManager.max_y - v10;
    
		if (_vm->_globals.Liste2[v3].field6 <= 0 || _vm->_globals.Liste2[v3].field8 <= 0)
		_vm->_globals.Liste2[v3].field0 = 0;

		v11 = idx;
		if (_vm->_globals.Liste2[v11].field0 == 1)
			_vm->_graphicsManager.Ajoute_Segment_Vesa(
                 _vm->_globals.Liste2[v11].field2,
                 _vm->_globals.Liste2[v11].field4,
                 _vm->_globals.Liste2[v11].field2 + _vm->_globals.Liste2[v11].field6,
                 _vm->_globals.Liste2[v11].field4 + _vm->_globals.Liste2[v11].field8);
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

void ObjectsManager::SCBOB(int idx) {
	int v1;
	signed __int16 v2;
	signed __int16 v3;
	signed __int16 v4;
	signed __int16 v6;
	signed __int16 v7;
	__int16 v8; 
	signed __int16 v9; 

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
								if (v6 < (signed __int16)v1) {
									v1 = 0;
									if (v9 >= v3 && v9 <= (signed __int16)(_vm->_globals.Cache[idx].field6 + v3)) {
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

void ObjectsManager::VERIFCACHE() {
	__int16 v1;
	signed __int16 v2;
	signed __int16 v3;
	signed __int16 v4;
	__int16 v5; 
	__int16 v6; 
	__int16 v7; 
	__int16 v8; 
	__int16 v9; 
	__int16 v10;
	__int16 v11;

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
	__int16 v0; 
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
    
		if (_vm->_globals.SAUVEGARDE->field357 == 1) {
			if (_vm->_globals.SAUVEGARDE->field353 == 1)
				_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.TETE, 832, 325, 0, 0, 0, 0);
			if (_vm->_globals.SAUVEGARDE->field355 == 1)
				_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.TETE, 866, 325, 1, 0, 0, 0);
			_vm->_graphicsManager.Ajoute_Segment_Vesa(532, 25, 560, 60);
			_vm->_graphicsManager.Ajoute_Segment_Vesa(566, 25, 594, 60);
		}
		if (_vm->_globals.SAUVEGARDE->field356 == 1) {
			_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.TETE, 832, 325, 0, 0, 0, 0);
			_vm->_graphicsManager.Ajoute_Segment_Vesa(532, 25, 560, 60);
		}

		if (_vm->_globals.SAUVEGARDE->field354 == 1) {
			_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.TETE, 832, 325, 0, 0, 0, 0);
			_vm->_graphicsManager.Ajoute_Segment_Vesa(532, 25, 560, 60);
		}
	}
}

void ObjectsManager::DEF_SPRITE(int idx) {
	int v2; 
	__int16 v3;
	__int16 v4;
	int v5; 
	int v6; 
	__int16 v7; 
	__int16 v8; 
	__int16 v9; 
	__int16 v10;
	__int16 v11;
	__int16 v12;
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
		_vm->_globals.Liste[v5].x2 = Sprite[v6].field30;
		_vm->_globals.Liste[v5].y2 = Sprite[v6].field32;
		v7 = _vm->_globals.Liste[v5].field2;
		v8 = _vm->_graphicsManager.min_x;
		
		if (v7 < _vm->_graphicsManager.min_x) {
			_vm->_globals.Liste[v5].x2 -= _vm->_graphicsManager.min_x - v7;
			_vm->_globals.Liste[v5].field2 = v8;
		}

		v9 = _vm->_globals.Liste[v5].field4;
		v10 = _vm->_graphicsManager.min_y;
		if (v9 < _vm->_graphicsManager.min_y) {
			_vm->_globals.Liste[v5].y2 -= _vm->_graphicsManager.min_y - v9;
			_vm->_globals.Liste[v5].field4 = v10;
		}
		v11 = _vm->_globals.Liste[v5].field2;
		if (_vm->_globals.Liste[v5].x2 + v11 > _vm->_graphicsManager.max_x)
			_vm->_globals.Liste[v5].x2 = _vm->_graphicsManager.max_x - v11;
		v12 = _vm->_globals.Liste[v5].field4;
		if ( _vm->_globals.Liste[v5].y2 + v12 > _vm->_graphicsManager.max_y)
			_vm->_globals.Liste[v5].y2 = _vm->_graphicsManager.max_y - v12;
		if ( _vm->_globals.Liste[v5].x2 <= 0 || _vm->_globals.Liste[v5].y2 <= 0)
			_vm->_globals.Liste[v5].field0 = 0;

		v13 = idx;
		if (_vm->_globals.Liste[v13].field0 == 1)
			_vm->_graphicsManager.Ajoute_Segment_Vesa(
			_vm->_globals.Liste[v13].field2,
				_vm->_globals.Liste[v13].field4,
				_vm->_globals.Liste[v13].field2 + _vm->_globals.Liste[v13].x2,
				_vm->_globals.Liste[v13].field4 + _vm->_globals.Liste[v13].y2);
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
	warning("TODO: CALCUL_SPRITE");
}

void ObjectsManager::AvantTri(int a1, int a2, int a3) {
	warning("TODO: AvantTri");
}

void ObjectsManager::AFF_BOB_ANIM() {
	warning("TODO: AFF_BOB_ANIM");
}

void ObjectsManager::AFF_VBOB() {
	warning("TODO: AFF_VBOB");
}

void ObjectsManager::BOITE(int a1, int a2, int a3, int a4, int a5) {
	warning("TODO: BOITE");
}

} // End of namespace Hopkins
