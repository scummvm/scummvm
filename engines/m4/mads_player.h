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

#ifndef M4_MADS_PLAYER_H
#define M4_MADS_PLAYER_H

#include "common/scummsys.h"
#include "m4/mads_scene.h"

namespace M4 {

#define PLAYER_SEQ_INDEX -2

class MadsPlayer {
private:
	int getScale(int yp);
	int getSpriteSlot();
	void setTicksAmount();
	void resetActionList();
	int queueAction(int v0, int v1);
	void idle();
	void move();
	void dirChanged();
	void reset();
	int scanPath(M4Surface *depthSurface, const Common::Point &srcPos, const Common::Point &destPos);
	void startMovement();
	void setupRouteNode(int *routeIndexP, int nodeIndex, int flags, int routeLength);
public:
	char _spritesPrefix[16];
	int _spriteSetCount;
	bool _spriteSetsPresent[8];
	Common::Point _playerPos;
	Common::Point _destPos;
	Common::Point _posChange;
	Common::Point _posDiff;
	int _hypotenuse;
	uint32 _priorTimer;
	uint _ticksAmount;
	int16 _direction, _newDirection;
	bool _stepEnabled;
	bool _visible, _priorVisible;
	bool _visible3;
	bool _forceRefresh;
	int16 _currentScale;
	int16 _yScale;
	int16 _currentDepth;
	int16 _spriteListStart, _spriteListIdx;
	bool _spritesChanged;
	uint16 _frameOffset, _frameNum;
	bool _moving;
	int _unk1;
	int _frameCount;
	int _frameListIndex;
	int _actionIndex;
	int _actionList[12];
	int _actionList2[12];
	int _unk2;
	int _unk3;
	int _xDirection, _yDirection;
	int _destFacing;
	int _special;
	int _next;
	int _routeCount;
	int _routeOffset;
	int _tempRoute[MAX_ROUTE_NODES];
	int _routeIndexes[MAX_ROUTE_NODES];
	bool _unk4;
	bool _v844BC;
	int _v844BE;
	bool _v844C0;
	int _v8452E;
	int _v8452C;
	int _v84530;
	int _routeLength;

	static const int _directionListIndexes[32];
public:
	MadsPlayer();

	bool loadSprites(const char *prefix);
	void update();
	void updateFrame();
	void setupFrame();
	void step();
	void nextFrame();
	void setDest(int destX, int destY, int facing);
	void turnToDestFacing();
	void setupRoute(bool bitFlag);
	void moveComplete();
};

} // End of namespace M4

#endif
