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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

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

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/Base/PartEmitter.h"
#include "engines/wintermute/Base/PartParticle.h"
#include "engines/wintermute/math/Vector2.h"
#include "engines/wintermute/math/Matrix4.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "engines/wintermute/Base/scriptables/ScStack.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BRegion.h"
#include "engines/wintermute/Base/BFileManager.h"
#include "engines/wintermute/utils/utils.h"
#include "engines/wintermute/PlatformSDL.h"
#include "common/str.h"
#include "common/math.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CPartEmitter, false)

//////////////////////////////////////////////////////////////////////////
CPartEmitter::CPartEmitter(CBGame *inGame, CBScriptHolder *Owner) : CBObject(inGame) {
	_width = _height = 0;

	CBPlatform::SetRectEmpty(&_border);
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

	_emitEvent = NULL;
	_owner = Owner;
}


//////////////////////////////////////////////////////////////////////////
CPartEmitter::~CPartEmitter(void) {
	for (int i = 0; i < _particles.GetSize(); i++) {
		delete _particles[i];
	}
	_particles.RemoveAll();

	for (int i = 0; i < _forces.GetSize(); i++) {
		delete _forces[i];
	}
	_forces.RemoveAll();


	for (int i = 0; i < _sprites.GetSize(); i++) {
		delete [] _sprites[i];
	}
	_sprites.RemoveAll();

	delete[] _emitEvent;
	_emitEvent = NULL;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CPartEmitter::addSprite(const char *Filename) {
	if (!Filename) return E_FAIL;

	// do we already have the file?
	for (int i = 0; i < _sprites.GetSize(); i++) {
		if (scumm_stricmp(Filename, _sprites[i]) == 0) return S_OK;
	}

	// check if file exists
	Common::SeekableReadStream *File = Game->_fileManager->openFile(Filename);
	if (!File) {
		Game->LOG(0, "Sprite '%s' not found", Filename);
		return E_FAIL;
	} else Game->_fileManager->closeFile(File);

	char *Str = new char[strlen(Filename) + 1];
	strcpy(Str, Filename);
	_sprites.Add(Str);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CPartEmitter::removeSprite(const char *Filename) {
	for (int i = 0; i < _sprites.GetSize(); i++) {
		if (scumm_stricmp(Filename, _sprites[i]) == 0) {
			delete [] _sprites[i];
			_sprites.RemoveAt(i);
			return S_OK;
		}
	}
	return E_FAIL;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CPartEmitter::initParticle(CPartParticle *Particle, uint32 CurrentTime, uint32 TimerDelta) {
	if (!Particle) return E_FAIL;
	if (_sprites.GetSize() == 0) return E_FAIL;

	int PosX = CBUtils::RandomInt(_posX, _posX + _width);
	int PosY = CBUtils::RandomInt(_posY, _posY + _height);
	float PosZ = CBUtils::RandomFloat(0.0f, 100.0f);

	float Velocity;
	if (_velocityZBased) Velocity = _velocity1 + PosZ * (_velocity2 - _velocity1) / 100;
	else Velocity = CBUtils::RandomFloat(_velocity1, _velocity2);

	float Scale;
	if (_scaleZBased) Scale = _scale1 + PosZ * (_scale2 - _scale1) / 100;
	else Scale = CBUtils::RandomFloat(_scale1, _scale2);

	int LifeTime;
	if (_lifeTimeZBased) LifeTime = _lifeTime2 - PosZ * (_lifeTime2 - _lifeTime1) / 100;
	else LifeTime = CBUtils::RandomInt(_lifeTime1, _lifeTime2);

	float Angle = CBUtils::RandomAngle(_angle1, _angle2);
	int SpriteIndex = CBUtils::RandomInt(0, _sprites.GetSize() - 1);

	float Rotation = CBUtils::RandomAngle(_rotation1, _rotation2);
	float AngVelocity = CBUtils::RandomFloat(_angVelocity1, _angVelocity2);
	float GrowthRate = CBUtils::RandomFloat(_growthRate1, _growthRate2);

	if (!CBPlatform::IsRectEmpty(&_border)) {
		int ThicknessLeft   = (int)(_borderThicknessLeft   - (float)_borderThicknessLeft   * PosZ / 100.0f);
		int ThicknessRight  = (int)(_borderThicknessRight  - (float)_borderThicknessRight  * PosZ / 100.0f);
		int ThicknessTop    = (int)(_borderThicknessTop    - (float)_borderThicknessTop    * PosZ / 100.0f);
		int ThicknessBottom = (int)(_borderThicknessBottom - (float)_borderThicknessBottom * PosZ / 100.0f);

		Particle->_border = _border;
		Particle->_border.left += ThicknessLeft;
		Particle->_border.right -= ThicknessRight;
		Particle->_border.top += ThicknessTop;
		Particle->_border.bottom -= ThicknessBottom;
	}

	Vector2 VecPos((float)PosX, (float)PosY);
	Vector2 VecVel(0, Velocity);

	Matrix4 MatRot;
	MatRot.RotationZ(Common::deg2rad(CBUtils::NormalizeAngle(Angle - 180)));
	MatRot.TransformVector2(VecVel);

	if (_alphaTimeBased) {
		Particle->_alpha1 = _alpha1;
		Particle->_alpha2 = _alpha2;
	} else {
		int Alpha = CBUtils::RandomInt(_alpha1, _alpha2);
		Particle->_alpha1 = Alpha;
		Particle->_alpha2 = Alpha;
	}

	Particle->_creationTime = CurrentTime;
	Particle->_pos = VecPos;
	Particle->_posZ = PosZ;
	Particle->_velocity = VecVel;
	Particle->_scale = Scale;
	Particle->_lifeTime = LifeTime;
	Particle->_rotation = Rotation;
	Particle->_angVelocity = AngVelocity;
	Particle->_growthRate = GrowthRate;
	Particle->_exponentialGrowth = _exponentialGrowth;
	Particle->_isDead = FAILED(Particle->setSprite(_sprites[SpriteIndex]));
	Particle->fadeIn(CurrentTime, _fadeInTime);


	if (Particle->_isDead) return E_FAIL;
	else return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CPartEmitter::update() {
	if (!_running) return S_OK;
	else return updateInternal(Game->_timer, Game->_timerDelta);
}

//////////////////////////////////////////////////////////////////////////
HRESULT CPartEmitter::updateInternal(uint32 CurrentTime, uint32 TimerDelta) {
	int NumLive = 0;

	for (int i = 0; i < _particles.GetSize(); i++) {
		_particles[i]->update(this, CurrentTime, TimerDelta);

		if (!_particles[i]->_isDead) NumLive++;
	}


	// we're understaffed
	if (NumLive < _maxParticles) {
		bool NeedsSort = false;
		if (CurrentTime - _lastGenTime > _genInterval) {
			_lastGenTime = CurrentTime;
			_batchesGenerated++;

			if (_maxBatches > 0 && _batchesGenerated > _maxBatches) {
				return S_OK;
			}

			int ToGen = MIN(_genAmount, _maxParticles - NumLive);
			while (ToGen > 0) {
				int FirstDeadIndex = -1;
				for (int i = 0; i < _particles.GetSize(); i++) {
					if (_particles[i]->_isDead) {
						FirstDeadIndex = i;
						break;
					}
				}

				CPartParticle *Particle;
				if (FirstDeadIndex >= 0) Particle = _particles[FirstDeadIndex];
				else {
					Particle = new CPartParticle(Game);
					_particles.Add(Particle);
				}
				initParticle(Particle, CurrentTime, TimerDelta);
				NeedsSort = true;

				ToGen--;
			}
		}
		if (NeedsSort && (_scaleZBased || _velocityZBased || _lifeTimeZBased))
			sortParticlesByZ();

		// we actually generated some particles and we're not in fast-forward mode
		if (NeedsSort && _overheadTime == 0) {
			if (_owner && _emitEvent) _owner->applyEvent(_emitEvent);
		}
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CPartEmitter::display(CBRegion *Region) {
	if (_sprites.GetSize() <= 1) Game->_renderer->startSpriteBatch();

	for (int i = 0; i < _particles.GetSize(); i++) {
		if (Region != NULL && _useRegion) {
			if (!Region->PointInRegion(_particles[i]->_pos.x, _particles[i]->_pos.y)) continue;
		}

		_particles[i]->display(this);
	}

	if (_sprites.GetSize() <= 1) Game->_renderer->endSpriteBatch();

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CPartEmitter::start() {
	for (int i = 0; i < _particles.GetSize(); i++) {
		_particles[i]->_isDead = true;
	}
	_running = true;
	_batchesGenerated = 0;


	if (_overheadTime > 0) {
		uint32 Delta = 500;
		int Steps = _overheadTime / Delta;
		uint32 CurrentTime = Game->_timer - _overheadTime;

		for (int i = 0; i < Steps; i++) {
			updateInternal(CurrentTime, Delta);
			CurrentTime += Delta;
		}
		_overheadTime = 0;
	}


	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CPartEmitter::sortParticlesByZ() {
	// sort particles by _posY
	qsort(_particles.GetData(), _particles.GetSize(), sizeof(CPartParticle *), CPartEmitter::compareZ);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
int CPartEmitter::compareZ(const void *Obj1, const void *Obj2) {
	CPartParticle *P1 = *(CPartParticle **)Obj1;
	CPartParticle *P2 = *(CPartParticle **)Obj2;

	if (P1->_posZ < P2->_posZ) return -1;
	else if (P1->_posZ > P2->_posZ) return 1;
	else return 0;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CPartEmitter::setBorder(int X, int Y, int Width, int Height) {
	CBPlatform::SetRect(&_border, X, Y, X + Width, Y + Height);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CPartEmitter::setBorderThickness(int ThicknessLeft, int ThicknessRight, int ThicknessTop, int ThicknessBottom) {
	_borderThicknessLeft = ThicknessLeft;
	_borderThicknessRight = ThicknessRight;
	_borderThicknessTop = ThicknessTop;
	_borderThicknessBottom = ThicknessBottom;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
CPartForce *CPartEmitter::addForceByName(const char *name) {
	CPartForce *Force = NULL;

	for (int i = 0; i < _forces.GetSize(); i++) {
		if (scumm_stricmp(name, _forces[i]->_name) == 0) {
			Force = _forces[i];
			break;
		}
	}
	if (!Force) {
		Force = new CPartForce(Game);
		if (Force) {
			Force->setName(name);
			_forces.Add(Force);
		}
	}
	return Force;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CPartEmitter::addForce(const char *name, CPartForce::TForceType Type, int PosX, int PosY, float Angle, float Strength) {
	CPartForce *Force = addForceByName(name);
	if (!Force) return E_FAIL;

	Force->_type = Type;
	Force->_pos = Vector2(PosX, PosY);

	Force->_direction = Vector2(0, Strength);
	Matrix4 MatRot;
	MatRot.RotationZ(Common::deg2rad(CBUtils::NormalizeAngle(Angle - 180)));
	MatRot.TransformVector2(Force->_direction);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CPartEmitter::removeForce(const char *name) {
	for (int i = 0; i < _forces.GetSize(); i++) {
		if (scumm_stricmp(name, _forces[i]->_name) == 0) {
			delete _forces[i];
			_forces.RemoveAt(i);
			return S_OK;
		}
	}
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CPartEmitter::scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// SetBorder
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SetBorder") == 0) {
		stack->CorrectParams(4);
		int BorderX      = stack->Pop()->GetInt();
		int BorderY      = stack->Pop()->GetInt();
		int BorderWidth  = stack->Pop()->GetInt();
		int BorderHeight = stack->Pop()->GetInt();

		stack->PushBool(SUCCEEDED(setBorder(BorderX, BorderY, BorderWidth, BorderHeight)));

		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// SetBorderThickness
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetBorderThickness") == 0) {
		stack->CorrectParams(4);
		int Left   = stack->Pop()->GetInt();
		int Right  = stack->Pop()->GetInt();
		int Top    = stack->Pop()->GetInt();
		int Bottom = stack->Pop()->GetInt();

		stack->PushBool(SUCCEEDED(setBorderThickness(Left, Right, Top, Bottom)));

		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// AddSprite
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AddSprite") == 0) {
		stack->CorrectParams(1);
		const char *SpriteFile = stack->Pop()->GetString();
		stack->PushBool(SUCCEEDED(addSprite(SpriteFile)));

		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// RemoveSprite
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RemoveSprite") == 0) {
		stack->CorrectParams(1);
		const char *SpriteFile = stack->Pop()->GetString();
		stack->PushBool(SUCCEEDED(removeSprite(SpriteFile)));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Start
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Start") == 0) {
		stack->CorrectParams(1);
		_overheadTime = stack->Pop()->GetInt();
		stack->PushBool(SUCCEEDED(start()));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Stop
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Stop") == 0) {
		stack->CorrectParams(0);

		for (int i = 0; i < _particles.GetSize(); i++) {
			delete _particles[i];
		}
		_particles.RemoveAll();

		_running = false;
		stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Pause
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Pause") == 0) {
		stack->CorrectParams(0);
		_running = false;
		stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Resume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Resume") == 0) {
		stack->CorrectParams(0);
		_running = true;
		stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddGlobalForce
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AddGlobalForce") == 0) {
		stack->CorrectParams(3);
		const char *forceName = stack->Pop()->GetString();
		float Angle = stack->Pop()->GetFloat();
		float Strength = stack->Pop()->GetFloat();

		stack->PushBool(SUCCEEDED(addForce(forceName, CPartForce::FORCE_GLOBAL, 0, 0, Angle, Strength)));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddPointForce
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AddPointForce") == 0) {
		stack->CorrectParams(5);
		const char *forceName = stack->Pop()->GetString();
		int PosX = stack->Pop()->GetInt();
		int PosY = stack->Pop()->GetInt();
		float Angle = stack->Pop()->GetFloat();
		float Strength = stack->Pop()->GetFloat();

		stack->PushBool(SUCCEEDED(addForce(forceName, CPartForce::FORCE_GLOBAL, PosX, PosY, Angle, Strength)));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RemoveForce
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RemoveForce") == 0) {
		stack->CorrectParams(1);
		const char *forceName = stack->Pop()->GetString();

		stack->PushBool(SUCCEEDED(removeForce(forceName)));

		return S_OK;
	}

	else return CBObject::scCallMethod(script, stack, thisStack, name);
}

//////////////////////////////////////////////////////////////////////////
CScValue *CPartEmitter::scGetProperty(const char *name) {
	_scValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Type") == 0) {
		_scValue->SetString("particle-emitter");
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// X
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "X") == 0) {
		_scValue->SetInt(_posX);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Y
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Y") == 0) {
		_scValue->SetInt(_posY);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Width
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Width") == 0) {
		_scValue->SetInt(_width);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Height
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Height") == 0) {
		_scValue->SetInt(_height);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Scale1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Scale1") == 0) {
		_scValue->SetFloat(_scale1);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Scale2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Scale2") == 0) {
		_scValue->SetFloat(_scale2);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// ScaleZBased
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ScaleZBased") == 0) {
		_scValue->SetBool(_scaleZBased);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Velocity1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Velocity1") == 0) {
		_scValue->SetFloat(_velocity1);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Velocity2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Velocity2") == 0) {
		_scValue->SetFloat(_velocity2);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// VelocityZBased
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "VelocityZBased") == 0) {
		_scValue->SetBool(_velocityZBased);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// LifeTime1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LifeTime1") == 0) {
		_scValue->SetInt(_lifeTime1);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// LifeTime2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LifeTime2") == 0) {
		_scValue->SetInt(_lifeTime2);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// LifeTimeZBased
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LifeTimeZBased") == 0) {
		_scValue->SetBool(_lifeTimeZBased);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Angle1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Angle1") == 0) {
		_scValue->SetInt(_angle1);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Angle2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Angle2") == 0) {
		_scValue->SetInt(_angle2);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AngVelocity1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AngVelocity1") == 0) {
		_scValue->SetFloat(_angVelocity1);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// AngVelocity2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AngVelocity2") == 0) {
		_scValue->SetFloat(_angVelocity2);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Rotation1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Rotation1") == 0) {
		_scValue->SetFloat(_rotation1);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Rotation2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Rotation2") == 0) {
		_scValue->SetFloat(_rotation2);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Alpha1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Alpha1") == 0) {
		_scValue->SetInt(_alpha1);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Alpha2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Alpha2") == 0) {
		_scValue->SetInt(_alpha2);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// AlphaTimeBased
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AlphaTimeBased") == 0) {
		_scValue->SetBool(_alphaTimeBased);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MaxParticles
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MaxParticles") == 0) {
		_scValue->SetInt(_maxParticles);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// NumLiveParticles (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "NumLiveParticles") == 0) {
		int NumAlive = 0;
		for (int i = 0; i < _particles.GetSize(); i++) {
			if (_particles[i] && !_particles[i]->_isDead) NumAlive++;
		}
		_scValue->SetInt(NumAlive);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// GenerationInterval
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GenerationInterval") == 0) {
		_scValue->SetInt(_genInterval);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// GenerationAmount
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GenerationAmount") == 0) {
		_scValue->SetInt(_genAmount);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// MaxBatches
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MaxBatches") == 0) {
		_scValue->SetInt(_maxBatches);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// FadeInTime
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "FadeInTime") == 0) {
		_scValue->SetInt(_fadeInTime);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// FadeOutTime
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "FadeOutTime") == 0) {
		_scValue->SetInt(_fadeOutTime);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// GrowthRate1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GrowthRate1") == 0) {
		_scValue->SetFloat(_growthRate1);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// GrowthRate2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GrowthRate2") == 0) {
		_scValue->SetFloat(_growthRate2);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// ExponentialGrowth
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ExponentialGrowth") == 0) {
		_scValue->SetBool(_exponentialGrowth);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// UseRegion
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "UseRegion") == 0) {
		_scValue->SetBool(_useRegion);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// EmitEvent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "EmitEvent") == 0) {
		if (!_emitEvent) _scValue->SetNULL();
		else _scValue->SetString(_emitEvent);
		return _scValue;
	}

	else return CBObject::scGetProperty(name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CPartEmitter::scSetProperty(const char *name, CScValue *Value) {
	//////////////////////////////////////////////////////////////////////////
	// X
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "X") == 0) {
		_posX = Value->GetInt();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// Y
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Y") == 0) {
		_posY = Value->GetInt();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// Width
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Width") == 0) {
		_width = Value->GetInt();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// Height
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Height") == 0) {
		_height = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Scale1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Scale1") == 0) {
		_scale1 = Value->GetFloat();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// Scale2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Scale2") == 0) {
		_scale2 = Value->GetFloat();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// ScaleZBased
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ScaleZBased") == 0) {
		_scaleZBased = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Velocity1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Velocity1") == 0) {
		_velocity1 = Value->GetFloat();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// Velocity2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Velocity2") == 0) {
		_velocity2 = Value->GetFloat();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// VelocityZBased
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "VelocityZBased") == 0) {
		_velocityZBased = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LifeTime1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LifeTime1") == 0) {
		_lifeTime1 = Value->GetInt();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// LifeTime2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LifeTime2") == 0) {
		_lifeTime2 = Value->GetInt();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// LifeTimeZBased
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LifeTimeZBased") == 0) {
		_lifeTimeZBased = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Angle1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Angle1") == 0) {
		_angle1 = Value->GetInt();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// Angle2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Angle2") == 0) {
		_angle2 = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AngVelocity1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AngVelocity1") == 0) {
		_angVelocity1 = Value->GetFloat();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// AngVelocity2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AngVelocity2") == 0) {
		_angVelocity2 = Value->GetFloat();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Rotation1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Rotation1") == 0) {
		_rotation1 = Value->GetFloat();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// Rotation2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Rotation2") == 0) {
		_rotation2 = Value->GetFloat();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Alpha1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Alpha1") == 0) {
		_alpha1 = Value->GetInt();
		if (_alpha1 < 0) _alpha1 = 0;
		if (_alpha1 > 255) _alpha1 = 255;
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// Alpha2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Alpha2") == 0) {
		_alpha2 = Value->GetInt();
		if (_alpha2 < 0) _alpha2 = 0;
		if (_alpha2 > 255) _alpha2 = 255;
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// AlphaTimeBased
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AlphaTimeBased") == 0) {
		_alphaTimeBased = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MaxParticles
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MaxParticles") == 0) {
		_maxParticles = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GenerationInterval
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GenerationInterval") == 0) {
		_genInterval = Value->GetInt();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GenerationAmount
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GenerationAmount") == 0) {
		_genAmount = Value->GetInt();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// MaxBatches
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MaxBatches") == 0) {
		_maxBatches = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// FadeInTime
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "FadeInTime") == 0) {
		_fadeInTime = Value->GetInt();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// FadeOutTime
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "FadeOutTime") == 0) {
		_fadeOutTime = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GrowthRate1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GrowthRate1") == 0) {
		_growthRate1 = Value->GetFloat();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GrowthRate2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GrowthRate2") == 0) {
		_growthRate2 = Value->GetFloat();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// ExponentialGrowth
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ExponentialGrowth") == 0) {
		_exponentialGrowth = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// UseRegion
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "UseRegion") == 0) {
		_useRegion = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// EmitEvent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "EmitEvent") == 0) {
		delete[] _emitEvent;
		_emitEvent = NULL;
		if (!Value->IsNULL()) CBUtils::SetString(&_emitEvent, Value->GetString());
		return S_OK;
	}

	else return CBObject::scSetProperty(name, Value);
}


//////////////////////////////////////////////////////////////////////////
const char *CPartEmitter::scToString() {
	return "[particle emitter]";
}




//////////////////////////////////////////////////////////////////////////
HRESULT CPartEmitter::persist(CBPersistMgr *persistMgr) {
	CBObject::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_width));
	persistMgr->transfer(TMEMBER(_height));

	persistMgr->transfer(TMEMBER(_angle1));
	persistMgr->transfer(TMEMBER(_angle2));

	persistMgr->transfer(TMEMBER(_velocity1));
	persistMgr->transfer(TMEMBER(_velocity2));
	persistMgr->transfer(TMEMBER(_velocityZBased));

	persistMgr->transfer(TMEMBER(_scale1));
	persistMgr->transfer(TMEMBER(_scale2));
	persistMgr->transfer(TMEMBER(_scaleZBased));

	persistMgr->transfer(TMEMBER(_maxParticles));

	persistMgr->transfer(TMEMBER(_lifeTime1));
	persistMgr->transfer(TMEMBER(_lifeTime2));
	persistMgr->transfer(TMEMBER(_lifeTimeZBased));

	persistMgr->transfer(TMEMBER(_genInterval));
	persistMgr->transfer(TMEMBER(_genAmount));

	persistMgr->transfer(TMEMBER(_running));
	persistMgr->transfer(TMEMBER(_overheadTime));

	persistMgr->transfer(TMEMBER(_border));
	persistMgr->transfer(TMEMBER(_borderThicknessLeft));
	persistMgr->transfer(TMEMBER(_borderThicknessRight));
	persistMgr->transfer(TMEMBER(_borderThicknessTop));
	persistMgr->transfer(TMEMBER(_borderThicknessBottom));

	persistMgr->transfer(TMEMBER(_fadeInTime));
	persistMgr->transfer(TMEMBER(_fadeOutTime));

	persistMgr->transfer(TMEMBER(_alpha1));
	persistMgr->transfer(TMEMBER(_alpha2));
	persistMgr->transfer(TMEMBER(_alphaTimeBased));

	persistMgr->transfer(TMEMBER(_angVelocity1));
	persistMgr->transfer(TMEMBER(_angVelocity2));

	persistMgr->transfer(TMEMBER(_rotation1));
	persistMgr->transfer(TMEMBER(_rotation2));

	persistMgr->transfer(TMEMBER(_growthRate1));
	persistMgr->transfer(TMEMBER(_growthRate2));
	persistMgr->transfer(TMEMBER(_exponentialGrowth));

	persistMgr->transfer(TMEMBER(_useRegion));

	persistMgr->transfer(TMEMBER_INT(_maxBatches));
	persistMgr->transfer(TMEMBER_INT(_batchesGenerated));

	persistMgr->transfer(TMEMBER(_emitEvent));
	persistMgr->transfer(TMEMBER(_owner));


	_sprites.persist(persistMgr);

	int NumForces;
	if (persistMgr->_saving) {
		NumForces = _forces.GetSize();
		persistMgr->transfer(TMEMBER(NumForces));
		for (int i = 0; i < _forces.GetSize(); i++) {
			_forces[i]->persist(persistMgr);
		}
	} else {
		persistMgr->transfer(TMEMBER(NumForces));
		for (int i = 0; i < NumForces; i++) {
			CPartForce *Force = new CPartForce(Game);
			Force->persist(persistMgr);
			_forces.Add(Force);
		}
	}

	int NumParticles;
	if (persistMgr->_saving) {
		NumParticles = _particles.GetSize();
		persistMgr->transfer(TMEMBER(NumParticles));
		for (int i = 0; i < _particles.GetSize(); i++) {
			_particles[i]->persist(persistMgr);
		}
	} else {
		persistMgr->transfer(TMEMBER(NumParticles));
		for (int i = 0; i < NumParticles; i++) {
			CPartParticle *Particle = new CPartParticle(Game);
			Particle->persist(persistMgr);
			_particles.Add(Particle);
		}
	}

	return S_OK;
}

} // end of namespace WinterMute
