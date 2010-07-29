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
// INCLUDES
// -----------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/GL.h>

#include "util/glsprites/glsprites.h"
#include "../bitmapresource.h"
#include "../animationresource.h"
#include "../fontresource.h"
#include "../panel.h"
#include "../renderobjectmanager.h"
#include "../image/vectorimage.h"
#include "package/packagemanager.h"
#include "kernel/inputpersistenceblock.h"
#include "kernel/outputpersistenceblock.h"

#include "openglgfx.h"
#include "glimage.h"
#include "swimage.h"

#include <algorithm>

using namespace std;

#define BS_LOG_PREFIX "OPENGLGFX"


// -----------------------------------------------------------------------------
// CONSTANTS
// -----------------------------------------------------------------------------

namespace
{
	const unsigned int BIT_DEPTH = 32;
	const unsigned int BACKBUFFER_COUNT = 1;
	const std::string PNG_EXTENSION(".png");
	const std::string PNG_S_EXTENSION("_s.png");
	const std::string ANI_EXTENSION("_ani.xml");
	const std::string FNT_EXTENSION("_fnt.xml");
	const std::string SWF_EXTENSION(".swf");
	const std::string B25S_EXTENSION(".b25s");
}


// -----------------------------------------------------------------------------
// CONSTRUCTION / DESTRUCTION
// -----------------------------------------------------------------------------

BS_OpenGLGfx::BS_OpenGLGfx(BS_Kernel * pKernel) :
	BS_GraphicEngine(pKernel),
	m_GLspritesInitialized(false)
{
}

// -----------------------------------------------------------------------------

BS_OpenGLGfx::~BS_OpenGLGfx()
{
	if (m_GLspritesInitialized) GLS_Quit();
}

// -----------------------------------------------------------------------------

BS_Service * BS_OpenGLGfx_CreateObject(BS_Kernel* pKernel)
{
	return new BS_OpenGLGfx(pKernel);
}


// -----------------------------------------------------------------------------
// INTERFACE
// -----------------------------------------------------------------------------

bool BS_OpenGLGfx::Init(int Width, int Height, int BitDepth, int BackbufferCount, bool Windowed)
{
	// Warnung ausgeben, wenn eine nicht unterstützte Bittiefe gewählt wurde.
	if (BitDepth != BIT_DEPTH)
	{
		BS_LOG_WARNINGLN("Can't use a bit depth of %d (not supported). Falling back to %d.", BitDepth, BIT_DEPTH);
		m_BitDepth = BIT_DEPTH;
	}

	// Warnung ausgeben, wenn nicht genau ein Backbuffer gewählt wurde.
	if (BackbufferCount != BACKBUFFER_COUNT)
	{
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

	// GLsprites initialisieren
	HWND hwnd = reinterpret_cast<HWND>(BS_Kernel::GetInstance()->GetWindow()->GetWindowHandle());
	GLS_Result Result = GLS_InitExternalWindow(Width, Height, Windowed ? GLS_False : GLS_True, hwnd);
	if (Result != GLS_OK)
	{
		BS_LOG_ERRORLN("Could not initialize GLsprites. Reason: %s", GLS_ResultString(Result));
		return false;
	}
	m_GLspritesInitialized = true;

	// Standardmäßig ist Vsync an.
	SetVsync(true);

	// Layer-Manager initialisieren.
	m_RenderObjectManagerPtr.reset(new BS_RenderObjectManager(Width, Height, BackbufferCount + 1));

	// Hauptpanel erstellen
	m_MainPanelPtr = m_RenderObjectManagerPtr->GetTreeRoot()->AddPanel(Width, Height, BS_ARGB(0,0,0,0));
	if (!m_MainPanelPtr.IsValid()) return false;
	m_MainPanelPtr->SetVisible(true);

	return true;
}

// -----------------------------------------------------------------------------

bool BS_OpenGLGfx::StartFrame(bool UpdateAll)
{
	// Berechnen, wie viel Zeit seit dem letzten Frame vergangen ist.
	// Dieser Wert kann über GetLastFrameDuration() von Modulen abgefragt werden, die zeitabhängig arbeiten.
	UpdateLastFrameDuration();

	// Den Layer-Manager auf den nächsten Frame vorbereiten
	m_RenderObjectManagerPtr->StartFrame();

	// GLsprites bescheidgeben
	GLS_Result Result = GLS_StartFrame();
	if (Result != GLS_OK)
	{
		BS_LOG_ERRORLN("Call to GLS_StartFrame() failed. Reason: %s", GLS_ResultString(Result));
		return false;
	}

	return true;
}

// -----------------------------------------------------------------------------

bool BS_OpenGLGfx::EndFrame()
{
	// Scene zeichnen
	m_RenderObjectManagerPtr->Render();

	// Debug-Lines zeichnen
	if (!m_DebugLines.empty())
	{
		glEnable(GL_LINE_SMOOTH);
		glBegin(GL_LINES);

		std::vector<DebugLine>::const_iterator iter = m_DebugLines.begin();
		for (; iter != m_DebugLines.end(); ++iter)
		{
			const unsigned int & Color = (*iter).Color;
			const BS_Vertex & Start = (*iter).Start;
			const BS_Vertex & End = (*iter).End;

			glColor4ub((Color >> 16) & 0xff, (Color >> 8) & 0xff, Color & 0xff, Color >> 24);
			glVertex2d(Start.X, Start.Y);
			glVertex2d(End.X, End.Y);
		}

		glEnd();
		glDisable(GL_LINE_SMOOTH);

		m_DebugLines.clear();
	}

	// Flippen
	GLS_Result Result = GLS_EndFrame();
	if (Result != GLS_OK)
	{
		BS_LOG_ERRORLN("Call to GLS_EndFrame() failed. Reason: %s", GLS_ResultString(Result));
		return false;
	}

	// Framecounter aktualisieren
	m_FPSCounter.Update();

	return true;
}

// -----------------------------------------------------------------------------

BS_RenderObjectPtr<BS_Panel> BS_OpenGLGfx::GetMainPanel()
{
	return m_MainPanelPtr;
}

// -----------------------------------------------------------------------------

void BS_OpenGLGfx::SetVsync(bool Vsync)
{
	GLS_Result Result = GLS_SetVSync(Vsync ? GLS_True : GLS_False);
	if (Result != GLS_OK) BS_LOG_WARNINGLN("Could not set vsync status. Reason: %s", GLS_ResultString(Result));
}

// -----------------------------------------------------------------------------

bool BS_OpenGLGfx::GetVsync() const
{
	GLS_Bool Status;
	GLS_Result Result = GLS_IsVsync(&Status);
	if (Result != GLS_OK)
	{
		BS_LOG_WARNINGLN("Could not get vsync status. Returning false. Reason: %s", GLS_ResultString(Result));
		return false;
	}

	return Status == GLS_True ? true : false;
}

// -----------------------------------------------------------------------------

bool BS_OpenGLGfx::Fill(const BS_Rect* FillRectPtr, unsigned int Color)
{
	BS_Rect Rect;

	if (!FillRectPtr)
	{
		Rect.left = 0;
		Rect.top = 0;
		Rect.right = m_Width;
		Rect.bottom = m_Height;
		FillRectPtr = &Rect;
	}

	glBegin(GL_QUADS);
		glColor4ub((Color >> 16) & 0xff, (Color >> 8) & 0xff, Color & 0xff, Color >> 24);

		glVertex2i(FillRectPtr->left, FillRectPtr->top);
		glVertex2i(FillRectPtr->right, FillRectPtr->top);
		glVertex2i(FillRectPtr->right, FillRectPtr->bottom);
		glVertex2i(FillRectPtr->left, FillRectPtr->bottom);
	glEnd();

	return glGetError() == 0;
}

// -----------------------------------------------------------------------------

bool BS_OpenGLGfx::GetScreenshot(unsigned int & Width, unsigned int & Height, vector<unsigned int> & Data)
{
	if (!ReadFramebufferContents(m_Width, m_Height, Data)) return false;

	// Die Größe des Framebuffers zurückgeben.
	Width = m_Width;
	Height = m_Height;

	// Bilddaten vom OpenGL-Format in unser eigenes Format umwandeln.
	ReverseRGBAComponentOrder(Data);
	FlipImagedataVertical(Width, Height, Data);

	return true;
}

// -----------------------------------------------------------------------------

bool BS_OpenGLGfx::ReadFramebufferContents(unsigned int Width, unsigned int Height, std::vector<unsigned int> & Data)
{
	Data.resize(Width * Height);
	glReadPixels(0, 0, Width, Height, GL_RGBA, GL_UNSIGNED_BYTE, &Data[0]);

	if (glGetError() == 0)
		return true;
	else
	{
		Data.clear();
		return false;
	}
}

// -----------------------------------------------------------------------------

void BS_OpenGLGfx::ReverseRGBAComponentOrder(vector<unsigned int> & Data)
{
	vector<unsigned int>::iterator It = Data.begin();
	while (It != Data.end())
	{
		unsigned int Pixel = *It;
		*It = (Pixel & 0xff00ff00) | ((Pixel >> 16) & 0xff) | ((Pixel & 0xff) << 16);
		++It;
	}
}

// -----------------------------------------------------------------------------

void BS_OpenGLGfx::FlipImagedataVertical(unsigned int Width, unsigned int Height, vector<unsigned int> & Data)
{
	vector<unsigned int> LineBuffer(Width);

	for (unsigned int Y = 0; Y < Height / 2; ++Y)
	{
		vector<unsigned int>::iterator Line1It = Data.begin() + Y * Width;
		vector<unsigned int>::iterator Line2It = Data.begin() + (Height - 1 - Y) * Width;
		copy(Line1It, Line1It + Width, LineBuffer.begin());
		copy(Line2It, Line2It + Width, Line1It);
		copy(LineBuffer.begin(), LineBuffer.end(), Line2It);
	}
}

// -----------------------------------------------------------------------------
// RESOURCE MANAGING
// -----------------------------------------------------------------------------

static bool DoesStringEndWith(const std::string & String, const std::string & OtherString)
{
	std::string::size_type StringPos = String.rfind(OtherString);
	if (StringPos == std::string::npos) return false;

	return StringPos + OtherString.size() == String.size();
}

// -----------------------------------------------------------------------------

BS_Resource * BS_OpenGLGfx::LoadResource(const std::string& FileName)
{
	BS_ASSERT(CanLoadResource(FileName));

	// Bild für den Softwarebuffer laden
	if (DoesStringEndWith(FileName, PNG_S_EXTENSION))
	{
		bool Result;
		BS_SWImage * pImage = new BS_SWImage(FileName, Result);
		if (!Result)
		{
			delete pImage;
			return 0;
		}

		BS_BitmapResource * pResource = new BS_BitmapResource(FileName, pImage);
		if (!pResource->IsValid())
		{
			delete pResource;
			return 0;
		}

		return pResource;
	}

	// Sprite-Bild laden
	if (DoesStringEndWith(FileName, PNG_EXTENSION) || DoesStringEndWith(FileName, B25S_EXTENSION))
	{
		bool Result;
		BS_GLImage * pImage = new BS_GLImage(FileName, Result);
		if (!Result)
		{
			delete pImage;
			return 0;
		}

		BS_BitmapResource * pResource = new BS_BitmapResource(FileName, pImage);
		if (!pResource->IsValid())
		{
			delete pResource;
			return 0;
		}

		return pResource;
	}


	// Vectorgraphik laden
	if (DoesStringEndWith(FileName, SWF_EXTENSION))
	{
		// Pointer auf Package-Manager holen
		BS_PackageManager * pPackage = BS_Kernel::GetInstance()->GetPackage();
		BS_ASSERT(pPackage);

		// Datei laden
		unsigned char* pFileData;
		unsigned int FileSize;
		if (!(pFileData = static_cast<unsigned char*>(pPackage->GetFile(FileName, &FileSize))))
		{
			BS_LOG_ERRORLN("File \"%s\" could not be loaded.", FileName.c_str());
			return 0;
		}

		bool Result;
		BS_VectorImage * pImage = new BS_VectorImage(pFileData, FileSize, Result);
		if (!Result)
		{
			delete pImage;
			delete [] pFileData;
			return 0;
		}

		BS_BitmapResource * pResource = new BS_BitmapResource(FileName, pImage);
		if (!pResource->IsValid())
		{
			delete pResource;
			delete [] pFileData;
			return 0;
		}

		delete [] pFileData;
		return pResource;
	}

	// Animation laden
	if (DoesStringEndWith(FileName, ANI_EXTENSION))
	{
		BS_AnimationResource * pResource = new BS_AnimationResource(FileName);
		if (pResource->IsValid())
			return pResource;
		else
		{
			delete pResource;
			return 0;
		}
	}

	// Font laden
	if (DoesStringEndWith(FileName, FNT_EXTENSION))
	{
		BS_FontResource * pResource = new BS_FontResource(BS_Kernel::GetInstance(), FileName);
		if (pResource->IsValid())
			return pResource;
		else
		{
			delete pResource;
			return 0;
		}
	}

	BS_LOG_ERRORLN("Service cannot load \"%s\".", FileName.c_str());
	return 0;
}

// -----------------------------------------------------------------------------

bool BS_OpenGLGfx::CanLoadResource(const std::string& FileName)
{
	return DoesStringEndWith(FileName, PNG_EXTENSION) ||
		   DoesStringEndWith(FileName, ANI_EXTENSION) ||
		   DoesStringEndWith(FileName, FNT_EXTENSION) ||
		   DoesStringEndWith(FileName, SWF_EXTENSION) ||
		   DoesStringEndWith(FileName, B25S_EXTENSION);
}


// -----------------------------------------------------------------------------
// DEBUGGING
// -----------------------------------------------------------------------------

void BS_OpenGLGfx::DrawDebugLine(const BS_Vertex & Start, const BS_Vertex & End, unsigned int Color)
{
	m_DebugLines.push_back(DebugLine(Start, End, Color));
}

// -----------------------------------------------------------------------------
// PERSISTENZ
// -----------------------------------------------------------------------------

bool BS_OpenGLGfx::Persist(BS_OutputPersistenceBlock & Writer)
{
	bool result = true;

	result &= BS_GraphicEngine::Persist(Writer);
	result &= m_RenderObjectManagerPtr->Persist(Writer);

	return result;
}

// -----------------------------------------------------------------------------

bool BS_OpenGLGfx::Unpersist(BS_InputPersistenceBlock & Reader)
{
	bool result = true;

	result &= BS_GraphicEngine::Unpersist(Reader);
	result &= m_RenderObjectManagerPtr->Unpersist(Reader);

	return result && Reader.IsGood();
}
