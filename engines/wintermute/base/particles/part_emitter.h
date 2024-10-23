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

#ifndef WINTERMUTE_PARTEMITTER_H
#define WINTERMUTE_PARTEMITTER_H


#include "engines/wintermute/base/base_object.h"
#include "engines/wintermute/base/particles/part_force.h"

namespace Wintermute {
class BaseRegion;
class PartParticle;
class PartEmitter : public BaseObject {
public:
	DECLARE_PERSISTENT(PartEmitter, BaseObject)

	PartEmitter(BaseGame *inGame, BaseScriptHolder *Owner);
	~PartEmitter() override;

	int32 _fadeOutTime;

	bool start();

	bool update() override;
	bool display() override { return display(nullptr); } // To avoid shadowing the inherited display-function.
	bool display(BaseRegion *region);

	bool sortParticlesByZ();
	bool addSprite(const char *filename);
	bool removeSprite(const char *filename);
	bool setBorder(int x, int y, int width, int height);
	bool setBorderThickness(int thicknessLeft, int thicknessRight, int thicknessTop, int thicknessBottom);

	bool addForce(const Common::String &name, PartForce::TForceType type, int posX, int posY, float angle, float strength);
	bool removeForce(const Common::String &name);

	BaseArray<PartForce *> _forces;

	// scripting interface
	ScValue *scGetProperty(const Common::String &name) override;
	bool scSetProperty(const char *name, ScValue *value) override;
	bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) override;
	const char *scToString() override;


private:
	int32 _width;
	int32 _height;

	int32 _angle1;
	int32 _angle2;

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

	int32 _maxParticles;

	int32 _lifeTime1;
	int32 _lifeTime2;
	bool _lifeTimeZBased;

	int32 _genInterval;
	int32 _genAmount;

	bool _running;
	int32 _overheadTime;

	int32 _maxBatches;
	int32 _batchesGenerated;

	Rect32 _border;
	int32 _borderThicknessLeft;
	int32 _borderThicknessRight;
	int32 _borderThicknessTop;
	int32 _borderThicknessBottom;

	int32 _fadeInTime;

	int32 _alpha1;
	int32 _alpha2;
	bool _alphaTimeBased;

	bool _useRegion;

	char *_emitEvent;
	BaseScriptHolder *_owner;

	PartForce *addForceByName(const Common::String &name);
	int static compareZ(const void *obj1, const void *obj2);
	bool initParticle(PartParticle *particle, uint32 currentTime, uint32 timerDelta);
	bool updateInternal(uint32 currentTime, uint32 timerDelta);
	uint32 _lastGenTime;
	BaseArray<PartParticle *> _particles;
	BaseArray<char *> _sprites;
};

} // End of namespace Wintermute

#endif
