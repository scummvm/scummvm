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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/particles/part_particle.h"
#include "engines/wintermute/base/particles/part_emitter.h"
#include "engines/wintermute/base/base_sprite.h"
#include "engines/wintermute/utils/utils.h"
#include "engines/wintermute/platform_osystem.h"
#include "common/str.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
PartParticle::PartParticle(BaseGame *inGame) : BaseClass(inGame) {
	_pos = Vector2(0.0f, 0.0f);
	_posZ = 0.0f;
	_velocity = Vector2(0.0f, 0.0f);
	_scale = 100.0f;
	_sprite = nullptr;
	_creationTime = 0;
	_lifeTime = 0;
	_isDead = true;
	_border.setEmpty();

	_state = PARTICLE_NORMAL;
	_fadeStart = 0;
	_fadeTime = 0;
	_currentAlpha = 255;

	_alpha1 = _alpha2 = 255;

	_rotation = 0.0f;
	_angVelocity = 0.0f;

	_growthRate = 0.0f;
	_exponentialGrowth = false;
}


//////////////////////////////////////////////////////////////////////////
PartParticle::~PartParticle(void) {
	delete _sprite;
	_sprite = nullptr;
}

//////////////////////////////////////////////////////////////////////////
bool PartParticle::setSprite(const Common::String &filename) {
	if (_sprite && _sprite->getFilename() && scumm_stricmp(filename.c_str(), _sprite->getFilename()) == 0) {
		_sprite->reset();
		return STATUS_OK;
	}

	delete _sprite;
	_sprite = nullptr;

	SystemClassRegistry::getInstance()->_disabled = true;
	_sprite = new BaseSprite(_gameRef, (BaseObject*)_gameRef);
	if (_sprite && DID_SUCCEED(_sprite->loadFile(filename))) {
		SystemClassRegistry::getInstance()->_disabled = false;
		return STATUS_OK;
	} else {
		delete _sprite;
		_sprite = nullptr;
		SystemClassRegistry::getInstance()->_disabled = false;
		return STATUS_FAILED;
	}

}

//////////////////////////////////////////////////////////////////////////
bool PartParticle::update(PartEmitter *emitter, uint32 currentTime, uint32 timerDelta) {
	if (_state == PARTICLE_FADEIN) {
		if (currentTime - _fadeStart >= (uint32)_fadeTime) {
			_state = PARTICLE_NORMAL;
			_currentAlpha = _alpha1;
		} else {
			_currentAlpha = (int)(((float)currentTime - (float)_fadeStart) / (float)_fadeTime * _alpha1);
		}

		return STATUS_OK;
	} else if (_state == PARTICLE_FADEOUT) {
		if (currentTime - _fadeStart >= (uint32)_fadeTime) {
			_isDead = true;
			return STATUS_OK;
		} else {
			_currentAlpha = _fadeStartAlpha - (int)(((float)currentTime - (float)_fadeStart) / (float)_fadeTime * _fadeStartAlpha);
		}

		return STATUS_OK;
	} else {
		// time is up
		if (_lifeTime > 0) {
			if (currentTime - _creationTime >= (uint32)_lifeTime) {
				if (emitter->_fadeOutTime > 0) {
					fadeOut(currentTime, emitter->_fadeOutTime);
				} else {
					_isDead = true;
				}
			}
		}

		// particle hit the border
		if (!_isDead && !_border.isRectEmpty()) {
			Point32 p;
			p.x = (int32)_pos.x;
			p.y = (int32)_pos.y;
			if (!BasePlatform::ptInRect(&_border, p)) {
				fadeOut(currentTime, emitter->_fadeOutTime);
			}
		}
		if (_state != PARTICLE_NORMAL) {
			return STATUS_OK;
		}

		// update alpha
		if (_lifeTime > 0) {
			int age = (int)(currentTime - _creationTime);
			int alphaDelta = (int)(_alpha2 - _alpha1);

			_currentAlpha = _alpha1 + (int)(((float)alphaDelta / (float)_lifeTime * (float)age));
		}

		// update position
		float elapsedTime = (float)timerDelta / 1000.f;

		for (uint32 i = 0; i < emitter->_forces.size(); i++) {
			PartForce *force = emitter->_forces[i];
			switch (force->_type) {
			case PartForce::FORCE_GLOBAL:
				_velocity += force->_direction * elapsedTime;
				break;

			case PartForce::FORCE_POINT: {
				Vector2 vecDist = force->_pos - _pos;
				float dist = fabs(vecDist.length());

				dist = 100.0f / dist;

				_velocity += force->_direction * dist * elapsedTime;
			}
			break;

			default:
				break;
			}
		}
		_pos += _velocity * elapsedTime;

		// update rotation
		_rotation += _angVelocity * elapsedTime;
		_rotation = BaseUtils::normalizeAngle(_rotation);

		// update scale
		if (_exponentialGrowth) {
			_scale += _scale / 100.0f * _growthRate * elapsedTime;
		} else {
			_scale += _growthRate * elapsedTime;
		}

		if (_scale <= 0.0f) {
			_isDead = true;
		}


		return STATUS_OK;
	}
}

//////////////////////////////////////////////////////////////////////////
bool PartParticle::display(PartEmitter *emitter) {
	if (!_sprite) {
		return STATUS_FAILED;
	}
	if (_isDead) {
		return STATUS_OK;
	}

	_sprite->getCurrentFrame();
	return _sprite->display((int)_pos.x, (int)_pos.y,
	                        nullptr,
	                        _scale, _scale,
	                        BYTETORGBA(255, 255, 255, _currentAlpha),
	                        _rotation,
	                        emitter->_blendMode);
}


//////////////////////////////////////////////////////////////////////////
bool PartParticle::fadeIn(uint32 currentTime, int fadeTime) {
	_currentAlpha = 0;
	_fadeStart = currentTime;
	_fadeTime = fadeTime;
	_state = PARTICLE_FADEIN;

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool PartParticle::fadeOut(uint32 currentTime, int fadeTime) {
	//_currentAlpha = 255;
	_fadeStartAlpha = _currentAlpha;
	_fadeStart = currentTime;
	_fadeTime = fadeTime;
	_state = PARTICLE_FADEOUT;

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool PartParticle::persist(BasePersistenceManager *persistMgr) {
	persistMgr->transferSint32(TMEMBER(_alpha1));
	persistMgr->transferSint32(TMEMBER(_alpha2));
	persistMgr->transferRect32(TMEMBER(_border));
	persistMgr->transferVector2(TMEMBER(_pos));
	persistMgr->transferFloat(TMEMBER(_posZ));
	persistMgr->transferVector2(TMEMBER(_velocity));
	persistMgr->transferFloat(TMEMBER(_scale));
	persistMgr->transferUint32(TMEMBER(_creationTime));
	persistMgr->transferSint32(TMEMBER(_lifeTime));
	persistMgr->transferBool(TMEMBER(_isDead));
	persistMgr->transferSint32(TMEMBER_INT(_state));
	persistMgr->transferUint32(TMEMBER(_fadeStart));
	persistMgr->transferSint32(TMEMBER(_fadeTime));
	persistMgr->transferSint32(TMEMBER(_currentAlpha));
	persistMgr->transferFloat(TMEMBER(_angVelocity));
	persistMgr->transferFloat(TMEMBER(_rotation));
	persistMgr->transferFloat(TMEMBER(_growthRate));
	persistMgr->transferBool(TMEMBER(_exponentialGrowth));
	persistMgr->transferSint32(TMEMBER(_fadeStartAlpha));

	if (persistMgr->getIsSaving()) {
		const char *filename = _sprite->getFilename();
		persistMgr->transferConstChar(TMEMBER(filename));
	} else {
		char *filename;
		persistMgr->transferCharPtr(TMEMBER(filename));
		SystemClassRegistry::getInstance()->_disabled = true;
		setSprite(filename);
		SystemClassRegistry::getInstance()->_disabled = false;
		delete[] filename;
		filename = nullptr;
	}

	return STATUS_OK;
}

} // End of namespace Wintermute
