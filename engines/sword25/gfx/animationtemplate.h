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
	static uint Create(const Common::String &SourceAnimation);
	static uint Create(const AnimationTemplate &Other);
	static uint Create(InputPersistenceBlock &Reader, uint Handle);
	AnimationTemplate *ResolveHandle(uint Handle) const;

private:
	AnimationTemplate(const Common::String &SourceAnimation);
	AnimationTemplate(const AnimationTemplate &Other);
	AnimationTemplate(InputPersistenceBlock &Reader, uint Handle);

public:
	~AnimationTemplate();

	virtual const Frame    &getFrame(uint Index) const {
		BS_ASSERT(Index < _frames.size());
		return _frames[Index];
	}
	virtual uint    getFrameCount() const {
		return _frames.size();
	}
	virtual void            unlock() {
		delete this;
	}

	bool isValid() const {
		return _valid;
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
		_animationType = Type;
	}

	/**
	    @brief Setzt die Abspielgeschwindigkeit.
	    @param FPS die Abspielgeschwindigkeit in Frames pro Sekunde.
	*/
	void SetFPS(int FPS);

	virtual bool persist(OutputPersistenceBlock &writer);
	virtual bool unpersist(InputPersistenceBlock &reader);

private:
	Common::Array<Frame>  _frames;
	AnimationResource    *_sourceAnimationPtr;
	bool                  _valid;

	AnimationResource *RequestSourceAnimation(const Common::String &SourceAnimation) const;
	bool ValidateSourceIndex(uint Index) const;
	bool ValidateDestIndex(uint Index) const;
};

} // End of namespace Sword25

#endif
