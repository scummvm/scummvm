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
	bool _rleFl;
	bool field2A;
	int _destX;
	int _destY;
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
	byte *_saveLoadSprite;
	byte *_saveLoadSprite2;
	byte *_spritePtr;
	const byte *S_old_spr;
	bool PERSO_ON;
	bool _saveLoadFl;
	bool _visibleFl;
	bool BOBTOUS;
	int my_anim;
	int _zoneNum;
	bool _forceZoneFl;
	bool _changeVerbFl;
	int _verb;
	int _lastLine;
	bool _changeHeadFl;
	bool _disableFl;
	bool _twoCharactersFl;
	Common::Point _characterPos;
	int PERI;
	bool OBSSEUL;
	int _jumpVerb;
	int _jumpZone;
	int _oldSpriteIndex;
	int S_old_ret;
public:
	ObjectsManager();
	void setParent(HopkinsEngine *vm);

	void changeObject(int objIndex);
	byte *CAPTURE_OBJET(int objIndex, bool mode);
	void removeObject(int objIndex);

	int getWidth(const byte *objectData, int idx);
	int getHeight(const byte *objectData, int idx);
	void sprite_alone(const byte *objectData, byte *sprite, int objIndex);
	void DEL_FICHIER_OBJ();

	byte *loadSprite(const Common::String &file);
	void setOffsetXY(byte *data, int idx, int xp, int yp, bool isSize);
	int getOffsetX(const byte *spriteData, int spriteIndex, bool isSize);
	int getOffsetY(const byte *spriteData, int spriteIndex, bool isSize);
	void displaySprite();

	void capture_mem_sprite(const byte *objectData, byte *sprite, int objIndex);
	void addObject(int objIndex);

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
	void beforeSort(SortMode triMode, int index, int priority);
	void displayBobAnim();
	void displayVBob();

	int getSpriteX(int idx);
	int getSpriteY(int idx);
	void clearSprite();
	void animateSprite(int idx);
	void addStaticSprite(const byte *spriteData, Common::Point pos, int idx, int spriteIndex, int a6, int a7, int a8, int a9);
	void removeSprite(int idx);
	void setSpriteX(int idx, int xp);
	void setSpriteIndex(int idx, int spriteIndex);
	void setSpriteY(int idx, int yp);
	void setSpriteSize(int idx, int size);
	void setFlipSprite(int idx, bool flip);

	void checkZone();
	void GOHOME();
	void GOHOME2();
	void loadLines(const Common::String &file);
	void loadZone(const Common::String &file);
	void CARRE_ZONE();
	void PLAN_BETA();
	void handleLeftButton();
	void PARADISE();
	void clearScreen();

	/**
	 * Change the currently active player
	 * @param oldCharacter		Previously played character
	 * @param newCharacter		New character to play
	 */
	void changeCharacterHead(PlayerCharacter oldCharacter, PlayerCharacter newCharacter);

	void computeAndSetSpriteSize();
	void PACOURS_PROPRE(int16 *a1);
	int16 *cityMapCarRoute(int x1, int y1, int x2, int y2);
	void nextVerbIcon();
	void handleRightButton();
	int MZONE();
	void initBorder(int a1);
	void nextObjectIcon(int idx);
	void takeInventoryObject(int idx);
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

	void enableZone(int idx);
	void disableZone(int idx);
	void OPTI_ONE(int idx, int fromPosi, int destPosi, int a4);
	int BOBPOSI(int idx);
	void setBobAnimation(int idx);
	void stopBobAnimation(int idx);
	void SET_BOBPOSI(int idx, int a2);
	int getBobPosX(int idx);
	int getBobPosY(int idx);
	int getBobFrameIndex(int idx);

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
