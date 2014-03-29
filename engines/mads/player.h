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
class MADSAction;

#define PLAYER_SPRITES_FILE_COUNT 8
#define MAX_ROUTE_NODES 22

/**
 * Player facings
 */
enum Facing {
	FACING_NORTH = 8, FACING_SOUTH = 2, FACING_EAST = 6, FACING_WEST = 4,
	FACING_NORTHEAST = 9, FACING_SOUTHEAST = 3,
	FACING_SOUTHWEST = 1, FACING_NORTHWEST = 7,
	FACING_NONE = 5
};

class Player {
private:
	static const int _directionListIndexes[32];
private:
	MADSEngine *_vm;
	bool _highSprites;
	bool _spriteSetsPresent[PLAYER_SPRITES_FILE_COUNT];
	bool _mirror;
	int _frameCount;
	int _frameListIndex;
	bool _v844BC;
	int _v8452E;
	int _v8452C;
	int _v84530;
	int _routeLength;
	int _stopWalkerList[12];
	int _stopWalkerTrigger[12];
	int _stopWalkerIndex;
	int _hypotenuse;

	void clearStopList();

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

	void setBaseFrameRate();

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
	MADSAction *_action;

	Facing _facing;
	Facing _turnToFacing;
	int _xDirection, _yDirection;
	Facing _targetFacing;
	bool _spritesLoaded;
	int _spritesStart;
	int _spritesIdx;
	int _numSprites;
	bool _stepEnabled;
	bool _spritesChanged;
	bool _visible;
	bool _priorVisible;
	bool _visible3;
	bool _walkAnywhere;
	int _frameNumber;
	bool _loadsFirst;
	bool _loadedFirst;
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
	int _velocity;
	int _upcomingTrigger;
	int _trigger;
	bool _unk4;
	bool _forceRefresh;
	int _centerOfGravity;
	int _currentDepth;
	int _currentScale;
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

	/**
	 * Called when the player has reached the given destination, start him
	 * turning to the specified facing
	 */
	void setFinalFacing();

	/**
	 * Stops the player walking
	 */
	void cancelWalk();

	/**
	 * Cancels any oustanding player action
	 */
	void cancelCommand();

	/**
	 * Set up control parameters for the current active series (the
	 * direction which the player is facing in) */
	void selectSeries();

	/*
	 * Moves to the next frame for the currently active player sprite set
	 */
	void updateFrame();

	void update();

	void idle();

	void setDest(const Common::Point &pt, Facing facing);

	void startWalking(const Common::Point &pos, Facing direction);

	void nextFrame();

	void step();
};

} // End of namespace MADS

#endif /* MADS_PLAYER_H */
