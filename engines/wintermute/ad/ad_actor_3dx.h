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

#ifndef WINTERMUTE_AD_ACTOR_3DX_H
#define WINTERMUTE_AD_ACTOR_3DX_H

#include "engines/wintermute/ad/ad_object_3d.h"
#include "engines/wintermute/base/base_animation_transition_time.h"
#include "engines/wintermute/base/base_point.h"
#include "engines/wintermute/base/gfx/xmath.h"
#include "engines/wintermute/coll_templ.h"

namespace Wintermute {

class AdAttach3DX;
class AdPath;
class AdPath3D;

class AdActor3DX : public AdObject3D {
public:
	PartEmitter *createParticleEmitter(bool followParent = false, int offsetX = 0, int offsetY = 0) override;
	virtual PartEmitter *createParticleEmitter(const char *boneName, DXVector3 offset);
	bool updatePartEmitter() override;
	Common::String _partBone;
	DXVector3 _partOffset;

	bool displayShadowVolume();
	bool restoreDeviceObjects() override;
	bool invalidateDeviceObjects() override;
	int32 _stateAnimChannel;
	void talk(const char *text, const char *sound = nullptr, uint32 duration = 0, const char *stances = nullptr, TTextAlign align = TAL_CENTER) override;
	int32 getHeight() override;
	bool playAnim3DX(const char *name, bool setState);
	bool playAnim3DX(int channel, const char *name, bool setState);

	uint32 getAnimTransitionTime(char *from, char *to) override;
	BaseArray<BaseAnimationTransitionTime *> _transitionTimes;

	virtual bool renderModel() override;

	uint32 _defaultTransTime;
	uint32 _defaultStopTransTime;

	float _afterWalkAngle;
	Common::String _talkAnimName;
	Common::String _idleAnimName;
	Common::String _walkAnimName;
	Common::String _turnLeftAnimName;
	Common::String _turnRightAnimName;

	int32 _talkAnimChannel;

	TDirectWalkMode _directWalkMode;
	TDirectTurnMode _directTurnMode;
	Common::String _directWalkAnim;
	Common::String _directTurnAnim;
	float _directWalkVelocity;
	float _directTurnVelocity;

	int32 _goToTolerance;

	DECLARE_PERSISTENT(AdActor3DX, AdObject3D)
	bool _turningLeft;

	void initLine3D(DXVector3 startPt, DXVector3 endPt, bool firstStep);
	void getNextStep3D();
	void followPath3D();

	void getNextStep2D();
	void followPath2D();

	void goTo3D(DXVector3 targetPos, float targetAngle = -1.0f);
	void goTo2D(int x, int y, float targetAngle = -1.0f);
	bool turnTo(float angle);
	DXVector3 _targetPoint3D;
	BasePoint *_targetPoint2D;
	float _targetAngle;
	bool display() override;
	bool update() override;
	AdActor3DX(BaseGame *inGame);
	virtual ~AdActor3DX();
	AdPath3D *_path3D;
	AdPath *_path2D;
	bool loadFile(const char *filename);
	bool loadBuffer(byte *buffer, bool complete = true);
	float dirToAngle(TDirection dir);
	TDirection angleToDir(float angle);

	bool updateAttachments();
	bool displayAttachments(bool registerObjects);

	// scripting interface
	ScValue *scGetProperty(const Common::String &name) override;
	bool scSetProperty(const char *name, ScValue *value) override;
	bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) override;
	const char *scToString() override;

private:
	bool parseEffect(byte *buffer);
	BaseArray<AdAttach3DX *> _attachments;
	bool turnToStep(float velocity);
	bool prepareTurn(float targetAngle);
	bool mergeAnimations(const char *filename);
	bool mergeAnimations2(const char *filename);
	bool unloadAnimation(const char *animName);
	bool isGoToNeeded(int x, int y);
};

} // namespace Wintermute

#endif
