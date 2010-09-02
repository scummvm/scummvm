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

class Kernel;
class Service;
class Resource;
class Panel;
class BS_Image;
class RenderObjectManager;


// -----------------------------------------------------------------------------
// CLASS DECLARATION
// -----------------------------------------------------------------------------

class OpenGLGfx : public GraphicEngine {
public:
	OpenGLGfx(Kernel *pKernel);
	virtual ~OpenGLGfx();

	// Interface
	// ---------
	virtual bool        Init(int Width, int Height, int BitDepth, int BackbufferCount, bool Windowed);
	virtual bool        StartFrame(bool UpdateAll);
	virtual bool        EndFrame();

	virtual RenderObjectPtr<Panel> GetMainPanel();

	virtual void        SetVsync(bool Vsync);
	virtual bool        GetVsync() const;

	virtual bool        Fill(const Common::Rect *FillRectPtr = 0, uint Color = BS_RGB(0, 0, 0));
	virtual bool        GetScreenshot(uint &Width, uint &Height, byte **Data);

	// Resource-Managing Methoden
	// --------------------------
	virtual Resource    *LoadResource(const Common::String &FileName);
	virtual bool            CanLoadResource(const Common::String &FileName);

	// Debugging Methoden
	// ------------------
	virtual void DrawDebugLine(const Vertex &Start, const Vertex &End, uint Color);

	// Persistenz Methoden
	// -------------------
	virtual bool Persist(OutputPersistenceBlock &Writer);
	virtual bool Unpersist(InputPersistenceBlock &Reader);

private:
	bool                m_GLspritesInitialized;
	byte *_backBuffer;

	RenderObjectPtr<Panel> m_MainPanelPtr;

	Common::ScopedPtr<RenderObjectManager>   m_RenderObjectManagerPtr;

	struct DebugLine {
		DebugLine(const Vertex &_Start, const Vertex &_End, uint _Color) :
			Start(_Start),
			End(_End),
			Color(_Color) {}
		DebugLine() {}

		Vertex       Start;
		Vertex       End;
		uint    Color;
	};

	Common::Array<DebugLine> m_DebugLines;

	static bool ReadFramebufferContents(uint Width, uint Height, byte **Data);
	static void ReverseRGBAComponentOrder(byte *Data, uint size);
	static void FlipImagedataVertical(uint Width, uint Height, byte *Data);
};

} // End of namespace Sword25

#endif
