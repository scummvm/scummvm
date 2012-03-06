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

	int m_Width;
	int m_Height;

	int m_Angle1;
	int m_Angle2;

	float m_Rotation1;
	float m_Rotation2;

	float m_AngVelocity1;
	float m_AngVelocity2;

	float m_GrowthRate1;
	float m_GrowthRate2;
	bool m_ExponentialGrowth;

	float m_Velocity1;
	float m_Velocity2;
	bool m_VelocityZBased;

	float m_Scale1;
	float m_Scale2;
	bool m_ScaleZBased;

	int m_MaxParticles;

	int m_LifeTime1;
	int m_LifeTime2;
	bool m_LifeTimeZBased;

	int m_GenInterval;
	int m_GenAmount;

	bool m_Running;
	int m_OverheadTime;

	int m_MaxBatches;
	int m_BatchesGenerated;

	RECT m_Border;
	int m_BorderThicknessLeft;
	int m_BorderThicknessRight;
	int m_BorderThicknessTop;
	int m_BorderThicknessBottom;

	int m_FadeInTime;
	int m_FadeOutTime;

	int m_Alpha1;
	int m_Alpha2;
	bool m_AlphaTimeBased;

	bool m_UseRegion;

	char *m_EmitEvent;
	CBScriptHolder *m_Owner;

	HRESULT Start();

	HRESULT Update();
	HRESULT Display(CBRegion *Region = NULL);

	HRESULT SortParticlesByZ();
	HRESULT AddSprite(char *Filename);
	HRESULT RemoveSprite(char *Filename);
	HRESULT SetBorder(int X, int Y, int Width, int Height);
	HRESULT SetBorderThickness(int ThicknessLeft, int ThicknessRight, int ThicknessTop, int ThicknessBottom);

	HRESULT AddForce(char *Name, CPartForce::TForceType Type, int PosX, int PosY, float Angle, float Strength);
	HRESULT RemoveForce(char *Name);

	CBArray<CPartForce *, CPartForce *> m_Forces;

	// scripting interface
	virtual CScValue *ScGetProperty(char *Name);
	virtual HRESULT ScSetProperty(char *Name, CScValue *Value);
	virtual HRESULT ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name);
	virtual char *ScToString();


private:
	CPartForce *AddForceByName(char *Name);
	int static CompareZ(const void *Obj1, const void *Obj2);
	HRESULT InitParticle(CPartParticle *Particle, uint32 CurrentTime, uint32 TimerDelta);
	HRESULT UpdateInternal(uint32 CurrentTime, uint32 TimerDelta);
	uint32 m_LastGenTime;
	CBArray<CPartParticle *, CPartParticle *> m_Particles;
	CBArray<char *, char *> m_Sprites;
};

} // end of namespace WinterMute

#endif
