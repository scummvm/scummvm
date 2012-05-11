/*
This file is part of WME Lite.
http://dead-code.org/redir.php?target=wmelite

Copyright (c) 2011 Jan Nedoma

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef __WmePartEmitter_H__
#define __WmePartEmitter_H__


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
