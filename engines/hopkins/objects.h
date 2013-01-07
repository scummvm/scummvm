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

#ifndef HOPKINS_OBJECTS_H
#define HOPKINS_OBJECTS_H

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/str.h"
#include "hopkins/globals.h"

#define MAX_SPRITE 5
namespace Hopkins {

struct SpriteItem {
	int _animationType;
	const byte *_spriteData;
	Common::Point _spritePos;
	int fieldC;
	int fieldE;
	int _spriteIndex;
	int field12;
	int field14;
	byte *field1C;
	int field20;
	int field22;
	int field24;
	int field26;
	bool _rleFl;
	bool field2A;
	int field2C;
	int field2E;
	int _width;
	int _height;
	int _zoomPct;
	int _reducePct;
};

class HopkinsEngine;

class ObjectsManager {
private:
	HopkinsEngine *_vm;

public:
	SpriteItem _sprite[6];
	bool _priorityFl;
	Common::Point _oldBorderPos; 
	int _oldBorderSpriteIndex;
	Common::Point _borderPos;
	int _borderSpriteIndex;
	int _saveLoadX, _saveLoadY;
	int I_old_x, I_old_y;
	int g_old_x, g_old_y;
	int FLAG_VISIBLE_EFFACE;
	byte *SL_SPR;
	byte *SL_SPR2;
	byte *_spritePtr;
	const byte *S_old_spr;
	bool PERSO_ON;
	bool _saveLoadFl;
	int SL_MODE;
	bool _visibleFl;
	bool BOBTOUS;
	int my_anim;
	int NUMZONE;
	bool _forceZoneFl;
	bool _changeVerbFl;
	int _verb;
	int Vold_taille;
	int SPEED_X, SPEED_Y;
	int SPEED_IMAGE;
	byte *SPEED_PTR;
	int _lastLine;
	int A_ANIM;
	int MA_ANIM;
	int MA_ANIM1;
	int A_DEPA;
	int MAX_DEPA;
	int MAX_DEPA1;
	bool CH_TETE;
	int T_RECTIF;
	bool _disableFl;
	bool _twoCharactersFl;
	Common::Point _characterPos;
	int PERI;
	int RECALL;
	int PTAILLE;
	int PEROFX;
	int PEROFY;
	int OBSSEUL;
	int NVVERBE;
	int NVZONE;
	int S_old_ani;
	int S_old_ret;
	int nouveau_x, nouveau_y;
	int nouveau_sens;
	int nouveau_anim;
public:
	ObjectsManager();
	void setParent(HopkinsEngine *vm);

	void changeObject(int objIndex);
	byte *CAPTURE_OBJET(int objIndex, int mode);
	void removeObject(int objIndex);

	int getWidth(const byte *objectData, int idx);
	int getHeight(const byte *objectData, int idx);
	int sprite_alone(const byte *objectData, byte *sprite, int objIndex);
	void DEL_FICHIER_OBJ();

	byte *loadSprite(const Common::String &file);
	void setOffsetXY(byte *data, int idx, int xp, int yp, bool isSize);
	int getOffsetX(const byte *spriteData, int spriteIndex, bool isSize);
	int getOffsetY(const byte *spriteData, int spriteIndex, bool isSize);
	void displaySprite();

	int capture_mem_sprite(const byte *objectData, byte *sprite, int objIndex);
	int addObject(int objIndex);

	void INIT_BOB();
	void BOB_ZERO(int idx);
	void DEF_BOB(int idx);
	void BOB_VISU(int idx);
	void BOB_OFF(int idx);
	void BOB_OFFSET(int idx, int v);
	void BOB_ADJUST(int idx, int v);
	void BOB_OFFSETY(int idx, int v);
	void SCBOB(int idx);
	void CALCUL_BOB(int idx);

	void checkCache();
	void DEF_SPRITE(int idx);
	void DEF_CACHE(int idx);
	void computeSprite(int idx);
	int beforeSort(SortMode triMode, int index, int priority);
	void displayBobAnim();
	void displayVBob();

	int getSpriteX(int idx);
	int getSpriteY(int idx);
	void clearSprite();
	void SPRITE_ON(int idx);
	void SPRITE(const byte *spriteData, Common::Point pos, int idx, int spriteIndex, int a6, int a7, int a8, int a9);
	void SPRITE2(const byte *spriteData, int idx, byte *a3, int a4, int a5);
	void removeSprite(int idx);
	void setSpriteX(int idx, int xp);
	void setSpriteIndex(int idx, int spriteIndex);
	void setSpriteY(int idx, int yp);
	void SETTAILLESPR(int idx, int a2);
	void setFlipSprite(int idx, bool flip);

	void checkZone();
	void GOHOME();
	void GOHOME2();
	void CHARGE_OBSTACLE(const Common::String &file);
	void loadZone(const Common::String &file);
	void CARRE_ZONE();
	void PLAN_BETA();
	void handleLeftButton();
	void PARADISE();
	void CLEAR_ECRAN();

	/**
	 * Change the currently active player
	 * @param oldCharacter		Previously played character
	 * @param newCharacter		New character to play
	 */
	void changeCharacterHead(PlayerCharacter oldCharacter, PlayerCharacter newCharacter);

	void VERIFTAILLE();
	void PACOURS_PROPRE(int16 *a1);
	int16 *PARC_VOITURE(int a1, int a2, int a3, int a4);
	void VERBEPLUS();
	void handleRightButton();
	int MZONE();
	void initBorder(int a1);
	void OBJETPLUS(int a1);
	void VALID_OBJET(int a1);
	void OPTI_OBJET();
	void handleSpecialGames();
	void BOB_VIVANT(int a1);
	void VBOB(byte *src, int idx, int xp, int yp, int frameIndex);
	void VBOB_OFF(int idx);
	void ACTION_DOS(int idx);
	void ACTION_DROITE(int idx);
	void Q_DROITE(int idx);
	void ACTION_FACE(int idx);
	void Q_GAUCHE(int idx);
	void ACTION_GAUCHE(int idx);

	void ZONE_ON(int idx);
	void disableZone(int idx);
	void OPTI_ONE(int a1, int a2, int a3, int a4);
	void AFFICHE_SPEED1(byte *speedData, int xp, int yp, int img);
	int BOBPOSI(int idx);
	void setBobAnimation(int idx);
	void stopBobAnimation(int idx);
	void SET_BOBPOSI(int idx, int a2);
	int getBobPosX(int idx);
	int BOBY(int idx);
	int BOBA(int idx);

	void INILINK(const Common::String &file);
	void SPECIAL_INI();
	void OPTI_BOBON(int idx1, int idx2, int idx3, int a4, int a5, int a6, int a7);
	void SCI_OPTI_ONE(int idx, int a2, int a3, int a4);
	void disableVerb(int idx, int a2);
	void enableVerb(int idx, int a2);
	int CALC_PROPRE(int idx);
	int colision(int xp, int yp);

	void ACTION(const byte *spriteData, const Common::String &a2, int a3, int a4, int speed, int a6);
	void SPACTION(byte *a1, const Common::String &animationSeq, int a3, int a4, int speed, int a6);
	void SPACTION1(byte *spriteData, const Common::String &animString, int a3, int a4, int speed);
	void handleForest(int screenId, int minX, int maxX, int minY, int maxY, int idx);
	void lockAnimX(int idx, int a2);

	/**
	 * Game scene control method
	 */
	void PERSONAGE(const Common::String &backgroundFile, const Common::String &linkFile,
			const Common::String &animFile, const Common::String &s4, int v, bool initializeScreen);

	/**
	 * Game scene control method
	 */
	void PERSONAGE2(const Common::String &backgroundFile, const Common::String &linkFile,
			const Common::String &animFile, const Common::String &s4, int v, bool initializeScreen);
};

} // End of namespace Hopkins

#endif /* HOPKINS_OBJECTS_H */
