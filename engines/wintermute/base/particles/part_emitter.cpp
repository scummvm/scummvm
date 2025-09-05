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
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/particles/part_emitter.h"
#include "engines/wintermute/base/particles/part_particle.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/base_region.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/gfx/3dutils.h"
#include "engines/wintermute/utils/utils.h"
#include "engines/wintermute/platform_osystem.h"
#include "engines/wintermute/dcgf.h"

#include "common/str.h"
#include "math/utils.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(PartEmitter, false)

//////////////////////////////////////////////////////////////////////////
PartEmitter::PartEmitter(BaseGame *inGame, BaseScriptHolder *owner) : BaseObject(inGame) {
	_width = _height = 0;

	BasePlatform::setRectEmpty(&_border);
	_borderThicknessLeft = _borderThicknessRight = _borderThicknessTop = _borderThicknessBottom = 0;

	_angle1 = _angle2 = 0;

	_velocity1 = _velocity2 = 0.0f;
	_velocityZBased = false;

	_scale1 = _scale2 = 100.0f;
	_scaleZBased = false;

	_maxParticles = 100;

	_lifeTime1 = _lifeTime2 = 1000;
	_lifeTimeZBased = false;

	_lastGenTime = 0;
	_genInterval = 0;
	_genAmount = 1;

	_overheadTime = 0;
	_running = false;

	_maxBatches = 0;
	_batchesGenerated = 0;

	_fadeInTime = _fadeOutTime = 0;

	_alpha1 = _alpha2 = 255;
	_alphaTimeBased = false;

	_rotation1 = _rotation2 = 0.0f;
	_angVelocity1 = _angVelocity2 = 0.0f;

	_growthRate1 = _growthRate2 = 0.0f;
	_exponentialGrowth = false;

	_useRegion = false;

	_emitEvent = nullptr;
	_owner = owner;
}


//////////////////////////////////////////////////////////////////////////
PartEmitter::~PartEmitter() {
	for (int32 i = 0; i < _particles.getSize(); i++) {
		delete _particles[i];
	}
	_particles.removeAll();

	for (int32 i = 0; i < _forces.getSize(); i++) {
		delete _forces[i];
	}
	_forces.removeAll();


	for (int32 i = 0; i < _sprites.getSize(); i++) {
		delete[] _sprites[i];
	}
	_sprites.removeAll();

	SAFE_DELETE_ARRAY(_emitEvent);
}

//////////////////////////////////////////////////////////////////////////
bool PartEmitter::addSprite(const char *filename) {
	if (!filename) {
		return STATUS_FAILED;
	}

	// do we already have the file?
	for (int32 i = 0; i < _sprites.getSize(); i++) {
		if (scumm_stricmp(filename, _sprites[i]) == 0) {
			return STATUS_OK;
		}
	}

	// check if file exists
	if (!BaseFileManager::getEngineInstance()->hasFile(filename)) {
		BaseEngine::LOG(0, "Sprite '%s' not found", filename);
		return STATUS_FAILED;
	}

	size_t filenameSize = strlen(filename) + 1;
	char *str = new char[filenameSize];
	Common::strcpy_s(str, filenameSize, filename);
	_sprites.add(str);

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool PartEmitter::removeSprite(const char *filename) {
	for (int32 i = 0; i < _sprites.getSize(); i++) {
		if (scumm_stricmp(filename, _sprites[i]) == 0) {
			delete[] _sprites[i];
			_sprites.removeAt(i);
			return STATUS_OK;
		}
	}
	return STATUS_FAILED;
}

//////////////////////////////////////////////////////////////////////////
bool PartEmitter::initParticle(PartParticle *particle, uint32 currentTime, uint32 timerDelta) {
	if (!particle) {
		return STATUS_FAILED;
	}
	if (_sprites.getSize() == 0) {
		return STATUS_FAILED;
	}

	int posX = BaseUtils::randomInt(_posX, _posX + _width);
	int posY = BaseUtils::randomInt(_posY, _posY + _height);
	float posZ = BaseUtils::randomFloat(0.0f, 100.0f);

	float velocity;
	if (_velocityZBased) {
		velocity = _velocity1 + posZ * (_velocity2 - _velocity1) / 100;
	} else {
		velocity = BaseUtils::randomFloat(_velocity1, _velocity2);
	}

	float scale;
	if (_scaleZBased) {
		scale = _scale1 + posZ * (_scale2 - _scale1) / 100;
	} else {
		scale = BaseUtils::randomFloat(_scale1, _scale2);
	}

	int lifeTime;
	if (_lifeTimeZBased) {
		lifeTime = (int)(_lifeTime2 - posZ * (_lifeTime2 - _lifeTime1) / 100);
	} else {
		lifeTime = BaseUtils::randomInt(_lifeTime1, _lifeTime2);
	}

	float angle = BaseUtils::randomAngle(_angle1, _angle2);
	int32 spriteIndex = BaseUtils::randomInt(0, _sprites.getSize() - 1);

	float rotation = BaseUtils::randomAngle(_rotation1, _rotation2);
	float angVelocity = BaseUtils::randomFloat(_angVelocity1, _angVelocity2);
	float growthRate = BaseUtils::randomFloat(_growthRate1, _growthRate2);

	if (!BasePlatform::isRectEmpty(&_border)) {
		int thicknessLeft   = (int)(_borderThicknessLeft   - (float)_borderThicknessLeft   * posZ / 100.0f);
		int thicknessRight  = (int)(_borderThicknessRight  - (float)_borderThicknessRight  * posZ / 100.0f);
		int thicknessTop    = (int)(_borderThicknessTop    - (float)_borderThicknessTop    * posZ / 100.0f);
		int thicknessBottom = (int)(_borderThicknessBottom - (float)_borderThicknessBottom * posZ / 100.0f);

		particle->_border = _border;
		particle->_border.left += thicknessLeft;
		particle->_border.right -= thicknessRight;
		particle->_border.top += thicknessTop;
		particle->_border.bottom -= thicknessBottom;
	}

	DXVector2 vecPos((float)posX, (float)posY);
	DXVector2 vecVel(0, velocity);

	DXMatrix matRot;
	DXMatrixRotationZ(&matRot, degToRad(BaseUtils::normalizeAngle(angle - 180.0)));
	DXVec2TransformCoord(&vecVel, &vecVel, &matRot);

	if (_alphaTimeBased) {
		particle->_alpha1 = _alpha1;
		particle->_alpha2 = _alpha2;
	} else {
		int alpha = BaseUtils::randomInt(_alpha1, _alpha2);
		particle->_alpha1 = alpha;
		particle->_alpha2 = alpha;
	}

	particle->_creationTime = currentTime;
	particle->_pos = vecPos;
	particle->_posZ = posZ;
	particle->_velocity = vecVel;
	particle->_scale = scale;
	particle->_lifeTime = lifeTime;
	particle->_rotation = rotation;
	particle->_angVelocity = angVelocity;
	particle->_growthRate = growthRate;
	particle->_exponentialGrowth = _exponentialGrowth;
	particle->_isDead = DID_FAIL(particle->setSprite(_sprites[spriteIndex]));
	particle->fadeIn(currentTime, _fadeInTime);


	if (particle->_isDead) {
		return STATUS_FAILED;
	} else {
		return STATUS_OK;
	}
}

//////////////////////////////////////////////////////////////////////////
bool PartEmitter::update() {
	if (!_running) {
		return STATUS_OK;
	} else {
		return updateInternal(_game->_timer, _game->_timerDelta);
	}
}

//////////////////////////////////////////////////////////////////////////
bool PartEmitter::updateInternal(uint32 currentTime, uint32 timerDelta) {
	int numLive = 0;

	for (int32 i = 0; i < _particles.getSize(); i++) {
		_particles[i]->update(this, currentTime, timerDelta);

		if (!_particles[i]->_isDead) {
			numLive++;
		}
	}


	// we're understaffed
	if (numLive < _maxParticles) {
		bool needsSort = false;
		if ((int)(currentTime - _lastGenTime) > _genInterval) {
			_lastGenTime = currentTime;
			_batchesGenerated++;

			if (_maxBatches > 0 && _batchesGenerated > _maxBatches) {
				return STATUS_OK;
			}

			int toGen = MIN(_genAmount, _maxParticles - numLive);
			while (toGen > 0) {
				int32 firstDeadIndex = -1;
				for (int32 i = 0; i < _particles.getSize(); i++) {
					if (_particles[i]->_isDead) {
						firstDeadIndex = i;
						break;
					}
				}

				PartParticle *particle;
				if (firstDeadIndex >= 0) {
					particle = _particles[firstDeadIndex];
				} else {
					particle = new PartParticle(_game);
					_particles.add(particle);
				}
				initParticle(particle, currentTime, timerDelta);
				needsSort = true;

				toGen--;
			}
		}
		if (needsSort && (_scaleZBased || _velocityZBased || _lifeTimeZBased)) {
			sortParticlesByZ();
		}

		// we actually generated some particles and we're not in fast-forward mode
		if (needsSort && _overheadTime == 0) {
			if (_owner && _emitEvent) {
				_owner->applyEvent(_emitEvent);
			}
		}
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool PartEmitter::display(BaseRegion *region) {
	if (_sprites.getSize() <= 1) {
		BaseEngine::getRenderer()->startSpriteBatch();
	}

	for (int32 i = 0; i < _particles.getSize(); i++) {
		if (region != nullptr && _useRegion) {
			if (!region->pointInRegion((int)_particles[i]->_pos._x, (int)_particles[i]->_pos._y)) {
				continue;
			}
		}

		_particles[i]->display(this);
	}

	if (_sprites.getSize() <= 1) {
		BaseEngine::getRenderer()->endSpriteBatch();
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool PartEmitter::start() {
	for (int32 i = 0; i < _particles.getSize(); i++) {
		_particles[i]->_isDead = true;
	}
	_running = true;
	_batchesGenerated = 0;


	if (_overheadTime > 0) {
		uint32 delta = 500;
		int steps = _overheadTime / delta;
		uint32 currentTime = _game->_timer - _overheadTime;

		for (int i = 0; i < steps; i++) {
			updateInternal(currentTime, delta);
			currentTime += delta;
		}
		_overheadTime = 0;
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool PartEmitter::sortParticlesByZ() {
	// sort particles by _posY
	qsort_msvc(_particles.getData(), _particles.getSize(), sizeof(PartParticle *), PartEmitter::compareZ);
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
int PartEmitter::compareZ(const void *obj1, const void *obj2) {
	void *o1 = const_cast<void *>(obj1);
	void *o2 = const_cast<void *>(obj2);
	PartParticle *p1 = *(PartParticle **)o1;
	PartParticle *p2 = *(PartParticle **)o2;

	if (p1->_posZ < p2->_posZ)
		return -1;
	else if (p1->_posZ > p2->_posZ)
		return 1;
	else
		return 0;
}

//////////////////////////////////////////////////////////////////////////
bool PartEmitter::setBorder(int x, int y, int width, int height) {
	BasePlatform::setRect(&_border, x, y, x + width, y + height);

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool PartEmitter::setBorderThickness(int thicknessLeft, int thicknessRight, int thicknessTop, int thicknessBottom) {
	_borderThicknessLeft = thicknessLeft;
	_borderThicknessRight = thicknessRight;
	_borderThicknessTop = thicknessTop;
	_borderThicknessBottom = thicknessBottom;

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
PartForce *PartEmitter::addForceByName(const char *name) {
	PartForce *force = nullptr;

	for (int32 i = 0; i < _forces.getSize(); i++) {
		if (scumm_stricmp(name, _forces[i]->_name) == 0) {
			force = _forces[i];
			break;
		}
	}
	if (!force) {
		force = new PartForce(_game);
		if (force) {
			force->setName(name);
			_forces.add(force);
		}
	}
	return force;
}


//////////////////////////////////////////////////////////////////////////
bool PartEmitter::addForce(const char *name, PartForce::TForceType type, int posX, int posY, float angle, float strength) {
	PartForce *force = addForceByName(name);
	if (!force) {
		return STATUS_FAILED;
	}

	force->_type = type;
	force->_pos = DXVector2(posX, posY);

	force->_direction = DXVector2(0, strength);
	DXMatrix matRot;
	DXMatrixRotationZ(&matRot, degToRad(BaseUtils::normalizeAngle(angle - 180.0)));
	DXVec2TransformCoord(&force->_direction, &force->_direction, &matRot);

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool PartEmitter::removeForce(const char *name) {
	for (int32 i = 0; i < _forces.getSize(); i++) {
		if (scumm_stricmp(name, _forces[i]->_name) == 0) {
			delete _forces[i];
			_forces.removeAt(i);
			return STATUS_OK;
		}
	}
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
bool PartEmitter::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// SetBorder
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SetBorder") == 0) {
		stack->correctParams(4);
		int borderX      = stack->pop()->getInt();
		int borderY      = stack->pop()->getInt();
		int borderWidth  = stack->pop()->getInt();
		int borderHeight = stack->pop()->getInt();

		stack->pushBool(DID_SUCCEED(setBorder(borderX, borderY, borderWidth, borderHeight)));

		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// SetBorderThickness
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetBorderThickness") == 0) {
		stack->correctParams(4);
		int left   = stack->pop()->getInt();
		int right  = stack->pop()->getInt();
		int top    = stack->pop()->getInt();
		int bottom = stack->pop()->getInt();

		stack->pushBool(DID_SUCCEED(setBorderThickness(left, right, top, bottom)));

		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// AddSprite
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AddSprite") == 0) {
		stack->correctParams(1);
		const char *spriteFile = stack->pop()->getString();
		stack->pushBool(DID_SUCCEED(addSprite(spriteFile)));

		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// RemoveSprite
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RemoveSprite") == 0) {
		stack->correctParams(1);
		const char *spriteFile = stack->pop()->getString();
		stack->pushBool(DID_SUCCEED(removeSprite(spriteFile)));

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Start
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Start") == 0) {
		stack->correctParams(1);
		_overheadTime = stack->pop()->getInt();
		stack->pushBool(DID_SUCCEED(start()));

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Stop
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Stop") == 0) {
		stack->correctParams(0);

		for (int32 i = 0; i < _particles.getSize(); i++) {
			delete _particles[i];
		}
		_particles.removeAll();

		_running = false;
		stack->pushBool(true);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Pause
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Pause") == 0) {
		stack->correctParams(0);
		_running = false;
		stack->pushBool(true);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Resume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Resume") == 0) {
		stack->correctParams(0);
		_running = true;
		stack->pushBool(true);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddGlobalForce
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AddGlobalForce") == 0) {
		stack->correctParams(3);
		const char *forceName = stack->pop()->getString();
		float angle = stack->pop()->getFloat();
		float strength = stack->pop()->getFloat();

		stack->pushBool(DID_SUCCEED(addForce(forceName, PartForce::FORCE_GLOBAL, 0, 0, angle, strength)));

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddPointForce
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AddPointForce") == 0) {
		stack->correctParams(5);
		const char *forceName = stack->pop()->getString();
		int posX = stack->pop()->getInt();
		int posY = stack->pop()->getInt();
		float angle = stack->pop()->getFloat();
		float strength = stack->pop()->getFloat();

		stack->pushBool(DID_SUCCEED(addForce(forceName, PartForce::FORCE_GLOBAL, posX, posY, angle, strength)));

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RemoveForce
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RemoveForce") == 0) {
		stack->correctParams(1);
		const char *forceName = stack->pop()->getString();

		stack->pushBool(DID_SUCCEED(removeForce(forceName)));

		return STATUS_OK;
	} else {
		return BaseObject::scCallMethod(script, stack, thisStack, name);
	}
}

//////////////////////////////////////////////////////////////////////////
ScValue *PartEmitter::scGetProperty(const char *name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Type") == 0) {
		_scValue->setString("particle-emitter");
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// X
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "X") == 0) {
		_scValue->setInt(_posX);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Y
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Y") == 0) {
		_scValue->setInt(_posY);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Width
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Width") == 0) {
		_scValue->setInt(_width);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Height
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Height") == 0) {
		_scValue->setInt(_height);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Scale1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Scale1") == 0) {
		_scValue->setFloat(_scale1);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Scale2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Scale2") == 0) {
		_scValue->setFloat(_scale2);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// ScaleZBased
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ScaleZBased") == 0) {
		_scValue->setBool(_scaleZBased);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Velocity1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Velocity1") == 0) {
		_scValue->setFloat(_velocity1);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Velocity2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Velocity2") == 0) {
		_scValue->setFloat(_velocity2);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// VelocityZBased
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "VelocityZBased") == 0) {
		_scValue->setBool(_velocityZBased);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// LifeTime1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LifeTime1") == 0) {
		_scValue->setInt(_lifeTime1);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// LifeTime2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LifeTime2") == 0) {
		_scValue->setInt(_lifeTime2);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// LifeTimeZBased
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LifeTimeZBased") == 0) {
		_scValue->setBool(_lifeTimeZBased);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Angle1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Angle1") == 0) {
		_scValue->setInt(_angle1);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Angle2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Angle2") == 0) {
		_scValue->setInt(_angle2);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AngVelocity1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AngVelocity1") == 0) {
		_scValue->setFloat(_angVelocity1);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// AngVelocity2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AngVelocity2") == 0) {
		_scValue->setFloat(_angVelocity2);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Rotation1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Rotation1") == 0) {
		_scValue->setFloat(_rotation1);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Rotation2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Rotation2") == 0) {
		_scValue->setFloat(_rotation2);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Alpha1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Alpha1") == 0) {
		_scValue->setInt(_alpha1);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Alpha2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Alpha2") == 0) {
		_scValue->setInt(_alpha2);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// AlphaTimeBased
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AlphaTimeBased") == 0) {
		_scValue->setBool(_alphaTimeBased);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MaxParticles
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MaxParticles") == 0) {
		_scValue->setInt(_maxParticles);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// NumLiveParticles (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "NumLiveParticles") == 0) {
		int numAlive = 0;
		for (int32 i = 0; i < _particles.getSize(); i++) {
			if (_particles[i] && !_particles[i]->_isDead) {
				numAlive++;
			}
		}
		_scValue->setInt(numAlive);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// GenerationInterval
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GenerationInterval") == 0) {
		_scValue->setInt(_genInterval);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// GenerationAmount
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GenerationAmount") == 0) {
		_scValue->setInt(_genAmount);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// MaxBatches
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MaxBatches") == 0) {
		_scValue->setInt(_maxBatches);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// FadeInTime
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "FadeInTime") == 0) {
		_scValue->setInt(_fadeInTime);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// FadeOutTime
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "FadeOutTime") == 0) {
		_scValue->setInt(_fadeOutTime);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// GrowthRate1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GrowthRate1") == 0) {
		_scValue->setFloat(_growthRate1);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// GrowthRate2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GrowthRate2") == 0) {
		_scValue->setFloat(_growthRate2);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// ExponentialGrowth
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ExponentialGrowth") == 0) {
		_scValue->setBool(_exponentialGrowth);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// UseRegion
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "UseRegion") == 0) {
		_scValue->setBool(_useRegion);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// EmitEvent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "EmitEvent") == 0) {
		if (!_emitEvent) {
			_scValue->setNULL();
		} else {
			_scValue->setString(_emitEvent);
		}
		return _scValue;
	} else {
		return BaseObject::scGetProperty(name);
	}
}


//////////////////////////////////////////////////////////////////////////
bool PartEmitter::scSetProperty(const char *name, ScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// X
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "X") == 0) {
		_posX = value->getInt();
		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// Y
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Y") == 0) {
		_posY = value->getInt();
		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// Width
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Width") == 0) {
		_width = value->getInt();
		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// Height
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Height") == 0) {
		_height = value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Scale1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Scale1") == 0) {
		_scale1 = value->getFloat();
		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// Scale2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Scale2") == 0) {
		_scale2 = value->getFloat();
		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// ScaleZBased
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ScaleZBased") == 0) {
		_scaleZBased = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Velocity1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Velocity1") == 0) {
		_velocity1 = value->getFloat();
		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// Velocity2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Velocity2") == 0) {
		_velocity2 = value->getFloat();
		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// VelocityZBased
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "VelocityZBased") == 0) {
		_velocityZBased = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LifeTime1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LifeTime1") == 0) {
		_lifeTime1 = value->getInt();
		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// LifeTime2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LifeTime2") == 0) {
		_lifeTime2 = value->getInt();
		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// LifeTimeZBased
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LifeTimeZBased") == 0) {
		_lifeTimeZBased = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Angle1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Angle1") == 0) {
		_angle1 = value->getInt();
		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// Angle2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Angle2") == 0) {
		_angle2 = value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AngVelocity1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AngVelocity1") == 0) {
		_angVelocity1 = value->getFloat();
		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// AngVelocity2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AngVelocity2") == 0) {
		_angVelocity2 = value->getFloat();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Rotation1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Rotation1") == 0) {
		_rotation1 = value->getFloat();
		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// Rotation2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Rotation2") == 0) {
		_rotation2 = value->getFloat();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Alpha1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Alpha1") == 0) {
		_alpha1 = value->getInt();
		if (_alpha1 < 0) {
			_alpha1 = 0;
		}
		if (_alpha1 > 255) {
			_alpha1 = 255;
		}
		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// Alpha2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Alpha2") == 0) {
		_alpha2 = value->getInt();
		if (_alpha2 < 0) {
			_alpha2 = 0;
		}
		if (_alpha2 > 255) {
			_alpha2 = 255;
		}
		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// AlphaTimeBased
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AlphaTimeBased") == 0) {
		_alphaTimeBased = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MaxParticles
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MaxParticles") == 0) {
		_maxParticles = value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GenerationInterval
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GenerationInterval") == 0) {
		_genInterval = value->getInt();
		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GenerationAmount
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GenerationAmount") == 0) {
		_genAmount = value->getInt();
		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// MaxBatches
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MaxBatches") == 0) {
		_maxBatches = value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// FadeInTime
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "FadeInTime") == 0) {
		_fadeInTime = value->getInt();
		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// FadeOutTime
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "FadeOutTime") == 0) {
		_fadeOutTime = value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GrowthRate1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GrowthRate1") == 0) {
		_growthRate1 = value->getFloat();
		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GrowthRate2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GrowthRate2") == 0) {
		_growthRate2 = value->getFloat();
		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// ExponentialGrowth
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ExponentialGrowth") == 0) {
		_exponentialGrowth = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// UseRegion
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "UseRegion") == 0) {
		_useRegion = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// EmitEvent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "EmitEvent") == 0) {
		SAFE_DELETE_ARRAY(_emitEvent);
		if (!value->isNULL()) {
			BaseUtils::setString(&_emitEvent, value->getString());
		}
		return STATUS_OK;
	} else {
		return BaseObject::scSetProperty(name, value);
	}
}


//////////////////////////////////////////////////////////////////////////
const char *PartEmitter::scToString() {
	return "[particle emitter]";
}




//////////////////////////////////////////////////////////////////////////
bool PartEmitter::persist(BasePersistenceManager *persistMgr) {
	BaseObject::persist(persistMgr);

	persistMgr->transferSint32(TMEMBER(_width));
	persistMgr->transferSint32(TMEMBER(_height));

	persistMgr->transferSint32(TMEMBER(_angle1));
	persistMgr->transferSint32(TMEMBER(_angle2));

	persistMgr->transferFloat(TMEMBER(_velocity1));
	persistMgr->transferFloat(TMEMBER(_velocity2));
	persistMgr->transferBool(TMEMBER(_velocityZBased));

	persistMgr->transferFloat(TMEMBER(_scale1));
	persistMgr->transferFloat(TMEMBER(_scale2));
	persistMgr->transferBool(TMEMBER(_scaleZBased));

	persistMgr->transferSint32(TMEMBER(_maxParticles));

	persistMgr->transferSint32(TMEMBER(_lifeTime1));
	persistMgr->transferSint32(TMEMBER(_lifeTime2));
	persistMgr->transferBool(TMEMBER(_lifeTimeZBased));

	persistMgr->transferSint32(TMEMBER(_genInterval));
	persistMgr->transferSint32(TMEMBER(_genAmount));

	persistMgr->transferBool(TMEMBER(_running));
	persistMgr->transferSint32(TMEMBER(_overheadTime));

	persistMgr->transferRect32(TMEMBER(_border));
	persistMgr->transferSint32(TMEMBER(_borderThicknessLeft));
	persistMgr->transferSint32(TMEMBER(_borderThicknessRight));
	persistMgr->transferSint32(TMEMBER(_borderThicknessTop));
	persistMgr->transferSint32(TMEMBER(_borderThicknessBottom));

	persistMgr->transferSint32(TMEMBER(_fadeInTime));
	persistMgr->transferSint32(TMEMBER(_fadeOutTime));

	persistMgr->transferSint32(TMEMBER(_alpha1));
	persistMgr->transferSint32(TMEMBER(_alpha2));
	persistMgr->transferBool(TMEMBER(_alphaTimeBased));

	persistMgr->transferFloat(TMEMBER(_angVelocity1));
	persistMgr->transferFloat(TMEMBER(_angVelocity2));

	persistMgr->transferFloat(TMEMBER(_rotation1));
	persistMgr->transferFloat(TMEMBER(_rotation2));

	persistMgr->transferFloat(TMEMBER(_growthRate1));
	persistMgr->transferFloat(TMEMBER(_growthRate2));
	persistMgr->transferBool(TMEMBER(_exponentialGrowth));

	persistMgr->transferBool(TMEMBER(_useRegion));

	persistMgr->transferSint32(TMEMBER_INT(_maxBatches));
	persistMgr->transferSint32(TMEMBER_INT(_batchesGenerated));

	persistMgr->transferCharPtr(TMEMBER(_emitEvent));
	persistMgr->transferPtr(TMEMBER_PTR(_owner));


	_sprites.persist(persistMgr);

	int32 numForces;
	if (persistMgr->getIsSaving()) {
		numForces = _forces.getSize();
		persistMgr->transferSint32(TMEMBER(numForces));
		for (int32 i = 0; i < _forces.getSize(); i++) {
			_forces[i]->persist(persistMgr);
		}
	} else {
		persistMgr->transferSint32(TMEMBER(numForces));
		for (int32 i = 0; i < numForces; i++) {
			PartForce *force = new PartForce(_game);
			force->persist(persistMgr);
			_forces.add(force);
		}
	}

	int32 numParticles;
	if (persistMgr->getIsSaving()) {
		numParticles = _particles.getSize();
		persistMgr->transferSint32(TMEMBER(numParticles));
		for (int32 i = 0; i < _particles.getSize(); i++) {
			_particles[i]->persist(persistMgr);
		}
	} else {
		persistMgr->transferSint32(TMEMBER(numParticles));
		for (int32 i = 0; i < numParticles; i++) {
			PartParticle *particle = new PartParticle(_game);
			particle->persist(persistMgr);
			_particles.add(particle);
		}
	}

	return STATUS_OK;
}

} // End of namespace Wintermute
