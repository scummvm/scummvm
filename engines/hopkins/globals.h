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

#ifndef HOPKINS_GLOBALS_H
#define HOPKINS_GLOBALS_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/events.h"

namespace Hopkins {


struct BqeAnimItem {
	byte *_data;
	bool _enabledFl;
};

struct BankItem {
	byte *_data;
	bool _loadedFl;
	Common::String _filename;
	int _fileHeader;
	int _objDataIdx;
};

struct ListeItem {
	bool _visibleFl;
	int _posX;
	int _posY;
	int _width;
	int _height;
};

struct LockAnimItem {
	bool _enableFl;
	int _posX;
};

struct VBobItem {
	byte *_spriteData;
	int field4;
	int _xp;
	int _yp;
	int _frameIndex;
	byte *_surface;
	int _oldX;
	int _oldY;
	int _oldFrameIndex;
	byte *_oldSpriteData;
};

struct ObjetWItem {
	byte field0;
	byte _idx;
	byte field2;
	byte field3;
	byte field4;
	byte field5;
	byte field6;
	byte field7;
};

/**
 * Mode for SortItem records
 */
enum SortMode { SORT_NONE = 0, SORT_BOB = 1, SORT_SPRITE = 2, SORT_CACHE = 3 };

/**
 * Structure to represent a pending display of either a Bob, Sprite, or Cache Item.
 */
struct SortItem {
	SortMode _sortMode;
	int _index;
	int _priority;
};

struct CacheItem {
	int _x;
	int _spriteIndex;
	int _y;
	int _width;
	int _height;
	int _useCount;
	byte *_spriteData;
	bool field10;
	int field14;
};

struct HopkinsItem {
	int field0;
	int field2;
};

struct CharacterLocation {
	Common::Point _pos;
	int _startSpriteIndex;
	int _location;
	int _zoomFactor;
};

enum SauvegardeOffset {
	svField1 = 1
	, svField2 = 2
	, svField3 = 3
	, svField4 = 4
	, svField5 = 5
	, svField6 = 6
	, svField8 = 8
	, svField9 = 9
	, svField10 = 10
	, svField13 = 13
	, svField80 = 80
	, svField94 = 94
	, svField95 = 95
	, svField113 = 113
	, svField117 = 117
	, svField121 = 121
	, svField122 = 122
	, svField123 = 123
	, svField132 = 132
	, svField133 = 133
	, svField135 = 135
	, svField166 = 166
	, svField167 = 167
	, svField170 = 170
	, svField171 = 171
	, svField172 = 172
	, svField173 = 173
	, svField176 = 176
	, svField177 = 177
	, svField180 = 180
	, svField181 = 181
	, svField182 = 182
	, svField183 = 183
	, svField184 = 184
	, svField185 = 185
	, svField186 = 186
	, svField187 = 187
	, svField188 = 188
	, svField189 = 189
	, svField190 = 190
	, svField191 = 191
	, svField192 = 192
	, svField193 = 193
	, svField194 = 194
	, svField220 = 220
	, svField225 = 225
	, svField228 = 228
	, svField231 = 231
	, svField253 = 253
	, svField261 = 261
	, svField270 = 270
	, svField300 = 300
	, svField311 = 311
	, svField312 = 312
	, svField318 = 318
	, svField320 = 320
	, svField330 = 330
	, svField333 = 333
	, svField338 = 338
	, svField339 = 339
	, svField340 = 340
	, svField341 = 341
	, svField349 = 349
	, svField352 = 352
	, svField353 = 353
	, svField354 = 354
	, svField355 = 355
	, svField356 = 356
	, svField357 = 357
	, svField399 = 399
	, svField401 = 401
};

enum PlayerCharacter { CHARACTER_HOPKINS = 0, CHARACTER_HOPKINS_CLONE = 1, CHARACTER_SAMANTHA = 2 };

// TODO: Sauvegrade1 fields should really be mapped into data array
struct Sauvegarde {
	byte _data[2050];
	CharacterLocation _cloneHopkins;
	CharacterLocation _realHopkins;
	CharacterLocation _samantha;
	int16 _inventory[35];	// Originally at offset 1300 of data array
	int16 _mapCarPosX;
	int16 _mapCarPosY;
};

struct CreditItem {
	bool _actvFl;
	int _colour;
	int _linePosY;
	int _lineSize;
	byte _line[50];
};

enum Language { LANG_EN = 0, LANG_FR = 1, LANG_SP = 2};

class HopkinsEngine;

/**
 * Engine Globals
 */
class Globals {
private:
	HopkinsEngine *_vm;

	byte *_inventoryObject;

	void INIT_ANIM();
	void INIT_VBOB();

public:
	bool _disableInventFl;
	bool _cityMapEnabledFl;
	bool _linuxEndDemoFl;
	bool _censorshipFl;
	bool _introSpeechOffFl;
	bool _cacheFl;
	bool _forceHideText;
	int _exitId;
	int _oceanDirection;
	int _oldDirection, _oldDirectionSpriteIdx;
	int _actionDirection;
	int _lastDirection;
	int _oldFrameIndex;
	int _hotspotTextColor;
	int _inventory[36];
	int _objectWidth, _objectHeight;
	int _screenId;
	int _prevScreenId;
	int _boxWidth;
	int _characterMaxPosY;
	int _baseMapColor;
	int _spriteSize[500];
	int _sortedDisplayCount;
	int _oldMouseZoneId;
	int _oldMouseX, _oldMouseY;
	uint _speed;
	byte *_answerBuffer;
	Sauvegarde *_saveData;
	Language _language;
	HopkinsItem Hopkins[70];
	SortItem _sortedDisplay[51];

	CreditItem _creditsItem[200];
	int _creditsLineNumb;
	int _creditsStep;
	int _creditsPosY;
	int Credit_bx;
	int Credit_bx1;
	int Credit_by;
	int Credit_by1;

	int _menuSpeed;
	int _menuSoundOff;
	int _menuTextOff;
	int _menuVoiceOff;
	int _menuMusicOff;
	int _menuDisplayType;
	int _menuScrollSpeed;

	byte *_optionDialogSpr;
	bool _optionDialogFl;
	uint32 _catalogPos;
	uint32 _catalogSize;
	LockAnimItem _lockedAnims[30];
	int _oldRouteFromX;
	int _oldRouteFromY;
	int _oldRouteDestX;
	int _oldRouteDestY;
	int _oldZoneNum;

	byte *TETE;
	byte *ICONE;
	bool NOMARCHE;
	int iRegul;
	byte *BUF_ZONE;
	int nbrligne; // Useless variable?
	BqeAnimItem Bqe_Anim[35];
	byte *SPRITE_ECRAN;
	byte *PERSO;
	int PERSO_TYPE;
	bool NOT_VERIF;
	ObjetWItem ObjetW[300];
	int NUM_FICHIER_OBJ;
	byte *ADR_FICHIER_OBJ;
	ListeItem Liste[6];
	ListeItem Liste2[35];
	BankItem Bank[8];
	byte *CACHE_BANQUE[6];
	CacheItem Cache[25];
	VBobItem VBob[30];
	int Compteur;
	int compteur_71;
	Common::String FICH_ZONE;
	bool GOACTION;
	int j_104;
	int _curGestureFile;
	byte *_gestureBuf;
	Common::String FICH_TEXTE;

	Globals();
	~Globals();
	void setParent(HopkinsEngine *vm);
	byte *allocMemory(int count);
	byte *freeMemory(byte *p);
	void setConfig();
	void loadObjects();
	void clearAll();
	void loadCharacterData();
	void resetCache();
	void loadCache(const Common::String &file);

	void CLEAR_VBOB();
	void CACHE_ON();
	void CACHE_OFF();
	void B_CACHE_OFF(int idx);
	void CACHE_SUB(int idx);
	void CACHE_ADD(int idx);
};

// Global null pointer
extern byte *g_PTRNUL;

} // End of namespace Hopkins

#endif /* HOPKINS_GLOBALS_H */
