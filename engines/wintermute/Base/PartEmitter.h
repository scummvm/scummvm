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


#include "engines/wintermute/Base/BObject.h"
#include "engines/wintermute/Base/PartForce.h"

namespace WinterMute {
class CBRegion;
class CPartParticle;
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

	ERRORCODE start();

	ERRORCODE update();
	ERRORCODE display() { return display(NULL); } // To avoid shadowing the inherited display-function.
	ERRORCODE display(CBRegion *region);

	ERRORCODE sortParticlesByZ();
	ERRORCODE addSprite(const char *filename);
	ERRORCODE removeSprite(const char *filename);
	ERRORCODE setBorder(int x, int y, int width, int height);
	ERRORCODE setBorderThickness(int thicknessLeft, int thicknessRight, int thicknessTop, int thicknessBottom);

	ERRORCODE addForce(const char *name, CPartForce::TForceType type, int posX, int posY, float angle, float strength);
	ERRORCODE removeForce(const char *name);

	CBArray<CPartForce *, CPartForce *> _forces;

	// scripting interface
	virtual CScValue *scGetProperty(const char *name);
	virtual ERRORCODE scSetProperty(const char *name, CScValue *value);
	virtual ERRORCODE scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name);
	virtual const char *scToString();


private:
	CPartForce *addForceByName(const char *name);
	int static compareZ(const void *obj1, const void *obj2);
	ERRORCODE initParticle(CPartParticle *particle, uint32 currentTime, uint32 timerDelta);
	ERRORCODE updateInternal(uint32 currentTime, uint32 timerDelta);
	uint32 _lastGenTime;
	CBArray<CPartParticle *, CPartParticle *> _particles;
	CBArray<char *, char *> _sprites;
};

} // end of namespace WinterMute

#endif
