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

#ifndef SWORD25_RENDER_OBJECT_PTR_H
#define SWORD25_RENDER_OBJECT_PTR_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/gfx/renderobjectregistry.h"

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

#endif
