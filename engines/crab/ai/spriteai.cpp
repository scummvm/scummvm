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

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "crab/crab.h"
#include "crab/animation/sprite.h"

namespace Crab {

using namespace pyrodactyl::anim;
using namespace pyrodactyl::ai;

//------------------------------------------------------------------------
// Purpose: Calculate distance between two sprites
//------------------------------------------------------------------------
double Sprite::DistSq(const Sprite &s) {
	double result = (pos.x - s.pos.x) * (pos.x - s.pos.x) + (pos.y - s.pos.y) * (pos.y - s.pos.y);
	return result;
}

//------------------------------------------------------------------------
// Purpose: Used for player movement
//------------------------------------------------------------------------
void Sprite::MoveToDest(pyrodactyl::event::Info &info, const SpriteConstant &sc) {
	if (ai_data.dest.active) {
		int num = 0;
		info.StatGet(id, pyrodactyl::stat::STAT_SPEED, num);
		++num;
		float player_speed = static_cast<float>(num);

		if (MoveToLoc(ai_data.dest, player_speed, sc)) {
			ai_data.dest.active = false;
			XVel(0.0f);
			YVel(0.0f);
		}
	}
}

void Sprite::MoveToDestPathfinding(pyrodactyl::event::Info &info, const SpriteConstant &sc) {
	if (ai_data.dest.active) {
		int num = 0;
		info.StatGet(id, pyrodactyl::stat::STAT_SPEED, num);
		++num;
		float player_speed = static_cast<float>(num);

		Rect b = BoundRect();

		// Use to provide a bit of leeway with reaching the goal.
		pathing.SetNodeBufferDistance((b.w * b.w) / 2.0f);

		// IF we either have a solution, have reached our destination, and it was our final destination OR
		// IF there is no solution OR
		// IF we haven't yet found a solution
		// THEN stop.
		if ((MoveToLocPathfinding(ai_data.dest, player_speed, sc) && pathing.solutionFound &&
			 pathing.GetImmediateDest() == Vector2i(pathing.destination.x, pathing.destination.y)) ||
			pathing.noSolution || !pathing.solutionFound) {
			ai_data.dest.active = false;
			XVel(0.0f);
			YVel(0.0f);
		}
	}
}

//------------------------------------------------------------------------
// Purpose: Move towards a location without any path finding
//------------------------------------------------------------------------
bool Sprite::MoveToLoc(Vector2i &dest, const float &velocity, const SpriteConstant &sc) {
	// Use the bound rectangle dimensions
	Rect b = BoundRect();

	// X axis
	if (b.x + b.w < dest.x)
		XVel(velocity * sc.walk_vel_mod.x);
	else if (b.x > dest.x)
		XVel(-velocity * sc.walk_vel_mod.x);
	else
		XVel(0.0f);

	// Y axis
	if (b.y + b.h < dest.y)
		YVel(velocity * sc.walk_vel_mod.y);
	else if (b.y > dest.y)
		YVel(-velocity * sc.walk_vel_mod.y);
	else
		YVel(0.0f);

	return b.Contains(dest);
}

// Move toward the location using pathfinding.
bool Sprite::MoveToLocPathfinding(Vector2i &dest, const float &velocity, const SpriteConstant &sc) {
	// Rect b = BoundRect();

	Vector2i immediateDest = pathing.GetImmediateDest();

	Vector2f vecTo = Vector2f((float)immediateDest.x, (float)immediateDest.y) - pathing.GetPosition();

	// If the destination is not the pathing goal, we must reach it exactly before moving on.
	if (immediateDest != Vector2i(pathing.destination.x, pathing.destination.y)) {
		Timer fps;
		float deltaTime = 1.0f / (float)g_engine->_screenSettings->fps;

		// Project how far we will travel next frame.
		Vector2f velVec = Vector2f(sc.walk_vel_mod.x * velocity * deltaTime, sc.walk_vel_mod.y * velocity * deltaTime);

		if (vecTo.Magnitude() > velVec.Magnitude()) {
			vecTo.Normalize();

			XVel(vecTo.x * sc.walk_vel_mod.x * velocity);
			YVel(vecTo.y * sc.walk_vel_mod.y * velocity);
		} else {
			XVel(0.0f);
			YVel(0.0f);
		}
	} else {
		Vector2i loc = pathing.GetImmediateDest();
		MoveToLoc(loc, velocity, sc);
	}

	// return(MoveToLoc(pathing.GetImmediateDest(), vel, sc) || (XVel() == 0.0f && YVel() == 0.0f));

	return (XVel() == 0.0f && YVel() == 0.0f);
}

//------------------------------------------------------------------------
// Purpose: AI routine for running to the nearest exit, then disappearing
//------------------------------------------------------------------------
void Sprite::Flee(pyrodactyl::event::Info &info, Common::Array<pyrodactyl::level::Exit> &area_exit, const SpriteConstant &sc) {
	switch (ai_data.flee.state) {
	case FLEESTATE_GETNEARESTEXIT: {
		if (area_exit.empty()) {
			// No valid exits in the level
			ai_data.flee.state = FLEESTATE_CANTFLEE;
			break;
		} else {
			ai_data.flee.state = FLEESTATE_GETNEARESTEXIT;

			// Standard way to find nearest exit
			int min_dist = INT_MAX;

			// Find the nearest exit
			for (auto &i : area_exit) {
				// Compare distance to the rough center of each exit
				int dist = Distance2D(pos.x, pos.y, i.dim.rect.x + i.dim.rect.w / 2, i.dim.rect.y + i.dim.rect.h / 2);
				if (dist < min_dist) {
					min_dist = dist;

					// Set the destination of sprite to this exit
					ai_data.Dest(i.dim.rect.x + i.dim.rect.w / 2, i.dim.rect.y + i.dim.rect.h / 2);

					pathing.SetDestination(Vector2f((float)ai_data.dest.x, (float)ai_data.dest.y));
				}
			}
		}
	} break;
	case FLEESTATE_RUNTOEXIT: {
		Rect b = BoundRect();
		if (b.Contains(ai_data.dest)) {
			// We have reached the exit, time to make the sprite disappear
			ai_data.flee.state = FLEESTATE_DISAPPEAR;
			break;
		} else {
			int num = 0;
			info.StatGet(id, pyrodactyl::stat::STAT_SPEED, num);
			++num;
			float velocity = static_cast<float>(num);

			// MoveToLoc(ai_data.dest, vel, sc);
			MoveToLocPathfinding(ai_data.dest, velocity, sc);
		}
	} break;
	case FLEESTATE_DISAPPEAR:
		visible.Result(false);
		break;
	default:
		break;
	}
}

//------------------------------------------------------------------------
// Purpose: AI routine for fighting the player
//------------------------------------------------------------------------
void Sprite::Attack(pyrodactyl::event::Info &info, Sprite &target_sp, const SpriteConstant &sc) {
	warning("STUB: Sprite::Attack()");

#if 0
	switch (ai_data.fight.state) {
	case FIGHTSTATE_GETNEXTMOVE: {
		ai_data.fight.state = FIGHTSTATE_GETINRANGE;
		ai_data.fight.delay.Start();

		unsigned int size = ai_data.fight.attack.size();
		if (size > 1)
			anim_set.fight.Next(ai_data.fight.attack[gRandom.Num() % ai_data.fight.attack.size()]);
		else if (size <= 0)
			ai_data.fight.state = FIGHTSTATE_CANTFIGHT;
		else
			anim_set.fight.Next(ai_data.fight.attack[0]);
	} break;
	case FIGHTSTATE_GETINRANGE: {
		// Set destination path to the player location
		Rect b = target_sp.BoundRect();
		Vector2i dest(b.x + b.w / 2, b.y + b.h / 2);
		SetDestPathfinding(dest);

		Rect p = BoundRect();
		pathing.SetPosition(Vector2f((float)(p.x + p.w / 2), (float)p.y + p.h / 2));
		pathing.Update(0);

		FightMove f;
		if (anim_set.fight.NextMove(f) && FightCollide(target_sp.BoxV(), target_sp.BoundRect(), f.ai.range, sc)) {
			if (ai_data.fight.delay.Ticks() > f.ai.delay)
				ai_data.fight.state = FIGHTSTATE_EXECUTEMOVE;
		} else if (input.Idle())
			MoveToDestPathfinding(info, sc);
	} break;
	case FIGHTSTATE_EXECUTEMOVE:
		UpdateMove(anim_set.fight.Next());
		ai_data.fight.state = FIGHTSTATE_GETNEXTMOVE;
		ai_data.fight.delay.Stop();
		break;
	default:
		break;
	}
#endif
}

void Sprite::FlyAround(const Rect &camera, const SpriteConstant &sc) {
	// Is this sprite flying right now?
	if (ai_data.walk.enabled) {
		// We're flying towards the left edge
		if (XVel() < 0) {
			// Are we completely out of the left edge of the camera?
			if (X() < camera.x - W()) {
				ai_data.walk.enabled = false;

				// Start the timer, set a semi-random time
				ai_data.walk.timer.Target(sc.fly.delay_min + (g_engine->getRandomNumber(sc.fly.delay_max)));
				ai_data.walk.timer.Start();
			}
		}
		// Flying towards the right edge
		else if (XVel() > 0) {
			// Are we completely out of the left edge of the camera?
			if (X() > camera.x + camera.w + W()) {
				ai_data.walk.enabled = false;

				// Start the timer, set a semi-random time
				ai_data.walk.timer.Target(sc.fly.delay_min + (g_engine->getRandomNumber(sc.fly.delay_max)));
				ai_data.walk.timer.Start();
			}
		}

		Move(sc);
	} else {
		// Safety condition in case timer isn't running
		if (!ai_data.walk.timer.Started())
			ai_data.walk.timer.Start();

		// Is it time to start flying?
		if (ai_data.walk.timer.TargetReached()) {
			// Stop the timer
			ai_data.walk.timer.Stop();

			// Decide if the sprite flies from the left or right of the camera
			if (g_engine->getRandomNumber(1)) {
				// Fly in from the right
				X(camera.x + camera.w + sc.fly.start.x);
				XVel(-1.0f * sc.fly.vel.x);

				// Sprite needs to face left
				dir = DIRECTION_LEFT;
			} else {
				// Fly in from the left
				X(camera.x - W() - sc.fly.start.x);
				XVel(sc.fly.vel.x);

				// Sprite needs to face right
				dir = DIRECTION_RIGHT;
			}

			Y(camera.y + sc.fly.start.y + (g_engine->getRandomNumber(camera.h - (2 * sc.fly.start.y))));
			YVel(sc.fly.vel.y);

			// Set state to flying
			ai_data.walk.enabled = true;
		}
	}
}

} // End of namespace Crab
