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

#ifndef __WmePartParticle_H__
#define __WmePartParticle_H__


#include "BBase.h"
#include "Vector2.h"

namespace WinterMute {

class CPartEmitter;
class CBSprite;
class CBPersistMgr;

class CPartParticle : public CBBase {
public:
	enum TParticleState {
	    PARTICLE_NORMAL, PARTICLE_FADEIN, PARTICLE_FADEOUT
	};

	CPartParticle(CBGame *inGame);
	virtual ~CPartParticle(void);

	float m_GrowthRate;
	bool m_ExponentialGrowth;

	float m_Rotation;
	float m_AngVelocity;

	int m_Alpha1;
	int m_Alpha2;

	RECT m_Border;
	Vector2 m_Pos;
	float m_PosZ;
	Vector2 m_Velocity;
	float m_Scale;
	CBSprite *m_Sprite;
	uint32 m_CreationTime;
	int m_LifeTime;
	bool m_IsDead;
	TParticleState m_State;

	HRESULT Update(CPartEmitter *Emitter, uint32 CurrentTime, uint32 TimerDelta);
	HRESULT Display(CPartEmitter *Emitter);

	HRESULT SetSprite(char *Filename);

	HRESULT FadeIn(uint32 CurrentTime, int FadeTime);
	HRESULT FadeOut(uint32 CurrentTime, int FadeTime);

	HRESULT Persist(CBPersistMgr *PersistMgr);
private:
	uint32 m_FadeStart;
	int m_FadeTime;
	int m_CurrentAlpha;
	int m_FadeStartAlpha;
};

} // end of namespace WinterMute

#endif
