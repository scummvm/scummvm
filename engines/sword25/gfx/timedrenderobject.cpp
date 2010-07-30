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

#include "sword25/gfx/timedrenderobject.h"

#include "sword25/gfx/renderobjectmanager.h"

// -----------------------------------------------------------------------------
// Konstruktion / Destruktion
// -----------------------------------------------------------------------------

BS_TimedRenderObject::BS_TimedRenderObject(BS_RenderObjectPtr<BS_RenderObject> pParent, TYPES Type, unsigned int Handle) :
	BS_RenderObject(pParent, Type, Handle)
{
	BS_ASSERT(GetManager());
	GetManager()->AttatchTimedRenderObject(this);
}

BS_TimedRenderObject::~BS_TimedRenderObject()
{
	BS_ASSERT(GetManager());
	GetManager()->DetatchTimedRenderObject(this);
}