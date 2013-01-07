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

struct ZonePItem {
	int _destX;
	int _destY;
	int field4;
	int field6;
	int field7;
	int field8;
	int field9;
	int fieldA;
	int fieldB;
	int fieldC;
	int fieldD;
	int fieldE;
	int fieldF;
	int field10;
	int field12;
	int field16;
};

struct CarreZoneItem {
	int field0;
	int field2;
	int field4;
	int field6;
	int field8;
	int fieldA;
	int fieldC;
	int fieldE;
};

struct BqeAnimItem {
	byte *_data;
	bool _enabledFl;
};

struct BankItem {
	byte *_data;
	bool _loadedFl;
	Common::String _filename;
	int _fileHeader;
	int field1A;
	uint32 field1C;
};

struct ListeItem {
	bool field0;
	int field2;
	int field4;
	int _width;
	int _height;
	int fieldA;
};

struct Liste2Item {
	bool _visibleFl;
	int _xp;
	int _yp;
	int _width;
	int _height;
};

struct BobItem {
	int field0;
	byte *_spriteData;
	int _xp;
	int _yp;
	int _frameIndex;
	int field10; // BOBPOSI ?
	int field12;
	int field14;
	bool _disabledAnimationFl;
	byte *_animData;
	bool field1C;
	int field1E;
	int field20;
	int field22;
	int _offsetY; // Unused variable?
	byte *field30;
	bool field34; // Set to true in B_CACHE_OFF()
	int field36;
	int _modeFlag;
	bool _isSpriteFl;
	bool _activeFl;
	int _oldX;
	int _oldY;
	int _oldWidth;
	int _oldHeight;
	int _oldX2;
	int _oldY2;
	int field4A;
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

struct BlocItem {
	uint16 _activeFl;
	int _x1;
	int _y1;
	int _x2;
	int _y2;
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
	int fieldA;
	byte *_spriteData;
	bool field10;
	int field14;
};

struct SegmentItem {
	int field0;
	int field2;
	int field4;
};

struct HopkinsItem {
	int field0;
	int field2;
};

struct CharacterLocation {
	Common::Point _pos;
	int field2;
	int _location;
	int field4;
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
	byte data[2050];
	CharacterLocation _cloneHopkins;
	CharacterLocation _realHopkins;
	CharacterLocation _samantha;
	int16 _inventory[35];	// Originally at offset 1300 of data array
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
public:
	byte *CACHE_BANQUE[6];
	ZonePItem ZONEP[106];
	CarreZoneItem CarreZone[101];
	BqeAnimItem Bqe_Anim[35];
	BankItem Bank[8];
	BobItem _bob[36];
	ListeItem Liste[6];
	Liste2Item Liste2[35];
	LockAnimItem _lockedAnims[30];
	VBobItem VBob[30];
	ObjetWItem ObjetW[300];
	BlocItem BLOC[250];
	CacheItem Cache[25];
	SegmentItem Segment[101];
	int BOBZONE[105];
	bool BOBZONE_FLAG[105];
	int STAILLE[500];
	int16 super_parcours[32002];
	int Param[2100];
	HopkinsItem Hopkins[70];
	int _inventory[36];
	SortItem _sortedDisplay[51];
	Language _language;
	int SVGA;
	bool _internetFl;
	bool PUBEXIT;
	int PERSO_TYPE;
	uint _speed;
	Common::String HOPIMAGE;
	Common::String HOPANIM;
	Common::String HOPLINK;
	Common::String HOPSAVE;
	Common::String HOPSOUND;
	Common::String HOPMUSIC;
	Common::String HOPVOICE;
	Common::String HOPANM;
	Common::String HOPTSVGA;
	Common::String HOPSVGA;
	Common::String HOPVGA;
	Common::String HOPSEQ;
	Common::String HOPSYSTEM;
	Common::String FICH_ZONE;
	Common::String FICH_TEXTE;
	Common::String _curFilename;
	bool _censorshipFl;
	int g_old_anim;
	int g_old_sens, g_old_sens2;
	int _lastDirection;
	byte *police;
	int police_l;
	int police_h;
	byte *TETE;
	byte *ICONE;
	byte *BUF_ZONE;
	byte *texte_tmp;
	int texte_long;
	byte *SPRITE_ECRAN;
	Sauvegarde *_saveData;
	byte *BUFFERTAPE;
	int16 *essai0;
	int16 *essai1;
	int16 *essai2;
	byte *inventaire2;
	byte *GESTE;
	int _curObjectIndex;
	int NUM_FICHIER_OBJ;
	int nbrligne; // Useless variable?
	int _boxWidth;
	byte *_inventoryObject;
	int GESTE_FLAG;
	byte *_forestSprite;
	bool _forestFl;
	byte *COUCOU;
	int16 *chemin;
	byte *cache_souris;
	int16 *BufLig;
	byte *Bufferdecor;
	byte *ADR_FICHIER_OBJ;
	byte *PERSO;
	int _objectWidth, _objectHeight;
	bool _newObjectFl;
	bool _helicopterFl;
	uint32 _catalogPos;
	uint32 _catalogSize;
	int iRegul;
	int _exitId;
	int PLANX, PLANY;
	int PLANI;
	bool NOSPRECRAN;
	int _screenId;
	int _prevScreenId;
	int fmusic;
	int Max_Propre;
	int Max_Ligne_Long;
	int Max_Perso_Y;
	bool _disableInventFl;
	bool NOMARCHE;
	int NBBLOC;
	bool NO_VISU;
	byte *OPTION_SPR;
	bool _optionDialogFl;
	int _menuDisplayType;
	int _menuSpeed;
	int _menuScrollType;
	int _menuScrollSpeed;
	int _menuSoundOff;
	int _menuVoiceOff;
	int _menuMusicOff;
	int _menuTextOff;
	int _sortedDisplayCount;
	bool NOT_VERIF;
	bool _cacheFl;
	bool NOPARLE;
	bool PLAN_FLAG;
	bool GOACTION;
	int Compteur;
	int _actionDirection;
	int SegmentEnCours;
	int couleur_40;

	int STOP_BUG;
	int MAX_COMPTE;

	int force_to_data_0;
	int oldzone_46;
	int old_x1_65;
	int old_y1_66;
	int old_x2_67;
	int old_zone_68;
	int old_x_69, old_y_70;
	int compteur_71;
	int zozo_73;
	int old_y2_68;
	int old_z_69;
	int j_104;

	int Credit_bx;
	int Credit_bx1;
	int Credit_by;
	int Credit_by1;
	int Credit_y;
	int Credit_lignes;
	CreditItem Credit[200];
	int Credit_step;
	int Credit_l;
	int Credit_h;

	int _oceanDirection;

	Globals();
	~Globals();
	void setParent(HopkinsEngine *vm);

	void setConfig();
	void clearAll();
	void loadCharacterData();
	void INIT_ANIM();
	void INIT_VBOB();
	void CLEAR_VBOB();
	void loadObjects();
	byte *allocMemory(int count);
	byte *freeMemory(byte *p);

	void resetCache();
	void CACHE_ON();
	void CACHE_OFF(int v1 = 0);
	void CACHE_SUB(int idx);
	void CACHE_ADD(int idx);
	void loadCache(const Common::String &file);
	void B_CACHE_OFF(int idx);
};

// Global null pointer
extern byte *g_PTRNUL;

} // End of namespace Hopkins

#endif /* HOPKINS_GLOBALS_H */
