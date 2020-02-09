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

#ifndef MUTATIONOFJB_OBJECTANIMATIONTASK_H
#define MUTATIONOFJB_OBJECTANIMATIONTASK_H

#include "mutationofjb/tasks/task.h"

#include "mutationofjb/timer.h"

namespace MutationOfJB {

struct Object;

class ObjectAnimationTask : public Task {
public:
	ObjectAnimationTask();

	void start() override;
	void update() override;

	/**
	 * Advances every object animation in the current scene to the next frame.
	 *
	 * Normally the animation restarts after the last object frame. However, some animations have random
	 * elements to them. If _randomFrame is set, the animation restarts when _randomFrame is reached.
	 * Additionally, there is a chance with each frame until _randomFrame that the animation may jump
	 * straight to _randomFrame and continue until the last frame, then wrap around to the first frame.
	 *
	 * Randomness is used to introduce variety - e.g. in the starting scene a perched bird occasionally
	 * spreads its wings.
	 */
	void updateObjects();

	/**
	 * Nasty, hacky stuff the original game does to make some complex animations
	 * in the Carnival and Tavern Earthquake scenes possible.
	 *
	 * @param object Object to process.
	 * @return Whether to draw the object. It's important to respect this, otherwise
	 * some of the hardcoded animations would suffer from graphical glitches.
	 */
	bool handleHardcodedAnimation(Object *const object);

private:
	Timer _timer;
};

}

#endif
