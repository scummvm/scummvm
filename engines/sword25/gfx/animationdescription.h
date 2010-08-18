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

#ifndef SWORD25_ANIMATIONDESCRIPTION_H
#define SWORD25_ANIMATIONDESCRIPTION_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/kernel/persistable.h"
#include "sword25/gfx/animation.h"

namespace Sword25 {

// -----------------------------------------------------------------------------
// Klassendefinition
// -----------------------------------------------------------------------------

class AnimationDescription : public BS_Persistable {
protected:
	AnimationDescription() :
		m_AnimationType(Animation::AT_LOOP),
		m_FPS(10),
		m_MillisPerFrame(0),
		m_ScalingAllowed(true),
		m_AlphaAllowed(true),
		m_ColorModulationAllowed(true)
	{};

public:
	struct Frame {
		// Die Hotspot-Angabe bezieht sich auf das ungeflippte Bild!!
		int         HotspotX;
		int         HotspotY;
		bool        FlipV;
		bool        FlipH;
		Common::String  FileName;
		Common::String  Action;
	};

	// -----------------------------------------------------------------------------
	// Abstrakte Methoden
	// -----------------------------------------------------------------------------

	virtual const Frame    &GetFrame(unsigned int Index) const = 0;
	virtual unsigned int    GetFrameCount() const = 0;
	virtual void            Unlock() = 0;

	// -----------------------------------------------------------------------------
	// Getter Methoden
	// -----------------------------------------------------------------------------

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

	virtual bool Persist(BS_OutputPersistenceBlock &Writer);
	virtual bool Unpersist(BS_InputPersistenceBlock &Reader);

protected:
	Animation::ANIMATION_TYPES   m_AnimationType;
	int                             m_FPS;
	int                             m_MillisPerFrame;
	bool                            m_ScalingAllowed;
	bool                            m_AlphaAllowed;
	bool                            m_ColorModulationAllowed;
};

} // End of namespace Sword25

#endif
