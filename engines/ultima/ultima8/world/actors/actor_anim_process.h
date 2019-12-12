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

#ifndef WORLD_ACTORS_ACTORANIMPROCESS_H
#define WORLD_ACTORS_ACTORANIMPROCESS_H

#include "ultima/ultima8/kernel/process.h"
#include "ultima/ultima8/world/actors/animation.h"

namespace Ultima8 {

class Actor;
struct AnimAction;
class AnimationTracker;
class Item;

class ActorAnimProcess : public Process {
public:
	ActorAnimProcess();
	//! note: this probably needs some more parameters
	ActorAnimProcess(Actor *actor, Animation::Sequence action, uint32 dir,
	                 uint32 steps = 0);

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE()

	virtual void run();

	virtual void terminate();

	virtual void dumpInfo();

	Animation::Sequence getAction() const {
		return action;
	}

	bool loadData(IDataSource *ids, uint32 version);
protected:
	virtual void saveData(ODataSource *ods);

	virtual bool init();

	//! perform special action for an animation
	void doSpecial();

	//! perform special action when hitting an opponent
	void doHitSpecial(Item *hit);

	Animation::Sequence action;
	uint32 dir;
	uint32 steps;

	AnimationTracker *tracker;
	int repeatcounter;
	uint32 currentstep;

	bool firstframe;

	bool animAborted;

	bool attackedSomething; // attacked and hit something with this animation
};

} // End of namespace Ultima8

#endif
