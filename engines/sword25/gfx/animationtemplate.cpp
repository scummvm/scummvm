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
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#define BS_LOG_PREFIX "ANIMATIONTEMPLATE"

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/kernel.h"
#include "sword25/kernel/resource.h"
#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/kernel/inputpersistenceblock.h"

#include "sword25/gfx/animationresource.h"
#include "sword25/gfx/animationtemplate.h"
#include "sword25/gfx/animationtemplateregistry.h"

namespace Sword25 {

// -----------------------------------------------------------------------------
// Konstruktion / Destruktion
// -----------------------------------------------------------------------------

unsigned int AnimationTemplate::Create(const Common::String &SourceAnimation) {
	AnimationTemplate *AnimationTemplatePtr = new AnimationTemplate(SourceAnimation);

	if (AnimationTemplatePtr->IsValid()) {
		return AnimationTemplateRegistry::GetInstance().ResolvePtr(AnimationTemplatePtr);
	} else {
		delete AnimationTemplatePtr;
		return 0;
	}
}

// -----------------------------------------------------------------------------

unsigned int AnimationTemplate::Create(const AnimationTemplate &Other) {
	AnimationTemplate *AnimationTemplatePtr = new AnimationTemplate(Other);

	if (AnimationTemplatePtr->IsValid()) {
		return AnimationTemplateRegistry::GetInstance().ResolvePtr(AnimationTemplatePtr);
	} else {
		delete AnimationTemplatePtr;
		return 0;
	}
}

// -----------------------------------------------------------------------------

unsigned int AnimationTemplate::Create(InputPersistenceBlock &Reader, unsigned int Handle) {
	AnimationTemplate *AnimationTemplatePtr = new AnimationTemplate(Reader, Handle);

	if (AnimationTemplatePtr->IsValid()) {
		return AnimationTemplateRegistry::GetInstance().ResolvePtr(AnimationTemplatePtr);
	} else {
		delete AnimationTemplatePtr;
		return 0;
	}
}

// -----------------------------------------------------------------------------

AnimationTemplate::AnimationTemplate(const Common::String &SourceAnimation) {
	// Objekt registrieren.
	AnimationTemplateRegistry::GetInstance().RegisterObject(this);

	m_Valid = false;

	// Die Animations-Resource wird für die gesamte Lebensdauer des Objektes gelockt
	m_SourceAnimationPtr = RequestSourceAnimation(SourceAnimation);

	// Erfolg signalisieren
	m_Valid = (m_SourceAnimationPtr != 0);
}

// -----------------------------------------------------------------------------

AnimationTemplate::AnimationTemplate(const AnimationTemplate &Other) {
	// Objekt registrieren.
	AnimationTemplateRegistry::GetInstance().RegisterObject(this);

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

AnimationTemplate::AnimationTemplate(InputPersistenceBlock &Reader, unsigned int Handle) {
	// Objekt registrieren.
	AnimationTemplateRegistry::GetInstance().RegisterObject(this, Handle);

	// Objekt laden.
	m_Valid = Unpersist(Reader);
}

// -----------------------------------------------------------------------------

AnimationResource *AnimationTemplate::RequestSourceAnimation(const Common::String &SourceAnimation) const {
	ResourceManager *RMPtr = Kernel::GetInstance()->GetResourceManager();
	Resource *ResourcePtr;
	if (NULL == (ResourcePtr = RMPtr->RequestResource(SourceAnimation)) || ResourcePtr->GetType() != Resource::TYPE_ANIMATION) {
		BS_LOG_ERRORLN("The resource \"%s\" could not be requested or is has an invalid type. The animation template can't be created.", SourceAnimation.c_str());
		return 0;
	}
	return static_cast<AnimationResource *>(ResourcePtr);
}

// -----------------------------------------------------------------------------

AnimationTemplate::~AnimationTemplate() {
	// Animations-Resource freigeben
	if (m_SourceAnimationPtr) {
		m_SourceAnimationPtr->Release();
	}

	// Objekt deregistrieren
	AnimationTemplateRegistry::GetInstance().DeregisterObject(this);
}

// -----------------------------------------------------------------------------

void AnimationTemplate::AddFrame(int Index) {
	if (ValidateSourceIndex(Index)) {
		m_Frames.push_back(m_SourceAnimationPtr->GetFrame(Index));
	}
}

// -----------------------------------------------------------------------------

void AnimationTemplate::SetFrame(int DestIndex, int SrcIndex) {
	if (ValidateDestIndex(DestIndex) && ValidateSourceIndex(SrcIndex)) {
		m_Frames[DestIndex] = m_SourceAnimationPtr->GetFrame(SrcIndex);
	}
}

// -----------------------------------------------------------------------------

bool AnimationTemplate::ValidateSourceIndex(unsigned int Index) const {
	if (Index > m_SourceAnimationPtr->GetFrameCount()) {
		BS_LOG_WARNINGLN("Tried to insert a frame (\"%d\") that does not exist in the source animation (\"%s\"). Ignoring call.",
		                 Index, m_SourceAnimationPtr->GetFileName().c_str());
		return false;
	} else
		return true;
}

// -----------------------------------------------------------------------------

bool AnimationTemplate::ValidateDestIndex(unsigned int Index) const {
	if (Index > m_Frames.size()) {
		BS_LOG_WARNINGLN("Tried to change a nonexistent frame (\"%d\") in a template animation. Ignoring call.",
		                 Index);
		return false;
	} else
		return true;
}

// -----------------------------------------------------------------------------

void AnimationTemplate::SetFPS(int FPS) {
	m_FPS = FPS;
	m_MillisPerFrame = 1000000 / m_FPS;
}

// -----------------------------------------------------------------------------

bool AnimationTemplate::Persist(OutputPersistenceBlock &Writer) {
	bool Result = true;

	// Parent persistieren.
	Result &= AnimationDescription::Persist(Writer);

	// Frameanzahl schreiben.
	Writer.Write(m_Frames.size());

	// Frames einzeln persistieren.
	Common::Array<const Frame>::const_iterator Iter = m_Frames.begin();
	while (Iter != m_Frames.end()) {
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

bool AnimationTemplate::Unpersist(InputPersistenceBlock &Reader) {
	bool Result = true;

	// Parent wieder herstellen.
	Result &= AnimationDescription::Unpersist(Reader);

	// Frameanzahl lesen.
	unsigned int FrameCount;
	Reader.Read(FrameCount);

	// Frames einzeln wieder herstellen.
	for (unsigned int i = 0; i < FrameCount; ++i) {
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
	Common::String SourceAnimation;
	Reader.Read(SourceAnimation);
	m_SourceAnimationPtr = RequestSourceAnimation(SourceAnimation);

	Reader.Read(m_Valid);

	return m_SourceAnimationPtr && Reader.IsGood() && Result;
}

} // End of namespace Sword25
