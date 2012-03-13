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

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/PartParticle.h"
#include "engines/wintermute/PartEmitter.h"
#include "engines/wintermute/BSprite.h"
#include "engines/wintermute/BGame.h"
#include "engines/wintermute/utils.h"
#include "engines/wintermute/PlatformSDL.h"
#include "common/str.h"
#include <math.h>

namespace WinterMute {

//////////////////////////////////////////////////////////////////////////
CPartParticle::CPartParticle(CBGame *inGame) : CBBase(inGame) {
	m_Pos = Vector2(0.0f, 0.0f);
	m_PosZ = 0.0f;
	m_Velocity = Vector2(0.0f, 0.0f);
	m_Scale = 100.0f;
	m_Sprite = NULL;
	m_CreationTime = 0;
	m_LifeTime = 0;
	m_IsDead = true;
	CBPlatform::SetRectEmpty(&m_Border);

	m_State = PARTICLE_NORMAL;
	m_FadeStart = 0;
	m_FadeTime = 0;
	m_CurrentAlpha = 255;

	m_Alpha1 = m_Alpha2 = 255;

	m_Rotation = 0.0f;
	m_AngVelocity = 0.0f;

	m_GrowthRate = 0.0f;
	m_ExponentialGrowth = false;
}


//////////////////////////////////////////////////////////////////////////
CPartParticle::~CPartParticle(void) {
	delete m_Sprite;
	m_Sprite = NULL;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CPartParticle::SetSprite(char *Filename) {
	if (m_Sprite && m_Sprite->m_Filename && scumm_stricmp(Filename, m_Sprite->m_Filename) == 0) {
		m_Sprite->Reset();
		return S_OK;
	}

	delete m_Sprite;
	m_Sprite = NULL;

	CSysClassRegistry::GetInstance()->m_Disabled = true;
	m_Sprite = new CBSprite(Game, Game);
	if (m_Sprite && SUCCEEDED(m_Sprite->LoadFile(Filename))) {
		CSysClassRegistry::GetInstance()->m_Disabled = false;
		return S_OK;
	} else {
		delete m_Sprite;
		m_Sprite = NULL;
		CSysClassRegistry::GetInstance()->m_Disabled = false;
		return E_FAIL;
	}

}

//////////////////////////////////////////////////////////////////////////
HRESULT CPartParticle::Update(CPartEmitter *Emitter, uint32 CurrentTime, uint32 TimerDelta) {
	if (m_State == PARTICLE_FADEIN) {
		if (CurrentTime - m_FadeStart >= m_FadeTime) {
			m_State = PARTICLE_NORMAL;
			m_CurrentAlpha = m_Alpha1;
		} else m_CurrentAlpha = ((float)CurrentTime - (float)m_FadeStart) / (float)m_FadeTime * m_Alpha1;

		return S_OK;
	} else if (m_State == PARTICLE_FADEOUT) {
		if (CurrentTime - m_FadeStart >= m_FadeTime) {
			m_IsDead = true;
			return S_OK;
		} else m_CurrentAlpha = m_FadeStartAlpha - ((float)CurrentTime - (float)m_FadeStart) / (float)m_FadeTime * m_FadeStartAlpha;

		return S_OK;
	} else {
		// time is up
		if (m_LifeTime > 0) {
			if (CurrentTime - m_CreationTime >= m_LifeTime) {
				if (Emitter->m_FadeOutTime > 0)
					FadeOut(CurrentTime, Emitter->m_FadeOutTime);
				else
					m_IsDead = true;
			}
		}

		// particle hit the border
		if (!m_IsDead && !CBPlatform::IsRectEmpty(&m_Border)) {
			POINT p;
			p.x = m_Pos.x;
			p.y = m_Pos.y;
			if (!CBPlatform::PtInRect(&m_Border, p)) FadeOut(CurrentTime, Emitter->m_FadeOutTime);
		}
		if (m_State != PARTICLE_NORMAL) return S_OK;

		// update alpha
		if (m_LifeTime > 0) {
			int Age = CurrentTime - m_CreationTime;
			int AlphaDelta = m_Alpha2 - m_Alpha1;

			m_CurrentAlpha = m_Alpha1 + ((float)AlphaDelta / (float)m_LifeTime * (float)Age);
		}

		// update position
		float ElapsedTime = (float)TimerDelta / 1000.f;

		for (int i = 0; i < Emitter->m_Forces.GetSize(); i++) {
			CPartForce *Force = Emitter->m_Forces[i];
			switch (Force->m_Type) {
			case CPartForce::FORCE_GLOBAL:
				m_Velocity += Force->m_Direction * ElapsedTime;
				break;

			case CPartForce::FORCE_POINT: {
				Vector2 VecDist = Force->m_Pos - m_Pos;
				float Dist = fabs(VecDist.Length());

				Dist = 100.0f / Dist;

				m_Velocity += Force->m_Direction * Dist * ElapsedTime;
			}
			break;
			}
		}
		m_Pos += m_Velocity * ElapsedTime;

		// update rotation
		m_Rotation += m_AngVelocity * ElapsedTime;
		m_Rotation = CBUtils::NormalizeAngle(m_Rotation);

		// update scale
		if (m_ExponentialGrowth)
			m_Scale += m_Scale / 100.0f * m_GrowthRate * ElapsedTime;
		else
			m_Scale += m_GrowthRate * ElapsedTime;

		if (m_Scale <= 0.0f) m_IsDead = true;


		return S_OK;
	}
}

//////////////////////////////////////////////////////////////////////////
HRESULT CPartParticle::Display(CPartEmitter *Emitter) {
	if (!m_Sprite) return E_FAIL;
	if (m_IsDead) return S_OK;

	m_Sprite->GetCurrentFrame();
	return m_Sprite->Display(m_Pos.x, m_Pos.y,
	                         NULL,
	                         m_Scale, m_Scale,
	                         DRGBA(255, 255, 255, m_CurrentAlpha),
	                         m_Rotation,
	                         Emitter->m_BlendMode);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CPartParticle::FadeIn(uint32 CurrentTime, int FadeTime) {
	m_CurrentAlpha = 0;
	m_FadeStart = CurrentTime;
	m_FadeTime = FadeTime;
	m_State = PARTICLE_FADEIN;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CPartParticle::FadeOut(uint32 CurrentTime, int FadeTime) {
	//m_CurrentAlpha = 255;
	m_FadeStartAlpha = m_CurrentAlpha;
	m_FadeStart = CurrentTime;
	m_FadeTime = FadeTime;
	m_State = PARTICLE_FADEOUT;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CPartParticle::Persist(CBPersistMgr *PersistMgr) {
	PersistMgr->Transfer(TMEMBER(m_Alpha1));
	PersistMgr->Transfer(TMEMBER(m_Alpha2));
	PersistMgr->Transfer(TMEMBER(m_Border));
	PersistMgr->Transfer(TMEMBER(m_Pos));
	PersistMgr->Transfer(TMEMBER(m_PosZ));
	PersistMgr->Transfer(TMEMBER(m_Velocity));
	PersistMgr->Transfer(TMEMBER(m_Scale));
	PersistMgr->Transfer(TMEMBER(m_CreationTime));
	PersistMgr->Transfer(TMEMBER(m_LifeTime));
	PersistMgr->Transfer(TMEMBER(m_IsDead));
	PersistMgr->Transfer(TMEMBER_INT(m_State));
	PersistMgr->Transfer(TMEMBER(m_FadeStart));
	PersistMgr->Transfer(TMEMBER(m_FadeTime));
	PersistMgr->Transfer(TMEMBER(m_CurrentAlpha));
	PersistMgr->Transfer(TMEMBER(m_AngVelocity));
	PersistMgr->Transfer(TMEMBER(m_Rotation));
	PersistMgr->Transfer(TMEMBER(m_GrowthRate));
	PersistMgr->Transfer(TMEMBER(m_ExponentialGrowth));
	PersistMgr->Transfer(TMEMBER(m_FadeStartAlpha));

	if (PersistMgr->m_Saving) {
		PersistMgr->Transfer(TMEMBER(m_Sprite->m_Filename));
	} else {
		char *Filename;
		PersistMgr->Transfer(TMEMBER(Filename));
		CSysClassRegistry::GetInstance()->m_Disabled = true;
		SetSprite(Filename);
		CSysClassRegistry::GetInstance()->m_Disabled = false;
		delete[] Filename;
		Filename = NULL;
	}

	return S_OK;
}

} // end of namespace WinterMute
