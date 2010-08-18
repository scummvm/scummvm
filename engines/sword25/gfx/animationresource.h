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

class TiXmlElement;

namespace Sword25 {

// -----------------------------------------------------------------------------
// Forward Declarations
// -----------------------------------------------------------------------------

class BS_Kernel;
class PackageManager;

// -----------------------------------------------------------------------------
// Class Definition
// -----------------------------------------------------------------------------

class AnimationResource : public BS_Resource, public AnimationDescription {
public:
	AnimationResource(const Common::String &FileName);
	virtual ~AnimationResource();

	virtual const Frame    &GetFrame(unsigned int Index) const {
		BS_ASSERT(Index < m_Frames.size());
		return m_Frames[Index];
	}
	virtual unsigned int    GetFrameCount() const {
		return m_Frames.size();
	}
	virtual void            Unlock() {
		Release();
	}

	Animation::ANIMATION_TYPES   GetAnimationType() const {
		return m_AnimationType;
	}
	int                             GetFPS() const {
		return m_FPS;
	}
	int                             GetMillisPerFrame() const {
		return m_MillisPerFrame;
	}
	bool                            IsScalingAllowed() const {
		return m_ScalingAllowed;
	}
	bool                            IsAlphaAllowed() const {
		return m_AlphaAllowed;
	}
	bool                            IsColorModulationAllowed() const {
		return m_ColorModulationAllowed;
	}
	bool                            IsValid() const {
		return m_Valid;
	}

private:
	bool                            m_Valid;

	Common::Array<Frame>                m_Frames;

	//@{
	/** @name Dokument-Parser Methoden */

	bool ParseAnimationTag(TiXmlElement &AnimationTag, int &FPS, Animation::ANIMATION_TYPES &AnimationType);
	bool ParseFrameTag(TiXmlElement &FrameTag, Frame &Frame, PackageManager &PackageManager);

	//@}

	bool ComputeFeatures();
	bool PrecacheAllFrames() const;
};

} // End of namespace Sword25

#endif
