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
#include "engines/wintermute/base/gfx/xactive_animation.h"
#include "engines/wintermute/base/gfx/xmodel.h"
#include "engines/wintermute/dcgf.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
ActiveAnimation::ActiveAnimation(BaseGame *inGame, XModel *model) : BaseClass(inGame) {
	_model = model;

	_animation = nullptr;

	_looping = false;
	_finished = true;
	_startTime = 0;
	_lastLocalTime = 0;

	_currentFrame = -1;
}

//////////////////////////////////////////////////////////////////////////
ActiveAnimation::~ActiveAnimation() {
	_animation = nullptr; // ref only
	_model = nullptr;     // ref only
}

//////////////////////////////////////////////////////////////////////////
bool ActiveAnimation::start(AnimationSet *animation, bool looping) {
	_animation = animation;
	_startTime = _gameRef->_currentTime;
	_looping = looping;
	_finished = false;
	_currentFrame = -1;

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool ActiveAnimation::resetStartTime() {
	_startTime = _gameRef->_currentTime;
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool ActiveAnimation::update(int slot, bool prevFrameOnly, float lerpValue, bool forceStartFrame) {
	// PrevFrameOnly means: don't update, just use the last pose, since
	// we're transitioning from it to another animation

	if (!_animation) {
		return false;
	}

	uint32 localTime = 0;
	//_gameRef->LOG(0, "%s %d %d %f %d", m_Animation->m_Name, Slot, PrevFrameOnly, LerpValue, ForceStartFrame);
	if (prevFrameOnly) {
		localTime = _lastLocalTime;
	} else {
		if (!_finished) {
			localTime = _gameRef->_currentTime - _startTime;
			if (localTime > _animation->getTotalTime()) {
				if (_looping) {
					if (_animation->getTotalTime() == 0) {
						localTime = 0;
					} else {
						localTime %= _animation->getTotalTime();
					}
				} else {
					_finished = true;
				}
			}
		}
	}
	if (_finished) {
		localTime = _animation->getTotalTime() - 1;
	}

	_lastLocalTime = localTime;

	if (forceStartFrame) {
		localTime = 0;
	}

	// handle events
	int frame = 0;
	if (_animation->getFrameTime() > 0) {
		frame = localTime / _animation->getFrameTime() + 1;
	}

	if (frame != _currentFrame) {
		// don't trigger events when transitioning
		if (!prevFrameOnly) {
			_animation->onFrameChanged(frame, _currentFrame);
		}

		_currentFrame = frame;
	}
	//_gameRef->LOG(0, "%s %d %f", m_Animation->m_Name, LocalTime, LerpValue);
	return _animation->update(slot, localTime, lerpValue);
}

//////////////////////////////////////////////////////////////////////////
char *ActiveAnimation::getName() {
	if (_animation) {
		return _animation->_name;
	} else {
		return nullptr;
	}
}

//////////////////////////////////////////////////////////////////////////
bool ActiveAnimation::persist(BasePersistenceManager *persistMgr) {
	persistMgr->transferSint32(TMEMBER(_currentFrame));
	persistMgr->transferUint32(TMEMBER(_startTime));
	persistMgr->transferBool(TMEMBER(_looping));
	persistMgr->transferBool(TMEMBER(_finished));
	persistMgr->transferUint32(TMEMBER(_lastLocalTime));

	if (persistMgr->getIsSaving()) {
		persistMgr->transferCharPtr(TMEMBER(_animation->_name));
	} else {
		char *animName;
		persistMgr->transferCharPtr(TMEMBER(animName));
		if (animName) {
			_animation = _model->getAnimationSetByName(animName);
		} else {
			_animation = nullptr;
		}

		delete[] animName;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool ActiveAnimation::setLooping(bool looping) {
	_looping = looping;
	return true;
}

} // namespace Wintermute
