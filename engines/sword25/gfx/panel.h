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

#ifndef SWORD25_PANEL_H
#define SWORD25_PANEL_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/gfx/renderobject.h"

namespace Sword25 {

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

} // End of namespace Sword25

#endif
