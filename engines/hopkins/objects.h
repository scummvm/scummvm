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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef HOPKINS_OBJECTS_H
#define HOPKINS_OBJECTS_H

#include "hopkins/globals.h"

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/str.h"

#define MAX_SPRITE 5
namespace Hopkins {

struct ObjectAuthIcon {
	byte _objectFileNum;
	byte _idx;
	byte _flag1;
	byte _flag2;
	byte _flag3;
	byte _flag4;
	byte _flag5;
	byte _flag6;
};

struct SpriteItem {
	int _animationType;
	const byte *_spriteData;
	Common::Point _spritePos;
	int _zoomFactor;
	bool _flipFl;
	int _spriteIndex;
	int _deltaX;
	int _deltaY;
	bool _rleFl;
	bool _activeFl;
	int _destX;
	int _destY;
	int _width;
	int _height;
	int _zoomPct;
	int _reducePct;
};

struct BobItem {
	int _bobMode;
	byte *_spriteData;
	int _xp;
	int _yp;
	int _frameIndex;
	int _animDataIdx;
	int _moveChange1;
	int _moveChange2;
	bool _disabledAnimationFl;
	byte *_animData;
	bool _bobMode10;
	int _bobModeChange;
	int _modeChangeCtr;
	int _modeChangeUnused;
	bool _disableFl; // Set to true in B_CACHE_OFF()
	int _zoomFactor;
	bool _flipFl;
	bool _isSpriteFl;
	bool _activeFl;
	int _oldX;
	int _oldY;
	int _oldWidth;
	int _oldHeight;
	int _oldX2;
	int _zooInmFactor;
	int _zoomOutFactor;
};

struct HidingItem {
	int _x;
	int _y;
	int _spriteIndex;
	int _width;
	int _height;
	int _useCount;
	byte *_spriteData;
	bool _resetUseCount;
	int _yOffset;
};

struct LockAnimItem {
	bool _enableFl;
	int _posX;
};

struct VBobItem {
	const byte *_spriteData;
	int _displayMode;
	int _xp;
	int _yp;
	int _frameIndex;
	byte *_surface;
	int _oldX;
	int _oldY;
	int _oldFrameIndex;
	const byte *_oldSpriteData;
};

struct ListeItem {
	bool _visibleFl;
	int _posX;
	int _posY;
	int _width;
	int _height;
};

/**
 * Mode for SortItem records
 */
enum SortMode { SORT_NONE = 0, SORT_BOB = 1, SORT_SPRITE = 2, SORT_HIDING = 3 };

/**
 * Structure to represent a pending display of either a Bob, Sprite, or Cache Item.
 */
struct SortItem {
	SortMode _sortMode;
	int _index;
	int _priority;
};

class HopkinsEngine;

class ObjectsManager {
private:
	HopkinsEngine *_vm;

	int _objectWidth, _objectHeight;
	int _oldBorderSpriteIndex;
	int _borderSpriteIndex;
	byte *_spritePtr;
	const byte *_oldSpriteData;
	int _verb;
	int _oldSpriteIndex;
	int _oldFrameIndex;
	int _oldDirectionSpriteIdx;
	Directions _oldDirection;
	Directions _lastDirection;
	bool _oldFlipFl;
	int _curGestureFile;
	byte *_gestureBuf;
	int _homeRateCounter;
	int _sortedDisplayCount;
	SortItem _sortedDisplay[51];
	byte *_hidingItemData[6];
	HidingItem _hidingItem[25];
	bool _hidingActiveFl;
	ObjectAuthIcon _objectAuthIcons[300];
	int _curObjectFileNum;
	byte *_objectDataBuf;

	VBobItem _vBob[30];
	ListeItem _liste[6];
	ListeItem _liste2[35];

	void initVBob();
	void clearVBob();

	void sprite_alone(const byte *objectData, byte *sprite, int objIndex);
	void removeObjectDataBuf();

	int getOffsetX(const byte *spriteData, int spriteIndex, bool isSize);
	int getOffsetY(const byte *spriteData, int spriteIndex, bool isSize);

	void capture_mem_sprite(const byte *objectData, byte *sprite, int objIndex);
	void setBobInfo(int idx);
	void computeHideCounter(int idx);
	void initBobVariables(int idx);

	void checkHidingItem();
	void displayHiding(int idx);
	void computeSprite(int idx);
	void beforeSort(SortMode sortMode, int index, int priority);
	void displayBobAnim();
	void displayVBob();
	void showSprite(int idx);
	void clearSprite();
	void setSpriteZoom(int idx, int zoomFactor);

	void loadZone(const Common::String &file);
	void changeCharacterHead(PlayerCharacter oldCharacter, PlayerCharacter newCharacter);
	void goHome2();

	void nextVerbIcon();
	void handleForest(int screenId, int minX, int maxX, int minY, int maxY, int idx);

	void sceneSpecialIni();
	void showActionAnimation(const byte *spriteData, const Common::String &actionStr, int speed, bool flipFl);
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
	int _saveLoadX, _saveLoadY;
	int _mapCarPosX, _mapCarPosY;
	int _oldCharacterPosX, _oldCharacterPosY;
	Common::Point _borderPos;
	Common::Point _oldBorderPos;
	Common::Point _characterPos;
	byte *_forestSprite;
	byte *_saveLoadSprite;
	byte *_saveLoadSprite2;
	byte *_headSprites;
	SpriteItem _sprite[6];
	BobItem _bob[36];
	LockAnimItem _lockedAnims[36];
	bool _charactersEnabledFl;
	bool _refreshBobMode10Fl;

	ObjectsManager(HopkinsEngine *vm);
	~ObjectsManager();

	void clearAll();

	int getWidth(const byte *objectData, int idx);
	int getHeight(const byte *objectData, int idx);
	byte *loadSprite(const Common::String &file);
	void loadLinkFile(const Common::String &file, bool OBSSEUL = false);
	void addStaticSprite(const byte *spriteData, Common::Point pos, int idx, int spriteIndex, int zoomFactor, bool flipFl, int deltaX, int deltaY);
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

	void resetBob(int idx);
	void hideBob(int idx);
	void displayBob(int idx);
	void setBobOffset(int idx, int offset);
	void setBobAnimDataIdx(int idx, int animIdx);
	void setBobAnimation(int idx);
	void stopBobAnimation(int idx);
	int getBobPosX(int idx);

	void handleCityMap();
	void clearScreen();
	void disableVerb(int idx, int a2);
	void enableVerb(int idx, int a2);
	void lockAnimX(int idx, int x);
	void handleLeftButton();
	void handleRightButton();
	void setOffsetXY(byte *data, int idx, int xp, int yp, bool isSize);
	void setVerb(int id);

	void doActionBack(int idx);
	void doActionRight(int idx);
	void doActionFront(int idx);
	void doActionLeft(int idx);
	void doActionDiagRight(int idx);
	void doActionDiagLeft(int idx);
	void loadObjects();
	byte *loadObjectFromFile(int objIndex, bool mode);
	void resetHidingItems();
	void resetHidingUseCount(int idx);
	void setHidingUseCount(int idx);
	void loadHidingItems(const Common::String &file);
	void enableHidingBehavior();
	void disableHidingBehavior();
	void disableHidingItem(int idx);

	void resetHomeRateCounter() { _homeRateCounter = 0; }
	void resetOldFrameIndex() { _oldFrameIndex = -1; }
	void resetOldDirection()  { _oldDirection = DIR_NONE; }
	int getObjectWidth()  { return _objectWidth; }
	int getObjectHeight() { return _objectHeight; }

	void showSpecialActionAnimationWithFlip(const byte *spriteData, const Common::String &animationSeq, int speed, bool flipFl);
	void showSpecialActionAnimation(const byte *spriteData, const Common::String &animString, int speed);
	void checkEventBobAnim(int idx, int animIdx, int animDataIdx, int a4);
	void setMultiBobAnim(int idx1, int idx2, int anim1Idx, int anim2Idx);
	void loadObjectIniFile();
	void quickDisplayBobSprite(int idx);
	void initVbob(const byte *src, int idx, int xp, int yp, int frameIndex);
	void disableVbob(int idx);
	void setAndPlayAnim(int idx, int animIdx, int destPosi, bool animAction);

	void sceneControl(const Common::String &backgroundFile, const Common::String &linkFile,
		const Common::String &animFile, const Common::String &s4, int soundNum, bool initializeScreen);
	void sceneControl2(const Common::String &backgroundFile, const Common::String &linkFile,
		const Common::String &animFile, const Common::String &s4, int soundNum, bool initializeScreen);
	void goHome();
	void paradise();
};

} // End of namespace Hopkins

#endif /* HOPKINS_OBJECTS_H */
