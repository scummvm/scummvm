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

#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/gfx/x/animation_channel.h"
#include "engines/wintermute/dcgf.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
AnimationChannel::AnimationChannel(BaseGame *inGame, ModelX *model) : BaseClass(inGame),
	_model(model), _transitioning(false),
	_transitionStart(0), _transtitionTime(0),
	_stopTransitionTime(0) {
	_anim[0] = _anim[1] = nullptr;
}

//////////////////////////////////////////////////////////////////////////
AnimationChannel::~AnimationChannel() {
	delete _anim[0];
	delete _anim[1];

	_model = nullptr; // ref only
}

//////////////////////////////////////////////////////////////////////////
bool AnimationChannel::playAnim(AnimationSet *animSet, uint32 transitionTime, uint32 stopTransitionTime) {
	_stopTransitionTime = stopTransitionTime;

	ActiveAnimation *anim = nullptr;
	if (animSet != nullptr) {
		anim = new ActiveAnimation(_gameRef, _model);
		anim->start(animSet, animSet->_looping);
	}

	if (transitionTime == 0) {
		delete _anim[0];
		delete _anim[1];
		_anim[0] = nullptr;
		_anim[1] = nullptr;

		_anim[0] = anim;
		_transitioning = false;
	} else {
		delete _anim[1];
		_anim[1] = nullptr;

		if (_anim[0]) {
			_anim[1] = anim;
			_transitioning = (anim != nullptr);
			_transtitionTime = transitionTime;
			_transitionStart = _gameRef->_currentTime;
		} else {
			_anim[0] = anim;
			delete _anim[1];
			_anim[1] = nullptr;
			_transitioning = false;
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AnimationChannel::stopAnim(uint32 transitionTime) {
	if (transitionTime == 0 || !_anim[0]) {
		_transitioning = false;
		delete _anim[0];
		delete _anim[1];
		_anim[0] = nullptr;
		_anim[1] = nullptr;
	} else {
		delete _anim[1];
		_anim[1] = nullptr;

		if (_anim[0]) {
			_anim[0]->setLooping(false);
		}

		_transitioning = true;
		_transtitionTime = transitionTime;
		_transitionStart = _gameRef->_currentTime;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AnimationChannel::update(bool debug) {
	if (_transitioning) {
		uint32 delta = _gameRef->_currentTime - _transitionStart;

		if (delta >= _transtitionTime) {
			_transitioning = false;

			// shift second animation to first slot and update it
			delete _anim[0];
			_anim[0] = _anim[1];
			_anim[1] = nullptr;

			if (_anim[0]) {
				// we have to reset the start time because we wasted some on transitioning
				_anim[0]->resetStartTime();
				return _anim[0]->update();
			}
		} else {
			float LerpValue = float(_gameRef->_currentTime - _transitionStart) / float(_transtitionTime);

			if (_anim[0]) {
				_anim[0]->update(0, true, LerpValue);
			}

			if (_anim[1]) {
				_anim[1]->update(1, true);
			} else {
				// disabled in WME 1.8.7, was causing glitches
				//m_Anim[0]->Update(1, true, 0, true);
			}

			return true;
		}
	} else {
		if (_anim[0]) {
			//return m_Anim[0]->Update();
			_anim[0]->update();
			if (_anim[0]->isFinished()) {
				stopAnim(_stopTransitionTime);
			}
			return true;
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AnimationChannel::isPlaying() {
	if (_anim[1] && !_anim[1]->isFinished()) {
		return true;
	} else if (_anim[0] && !_anim[0]->isFinished()) {
		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
char *AnimationChannel::getName() {
	if (_anim[1]) {
		return _anim[1]->getName();
	} else if (_anim[0]) {
		return _anim[0]->getName();
	} else {
		return nullptr;
	}
}

//////////////////////////////////////////////////////////////////////////
bool AnimationChannel::persist(BasePersistenceManager *persistMgr) {
	persistMgr->transferBool(TMEMBER(_transitioning));
	persistMgr->transferUint32(TMEMBER(_transitionStart));
	persistMgr->transferUint32(TMEMBER(_transtitionTime));

	persistMgr->transferUint32(TMEMBER(_stopTransitionTime));

	for (int i = 0; i < 2; i++) {
		bool animExists = false;
		if (persistMgr->getIsSaving()) {
			animExists = (_anim[i] != nullptr);
		}

		persistMgr->transferBool(TMEMBER(animExists));

		if (!persistMgr->getIsSaving()) {
			if (animExists)
				_anim[i] = new ActiveAnimation(_gameRef, _model);
			else
				_anim[i] = nullptr;
		}
		if (_anim[i]) {
			_anim[i]->persist(persistMgr);
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AnimationChannel::unloadAnim(AnimationSet *animSet) {
	if (_anim[0] && _anim[0]->getAnimSet() == animSet) {
		delete _anim[0];
		_anim[0] = nullptr;
	}

	if (_anim[1] && _anim[1]->getAnimSet() == animSet) {
		delete _anim[1];
		_anim[1] = nullptr;
	}

	return true;
}

} // namespace Wintermute
