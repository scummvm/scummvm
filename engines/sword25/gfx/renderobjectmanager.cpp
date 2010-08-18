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

#include "sword25/gfx/renderobjectmanager.h"

#include "sword25/kernel/kernel.h"
#include "sword25/kernel/inputpersistenceblock.h"
#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/gfx/graphicengine.h"
#include "sword25/gfx/animationtemplateregistry.h"
#include "sword25/math/rect.h"
#include "sword25/gfx/renderobject.h"
#include "sword25/gfx/timedrenderobject.h"
#include "sword25/gfx/rootrenderobject.h"

namespace Sword25 {

#define BS_LOG_PREFIX "RENDEROBJECTMANAGER"

// -----------------------------------------------------------------------------
// Konstruktion / Desktruktion
// -----------------------------------------------------------------------------

RenderObjectManager::RenderObjectManager(int Width, int Height, int FramebufferCount) :
	m_FrameStarted(false) {
	// Wurzel des BS_RenderObject-Baumes erzeugen.
	m_RootPtr = (new RootRenderObject(this, Width, Height))->GetHandle();
}

// -----------------------------------------------------------------------------

RenderObjectManager::~RenderObjectManager() {
	// Die Wurzel des Baumes löschen, damit werden alle BS_RenderObjects mitgelöscht.
	m_RootPtr.Erase();
}

// -----------------------------------------------------------------------------
// Interface
// -----------------------------------------------------------------------------

void RenderObjectManager::StartFrame() {
	m_FrameStarted = true;

	// Verstrichene Zeit bestimmen
	int TimeElapsed = Kernel::GetInstance()->GetGfx()->GetLastFrameDurationMicro();

	// Alle BS_TimedRenderObject Objekte über den Framestart und die verstrichene Zeit in Kenntnis setzen
	RenderObjectList::iterator Iter = m_TimedRenderObjects.begin();
	for (; Iter != m_TimedRenderObjects.end(); ++Iter)
		(*Iter)->FrameNotification(TimeElapsed);
}

// -----------------------------------------------------------------------------

bool RenderObjectManager::Render() {
	// Den Objekt-Status des Wurzelobjektes aktualisieren. Dadurch werden rekursiv alle Baumelemente aktualisiert.
	// Beim aktualisieren des Objekt-Status werden auch die Update-Rects gefunden, so dass feststeht, was neu gezeichnet
	// werden muss.
	if (!m_RootPtr.IsValid() || !m_RootPtr->UpdateObjectState()) return false;

	m_FrameStarted = false;

	// Die Render-Methode der Wurzel aufrufen. Dadurch wird das rekursive Rendern der Baumelemente angestoßen.
	return m_RootPtr->Render();
}

// -----------------------------------------------------------------------------

void RenderObjectManager::AttatchTimedRenderObject(RenderObjectPtr<TimedRenderObject> RenderObjectPtr) {
	m_TimedRenderObjects.push_back(RenderObjectPtr);
}

// -----------------------------------------------------------------------------

void RenderObjectManager::DetatchTimedRenderObject(RenderObjectPtr<TimedRenderObject> RenderObjectPtr) {
	for (uint i = 0; i < m_TimedRenderObjects.size(); i++)
		if (m_TimedRenderObjects[i] == RenderObjectPtr) {
			m_TimedRenderObjects.remove_at(i);
			break;
		}
}

// -----------------------------------------------------------------------------
// Persistenz
// -----------------------------------------------------------------------------

bool RenderObjectManager::Persist(OutputPersistenceBlock &Writer) {
	bool Result = true;

	// Alle Kinder des Wurzelknotens speichern. Dadurch werden alle BS_RenderObjects gespeichert rekursiv gespeichert.
	Result &= m_RootPtr->PersistChildren(Writer);

	Writer.Write(m_FrameStarted);

	// Referenzen auf die TimedRenderObjects persistieren.
	Writer.Write(m_TimedRenderObjects.size());
	RenderObjectList::const_iterator Iter = m_TimedRenderObjects.begin();
	while (Iter != m_TimedRenderObjects.end()) {
		Writer.Write((*Iter)->GetHandle());
		++Iter;
	}

	// Alle BS_AnimationTemplates persistieren.
	Result &= AnimationTemplateRegistry::GetInstance().Persist(Writer);

	return Result;
}

// -----------------------------------------------------------------------------

bool RenderObjectManager::Unpersist(InputPersistenceBlock &Reader) {
	bool Result = true;

	// Alle Kinder des Wurzelknotens löschen. Damit werden alle BS_RenderObjects gelöscht.
	m_RootPtr->DeleteAllChildren();

	// Alle BS_RenderObjects wieder hestellen.
	if (!m_RootPtr->UnpersistChildren(Reader)) return false;

	Reader.Read(m_FrameStarted);

	// Momentan gespeicherte Referenzen auf TimedRenderObjects löschen.
	m_TimedRenderObjects.resize(0);

	// Referenzen auf die TimedRenderObjects wieder herstellen.
	unsigned int TimedObjectCount;
	Reader.Read(TimedObjectCount);
	for (unsigned int i = 0; i < TimedObjectCount; ++i) {
		unsigned int Handle;
		Reader.Read(Handle);
		m_TimedRenderObjects.push_back(Handle);
	}

	// Alle BS_AnimationTemplates wieder herstellen.
	Result &= AnimationTemplateRegistry::GetInstance().Unpersist(Reader);

	return Result;
}

} // End of namespace Sword25
