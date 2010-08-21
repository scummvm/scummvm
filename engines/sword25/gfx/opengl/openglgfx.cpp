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

#include "common/system.h"

#include "sword25/gfx/bitmapresource.h"
#include "sword25/gfx/animationresource.h"
#include "sword25/gfx/fontresource.h"
#include "sword25/gfx/panel.h"
#include "sword25/gfx/renderobjectmanager.h"
#include "sword25/gfx/image/vectorimage.h"
#include "sword25/package/packagemanager.h"
#include "sword25/kernel/inputpersistenceblock.h"
#include "sword25/kernel/outputpersistenceblock.h"

#include "sword25/gfx/opengl/openglgfx.h"
#include "sword25/gfx/opengl/glimage.h"
#include "sword25/gfx/opengl/swimage.h"

namespace Sword25 {

#define BS_LOG_PREFIX "OPENGLGFX"


// -----------------------------------------------------------------------------
// CONSTANTS
// -----------------------------------------------------------------------------

namespace {
const int BIT_DEPTH = 32;
const int BACKBUFFER_COUNT = 1;
const Common::String PNG_EXTENSION(".png");
const Common::String PNG_S_EXTENSION("_s.png");
const Common::String ANI_EXTENSION("_ani.xml");
const Common::String FNT_EXTENSION("_fnt.xml");
const Common::String SWF_EXTENSION(".swf");
const Common::String B25S_EXTENSION(".b25s");
}


// -----------------------------------------------------------------------------
// CONSTRUCTION / DESTRUCTION
// -----------------------------------------------------------------------------

OpenGLGfx::OpenGLGfx(Kernel *pKernel) :
	GraphicEngine(pKernel),
	m_GLspritesInitialized(false) {
}

// -----------------------------------------------------------------------------

OpenGLGfx::~OpenGLGfx() {
	_backSurface.free();
}

// -----------------------------------------------------------------------------

Service *OpenGLGfx_CreateObject(Kernel *pKernel) {
	return new OpenGLGfx(pKernel);
}


// -----------------------------------------------------------------------------
// INTERFACE
// -----------------------------------------------------------------------------

bool OpenGLGfx::Init(int Width, int Height, int BitDepth, int BackbufferCount, bool Windowed) {
	// Warnung ausgeben, wenn eine nicht unterstützte Bittiefe gewählt wurde.
	if (BitDepth != BIT_DEPTH) {
		BS_LOG_WARNINGLN("Can't use a bit depth of %d (not supported). Falling back to %d.", BitDepth, BIT_DEPTH);
		m_BitDepth = BIT_DEPTH;
	}

	// Warnung ausgeben, wenn nicht genau ein Backbuffer gewählt wurde.
	if (BackbufferCount != BACKBUFFER_COUNT) {
		BS_LOG_WARNINGLN("Can't use %d backbuffers (not supported). Falling back to %d.", BackbufferCount, BACKBUFFER_COUNT);
		BackbufferCount = BACKBUFFER_COUNT;
	}

	// Parameter in lokale Variablen kopieren
	m_Width = Width;
	m_Height = Height;
	m_BitDepth = BitDepth;
	m_Windowed = Windowed;
	m_ScreenRect.left = 0;
	m_ScreenRect.top = 0;
	m_ScreenRect.right = m_Width;
	m_ScreenRect.bottom = m_Height;

	_backSurface.create(Width, Height, 4);

	// We already iniitalized gfx after the engine creation
	m_GLspritesInitialized = true;

	// Standardmäßig ist Vsync an.
	SetVsync(true);

	// Layer-Manager initialisieren.
	m_RenderObjectManagerPtr.reset(new RenderObjectManager(Width, Height, BackbufferCount + 1));

	// Hauptpanel erstellen
	m_MainPanelPtr = m_RenderObjectManagerPtr->GetTreeRoot()->AddPanel(Width, Height, BS_ARGB(0, 0, 0, 0));
	if (!m_MainPanelPtr.IsValid()) return false;
	m_MainPanelPtr->SetVisible(true);

	return true;
}

// -----------------------------------------------------------------------------

bool OpenGLGfx::StartFrame(bool UpdateAll) {
	// Berechnen, wie viel Zeit seit dem letzten Frame vergangen ist.
	// Dieser Wert kann über GetLastFrameDuration() von Modulen abgefragt werden, die zeitabhängig arbeiten.
	UpdateLastFrameDuration();

	// Den Layer-Manager auf den nächsten Frame vorbereiten
	m_RenderObjectManagerPtr->StartFrame();

	return true;
}

// -----------------------------------------------------------------------------

bool OpenGLGfx::EndFrame() {
	// Scene zeichnen
	m_RenderObjectManagerPtr->Render();

	g_system->updateScreen();

	// Debug-Lines zeichnen
	if (!m_DebugLines.empty()) {
#if 0
		glEnable(GL_LINE_SMOOTH);
		glBegin(GL_LINES);

		Common::Array<DebugLine>::const_iterator iter = m_DebugLines.begin();
		for (; iter != m_DebugLines.end(); ++iter) {
			const unsigned int &Color = (*iter).Color;
			const BS_Vertex &Start = (*iter).Start;
			const BS_Vertex &End = (*iter).End;

			glColor4ub((Color >> 16) & 0xff, (Color >> 8) & 0xff, Color & 0xff, Color >> 24);
			glVertex2d(Start.X, Start.Y);
			glVertex2d(End.X, End.Y);
		}

		glEnd();
		glDisable(GL_LINE_SMOOTH);
#endif

		warning("STUB: Drawing debug lines");

		m_DebugLines.clear();
	}

	// Framecounter aktualisieren
	m_FPSCounter.Update();

	return true;
}

// -----------------------------------------------------------------------------

RenderObjectPtr<Panel> OpenGLGfx::GetMainPanel() {
	return m_MainPanelPtr;
}

// -----------------------------------------------------------------------------

void OpenGLGfx::SetVsync(bool Vsync) {
	warning("STUB: SetVsync(%d)", Vsync);
}

// -----------------------------------------------------------------------------

bool OpenGLGfx::GetVsync() const {
	warning("STUB: GetVsync()");

	return true;
}

// -----------------------------------------------------------------------------

bool OpenGLGfx::Fill(const Common::Rect *fillRectPtr, unsigned int color) {
	Common::Rect rect(m_Width - 1, m_Height - 1);

	if (fillRectPtr) {
		rect = *fillRectPtr;
	}

	if (fillRectPtr->width() > 0 && fillRectPtr->height() > 0) {
		_backSurface.fillRect(rect, color);
		g_system->copyRectToScreen((byte *)_backSurface.getBasePtr(fillRectPtr->left, fillRectPtr->top), _backSurface.pitch, fillRectPtr->left, fillRectPtr->top, fillRectPtr->width(), fillRectPtr->height());
	}

	return true;
}

// -----------------------------------------------------------------------------

bool OpenGLGfx::GetScreenshot(unsigned int &Width, unsigned int &Height, byte **Data) {
	if (!ReadFramebufferContents(m_Width, m_Height, Data))
		return false;

	// Die Größe des Framebuffers zurückgeben.
	Width = m_Width;
	Height = m_Height;

	// Bilddaten vom OpenGL-Format in unser eigenes Format umwandeln.
	ReverseRGBAComponentOrder(*Data, Width * Height);
	FlipImagedataVertical(Width, Height, *Data);

	return true;
}

// -----------------------------------------------------------------------------

bool OpenGLGfx::ReadFramebufferContents(unsigned int Width, unsigned int Height, byte **Data) {
    *Data = (byte *)malloc(Width * Height * 4);
	
	return true;
}

// -----------------------------------------------------------------------------

void OpenGLGfx::ReverseRGBAComponentOrder(byte *Data, uint size) {
	uint32 *ptr = (uint32 *)Data;

	for (uint i = 0; i < size; i++) {
		unsigned int Pixel = *ptr;
		*ptr = (Pixel & 0xff00ff00) | ((Pixel >> 16) & 0xff) | ((Pixel & 0xff) << 16);
		++ptr;
	}
}

// -----------------------------------------------------------------------------

void OpenGLGfx::FlipImagedataVertical(unsigned int Width, unsigned int Height, byte *Data) {
#if 0 // TODO
	vector<unsigned int> LineBuffer(Width);

	for (unsigned int Y = 0; Y < Height / 2; ++Y) {
		vector<unsigned int>::iterator Line1It = Data.begin() + Y * Width;
		vector<unsigned int>::iterator Line2It = Data.begin() + (Height - 1 - Y) * Width;
		copy(Line1It, Line1It + Width, LineBuffer.begin());
		copy(Line2It, Line2It + Width, Line1It);
		copy(LineBuffer.begin(), LineBuffer.end(), Line2It);
	}
#endif
}

// -----------------------------------------------------------------------------
// RESOURCE MANAGING
// -----------------------------------------------------------------------------

Resource *OpenGLGfx::LoadResource(const Common::String &FileName) {
	BS_ASSERT(CanLoadResource(FileName));

	// Bild für den Softwarebuffer laden
	if (FileName.hasSuffix(PNG_S_EXTENSION)) {
		bool Result;
		SWImage *pImage = new SWImage(FileName, Result);
		if (!Result) {
			delete pImage;
			return 0;
		}

		BitmapResource *pResource = new BitmapResource(FileName, pImage);
		if (!pResource->IsValid()) {
			delete pResource;
			return 0;
		}

		return pResource;
	}

	// Sprite-Bild laden
	if (FileName.hasSuffix(PNG_EXTENSION) || FileName.hasSuffix(B25S_EXTENSION)) {
		bool Result;
		GLImage *pImage = new GLImage(FileName, Result);
		if (!Result) {
			delete pImage;
			return 0;
		}

		BitmapResource *pResource = new BitmapResource(FileName, pImage);
		if (!pResource->IsValid()) {
			delete pResource;
			return 0;
		}

		return pResource;
	}


	// Vectorgraphik laden
	if (FileName.hasSuffix(SWF_EXTENSION)) {
		// Pointer auf Package-Manager holen
		PackageManager *pPackage = Kernel::GetInstance()->GetPackage();
		BS_ASSERT(pPackage);

		// Datei laden
		unsigned char *pFileData;
		unsigned int FileSize;
		if (!(pFileData = static_cast<unsigned char *>(pPackage->GetFile(FileName, &FileSize)))) {
			BS_LOG_ERRORLN("File \"%s\" could not be loaded.", FileName.c_str());
			return 0;
		}

		bool Result;
		VectorImage *pImage = new VectorImage(pFileData, FileSize, Result);
		if (!Result) {
			delete pImage;
			delete [] pFileData;
			return 0;
		}

		BitmapResource *pResource = new BitmapResource(FileName, pImage);
		if (!pResource->IsValid()) {
			delete pResource;
			delete [] pFileData;
			return 0;
		}

		delete [] pFileData;
		return pResource;
	}

	// Animation laden
	if (FileName.hasSuffix(ANI_EXTENSION)) {
		AnimationResource *pResource = new AnimationResource(FileName);
		if (pResource->IsValid())
			return pResource;
		else {
			delete pResource;
			return 0;
		}
	}

	// Font laden
	if (FileName.hasSuffix(FNT_EXTENSION)) {
		FontResource *pResource = new FontResource(Kernel::GetInstance(), FileName);
		if (pResource->IsValid())
			return pResource;
		else {
			delete pResource;
			return 0;
		}
	}

	BS_LOG_ERRORLN("Service cannot load \"%s\".", FileName.c_str());
	return 0;
}

// -----------------------------------------------------------------------------

bool OpenGLGfx::CanLoadResource(const Common::String &FileName) {
	return FileName.hasSuffix(PNG_EXTENSION) ||
		FileName.hasSuffix(ANI_EXTENSION) ||
		FileName.hasSuffix(FNT_EXTENSION) ||
		FileName.hasSuffix(SWF_EXTENSION) ||
		FileName.hasSuffix(B25S_EXTENSION);
}


// -----------------------------------------------------------------------------
// DEBUGGING
// -----------------------------------------------------------------------------

void OpenGLGfx::DrawDebugLine(const Vertex &Start, const Vertex &End, unsigned int Color) {
	m_DebugLines.push_back(DebugLine(Start, End, Color));
}

// -----------------------------------------------------------------------------
// PERSISTENZ
// -----------------------------------------------------------------------------

bool OpenGLGfx::Persist(OutputPersistenceBlock &Writer) {
	bool result = true;

	result &= GraphicEngine::Persist(Writer);
	result &= m_RenderObjectManagerPtr->Persist(Writer);

	return result;
}

// -----------------------------------------------------------------------------

bool OpenGLGfx::Unpersist(InputPersistenceBlock &Reader) {
	bool result = true;

	result &= GraphicEngine::Unpersist(Reader);
	result &= m_RenderObjectManagerPtr->Unpersist(Reader);

	return result && Reader.IsGood();
}

} // End of namespace Sword25
