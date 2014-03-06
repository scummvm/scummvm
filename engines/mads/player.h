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

class Player {
private:
	MADSEngine *_vm;
	MADSAction *_action;

	void reset();

	void resetActionList();

	void move();

	void postUpdate();
public:
	int _direction;
	int _newDirection;
	int _destFacing;
	bool _spritesLoaded;
	int _spritesStart;
	int _numSprites;
	bool _stepEnabled;
	bool _spritesChanged;
	bool _visible;
	bool _visible3;
	Common::Point _playerPos;
	Common::Point _destPos;
	bool _moving;
	int _v844C0, _v844BE;
	int _next;
	int _routeCount;
	int _special;
	int _ticksAmount;
	uint32 _priorTimer;
	int _unk3;
	bool _forceRefresh;
	Common::String _spritesPrefix;
public:
	Player(MADSEngine *vm);

	void loadSprites(const Common::String &prefix);

	void turnToDestFacing();

	void moveComplete();

	void setupFrame();

	void updateFrame();

	void update();

	void idle();

	void setDest(const Common::Point &pt, int facing);

	void nextFrame();
};

} // End of namespace MADS

#endif /* MADS_PLAYER_H */
