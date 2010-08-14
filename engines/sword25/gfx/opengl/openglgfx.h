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

#ifndef SWORD25_OPENGLGFX_H
#define SWORD25_OPENGLGFX_H

// -----------------------------------------------------------------------------
// INCLUDES
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/gfx/graphicengine.h"
#include "sword25/gfx/renderobjectptr.h"

namespace Sword25 {

// -----------------------------------------------------------------------------
// FORWARD DECLARATIONS
// -----------------------------------------------------------------------------

class BS_Kernel;
class BS_Service;
class BS_Resource;
class BS_Panel;
class BS_Image;
class BS_RenderObjectManager;


// -----------------------------------------------------------------------------
// CLASS DECLARATION
// -----------------------------------------------------------------------------

class BS_OpenGLGfx : public BS_GraphicEngine {
public:
	BS_OpenGLGfx(BS_Kernel *pKernel);
	virtual ~BS_OpenGLGfx();

	// Interface
	// ---------
	virtual bool        Init(int Width, int Height, int BitDepth, int BackbufferCount, bool Windowed);
	virtual bool        StartFrame(bool UpdateAll);
	virtual bool        EndFrame();

	virtual BS_RenderObjectPtr<BS_Panel> GetMainPanel();

	virtual void        SetVsync(bool Vsync);
	virtual bool        GetVsync() const;

	virtual bool        Fill(const BS_Rect *FillRectPtr = 0, unsigned int Color = BS_RGB(0, 0, 0));
	virtual bool        GetScreenshot(unsigned int &Width, unsigned int &Height, byte **Data);

	// Resource-Managing Methoden
	// --------------------------
	virtual BS_Resource    *LoadResource(const Common::String &FileName);
	virtual bool            CanLoadResource(const Common::String &FileName);

	// Debugging Methoden
	// ------------------
	virtual void DrawDebugLine(const BS_Vertex &Start, const BS_Vertex &End, unsigned int Color);

	// Persistenz Methoden
	// -------------------
	virtual bool Persist(BS_OutputPersistenceBlock &Writer);
	virtual bool Unpersist(BS_InputPersistenceBlock &Reader);

private:
	bool                m_GLspritesInitialized;
	byte *_backBuffer;

	BS_RenderObjectPtr<BS_Panel> m_MainPanelPtr;

	std::auto_ptr<BS_RenderObjectManager>   m_RenderObjectManagerPtr;

	struct DebugLine {
		DebugLine(const BS_Vertex &_Start, const BS_Vertex &_End, unsigned int _Color) :
			Start(_Start),
			End(_End),
			Color(_Color) {}
		DebugLine() {}

		BS_Vertex       Start;
		BS_Vertex       End;
		unsigned int    Color;
	};

	Common::Array<DebugLine> m_DebugLines;

	static bool ReadFramebufferContents(unsigned int Width, unsigned int Height, byte **Data);
	static void ReverseRGBAComponentOrder(byte *Data, uint size);
	static void FlipImagedataVertical(unsigned int Width, unsigned int Height, byte *Data);
};

} // End of namespace Sword25

#endif
