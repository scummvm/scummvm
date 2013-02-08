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
	int _zoomFactor;
	bool _flipFl;
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

	int _oldBorderSpriteIndex;
	int _borderSpriteIndex;
	byte *_spritePtr;
	const byte *_oldSpriteData;
	int _verb;
	int _oldSpriteIndex;
	bool _oldFlipFl;
private:
	void sprite_alone(const byte *objectData, byte *sprite, int objIndex);
	void DEL_FICHIER_OBJ();

	int getOffsetX(const byte *spriteData, int spriteIndex, bool isSize);
	int getOffsetY(const byte *spriteData, int spriteIndex, bool isSize);

	void capture_mem_sprite(const byte *objectData, byte *sprite, int objIndex);
	void initBob();
	void setBobInfo(int idx);
	void SCBOB(int idx);
	void CALCUL_BOB(int idx);

	void checkCache();
	void DEF_SPRITE(int idx);
	void displayCache(int idx);
	void computeSprite(int idx);
	void beforeSort(SortMode sortMode, int index, int priority);
	void displayBobAnim();
	void displayVBob();

	void clearSprite();
	void setSpriteZoom(int idx, int zoomFactor);
	
	void GOHOME2();
	void loadZone(const Common::String &file);

	/**
	 * Change the currently active player
	 * @param oldCharacter		Previously played character
	 * @param newCharacter		New character to play
	 */
	void changeCharacterHead(PlayerCharacter oldCharacter, PlayerCharacter newCharacter);

	void nextVerbIcon();

	int getBobFrameIndex(int idx);

	void SPECIAL_INI();

	void ACTION(const byte *spriteData, const Common::String &actionStr, int a3, int a4, int speed, bool flipFl);
	void handleForest(int screenId, int minX, int maxX, int minY, int maxY, int idx);
public:
	bool _disableFl;
	bool _forestFl;
	bool _visibleFl;
	bool _saveLoadFl;
	bool _forceZoneFl;
	bool _changeVerbFl;
	bool _helicopterFl;
	bool _twoCharactersFl;
	bool _changeHeadFl;
	bool _priorityFl;
	int _jumpVerb;
	int _jumpZone;
	int _zoneNum;
	int _eraseVisibleCounter;
	int _curObjectIndex;
	int _startSpriteIndex;
	int _oldInventoryPosX, _oldInventoryPosY;
	int _saveLoadX, _saveLoadY;
	int _mapCarPosX, _mapCarPosY;
	int _oldCharacterPosX, _oldCharacterPosY;
	Common::Point _borderPos;
	Common::Point _oldBorderPos; 
	Common::Point _characterPos;
	byte *_forestSprite;
	byte *_saveLoadSprite;
	byte *_saveLoadSprite2;
	SpriteItem _sprite[6];

	bool PERSO_ON;
	bool BOBTOUS;
	bool OBSSEUL;

	ObjectsManager();
	~ObjectsManager();

	void setParent(HopkinsEngine *vm);
	void clearAll();

	int getWidth(const byte *objectData, int idx);
	int getHeight(const byte *objectData, int idx);
	byte *loadSprite(const Common::String &file);
	void addStaticSprite(const byte *spriteData, Common::Point pos, int idx, int spriteIndex, int zoomFactor, bool flipFl, int a8, int a9);
	void animateSprite(int idx);
	void removeSprite(int idx);
	void setSpriteX(int idx, int xp);
	void setSpriteY(int idx, int yp);
	int getSpriteX(int idx);
	int getSpriteY(int idx);
	void setSpriteIndex(int idx, int spriteIndex);
	void displaySprite();
	void computeAndSetSpriteSize();
	void setFlipSprite(int idx, bool flip);

	int getBobAnimDataIdx(int idx);
	void initBorder(int zoneIdx);
	void nextObjectIcon(int idx);
	void takeInventoryObject(int idx);
	void handleSpecialGames();

	void addObject(int objIndex);
	void changeObject(int objIndex);
	void removeObject(int objIndex);

	void checkZone();
	void enableZone(int idx);
	void disableZone(int idx);

	void resetBob(int idx);
	void setBobAnimDataIdx(int idx, int animIdx);
	void setBobAnimation(int idx);
	void stopBobAnimation(int idx);
	int getBobPosX(int idx);
	int getBobPosY(int idx);

	void handleCityMap();
	void clearScreen();
	void disableVerb(int idx, int a2);
	void enableVerb(int idx, int a2);
	void lockAnimX(int idx, int x);
	void handleLeftButton();
	void handleRightButton();
	void setOffsetXY(byte *data, int idx, int xp, int yp, bool isSize);

	void PERSONAGE(const Common::String &backgroundFile, const Common::String &linkFile,
		const Common::String &animFile, const Common::String &s4, int v, bool initializeScreen);
	void PERSONAGE2(const Common::String &backgroundFile, const Common::String &linkFile,
		const Common::String &animFile, const Common::String &s4, int v, bool initializeScreen);
	byte *CAPTURE_OBJET(int objIndex, bool mode);
	void OPTI_OBJET();
	void BOB_OFF(int idx);
	void BOB_VISU(int idx);
	void INILINK(const Common::String &file);
	void SPACTION(byte *spriteData, const Common::String &animationSeq, int a3, int a4, int speed, bool flipFl);
	void BOB_VIVANT(int idx);
	void VBOB(byte *src, int idx, int xp, int yp, int frameIndex);
	void VBOB_OFF(int idx);
	void ACTION_DOS(int idx);
	void ACTION_DROITE(int idx);
	void Q_DROITE(int idx);
	void ACTION_FACE(int idx);
	void Q_GAUCHE(int idx);
	void ACTION_GAUCHE(int idx);
	void OPTI_ONE(int idx, int animIdx, int destPosi, int animAction);
	void SCI_OPTI_ONE(int idx, int animIdx, int a3, int a4);
	void GOHOME();
	void OPTI_BOBON(int idx1, int idx2, int idx3, int anim1Idx, int anim2Idx, int anim3Idx);
	void BOB_OFFSET(int idx, int offset);
	void SPACTION1(byte *spriteData, const Common::String &animString, int a3, int a4, int speed);
	void PARADISE();
};

} // End of namespace Hopkins

#endif /* HOPKINS_OBJECTS_H */
