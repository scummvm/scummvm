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

#include "sword25/gfx/panel.h"

#include "sword25/kernel/inputpersistenceblock.h"
#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/gfx/graphicengine.h"
#include "sword25/gfx/image/image.h"

namespace Sword25 {

// -----------------------------------------------------------------------------

#define BS_LOG_PREFIX "PANEL"

// -----------------------------------------------------------------------------
// Construction/Destruction
// -----------------------------------------------------------------------------

BS_Panel::BS_Panel(BS_RenderObjectPtr<BS_RenderObject> ParentPtr, int Width, int Height, unsigned int Color) :
	BS_RenderObject(ParentPtr, BS_RenderObject::TYPE_PANEL),
	m_Color(Color) {
	m_InitSuccess = false;

	m_Width = Width;
	m_Height = Height;

	if (m_Width < 0) {
		BS_LOG_ERRORLN("Tried to initialise a panel with an invalid width (%d).", m_Width);
		return;
	}

	if (m_Height < 0) {
		BS_LOG_ERRORLN("Tried to initialise a panel with an invalid height (%d).", m_Height);
		return;
	}

	m_InitSuccess = true;
}

// -----------------------------------------------------------------------------

BS_Panel::BS_Panel(BS_InputPersistenceBlock &Reader, BS_RenderObjectPtr<BS_RenderObject> ParentPtr, unsigned int Handle) :
	BS_RenderObject(ParentPtr, BS_RenderObject::TYPE_PANEL, Handle) {
	m_InitSuccess = Unpersist(Reader);
}

// -----------------------------------------------------------------------------

BS_Panel::~BS_Panel() {
}

// -----------------------------------------------------------------------------
// Rendern
// -----------------------------------------------------------------------------

bool BS_Panel::DoRender() {
	// Falls der Alphawert 0 ist, ist das Panel komplett durchsichtig und es muss nichts gezeichnet werden.
	if (m_Color >> 24 == 0) return true;

	BS_GraphicEngine *GfxPtr = static_cast<BS_GraphicEngine *>(BS_Kernel::GetInstance()->GetService("gfx"));
	BS_ASSERT(GfxPtr);

	return GfxPtr->Fill(&m_BBox, m_Color);
}

// -----------------------------------------------------------------------------
// Persistenz
// -----------------------------------------------------------------------------

bool BS_Panel::Persist(BS_OutputPersistenceBlock &Writer) {
	bool Result = true;

	Result &= BS_RenderObject::Persist(Writer);
	Writer.Write(m_Color);

	Result &= BS_RenderObject::PersistChildren(Writer);

	return Result;
}

// -----------------------------------------------------------------------------

bool BS_Panel::Unpersist(BS_InputPersistenceBlock &Reader) {
	bool Result = true;

	Result &= BS_RenderObject::Unpersist(Reader);

	unsigned int Color;
	Reader.Read(Color);
	SetColor(Color);

	Result &= BS_RenderObject::UnpersistChildren(Reader);

	return Reader.IsGood() && Result;
}

} // End of namespace Sword25
