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

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#include "crab/crab.h"
#include "crab/ScreenSettings.h"
#include "crab/animation/sprite.h"

namespace Crab {

using namespace pyrodactyl::anim;
using namespace pyrodactyl::ai;

//------------------------------------------------------------------------
// Purpose: Calculate distance between two sprites
//------------------------------------------------------------------------
double Sprite::distSq(const Sprite &s) {
	double result = (_pos.x - s._pos.x) * (_pos.x - s._pos.x) + (_pos.y - s._pos.y) * (_pos.y - s._pos.y);
	return result;
}

//------------------------------------------------------------------------
// Purpose: Used for player movement
//------------------------------------------------------------------------
void Sprite::moveToDest(pyrodactyl::event::Info &info, const SpriteConstant &sc) {
	if (_aiData._dest._active) {
		int num = 0;
		info.statGet(_id, pyrodactyl::stat::STAT_SPEED, num);
		++num;
		float playerSpeed = static_cast<float>(num);

		if (moveToLoc(_aiData._dest, playerSpeed, sc)) {
			_aiData._dest._active = false;
			xVel(0.0f);
			yVel(0.0f);
		}
	}
}

void Sprite::moveToDestPathfinding(pyrodactyl::event::Info &info, const SpriteConstant &sc) {
	if (_aiData._dest._active) {
		int num = 0;
		info.statGet(_id, pyrodactyl::stat::STAT_SPEED, num);
		++num;
		float playerSpeed = static_cast<float>(num);

		Rect b = boundRect();

		// Use to provide a bit of leeway with reaching the goal.
		_pathing.setNodeBufferDistance((b.w * b.w) / 2.0f);

		// IF we either have a solution, have reached our destination, and it was our final destination OR
		// IF there is no solution OR
		// IF we haven't yet found a solution
		// THEN stop.
		if ((moveToLocPathfinding(_aiData._dest, playerSpeed, sc) && _pathing._solutionFound &&
			 _pathing.getImmediateDest() == Vector2i(_pathing._destination.x, _pathing._destination.y)) ||
			_pathing._noSolution || !_pathing._solutionFound) {
			_aiData._dest._active = false;
			xVel(0.0f);
			yVel(0.0f);
		}
	}
}

//------------------------------------------------------------------------
// Purpose: Move towards a location without any path finding
//------------------------------------------------------------------------
bool Sprite::moveToLoc(Vector2i &dest, const float &velocity, const SpriteConstant &sc) {
	// Use the bound rectangle dimensions
	Rect b = boundRect();

	// X axis
	if (b.x + b.w < dest.x)
		xVel(velocity * sc._walkVelMod.x);
	else if (b.x > dest.x)
		xVel(-velocity * sc._walkVelMod.x);
	else
		xVel(0.0f);

	// Y axis
	if (b.y + b.h < dest.y)
		yVel(velocity * sc._walkVelMod.y);
	else if (b.y > dest.y)
		yVel(-velocity * sc._walkVelMod.y);
	else
		yVel(0.0f);

	return b.contains(dest);
}

// Move toward the location using pathfinding.
bool Sprite::moveToLocPathfinding(Vector2i &dest, const float &velocity, const SpriteConstant &sc) {
	// Rect b = BoundRect();

	Vector2i immediateDest = _pathing.getImmediateDest();

	Vector2f vecTo = Vector2f((float)immediateDest.x, (float)immediateDest.y) - _pathing.getPosition();

	// If the destination is not the pathing goal, we must reach it exactly before moving on.
	if (immediateDest != Vector2i(_pathing._destination.x, _pathing._destination.y)) {
		Timer fps;
		float deltaTime = 1.0f / (float)g_engine->_screenSettings->_fps;

		// Project how far we will travel next frame.
		Vector2f velVec = Vector2f(sc._walkVelMod.x * velocity * deltaTime, sc._walkVelMod.y * velocity * deltaTime);

		if (vecTo.magnitude() > velVec.magnitude()) {
			vecTo.normalize();

			xVel(vecTo.x * sc._walkVelMod.x * velocity);
			yVel(vecTo.y * sc._walkVelMod.y * velocity);
		} else {
			xVel(0.0f);
			yVel(0.0f);
		}
	} else {
		Vector2i loc = _pathing.getImmediateDest();
		moveToLoc(loc, velocity, sc);
	}

	// return(MoveToLoc(pathing.GetImmediateDest(), vel, sc) || (xVel() == 0.0f && yVel() == 0.0f));

	return (xVel() == 0.0f && yVel() == 0.0f);
}

//------------------------------------------------------------------------
// Purpose: AI routine for running to the nearest exit, then disappearing
//------------------------------------------------------------------------
void Sprite::flee(pyrodactyl::event::Info &info, Common::Array<pyrodactyl::level::Exit> &areaExit, const SpriteConstant &sc) {
	switch (_aiData._flee._state) {
	case FLEESTATE_GETNEARESTEXIT: {
		if (areaExit.empty()) {
			// No valid exits in the level
			_aiData._flee._state = FLEESTATE_CANTFLEE;
			break;
		} else {
			_aiData._flee._state = FLEESTATE_GETNEARESTEXIT;

			// Standard way to find nearest exit
			int min_dist = INT_MAX;

			// Find the nearest exit
			for (auto &i : areaExit) {
				// Compare distance to the rough center of each exit
				int dist = distance2D(_pos.x, _pos.y, i._dim._rect.x + i._dim._rect.w / 2, i._dim._rect.y + i._dim._rect.h / 2);
				if (dist < min_dist) {
					min_dist = dist;

					// Set the destination of sprite to this exit
					_aiData.dest(i._dim._rect.x + i._dim._rect.w / 2, i._dim._rect.y + i._dim._rect.h / 2);

					_pathing.setDestination(Vector2f((float)_aiData._dest.x, (float)_aiData._dest.y));
				}
			}
		}
	} break;
	case FLEESTATE_RUNTOEXIT: {
		Rect b = boundRect();
		if (b.contains(_aiData._dest)) {
			// We have reached the exit, time to make the sprite disappear
			_aiData._flee._state = FLEESTATE_DISAPPEAR;
			break;
		} else {
			int num = 0;
			info.statGet(_id, pyrodactyl::stat::STAT_SPEED, num);
			++num;
			float velocity = static_cast<float>(num);

			// MoveToLoc(_aiData.dest, vel, sc);
			moveToLocPathfinding(_aiData._dest, velocity, sc);
		}
	} break;
	case FLEESTATE_DISAPPEAR:
		_visible.result(false);
		break;
	default:
		break;
	}
}

//------------------------------------------------------------------------
// Purpose: AI routine for fighting the player
//------------------------------------------------------------------------
void Sprite::attack(pyrodactyl::event::Info &info, Sprite &targetSp, const SpriteConstant &sc) {
	switch (_aiData._fight._state) {
	case FIGHTSTATE_GETNEXTMOVE: {
		_aiData._fight._state = FIGHTSTATE_GETINRANGE;
		_aiData._fight._delay.start();

		uint size = _aiData._fight._attack.size();
		if (size > 1)
			_animSet._fight.next(_aiData._fight._attack[g_engine->getRandomNumber(_aiData._fight._attack.size())]);
		else if (size <= 0)
			_aiData._fight._state = FIGHTSTATE_CANTFIGHT;
		else
			_animSet._fight.next(_aiData._fight._attack[0]);
	}
		break;
	case FIGHTSTATE_GETINRANGE: {
		// Set destination path to the player location
		Rect b = targetSp.boundRect();
		Vector2i dest(b.x + b.w / 2, b.y + b.h / 2);
		setDestPathfinding(dest);

		Rect p = boundRect();
		_pathing.setPosition(Vector2f((float)(p.x + p.w / 2), (float)p.y + p.h / 2));
		_pathing.update(0);

		FightMove f;
		if (_animSet._fight.nextMove(f) && fightCollide(targetSp.boxV(), targetSp.boundRect(), f._ai._range, sc)) {
			if (_aiData._fight._delay.ticks() > f._ai._delay)
				_aiData._fight._state = FIGHTSTATE_EXECUTEMOVE;
		} else if (_input.idle())
			moveToDestPathfinding(info, sc);
	}
		break;
	case FIGHTSTATE_EXECUTEMOVE:
		updateMove(_animSet._fight.next());
		_aiData._fight._state = FIGHTSTATE_GETNEXTMOVE;
		_aiData._fight._delay.stop();
		break;
	default:
		break;
	}
}

void Sprite::flyAround(const Rect &camera, const SpriteConstant &sc) {
	// Is this sprite flying right now?
	if (_aiData._walk._enabled) {
		// We're flying towards the left edge
		if (xVel() < 0) {
			// Are we completely out of the left edge of the camera?
			if (x() < camera.x - w()) {
				_aiData._walk._enabled = false;

				// Start the timer, set a semi-random time
				_aiData._walk._timer.target(sc._fly._delayMin + (g_engine->getRandomNumber(sc._fly._delayMax)));
				_aiData._walk._timer.start();
			}
		} else if (xVel() > 0) { // Flying towards the right edge
			// Are we completely out of the left edge of the camera?
			if (x() > camera.x + camera.w + w()) {
				_aiData._walk._enabled = false;

				// Start the timer, set a semi-random time
				_aiData._walk._timer.target(sc._fly._delayMin + (g_engine->getRandomNumber(sc._fly._delayMax)));
				_aiData._walk._timer.start();
			}
		}

		move(sc);
	} else {
		// Safety condition in case timer isn't running
		if (!_aiData._walk._timer.started())
			_aiData._walk._timer.start();

		// Is it time to start flying?
		if (_aiData._walk._timer.targetReached()) {
			// Stop the timer
			_aiData._walk._timer.stop();

			// Decide if the sprite flies from the left or right of the camera
			if (g_engine->getRandomNumber(1)) {
				// Fly in from the right
				x(camera.x + camera.w + sc._fly._start.x);
				xVel(-1.0f * sc._fly._vel.x);

				// Sprite needs to face left
				_dir = DIRECTION_LEFT;
			} else {
				// Fly in from the left
				x(camera.x - w() - sc._fly._start.x);
				xVel(sc._fly._vel.x);

				// Sprite needs to face right
				_dir = DIRECTION_RIGHT;
			}

			y(camera.y + sc._fly._start.y + (g_engine->getRandomNumber(camera.h - (2 * sc._fly._start.y))));
			yVel(sc._fly._vel.y);

			// Set state to flying
			_aiData._walk._enabled = true;
		}
	}
}

} // End of namespace Crab
