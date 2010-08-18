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

#ifndef SWORD25_ANIMATION_TEMPLATE_H
#define SWORD25_ANIMATION_TEMPLATE_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/kernel/persistable.h"
#include "sword25/gfx/animationdescription.h"

namespace Sword25 {

// -----------------------------------------------------------------------------
// Forward declarations
// -----------------------------------------------------------------------------

class AnimationResource;

// -----------------------------------------------------------------------------
// Klassendefinition
// -----------------------------------------------------------------------------

class AnimationTemplate : public AnimationDescription {
public:
	static unsigned int Create(const Common::String &SourceAnimation);
	static unsigned int Create(const AnimationTemplate &Other);
	static unsigned int Create(InputPersistenceBlock &Reader, unsigned int Handle);
	AnimationTemplate *ResolveHandle(unsigned int Handle) const;

private:
	AnimationTemplate(const Common::String &SourceAnimation);
	AnimationTemplate(const AnimationTemplate &Other);
	AnimationTemplate(InputPersistenceBlock &Reader, unsigned int Handle);

public:
	~AnimationTemplate();

	virtual const Frame    &GetFrame(unsigned int Index) const {
		BS_ASSERT(Index < m_Frames.size());
		return m_Frames[Index];
	}
	virtual unsigned int    GetFrameCount() const {
		return m_Frames.size();
	}
	virtual void            Unlock() {
		delete this;
	}

	bool IsValid() const {
		return m_Valid;
	}

	/**
	    @brief Fügt einen neuen Frame zur Animation hinzu.

	    Der Frame wird an das Ende der Animation angehängt.

	    @param Index der Index des Frames in der Quellanimation
	*/
	void AddFrame(int Index);

	/**
	    @brief Ändert einen bereits in der Animation vorhandenen Frame.
	    @param DestIndex der Index des Frames der überschrieben werden soll
	    @param SrcIndex der Index des einzufügenden Frames in der Quellanimation
	*/
	void SetFrame(int DestIndex, int SrcIndex);

	/**
	    @brief Setzt den Animationstyp.
	    @param Type der Typ der Animation. Muss aus den enum BS_Animation::ANIMATION_TYPES sein.
	*/
	void SetAnimationType(Animation::ANIMATION_TYPES Type) {
		m_AnimationType = Type;
	}

	/**
	    @brief Setzt die Abspielgeschwindigkeit.
	    @param FPS die Abspielgeschwindigkeit in Frames pro Sekunde.
	*/
	void SetFPS(int FPS);

	virtual bool Persist(OutputPersistenceBlock &Writer);
	virtual bool Unpersist(InputPersistenceBlock &Reader);

private:
	Common::Array<Frame>  m_Frames;
	AnimationResource       *m_SourceAnimationPtr;
	bool                        m_Valid;

	AnimationResource *RequestSourceAnimation(const Common::String &SourceAnimation) const;
	bool ValidateSourceIndex(unsigned int Index) const;
	bool ValidateDestIndex(unsigned int Index) const;
};

} // End of namespace Sword25

#endif
