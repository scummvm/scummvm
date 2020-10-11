/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#ifndef WINTERMUTE_ACTIVE_ANIMATION_H
#define WINTERMUTE_ACTIVE_ANIMATION_H

#include "engines/wintermute/base/base.h"
#include "engines/wintermute/base/gfx/x/animation_set.h"

namespace Wintermute {

class BasePersistenceManager;

class ActiveAnimation : public BaseClass {
public:
	ActiveAnimation(BaseGame *inGame, ModelX *model);
	virtual ~ActiveAnimation();

	bool start(AnimationSet *animation, bool looping = false);
	bool update(int slot = 0, bool prevFrameOnly = false, float lerpValue = 0.0f, bool forceStartFrame = false);
	bool resetStartTime();
	bool persist(BasePersistenceManager *persistMgr);
	bool setLooping(bool looping);

	char *getName();

	AnimationSet *getAnimSet() {
		return _animation;
	};

	bool isLooping() {
		return _looping;
	};

	bool isFinished() {
		return _finished;
	};

private:
	ModelX *_model;
	int32 _currentFrame;
	uint32 _startTime;
	bool _looping;
	bool _finished;
	uint32 _lastLocalTime;

	AnimationSet *_animation;
};

} // namespace Wintermute

#endif
