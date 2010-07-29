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

#ifndef BS_PANEL_H
#define BS_PANEL_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "kernel/common.h"
#include "renderobject.h"

// -----------------------------------------------------------------------------
// Class Definition
// -----------------------------------------------------------------------------

class BS_Panel : public BS_RenderObject
{
friend class BS_RenderObject;

private:
	BS_Panel(BS_RenderObjectPtr<BS_RenderObject> ParentPtr, int Width, int Height, unsigned int Color);
	BS_Panel(BS_InputPersistenceBlock & Reader, BS_RenderObjectPtr<BS_RenderObject> ParentPtr, unsigned int Handle);

public:
	virtual ~BS_Panel();

	unsigned int	GetColor() const { return m_Color; }
	void			SetColor(unsigned int Color) { m_Color = Color; ForceRefresh(); }

	virtual bool Persist(BS_OutputPersistenceBlock & Writer);
	virtual bool Unpersist(BS_InputPersistenceBlock & Reader);

protected:
	virtual bool DoRender();

private:
	unsigned int m_Color;
};

#endif
