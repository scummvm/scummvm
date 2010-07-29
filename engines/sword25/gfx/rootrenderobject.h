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

#ifndef BS_ROOTRENDEROBJECT_H
#define BS_ROOTRENDEROBJECT_H

// Includes
#include "kernel/common.h"
#include "renderobject.h"

// -----------------------------------------------------------------------------
// Forward Declarations
// -----------------------------------------------------------------------------

class BS_Kernel;

// Klassendefinition
class BS_RenderObjectManager;

class BS_RootRenderObject : public BS_RenderObject
{
friend BS_RenderObjectManager;

private:
	BS_RootRenderObject(BS_RenderObjectManager * ManagerPtr, int Width, int Height) :
	   BS_RenderObject(BS_RenderObjectPtr<BS_RenderObject>(), TYPE_ROOT)
	{
		m_ManagerPtr = ManagerPtr;
		m_Width = Width;
		m_Height = Height;
	}

protected:
	virtual bool DoRender() { return true; }
};

#endif
