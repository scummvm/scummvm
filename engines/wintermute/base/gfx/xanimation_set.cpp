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
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/gfx/xanimation_set.h"
#include "engines/wintermute/base/gfx/xmodel.h"
#include "engines/wintermute/dcgf.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
AnimationSet::AnimationSet(BaseGame *inGame, XModel *model) : BaseNamedObject(inGame) {
	_frameTime = -1;
	_totalTime = 0;
	_looping = false;
	_model = model;
}

//////////////////////////////////////////////////////////////////////////
AnimationSet::~AnimationSet() {
	// remove child animations
	for (uint32 i = 0; i < _animations.size(); i++) {
		delete _animations[i];
	}
	_animations.clear();

	// remove events
	for (uint32 i = 0; i < _events.size(); i++) {
		delete _events[i];
	}
	_events.clear();
}

//////////////////////////////////////////////////////////////////////////
bool AnimationSet::findBones(FrameNode *rootFrame) {
	for (uint32 i = 0; i < _animations.size(); i++) {
		_animations[i]->findBone(rootFrame);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AnimationSet::addAnimation(Animation *anim) {
	if (!anim) {
		return false;
	} else {
		_animations.add(anim);
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////
bool AnimationSet::addEvent(AnimationEvent *event) {
	if (!event) {
		return false;
	} else {
		int frameTime = getFrameTime();
		if (frameTime < 0) {
			_gameRef->LOG(0, "Error adding animation event %s, no keyframes found", event->_eventName);
			delete event;
			return false;
		}

		int totalFrames = 0;
		if (frameTime > 0)
			totalFrames = getTotalTime() / frameTime + 1;

		if (event->_frame < 1)
			event->_frame = 1;
		if (event->_frame > totalFrames)
			event->_frame = totalFrames;

		_events.add(event);
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////
bool AnimationSet::update(int slot, uint32 localTime, float lerpValue) {
	bool res;
	for (uint32 i = 0; i < _animations.size(); i++) {
		res = _animations[i]->update(slot, localTime * ((float)_model->_ticksPerSecond / 1000.0f), lerpValue);
		if (!res) {
			return res;
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
int AnimationSet::getFrameTime() {
	if (_frameTime >= 0) {
		return _frameTime;
	}

	_frameTime = 0;
	for (uint32 i = 0; i < _animations.size(); i++) {
		int frameTime = _animations[i]->getFrameTime();
		if (_frameTime == 0) {
			_frameTime = frameTime / ((float)_model->_ticksPerSecond / 1000.0f);
		} else if (frameTime > 0) {
			_frameTime = MIN(float(_frameTime), frameTime / ((float)_model->_ticksPerSecond / 1000.0f));
		}
	}
	return _frameTime;
}

//////////////////////////////////////////////////////////////////////////
uint32 AnimationSet::getTotalTime() {
	if (_totalTime > 0) {
		return _totalTime;
	}

	_totalTime = 0;
	for (uint32 i = 0; i < _animations.size(); i++) {
		_totalTime = MAX((float)_totalTime, _animations[i]->getTotalTime() / ((float)_model->_ticksPerSecond / 1000.0f));
	}
	return _totalTime;
}

//////////////////////////////////////////////////////////////////////////
bool AnimationSet::onFrameChanged(int currentFrame, int prevFrame) {
	if (!_model || !_model->_owner) {
		return true;
	}

	if (prevFrame > currentFrame) {
		for (uint32 i = 0; i < _events.size(); i++) {
			if (_events[i]->_frame > prevFrame) {
				_model->_owner->applyEvent(_events[i]->_eventName);
			}
		}
		prevFrame = -1;
	}

	for (uint32 i = 0; i < _events.size(); i++) {
		if (_events[i]->_frame > prevFrame && _events[i]->_frame <= currentFrame) {
			_model->_owner->applyEvent(_events[i]->_eventName);
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AnimationSet::persist(BasePersistenceManager *persistMgr) {
	persistMgr->transferBool(TMEMBER(_looping));

	// persist events
	int32 numEvents;
	if (persistMgr->getIsSaving()) {
		numEvents = _events.size();
	}

	persistMgr->transferSint32(TMEMBER(numEvents));

	for (int i = 0; i < numEvents; i++) {
		if (persistMgr->getIsSaving()) {
			_events[i]->persist(persistMgr);
		} else {
			AnimationEvent *rvent = new AnimationEvent();
			rvent->persist(persistMgr);
			_events.add(rvent);
		}
	}

	return true;
}

} // namespace Wintermute
