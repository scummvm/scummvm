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

#ifndef WINTERMUTE_PARTEMITTER_H
#define WINTERMUTE_PARTEMITTER_H


#include "BObject.h"
#include "PartParticle.h"
#include "PartForce.h"

namespace WinterMute {
class CBRegion;

class CPartEmitter : public CBObject {
public:
	DECLARE_PERSISTENT(CPartEmitter, CBObject)

	CPartEmitter(CBGame *inGame, CBScriptHolder *Owner);
	virtual ~CPartEmitter(void);

	int _width;
	int _height;

	int _angle1;
	int _angle2;

	float _rotation1;
	float _rotation2;

	float _angVelocity1;
	float _angVelocity2;

	float _growthRate1;
	float _growthRate2;
	bool _exponentialGrowth;

	float _velocity1;
	float _velocity2;
	bool _velocityZBased;

	float _scale1;
	float _scale2;
	bool _scaleZBased;

	int _maxParticles;

	int _lifeTime1;
	int _lifeTime2;
	bool _lifeTimeZBased;

	int _genInterval;
	int _genAmount;

	bool _running;
	int _overheadTime;

	int _maxBatches;
	int _batchesGenerated;

	RECT _border;
	int _borderThicknessLeft;
	int _borderThicknessRight;
	int _borderThicknessTop;
	int _borderThicknessBottom;

	int _fadeInTime;
	int _fadeOutTime;

	int _alpha1;
	int _alpha2;
	bool _alphaTimeBased;

	bool _useRegion;

	char *_emitEvent;
	CBScriptHolder *_owner;

	HRESULT Start();

	HRESULT Update();
	HRESULT Display(CBRegion *Region = NULL);

	HRESULT SortParticlesByZ();
	HRESULT AddSprite(const char *Filename);
	HRESULT RemoveSprite(const char *Filename);
	HRESULT SetBorder(int X, int Y, int Width, int Height);
	HRESULT SetBorderThickness(int ThicknessLeft, int ThicknessRight, int ThicknessTop, int ThicknessBottom);

	HRESULT AddForce(const char *Name, CPartForce::TForceType Type, int PosX, int PosY, float Angle, float Strength);
	HRESULT RemoveForce(const char *Name);

	CBArray<CPartForce *, CPartForce *> _forces;

	// scripting interface
	virtual CScValue *ScGetProperty(const char *Name);
	virtual HRESULT ScSetProperty(const char *Name, CScValue *Value);
	virtual HRESULT ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name);
	virtual char *ScToString();


private:
	CPartForce *AddForceByName(const char *Name);
	int static CompareZ(const void *Obj1, const void *Obj2);
	HRESULT InitParticle(CPartParticle *Particle, uint32 CurrentTime, uint32 TimerDelta);
	HRESULT UpdateInternal(uint32 CurrentTime, uint32 TimerDelta);
	uint32 _lastGenTime;
	CBArray<CPartParticle *, CPartParticle *> _particles;
	CBArray<char *, char *> _sprites;
};

} // end of namespace WinterMute

#endif
