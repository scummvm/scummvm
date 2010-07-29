// -----------------------------------------------------------------------------
// This file is part of Broken Sword 2.5
// Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdörfer
//
// Broken Sword 2.5 is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Broken Sword 2.5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Broken Sword 2.5; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
// -----------------------------------------------------------------------------

#define BS_LOG_PREFIX "ANIMATIONTEMPLATE"

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "kernel/kernel.h"
#include "kernel/resource.h"
#include "kernel/outputpersistenceblock.h"
#include "kernel/inputpersistenceblock.h"

#include "animationresource.h"
#include "animationtemplate.h"
#include "animationtemplateregistry.h"

// -----------------------------------------------------------------------------
// Konstruktion / Destruktion
// -----------------------------------------------------------------------------

unsigned int BS_AnimationTemplate::Create(const std::string & SourceAnimation)
{
	BS_AnimationTemplate * AnimationTemplatePtr = new BS_AnimationTemplate(SourceAnimation);

	if (AnimationTemplatePtr->IsValid())
	{
		return BS_AnimationTemplateRegistry::GetInstance().ResolvePtr(AnimationTemplatePtr);
	}
	else
	{
		delete AnimationTemplatePtr;
		return 0;
	}
}

// -----------------------------------------------------------------------------

unsigned int BS_AnimationTemplate::Create(const BS_AnimationTemplate & Other)
{
	BS_AnimationTemplate * AnimationTemplatePtr = new BS_AnimationTemplate(Other);

	if (AnimationTemplatePtr->IsValid())
	{
		return BS_AnimationTemplateRegistry::GetInstance().ResolvePtr(AnimationTemplatePtr);
	}
	else
	{
		delete AnimationTemplatePtr;
		return 0;
	}
}

// -----------------------------------------------------------------------------

unsigned int BS_AnimationTemplate::Create(BS_InputPersistenceBlock & Reader, unsigned int Handle)
{
	BS_AnimationTemplate * AnimationTemplatePtr = new BS_AnimationTemplate(Reader, Handle);

	if (AnimationTemplatePtr->IsValid())
	{
		return BS_AnimationTemplateRegistry::GetInstance().ResolvePtr(AnimationTemplatePtr);
	}
	else
	{
		delete AnimationTemplatePtr;
		return 0;
	}
}

// -----------------------------------------------------------------------------

BS_AnimationTemplate::BS_AnimationTemplate(const std::string & SourceAnimation)
{
	// Objekt registrieren.
	BS_AnimationTemplateRegistry::GetInstance().RegisterObject(this);

	m_Valid = false;

	// Die Animations-Resource wird für die gesamte Lebensdauer des Objektes gelockt
	m_SourceAnimationPtr = RequestSourceAnimation(SourceAnimation);

	// Erfolg signalisieren
	m_Valid = (m_SourceAnimationPtr != 0);
}

// -----------------------------------------------------------------------------

BS_AnimationTemplate::BS_AnimationTemplate(const BS_AnimationTemplate & Other)
{
	// Objekt registrieren.
	BS_AnimationTemplateRegistry::GetInstance().RegisterObject(this);

	m_Valid = false;

	// Die Animations-Resource wird für die gesamte Lebensdauer des Objektes gelockt.
	if (!Other.m_SourceAnimationPtr) return;
	m_SourceAnimationPtr = RequestSourceAnimation(Other.m_SourceAnimationPtr->GetFileName());

	// Alle Member kopieren.
	m_AnimationType = Other.m_AnimationType;
	m_FPS = Other.m_FPS;
	m_MillisPerFrame = Other.m_MillisPerFrame;
	m_ScalingAllowed = Other.m_ScalingAllowed;
	m_AlphaAllowed = Other.m_AlphaAllowed;
	m_ColorModulationAllowed = Other.m_ColorModulationAllowed;
	m_Frames = Other.m_Frames;
	m_SourceAnimationPtr = Other.m_SourceAnimationPtr;
	m_Valid = Other.m_Valid;

	m_Valid &= (m_SourceAnimationPtr != 0);
}

// -----------------------------------------------------------------------------

BS_AnimationTemplate::BS_AnimationTemplate(BS_InputPersistenceBlock & Reader, unsigned int Handle)
{
	// Objekt registrieren.
	BS_AnimationTemplateRegistry::GetInstance().RegisterObject(this, Handle);	

	// Objekt laden.
	m_Valid	= Unpersist(Reader);
}

// -----------------------------------------------------------------------------

BS_AnimationResource * BS_AnimationTemplate::RequestSourceAnimation(const std::string & SourceAnimation) const
{
	BS_ResourceManager * RMPtr = BS_Kernel::GetInstance()->GetResourceManager();
	BS_Resource * ResourcePtr;
	if (NULL == (ResourcePtr = RMPtr->RequestResource(SourceAnimation)) || ResourcePtr->GetType() != BS_Resource::TYPE_ANIMATION)
	{
		BS_LOG_ERRORLN("The resource \"%s\" could not be requested or is has an invalid type. The animation template can't be created.", SourceAnimation.c_str());
		return 0;
	}
	return static_cast<BS_AnimationResource *>(ResourcePtr);
}

// -----------------------------------------------------------------------------

BS_AnimationTemplate::~BS_AnimationTemplate()
{
	// Animations-Resource freigeben
	if (m_SourceAnimationPtr)
	{
		m_SourceAnimationPtr->Release();
	}

	// Objekt deregistrieren
	BS_AnimationTemplateRegistry::GetInstance().DeregisterObject(this);
}

// -----------------------------------------------------------------------------

void BS_AnimationTemplate::AddFrame(int Index)
{
	if (ValidateSourceIndex(Index))
	{
		m_Frames.push_back(m_SourceAnimationPtr->GetFrame(Index));
	}
}

// -----------------------------------------------------------------------------

void BS_AnimationTemplate::SetFrame(int DestIndex, int SrcIndex)
{
	if (ValidateDestIndex(DestIndex) && ValidateSourceIndex(SrcIndex))
	{
		m_Frames[DestIndex] = m_SourceAnimationPtr->GetFrame(SrcIndex);
	}
}

// -----------------------------------------------------------------------------

bool BS_AnimationTemplate::ValidateSourceIndex(unsigned int Index) const
{
	if (Index > m_SourceAnimationPtr->GetFrameCount())
	{
		BS_LOG_WARNINGLN("Tried to insert a frame (\"%d\") that does not exist in the source animation (\"%s\"). Ignoring call.",
						 Index, m_SourceAnimationPtr->GetFileName().c_str());
		return false;
	}
	else
		return true;
}

// -----------------------------------------------------------------------------

bool BS_AnimationTemplate::ValidateDestIndex(unsigned int Index) const
{
	if (Index > m_Frames.size())
	{
		BS_LOG_WARNINGLN("Tried to change a nonexistent frame (\"%d\") in a template animation. Ignoring call.",
						 Index);
		return false;
	}
	else
		return true;
}

// -----------------------------------------------------------------------------

void BS_AnimationTemplate::SetFPS(int FPS)
{
	m_FPS = FPS;
	m_MillisPerFrame = 1000000 / m_FPS;
}

// -----------------------------------------------------------------------------

bool BS_AnimationTemplate::Persist(BS_OutputPersistenceBlock & Writer)
{
	bool Result = true;

	// Parent persistieren.
	Result &= BS_AnimationDescription::Persist(Writer);

	// Frameanzahl schreiben.
	Writer.Write(m_Frames.size());

	// Frames einzeln persistieren.
	std::vector<const Frame>::const_iterator Iter = m_Frames.begin();
	while (Iter != m_Frames.end())
	{
		Writer.Write(Iter->HotspotX);
		Writer.Write(Iter->HotspotY);
		Writer.Write(Iter->FlipV);
		Writer.Write(Iter->FlipH);
		Writer.Write(Iter->FileName);
		Writer.Write(Iter->Action);
		++Iter;
	}

	// Restliche Member persistieren.
	Writer.Write(m_SourceAnimationPtr->GetFileName());
	Writer.Write(m_Valid);

	return Result;
}

// -----------------------------------------------------------------------------

bool BS_AnimationTemplate::Unpersist(BS_InputPersistenceBlock & Reader)
{
	bool Result = true;

	// Parent wieder herstellen.
	Result &= BS_AnimationDescription::Unpersist(Reader);

	// Frameanzahl lesen.
	unsigned int FrameCount;
	Reader.Read(FrameCount);

	// Frames einzeln wieder herstellen.
	for (unsigned int i = 0; i < FrameCount; ++i)
	{
		Frame frame;
		Reader.Read(frame.HotspotX);
		Reader.Read(frame.HotspotY);
		Reader.Read(frame.FlipV);
		Reader.Read(frame.FlipH);
		Reader.Read(frame.FileName);
		Reader.Read(frame.Action);

		m_Frames.push_back(frame);
	}

	// Die Animations-Resource wird für die gesamte Lebensdauer des Objektes gelockt
	std::string SourceAnimation;
	Reader.Read(SourceAnimation);
	m_SourceAnimationPtr = RequestSourceAnimation(SourceAnimation);

	Reader.Read(m_Valid);

	return m_SourceAnimationPtr && Reader.IsGood() && Result;
}
