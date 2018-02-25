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

#ifndef BLADERUNNER_SCENE_H
#define BLADERUNNER_SCENE_H

#include "bladerunner/vector.h"

namespace BladeRunner {

class BladeRunnerEngine;
class BoundingBox;
class Regions;
class Set;
class VQAPlayer;

class Scene {
	BladeRunnerEngine *_vm;

	int         _setId;
	int         _sceneId;
	VQAPlayer  *_vqaPlayer;

	int         _defaultLoop;
	bool        _defaultLoopSet;
	bool        _defaultLoopPreloadedSet;
	int         _specialLoopMode;
	int         _specialLoop;
	// int         _introFinished;
	int         _nextSetId;
	int         _nextSceneId;
	int         _frame;

	Vector3     _actorStartPosition;
	int         _actorStartFacing;
	bool        _playerWalkedIn;

public:
	Set        *_set;
	Regions    *_regions;
	Regions    *_exits;

public:
	Scene(BladeRunnerEngine *vm);
	~Scene();

	bool open(int setId, int sceneId, bool isLoadingGame);
	bool close(bool isLoadingGame);
	int  advanceFrame();
	void setActorStart(Vector3 position, int facing);

	void loopSetDefault(int loopId);
	void loopStartSpecial(int specialLoopMode, int loopId, bool immediately);

	int getSetId() const { return _setId; }
	int getSceneId() const { return _sceneId; }

	bool didPlayerWalkIn() { bool r = _playerWalkedIn; _playerWalkedIn = false; return r; }

	int findObject(const char *objectName);
	bool objectSetHotMouse(int objectId);
	bool objectGetBoundingBox(int objectId, BoundingBox *boundingBox);
	void objectSetIsClickable(int objectId, bool isClickable, bool sceneLoaded);
	void objectSetIsObstacle(int objectId, bool isObstacle, bool sceneLoaded, bool updateWalkpath);
	void objectSetIsObstacleAll(bool isObstacle, bool sceneLoaded);
	void objectSetIsTarget(int objectId, bool isTarget, bool sceneLoaded);
	const char *objectGetName(int objectId);

private:
	void loopEnded(int frame, int loopId);
	static void loopEndedStatic(void *data, int frame, int loopId);
};

} // End of namespace BladeRunner

#endif
