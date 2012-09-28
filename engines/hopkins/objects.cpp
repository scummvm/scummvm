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
	BOBTOUS = false;
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
	warning("VERIFZONE");
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
		_vm->_globals.B_CACHE_OFF(v2++);
	} while (v2 <= 15);
	_vm->_globals.B_CACHE_OFF(19);
	_vm->_globals.B_CACHE_OFF(20);
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
			if (_vm->_globals.SAUVEGARDE->field170 == 1 && !_vm->_globals.SAUVEGARDE->field171) {
				_vm->_globals.SAUVEGARDE->field171 = 1;
				_vm->_globals.NOPARLE = true;
				_vm->_talkManager.PARLER_PERSO("APPEL1.pe2");
				_vm->_globals.NOPARLE = false;
				v5 = 0;
			}
			if (_vm->_globals.SAUVEGARDE->field80 == 1 && !_vm->_globals.SAUVEGARDE->field172) {
				_vm->_globals.SAUVEGARDE->field172 = 1;
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
	warning("TODO: BTGAUCHE");
}

void ObjectsManager::PARADISE() {
	warning("TODO: PARADISE");
}

void ObjectsManager::CLEAR_ECRAN() {
	warning("TODO: CLEAR_ECRAN");
}

} // End of namespace Hopkins
