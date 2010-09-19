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

#define BS_LOG_PREFIX "GRAPHICENGINE"

#include "common/system.h"

#include "sword25/gfx/bitmapresource.h"
#include "sword25/gfx/animationresource.h"
#include "sword25/gfx/fontresource.h"
#include "sword25/gfx/panel.h"
#include "sword25/gfx/renderobjectmanager.h"
#include "sword25/gfx/screenshot.h"
#include "sword25/gfx/image/renderedimage.h"
#include "sword25/gfx/image/swimage.h"
#include "sword25/gfx/image/vectorimage.h"
#include "sword25/package/packagemanager.h"
#include "sword25/kernel/inputpersistenceblock.h"
#include "sword25/kernel/outputpersistenceblock.h"


#include "sword25/gfx/graphicengine.h"

namespace Lua {
extern "C"
{
#include "sword25/util/lua/lua.h"
#include "sword25/util/lua/lauxlib.h"
}
}

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


namespace Sword25 {

using namespace Lua;

static const uint FRAMETIME_SAMPLE_COUNT = 5;       // Anzahl der Framezeiten über die, die Framezeit gemittelt wird

GraphicEngine::GraphicEngine(Kernel *pKernel) :
	m_Width(0),
	m_Height(0),
	m_BitDepth(0),
	m_Windowed(0),
	m_LastTimeStamp((uint64) - 1), // max. BS_INT64 um beim ersten Aufruf von _UpdateLastFrameDuration() einen Reset zu erzwingen
	m_LastFrameDuration(0),
	m_TimerActive(true),
	m_FrameTimeSampleSlot(0),
	m_RepaintedPixels(0),
	_thumbnail(NULL),
	ResourceService(pKernel) {
	m_FrameTimeSamples.resize(FRAMETIME_SAMPLE_COUNT);

	if (!RegisterScriptBindings())
		BS_LOG_ERRORLN("Script bindings could not be registered.");
	else
		BS_LOGLN("Script bindings registered.");
}

GraphicEngine::~GraphicEngine() {
	_backSurface.free();
	_frameBuffer.free();
	delete _thumbnail;
}

Service *GraphicEngine_CreateObject(Kernel *pKernel) {
	return new GraphicEngine(pKernel);
}

bool GraphicEngine::Init(int Width, int Height, int BitDepth, int BackbufferCount, bool Windowed) {
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
	_frameBuffer.create(Width, Height, 4);

	// Standardmäßig ist Vsync an.
	SetVsync(true);

	// Layer-Manager initialisieren.
	_renderObjectManagerPtr.reset(new RenderObjectManager(Width, Height, BackbufferCount + 1));

	// Hauptpanel erstellen
	m_MainPanelPtr = _renderObjectManagerPtr->getTreeRoot()->addPanel(Width, Height, BS_ARGB(0, 0, 0, 0));
	if (!m_MainPanelPtr.isValid()) return false;
	m_MainPanelPtr->setVisible(true);

	return true;
}

// -----------------------------------------------------------------------------

bool GraphicEngine::StartFrame(bool UpdateAll) {
	// Berechnen, wie viel Zeit seit dem letzten Frame vergangen ist.
	// Dieser Wert kann über GetLastFrameDuration() von Modulen abgefragt werden, die zeitabhängig arbeiten.
	UpdateLastFrameDuration();

	// Den Layer-Manager auf den nächsten Frame vorbereiten
	_renderObjectManagerPtr->startFrame();

	return true;
}

// -----------------------------------------------------------------------------

bool GraphicEngine::EndFrame() {
	// Scene zeichnen
	_renderObjectManagerPtr->render();

	// FIXME: The frame buffer surface is only used as the base for creating thumbnails when saving the
	// game, since the _backSurface is blanked. Currently I'm doing a slightly hacky check and only
	// copying the back surface if line 50 (the first line after the interface area) is non-blank
	if (READ_LE_UINT32((byte *)_backSurface.pixels + (_backSurface.pitch * 50)) & 0xffffff) {
		// Make a copy of the current frame into the frame buffer
		Common::copy((byte *)_backSurface.pixels, (byte *)_backSurface.pixels + 
			(_backSurface.pitch * _backSurface.h), (byte *)_frameBuffer.pixels); 
	}

	g_system->updateScreen();

	// Debug-Lines zeichnen
	if (!m_DebugLines.empty()) {
#if 0
		glEnable(GL_LINE_SMOOTH);
		glBegin(GL_LINES);

		Common::Array<DebugLine>::const_iterator iter = m_DebugLines.begin();
		for (; iter != m_DebugLines.end(); ++iter) {
			const uint &Color = (*iter).Color;
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

RenderObjectPtr<Panel> GraphicEngine::GetMainPanel() {
	return m_MainPanelPtr;
}

// -----------------------------------------------------------------------------

void GraphicEngine::SetVsync(bool Vsync) {
	warning("STUB: SetVsync(%d)", Vsync);
}

// -----------------------------------------------------------------------------

bool GraphicEngine::GetVsync() const {
	warning("STUB: GetVsync()");

	return true;
}

// -----------------------------------------------------------------------------

bool GraphicEngine::fill(const Common::Rect *fillRectPtr, uint color) {
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

Graphics::Surface *GraphicEngine::GetScreenshot() {
	return &_frameBuffer;
}

// -----------------------------------------------------------------------------
// RESOURCE MANAGING
// -----------------------------------------------------------------------------

Resource *GraphicEngine::LoadResource(const Common::String &FileName) {
	BS_ASSERT(CanLoadResource(FileName));

	// Bild für den Softwarebuffer laden
	if (FileName.hasSuffix(PNG_S_EXTENSION)) {
		bool Result = false;
		SWImage *pImage = new SWImage(FileName, Result);
		if (!Result) {
			delete pImage;
			return 0;
		}

		BitmapResource *pResource = new BitmapResource(FileName, pImage);
		if (!pResource->isValid()) {
			delete pResource;
			return 0;
		}

		return pResource;
	}

	// Sprite-Bild laden
	if (FileName.hasSuffix(PNG_EXTENSION) || FileName.hasSuffix(B25S_EXTENSION)) {
		bool Result = false;
		RenderedImage *pImage = new RenderedImage(FileName, Result);
		if (!Result) {
			delete pImage;
			return 0;
		}

		BitmapResource *pResource = new BitmapResource(FileName, pImage);
		if (!pResource->isValid()) {
			delete pResource;
			return 0;
		}

		return pResource;
	}


	// Vectorgraphik laden
	if (FileName.hasSuffix(SWF_EXTENSION)) {
		debug(2, "VectorImage: %s", FileName.c_str());

		// Pointer auf Package-Manager holen
		PackageManager *pPackage = Kernel::GetInstance()->GetPackage();
		BS_ASSERT(pPackage);

		// Datei laden
		byte *pFileData;
		uint FileSize;
		if (!(pFileData = static_cast<byte *>(pPackage->GetFile(FileName, &FileSize)))) {
			BS_LOG_ERRORLN("File \"%s\" could not be loaded.", FileName.c_str());
			return 0;
		}

		bool Result = false;
		VectorImage *pImage = new VectorImage(pFileData, FileSize, Result, FileName);
		if (!Result) {
			delete pImage;
			delete [] pFileData;
			return 0;
		}

		BitmapResource *pResource = new BitmapResource(FileName, pImage);
		if (!pResource->isValid()) {
			delete pResource;
			delete[] pFileData;
			return 0;
		}

		delete[] pFileData;
		return pResource;
	}

	// Animation laden
	if (FileName.hasSuffix(ANI_EXTENSION)) {
		AnimationResource *pResource = new AnimationResource(FileName);
		if (pResource->isValid())
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

bool GraphicEngine::CanLoadResource(const Common::String &FileName) {
	return FileName.hasSuffix(PNG_EXTENSION) ||
		FileName.hasSuffix(ANI_EXTENSION) ||
		FileName.hasSuffix(FNT_EXTENSION) ||
		FileName.hasSuffix(SWF_EXTENSION) ||
		FileName.hasSuffix(B25S_EXTENSION);
}


// -----------------------------------------------------------------------------
// DEBUGGING
// -----------------------------------------------------------------------------

void GraphicEngine::DrawDebugLine(const Vertex &Start, const Vertex &End, uint Color) {
	m_DebugLines.push_back(DebugLine(Start, End, Color));
}

void  GraphicEngine::UpdateLastFrameDuration() {
	// Aktuelle Zeit holen
	uint64_t CurrentTime = Kernel::GetInstance()->GetMicroTicks();

	// Verstrichene Zeit seit letztem Frame berechnen und zu große Zeitsprünge ( > 250 msek.) unterbinden
	// (kann vorkommen bei geladenen Spielständen, während des Debuggings oder Hardwareungenauigkeiten)
	m_FrameTimeSamples[m_FrameTimeSampleSlot] = static_cast<uint>(CurrentTime - m_LastTimeStamp);
	if (m_FrameTimeSamples[m_FrameTimeSampleSlot] > 250000) m_FrameTimeSamples[m_FrameTimeSampleSlot] = 250000;
	m_FrameTimeSampleSlot = (m_FrameTimeSampleSlot + 1) % FRAMETIME_SAMPLE_COUNT;

	// Die Framezeit wird über mehrere Frames gemittelt um Ausreisser zu eliminieren
	Common::Array<uint>::const_iterator it = m_FrameTimeSamples.begin();
	uint Sum = *it;
	for (it++; it != m_FrameTimeSamples.end(); it++) Sum += *it;
	m_LastFrameDuration = Sum / FRAMETIME_SAMPLE_COUNT;

	// _LastTimeStamp auf die Zeit des aktuellen Frames setzen
	m_LastTimeStamp = CurrentTime;
}

namespace {
bool DoSaveScreenshot(GraphicEngine &GraphicEngine, const Common::String &Filename) {
	Graphics::Surface *data = GraphicEngine.GetScreenshot();
	if (!data) {
		BS_LOG_ERRORLN("Call to GetScreenshot() failed. Cannot save screenshot.");
		return false;
	}

	Common::FSNode f(Filename);
	Common::WriteStream *stream = f.createWriteStream();
	if (!stream) {
		BS_LOG_ERRORLN("Call to GetScreenshot() failed. Cannot save screenshot.");
		return false;
	}

	bool result = Screenshot::SaveToFile(data, stream);
	delete stream;

	return result;
}
}

bool GraphicEngine::SaveScreenshot(const Common::String &Filename) {
	return DoSaveScreenshot(*this, Filename);
}

bool GraphicEngine::SaveThumbnailScreenshot(const Common::String &Filename) {
	// Note: In ScumMVM, rather than saivng the thumbnail to a file, we store it in memory 
	// until needed when creating savegame files
	delete _thumbnail;
	_thumbnail = Screenshot::createThumbnail(&_frameBuffer);
	return true;
}

void GraphicEngine::ARGBColorToLuaColor(lua_State *L, uint Color) {
	lua_Number Components[4] = {
		(Color >> 16) & 0xff,   // Rot
		(Color >> 8) & 0xff,    // Grün
		Color & 0xff,          // Blau
		Color >> 24,           // Alpha
	};

	lua_newtable(L);

	for (uint i = 1; i <= 4; i++) {
		lua_pushnumber(L, i);
		lua_pushnumber(L, Components[i - 1]);
		lua_settable(L, -3);
	}
}

uint GraphicEngine::LuaColorToARGBColor(lua_State *L, int StackIndex) {
#ifdef DEBUG
	int __startStackDepth = lua_gettop(L);
#endif

	// Sicherstellen, dass wir wirklich eine Tabelle betrachten
	luaL_checktype(L, StackIndex, LUA_TTABLE);
	// Größe der Tabelle auslesen
	uint n = luaL_getn(L, StackIndex);
	// RGB oder RGBA Farben werden unterstützt und sonst keine
	if (n != 3 && n != 4) luaL_argcheck(L, 0, StackIndex, "at least 3 of the 4 color components have to be specified");

	// Rote Farbkomponente auslesen
	lua_rawgeti(L, StackIndex, 1);
	uint Red = static_cast<uint>(lua_tonumber(L, -1));
	if (!lua_isnumber(L, -1) || Red >= 256) luaL_argcheck(L, 0, StackIndex, "red color component must be an integer between 0 and 255");
	lua_pop(L, 1);

	// Grüne Farbkomponente auslesen
	lua_rawgeti(L, StackIndex, 2);
	uint Green = static_cast<uint>(lua_tonumber(L, -1));
	if (!lua_isnumber(L, -1) || Green >= 256) luaL_argcheck(L, 0, StackIndex, "green color component must be an integer between 0 and 255");
	lua_pop(L, 1);

	// Blaue Farbkomponente auslesen
	lua_rawgeti(L, StackIndex, 3);
	uint Blue = static_cast<uint>(lua_tonumber(L, -1));
	if (!lua_isnumber(L, -1) || Blue >= 256) luaL_argcheck(L, 0, StackIndex, "blue color component must be an integer between 0 and 255");
	lua_pop(L, 1);

	// Alpha Farbkomponente auslesen
	uint Alpha = 0xff;
	if (n == 4) {
		lua_rawgeti(L, StackIndex, 4);
		Alpha = static_cast<uint>(lua_tonumber(L, -1));
		if (!lua_isnumber(L, -1) || Alpha >= 256) luaL_argcheck(L, 0, StackIndex, "alpha color component must be an integer between 0 and 255");
		lua_pop(L, 1);
	}

#ifdef DEBUG
	BS_ASSERT(__startStackDepth == lua_gettop(L));
#endif

	return (Alpha << 24) | (Red << 16) | (Green << 8) | Blue;
}

bool GraphicEngine::persist(OutputPersistenceBlock &writer) {
	writer.write(m_TimerActive);

	bool result = _renderObjectManagerPtr->persist(writer);

	return result;
}

bool GraphicEngine::unpersist(InputPersistenceBlock &reader) {
	reader.read(m_TimerActive);
	_renderObjectManagerPtr->unpersist(reader);

	return reader.isGood();
}

} // End of namespace Sword25
