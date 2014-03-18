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

#ifndef MADS_PLAYER_H
#define MADS_PLAYER_H

#include "common/scummsys.h"
#include "common/str.h"

namespace MADS {

class MADSEngine;
class Action;

#define PLAYER_SPRITES_FILE_COUNT 8

class Player {
private:
	static const int _directionListIndexes[32];
private:
	MADSEngine *_vm;
	MADSAction *_action;
	bool _highSprites;
	bool _spriteSetsPresent[PLAYER_SPRITES_FILE_COUNT];
	int _currentDepth;
	int _currentScale;
	int _frameOffset;
	int _frameNum;
	int _yScale;
	int _frameCount;
	int _frameListIndex;
	int _actionIndex;
	bool _v844BC;
	int _v8452E;
	int _v8452C;
	int _v84530;
	int _routeLength;
	int _actionList[12];
	int _actionList2[12];
	int _hypotenuse;

	void reset();

	void resetActionList();

	void move();

	void postUpdate();

	/**
	 * Get the sprite slot index for the player
	 */
	int getSpriteSlot();

	/**
	 * Get the scale for the player at the given Y position
	 */
	int getScale(int yp);

	void setTicksAmount();

	void setupRoute();

	void setupRoute(bool bitFlag);

	void setupRouteNode(int *routeIndexP, int nodeIndex, int flags, int routeLength);

	/**
	* Scans along an edge connecting two points within the depths/walk surface, and returns the information of the first
	* pixel high nibble encountered with a non-zero value
	*/
	int scanPath(MSurface &depthSurface, const Common::Point &srcPos, const Common::Point &destPos);

	/**
	* Starts a player moving to a given destination
	*/
	void startMovement();

	void dirChanged();
public:
	int _direction;
	int _newDirection;
	int _xDirection, _yDirection;
	int _destFacing;
	bool _spritesLoaded;
	int _spritesStart;
	int _spritesIdx;
	int _numSprites;
	bool _stepEnabled;
	bool _spritesChanged;
	bool _visible;
	bool _priorVisible;
	bool _visible3;
	Common::Point _playerPos;
	Common::Point _destPos;
	Common::Point _posChange;
	Common::Point _posDiff;
	bool _moving;
	int _newSceneId, _v844BE;
	int _next;
	int _special;
	int _ticksAmount;
	uint32 _priorTimer;
	int _unk1;
	int _unk2;
	int _unk3;
	bool _unk4;
	bool _forceRefresh;
	Common::String _spritesPrefix;
	int _routeCount;
	int _routeOffset;
	int _tempRoute[MAX_ROUTE_NODES];
	int _routeIndexes[MAX_ROUTE_NODES];
public:
	Player(MADSEngine *vm);

	/**
	 * Load sprites for the player
	 */
	bool loadSprites(const Common::String &prefix);

	void turnToDestFacing();

	void moveComplete();

	void setupFrame();

	void updateFrame();

	void update();

	void idle();

	void setDest(const Common::Point &pt, int facing);

	void startWalking(const Common::Point &pos, int direction);

	void nextFrame();
};

} // End of namespace MADS

#endif /* MADS_PLAYER_H */
