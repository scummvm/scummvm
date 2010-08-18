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

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/kernel/inputpersistenceblock.h"
#include "sword25/gfx/animationdescription.h"

namespace Sword25 {

// -----------------------------------------------------------------------------
// Persistenz
// -----------------------------------------------------------------------------

bool AnimationDescription::Persist(OutputPersistenceBlock &Writer) {
	Writer.Write(static_cast<unsigned int>(m_AnimationType));
	Writer.Write(m_FPS);
	Writer.Write(m_MillisPerFrame);
	Writer.Write(m_ScalingAllowed);
	Writer.Write(m_AlphaAllowed);
	Writer.Write(m_ColorModulationAllowed);

	return true;
}

// -----------------------------------------------------------------------------

bool AnimationDescription::Unpersist(InputPersistenceBlock &Reader) {
	unsigned int AnimationType;
	Reader.Read(AnimationType);
	m_AnimationType = static_cast<Animation::ANIMATION_TYPES>(AnimationType);
	Reader.Read(m_FPS);
	Reader.Read(m_MillisPerFrame);
	Reader.Read(m_ScalingAllowed);
	Reader.Read(m_AlphaAllowed);
	Reader.Read(m_ColorModulationAllowed);

	return Reader.IsGood();
}

} // End of namespace Sword25
