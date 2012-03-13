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
#include "engines/wintermute/PartEmitter.h"
#include "engines/wintermute/Vector2.h"
#include "engines/wintermute/Matrix4.h"
#include "engines/wintermute/scriptables/ScValue.h"
#include "engines/wintermute/scriptables/ScStack.h"
#include "engines/wintermute/BGame.h"
#include "engines/wintermute/BRegion.h"
#include "engines/wintermute/BFileManager.h"
#include "engines/wintermute/utils.h"
#include "engines/wintermute/PlatformSDL.h"
#include "common/str.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CPartEmitter, false)

//////////////////////////////////////////////////////////////////////////
CPartEmitter::CPartEmitter(CBGame *inGame, CBScriptHolder *Owner) : CBObject(inGame) {
	m_Width = m_Height = 0;

	CBPlatform::SetRectEmpty(&m_Border);
	m_BorderThicknessLeft = m_BorderThicknessRight = m_BorderThicknessTop = m_BorderThicknessBottom = 0;

	m_Angle1 = m_Angle2 = 0;

	m_Velocity1 = m_Velocity2 = 0.0f;
	m_VelocityZBased = false;

	m_Scale1 = m_Scale2 = 100.0f;
	m_ScaleZBased = false;

	m_MaxParticles = 100;

	m_LifeTime1 = m_LifeTime2 = 1000;
	m_LifeTimeZBased = false;

	m_LastGenTime = 0;
	m_GenInterval = 0;
	m_GenAmount = 1;

	m_OverheadTime = 0;
	m_Running = false;

	m_MaxBatches = 0;
	m_BatchesGenerated = 0;

	m_FadeInTime = m_FadeOutTime = 0;

	m_Alpha1 = m_Alpha2 = 255;
	m_AlphaTimeBased = false;

	m_Rotation1 = m_Rotation2 = 0.0f;
	m_AngVelocity1 = m_AngVelocity2 = 0.0f;

	m_GrowthRate1 = m_GrowthRate2 = 0.0f;
	m_ExponentialGrowth = false;

	m_UseRegion = false;

	m_EmitEvent = NULL;
	m_Owner = Owner;
}


//////////////////////////////////////////////////////////////////////////
CPartEmitter::~CPartEmitter(void) {
	for (int i = 0; i < m_Particles.GetSize(); i++) {
		delete m_Particles[i];
	}
	m_Particles.RemoveAll();

	for (int i = 0; i < m_Forces.GetSize(); i++) {
		delete m_Forces[i];
	}
	m_Forces.RemoveAll();


	for (int i = 0; i < m_Sprites.GetSize(); i++) {
		delete [] m_Sprites[i];
	}
	m_Sprites.RemoveAll();

	delete[] m_EmitEvent;
	m_EmitEvent = NULL;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CPartEmitter::AddSprite(char *Filename) {
	if (!Filename) return E_FAIL;

	// do we already have the file?
	for (int i = 0; i < m_Sprites.GetSize(); i++) {
		if (scumm_stricmp(Filename, m_Sprites[i]) == 0) return S_OK;
	}

	// check if file exists
	CBFile *File = Game->m_FileManager->OpenFile(Filename);
	if (!File) {
		Game->LOG(0, "Sprite '%s' not found", Filename);
		return E_FAIL;
	} else Game->m_FileManager->CloseFile(File);

	char *Str = new char[strlen(Filename) + 1];
	strcpy(Str, Filename);
	m_Sprites.Add(Str);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CPartEmitter::RemoveSprite(char *Filename) {
	for (int i = 0; i < m_Sprites.GetSize(); i++) {
		if (scumm_stricmp(Filename, m_Sprites[i]) == 0) {
			delete [] m_Sprites[i];
			m_Sprites.RemoveAt(i);
			return S_OK;
		}
	}
	return E_FAIL;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CPartEmitter::InitParticle(CPartParticle *Particle, uint32 CurrentTime, uint32 TimerDelta) {
	if (!Particle) return E_FAIL;
	if (m_Sprites.GetSize() == 0) return E_FAIL;

	int PosX = CBUtils::RandomInt(m_PosX, m_PosX + m_Width);
	int PosY = CBUtils::RandomInt(m_PosY, m_PosY + m_Height);
	float PosZ = CBUtils::RandomFloat(0.0f, 100.0f);

	float Velocity;
	if (m_VelocityZBased) Velocity = m_Velocity1 + PosZ * (m_Velocity2 - m_Velocity1) / 100;
	else Velocity = CBUtils::RandomFloat(m_Velocity1, m_Velocity2);

	float Scale;
	if (m_ScaleZBased) Scale = m_Scale1 + PosZ * (m_Scale2 - m_Scale1) / 100;
	else Scale = CBUtils::RandomFloat(m_Scale1, m_Scale2);

	int LifeTime;
	if (m_LifeTimeZBased) LifeTime = m_LifeTime2 - PosZ * (m_LifeTime2 - m_LifeTime1) / 100;
	else LifeTime = CBUtils::RandomInt(m_LifeTime1, m_LifeTime2);

	float Angle = CBUtils::RandomAngle(m_Angle1, m_Angle2);
	int SpriteIndex = CBUtils::RandomInt(0, m_Sprites.GetSize() - 1);

	float Rotation = CBUtils::RandomAngle(m_Rotation1, m_Rotation2);
	float AngVelocity = CBUtils::RandomFloat(m_AngVelocity1, m_AngVelocity2);
	float GrowthRate = CBUtils::RandomFloat(m_GrowthRate1, m_GrowthRate2);

	if (!CBPlatform::IsRectEmpty(&m_Border)) {
		int ThicknessLeft   = m_BorderThicknessLeft   - (float)m_BorderThicknessLeft   * PosZ / 100.0f;
		int ThicknessRight  = m_BorderThicknessRight  - (float)m_BorderThicknessRight  * PosZ / 100.0f;
		int ThicknessTop    = m_BorderThicknessTop    - (float)m_BorderThicknessTop    * PosZ / 100.0f;
		int ThicknessBottom = m_BorderThicknessBottom - (float)m_BorderThicknessBottom * PosZ / 100.0f;

		Particle->m_Border = m_Border;
		Particle->m_Border.left += ThicknessLeft;
		Particle->m_Border.right -= ThicknessRight;
		Particle->m_Border.top += ThicknessTop;
		Particle->m_Border.bottom -= ThicknessBottom;
	}

	Vector2 VecPos((float)PosX, (float)PosY);
	Vector2 VecVel(0, Velocity);

	Matrix4 MatRot;
	MatRot.RotationZ(DegToRad(CBUtils::NormalizeAngle(Angle - 180)));
	MatRot.TransformVector2(VecVel);

	if (m_AlphaTimeBased) {
		Particle->m_Alpha1 = m_Alpha1;
		Particle->m_Alpha2 = m_Alpha2;
	} else {
		int Alpha = CBUtils::RandomInt(m_Alpha1, m_Alpha2);
		Particle->m_Alpha1 = Alpha;
		Particle->m_Alpha2 = Alpha;
	}

	Particle->m_CreationTime = CurrentTime;
	Particle->m_Pos = VecPos;
	Particle->m_PosZ = PosZ;
	Particle->m_Velocity = VecVel;
	Particle->m_Scale = Scale;
	Particle->m_LifeTime = LifeTime;
	Particle->m_Rotation = Rotation;
	Particle->m_AngVelocity = AngVelocity;
	Particle->m_GrowthRate = GrowthRate;
	Particle->m_ExponentialGrowth = m_ExponentialGrowth;
	Particle->m_IsDead = FAILED(Particle->SetSprite(m_Sprites[SpriteIndex]));
	Particle->FadeIn(CurrentTime, m_FadeInTime);


	if (Particle->m_IsDead) return E_FAIL;
	else return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CPartEmitter::Update() {
	if (!m_Running) return S_OK;
	else return UpdateInternal(Game->m_Timer, Game->m_TimerDelta);
}

//////////////////////////////////////////////////////////////////////////
HRESULT CPartEmitter::UpdateInternal(uint32 CurrentTime, uint32 TimerDelta) {
	int NumLive = 0;

	for (int i = 0; i < m_Particles.GetSize(); i++) {
		m_Particles[i]->Update(this, CurrentTime, TimerDelta);

		if (!m_Particles[i]->m_IsDead) NumLive++;
	}


	// we're understaffed
	if (NumLive < m_MaxParticles) {
		bool NeedsSort = false;
		if (CurrentTime - m_LastGenTime > m_GenInterval) {
			m_LastGenTime = CurrentTime;
			m_BatchesGenerated++;

			if (m_MaxBatches > 0 && m_BatchesGenerated > m_MaxBatches) {
				return S_OK;
			}

			int ToGen = std::min(m_GenAmount, m_MaxParticles - NumLive);
			while (ToGen > 0) {
				int FirstDeadIndex = -1;
				for (int i = 0; i < m_Particles.GetSize(); i++) {
					if (m_Particles[i]->m_IsDead) {
						FirstDeadIndex = i;
						break;
					}
				}

				CPartParticle *Particle;
				if (FirstDeadIndex >= 0) Particle = m_Particles[FirstDeadIndex];
				else {
					Particle = new CPartParticle(Game);
					m_Particles.Add(Particle);
				}
				InitParticle(Particle, CurrentTime, TimerDelta);
				NeedsSort = true;

				ToGen--;
			}
		}
		if (NeedsSort && (m_ScaleZBased || m_VelocityZBased || m_LifeTimeZBased))
			SortParticlesByZ();

		// we actually generated some particles and we're not in fast-forward mode
		if (NeedsSort && m_OverheadTime == 0) {
			if (m_Owner && m_EmitEvent) m_Owner->ApplyEvent(m_EmitEvent);
		}
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CPartEmitter::Display(CBRegion *Region) {
	if (m_Sprites.GetSize() <= 1) Game->m_Renderer->StartSpriteBatch();

	for (int i = 0; i < m_Particles.GetSize(); i++) {
		if (Region != NULL && m_UseRegion) {
			if (!Region->PointInRegion(m_Particles[i]->m_Pos.x, m_Particles[i]->m_Pos.y)) continue;
		}

		m_Particles[i]->Display(this);
	}

	if (m_Sprites.GetSize() <= 1) Game->m_Renderer->EndSpriteBatch();

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CPartEmitter::Start() {
	for (int i = 0; i < m_Particles.GetSize(); i++) {
		m_Particles[i]->m_IsDead = true;
	}
	m_Running = true;
	m_BatchesGenerated = 0;


	if (m_OverheadTime > 0) {
		uint32 Delta = 500;
		int Steps = m_OverheadTime / Delta;
		uint32 CurrentTime = Game->m_Timer - m_OverheadTime;

		for (int i = 0; i < Steps; i++) {
			UpdateInternal(CurrentTime, Delta);
			CurrentTime += Delta;
		}
		m_OverheadTime = 0;
	}


	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CPartEmitter::SortParticlesByZ() {
	// sort particles by m_PosY
	qsort(m_Particles.GetData(), m_Particles.GetSize(), sizeof(CPartParticle *), CPartEmitter::CompareZ);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
int CPartEmitter::CompareZ(const void *Obj1, const void *Obj2) {
	CPartParticle *P1 = *(CPartParticle **)Obj1;
	CPartParticle *P2 = *(CPartParticle **)Obj2;

	if (P1->m_PosZ < P2->m_PosZ) return -1;
	else if (P1->m_PosZ > P2->m_PosZ) return 1;
	else return 0;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CPartEmitter::SetBorder(int X, int Y, int Width, int Height) {
	CBPlatform::SetRect(&m_Border, X, Y, X + Width, Y + Height);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CPartEmitter::SetBorderThickness(int ThicknessLeft, int ThicknessRight, int ThicknessTop, int ThicknessBottom) {
	m_BorderThicknessLeft = ThicknessLeft;
	m_BorderThicknessRight = ThicknessRight;
	m_BorderThicknessTop = ThicknessTop;
	m_BorderThicknessBottom = ThicknessBottom;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
CPartForce *CPartEmitter::AddForceByName(char *Name) {
	CPartForce *Force = NULL;

	for (int i = 0; i < m_Forces.GetSize(); i++) {
		if (scumm_stricmp(Name, m_Forces[i]->m_Name) == 0) {
			Force = m_Forces[i];
			break;
		}
	}
	if (!Force) {
		Force = new CPartForce(Game);
		if (Force) {
			Force->SetName(Name);
			m_Forces.Add(Force);
		}
	}
	return Force;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CPartEmitter::AddForce(char *Name, CPartForce::TForceType Type, int PosX, int PosY, float Angle, float Strength) {
	CPartForce *Force = AddForceByName(Name);
	if (!Force) return E_FAIL;

	Force->m_Type = Type;
	Force->m_Pos = Vector2(PosX, PosY);

	Force->m_Direction = Vector2(0, Strength);
	Matrix4 MatRot;
	MatRot.RotationZ(DegToRad(CBUtils::NormalizeAngle(Angle - 180)));
	MatRot.TransformVector2(Force->m_Direction);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CPartEmitter::RemoveForce(char *Name) {
	for (int i = 0; i < m_Forces.GetSize(); i++) {
		if (scumm_stricmp(Name, m_Forces[i]->m_Name) == 0) {
			delete m_Forces[i];
			m_Forces.RemoveAt(i);
			return S_OK;
		}
	}
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CPartEmitter::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// SetBorder
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "SetBorder") == 0) {
		Stack->CorrectParams(4);
		int BorderX      = Stack->Pop()->GetInt();
		int BorderY      = Stack->Pop()->GetInt();
		int BorderWidth  = Stack->Pop()->GetInt();
		int BorderHeight = Stack->Pop()->GetInt();

		Stack->PushBool(SUCCEEDED(SetBorder(BorderX, BorderY, BorderWidth, BorderHeight)));

		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// SetBorderThickness
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetBorderThickness") == 0) {
		Stack->CorrectParams(4);
		int Left   = Stack->Pop()->GetInt();
		int Right  = Stack->Pop()->GetInt();
		int Top    = Stack->Pop()->GetInt();
		int Bottom = Stack->Pop()->GetInt();

		Stack->PushBool(SUCCEEDED(SetBorderThickness(Left, Right, Top, Bottom)));

		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// AddSprite
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AddSprite") == 0) {
		Stack->CorrectParams(1);
		char *SpriteFile = Stack->Pop()->GetString();
		Stack->PushBool(SUCCEEDED(AddSprite(SpriteFile)));

		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// RemoveSprite
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "RemoveSprite") == 0) {
		Stack->CorrectParams(1);
		char *SpriteFile = Stack->Pop()->GetString();
		Stack->PushBool(SUCCEEDED(RemoveSprite(SpriteFile)));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Start
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Start") == 0) {
		Stack->CorrectParams(1);
		m_OverheadTime = Stack->Pop()->GetInt();
		Stack->PushBool(SUCCEEDED(Start()));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Stop
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Stop") == 0) {
		Stack->CorrectParams(0);

		for (int i = 0; i < m_Particles.GetSize(); i++) {
			delete m_Particles[i];
		}
		m_Particles.RemoveAll();

		m_Running = false;
		Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Pause
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Pause") == 0) {
		Stack->CorrectParams(0);
		m_Running = false;
		Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Resume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Resume") == 0) {
		Stack->CorrectParams(0);
		m_Running = true;
		Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddGlobalForce
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AddGlobalForce") == 0) {
		Stack->CorrectParams(3);
		char *Name = Stack->Pop()->GetString();
		float Angle = Stack->Pop()->GetFloat();
		float Strength = Stack->Pop()->GetFloat();

		Stack->PushBool(SUCCEEDED(AddForce(Name, CPartForce::FORCE_GLOBAL, 0, 0, Angle, Strength)));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddPointForce
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AddPointForce") == 0) {
		Stack->CorrectParams(5);
		char *Name = Stack->Pop()->GetString();
		int PosX = Stack->Pop()->GetInt();
		int PosY = Stack->Pop()->GetInt();
		float Angle = Stack->Pop()->GetFloat();
		float Strength = Stack->Pop()->GetFloat();

		Stack->PushBool(SUCCEEDED(AddForce(Name, CPartForce::FORCE_GLOBAL, PosX, PosY, Angle, Strength)));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RemoveForce
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "RemoveForce") == 0) {
		Stack->CorrectParams(1);
		char *Name = Stack->Pop()->GetString();

		Stack->PushBool(SUCCEEDED(RemoveForce(Name)));

		return S_OK;
	}

	else return CBObject::ScCallMethod(Script, Stack, ThisStack, Name);
}

//////////////////////////////////////////////////////////////////////////
CScValue *CPartEmitter::ScGetProperty(char *Name) {
	m_ScValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		m_ScValue->SetString("particle-emitter");
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// X
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "X") == 0) {
		m_ScValue->SetInt(m_PosX);
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Y
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Y") == 0) {
		m_ScValue->SetInt(m_PosY);
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Width
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Width") == 0) {
		m_ScValue->SetInt(m_Width);
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Height
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Height") == 0) {
		m_ScValue->SetInt(m_Height);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Scale1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Scale1") == 0) {
		m_ScValue->SetFloat(m_Scale1);
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Scale2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Scale2") == 0) {
		m_ScValue->SetFloat(m_Scale2);
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// ScaleZBased
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ScaleZBased") == 0) {
		m_ScValue->SetBool(m_ScaleZBased);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Velocity1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Velocity1") == 0) {
		m_ScValue->SetFloat(m_Velocity1);
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Velocity2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Velocity2") == 0) {
		m_ScValue->SetFloat(m_Velocity2);
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// VelocityZBased
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "VelocityZBased") == 0) {
		m_ScValue->SetBool(m_VelocityZBased);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// LifeTime1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "LifeTime1") == 0) {
		m_ScValue->SetInt(m_LifeTime1);
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// LifeTime2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "LifeTime2") == 0) {
		m_ScValue->SetInt(m_LifeTime2);
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// LifeTimeZBased
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "LifeTimeZBased") == 0) {
		m_ScValue->SetBool(m_LifeTimeZBased);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Angle1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Angle1") == 0) {
		m_ScValue->SetInt(m_Angle1);
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Angle2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Angle2") == 0) {
		m_ScValue->SetInt(m_Angle2);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AngVelocity1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AngVelocity1") == 0) {
		m_ScValue->SetFloat(m_AngVelocity1);
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// AngVelocity2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AngVelocity2") == 0) {
		m_ScValue->SetFloat(m_AngVelocity2);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Rotation1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Rotation1") == 0) {
		m_ScValue->SetFloat(m_Rotation1);
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Rotation2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Rotation2") == 0) {
		m_ScValue->SetFloat(m_Rotation2);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Alpha1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Alpha1") == 0) {
		m_ScValue->SetInt(m_Alpha1);
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Alpha2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Alpha2") == 0) {
		m_ScValue->SetInt(m_Alpha2);
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// AlphaTimeBased
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AlphaTimeBased") == 0) {
		m_ScValue->SetBool(m_AlphaTimeBased);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MaxParticles
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MaxParticles") == 0) {
		m_ScValue->SetInt(m_MaxParticles);
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// NumLiveParticles (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "NumLiveParticles") == 0) {
		int NumAlive = 0;
		for (int i = 0; i < m_Particles.GetSize(); i++) {
			if (m_Particles[i] && !m_Particles[i]->m_IsDead) NumAlive++;
		}
		m_ScValue->SetInt(NumAlive);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// GenerationInterval
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GenerationInterval") == 0) {
		m_ScValue->SetInt(m_GenInterval);
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// GenerationAmount
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GenerationAmount") == 0) {
		m_ScValue->SetInt(m_GenAmount);
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// MaxBatches
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MaxBatches") == 0) {
		m_ScValue->SetInt(m_MaxBatches);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// FadeInTime
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "FadeInTime") == 0) {
		m_ScValue->SetInt(m_FadeInTime);
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// FadeOutTime
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "FadeOutTime") == 0) {
		m_ScValue->SetInt(m_FadeOutTime);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// GrowthRate1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GrowthRate1") == 0) {
		m_ScValue->SetFloat(m_GrowthRate1);
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// GrowthRate2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GrowthRate2") == 0) {
		m_ScValue->SetFloat(m_GrowthRate2);
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// ExponentialGrowth
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ExponentialGrowth") == 0) {
		m_ScValue->SetBool(m_ExponentialGrowth);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// UseRegion
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "UseRegion") == 0) {
		m_ScValue->SetBool(m_UseRegion);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// EmitEvent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "EmitEvent") == 0) {
		if (!m_EmitEvent) m_ScValue->SetNULL();
		else m_ScValue->SetString(m_EmitEvent);
		return m_ScValue;
	}

	else return CBObject::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CPartEmitter::ScSetProperty(char *Name, CScValue *Value) {
	//////////////////////////////////////////////////////////////////////////
	// X
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "X") == 0) {
		m_PosX = Value->GetInt();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// Y
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Y") == 0) {
		m_PosY = Value->GetInt();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// Width
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Width") == 0) {
		m_Width = Value->GetInt();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// Height
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Height") == 0) {
		m_Height = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Scale1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Scale1") == 0) {
		m_Scale1 = Value->GetFloat();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// Scale2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Scale2") == 0) {
		m_Scale2 = Value->GetFloat();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// ScaleZBased
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ScaleZBased") == 0) {
		m_ScaleZBased = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Velocity1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Velocity1") == 0) {
		m_Velocity1 = Value->GetFloat();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// Velocity2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Velocity2") == 0) {
		m_Velocity2 = Value->GetFloat();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// VelocityZBased
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "VelocityZBased") == 0) {
		m_VelocityZBased = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LifeTime1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "LifeTime1") == 0) {
		m_LifeTime1 = Value->GetInt();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// LifeTime2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "LifeTime2") == 0) {
		m_LifeTime2 = Value->GetInt();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// LifeTimeZBased
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "LifeTimeZBased") == 0) {
		m_LifeTimeZBased = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Angle1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Angle1") == 0) {
		m_Angle1 = Value->GetInt();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// Angle2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Angle2") == 0) {
		m_Angle2 = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AngVelocity1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AngVelocity1") == 0) {
		m_AngVelocity1 = Value->GetFloat();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// AngVelocity2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AngVelocity2") == 0) {
		m_AngVelocity2 = Value->GetFloat();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Rotation1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Rotation1") == 0) {
		m_Rotation1 = Value->GetFloat();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// Rotation2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Rotation2") == 0) {
		m_Rotation2 = Value->GetFloat();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Alpha1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Alpha1") == 0) {
		m_Alpha1 = Value->GetInt();
		if (m_Alpha1 < 0) m_Alpha1 = 0;
		if (m_Alpha1 > 255) m_Alpha1 = 255;
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// Alpha2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Alpha2") == 0) {
		m_Alpha2 = Value->GetInt();
		if (m_Alpha2 < 0) m_Alpha2 = 0;
		if (m_Alpha2 > 255) m_Alpha2 = 255;
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// AlphaTimeBased
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AlphaTimeBased") == 0) {
		m_AlphaTimeBased = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MaxParticles
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MaxParticles") == 0) {
		m_MaxParticles = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GenerationInterval
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GenerationInterval") == 0) {
		m_GenInterval = Value->GetInt();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GenerationAmount
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GenerationAmount") == 0) {
		m_GenAmount = Value->GetInt();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// MaxBatches
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MaxBatches") == 0) {
		m_MaxBatches = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// FadeInTime
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "FadeInTime") == 0) {
		m_FadeInTime = Value->GetInt();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// FadeOutTime
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "FadeOutTime") == 0) {
		m_FadeOutTime = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GrowthRate1
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GrowthRate1") == 0) {
		m_GrowthRate1 = Value->GetFloat();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GrowthRate2
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GrowthRate2") == 0) {
		m_GrowthRate2 = Value->GetFloat();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// ExponentialGrowth
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ExponentialGrowth") == 0) {
		m_ExponentialGrowth = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// UseRegion
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "UseRegion") == 0) {
		m_UseRegion = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// EmitEvent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "EmitEvent") == 0) {
		SAFE_DELETE_ARRAY(m_EmitEvent);
		if (!Value->IsNULL()) CBUtils::SetString(&m_EmitEvent, Value->GetString());
		return S_OK;
	}

	else return CBObject::ScSetProperty(Name, Value);
}


//////////////////////////////////////////////////////////////////////////
char *CPartEmitter::ScToString() {
	return "[particle emitter]";
}




//////////////////////////////////////////////////////////////////////////
HRESULT CPartEmitter::Persist(CBPersistMgr *PersistMgr) {
	CBObject::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(m_Width));
	PersistMgr->Transfer(TMEMBER(m_Height));

	PersistMgr->Transfer(TMEMBER(m_Angle1));
	PersistMgr->Transfer(TMEMBER(m_Angle2));

	PersistMgr->Transfer(TMEMBER(m_Velocity1));
	PersistMgr->Transfer(TMEMBER(m_Velocity2));
	PersistMgr->Transfer(TMEMBER(m_VelocityZBased));

	PersistMgr->Transfer(TMEMBER(m_Scale1));
	PersistMgr->Transfer(TMEMBER(m_Scale2));
	PersistMgr->Transfer(TMEMBER(m_ScaleZBased));

	PersistMgr->Transfer(TMEMBER(m_MaxParticles));

	PersistMgr->Transfer(TMEMBER(m_LifeTime1));
	PersistMgr->Transfer(TMEMBER(m_LifeTime2));
	PersistMgr->Transfer(TMEMBER(m_LifeTimeZBased));

	PersistMgr->Transfer(TMEMBER(m_GenInterval));
	PersistMgr->Transfer(TMEMBER(m_GenAmount));

	PersistMgr->Transfer(TMEMBER(m_Running));
	PersistMgr->Transfer(TMEMBER(m_OverheadTime));

	PersistMgr->Transfer(TMEMBER(m_Border));
	PersistMgr->Transfer(TMEMBER(m_BorderThicknessLeft));
	PersistMgr->Transfer(TMEMBER(m_BorderThicknessRight));
	PersistMgr->Transfer(TMEMBER(m_BorderThicknessTop));
	PersistMgr->Transfer(TMEMBER(m_BorderThicknessBottom));

	PersistMgr->Transfer(TMEMBER(m_FadeInTime));
	PersistMgr->Transfer(TMEMBER(m_FadeOutTime));

	PersistMgr->Transfer(TMEMBER(m_Alpha1));
	PersistMgr->Transfer(TMEMBER(m_Alpha2));
	PersistMgr->Transfer(TMEMBER(m_AlphaTimeBased));

	PersistMgr->Transfer(TMEMBER(m_AngVelocity1));
	PersistMgr->Transfer(TMEMBER(m_AngVelocity2));

	PersistMgr->Transfer(TMEMBER(m_Rotation1));
	PersistMgr->Transfer(TMEMBER(m_Rotation2));

	PersistMgr->Transfer(TMEMBER(m_GrowthRate1));
	PersistMgr->Transfer(TMEMBER(m_GrowthRate2));
	PersistMgr->Transfer(TMEMBER(m_ExponentialGrowth));

	PersistMgr->Transfer(TMEMBER(m_UseRegion));

	PersistMgr->Transfer(TMEMBER_INT(m_MaxBatches));
	PersistMgr->Transfer(TMEMBER_INT(m_BatchesGenerated));

	PersistMgr->Transfer(TMEMBER(m_EmitEvent));
	PersistMgr->Transfer(TMEMBER(m_Owner));


	m_Sprites.Persist(PersistMgr);

	int NumForces;
	if (PersistMgr->m_Saving) {
		NumForces = m_Forces.GetSize();
		PersistMgr->Transfer(TMEMBER(NumForces));
		for (int i = 0; i < m_Forces.GetSize(); i++) {
			m_Forces[i]->Persist(PersistMgr);
		}
	} else {
		PersistMgr->Transfer(TMEMBER(NumForces));
		for (int i = 0; i < NumForces; i++) {
			CPartForce *Force = new CPartForce(Game);
			Force->Persist(PersistMgr);
			m_Forces.Add(Force);
		}
	}

	int NumParticles;
	if (PersistMgr->m_Saving) {
		NumParticles = m_Particles.GetSize();
		PersistMgr->Transfer(TMEMBER(NumParticles));
		for (int i = 0; i < m_Particles.GetSize(); i++) {
			m_Particles[i]->Persist(PersistMgr);
		}
	} else {
		PersistMgr->Transfer(TMEMBER(NumParticles));
		for (int i = 0; i < NumParticles; i++) {
			CPartParticle *Particle = new CPartParticle(Game);
			Particle->Persist(PersistMgr);
			m_Particles.Add(Particle);
		}
	}

	return S_OK;
}

} // end of namespace WinterMute
