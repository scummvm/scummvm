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

#ifndef GNAP_SCENE49_H
#define GNAP_SCENE49_H

#include "gnap/debugger.h"
#include "gnap/scenes/scenecore.h"

namespace Gnap {

class GnapEngine;

struct Scene49Obstacle {
	int _currSequenceId;
	int _closerSequenceId;
	int _passedSequenceId;
	int _splashSequenceId;
	int _collisionSequenceId;
	int _prevId;
	int _currId;
	int _laneNum;
};

struct ObstacleDef {
	int _sequenceId;
	int _ticks;
};

class Scene49: public Scene {
public:
	Scene49(GnapEngine *vm);
	~Scene49() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb() {};

private:
	int _scoreBarPos;
	int _scoreLevel;
	bool _scoreBarFlash;
	int _obstacleIndex;
	Scene49Obstacle _obstacles[5];
	int _truckSequenceId;
	int _truckId;
	int _truckLaneNum;

	void checkObstacles();
	void updateObstacle(int id);
	void increaseScore(int amount);
	void decreaseScore(int amount);
	void refreshScoreBar();
	void clearObstacle(int index);
};

} // End of namespace Gnap
#endif // GNAP_SCENE49_H
