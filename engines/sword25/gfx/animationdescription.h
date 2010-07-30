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

#ifndef SWORD25_ANIMATIONDESCRIPTION_H
#define SWORD25_ANIMATIONDESCRIPTION_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/kernel/persistable.h"
#include "sword25/gfx/animation.h"

// -----------------------------------------------------------------------------
// Klassendefinition
// -----------------------------------------------------------------------------

class BS_AnimationDescription : public BS_Persistable
{
protected:
	BS_AnimationDescription() :
		m_AnimationType(BS_Animation::AT_LOOP),
		m_FPS(10),
		m_MillisPerFrame(0),
		m_ScalingAllowed(true),
		m_AlphaAllowed(true),
		m_ColorModulationAllowed(true)
	{};

public:
	struct Frame
	{
		// Die Hotspot-Angabe bezieht sich auf das ungeflippte Bild!!
		int			HotspotX;
		int			HotspotY;
		bool		FlipV;
		bool		FlipH;
		std::string	FileName;
		std::string	Action;
	};

	// -----------------------------------------------------------------------------
	// Abstrakte Methoden
	// -----------------------------------------------------------------------------
	
	virtual const Frame &	GetFrame(unsigned int Index) const = 0;
	virtual unsigned int	GetFrameCount() const = 0;
	virtual void			Unlock() = 0;

	// -----------------------------------------------------------------------------
	// Getter Methoden
	// -----------------------------------------------------------------------------
	
	BS_Animation::ANIMATION_TYPES	GetAnimationType() const { return m_AnimationType; }
	int								GetFPS() const { return m_FPS; }
	int								GetMillisPerFrame() const { return m_MillisPerFrame; }
	bool							IsScalingAllowed() const { return m_ScalingAllowed; }
	bool							IsAlphaAllowed() const { return m_AlphaAllowed; }
	bool							IsColorModulationAllowed() const { return m_ColorModulationAllowed; }

	virtual bool Persist(BS_OutputPersistenceBlock & Writer);
	virtual bool Unpersist(BS_InputPersistenceBlock & Reader);

protected:
	BS_Animation::ANIMATION_TYPES	m_AnimationType;
	int								m_FPS;
	int								m_MillisPerFrame;
	bool							m_ScalingAllowed;
	bool							m_AlphaAllowed;
	bool							m_ColorModulationAllowed;
};

#endif
