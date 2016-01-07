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

#ifndef MADS_PLAYER_H
#define MADS_PLAYER_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/serializer.h"

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
	FACING_NONE = 5, FACING_DUMMY = 0
};

struct StopWalkerEntry {
	int _stack;
	int _trigger;

	StopWalkerEntry() : _stack(0), _trigger(0) {}
	StopWalkerEntry(int stack, int trigger) : _stack(stack), _trigger(trigger) {}

	void synchronize(Common::Serializer &s);
};

class StopWalkers : public Common::FixedStack<StopWalkerEntry, 12> {
public:
	StopWalkers() : Common::FixedStack<StopWalkerEntry, 12>() {}

	void synchronize(Common::Serializer &s);
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
	int _distAccum;
	int _pixelAccum;
	int _deltaDistance;
	int _totalDistance;

	void clearStopList();

	/**
	 * If the player is moving, handles figuring out the correct motion
	 */
	void move();

	/**
	 * Update the player's frame number
	 */
	void setFrame();

	/**
	 * Get the sprite slot index for the player
	 */
	int getSpriteSlot();

	/**
	 * Get the scale for the player at the given Y position
	 */
	int getScale(int yp);

	/**
	 * Sets the frame rate for the current sprite set
	 */
	void setBaseFrameRate();

	/**
	* Starts a player moving to a given destination
	*/
	void startMovement();

	void changeFacing();

	void activateTrigger();
public:
	MADSAction *_action;

	Facing _facing;
	Facing _turnToFacing;
	Facing _prepareWalkFacing;
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
	bool _beenVisible;
	bool _walkAnywhere;
	int _frameNumber;
	bool _loadsFirst;
	bool _loadedFirst;
	Common::Point _playerPos;
	Common::Point _targetPos;
	Common::Point _posChange;
	Common::Point _posDiff;
	Common::Point _prepareWalkPos;
	bool _moving;
	int _walkOffScreen, _walkOffScreenSceneId;
	int _special;
	int _ticksAmount;
	uint32 _priorTimer;
	int _velocity;
	int _upcomingTrigger;
	int _trigger;
	bool _scalingVelocity;
	bool _forceRefresh;
	bool _forcePrefix;
	bool _needToWalk;
	bool _readyToWalk;
	bool _commandsAllowed;
	bool _enableAtTarget;
	int _centerOfGravity;
	int _currentDepth;
	int _currentScale;
	Common::String _spritesPrefix;

	int _walkTrigger;
	TriggerMode _walkTriggerDest;
	ActionDetails _walkTriggerAction;
	StopWalkers _stopWalkers;
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

	/**
	 * Handler method for when the player is not moving
	 */
	void idle();

	/**
	 * Starts the player walking towards a given point and direction facing
	 * @param pos		Destination location
	 * @param facing	Direction to face once the destination is reached
	 */
	void startWalking(const Common::Point &pt, Facing facing);

	/**
	 * Used by the game scripst to make the player walk to a given destination.
	 * The difference from startWalking is that this contains several extra
	 * layers of checking that startWalking bypasses.
	 */
	void walk(const Common::Point &pos, Facing facing);

	/**
	 * If a new walk sequence is pending, and has been okayed by the preparser,
	 * start the actual walking
	 */
	void newWalk();

	void nextFrame();

	/**
	 * Add a walker to the current queue
	 */
	void addWalker(int walker, int trigger);

	/**
	* Delete any sprites used by the player
	*/
	void releasePlayerSprites();

	/**
	 * Serialize the data of the player
	 */
	void synchronize(Common::Serializer &s);

	static void preloadSequences(const Common::String &prefix, int level) {
		// No implementation in ScummVM
	}

	void removePlayerSprites();

	void firstWalk(Common::Point fromPos, Facing fromFacing, Common::Point destPos, Facing destFacing, bool enableFl);

	void setWalkTrigger(int val);

	void resetFacing(Facing facing);

};

} // End of namespace MADS

#endif /* MADS_PLAYER_H */
