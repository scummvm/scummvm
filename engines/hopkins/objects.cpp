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
	for (int i = 0; i < 6; ++i) {
		Common::fill((byte *)&Sprite[i], (byte *)&Sprite[i] + sizeof(SpriteItem), 0);
	}

	PRIORITY = 0;
	old_cadx = old_cady = old_cadi = 0;
	cadx = cady = cadi = 0;
	SL_X = SL_Y = 0;
	I_old_x = I_old_y = 0;
	g_old_x = g_old_y = 0;
	FLAG_VISIBLE_EFFACE = 0;
	SL_SPR = g_PTRNUL;
	SL_SPR2 = g_PTRNUL;
	sprite_ptr = g_PTRNUL;
	S_old_spr = g_PTRNUL;
	PERSO_ON = false;
	SL_FLAG = false;
	SL_MODE = false;
	FLAG_VISIBLE = false;
	DESACTIVE_CURSOR = 0;
	BOBTOUS = false;
	my_anim = 0;
	GOACTION = 0;
	NUMZONE = 0;
	ARRET_PERSO_FLAG = 0;
	ARRET_PERSO_NUM = 0;
	FORCEZONE = 0;
	CHANGEVERBE = 0;
	verbe = 0;
	Vold_taille = 0;
	SPEED_FLAG = false;
	SPEED_X = SPEED_Y = 0;
	SPEED_IMAGE = 0;
	SPEED_PTR = g_PTRNUL;
	DERLIGNE = 0;
	A_ANIM = 0;
	MA_ANIM = 0;
	MA_ANIM1 = 0;
	A_DEPA = 0;
	MAX_DEPA = 0;
	MAX_DEPA1 = 0;
	CH_TETE = 0;
	T_RECTIF = 0;
	DESACTIVE = 0;
	DEUXPERSO = 0;
	PERX = PERY = 0;
	PERI = 0;
	RECALL = 0;
	PTAILLE = 0;
	PEROFX = 0;
	PEROFY = 0;
	OBSSEUL = 0;
	NVVERBE = 0;
	NVZONE = 0;
	S_old_ani = 0; 
	S_old_ret = 0;
	nouveau_x = nouveau_y = 0;
	nouveau_sens = 0;
	nouveau_anim = 0;
}

void ObjectsManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
}

// Change Object
byte *ObjectsManager::CHANGE_OBJET(int objIndex) {
	byte *result = CAPTURE_OBJET(objIndex, 1);
	_vm->_globals.Bufferobjet = result;
	_vm->_globals.Nouv_objet = 1;
	_vm->_globals.OBJET_EN_COURS = objIndex;
	return result;
}

byte *ObjectsManager::CAPTURE_OBJET(int objIndex, int mode) {
	byte *dataP;

	dataP = 0;
	int val1 = _vm->_globals.ObjetW[objIndex].field0;
	int val2 = _vm->_globals.ObjetW[objIndex].field1;

	if (mode == 1)
	    ++val2;
	if (val1 != _vm->_globals.NUM_FICHIER_OBJ) {
		if (_vm->_globals.ADR_FICHIER_OBJ != g_PTRNUL)
			ObjectsManager::DEL_FICHIER_OBJ();
		if (val1 == 1) {
			_vm->_fileManager.CONSTRUIT_SYSTEM("OBJET1.SPR");
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
		if (dataP == g_PTRNUL)
			error("CAPTURE_OBJET");
			
		capture_mem_sprite(_vm->_globals.ADR_FICHIER_OBJ, dataP, val2);
		break;

	case 1:
		sprite_alone(_vm->_globals.ADR_FICHIER_OBJ, _vm->_globals.Bufferobjet, val2);
		dataP = _vm->_globals.Bufferobjet;
		break;

	case 3:
		capture_mem_sprite(_vm->_globals.ADR_FICHIER_OBJ, _vm->_globals.INVENTAIRE_OBJET, val2);
		dataP = _vm->_globals.INVENTAIRE_OBJET;
		break;

	default:
		break;
	}

	return dataP;
}

// Delete Object
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
		v3 += (int16)READ_LE_UINT16(v3) + 16;
  
	const byte *v5 = v3 + 8;
	int result = (int16)READ_LE_UINT16(v5);
	if (isSize)
		result = (int16)READ_LE_UINT16(v5 + 4);

	return result;
}

int ObjectsManager::get_offsety(const byte *spriteData, int spriteIndex, bool isSize) {
	const byte *v3 = spriteData + 3;
	for (int i = spriteIndex; i; --i)
		v3 += (int16)READ_LE_UINT16(v3) + 16;
  
	const byte *v5 = v3 + 10;
	int result = (int16)READ_LE_UINT16(v5);
	if (isSize)
		result = (int16)READ_LE_UINT16(v5 + 4);

	return result;
}

// Get Width
int ObjectsManager::Get_Largeur(const byte *objectData, int idx) {
	const byte *rectP = objectData + 3;
	for (int i = idx; i; --i)
		rectP += READ_LE_UINT32(rectP) + 16;
	
	return (int16)READ_LE_UINT16(rectP + 4);
}

// Get height
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
	int result = (int16)READ_LE_UINT16(objP) * (int16)READ_LE_UINT16(objP + 2);

	memcpy(sprite + 3, objP - 4, result + 16);
	return result;
}

byte *ObjectsManager::DEL_FICHIER_OBJ() {
	_vm->_globals.NUM_FICHIER_OBJ = 0;
	if (_vm->_globals.ADR_FICHIER_OBJ != g_PTRNUL)
		_vm->_globals.ADR_FICHIER_OBJ = _vm->_fileManager.LIBERE_FICHIER(_vm->_globals.ADR_FICHIER_OBJ);
  
	byte *result = g_PTRNUL;
	_vm->_globals.ADR_FICHIER_OBJ = g_PTRNUL;
	return result;
}

// Load Sprite
byte *ObjectsManager::CHARGE_SPRITE(const Common::String &file) {
	_vm->_fileManager.DMESS1();
	return _vm->_fileManager.CHARGE_FICHIER(file);
}

int ObjectsManager::capture_mem_sprite(const byte *objectData, byte *sprite, int objIndex) {
	const byte *objP = objectData + 3;
	for (int i = objIndex; i; --i) {
		objP += READ_LE_UINT32(objP) + 16;
	}

	objP += 4;
	int result = (int16)READ_LE_UINT16(objP) * (int16)READ_LE_UINT16(objP + 2);

	memcpy(sprite, objP + 12, result);
	return result;
}

// Add Object
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

// Display Sprite
void ObjectsManager::AFF_SPRITES() {
	int v1;
	int v2;
	int destX;
	int destY;
	int v6;
	int v7;
	int v8;
	int v9;
	int v11;
	uint16 *v12;
	int v13; 
	int v20;
	int v21;
	int y1_1;
	int y1_2;
	int v25;
	int v26;
	int v27;
	int x1_1;
	int x1_2;
	int v33;
	int v34;
	int v35;
	uint16 arr[50];

	// Handle copying any background areas that text are going to be drawn on
	_vm->_globals.NBTRI = 0;
	for (int idx = 0; idx <= 10; ++idx) {
		if (_vm->_fontManager.ListeTxt[idx].enabled && _vm->_fontManager.Txt[idx].field3FC != 2) {
			v1 = _vm->_fontManager.ListeTxt[idx].xp;
			x1_1 = v1 - 2;
      
			if ((int16)(v1 - 2) < _vm->_graphicsManager.min_x)
				x1_1 = _vm->_graphicsManager.min_x;
			v2 = _vm->_fontManager.ListeTxt[idx].yp;
			y1_1 = v2 - 2;
      
			if ((int16)(v2 - 2) < _vm->_graphicsManager.min_y)
				y1_1 = _vm->_graphicsManager.min_y;
			destX = v1 - 2;
			if (destX < _vm->_graphicsManager.min_x)
				destX = _vm->_graphicsManager.min_x;
			destY = v2 - 2;
			if (destY < _vm->_graphicsManager.min_y)
				destY = _vm->_graphicsManager.min_y;
			
			_vm->_graphicsManager.SCOPY(_vm->_graphicsManager.VESA_SCREEN, x1_1, y1_1,
				_vm->_fontManager.ListeTxt[idx].width + 4, _vm->_fontManager.ListeTxt[idx].height + 4,
				_vm->_graphicsManager.VESA_BUFFER,
				destX, destY);
			_vm->_fontManager.ListeTxt[idx].enabled = false;
		}
	}

	if (!PERSO_ON) {
		for (int idx = 0; idx < 5; ++idx) {
			if (_vm->_globals.Liste[idx].field0) {
				v6 = _vm->_globals.Liste[idx].field2;
				x1_2 = v6 - 2;
				if ((int16)(v6 - 2) < _vm->_graphicsManager.min_x)
					x1_2 = _vm->_graphicsManager.min_x;
				v7 = _vm->_globals.Liste[idx].field4;
				y1_2 = v7 - 2;
				if ((int16)(v7 - 2) < _vm->_graphicsManager.min_y)
					y1_2 = _vm->_graphicsManager.min_y;
				v8 = v6 - 2;
				if (v8 < _vm->_graphicsManager.min_x)
					v8 = _vm->_graphicsManager.min_x;
				v9 = v7 - 2;
				if (v9 < _vm->_graphicsManager.min_y)
					v9 = _vm->_graphicsManager.min_y;
	        
				_vm->_graphicsManager.SCOPY(_vm->_graphicsManager.VESA_SCREEN, x1_2, y1_2, 
					_vm->_globals.Liste[idx].width + 4, _vm->_globals.Liste[idx].height + 4, 
					_vm->_graphicsManager.VESA_BUFFER, v8, v9);
				_vm->_globals.Liste[idx].field0 = 0;
			}
		} 
	}
  
	AFF_BOB_ANIM();
	AFF_VBOB();

	if (!PERSO_ON) {
		// Handle drawing characters on the screen
		for (int idx = 0; idx < 5; ++idx) {
			_vm->_globals.Liste[idx].field0 = 0;
			if (Sprite[idx].field0 == 1) {
				CALCUL_SPRITE(idx);
				if (Sprite[idx].field2A == 1)
					AvantTri(TRI_SPRITE, idx, Sprite[idx].field32 + Sprite[idx].field2E);
			}
		} 
    
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
				switch (_vm->_globals.Tri[v13].triMode) {
				case TRI_BOB:
					DEF_BOB(_vm->_globals.Tri[v13].index);
					break;
				case TRI_SPRITE:
					DEF_SPRITE(_vm->_globals.Tri[v13].index);
					break;
				case TRI_CACHE:
					DEF_CACHE(_vm->_globals.Tri[v13].index);
					break;
				default:
					break;
				}
				_vm->_globals.Tri[v13].triMode = TRI_NONE;
				++v35;
			} while (v35 < _vm->_globals.NBTRI + 1);
		}
	} else {
		if (_vm->_globals.NBTRI + 1 > 1) {
			for (int idx = 1; idx < (_vm->_globals.NBTRI + 1); ++idx) {
				switch (_vm->_globals.Tri[idx].triMode) {
				case TRI_BOB:
					DEF_BOB(_vm->_globals.Tri[idx].index);
					break;
				case TRI_SPRITE:
					DEF_SPRITE(_vm->_globals.Tri[idx].index);
					break;
				case TRI_CACHE:
					DEF_CACHE(_vm->_globals.Tri[idx].index);
					break;
				default:
					break;
				}
				_vm->_globals.Tri[idx].triMode = TRI_NONE;
			}
		}
	}

	// Reset the Tri array
	for (int idx = 0; idx < 50; ++idx) {
		_vm->_globals.Tri[idx].triMode = TRI_NONE;
		_vm->_globals.Tri[idx].index = 0;
		_vm->_globals.Tri[idx].field4 = 0;
		_vm->_globals.Tri[idx].field6 = 0;
	}
  
	_vm->_globals.NBTRI = 0;
	if (_vm->_dialogsManager.AFFINVEN) {
		_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager.VESA_BUFFER, _vm->_dialogsManager.Winventaire, _vm->_dialogsManager.inventairex, _vm->_dialogsManager.inventairey, _vm->_dialogsManager.inventairel, _vm->_dialogsManager.inventaireh);
		if (old_cadx && old_cady)
			_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager.VESA_BUFFER, _vm->_dialogsManager.inventaire2, old_cadx + 300, old_cady + 300, old_cadi + 1);
		if (cadx && cady)
			_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager.VESA_BUFFER, _vm->_dialogsManager.inventaire2, cadx + 300, cady + 300, cadi);
		_vm->_graphicsManager.Ajoute_Segment_Vesa(_vm->_dialogsManager.inventairex, _vm->_dialogsManager.inventairey, _vm->_dialogsManager.inventairex + _vm->_dialogsManager.inventairel, _vm->_dialogsManager.inventairey + _vm->_dialogsManager.inventaireh);
	}
  
	if (SL_FLAG == 1) {
		_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager.VESA_BUFFER, SL_SPR, _vm->_eventsManager.start_x + 183, 60, 0x112u, 353);
		if (SL_X && SL_Y)
			_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager.VESA_BUFFER, SL_SPR2, SL_X + _vm->_eventsManager.start_x + 300, SL_Y + 300, 0);
    
		_vm->_graphicsManager.Ajoute_Segment_Vesa(_vm->_eventsManager.start_x + 183, 60, _vm->_eventsManager.start_x + 457, 413);
	}

	// If the Options dialog is activated, draw the elements
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

	// Loop to draw any on-screen text
	for (int idx = 0; idx <= 10; ++idx) {
		if (_vm->_fontManager.Txt[idx].textOn) {
			if ((uint16)(_vm->_fontManager.Txt[idx].field3FC - 2) > 1)
				_vm->_fontManager.BOITE(idx,
					_vm->_fontManager.Txt[idx].messageId, _vm->_fontManager.Txt[idx].filename,
					_vm->_eventsManager.start_x + _vm->_fontManager.Txt[idx].xp, _vm->_fontManager.Txt[idx].yp);
			else
				_vm->_fontManager.BOITE(
				  idx,
				  _vm->_fontManager.Txt[idx].messageId,
				  _vm->_fontManager.Txt[idx].filename,
				  _vm->_fontManager.Txt[idx].xp,
				  _vm->_fontManager.Txt[idx].yp);
			_vm->_fontManager.ListeTxt[idx].enabled = true;
			
			if ((uint16)(_vm->_fontManager.Txt[idx].field3FC - 2) > 1)
				_vm->_fontManager.ListeTxt[idx].xp = _vm->_eventsManager.start_x + _vm->_fontManager.Txt[idx].xp;
			else
				_vm->_fontManager.ListeTxt[idx].xp = _vm->_fontManager.Txt[idx].xp;
      
			_vm->_fontManager.ListeTxt[idx].yp = _vm->_fontManager.Txt[idx].yp;
			_vm->_fontManager.ListeTxt[idx].width = _vm->_fontManager.Txt[idx].width;
			_vm->_fontManager.ListeTxt[idx].height = _vm->_fontManager.Txt[idx].height;

			if (_vm->_fontManager.ListeTxt[idx].xp < _vm->_graphicsManager.min_x)
				_vm->_fontManager.ListeTxt[idx].xp = _vm->_graphicsManager.min_x - 1;
			if (_vm->_fontManager.ListeTxt[idx].yp < _vm->_graphicsManager.min_y)
				_vm->_fontManager.ListeTxt[idx].yp = _vm->_graphicsManager.min_y - 1;

			v20 = _vm->_fontManager.ListeTxt[idx].xp;
			if (_vm->_fontManager.ListeTxt[idx].width + v20 > _vm->_graphicsManager.max_x)
				_vm->_fontManager.ListeTxt[idx].width = _vm->_graphicsManager.max_x - v20;
			v21 = _vm->_fontManager.ListeTxt[idx].yp;
			if (_vm->_fontManager.ListeTxt[idx].height + v21 > _vm->_graphicsManager.max_y)
				_vm->_fontManager.ListeTxt[idx].height = _vm->_graphicsManager.max_y - v21;
			if (_vm->_fontManager.ListeTxt[idx].width <= 0 || _vm->_fontManager.ListeTxt[idx].height <= 0)
				_vm->_fontManager.ListeTxt[idx].enabled = false;
		}
	}
  
	_vm->_dialogsManager.INVENT_ANIM();
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
	bob.field4 = g_PTRNUL;
	bob.field8 = 0;
	bob.fieldA = 0;
	bob.fieldC = 0;
	bob.fieldE = 0;
	bob.field10 = 0;
	bob.field12 = 0;
	bob.field14 = 0;
	bob.field16 = 0;
	bob.field18 = g_PTRNUL;
	bob.field1C = 0;
	bob.field1E = 0;
	bob.field20 = 0;
	bob.field22 = 0;
	bob.field24 = 0;
	bob.field26 = 0;
	bob.field28 = 0;
	bob.field2A = 0;
	bob.field2C = 0;
	bob.field30 = g_PTRNUL;
	bob.field34 = 0;
	bob.field36 = 0;
	bob.field38 = 0;
	bob.field46 = 0;

	item.field0 = 0;
	item.xp = 0;
	item.yp = 0;
	item.width = 0;
	item.height = 0;
}

void ObjectsManager::DEF_BOB(int idx) {
	int v5;
	int v6;
	int v7;
	int v8;
	int v9;
	int v10;
	int xp;
	int yp;

	if (_vm->_globals.Bob[idx].field3C) {
		xp = _vm->_globals.Bob[idx].field3E;
		yp = _vm->_globals.Bob[idx].field40;

		if (_vm->_globals.Bob[idx].field3A)
			_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.Bob[idx].field4, 
				xp + 300, yp + 300, _vm->_globals.Bob[idx].fieldC);
		else
			_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager.VESA_BUFFER, 
				_vm->_globals.Bob[idx].field4, xp + 300, 
				_vm->_globals.Bob[idx].field40 + 300, _vm->_globals.Bob[idx].fieldC,
				_vm->_globals.Bob[idx].field4A, _vm->_globals.Bob[idx].field48,
				_vm->_globals.Bob[idx].field38);

		_vm->_globals.Liste2[idx].field0 = 1;
		_vm->_globals.Liste2[idx].xp = xp;
		_vm->_globals.Liste2[idx].yp = yp;
		
		_vm->_globals.Liste2[idx].width = _vm->_globals.Bob[idx].field42;
		_vm->_globals.Liste2[idx].height = _vm->_globals.Bob[idx].field44;

		v5 = _vm->_globals.Liste2[idx].xp;
		v6 = _vm->_graphicsManager.min_x;
		if (v5 < _vm->_graphicsManager.min_x) {
			_vm->_globals.Liste2[idx].width -= _vm->_graphicsManager.min_x - v5;
			_vm->_globals.Liste2[idx].xp = v6;
		}
    
		v7 = _vm->_globals.Liste2[idx].yp;
		v8 = _vm->_graphicsManager.min_y;
		if (v7 < _vm->_graphicsManager.min_y) {
			_vm->_globals.Liste2[idx].height -= _vm->_graphicsManager.min_y - v7;
			_vm->_globals.Liste2[idx].yp = v8;
		}
    
		v9 = _vm->_globals.Liste2[idx].xp;
		if (_vm->_globals.Liste2[idx].width + v9 > _vm->_graphicsManager.max_x)
			_vm->_globals.Liste2[idx].width = _vm->_graphicsManager.max_x - v9;
		v10 = _vm->_globals.Liste2[idx].yp;
		if (_vm->_globals.Liste2[idx].height + v10 > _vm->_graphicsManager.max_y)
			_vm->_globals.Liste2[idx].height = _vm->_graphicsManager.max_y - v10;
    
		if (_vm->_globals.Liste2[idx].width <= 0 || _vm->_globals.Liste2[idx].height <= 0)
		_vm->_globals.Liste2[idx].field0 = 0;

		if (_vm->_globals.Liste2[idx].field0 == 1)
			_vm->_graphicsManager.Ajoute_Segment_Vesa(
                 _vm->_globals.Liste2[idx].xp,
                 _vm->_globals.Liste2[idx].yp,
                 _vm->_globals.Liste2[idx].xp + _vm->_globals.Liste2[idx].width,
                 _vm->_globals.Liste2[idx].yp + _vm->_globals.Liste2[idx].height);
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
		v1 = (int16)READ_LE_UINT16(data);
		v9 = (int16)READ_LE_UINT16(data + 2);
		v8 = (int16)READ_LE_UINT16(data + 4);
		v7 = (int16)READ_LE_UINT16(data + 6);
		v6 = (int16)READ_LE_UINT16(data + 8);
		if ((int16)READ_LE_UINT16(data)) {
			if (_vm->_globals.Bank[v1].field4) {
				if (!v9)
					v9 = 1;
				if (!v6)
					v6 = -1;
			
				if ((int16)READ_LE_UINT16(data + 24)) {
					_vm->_globals.Bob[idx].field3A = 0;
          
					if (_vm->_globals.Bank[v1].fileHeader == 1) {
						_vm->_globals.Bob[idx].field3A = 1;
						_vm->_globals.Bob[idx].field36 = 0;
						_vm->_globals.Bob[idx].field38 = 0;
					}

					_vm->_globals.Bob[idx].field18 = _vm->_globals.Bqe_Anim[idx].data;
					_vm->_globals.Bob[idx].field0 = 10;
					_vm->_globals.Bob[idx].field4 = _vm->_globals.Bank[v1].data;

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
    
		v13 = _vm->_globals.Bob[idx].field8 - v22;
		v14 = _vm->_globals.Bob[idx].fieldA - v17;
		_vm->_globals.Bob[idx].field3C = 1;
		_vm->_globals.Bob[idx].field3E = v13;
		_vm->_globals.Bob[idx].field40 = v14;
		_vm->_globals.Bob[idx].field48 = v21;
		_vm->_globals.Bob[idx].field4A = v20;

		_vm->_globals.Liste2[idx].field0 = 1;
		_vm->_globals.Liste2[idx].xp = v13;
		_vm->_globals.Liste2[idx].yp = v14;

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
					if (Sprite[v10].spriteIndex != 250) {
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
				
				AvantTri(TRI_CACHE, v8, v5);
				_vm->_globals.Cache[v8].fieldA = 1;
				_vm->_globals.Cache[v8].field10 = 1;
			}
		}
		++v8;
	} while (v8 <= 19);
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
			v3 + 300, v4 + 300, Sprite[v2].spriteIndex);
		else
			_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager.VESA_BUFFER, Sprite[v2].spriteData, 
				v3 + 300, v4 + 300,  Sprite[v2].spriteIndex, Sprite[v2].field36, Sprite[v2].field34, Sprite[v2].fieldE);

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

// Compute Sprite
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

	Sprite[idx].field2A = 0;
	int spriteIndex = Sprite[idx].spriteIndex;
	if (spriteIndex != 250) {
		if (Sprite[idx].fieldE) {
			v5 = get_offsetx(Sprite[idx].spriteData, spriteIndex, 1);
			v22 = Sprite[idx].field12 + v5;
			v4 = Sprite[idx].field12 + v5;
			v6 = get_offsety(Sprite[idx].spriteData, Sprite[idx].spriteIndex, 1);
		} else {
			v3 = get_offsetx(Sprite[idx].spriteData, spriteIndex, 0);
			v22 = Sprite[idx].field12 + v3;
			v4 = Sprite[idx].field12 + v3;
			v6 = get_offsety(Sprite[idx].spriteData, Sprite[idx].spriteIndex, 0);
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
    
		v15 = Sprite[idx].spritePos.x - v22;
		v16 = Sprite[idx].spritePos.y - v7;
		Sprite[idx].field2C = v15;
		Sprite[idx].field2E = v16;
		Sprite[idx].field2A = 1;
		Sprite[idx].field34 = zoomPercent;
		Sprite[idx].field36 = reducePercent;
	
		v17 = idx;
		_vm->_globals.Liste[v17].field0 = 1;
		_vm->_globals.Liste[v17].field2 = v15;
		_vm->_globals.Liste[v17].field4 = v16;
		width = Get_Largeur(Sprite[idx].spriteData, Sprite[idx].spriteIndex);
		height = Get_Hauteur(Sprite[idx].spriteData, Sprite[idx].spriteIndex);

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

// Before Sort
int ObjectsManager::AvantTri(TriMode triMode, int index, int a3) {
	int result;

	++_vm->_globals.NBTRI;
	if (_vm->_globals.NBTRI > 48)
		error("NBTRI too high");
  
	result = _vm->_globals.NBTRI;
	_vm->_globals.Tri[result].triMode = triMode;
	_vm->_globals.Tri[result].index = index;
	_vm->_globals.Tri[result].field4 = a3;
  
	return result;
}

// Display BOB Anim
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
			if (_vm->_globals.Bob[idx].field18 == g_PTRNUL || _vm->_globals.Bob[idx].field16 || v1 <= 0)
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
			_vm->_globals.Bob[idx].field8 = (int16)READ_LE_UINT16(v20 + 2 * v24);
			if (_vm->_globals.BL_ANIM[idx].v1 == 1)
				_vm->_globals.Bob[idx].field8 = _vm->_globals.BL_ANIM[idx].v2;
			if ( PERSO_ON == 1 && idx > 20 )
				_vm->_globals.Bob[idx].field8 += _vm->_eventsManager.start_x;
      
			_vm->_globals.Bob[idx].fieldA = (int16)READ_LE_UINT16(v20 + 2 * v24 + 2);
			_vm->_globals.Bob[idx].field12 = (int16)READ_LE_UINT16(v20 + 2 * v24 + 4);
			_vm->_globals.Bob[idx].field36 = (int16)READ_LE_UINT16(v20 + 2 * v24 + 6);
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
					_vm->_globals.Bob[idx].field8 = (int16)READ_LE_UINT16(v21);
					
					if (_vm->_globals.BL_ANIM[idx].v1 == 1)
						_vm->_globals.Bob[idx].field8 = _vm->_globals.BL_ANIM[idx].v2;
					if (PERSO_ON == 1 && idx > 20)
						_vm->_globals.Bob[idx].field8 += _vm->_eventsManager.start_x;

					_vm->_globals.Bob[idx].fieldA = (int16)READ_LE_UINT16(v21 + 2);
					_vm->_globals.Bob[idx].field12 = (int16)READ_LE_UINT16(v21 + 4);
					_vm->_globals.Bob[idx].field36 = (int16)READ_LE_UINT16(v21 + 6);
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
									_vm->_globals.Liste2[v27].xp, _vm->_globals.Liste2[v27].yp,
									_vm->_globals.Liste2[v27].width, _vm->_globals.Liste2[v27].height,
									_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.Liste2[v27].xp,
									_vm->_globals.Liste2[v27].yp);
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
						_vm->_globals.Liste2[v27].xp, _vm->_globals.Liste2[v27].yp,
						_vm->_globals.Liste2[v27].width, _vm->_globals.Liste2[v27].height,
						_vm->_graphicsManager.VESA_BUFFER,
						_vm->_globals.Liste2[v27].xp, _vm->_globals.Liste2[v27].yp);
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
			AvantTri(TRI_BOB, v28, v19);
		}
	} while (v28 != 35);
}

// Display VBOB
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
      
			if (g_PTRNUL != _vm->_globals.VBob[idx].field10)
				_vm->_globals.dos_free2(_vm->_globals.VBob[idx].field10);
      
			_vm->_globals.VBob[idx].field4 = 0;
			_vm->_globals.VBob[idx].field10 = g_PTRNUL;
			_vm->_globals.VBob[idx].field0 = g_PTRNUL;
			_vm->_globals.VBob[idx].field6 = 0;
			_vm->_globals.VBob[idx].field8 = 0;
			_vm->_globals.VBob[idx].field14 = 0;
			_vm->_globals.VBob[idx].field16 = 0;
			_vm->_globals.VBob[idx].fieldA = 0;
			_vm->_globals.VBob[idx].field18 = 0;
			_vm->_globals.VBob[idx].field1C = g_PTRNUL;
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
      
			if (g_PTRNUL != _vm->_globals.VBob[idx].field10)
				_vm->_globals.dos_free2(_vm->_globals.VBob[idx].field10);
      
			_vm->_globals.VBob[idx].field10 = g_PTRNUL;
			_vm->_globals.VBob[idx].field14 = _vm->_globals.VBob[idx].field6;
			_vm->_globals.VBob[idx].field16 = _vm->_globals.VBob[idx].field8;
			_vm->_globals.VBob[idx].field18 = _vm->_globals.VBob[idx].fieldA;
		}

		if (_vm->_globals.VBob[idx].field4 == 1) {
			width = Get_Largeur(_vm->_globals.VBob[idx].field0, _vm->_globals.VBob[idx].fieldA);
			height = Get_Hauteur(_vm->_globals.VBob[idx].field0, _vm->_globals.VBob[idx].fieldA);
      
			if (g_PTRNUL != _vm->_globals.VBob[idx].field10)
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
	return Sprite[idx].spritePos.x;
}

int ObjectsManager::YSPR(int idx) {
	if (idx > 5)
		error("request of the Coord. y a sprite > MAX_SPRITE.");
	return Sprite[idx].spritePos.y;
}

void ObjectsManager::SPRITE_NOW(const byte *spriteData, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
	_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager.VESA_BUFFER, spriteData, a2, a3, a5, a6, a7, a8);
}

void ObjectsManager::CLEAR_SPR() {
	int idx;

	idx = 0;
	do {
		Sprite[idx].field1C = g_PTRNUL;
		Sprite[idx].spriteData = g_PTRNUL;
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
		error("Attempt to display a sprite > MAX_SPRITE.");
  
	Sprite[idx].field0 = 1;
}

void ObjectsManager::SPRITE(const byte *spriteData, int xp, int yp, int idx, int spriteIndex, int a6, int a7, int a8, int a9) {
	if (idx > 5)
		error("Attempt to display a sprite > MAX_SPRITE.");
	Sprite[idx].spriteData = spriteData;
	Sprite[idx].spritePos = Common::Point(xp, yp);
	Sprite[idx].spriteIndex = spriteIndex;
	Sprite[idx].fieldC = a6;
	Sprite[idx].field12 = a8;
	Sprite[idx].field14 = a9;
	Sprite[idx].field1C = g_PTRNUL;
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
	if (Sprite[idx].field1C != g_PTRNUL)
		_vm->_globals.dos_free2(Sprite[idx].field1C);
	Sprite[idx].field1C = g_PTRNUL;
}

void ObjectsManager::SPRITE_GEL(int idx) {
	Sprite[idx].field0 = 3;
}

int ObjectsManager::SXSPR(int idx) {
	if (idx > 5)
		error("request of the size of a sprite x > MAX_SPRITE.");
	return Get_Largeur(Sprite[idx].spriteData, Sprite[idx].spriteIndex);
}

int ObjectsManager::SYSPR(int idx) {
	if (idx > 5)
		error("request of the size of a sprite y > MAX_SPRITE.");
	return Get_Hauteur(Sprite[idx].spriteData, Sprite[idx].spriteIndex);
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
	Sprite[idx].spritePos.x = xp;
}

void ObjectsManager::SETANISPR(int idx, int spriteIndex) {
	if (idx > 5)
		error("Set the Coord. x a sprite> MAX_SPRITE.");

	Sprite[idx].spriteIndex = spriteIndex;
}

void ObjectsManager::SETYSPR(int idx, int yp) {
	if ( idx > 5 )
		error("Set the Coord. y a sprite> MAX_SPRITE.");
	Sprite[idx].spritePos.y = yp;
}

// Set Sprite Size
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
		if (_vm->_globals.NOMARCHE || (_vm->_globals.chemin == (int16 *)g_PTRNUL) || v3 > 4u) {
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
	int16 v0; 
	int16 v1; 
	int16 v3; 
	int16 v4; 
	unsigned int v5;
	unsigned int v6;
	unsigned int v7;
	unsigned int v8;
	int16 v9; 
	int16 v10;
	unsigned int v11;
	unsigned int v12;
	unsigned int v13;
	unsigned int v14;
	int v15; 
	int16 v16; 
	unsigned int v17; 
	unsigned int v18; 
	int v19; 
	int16 v20; 
	unsigned int v21; 
	unsigned int v22; 
	int16 v23; 
	int16 v24; 
	unsigned int v25; 
	unsigned int v26; 
	unsigned int v27; 
	unsigned int v28; 
	int16 v29; 
	int16 v30; 
	unsigned int v31;
	unsigned int v32;
	unsigned int v33;
	unsigned int v34; 
	int16 v35;
	int16 v36;
	unsigned int v37;
	unsigned int v38;
	unsigned int v39;
	unsigned int v40;
	int16 v41; 
	int16 v42; 
	unsigned int v43;
	unsigned int v44;
	unsigned int v45;
	unsigned int v46;
	int16 v47;
	int16 v48; 
	int16 v49; 
	int v50; 
	int16 v51;
	int16 v52;
	int16 v54;
	int v55; 
	int16 v56;
	int16 v57;
	int16 v58;

	v0 = 0;
	v58 = 0;
	v1 = 0;

	if (_vm->_globals.chemin == (int16 *)g_PTRNUL)
		return;
	if (_vm->_globals.Compteur > 1) {
		--_vm->_globals.Compteur;
		return;
	}
	_vm->_globals.Compteur = 0;
	if (_vm->_globals.g_old_sens == -1) {
		VERIFTAILLE();
		nouveau_x = *_vm->_globals.chemin;
		_vm->_globals.chemin++;

		nouveau_y = *_vm->_globals.chemin;
		_vm->_globals.chemin++;

		nouveau_sens = *_vm->_globals.chemin;
		_vm->_globals.chemin++;

		nouveau_anim = *_vm->_globals.chemin;
		_vm->_globals.chemin++;

		if (nouveau_x != -1 || nouveau_y != -1) {
			_vm->_globals.g_old_sens = nouveau_sens;
			_vm->_globals.g_old_sens2 = nouveau_sens;
			_vm->_globals.g_old_anim = 0;
			g_old_x = nouveau_x;
			g_old_y = nouveau_y;
		} else {
			SETANISPR(0, _vm->_globals.g_old_sens + 59);
			_vm->_globals.ACTION_SENS = 0;
			if (GOACTION == 1)
				v54 = _vm->_globals.SAUVEGARDE->data[svField2];
			else
				v54 = NUMZONE;
			_vm->_globals.chemin = (int16 *)g_PTRNUL;
			VERIFTAILLE();
			SETFLIPSPR(0, 0);
			_vm->_globals.Compteur = 0;
			_vm->_globals.chemin = (int16 *)g_PTRNUL;
			_vm->_globals.g_old_sens = -1;
			if (v54 > 0) {
				v55 = v54;
				if (_vm->_globals.ZONEP[v55].destX) {
					v56 = _vm->_globals.ZONEP[v55].destY;
					if (v56) {
						if (v56 != 31) {
							v57 = _vm->_globals.ZONEP[v55].field4;
							if (v57 == -1) {
								_vm->_globals.ZONEP[v55].destX = 0;
								_vm->_globals.ZONEP[v55].destY = 0;
								_vm->_globals.ZONEP[v55].field4 = 0;
							} else {
								SETANISPR(0, v57);
								_vm->_globals.ACTION_SENS = _vm->_globals.ZONEP[v55].field4 - 59;
							}
						}
					}
				}
			}
		}
LABEL_241:
		_vm->_globals.Compteur = 0;
		return;
	}
	if (_vm->_globals.g_old_sens == 3) {
		if ((uint16)(_vm->_globals.g_old_anim - 24) > 0xBu) {
			v0 = g_old_x;
			v58 = g_old_y;
		} else {
			v3 = _vm->_globals.Hopkins[_vm->_globals.g_old_anim].field0;
			v4 = _vm->_globals.Hopkins[_vm->_globals.g_old_anim].field2;

			if (Sprite[0].fieldC < 0) {
				v5 = Sprite[0].fieldC;
				if (Sprite[0].fieldC < 0)
					v5 = -Sprite[0].fieldC;
				v3 = _vm->_graphicsManager.Reel_Reduc(v3, v5);
				v6 = Sprite[0].fieldC;
				if (Sprite[0].fieldC < 0)
					v6 = -Sprite[0].fieldC;
				v4 = _vm->_graphicsManager.Reel_Reduc(v4, v6);
			}
			if (Sprite[0].fieldC > 0) {
				v7 = Sprite[0].fieldC;
				if (Sprite[0].fieldC < 0)
					v7 = -Sprite[0].fieldC;
				v3 = _vm->_graphicsManager.Reel_Zoom(v3, v7);
				v8 = Sprite[0].fieldC;
				if (Sprite[0].fieldC < 0)
					v8 = -Sprite[0].fieldC;
				v4 = _vm->_graphicsManager.Reel_Zoom(v4, v8);
			}
			v0 = v3 + g_old_x;
			v58 = g_old_y + v4;
			v1 = _vm->_globals.g_old_anim + 1;
			if (_vm->_globals.g_old_anim != 35)
				goto LABEL_23;
		}
		v1 = 24;
LABEL_23:
		_vm->_globals.Compteur = 5 / _vm->_globals.vitesse;
	}
	if (_vm->_globals.g_old_sens != 7)
		goto LABEL_43;
	if ((uint16)(_vm->_globals.g_old_anim - 24) > 0xBu) {
		v0 = g_old_x;
		v58 = g_old_y;
	} else {
		v9 = _vm->_globals.Hopkins[_vm->_globals.g_old_anim].field0;
		v10 = _vm->_globals.Hopkins[_vm->_globals.g_old_anim].field2;
		if (Sprite[0].fieldC < 0) {
			v11 = Sprite[0].fieldC;
			if (Sprite[0].fieldC < 0)
				v11 = -Sprite[0].fieldC;
			v9 = _vm->_graphicsManager.Reel_Reduc(v9, v11);
			v12 = Sprite[0].fieldC;
			if (Sprite[0].fieldC < 0)
				v12 = -Sprite[0].fieldC;
			v10 = _vm->_graphicsManager.Reel_Reduc(v10, v12);
		}
		if (Sprite[0].fieldC > 0) {
			v13 = Sprite[0].fieldC;
			if (Sprite[0].fieldC < 0)
				v13 = -Sprite[0].fieldC;
			v9 = _vm->_graphicsManager.Reel_Zoom(v9, v13);
			v14 = Sprite[0].fieldC;
			if (Sprite[0].fieldC < 0)
				v14 = -Sprite[0].fieldC;
			v10 = _vm->_graphicsManager.Reel_Zoom(v10, v14);
		}
		v0 = g_old_x - v9;
		v58 = g_old_y - v10;
		v1 = _vm->_globals.g_old_anim + 1;
		if (_vm->_globals.g_old_anim != 35)
			goto LABEL_42;
	}
	v1 = 24;
LABEL_42:
	_vm->_globals.Compteur = 5 / _vm->_globals.vitesse;
LABEL_43:
	if (_vm->_globals.g_old_sens != 1)
		goto LABEL_60;
	if (_vm->_globals.g_old_anim > 11) {
		v0 = g_old_x;
		v58 = g_old_y;
	} else {
		v15 = _vm->_globals.Hopkins[_vm->_globals.g_old_anim].field2;
		if (v15 < 0)
			v15 = -v15;
		v16 = v15;
		if (Sprite[0].fieldC < 0) {
			v17 = Sprite[0].fieldC;
			if (Sprite[0].fieldC < 0)
				v17 = -Sprite[0].fieldC;
			v16 = _vm->_graphicsManager.Reel_Reduc(v16, v17);
		}
		if (Sprite[0].fieldC > 0) {
			v18 = Sprite[0].fieldC;
			if (Sprite[0].fieldC < 0)
				v18 = -Sprite[0].fieldC;
			v16 = _vm->_graphicsManager.Reel_Zoom(v16, v18);
		}
		v0 = g_old_x;
		v58 = g_old_y - v16;
		v1 = _vm->_globals.g_old_anim + 1;
		if (_vm->_globals.g_old_anim != 11)
			goto LABEL_59;
	}
	v1 = 0;
LABEL_59:
	_vm->_globals.Compteur = 4 / _vm->_globals.vitesse;
LABEL_60:
	if (_vm->_globals.g_old_sens != 5)
		goto LABEL_77;
	if ((uint16)(_vm->_globals.g_old_anim - 48) > 0xBu) {
		v0 = g_old_x;
		v58 = g_old_y;
	} else {
		v19 = _vm->_globals.Hopkins[_vm->_globals.g_old_anim].field2;
		if (v19 < 0)
			v19 = -v19;
		v20 = v19;
		if (Sprite[0].fieldC < 0) {
			v21 = Sprite[0].fieldC;
			if (Sprite[0].fieldC < 0)
				v21 = -Sprite[0].fieldC;
			v20 = _vm->_graphicsManager.Reel_Reduc(v20, v21);
		}
		if (Sprite[0].fieldC > 0) {
			v22 = Sprite[0].fieldC;
			if (Sprite[0].fieldC < 0)
				v22 = -Sprite[0].fieldC;
			v20 = _vm->_graphicsManager.Reel_Zoom(v20, v22);
		}
		v0 = g_old_x;
		v58 = v20 + g_old_y;
		v1 = _vm->_globals.g_old_anim + 1;
		if (_vm->_globals.g_old_anim != 59)
			goto LABEL_76;
	}
	v1 = 48;
LABEL_76:
	_vm->_globals.Compteur = 4 / _vm->_globals.vitesse;
LABEL_77:
	if (_vm->_globals.g_old_sens != 2)
		goto LABEL_96;
	if ((uint16)(_vm->_globals.g_old_anim - 12) > 0xBu) {
		v0 = g_old_x;
		v58 = g_old_y;
	} else {
		v23 = _vm->_globals.Hopkins[_vm->_globals.g_old_anim].field0;
		v24 = _vm->_globals.Hopkins[_vm->_globals.g_old_anim].field2;
		if (Sprite[0].fieldC < 0) {
			v25 = Sprite[0].fieldC;
			if (Sprite[0].fieldC < 0)
				v25 = -Sprite[0].fieldC;
			v23 = _vm->_graphicsManager.Reel_Reduc(v23, v25);
			v26 = Sprite[0].fieldC;
			if (Sprite[0].fieldC < 0)
				v26 = -Sprite[0].fieldC;
			v24 = _vm->_graphicsManager.Reel_Reduc(v24, v26);
		}
		if (Sprite[0].fieldC > 0) {
			v27 = Sprite[0].fieldC;
			if (Sprite[0].fieldC < 0)
				v27 = -Sprite[0].fieldC;
			v23 = _vm->_graphicsManager.Reel_Zoom(v23, v27);
			v28 = Sprite[0].fieldC;
			if (Sprite[0].fieldC < 0)
				v28 = -Sprite[0].fieldC;
			v24 = _vm->_graphicsManager.Reel_Zoom(v24, v28);
		}
		v0 = v23 + g_old_x;
		v58 = g_old_y + v24;
		v1 = _vm->_globals.g_old_anim + 1;
		if (_vm->_globals.g_old_anim != 23)
			goto LABEL_95;
	}
	v1 = 12;
LABEL_95:
	_vm->_globals.Compteur = 5 / _vm->_globals.vitesse;
LABEL_96:
	if (_vm->_globals.g_old_sens != 8)
		goto LABEL_115;
	if ((uint16)(_vm->_globals.g_old_anim - 12) > 0xBu) {
		v0 = g_old_x;
		v58 = g_old_y;
	} else {
		v29 = _vm->_globals.Hopkins[_vm->_globals.g_old_anim].field0;
		v30 = _vm->_globals.Hopkins[_vm->_globals.g_old_anim].field2;
		if (Sprite[0].fieldC < 0) {
			v31 = Sprite[0].fieldC;
			if (Sprite[0].fieldC < 0)
				v31 = -Sprite[0].fieldC;
			v29 = _vm->_graphicsManager.Reel_Reduc(v29, v31);
			v32 = Sprite[0].fieldC;
			if (Sprite[0].fieldC < 0)
				v32 = -Sprite[0].fieldC;
			v30 = _vm->_graphicsManager.Reel_Reduc(v30, v32);
		}
		if (Sprite[0].fieldC > 0) {
			v33 = Sprite[0].fieldC;
			if (Sprite[0].fieldC < 0)
				v33 = -Sprite[0].fieldC;
			v29 = _vm->_graphicsManager.Reel_Zoom(v29, v33);
			v34 = Sprite[0].fieldC;
			if (Sprite[0].fieldC < 0)
				v34 = -Sprite[0].fieldC;
			v30 = _vm->_graphicsManager.Reel_Zoom(v30, v34);
		}
		v0 = g_old_x - v29;
		v58 = g_old_y + v30;
		v1 = _vm->_globals.g_old_anim + 1;
		if (_vm->_globals.g_old_anim != 23)
			goto LABEL_114;
	}
	v1 = 12;
LABEL_114:
	_vm->_globals.Compteur = 5 / _vm->_globals.vitesse;
LABEL_115:
	if (_vm->_globals.g_old_sens != 4)
		goto LABEL_134;
	if ((uint16)(_vm->_globals.g_old_anim - 36) > 0xBu) {
		v0 = g_old_x;
		v58 = g_old_y;
	} else {
		v35 = _vm->_globals.Hopkins[_vm->_globals.g_old_anim].field0;
		v36 = _vm->_globals.Hopkins[_vm->_globals.g_old_anim].field2;
		if (Sprite[0].fieldC < 0) {
			v37 = Sprite[0].fieldC;
			if (Sprite[0].fieldC < 0)
				v37 = -Sprite[0].fieldC;
			v35 = _vm->_graphicsManager.Reel_Reduc(v35, v37);
			v38 = Sprite[0].fieldC;
			if (Sprite[0].fieldC < 0)
				v38 = -Sprite[0].fieldC;
			v36 = _vm->_graphicsManager.Reel_Reduc(v36, v38);
		}
		if (Sprite[0].fieldC > 0) {
			v39 = Sprite[0].fieldC;
			if (Sprite[0].fieldC < 0)
				v39 = -Sprite[0].fieldC;
			v35 = _vm->_graphicsManager.Reel_Zoom(v35, v39);
			v40 = Sprite[0].fieldC;
			if (Sprite[0].fieldC < 0)
				v40 = -Sprite[0].fieldC;
			v36 = _vm->_graphicsManager.Reel_Zoom(v36, v40);
		}
		v0 = v35 + g_old_x;
		v58 = g_old_y + v36;
		v1 = _vm->_globals.g_old_anim + 1;
		if (_vm->_globals.g_old_anim != 47)
			goto LABEL_133;
	}
	v1 = 36;
LABEL_133:
	_vm->_globals.Compteur = 5 / _vm->_globals.vitesse;
LABEL_134:
	if (_vm->_globals.g_old_sens != 6)
		goto LABEL_153;
	if ((uint16)(_vm->_globals.g_old_anim - 36) > 0xBu) {
		v0 = g_old_x;
		v58 = g_old_y;
	} else {
		v41 = _vm->_globals.Hopkins[_vm->_globals.g_old_anim].field0;
		v42 = _vm->_globals.Hopkins[_vm->_globals.g_old_anim].field2;
		if (Sprite[0].fieldC < 0) {
			v43 = Sprite[0].fieldC;
			if (Sprite[0].fieldC < 0)
				v43 = -Sprite[0].fieldC;
			v41 = _vm->_graphicsManager.Reel_Reduc(v41, v43);
			v44 = Sprite[0].fieldC;
			if (Sprite[0].fieldC < 0)
				v44 = -Sprite[0].fieldC;
			v42 = _vm->_graphicsManager.Reel_Reduc(v42, v44);
		}
		if (Sprite[0].fieldC > 0) {
			v45 = Sprite[0].fieldC;
			if (Sprite[0].fieldC < 0)
				v45 = -Sprite[0].fieldC;
			v41 = _vm->_graphicsManager.Reel_Zoom(v41, v45);
			v46 = Sprite[0].fieldC;
			if (Sprite[0].fieldC < 0)
				v46 = -Sprite[0].fieldC;
			v42 = _vm->_graphicsManager.Reel_Zoom(v42, v46);
		}
		v0 = g_old_x - v41;
		v58 = g_old_y + v42;
		v1 = _vm->_globals.g_old_anim + 1;
		if (_vm->_globals.g_old_anim != 47)
			goto LABEL_152;
	}
	v1 = 36;
LABEL_152:
	_vm->_globals.Compteur = 5 / _vm->_globals.vitesse;
LABEL_153:
	v47 = 0;
	do {
		nouveau_x = *_vm->_globals.chemin;
		_vm->_globals.chemin++;

		v48 = *_vm->_globals.chemin;
		nouveau_y = *_vm->_globals.chemin;
		_vm->_globals.chemin++;

		nouveau_sens = *_vm->_globals.chemin;
		_vm->_globals.chemin++;
		nouveau_anim = *_vm->_globals.chemin;
		_vm->_globals.chemin++;

		if (nouveau_x == -1 && v48 == -1) {
			if (GOACTION == 1)
				v49 = _vm->_globals.SAUVEGARDE->data[svField2];
			else
				v49 = NUMZONE;
			SETANISPR(0, _vm->_globals.g_old_sens + 59);
			_vm->_globals.ACTION_SENS = 0;
			_vm->_globals.chemin = (int16 *)g_PTRNUL;
			VERIFTAILLE();
			SETFLIPSPR(0, 0);
			_vm->_globals.Compteur = 0;
			_vm->_globals.g_old_sens = -1;
			g_old_x = XSPR(0);
			g_old_y = YSPR(0);

			if (v49 > 0) {
				v50 = v49;
				if (_vm->_globals.ZONEP[v50].destX) {
					v51 = _vm->_globals.ZONEP[v50].destY;
					if (v51) {
						if (v51 != 31) {
							v52 = _vm->_globals.ZONEP[v50].field4;
							if (v52 == -1) {
								_vm->_globals.ZONEP[v50].destX = 0;
								_vm->_globals.ZONEP[v50].destY = 0;
								_vm->_globals.ZONEP[v50].field4 = 0;
							} else {
								SETANISPR(0, v52);
								_vm->_globals.ACTION_SENS = _vm->_globals.ZONEP[v50].field4 - 59;
							}
						}
					}
				}
			}
			goto LABEL_241;
		}
		if (_vm->_globals.g_old_sens != nouveau_sens)
			break;
		if (nouveau_sens == 3 && nouveau_x >= v0)
			v47 = 1;
		if (_vm->_globals.g_old_sens == 7 && nouveau_x <= v0)
			v47 = 1;
		if (_vm->_globals.g_old_sens == 1 && nouveau_y <= v58)
			v47 = 1;
		if (_vm->_globals.g_old_sens == 5 && nouveau_y >= v58)
			v47 = 1;
		if (_vm->_globals.g_old_sens == 2 && nouveau_x >= v0)
			v47 = 1;
		if (_vm->_globals.g_old_sens == 8 && nouveau_x <= v0)
			v47 = 1;
		if (_vm->_globals.g_old_sens == 4 && nouveau_x >= v0)
			v47 = 1;
		if (_vm->_globals.g_old_sens == 6 && nouveau_x <= v0)
			v47 = 1;
	} while (v47 != 1);
	if (v47 == 1) {
		VERIFTAILLE();
		if (_vm->_globals.g_old_sens == 7)
			SETFLIPSPR(0, 1);
		if (_vm->_globals.g_old_sens == 3)
			SETFLIPSPR(0, 0);
		if (_vm->_globals.g_old_sens == 1)
			SETFLIPSPR(0, 0);
		if (_vm->_globals.g_old_sens == 5)
			SETFLIPSPR(0, 0);
		if (_vm->_globals.g_old_sens == 2)
			SETFLIPSPR(0, 0);
		if (_vm->_globals.g_old_sens == 8)
			SETFLIPSPR(0, 1);
		if (_vm->_globals.g_old_sens == 4)
			SETFLIPSPR(0, 0);
		if (_vm->_globals.g_old_sens == 6)
			SETFLIPSPR(0, 1);
		SETXSPR(0, nouveau_x);
		SETYSPR(0, nouveau_y);
		SETANISPR(0, v1);
	} else {
		if (_vm->_globals.g_old_sens == 7)
			SETFLIPSPR(0, 1);
		if (_vm->_globals.g_old_sens == 3)
			SETFLIPSPR(0, 0);
		if (_vm->_globals.g_old_sens == 1)
			SETFLIPSPR(0, 0);
		if (_vm->_globals.g_old_sens == 5)
			SETFLIPSPR(0, 0);
		if (_vm->_globals.g_old_sens == 2)
			SETFLIPSPR(0, 0);
		if (_vm->_globals.g_old_sens == 8)
			SETFLIPSPR(0, 1);
		if (_vm->_globals.g_old_sens == 4)
			SETFLIPSPR(0, 0);
		if (_vm->_globals.g_old_sens == 6)
			SETFLIPSPR(0, 1);
		_vm->_globals.Compteur = 0;
	}
	_vm->_globals.g_old_sens = nouveau_sens;
	_vm->_globals.g_old_sens2 = nouveau_sens;
	_vm->_globals.g_old_anim = v1;
	g_old_x = nouveau_x;
	g_old_y = nouveau_y;
}

void ObjectsManager::GOHOME2() {
	signed int v0;
	int16 v2; 

	v0 = 2;
	if (_vm->_globals.chemin != (int16 *)g_PTRNUL) {
		if (_vm->_globals.vitesse == 2)
			v0 = 4;
		if (_vm->_globals.vitesse == 3)
			v0 = 6;
		_vm->_globals.j_104 = 0;
		if (v0) {
			while (1) {
				nouveau_x = *_vm->_globals.chemin;
				_vm->_globals.chemin++;

				v2 = *_vm->_globals.chemin;
				nouveau_y = *_vm->_globals.chemin;
				_vm->_globals.chemin++;

				nouveau_sens = *_vm->_globals.chemin;
				_vm->_globals.chemin++;
				nouveau_anim = *_vm->_globals.chemin;
				_vm->_globals.chemin++;

				if (nouveau_x == -1) {
					if (v2 == -1)
						break;
				}
				++_vm->_globals.j_104;
				if (_vm->_globals.j_104 >= v0)
					goto LABEL_19;
			}
			if (_vm->_globals.last_sens == 1)
				SETANISPR(0, 0);
			if (_vm->_globals.last_sens == 3)
				SETANISPR(0, 1);
			if (_vm->_globals.last_sens == 5)
				SETANISPR(0, 2);
			if (_vm->_globals.last_sens == 7)
				SETANISPR(0, 3);
			
			_vm->_globals.chemin = (int16 *)g_PTRNUL;
			my_anim = 0;
			A_ANIM = 0;
			A_DEPA = 0;
		} else {
LABEL_19:
			_vm->_globals.last_sens = nouveau_sens;
			SETXSPR(0, nouveau_x);
			SETYSPR(0, nouveau_y);
			if (_vm->_globals.last_sens == 1)
				SETANISPR(0, 4);
			if (_vm->_globals.last_sens == 3)
				SETANISPR(0, 5);
			if (_vm->_globals.last_sens == 5)
				SETANISPR(0, 6);
			if (_vm->_globals.last_sens == 7)
				SETANISPR(0, 7);

			if (my_anim++ > 1)
				my_anim = 0;
		}
	}
}

// Load Obstacle
void ObjectsManager::CHARGE_OBSTACLE(const Common::String &file) {
	int16 v1; 
	byte *ptr; 
	int16 v4; 
	int16 v5; 

	_vm->_linesManager.RESET_OBSTACLE();
	_vm->_linesManager.TOTAL_LIGNES = 0;
	DERLIGNE = 0;
	_vm->_fileManager.CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, file);
	ptr = _vm->_fileManager.CHARGE_FICHIER(_vm->_globals.NFICHIER);
	v4 = 0;
	v5 = 0;
	do {
		v1 = (int16)READ_LE_UINT16((uint16 *)ptr + v4);
		if (v1 != -1) {
			_vm->_linesManager.AJOUTE_LIGNE(
			    v5,
			    v1,
			    (int16)READ_LE_UINT16((uint16 *)ptr + v4 + 1),
			    (int16)READ_LE_UINT16((uint16 *)ptr + v4 + 2),
			    (int16)READ_LE_UINT16((uint16 *)ptr + v4 + 3),
			    (int16)READ_LE_UINT16((uint16 *)ptr + v4 + 4),
			    1);
			++_vm->_linesManager.TOTAL_LIGNES;
		}
		v4 += 5;
		++v5;
	} while (v1 != -1);
	_vm->_linesManager.INIPARCOURS();
	_vm->_globals.dos_free2(ptr);
}

// Load Zone
void ObjectsManager::CHARGE_ZONE(const Common::String &file) {
	signed int v1; 
	int v2; 
	int v3; 
	int v4; 
	int v5; 
	int v6; 
	int16 v7; 
	int v8; 
	byte *v9; 
	int v10; 
	signed int v11; 
	int v12; 
	byte *v13; 
	int v14; 
	signed int v15; 
	int16 v17; 
	int16 v18; 
	byte *ptr; 

	v1 = 1;
	do {
		v2 = v1;
		_vm->_globals.ZONEP[v2].destX = 0;
		_vm->_globals.ZONEP[v2].destY = 0;
		_vm->_globals.ZONEP[v2].field4 = 0;
		_vm->_globals.ZONEP[v2].field6 = 0;
		_vm->_globals.ZONEP[v2].field7 = 0;
		_vm->_globals.ZONEP[v2].field8 = 0;
		_vm->_globals.ZONEP[v2].field9 = 0;
		_vm->_globals.ZONEP[v2].fieldA = 0;
		_vm->_globals.ZONEP[v2].fieldB = 0;
		_vm->_globals.ZONEP[v2].fieldC = 0;
		_vm->_globals.ZONEP[v2].fieldD = 0;
		_vm->_globals.ZONEP[v2].fieldE = 0;
		_vm->_globals.ZONEP[v2].fieldF = 0;
		_vm->_globals.ZONEP[v2].field12 = 0;
		_vm->_globals.ZONEP[v2].field10 = 0;
		++v1;
	} while (v1 <= 100);

	_vm->_fileManager.CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, file);
	
	Common::File f;
	if (!f.exists(_vm->_globals.NFICHIER))
		error("File not found : %s", _vm->_globals.NFICHIER.c_str());

	ptr = _vm->_fileManager.CHARGE_FICHIER(_vm->_globals.NFICHIER);
	v4 = 0;
	v18 = 0;
	v17 = 0;
	do {
		v3 = (int16)READ_LE_UINT16((uint16 *)ptr + v4);
		if (v3 != -1) {
			v5 = v3;
			v6 = v3;
			_vm->_linesManager.AJOUTE_LIGNE_ZONE(
			    v18,
			    READ_LE_UINT16((uint16 *)ptr + v4 + 1),
			    READ_LE_UINT16((uint16 *)ptr + v4 + 2),
			    READ_LE_UINT16((uint16 *)ptr + v4 + 3),
			    READ_LE_UINT16((uint16 *)ptr + v4 + 4),
			    v3);
			_vm->_globals.ZONEP[v5].field10 = 1;
			v3 = v6;
		}
		v4 += 5;
		++v18;
		++v17;
	} while (v3 != -1);
	v7 = 1;
	do {
		v8 = v7;
		_vm->_globals.ZONEP[v8].destX = (int16)READ_LE_UINT16((uint16 *)ptr + v4);
		_vm->_globals.ZONEP[v8].destY = (int16)READ_LE_UINT16((uint16 *)ptr + v4 + 1);
		_vm->_globals.ZONEP[v8].field4 = (int16)READ_LE_UINT16((uint16 *)ptr + v4 + 2);
		v4 += 3;
		++v7;
	} while (v7 <= 100);

	v9 = (ptr + 10 * v17 + 606);
	v10 = 0;
	v11 = 1;
	do {
		v12 = v11;

		_vm->_globals.ZONEP[v12].field6 = *(v10 + v9);
		_vm->_globals.ZONEP[v12].field7 = *(v9 + v10 + 1);
		_vm->_globals.ZONEP[v12].field8 = *(v9 + v10 + 2);
		_vm->_globals.ZONEP[v12].field9 = *(v9 + v10 + 3);
		_vm->_globals.ZONEP[v12].fieldA = *(v9 + v10 + 4);
		_vm->_globals.ZONEP[v12].fieldB = *(v9 + v10 + 5);
		_vm->_globals.ZONEP[v12].fieldC = *(v9 + v10 + 6);
		_vm->_globals.ZONEP[v12].fieldD = *(v9 + v10 + 7);
		_vm->_globals.ZONEP[v12].fieldE = *(v9 + v10 + 8);
		_vm->_globals.ZONEP[v12].fieldF = *(v9 + v10 + 9);

		v10 += 10;
		++v11;
	} while (v11 <= 100);
	v13 = v9 + 1010;
	v14 = 0;
	v15 = 1;
	do
		_vm->_globals.ZONEP[v15++].field12 = READ_LE_UINT16(v13 + 2 * v14++);
	while (v15 <= 100);

	_vm->_globals.dos_free2(ptr);
	CARRE_ZONE();
}

// Square Zone
void ObjectsManager::CARRE_ZONE() {
	int16 *dataP;
	int v4; 
	int v5; 
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
		dataP = _vm->_linesManager.LigneZone[idx].zoneData;
		if (dataP != (int16 *)g_PTRNUL) {
			v4 = _vm->_linesManager.LigneZone[idx].field2;
			_vm->_globals.CarreZone[v4].field0 = 1;
			if (_vm->_globals.CarreZone[v4].fieldC < idx)
				_vm->_globals.CarreZone[v4].fieldC = idx;
			if (_vm->_globals.CarreZone[v4].fieldA > idx)
				_vm->_globals.CarreZone[v4].fieldA = idx;

			v13 = 0;
			v12 = _vm->_linesManager.LigneZone[idx].count;
			if (v12 > 0) {
				do {
					v5 = *dataP++;
					v11 = *dataP++;

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
	_vm->_dialogsManager.INVENTFLAG = false;
	_vm->_eventsManager.KEY_INVENT = 0;
	_vm->_globals.Max_Propre = 1;
	_vm->_globals.Max_Ligne_Long = 1;
	_vm->_globals.Max_Propre_Gen = 1;
	_vm->_globals.Max_Perso_Y = 440;
	_vm->_globals.NOSPRECRAN = 1;
	_vm->_globals.PLAN_FLAG = 1;
	_vm->_graphicsManager.NOFADE = false;
	_vm->_globals.NOMARCHE = 0;
	sprite_ptr = g_PTRNUL;
	_vm->_globals.SORTIE = 0;
	_vm->_globals.AFFLI = 0;
	_vm->_globals.AFFIVBL = 0;
	_vm->_globals.NOT_VERIF = 1;
	_vm->_soundManager.WSOUND(31);
	_vm->_globals.iRegul = 1;
	_vm->_graphicsManager.LOAD_IMAGE("PLAN");
	CHARGE_OBSTACLE("PLAN.OB2");
	_vm->_globals.CHARGE_CACHE("PLAN.CA2");
	CHARGE_ZONE("PLAN.ZO2");
	_vm->_fileManager.CONSTRUIT_SYSTEM("VOITURE.SPR");
	sprite_ptr = _vm->_fileManager.CHARGE_FICHIER(_vm->_globals.NFICHIER);
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
	_vm->_globals.chemin = (int16 *)g_PTRNUL;
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
		
		if (_vm->_globals.chemin == (int16 *)g_PTRNUL && _vm->_globals.GOACTION == 1)
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

// Left Button
void ObjectsManager::BTGAUCHE() {
	int srcX, srcY;
	int destX, destY;
	int v1;
	int v2;
	int16 *v3; 
	int16 *v4; 
	int16 *v5; 
	int zoneCount; 
	int zoneX;
	int zoneY;
	int16 *v9; 
	int v10;
	int v11;
	int v12;
	int16 *v13; 
	int16 *v16; 
	int v17;
	int v18;

	_vm->_fontManager.TEXTE_OFF(9);
	destX = _vm->_eventsManager.XMOUSE();
	destY = _vm->_eventsManager.YMOUSE();

	if (!_vm->_dialogsManager.INVENTFLAG && !_vm->_globals.PLAN_FLAG && destX > _vm->_graphicsManager.ofscroll - 30 && destX < _vm->_graphicsManager.ofscroll + 50 && (uint16)(destY + 29) <= 0x4Eu) {
		v1 = _vm->_eventsManager.btsouris;
		_vm->_dialogsManager.INVENTFLAG = 1;
		_vm->_dialogsManager.showInventory();
		_vm->_dialogsManager.INVENTFLAG = 0;
		_vm->_eventsManager.KEY_INVENT = 0;
		if (!_vm->_globals.SORTIE) {
			_vm->_dialogsManager.INVENTFLAG = 0;
			_vm->_eventsManager.btsouris = v1;
		}
		return;
	}
	if (_vm->_globals.SAUVEGARDE->data[svField354] == 1
	        && !_vm->_globals.PLAN_FLAG
	        && (uint16)(destX - 533) <= 0x1Au
	        && (uint16)(destY - 26) <= 0x21u) {
		CHANGE_TETE(1, 0);
		return;
	}
	if (_vm->_globals.SAUVEGARDE->data[svField356] == 1
	        && !_vm->_globals.PLAN_FLAG
	        && (uint16)(destX - 533) <= 0x1Au
	        && (uint16)(destY - 26) <= 0x21u) {
		CHANGE_TETE(2, 0);
		return;
	}
	if (_vm->_globals.SAUVEGARDE->data[svField357] == 1) {
		if (_vm->_globals.SAUVEGARDE->data[svField353] == 1
		        && !_vm->_globals.PLAN_FLAG
		        && (uint16)(destX - 533) <= 0x1Au
		        && (uint16)(destY - 26) <= 0x21u) {
			CHANGE_TETE(0, 1);
			return;
		}
		if (_vm->_globals.SAUVEGARDE->data[svField355] == 1
		        && !_vm->_globals.PLAN_FLAG
		        && (uint16)(destX - 567) <= 0x1Au
		        && (uint16)(destY - 26) <= 0x21u) {
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
			v3[v2] = v4[v2];
			++v2;
		} while (v4[v2] != -1);

		v5 = _vm->_globals.essai2;
		v5[v2] = -1;
		v5[v2 + 1] = -1;
		v5[v2 + 2] = -1;
		v5[v2 + 3] = -1;
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
		zoneCount = NUMZONE;
		zoneX = _vm->_globals.ZONEP[zoneCount].destX;
		if (zoneX) {
			zoneY = _vm->_globals.ZONEP[zoneCount].destY;
			if (zoneY) {
				if (zoneY != 31) {
					destX = zoneX;
					destY = zoneY;
				}
			}
		}
	}
	GOACTION = 0;
	v9 = _vm->_globals.chemin;
	_vm->_globals.chemin = (int16 *)g_PTRNUL;
	if (_vm->_globals.FORET && ((uint16)(NUMZONE - 20) <= 1u || (uint16)(NUMZONE - 22) <= 1u)) {
		if (YSPR(0) <= 374 || YSPR(0) > 410) {
			v10 = XSPR(0);
			v11 = YSPR(0);
			v12 = XSPR(0);
			v13 = _vm->_linesManager.PARCOURS2(v12, v11, v10, 390);
			_vm->_globals.chemin = v13;
			if (v13 != (int16 *)g_PTRNUL)
				PACOURS_PROPRE(v13);
			g_old_x = XSPR(0);
			g_old_y = YSPR(0);
			_vm->_globals.Compteur = 0;
			if (_vm->_globals.chemin != (int16 *)g_PTRNUL || v9 == _vm->_globals.chemin) {
LABEL_64:
				_vm->_globals.g_old_sens = -1;
				goto LABEL_65;
			}
			goto LABEL_63;
		}
		_vm->_globals.chemin = (int16 *)g_PTRNUL;
		SETANISPR(0, _vm->_globals.g_old_sens2 + 59);
		_vm->_globals.ACTION_SENS = 0;
		_vm->_globals.chemin = (int16 *)g_PTRNUL;
		VERIFTAILLE();
		SETFLIPSPR(0, 0);
		_vm->_globals.Compteur = 0;
		_vm->_globals.g_old_sens = -1;
		goto LABEL_65;
	}
	if (!_vm->_globals.NOMARCHE) {
		if (!_vm->_globals.PLAN_FLAG) {
			srcY = YSPR(0);
			srcX = XSPR(0);
			v16 = _vm->_linesManager.PARCOURS2(srcX, srcY, destX, destY);
			_vm->_globals.chemin = v16;
			if (v16 != (int16 *)g_PTRNUL)
				PACOURS_PROPRE(v16);
			g_old_x = XSPR(0);
			g_old_y = YSPR(0);
			_vm->_globals.Compteur = 0;
			if (_vm->_globals.chemin != (int16 *)g_PTRNUL || v9 == _vm->_globals.chemin)
				goto LABEL_64;
LABEL_63:
			_vm->_globals.chemin = v9;
		}
LABEL_65:
		if (!_vm->_globals.NOMARCHE && _vm->_globals.PLAN_FLAG == 1) {
			v17 = YSPR(0);
			v18 = XSPR(0);
			_vm->_globals.chemin = PARC_VOITURE(v18, v17, destX, destY);
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
		_vm->_globals.chemin = (int16 *)g_PTRNUL;
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

// Clear Screen
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
		_vm->_globals.BOBZONE_FLAG[v2] = false;
		++v1;
	} while (v1 <= 48);
	_vm->_eventsManager.btsouris = 4;
	verbe = 4;
	NUMZONE = 0;
	Vold_taille = 0;
	SPEED_FLAG = 0;
	SPEED_PTR = g_PTRNUL;
	SPEED_X = 0;
	SPEED_Y = 0;
	SPEED_IMAGE = 0;
	FORCEZONE = 1;
	_vm->_linesManager.TOTAL_LIGNES = 0;
	DERLIGNE = 0;
	_vm->_globals.chemin = (int16 *)g_PTRNUL;
	if (_vm->_globals.COUCOU != g_PTRNUL)
		_vm->_globals.COUCOU = _vm->_fileManager.LIBERE_FICHIER(_vm->_globals.COUCOU);
	if (g_PTRNUL != _vm->_globals.SPRITE_ECRAN)
		_vm->_globals.SPRITE_ECRAN = _vm->_fileManager.LIBERE_FICHIER(_vm->_globals.SPRITE_ECRAN);
	_vm->_eventsManager.start_x = 0;
	_vm->_eventsManager.souris_n = 0;
	Vold_taille = 200;
	_vm->_globals.SAUVEGARDE->data[svField1] = 0;
	_vm->_globals.SAUVEGARDE->data[svField2] = 0;
	GOACTION = 0;
	FORCEZONE = 1;
	CHANGEVERBE = 0;
	_vm->_globals.NOSPRECRAN = 0;
	_vm->_globals.chemin = (int16 *)g_PTRNUL;
	_vm->_globals.g_old_sens = -1;
	my_anim = 1;
	A_ANIM = 0;
	MA_ANIM = 0;
	MA_ANIM1 = 0;
	A_DEPA = 0;
	MAX_DEPA = 0;
	MAX_DEPA1 = 0;
	_vm->_graphicsManager.RESET_SEGMENT_VESA();
}

// Change Face/Head
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
	_vm->_globals.chemin = (int16 *)g_PTRNUL;
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
				_vm->_fileManager.CONSTRUIT_SYSTEM("PERSO.SPR");
				_vm->_fileManager.CHARGE_FICHIER2(_vm->_globals.NFICHIER, _vm->_globals.PERSO);
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
			_vm->_fileManager.CONSTRUIT_SYSTEM("PSAMAN.SPR");
			_vm->_fileManager.CHARGE_FICHIER2(_vm->_globals.NFICHIER, _vm->_globals.PERSO);
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

// Check Size
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

void ObjectsManager::PACOURS_PROPRE(int16 *a1) {
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
	v2 = a1[1];
	v15 = a1[2];
	if (a1[0] != -1 || v2 != -1) {
		for (;;) {
			if (v14 != -1 && v15 != v14) {
				v11 = v1;
				v12 = 0;
				v10 = CALC_PROPRE(v2);
				v4 = a1[v1];
				v9 = a1[v1];
				v5 = a1[v1 + 1];
				v6 = 0;
				while (v4 != -1 || v5 != -1) {
					int idx = v1;
					v1 += 4;
					++v12;
					if (a1[idx + 2] != v15)
						v6 = 1;
					if (v6 == 1)
						break;
					v4 = a1[v1];
					v9 = a1[v1];
					v5 = a1[v1 + 1];
				}
				if (v12 < v10) {
					v7 = v11;
					v8 = 0;
					if (v12 > 0) {
						do {
							a1[v7 + 2] = v14;
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
			v2 = a1[v1 + 1];
			v15 = a1[v1 + 2];
			if (a1[v1] == -1) {
				if (v2 == -1)
					break;
			}
		}
	}
}

int16 *ObjectsManager::PARC_VOITURE(int a1, int a2, int a3, int a4) {
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
	int16 *result; 
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
	int16 *v33; 
	int v34; 
	int v35; 
	int i; 
	int16 *v37;
	int v38;
	int v39; 
	int v40;
	int16 *v41; 
	int v42;
	int v43; 
	int k;
	int16 *v45; 
	int v46;
	int v47; 
	int v48; 
	int v49;
	int16 *v50; 
	int v51;
	int v52; 
	int v53;
	int16 *v54; 
	int v55;
	int v56; 
	int v57;
	int16 *v58; 
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
					v28 = (int16)READ_LE_UINT16(_vm->_globals.essai2 + 2 * (int16)v27);
					v29 = (int16)READ_LE_UINT16(_vm->_globals.essai2 + 2 * (int16)v27 + 2);
					v66 = (int16)READ_LE_UINT16(_vm->_globals.essai2 + 2 * (int16)v27 + 4);
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
					v33[v32] = v28;
					v33[v32 + 1] = v29;
					v33[v32 + 2] = v66;
					v33[v32 + 3] = 0;
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
				v37 = _vm->_linesManager.Ligne[v69].lineData;
				v38 = v37[2 * v35];
				int v37_2 = v37[2 * v35 + 1];
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
						_vm->_linesManager.Ligne[j].lineData[0],
						_vm->_linesManager.Ligne[j].lineData[1],
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
						v41 = _vm->_linesManager.Ligne[j].lineData;
						v42 = v41[2 * v40];
						int v41_2 = v41[2 * v40 + 1];
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
				v45 = _vm->_linesManager.Ligne[v69].lineData;
				v46 = v45[2 * k];
				int v45_2 = v45[2 * k + 1];
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
						_vm->_linesManager.Ligne[l].lineData[2 * _vm->_linesManager.Ligne[v48].field0 - 2],
						_vm->_linesManager.Ligne[l].lineData[2 * _vm->_linesManager.Ligne[v48].field0 - 1],
				        v67,
				        l,
				        v73,
				        0) == 1)
					goto LABEL_88;
				v49 = _vm->_linesManager.Ligne[v48].field0 - 2;
				if ((_vm->_linesManager.Ligne[v48].field0 - 2) > 0) {
					do {
						v50 = _vm->_linesManager.Ligne[l].lineData;
						v51 = v50[2 * v49];
						int v50_2 = v50[2 * v49 + 1];
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
						v58 = _vm->_linesManager.Ligne[v73].lineData;
						v59 = v58[2 * v57];
						int v58_2 = v58[2 * v57 + 1];
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
					v54 = _vm->_linesManager.Ligne[v73].lineData;
					v55 = v54[2 * v53];
					int v54_2 = v54[2 * v53 + 1];
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
		result = &_vm->_globals.super_parcours[0];
	} else {
		result = (int16 *)g_PTRNUL;
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
	int16 v1;
	int16 v2;
	int v3; 
	int16 v4;
	int16 v5;
	int v6; 
	int16 v7;
	int v8; 
	int v9; 
	int16 yCurrent;
	int16 v11;
	int16 j; 
	int16 k; 
	int16 xCurrent; 
	int v15; 
	int16 v16;
	int16 v17;
	int16 v18;
	int16 v19;
	int16 yp;
	int16 xp;

	v19 = 0;
	v18 = 0;
	v17 = 0;
	v16 = 0;
	xp = _vm->_eventsManager.souris_x + _vm->_eventsManager.ofset_souris_x;
	yp = _vm->_eventsManager.souris_y + _vm->_eventsManager.ofset_souris_y;
	if ((_vm->_eventsManager.souris_y + _vm->_eventsManager.ofset_souris_y) > 19) {
		v1 = 0;
		do {
			v2 = _vm->_globals.BOBZONE[v1];
			if (v2) {
				if (_vm->_globals.BOBZONE_FLAG[v1]) {
					v3 = v2;
					v15 = v3;
					if (_vm->_globals.Bob[v3].field0) {
						if (_vm->_globals.Bob[v3].fieldC != 250) {
							if (!_vm->_globals.Bob[v3].field16) {
								v4 = _vm->_globals.Bob[v3].field3E;
								if (xp > v4) {
									if (xp < _vm->_globals.Bob[v3].field42 + v4) {
										v5 = _vm->_globals.Bob[v3].field40;
										if (yp > v5) {
											if (yp < _vm->_globals.Bob[v3].field44 + v5) {
												v6 = v1;
												if (_vm->_globals.ZONEP[v1].field4 == -1) {
													_vm->_globals.ZONEP[v6].destX = 0;
													_vm->_globals.ZONEP[v1].destY = 0;
												}
												if (!_vm->_globals.ZONEP[v6].destX) {
													if (!_vm->_globals.ZONEP[v1].destY) {
														_vm->_globals.ZONEP[v6].destX = _vm->_globals.Bob[v15].field42 + _vm->_globals.Bob[v15].field3E;
														_vm->_globals.ZONEP[v1].destY = _vm->_globals.Bob[v15].field44 + _vm->_globals.Bob[v15].field40 + 6;
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
					if (_vm->_globals.CarreZone[v7].field2 <= xp
					        && _vm->_globals.CarreZone[v7].field4 >= xp
					        && _vm->_globals.CarreZone[v7].field6 <= yp
					        && _vm->_globals.CarreZone[v7].field8 >= yp) {
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

		for (yCurrent = yp; yCurrent >= 0; --yCurrent) {
			v11 = colision(xp, yCurrent);
			v19 = v11;
			if (v11 != -1 && _vm->_globals.ZONEP[v11].field10 == 1)
				break;
		}
		if (v19 == -1)
			goto LABEL_58;
		for (j = yp; j < _vm->_graphicsManager.max_y; ++j) {
			v18 = colision(xp, j);
			if (v18 != -1 && _vm->_globals.ZONEP[v19].field10 == 1)
				break;
		}
		if (v18 == -1)
			goto LABEL_58;
		for (k = xp; k >= 0; --k) {
			v16 = colision(k, yp);
			if (v16 != -1 && _vm->_globals.ZONEP[v19].field10 == 1)
				break;
		}
		if (v16 == -1)
			goto LABEL_58;

		for (xCurrent = xp; _vm->_graphicsManager.max_x > xCurrent; ++xCurrent) {
			v17 = colision(xCurrent, yp);
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
	data = _vm->_fileManager.RECHERCHE_CAT(file, 1);
	if (data == g_PTRNUL) {
		_vm->_fileManager.CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, file);
		data = _vm->_fileManager.CHARGE_FICHIER(_vm->_globals.NFICHIER);
	}
	
	if ((data == g_PTRNUL) || *data != 'I' || *(data + 1) != 'N' || *(data + 2) != 'I') {
		error("Not an INI file");
	} else {
		v7 = 0;
		do {
			v5 = _vm->_scriptManager.Traduction(data + 20 * v0);
			if (v5 == 2)
				v0 = _vm->_scriptManager.Control_Goto(data + 20 * v0);
			if (v5 == 3)
				v0 = _vm->_scriptManager.Control_If(data, v0);
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
					_vm->_fileManager.CONSTRUIT_LINUX("TEMP1.SCR");
					_vm->_saveLoadManager.SAUVE_FICHIER(_vm->_globals.NFICHIER, _vm->_graphicsManager.VESA_SCREEN, 0x4B000u);
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
					_vm->_fileManager.CONSTRUIT_LINUX("TEMP1.SCR");
					_vm->_fileManager.bload(_vm->_globals.NFICHIER, _vm->_graphicsManager.VESA_SCREEN);
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

void ObjectsManager::BOB_VIVANT(int idx) {
	int v1;
	int v2;
	int v3; 
	int v4; 

	v1 = 5 * idx;
	v2 = (int16)READ_LE_UINT16(_vm->_talkManager.ADR_ANIM + 2 * v1);
	v3 = (int16)READ_LE_UINT16(_vm->_talkManager.ADR_ANIM + 2 * v1 + 2);
	v4 = *(_vm->_talkManager.ADR_ANIM + 2 * v1 + 8);
	if ((int16)READ_LE_UINT16(_vm->_talkManager.ADR_ANIM + 2 * v1 + 4)) {
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
	
	if (_vm->_globals.VBob[idx].field4 <= 1) {
		_vm->_globals.VBob[idx].field4 = 1;
		_vm->_globals.VBob[idx].field6 = a3;
		_vm->_globals.VBob[idx].field8 = a4;
		_vm->_globals.VBob[idx].fieldA = a5;
		_vm->_globals.VBob[idx].field14 = a3;
		_vm->_globals.VBob[idx].field16 = a4;
		_vm->_globals.VBob[idx].field18 = a5;
		_vm->_globals.VBob[idx].field0 = a1;
		_vm->_globals.VBob[idx].field1C = a1;
		if (_vm->_globals.VBob[idx].field10 != g_PTRNUL)
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
		if (g_PTRNUL != _vm->_globals.GESTE)
			_vm->_globals.GESTE = _vm->_globals.dos_free2(_vm->_globals.GESTE);
		_vm->_globals.GESTE_FLAG = 1;

		_vm->_fileManager.CONSTRUIT_SYSTEM("DOS.SPR");
		_vm->_globals.GESTE = _vm->_fileManager.CHARGE_FICHIER(_vm->_globals.NFICHIER);
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
		if (_vm->_globals.GESTE != g_PTRNUL)
			_vm->_globals.GESTE = _vm->_globals.dos_free2(_vm->_globals.GESTE);
		_vm->_globals.GESTE_FLAG = 3;
		_vm->_fileManager.CONSTRUIT_SYSTEM("PROFIL.SPR");
		_vm->_globals.GESTE = _vm->_fileManager.CHARGE_FICHIER(_vm->_globals.NFICHIER);
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
		if (_vm->_globals.GESTE != g_PTRNUL)
			_vm->_globals.GESTE = _vm->_globals.dos_free2(_vm->_globals.GESTE);
		_vm->_globals.GESTE_FLAG = 4;
		_vm->_fileManager.CONSTRUIT_SYSTEM("3Q.SPR");
		_vm->_globals.GESTE = _vm->_fileManager.CHARGE_FICHIER(_vm->_globals.NFICHIER);
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
		if (_vm->_globals.GESTE != g_PTRNUL)
			_vm->_globals.GESTE = _vm->_globals.dos_free2(_vm->_globals.GESTE);
		_vm->_globals.GESTE_FLAG = 2;
		_vm->_fileManager.CONSTRUIT_SYSTEM("FACE.SPR");
		_vm->_globals.GESTE = _vm->_fileManager.CHARGE_FICHIER(_vm->_globals.NFICHIER);
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
		if (_vm->_globals.GESTE != g_PTRNUL)
			_vm->_globals.GESTE = _vm->_globals.dos_free2(_vm->_globals.GESTE);
		_vm->_globals.GESTE_FLAG = 4;
		_vm->_fileManager.CONSTRUIT_SYSTEM("3Q.SPR");
		_vm->_globals.GESTE = _vm->_fileManager.CHARGE_FICHIER(_vm->_globals.NFICHIER);
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
		if (_vm->_globals.GESTE != g_PTRNUL)
			_vm->_globals.GESTE = _vm->_globals.dos_free2(_vm->_globals.GESTE);
		_vm->_globals.GESTE_FLAG = 3;
		_vm->_fileManager.CONSTRUIT_SYSTEM("PROFIL.SPR");
		_vm->_globals.GESTE = _vm->_fileManager.CHARGE_FICHIER(_vm->_globals.NFICHIER);
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
		_vm->_globals.BOBZONE_FLAG[idx] = true;
	} else {
		_vm->_globals.ZONEP[idx].field10 = 1;
	}
}

void ObjectsManager::ZONE_OFF(int idx) {
	if (_vm->_globals.BOBZONE[idx]) {
		_vm->_globals.BOBZONE_FLAG[idx] = false;
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
	int16 v27;
	int16 v28;
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
	ptr = _vm->_fileManager.RECHERCHE_CAT(filename, 3);
	nbytes = _vm->_globals.CAT_TAILLE;
	if (ptr == g_PTRNUL) {
		_vm->_fileManager.CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, filename);
		
		if (!f.open(_vm->_globals.NFICHIER))
			error("Error opening file - %s", _vm->_globals.NFICHIER.c_str());

		nbytes = f.size();
		ptr = _vm->_globals.dos_malloc2(nbytes);
		if (g_PTRNUL == ptr)
			error("INILINK");
		_vm->_fileManager.bload_it(f, ptr, nbytes);
		f.close();
	}
	if (!OBSSEUL) {
		for (int idx = 0; idx < 500; ++idx)
			_vm->_globals.STAILLE[idx] = (int16)READ_LE_UINT16((uint16 *)ptr + idx);

		_vm->_globals.RESET_CACHE();
		
		filename2 = Common::String((const char *)ptr + 1000);
		if (!filename2.empty()) {
			_vm->_globals.CACHE_BANQUE[1] = _vm->_fileManager.RECHERCHE_CAT(filename2, 8);
			
			if (_vm->_globals.CACHE_BANQUE[1] || _vm->_globals.CACHE_BANQUE[1] == g_PTRNUL) {
				_vm->_globals.CAT_FLAG = 0;
				_vm->_fileManager.CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, filename2);
			} else {
				_vm->_globals.CAT_FLAG = 1;
				_vm->_fileManager.CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, "RES_SLI.RES");
			}

			_vm->_globals.CACHE_BANQUE[1] = _vm->_fileManager.CHARGE_FICHIER(_vm->_globals.NFICHIER);
			_vm->_globals.CAT_FLAG = 0;
			v36 = 60;
			v37 = ptr + 1000;
			v40 = 0;

			do {
				v8 = (int16)READ_LE_UINT16(v37 + 2 * v36);
				v9 = (int16)READ_LE_UINT16(v37 + 2 * v36 + 2);
				v10 = (int16)READ_LE_UINT16(v37 + 2 * v36 + 4);
				v11 = v40;

				_vm->_globals.Cache[v11].field14 = (int16)READ_LE_UINT16(v37 + 2 * v36 + 8);
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
			if (*(ptr + v41) == 'O' && *(ptr + v41 + 1) == 'B' && *(ptr + v41 + 2) == '2') {
				v16 = ptr + v41 + 4;
				v32 = 0;
				v34 = 0;
				_vm->_linesManager.TOTAL_LIGNES = 0;
				do {
					v27 = (int16)READ_LE_UINT16(v16 + 2 * v32);
					if (v27 != -1) {
						_vm->_linesManager.AJOUTE_LIGNE(
						    v34,
						    v27,
						    (int16)READ_LE_UINT16(v16 + 2 * v32 + 2),
						    (int16)READ_LE_UINT16(v16 + 2 * v32 + 4),
						    (int16)READ_LE_UINT16(v16 + 2 * v32 + 6),
						    (int16)READ_LE_UINT16(v16 + 2 * v32 + 8),
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
						v19 = v18;
						_vm->_globals.ZONEP[v19].destX = 0;
						_vm->_globals.ZONEP[v19].destY = 0;
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
						v28 = (int16)READ_LE_UINT16(v17 + 2 * v33);
						if (v28 != -1) {
							_vm->_linesManager.AJOUTE_LIGNE_ZONE(
							    v35,
							    (int16)READ_LE_UINT16(v17 + 2 * v33 + 2),
							    (int16)READ_LE_UINT16(v17 + 2 * v33 + 4),
							    (int16)READ_LE_UINT16(v17 + 2 * v33 + 6),
							    (int16)READ_LE_UINT16(v17 + 2 * v33 + 8),
							    v28);
							_vm->_globals.ZONEP[v28].field10 = 1;
						}
						v33 += 5;
						++v35;
						++v31;
					} while (v28 != -1);
					v20 = 1;
					do {
						v21 = v20;
						_vm->_globals.ZONEP[v21].destX = (int16)READ_LE_UINT16(v17 + 2 * v33);
						_vm->_globals.ZONEP[v21].destY = (int16)READ_LE_UINT16(v17 + 2 * v33 + 2);
						_vm->_globals.ZONEP[v21].field4 = (int16)READ_LE_UINT16(v17 + 2 * v33 + 4);
						v33 += 3;
						++v20;
					} while (v20 <= 100);

					v22 = ptr + v42 + (10 * v31 + 606) + 4;
					v29 = 0;
					v23 = 1;
					do {
						v24 = v23;
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
						_vm->_globals.ZONEP[v25].field12 = (int16)READ_LE_UINT16(v22 + 1010 + 2 * v30++);
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
		_vm->_globals.BOBZONE_FLAG[20] = true;
		_vm->_globals.BOBZONE_FLAG[21] = true;
		_vm->_globals.BOBZONE_FLAG[22] = true;
		_vm->_globals.BOBZONE_FLAG[23] = true;
		VERBE_ON(20, 5);
		VERBE_ON(21, 5);
		VERBE_ON(22, 5);
		VERBE_ON(23, 5);
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
			BOBANIM_ON(8);
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

int ObjectsManager::colision(int xp, int yp) {
	if (_vm->_globals.SegmentEnCours <= 0)
		return -1;

	int xMax = xp + 4;
	int xMin = xp - 4;

	for (int idx = 0; idx <= _vm->_globals.SegmentEnCours; ++idx) {
		int field2 = _vm->_globals.Segment[idx].field2;
		if (_vm->_globals.Segment[idx].field4 < field2)
			continue;

		int yMax = yp + 4;
		int yMin = yp - 4;

		do {
			int16 *dataP = _vm->_linesManager.LigneZone[field2].zoneData;
			if (dataP != (int16 *)g_PTRNUL) {
				int count = _vm->_linesManager.LigneZone[field2].count;
				int v1 = *dataP;
				int v2 = *(dataP + 1);
				int v3 = *(dataP + count * 2 - 2);
				int v4 = *(dataP + count * 2 - 1);

				bool flag = true;
				if (v1 < v3 && (xMax < v1 || xMin > v3))
					flag = false;
				if (v1 >= v3 && (xMin > v1 || xMax < v3))
					flag = false;
				if (v2 < v4 && (yMax < v2 || yMin > v4))
					flag = false;
				if (v2 >= v4 && (yMin > v2 || yMax < v4))
					flag = false;

				if (flag && _vm->_linesManager.LigneZone[field2].count > 0) {
					for (int v5 = 0; v5 < count; ++v5) {
						int xCheck = *dataP++;
						int yCheck = *dataP++;

						if ((xp == xCheck || (xp + 1) == xCheck) && (yp == yCheck))
							return _vm->_linesManager.LigneZone[field2].field2;
					}
				}
			}
		} while (++field2 <= _vm->_globals.Segment[idx].field4);
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
	int spriteIndex; 
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
	spriteIndex = Sprite[0].spriteIndex;
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
				Sprite[0].spriteIndex = spriteIndex;
				Sprite[0].field12 -= a3;
				Sprite[0].field14 -= a4;
				Sprite[0].fieldE = v16;
			} else {
				Sprite[0].spriteData = spriteData;
				Sprite[0].spriteIndex = v13;
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
}

void ObjectsManager::SPACTION(byte *a1, const Common::String &a2, int a3, int a4, int a5, int a6) {
	int v6; 
	int16 v7; 
	char v8; 
	int v9; 
	int16 v10; 
	int v11; 
	int spriteIndex; 
	char v14; 
	int16 v15; 
	Common::String v16; 

	v15 = 0;
	v6 = 0;
	spriteIndex = 0;
	v16 = "     ";
	v14 = a5;
	if (_vm->_globals.vitesse == 2)
		v14 = a5 / 2;
	if (_vm->_globals.vitesse == 3)
		v14 = a5 / 3;
	S_old_spr = Sprite[0].spriteData;
	S_old_ani = Sprite[0].spriteIndex;
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
				spriteIndex = v9;
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
		if (spriteIndex != -1) {
			Sprite[0].spriteData = a1;
			Sprite[0].spriteIndex = spriteIndex;
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
	} while (spriteIndex != -1);
}

void ObjectsManager::SPACTION1(byte *a1, const Common::String &a2, int a3, int a4, int a5) {
	int v5; 
	int v6; 
	char v7; 
	int v8; 
	int v9; 
	int v10; 
	int spriteIndex; 
	int v13; 
	int v14; 
	Common::String v15; 
	int v16;

	v14 = 0;
	v5 = 0;
	spriteIndex = 0;
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
			spriteIndex = v8;
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
			if (spriteIndex == -1) {
				Sprite[0].spriteData = S_old_spr;
				Sprite[0].spriteIndex = S_old_ani;
				Sprite[0].field12 -= a3;
				Sprite[0].field14 -= a4;
				Sprite[0].fieldE = S_old_ret;
			} else {
				Sprite[0].spriteData = a1;
				Sprite[0].spriteIndex = spriteIndex;
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
			if (spriteIndex == -1)
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
						_vm->_dialogsManager.VIRE_INVENT = true;
						_vm->_soundManager.PLAY_SAMPLE2(1);
						_vm->_globals.SAUVEGARDE->data[v6] = 4;
					}
					if (a6 == 2 && BOBPOSI(2) == 26) {
						_vm->_dialogsManager.VIRE_INVENT = true;
						_vm->_soundManager.PLAY_SAMPLE2(1);
						_vm->_globals.SAUVEGARDE->data[v6] = 4;
					}
					if (a6 == 3 && BOBPOSI(3) == 27) {
						_vm->_dialogsManager.VIRE_INVENT = true;
						_vm->_soundManager.PLAY_SAMPLE2(1);
						_vm->_globals.SAUVEGARDE->data[v6] = 4;
					}
					if (a6 == 4 && BOBPOSI(4) == 27) {
						_vm->_dialogsManager.VIRE_INVENT = true;
						_vm->_soundManager.PLAY_SAMPLE2(1);
						_vm->_dialogsManager.VIRE_INVENT = true;
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
	_vm->_dialogsManager.INVENTFLAG = 0;
	_vm->_eventsManager.KEY_INVENT = 0;
	_vm->_dialogsManager.VIRE_INVENT = false;
	_vm->_graphicsManager.ofscroll = 0;
	_vm->_globals.PLAN_FLAG = 0;
	_vm->_globals.iRegul = 1;
	_vm->_soundManager.WSOUND(v);
	_vm->_globals.chemin = (int16 *)g_PTRNUL;
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
		_vm->_globals.chemin = (int16 *)g_PTRNUL;
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
		_vm->_globals.g_old_sens = -1;
		_vm->_globals.Compteur = 0;
		_vm->_globals.chemin = (int16 *)g_PTRNUL;
		v7 = YSPR(0);
		v8 = XSPR(0);
		_vm->_globals.chemin = _vm->_linesManager.PARCOURS2(v8, v7, 330, 345);
		_vm->_globals.NOT_VERIF = 1;
		do {
			GOHOME();
			_vm->_eventsManager.VBL();
		} while (_vm->_globals.chemin != (int16 *)g_PTRNUL);
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
		_vm->_dialogsManager.TEST_INVENT();
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
	int mouseButtons;
	bool breakFlag;
	int xp, yp;

	_vm->_dialogsManager.INVENTFLAG = 0;
	_vm->_eventsManager.KEY_INVENT = 0;
	verbe = 4;
	_vm->_globals.MAX_COMPTE = 6;
	_vm->_graphicsManager.ofscroll = 0;
	_vm->_dialogsManager.VIRE_INVENT = false;
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
		INILINK(s2);
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
		_vm->_fileManager.CONSTRUIT_SYSTEM("PERSO.SPR");
		_vm->_fileManager.CHARGE_FICHIER2(_vm->_globals.NFICHIER, _vm->_globals.PERSO);
		_vm->_globals.PERSO_TYPE = 0;
	}
	if (!_vm->_globals.PERSO_TYPE) {
LABEL_70:
		if (_vm->_globals.SAUVEGARDE->data[svField122] == 1) {
			_vm->_fileManager.CONSTRUIT_SYSTEM("HOPFEM.SPR");
			_vm->_fileManager.CHARGE_FICHIER2(_vm->_globals.NFICHIER, _vm->_globals.PERSO);
			_vm->_globals.PERSO_TYPE = 1;
		}
	}
	if (_vm->_globals.PERSO_TYPE != 2 && _vm->_globals.SAUVEGARDE->data[svField356] == 1) {
		_vm->_fileManager.CONSTRUIT_SYSTEM("PSAMAN.SPR");
		_vm->_fileManager.CHARGE_FICHIER2(_vm->_globals.NFICHIER, _vm->_globals.PERSO);
		_vm->_globals.PERSO_TYPE = 2;
	}
	_vm->_globals.HOPKINS_DATA();
	if (!_vm->_globals.PERSO_TYPE)
		SPRITE(_vm->_globals.PERSO, PERX, PERY, 0, PERI, 0, 0, 34, 190);
	if (_vm->_globals.PERSO_TYPE == 1)
		SPRITE(_vm->_globals.PERSO, PERX, PERY, 0, PERI, 0, 0, 28, 155);
	if (_vm->_globals.PERSO_TYPE == 2)
		SPRITE(_vm->_globals.PERSO, PERX, PERY, 0, PERI, 0, 0, 20, 127);
	_vm->_eventsManager.souris_xy(PERX, PERY);
	if (_vm->_graphicsManager.DOUBLE_ECRAN == 1)
		_vm->_graphicsManager.SCROLL = (int16)XSPR(0) - 320;
	VERIFTAILLE();
	SPRITE_ON(0);
	_vm->_globals.CACHE_ON();
	_vm->_globals.chemin = (int16 *)g_PTRNUL;
	VERIFTAILLE();
	SPECIAL_INI(s2);
	_vm->_eventsManager.souris_n = 4;
	g_old_x = PERX;
	g_old_y = PERY;
	_vm->_globals.g_old_sens = -1;
	_vm->_globals.Compteur = 0;
	_vm->_globals.BPP_NOAFF = 1;

	for (int idx = 0; idx < 5; ++idx) {
		_vm->_eventsManager.VBL();
	}

	_vm->_globals.BPP_NOAFF = 0;
	_vm->_globals.iRegul = 1;
	if (!_vm->_graphicsManager.NOFADE)
		_vm->_graphicsManager.FADE_INW();
	_vm->_graphicsManager.NOFADE = 0;
	_vm->_eventsManager.CHANGE_MOUSE(4);

	int xCheck = 0; 
	int yCheck = 0; 

	breakFlag = false;
	while (!_vm->shouldQuit() && !breakFlag) {
		mouseButtons = _vm->_eventsManager.BMOUSE();
		if (mouseButtons) {
			if (mouseButtons == 1) {
				if (verbe == 16 && _vm->_eventsManager.btsouris == 16) {
					xp = _vm->_eventsManager.XMOUSE();
					yp = _vm->_eventsManager.YMOUSE();

					if (xCheck == xp) {
						if (yCheck == yp) {
							_vm->_globals.chemin = (int16 *)g_PTRNUL;
							PARADISE();
							if (_vm->_globals.SORTIE)
								breakFlag = true;
						}
					}
					xCheck = xp;
					yCheck = yp;
				}
				BTGAUCHE();
			} else if (mouseButtons == 2) {
				BTDROITE();
			}
		}
		if (!_vm->_globals.SORTIE) {
			_vm->_dialogsManager.TEST_INVENT();
			VERIFZONE();
			if (_vm->_globals.chemin == (int16 *)g_PTRNUL 
					|| (GOHOME(), _vm->_globals.chemin == (int16 *)g_PTRNUL)) {
				if (GOACTION == 1)
					PARADISE();
			}
			SPECIAL_JEU();
			_vm->_eventsManager.VBL();
			if (!_vm->_globals.SORTIE)
				continue;
		}
		breakFlag = true;
	}

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
