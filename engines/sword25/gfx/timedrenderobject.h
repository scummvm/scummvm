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
// System Includes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Engine Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/gfx/renderobject.h"

// -----------------------------------------------------------------------------
// Forward Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Class Definition
// -----------------------------------------------------------------------------

/**
	@brief 
*/

class BS_TimedRenderObject : public BS_RenderObject
{
public:
	BS_TimedRenderObject(BS_RenderObjectPtr<BS_RenderObject> pParent, TYPES Type, unsigned int Handle = 0);
	BS_TimedRenderObject::~BS_TimedRenderObject();

	/**
		@brief Teilt dem Objekt mit, dass ein neuer Frame begonnen wird.

		Diese Methode wird jeden Frame an jedem BS_TimedRenderObject aufgerufen um diesen zu ermöglichen
		ihren Zustand Zeitabhängig zu verändern (z.B. Animationen).<br>
		@param int TimeElapsed gibt an wie viel Zeit (in Microsekunden) seit dem letzten Frame vergangen ist.
	*/
	virtual void FrameNotification(int TimeElapsed) = 0;
};