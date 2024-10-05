/* ScummVM - Graphic Adventure Engine
*
* ScummVM is the legal property of its developers, whose names
* are too numerous to list here. Please refer to the COPYRIGHT
* file distributed with this source distribution.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#ifndef DARKSEED_PLAYER_H
#define DARKSEED_PLAYER_H

#include "common/rect.h"
#include "darkseed/nsp.h"

namespace Darkseed {

class Player {
private:
	Nsp _cPlayerSprites;
	Nsp _gPlayerSprites;

public:
	Nsp _animations;
	int _frameIdx = 0;
	int _direction = 0;
	Common::Point _position;
	Common::Point _positionLong; // the original sometimes seems to use a long (4 byte) version of the location
	Common::Point _walkTarget;
	Common::Point _finalTarget;

	int16 _playerSpriteWalkIndex_maybe = 0;
	int16 _playerWalkFrameDeltaOffset = 0;
	int16 _playerNewFacingDirection_maybe = 0;
	uint16 _playerWalkFrameIdx = 0;
	bool _actionToPerform = false; // player is pathfinding to some destination?
	bool _playerIsChangingDirection = false; // AKA _Rotating
	bool _isAutoWalkingToBed = false;
	bool _heroMoving = false; // maybe set to true while player is walking around the room.
	bool _herowaiting = false;
	int _walkPathIndex = -1;
	uint16 _numConnectorsInWalkPath = 0;
	Common::Array<Common::Point> _connectorList;
	int16 _sequenceRotation = -1;
	bool _walkToSequence = false;
	Common::Point _walkToSequencePoint;
	bool _flipSprite = false;

public:
	Player();
	bool loadAnimations(const Common::Path &filename);
	const Sprite &getSprite(int frameNo);
	void updateSprite();
	void draw();

	bool isAtPosition(int x, int y) const;
	bool isAtWalkTarget() const;
	void calculateWalkTarget();

	void changeDirection(int16 oldDir, int16 newDir);
	void playerFaceWalkTarget();
	int getWidth();
	int getHeight();
	void updatePlayerPositionAfterRoomChange();
	void setplayertowardsbedroom();
	void walkToNextConnector();

private:
	void createConnectorPathToDest();
	Common::Point getClosestUnusedConnector(int16 x, int16 y, bool mustHaveCleanLine = false);
	void reverseConnectorList();
	void OptimisePath();
};

} // namespace Darkseed

#endif // DARKSEED_PLAYER_H
