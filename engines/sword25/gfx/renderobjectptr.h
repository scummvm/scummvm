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

#ifndef SWORD25_RENDER_OBJECT_PTR_H
#define SWORD25_RENDER_OBJECT_PTR_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/gfx/renderobjectregistry.h"

namespace Sword25 {

// -----------------------------------------------------------------------------
// Forward Declarations
// -----------------------------------------------------------------------------

class BS_RenderObject;

// -----------------------------------------------------------------------------
// Klassendeklaration
// -----------------------------------------------------------------------------

template<class T>
class BS_RenderObjectPtr
{
public:
	BS_RenderObjectPtr() : m_Handle(0)
	{}

	BS_RenderObjectPtr(unsigned int Handle) : m_Handle(Handle)
	{}

	BS_RenderObjectPtr(BS_RenderObject * RenderObjectPtr)
	{
		m_Handle = RenderObjectPtr->GetHandle();
	}

	T * operator->() const 
	{
		return static_cast<T *>(BS_RenderObjectRegistry::GetInstance().ResolveHandle(m_Handle));
	}

	bool operator==(const BS_RenderObjectPtr<T> & other)
	{
		return m_Handle == other.m_Handle;
	}

	bool IsValid() const
	{
		return BS_RenderObjectRegistry::GetInstance().ResolveHandle(m_Handle) != 0;
	}

	void Erase()
	{
		delete static_cast<T *>(BS_RenderObjectRegistry::GetInstance().ResolveHandle(m_Handle));
		m_Handle = 0;
	}

private:
	unsigned int m_Handle;
};

} // End of namespace Sword25

#endif
