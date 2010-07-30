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

#ifndef SWORD25_ANIMATIONRESOURCE_H
#define SWORD25_ANIMATIONRESOURCE_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/kernel/resource.h"
#include "sword25/gfx/animationdescription.h"
#include "sword25/gfx/animation.h"

#include "sword25/kernel/memlog_off.h"
#include <vector>
#include "sword25/kernel/memlog_on.h"

// -----------------------------------------------------------------------------
// Forward Declarations
// -----------------------------------------------------------------------------

class BS_Kernel;
class BS_PackageManager;
class TiXmlElement;

// -----------------------------------------------------------------------------
// Class Definition
// -----------------------------------------------------------------------------

class BS_AnimationResource : public BS_Resource, public BS_AnimationDescription
{
public:
	BS_AnimationResource(const std::string & FileName);
	virtual ~BS_AnimationResource();

	virtual const Frame &	GetFrame(unsigned int Index) const { BS_ASSERT(Index < m_Frames.size()); return m_Frames[Index]; }
	virtual unsigned int	GetFrameCount() const { return m_Frames.size(); }
	virtual void			Unlock() { Release(); }

	BS_Animation::ANIMATION_TYPES	GetAnimationType() const { return m_AnimationType; }
	int								GetFPS() const { return m_FPS; }
	int								GetMillisPerFrame() const { return m_MillisPerFrame; }
	bool							IsScalingAllowed() const { return m_ScalingAllowed; }
	bool							IsAlphaAllowed() const { return m_AlphaAllowed; }
	bool							IsColorModulationAllowed() const { return m_ColorModulationAllowed; }
	bool							IsValid() const { return m_Valid; }

private:
	bool							m_Valid;

	std::vector<Frame>				m_Frames;

	//@{
	/** @name Dokument-Parser Methoden */

	bool ParseAnimationTag(TiXmlElement& AnimationTag, int& FPS, BS_Animation::ANIMATION_TYPES & AnimationType);
	bool ParseFrameTag(TiXmlElement& FrameTag, Frame& Frame, BS_PackageManager& PackageManager);

	//@}

	bool ComputeFeatures();
	bool PrecacheAllFrames() const;
};

#endif
