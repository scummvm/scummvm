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

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/kernel/inputpersistenceblock.h"
#include "sword25/gfx/animationdescription.h"

// -----------------------------------------------------------------------------
// Persistenz
// -----------------------------------------------------------------------------

bool BS_AnimationDescription::Persist(BS_OutputPersistenceBlock & Writer)
{
	Writer.Write(static_cast<unsigned int>(m_AnimationType));
	Writer.Write(m_FPS);
	Writer.Write(m_MillisPerFrame);
	Writer.Write(m_ScalingAllowed);
	Writer.Write(m_AlphaAllowed);
	Writer.Write(m_ColorModulationAllowed);

	return true;
}

// -----------------------------------------------------------------------------

bool BS_AnimationDescription::Unpersist(BS_InputPersistenceBlock & Reader)
{
	unsigned int AnimationType;
	Reader.Read(AnimationType);
	m_AnimationType = static_cast<BS_Animation::ANIMATION_TYPES>(AnimationType);
	Reader.Read(m_FPS);
	Reader.Read(m_MillisPerFrame);
	Reader.Read(m_ScalingAllowed);
	Reader.Read(m_AlphaAllowed);
	Reader.Read(m_ColorModulationAllowed);

	return Reader.IsGood();
}
